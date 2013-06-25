//
//   KnobsWidget.cpp
//   QWave
//   Copyright (c) 2012-2013, Bruno Kremel
//   All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    3. All advertising materials mentioning features or use of this software
//       must display the following acknowledgement:
//       This product includes software developed by Bruno Kremel.
//    4. Neither the name of Bruno Kremel nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY Bruno Kremel ''AS IS'' AND ANY
//    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL Bruno Kremel BE LIABLE FOR ANY
//    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "KnobsWidget.h"
#include "ui_KnobsWidget.h"
#include "Board.h"
#include "../Device/Capabilities.h"
#include <QPair>

#define ATTENUATION_LOW "1:2"
#define ATTENUATION_HIGH "1:10"
#define COUPLING_AC "AC"
#define COUPLING_DC "DC"

#define DECIMATION_0 "1:1"
#define DECIMATION_1 "1:2"
#define DECIMATION_2 "1:4"
#define DECIMATION_3 "1:8"
#define DECIMATION_4 "1:16"
#define DECIMATION_5 "1:32"
#define DECIMATION_6 "1:64"
#define DECIMATION_7 "1:128"
#define DECIMATION_8 "1:256"

#define DECIMATION_SIMPLE "Simple"
#define DECIMATION_DITHERING "Dithering"
#define DECIMATION_PEAK_DETECTION "Peak detection"
#define DECIMATION_SMOOTHING "Smoothing"

#define LEVEL_1_8 255
#define LEVEL_2_5 100
#define LEVEL_3_3 55
#define LEVEL_5_0 0

#define LEVEL_1_8_TEXT "1.8V"
#define LEVEL_2_5_TEXT "2.5V"
#define LEVEL_3_3_TEXT "3.3V"
#define LEVEL_5_0_TEXT "5.0V"

#define DECIMATION_0_DIV 8000
#define DECIMATION_1_DIV 16000
#define DECIMATION_2_DIV 32000
#define DECIMATION_3_DIV 64000
#define DECIMATION_4_DIV 128000
#define DECIMATION_5_DIV 256000
#define DECIMATION_6_DIV 512000
#define DECIMATION_7_DIV 1024000
#define DECIMATION_8_DIV 2048000

