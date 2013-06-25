//
//   Ft245Device.cpp
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


#include "Ft245Device.h"
#include "Capabilities.h"
#include <QDebug>
#include <QThread>
#include <QSemaphore>
#include "../Exceptions/DeviceException.h"

#define READ_LENGTH 16384
#define WRITE_LENGTH 10
#define UPDATE_BOUNDARY 512
#define DIE_BOUNDARY 5000
#define GARBAGE 4
#define REFRESH_RATE_DELAY 25000

/*
Protocol:
 Commands:
 Bit lengths                   | 8bit                           | 72bit     |
 Field description          | Message number       | Data      |

 RESET                          | 0x00                          | DON'T CARE
 SET_VREF                     | 0x10                          | 8bit - TopA, 8bit - TopB, 8bit - BotA, 8bit - BotB
 SET_VLOG                    | 0x11                          | 8bit - 7bit - value
 SET_ATTENUATORS      | 0x20                         | 8bit - 1bit - att. relay ADC1, 8bit - 1bit att. relay ADC2
 SET_COUPLINGS           | 0x21                         | 1bit - ac/dc relay ADC1, 1bit ac/dc relay ADC2
 SET_DECIMATION         | 0x30                         | 8bit - 4bit - decimationRatioBase, 8bit -2bit - decimationStyle
 SET_ANALOGTRIGGER   | 0x31                         | Trigger adc1: 8bit - value, 8bit -  1bit - triggerType, 1bit - slope |
                                                                          Trigger adc2: 8bit - value, 8bit - 1bit - triggerType, 1bit - slope
 SET_DIGITALTRIGGER    | 0x32                         | 64bit - pattern, 2bit - samples
 START_CAPTURE           | 0x40                         | 2bit - channel selection
                                                                             0x00 - analog1 only, 0x01 analog2 only, 0x02 analog1 + digi,
                                                                             0x03 - analog2 + digi, 0x04 analog1+analog2, 0x05 an1+an2+dig
                                                                             0x06 - dig only
 Respones:
 DATA_DUMP                  | 0x50                         | burst of data (longer than 72bit) where first two bytes are respective ADC data and
                                                                           second two bytes are data of logic analyzer
 DATA_OK                      | 0x80                         | DON't CARE
 */

//opcodes
static const uint8_t RESET                           = 0x00;
static const uint8_t SET_VREF                      = 0x10;
static const uint8_t SET_VLOG                     = 0x11;
static const uint8_t SET_ATTENUATORS       = 0x20;
static const uint8_t SET_COUPLINGS            = 0x21;
static const uint8_t SET_DECIMATION          = 0x30;
static const uint8_t SET_ANALOGTRIGGER   = 0x31;
static const uint8_t SET_DIGITALTRIGGER    = 0x32;
static const uint8_t START_CAPTURE           = 0x40;
// reply
static const uint8_t DATA_DUMP = 0x50;
static const uint8_t DATA_OK = 0x80;

#define TRIGGER_RISING 0x40
#define TRIGGER_FALLING 0x00
#define TRIGGER_RISING_EDGE 0xC0
#define TRIGGER_FALLING_EDGE 0x80

Ft245Device::Ft245Device()
    :   dataSemaphore(1),
        analogTriggerType1(RISING),
        analogTriggerType2(RISING),
        analogTriggerValue1(150),
        analogTriggerValue2(150),
        digitalTriggerType(SIGNLE_SAMPLE),
        channel1co(AC_COUPLING),
        channel2co(AC_COUPLING),
        attenuator1(ATTENUATOR_HIGH),
        attenuator2(ATTENUATOR_HIGH),
        ratioBase(0),
        style(0),
        logicSupply(0)
{
    this->ft245cnt = new Ft245sync(READ_LENGTH, WRITE_LENGTH, GPIO_STATE);
    rawDataList.push_back(rawAnalogDataAdc1);
    rawDataList.push_back(rawAnalogDataAdc2);
    rawDataList.push_back((uint8_t*)rawDigitalData);
    digitalTriggerValues.push_back(0xAA);
    digitalTriggerValues.push_back(0xAB);
    digitalTriggerValues.push_back(0xAC);
    digitalTriggerValues.push_back(0xAD);
    topVref.push_back(255);
    topVref.push_back(255);
    botVref.push_back(0);
    botVref.push_back(0);
    setAttenuator(0, attenuator1);
    setAttenuator(1, attenuator2);
    setCoupling(0, channel1co);
    setCoupling(1, channel2co);
    setDecimation(ratioBase, style);
    QVector<uint8_t> val;
    val.push_back(analogTriggerValue1);
    setTrigger(ANALOG, analogTriggerType1, val, 0);
    setTrigger(DIGITAL, digitalTriggerType, digitalTriggerValues, 0);
    setOffsetAndGain(topVref, botVref);
    setLogicSupply(logicSupply);
}

