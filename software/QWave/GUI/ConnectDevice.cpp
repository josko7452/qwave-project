//
//   ConnectDevice.cpp
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

#include "ConnectDevice.h"
#include "ui_ConnectDevice.h"
#include "../Exceptions/DeviceException.h"
#include <QMessageBox>

ConnectDevice::ConnectDevice(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDevice)
{
    ui->setupUi(this);
    ui->comboBox->addItem("QWave FT245 probe");
    ui->comboBox->addItem("DUMMY");
}

ConnectDevice::~ConnectDevice()
{
    delete ui;
}

/**
 * This method presents dialog itself and then initiates connection of selected device if OK was pressed.
 * @brief ConnectDevice::connectDevice
 * @param controller
 */
void ConnectDevice::connectDevice(CaptureController* controller)
{
    int result = this->exec();
    if(result == QDialog::Accepted)
    {
        try
        {
            if(ui->comboBox->currentIndex() == 0)
            {
                controller->addFt245Device();
            }
            else
            {
                controller->addDummyDevice();
            }
        }
        catch(DeviceException ex)
        {
            QMessageBox::warning(this, "Connection error", ex.getMessage());
        }
    }
}
