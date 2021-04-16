add_library( runtime INTERFACE )
target_include_directories( runtime 
    INTERFACE
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory( native )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/runtime/lart.h
    ${PROJECT_BINARY_DIR}/include/lart.h
    COPYONLY
)

