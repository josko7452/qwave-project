//
//   Board.cpp
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

#include "Board.h"
#include "Plot.h"
#include "../Datamodel/PlotData.h"
#include <QFileInfo>
#include <QRegExp>
#include "../Datamodel/FileSplitter.h"
#include "../Exceptions/IOException.h"
#include <QDebug>
#include <QMenu>
#include "Window.h"
#include "ProbeAssignDialog.h"
#include "TriggerSetupDialog.h"
#include "../Device/Capabilities.h"
#include "ConnectDevice.h"

/**
 * Constructor of board, borad represents a "tab" in apllication.
 * @brief Board::Board
 * @param name name of tab
 * @param parent parent widget, in this application it is always instance of Window
 */
Board::Board(QString & name, QWidget * parent) : name(name), QWidget(parent), randomSign('!'), controller(this), filePath("")
{
    ui.setupUi(this);
    treeModel = new PlotTreeModel();
    ui.treeWidget->setModel(treeModel);
    ui.plotCanvas->setBoard(this);
    ui.commonKnobs->setBoard(this);
    //ui.commonKnobGroup->setMinimumWidth(170);
    //ui.commonKnobGroup->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    connect(ui.plotNamesColumn, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this,
            SLOT(changePageNames(QListWidgetItem*, QListWidgetItem*)));
    connect(ui.commonKnobs, SIGNAL(scaleTimeUpdated()), ui.plotCanvas, SLOT(repaint()));
    //connect(ui.commonKnobs, SIGNAL(scaleTimeUpdated()), this, SLOT(updateFromTo()));
    connect(ui.treeWidget, SIGNAL(itemSelected()), this, SLOT(refreshPlotNames()));
    connect(ui.plotNamesColumn, SIGNAL(pressed(QModelIndex)), this, SLOT(plotClickAction(QModelIndex)));
    ui.plotNamesColumn->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.plotNamesColumn, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));
    controller.setPlotCanvas(ui.plotCanvas);
    ui.horizontalScrollBar->hide();
    connect(ui.horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(updateFromScroll(int)));
    connect(&controller, SIGNAL(newFrom(uint64_t)), this, SLOT(setFrom(uint64_t)));
}

/**
 * Connects Window to board.
 * @brief Board::connectWindow
 * @param window pointer to window instance
 */
void Board::connectWindow(QObject * window)
{
    connect(this, SIGNAL(updateStatusBar(QString)), static_cast<Window*>(window), SLOT(setStatusBarText(QString)));
}

Board::~Board() 
{
    delete(treeModel);
    qDebug() << "Board deleting";
};

/**
 * This function is triggered when user cliks on some of the plots in list of plots.
 * @brief Board::plotClickAction
 * @param index index of selected plot
 */
void Board::plotClickAction(const QModelIndex & index)
{
    if(QApplication::mouseButtons() == Qt::LeftButton)
    {
        qDebug("Left click");
        ui.plotCanvas->setPlotActive(index.row());
    }
}

/**
 * This function is triggered when user right clicks on some of the plots in list of plots and presents context menu.
 * @brief Board::showPlotContextMenu
 * @param p point whre used clicked
 */
void Board::showPlotContextMenu(QPoint p)
{
    qDebug("Context menu");
    QMenu contextMenu;
    QAction interpolate("Toggle interpolation", this);
    if(ui.plotCanvas->isAnalog(ui.plotNamesColumn->currentIndex().row()))
    {
        contextMenu.addAction(&interpolate);
        connect(&interpolate, SIGNAL(triggered(bool)), this, SLOT(setInterpolateOnPlot()));
    }
    QAction remove("Remove", this);
    contextMenu.addAction(&remove);
    QAction setTrigger("Set trigger", this);
    contextMenu.addAction(&setTrigger);
    connect(&setTrigger, SIGNAL(triggered()), this, SLOT(openTriggerDialog()));
    contextMenu.exec(ui.plotNamesColumn->mapToGlobal(p));
}

/**
 * This function is triggered by user when he selects to set trigger on some signal.
 * @brief Board::openTriggerDialog
 */
