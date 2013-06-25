//
//   PlotTreeModel.cpp
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
#include "PlotTreeModel.h"
#include "Constants.h"
#include <QDebug>
#include <QSet>
#include "PlotData.h"
#include <QDateTime>


#define TOK_DATE "$date"
#define TOK_COMMENT "$comment"
#define TOK_END "$end"
#define TOK_ENDDEFINITIONS "$enddefinitions"
#define TOK_VERSION "$version"
#define TOK_TIMESCALE "$timescale"
#define TOK_SCOPE "$scope"
#define TOK_UPSCOPE "$upscope"
#define TOK_DUMPVARS "$dumpvars"
#define TOK_MODULE "module"
#define SPACE " "

#define TOP "top"

/**
  * Destructor
  */
PlotTreeModel::~PlotTreeModel()
{
    //QStandardItemModel::~QStandardItemModel();
}

/**
 * @brief PlotTreeModel::initHierarchy initialize tree hierarchy of plots.
 */
void PlotTreeModel::initHierarchy()
{
    currentTopLevelItem = this->invisibleRootItem();
    PlotTreeItem * newItem = new PlotTreeItem(TOP);
    currentTopLevelItem->appendRow(newItem);
    lastTopLevelItem.push(currentTopLevelItem);
    currentTopLevelItem = newItem;
}

/**
 * @brief PlotTreeModel::saveToFile save model to VCD file.
 * @param file
 */
void PlotTreeModel::saveToFile(QFile* file)
{
    timescaleInPs = static_cast<PlotTreeItem*>(this->invisibleRootItem()->child(0))->plotData()[0]->getDiv();
    QTextStream out(file);
    if(date != "")
        out << TOK_DATE << endl << SPACE << this->date << endl << TOK_END << endl;
    if(version != "")
        out << TOK_VERSION << endl << SPACE << this->version << endl << TOK_END <<endl;
    if(comment != "")
        out << TOK_COMMENT<< endl <<SPACE << this->comment << endl << TOK_END << endl;
    out << TOK_TIMESCALE << SPACE << this->timescaleInPs << "ps" << SPACE << TOK_END << endl;
    printScopes(out, static_cast<PlotTreeItem*>(this->invisibleRootItem()->child(0)));
    out << TOK_ENDDEFINITIONS << SPACE << TOK_END << endl;
    out << TOK_DUMPVARS << endl;
    printDumps(out);
}

/**
 * @brief PlotTreeModel::printScopes print scopes to VCD file
 * @param out
 * @param topScope
 */
void PlotTreeModel::printScopes(QTextStream& out, PlotTreeItem* topScope)
{
    itemsToDump.push_back(topScope);
    out << TOK_SCOPE << SPACE << TOK_MODULE << SPACE<< topScope->text() << SPACE << TOK_END << endl;
    topScope->printSignals(out);
    for(int row; row < topScope->rowCount(); ++row)
    {
        this->printScopes(out, static_cast<PlotTreeItem*>(topScope->child(row)));
    }
    out << TOK_UPSCOPE << SPACE << TOK_END << endl;
}

/**
 * @brief PlotTreeModel::getNearestTime
 * @param from
 * @return returns nearest time from all nodes
 */
uint64_t PlotTreeModel::getNearestTime(uint64_t from)
{
    uint64_t time = UINT64_MAX;
    for(int i = 0; i < itemsToDump.count(); ++i)
    {
        uint64_t tm = static_cast<PlotTreeItem*>(itemsToDump[i])->getNearestTime(from);
        if(time > tm)
        {
            time = tm;
        }
    }
    return time;
}

/**
 * @brief PlotTreeModel::getBitChar
 * @param bit
 * @param data
 * @param time
 * @return get model value from character in VCD
 */
char PlotTreeModel::getBitChar(int bit, PlotData* data, uint64_t time)
{
    char value = 'x';
    if(data->getDataAtBit(bit)[time] == HIGH)
    {
        value = '1';
    }
    else if(data->getDataAtBit(bit)[time]  == LOW)
    {
        value = '0';
    }
    else if(data->getDataAtBit(bit)[time]  == NO_INFORMATION)
    {
        value = 'x';
    }
    else if(data->getDataAtBit(bit)[time]  == HIGH_IMPEDANCE)
    {
        value = 'u';
    }
    return value;
}

