cmake_minimum_required (VERSION 3.18.2)
cmake_policy(SET CMP0028 NEW)
#cmake_policy(SET CMP0053 OLD)

project (qtplatz)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 CONFIG REQUIRED Network Core Widgets PrintSupport )

file(GLOB ${PROJECT_NAME}_Resources *.qrc)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/shared/qtsingleapplication
  ${CMAKE_SOURCE_DIR}/src/shared/qtlockedfile
  ${CMAKE_SOURCE_DIR}/src
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  )

if (WIN32 )
  set( lockedfile_SOURCES
    ../shared/qtlockedfile/qtlockedfile_win.cpp
    )
else()
  set( lockedfile_SOURCES
    ../shared/qtlockedfile/qtlockedfile_unix.cpp
    )
endif()

set( SOURCES
  main.cpp
  ../tools/qtcreatorcrashhandler/crashhandlersetup.cpp
  ../tools/qtcreatorcrashhandler/crashhandlersetup.h
  ../shared/qtsingleapplication/qtsingleapplication.cpp
  ../shared/qtsingleapplication/qtlocalpeer.cpp
  ../shared/qtlockedfile/qtlockedfile.cpp
  )

# remove 'WIN32' below for run with console
if ( APPLE )
  set ( BUNDLE MACOSX_BUNDLE )
elseif ( WIN32 )
  set ( BUNDLE WIN32 )
endif()

add_executable( qtplatz ${BUNDLE}
  ${SOURCES}
  ${lockedfile_SOURCES}
  ${${PROJECT_NAME}_Resources}
  qtplatz.rc
  )

target_link_libraries(
  qtplatz LINK_PUBLIC
  extensionsystem
  aggregation
  utils
  Qt5::Core
  Qt5::Network
)

if (APPLE)
  set_target_properties( qtplatz PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_BUNDLE_NAME "QtPlatz"
    MACOSX_BUNDLE_ICON_FILE "${CURRENT_SOURCE_DIR}/qtcreator.ico"
    MACOSX_BUNDLE_BUNDLE_IDENTIFIER "com.ms-cheminfo.qtplatz"
    )
endif()

install( TARGETS qtplatz
  RUNTIME DESTINATION bin COMPONENT applications
  BUNDLE DESTINATION bin COMPONENT applications
  )

install( FILES qtcreator.ico DESTINATION share/qtplatz COMPONENT applications )
