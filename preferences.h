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
    void on_poolDirButton_clicked();    /// 3 slot invoked when button clicked
    void on_addHelper_clicked();
    void on_removeHelper_clicked();

    void clicked_buttonBoxCancel();     /// 3 custom slot invoked when button clicked
    void clicked_buttonBoxOk();
    void clicked_buttonBoxApply();

    void on_stemCBox_activated(int);        /// 3 slot invoked when ComboBox item selected
    void on_stopwordsCBox_activated(int);
    void on_poolCBox_activated(const QString&);
private:
    Ui::Preferences* const ui;
    QPushButton *buttonOk, *buttonApply;
    bool changesAlreadyApplied;
    void fillPageInitValue() const;       /// initial value for combobox, list, fields
    void refreshHelpersList() const;      /// read .conf file and update the list of helpers
    const XDGSearch::poolType collectWidgetValue(const XDGSearch::Configuration&);    /// read UI combobox, list, field storing each value into a tuple
};

#endif /// PREFERENCES_H
