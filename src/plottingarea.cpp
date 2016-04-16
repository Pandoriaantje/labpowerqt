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

#include "plottingarea.h"

namespace setcon = settings_constants;
namespace globcon = global_constants;
namespace utils = global_utilities;

PlottingArea::PlottingArea(QWidget *parent) : QWidget(parent)
{
    this->autoScroll = true;
    this->startPoint = std::chrono::system_clock::now();
    this->currentDataPointKey = std::chrono::system_clock::now();
    this->lastAction = nullptr;
    this->dataDisplayFrameHeight = -1;

    // TODO: Maybe it would be nice for the user to define the min max zoom ranges
    this->lowZoom = 60;
    this->highZoom = 3600;

    this->setupUI();
    this->setupGraph();
}

void PlottingArea::addData(const int &channel, const double &data,
                           const std::chrono::system_clock::time_point &t,
                           const global_constants::DATATYPE &type)
{
    // FIXME: I think this method crashes when specifiying more channels than
    // actually exist
    /*
     * We actually can calculate the index of our graph using a simple formula
     * (a - 1) * 5 + b
     */
    int index = (channel - 1) * 5 + static_cast<int>(type);
    auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     t.time_since_epoch())
                     .count();
    double key = msecs / 1000.0;

    this->currentDataPointKey = t;

    this->plot->graph(index)->addData(key, data);

    if (this->autoScroll) {
        this->plot->xAxis->setRange(key, this->plot->xAxis->range().size(),
                                    Qt::AlignRight);
    }

    this->plot->replot();
}