/**
 * @brief PlotTreeModel::getChangedForTime
 * @param time
 * @return returns vector all changed plots by their VCD sign
 */
QVector<char> PlotTreeModel::getChangedForTime(uint64_t time)
{
    QVector<char> vector;
    for(int i = 0; i < itemsToDump.count(); ++i)
    {
        QVector<char> newVec = static_cast<PlotTreeItem*>(itemsToDump[i])->getChangedForTime(time);
        vector += newVec;
    }
    return vector;
}

/**
 * @brief PlotTreeModel::printDumps
 * Prints dump part of VCD file.
 * @param out
 */
void PlotTreeModel::printDumps(QTextStream& out)
{
    uint64_t currentTime = 0;
    while(currentTime != UINT64_MAX)
    {
        if(currentTime > 0)
            out << "#" << currentTime << endl;
        QList<char> signs =getChangedForTime(currentTime).toList();
        signs = QSet<char>::fromList(signs).toList();
        for(int i = 0; i < signs.count(); ++i)
        {
            PlotData* data = signToData[signs[i]];
            if(data->getType() ==  PlotData::Linear)
            {
                out << "r" << data->getDataAtBit(0)[currentTime] << " " << signs[i] << endl;
            }
            else
            {
                if(data->getBitwidth() > 1)
                {
                    out << "b";
                    for(int bit = 0; bit < data->getBitwidth(); ++bit)
                    {
                        out << getBitChar(bit, data, currentTime);
                    }
                    out << " " << signs[i] << endl;
                }
                else
                {
                    out << getBitChar(0, data, currentTime) << signs[i] << endl;
                }
            }
        }
        if(currentTime == 0)
            out << TOK_END << endl;
        currentTime = getNearestTime(currentTime);
    }
}

/**
 * This method loads header of VCD file and constructs corresponding data model.
 */
void PlotTreeModel::loadFromFile(FileSplitter & splitter)
{
    currentTopLevelItem = this->invisibleRootItem();
    splitter.getToken();
    while(splitter.currentToken() != TOK_ENDDEFINITIONS)
    {
        //qDebug() << token;
        if(splitter.currentToken()== TOK_COMMENT)
        {
            splitter.getToken();
            while(splitter.currentToken() != TOK_END)
            {
                comment = comment + " " + splitter.currentToken();
                splitter.getToken();
            }
        }
        else
        {
            if(splitter.currentToken() == TOK_DATE)
            {
                splitter.getToken();
                while(splitter.currentToken() != TOK_END)
                {
                    date = date + " " + splitter.currentToken();
                    splitter.getToken();
                }
            }
            else
            {
                if(splitter.currentToken() == TOK_VERSION)
                {
                    splitter.getToken();
                    while(splitter.currentToken() != TOK_END)
                    {
                        version = version + " " + splitter.currentToken();
                        splitter.getToken();
                    }
                }
                else
                {
                    if(splitter.currentToken()  == TOK_TIMESCALE)
                    {
                        QString timescale;
                        splitter.getToken();
                        while(splitter.currentToken() != TOK_END)
                        {
                            timescale = timescale + " " + splitter.currentToken();
                            splitter.getToken();
                        }
                        qDebug() << timescale;
                        QRegExp regExp(tr("(\\s*)(\\d+)(ps|ns|ms|us|s)(\\s*)"));
                        if(regExp.exactMatch(timescale))
                        {
                            if(S == regExp.cap(3))
                            {
                                timescaleInPs = regExp.cap(2).toDouble()*E12;
                            }
                            else
                                if (MS == regExp.cap(3))
                                {
                                    timescaleInPs = regExp.cap(2).toDouble()*E9;
                                }
                                else
                                    if(US == regExp.cap(3))
                                    {
                                        timescaleInPs = regExp.cap(2).toDouble()*E6;
                                    }
                                    else
                                        if (NS == regExp.cap(3))
                                        {
                                            timescaleInPs = regExp.cap(2).toDouble()*E3;
                                        } 
                                        else if (PS == regExp.cap(3))
                                        {
                                            timescaleInPs = regExp.cap(2).toDouble();
                                        }
                        }
                        else
                        {
                            throw IOException("Invalid $timescale on line: "+QString::number(splitter.getLineNumber()));
                        }
                    }
                    else
                    {
                        if(splitter.currentToken() == TOK_SCOPE)
                        {
                            loadScope(splitter);
                            static_cast<PlotTreeItem*>(currentTopLevelItem)->loadFromFile(splitter, timescaleInPs, signToData); //load what is under scope
                            continue;
                        }
                        else
                        {
                            if(splitter.currentToken() == TOK_END or splitter.currentToken() == "")
                            {
                                splitter.getToken();
                                continue;
                            }
                            else
                            {
                                if(splitter.currentToken() == TOK_UPSCOPE)
                                {
                                    if(lastTopLevelItem.isEmpty())
                                        throw IOException("Unexpected '$upscope' on line: "+QString::number(splitter.getLineNumber()));
                                    currentTopLevelItem = lastTopLevelItem.pop();
                                }
                                else
                                {
                                    throw IOException("Expected vcd directive on line: "+QString::number(splitter.getLineNumber()));
                                }
                            }
                        }
                    }
                }
            }
        }
        splitter.getToken();
    }
    splitter.getToken();
    if(splitter.currentToken() != TOK_END)
    {
        throw IOException("Expected '$end' tag of $enddefinitions on line: "+QString::number(splitter.getLineNumber()));
    }
    else
    {
        loadDump(splitter);
    }
   /* for(int i = 0; i < signToData['$']->lastPositionOnBit(0); ++i)
    {
        qDebug() << "At pos: " << i << " value: " << signToData['$']->getDataAtBit(0)[i];
    } */
}

