add_subdirectory( exec )

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/include/runtime/lart.h
    ${PROJECT_BINARY_DIR}/include/runtime/lart.h
    COPYONLY
)