// to be honest this method has mutated in an unmaintainable monster :(
void PlottingArea::setupGraph()
{
    if (this->plot->graphCount() > 0) {
        this->resetGraph();
    }

    QSettings settings;
    settings.beginGroup(setcon::DEVICE_GROUP);
    settings.beginGroup(settings.value(setcon::DEVICE_ACTIVE).toString());
    if (settings.contains(setcon::DEVICE_PORT)) {

        this->setupGraphPlot(settings);

        int graphIndex = 0;

        for (int i = 1; i <= settings.value(setcon::DEVICE_CHANNELS).toInt();
             i++) {
            // the box that controls the visibility of the graphs
            QGroupBox *graphVisibilityBox =
                new QGroupBox("Channel " + QString::number(i));
            graphVisibilityBox->setLayout(new QHBoxLayout());
            this->controlData->layout()->addWidget(graphVisibilityBox);
            // the box that controls color, line thickness and style
            QGroupBox *appearanceBox =
                new QGroupBox("Channel " + QString::number(i));
            appearanceBox->setLayout(new QVBoxLayout());
            this->controlAppearance->layout()->addWidget(appearanceBox);
            QGroupBox *dataDisplayBox =
                new QGroupBox("Channel " + QString::number(i));
            dataDisplayBox->setLayout(new QHBoxLayout());
            dataDisplayBox->layout()->setSpacing(20);
            this->dataDisplayChannels->layout()->addWidget(dataDisplayBox);
            std::map<globcon::DATATYPE, QLabel *> chanDisplayLabels;
            QGridLayout *dataDisplayVoltageGrid = new QGridLayout();
            QGridLayout *dataDisplayCurrentGrid = new QGridLayout();
            QGridLayout *dataDisplayWattageGrid = new QGridLayout();
            dynamic_cast<QHBoxLayout *>(dataDisplayBox->layout())
                ->addLayout(dataDisplayVoltageGrid);
            dynamic_cast<QHBoxLayout *>(dataDisplayBox->layout())
                ->addLayout(dataDisplayCurrentGrid);
            dynamic_cast<QHBoxLayout *>(dataDisplayBox->layout())
                ->addLayout(dataDisplayWattageGrid);

            for (int j = 0; j < 5; j++) {
                globcon::DATATYPE dt = static_cast<globcon::DATATYPE>(j);

                /*
                 * For every graph we add:
                 * A Switch to turn visibility on or off
                 * A ComboBox to set LineStyle
                 * A SpinBox to set the Line Thickness
                 * A Button to choose the graph color
                 * Two labels to display the data below the plot on mouseover
                 */
                QCheckBox *cbVisibilitySwitch = new QCheckBox();
                cbVisibilitySwitch->setText(this->datatypeStrings.at(dt));
                QLabel *labelGraphProps =
                    new QLabel(this->datatypeStrings.at(dt));
                labelGraphProps->setMinimumWidth(120);
                QComboBox *graphLineStyle = new QComboBox();
                graphLineStyle->addItem("Solid", QVariant(0));
                graphLineStyle->addItem("Dotted", QVariant(0));

                QSpinBox *graphLineThickness = new QSpinBox();
                graphLineThickness->setMinimum(1);
                graphLineThickness->setMaximum(5);
                graphLineThickness->setValue(1);

                QPushButton *graphColor = new QPushButton();
                graphColor->setToolTip("Choose graph color");
                graphColor->setIconSize(QSize(64, 16));
                // the image that is displayed in the color choose button
                QPixmap pic(64, 16);

                QLabel *dataDisplayLabel =
                    new QLabel(this->datatypeStrings.at(dt));
                QLabel *dataDisplayLabelValue = new QLabel("--");
                chanDisplayLabels.insert({dt, dataDisplayLabelValue});
                dataDisplayLabelValue->setMinimumWidth(40);
                dataDisplayLabelValue->setAlignment(
                    Qt::AlignmentFlag::AlignRight);

                dataDisplayBox->layout()->addWidget(dataDisplayLabel);
                dataDisplayBox->layout()->addWidget(dataDisplayLabelValue);

                if (dt == globcon::DATATYPE::SETCURRENT ||
                    dt == globcon::DATATYPE::CURRENT) {
                    this->plot->addGraph(this->plot->xAxis, this->currentAxis);
                    if (dt == globcon::DATATYPE::CURRENT) {
                        dataDisplayCurrentGrid->addWidget(dataDisplayLabel, 0,
                                                          0);
                        dataDisplayCurrentGrid->addWidget(dataDisplayLabelValue,
                                                          0, 1);
                    }
                    if (dt == globcon::DATATYPE::SETCURRENT) {
                        dataDisplayCurrentGrid->addWidget(dataDisplayLabel, 1,
                                                          0);
                        dataDisplayCurrentGrid->addWidget(dataDisplayLabelValue,
                                                          1, 1);
                    }
                } else if (dt == globcon::DATATYPE::WATTAGE) {
                    this->plot->addGraph(this->plot->xAxis, this->wattageAxis);
                    dataDisplayWattageGrid->addWidget(dataDisplayLabel, 0, 0);
                    dataDisplayLabel->setAlignment(Qt::AlignmentFlag::AlignTop |
                                                   Qt::AlignmentFlag::AlignLeft);
                    dataDisplayWattageGrid->addWidget(dataDisplayLabelValue, 0,
                                                      1);
                } else {
                    this->plot->addGraph();
                    if (dt == globcon::DATATYPE::VOLTAGE) {
                        dataDisplayVoltageGrid->addWidget(dataDisplayLabel, 0,
                                                          0);
                        dataDisplayVoltageGrid->addWidget(dataDisplayLabelValue,
                                                          0, 1);
                    }
                    if (dt == globcon::DATATYPE::SETVOLTAGE) {
                        dataDisplayVoltageGrid->addWidget(dataDisplayLabel, 1,
                                                          0);
                        dataDisplayVoltageGrid->addWidget(dataDisplayLabelValue,
                                                          1, 1);
                    }
                }

                graphVisibilityBox->layout()->addWidget(cbVisibilitySwitch);
                QHBoxLayout *appearanceElemLayout = new QHBoxLayout();
                appearanceElemLayout->addWidget(labelGraphProps);
                appearanceElemLayout->addWidget(graphLineStyle);
                appearanceElemLayout->addWidget(graphLineThickness);
                appearanceElemLayout->addWidget(graphColor);
                appearanceElemLayout->addStretch();
                dynamic_cast<QVBoxLayout *>(appearanceBox->layout())
                    ->addLayout(appearanceElemLayout);

                // connect visibility checkbox with a lambda
                QObject::connect(
                    cbVisibilitySwitch, &QCheckBox::toggled,
                    [this, graphIndex](bool checked) {
                        QSettings settings;
                        settings.beginGroup(setcon::PLOT_GROUP);
                        QString key =
                            QString(setcon::PLOT_GRAPH_VISIBLE).arg(graphIndex);
                        if (checked) {
                            this->plot->graph(graphIndex)->setVisible(true);
                            this->plot->graph(graphIndex)->addToLegend();
                        } else {
                            this->plot->graph(graphIndex)->setVisible(false);
                            this->plot->graph(graphIndex)->removeFromLegend();
                        }
                        settings.setValue(key, checked);
                        this->yAxisVisibility();
                    });
                // connect linestyle
                QObject::connect(
                    graphLineStyle, static_cast<void (QComboBox::*)(int)>(
                                        &QComboBox::currentIndexChanged),
                    [this, graphLineStyle, graphIndex](int idx) {
                        QSettings settings;
                        settings.beginGroup(setcon::PLOT_GROUP);
                        QString key =
                            QString(setcon::PLOT_GRAPH_LS).arg(graphIndex);
                        QPen graphPen = this->plot->graph(graphIndex)->pen();
                        if (idx == 0) {
                            graphPen.setStyle(Qt::PenStyle::SolidLine);
                        } else {
                            graphPen.setStyle(Qt::PenStyle::DotLine);
                        }
                        settings.setValue(key, idx);
                        this->plot->graph(graphIndex)->setPen(graphPen);
                        this->plot->replot();
                    });
                // connect line thickness
                QObject::connect(
                    graphLineThickness, static_cast<void (QSpinBox::*)(int)>(
                                            &QSpinBox::valueChanged),
                    [this, graphIndex, graphLineThickness](int value) {
                        QSettings settings;
                        settings.beginGroup(setcon::PLOT_GROUP);
                        QString key =
                            QString(setcon::PLOT_GRAPH_LINE).arg(graphIndex);
                        QPen graphPen = this->plot->graph(graphIndex)->pen();
                        graphPen.setWidth(value);
                        settings.setValue(key, value);
                        this->plot->graph(graphIndex)->setPen(graphPen);
                        this->plot->replot();
                    });
                // connect color button
                QObject::connect(
                    graphColor, &QPushButton::clicked,
                    [this, graphColor, graphIndex]() {
                        QSettings settings;
                        settings.beginGroup(setcon::PLOT_GROUP);
                        QString key =
                            QString(setcon::PLOT_GRAPH_COLOR).arg(graphIndex);
                        QPen graphPen = this->plot->graph(graphIndex)->pen();
                        QColor col = QColorDialog::getColor(
                            graphPen.color(), this, "Choose a color");
                        if (col.isValid()) {
                            graphPen.setColor(col);
                            this->plot->graph(graphIndex)->setPen(graphPen);
                            QPixmap pic = graphColor->icon().pixmap(64, 16);
                            pic.fill(col);
                            graphColor->setIcon(pic);
                            settings.setValue(key, col);
                            this->plot->replot();
                        }
                    });

                settings.endGroup();
                settings.endGroup();
                settings.beginGroup(setcon::PLOT_GROUP);
                QString colkey =
                    QString(setcon::PLOT_GRAPH_COLOR).arg(graphIndex);
                QColor graphCol;
                if (dt == globcon::DATATYPE::SETCURRENT ||
                    dt == globcon::DATATYPE::CURRENT) {
                    graphCol =
                        QColor(settings.value(colkey,
                                              this->currentGraphColors.at(i - 1))
                                   .toString());
                } else if (dt == globcon::DATATYPE::VOLTAGE ||
                           dt == globcon::DATATYPE::SETVOLTAGE) {
                    graphCol =
                        QColor(settings.value(colkey,
                                              this->voltageGraphColors.at(i - 1))
                                   .toString());

                } else {
                    graphCol =
                        QColor(settings.value(colkey,
                                              this->wattageGraphColors.at(i - 1))
                                   .toString());
                }
                pic.fill(graphCol);
                graphColor->setIcon(pic);
                // init a QPen for our graph
                QPen graphPen;
                graphPen.setColor(graphCol);

                this->plot->graph(graphIndex)->setPen(graphPen);
                // set graph name
                this->plot->graph(graphIndex)
                    ->setName(this->graphNames.at(dt).arg(QString::number(i)));

                QString viskey =
                    QString(setcon::PLOT_GRAPH_VISIBLE).arg(graphIndex);
                QString linekey =
                    QString(setcon::PLOT_GRAPH_LINE).arg(graphIndex);
                QString lskey = QString(setcon::PLOT_GRAPH_LS).arg(graphIndex);
                if (dt == globcon::DATATYPE::VOLTAGE ||
                    dt == globcon::DATATYPE::CURRENT ||
                    dt == globcon::DATATYPE::WATTAGE) {
                    // these are by default visible and have a solid linestyle
                    cbVisibilitySwitch->setChecked(
                        settings.value(viskey, true).toBool());
                    graphLineStyle->setCurrentIndex(
                        settings.value(lskey, 0).toInt());
                } else {
                    // these ones are by default invisible and have a dotted
                    // line.
                    cbVisibilitySwitch->setChecked(
                        settings.value(viskey, false).toBool());
                    graphLineStyle->setCurrentIndex(
                        settings.value(lskey, 1).toInt());
                }
                graphLineThickness->setValue(settings.value(linekey, 2).toInt());
                settings.endGroup();
                settings.beginGroup(setcon::DEVICE_GROUP);
                settings.beginGroup(
                    settings.value(setcon::DEVICE_ACTIVE).toString());

                graphIndex++;
            }
            dynamic_cast<QHBoxLayout *>(graphVisibilityBox->layout())
                ->addStretch();
            dynamic_cast<QHBoxLayout *>(dataDisplayBox->layout())->addStretch();
            this->dataDisplayLabels.insert(
                {static_cast<globcon::CHANNEL>(i), chanDisplayLabels});
        }
    }
    this->plot->replot();
}

