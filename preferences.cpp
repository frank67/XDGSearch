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
      QDialog(parent)
    , ui(new Ui::Preferences)
    , changesAlreadyApplied(false)
{
    ui->setupUi(this);  /// prepares the UI

    fillPageInitValue();
    buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    buttonApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    buttonOk->setEnabled(false);    buttonApply->setEnabled(false);

    /// binds clicked signal to the appropriate slot
    QObject::connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(clicked_buttonBoxCancel()));
    QObject::connect(buttonOk, SIGNAL(clicked()), SLOT(clicked_buttonBoxOk()));
    QObject::connect(buttonApply, SIGNAL(clicked()), SLOT(clicked_buttonBoxApply()));

    QWidget::setTabOrder(ui->poolCBox, ui->poolDirName);        /// 9 statements that establishes a tab path between widgets
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

void Preferences::fillPageInitValue() const
{
    const std::string stemLanguages = "none " + Xapian::Stem::get_available_languages();    /// retrieves all language's stem supported by XAPIAN
    std::istringstream iss(stemLanguages);
    for(std::string s; iss >> s; /* null */)
        ui ->stemCBox->addItem(QString::fromStdString(s));  /// populate the stemCBox widget

    QDirIterator dirIt(QString("./stopwords"), QDir::Files, QDirIterator::NoIteratorFlags);

    ui ->stopwordsCBox->addItem("none");        /// adds "none" as first stopwordsCBox widget item
    while(dirIt.hasNext())  {
        dirIt.next();
        ui ->stopwordsCBox->addItem(dirIt.fileName());  /// populate the stopwordsCBox widget
    }

    for(auto p = XDGSearch::Pool::DESKTOP; p != XDGSearch::Pool::END; ++p)   {
        XDGSearch::Configuration conf(p);
        auto pt = conf.enqueryPool();
        if(!std::get<XDGSearch::LOCALPOOLNAME>(pt).empty())
            ui ->poolCBox ->addItem(QString::fromStdString(std::get<XDGSearch::LOCALPOOLNAME>(pt)), QVariant::fromValue(p));   /// populate the poolCBox widget
        /// since the first pool is DESKTOP (see for() statements definition) populates the following widget fields
        if(p == XDGSearch::Pool::DESKTOP)   {
            ui ->poolDirName->setText(QString::fromStdString(std::get<XDGSearch::POOLDIRPATH>(pt))); /// populate with DESKTOP pool dirName
            refreshHelpersList();   /// populates helperList widget with DESKTOP pool helpers
            ui ->stemCBox->setCurrentText(QString::fromStdString(std::get<XDGSearch::STEMMING>(pt))); /// populate with DESKTOP pool stemmer
            ui ->stopwordsCBox->setCurrentText(QString::fromStdString(std::get<XDGSearch::STOPWORDSFILE>(pt)));    /// populate with DESKTOP pool stopwords file
        }
    }
}

void Preferences::on_poolDirButton_clicked()
{   /// allows the user to change pool's directory
    const QString directory = QFileDialog::getExistingDirectory(this
                            , QObject::trUtf8("Select directory")
                            , QDir::homePath()
                            , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if( !directory.isEmpty() )  {   /// if a directory was chosen from the previous dialog then updates widget
        ui ->poolDirName->setText(directory);
        buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
    }
}

void Preferences::on_addHelper_clicked()
{   /// allows the user to add an helper to the pool's helpers list
    Helpers hlp;
    if(! (hlp.exec() == QDialog::Accepted) )    /// shows the Helpers dialog window
        return;

    const QString helperToAdd = hlp.getHelperName();    /// OK was clicked therefore fetchs the helper's name
    if((ui->helpersList->findItems(helperToAdd, Qt::MatchFixedString)).empty()) /// findItems returns a QList if it is empty the helper wasn't already in the list
        ui->helpersList->addItem(helperToAdd);

    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::on_removeHelper_clicked()
{   /// allows the user to delete an helper from the list
    const QListWidgetItem* const item = ui->helpersList->currentItem();
    if( !item->isSelected() )   {   /// warns the user that a selection was necessary
        QMessageBox* mb = new QMessageBox(  QMessageBox::Warning
                                          , QCoreApplication::applicationName()
                                          , QObject::trUtf8("Please select an helper item in the list view.")
                                          , QMessageBox::StandardButton::Ok
                                          , this);
        mb->exec();
        delete mb;
        return;
    } else
        if(ui->helpersList->count() != 1)   /// avoids to empty all the list (at least one helper is necessary)
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
        const XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());    /// builds Configuration object using the current pool's poolCBox item
        const auto pt = collectWidgetValue(conf);     /// retrieves this window's fields value in order to save them into the .conf file
        conf.writeSettings(pt);
    }
    this->close();
}

