//
//  TriggerSetupDIalog.cpp
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


#include "TriggerSetupDialog.h"
#include "ui_TriggerSetupDialog.h"
#include "../Device/Capabilities.h"
#include <QMessageBox>

#define DEC 0
#define HEX 1
#define BIN 2
#define OCT 3
#define WRONG_DATA "Failed to parse numbers, incorrect data provided"

TriggerSetupDialog::TriggerSetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TriggerSetupDialog)
{
    ui->setupUi(this);
}

TriggerSetupDialog::~TriggerSetupDialog()
{
    delete ui;
}

/**
 * @brief TriggerSetupDialog::assignTrigger this function presents dialog and when users
 * clicks OK it assigns trigger to selected device's probe.
 * @param device
 * @param kind
 * @param channel
 */
void TriggerSetupDialog::assignTrigger(AbstractDevice* device, int kind, int channel)
{
    QPair<int, QVector<uint8_t> > triggerSetup = device->getTrigger(kind, channel);
    if(kind == ANALOG)
    {
        ui->stackedWidget->setCurrentIndex(0);
        if(triggerSetup.first == RISING)
        {
            ui->risingRadio->toggle();
            ui->thresholdRadio->toggle();
        }
        else if(triggerSetup.first == FALLING)
        {
            ui->fallingRadio->toggle();
            ui->thresholdRadio->toggle();
        }
        else if(triggerSetup.first == RISING_SLOPE)
        {
            ui->risingRadio->toggle();
            ui->edgeRadio->toggle();
        }
        else if(triggerSetup.first == FALLING_SLOPE)
        {
            ui->fallingRadio->toggle();
            ui->edgeRadio->toggle();
        }
        ui->valueSpinBox->setValue(triggerSetup.second.at(0));
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
    ui->formatCombo->addItem("Decimal");
    ui->formatCombo->addItem("Hex");
    ui->formatCombo->addItem("Binary");
    ui->formatCombo->addItem("Octal");
    for(int i = 1; i < 5; ++i)
    {
        ui->samplesCombo->addItem(QString::number(i));
    }
    ui->formatCombo->setCurrentIndex(0);
    if(triggerSetup.second.count() > 0)
    {
        ui->sample1Edit->setText(QString::number(triggerSetup.second.at(0)));
        ui->samplesCombo->setCurrentIndex(0);
    }
    if(triggerSetup.second.count() > 1)
    {
        ui->sample2Edit->setText(QString::number(triggerSetup.second.at(1)));
        ui->samplesCombo->setCurrentIndex(1);
    }
    if(triggerSetup.second.count() > 2)
    {
        ui->sample3Edit->setText(QString::number(triggerSetup.second.at(2)));
        ui->samplesCombo->setCurrentIndex(2);
    }
    if(triggerSetup.second.count() > 3)
    {
        ui->sample4Edit->setText(QString::number(triggerSetup.second.at(3)));
        ui->samplesCombo->setCurrentIndex(3);
    }

    int result = this->exec();
    if(result == QDialog::Accepted)
    {
        if(kind == ANALOG)
        {
            QVector<uint8_t> val;
            val.push_back(ui->valueSpinBox->value());
            if(ui->edgeRadio->isChecked() and ui->risingRadio->isChecked())
            {
                device->setTrigger(kind,  RISING_SLOPE, val, channel);
            }
            else if(ui->edgeRadio->isChecked() and ui->fallingRadio->isChecked())
            {
                device->setTrigger(kind, FALLING_SLOPE, val, channel);
            }
            else if(ui->thresholdRadio->isChecked() and ui->risingRadio->isChecked())
            {
                device->setTrigger(kind, RISING, val, channel);
            }
            else if(ui->thresholdRadio->isChecked() and ui->fallingRadio->isChecked())
            {
                device->setTrigger(kind, FALLING, val, channel);
            }
        }
        else
        {
            QVector<uint8_t> val;
            int base;
            if(ui->formatCombo->currentIndex() == DEC)
            {
                base = 10;
            }
            else if(ui->formatCombo->currentIndex() == HEX)
            {
                base = 16;
            }
            else if(ui->formatCombo->currentIndex() == OCT)
            {
                base = 8;
            }
            else if(ui->formatCombo->currentIndex() == BIN)
            {
                base = 2;
            }
            if(ui->samplesCombo->currentIndex() >= 0)
            {
                bool ok;
                uint8_t value = ui->sample1Edit->text().toUInt(&ok, base);
                if(not ok)
                {
                    QMessageBox::warning(this, "Data error", WRONG_DATA);
                    return;
                }
                val.push_back(value);
            }
            if(ui->samplesCombo->currentIndex() >= 1)
            {
                bool ok;
                uint8_t value = ui->sample2Edit->text().toUInt(&ok, base);
                if(not ok)
                {
                    QMessageBox::warning(this, "Data error", WRONG_DATA);
                    return;
                }
                val.push_back(value);
            }
            if(ui->samplesCombo->currentIndex() >= 2)
            {
                bool ok;
                uint8_t value = ui->sample3Edit->text().toUInt(&ok, base);
                if(not ok)
                {
                    QMessageBox::warning(this, "Data error", WRONG_DATA);
                    return;
                }
                val.push_back(value);
            }
            if(ui->samplesCombo->currentIndex() >= 3)
            {
                bool ok;
                uint8_t value = ui->sample4Edit->text().toUInt(&ok, base);
                if(not ok)
                {
                    QMessageBox::warning(this, "Data error", WRONG_DATA);
                    return;
                }
                val.push_back(value);
            }
            device->setTrigger(kind, ui->samplesCombo->currentIndex(), val, 0);
        }
    }
}
