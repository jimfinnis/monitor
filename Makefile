#############################################################################
# Makefile for building: monitor
# Generated by qmake (2.01a) (Qt 4.6.2) on: Wed Mar 20 15:48:42 2013
# Project:  monitor.pro
# Template: app
# Command: /usr/bin/qmake-qt4 -unix -o Makefile monitor.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -g -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -g -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtNetwork -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I/usr/include/marble -I. -I.
LINK          = g++
LFLAGS        = 
LIBS          = $(SUBLIBS)  -L/usr/lib -L/usr/local/lib -lmarblewidget -lQtGui -lQtNetwork -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/bin/qmake-qt4
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cpp \
		mainwindow.cpp \
		datamgr.cpp \
		widgetmgr.cpp \
		tokens.cpp \
		config.cpp \
		tokeniser.cpp \
		udp.cpp \
		expr.cpp \
		doubletime.cpp \
		etokens.cpp \
		qcommandline.cpp \
		widgets/statusBlock.cpp \
		widgets/gauge.cpp \
		widgets/compass.cpp \
		widgets/compassPanel.cpp \
		widgets/number.cpp \
		widgets/graph.cpp \
		widgets/status.cpp \
		widgets/switch.cpp \
		widgets/map.cpp moc_mainwindow.cpp \
		moc_qcommandline.cpp \
		moc_udp.cpp \
		moc_statusBlock.cpp \
		moc_gauge.cpp \
		moc_number.cpp \
		moc_compass.cpp \
		moc_compassPanel.cpp \
		moc_graph.cpp \
		moc_status.cpp \
		moc_switch.cpp \
		moc_map.cpp
OBJECTS       = main.o \
		mainwindow.o \
		datamgr.o \
		widgetmgr.o \
		tokens.o \
		config.o \
		tokeniser.o \
		udp.o \
		expr.o \
		doubletime.o \
		etokens.o \
		qcommandline.o \
		statusBlock.o \
		gauge.o \
		compass.o \
		compassPanel.o \
		number.o \
		graph.o \
		status.o \
		switch.o \
		map.o \
		moc_mainwindow.o \
		moc_qcommandline.o \
		moc_udp.o \
		moc_statusBlock.o \
		moc_gauge.o \
		moc_number.o \
		moc_compass.o \
		moc_compassPanel.o \
		moc_graph.o \
		moc_status.o \
		moc_switch.o \
		moc_map.o
DIST          = /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		/usr/share/qt4/mkspecs/features/debug.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		/usr/share/qt4/mkspecs/features/include_source_dir.prf \
		monitor.pro
QMAKE_TARGET  = monitor
DESTDIR       = 
TARGET        = monitor

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET): ui_mainwindow.h $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: monitor.pro  /usr/share/qt4/mkspecs/linux-g++/qmake.conf /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		/usr/share/qt4/mkspecs/features/debug.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		/usr/share/qt4/mkspecs/features/include_source_dir.prf \
		/usr/lib/libQtGui.prl \
		/usr/lib/libQtNetwork.prl \
		/usr/lib/libQtCore.prl
	$(QMAKE) -unix -o Makefile monitor.pro
/usr/share/qt4/mkspecs/common/g++.conf:
/usr/share/qt4/mkspecs/common/unix.conf:
/usr/share/qt4/mkspecs/common/linux.conf:
/usr/share/qt4/mkspecs/qconfig.pri:
/usr/share/qt4/mkspecs/features/qt_functions.prf:
/usr/share/qt4/mkspecs/features/qt_config.prf:
/usr/share/qt4/mkspecs/features/exclusive_builds.prf:
/usr/share/qt4/mkspecs/features/default_pre.prf:
/usr/share/qt4/mkspecs/features/debug.prf:
/usr/share/qt4/mkspecs/features/default_post.prf:
/usr/share/qt4/mkspecs/features/warn_on.prf:
/usr/share/qt4/mkspecs/features/qt.prf:
/usr/share/qt4/mkspecs/features/unix/thread.prf:
/usr/share/qt4/mkspecs/features/moc.prf:
/usr/share/qt4/mkspecs/features/resources.prf:
/usr/share/qt4/mkspecs/features/uic.prf:
/usr/share/qt4/mkspecs/features/yacc.prf:
/usr/share/qt4/mkspecs/features/lex.prf:
/usr/share/qt4/mkspecs/features/include_source_dir.prf:
/usr/lib/libQtGui.prl:
/usr/lib/libQtNetwork.prl:
/usr/lib/libQtCore.prl:
qmake:  FORCE
	@$(QMAKE) -unix -o Makefile monitor.pro