void Preferences::clicked_buttonBoxApply()
{
    const XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());    /// builds Configuration object using the current pool's poolCBox item
    const auto pt = collectWidgetValue(conf); /// retrieves this window's fields value in order to save them into the .conf file
    conf.writeSettings(pt);

    buttonOk->setEnabled(true);
    changesAlreadyApplied = true;
}

const XDGSearch::poolType Preferences::collectWidgetValue(const XDGSearch::Configuration& cfg)
{   /// retrieves this window's fields value in order to save them into the .conf file
    XDGSearch::poolType retval = cfg.enqueryPool();
    std::get<XDGSearch::POOLDIRPATH>(retval) = ui->poolDirName->text().toStdString();

    std::get<XDGSearch::POOLHELPERS>(retval).clear();
    for(int i = 0; ui->helpersList->item(i) != 0; ++i)    { /// iteration to build an helpers comma separated list
        if(!std::get<XDGSearch::POOLHELPERS>(retval).empty())
            std::get<XDGSearch::POOLHELPERS>(retval) += ',';
        std::get<XDGSearch::POOLHELPERS>(retval) += ui->helpersList->item(i)->text().toStdString();
    }

    std::get<XDGSearch::STEMMING>(retval) = ui->stemCBox->currentText().toStdString();
    std::get<XDGSearch::STOPWORDSFILE>(retval) = ui->stopwordsCBox->currentText().toStdString();

    return retval;
}

void Preferences::on_stemCBox_activated(int index)
{
    Q_UNUSED(index) /// QT's macro to avoid compiler warning: unused parameter
    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::on_stopwordsCBox_activated(int index)
{
    Q_UNUSED(index)
    buttonOk->setEnabled(false);    buttonApply->setEnabled(true);   changesAlreadyApplied = false;
}

void Preferences::on_poolCBox_activated(const QString& arg1)
{   /// when the user choose an item of the comboBox then the window's fields will be updated
    Q_UNUSED(arg1)
    XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());
    const auto pt = conf.enqueryPool();

    ui ->poolDirName->setText(QString::fromStdString(std::get<XDGSearch::POOLDIRPATH>(pt))); /// updates the directory name field

    std::istringstream iss(std::get<XDGSearch::POOLHELPERS>(pt));
    ui->helpersList->clear();
    for(std::string s; std::getline(iss, s, ','); /* null */)
        ui->helpersList->addItem(QString::fromStdString(s));    /// adds items to the helpers list

    ui->helpersList->setCurrentRow(0);

    ui ->stemCBox->setCurrentText(QString::fromStdString(std::get<XDGSearch::STEMMING>(pt))); /// updates the stem ComboBox
    ui ->stopwordsCBox->setCurrentText(QString::fromStdString(std::get<XDGSearch::STOPWORDSFILE>(pt)));    /// updates the stopwords ComboBox

    buttonOk->setEnabled(true);    buttonApply->setEnabled(false);   changesAlreadyApplied = false;
}

void Preferences::refreshHelpersList() const
{   /// populates helperList widget with the selected item of the poolCBox ComboBox
    ui->helpersList->clear();
    XDGSearch::Configuration conf(ui->poolCBox->currentData().value<XDGSearch::Pool>());
    const auto pt = conf.enqueryPool();

    std::istringstream iss(std::get<XDGSearch::POOLHELPERS>(pt));
    for(std::string s; std::getline(iss, s, ','); /* null */)
        ui->helpersList->addItem(QString::fromStdString(s));    /// adds items to the helpers list

    ui->helpersList->setCurrentRow(0);
}
