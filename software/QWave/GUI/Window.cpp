//
//   Window.cpp
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


#include "Window.h"
#include <QDate>
#include <QDir>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include "../Exceptions/IOException.h"
#include "ProgressBarDialog.h"

Window::Window() : currentMeasurementId(0)
{   
    createMenuActions();
    createMenus();
    //closeTabButton = new QPushButton("Close");
    //connect(closeTabButton, SIGNAL(clicked()), this, SLOT(closeTab()));
    createTabbar();
    connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    mainWidget = new QWidget(this);
    layout = new QVBoxLayout();
    layout->addWidget(tabBar);
    mainWidget->setLayout(layout);
    setCentralWidget(mainWidget);
    setWindowTitle("QWave");
    statusBarLabel = new QLabel("Ready..");
    statusBar()->addWidget(statusBarLabel);
}

Window::~Window()
{
    //delete(closeTabButton);
    //menu items
    delete(fileMenu);
    //delete(editMenu);
    delete(measurementMenu);
    delete(helpMenu);
    //submenu items for file
    delete(newTabAction);
    //delete(closeTabAction);
    delete(openAction);
    delete(saveAction);
    delete(saveAsAction);
    delete(quitAction);
    //submenu items for measurement
    delete(addMeasurementAction);
    //delete(removeMeasurementAction);
    //delete(chooseMeasurementsAction);
    //submenu items for edit
    //delete(preferencesAction);
    //delete(measurementPreferencesAction);
    //submenu items for help menu
    delete(aboutAction);
    delete(helpAction);
    //active board
    for(int i = 0; i < tabBar->count(); ++i)
    {
        Board *  boardRemoved = static_cast<Board*>(tabBar->widget(i));
        tabBar->removeTab(i);
        qDebug() << "window: deleting board";
        delete(boardRemoved);
    }
    delete(tabBar);
    delete(statusBarLabel);
    delete(layout);
    delete(mainWidget);
}

/**
 * @brief Window::createMenuActions creates menu actions.
 */
