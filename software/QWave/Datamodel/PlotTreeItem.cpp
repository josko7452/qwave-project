//
//   PlotTreeItem.cpp
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


#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include "PlotTreeItem.h"
#include "PlotData.h"
#include <QDebug>

#define TOK_UPSCOPE "$upscope"
#define TOK_SCOPE "$scope"
#define TOK_END "$end"
#define TOK_VAR "$var"
#define TOK_WIRE "wire"
#define TOK_REAL "real"
#define TOK_REG "reg"
#define SPACE " "

PlotTreeItem::~PlotTreeItem()
{
    qDebug() << "Deleting item ";
    for(int i = 0; i < data.size(); ++i)
    {
        delete(data.at(i));
    }
}

/**
 * @brief PlotTreeItem::setPlotData
 * @param newData new data vector to set to this node
 */
void PlotTreeItem::setPlotData(QVector<PlotData *> newData)
{
    data = newData;
}

/**
 * @brief PlotTreeItem::plotData
 * @return vector of data in this node.
 */
QVector<PlotData *> PlotTreeItem::plotData()
{
    return data;
}

/**
 * @brief PlotTreeItem::getChangedForTime
 * @param time
 * @return returns all signals that has transition for time
 */
QVector<char> PlotTreeItem::getChangedForTime(uint64_t time)
{
    QVector<char> vector;
    for(int i = 0; i < data.count(); ++i)
    {
        if(time == 0)
        {
            vector.push_back(data[i]->getSign());
        }
        else
        {
            if(data[i]->comparePrevious(time))
            {
                vector.push_back(data[i]->getSign());
            }
        }
    }
    return vector;
}

/**
 * @brief PlotTreeItem::getNearestTime
 * @param from
 * @return returns nearest transition from time from on this node.
 */
uint64_t PlotTreeItem::getNearestTime(uint64_t from)
{
    uint64_t time = UINT64_MAX;
    for(int i = 0; i < data.count(); ++i)
    {
        uint64_t tm = data.at(i)->getNearestTime(from);
        if(time > tm)
        {
            time = tm;
        }
    }
    return time;
}

/**
 * @brief PlotTreeItem::printSignals prints list of signals in VCD formato to output file.
 * @param out
 */
void PlotTreeItem::printSignals(QTextStream& out)
{
    for(int i = 0; i < data.count(); ++i)
    {
        PlotData* plotdata = data.at(i);
        QString sigType;
        if(plotdata->getType() == PlotData::Logic)
        {
            if(plotdata->getBitwidth() > 1)
            {
                sigType = TOK_REG;
            }
            else
            {
                sigType = TOK_WIRE;
            }
        }
        else
        {
            sigType = TOK_REAL;
        }
        out << TOK_VAR << SPACE << sigType << SPACE << plotdata->getBitwidth() << SPACE
              << plotdata->getSign() << SPACE << plotdata->getName() << SPACE << TOK_END << endl;
    }
}

/**
 * @brief PlotTreeItem::loadFromFile loads node signals from VCD file
 * @param splitter
 * @param divInNs
 * @param signToData
 */
void PlotTreeItem::loadFromFile(FileSplitter & splitter, uint64_t divInNs, QHash<char, PlotData*> & signToData)
{
    enum readState { start, type, width, sign, name };
    int plotType;
    int bitwidth;
    char datasign;
    QString dataname;
    int state = start;
    splitter.getToken();
    while(splitter.currentToken() != TOK_UPSCOPE and splitter.currentToken() != TOK_SCOPE)
    {
        if(splitter.currentToken() == "")
        {
            splitter.getToken();
            continue;
        }
        if(splitter.currentToken() == TOK_END)
        {
            /*if(dataname == "CLK")
            {
                qDebug() << "mame CLK";
            }*/
            PlotData * newPlotData;
            if(signToData.contains(datasign))
            {
                newPlotData = new PlotData(plotType, dataname, divInNs, bitwidth, datasign,
                                                      signToData[datasign]->getShadowData(), signToData[datasign]->getShadowDataPosition(),
                                                      signToData[datasign]->getShadowDataAllocated());
            }
            else
            {
                newPlotData = new PlotData(plotType, dataname, divInNs, bitwidth, datasign);
                signToData[datasign] = newPlotData;
            }
            data.append(newPlotData);
            state = start;
        }
        else
        {
            switch (state)
            {
                case start:
                {
                    if(splitter.currentToken() == TOK_VAR)
                    {
                        state = type;
                    }
                    else
                    {    
                        throw IOException("Expected $var on line: "+QString::number(splitter.getLineNumber()));
                    }
                    break;
                }
                case type:
                {
                    if(splitter.currentToken() == TOK_WIRE or splitter.currentToken() == TOK_REG)
                    {
                        plotType = PlotData::Logic;
                    }
                    else
                        if(splitter.currentToken() == TOK_REAL)
                        {
                            plotType = PlotData::Linear;
                        }
                    state = width;
                    break;
                }
                case width:
                {
                    QRegExp regExp("(\\d+)");
                    if(regExp.exactMatch(splitter.currentToken()))
                    {
                        bitwidth = splitter.currentToken().toInt();
                    }
                    else
                    {
                        throw IOException("Expected valid number on line after type (third column): "+QString::number(splitter.getLineNumber()));
                    }
                    state = sign;
                    break;
                }
                case sign:
                {
                    if(splitter.currentToken().length() == 1)
                    {
                        datasign = splitter.currentToken().at(0).toLatin1();
                    }
                    else
                    {
                        throw IOException("Expected valid sign on line after bitwidth (fourth column): "+QString::number(splitter.getLineNumber()));
                    }
                    state = name;
                    break;
                }
                case name:
                {
                    dataname = splitter.currentToken();
                    break;
                }
                default:
                {
                    throw IOException("Expected $end on line:"+QString::number(splitter.getLineNumber()));
                    break;
                }
            }
        }
        splitter.getToken();
    }
}
