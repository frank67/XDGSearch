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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "indexer.h"    // first because required by Xapian
#include "ui_mainwindow.h"
#include <QMainWindow>
#include <memory>
#include <sstream>


namespace Ui {
    class MainWindow;
}

namespace XDGSearch {
    class Configuration;    // declaration for further use in the MainWindow class
    void rebuildDB(const XDGSearch::Pool&);  // threaded function to build pool's databases
}

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
private slots:
    void on_actionHistory_triggered();      // 5 slot invoked by each popup menu item
    void on_actionRebuild_current_Pool_triggered();
    void on_actionRebuild_All_triggered();
    void on_actionPreferences_triggered();
    void on_actionAbout_triggered();

    void on_resultPane_anchorClicked(const QUrl&);      // run proper application for each url type

    void on_sought_returnPressed();     // query database and shows result

    void on_sought_textEdited(const QString&);

    void on_poolCBox_activated(int);

private:
    Ui::MainWindow* const ui;
    std::unique_ptr<XDGSearch::Configuration> conf;
    void readMainWindowSizeAndPosition();        // set the MainWindow position and geometry reading the .conf file
    void populateCBox();        // set the combobox adding local pools name
    bool maybeQuit();           // ask confirmation for quitting
    bool maybeBuildDB();        // ask confirmation for build pool's database (if it doesn't exist)
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;    // close MainWindow
    void showSplashScreenText();
};

#endif // MAINWINDOW_H
