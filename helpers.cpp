/* XDGSearch is a XAPIAN based file indexer and search tool.

    Copyright (C) 2016  Franco Martelli

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "helpers.h"
#include "ui_helpers.h"
#include <iostream>

Helpers::Helpers(QWidget *parent) :
      QDialog(parent)
    , ui(new Ui::Helpers)
    , conf(std::unique_ptr<XDGSearch::Configuration>(new XDGSearch::Configuration))
    , isNameAdding(false)
    , granularityEdited(false)
    , newItemAdded(nullptr)
{
    ui->setupUi(this);  // prepares the UI
    buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);     // bind a pointer to buttonbox OK button
    buttonApply = ui->buttonBox->button(QDialogButtonBox::Apply);   // bind a pointer to buttonbox Apply button
    // 4 connect to call proper member function (slot) when "clicked" signal is raised
    QObject::connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), SLOT(clicked_buttonBoxRestoreDefault()));
    QObject::connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(clicked_buttonBoxCancel()));
    QObject::connect(buttonOk, SIGNAL(clicked()), SLOT(clicked_buttonBoxOk()));
    QObject::connect(buttonApply, SIGNAL(clicked()), SLOT(clicked_buttonBoxApply()));

    buttonOk->setEnabled(false);        // shows a disabled button
    buttonApply->setEnabled(false);     // shows a disabled button

    refreshHelpersList();               // shows a list of helpers
    // define a tabbing path order, suitable for editing purpose
    QWidget::setTabOrder(ui->helperName, ui->helperCmdLine);
    QWidget::setTabOrder(ui->helperCmdLine, ui->helperFileExt);
    QWidget::setTabOrder(ui->helperFileExt, ui->helperGranularity);
    //QWidget::setTabOrder(ui->helperGranularity, buttonOk);
}

Helpers::~Helpers()
{
    delete ui;
}

void Helpers::on_addHelper_clicked()    // prepares the UI to add a provided by the user helper
{
    if(ui->helpersList->isEnabled())    {
        ui->helpersList->setEnabled(false);
        buttonOk->setEnabled(false);
        buttonApply->setEnabled(false);
        isNameAdding = true;
        granularityEdited = false;

        ui->helperName->setText(QString("<new>"));
        ui->helperName->selectAll();        // the text <new> is shown as selected
        ui->helperName->setFocus();         // move the focus to helperName so the user can start typing now
        // erase contents of 3 UI fields, the user shall fill them
        ui->helperCmdLine->clear(); ui->helperFileExt->clear(); ui->helperGranularity->clear();
    }
}

void Helpers::on_removeHelper_clicked()     // irreversibly erase selected helper
{
    if(  ui->helpersList->currentItem()->isSelected()
      && ui->helpersList->isEnabled() )
    {
        conf ->removeHelper(ui->helpersList->currentItem()->text().toStdString());
        ui->helpersList->takeItem(ui->helpersList->currentRow());
    }
}

void Helpers::clicked_buttonBoxCancel()
{
    this->close();
}

void Helpers::clicked_buttonBoxOk()     // store selected helper name into class member variable then close this dialog
{
    if(ui->helpersList->currentItem()->isSelected())
        selectedHelper = ui->helpersList->currentItem()->text();
    else
        return;

    this->close();
}

void Helpers::clicked_buttonBoxApply()  // write helper's changes to .conf file then enable/disable dialog's widget
{
    XDGSearch::helperType htItem;
    std::get<0>(htItem) = ui->helperName->text().toStdString();
    std::get<1>(htItem) = ui->helperFileExt->text().toStdString();
    std::get<2>(htItem) = ui->helperCmdLine->text().toStdString();
    std::get<3>(htItem) = ui->helperGranularity->value();

    conf ->writeSettings(htItem);
    buttonOk->setEnabled(true);
    ui->helpersList->setEnabled(true);
    buttonApply->setEnabled(false);
}

void Helpers::clicked_buttonBoxRestoreDefault() // restore initial helpers default setting
{
    conf ->defaultSettings("");
    refreshHelpersList();
}

void Helpers::on_helperName_editingFinished()   // once assigned an helper name puts it into the helpersList widget
{
    if(isNameAdding)    {
        isNameAdding = false;
        newItemAdded = new QListWidgetItem;
        newItemAdded->setText(ui->helperName->text());
        ui->helpersList->addItem(newItemAdded);
        ui->helpersList->setCurrentItem(newItemAdded);
    }
    if(ui->helpersList->currentItem())
        ui->helpersList->currentItem()->setText(ui->helperName->text());
    toggleWidgetOnEditing();
}

void Helpers::on_helperCmdLine_editingFinished()
{
    toggleWidgetOnEditing();
}

void Helpers::on_helperFileExt_editingFinished()
{
    toggleWidgetOnEditing();
}

void Helpers::on_helperGranularity_editingFinished()
{
    granularityEdited = true;
    toggleWidgetOnEditing();
}

void Helpers::on_helpersList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{   // when current item changed retrieves helper's item values and assign them to the ui widgets
    Q_UNUSED(previous)
    XDGSearch::helperType htItem;

    if(current)
        htItem  = conf ->enqueryHelper(current->text().toStdString());

    if(std::get<0>(htItem).empty())
        return;

    ui->helperName->setText(QString::fromStdString(std::get<0>(htItem)));
    ui->helperFileExt->setText(QString::fromStdString(std::get<1>(htItem)));
    ui->helperCmdLine->setText(QString::fromStdString(std::get<2>(htItem)));
    ui->helperGranularity->setValue(std::get<3>(htItem));
    buttonOk->setEnabled(true);
}

void Helpers::toggleWidgetOnEditing()   // when a field is edited accordingly sets buttons and list
{
    if(  !ui->helperName->text().isEmpty()
      && !ui->helperCmdLine->text().isEmpty()
      && !ui->helperFileExt->text().isEmpty()
      && granularityEdited )
    {
        buttonApply->setEnabled(true);
    }
    else
    {
        buttonOk->setEnabled(false);
        buttonApply->setEnabled(false);
        granularityEdited = false;
    }
    ui->helpersList->setEnabled(false);
}

void Helpers::refreshHelpersList()  // retrieves helper's names from the .conf file then they'll be added to the helperList widget
{
    const QStringList helpersNameList = conf ->getHelpersNameList();

    ui->helpersList->clearSelection();
    ui->helpersList->clear();

    for(const auto& s : helpersNameList)
        ui->helpersList->addItem(s);
}
