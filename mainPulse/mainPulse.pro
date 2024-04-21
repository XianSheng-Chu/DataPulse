QT       += core gui
QT       += sql
QT       += concurrent
QT += network
QT += charts
DEFINES += QTCSSH_LIBRARY
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    PulseUI/pchart.cpp \
    PulseUI/pchartview.cpp \
    PulseUI/pcreateconnectdialog.cpp \
    PulseUI/pcreateruledialog.cpp \
    PulseUI/pjobwindow.cpp \
    PulseUI/pstandarditemmodel.cpp \
    PulseUI/pstystyleditemdelegate.cpp \
    TaskSchedule/jobdbrunnable.cpp \
    TaskSchedule/jobrunnable.cpp \
    TaskSchedule/jobthreadfactory.cpp \
    TaskSchedule/monitoringcyclethread.cpp \
    TaskSchedule/pdatabasesoure.cpp \
    main.cpp \
    mainwindow.cpp \
    PulseUI/ptreeview.cpp \


HEADERS += \
    PulseUI/pchart.h \
    PulseUI/pchartview.h \
    PulseUI/pcreateconnectdialog.h \
    PulseUI/pcreateruledialog.h \
    PulseUI/pjobwindow.h \
    PulseUI/pstandarditemmodel.h \
    PulseUI/pstystyleditemdelegate.h \
    TaskSchedule/jobdbrunnable.h \
    TaskSchedule/jobrunnable.h \
    TaskSchedule/jobthreadfactory.h \
    TaskSchedule/monitoringcyclethread.h \
    TaskSchedule/pdatabasesoure.h \
    mainwindow.h \
    PulseUI/ptreeview.h \



    mainwindow.h

FORMS += \
    PulseUI/pcreateconnectdialog.ui \
    PulseUI/pcreateruledialog.ui \
    PulseUI/pjobwindow.ui \
    mainwindow.ui \

QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += -Od
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc


