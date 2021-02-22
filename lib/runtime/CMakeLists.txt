add_subdirectory( exec )

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/include/runtime/lart.h
    ${PROJECT_BINARY_DIR}/include/runtime/lart.h
    COPYONLY
)

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/runtime/exec/dfsan-blacklist.txt
    ${PROJECT_BINARY_DIR}/runtime/dfsan-blacklist.txt
    COPYONLY
)

