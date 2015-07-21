
if ( WIN32 )
  set( SO "dll")
  set( dest "bin" )
elseif( APPLE )
  set( SO "dylib" )
  set( dest "lib/qtplatz" )  
else()
  set( SO "so" )
  set( dest "lib/qtplatz" )  
endif()

