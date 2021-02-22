include(ExternalProject)

set( SVFLIB_ROOT ${CMAKE_CURRENT_BINARY_DIR}/svflib )
set( SVFLIB_SRC ${${SVFLIB}_SOURCE_DIR} )

ExternalProject_Add( ${SVFLIB}
    PREFIX           ${SVFLIB_ROOT}

    SOURCE_DIR       ${SVFLIB_SRC}

    DOWNLOAD_COMMAND "" # content is fetched dufing configuraion

    CMAKE_ARGS
        -DLLVM_DIR=${LLVM_DIR}
        -DENABLE_TESTING:BOOL=OFF
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target install --config Release

    BUILD_BYPRODUCTS ${SVFLIB_ROOT}/lib/libSvf.a
    BUILD_BYPRODUCTS ${SVFLIB_ROOT}/include
)

add_library( svf STATIC IMPORTED GLOBAL )

set_property( TARGET svf PROPERTY
    IMPORTED_LOCATION ${SVFLIB_ROOT}/lib/libSvf.a )

file( MAKE_DIRECTORY ${SVFLIB_ROOT}/include )
target_include_directories( svf
    INTERFACE
        ${SVFLIB_SRC}/include
        ${SVFLIB_ROOT}/include
)

add_library( svf::svf ALIAS svf )
