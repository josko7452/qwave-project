//
//   PlotCanvas.cpp
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

#include "PlotCanvas.h"
#include <QPainter>
#include "Board.h"
#include <QDebug>
#include <QSize>

PlotCanvas::PlotCanvas(QWidget * parent) : QWidget(parent), board(NULL), from(0), to(0), scale(0), divSize(10), markerPosition(1), markerPosition2(1), showSecondMarker(false)
{
    setStyleSheet("background-color: black");
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::StrongFocus);
    refreshScaleTo();
    repaint();
}

/**
 * @brief PlotCanvas::toggleInterpolate Toggles interpolation
 * of given plot.
 * @param index index of toggled plot.
 */
void PlotCanvas::toggleInterpolate(int index)
{
    plots.at(index)->toggleInterpolate();
    repaint();
}

/**
 * @brief PlotCanvas::setPlotActive activates given plot.
 * @param index index of activated plot.
 */
void PlotCanvas::setPlotActive(int index)
{
    for(int i = 0; i < plots.count(); i++ )
    {
        plots[i]->setInactive();
    }
    plots[index]->setActive();
    repaint();
}

/**
 * Deletion of plot canvas.
 * @brief PlotCanvas::~PlotCanvas
 */
PlotCanvas::~PlotCanvas()
{
    clear();
}

/**
 * @brief PlotCanvas::getDivCount calculates divs to fit in canvas width.
 * @return divs that would fit into canvas width.
 */
int PlotCanvas::getDivCount()
{
    qDebug() << "W: " << width() << "DIV_SPACING_PX: " << DIV_SPACING_PX;
    return width()/DIV_SPACING_PX;
}

/**
 * @brief PlotCanvas::smallestDiv
 * @return returns smallest div of plots in canvas.
 */
uint64_t PlotCanvas::smallestDiv()
{
    if(plots.size() == 0)
        return DEFAULT_DIV;
    uint64_t smallestDiv = plots.at(0)->getSmallestDiv();
    for(int i = 1; i < plots.size(); ++i)
    {
        if(plots.at(i)->getSmallestDiv() < smallestDiv)
        {
            smallestDiv = plots.at(i)->getSmallestDiv();
        }
    }
    return smallestDiv;
}

/**
 * Returns longest plot length.
 * @brief PlotCanvas::longestLength
 * @return
 */
uint64_t PlotCanvas::longestLength()
{
    return 1000000ULL;
}

/**
 * Generates marker text from absolute time.
 */
QString PlotCanvas::getMarker(uint64_t marker)
{
    if(marker == 0)
    {
        return QString::number(marker);
    }
    if(marker > E12)
    {
        return (QString::number((double)marker/E12, 'f', 2)+S);
    }
    else
        if(marker> E9)
        {
            return (QString::number((double)marker/E9, 'f', 2)+MS);
        }
        else
            if(marker > E6)
            {
                return (QString::number((double)marker/E6, 'f', 2)+US);
            }
            else if(marker > E3)
            {
                return (QString::number((double)marker/E3, 'f', 2)+NS);
            }
            else
            {
                return (QString::number(marker)+PS);
            }
}

/**
 * Draws plotting grid with proper time markers and division tics.
 */
void PlotCanvas::drawGrid(QPainter * painter)
{
    //horizontal line dividing small ticks and markers from ploting space
    painter->setPen(QColor(51, 41, 131));
    painter->drawLine(0, MARKER_HEIGHT, width(),  MARKER_HEIGHT);
    //spacing of small marker ticks
    uint64_t marker = from;
    for(int i = 0; i < width(); i += DIV_SPACING_PX)
    {
        for(int ii = 0; ii < DIV_SPACING_PX; ii += SMALL_SPACING)
        { //painting small markers ticks
            painter->drawLine(i+ii, MARKER_HEIGHT, i+ii, MARKER_HEIGHT*2);
            //paint marker cursor
        }
        // painting vertical lines
        painter->drawLine(i, 0, i, height());
        painter->setPen(QColor(255, 255, 255));
        // painting marker text
        painter->drawText(i, 2, MARKER_WIDTH, MARKER_HEIGHT, 0, getMarker(marker));
        painter->setPen(QColor(51, 41, 131));
        marker += scale;
    }
    painter->setPen(QColor(150, 80, 20));
    painter->drawLine(markerPosition, 0, markerPosition, height());
    if(showSecondMarker)
    {
        painter->setPen(QColor(150, 80, 20));
        painter->drawLine(markerPosition2, 0, markerPosition2, height());
    }
}

/**
 * Triggered when user clics into canvas. Shows second marker.
 * @brief PlotCanvas::mousePressEvent
 * @param e
 */
