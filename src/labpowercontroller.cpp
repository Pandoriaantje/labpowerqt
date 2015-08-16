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
        try {
            QString portName = settings.value(setcon::DEVICE_PORT).toString();
            if (!this->powerSupplyConnector ||
                this->powerSupplyConnector->getserialPortName() != portName) {
                this->powerSupplyConnector =
                    std::unique_ptr<KoradSCPI>(new KoradSCPI(portName));
                QObject::connect(
                    this->powerSupplyConnector.get(),
                    SIGNAL(requestFinished(std::shared_ptr<SerialCommand>)),
                    this, SLOT(dataReceived(std::shared_ptr<SerialCommand>)));
            }
        } catch (const std::runtime_error &ex) {
            qDebug() << Q_FUNC_INFO << "Could not open com port: " << ex.what();
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

void LabPowerController::setVoltage(double value) {}

void LabPowerController::readSetVoltage() {}

void LabPowerController::readCurrentVoltage() {}

void LabPowerController::getIdentification()
{
    this->powerSupplyConnector->getIdentification();
}

void LabPowerController::dataReceived(std::shared_ptr<SerialCommand> com)
{
    qDebug() << Q_FUNC_INFO << "com: " << com->getReply();
}