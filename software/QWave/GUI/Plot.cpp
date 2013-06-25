//
//   Plot.cpp
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

#include <QPainter>
#include "Plot.h"
#include "../Datamodel/Constants.h"
#include <QDebug>
#include "PlotCanvas.h"
#include <math.h>

Plot::Plot(PlotData * data, QString name) : plotName(name), data(data), analogInterpolated(true), active(false), plotCanvas(NULL)
{
    if(data == NULL or data->getType() == PlotData::Linear)
    {
        heightOfPlot = DEFAULT_HEIGHT_LIN;
    }
    else if(data->getType() == PlotData::Logic)
    {
        heightOfPlot = DEFAULT_HEIGHT_LOG;
    }
}

/**
 * @brief Plot::setData This method sets data to this plot.
 * @param newData
 */
void Plot::setData(PlotData * newData)
{
    if(data != NULL)
        delete(data);
    data = newData;
    if(data == NULL or data->getType() == PlotData::Linear)
    {
        heightOfPlot = DEFAULT_HEIGHT_LIN;
    }
    else if(data->getType() == PlotData::Logic)
    {
        heightOfPlot = DEFAULT_HEIGHT_LOG;
    }
}

Plot::~Plot()
{

}

/**
 * This function paints actual plot into plot canvas.
 * It determines what it should paint and calls appropriate painting functions.
 */
void Plot::paint(QPainter * painter, int startCoordY,  int width, uint64_t fromTime)
{   
    if(plotCanvas == NULL)
        return;
    qDebug() << "Div size: " << static_cast<PlotCanvas*>(plotCanvas)->getDivSize();
    if(data == NULL)
        return;
    if(data->getType() == PlotData::Logic)
    { // we have logic plot
        if(data->getBitwidth() == 1)
        { // we have plot of sigle bit signal
            paintWire(painter, startCoordY, static_cast<PlotCanvas*>(plotCanvas)->getDivSize(), width, fromTime);
        }
        else
        { // we have plot of some register
            paintRegister(painter, startCoordY, static_cast<PlotCanvas*>(plotCanvas)->getDivSize(), width, fromTime);
        }
    }
    else
    { // we have linear plot /i.e. oscillograph/
        paintLinear(painter, startCoordY, static_cast<PlotCanvas*>(plotCanvas)->getDivSize(), width, fromTime);
    }
    if(active)
    {
        qDebug("Activating..");
        painter->setPen(QColor(0, 0, 255)); //blue
        painter->drawLine(1, startCoordY-1, width, startCoordY-1);
        painter->drawLine(1, startCoordY+heightOfPlot+1, width, startCoordY+heightOfPlot+1);
    }
}

/**
 * This function paints one bit signal according to data.
 */
