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

#include "wizard.h"
#include "ui_wizard.h"
#include <QFileDialog>
#include "configuration.h"

Wizard::Wizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::Wizard)
{
    ui->setupUi(this);  // prepares the UI
}

Wizard::~Wizard()
{
    delete ui;
}

bool Wizard::validateCurrentPage()
{   // overrided inherited member in order to validate each current page
    switch (this ->currentId()) {
    case WIZARDPAGE1 :
        return validatePage1();
    case WIZARDPAGE2 :
        return validatePage2();
    case WIZARDPAGE3 :
        return validatePage3();
    default:
        return true;
    }
}

bool Wizard::validatePage1()
{   // fails if a directory pool name has no value
    if(  ui ->desktopDir->text().isEmpty()
      || ui ->templateDir->text().isEmpty()
      || ui ->publicshareDir->text().isEmpty()
      || ui ->documentsDir->text().isEmpty()
      || ui ->musicDir->text().isEmpty()
      || ui ->picturesDir->text().isEmpty()
      || ui ->videosDir->text().isEmpty() )
        return false;
    return true;
}

bool Wizard::validatePage2()
{   // fails if Source directory pool name has no value
    if(!  ui ->sourcesNoConfig->isChecked()
       && ui ->sourcesDir->text().isEmpty() )
        return false;
    return true;
}

bool Wizard::validatePage3()
{
    return true;
}


void Wizard::initializePage(int id)
{   // overrided inherited member in order to fill fields of each current page
    switch (id) {
    case WIZARDINTRO :
        break;
    case WIZARDPAGE1 :
        fillPage1Widget();
        break;
    case WIZARDPAGE2 :
        fillPage2Widget();
        break;
    case WIZARDPAGE3 :
        fillPage3Widget();
        break;
    default:
        break;
    }
}

void Wizard::on_desktopDirButton_clicked()
{   // ask the user to provide a directory name for the DESKTOP pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->desktopDir->setText(directory);
}

void Wizard::on_templateDirButton_clicked()
{   // ask the user to provide a directory name for the TEMPLATE pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->templateDir->setText(directory);
}

void Wizard::on_publicshareDirButton_clicked()
{   // ask the user to provide a directory name for the PUBLICSHARE pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->publicshareDir->setText(directory);
}

void Wizard::on_documentsDirButton_clicked()
{   // ask the user to provide a directory name for the DOCUMENTS pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->documentsDir->setText(directory);
}

void Wizard::on_musicDirButton_clicked()
{   // ask the user to provide a directory name for the MUSIC pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->musicDir->setText(directory);
}

void Wizard::on_picturesDirButton_clicked()
{   // ask the user to provide a directory name for the PICTURES pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->picturesDir->setText(directory);
}

void Wizard::on_videosDirButton_clicked()
{   // ask the user to provide a directory name for the VIDEOS pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->videosDir->setText(directory);
}