void PlottingArea::setupUI()
{
    QGridLayout *mainLayout = new QGridLayout();
    this->setLayout(mainLayout);

    this->controlBar = new QToolBar();
    this->controlBar->setFloatable(false);
    this->controlBar->setMovable(false);
    mainLayout->addWidget(this->controlBar, 0, 0);
    this->actionGeneral = this->controlBar->addAction("General");
    this->actionGeneral->setIcon(QPixmap(":/icons/gear_32.png"));
    this->actionData = this->controlBar->addAction("Visibility");
    this->actionData->setIcon(QPixmap(":/icons/visibility_32.png"));
    this->actionAppearance = this->controlBar->addAction("Appearance");
    this->actionAppearance->setIcon(QPixmap(":/icons/graph_32.png"));
    QObject::connect(this->controlBar, &QToolBar::actionTriggered, this,
                     &PlottingArea::toolbarActionTriggered);

    this->controlStack = new QStackedWidget();
    mainLayout->addWidget(this->controlStack, 1, 0);

    this->controlGeneral = new QWidget();
    QVBoxLayout *controlGeneralLay = new QVBoxLayout();
    this->controlGeneralScroll = new QScrollArea();
    this->controlGeneral->setLayout(controlGeneralLay);

    QHBoxLayout *generalCBLayout = new QHBoxLayout();
    controlGeneralLay->addLayout(generalCBLayout);
    this->cbGeneralAutoscrl = new QCheckBox();
    this->cbGeneralAutoscrl->setText("Auto Scroll");
    this->cbGeneralAutoscrl->setToolTip(
        "When this checkbox is activated the plot will be scrolled so the most "
        "recent data is visible. Panning or zooming disables this option.");
    this->cbGeneralAutoscrl->setChecked(true);
    generalCBLayout->addWidget(this->cbGeneralAutoscrl);
    QObject::connect(this->cbGeneralAutoscrl, &QCheckBox::stateChanged, this,
                     &PlottingArea::generalCBCheckState);
    QCheckBox *dataDisplayArea = new QCheckBox("Show data under Plot");
    dataDisplayArea->setToolTip("Activate this option to show the data at the "
                                "mouse cursor position under the plot.");
    dataDisplayArea->setChecked(true);
    generalCBLayout->addWidget(dataDisplayArea);
    QCheckBox *generalShowGrid = new QCheckBox("Show Grid");
    generalShowGrid->setChecked(true);
    generalShowGrid->setToolTip("Show Grid in Plot");
    generalCBLayout->addWidget(generalShowGrid);
    QCheckBox *generalShowLegend = new QCheckBox("Show Legend");
    generalShowLegend->setChecked(false);
    generalShowLegend->setToolTip("Show Plot Legend");
    generalCBLayout->addWidget(generalShowLegend);
    generalCBLayout->addStretch();
    QPushButton *generalDiscardData = new QPushButton("Discard Data");
    generalDiscardData->setToolTip(
        "Discard all the Data in the Plot. This will not affect Recordings");
    generalDiscardData->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    controlGeneralLay->addWidget(generalDiscardData);
    QHBoxLayout *generalExportLayout = new QHBoxLayout();
    controlGeneralLay->addLayout(generalExportLayout);
    QPushButton *generalExport = new QPushButton("Export as");
    generalExport->setToolTip(
        "Export the currently visible ViewPort of the plot as Image");
    generalExport->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QComboBox *generalImageFormat = new QComboBox();
    generalImageFormat->addItems({"jpg", "png", "pdf"});
    generalImageFormat->setToolTip("Choose the image format");
    generalExportLayout->addWidget(generalExport);
    generalExportLayout->addWidget(generalImageFormat);
    generalExportLayout->addStretch();
    controlGeneralLay->addStretch();

    this->animationGroupDataDisplay =
        std::unique_ptr<QParallelAnimationGroup>(new QParallelAnimationGroup());
    QPropertyAnimation *maxHeightAniDisplay = new QPropertyAnimation();
    maxHeightAniDisplay->setPropertyName("maximumHeight");
    this->animationGroupDataDisplay->addAnimation(maxHeightAniDisplay);

    QObject::connect(dataDisplayArea, &QCheckBox::stateChanged, [this](
                                                                    int state) {
        QSettings settings;
        settings.beginGroup(setcon::PLOT_GROUP);
        settings.setValue(setcon::PLOT_SHOW_DATA, state);
        if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
            QVariant oldStartValue =
                dynamic_cast<QPropertyAnimation *>(
                    this->animationGroupDataDisplay->animationAt(0))
                    ->startValue();
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupDataDisplay->animationAt(0))
                ->setStartValue(0);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupDataDisplay->animationAt(0))
                ->setEndValue(oldStartValue);
            this->animationGroupDataDisplay->start();
        } else {
            if (this->dataDisplayFrameHeight == -1) {
                this->dataDisplayFrameHeight = this->dataDisplayFrame->height();
            }
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupDataDisplay->animationAt(0))
                ->setStartValue(this->dataDisplayFrameHeight);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupDataDisplay->animationAt(0))
                ->setEndValue(0);
            this->animationGroupDataDisplay->start();
        }
    });
    QObject::connect(generalDiscardData, &QPushButton::clicked, [this]() {
        if (QMessageBox::question(
                this, "Discard Data",
                "Do you really want to discard the data in the "
                "plot? This will not affect Recordings")) {
            for (int i = 0; i < this->plot->graphCount(); i++) {
                this->plot->graph(i)->clearData();
            }
            this->startPoint = std::chrono::system_clock::now();
        }
    });
    QObject::connect(
        generalExport, &QPushButton::clicked, [this, generalImageFormat]() {
            // close the upper control area to maximize the plot viewport
            this->toolbarActionTriggered(this->actionGeneral);
            QString imageFile = QFileDialog::getSaveFileName(
                this, "Choose file to export image",
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                "Export Files (*.jpg *.png *.pdf)");
            if (imageFile != "") {
                if (generalImageFormat->currentIndex() == 0)
                    this->plot->saveJpg(imageFile, 0, 0, 2.0);
                if (generalImageFormat->currentIndex() == 1)
                    this->plot->savePng(imageFile, 0, 0, 2.0);
                if (generalImageFormat->currentIndex() == 2)
                    this->plot->savePdf(imageFile, false, 0, 0, "LabPowerQt");
            }
            this->toolbarActionTriggered(this->actionGeneral);
        });
    QObject::connect(
        generalShowGrid, &QCheckBox::stateChanged, [this](int state) {
            QSettings settings;
            settings.beginGroup(setcon::PLOT_GROUP);
            settings.setValue(setcon::PLOT_SHOW_GRID, state);
            if (state == static_cast<int>(Qt::CheckState::Checked)) {
                this->plot->xAxis->grid()->setPen(this->xAxisGridPen);
                this->plot->yAxis->grid()->setPen(this->yAxisGridPen);
            } else {
                this->plot->xAxis->grid()->setPen(QPen(Qt::NoPen));
                this->plot->yAxis->grid()->setPen(QPen(Qt::NoPen));
            }
            this->plot->replot();
        });
    QObject::connect(
        generalShowLegend, &QCheckBox::stateChanged, [this](int state) {
            QSettings settings;
            settings.beginGroup(setcon::PLOT_GROUP);
            settings.setValue(setcon::PLOT_SHOW_LEGEND, state);
            if (state == static_cast<int>(Qt::CheckState::Checked)) {
                this->plot->legend->setVisible(true);
            } else {
                this->plot->legend->setVisible(false);
            }
            this->plot->replot();
        });
    this->controlGeneralScroll->setWidget(this->controlGeneral);
    this->controlGeneralScroll->setWidgetResizable(true);
    this->controlStack->addWidget(this->controlGeneralScroll);

    // control data widget. Allows to toggle graphs to display. Will be filled
    // later.
    this->controlData = new QWidget();
    this->controlData->setLayout(new QVBoxLayout());
    this->controlDataScroll = new QScrollArea();
    this->controlDataScroll->setWidget(this->controlData);
    this->controlDataScroll->setWidgetResizable(true);
    this->controlStack->addWidget(this->controlDataScroll);

    this->controlAppearance = new QWidget();
    this->controlAppearance->setLayout(new QVBoxLayout());
    this->controlAppearanceScroll = new QScrollArea();
    this->controlAppearanceScroll->setWidget(this->controlAppearance);
    this->controlAppearanceScroll->setWidgetResizable(true);
    this->controlStack->addWidget(this->controlAppearanceScroll);

    // Show general widget
    this->controlStack->setCurrentIndex(0);
    this->controlStack->setMinimumHeight(0);
    this->controlStack->setMaximumHeight(0);

    this->animationGroupControl =
        std::unique_ptr<QParallelAnimationGroup>(new QParallelAnimationGroup());
    QPropertyAnimation *minHeightAni = new QPropertyAnimation();
    QPropertyAnimation *maxHeightAni = new QPropertyAnimation();
    minHeightAni->setTargetObject(this->controlStack);
    maxHeightAni->setTargetObject(this->controlStack);
    minHeightAni->setPropertyName("minimumHeight");
    maxHeightAni->setPropertyName("maximumHeight");
    this->animationGroupControl->addAnimation(minHeightAni);
    this->animationGroupControl->addAnimation(maxHeightAni);

    // generate plot widget
    this->plot = new QCustomPlot();
    this->plot->setSizePolicy(QSizePolicy::Policy::Expanding,
                              QSizePolicy::Policy::Expanding);
    this->xAxisGridPen = this->plot->xAxis->grid()->pen();
    this->yAxisGridPen = this->plot->yAxis->grid()->pen();
    mainLayout->addWidget(this->plot, 2, 0);
    // take as much space as possible
    mainLayout->setRowStretch(2, 100);

    this->dataDisplayFrame = new QFrame();
    this->dataDisplayFrame->setLayout(new QVBoxLayout());
    mainLayout->addWidget(this->dataDisplayFrame, 3, 0);
    maxHeightAniDisplay->setTargetObject(this->dataDisplayFrame);
    // Display DateTime
    this->dataDisplayDT = new QLabel();
    this->dataDisplayDT->setAlignment(Qt::AlignmentFlag::AlignCenter);
    this->dataDisplayDT->setSizePolicy(QSizePolicy::Expanding,
                                       QSizePolicy::Fixed);
    QDateTime dateTime = QDateTime::currentDateTime();
    this->dataDisplayDT->setText(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
    this->dataDisplayFrame->layout()->addWidget(this->dataDisplayDT);
    // display mouseover data in a QFrame (maybe QWidget would be better??
    this->dataDisplayChannels = new QFrame();
    this->dataDisplayChannels->setLayout(new QVBoxLayout());
    this->dataDisplayFrame->layout()->addWidget(this->dataDisplayChannels);

    QSettings settings;
    settings.beginGroup(setcon::PLOT_GROUP);
    dataDisplayArea->setChecked(
        settings.value(setcon::PLOT_SHOW_DATA, true).toBool());
    generalShowGrid->setChecked(
        settings.value(setcon::PLOT_SHOW_GRID, true).toBool());
    generalShowLegend->setChecked(
        settings.value(setcon::PLOT_SHOW_LEGEND, false).toBool());
}

