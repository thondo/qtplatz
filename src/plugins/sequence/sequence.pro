#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:59:52
#
#-------------------------------------------------

QT       += xml

TARGET = sequence
TEMPLATE = lib
PROVIDER = ScienceLiaison
include(../../adiplugin.pri)
include(sequence_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

DEFINES += SEQUENCE_LIBRARY

SOURCES +=  sequenceplugin.cpp \
    sequenceeditor.cpp \
    sequenceeditorfactory.cpp \
    sequencemode.cpp \
    sequence.cpp \
    sequencemanager.cpp

HEADERS += sequence_global.h \
    sequenceplugin.h \
    sequenceeditor.h \
    sequenceeditorfactory.h \
    sequencemode.h \
    constants.h \
    sequence.h \
    sequencemanager.h

OTHER_FILES += sequence.pluginspec \
    sequence-mimetype.xml \
    sequence_dependencies.pri

RESOURCES += \
    sequence.qrc
