//
//   CaptureController.cpp
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


#include "CaptureController.h"
#include "DummyDevice.h"
#include "Ft245Device.h"
#include "../Datamodel/Constants.h"
#include <QSet>
#include <QSetIterator>
#include "Capabilities.h"

#define BLOCKS_BEFORE_DISSALOWED_OVERLAP 2048

CaptureController::CaptureController(QObject *parent) :
    QObject(parent), currentDataIndex(0), canvas(NULL), run(1), lastend(0)
{

}

/**
 * @brief CaptureController::updateData this method is called when all captured data from device was transmitted.
 * This method converts raw data into data model and updates canvas view to show new data.
 */
void CaptureController::updateData()
{
    uint i;
    qDebug() <<"updating data in controller";
    QVector<uint8_t*> dataList = static_cast<AbstractDevice*>(sender())->getRawDataList();
    PlotData* plotData = NULL;
    if(static_cast<AbstractDevice*>(sender())->getRawDataLength() != 0)
    {
        for(int dataId = 0; dataId < dataList.count(); ++dataId)
        {
            QPair<PlotData*, int > pair = deviceDataMap[dataId][static_cast<AbstractDevice*>(sender())][0];
            if(pair.first == NULL)
            {
                continue;
            }
            plotData = pair.first;
            uint8_t* data = dataList.at(dataId);
            for(i = currentDataIndex; i < static_cast<AbstractDevice*>(sender())->getRawDataLength(); ++i)
            {
                if(plotData->getType() == PlotData::Logic)
                {
                    for(int bitNumber = 0; bitNumber < 16; ++bitNumber)
                    {
                            QPair<PlotData*, int > pair = deviceDataMap[dataId][static_cast<AbstractDevice*>(sender())][bitNumber];
                            PlotData* plotData = pair.first;
                            if(plotData == NULL)
                                continue;
                            int bw = plotData->getBitwidth();
                            for(int bitNum = 0; bitNum < bw; ++bitNum)
                            {
                                static_cast<AbstractDevice*>(sender())->lockData();
                                uint16_t dataint = reinterpret_cast<uint16_t*>(data)[i];
                                static_cast<AbstractDevice*>(sender())->unlockData();
                                if(((dataint >> (bitNum+bitNumber)) & 1))
                                {
                                    plotData->appendDataAtBit(bw-1-bitNum, HIGH);
                                }
                                else
                                {
                                    plotData->appendDataAtBit(bw-1-bitNum, LOW);
                                }
                                //qDebug() << static_cast<AbstractDevice*>(sender())->getRawDataLength();
                                //qDebug("Thread id %d", reinterpret_cast<ulong>(QThread::currentThreadId()));
                                //qDebug() << "Last append digital: " << plotData->lastPositionOnBit(bw-1-bitNum);
                            }
                     }
                }
                else
                {
                    static_cast<AbstractDevice*>(sender())->lockData();
                    plotData->appendDataAtBit(0, data[i]);
                    //qDebug("Thread id %d", reinterpret_cast<ulong>(QThread::currentThreadId()));
                    //qDebug() << "Last append analog: " << plotData->lastPositionOnBit(0);
                    static_cast<AbstractDevice*>(sender())->unlockData();
                }
            }
        }
        currentDataIndex = i;
        if(canvas != NULL)
            canvas->repaint();
        if(plotData != NULL)
        {
            emit newFrom((lastend+BLOCKS_BEFORE_DISSALOWED_OVERLAP*2)*plotData->getDiv());
            lastend = plotData->lastPositionOnBit(0);
        }
        ++run;
        qDebug() << "run number: " << run;
    }
    if(continousSet and run < INT_MAX)
    {
        startCapture(continousSet, oldPlots);
    }
}

/**
 * @brief CaptureController::assignDevicePlotData
 * This method is called when user assigns plot to device and creates
 *  map of which plotData and which bit number is assigned to which device and it's rawDataId and
 *  how many wires are in that plot.
 * @param plotData
 * @param device
 * @param deviceRawDataId
 * @param bitNumber
 * @param wires
 */