void PlottingArea::resetGraph()
{
    this->dataDisplayLabels.clear();

    utils::clearLayout(this->controlData->layout());
    utils::clearLayout(this->controlAppearance->layout());

    // TODO: Does this work? really have to check this!
    this->layout()->removeWidget(this->plot);
    delete this->plot;
    this->plot = new QCustomPlot();
    this->plot->setSizePolicy(QSizePolicy::Policy::Expanding,
                              QSizePolicy::Policy::Expanding);
    dynamic_cast<QGridLayout *>(this->layout())->addWidget(this->plot, 2, 0);

    utils::clearLayout(this->dataDisplayChannels->layout());
}

void PlottingArea::setupGraphPlot(const QSettings &settings)
{
    if (settings.value(setcon::DEVICE_POLL_FREQ).toInt() < 1000) {
        highZoom = 1800;
    }

    if (settings.value(setcon::DEVICE_POLL_FREQ).toInt() > 1000) {
        highZoom = 7200;
    }

    // define interactions
    this->plot->setInteractions(QCP::Interaction::iRangeDrag |
                                QCP::Interaction::iRangeZoom |
                                QCP::Interaction::iSelectLegend);

    // don't add graphs to legend automagically
    this->plot->setAutoAddPlottableToLegend(false);
    // TODO: How can we be sure the legend has index 0?
    // Place legend top left
    this->plot->axisRect()->insetLayout()->setInsetAlignment(
        0, Qt::AlignLeft | Qt::AlignTop);

    // zooming and dragging only in horizontal direction (no zooming on yAxis)
    this->plot->axisRect()->setRangeZoom(Qt::Orientation::Horizontal);
    this->plot->axisRect()->setRangeDrag(Qt::Orientation::Horizontal);

    this->voltageAxis = this->plot->yAxis;
    this->currentAxis =
        this->plot->axisRect()->addAxis(QCPAxis::AxisType::atLeft);
    this->wattageAxis =
        this->plot->axisRect()->addAxis(QCPAxis::AxisType::atLeft);
    this->voltageAxis->setLabel("Voltage V");
    this->voltageAxis->setRangeLower(
        settings.value(setcon::DEVICE_VOLTAGE_MIN).toDouble() - 1);
    this->voltageAxis->setRangeUpper(
        settings.value(setcon::DEVICE_VOLTAGE_MAX).toDouble() + 1);
    this->currentAxis->setLabel("Current A");
    this->currentAxis->setRangeLower(
        settings.value(setcon::DEVICE_CURRENT_MIN).toDouble() - 1);
    this->currentAxis->setRangeUpper(
        settings.value(setcon::DEVICE_CURRENT_MAX).toDouble() + 1);
    this->wattageAxis->setLabel("Wattage W");
    this->wattageAxis->setRangeLower(
        settings.value(setcon::DEVICE_VOLTAGE_MIN).toDouble() *
            settings.value(setcon::DEVICE_CURRENT_MIN).toDouble() -
        1);
    this->wattageAxis->setRangeUpper(
        settings.value(setcon::DEVICE_VOLTAGE_MAX).toDouble() *
            settings.value(setcon::DEVICE_CURRENT_MAX).toDouble() +
        1);
    // xaxis label and ticks
    this->plot->xAxis->setLabel("Time");
    this->plot->xAxis->setTickLabelType(QCPAxis::LabelType::ltDateTime);
    this->plot->xAxis->setDateTimeFormat("HH:mm:ss");
    // this->plot->xAxis->setAutoTickStep(false);
    // this->plot->xAxis->setTickStep(30);
    this->plot->xAxis->setTickLabelRotation(45);
    this->plot->axisRect()->setupFullAxesBox(true);
    auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     this->startPoint.time_since_epoch())
                     .count();
    // initial range
    this->plot->xAxis->setRange(msecs / 1000.0, 300, Qt::AlignRight);

    QObject::connect(
        this->plot->xAxis,
        static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(
            &QCPAxis::rangeChanged),
        this, &PlottingArea::xAxisRangeChanged);
    QObject::connect(this->plot, &QCustomPlot::beforeReplot, this,
                     &PlottingArea::beforeReplotHandle);

    QObject::connect(this->plot, &QCustomPlot::mouseMove, this,
                     &PlottingArea::mouseMoveHandler);
}

