# find libxml2

if ( WIN32 )
  return()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES Darwin )
  
  find_path( libxml2_INCLUDE_DIR NAMES libxml/xpath.h PATHS /opt/local/include/libxml2 )

elseif (${CMAKE_SYSTEM_NAME} MATCHES Linux )

  find_path( libxml2_INCLUDE_DIR NAMES libxml/xpath.h PATHS /usr/include/libxml2 /usr/local/include/libxml2 )

  find_library( libxml2_LIBRARY NAMES xml2 libxml2 PATHS /usr/lib )

endif()

if ( libxml2_INCLUDE_DIR )
  set( libxml2_FOUND 1 )
  set( libxml2_INCLUDE_DIRS ${libxml2_INCLUDE_DIR} )
  set( libxml2_LIBRARIES ${libxml2_LIBRARY} )
endif()

if ( libxml2_FOUND )
  message( STATUS "Found libxml2 in " ${libxml2_LIBRARY} ", " ${libxml2_INCLUDE_DIR} )
else()
  message( STATUS "libxml2 not find" )
endif()

