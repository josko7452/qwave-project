#-------------------------------------------------
#
# Project created by QtCreator 2012-10-09T22:16:50
#
#-------------------------------------------------

QT       += core gui
INCLUDEPATH += /opt/local/include
LIBS += -L/opt/local/lib -lftdi
TARGET = QWave
TEMPLATE = app
DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += \
    main.cpp \
    GUI/Window.cpp \
    GUI/PlotTreeWidget.cpp \
    GUI/PlotCanvas.cpp \
    GUI/Plot.cpp \
    GUI/Board.cpp \
    Datamodel/PlotTreeModel.cpp \
    Datamodel/PlotTreeItem.cpp \
    Datamodel/PlotData.cpp \
    GUI/CommonKnobs/CommonKnobs.cpp \
    GUI/CommonKnobs/TimeControl.cpp \
    GUI/CommonKnobs/ScaleControl.cpp \
    GUI/CommonKnobs/TimeSpinBox.cpp \
    Datamodel/FileSplitter.cpp \
    GUI/ProgressBarDialog.cpp \
    Device/DummyDevice.cpp \
    Device/CaptureController.cpp \
    GUI/ProbeAssignDialog.cpp \
    GUI/TriggerSetupDialog.cpp \
    GUI/KnobsWidget.cpp \
    GUI/ConnectDevice.cpp \
    Device/ft245sync.cpp \
    Device/Ft245Device.cpp \
    MyApplication.cpp

HEADERS  += \
    GUI/Window.h \
    GUI/PlotTreeWidget.h \
    GUI/PlotCanvas.h \
    GUI/Plot.h \
    GUI/Board.h \
    Exceptions/IOException.h \
    Exceptions/Exception.h \
    Device/DeviceController.h \
    Datamodel/PlotTreeModel.h \
    Datamodel/PlotTreeItem.h \
    Datamodel/PlotData.h \
    Datamodel/Constants.h \
    GUI/CommonKnobs/CommonKnobs.h \
    GUI/CommonKnobs/TimeControl.h \
    GUI/CommonKnobs/ScaleControl.h \
    GUI/CommonKnobs/TimeSpinBox.h \
    Datamodel/FileSplitter.h \
    GUI/ProgressBarDialog.h \
    Device/AbstractDevice.h \
    Device/DummyDevice.h \
    Device/Ft245Device.h \
    Device/Capabilities.h \
    Exceptions/DeviceException.h \
    Device/CaptureController.h \
    GUI/ProbeAssignDialog.h \
    GUI/TriggerSetupDialog.h \
    GUI/KnobsWidget.h \
    GUI/ConnectDevice.h \
    Device/ft245sync.h \
    MyApplication.h

FORMS    += mainwindow.ui \
    GUI/Board.ui \
    GUI/CommonKnobs/TimeControl.ui \
    GUI/CommonKnobs/CommonKnobs.ui \
    GUI/CommonKnobs/ScaleControl.ui \
    GUI/PlotTreeWidget.ui \
    GUI/ProgressBarDialog.ui \
    GUI/ProbeAssignDialog.ui \
    GUI/TriggerSetupDialog.ui \
    GUI/KnobsWidget.ui \
    GUI/ConnectDevice.ui
