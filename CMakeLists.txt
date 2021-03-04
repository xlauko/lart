cmake_minimum_required( VERSION 3.17 )

set( LART_VERSION_MAJOR 0 )
set( LART_VERSION_MINOR 1 )
set( LART_VERSION_PATCH 0 )

set(CMAKE_VERBOSE_MAKEFILE ON)

if( NOT PACKAGE_VERSION )
  set( PACKAGE_VERSION "${LART_VERSION_MAJOR}.${LART_VERSION_MINOR}.${LART_VERSION_PATCH}" )
  if( DEFINED LART_VERSION_TWEAK )
    set( PACKAGE_VERSION "${PACKAGE_VERSION}-${LART_VERSION_TWEAK}" )
  endif()
  set( LART_VERSION_INFO ${PACKAGE_VERSION} )
endif()

project( lart VERSION ${PACKAGE_VERSION}
	DESCRIPTION "LLVM Abstraction and Refinement Tool"
    HOMEPAGE_URL "https://github.com/xlauko/lart"
)

set( CMAKE_PREFIX_PATH "/prefix" )

include( cmake/project_settings.cmake )

add_library( lart_project_options INTERFACE )
target_compile_features( lart_project_options INTERFACE cxx_std_20 )
target_include_directories( lart_project_options INTERFACE include external )

# Link lart to use the warnings specified in compiler_warnings.cmake
add_library( lart_project_warnings INTERFACE )

# standard compiler warnings
include( cmake/compiler_warnings.cmake )
set_project_warnings( lart_project_warnings )

# sanitizer options if supported by compiler
# include( cmake/sanitizers.cmake )
# enable_sanitizers( lart_project_options )

# clang time profiling
if ( CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" )
  option( ENABLE_BUILD_WITH_TIME_TRACE
    "Enable -ftime-trace to generate time tracing .json files on clang" OFF
  )
  if ( ENABLE_BUILD_WITH_TIME_TRACE )
    add_compile_definitions( lart_project_options INTERFACE -ftime-trace )
  endif()
endif()

# allow for static analysis options
include( cmake/static_analyzers.cmake )

# VERIFY LLVM INSTALLATION DIR

# Set this to a valid LLVM installation dir
set( LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory" )

set( LIBCXX_INSTALL_DIR "" CACHE PATH "LIBCXX installation directory" )


# A bit of a sanity checking
set( LLVM_INCLUDE_DIR "${LLVM_INSTALL_DIR}/include/llvm" )
if( NOT EXISTS "${LLVM_INCLUDE_DIR}" )
  message( FATAL_ERROR
    " LLVM_INSTALL_DIR (${LT_LLVM_INCLUDE_DIR}) is invalid." )
endif()

set( LLVM_CMAKE_FILE "${LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake" )
if( NOT EXISTS "${LLVM_CMAKE_FILE}" )
  message(FATAL_ERROR
    " LLVM_CMAKE_FILE (${LLVM_CMAKE_FILE}) is invalid.")
endif()

# Add the location of LLVMConfig.cmake to CMake search paths (so that
# find_package can locate it)
list( APPEND CMAKE_PREFIX_PATH "${LLVM_INSTALL_DIR}/lib/cmake/llvm/" )

find_package( LLVM REQUIRED CONFIG 10.0 )

message( STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}" )
message( STATUS "Using LLVMConfig.cmake from ${LLVM_DIR}" )

add_definitions( ${LLVM_DEFINITIONS} )
link_directories( ${LLVM_LIBRARY_DIRS} )
include_directories( SYSTEM ${LLVM_INCLUDE_DIRS} )

find_program( CLANG_BINARY "clang"
  HINTS ${LLVM_TOOLS_BINARY_DIR}
  DOC "The clang binary to use in the lart wrapper script.")
find_program( CLANGPP_BINARY "clang++"
  HINTS ${LLVM_TOOLS_BINARY_DIR}
  DOC "The clang binary to use in the lart++ wrapper script.")
if ( NOT CLANG_BINARY )
  message( FATAL_ERROR "Clang not found; please make sure that the version corresponding to your LLVM installation is available." )
endif()

add_custom_target( lit
  COMMAND lit -v "${CMAKE_CURRENT_BINARY_DIR}/test"
  DEPENDS lart
)

add_subdirectory( external )
add_subdirectory( lib )

option( BUILD_TESTS "Build tests" OFF )

if ( BUILD_TESTS )
  enable_testing()
  add_subdirectory( test )
endif()
