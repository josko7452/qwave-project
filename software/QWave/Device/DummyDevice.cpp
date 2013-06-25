//
//   DummyDevice.cpp
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


#include "DummyDevice.h"
#include <QThread>
#include <math.h>
#include "../Exceptions/DeviceException.h"
#include <QDebug>
#include "CaptureController.h"

DummyDevice::DummyDevice(int period, bool simulateFtdiError) : period(period),
                                            analogTriggerType1(RISING),
                                            analogTriggerType2(RISING),
                                            analogTriggerValue1(0),
                                            analogTriggerValue2(0),
                                            digitalTriggerType(SIGNLE_SAMPLE),
                                            attenuator1(ATTENUATOR_HIGH),
                                            attenuator2(ATTENUATOR_HIGH),
                                            channel1co(AC_COUPLING),
                                            channel2co(AC_COUPLING),
                                            ratioBase(0),
                                            style(0),
                                            logicSupply(100),
                                            dataSemaphore(1)
{
    rawDataList.push_back(rawAnalogDataAdc1);
    rawDataList.push_back(rawAnalogDataAdc2);
    rawDataList.push_back((uint8_t*)rawDigitalData);
    if(simulateFtdiError)
    {
        throw DeviceException("FTDI device unavailable (simulated)", FTDI_UNAVAILABLE);
    }
    digitalTriggerValues.push_back(0xAA);
    digitalTriggerValues.push_back(0xAB);
    digitalTriggerValues.push_back(0xAC);
    digitalTriggerValues.push_back(0xAD);
    topVref.push_back(100);
    topVref.push_back(120);
    botVref.push_back(30);
    botVref.push_back(40);
}

DummyDevice::~DummyDevice()
{
}

/**
 * @brief DummyDevice::getRawDataList
 * @return returns list of raw data, it is shared data so it must be locked !!!
 *  by lockData() to prevent race condition and unlocked by unlockData() after
 * use to prevent deadlock.
 */
QVector<uint8_t*> DummyDevice::getRawDataList()
{
    return rawDataList;
}

/**
 * @brief DummyDevice::getRawDataLength
 * @return returns lenght of captured data.
 */
unsigned int DummyDevice::getRawDataLength()
{
    return dataLength;
}

/**
 * @brief DummyDevice::lockData locks shared data from outside of Dummy Device class
 */
void DummyDevice::lockData()
{
    dataSemaphore.acquire();
}

/**
 * @brief DummyDevice::unlockData unlocks data from outside of Dummy Device class
 */
void DummyDevice::unlockData()
{
    dataSemaphore.release();
}

/**
 * @brief DummyDevice::setTrigger sets dummy trigers
 * @param kind
 * @param type
 * @param values
 * @param channel
 */
void DummyDevice::setTrigger(int kind, int type, QVector<uint8_t> values, int channel)
{
    if(kind == ANALOG)
    {
        if(channel == 0)
        {
            analogTriggerType1 = type;
            analogTriggerValue1 = values.at(0);
        }
        else
        {
            analogTriggerType2 = type;
            analogTriggerValue2 = values.at(0);
        }
    }
    else if(kind == DIGITAL)
    {
        digitalTriggerType = type;
        digitalTriggerValues = values;
    }
    qDebug() << "Setting trigger kind: " << kind << "type: " << type << "values: " << values;
}

/**
 * @brief DummyDevice::setAttenuator sets dummy attenuators
 * @param channel
 * @param attenuator
 */
void DummyDevice::setAttenuator(int channel, int attenuator)
{
    if(channel == 0)
    {
        attenuator1 = attenuator;
    }
    else
    {
        attenuator2 = attenuator;
    }
    qDebug() << "Attenuator set: " << attenuator << "channel: " << channel;
}

/**
 * @brief DummyDevice::setOffsetAndGain sets dummy reference voltages
 * @param top
 * @param bottom
 */
void DummyDevice::setOffsetAndGain(QVector<uint8_t> top, QVector<uint8_t> bottom)
{
    topVref = top;
    botVref = bottom;
    qDebug() << "Setting offset and gain top vrefs: " << top << "bot vrefs: " << bottom;
}

/**
 * @brief DummyDevice::setCoupling sets dummy coupling
 * @param channel
 * @param coupling
 */
void DummyDevice::setCoupling(int channel, int coupling)
{
    if(channel == 0)
    {
        channel1co = coupling;
    }
    else
    {
        channel2co = coupling;
    }
    qDebug() << "Setting coupling: " << coupling << "channel: " << channel;
}

/**
 * @brief DummyDevice::setDecimation sets dummy decimation
 * @param ratioBase
 * @param style
 */
void DummyDevice::setDecimation(int ratioBase, int style)
{
    this->ratioBase = ratioBase;
    this->style = style;
    qDebug() << "Setting decimation ratio base: " << ratioBase << " decimation style: " << style;
}

/**
 * @brief DummyDevice::setLogicSupply sets dummy logic supply voltage
 * @param value
 */
void DummyDevice::setLogicSupply(uint8_t value)
{
    this->logicSupply = value;
}

/**
 * @brief DummyDeviceCapturer::doDummyCapture thread where actual capture happends
 * data is always locked before any assigments to prevent race condition.
 */
