//
//   TimeControl.h
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


#ifndef TIMECONTROL_H
#define TIMECONTROL_H

#include <QWidget>
#include <QLabel>
#include "../../Datamodel/Constants.h"
#include "TimeSpinBox.h"
#include <ui_TimeControl.h>
#include <inttypes.h>

/**
 * @brief The TimeControl class represents knobs to control canvas time.
 */
class TimeControl : public QWidget
{
    Q_OBJECT
private:
    Ui::TimeControl ui;

private slots:
    void updateValuesFrom();
    QString getScaleText(uint64_t value);
signals:
    void valueChanged(uint64_t);
public:
    void updateFrom(uint64_t fromInNs);
    uint64_t getFrom() { return ui.beginTimeSpinBox->value(); }
    void updateTo(uint64_t toInNs);

    explicit TimeControl(QWidget * parent = 0);
    ~TimeControl();
};

#endif // TIMECONTROL_H
