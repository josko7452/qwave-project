//
//   PlotData.h
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


#ifndef QWave_PlotData_h
#define QWave_PlotData_h

#include <QObject>
#include <QPair>
#include <QVector>
#include <QDebug>
#include <inttypes.h>

static const int DEFAULT_DIV = 1;
/**
 * Represents DATAMODEL of plot.
 * holds pointer to vector of data
 */
class PlotData
{
    bool isShadow;
    QString name;
    char sign;
    int type;
    int bitwidth;
    QVector<unsigned char *> * data;
    QVector<size_t> * dataPosition;
    QVector<size_t> * dataAllocated;
    uint64_t divInPs;
    uint64_t getNearestTimeOnBit(uint64_t from, int bit);
public:
    enum PlotType { Logic, Linear };
    PlotData(int type, QString name, uint64_t divInPs, int width, char sign);
    PlotData(int type, QString name, uint64_t divInPs, int width, char sign, QVector<unsigned char *> * shadowData, QVector<size_t> * shadowDataPosition, QVector<size_t> * shadowDataAllocated);
    ~PlotData()
    {
        if(isShadow)
            return;
        for(int i = 0; i < data->size(); ++i)
        {
            free(data->at(i));
        }
        delete(data);
        delete(dataPosition);
        delete(dataAllocated);
        qDebug() << "deleted PlotData: " << name << "sign: " << sign;
    }
    
    void setDataAtBit(int bitNumber, unsigned char * newData)
    {
        delete(data->at(bitNumber));
        (*data)[bitNumber] = newData;
    }
    QVector<unsigned char *> * getShadowData() { return data; }
    QVector<size_t> * getShadowDataPosition() { return dataPosition; }
    QVector<size_t> * getShadowDataAllocated() { return dataAllocated; }
    bool getIsShadow() { return isShadow; }
    unsigned char * getDataAtBit(int bitNumber)
    {
        if(bitNumber <= data->count())
            return data->at(bitNumber);
        else
            return NULL;
    }
    int getType() { return type; }
    char getSign() { return sign; }
    int getBitwidth() { return bitwidth; }
    const QString & getName() const { return name; }
    uint64_t getDiv() { return divInPs; }
    void setDiv(uint64_t div) { this->divInPs = div; }
    void appendDataAtBit(int bitNumber, unsigned char newData);
    size_t lastPositionOnBit(int bitNumber) { return dataPosition->at(bitNumber); }
    uint64_t getNearestTime(uint64_t from);
    bool comparePrevious(uint64_t from);
};

#endif
