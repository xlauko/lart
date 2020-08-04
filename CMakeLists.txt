cmake_minimum_required( VERSION 3.17 )

set( LART_VERSION_MAJOR 0 )
set( LART_VERSION_MINOR 1 )
set( LART_VERSION_PATCH 0 )

if( NOT PACKAGE_VERSION )
  set( PACKAGE_VERSION "${LART_VERSION_MAJOR}.${LART_VERSION_MINOR}.${LART_VERSION_PATCH}" )
  if( DEFINED LART_VERSION_TWEAK )
    set( PACKAGE_VERSION "${PACKAGE_VERSION}-${LART_VERSION_TWEAK}" )
  endif()
  set( LART_VERSION_INFO ${PACKAGE_VERSION} )
endif()

project( "lart" VERSION ${PACKAGE_VERSION}
	DESCRIPTION "LLVM Abstraction and Refinement Tool"
    HOMEPAGE_URL "https://github.com/xlauko/lart"
)

include( cmake/project_settings.cmake )

add_library( project_options INTERFACE )
target_compile_features( project_options INTERFACE cxx_std_20 )

# Link lart to use the warnings specified in compiler_warnings.cmake
add_library( project_warnings INTERFACE )

# standard compiler warnings
include( cmake/compiler_warnings.cmake )
set_project_warnings( project_warnings )

# sanitizer options if supported by compiler
include( cmake/sanitizers.cmake )
enable_sanitizers( project_options )

# clang time profiling
if ( CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" )
  option( ENABLE_BUILD_WITH_TIME_TRACE
    "Enable -ftime-trace to generate time tracing .json files on clang" OFF
  )
  if ( ENABLE_BUILD_WITH_TIME_TRACE )
    add_compile_definitions( project_options INTERFACE -ftime-trace )
  endif()
endif()

# allow for static analysis options
include( cmake/static_analyzers.cmake )

find_package( LLVM REQUIRED CONFIG )

message( STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}" )
message( STATUS "Using LLVMConfig.cmake from ${LLVM_DIR}" )

add_definitions( ${LLVM_DEFINITIONS} )
include_directories( SYSTEM ${LLVM_INCLUDE_DIRS} )

add_subdirectory( lib )
add_subdirectory( test )
