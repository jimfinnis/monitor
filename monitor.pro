#-------------------------------------------------
#
# Project created by QtCreator 2013-02-14T12:15:37
#
#-------------------------------------------------

QT       += core gui network
CONFIG  += debug
#LIBS += -lrt

TARGET = monitor
TEMPLATE = app

SOURCES += main.cpp\
    app.cpp\
    window.cpp\
    datamgr.cpp \
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
    widgets/momentary.cpp \
    widgets/slider.cpp \
    widgets/map.cpp

HEADERS  += app.h \
    window.h \
    datamgr.h \
    datarenderer.h \
    qcommandline.h\
    tokens.h \
    config.h \
    tokeniser.h \
    udp.h \
    doubletime.h\
    expr.h \
    nudgeable.h\
    keyhandler.h\
    etokens.h \
    widgets/buttondraw.h \
    widgets/statusBlock.h \
    widgets/gauge.h \
    widgets/number.h \
    widgets/compass.h\
    widgets/compassPanel.h\
    widgets/graph.h \
    widgets/status.h \
    widgets/switch.h \
    widgets/momentary.h \
    widgets/slider.h \
    widgets/map.h

//FORMS    += mainwindow.ui
RESOURCES += res.qrc
INCLUDEPATH += /usr/include/marble

LIBS += -L/usr/local/lib -lmarblewidget