/**
 * @brief Ft245Device::getRawDataList
 * @return returns list of raw data, it is shared data so it must be locked !!!
 *  by lockData() to prevent race condition and unlocked by unlockData() after
 * use to prevent deadlock.
 */
QVector<uint8_t*> Ft245Device::getRawDataList()
{
    return rawDataList;
}

/**
 * @brief Ft245Device::getRawDataLength
 * @return returns lenght of captured data.
 */
unsigned int Ft245Device::getRawDataLength()
{
    return dataLength;
}

/**
 * @brief Ft245Device::lockData locks shared data from outside of Dummy Device class
 */
void Ft245Device::lockData()
{
    dataSemaphore.acquire();
}

/**
 * @brief Ft245Device::unlockData unlocks data from outside of Dummy Device class
 */
void Ft245Device::unlockData()
{
    dataSemaphore.release();
}

/**
 * @brief writeWithAck Writes commad to Ft245 bus and waits for device acknowledge.
 *  If device returns anything else than acknowledge try to resend command.
 *  If number of tryes reaches DIE_BOUNDARY then throw exception that connection was lost.
 * @param buf
 * @param ft245cnt
 */
void writeWithAck(unsigned char* buf, Ft245sync* ft245cnt)
{
    unsigned char recvbuf[READ_LENGTH];
    recvbuf[0] = 0;
    int lockupCnt = 0;
    while(recvbuf[0] != DATA_OK)
    {
        if(ft245cnt->write(buf) <  0)
        {
            qDebug() << "Error writing data: " << buf;
        }
        if(ft245cnt->read(recvbuf) < 0)
        {
            qDebug() << "Error receiving data";
        }
        if(lockupCnt++ > DIE_BOUNDARY)
        {
            throw DeviceException("Couldn't send command", FTDI_ERROR);
        }
    }
}

/**
 * @brief Ft245::setTrigger sets trigger to real device by sending appropriate command
 * @param kind
 * @param type
 * @param values
 * @param channel
 */
void Ft245Device::setTrigger(int kind, int type, QVector<uint8_t> values, int channel)
{
    qDebug() << "Setting trigger kind: " << kind << "type: " << type << "values: " << values;
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
        unsigned char buf[10];
        buf[0] = SET_ANALOGTRIGGER;
        buf[1] = analogTriggerValue2;
        if(analogTriggerType2 == RISING)
            buf[2] = TRIGGER_RISING;
        else if(analogTriggerType2== RISING_SLOPE)
            buf[2] = TRIGGER_RISING_EDGE;
        else if(analogTriggerType2== FALLING)
            buf[2] = TRIGGER_FALLING;
        else
            buf[2] = TRIGGER_FALLING_EDGE;
        buf[3] = analogTriggerValue1;
        if(analogTriggerType1 == RISING)
            buf[4] = TRIGGER_RISING;
        else if(analogTriggerType1 == RISING_SLOPE)
            buf[4] = TRIGGER_RISING_EDGE;
        else if(analogTriggerType1 == FALLING)
            buf[4] = TRIGGER_FALLING;
        else
            buf[4] = TRIGGER_FALLING_EDGE;
        writeWithAck(buf, ft245cnt);
        qDebug() << "Setting trigger: " << buf[0] << ", " << buf[1] << ", " << buf[2] << ", " << buf[3] << ", "<< buf[4];
    }
    else if(kind == DIGITAL)
    {
        digitalTriggerType = type;
        digitalTriggerValues = values;
        unsigned char buf[10] = { SET_DIGITALTRIGGER, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        for(int i = 0; i < values.count(); ++ i)
        {
            buf[i+1] = values[i];
        }
        if(type == SIGNLE_SAMPLE)
            buf[9] = 0x00;
        else if(type == TWO_SAMPLES)
            buf[9] = 0x10;
        else if(type == THREE_SAMPLES)
            buf[9] = 0x20;
        else
            buf[9] = 0x30;
        writeWithAck(buf, ft245cnt);
    }
}

