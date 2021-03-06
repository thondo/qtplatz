# adfs

project( adfs )

cmake_policy( SET CMP0022 NEW )

set( SHARED_ADFS_LIBRARY ON )

if ( SHARED_ADFS_LIBRARY )
  find_package(Threads REQUIRED)
  add_definitions( -DADFS_SHARED_LIBRARY -DADFS_LIBRARY )
endif()

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4669 )
endif()

add_definitions(-DSQLITE_THREADSAFE)

if ( WIN32 )
  set( adfs_SOURCE apiwin32.cpp apiwin32.hpp )
else()
  set( adfs_SOURCE apiposix.cpp apiposix.hpp )
  set( oslibs ${CMAKE_DL_LIBS} Threads::Threads )
endif()

add_library( ${PROJECT_NAME} SHARED
  adfs.cpp
  attributes.cpp
  filesystem.cpp
  fs.cpp
  folder.cpp
  file.cpp
  sqlite.cpp
  sqlite3.c
  adfs.hpp
  attributes.hpp
  cpio.hpp
  filesystem.hpp
  fs.hpp
  folder.hpp
  file.hpp
  sqlite.hpp
  sqlite3.h
  ${adfs_SOURCE}
  )

target_link_libraries( adfs LINK_PUBLIC
  adportable
  adportable_serializer
  ${oslibs}
  ${Boost_LIBRARIES}
  )

add_executable( sqlite3  shell.c sqlite3.c )

find_package( Threads REQUIRED )
target_link_libraries( sqlite3 Threads::Threads ${CMAKE_DL_LIBS} )

install( TARGETS ${PROJECT_NAME} sqlite3
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  adfs_global.h
  adfs.hpp
  apiposix.hpp
  apiwin32.hpp
  attributes.hpp
  CMakeLists.txt
  cpio.hpp
  file.hpp
  filesystem.hpp
  folder.hpp
  fs.hpp
  sqlite3ext.h
  sqlite3.h
  sqlite.hpp
  DESTINATION include/adfs COMPONENT headers )
