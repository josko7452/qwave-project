//
//   TimeControl.cpp
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


#include "TimeControl.h"
#include <QDebug>

TimeControl::TimeControl(QWidget * parent) : QWidget(parent)
{
    ui.setupUi(this);
    ui.beginTimeSpinBox->setRange(0, 100000000000000ULL);
    ui.beginTimeSpinBox->setValue(0.0);
    ui.beginTimeSpinBox->setSingleStep(1000);
    connect(ui.beginTimeSpinBox, SIGNAL(editingFinished()), this, SLOT(updateValuesFrom()));
}

TimeControl::~TimeControl()
{

}

void TimeControl::updateValuesFrom()
{
    qDebug() <<"Spin box value: " << ui.beginTimeSpinBox->value();
    emit valueChanged(ui.beginTimeSpinBox->value());
}

void TimeControl::updateFrom(uint64_t fromInNs)
{
    ui.beginTimeSpinBox->setValue(fromInNs);
}

/**
 * @brief TimeControl::getScaleText returns scale in human readable format from ps.
 * @param value
 * @return
 */
QString TimeControl::getScaleText(uint64_t value)
{
    qDebug() << "Text from value (getScaleText) " << value;
    if(value > E12)
    {
        return (QString::number(value/E9)+" "+S);
    }
    else if(value > E9)
    {
        return (QString::number(value/E9)+" "+MS);
    }
    else if(value > E6)
    {
        return (QString::number(value/E6)+" "+US);
    }
    else if(value > E3)
    {
        return (QString::number(value/E3)+" "+NS);
    }
    else
    {
        return (QString::number(value)+" "+S);
    }
}

void TimeControl::updateTo(uint64_t toInPs)
{
    ui.to->setText(getScaleText(toInPs));
}