KnobsWidget::KnobsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KnobsWidget)
{
    ui->setupUi(this);
    ui->attenuatorCombo->addItem(ATTENUATION_HIGH);
    ui->attenuatorCombo->addItem(ATTENUATION_LOW);
    ui->couplingCombo->addItem(COUPLING_AC);
    ui->couplingCombo->addItem(COUPLING_DC);
    ui->decimationCombo->addItem(DECIMATION_0);
    ui->decimationCombo->addItem(DECIMATION_1);
    ui->decimationCombo->addItem(DECIMATION_2);
    ui->decimationCombo->addItem(DECIMATION_3);
    ui->decimationCombo->addItem(DECIMATION_4);
    ui->decimationCombo->addItem(DECIMATION_5);
    ui->decimationCombo->addItem(DECIMATION_6);
    ui->decimationCombo->addItem(DECIMATION_7);
    ui->decimationCombo->addItem(DECIMATION_8);
    ui->styleCombo->addItem(DECIMATION_SIMPLE);
    ui->styleCombo->addItem(DECIMATION_DITHERING);
    ui->styleCombo->addItem(DECIMATION_PEAK_DETECTION);
    ui->styleCombo->addItem(DECIMATION_SMOOTHING);
    ui->logicCombo->addItem(LEVEL_1_8_TEXT);
    ui->logicCombo->addItem(LEVEL_2_5_TEXT);
    ui->logicCombo->addItem(LEVEL_3_3_TEXT);
    ui->logicCombo->addItem(LEVEL_5_0_TEXT);

    connect(ui->attenuatorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(attenuatorChanged(int)));
    connect(ui->couplingCombo, SIGNAL(currentIndexChanged(int)), this,  SLOT(coupligChanged(int)));
    connect(ui->offsetSlider, SIGNAL(valueChanged(int)), this, SLOT(adjustOffset(int)));
    connect(ui->scaleSlider, SIGNAL(valueChanged(int)), this, SLOT(adjustScale(int)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(startCapture()));
    connect(ui->attenuatorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(attenuatorChanged(int)));
    connect(ui->decimationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(decimationChanged()));
    connect(ui->styleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(decimationChanged()));
    connect(ui->logicCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(logicLevelChanged(int)));
    ui->hideStack->setCurrentIndex(0);
    capturePending = false;
}

KnobsWidget::~KnobsWidget()
{
    delete ui;
}

/**
 * This method is called when user selects plot from list of plots.
 * It allows knob widget to update it state according to selected probe.
 * @brief KnobsWidget::setCurrentPlot
 * @param plot
 */
void KnobsWidget::setCurrentPlot(Plot* plot)
{
    this->currentPlot = plot;
    if(plot->isAnalog())
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
    ui->hideStack->setCurrentIndex(1); // unhide controls
    //set current status
    int index = static_cast<Board*>(board)->getCurrentDataId();
    if(currentDevice->getAttenuator(index) == ATTENUATOR_HIGH)
        ui->attenuatorCombo->setCurrentIndex(0);
    else
        ui->attenuatorCombo->setCurrentIndex(1);
    if(currentDevice->getCoupling(index) == AC_COUPLING)
        ui->couplingCombo->setCurrentIndex(0);
    else
        ui->couplingCombo->setCurrentIndex(1);
    QPair<int, int> decimation = currentDevice->getDecimation();
    ui->decimationCombo->setCurrentIndex(decimation.first);
    if(decimation.first == 0)
        plot->getData()->setDiv(DECIMATION_0_DIV);
    else if(decimation.first == 1)
        plot->getData()->setDiv(DECIMATION_1_DIV);
    else if(decimation.first == 2)
        plot->getData()->setDiv(DECIMATION_2_DIV);
    else if(decimation.first == 3)
        plot->getData()->setDiv(DECIMATION_3_DIV);
    else if(decimation.first == 4)
        plot->getData()->setDiv(DECIMATION_4_DIV);
    else if(decimation.first == 5)
        plot->getData()->setDiv(DECIMATION_5_DIV);
    else if(decimation.first == 6)
        plot->getData()->setDiv(DECIMATION_6_DIV);
    else if(decimation.first == 7)
        plot->getData()->setDiv(DECIMATION_7_DIV);
    else if(decimation.first == 8)
        plot->getData()->setDiv(DECIMATION_8_DIV);
    ui->styleCombo->setCurrentIndex(decimation.second);
    if(currentDevice->getLogicSupply() <= LEVEL_5_0)
        ui->logicCombo->setCurrentIndex(3);
    else if(currentDevice->getLogicSupply() <= LEVEL_3_3)
        ui->logicCombo->setCurrentIndex(2);
    else if(currentDevice->getLogicSupply() <= LEVEL_2_5)
        ui->logicCombo->setCurrentIndex(1);
    else
        ui->logicCombo->setCurrentIndex(0);
    if(plot->isAnalog())
    {
        QPair<QVector<uint8_t>, QVector<uint8_t> > vrefs = currentDevice->getOffsetAndGain();
        setOffsetScaleSliders(vrefs.first.at(index),  vrefs.second.at(index));
    }
}

/**
 * This method is called to set state of offset and scale sliders.
 * @brief KnobsWidget::setOffsetScaleSliders
 * @param top
 * @param bottom
 */
void KnobsWidget::setOffsetScaleSliders(uint8_t top, uint8_t bottom)
{
    uint8_t offset = (top+bottom)/2;
    double scale = UINT8_MAX/((double)top-bottom);
    ui->scaleSlider->blockSignals(true);
    ui->offsetSlider->blockSignals(true);
    ui->scaleSlider->setValue(scale*10);
    ui->offsetSlider->setValue(offset);
    ui->scaleSlider->blockSignals(false);
    ui->offsetSlider->blockSignals(false);
}

/**
 * This method is called when user adjust offset and it calculates appropriate reference voltages.
 * @brief KnobsWidget::adjustOffset
 * @param value
 */
void KnobsWidget::adjustOffset(int value)
{
    int index = static_cast<Board*>(board)->getCurrentDataId();
    QPair<QVector<uint8_t>, QVector<uint8_t> > vrefs = currentDevice->getOffsetAndGain();
    uint8_t top = vrefs.first.at(index);
    uint8_t bot = vrefs.second.at(index);
    int currentDifference = top-(top+bot)/2;
    if((value + currentDifference) > UINT8_MAX)
    {
        currentDifference += UINT8_MAX - (value + currentDifference);
    }
    if((value - currentDifference) < 0)
    {
        currentDifference += value - currentDifference;
    }
    vrefs.first[index] = value+currentDifference;
    vrefs.second[index] = value-currentDifference;
    currentDevice->setOffsetAndGain(vrefs.first, vrefs.second);
    setOffsetScaleSliders(vrefs.first.at(index),  vrefs.second.at(index));
}

/**
 * This method is called when user adjusts scale and it calculates appropriate reference volateges.
 * @brief KnobsWidget::adjustScale
 * @param value
 */
void KnobsWidget::adjustScale(int value)
{
    int currentDifference = UINT8_MAX/((value/10)*2);
    int index = static_cast<Board*>(board)->getCurrentDataId();
    QPair<QVector<uint8_t>, QVector<uint8_t> > vrefs = currentDevice->getOffsetAndGain();
    uint8_t top = vrefs.first.at(index);
    uint8_t bot = vrefs.second.at(index);
    int offset = (top+bot)/2;
    if(offset+currentDifference > UINT8_MAX)
    {
        offset += UINT8_MAX - (offset+currentDifference);
    }
    if((offset - currentDifference) < 0)
    {
        offset -= offset-currentDifference;
    }
    vrefs.first[index] = offset+currentDifference;
    vrefs.second[index] = offset-currentDifference;
    currentDevice->setOffsetAndGain(vrefs.first, vrefs.second);
    setOffsetScaleSliders(vrefs.first.at(index),  vrefs.second.at(index));
}

/**
 * This method is called when user changes state of attenuator.
 * @brief KnobsWidget::attenuatorChanged
 * @param attenuator
 */
void KnobsWidget::attenuatorChanged(int attenuator)
{
    int currentDataId = static_cast<Board*>(board)->getCurrentDataId();
    if(attenuator == 0)
    {
        currentDevice->setAttenuator(currentDataId, ATTENUATOR_HIGH);
    }
    else
    {
        currentDevice->setAttenuator(currentDataId, ATTENUATOR_LOW);
    }
}

/**
 * This method is called when user changes state of coupling.
 * @brief KnobsWidget::coupligChanged
 * @param coupling
 */
void KnobsWidget::coupligChanged(int coupling)
{
    int currentDataId = static_cast<Board*>(board)->getCurrentDataId();
    if(coupling == 0)
        currentDevice->setCoupling(currentDataId, AC_COUPLING);
    else
        currentDevice->setCoupling(currentDataId, DC_COUPLING);
}

/**
 * This method is called when user change decimation.
 * @brief KnobsWidget::decimationChanged
 */
void KnobsWidget::decimationChanged()
{
    currentDevice->setDecimation(ui->decimationCombo->currentIndex(), ui->styleCombo->currentIndex());
    for(int i = 0; i < static_cast<Board*>(board)->getPlotCanvas()->getPlots().count(); ++i)
    {
        Plot* plot = static_cast<Board*>(board)->getPlotCanvas()->getPlot(i);
        if(ui->decimationCombo->currentIndex() == 0)
            plot->getData()->setDiv(DECIMATION_0_DIV);
        else if(ui->decimationCombo->currentIndex() == 1)
            plot->getData()->setDiv(DECIMATION_1_DIV);
        else if(ui->decimationCombo->currentIndex() == 2)
            plot->getData()->setDiv(DECIMATION_2_DIV);
        else if(ui->decimationCombo->currentIndex()== 3)
            plot->getData()->setDiv(DECIMATION_3_DIV);
        else if(ui->decimationCombo->currentIndex() == 4)
            plot->getData()->setDiv(DECIMATION_4_DIV);
        else if(ui->decimationCombo->currentIndex() == 5)
            plot->getData()->setDiv(DECIMATION_5_DIV);
        else if(ui->decimationCombo->currentIndex() == 6)
            plot->getData()->setDiv(DECIMATION_6_DIV);
        else if(ui->decimationCombo->currentIndex() == 7)
            plot->getData()->setDiv(DECIMATION_7_DIV);
        else if(ui->decimationCombo->currentIndex() == 8)
            plot->getData()->setDiv(DECIMATION_8_DIV);
    }
    static_cast<Board*>(board)->refreshPlotNames();
}

/**
 * @brief KnobsWidget::logicLevelChanged this method is called
 * when user changes logic level.
 * @param logicLevel
 */
void KnobsWidget::logicLevelChanged(int logicLevel)
{
    if(logicLevel == 0)
    {
        currentDevice->setLogicSupply(LEVEL_1_8);
    }
    else if(logicLevel == 1)
    {
        currentDevice->setLogicSupply(LEVEL_2_5);
    }
    else if(logicLevel == 2)
    {
        currentDevice->setLogicSupply(LEVEL_3_3);
    }
    else if(logicLevel == 3)
    {
        currentDevice->setLogicSupply(LEVEL_5_0);
    }
}

/**
 * This method is called when user pushes Arm button.
 * It initietes data capture from device.
 * @brief KnobsWidget::startCapture
 */
void KnobsWidget::startCapture()
{
    if(this->capturePending)
    {
        ui->pushButton->setText("Arm");
        this->capturePending = false;
        static_cast<Board*>(board)->stopCapture();
    }
    else
    {
        if(ui->rearmCheck->checkState() == Qt::Checked)
        {
            ui->pushButton->setText("Stop");
            this->capturePending = true;
            static_cast<Board*>(board)->startCapture(true);
        }
        else
            static_cast<Board*>(board)->startCapture(false);
    }
}
