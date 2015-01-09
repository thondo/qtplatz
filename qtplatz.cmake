# qtplatz.cmake

set(Boost_USE_STATIC_LIBS ON)
set(BOOST_VERSION boost-1_57)

include(CheckSymbolExists)

if(WIN32)

  check_symbol_exists( "_M_AMD64" "" RTC_ARCH_X64 )

  if ( NOT RTC_ARCH_X64 )
    check_symbol_exists( "_M_IX86" "" RTC_ARCH_X86 )
  endif( NOT RTC_ARCH_X64 )

  check_symbol_exists( "__i386__" "" RTC_ARCH_X86 )
  check_symbol_exists( "__x86_64__" "" RTC_ARCH_X64 )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
  set(Boost_INCLUDE_DIR "C:/Boost/include/${BOOST_VERSION}")

  if ( RTC_ARCH_X64 )
    set( Boost_LIBRARY_DIR "C:/Boost/x86_64/lib")
    set( CMAKE_PREFIX_PATH "C:\Qt\5.4\msvc2013_64_opengl" )
    message(STATUS "build for x86_64 64bit")
  endif( RTC_ARCH_X64 )

  if ( RTC_ARCH_X86 )
    set( Boost_LIBRARY_DIR "C:/Boost/lib")
    set( CMAKE_PREFIX_PATH "C:\Qt\5.4\msvc2013_opengl" )
    message(STATUS "build for x86 32bit")
  endif( RTC_ARCH_X86 )

else( WIN32 )

  check_symbol_exists( "__arm__" "" RTC_ARCH_ARM )
  if ( RTC_ARCH_ARM )
    set(Boost_INCLUDE_DIR "/usr/local/arm-linux-gnueabihf/usr/local/include")
    set(Boost_LIBRARY_DIR "/usr/local/arm-linux-gnueabihf/usr/local/lib")
  else()
    set(Boost_INCLUDE_DIR "/usr/local/${BOOST_VERSION}/include")
    set(Boost_LIBRARY_DIR "/usr/local/${BOOST_VERSION}/lib")
  endif()

endif()

find_package(Boost REQUIRED)

add_definitions(-DUNICODE -D_UNICODE)

if (MSVC)

  message(STATUS "Using ${CMAKE_CXX_COMPILER}. C++11 support is native.")

else()

  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()

  if ( RTC_ARCH_ARM )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()

endif()

set(CMAKE_INSTALL_PREFIX ./bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )
link_directories(${Boost_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib)


