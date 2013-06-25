//
//   PlotData.cpp
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

#include "PlotData.h"
#include "../Exceptions/Exception.h"
#include <QDebug>

static const size_t PREALLOCATE = 100;

PlotData::PlotData(int type, QString name, uint64_t divInPs, int width, char sign) : type(type), name(name), divInPs(divInPs), bitwidth(width), sign(sign), isShadow(false)
{
    data = new QVector<unsigned char *>;
    dataPosition = new QVector<size_t>;
    dataAllocated = new QVector<size_t>;
    for (int i = 0; i < bitwidth; ++i)
    {
        unsigned char * newData = static_cast<unsigned char*>(malloc(PREALLOCATE*sizeof(unsigned char)));
        if(newData == NULL)
        {
            throw Exception("Cannot allocate memory.");
        }
        data->append(newData);
        dataPosition->append(0);
        dataAllocated->append(PREALLOCATE);
    }
}

PlotData::PlotData(int type, QString name, uint64_t divInPs, int width, char sign, QVector<unsigned char *> * shadowData, QVector<size_t> * shadowDataPosition, QVector<size_t> * shadowDataAllocated) : type(type), name(name), divInPs(divInPs), bitwidth(width), sign(sign), isShadow(true)
{
    data = shadowData;
    dataPosition = shadowDataPosition;
    dataAllocated = shadowDataAllocated;
}

/**
 * @brief PlotData::appendDataAtBit appends new data newData at bitNumber bit.
 * @param bitNumber bit number to append data on.
 * @param newData new data to append.
 */
void PlotData::appendDataAtBit(int bitNumber, unsigned char newData)
{
    if(dataPosition->at(bitNumber) >= dataAllocated->at(bitNumber))
    {
        (*dataAllocated)[bitNumber] = dataAllocated->at(bitNumber)*2;
        unsigned char * newData = data->at(bitNumber);
        (*data)[bitNumber] = static_cast<unsigned char*>(realloc(newData, dataAllocated->at(bitNumber)*sizeof(unsigned char)));
        if(data->at(bitNumber) == NULL)
        {
            throw Exception("Cannot reallocate memory.");
        }
    }
    //qDebug() << "Adding: " << newData << " to " << dataPosition[bitNumber];
    if(dataPosition->at(bitNumber) > 9990 and dataPosition->at(bitNumber) < 10020)
    {
        qDebug() << "Adding: " << newData << " to bit: " << bitNumber << " to pos: " << (*dataPosition)[bitNumber];
    }
    data->at(bitNumber)[(*dataPosition)[bitNumber]++] = newData;
}

/**
 * @brief PlotData::getNearestTimeOnBit
 * @param from
 * @param bit
 * @return returns nearest transition on bit number bit from time from.
 */
uint64_t  PlotData::getNearestTimeOnBit(uint64_t from, int bit)
{
    if(dataPosition->at(bit) == from)
    {
        return UINT64_MAX;
    }
    uint8_t valueOrigin = data->at(bit)[from];
    for(; from < dataPosition->at(bit); ++from)
    {
        if(data->at(bit)[from] != valueOrigin)
        {
            return from;
        }
    }
    return from;
}

/**
 * @brief PlotData::getNearestTime
 * @param from
 * @return returns nearest transition from time from.
 */
uint64_t PlotData::getNearestTime(uint64_t from)
{
    uint64_t time = UINT64_MAX;
    for(int i = 0; i < data->count(); ++i)
    {
        uint64_t tm = getNearestTimeOnBit(from, i);
        if(tm < time)
        {
            time = tm;
        }
    }
    return time;
}

/**
 * @brief PlotData::comparePrevious compares data on all bits with previous sample.
 * @param from
 * @return returns true if difference was found, else returns false.
 */
bool PlotData::comparePrevious(uint64_t from)
{
    for(int i = 0; i < data->count(); ++i)
    {
        qDebug() << data->at(i)[from] << " vs " << data->at(i)[from-1];
        if(data->at(i)[from] != data->at(i)[from-1])
        {
            return true;
        }
    }
    return false;
}
