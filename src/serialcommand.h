// labpowerqt is a Gui application to control programmable lab power supplies
// Copyright © 2015, 2016 Christian Rapp <0x2a at posteo dot org>

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

#ifndef SERIALCOMMAND
#define SERIALCOMMAND

#include <QByteArray>
#include <QVariant>
#include <memory>

/**
 * @brief Serial command with return value
 *
 * @details
 * This struct represents a serial command with the reply that was received (if any)
 */
struct SerialCommand {
public:
    /**
     * @brief SerialCommand default constructor that we need to register our
     * custom Type
     */
    SerialCommand() { this->command = 100; }
    SerialCommand(int command, int channel = 1, QVariant value = QVariant(),
                  bool withReply = false, int replyLength = 0)
        : command(command),
          powerSupplyChannel(channel),
          value(value),
          commandWithReply(withReply),
          lengthBytesReply(replyLength)
    {
    }

    /**
     * @brief Get the command that was or will be send to the hardware device
     *
     * @return Command as integer
     * @details
     * Can be cast to PowerSupplySCPI_constants::COMMANDS
     */
    int getCommand() { return this->command; }
    int getPowerSupplyChannel() { return this->powerSupplyChannel; }
    QVariant getValue() { return this->value; }
    void setValue(const QVariant &value) { this->value = value; }
    bool getCommandWithReply() { return this->commandWithReply; }
    int getLengthBytesReply() { return this->lengthBytesReply; }
private:
    int command;
    int powerSupplyChannel;
    QVariant value;
    bool commandWithReply;
    int lengthBytesReply;
};

// Register our metatype. Needed to send this kind of object wrapped in a std
// smart pointer via SIGNAL/SLOT mechanism
Q_DECLARE_METATYPE(std::shared_ptr<SerialCommand>)

#endif  // SERIALCOMMAND