/**
 * This method loads dump section of VCD files into datamodel.
 */
void PlotTreeModel::loadDump(FileSplitter & splitter)
{
    size_t position = 0;
    enum dumpParseState { start, dumpvars, dump };
    int state = start;
    splitter.getToken();
    while(splitter.currentToken() != "!$EOF$!")
    { // iterating to end of file
        if(splitter.currentToken() == "")
        {
            splitter.getToken();
            continue;
        }
        //qDebug() << token;
        switch (state) 
        {
            case start:
            {
                if(splitter.currentToken() == TOK_DUMPVARS)
                {
                    state = dumpvars;
                }
                else
                {
                    throw IOException("Expected '$dumpvars' on line: "+QString::number(splitter.getLineNumber()));
                }
                break;
            }
            case dumpvars:
            {
                if(splitter.currentToken() == TOK_END)
                {
                    state = dump;
                }
                else
                {
                    QString currTok(splitter.currentToken());
                    parseDump(splitter, position, currTok);
                }
                break;
            }
            case dump:
            {
            QString token(splitter.currentToken());
                if('#' == token.at(0).toAscii())
                {
                    token.remove(0, 1);
                    position = token.toULong();
                }
                else
                {
                    parseDump(splitter, position, token);
                }
                state = dump;
                break;
            }
            default:
            {
                break;
            }
        }
        splitter.getToken();
    }
    // finally filling up to last read position
    //qDebug() << "Pos: " << position;
    QHash<char, PlotData*>::iterator i;
    for (i = signToData.begin(); i != signToData.end(); ++i)
    {
        if(i.value()->getIsShadow())
            return;
        for (int bitNumber = 0; bitNumber < i.value()->getBitwidth(); ++bitNumber) 
        {
            for(size_t currentSample = i.value()->lastPositionOnBit(bitNumber); currentSample <= position; ++currentSample)
            {
                i.value()->appendDataAtBit(bitNumber, i.value()->getDataAtBit(bitNumber)[currentSample-1]);
            }
        }
    }
}

/**
 * This function parses dump sample from VCD file.
 * First it detects wether it is real, register or single value.
 */