void PlotCanvas::mousePressEvent(QMouseEvent * e)
{
    qDebug() << "Mys klikla x: " << e->x() << "y: " << e->y();
    if(e->button() == Qt::LeftButton)
    {
        showSecondMarker = true;
        repaint();
    }
}

/**
 * @brief PlotCanvas::getScaleText
 * @param value
 * @return returns text indicating equavalent frequency to given distance in ps.
 */
QString PlotCanvas::getScaleText(uint64_t value)
{
    //qDebug() << "Text from value (getScaleText) " << value;
    double frequency = E15/(double)value;
    QString freq = "";
    if(frequency < E3)
    {
        freq = ("| Equivalent Frequency: "+QString::number(static_cast<double>(frequency), 'f', 2)+E3_HZ);
    }
    else
    if (frequency < E6)
    {
        freq = ("| Equivalent Frequency: "+QString::number(static_cast<double>(frequency)/E3, 'f', 2)+HERTZ);
    }
    else
    if(frequency < E9)
    {
        freq = ("| Equivalent Frequency: "+QString::number(static_cast<double>(frequency)/E6, 'f', 2)+KHZ);
    }
    else
    {
        freq = ("| Equivalent Frequency: "+QString::number(static_cast<double>(frequency)/E9, 'f', 2)+MHZ);
    }
    if(value > E12)
    {
        return (QString::number((double)value/E9, 'f', 2)+" "+S+" "+freq);
    }
    if(value > E9)
    {
        return (QString::number((double)value/E9, 'f', 2)+" "+MS+" "+freq);
    }
    else if(value > E6)
    {
        return (QString::number((double)value/E6, 'f', 2)+" "+US+" "+freq);
    }
    else if(value > E3)
    {
        return (QString::number((double)value/E3, 'f', 2)+" "+NS+" "+freq);
    }
    else
    {
        return (QString::number((double)value, 'f', 2)+" "+PS+" "+freq);
    }
}

/**
 * Triggered if mouse is moved in canvas region.
 * Allows second marker to move when LMB is held.
 * @brief PlotCanvas::mouseMoveEvent
 * @param e
 */
void PlotCanvas::mouseMoveEvent(QMouseEvent * e)
{
    if(showSecondMarker)
    {
        markerPosition2 = 1+(e->x()/SMALL_SPACING)*SMALL_SPACING;
        if(plots.count() > 0)
        {
            static_cast<Board*>(board)->setStatusBarText(QString("Distance: ")+getScaleText(((plots[0]->getData()->getDiv()*abs(markerPosition-markerPosition2))/divSize)));
            //qDebug() << "Distance: " << (plots[0]->getData()->getDiv()*abs(markerPosition-markerPosition2))/divSize;
        }
        repaint();
    }
}

/**
 * Triggered when user releases mouse click. Hides secon marker.
 * @brief PlotCanvas::mouseReleaseEvent
 * @param e
 */
void PlotCanvas::mouseReleaseEvent(QMouseEvent * e)
{
    showSecondMarker = false;
    if(e->button() == Qt::LeftButton)
    {
        qDebug() << "Move cursor";
        markerPosition = 1+(e->x()/SMALL_SPACING)*SMALL_SPACING;
    }
    repaint();
}

/**
 * @brief PlotCanvas::keyPressEvent triggered when user pushes key inside canvas.
 *  Used to move marker when user push cursor keys.
 * @param e
 */
void PlotCanvas::keyPressEvent(QKeyEvent * e )
{
    qDebug() << "Key pressed";
    if(e->key() == Qt::Key_Left)
    {
        if(0 >= (markerPosition-DIV_SPACING_PX))
        {
            if(from-smallestDiv()*width()/divSize > 0)
            {
                markerPosition = 1+(width()/DIV_SPACING_PX)*DIV_SPACING_PX;
                setFrom(from-smallestDiv()*width()/divSize);
            }
            else
            {
                setFrom(0);
            }
        }
        else
        {
            markerPosition = markerPosition-DIV_SPACING_PX;
        }
    }
    else if(e->key() == Qt::Key_Right)
    {
        if((markerPosition+DIV_SPACING_PX) >= width())
        {
            markerPosition = 1;
            setFrom(from+smallestDiv()*width()/divSize);
        }
        else
        {
            markerPosition = markerPosition+DIV_SPACING_PX;
        }
    }
    repaint();
}

/**
 * Called by Qt when there is need to repaint canvas.  Used to paint all objects on canvas.
 * @brief PlotCanvas::paintEvent
 * @param e
 */