void PlottingArea::yAxisRange(const QCPRange &currentXRange,
                              const QSettings &settings)
{
    YAxisHelper yax;
    YAxisBounds yaxb =
        yax.getyAxisBounds(currentXRange, this->plot,
                           settings.value(setcon::DEVICE_CHANNELS).toInt());

    this->voltageAxis->setRange(
        QCPRange(yaxb.voltageLower - 0.5, yaxb.voltageUpper + 0.5));
    this->currentAxis->setRange(
        QCPRange(yaxb.currentLower - 0.5, yaxb.currentUpper + 0.5));
    this->wattageAxis->setRange(
        QCPRange(yaxb.wattageLower - 0.5, yaxb.wattageUpper + 0.5));

    // replotting here causes some weird effects.
    // this->plot->replot();
}

void PlottingArea::yAxisVisibility()
{
    QSettings settings;
    settings.beginGroup(setcon::DEVICE_GROUP);
    settings.beginGroup(settings.value(setcon::DEVICE_ACTIVE).toString());

    int voltage = 0;
    int current = 0;
    int wattage = 0;

    int channel = 0;
    for (int i = 0; i < this->plot->graphCount(); i++) {
        if (i != 0 && i % 5 == 0)
            channel++;
        int dtindex = i - (channel * 5);
        globcon::DATATYPE dt = static_cast<globcon::DATATYPE>(dtindex);
        if (dt == globcon::DATATYPE::VOLTAGE ||
            dt == globcon::DATATYPE::SETVOLTAGE) {
            if (this->plot->graph(i)->visible())
                voltage = voltage | 1;
            continue;
        }
        if (dt == globcon::DATATYPE::CURRENT ||
            dt == globcon::DATATYPE::SETCURRENT) {
            if (this->plot->graph(i)->visible())
                current = current | 1;
            continue;
        }
        if (this->plot->graph(i)->visible())
            wattage = wattage | 1;
    }

    if (voltage == 0) {
        if (this->voltageAxis->visible())
            this->voltageAxis->setVisible(false);
    } else {
        if (!this->voltageAxis->visible())
            this->voltageAxis->setVisible(true);
    }
    if (current == 0) {
        if (this->currentAxis->visible())
            this->currentAxis->setVisible(false);
    } else {
        if (!this->currentAxis->visible())
            this->currentAxis->setVisible(true);
    }
    if (wattage == 0) {
        if (this->wattageAxis->visible())
            this->wattageAxis->setVisible(false);
    } else {
        if (!this->wattageAxis->visible())
            this->wattageAxis->setVisible(true);
    }
    this->plot->replot();
}

