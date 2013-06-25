//
//   PlotTreeModel.h
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


#ifndef QWave_PlotTreeModel_h
#define QWave_PlotTreeModel_h

#include <QStandardItemModel>
#include <QStack>
#include <QDateTime>
#include "PlotTreeItem.h"
#include "FileSplitter.h"
#include "../Exceptions/IOException.h"
#include <QFile>

/**
 * @brief The PlotTreeModel class represents tree model of plots.
 */
class PlotTreeModel : public QStandardItemModel
{
    Q_OBJECT
    QString date;
    QString version;
    QString comment;
    uint64_t timescaleInPs;
    
    QStandardItem * currentTopLevelItem;
    QStack<QStandardItem*> lastTopLevelItem;
    QHash<char, PlotData*> signToData;
    QVector<QStandardItem*> itemsToDump;
    void parseDump(FileSplitter & splitter, size_t sampleNumber, QString & token);
    void loadScope(FileSplitter & splitter);
    void loadDump(FileSplitter & splitter);
    void parseBit(char sourceValue, char sign, int bitNumber, size_t sampleNumber, FileSplitter & splitter);
    void insertValue(char sign, int bitNumber, unsigned char value, size_t sampleNumber);
    void printScopes(QTextStream& out, PlotTreeItem* topScope);
    uint64_t getNearestTime(uint64_t from);
    QVector<char> getChangedForTime(uint64_t time);
    char getBitChar(int bit, PlotData* data, uint64_t time);
    void printDumps(QTextStream& out);
public:
    PlotTreeModel() : QStandardItemModel(), date(QDateTime::currentDateTime().toString(Qt::ISODate)), comment(""), version("QWave") {}
    ~PlotTreeModel();
    void loadFromFile(FileSplitter & splitter);
    void saveToFile(QFile* file);
    void initHierarchy();
    void registerData(PlotData* data);
    void setCurrentTop(QStandardItem * currentTopLevelItem) { this->currentTopLevelItem = currentTopLevelItem; }
    void setTimeScale(uint64_t timescale) { timescaleInPs = timescale;  }
    void setComment(QString comment) { this->comment = comment; }
};
#endif
