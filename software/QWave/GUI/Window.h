//
//   Window.h
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


#ifndef QWave_Window_h
#define QWave_Window_h

#include <QMainWindow>
#include <QMessageBox>
#include <QStatusBar>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QWidget>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QFile>
#include <Qt>
#include <QQueue>
#include "Board.h"

/**
 * @brief The Window class represents Window of application caring out menubar
 *  and making placeholder for tabbar with tabs made of Board(s).
 */
class Window : public QMainWindow
{
    Q_OBJECT
private:
    //push buttons
    QPushButton * closeTabButton;
    //menu items
    QMenu * fileMenu;
    QMenu * deviceMenu;
    QMenu * measurementMenu;
    QMenu * helpMenu;
    //submenu items for file
    QAction * newTabAction;
    QAction * closeTabAction;
    QAction * openAction;
    QAction * saveAction;
    QAction * saveAsAction;
    QAction * quitAction;
    //submenu items for measurement
    QAction * addMeasurementAction;
    //QAction * removeMeasurementAction;
    QAction * chooseMeasurementsAction;
    //submenu items for edit
    QAction * preferencesAction;
    QAction * measurementPreferencesAction;
    //
    QAction * connectDeviceAction;
    //submenu items for help menu
    QAction * aboutAction;
    QAction * helpAction;
    //active board
    QTabWidget * tabBar;
    QWidget * mainWidget;
    int currentMeasurementId;
    
    QLabel * statusBarLabel;
    
    QLayout * layout;
    //auxiliary create functions
    void createMenuActions();
    void createMenus();
    void createTabbar();
    QString getDefaultTabLabel();
    QFrame * newHorizontalSeparator();
    QLabel * newLabel(QString & label);
    //slots for actions
private slots:
    void newTab();
    void openFile();
    void saveFile();
    void saveAsFile();
    void preferences();
    void about();
    void help();
    void measurementPreferences();
    void addMeasurement();
    void removeMeasurement();
    void chooseMeasurements();
    void closeTab(int index);
    void connectDevice();
    
public slots:
    void handleDisconnectedDeice();
    void handleConnectedDevice();
    void setStatusBarText(QString);
    
signals:
    void deviceDiscoveryInvoked();
    
public:
    Window();
    ~Window();
};


#endif