void Plot::paintWire(QPainter * painter, int startCoordY, double divWidthF, int width, uint64_t fromTime)
{
    int sampleAdder = 1;
    int divWidth = 1;
    double sampleBase = (double)fromTime/data->getDiv();
    size_t sample = ceil(sampleBase); //getting real starting time
    int skew = ceil(divWidthF*((double)sample-sampleBase));
    if(divWidthF < 1)
    {
        sampleAdder = ceil(1/divWidthF);
    }
    else
    {
        divWidth = divWidthF;
    }
    for(int i = 0; i < width; i += divWidth)
    { // going thru viewport width
        if(sample >= data->lastPositionOnBit(0))
        {
            //we have reached end of data block
            continue;
        }
        painter->setPen(QColor(37, 254, 0)); //green
        if(skew > 0 and i == 0)
        {
            switch (data->getDataAtBit(0)[sample-sampleAdder])
            { // drawing line
            case HIGH:
            {
                painter->drawLine(i ,startCoordY, i+skew, startCoordY);
                break;
            }
            case LOW:
            {
                painter->drawLine(i, startCoordY+heightOfPlot, i+skew, startCoordY+heightOfPlot);
                break;
            }
            case HIGH_IMPEDANCE:
            {
                painter->setPen(QColor(254, 37, 0)); //red - high inpedance
                painter->drawLine(i, startCoordY, i+skew, startCoordY);
                painter->drawLine(i, startCoordY+heightOfPlot, i+skew, startCoordY+heightOfPlot);
                break;
            }
            case NO_INFORMATION:
            {
                painter->setPen(QColor(254, 37, 40)); //red - no info
                painter->drawLine(i, startCoordY, i+skew, startCoordY);
                painter->drawLine(i, startCoordY+heightOfPlot, i+skew, startCoordY+heightOfPlot);
                break;
            }
            default:
                qDebug() << "Bad data at: " << sample << " value: " << data->getDataAtBit(0)[sample];
                break;
            }
            if(data->getDataAtBit(0)[sample-sampleAdder] != data->getDataAtBit(0)[sample])
            {
                painter->drawLine(i+skew, startCoordY, i+skew, startCoordY+heightOfPlot);
            }
        }
        switch (data->getDataAtBit(0)[sample]) 
        { // drawing line
            case HIGH:
            { 
                painter->drawLine(i+skew, startCoordY, i+skew+divWidth, startCoordY);
                break;
            }
            case LOW:
            {
                painter->drawLine(i+skew, startCoordY+heightOfPlot, i+skew+divWidth, startCoordY+heightOfPlot);
                break;
            }
            case HIGH_IMPEDANCE:
            {
                painter->setPen(QColor(254, 37, 0)); //red - high inpedance
                painter->drawLine(i+skew, startCoordY, i+skew+divWidth, startCoordY);
                painter->drawLine(i+skew, startCoordY+heightOfPlot, i+skew+divWidth, startCoordY+heightOfPlot);
                break;
            }
            case NO_INFORMATION:
            {
                painter->setPen(QColor(254, 37, 40)); //red - no info
                painter->drawLine(i+skew, startCoordY, i+skew+divWidth, startCoordY);
                painter->drawLine(i+skew, startCoordY+heightOfPlot, i+skew+divWidth, startCoordY+heightOfPlot);
                break;
            }
            default:
                qDebug() << "Bad data at: " << sample << " value: " << data->getDataAtBit(0)[sample];
                break;
        }
        if(data->getDataAtBit(0)[sample] != data->getDataAtBit(0)[sample+sampleAdder])
        {
            painter->drawLine(i+skew+divWidth, startCoordY, i+skew+divWidth, startCoordY+heightOfPlot);
        }
        sample += sampleAdder;
    }
}

/**
 * This function draws text on screen on given coordinates.
 * Within giveh width and heigth, if text would not fit it is truncated and last charater is "+"
 * indicating text overlap.
 * @brief Plot::drawText
 * @param painter
 * @param x
 * @param y
 * @param w
 * @param h
 * @param text
 */
void Plot::drawText(QPainter* painter, int x, int y, int w, int h, QString text)
{
    QFontMetrics fm(painter->font());
    int width = fm.width(text);
    while(width > w)
    {
        text.chop(2);
        if(text.size() == 0)
            return;
        text.append("+");
        width = fm.width(text);
    }
    painter->drawText(x, y, width-1, h, 0, text);
}

/**
 * This function paints register signal according to data.
 */
