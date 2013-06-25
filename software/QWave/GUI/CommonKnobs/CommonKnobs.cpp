//
//   CommonKnobs.cpp
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


#include "CommonKnobs.h"
#include <QDebug>
#include <QRegExp>
#include "../Board.h"

CommonKnobs::CommonKnobs(QWidget * parent) : QWidget(parent)
{
    ui.setupUi(this);
}

void CommonKnobs::setBoard(QWidget * board)
{
    this->board = board;
    ui.scaleControl->connectPlotCanvas(static_cast<Board*>(board)->getPlotCanvas());
    connect(static_cast<Board*>(board)->getPlotCanvas(), SIGNAL(fromToUpdated(uint64_t,uint64_t)), this, SLOT( updateFromTo(uint64_t,uint64_t)));
    connect(ui.timeControl, SIGNAL(valueChanged(uint64_t)), static_cast<Board*>(board)->getPlotCanvas(), SLOT(setFrom(uint64_t)));
}

CommonKnobs::~CommonKnobs()
{

};

void CommonKnobs::updateFromTo(uint64_t from, uint64_t to)
{
    ui.timeControl->updateFrom(from);
    ui.timeControl->updateTo(to);
}
