
set (git_cmd "git")
set (git_arg "describe")

execute_process(COMMAND ${git_cmd} ${git_arg}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  RESULT_VARIABLE git_result
  OUTPUT_VARIABLE git_describe
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

configure_file(
  ${PROJECT_SOURCE_DIR}/version.h.in
  ${PROJECT_BINARY_DIR}/version.h
  )

add_subdirectory( libs )
# message( STATUS "************** end of src/libs *****************" )

if ( WITH_QT5 AND NOT CMAKE_CROSSCOMPILING )
  # message( STATUS "------------ src/adplugins ----------------" )
  add_subdirectory( adplugins )
  # message( STATUS "------------ src/app ----------------------" )
  add_subdirectory( app )
  # message( STATUS "------------ src/plugins ------------------" )
  add_subdirectory( plugins )
endif()

if ( VERBOSE )
  message( STATUS "------------ src/tools --------------------" )
endif()
add_subdirectory( tools )
