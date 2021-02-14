add_subdirectory( cc )
add_subdirectory( lava )

if (BUILD_LART_RUNTIME)
    add_subdirectory( runtime )
    add_subdirectory( lamp )
endif()

