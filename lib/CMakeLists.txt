add_subdirectory( cc )

option( BUILD_LART_RUNTIME "Build LART Runtime" OFF )

if ( BUILD_LART_RUNTIME )
    add_subdirectory( runtime )
    add_subdirectory( lava )
    add_subdirectory( lamp )
endif()
