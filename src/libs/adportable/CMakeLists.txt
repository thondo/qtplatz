
project( adportable )

add_definitions(-DADPORTABLE_LIBRARY)

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions( -wd4251 -wd4244 )
endif()

add_library( ${PROJECT_NAME} SHARED
  array_wrapper.hpp
  base64.cpp
  bzip2.cpp
  configloader.cpp
  configuration.cpp
  constants.cpp
  ConvertUTF.c
  date_string.cpp
  date_time.cpp
  debug.cpp
  debug_core.cpp
  fft.cpp
  graycode.cpp
  graycode.hpp
  http_client.cpp
  histogram_processor.cpp
  histogram_processor.hpp
  lifecycleframe.cpp
  mblock.hpp
  mslockref.cpp
  mslockref.hpp
  polfit.cpp
  profile.cpp
  semaphore.hpp
  serialport.cpp
  sgfilter.cpp
  spectrum_processor.cpp
  string.cpp
  textfile.cpp
  counting/threshold_finder.hpp
  counting/threshold_index.hpp
  timer.cpp
  timesquaredscanlaw.cpp
  utf.cpp
  uuid.cpp
  fft4g_h.c
  waveform_averager.hpp
  waveform_peakfinder.cpp
  waveform_peakfinder.hpp
  waveform_processor.hpp
  waveform_simulator.hpp
  waveform_simulator.cpp
  waveform_wrapper.hpp
  )

target_include_directories( adportable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  xmlparser
  ${Boost_LIBRARIES}
  Threads::Threads
  )

# message( STATUS "Boost_LIBRARIES = " ${Boost_LIBRARIES} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )

install( FILES
  debug.hpp
  float.hpp
  fft.hpp
  formula_parser.hpp
  graycode.hpp
  base64.hpp
  base85.hpp
  binary_search.hpp
  binary_serializer.hpp
  serializer.hpp
  DESTINATION include/adportable COMPONENT headers
  )
