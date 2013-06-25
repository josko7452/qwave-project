//
//   AbstractDevice.h
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

#ifndef ABSTRACTDEVICE_H
#define ABSTRACTDEVICE_H

#include <QVector>
#include <QObject>
#include <inttypes.h>
#include "Capabilities.h"
#include <QPair>
class CaptureController;
/**
 * @brief The AbstractDevice class represent abstract functions usable to deal with various concrete devices.
 */
class AbstractDevice : public QObject
{
public:
    virtual QString getName() = 0;
    virtual int getCapability() = 0;
    virtual QVector<uint8_t*> getRawDataList() = 0;
    virtual unsigned int getRawDataLength() = 0;
    virtual void lockData() = 0;
    virtual void unlockData() = 0;
    virtual void setTrigger(int kind, int type, QVector<uint8_t> values, int channel) = 0;
    virtual void setOffsetAndGain(QVector<uint8_t> top, QVector<uint8_t> bottom) = 0;
    virtual void setAttenuator(int channel, int attenuator) = 0;
    virtual void setCoupling(int channel, int coupling) = 0;
    virtual void setDecimation(int ratioBase, int style) = 0;
    virtual void setLogicSupply(uint8_t value) = 0;
    virtual uint8_t getLogicSupply() = 0;
    virtual QPair<int, QVector<uint8_t> > getTrigger(int kind, int channel) = 0;
    virtual QPair<QVector<uint8_t>, QVector<uint8_t> > getOffsetAndGain() = 0;
    virtual int getAttenuator(int channel) = 0;
    virtual int getCoupling(int channel) = 0;
    virtual QPair<int, int> getDecimation() = 0;
    virtual void armDevice(int armedChannels, CaptureController* capc) = 0;
};

#endif // ABSTRACTDEVICE_H