/**
 * @brief Ft245Device::setAttenuator sets attenuator to real device by sending command
 * @param channel
 * @param attenuator
 */
void Ft245Device::setAttenuator(int channel, int attenuator)
{
    if(channel == 0)
    {
        attenuator1 = attenuator;
    }
    else
    {
        attenuator2 = attenuator;
    }
    unsigned char buf[10];
    buf[0] = SET_ATTENUATORS;
    if(attenuator1 == ATTENUATOR_LOW)
        buf[1] = 0xFF;
    else
        buf[1] = 0x00;
    if(attenuator2 == ATTENUATOR_LOW)
        buf[2] = 0xFF;
    else
        buf[2] = 0x00;
    qDebug() << "Attenuator set: " << attenuator << "channel: " << channel;
    writeWithAck(buf, ft245cnt);
}

/**
 * @brief Ft245Device::setOffsetAndGain set voltage references by sending appropriate command
 * @param top
 * @param bottom
 */
void Ft245Device::setOffsetAndGain(QVector<uint8_t> top, QVector<uint8_t> bottom)
{
    topVref = top;
    botVref = bottom;
    unsigned char buf[10];
    buf[0] = SET_VREF;
    buf[1] = top[0];
    buf[2] = bottom[0];
    buf[3] = top[1];
    buf[4] = bottom[1];
    qDebug() << "Setting offset and gain top vrefs: " << top << "bot vrefs: " << bottom;
    writeWithAck(buf, ft245cnt);
}

/**
 * @brief Ft245Device::setCoupling set coupling of device by sending appropriate command
 * @param channel
 * @param coupling
 */
void Ft245Device::setCoupling(int channel, int coupling)
{
    if(channel == 0)
    {
        channel1co = coupling;
    }
    else
    {
        channel2co = coupling;
    }
    unsigned char buf[10];
    buf[0] = SET_COUPLINGS;
    if(channel1co == AC_COUPLING)
    {
        buf[1] = 0x00;
    }
    else
    {
        buf[1] = 0xFF;
    }
    if(channel2co == AC_COUPLING)
    {
        buf[2] = 0x00;
    }
    else
    {
        buf[2] = 0xFF;
    }
    qDebug() << "Setting coupling: " << coupling << "channel: " << channel;
    writeWithAck(buf, ft245cnt);
}

/**
 * @brief Ft245Device::setDecimation set decimation of real device by sending appropriate command
 * @param ratioBase
 * @param style
 */
void Ft245Device::setDecimation(int ratioBase, int style)
{
    this->ratioBase = ratioBase;
    this->style = style;
    unsigned char buf[10];
    buf[0] = SET_DECIMATION;
    buf[1] = ratioBase;
    buf[2] = style;
    qDebug() << "Setting decimation ratio base: " << ratioBase << " decimation style: " << style;
    writeWithAck(buf, ft245cnt);
}

/**
 * @brief Ft245Device::setLogicSupply set logic supply by sending appropriate command
 * @param value
 */
void Ft245Device::setLogicSupply(uint8_t value)
{
    this->logicSupply = value;
    unsigned char buf[10];
    buf[0] = SET_VLOG;
    buf[1] = value;
    writeWithAck(buf, ft245cnt);
}

/**
 * @brief Ft245Device::armDevice arming of device
 * creates capture object and changes it's thread affinity using QThread.
 * @param armedChannels
 * @param capc
 */