void CaptureController::assignDevicePlotData(PlotData* plotData,
                                             AbstractDevice * device,
                                             int deviceRawDataId,
                                             int bitNumber,
                                             int wires)
{
    QMap<AbstractDevice*,  QMap<int, QPair <PlotData*, int> > > map = deviceDataMap[deviceRawDataId];
    QMap<int, QPair <PlotData*, int> > pair = map[device];
    QPair <PlotData*, int> wiresPair(plotData, wires);
    pair.insert(bitNumber, wiresPair);
    map.insert(device, pair);
    deviceDataMap.insert(deviceRawDataId, map);
    if(plotData->getType() == PlotData::Logic)
    {
        currentWireIndex[device] = currentWireIndex[device]+wires;
    }
    else
    {
        if(deviceRawDataId == 0)
        {
            analog1free[device] = false;
        }
        else
        {
            analog2free[device] = false;
        }
    }
    plotDataIdMap.insert(plotData, deviceRawDataId);
    plotDeviceMap.insert(plotData, device);
}

/**
 * @brief CaptureController::addDummyDevice creates new dummy device and adds it to device controller.
 */
void CaptureController::addDummyDevice()
{
    DummyDevice* dev = new DummyDevice();
    connect(dev, SIGNAL(dataUpdated()), this, SLOT(updateData()), Qt::QueuedConnection);
    controller.addDevice(dev);
    currentWireIndex[dev] = 0;
    analog1free[dev] = true;
    analog2free[dev] = true;
}

/**
 * @brief CaptureController::addFt245Device creates new ft245device and adds it to device controller.
 */
void CaptureController::addFt245Device()
{
    Ft245Device* dev = new Ft245Device();
    connect(dev, SIGNAL(dataUpdated()), this, SLOT(updateData()), Qt::QueuedConnection);
    controller.addDevice(dev);
    currentWireIndex[dev] = 0;
    analog1free[dev] = true;
    analog2free[dev] = true;
}

/**
 * @brief CaptureController::getDeviceByPlot
 * @param plot
 * @return  returns device assigned to given plot
 */
AbstractDevice* CaptureController::getDeviceByPlot(Plot* plot)
{
    return plotDeviceMap[plot->getData()];
}

/**
 * @brief CaptureController::getDataIdByPlot
 * @param plot
 * @return return data id assigned to given plot
 */
int CaptureController::getDataIdByPlot(Plot* plot)
{
    return plotDataIdMap[plot->getData()];
}

/**
 * @brief CaptureController::startCapture
 * Triggered when user pushes arm button, caries out process of correctly initializing capture, by
 * examining opened plots to set combination of triggered inputs.
 * @param continous
 * @param plots
 */
void CaptureController::startCapture(bool continous, QVector<Plot*> plots)
{
    currentDataIndex = 0;
    captureCompleted = false;
    capturePending = true;
    continousSet = continous;
    oldPlots = plots;
    QSet<AbstractDevice*> devices;
    QMap<AbstractDevice*, bool> hasAnalog1;
    QMap<AbstractDevice*, bool> hasAnalog2;
    QMap<AbstractDevice*, bool> hasDigital;
    for(int i = 0; i < controller.getlistOfDevices()->count(); ++i)
    {
        AbstractDevice* dev = controller.getDevice(i);
        hasAnalog1[dev] = false;
        hasAnalog2[dev] = false;
        hasDigital[dev] = false;
    }
    for(int i = 0; i < plots.count(); ++i)
    {
        AbstractDevice* dev = plotDeviceMap[plots[i]->getData()];
        devices.insert(dev);
        if(plotDataIdMap[plots[i]->getData()] == 0)
            hasAnalog1[dev] = true;
        if(plotDataIdMap[plots[i]->getData()] == 1)
            hasAnalog2[dev] = true;
        if(plotDataIdMap[plots[i]->getData()] == 2)
            hasDigital[dev] = true;
    }
    QSetIterator<AbstractDevice*> i(devices);
    while(i.hasNext())
    {
        AbstractDevice* dev = i.next();
        if(hasAnalog1[dev])
        {
            if(hasAnalog2[dev])
            {
                if(hasDigital[dev])
                {
                    dev->armDevice(ANALOG1_ANALOG2_DIGITAL, this);
                }
                else
                {
                    dev->armDevice(ANALOG1_ANALOG2, this);
                }
            }
            else
            {
                if(hasDigital[dev])
                {
                    dev->armDevice(ANALOG1_DIGITAL, this);
                }
                else
                {
                    dev->armDevice(ANALOG1, this);
                }
            }
        }
        else
        {
            if(hasAnalog2[dev])
            {
                if(hasDigital[dev])
                {
                    dev->armDevice(ANALOG2_DIGITAL, this);
                }
                else
                {
                    dev->armDevice(ANALOG2, this);
                }
            }
            else
            {
                if(hasDigital[dev])
                {
                    dev->armDevice(ONLY_DIGITAL, this);
                }
            }
        }
     }
}
