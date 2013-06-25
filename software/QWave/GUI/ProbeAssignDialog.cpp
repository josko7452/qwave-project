//
//   ProbeAssignDialog.cpp
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

#include "ProbeAssignDialog.h"
#include "ui_ProbeAssignDialog.h"

#define LOGIC_MAX_BITS 16

ProbeAssignDialog::ProbeAssignDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProbeAssignDialog)
{
    ui->setupUi(this);
    connect(ui->analogRadio, SIGNAL(released()), this, SLOT(radioChanged()));
    connect(ui->wireRadio, SIGNAL(released()), this, SLOT(radioChanged()));
    connect(ui->registerRadio, SIGNAL(released()), this, SLOT(radioChanged()));
    radioChanged();
}

/**
 * @brief ProbeAssignDialog::radioChanged triggered when user selects radio button to chose which signal type to assign.
 * Changes page of stacked widget accordingly.
 */
void ProbeAssignDialog::radioChanged()
{
    if(ui->analogRadio->isChecked())
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else if(ui->wireRadio->isChecked())
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
    else if(ui->registerRadio->isChecked())
    {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

ProbeAssignDialog::~ProbeAssignDialog()
{
    delete ui;
}

/**
 * @brief ProbeAssignDialog::getDevicePointer
 * @param controller
 * @return returns pointer to selected device.
 */
AbstractDevice* ProbeAssignDialog::getDevicePointer(DeviceController * controller)
{
    return controller->getDevice(ui->probeChoserCombo->currentIndex());
}

/**
 * @brief ProbeAssignDialog::assignDialog Presents actual assign dialog.
 *  When users selects signal to assign and selects OK this method
 *  carries out creating appropriate plot and assigning device to this plot.
 * @param controller
 * @param board
 */
void ProbeAssignDialog::assignDialog(CaptureController * controller, Board * board)
{
    //load list of probes
    for(int i = 0; i < controller->getDeviceController()->getlistOfDevices()->count(); ++i)
    {
        ui->probeChoserCombo->addItem(controller->getDeviceController()->getDevice(i)->getName());
    }
    if(controller->isAnalog1Free(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())))
        ui->channelCombo->addItem("Channel 1");
    if(controller->isAnalog2Free(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())))
        ui->channelCombo->addItem("Channel 2");
    //create bitwidth list
    for(int i = 1; i < LOGIC_MAX_BITS-controller->getCurrentWireIndex(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex()))+1; ++i)
    {
        ui->widthCombo->addItem(QString::number(i));
    }
    ui->regWireNumber->setText(QString::number(controller->getCurrentWireIndex(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex()))));
    ui->wireNumber->setText(QString::number(controller->getCurrentWireIndex(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex()))));

    int result = this->exec();
    if(result == QDialog::Accepted)
    {
        Plot* plot;
        int type;
        PlotData* newData;
        plot = board->createMeasurementPlot(type);
        if(ui->analogRadio->isChecked())
        {
            if(!controller->isAnalog1Free(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())) and
               !controller->isAnalog2Free(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())))
                return;
            type = PlotData::Linear;
            newData = new PlotData(type, plot->getName(), (uint64_t)1,  1, board->getRandomSign());
            if(!controller->isAnalog1Free(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())))
                controller->assignDevicePlotData(newData, getDevicePointer(controller->getDeviceController()),
                                                 1, 0, 0);
            else
                if(!controller->isAnalog2Free(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())))
                    controller->assignDevicePlotData(newData, getDevicePointer(controller->getDeviceController()),
                                                     0, 0, 0);
                else
                    controller->assignDevicePlotData(newData, getDevicePointer(controller->getDeviceController()),
                                                     ui->channelCombo->currentIndex(), 0, 0);
        }
        else if(ui->wireRadio->isChecked())
        {
            type = PlotData::Logic;
            newData = new PlotData(type, plot->getName(), (uint64_t)1,  1, board->getRandomSign());
            controller->assignDevicePlotData(newData, getDevicePointer(controller->getDeviceController()), 2,
                                                                 controller->getCurrentWireIndex(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())), 1);
        }
        else if(ui->registerRadio->isChecked())
        {
            type = PlotData::Logic;
            newData = new PlotData(type, plot->getName(), (uint64_t)1,  ui->widthCombo->currentIndex()+1, board->getRandomSign());
            controller->assignDevicePlotData(newData, getDevicePointer(controller->getDeviceController()), 2,
                                                                 controller->getCurrentWireIndex(controller->getDeviceController()->getDevice(ui->probeChoserCombo->currentIndex())),
                                                                 ui->widthCombo->currentIndex()+1);
        }
        newData->setDiv(8000);
        plot->setData(newData);
        board->registerData(newData);
    }
}
