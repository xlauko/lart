cmake_minimum_required( VERSION 3.16 )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake;"
)

include( lamp )

register_domain( unit )
register_domain( constant )
register_domain( trivial )
register_domain( term )
register_domain( pa )

add_library( lamp-api INTERFACE )
target_include_directories( lamp-api
    INTERFACE
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lamp.h
    ${PROJECT_BINARY_DIR}/include/lamp.h
    COPYONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lamp-pointers.h
    ${PROJECT_BINARY_DIR}/include/lamp-pointers.h
    COPYONLY
)