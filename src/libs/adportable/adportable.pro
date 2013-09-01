#-------------------------------------------------
#
# Project created by QtCreator 2010-07-04T06:28:41
#
#-------------------------------------------------

QT       -= core gui

TARGET = adportable
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatzstaticlib.pri)
include(../../boost.pri)

SOURCES += adportable.cpp \
    configloader.cpp \
    configuration.cpp \
    constants.cpp \
    ConvertUTF.c \
    date_string.cpp \
    debug.cpp \
    fft.cpp \
    polfit.cpp \
    portable_binary_oarchive.cpp \
    portable_binary_iarchive.cpp \
    posix_path.cpp \
    serialport.cpp \
    spectrum_processor.cpp \
    string.cpp \
    utf.cpp \
    lifecycleframe.cpp \
    timer.cpp \
    profile.cpp

win32 {
   SOURCES += protocollifecycle.cpp
}

HEADERS += adportable.hpp \
    array_wrapper.hpp \
    binary_search.hpp \
    configloader.hpp \
    configuration.hpp \
    constants.hpp \
    ConvertUTF.h \
    date_string.hpp \
    debug.hpp \
    differential.hpp \
    fft.hpp \
    float.hpp \
    is_equal.hpp \
    moment.hpp \
    polfit.hpp \
    posix_path.hpp \
    protocollifecycle.hpp \
    safearray.hpp \
    serialport.hpp \
    serializer.hpp \
    spectrum_processor.hpp \
    string.hpp \
    utf.hpp \
    lifecycleframe.hpp \
    timer.hpp \
    profile.hpp