void PlottingArea::beforeReplotHandle()
{
    // qDebug() << Q_FUNC_INFO << "before replot";
}

void PlottingArea::xAxisRangeChanged(const QCPRange &newRange,
                                     const QCPRange &oldRange)
{
    QSettings settings;
    settings.beginGroup(setcon::DEVICE_GROUP);
    settings.beginGroup(settings.value(setcon::DEVICE_ACTIVE).toString());

    long newRangeUpperMS = static_cast<long>(newRange.upper * 1000);
    std::chrono::milliseconds newRangeUpperMSDuration(newRangeUpperMS);
    long newRangeLowerMS = static_cast<long>(newRange.lower * 1000);
    std::chrono::milliseconds newRangeLowerMSDuration(newRangeLowerMS);

    // generate a timepoint from newRange upper and lower
    std::chrono::system_clock::time_point tpNewUpper;
    tpNewUpper =
        tpNewUpper + std::chrono::duration_cast<std::chrono::milliseconds>(
                         newRangeUpperMSDuration);
    std::chrono::system_clock::time_point tpNewLower;
    tpNewLower =
        tpNewLower + std::chrono::duration_cast<std::chrono::milliseconds>(
                         newRangeLowerMSDuration);
    // get the upper timepoint based on seconds. to many bit flips with
    // milliseconds precision
    std::chrono::system_clock::time_point tpNewUpperSecs =
        std::chrono::time_point_cast<std::chrono::seconds>(tpNewUpper);
    std::chrono::system_clock::time_point tpNewLowerSecs =
        std::chrono::time_point_cast<std::chrono::seconds>(tpNewLower);

    // calculate delta between upper and lower
    std::chrono::duration<double> deltaSecsUpperLower =
        tpNewUpperSecs - tpNewLowerSecs;

    long oldRangeUpperMS = static_cast<long>(oldRange.upper * 1000);
    std::chrono::milliseconds oldRangeUpperMSDuration(oldRangeUpperMS);
    long oldRangeLowerMS = static_cast<long>(oldRange.lower * 1000);
    std::chrono::milliseconds oldRangeLowerMSDuration(oldRangeLowerMS);

    // generate a timepoint from newRange upper and lower
    std::chrono::system_clock::time_point tpOldUpper;
    tpOldUpper =
        tpOldUpper + std::chrono::duration_cast<std::chrono::milliseconds>(
                         oldRangeUpperMSDuration);
    std::chrono::system_clock::time_point tpOldLower;
    tpOldLower =
        tpOldLower + std::chrono::duration_cast<std::chrono::milliseconds>(
                         oldRangeLowerMSDuration);
    // get the upper timepoint based on seconds. to many bit flips with
    // milliseconds precision
    std::chrono::system_clock::time_point tpOldUpperSecs =
        std::chrono::time_point_cast<std::chrono::seconds>(tpOldUpper);
    std::chrono::system_clock::time_point tpOldLowerSecs =
        std::chrono::time_point_cast<std::chrono::seconds>(tpOldLower);

    // calculate delta between upper and lower
    std::chrono::duration<double> deltaSecsOldUpperLower =
        tpOldUpperSecs - tpOldLowerSecs;

    // get a seconds based timepoint from the last key that was set when data was
    // added
    std::chrono::system_clock::time_point currentDataPointKeySecs =
        std::chrono::time_point_cast<std::chrono::seconds>(
            this->currentDataPointKey);
    auto curDataPointmsEpoch =
        std::chrono::time_point_cast<std::chrono::milliseconds>(
            this->currentDataPointKey)
            .time_since_epoch();
    std::chrono::system_clock::time_point startPointSecs =
        std::chrono::time_point_cast<std::chrono::seconds>(this->startPoint);
    auto start_msEpoch =
        std::chrono::time_point_cast<std::chrono::milliseconds>(this->startPoint)
            .time_since_epoch();

    // check if we need to toggle autoScroll
    if (this->autoScroll && tpNewUpperSecs < currentDataPointKeySecs) {
        // Stop autoscroll when panning to the left
        this->cbGeneralAutoscrl->setCheckState(Qt::CheckState::Unchecked);
    } else if (!this->autoScroll && tpNewUpperSecs >= currentDataPointKeySecs) {
        // Panning to the most recent x value starts autoscrolling
        this->cbGeneralAutoscrl->setCheckState(Qt::CheckState::Checked);
    }

    if (tpNewUpperSecs > currentDataPointKeySecs) {
        double maxUpper = (curDataPointmsEpoch.count() / 1000.0);
        QCPRange updatedRange(maxUpper - newRange.size(), maxUpper);
        //        if (deltaSecsUpperLower == deltaSecsOldUpperLower)
        //            updatedRange.lower = maxUpper - newRange.size();
        QObject::disconnect(
            this->plot->xAxis,
            static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(
                &QCPAxis::rangeChanged),
            0, 0);
        this->plot->xAxis->setRange(updatedRange);
        QObject::connect(
            this->plot->xAxis,
            static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(
                &QCPAxis::rangeChanged),
            this, &PlottingArea::xAxisRangeChanged);
        return;
    }

    // don't pan to much to the left. startPoint is the lowest possible time key
    if (tpNewUpperSecs < startPointSecs) {
        double maxUpper = (start_msEpoch.count() / 1000.0);
        QCPRange updatedRange(maxUpper - newRange.size(), maxUpper);
        QObject::disconnect(
            this->plot->xAxis,
            static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(
                &QCPAxis::rangeChanged),
            0, 0);
        this->plot->xAxis->setRange(updatedRange);
        QObject::connect(
            this->plot->xAxis,
            static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(
                &QCPAxis::rangeChanged),
            this, &PlottingArea::xAxisRangeChanged);
        return;
    }

    // limit zoom
    if (deltaSecsUpperLower < std::chrono::duration<double>(this->lowZoom) ||
        deltaSecsUpperLower > std::chrono::duration<double>(this->highZoom)) {
        QObject::disconnect(
            this->plot->xAxis,
            static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(
                &QCPAxis::rangeChanged),
            0, 0);
        QCPRange updatedRange = oldRange;
        if (deltaSecsOldUpperLower >
            std::chrono::duration<double>(this->highZoom)) {
            updatedRange.lower = (curDataPointmsEpoch.count() / 1000.0) -
                                 static_cast<double>(this->highZoom);
            updatedRange.upper = curDataPointmsEpoch.count() / 1000.0;
        }
        if (deltaSecsOldUpperLower <
            std::chrono::duration<double>(this->highZoom)) {
            updatedRange.lower = (curDataPointmsEpoch.count() / 1000.0) -
                                 static_cast<double>(this->lowZoom);
            updatedRange.upper = curDataPointmsEpoch.count() / 1000.0;
        }
        this->plot->xAxis->setRange(updatedRange);
        QObject::connect(
            this->plot->xAxis,
            static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(
                &QCPAxis::rangeChanged),
            this, &PlottingArea::xAxisRangeChanged);
        // TODO: Use the statusbar to inform user about minimum maximum zoom
        // level.
        return;
    }

    // first set y axis range
    // this->yAxisRange(newRange, settings);
}