void PlotCanvas::paintEvent(QPaintEvent * e)
{
    if(board == NULL)
        return;
    QStyleOption o;                                                                                                                                                                  
    o.initFrom(this);  
    QPainter * painter = new QPainter();
    painter->begin(this);
    //painter->setRenderHint(QPainter::Antialiasing);
    style()->drawPrimitive(QStyle::PE_Widget, &o, painter, this);  //drawing stylesheet background 
    if(plots.count() == 0)
    {
        painter->setPen(QColor(255, 255, 180));
        painter->drawText(50, 50, 140, 20, 0, "No data to plot");
        painter->end();
        delete(painter);
        return;
    }
    drawGrid(painter); // drawing grid
    int lastCoordY = MARKER_HEIGHT*2+PLOT_SPACING_PX;
    painter->setPen(QColor(0, 255, 0));
    for(int i = 0; i < plots.size(); ++i)
    { // painting plots and markers
        int newWidth = width();
        plots.at(i)->paint(painter, lastCoordY, newWidth, from);
        lastCoordY += plots.at(i)->getHeight() + PLOT_SPACING_PX;
        painter->setPen(QColor(40, 40, 60));
        painter->drawLine(0, lastCoordY-PLOT_SPACING_PX/2, width(), lastCoordY-PLOT_SPACING_PX/2);
        painter->setPen(QColor(0, 255, 0));
    }
    painter->end();
    delete(painter);
}

/**
 * @brief PlotCanvas::addPlot adds given plot to canvas.
 * @param plot
 */
void PlotCanvas::addPlot(Plot * plot)
{
    plots.push_back(plot);
    plot->connectPlotCanvas(this);
    updateGeometry();
    repaint();
}

/**
 * @brief PlotCanvas::calculateWidth calculates width of canvas.
 *  Set to small value to fool QScrollArea to not create horizontal scrollbar.
 * @return
 */
int PlotCanvas::calculateWidth() const
{
    //TODO: Properly calculate size
    return PLOT_SPACING_PX;
}

/**
 * @brief PlotCanvas::calculateHeight calculates height of canvas.
 * @return
 */
int PlotCanvas::calculateHeight() const
{
    int height = 2*MARKER_HEIGHT;
    for(int i = 0; i < plots.size(); ++i)
    {
        height += plots.at(i)->getHeight() + PLOT_SPACING_PX*2;
    }
    return height;
}

/**
 * @brief PlotCanvas::sizeHint
 * @return returns size hint by caluclated height and width.
 */
QSize PlotCanvas::sizeHint() const
{
    return QSize(calculateWidth(), calculateHeight());
}

/**
 * Clears all plots from canvas.
 */
void PlotCanvas::clear()
{
    for (int i = 0; i < plots.size(); ++i) 
    {
        delete(plots.at(i));
    }
    plots.clear();
}

/**
 * @brief PlotCanvas::setFrom
 * Sets origin in from which plots are plotted.
 * @param from
 */
void PlotCanvas::setFrom(uint64_t from)
{
    this->from = from;
    this->scale = (to-from)/getDivCount();
    refreshScaleTo();
    repaint();
}

/**
 * @brief PlotCanvas::zoomIn zooms in by changing divSize.
 */
void PlotCanvas::zoomIn()
{
    if(plots.count() < 1)
        return;
    if(divSize >= MAX_DIVSIZEPX)
        return;
    divSize = divSize*2.0;
    refreshScaleTo();
    qDebug() << "zoomIn();";
}

/**
 * @brief PlotCanvas::zoomOut zooms out by changing divSize.
 */
void PlotCanvas::zoomOut()
{
    if(plots.count() < 1)
        return;
    if(divSize <= MIN_DIVSIZEPX)
        return;
    divSize = divSize/2.0;
    refreshScaleTo();
    qDebug() << "zoomOut();";
}

/**
 * @brief PlotCanvas::refreshScaleTo refreshes scale and from/to values.
 */
void PlotCanvas::refreshScaleTo()
{
    if(plots.count() > 0)
    {
        qDebug() << "We have DIV_SPACING: " << DIV_SPACING_PX << " divSize: " << divSize << " getDiv(): " << smallestDiv();
        scale = (DIV_SPACING_PX/divSize)*smallestDiv();
        to = from + scale*getDivCount();
        qDebug() << "New scale: " << scale << "New to: " << to;
    }
    emit scaleUpdated(scale);
    emit fromToUpdated(from, to);
    if(board != NULL)
        static_cast<Board*>(board)->updateFromTo();
    repaint();
}

/**
 * @brief PlotCanvas::getMaxTime
 * Returns time of last timestamp of all plots in canvas.
 * @return
 */
uint64_t PlotCanvas::getMaxTime()
{
    uint64_t maxtime = 0;
    for(int i = 0; i < plots.count(); ++i)
    {
        for(int ii = 0; ii < plots[i]->getData()->getBitwidth(); ++ii)
        {
            uint64_t time = plots[i]->getData()->lastPositionOnBit(ii)*plots[i]->getData()->getDiv();
            if(time > maxtime)
            {
                maxtime = time;
            }
        }
    }
    return maxtime;
}
