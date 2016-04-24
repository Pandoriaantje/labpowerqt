// labpowerqt is a Gui application to control programmable lab power supplies
// Copyright © 2015, 2016 Christian Rapp <0x2a at posteo dot org>
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

#ifndef DEVICEWIZARDFINAL_H
#define DEVICEWIZARDFINAL_H

#include <QWidget>
#include <QWizardPage>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QSettings>

#include <QMessageBox>

#include "settingsdefinitions.h"

/**
 * @brief Final page of the device wizard, checks if the name is unique
 */
class DeviceWizardFinal : public QWizardPage
{
    Q_OBJECT

public:
    DeviceWizardFinal(QWidget *parent = 0);

    void initializePage() Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;

private:
    QLineEdit *deviceName;
    QLabel *summary;
    QSettings settings;

    bool nameOkay;

    void checkDeviceName();
};

#endif  // DEVICEWIZARDFINAL_H
