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

    ui->setupUi(this);  // prepares the UI
    ui->menuButton->addAction(ui->actionRebuild_current_Pool);  // 5 slot for menuButton widget
    ui->menuButton->addAction(ui->actionRebuild_All);
    //ui->menuButton->addAction(ui->actionHistory);
    ui->menuButton->addAction(ui->actionPreferences);
    ui->menuButton->addAction(ui->actionAbout);
    ui->menuButton->addAction(ui->action_Quit);

    ui ->resultPane->setAlignment(Qt::AlignCenter);
    readMainWindowSizeAndPosition();
    showSplashScreenText();
    populateCBox();     // populates poolCBox widget with local pool name

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readMainWindowSizeAndPosition()
{
    const QByteArray geometry = conf ->readMainWindowGeometry();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        QWidget::restoreGeometry(geometry);
    }
}

void MainWindow::populateCBox()
{   // iteration that retrieves each local pool name and adds it to poolCBox
    for(auto p = XDGSearch::Pool::DESKTOP; p != XDGSearch::Pool::END; ++p)   {
        XDGSearch::Configuration conf(p);
        auto pt = conf.enqueryPool();
        if(!std::get<1>(pt).empty())
            ui ->poolCBox ->addItem(QString::fromStdString(std::get<1>(pt)), QVariant::fromValue(p));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeQuit()) {  // if not disabled by the user opens a dialog to ask close window confirmation
        conf ->saveMainWindowGeometry(QWidget::saveGeometry());
        event->accept();
    } else
        event->ignore();
}

bool MainWindow::maybeQuit()
{
    if(conf ->askForConfirmation())
        return true;    // the user has disabled the close window dialog (the checkBox was clicked)

    bool retval = false;
    // a dialog will be shown to ask for confirmation to close window
    QMessageBox* mb = new QMessageBox(  QMessageBox::Question
                                      , QCoreApplication::applicationName()
                                      , QObject::trUtf8("Do you really want to quit?")
                                      , QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No
                                      , this);
    // provide a checkBox into the dialog window, if clicked the dialog window won't be shown
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
{   // dialog window to ask confirmation for build a database
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
{   // rebuild and overwrite the database pointed by poolCBox combobox
    const QString statusBarMessage = QString(QObject::trUtf8(" Indexing: "))
                                   + ui ->poolCBox->currentText()
                                   + QString(QObject::trUtf8(" pool ..."));

    ui ->statusBar->showMessage(statusBarMessage);  // informs the user by displaying a message in the status bar
    XDGSearch::Indexer idx(ui ->poolCBox->currentData().value<XDGSearch::Pool>());  // build indexer by passing the pool value pointed from poolCBox
    idx.populateDB();   // rebuild and overwrite the database
    ui ->statusBar->showMessage(QString(QObject::trUtf8(" Done!")), 2000);  // displays " Done!" timed out by 2 sec
}

void MainWindow::on_actionRebuild_All_triggered()
{   // rebuild and overwrite all the databases
    const QString statusBarMessage = QString(QObject::trUtf8(" Indexing all pool in progress ..."));
    std::forward_list<std::unique_ptr<std::thread>> idxThread;


    for(auto p = XDGSearch::Pool::DESKTOP; p != XDGSearch::Pool::END; ++p)
        idxThread.push_front(std::unique_ptr<std::thread>(new std::thread(XDGSearch::rebuildDB, p)));

    ui->statusBar->showMessage(statusBarMessage);

    for(auto& t : idxThread)
        t ->join();

    ui->statusBar->showMessage(QString(QObject::trUtf8(" Done!")), 2000);
}

void XDGSearch::rebuildDB(const XDGSearch::Pool& p)
{
    XDGSearch::Indexer idx(p);

    idx.populateDB();
}

void MainWindow::on_actionPreferences_triggered()
{   // displays preference dialog
    QDialog* d = new Preferences(this);
    d->exec();
    delete d;
}

void MainWindow::on_resultPane_anchorClicked(const QUrl& u)
{   // starts the application associated to the file extension
    QDesktopServices::openUrl(u);
}

void MainWindow::on_sought_returnPressed()
{   // performs the query of the sought terms and it will display the database's documents that match the search
    const XDGSearch::Pool p = ui ->poolCBox->currentData().value<XDGSearch::Pool>();
    const QString statusBarMessage = QString(QObject::trUtf8(" Indexing: "))
                                   + ui ->poolCBox->currentText()
                                   + QString(QObject::trUtf8(" pool ..."));
    const std::string soughtTerms = ui ->sought->text().toStdString();
    XDGSearch::Indexer idx(p);

    if(conf ->isPopulatedDB(p))
        idx.seek(soughtTerms);
    else    {
        if(maybeBuildDB())  {   // asks the user whether wanna build the database
            ui ->statusBar->showMessage(statusBarMessage);
            idx.populateDB();
            ui ->statusBar->showMessage(QString(QObject::trUtf8(" Done!")), 2000);
            idx.seek(soughtTerms);
        } else  {       // informs the user that rebuild database is necessary
            ui ->statusBar->showMessage(QString(QObject::trUtf8(" Rebuilding database is necessary")), 2000);
            return;
        }
    }
    ui ->resultPane->setHtml(QString::fromStdString(idx.getResult()));  // show results into resultPane widget
}

void MainWindow::on_actionAbout_triggered()
{   // displays "about" application window
    const QString title = QString("XDGSearch ")
                        + QCoreApplication::applicationVersion();
    const QString body  = QString("<html><head/><body><p><span style=\" font-size:11pt; font-weight:600;\">")
                        + title
                        + "</span></p><p>Based on QT "
                        + QT_VERSION_STR
                        + "</p><p>Copyright &copy; 2016 Franco Martelli All rights reserved.</p>"
                        + "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."
                        + "</p></body></html>";

    QMessageBox::about(this, title, body);
}

void MainWindow::on_sought_textEdited(const QString& text)
{
    if(text.isEmpty())
        showSplashScreenText();
}

void MainWindow::showSplashScreenText()
{
    std::ostringstream composeHTML;
    composeHTML  << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">" << "\n"
                 << "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">" << "\n"
                 << "p, li { white-space: pre-wrap; }" << "\n"
                 << "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">" << "\n"
                 << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>" << "\n"
                 << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>" << "\n"
                 << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>" << "\n"
                 << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>" << "\n"
                 << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>" << "\n"
                 << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>" << "\n"
                 << "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600; color:#bababa;\">Unexpected</span><span style=\" color:#bababa;\"> 'No item found'</span></p>" << "\n"
                 << "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600; color:#bababa;\">- Have you rebuild the database?</span></p>" << "\n"
                 << "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600; color:#bababa;\">- Have you right helper installed?</span></p></body></html>" << "\n";

    ui ->resultPane->setHtml(QString::fromStdString(composeHTML.str()));  // show slash-screen text into resultPane widget
}

void MainWindow::on_poolCBox_activated(int index)
{
    Q_UNUSED(index)
    showSplashScreenText();
}
