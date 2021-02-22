cmake_minimum_required( VERSION 3.17 )

add_library( lart-exec
  OBJECT
    shadow.cpp
    stash.cpp
    taint.cpp
    lart.cpp
)

set_property( TARGET lart-exec PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET lart-exec PROPERTY CXX_STANDARD 20 )

# link against sanitized libc++
add_compile_options( "-nostdinc++;-fno-exceptions;-fno-rtti" )
add_link_options( "-stdlib=libc++;-Wl,-rpath,${LLVM_INSTALL_DIR}/lib" )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=dataflow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-blacklist=${PROJECT_BINARY_DIR}/runtime/dfsan-blacklist.txt")

llvm_map_components_to_libnames( libcxx )

target_include_directories( lart-exec
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${PROJECT_SOURCE_DIR}/lib/include/runtime
    $<BUILD_INTERFACE:${LLVM_INSTALL_DIR}/include>
    $<BUILD_INTERFACE:${LLVM_INSTALL_DIR}/include/c++/v1>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries( lart-exec
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

add_dependencies( lart-exec libcxx )
add_library( lart-exec-runtime STATIC $<TARGET_OBJECTS:lart-exec> )
