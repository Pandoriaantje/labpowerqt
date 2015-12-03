// This file is part of labpowerqt, a Gui application to control programmable
// lab power supplies.
// Copyright © 2015 Christian Rapp <0x2a at posteo dot org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TABHISTORY_H
#define TABHISTORY_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>

#include <QSqlDatabase>
#include <QSqlTableModel>

#include <QSettings>

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include <memory>

#include "settingsdefinitions.h"
#include "databasedef.h"

class TabHistory : public QWidget
{
    Q_OBJECT
public:
    explicit TabHistory(QWidget *parent = 0);

signals:

public slots:

    void updateModel(bool status);

private:
    QGridLayout *lay;
    std::unique_ptr<QSqlTableModel> tblModel;
    QTableView *tblView;

    void setupUI();
};

#endif // TABHISTORY_H
