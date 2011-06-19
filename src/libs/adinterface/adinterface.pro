#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T06:44:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = adinterface
TEMPLATE = lib
CONFIG += staticlib
include(../../qtplatz_library.pri)
include(../../boost.pri)
include(adinterface_dependencies.pri)

IDLFILES += \
    brokerevent.idl \
    controlmethod.idl \
    controlserver.idl \
    eventlog.idl \
    global_constants.idl \
    instrument.idl \
    loghandler.idl \
    receiver.idl \
    samplebroker.idl \
    signalobserver.idl \
    broker.idl

SOURCES += interface.cpp \
        eventlog_helper.cpp

HEADERS += interface.hpp \
        eventlog_helper.hpp

PRE_TARGETDEPS += eventlogC.cpp

TAO_IDL = tao_idl

tao_idlC.name = TAO_IDL_C ${QMAKE_FILE_IN}
tao_idlC.input = IDLFILES
tao_idlC.output = ${QMAKE_FILE_BASE}C.cpp
tao_idlC.variable_out = GENERATED_FILES
tao_idlC.depends = ${QMAKE_FILE_IN}
tao_idlC.commands = $${TAO_IDL} -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h -I$${TAO_ROOT} -I$${PWD} -I. ${QMAKE_FILE_IN}
tao_idlC.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += tao_idlC
tao_idlC.variable_out = SOURCES

## following is the workaournd for qmake, which does not support multiple output on qake_extra_compiler

tao_idlS.input = IDLFILES
tao_idlS.output = ${QMAKE_FILE_BASE}S.cpp
tao_idlS.variable_out = GENERATED_FILES
tao_idlS.depends = ${QMAKE_FILE_IN}
tao_idlS.commands = tao_idl -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h -I$${TAO_ROOT} -I$${PWD} -I. ${QMAKE_FILE_IN}
tao_idlS.name = TAO_IDL_S ${QMAKE_FILE_IN}
tao_idlS.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += tao_idlS
tao_idlS.variable_out = SOURCES

GENERATED_FILES += \
        brokerC.cpp \
        brokerS.cpp \
        brokereventC.cpp \
        brokereventS.cpp \
    controlserverC.cpp \
	controlserverS.cpp \
	controlmethodC.cpp \ 
	controlmethodS.cpp \ 
	global_constantsC.cpp \
	global_constantsS.cpp \
	receiverC.cpp \
	receiverS.cpp \
	samplebrokerC.cpp \
        samplebrokerS.cpp \
    signalobserverC.cpp \
    signalobserverS.cpp \
    eventlogC.h


OTHER_FILES += \
    broker.idl \
    brokerevent.idl \
    controlmethod.idl \
    controlserver.idl \
    eventlog.idl \
    global_constants.idl \
    instrument.idl \
    loghandler.idl \
    receiver.idl \
    samplebroker.idl \
    signalobserver.idl \
    adinterface_dependencies.pri