void PlotTreeModel::parseDump(FileSplitter & splitter, size_t sampleNumber, QString & token)
{
    //qDebug() << token.at(0).toAscii();
    if(token.at(0).toAscii() == 'b' or token.at(0).toAscii() == 'r')
    { // we have binary or real
        char datasign;
        splitter.getToken();
        QString signToken = splitter.currentToken();
        if(signToken.length() == 1)
        {
            datasign = signToken.at(0).toLatin1();
        }
        else
        {
            throw IOException("Expected valid sign on line: "+QString::number(splitter.getLineNumber()));
        }
        if(token.at(0).toAscii() == 'b')
        { // we have register
            for(int i = 1; i < token.size(); ++i) // from 1 to omit b/r
            {
                parseBit(token.at(i).toAscii(), datasign, i-1, sampleNumber, splitter);
            }
        }
        else
        {
            token.remove(0, 1);
            bool succ;
            unsigned char newValue = token.toUInt(&succ, 10);
            if(succ)
            {
                insertValue(datasign, 0, newValue, sampleNumber);
            }
            else
            {
                throw IOException("Expected valid value on line: "+QString::number(splitter.getLineNumber()));
            }
        }
    }
    else
    {
        if(token.length() == 2 and token.at(0).isLetterOrNumber())
        {
            parseBit(token.at(0).toAscii(), token.at(1).toAscii(), 0, sampleNumber, splitter);
        }
        else
        {
            throw IOException("Expected valid value and sign on line: "+QString::number(splitter.getLineNumber()));
        }
    }
}

/**
 * This function analyzes binary to one of four categories LOW, HIGH, HIGH_IMPEDANCE and NO_INFORMATION
 * and adds it to corresponding PlotData determined by sign of data dump. 
 */
void PlotTreeModel::parseBit(char sourceValue, char sign, int bitNumber, size_t sampleNumber, FileSplitter & splitter)
{
    unsigned char value;
    switch (sourceValue) 
    {
        case '0':
        {
            value = LOW;
            break;
        }
        case '1':
        {
            value = HIGH;
            break;
        }
        case 'u':
        {
            value = HIGH_IMPEDANCE;
            break;
        }
        case 'x':
        {
            value = NO_INFORMATION;
            break;
        }
        default:
        {
            throw IOException("Expected valid value on line: "+QString::number(splitter.getLineNumber()));
            break;   
        }
    }
    insertValue(sign, bitNumber, value, sampleNumber);
}

/**
 * Inserts value to corresponding bit and sample. Fills up empty space with last values.
 */
void PlotTreeModel::insertValue(char sign, int bitNumber, unsigned char value, size_t sampleNumber)
{
    PlotData * currentData = signToData[sign];
    if(currentData == NULL)
    {
        throw IOException("Dump data sign not in declaration: "+sign);
    }
    unsigned char fill;
    if(currentData->lastPositionOnBit(bitNumber) == 0)
        fill = currentData->getDataAtBit(bitNumber)[currentData->lastPositionOnBit(bitNumber)];
    else
        fill = currentData->getDataAtBit(bitNumber)[currentData->lastPositionOnBit(bitNumber)-1];
    for(size_t currentSample = currentData->lastPositionOnBit(bitNumber); currentSample < sampleNumber; ++currentSample)
    {
        currentData->appendDataAtBit(bitNumber, fill);
    }
    currentData->appendDataAtBit(bitNumber, value);
}

/**
 * This method loads scope section of header of VCD file.
 */
void PlotTreeModel::loadScope(FileSplitter & splitter)
{
    bool gotModule = false;
    splitter.getToken();
    while(splitter.currentToken() != TOK_END)
    {
        if(gotModule)
        {
            PlotTreeItem * newItem = new PlotTreeItem(splitter.currentToken());
            currentTopLevelItem->appendRow(newItem);
            lastTopLevelItem.push(currentTopLevelItem);
            currentTopLevelItem = newItem;
        }
        else
            if(splitter.currentToken() == TOK_MODULE)
            {
                gotModule = true;
            }
        splitter.getToken();
    }
}

/**
 * @brief PlotTreeModel::registerData
 * register data for given sign.
 * @param data
 */
void PlotTreeModel::registerData(PlotData* data)
{
    signToData[data->getSign()] = data;
    static_cast<PlotTreeItem*>(currentTopLevelItem)->appendPlotData(data);
}
