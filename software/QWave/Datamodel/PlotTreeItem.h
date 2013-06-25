//
//   PlotTreeItem.h
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

#ifndef QWave_PlotTreeItem_h
#define QWave_PlotTreeItem_h

#include <QList>
#include "PlotData.h"
#include "FileSplitter.h"
#include "../Exceptions/IOException.h"
#include <QVector>
#include <QStandardItem>

/**
 * @brief The PlotTreeItem class represents item in node hierarchy of plots.
 */
class PlotTreeItem : public QStandardItem
{
private:
    QVector<PlotData *> data;
public:
    void setPlotData(QVector<PlotData *> newData);
    void appendPlotData(PlotData* data) { this->data.push_back(data); }
    QVector<PlotData *> plotData();
    void printSignals(QTextStream& out);
    void loadFromFile(FileSplitter & splitter, uint64_t divInNs, QHash<char, PlotData*> & signToData);
    uint64_t getNearestTime(uint64_t from);
    QVector<char> getChangedForTime(uint64_t time);
    PlotTreeItem(const QString & text) : QStandardItem(text) {}
    ~PlotTreeItem();
};

#endif
