include(../common.pri)
TARGET = ut_devicelock

INCLUDEPATH += $$DEVICELOCKSRCDIR
QMAKE_CXXFLAGS += `pkg-config --cflags-only-I qmsystem2-qt5`

QT += dbus

LIBS += -lrt

# unit test and unit
SOURCES += \
    ut_devicelock.cpp \
    $$DEVICELOCKSRCDIR/devicelock.cpp \
    $$STUBSDIR/stubbase.cpp

# unit test and unit
HEADERS += \
    ut_devicelock.h \
    $$DEVICELOCKSRCDIR/devicelock.h \
    /usr/include/qmsystem2-qt5/qmlocks.h \
    /usr/include/qmsystem2-qt5/qmactivity.h \
    /usr/include/qmsystem2-qt5/qmdisplaystate.h
