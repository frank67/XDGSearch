/* XDGSearch is a XAPIAN based file indexer and search tool.

    Copyright (C) 2016,2017  Franco Martelli

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

#ifndef XDGSEARCH_INCLUDED_HELPERS_H
#define XDGSEARCH_INCLUDED_HELPERS_H

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

    void clicked_buttonBoxCancel();     /// 4 custom slots invoked after signal emitted: see connect in the class constructor definition
    void clicked_buttonBoxOk();
    void on_helpersList_itemDoubleClicked(QListWidgetItem *item);
    void on_helpersList_itemClicked(QListWidgetItem *item);

private:
    Ui::Helpers* const ui;
    std::unique_ptr<XDGSearch::Configuration> const conf;
    QString selectedHelper;
    QPushButton *buttonOk;

    void refreshHelpersList() const;
};

#endif /// HELPERS_H
