add_subdirectory( sc )

set( SVFLIB "svflib" )

include(FetchContent)
include(ExternalProject)

FetchContent_Declare( ${SVFLIB}
    GIT_REPOSITORY  https://github.com/xlauko/SVF.git
    GIT_TAG         master
    GIT_SHALLOW     ON
    GIT_PROGRESS    ON

    BUILD_COMMAND   ""
)

FetchContent_GetProperties( ${SVFLIB} )
if( NOT ${SVFLIB}_POPULATED )
  FetchContent_Populate( ${SVFLIB} )
endif()

set( SVFLIB_ROOT ${CMAKE_CURRENT_BINARY_DIR}/svflib )

include( svflib.cmake )
