#-------------------------------------------------
#
# Project created by QtCreator 2014-02-15T13:47:49
#
#-------------------------------------------------

QT       += widgets svg printsupport

TARGET = adwidgets
TEMPLATE = lib

include(../../qtplatzlibrary.pri)
include(../../boost.pri)

LIBS += -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adprot) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(qtwrapper)

!win32 {
  LIBS += -lboost_filesystem -lboost_system 
}

DEFINES += ADWIDGETS_LIBRARY

SOURCES += adwidgets.cpp \
        targetingform.cpp \
        tableview.cpp \
        targetingwidget.cpp \
        targetingtable.cpp \
        targetingadducts.cpp \
        peptidewidget.cpp \
        peptidetable.cpp \
        delegatehelper.cpp \
        controlmethodtable.cpp \
        controlmethodwidget.cpp \
        centroidform.cpp \
        mspeaktable.cpp \
        msquantable.cpp \
    mscalibrateform.cpp \
    msreferencetable.cpp \
    mscalibratewidget.cpp \
    msreferencedialog.cpp \
    mscalibratesummarytable.cpp \
    mstoleranceform.cpp

HEADERS += adwidgets.hpp\
        adwidgets_global.hpp \
        targetingform.hpp \
        spin_t.hpp \
        tableview.hpp \
        targetingwidget.hpp \
        targetingtable.hpp \
        targetingadducts.hpp \
        peptidewidget.hpp \
        peptidetable.hpp \
        delegatehelper.hpp \
        controlmethodtable.hpp \
        controlmethodwidget.hpp \
        centroidform.hpp \
        mspeaktable.hpp \
        msquantable.hpp \
    mscalibrateform.hpp \
    msreferencetable.hpp \
    mscalibratewidget.hpp \
    msreferencedialog.hpp \
    mscalibratesummarytable.hpp \
    mstoleranceform.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    targetingform.ui \
    centroidform.ui \
    mscalibrateform.ui \
    msreferencedialog.ui \
    mstoleranceform.ui
