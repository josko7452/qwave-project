//
//   KnobsWidget.h
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

#ifndef KNOBSWIDGET_H
#define KNOBSWIDGET_H

#include <QWidget>
#include "../Device/AbstractDevice.h"
#include "Plot.h"

namespace Ui {
class KnobsWidget;
}

/**
 * This class represents knobs used to control digital or analog probe.
 * Contains controls to set scale/offset, decimation, logic level, coupling and attenuation.
 * @brief The KnobsWidget class
 */
class KnobsWidget : public QWidget
{
    Q_OBJECT
    QWidget * board;
    void setOffsetScaleSliders(uint8_t top, uint8_t bottom);
    Plot* currentPlot;
    bool capturePending;
public:
    AbstractDevice* currentDevice;
    explicit KnobsWidget(QWidget *parent = 0);
    ~KnobsWidget();
    void setCurrentDevice(AbstractDevice* device) { currentDevice = device; }
    void setCurrentPlot(Plot* plot);
    void setBoard(QWidget * board) { this->board = board; }
private slots:
    void decimationChanged();
    void attenuatorChanged(int attenuator);
    void adjustOffset(int value);
    void adjustScale(int value);
    void coupligChanged(int coupling);
    void logicLevelChanged(int logicLevel);
    void startCapture();
private:
    Ui::KnobsWidget *ui;
};

#endif // KNOBSWIDGET_H
