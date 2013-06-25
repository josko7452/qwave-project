//
//   TimeSpinBox.cpp
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


#include "TimeSpinBox.h"
#include <QDebug>

TimeSpinBox::TimeSpinBox(QWidget * parent) : QDoubleSpinBox(parent)
{

}

/**
 * @brief TimeSpinBox::validate validator which looks for valid  double number and ps/ns/ms/us/s suffix.
 * @param input
 * @param pos
 * @return
 */
QValidator::State TimeSpinBox::validate ( QString & input, int & pos ) const
{
    QRegExp regExp(tr("(\\d+)(\\s*)(ps|ns|ms|us|s)"));
    if (regExp.exactMatch(input))
    {
        return QValidator::Acceptable;
    }
    else
    {
        return QValidator::Intermediate;
    }
}

/**
 * @brief TimeSpinBox::valueFromText  creates absolute ps value from value in spin box and suffix (ps/ns...)
 * @param text
 * @return
 */
double TimeSpinBox::valueFromText(const QString &text) const
{
    QRegExp regExp(tr("(\\d+)(\\s*)(ps|ns|ms|us|s)"));
    if (regExp.exactMatch(text))
    {
        if(S == regExp.cap(3))
        {
            return regExp.cap(1).toDouble()*E12;
        }
        else if (MS == regExp.cap(3))
        {
            return regExp.cap(1).toDouble()*E9;
        }
        else if(US == regExp.cap(3))
        {
            return regExp.cap(1).toDouble()*E6;
        }
        else if (NS == regExp.cap(3))
        {
            qDebug() << regExp.cap(1);
            return regExp.cap(1).toDouble()*E3;
        }
        else if (PS == regExp.cap(3))
        {
            return regExp.cap(1).toDouble();
        }
    }
    return 0;
}

/**
 * @brief TimeSpinBox::textFromValue crates human readable text from picoseconds to show in spinbox.
 * @param value
 * @return
 */
QString TimeSpinBox::textFromValue(double value) const
{
    qDebug() << "Time Spinbox textFromValue: " << value;
    if(value > E12)
    {
        return (QString::number(value/E12)+" "+S);
    }
    else if(value > E9)
    {
        return (QString::number(value/E9)+" "+MS);
    }
    else if(value > E6)
    {
        return (QString::number(value/E6)+" "+US);
    }
    else if(value > E3)
    {
        return (QString::number(value/E3)+" "+NS);
    }
    else
    {
        return (QString::number(value)+" "+PS);
    }
}
