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

#ifndef FLOATINGVALUESDIALOG_H
#define FLOATINGVALUESDIALOG_H

#include <QDialog>

#include <QGridLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QToolButton>

#include <QDebug>

#include <memory>

struct FloatingValuesDialogData {
    FloatingValuesDialogData(){};

    double voltage;
    double current;
};

class FloatingValuesDialog : public QDialog
{

    Q_OBJECT

public:
    enum dataType { VOLTAGE, CURRENT };

    FloatingValuesDialog(
                         QWidget *parent = 0, Qt::WindowFlags f = 0);

    void setValuesDialogData(std::shared_ptr<FloatingValuesDialogData> data);

signals:

    void doubleValueAccepted(double val, int sourceWidget, int sourceChannel);

public slots:

    void setSourceWidget(int sourcew);
    void setSourceChannel(int channel);
    void setInputWidget(int w);
    void setInputWidgetValue(double value);
    void setInputWidgetValue(int trackingMode);
    void updateDeviceSpecs(double voltageMin, double voltageMax,
                           uint voltagePrecision, double currentMin,
                           double currentMax, uint currentPrecision,
                           uint noOfChannels);

private:
    QGridLayout *mainLayout;
    QStackedWidget *stackedContainer;
    QToolButton *acceptButton;

    QDoubleSpinBox *voltageSpinBox;
    QDoubleSpinBox *currentSpinBox;

    int sourceWidget;
    int sourceChannel;

    std::shared_ptr<FloatingValuesDialogData> data;

    void createUI();

private slots:
    void accept();
    void reject();
};

#endif // FLOATINGVALUESDIALOG_H
