/* XDGSearch is a XAPIAN based file indexer and search tool.

    Copyright (C) 2016,2017,2018  Franco Martelli

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
// #include <iostream>

Helpers::Helpers(QWidget *parent) :
      QDialog(parent)
    , ui(new Ui::Helpers)
    , conf(std::unique_ptr<XDGSearch::Configuration>(new XDGSearch::Configuration))
    , selectedHelper("")
{
    ui->setupUi(this);  /// prepares the UI
    buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);     /// bind a pointer to buttonbox OK button

    /// 4 connect to call proper member function (slot) when "clicked" signal is raised
    QObject::connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(clicked_buttonBoxCancel()));
    QObject::connect(buttonOk, SIGNAL(clicked()), SLOT(clicked_buttonBoxOk()));

    buttonOk->setText(QString(QObject::trUtf8("Select")));
    buttonOk->setEnabled(false);        /// shows a disabled button

    refreshHelpersList();               /// shows a list of helpers
}

Helpers::~Helpers()
{
    delete ui;
}

void Helpers::clicked_buttonBoxCancel()
{
    this->close();
}

void Helpers::clicked_buttonBoxOk()     /// store selected helper name into class member variable then close this dialog
{
    if(ui->helpersList->currentItem()->isSelected())
        selectedHelper = ui->helpersList->currentItem()->text();
    else
        return;
    this->close();
}
void Helpers::refreshHelpersList() const /// retrieves helper's names from the .conf file then they'll be added to the helperList widget
{
    const QStringList helpersNameList = conf ->getHelpersNameList();

    ui->helpersList->clearSelection();
    ui->helpersList->clear();

    for(const auto& s : helpersNameList)
        ui->helpersList->addItem(s);
}

void Helpers::on_helpersList_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    buttonOk->clicked();
}

void Helpers::on_helpersList_itemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    buttonOk->setEnabled(true);
}
