#include "labpowercontroller.h"

namespace setcon = settings_constants;

LabPowerController::LabPowerController()
{
    this->powerSupplyConnector = nullptr;
    this->setupPowerSupplyConnector();
}

void LabPowerController::setupPowerSupplyConnector()
{
    QSettings settings;
    settings.beginGroup(setcon::DEVICE_GROUP);
    if (settings.contains(setcon::DEVICE_PORT)) {
        QString portName = settings.value(setcon::DEVICE_PORT).toString();
        if (!this->powerSupplyConnector ||
            this->powerSupplyConnector->getserialPortName() != portName) {


            this->powerSupplyConnector =
                std::unique_ptr<KoradSCPI>(new KoradSCPI(portName));

            QObject::connect(this->powerSupplyConnector.get(),
                             SIGNAL(errorOpen(const QString&)),
                             SLOT(deviceError(const QString&)));
            QObject::connect(
                this->powerSupplyConnector.get(),
                SIGNAL(requestFinished(std::shared_ptr<SerialCommand>)), this,
                SLOT(receiveData(std::shared_ptr<SerialCommand>)));
            QObject::connect(
                this->powerSupplyConnector.get(),
                SIGNAL(statusReady(std::shared_ptr<PowerSupplyStatus>)), this,
                SLOT(receiveStatus(std::shared_ptr<PowerSupplyStatus>)));

            this->powerSupplyConnector->startBackgroundThread();
        }
    } else {
        // TODO: Notify model that no valid power supply is connected.
    }
    settings.endGroup();
}

void LabPowerController::freeSerialPort()
{
    if (this->powerSupplyConnector) {
        this->powerSupplyConnector.release();
        this->powerSupplyConnector = nullptr;
    }
}

void LabPowerController::deviceError(const QString &errorString)
{
    qDebug() << Q_FUNC_INFO << "Could not open device: " << errorString;
}

void LabPowerController::deviceReadWriteError(const QString &errorString)
{
    qDebug() << Q_FUNC_INFO << "Could not open Read/write to device: " << errorString;
}

void LabPowerController::setVoltage(const double &value)
{
    this->powerSupplyConnector->setVoltage(1, value);
}

void LabPowerController::getIdentification()
{
    this->powerSupplyConnector->getIdentification();
}

void LabPowerController::getStatus() { this->powerSupplyConnector->getStatus(); }

void LabPowerController::receiveData(std::shared_ptr<SerialCommand> com)
{
    qDebug() << Q_FUNC_INFO << "com: " << com->getValue();
}

void LabPowerController::receiveStatus(std::shared_ptr<PowerSupplyStatus> status)
{
    qDebug() << Q_FUNC_INFO << "PowerSupply Status: ";
    qDebug() << Q_FUNC_INFO << "Beeper: " << status->getBeeper();
    qDebug() << Q_FUNC_INFO << "Locked: " << status->getLocked();
    qDebug() << Q_FUNC_INFO << "Actual Current: " << status->getActualCurrent(1);
    qDebug() << Q_FUNC_INFO << "Actual Voltage: " << status->getActualVoltage(1);
    qDebug() << Q_FUNC_INFO
             << "Adjusted Current: " << status->getAdjustedCurrent(1);
    qDebug() << Q_FUNC_INFO
             << "Adjusted Voltage: " << status->getAdjustedVoltage(1);
}
