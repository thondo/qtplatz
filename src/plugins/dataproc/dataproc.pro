#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T18:58:15
#
#-------------------------------------------------

QT += core svg printsupport
greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets

PROVIDER = MS-Cheminformatics

include(../../qtplatzplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

LIBS += -l$$qtLibraryTarget(Core)
LIBS += -l$$qtLibraryTarget(adwplot) -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(acewrapper) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(chromatogr) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adorbmgr)

!win32 {
#  LIBS += -lTAO_Utils -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE
  LIBS += -lboost_date_time -lboost_system -lboost_filesystem \
          -lboost_serialization
}

LIBS += -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( xmlparser )

linux-*: LIBS += -lqwt -ldl

INCLUDEPATH *= $$OUT_PWD/../../libs
INCLUDEPATH *= $(QWT)/include
DEFINES += ANALYSIS_LIBRARY

SOURCES += \
    actionmanager.cpp \
    assign_masses.cpp \
    assign_peaks.cpp \
    chromatogramwnd.cpp \
    calibrate_masses.cpp \
    dataproc.cpp \
    dataproceditor.cpp \
    dataprocessor.cpp \
    dataprocessorfactory.cpp \
    dataprochandler.cpp \
    dataprocplugin.cpp \
    elementalcompwnd.cpp \
    ifileimpl.cpp \
    mscalibrationwnd.cpp \
    mscalibspectrawnd.cpp \
    msprocessingwnd.cpp \
    navigationdelegate.cpp \
    navigationwidget.cpp \
    navigationwidgetfactory.cpp \
    sessionmanager.cpp \
    mainwindow.cpp \
    mode.cpp \
    editorfactory.cpp \
    isequenceimpl.cpp

HEADERS += \
    dataproc_global.h \
    actionmanager.hpp \
    chromatogramwnd.hpp \
    dataproceditor.hpp \
    dataprocessor.hpp \
    dataprocessorfactory.hpp \
    dataprochandler.hpp \
    dataprocplugin.hpp \
    elementalcompwnd.hpp \
    ifileimpl.hpp \
    mscalibrationwnd.hpp \
    mscalibspectrawnd.hpp \
    msprocessingwnd.hpp \
    navigationdelegate.hpp \
    navigationwidget.hpp \
    navigationwidgetfactory.hpp \
    sessionmanager.hpp \
    mainwindow.hpp \
    mode.hpp \
    editorfactory.hpp \
    isequenceimpl.hpp \
    assign_masses.hpp \
    calibrate_masses.hpp \
    assign_peaks.hpp

OTHER_FILES += \
    dataproc.pluginspec \
    dataproc-mimetype.xml \
    dataproc.config.xml

PLUGINSPECS += dataproc-mimetype.xml

RESOURCES += \
    dataproc.qrc
