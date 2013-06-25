//
//   ScaleControl.h
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


#ifndef SCALECONTROL_H
#define SCALECONTROL_H

#include <inttypes.h>
#include <QWidget>
#include <QLabel>
#include "../../Datamodel/Constants.h"
#include <ui_ScaleControl.h>
#include "../PlotCanvas.h"

/**
 * @brief The ScaleControl class knobs to control canvas scale.
 */
class ScaleControl : public QWidget
{
    Q_OBJECT
private:
    Ui::ScaleControl ui;
    QString getScaleText(uint64_t value);
    void scaleToFrequency(uint64_t scale);
private slots:
    void updateScale(uint64_t scale);
public:

    void connectPlotCanvas(PlotCanvas* plotCanvas);
    explicit ScaleControl(QWidget * parent = 0);
    ~ScaleControl();
};

#endif // SCALECONTROL_H
