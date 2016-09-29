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
    QDialog(parent),
    ui(new Ui::Helpers),
    isNameAdding(false),
    granularityEdited(false),
    newItemAdded(nullptr)
{
    ui->setupUi(this);
    buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    buttonApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    QObject::connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), SLOT(clicked_buttonBoxRestoreDefault()));
    QObject::connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(clicked_buttonBoxCancel()));
    QObject::connect(buttonOk, SIGNAL(clicked()), SLOT(clicked_buttonBoxOk()));
    QObject::connect(buttonApply, SIGNAL(clicked()), SLOT(clicked_buttonBoxApply()));
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);

    refreshHelpersList();

    QWidget::setTabOrder(ui->helperName, ui->helperCmdLine);
    QWidget::setTabOrder(ui->helperCmdLine, ui->helperFileExt);
    QWidget::setTabOrder(ui->helperFileExt, ui->helperGranularity);
    //QWidget::setTabOrder(ui->helperGranularity, buttonOk);
}

Helpers::~Helpers()
{
    delete ui;
}

void Helpers::on_addHelper_clicked()
{
    if(ui->helpersList->isEnabled())    {
        ui->helpersList->setEnabled(false);
        buttonOk->setEnabled(false);
        buttonApply->setEnabled(false);
        isNameAdding = true;
        granularityEdited = false;

        ui->helperName->setText(QString("<new>"));
        ui->helperName->selectAll();
        ui->helperName->setFocus();
        ui->helperCmdLine->clear(); ui->helperFileExt->clear(); ui->helperGranularity->clear();
    }
}

void Helpers::on_removeHelper_clicked()
{
    if(  ui->helpersList->currentItem()->isSelected()
      && ui->helpersList->isEnabled() )
    {
        conf.removeHelper(ui->helpersList->currentItem()->text().toStdString());
        ui->helpersList->takeItem(ui->helpersList->currentRow());
    }
}

void Helpers::clicked_buttonBoxCancel()
{
    this->close();
}

void Helpers::clicked_buttonBoxOk()
{
    /*XDGSearch::helperType htItem;
    std::get<0>(htItem) = ui->helperName->text().toStdString();
    std::get<1>(htItem) = ui->helperFileExt->text().toStdString();
    std::get<2>(htItem) = ui->helperCmdLine->text().toStdString();
    std::get<3>(htItem) = ui->helperGranularity->value();
    conf.writeSettings(htItem);
*/
    if(ui->helpersList->currentItem()->isSelected())
        selectedHelper = ui->helpersList->currentItem()->text();
    else
        return;

    this->close();
}

void Helpers::clicked_buttonBoxApply()
{
    XDGSearch::helperType htItem;
    std::get<0>(htItem) = ui->helperName->text().toStdString();
    std::get<1>(htItem) = ui->helperFileExt->text().toStdString();
    std::get<2>(htItem) = ui->helperCmdLine->text().toStdString();
    std::get<3>(htItem) = ui->helperGranularity->value();

    conf.writeSettings(htItem);
    buttonOk->setEnabled(true);
    ui->helpersList->setEnabled(true);
    buttonApply->setEnabled(false);
}

void Helpers::clicked_buttonBoxRestoreDefault()
{
    conf.defaultSettings("");
    refreshHelpersList();
}

void Helpers::on_helperName_editingFinished()
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
    commitHelperChanges();
}

void Helpers::on_helperCmdLine_editingFinished()
{
    commitHelperChanges();
}

void Helpers::on_helperFileExt_editingFinished()
{
    commitHelperChanges();
}

void Helpers::on_helperGranularity_editingFinished()
{
    granularityEdited = true;
    commitHelperChanges();
}

void Helpers::on_helpersList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    XDGSearch::helperType htItem;
    if(current)
        htItem  = conf.enqueryHelper(current->text().toStdString());

    if(std::get<0>(htItem).empty())
        return;

    ui->helperName->setText(QString::fromStdString(std::get<0>(htItem)));
    ui->helperFileExt->setText(QString::fromStdString(std::get<1>(htItem)));
    ui->helperCmdLine->setText(QString::fromStdString(std::get<2>(htItem)));
    ui->helperGranularity->setValue(std::get<3>(htItem));
    buttonOk->setEnabled(true);

    if(previous)    {/*
        std::get<0>(htItem) = ui->helperName->text().toStdString();
        std::get<1>(htItem) = ui->helperFileExt->text().toStdString();
        std::get<2>(htItem) = ui->helperCmdLine->text().toStdString();
        std::get<3>(htItem) = ui->helperGranularity->value();

        conf.writeSettings(htItem);*/
    }
}

void Helpers::commitHelperChanges()
{
    if(  !ui->helperName->text().isEmpty()
      && !ui->helperCmdLine->text().isEmpty()
      && !ui->helperFileExt->text().isEmpty()
      && granularityEdited )
    {
        buttonApply->setEnabled(true);

        /*XDGSearch::helperType ht;
        std::get<0>(ht) = ui->helperName->text().toStdString();
        std::get<1>(ht) = ui->helperFileExt->text().toStdString();
        std::get<2>(ht) = ui->helperCmdLine->text().toStdString();
        std::get<3>(ht) = ui->helperGranularity->value();

        conf.writeSettings(ht);*/
    } else  {
        buttonApply->setEnabled(false);
        granularityEdited = false;
    }
    ui->helpersList->setEnabled(false);
}

void Helpers::refreshHelpersList()
{
    const QStringList helpersNameList = conf.getHelpersNameList();

    ui->helpersList->clearSelection();
    ui->helpersList->clear();

    for(const auto& s : helpersNameList)
        ui->helpersList->addItem(s);
}