void Ft245Device::armDevice(int armedChannels, CaptureController* capc)
{
    qDebug() << "Arming device: " << armedChannels;
    Ft245DeviceCapturer * worker = new Ft245DeviceCapturer(armedChannels, ft245cnt, rawAnalogDataAdc1, rawAnalogDataAdc2, rawDigitalData, &dataSemaphore);
    QThread *workerThread = new QThread(this);
    connect(workerThread, SIGNAL(started()), worker, SLOT(doCapture()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(end()), workerThread, SLOT(quit()));
    //connect(worker)
    int res = connect(worker, SIGNAL(newDataLength(uint)), this, SLOT(updateDataLength(uint)));
    qDebug() <<"Connection: " << res;
    worker->moveToThread(workerThread);
    // Starts an event loop, and emits workerThread->started()
    workerThread->start();
}

/**
 * @brief Ft245DeviceCapturer::doCapture hread where actual capture happends
 * data is always locked before any assigments to prevent race condition.
 */
void Ft245DeviceCapturer::doCapture()
{
    int incoming = READ_LENGTH;
    unsigned char buf[WRITE_LENGTH];
    buf[0] = START_CAPTURE;
    buf[1] = armedChannels;
    qDebug() << "Starting capture";
    ft245cnt->write(buf);
    unsigned char bufIn[READ_LENGTH];
    int wait = 0;
    int tryes = 0;
    bool firstRead = true;
    uint32_t index = 0;
    uint32_t dataIndex = 0;
    int skew = 0;
    while(incoming == READ_LENGTH)
    {
        while((incoming = ft245cnt->read(bufIn)) == 0)
        {
            if(wait++ > 5)
            {
                usleep(500000);
                wait = 0;
                if(tryes++ > 4)
                {
                    emit end();
                    emit newDataLength(0);
                    return;
                }
                ++tryes;
            }
        }
        if(incoming < 0)
        {
            emit end();
            emit newDataLength(0);
            //throw DeviceException("No incoming data from device", FTDI_ERROR);
        }
        if(firstRead)
        {
            while(bufIn[dataIndex++] == DATA_OK) {}
            while(bufIn[dataIndex++] == DATA_DUMP) {}
            dataIndex--;
            dataIndex+=GARBAGE;
            firstRead = false;
        }
        else
        {
            dataIndex = 0;
        }
        while(dataIndex < (uint)incoming)
        {
            for(int i = skew; i < 4; ++i)
            {
                skew = 0;
                if(dataIndex == (uint)incoming)
                {
                    skew = i;
                    break;
                }
                if(i == 0)
                {
                    dataSemaphore->acquire();
                    rawAnalogDataAdc2[index] = bufIn[dataIndex];
                    dataSemaphore->release();
                }
                else if(i == 1)
                {
                    dataSemaphore->acquire();
                    rawAnalogDataAdc1[index] = bufIn[dataIndex];
                    dataSemaphore->release();
                }
                else if(i == 2)
                {
                    dataSemaphore->acquire();
                    rawDigitalData[index] =  ((uint16_t)bufIn[dataIndex] << 8) ^ bufIn[dataIndex+1];
                    dataSemaphore->release();
                }
                dataIndex++;
            }
            if(dataIndex != (uint)incoming)
                index++;
        }
    }
    usleep(REFRESH_RATE_DELAY); //practically defines plot refresh rate !, now set to approx 24FPS for Core 2 Duo 2,4GHz
    emit newDataLength(index);
    emit end();
}

/**
 * @brief Ft245Device::getTrigger gets current triggers
 * @param kind
 * @param channel
 * @return
 */
QPair<int, QVector<uint8_t> >Ft245Device::getTrigger(int kind, int channel)
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
 * @brief Ft245Device::getOffsetAndGain gets current reference voltages
 * @return
 */
QPair<QVector<uint8_t>, QVector<uint8_t> > Ft245Device::getOffsetAndGain()
{
    return QPair<QVector<uint8_t>, QVector<uint8_t> >(topVref, botVref);
}

/**
 * @brief Ft245Device::getAttenuator gets current attenuator states
 * @param channel
 * @return
 */
int Ft245Device::getAttenuator(int channel)
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
 * @brief Ft245Device::getCoupling gets current coupling states
 * @param channel
 * @return
 */
int Ft245Device::getCoupling(int channel)
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
 * @brief Ft245Device::getDecimation gets current decimation states
 * @return
 */
QPair<int, int> Ft245Device::getDecimation()
{
    return QPair<int, int>(ratioBase, style);
}

/**
 * @brief Ft245Device::updateDataLength update length of captured data.
 * @param dataLength
 */
void Ft245Device::updateDataLength(unsigned int dataLength)
{
    this->dataLength = dataLength;
    //qDebug() << "Emiting dataUpdated";
    emit dataUpdated();
}

