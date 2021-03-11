add_subdirectory( cc )

option( BUILD_LART_RUNTIME "Build LART Runtime" ON )

if ( BUILD_LART_RUNTIME )
    
    if ( NOT LIBCXX_INSTALL_DIR )
      message( FATAL_ERROR "Sanitized libc++ not found." )
    endif()

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
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CLANGPP_BINARY}
            -DCMAKE_C_COMPILER=${CLANG_BINARY}
            -DLIBCXX_INSTALL_DIR=${LIBCXX_INSTALL_DIR}
    )

    configure_file(
        ${PROJECT_SOURCE_DIR}/lib/include/lamp/lamp.h
        ${PROJECT_BINARY_DIR}/include/lamp.h
        COPYONLY
    )
endif()
