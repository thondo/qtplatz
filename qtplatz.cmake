# qtplatz.cmake

set( BOOST_VERSION boost-1_58 )

list( APPEND CMAKE_MODULE_PATH ${QTPLATZ_SOURCE_DIR}/cmake/Modules )

find_package( arch )

#####################
# boost setup
#
if( WIN32 )
  set( BOOST_ROOT "C:/Boost/include/${BOOST_VERSION}" )
  set( Boost_USE_STATIC_LIBS ON )

  if ( RTC_ARCH_X64 )
    
    set( Boost_LIBRARY_DIR "C:/Boost/x86_64/lib" )
    
  else()

    set( Boost_LIBRARY_DIR "C:/Boost/lib" )    
    
  endif()

else()

  if ( RTC_ARCH_ARM )
    # do nothing -- expect boost on /usr/local/include and /usr/local/lib
  else()
    find_path( _boost NAMES include/boost PATHS "/usr/local/${BOOST_VERSION}/include" )
    if ( _boost )
      set(Boost_INCLUDE_DIR "${_boost}/include")
      set(Boost_LIBRARY_DIR "${_boost}/lib")      
    endif()
  endif()

endif()

#####################
# Qt5 setup
#

#set( CMAKE_PREFIX_PATH $ENV{QTDIR} )

if ( NOT CMAKE_CROSSCOMPILING )
  execute_process( COMMAND qmake -query QT_INSTALL_PREFIX
    OUTPUT_VARIABLE QTDIR ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
  if ( QTDIR )
    set ( CMAKE_PREFIX_PATH ${QTDIR} ${CMAKE_PREFIX_PATH} )
  endif()
endif()

#####################
# Compiler setup
#
if (MSVC)

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
  add_definitions(-DUNICODE -D_UNICODE)
  message(STATUS "Using ${CMAKE_CXX_COMPILER}. C++11 support is native.")

else()

  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++03" COMPILER_SUPPORTS_CXX03)

  if(COMPILER_SUPPORTS_CXX11)
    if ( RTC_ARCH_ARM )
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()

  if ( ${CMAKE_SYSTEM_NAME} MATCHES Linux )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()

endif()

