# sc ----=================================================================== #
add_subdirectory( sc )

# svflib =================================================================== #
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
  message(STATUS "Populating svflib")
  FetchContent_Populate( ${SVFLIB} )
endif()

set( SVFLIB_ROOT ${CMAKE_CURRENT_BINARY_DIR}/svflib )

include( svflib.cmake )

# spdlog =================================================================== #
include(FetchContent REQUIRED)
FetchContent_Declare( spdlog
  GIT_REPOSITORY  https://github.com/xlauko/spdlog.git
  GIT_TAG         v1.8.2
)
FetchContent_GetProperties(spdlog)

if(NOT spdlog_POPULATED)
  message(STATUS "Populating spdlog")
  FetchContent_Populate(spdlog)
  set(SPDLOG_BUILD_TESTS OFF CACHE INTERNAL "")
  add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR} EXCLUDE_FROM_ALL)
  set(SPDLOG_INCLUDE_DIR ${spdlog_SOURCE_DIR}/include)
endif()
