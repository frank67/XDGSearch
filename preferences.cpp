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

#include <xapian.h>
#include "preferences.h"
#include "ui_preferences.h"
#include "helpers.h"
#include <sstream>
#include <QDirIterator>
#include <QFileDialog>
#include <QMessageBox>


Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences),
    changesAlreadyApplied(false)
{
    ui->setupUi(this);

    fillPageInitValue();
    buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    buttonApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    buttonOk->setEnabled(false);    buttonApply->setEnabled(false);

    QObject::connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(clicked_buttonBoxCancel()));
    QObject::connect(buttonOk, SIGNAL(clicked()), SLOT(clicked_buttonBoxOk()));
    QObject::connect(buttonApply, SIGNAL(clicked()), SLOT(clicked_buttonBoxApply()));

    QWidget::setTabOrder(ui->poolCBox, ui->poolDirName);
    QWidget::setTabOrder(ui->poolDirName, ui->poolDirButton);
    QWidget::setTabOrder(ui->poolDirButton, ui->stemCBox);
    QWidget::setTabOrder(ui->stemCBox, ui->stopwordsCBox);
    QWidget::setTabOrder(ui->stopwordsCBox, ui->helpersList);
    QWidget::setTabOrder(ui->helpersList, ui->addHelper);
    QWidget::setTabOrder(ui->addHelper, ui->removeHelper);
    QWidget::setTabOrder(ui->removeHelper, ui->buttonBox);
    QWidget::setTabOrder(ui->buttonBox, ui->poolCBox);
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::fillPageInitValue()
{
    const std::string stemLanguages = "none " + Xapian::Stem::get_available_languages();
    std::istringstream iss(stemLanguages);
    for(std::string s; iss >> s; /* null */)
        ui ->stemCBox->addItem(QString::fromStdString(s));

    QDirIterator dirIt("stopwords", QDir::Files);

    ui ->stopwordsCBox->addItem("none");
    while(dirIt.hasNext())
        ui ->stopwordsCBox->addItem(dirIt.fileName());

    for(auto p = XDGSearch::Pool::DESKTOP; p != XDGSearch::Pool::END; ++p)   {
        XDGSearch::Configuration conf(p);
        auto pt = conf.enqueryPool();
        if(!std::get<1>(pt).empty())
            ui ->poolCBox ->addItem(QString::fromStdString(std::get<1>(pt)), QVariant::fromValue(p));

        if(p == XDGSearch::Pool::DESKTOP)   {
            ui ->poolDirName->setText(QString::fromStdString(std::get<3>(pt)));
            refreshHelpersList();

            ui ->stemCBox->setCurrentText(QString::fromStdString(std::get<4>(pt)));
            if(std::get<5>(pt).empty())
                ui ->stopwordsCBox->setCurrentText(QString("none"));
            else
                ui ->stopwordsCBox->setCurrentText(QString::fromStdString(std::get<5>(pt)));
        }
    }
}

void Preferences::on_poolDirButton_clicked()
{
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->poolDirName->setText(directory);
    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::on_addHelper_clicked()
{
    Helpers hlp;
    if(! (hlp.exec() == QDialog::Accepted))
        return;

    const QString helperToAdd = hlp.getHelperName();
    if((ui->helpersList->findItems(helperToAdd, Qt::MatchFixedString)).empty())
        ui->helpersList->addItem(helperToAdd);

    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::on_removeHelper_clicked()
{
    QListWidgetItem* item = ui->helpersList->currentItem();
    if(!item->isSelected()) {
        QMessageBox* mb = new QMessageBox(  QMessageBox::Warning
                                          , QCoreApplication::applicationName()
                                          , QObject::trUtf8("Please select an helper item in the list view.")
                                          , QMessageBox::StandardButton::Ok
                                          , this);
        mb->exec();
        delete mb;
        return;
    } else
        if(ui->helpersList->count() != 1)
            ui->helpersList->takeItem(ui->helpersList->currentRow());

    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::clicked_buttonBoxCancel()
{
    this->close();
}

void Preferences::clicked_buttonBoxOk()
{
    if(!changesAlreadyApplied)
    {
        XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());
        auto pt = collectFieldsValue(conf);
        conf.writeSettings(pt);
    }
    this->close();
}

void Preferences::clicked_buttonBoxApply()
{
    XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());
    auto pt = collectFieldsValue(conf);
    conf.writeSettings(pt);

    buttonOk->setEnabled(true);
    changesAlreadyApplied = true;
}

XDGSearch::poolType Preferences::collectFieldsValue(const XDGSearch::Configuration& cfg)
{
    XDGSearch::poolType retval = cfg.enqueryPool();
    std::get<3>(retval) = ui->poolDirName->text().toStdString();

    std::get<2>(retval).clear();
    for(int i = 0; ui->helpersList->item(i) != 0; ++i)    {
        if(!std::get<2>(retval).empty())
            std::get<2>(retval) += ',';
        std::get<2>(retval) += ui->helpersList->item(i)->text().toStdString();
    }

    std::get<4>(retval) = ui->stemCBox->currentText().toStdString();
    std::get<5>(retval) = ui->stopwordsCBox->currentText().toStdString();

    return retval;
}

void Preferences::on_stemCBox_activated(int index)
{
    Q_UNUSED(index)
    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::on_stopwordsCBox_activated(int index)
{
    Q_UNUSED(index)
    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::on_poolCBox_activated(const QString& arg1)
{
    Q_UNUSED(arg1)
    XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());
    auto pt = conf.enqueryPool();

    ui ->poolDirName->setText(QString::fromStdString(std::get<3>(pt)));

    std::istringstream iss(std::get<2>(pt));
    ui->helpersList->clear();
    for(std::string s; std::getline(iss, s, ','); /* null */)
        ui->helpersList->addItem(QString::fromStdString(s));

    ui->helpersList->setCurrentRow(0);

    ui ->stemCBox->setCurrentText(QString::fromStdString(std::get<4>(pt)));
    if(std::get<5>(pt).empty())
        ui ->stopwordsCBox->setCurrentText(QString("none"));
    else
        ui ->stopwordsCBox->setCurrentText(QString::fromStdString(std::get<5>(pt)));

    buttonOk->setEnabled(true);    buttonApply->setEnabled(false);   changesAlreadyApplied = false;
}

void Preferences::refreshHelpersList()
{
    ui->helpersList->clear();
    XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());
    auto pt = conf.enqueryPool();

    std::istringstream iss(std::get<2>(pt));
    for(std::string s; std::getline(iss, s, ','); /* null */)
        ui->helpersList->addItem(QString::fromStdString(s));

    ui->helpersList->setCurrentRow(0);
}