void Plot::paintRegister(QPainter * painter, int startCoordY, double divWidthF, int width, uint64_t fromTime)
{
    int sampleAdder = 1;
    int divWidth = 1;
    double sampleBase = (double)fromTime/data->getDiv();
    size_t sample = ceil(sampleBase); //getting real starting time
    int skew = ceil(divWidthF*((double)sample-sampleBase));
    if(divWidthF < 1)
    {
        sampleAdder = ceil(1/divWidthF);
    }
    else
    {
        divWidth = divWidthF;
    }
    for(int i = 0; i < width; i += divWidth)
    { // going thru viewport width
        bool prevDiffers = false;
        bool nextDiffers = false;
        for(int ii = 0; ii < data->getBitwidth(); ++ii)
        {
            if(sample >= data->lastPositionOnBit(ii))
            {
                //we have reached end of data block
                return;
            }
            painter->setPen(QColor(37, 254, 0)); //green
            if(data->getDataAtBit(ii)[sample] == HIGH_IMPEDANCE)
            {
                painter->setPen(QColor(254, 37, 0)); //red - high impedance
            }
            if(data->getDataAtBit(ii)[sample] == NO_INFORMATION)
            {
                painter->setPen(QColor(254, 37, 40)); //red - no info
            }
            if(sample != 0 and data->getDataAtBit(ii)[sample] != data->getDataAtBit(ii)[sample-sampleAdder])
            {
                prevDiffers = true;
            }
            if(data->getDataAtBit(ii)[sample] != data->getDataAtBit(ii)[sample+sampleAdder])
            {
                nextDiffers = true;
            }
        }
        if(skew > 0 and i == 0)
        {
            if(nextDiffers)
            { // draw transition to next sample
                painter->drawLine(i+skew-REGISTER_TRANSITION_LENGTH, startCoordY, skew, startCoordY+heightOfPlot/2);
                painter->drawLine(i+skew-REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot, skew, startCoordY+heightOfPlot/2);
            }
            else
            { // no transition draw straight line
                painter->drawLine(skew-REGISTER_TRANSITION_LENGTH, startCoordY, skew, startCoordY);
                painter->drawLine(skew-REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot, skew, startCoordY+heightOfPlot);
            }
            //draw common lines to transitionfull and transitionless samples
            painter->drawLine(i, startCoordY, skew-REGISTER_TRANSITION_LENGTH, startCoordY);
            painter->drawLine(i, startCoordY+heightOfPlot, skew-REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot);
        }
        if(prevDiffers)
        { // draw transition from previous sample
            painter->drawLine(i+skew, startCoordY+heightOfPlot/2, i+skew+REGISTER_TRANSITION_LENGTH, startCoordY);
            painter->drawLine(i+skew, startCoordY+heightOfPlot/2, i+skew+REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot);
        }
        else
        { // no transition draw straight line
            painter->drawLine(i+skew, startCoordY, i+skew+REGISTER_TRANSITION_LENGTH, startCoordY);
            painter->drawLine(i+skew, startCoordY+heightOfPlot, i+skew+REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot);
        }
        if(nextDiffers)
        { // draw transition to next sample
            painter->drawLine(i+skew+divWidth, startCoordY+heightOfPlot/2, i+skew+divWidth-REGISTER_TRANSITION_LENGTH, startCoordY);
            painter->drawLine(i+skew+divWidth, startCoordY+heightOfPlot/2, i+skew+divWidth-REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot);
        }
        else
        { // no transition draw straight line
            painter->drawLine(i+skew+divWidth, startCoordY, i+skew+divWidth-REGISTER_TRANSITION_LENGTH, startCoordY);
            painter->drawLine(i+skew+divWidth, startCoordY+heightOfPlot, i+skew+divWidth-REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot);
        }
        if(prevDiffers)
        {
            //also write datastring
            QPen pen = painter->pen();
            painter->setPen(QColor(255, 255, 255));
            drawText(painter, i+REGISTER_TRANSITION_LENGTH+skew, startCoordY+2, divWidth+REGISTER_TRANSITION_LENGTH, DATASTRING_HEIGHT, getDatastring(sample));
            painter->setPen(pen);
        }
        if(i == 0)
        {
            QPen pen = painter->pen();
            painter->setPen(QColor(255, 255, 255));
            if(skew > 0)
                drawText(painter, i, startCoordY+2, skew, DATASTRING_HEIGHT, getDatastring(sample-sampleAdder));
            else if(sample == 0 or !nextDiffers)
            {
                drawText(painter, i, startCoordY+2, divWidth, DATASTRING_HEIGHT, getDatastring(sample));
            }
            painter->setPen(pen);
        }
        //draw common lines to transitionfull and transitionless samples
        painter->drawLine(i+skew+REGISTER_TRANSITION_LENGTH, startCoordY, i+skew+divWidth-REGISTER_TRANSITION_LENGTH, startCoordY);
        painter->drawLine(i+skew+REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot, i+skew+divWidth-REGISTER_TRANSITION_LENGTH, startCoordY+heightOfPlot);
        sample += sampleAdder;
    }
                
}

/**
 * This function creates datastring string out of data.
 */
QString Plot::getDatastring(int sample)
{
    QString value;
    bool incomplete = false;
    for(int i = 0; i < data->getBitwidth(); ++i)
    {
        if(sample >= data->lastPositionOnBit(i))
        {
            //we have reached end of data block
            return QString("");
        }
        switch (data->getDataAtBit(i)[sample]) 
        { 
            case HIGH:
            { 
                value.append('1');
                break;
            }
            case LOW:
            {
                value.append('0');
                break;
            }
            case HIGH_IMPEDANCE:
            {
                incomplete = true;
                value.append('X');
                break;
            }
            case NO_INFORMATION:
            {
                incomplete = true;
                value.append('U');
                break;
            }
            default:
                break;
        }
    }
    if(incomplete)
    {
        return value;
    }
    else
    {
        bool b;
        //                                                                       ceiling result
        QString result = QString("%1").arg(value.toInt(&b, 2), data->getBitwidth()/4+data->getBitwidth()%4, 16, QChar('0').toUpper());
        return result;
    }
}