void Board::openTriggerDialog()
{
    TriggerSetupDialog triggerDialog;
    int plotIndexRow = ui.plotNamesColumn->currentIndex().row();
    AbstractDevice* device = controller.getDeviceByPlot(ui.plotCanvas->getPlot(plotIndexRow));
    int kind;
    if(ui.plotCanvas->isAnalog(plotIndexRow))
    {
        kind = ANALOG;
    }
    else
    {
        kind = DIGITAL;
    }

    triggerDialog.assignTrigger(device, kind, controller.getDataIdByPlot(ui.plotCanvas->getPlot(plotIndexRow)));
}

/**
 * This helper function sets interpolation on selected plot.
 * @brief Board::setInterpolateOnPlot
 */
void Board::setInterpolateOnPlot()
{
    ui.plotCanvas->toggleInterpolate(ui.plotNamesColumn->currentIndex().row());
}

/**
 * This function is triggered after user creates new plot, it correctly creates plot and registers it to board.
 * @brief Board::createMeasurementPlot
 * @param type type of plot to create.
 * @return
 */
Plot* Board::createMeasurementPlot(int type)
{
    Plot * newPlot = new Plot(NULL, "S"+QString::number(ui.plotNamesColumn->count()));
    ui.plotCanvas->addPlot(newPlot);
    QListWidgetItem * plotTextButton = new QListWidgetItem(ui.plotNamesColumn);
    plotTextButton->setText(newPlot->getName());
    plotTextButton->setTextAlignment(Qt::AlignHCenter);
    plotTextButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui.scopeKnobs->setBoard(this);
    return newPlot;
}

/**
 * Presents ProbeAssignDialog to create plot and assign probe to this newly created plot.
 * @brief Board::addMeasurementPlot
 */
void Board::addMeasurementPlot()
{
    if(controller.hasDevice())
    {
        ProbeAssignDialog probeAssignDialog(this);
        probeAssignDialog.assignDialog(&controller, this);
        refreshPlotNames();
    }
    else
    {
        QMessageBox::warning(this, "No device", "You need to connect device first!");
    }
}

/**
 * Remove plot from list of plots.
 * @brief Board::removeMeasurementPlot
 */
void Board::removeMeasurementPlot()
{
    //TODO: reimplement to conform model view
    ui.plotNamesColumn->removeItemWidget(ui.plotNamesColumn->currentItem());
}

/**
 * Triggered when user selects plot in list of plots. Changes current selectted plot and changes knobs appropriate to selected plot.
 * @brief Board::changePageNames
 * @param current
 * @param previous
 */
void Board::changePageNames(QListWidgetItem * current, QListWidgetItem * previous)
{
    int currentWidget = ui.plotNamesColumn->row(current);
    if(controller.hasDevice() and currentWidget != -1)
    {
        ui.scopeKnobs->setCurrentDevice(controller.getDeviceByPlot(ui.plotCanvas->getPlot(ui.plotNamesColumn->row(current))));
        ui.scopeKnobs->setCurrentPlot(ui.plotCanvas->getPlot(ui.plotNamesColumn->row(current)));
    }
    //plotNamesColumn->setCurrentRow(currentWidget); //probably unnecessary
    //ui.plotSettings->setCurrentIndex(currentWidget);
}

/**
 * Refreshes list of plot nasmes in list of plots.
 * @brief Board::refreshPlotNames
 */
void Board::refreshPlotNames()
{
    QModelIndex idxs = ui.plotNamesColumn->currentIndex();
    ui.plotNamesColumn->clear();
    ui.plotCanvas->clear();
    PlotTreeItem * treeItem = static_cast<PlotTreeItem*>(ui.treeWidget->selectedItem());
    if(treeItem == NULL)
        return;
    for(int i = 0; i < treeItem->plotData().size(); ++i)
    {
        //qDebug() << treeItem->plotData().at(i)->getName();
        ui.plotNamesColumn->addItem(treeItem->plotData().at(i)->getName()+" ["+QString::number(treeItem->plotData().at(i)->getBitwidth())+"]");
        ui.plotCanvas->addPlot(new Plot(treeItem->plotData().at(i), treeItem->plotData().at(i)->getName()));
    }
    ui.plotCanvas->zoomOut();
    ui.plotCanvas->zoomIn();
    treeModel->setCurrentTop(treeItem);
    repaint();
    if(idxs.isValid())
    {
        ui.plotNamesColumn->setCurrentIndex(idxs);
    }
}

