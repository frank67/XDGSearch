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

#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include <sstream>
#include <forward_list>
#include <tuple>


namespace Ui {
class Wizard;
}

namespace XDGSearch {
using poolType = std::tuple<  std::string       //  0 PoolName
                            , std::string       //  1 localPoolName
                            , std::string       //  2 PoolHelpers
                            , std::string       //  3 PoolDirPath
                            , std::string       //  4 stemming
                            , std::string>;     //  5 stopwordsfile
}

class Wizard final : public QWizard
{
    Q_OBJECT

public:
    explicit Wizard(QWidget *parent = 0);
    ~Wizard();
protected:
    bool validateCurrentPage() Q_DECL_OVERRIDE;
    void initializePage(int) Q_DECL_OVERRIDE;

private slots:
    void on_desktopDirButton_clicked();

    void on_templateDirButton_clicked();

    void on_publicshareDirButton_clicked();

    void on_documentsDirButton_clicked();

    void on_musicDirButton_clicked();

    void on_picturesDirButton_clicked();

    void on_videosDirButton_clicked();

    void on_sourcesDirButton_clicked();

    void on_sourcesNoConfig_stateChanged(int);

    void on_Wizard_accepted();

private:
    void fillPage1Fields();
    void fillPage2Fields();
    void fillPage3Fields();
    bool validatePage1();
    bool validatePage2();
    bool validatePage3();
    Ui::Wizard *ui;
    enum { WIZARDINTRO = 0
         , WIZARDPAGE1 = 1
         , WIZARDPAGE2 = 2
         , WIZARDPAGE3 = 3 };
    std::ostringstream report;
    std::forward_list<XDGSearch::poolType> confPools;
};

#endif // WIZARD_H
