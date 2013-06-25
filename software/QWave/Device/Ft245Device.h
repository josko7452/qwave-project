//
//   Ft245Device.h
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



#ifndef FT245DEVICE_H
#define FT245DEVICE_H

#include "AbstractDevice.h"
#include "ft245sync.h"
#include <QSemaphore>

static const char * DEVICE_NAME_FT245 = "QWave FT245 Probe";
#define DATA_LENGTH 65536
#define GPIO_STATE 0xFF

void writeWithAck(unsigned char* buf, Ft245sync* ft245cnt);

/**
 * @brief The Ft245DeviceCapturer class represents data capturer for QWave FT245 SYNC FIFO based  MSO scope.
 */
class Ft245DeviceCapturer : public QObject
{
    Q_OBJECT
    int armedChannels;
    Ft245sync* ft245cnt;
    uint8_t* rawAnalogDataAdc1;
    uint8_t* rawAnalogDataAdc2;
    uint16_t* rawDigitalData;
    QSemaphore* dataSemaphore;
signals:
    void newDataLength(unsigned int dataLengths);
    void end();
public:
    Ft245DeviceCapturer(int armedChannels, Ft245sync* ft245cnt,
                                     uint8_t* rawAnalogDataAdc1, uint8_t* rawAnalogDataAdc2,
                                     uint16_t* rawDigitalData, QSemaphore* dataSemaphore) :
        armedChannels(armedChannels),
        rawAnalogDataAdc1(rawAnalogDataAdc1),
        rawAnalogDataAdc2(rawAnalogDataAdc2),
        rawDigitalData(rawDigitalData),
        dataSemaphore(dataSemaphore),
        ft245cnt(ft245cnt) {}
public slots:
    void doCapture();
};

/**
 * @brief The Ft245Device class represents QWave FT245 SYNC FIFO based MSO scope controller.
 */
class Ft245Device : public AbstractDevice
{
    Q_OBJECT
    uint8_t rawAnalogDataAdc1[DATA_LENGTH];
    uint8_t rawAnalogDataAdc2[DATA_LENGTH];
    uint16_t rawDigitalData[DATA_LENGTH];
    QSemaphore dataSemaphore;
    Ft245sync* ft245cnt;
    QVector<uint8_t*> rawDataList;
    unsigned int dataLength;
    int analogTriggerType1;
    int analogTriggerType2;
    uint8_t analogTriggerValue1;
    uint8_t analogTriggerValue2;
    int digitalTriggerType;
    QVector<uint8_t> digitalTriggerValues;
    QVector<uint8_t> topVref;
    QVector<uint8_t> botVref;
    int attenuator1;
    int attenuator2;
    int channel1co;
    int channel2co;
    int ratioBase;
    int style;
    uint8_t logicSupply;
public:
    Ft245Device();
    ~Ft245Device() { delete(ft245cnt); }
    int getCapability() { return TWO_ANALOG_ONE_DIGITAL; }
    QVector<uint8_t*> getRawDataList();
    unsigned int getRawDataLength();
    QString getName() { return DEVICE_NAME_FT245; }
    void lockData();
    void unlockData();
    void setTrigger(int kind, int type, QVector<uint8_t> values, int channel);
    void setOffsetAndGain(QVector<uint8_t> top, QVector<uint8_t> bottom);
    void setAttenuator(int channel, int attenuator);
    void setCoupling(int channel, int coupling);
    void armDevice(int armedChannels, CaptureController* capc);
    void setDecimation(int ratioBase, int style);
    void setLogicSupply(uint8_t value);
    uint8_t getLogicSupply() { return logicSupply; }
    QPair<int, QVector<uint8_t> > getTrigger(int kind, int channel);
    QPair<QVector<uint8_t>, QVector<uint8_t> > getOffsetAndGain();
    int getAttenuator(int channel);
    int getCoupling(int channel);
    QPair<int, int> getDecimation();
public slots:
    void updateDataLength(unsigned int dataLength);
signals:
    void dataUpdated(); //emited when it is convient to refresh viewport
};

#endif // FT245DEVICE_H
