add_subdirectory( native )

add_library( runtime INTERFACE )

target_include_directories( runtime
    INTERFACE
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# TODO: (use install)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/runtime/lart.h
    ${PROJECT_BINARY_DIR}/include/lart.h
    COPYONLY
)
