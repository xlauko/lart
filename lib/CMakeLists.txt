add_subdirectory( cc )

add_subdirectory( runtime )

include(ExternalProject)

# hack around different compilers
ExternalProject_Add( lamp
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lamp
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/lamp
    DOWNLOAD_COMMAND "" # None - source is present
    UPDATE_COMMAND   ""
    INSTALL_COMMAND  ""
    PREFIX           "lamp"
    BUILD_ALWAYS     ON
    CMAKE_ARGS
        -DCMAKE_CXX_COMPILER=${CLANGPP_BINARY}
        -DCMAKE_C_COMPILER=${CLANG_BINARY}
)

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/include/lamp/lamp.h
    ${PROJECT_BINARY_DIR}/include/lamp.h
    COPYONLY
)
