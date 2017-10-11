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

#ifndef HELPERS_H
#define HELPERS_H

#include <QDialog>
#include <QPushButton>
#include <QListWidgetItem>
#include "configuration.h"

namespace Ui {
class Helpers;
}

class Helpers : public QDialog
{
    Q_OBJECT
public:
    explicit Helpers(QWidget *parent = 0);
    ~Helpers();
    const QString getHelperName() const   { return selectedHelper; }
private slots:
    void on_addHelper_clicked();    /// 2 slots invoked on button clicked
    void on_removeHelper_clicked();

    void clicked_buttonBoxCancel();     /// 4 custom slots invoked after signal emitted: see connect in the class constructor definition
    void clicked_buttonBoxOk();
    void clicked_buttonBoxRestoreDefault();
    void clicked_buttonBoxApply();

    void on_helperName_editingFinished();   /// 4 slots invoked when the user ends editing
    void on_helperCmdLine_editingFinished();
    void on_helperFileExt_editingFinished();
    void on_helperGranularity_editingFinished();

    void on_helpersList_currentItemChanged( QListWidgetItem *current
                                          , QListWidgetItem *previous );    /// slot invoked when selected a item in the list
private:
    Ui::Helpers* const ui;
    std::unique_ptr<XDGSearch::Configuration> const conf;
    QString selectedHelper;
    QPushButton *buttonOk, *buttonApply;
    bool isNameAdding, granularityEdited;
    QListWidgetItem *newItemAdded;

    void toggleWidgetOnEditing();
    void refreshHelpersList();
};

#endif /// HELPERS_H
