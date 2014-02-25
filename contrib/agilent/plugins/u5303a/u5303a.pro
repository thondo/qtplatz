# U5303A plugin qmake 
lessThan( QT_MAJOR_VERSION, 5 ): error("Qt5 or later version required")

QT += core svg printsupport widgets

PROVIDER = MS-Cheminformatics
DEFINES += U5303A_LIBRARY

include(../../agilentplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)

#INCLUDEPATH *= $$OUT_PWD/../../src/libs

# u5303 files

SOURCES += u5303aplugin.cpp

HEADERS += u5303aplugin.hpp \
        u5303a_global.hpp \
        u5303a_constants.hpp

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
#QTCREATOR_SOURCES = $$(QTC_SOURCE)

## set the QTC_BUILD environment variable to override the setting here
#IDE_BUILD_TREE = $$(QTC_BUILD)

QTC_PLUGIN_NAME = u5303
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######
