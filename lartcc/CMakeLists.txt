include( ${PROJECT_SOURCE_DIR}/cmake/project_settings.cmake )

add_library( lart_project_options INTERFACE )
target_compile_features( lart_project_options INTERFACE cxx_std_20 )
target_include_directories( lart_project_options INTERFACE include external )

# Link lart to use the warnings specified in compiler_warnings.cmake
add_library( lart_project_warnings INTERFACE )

# standard compiler warnings
include( ${PROJECT_SOURCE_DIR}/cmake/compiler_warnings.cmake )
set_project_warnings( lart_project_warnings )

# sanitizer options if supported by compiler
# include( ${PROJECT_SOURCE_DIR}/cmake/sanitizers.cmake )
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
include( ${PROJECT_SOURCE_DIR}/cmake/static_analyzers.cmake )

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

find_package( LLVM REQUIRED CONFIG 12.0 )

message( STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}" )
message( STATUS "Using LLVMConfig.cmake from ${LLVM_DIR}" )

add_definitions( ${LLVM_DEFINITIONS} )
link_directories( ${LLVM_LIBRARY_DIRS} )
include_directories( SYSTEM ${LLVM_INCLUDE_DIRS} )

add_library( lartcc
  SHARED
    assume.cpp
    alias.cpp
    driver.cpp
    dfa.cpp
    lifter.cpp
    pass.cpp
    preprocess.cpp
    syntactic.cpp
    taint.cpp
)

set_target_properties( lartcc PROPERTIES
  COMPILE_FLAGS "-fcoroutines"
)

set_property(TARGET lartcc PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories( lartcc
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_dependencies( lartcc svflib )

find_package( spdlog CONFIG REQUIRED )
find_package( fmt CONFIG REQUIRED )

target_link_libraries( lartcc PRIVATE
  llvmsc::llvmsc
  svflib
  fmt::fmt-header-only
  spdlog::spdlog_header_only
)

target_link_libraries( lartcc PRIVATE lart_project_options lart_project_warnings )

if( NOT LLVM_ENABLE_RTTI )
  set_target_properties( lartcc PROPERTIES COMPILE_FLAGS "-fno-rtti" )
endif()

install (TARGETS lartcc
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)