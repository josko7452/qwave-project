//
//   PlotCanvas.h
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

#ifndef QWave_PlotCanvas_h
#define QWave_PlotCanvas_h

#include <QWidget>
#include "Plot.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QString>
#include <QScrollBar>

const static int PLOT_SPACING_PX = 5;
const static int DIV_SPACING_PX = 160;
const static int MARKER_HEIGHT = 20;
const static int MARKER_WIDTH = 60;
const static int SMALLER_SPACINGS_COUNT = 20;
const static int SMALL_SPACING = DIV_SPACING_PX/SMALLER_SPACINGS_COUNT;

const static double MAX_DIVSIZEPX = 160;
const static double MIN_DIVSIZEPX = 0.0000001;

/**
 *  This class represents widget with canvas displaying plots.
 */
class PlotCanvas : public QWidget
{
    Q_OBJECT
    
    double divSize;
    uint64_t from;
    uint64_t to;
    uint64_t scale;
    uint64_t smallestDiv();
    uint64_t longestLength();
    void drawGrid(QPainter * painter);
    QVector<Plot*> plots;
    int calculateHeight() const;
    int calculateWidth() const;
    int markerPosition;
    int markerPosition2;
    bool showSecondMarker;
    QWidget * board;
    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);
    void keyPressEvent(QKeyEvent * e );
    QString getMarker(uint64_t marker);
    void refreshScaleTo();
public:
    QSize sizeHint() const;
    void setBoard(QWidget * board) { this->board = board; }
    void paintEvent(QPaintEvent * e);
    bool isAnalog(int row) { return plots.at(row)->isAnalog(); }
    void toggleInterpolate(int index);
    PlotCanvas(QWidget * parent = 0);
    ~PlotCanvas();
    //virtual QSize sizeHint () const;
    void addPlot(Plot * plot);
    int getDivCount();
    void clear();
    void setPlotActive(int index);
    QString getScaleText(uint64_t value);
    uint64_t getScale() { return scale; }
    double getDivSize() { return divSize; }
    Plot* getPlot(int index) { return plots.at(index); }
    QVector<Plot*> getPlots() { return plots; }
    uint64_t getMaxTime();
public slots:
    void zoomIn();
    void zoomOut();
    void setFrom(uint64_t from);
signals:
    void scaleUpdated(uint64_t);
    void fromToUpdated(uint64_t, uint64_t);
};


#endif
