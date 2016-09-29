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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QPushButton>
#include "configuration.h"

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

private slots:
    void on_poolDirButton_clicked();

    void on_addHelper_clicked();

    void on_removeHelper_clicked();

    void clicked_buttonBoxCancel();
    void clicked_buttonBoxOk();
    void clicked_buttonBoxApply();

    void on_stemCBox_activated(int index);

    void on_stopwordsCBox_activated(int index);

    void on_poolCBox_activated(const QString &arg1);

private:
    Ui::Preferences *ui;
    QPushButton *buttonOk, *buttonApply;
    bool changesAlreadyApplied;
    void fillPageInitValue();
    void refreshHelpersList();
    XDGSearch::poolType collectFieldsValue(const XDGSearch::Configuration&);
};

#endif // PREFERENCES_H