void PlottingArea::mouseMoveHandler(QMouseEvent *event)
{
    // don't do anything if there are no graphs.
    // TODO: I don't know but I think this check is nonsense.
    if (this->plot->graphCount() == 0)
        return;

    QSettings settings;
    settings.beginGroup(setcon::DEVICE_GROUP);
    settings.beginGroup(settings.value(setcon::DEVICE_ACTIVE).toString());
    if (settings.contains(setcon::DEVICE_CHANNELS)) {
        // get the coordinate on the x axis from the event position
        double x = this->plot->xAxis->pixelToCoord(event->pos().x());
        // calculate a datetime object fromthe x coordinate
        QDateTime dateTime =
            QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(x * 1000));
        this->dataDisplayDT->setText(
            dateTime.toString("yyyy-MM-dd HH:mm:ss.zzz"));
        for (int i = 1; i <= settings.value(setcon::DEVICE_CHANNELS).toInt();
             i++) {
            for (int c = 0; c < 5; c++) {
                // which datatype is this
                globcon::DATATYPE dt = static_cast<globcon::DATATYPE>(c);
                // if the graph is visible get the data
                if (this->plot->graph(c)->visible()) {
                    QCPDataMap::Iterator setVit =
                        this->plot->graph(c)->data()->upperBound(x);
                    if (setVit != this->plot->graph(c)->data()->end()) {
                        int accuracy = 3;
                        if (dt == globcon::VOLTAGE ||
                            dt == globcon::SETVOLTAGE) {
                            accuracy =
                                settings.value(setcon::DEVICE_VOLTAGE_ACCURACY)
                                    .toInt();
                        }
                        if (dt == globcon::CURRENT ||
                            dt == globcon::SETCURRENT) {
                            accuracy =
                                settings.value(setcon::DEVICE_CURRENT_ACCURACY)
                                    .toInt();
                        }
                        this->dataDisplayLabels.at(static_cast<globcon::CHANNEL>(
                                                       i))
                            .at(dt)
                            ->setText(
                                QString::number(setVit->value, 'f', accuracy));
                    }
                } else {
                    this->dataDisplayLabels.at(static_cast<globcon::CHANNEL>(i))
                        .at(dt)
                        ->setText("--");
                }
            }
        }
    }
}