void Wizard::on_sourcesDirButton_clicked()
{   // ask the user to provide a directory name for the SOURCES pool
    const QString directory = QFileDialog::getExistingDirectory(this
                              , QObject::trUtf8("Select directory")
                              , QDir::homePath()
                              , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui ->sourcesDir->setText(directory);
}

void Wizard::fillPage1Widget()
{
    confPools.clear();  // clears container that holds configurations of all pools
    // iteration to retrieve directory name and store the pool configuration into confPool container
    for(auto p = XDGSearch::Pool::DESKTOP; p != XDGSearch::Pool::END; ++p)   {
        XDGSearch::Configuration conf(p);
        switch (p) {
        case XDGSearch::Pool::DESKTOP : if(!std::get<3>(conf.getPools()).empty())  {
                ui ->desktopDir->setText(QString::fromStdString(std::get<3>(conf.getPools())));
                ui ->desktopDirButton->setDisabled(true);
                confPools.push_front(conf.getPools());
            }
            break;
        case XDGSearch::Pool::TEMPLATES : if(!std::get<3>(conf.getPools()).empty())  {
                ui ->templateDir->setText(QString::fromStdString(std::get<3>(conf.getPools())));
                ui ->templateDirButton->setDisabled(true);
                confPools.push_front(conf.getPools());
            }
            break;
        case XDGSearch::Pool::PUBLICSHARE : if(!std::get<3>(conf.getPools()).empty())  {
                ui ->publicshareDir->setText(QString::fromStdString(std::get<3>(conf.getPools())));
                ui ->publicshareDirButton->setDisabled(true);
                confPools.push_front(conf.getPools());
            }
            break;
        case XDGSearch::Pool::DOCUMENTS : if(!std::get<3>(conf.getPools()).empty())  {
                ui ->documentsDir->setText(QString::fromStdString(std::get<3>(conf.getPools())));
                ui ->documentsDirButton->setDisabled(true);
                confPools.push_front(conf.getPools());
            }
            break;
        case XDGSearch::Pool::MUSIC : if(!std::get<3>(conf.getPools()).empty())  {
                ui ->musicDir->setText(QString::fromStdString(std::get<3>(conf.getPools())));
                ui ->musicDirButton->setDisabled(true);
                confPools.push_front(conf.getPools());
            }
            break;
        case XDGSearch::Pool::PICTURES : if(!std::get<3>(conf.getPools()).empty())  {
                ui ->picturesDir->setText(QString::fromStdString(std::get<3>(conf.getPools())));
                ui ->picturesDirButton->setDisabled(true);
                confPools.push_front(conf.getPools());
            }
            break;
        case XDGSearch::Pool::VIDEOS : if(!std::get<3>(conf.getPools()).empty())  {
                ui ->videosDir->setText(QString::fromStdString(std::get<3>(conf.getPools())));
                ui ->videosDirButton->setDisabled(true);
                confPools.push_front(conf.getPools());
            }
            break;
        default:
            break;
        }
    }
}

void Wizard::fillPage2Widget()
{
    return;
}

void Wizard::fillPage3Widget()
{   // prepares a summary report of the configurations made to display before to accept the Wizard
    std::ostringstream().swap(report);
    confPools.remove_if( [] (const XDGSearch::poolType& p)  // removes Sources pool from the container
                            { return std::get<0>(p) == "XDG_SOURCES_DIR"; });

    report  <<  "\nSummary:\n\n"            // summary report header
            <<  "<Pool name>\t\t<Directory>\n";

    for(const auto& p : confPools)
        report  << " "  << std::get<1>(p)   << ":\t\t\""    // localized pool name
                        << std::get<3>(p)   << "\"\n";      // pool's directory name

    if( !ui ->sourcesNoConfig->isChecked() )    {
        report  <<  " Sources"    << ":\t\t\""
                <<  ui ->sourcesDir->text().toStdString()   << "\"";
        confPools.push_front(std::make_tuple( "XDG_SOURCES_DIR"
                                            , "Sources"
                                            , "text"
                                            , ui ->sourcesDir->text().toStdString()
                                            , "none"
                                            , "none" ));
    } else  {
        report  <<  " Sources"    << ":\t\t- "
                <<  "disabled"   << " -";
        confPools.push_front(std::make_tuple("XDG_SOURCES_DIR", "", "", "", "none", "none"));
    }

    ui ->summaryLabel->setText(QString::fromStdString(report.str()));   // displays the resulting report
}

void Wizard::on_sourcesNoConfig_stateChanged(int s)
{   // on toggling of sourcesNoConfig accordingly sets the widgets
    switch (s) {
    case Qt::Unchecked :
        ui ->sourcesDirButton->setDisabled(false);
        ui ->sourcesDir->setDisabled(false);
        ui ->sourcesDir->setText("");
        break;
    case Qt::Checked :
        ui ->sourcesDirButton->setDisabled(true);
        ui ->sourcesDir->setDisabled(true);
        ui ->sourcesDir->setText("disabled");
        break;
    default:
        break;
    }
}

void Wizard::on_Wizard_accepted()
{   // once accepted write configurations stored in confPool into the .conf file
    XDGSearch::Configuration conf;  // no pool provided to the constructor, Settings derivated class is used
    for(const auto& p : confPools)
        conf.writeSettings(p);

    conf.initSettings();    // after that pool settings are been written now it writes initial helpers settings

}
