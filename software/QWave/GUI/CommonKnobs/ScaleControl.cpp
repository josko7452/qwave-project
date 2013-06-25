//
//   ScaleControl.cpp
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


#include "ScaleControl.h"
#include <QDebug>

ScaleControl::ScaleControl(QWidget * parent) : QWidget(parent)
{
    ui.setupUi(this);
}

void ScaleControl::connectPlotCanvas(PlotCanvas* plotCanvas)
{
    connect(ui.pushButtonPlus, SIGNAL(clicked()), plotCanvas, SLOT(zoomIn()));
    connect(ui.pushButtonMinus, SIGNAL(clicked()), plotCanvas, SLOT(zoomOut()));
    connect(plotCanvas, SIGNAL(scaleUpdated(uint64_t)), this, SLOT(updateScale(uint64_t)));
}

/**
 * @brief ScaleControl::getScaleText Traslates scale in ps to human readable format.
 * @param value
 * @return
 */
QString ScaleControl::getScaleText(uint64_t value)
{
    qDebug() << "Text from value (getScaleText) " << value;
    if(value > E12)
    {
        return (QString::number((double)value/E9, 'f', 2)+" "+DIV_S);
    }
    else if(value > E9)
    {
        return (QString::number((double)value/E9, 'f', 2)+" "+DIV_MS);
    }
    else if(value > E6)
    {
        return (QString::number((double)value/E6, 'f', 2)+" "+DIV_US);
    }
    else if(value > E3)
    {
        return (QString::number((double)value/E3, 'f', 2)+" "+DIV_NS);
    }
    else
    {
        return (QString::number(value)+" "+DIV_PS);
    }
}

/**
 * @brief ScaleControl::scaleToFrequency translates scale in ps to equivalent frequency.
 * @param scale
 */
void ScaleControl::scaleToFrequency(uint64_t scale)
{
    if(scale == 0)
        return;
   double frequency = E15/(double)scale;
   if(frequency < E3)
   {
       ui.frequencyLabel->setText("Freq: "+QString::number(static_cast<double>(frequency), 'f', 2)+E3_HZ);
   }
   else
   if (frequency < E6)
   {
       ui.frequencyLabel->setText("Freq: "+QString::number(static_cast<double>(frequency)/E3, 'f', 2)+HERTZ);
   }
   else
   if(frequency < E9)
   {
       ui.frequencyLabel->setText("Freq: "+QString::number(static_cast<double>(frequency)/E6, 'f', 2)+KHZ);
   }
   else
   {
       ui.frequencyLabel->setText("Freq: "+QString::number(static_cast<double>(frequency)/E9, 'f', 2)+MHZ);
   }
}

void ScaleControl::updateScale(uint64_t scale)
{
    ui.scaleLabelv->setText(getScaleText(scale));
    scaleToFrequency(scale);
}

ScaleControl::~ScaleControl()
{

}

