//
//   CaptureController.h
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


#ifndef CAPTURECONTROLLER_H
#define CAPTURECONTROLLER_H

#include <QObject>
#include <QMap>
#include <QPair>
#include "DeviceController.h"
#include "../Datamodel/PlotData.h"
#include "../GUI/Plot.h"
#include "../GUI/PlotCanvas.h"

/**
 * @brief The CaptureController class controls the data capture process
 *  and consequent coversion of data to appropriate data models. Class also deals with correct
 * settings of vizualization of captured data.
 */
class CaptureController : public QObject
{
    Q_OBJECT
    DeviceController controller;
    QMap<int, QMap<AbstractDevice*, QMap<int, QPair <PlotData*, int> > > > deviceDataMap;
    QMap<AbstractDevice*, int> currentWireIndex;
    QMap<AbstractDevice*, bool> analog1free;
    QMap<AbstractDevice*, bool> analog2free;
    QMap<PlotData*, int> plotDataIdMap;
    QMap<PlotData*, AbstractDevice*> plotDeviceMap;
    unsigned int currentDataIndex;
    PlotCanvas* canvas;
    bool capturePending;
    bool continousSet;
    bool captureCompleted;
    QVector<Plot*> oldPlots;
    int run;
    uint64_t lastend;
public:
    bool hasDevice() { return controller.getlistOfDevices()->count() > 0;  }
    CaptureController(QObject *parent = 0);
    AbstractDevice* getDeviceByPlot(Plot* plot);
    int getDataIdByPlot(Plot* plot);
    void startCapture(bool continous, QVector<Plot*> plots);
    void setPlotCanvas(PlotCanvas* canvas) { this->canvas = canvas; }
    void stopCapture() { continousSet = false; }
public slots:
    void updateData();
    void addDummyDevice();
    void addFt245Device();
    void assignDevicePlotData(PlotData* plotData, AbstractDevice * device, int deviceRawDataId, int bitNumber, int wires);
    DeviceController* getDeviceController() { return &controller; }
    int getCurrentWireIndex(AbstractDevice* device) { return currentWireIndex[device]; }
    bool isAnalog1Free(AbstractDevice* dev) { return analog1free[dev]; }
    bool isAnalog2Free(AbstractDevice* dev) { return analog2free[dev]; }
signals:
    void newFrom(uint64_t);
};

#endif // CAPTURECONTROLLER_H
