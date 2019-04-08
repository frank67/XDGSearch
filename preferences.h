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

#ifndef XDGSEARCH_INCLUDED_PREFERENCES_H
#define XDGSEARCH_INCLUDED_PREFERENCES_H

#include <QDialog>
#include <QPushButton>
#include <QListWidgetItem>
#include "configuration.h"

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT
public:
    explicit Preferences(QWidget *parent = nullptr);
    Preferences(Preferences&&) = delete;
    Preferences& operator=(Preferences&&) = delete;
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

    void on_tabWidget_currentChanged(int index);

    void on_helperName_editingFinished();
    void on_helperCmdLine_editingFinished();
    void on_helperFileExt_editingFinished();

    void on_newHelper_clicked();
    void on_deleteHelper_clicked();
    void on_helperDefaults_clicked();

    void on_allHelpersList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_helperGranularity_valueChanged(int arg1);

private:
    Ui::Preferences* const ui;
    QPushButton *buttonOk, *buttonApply;
    bool changesAlreadyApplied;
    void fillPageInitValue() const;       /// initial value for combobox, list, fields
    void refreshHelpersList() const;      /// read .conf file and update the list of helpers
    const XDGSearch::poolType collectWidgetValue(const XDGSearch::Configuration&);    /// read UI combobox, list, field storing each value into a tuple

    std::unique_ptr<XDGSearch::Configuration> const conf;
    QString selectedHelper;
    bool isNameAdding, isDialogWindowShown;
    QListWidgetItem* newItemAdded;
    int currentTabNumber;

    void toggleWidgetOnEditing();
    void refreshallHelpersList() const;
};

#endif /// XDGSEARCH_INCLUDED_PREFERENCES_H
