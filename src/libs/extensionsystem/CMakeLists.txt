cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

project( extensionsystem )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 REQUIRED Network Widgets Core Gui PrintSupport )

add_definitions(
  -DEXTENSIONSYSTEM_LIBRARY 
  -DIDE_TEST_DIR="${CMAKE_SOURCE_DIR}"
  -DCMAKE_AUTOMOC
  -DQT_DISABLE_DEPRECATED_BEFORE=0x040900
  -DCMAKE_AUTOMOC
  )

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  )

#unix:LIBS += $$QMAKE_LIBS_DYNLOAD

set( ${PROJECT_NAME}_HEADERS
  pluginerrorview.h
  plugindetailsview.h
  invoker.h
  iplugin.h
  iplugin_p.h
  extensionsystem_global.h
  pluginmanager.h
  pluginmanager_p.h
  pluginspec.h
  pluginspec_p.h
  pluginview.h
  optionsparser.h
  plugincollection.h
  pluginerroroverview.h
  )

set( ${PROJECT_NAME}_SOURCES
  pluginerrorview.cpp
  plugindetailsview.cpp
  invoker.cpp
  iplugin.cpp
  pluginmanager.cpp
  pluginspec.cpp
  pluginview.cpp
  optionsparser.cpp
  plugincollection.cpp
  pluginerroroverview.cpp
  )

set( {PROJECT_NAME}_FORMS
  pluginerrorview.ui
  plugindetailsview.ui
  pluginerroroverview.ui
)

set( ${PROJECT_NAME}_RESOURCES pluginview.qrc )

add_library(
  ${PROJECT_NAME} SHARED
  ${${PROJECT_NAME}_SOURCES} 
  ${${PROJECT_NAME}_HEADERS} 
  ${${PROJECT_NAME}_RESOURCES}
  )

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  adportable
  utils
  Qt5::Widgets
  Qt5::Core
)

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT runtime_libraries )