void PlottingArea::toolbarActionTriggered(QAction *action)
{
    // set stackedControl to a widget according to action
    if (action == this->actionGeneral) {
        this->controlStack->setCurrentWidget(this->controlGeneralScroll);
    } else if (action == this->actionData) {
        this->controlStack->setCurrentWidget(this->controlDataScroll);
    } else if (action == this->actionAppearance) {
        this->controlStack->setCurrentWidget(this->controlAppearanceScroll);
    }
    if (lastAction == action) {
        if (this->controlStack->maximumHeight() > 0) {
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(0))
                ->setStartValue(120);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(0))
                ->setEndValue(0);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(1))
                ->setStartValue(120);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(1))
                ->setEndValue(0);
            this->animationGroupControl->start();
        } else {
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(0))
                ->setStartValue(0);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(0))
                ->setEndValue(120);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(1))
                ->setStartValue(0);
            dynamic_cast<QPropertyAnimation *>(
                this->animationGroupControl->animationAt(1))
                ->setEndValue(120);
            this->animationGroupControl->start();
        }
    } else if (this->controlStack->maximumHeight() == 0) {
        dynamic_cast<QPropertyAnimation *>(
            this->animationGroupControl->animationAt(0))
            ->setStartValue(0);
        dynamic_cast<QPropertyAnimation *>(
            this->animationGroupControl->animationAt(0))
            ->setEndValue(120);
        dynamic_cast<QPropertyAnimation *>(
            this->animationGroupControl->animationAt(1))
            ->setStartValue(0);
        dynamic_cast<QPropertyAnimation *>(
            this->animationGroupControl->animationAt(1))
            ->setEndValue(120);
        this->animationGroupControl->start();
    }
    this->lastAction = action;
}

void PlottingArea::generalCBCheckState(int state)
{
    if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
        this->autoScroll = true;
    } else {
        this->autoScroll = false;
    }
}