dist: 
	@$(CHK_DIR_EXISTS) .tmp/monitor1.0.0 || $(MKDIR) .tmp/monitor1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/monitor1.0.0/ && $(COPY_FILE) --parents mainwindow.h datamgr.h datarenderer.h widgetmgr.h qcommandline.h tokens.h config.h tokeniser.h udp.h doubletime.h expr.h etokens.h widgets/statusBlock.h widgets/gauge.h widgets/number.h widgets/compass.h widgets/compassPanel.h widgets/graph.h widgets/status.h widgets/switch.h widgets/map.h .tmp/monitor1.0.0/ && $(COPY_FILE) --parents main.cpp mainwindow.cpp datamgr.cpp widgetmgr.cpp tokens.cpp config.cpp tokeniser.cpp udp.cpp expr.cpp doubletime.cpp etokens.cpp qcommandline.cpp widgets/statusBlock.cpp widgets/gauge.cpp widgets/compass.cpp widgets/compassPanel.cpp widgets/number.cpp widgets/graph.cpp widgets/status.cpp widgets/switch.cpp widgets/map.cpp .tmp/monitor1.0.0/ && $(COPY_FILE) --parents mainwindow.ui .tmp/monitor1.0.0/ && (cd `dirname .tmp/monitor1.0.0` && $(TAR) monitor1.0.0.tar monitor1.0.0 && $(COMPRESS) monitor1.0.0.tar) && $(MOVE) `dirname .tmp/monitor1.0.0`/monitor1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/monitor1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc_mainwindow.cpp moc_qcommandline.cpp moc_udp.cpp moc_statusBlock.cpp moc_gauge.cpp moc_number.cpp moc_compass.cpp moc_compassPanel.cpp moc_graph.cpp moc_status.cpp moc_switch.cpp moc_map.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_mainwindow.cpp moc_qcommandline.cpp moc_udp.cpp moc_statusBlock.cpp moc_gauge.cpp moc_number.cpp moc_compass.cpp moc_compassPanel.cpp moc_graph.cpp moc_status.cpp moc_switch.cpp moc_map.cpp
moc_mainwindow.cpp: udp.h \
		mainwindow.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) mainwindow.h -o moc_mainwindow.cpp

moc_qcommandline.cpp: qcommandline.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) qcommandline.h -o moc_qcommandline.cpp

moc_udp.cpp: udp.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) udp.h -o moc_udp.cpp

moc_statusBlock.cpp: widgets/statusBlock.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/statusBlock.h -o moc_statusBlock.cpp

moc_gauge.cpp: datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		widgets/gauge.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/gauge.h -o moc_gauge.cpp

moc_number.cpp: datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		widgets/number.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/number.h -o moc_number.cpp

moc_compass.cpp: datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		widgets/compass.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/compass.h -o moc_compass.cpp

moc_compassPanel.cpp: widgets/compass.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		widgets/compassPanel.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/compassPanel.h -o moc_compassPanel.cpp

moc_graph.cpp: datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		widgets/graph.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/graph.h -o moc_graph.cpp

moc_status.cpp: widgets/statusBlock.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		widgets/status.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/status.h -o moc_status.cpp

moc_switch.cpp: datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		widgets/switch.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/switch.h -o moc_switch.cpp

moc_map.cpp: datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		widgets/map.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) widgets/map.h -o moc_map.cpp

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all: ui_mainwindow.h
compiler_uic_clean:
	-$(DEL_FILE) ui_mainwindow.h
ui_mainwindow.h: mainwindow.ui
	/usr/bin/uic-qt4 mainwindow.ui -o ui_mainwindow.h

compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean compiler_uic_clean 

####### Compile

main.o: main.cpp mainwindow.h \
		udp.h \
		qcommandline.h \
		datamgr.h \
		exception.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

