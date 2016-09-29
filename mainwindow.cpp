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

#include "mainwindow.h"
#include "configuration.h"
#include "preferences.h"

#include <QDesktopWidget>
#include <QCloseEvent>
#include <QMessageBox>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDialog>
//#include <QtConcurrent/QtConcurrent>
//#include <QFuture>
#include <thread>
#include <memory>
#include <forward_list>

namespace Ui {
    class MainWindow;
}

MainWindow::MainWindow(QWidget *parent) :
      QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , conf(std::unique_ptr<XDGSearch::Configuration>(new XDGSearch::Configuration))
{

    ui->setupUi(this);
    ui->menuButton->addAction(ui->actionRebuild_current_Pool);
    ui->menuButton->addAction(ui->actionRebuild_All);
    ui->menuButton->addAction(ui->actionHistory);
    ui->menuButton->addAction(ui->actionPreferences);
    ui->menuButton->addAction(ui->action_Quit);

    readSettings();
    populateCBox();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readSettings()
{
    const QByteArray geometry = conf ->readMainWindowGeometry();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::populateCBox()
{
    for(auto p = XDGSearch::Pool::DESKTOP; p != XDGSearch::Pool::END; ++p)   {
        XDGSearch::Configuration conf(p);
        auto pt = conf.enqueryPool();
        if(!std::get<1>(pt).empty())
            ui ->poolCBox ->addItem(QString::fromStdString(std::get<1>(pt)), QVariant::fromValue(p));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeQuit()) {
        conf ->saveMainWindowGeometry(saveGeometry());
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::maybeQuit()
{
    if(conf ->askForConfirmation())
        return true;

    bool retval = false;
    QMessageBox* mb = new QMessageBox(  QMessageBox::Question
                                      , QCoreApplication::applicationName()
                                      , QObject::trUtf8("Do you really want to quit?")
                                      , QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No
                                      , this);
    QCheckBox* cb = new QCheckBox(QObject::trUtf8("Don't ask me again"), mb);
    mb ->setCheckBox(cb);

    if(mb ->exec() == QMessageBox::Yes)
    {
        if(cb ->isChecked())
            conf ->setAskForConfirmation(true);
        retval = true;
    }
    delete cb;  delete mb;

    return retval;
}

bool MainWindow::maybeBuildDB()
{
    bool retval = false;
    QMessageBox* mb = new QMessageBox(  QMessageBox::Question
                                      , QCoreApplication::applicationName()
                                      , QObject::trUtf8("Actually the database you are quering does not exist!\n"
                                                        "Do you want to build it now?")
                                      , QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No
                                      , this);

    if(mb ->exec() == QMessageBox::Yes)
        retval = true;

    delete mb;

    return retval;
}

void MainWindow::on_actionHistory_triggered()
{

}

void MainWindow::on_actionRebuild_current_Pool_triggered()
{
    const QString statusBarMessage = QString(QObject::trUtf8(" Indexing: "))
                                   + ui ->poolCBox->currentText()
                                   + QString(QObject::trUtf8(" pool ..."));

    ui ->statusBar->showMessage(statusBarMessage);
    XDGSearch::indexer idx(ui ->poolCBox->currentData().value<XDGSearch::Pool>());
    idx.populateDB();
    ui ->statusBar->showMessage(QString(QObject::trUtf8(" Done!")), 2000);
}

void MainWindow::on_actionRebuild_All_triggered()
{
    const QString statusBarMessage = QString(QObject::trUtf8(" Indexing all pool in progress ..."));
    std::forward_list<std::unique_ptr<std::thread>> idxThread;


    for(auto p = XDGSearch::Pool::DESKTOP; p != XDGSearch::Pool::END; ++p)   {
        idxThread.push_front(std::unique_ptr<std::thread>(new std::thread(  XDGSearch::rebuildAllDB, p)));

        //QFuture<void> future = QtConcurrent::run(MainWindow::rebuildAllDB, p);
    }
    ui->statusBar->showMessage(statusBarMessage);

    for(auto& t : idxThread)
        t ->join();

    ui->statusBar->showMessage(QString(QObject::trUtf8(" Done!")), 2000);
}

void XDGSearch::rebuildAllDB(const XDGSearch::Pool& p)
{
    XDGSearch::indexer idx(p);

    idx.populateDB();
}

void MainWindow::on_actionPreferences_triggered()
{

    QDialog* q = new Preferences(this);
    q->exec();
    delete q;
}

void MainWindow::on_resultPane_anchorClicked(const QUrl& u)
{
    QDesktopServices::openUrl(u);
}

void MainWindow::on_sought_returnPressed()
{
    const XDGSearch::Pool p = ui ->poolCBox->currentData().value<XDGSearch::Pool>();
    const QString statusBarMessage = QString(QObject::trUtf8(" Indexing: "))
                                   + ui ->poolCBox->currentText()
                                   + QString(QObject::trUtf8(" pool ..."));
    const std::string s = ui ->sought->text().toStdString();
    XDGSearch::indexer idx(p);

    if(conf ->isPopulatedDB(p))
        idx.seek(s);
    else    {
        if(maybeBuildDB())  {
            ui ->statusBar->showMessage(statusBarMessage);
            idx.populateDB();
            ui ->statusBar->showMessage(QString(QObject::trUtf8(" Done!")), 2000);
            idx.seek(s);
        }
    }
    ui ->resultPane->setHtml(QString::fromStdString(idx.getResult()));
}
