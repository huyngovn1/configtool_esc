QT += core gui widgets serialport
CONFIG += c++17
TEMPLATE = app
TARGET = BasicEscConfig
INCLUDEPATH += src
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/backend/escsession.cpp \
    src/model/settingsmodel.cpp \
    src/protocol/BF_ROOTLOADER.cpp \
    src/protocol/fourwayif.cpp
HEADERS += src/mainwindow.h \
    src/backend/escsession.h \
    src/backend/serialframes.h \
    src/backend/memoryaddress.h \
    src/model/settingsmodel.h \
    src/protocol/BF_ROOTLOADER.h \
    src/protocol/fourwayif.h
FORMS += src/mainwindow.ui
msvc:QMAKE_CXXFLAGS += /utf-8