void Window::createMenuActions()
{
    newTabAction = new QAction(tr("&New tab"), this);
    newTabAction->setShortcut(tr("Ctrl+N"));
    newTabAction->setStatusTip(tr("Open a new tab"));
    connect(newTabAction, SIGNAL(triggered()), this, SLOT(newTab()));
    //openAction initialization
    openAction = new QAction(tr("&Open"), this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setStatusTip(tr("Open file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    //saveAction initialization
    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save file"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));
    //saveAsAction initialization
    saveAsAction = new QAction(tr("&Save As.."), this);
    saveAsAction->setStatusTip(tr("Save file as.."));
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAsFile()));
    //quitAction initialization
    quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    quitAction->setStatusTip(tr("Quit Application"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    //measurement menu actions
    //addMeasurementAction initialization
    addMeasurementAction = new QAction(tr("&Add measurement plot"), this);
    addMeasurementAction->setShortcut(tr("Ctrl+M"));
    addMeasurementAction->setStatusTip(tr("Add plot of signal from selected probe"));
    connect(addMeasurementAction, SIGNAL(triggered()), this, SLOT(addMeasurement()));
    //removeMeasurementAction initialization
    //removeMeasurementAction = new QAction(tr("&Remove measurement plot"), this);
    //removeMeasurementAction->setShortcut(tr("Ctrl+K"));
    //removeMeasurementAction->setStatusTip(tr("Remove selected measurement plot"));
    //connect(addMeasurementAction, SIGNAL(triggered()), this, SLOT(removeMeasurement()));
    //removeMeasurementAction initialization
    //chooseMeasurementsAction = new QAction(tr("&Choose displayed measurements"), this);
    //chooseMeasurementsAction->setShortcut(tr("Ctrl+K"));
    //chooseMeasurementsAction->setStatusTip(tr("Choose probes which signal is displayed in plot"));
    //connect(addMeasurementAction, SIGNAL(triggered()), this, SLOT(chooseMeasurements()));
    connectDeviceAction = new QAction(tr("&Connect device..."), this);
    connectDeviceAction->setShortcut(tr("Ctrl+L"));
    connectDeviceAction->setStatusTip(tr("Connect and initialize capture device"));
    connect(connectDeviceAction, SIGNAL(triggered()), this, SLOT(connectDevice()));
    //edit menu actions
    //preferences action
    preferencesAction = new QAction(tr("&Preferences"), this);
    preferencesAction->setShortcut(tr("Ctrl+P"));
    preferencesAction->setStatusTip(tr("Preferences of application"));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(preferences()));
    //mesurementPreferences action
    measurementPreferencesAction = new QAction(tr("&Measurement preferences"), this);
    measurementPreferencesAction->setShortcut(tr("Ctrl+J"));
    measurementPreferencesAction->setStatusTip(tr("Preferences of measurement plot"));
    connect(measurementPreferencesAction, SIGNAL(triggered()), this, SLOT(measurementPreferences()));
    //help menu actions
    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("About this application"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    helpAction = new QAction(tr("&Help"), this);
    aboutAction->setStatusTip(tr("QWave help"));
    connect(helpAction, SIGNAL(triggered()), this, SLOT(help()));
    
}

/**
 * @brief Window::createMenus create menus of menubar.
 */
void Window::createMenus()
{
    //file menu initialization
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newTabAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addAction(quitAction);
    //edit menu initialization
    //editMenu = menuBar()->addMenu(tr("&Edit"));
    //editMenu->addAction(preferencesAction);
    //editMenu->addAction(measurementPreferencesAction);
    //measurement menu initialization
    measurementMenu = menuBar()->addMenu(tr("&Measurement"));
    measurementMenu->addAction(addMeasurementAction);
    //measurementMenu->addAction(removeMeasurementAction);
    deviceMenu = menuBar()->addMenu(tr("&Device"));
    deviceMenu->addAction(connectDeviceAction);
    //measurementMenu->addAction(chooseMeasurementsAction);
    //Help menu initializations
    helpMenu = menuBar()->addMenu(tr("&Help"));
    //helpMenu->addAction(helpAction);
    helpMenu->addAction(aboutAction);
}

/**
 * @brief Window::getDefaultTabLabel generates default tab label.
 * @return
 */
QString Window::getDefaultTabLabel()
{
    QString label(QDateTime::currentDateTime().toString(Qt::ISODate));
    label = "Unnamed measurement "+label;
    return label;
}

/**
 * @brief Window::createTabbar creates tabbar.
 */
void Window::createTabbar()
{
    tabBar = new QTabWidget(this);
    tabBar->setMinimumSize(1000, 640);
    tabBar->setTabShape(QTabWidget::Rounded);
    tabBar->setDocumentMode(true);
    tabBar->setTabsClosable(true);
    //tabBar->setCornerWidget(closeTabButton);
    newTab();
}

/**
 * @brief Window::newTab triggered when users open new tab to create new instance of Board and add it to menubar.
 */
void Window::newTab()
{
    QString tabLabel = getDefaultTabLabel();
    Board * board = new Board(tabLabel, tabBar);
    tabBar->addTab(board, tabLabel);
    board->connectWindow(this);
    board->initHierarchy();
}

/**
 * @brief Window::openFile presents open file dialog and tryes to open selected file.
 */
void Window::openFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open file"),
                                                      QDir::currentPath());
    QString name;
    int index = tabBar->currentIndex();
    for(int i = 0; i < files.size(); ++i)
    {
        try
        {
            QFile * file = new QFile(files.at(i));
            QFileInfo fi(files.at(i));
            name = fi.fileName();
            name = name.split('.').at(0);
            Board * newBoard = new Board(name, tabBar);
            QFuture<void> future = QtConcurrent::run(newBoard, &Board::loadFromFile, file);
            ProgressBarDialog dialog(future, "Loading "+name, this);
            dialog.wait();
            index = tabBar->addTab(newBoard, name);
            newBoard->connectWindow(this);
            delete(file);
            setStatusBarText("Measurement/simulation "+name+" opened sucessfully.");
        }
        catch (IOException e)
        {
            QMessageBox::critical(this, "File error",
                                  e.getMessage());
            setStatusBarText("Measurement/simulation "+name+" could not be opened.");
        }
        
    }
    tabBar->setCurrentIndex(index);
}

/**
 * @brief Window::saveFile save to previously used file or calls saveAsFile().
 */
void Window::saveFile()
{
    if("" == ((static_cast<Board*>(tabBar->currentWidget()))->getFilePath()))
    {
        saveAsFile();
    }
    else
    {
        try
        {
            QFile * qfile = new QFile((static_cast<Board*>(tabBar->currentWidget()))->getFilePath());
            QFuture<void> future = QtConcurrent::run(static_cast<Board*>(tabBar->currentWidget()), &Board::saveToFile, qfile);
            ProgressBarDialog dialog(future, "Saving "+qfile->fileName(), this);
            dialog.wait();
            //(static_cast<Board*>(tabBar->currentWidget()))->saveToFile(qfile);
        }
        catch (IOException e)
        {
            QMessageBox::critical(this, "File error",
                                  e.getMessage());
        }
    }
}

/**
 * @brief Window::saveAsFile save measurement to selected file.
 */
void Window::saveAsFile()
{
    QString file = QFileDialog::getSaveFileName();
    try
    {
        QFile * qfile = new QFile(file);
        QFuture<void> future = QtConcurrent::run(static_cast<Board*>(tabBar->currentWidget()), &Board::saveToFile, qfile);
        ProgressBarDialog dialog(future, "Saving "+qfile->fileName(), this);
        dialog.wait();
        (static_cast<Board*>(tabBar->currentWidget()))->setFilePath(file);
        //(static_cast<Board*>(tabBar->currentWidget()))->saveToFile(qfile);
    }
    catch (IOException e)
    {
        QMessageBox::critical(this, "File error",
                              e.getMessage());
    }
}

void Window::preferences()
{
    
}

/**
 * @brief Window::addMeasurement add measurement plot to canvas.
 */
void Window::addMeasurement()
{
    static_cast<Board*>(tabBar->currentWidget())->addMeasurementPlot();
}

/**
 * @brief Window::removeMeasurement removes measurement plot from canvas.
 */
void Window::removeMeasurement()
{
    static_cast<Board*>(tabBar->currentWidget())->removeMeasurementPlot();
}

/**
 * @brief Window::closeTab closes selected tab.
 * @param index
 */
void Window::closeTab(int index)
{
    Board *  boardRemoved = static_cast<Board*>(tabBar->widget(index));
    tabBar->removeTab(index);
    delete(boardRemoved);
}

void Window::handleDisconnectedDeice()
{
    setStatusBarText("Probing device disconnected!");
}

void Window::handleConnectedDevice()
{
    setStatusBarText("New probing device available");
}

/**
 * Shows about message.
 */
void Window::about()
{
    QMessageBox::about(this, "About QWave", 
                       "QWave.\n"
                       "Oscilloscope and logic analyzer "
                       "for use with compatibile probes. \n"
                       "This application is written by Bruno Kremel");
}

void Window::help()
{
    
}

void Window::measurementPreferences()
{
    
}

void Window::chooseMeasurements()
{
    
}

/**
 * @brief Window::connectDevice triggers connect device in Board.
 */
void Window::connectDevice()
{
    static_cast<Board*>(tabBar->currentWidget())->connectDevice();
}

/**
 * @brief Window::setStatusBarText sets status bar text of window.
 * @param text
 */
void Window::setStatusBarText(QString text)
{
    statusBarLabel->setText(text);
}
