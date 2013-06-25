//
//   Plot.h
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

#ifndef QWave_Plot_h
#define QWave_Plot_h

#include <QWidget>
#include <QColor>
#include <QPaintEvent>
#include <QListWidgetItem>
#include "../Datamodel/PlotData.h"

const static int DEFAULT_HEIGHT_LOG = 20;
const static int DEFAULT_HEIGHT_LIN = 255;

const static int REGISTER_TRANSITION_LENGTH = 4;
const static int DATASTRING_WIDTH = 60;
const static int DATASTRING_HEIGHT = 20;

/**
 * This class represents plot displaying measured data, self painted.
 */
class Plot : public QObject
{
    Q_OBJECT
    QString plotName;
    int heightOfPlot;
    PlotData * data;
    //int calculateDivSize(uint64_t scale, int widthOfDiv);
    QString getDatastring(int sample);
    bool analogInterpolated;
    bool active;
    QObject* plotCanvas;
    void drawText(QPainter* painter, int x, int y, int w, int h, QString text);
    void paintWire(QPainter * painter, int startCoordY, double divWidth, int width, uint64_t fromTime);
    void paintRegister(QPainter * painter, int startCoordY, double divWidth, int width, uint64_t fromTime);
    void paintLinear(QPainter * painter, int startCoordY, double divWidth, int width, uint64_t fromTime);
public:
    Plot(PlotData * data, QString name);  
    ~Plot();
    PlotData* getData() { return data; }
    void setData(PlotData * newData);
    void paint(QPainter * painter, int startCoordY, int width, uint64_t fromTime);
    void toggleInterpolate() { this->analogInterpolated = not this->analogInterpolated; }
    int getHeight() { return heightOfPlot; }
    void setActive() { active = true; }
    void setInactive() { active = false; }
    bool isAnalog() { if(data->getType() == PlotData::Linear) return true; else return false; }
    void connectPlotCanvas(QObject* plotCanvas) { this->plotCanvas = plotCanvas; }
    uint64_t getSmallestDiv();
    QString getName() { return plotName; }
};

#endif
