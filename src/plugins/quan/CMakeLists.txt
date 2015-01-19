project( Quan )
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Script Network WebKit WebKitWidgets )
find_package( qwt )
find_package( Boost REQUIRED COMPONENTS system date_time regex thread filesystem wserialization serialization )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set(COMPAT_VERSION 3.2.81)
set(VERSION 3.1.4)

configure_file(
  "${PROJECT_SOURCE_DIR}/pluginspec.in"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pluginspec"  
  )

add_definitions(
  -DQUAN_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( quan_SOURCES
  quanplugin.cpp
  mainwindow.cpp
  quanmode.cpp
  quandocument.cpp
  panelswidget.cpp
  paneldata.cpp
  doubletabwidget.cpp
  datasequencewidget.cpp
  datasequencetree.cpp
  compoundswidget.cpp
  compoundstable.cpp
  quanconfigform.cpp
  quanconfigwidget.cpp
  quandatawriter.cpp
  quansampleprocessor.cpp
  processmethodwidget.cpp
  quanreportwidget.cpp
  quanquerywidget.cpp
  quanprocessor.cpp
  quanresulttable.cpp
  quanqueryform.cpp
  quanconnection.cpp
  quanquery.cpp
  quanplotwidget.cpp
  quanresultwnd.cpp
  quanresultwidget.cpp
  quancmpdwidget.cpp
  quanplotdata.cpp
  quanpublisher.cpp
  quansvgplot.cpp
  quanplot.cpp
  quanfactory.cpp
  quaneditor.cpp
  )

set( quan_HEADERS
  quanplugin.hpp
  quan_global.hpp
  quanconstants.hpp
  mainwindow.hpp
  quanmode.hpp
  quandocument.hpp
  panelswidget.hpp
  paneldata.hpp
  doubletabwidget.hpp
  datasequencewidget.hpp
  datasequencetree.hpp
  compoundswidget.hpp
  compoundstable.hpp
  quanconfigform.hpp
  quanconfigwidget.hpp
  quandatawriter.hpp
  quansampleprocessor.hpp
  processmethodwidget.hpp
  quanreportwidget.hpp
  quanquerywidget.hpp
  quanprocessor.hpp
  quanresulttable.hpp
  quanqueryform.hpp
  quanconnection.hpp
  quanquery.hpp
  quanplotwidget.hpp
  quanresultwidget.hpp
  quanresultwnd.hpp
  quancmpdwidget.hpp
  quanplotdata.hpp
  quanpublisher.hpp
  quanprogress.hpp
  quansvgplot.hpp
  quanplot.hpp
  quanfactory.hpp
  quaneditor.hpp
  )

set( quan_RESOURCES quan.qrc )
set( quan_FORMS doubletabwidget.ui  quanconfigform.ui  quanqueryform.ui )

add_library( Quan MODULE
  ${quan_SOURCES}
  ${quan_HEADERS}
  ${quan_RESOURCES}
  )

target_link_libraries(
  Quan LINK_PUBLIC
  adplugin
  adcontrols
  adlog
  adutils
  portfolio
  adfs
  adprot
  adextension
  adplot
  adwidgets
  adportable
  adpublisher
  qtwrapper
  xmlparser
  Core
  extensionsystem
  ${Boost_LIBRARIES}
  )
  
#linux-*: LIBS += -lqwt -ldl -lrt
#macx: QMAKE_LFLAGS+=-Wl,-search_paths_first
