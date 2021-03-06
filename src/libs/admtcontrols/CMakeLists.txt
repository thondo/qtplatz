cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )

project( admtcontrols )
set( PROVIDER MS-Cheminformatics )

add_definitions( -DADMTCONTROLS_LIBRARY )

include_directories(
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}
  )

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( ${PROJECT_NAME} SHARED
  electricsectormethod.cpp
  electricsectormethod.hpp
  orbitprotocol.cpp
  orbitprotocol.hpp
  scanlaw.cpp
  scanlaw.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adportable
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
