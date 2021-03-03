include(ExternalProject)

# hack around different compilers
ExternalProject_Add( native
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/native
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/native
    DOWNLOAD_COMMAND "" # None - source is present
    UPDATE_COMMAND   ""
    INSTALL_COMMAND  ""
    PREFIX           "native"
    BUILD_ALWAYS     ON
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_COMPILER=${CLANGPP_BINARY}
        -DCMAKE_C_COMPILER=${CLANG_BINARY}
)

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/include/runtime/lart.h
    ${PROJECT_BINARY_DIR}/include/lart.h
    COPYONLY
)