/**
 * Loat plot hierarchy from VCD file.
 * @brief Board::loadFromFile
 * @param file file to load VCD from.
 */
void Board::loadFromFile(QFile * file)
{
    if(!file->open(QIODevice::ReadOnly |
                   QIODevice::Text))
    {
        QFileInfo fi(file->fileName());
        throw IOException("Could not open file: "+fi.fileName());
    }
    
    FileSplitter splitter = FileSplitter(file);
    treeModel->loadFromFile(splitter);
    refreshPlotNames();
}

/**
 * Saves current plot hierarchy to VCD file.
 * @brief Board::saveToFile
 * @param file file to save to.
 */
void Board::saveToFile(QFile *file)
{
    if(!file->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QFileInfo fi(file->fileName());
        throw IOException("Could not open file: "+fi.fileName());
    }
    //treeModel->setTimeScale()
    treeModel->setComment(file->fileName());
    treeModel->saveToFile(file);
}

/**
 * Set starting time of viewport.
 * @brief Board::setFrom
 * @param from
 */
void Board::setFrom(uint64_t from)
{
    ui.plotCanvas->setFrom(from);
}

/**
 *  Updates scrollbar position according to current from and to.
 * @brief Board::updateFromTo
 */
void Board::updateFromTo()
{
    uint64_t maxtime = ui.plotCanvas->getMaxTime();
    if(maxtime == 0)
    {
        ui.horizontalScrollBar->hide();
    }
    else
    {
        //if(maxtime > E20)
           // scrollbarDivision = E18;
        if(maxtime > E19)
            scrollbarDivision = E17;
        else if(maxtime > E18)
            scrollbarDivision = E16;
        else if(maxtime > E17)
            scrollbarDivision = E15;
        else if(maxtime > E16)
            scrollbarDivision = E14;
        else if(maxtime > E15)
            scrollbarDivision = E13;
        else if(maxtime > E14)
            scrollbarDivision = E12;
        else if(maxtime > E13)
            scrollbarDivision = E11;
        else if(maxtime > E12)
            scrollbarDivision = E10;
        else if(maxtime > E11)
            scrollbarDivision = E9;
        else if(maxtime > E10)
            scrollbarDivision = E8;
        else if(maxtime > E9)
            scrollbarDivision = E7;
        else if(maxtime > E8)
            scrollbarDivision = E6;
        else if(maxtime > E7)
            scrollbarDivision = E5;
        else if(maxtime > E6)
            scrollbarDivision = E4;
        else if(maxtime > E5)
            scrollbarDivision = E3;
        else if(maxtime > E4)
            scrollbarDivision = E2;
        else
            scrollbarDivision = 1;
        //qDebug() << "Maxtime: " << maxtime << "from: " << ui.commonKnobs->getFrom();
        ui.horizontalScrollBar->blockSignals(true);
        ui.horizontalScrollBar->setRange(0, maxtime/scrollbarDivision);
        ui.horizontalScrollBar->setValue(ui.commonKnobs->getFrom()/scrollbarDivision);
        ui.horizontalScrollBar->setSingleStep(ui.horizontalScrollBar->maximum()/10);
        ui.horizontalScrollBar->show();
        ui.horizontalScrollBar->blockSignals(false);
    }
}
/**
 * Triggered when users clicks Arm.
 * @brief Board::startCapture
 * @param continous
 */
void Board::startCapture(bool continous)
{
    this->controller.startCapture(continous, ui.plotCanvas->getPlots());
}

/**
 * Triggered when users select connec device and presets ConnectDevice dialog.
 * @brief Board::connectDevice
 */
void Board::connectDevice()
{
    ConnectDevice connectDevice;
    connectDevice.connectDevice(&controller);
}
