cmake_minimum_required( VERSION 3.17 )

add_library( lart-exec-runtime
  OBJECT
    shadow.cpp
    stash.cpp
    taint.cpp
)

set_property( TARGET lart-exec-runtime PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET lart-exec-runtime PROPERTY CXX_STANDARD 20 )

# target_compile_options( lart-exec-runtime INTERFACE "-fcoroutines" )

# link against sanitized libc++
add_compile_options( "-fsanitize=dataflow;-nostdinc++;-fno-exceptions" )
add_link_options( "-fsanitize=dataflow;-stdlib=libc++;-Wl,-rpath,${LLVM_INSTALL_DIR}/lib" )

llvm_map_components_to_libnames( libcxx )

target_include_directories( lart-exec-runtime
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${PROJECT_SOURCE_DIR}/lib/include/runtime
    $<BUILD_INTERFACE:${LLVM_INSTALL_DIR}/include>
    $<BUILD_INTERFACE:${LLVM_INSTALL_DIR}/include/c++/v1>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries( lart-exec-runtime
  INTERFACE
    libcxx
  PRIVATE
    project_warnings
    project_options
)

add_library( libcxx INTERFACE )
add_dependencies( libcxx llvm )

target_include_directories( libcxx INTERFACE ${LLVM_INSTALL_DIR}/include/c++/v1 )
target_link_directories( libcxx INTERFACE ${LLVM_INSTALL_DIR}/lib )

add_dependencies( lart-exec-runtime libcxx )

add_library( lart_exec_static STATIC $<TARGET_OBJECTS:lart-exec-runtime> )
