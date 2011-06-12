#-------------------------------------------------
#
# Project created by QtCreator 2010-08-11T12:03:35
#
#-------------------------------------------------

TARGET = servant
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)

include(servant_dependencies.pri)
include(../../libs/acewrapper/acewrapper_dependencies.pri)
LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)
include(../../libs/adinterface/adinterface_dependencies.pri)

LIBS += -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adinterface) -l$$qtLibraryTarget(acewrapper) \
    -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adplugin) \
    -l$$qtLibraryTarget(qtwrapper) -l$$qtLibraryTarget(adbroker)

!win32 {
  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
  LIBS *= -lboost_serialization -lboost_date_time
}

DEFINES += SERVANT_LIBRARY
INCLUDEPATH *= $$OUT_PWD/../../libs ../../servants

SOURCES += logger.cpp \
        orbservantmanager.cpp \
        outputwindow.cpp \
        servant.cpp \
    servantmode.cpp \
    servantplugin.cpp \
    servantpluginimpl.cpp \
    servantuimanager.cpp

HEADERS += servant_global.h \
        logger.hpp \
    orbservantmanager.hpp \
    outputwindow.hpp \
    servant.hpp \
    servantmode.hpp \
    servantplugin.hpp \
    servantpluginimpl.hpp \
    servantuimanager.hpp

OTHER_FILES += \
    servant.pluginspec \
    servant.config.xml

