// labpowerqt is a Gui application to control programmable lab power supplies
// Copyright © 2015 Christian Rapp <0x2a at posteo dot org>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef LABPOWERMODEL_H
#define LABPOWERMODEL_H

#include <QObject>
#include <QDebug>
#include <QString>

#include <memory>
#include <map>

#include "global.h"
#include "powersupplystatus.h"

class LabPowerModel : public QObject
{

    Q_OBJECT

public:
    LabPowerModel();

    bool getDeviceConnected();
    void setDeviceConnected(bool connected);

    QString getDeviceIdentification();
    void setDeviceIdentification(const QString &id);

    bool getDeviceLocked();
    bool getDeviceMute();

    bool getOutput(const global_constants::CHANNEL &c);
    global_constants::MODE getChannelMode(const global_constants::CHANNEL &c);

    double getVoltage(const global_constants::CHANNEL &c);
    double getActualVoltage(const global_constants::CHANNEL &c);
    double getCurrent(const global_constants::CHANNEL &c);
    double getActualCurrent(const global_constants::CHANNEL &c);
    double getWattage(const global_constants::CHANNEL &c);

signals:

    void statusUpdate();
    void deviceConnectionStatus(bool connected);
    void deviceID();

public slots:

    void updatePowerSupplyStatus(std::shared_ptr<PowerSupplyStatus> status);

private:

    std::shared_ptr<PowerSupplyStatus> status;

    bool deviceConnected;
    QString deviceIdentification;



};

#endif // LABPOWERMODEL_H
