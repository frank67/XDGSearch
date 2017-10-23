# XDGSearch is a XAPIAN based file indexer and search tool.
#
#    Copyright (C) 2016  Franco Martelli
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.


#-------------------------------------------------
#
# Project created by QtCreator 2016-06-22T17:32:58
#
#-------------------------------------------------

QT      += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG   += c++11
QMAKE_CXXFLAGS *= $(shell dpkg-buildflags --get CXXFLAGS)

TARGET = xdgsearch
TEMPLATE = app
LIBS     += -lQt5Core \
            -lQt5Gui \
            -lQt5Widgets \
            -lxapian \
            -pthread

INCLUDEPATH += /usr/include/xapian

# The application version
VERSION = 0.15.1

# Defines the preprocessor macro to get the application version available in the application code
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += main.cpp\
    mainwindow.cpp \
    configuration.cpp \
    wizard.cpp \
    indexer.cpp \
    preferences.cpp \
    helpers.cpp

HEADERS  += mainwindow.h \
    configuration.h \
    wizard.h \
    indexer.h \
    preferences.h \
    helpers.h

FORMS    += mainwindow.ui \
    wizard.ui \
    preferences.ui \
    helpers.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    COPYING \
    icon/COPYING \
    icon/COPYING.LESSER \
    icon/README.md \
    README.md
