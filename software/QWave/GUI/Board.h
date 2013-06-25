//
//  Board.h
//  QWave
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

#ifndef QWave_Board_h
#define QWave_Board_h

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSplitter>
#include <QScrollArea>
#include "CommonKnobs/CommonKnobs.h"
#include "PlotCanvas.h"
#include "PlotTreeWidget.h"
#include "../Datamodel/PlotTreeModel.h"
#include <ui_Board.h>
#include "../Device/CaptureController.h"

/**
 * This class represents "tab" of applications and provides wrapper for all underlying graphics widgets.
 * @brief The Board class
 */
class Board : public QWidget
{
    Q_OBJECT
private:
    QString filePath;
    Ui::Board ui;
    QString name;
    PlotTreeModel * treeModel;
    CaptureController controller;
    char randomSign;
    ulong scrollbarDivision;
private slots:
    void changePageNames(QListWidgetItem* current, QListWidgetItem* previous);
    void plotClickAction(const QModelIndex & index);
    void showPlotContextMenu(QPoint p);
    void setInterpolateOnPlot();
    void openTriggerDialog();
public slots:
    void refreshPlotNames();
    void updateFromScroll(int val) { ui.plotCanvas->setFrom(val*scrollbarDivision); }
    void setFrom( uint64_t from);
public:
    void updateFromTo();
    void registerData(PlotData* data) { treeModel->registerData(data); }
    void initHierarchy() { treeModel->initHierarchy(); }
    QString getFilePath() { return filePath; }
    void setFilePath(QString filePath) { this->filePath = filePath; }
    void connectDevice();
    void startCapture(bool contionous);
    void stopCapture() { controller.stopCapture(); }
    AbstractDevice* getCurrentDevice() { return controller.getDeviceByPlot(ui.plotCanvas->getPlot(ui.plotNamesColumn->currentIndex().row())); }
    int getCurrentDataId() { return controller.getDataIdByPlot(ui.plotCanvas->getPlot(ui.plotNamesColumn->currentIndex().row())); }
    Plot* createMeasurementPlot(int type);
    char getRandomSign() { return randomSign++; }
    void setStatusBarText(QString text) { emit updateStatusBar(text); }
    void connectWindow(QObject * window);
    void addMeasurementPlot();
    void removeMeasurementPlot();
    int getDivCount() { return ui.plotCanvas->getDivCount(); }
    PlotCanvas* getPlotCanvas() { return ui.plotCanvas; }
    uint64_t getScale() { return ui.plotCanvas->getScale(); }
    void loadFromFile(QFile* file);
    void saveToFile(QFile* file);
    Board(QString & name, QWidget * parent = 0);
    ~Board();
signals:
    void updateStatusBar(QString);
};

#endif