mainwindow.o: mainwindow.cpp mainwindow.h \
		udp.h \
		ui_mainwindow.h \
		qcommandline.h \
		datamgr.h \
		exception.h \
		widgetmgr.h \
		config.h \
		widgets/gauge.h \
		datarenderer.h \
		tokeniser.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o mainwindow.o mainwindow.cpp

datamgr.o: datamgr.cpp datamgr.h \
		exception.h \
		expr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o datamgr.o datamgr.cpp

widgetmgr.o: widgetmgr.cpp widgetmgr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o widgetmgr.o widgetmgr.cpp

tokens.o: tokens.cpp tokens.h \
		tokeniser.h \
		exception.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tokens.o tokens.cpp

config.o: config.cpp tokeniser.h \
		exception.h \
		tokens.h \
		config.h \
		datamgr.h \
		expr.h \
		widgets/gauge.h \
		datarenderer.h \
		widgets/graph.h \
		widgets/compass.h \
		widgets/status.h \
		widgets/statusBlock.h \
		widgets/number.h \
		widgets/map.h \
		widgets/switch.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o config.o config.cpp

tokeniser.o: tokeniser.cpp tokeniser.h \
		exception.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tokeniser.o tokeniser.cpp

udp.o: udp.cpp udp.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o udp.o udp.cpp

expr.o: expr.cpp expr.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		etokens.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o expr.o expr.cpp

doubletime.o: doubletime.cpp doubletime.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o doubletime.o doubletime.cpp

etokens.o: etokens.cpp etokens.h \
		tokeniser.h \
		exception.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o etokens.o etokens.cpp

qcommandline.o: qcommandline.cpp qcommandline.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o qcommandline.o qcommandline.cpp

statusBlock.o: widgets/statusBlock.cpp widgets/statusBlock.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o statusBlock.o widgets/statusBlock.cpp

gauge.o: widgets/gauge.cpp widgets/gauge.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		config.h \
		widgetmgr.h \
		tokens.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o gauge.o widgets/gauge.cpp

compass.o: widgets/compass.cpp widgets/compass.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		config.h \
		widgetmgr.h \
		tokens.h \
		widgets/compassPanel.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o compass.o widgets/compass.cpp

compassPanel.o: widgets/compassPanel.cpp widgets/compassPanel.h \
		widgets/compass.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o compassPanel.o widgets/compassPanel.cpp

number.o: widgets/number.cpp widgets/number.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		config.h \
		widgetmgr.h \
		tokens.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o number.o widgets/number.cpp

graph.o: widgets/graph.cpp widgets/graph.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		config.h \
		widgetmgr.h \
		tokens.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o graph.o widgets/graph.cpp

status.o: widgets/status.cpp config.h \
		datamgr.h \
		exception.h \
		widgetmgr.h \
		tokens.h \
		tokeniser.h \
		widgets/status.h \
		widgets/statusBlock.h \
		datarenderer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o status.o widgets/status.cpp

switch.o: widgets/switch.cpp widgets/switch.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		config.h \
		widgetmgr.h \
		tokens.h \
		doubletime.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o switch.o widgets/switch.cpp

map.o: widgets/map.cpp widgets/map.h \
		datarenderer.h \
		datamgr.h \
		exception.h \
		tokeniser.h \
		config.h \
		widgetmgr.h \
		tokens.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o map.o widgets/map.cpp

moc_mainwindow.o: moc_mainwindow.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_mainwindow.o moc_mainwindow.cpp

moc_qcommandline.o: moc_qcommandline.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_qcommandline.o moc_qcommandline.cpp

moc_udp.o: moc_udp.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_udp.o moc_udp.cpp

moc_statusBlock.o: moc_statusBlock.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_statusBlock.o moc_statusBlock.cpp

moc_gauge.o: moc_gauge.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_gauge.o moc_gauge.cpp

moc_number.o: moc_number.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_number.o moc_number.cpp

moc_compass.o: moc_compass.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_compass.o moc_compass.cpp

moc_compassPanel.o: moc_compassPanel.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_compassPanel.o moc_compassPanel.cpp

moc_graph.o: moc_graph.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_graph.o moc_graph.cpp

moc_status.o: moc_status.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_status.o moc_status.cpp

moc_switch.o: moc_switch.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_switch.o moc_switch.cpp

moc_map.o: moc_map.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_map.o moc_map.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