/**
 * This function draws linear data into plot.
 * @brief Plot::paintLinear
 * @param painter
 * @param startCoordY
 * @param divWidthF
 * @param width
 * @param fromTime
 */
void Plot::paintLinear(QPainter * painter, int startCoordY, double divWidthF, int width, uint64_t fromTime)
{
    if(data->lastPositionOnBit(0) == 0)
        return;
    int divWidth = 1;
    int sampleAdder = 1;
    double sampleBase = (double)fromTime/data->getDiv();
    size_t sample = ceil(sampleBase); //getting real starting time
    int skew = ceil(divWidthF*((double)sample-sampleBase));
    if(divWidthF < 1)
    {
        sampleAdder = ceil(1/divWidthF);
    }
    else
    {
        divWidth = divWidthF;
    }
    for(int i = 0; i < width; i += divWidth)
    {
        uint32_t sampleResult = 0;
        if(sampleAdder > 1)
        {
            for(int s = -sampleAdder; s < 0; ++s)
            {
                if(((long)sample + s > 0) and (sample + s < data->lastPositionOnBit(0))-1)
                    sampleResult += data->getDataAtBit(0)[sample+s];
                else
                    sampleResult += data->getDataAtBit(0)[sample];
            }
            sampleResult = sampleResult/sampleAdder;
        }
        else
        {
            sampleResult = data->getDataAtBit(0)[sample];
        }
        if(analogInterpolated)
        {
            uint32_t sampleResult1 = 0;
            if(sampleAdder > 1)
            {
                for(int s = 0; s < sampleAdder; ++s)
                {
                    if((sample + s > 0) and (sample + s < data->lastPositionOnBit(0))-1)
                        sampleResult1 += data->getDataAtBit(0)[sample+s];
                    else
                        sampleResult1 += data->getDataAtBit(0)[sample];
                }
                sampleResult1 = sampleResult1/sampleAdder;
            }
            else
            {
                if(sample + sampleAdder < data->lastPositionOnBit(0)-1)
                    sampleResult1 = data->getDataAtBit(0)[sample+sampleAdder];
                else
                    sampleResult1 = data->getDataAtBit(0)[sample];
            }
           // qDebug("Will interpolate");
            if(skew > 0 and i == 0)
            {
                uint32_t previousSampleResult;
                if(sampleAdder > 1)
                {
                    for(int s = -2*sampleAdder; s < -sampleAdder; ++s)
                    {
                        //if((sample + s > 0) and (sample + s < data->lastPositionOnBit(0)))
                            previousSampleResult += data->getDataAtBit(0)[sample+s];
                        //else
                        //    sampleResult += data->getDataAtBit(0)[sample];
                    }
                    previousSampleResult = previousSampleResult/sampleAdder;
                }
                else
                {
                    previousSampleResult = data->getDataAtBit(0)[sample-sampleAdder];
                }
                painter->drawLine(i+skew-divWidth, startCoordY+(heightOfPlot*(255-previousSampleResult)/255),
                                  i+skew, startCoordY+(heightOfPlot*(255-sampleResult)/255));
            }
            painter->drawLine(i+skew, startCoordY+(heightOfPlot*(255-sampleResult)/255),
                              i+skew+divWidth, startCoordY+(heightOfPlot*(255-sampleResult1)/255));
        }
        else
        {
            //qDebug("Will scatter");
            painter->drawPoint(i+skew, startCoordY+(heightOfPlot*(255-sampleResult))/255);
            /*qDebug() << "Data at: " << sample << " is:"
                     << sampleResult
                     << "result is: "
                     << sampleResult/255;*/
        }
        sample += sampleAdder;
        if(sample > data->lastPositionOnBit(0)-2)
            return;
    }
}

/**
 * This function returs div of this plot.
 * @brief Plot::getSmallestDiv
 * @return
 */
uint64_t Plot::getSmallestDiv()
{
    return data->getDiv();
}
