
if ( NOT Boost_FOUND )
  find_package( Boost REQUIRED )
endif()

include( "soname" )

if ( WITH_PYTHON3 AND ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM ) )
  find_package( Python3 COMPONENTS Interpreter Development )
  if ( Python3_FOUND )
    set( python3 "python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}" )
  endif()
endif()

if ( Boost_FOUND )

  set ( libs
    atomic
    bzip2
    container
    chrono
    date_time
    filesystem
    iostreams
    json
    locale
    log
    log_setup
    program_options
    regex
    random
    serialization
    system
    thread
    timer
    wserialization
    ${python3}
    )

  if ( WIN32 )
    file( GLOB files ${Boost_LIBRARY_DIRS}/boost_*-mt-x64-1_*.dll )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
    if ( VERBOSE )
      foreach ( file ${files} )
        message( STATUS "boost-cpack file: " ${file} " --> " ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} )
      endforeach()
    endif()
  else()
    foreach ( lib ${libs} )
      file( GLOB files ${Boost_LIBRARY_DIRS}/libboost_${lib}.${SO}* )
      install( PROGRAMS ${files} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
    endforeach()
  endif()

endif()
