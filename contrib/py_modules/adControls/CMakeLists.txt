cmake_minimum_required ( VERSION 3.18.2 )

project ( adControls )

find_package( Boost 1.67 COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR} REQUIRED )
if ( Boost_FOUND )
  include_directories( ${Boost_INCLUDE_DIRS} )
endif()

if ( Python3_FOUND )
  include_directories( ${Python3_INCLUDE_DIRS} )
endif()

add_library( ${PROJECT_NAME} MODULE
  adcontrols.cpp
  chemicalformula.cpp
  chemicalformula.hpp
  isotopecluster.cpp
  isotopecluster.hpp
  peakresult.cpp
  peakresult.hpp
  uuid.cc
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  ${Python3_LIBRARIES}
  ${Boost_LIBRARIES}
  )

message( STATUS "PROJECT_BIANRY_DIR: " ${PROJECT_BINARY_DIR} )

if ( WIN32 )
  set ( suffix ".dll" )
else()
  set ( suffix ".so" )
endif()

set ( CMAKE_INSTALL_RPATH "@loader_path/../../../../../Contents/Frameworks" )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  PREFIX ""
  SUFFIX ${suffix}
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PYTHON_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PYTHON_MODULE_DIR}
  )

if ( APPLE )
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    INSTALL_RPATH "@loader_path/../../../../../Contents/Frameworks"
    )
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT modules
  LIBRARY DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT modules
  )
