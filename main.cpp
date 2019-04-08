/* XDGSearch is a XAPIAN based file indexer and search tool.

    Copyright (C) 2016,2017,2018,2019  Franco Martelli

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
#include <QApplication>
#include <QCommandLineParser>
#include "wizard.h"
#include "configuration.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu);
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo);
    QCoreApplication::setOrganizationName("XDGSearch");
    QCoreApplication::setApplicationName("xdgsearch");
    QCoreApplication::setApplicationVersion(APP_VERSION);   /// the version number is stored into xdgsearch.pro file
    QCommandLineParser parser;
    parser.setApplicationDescription("XDGSearch is a XAPIAN based file indexer and search tool.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    const XDGSearch::Configuration conf;
    const bool&& b = conf.isFirstRun();

    if(b)   {
        Wizard wiz;
        if(wiz.exec() == QDialog::Accepted) {   /// opens wizard dialog and if accepted opens xdgsearch's mainwindow
            MainWindow w0;
            w0.show();
            return app.exec();
        }
    } else  {
        MainWindow w1;
        w1.show();
        return app.exec();
    }

    return EXIT_FAILURE;
}
