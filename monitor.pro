#-------------------------------------------------
#
# Project created by QtCreator 2013-02-14T12:15:37
#
#-------------------------------------------------

QT       += core gui network
CONFIG  += debug

TARGET = monitor
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    datamgr.cpp \
    widgetmgr.cpp \
    tokens.cpp \
    config.cpp \
    tokeniser.cpp \
    udp.cpp \
    expr.cpp \
    doubletime.cpp\
    etokens.cpp \
    qcommandline.cpp\
    widgets/statusBlock.cpp \
    widgets/gauge.cpp \
    widgets/compass.cpp \
    widgets/compassPanel.cpp \
    widgets/number.cpp \
    widgets/graph.cpp \
    widgets/status.cpp \
    widgets/switch.cpp \
    widgets/map.cpp

HEADERS  += mainwindow.h \
    datamgr.h \
    datarenderer.h \
    widgetmgr.h \
    qcommandline.h\
    tokens.h \
    config.h \
    tokeniser.h \
    udp.h \
    doubletime.h\
    expr.h \
    etokens.h \
    widgets/statusBlock.h \
    widgets/gauge.h \
    widgets/number.h \
    widgets/compass.h\
    widgets/compassPanel.h\
    widgets/graph.h \
    widgets/status.h \
    widgets/switch.h \
    widgets/map.h

FORMS    += mainwindow.ui
INCLUDEPATH += /usr/include/marble

LIBS += -L/usr/local/lib -lmarblewidget