void DummyDeviceCapturer::doDummyCapture()
{
    usleep(100);
    qDebug() << "Capture started on combinaton: ";
    qDebug() << armedChannels;
    for(int i = 0; i < TEST_LENGTH; ++i)
    {
        int ratio = 1; //<< ratioBase;
        struct timespec t;
        t.tv_nsec= 16;
        t.tv_sec = 0;
        nanosleep(&t, NULL);
        //qDebug() << "Slept for 16ns";
        if(armedChannels == ANALOG1)
        {
            //qDebug() << "Analog armed, locking data";
            dataSemaphore->acquire();
            rawAnalogDataAdc1[i] = 127*(1+sin(i/(3.14*ratio)));
            dataSemaphore->release();
        }
        else if(armedChannels == ANALOG2)
        {
            dataSemaphore->acquire();
            rawAnalogDataAdc2[i] = 127*(1+cos(i/(3.14*ratio)));
            dataSemaphore->release();
        }
        else if(armedChannels == ANALOG1_ANALOG2)
        {
            dataSemaphore->acquire();
            rawAnalogDataAdc1[i] = 127*(1+sin(i/(3.14*ratio)));
            rawAnalogDataAdc2[i] = 127*(1+cos(i/(3.14*ratio)));
            dataSemaphore->release();
        }
        else if(armedChannels == ANALOG1_DIGITAL)
        {
            dataSemaphore->acquire();
            rawAnalogDataAdc1[i] = 127*(1+sin(i/(3.14*ratio)));
            rawDigitalData[i] = i % 65536;
            dataSemaphore->release();
        }
        else if(armedChannels == ANALOG2_DIGITAL)
        {
            dataSemaphore->acquire();
            rawAnalogDataAdc2[i] = 127*(1+cos(i/(3.14*ratio)));
            rawDigitalData[i] = i % 65536;
            dataSemaphore->release();
        }
        else if(armedChannels == ANALOG1_ANALOG2_DIGITAL)
        {
            dataSemaphore->acquire();
            rawDigitalData[i] = i % 65536;
            rawAnalogDataAdc1[i] = 127*(1+sin(i/(3.14*ratio)));
            rawAnalogDataAdc2[i] = 127*(1+cos(i/(3.14*ratio)));
            dataSemaphore->release();
        }
        else if(armedChannels == ONLY_DIGITAL)
        {
            dataSemaphore->acquire();
            rawDigitalData[i] = i % 65536;
            dataSemaphore->release();
        }
        /*if((i % 500) == 0)
        {
            //qDebug() << "500 captured";
            //qDebug("Thread id %d", reinterpret_cast<ulong>(QThread::currentThreadId()));
            emit newDataLength(i);
            usleep(1);
        }*/
    }
    qDebug() << "finished thread";
    emit newDataLength(TEST_LENGTH);
    emit end();
}

/**
 * @brief DummyDevice::armDevice dummy arming of device
 * creates capture object and changes it's thread affinity using QThread.
 * @param armedChannels
 * @param capc
 */
void DummyDevice::armDevice(int armedChannels, CaptureController* capc)
{
    qDebug() << "Arming device: " << armedChannels;
    DummyDeviceCapturer * worker = new DummyDeviceCapturer(armedChannels, analogTriggerType1,  analogTriggerType2, analogTriggerValue1,
                                                           analogTriggerValue2, digitalTriggerType, digitalTriggerValues,
                                                           topVref, rawAnalogDataAdc1, rawAnalogDataAdc2, botVref, attenuator1, attenuator2, channel1co, channel2co, ratioBase, style, rawDigitalData, &dataSemaphore);
    QThread *workerThread = new QThread(this);
    connect(workerThread, SIGNAL(started()), worker, SLOT(doDummyCapture()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(end()), workerThread, SLOT(quit()));
    //connect(worker, SIGNAL(end()), capc, SLOT(captureFinished()), Qt::QueuedConnection);
    //connect(worker)
    int res = connect(worker, SIGNAL(newDataLength(uint)), this, SLOT(updateDataLength(uint)));
    qDebug() <<"Connection: " << res;
    worker->moveToThread(workerThread);
    // Starts an event loop, and emits workerThread->started()
    workerThread->start();
}

/**
 * @brief DummyDevice::getTrigger get current state of triggers
 * @param kind
 * @param channel
 * @return
 */
QPair<int, QVector<uint8_t> > DummyDevice::getTrigger(int kind, int channel)
{
    if(kind == ANALOG)
    {
        QVector<uint8_t> val;
        if(channel == 0)
        {
            val.push_back(analogTriggerValue1);
            return QPair<int, QVector<uint8_t> >(analogTriggerType1, val);
        }
        else
        {
            val.push_back(analogTriggerValue2);
            return QPair<int, QVector<uint8_t> >(analogTriggerType2, val);
        }
    }
    else
    {
        return QPair<int, QVector<uint8_t> >(digitalTriggerType, digitalTriggerValues);
    }
}

/**
 * @brief DummyDevice::getOffsetAndGain get current state of voltage references
 * @return
 */
QPair<QVector<uint8_t>, QVector<uint8_t> > DummyDevice::getOffsetAndGain()
{
    return QPair<QVector<uint8_t>, QVector<uint8_t> >(topVref, botVref);
}

/**
 * @brief DummyDevice::getAttenuator get current state of attenuators
 * @param channel
 * @return
 */
int DummyDevice::getAttenuator(int channel)
{
    if(channel == 0)
    {
        return  attenuator1;
    }
    else
    {
        return attenuator2;
    }
}

/**
 * @brief DummyDevice::getCoupling get current state of couplings.
 * @param channel
 * @return
 */
int DummyDevice::getCoupling(int channel)
{
    if(channel == 0)
    {
        return channel1co;
    }
    else
    {
        return channel2co;
    }
}

/**
 * @brief DummyDevice::getDecimation get current state of decimation
 * @return
 */
QPair<int, int> DummyDevice::getDecimation()
{
    return QPair<int, int>(ratioBase, style);
}

/**
 * @brief DummyDevice::updateDataLength update length of captured data.
 * @param dataLength
 */
void DummyDevice::updateDataLength(unsigned int dataLength)
{
    this->dataLength = dataLength;
    //qDebug() << "Emiting dataUpdated";
    emit dataUpdated();
}
