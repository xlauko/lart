cmake_minimum_required( VERSION 3.16 )

project( lart-native )

add_library( native
  STATIC
    config.cpp
    choose.cpp
    shadow.cpp
    stash.cpp
    taint.cpp
    lart.cpp
    trace.cpp
)

set_property( TARGET native PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET native PROPERTY CXX_STANDARD 20 )

target_compile_options( native PUBLIC
  -fno-exceptions -fno-rtti -fsanitize=dataflow
  -nostdinc++
)

target_link_options( native
  PUBLIC
    -Wl,-rpath,${LLVM_INSTALL_DIR}/lib
    -L${LIBCXX_INSTALL_DIR}/lib
    -Wl,-rpath,${LIBCXX_INSTALL_DIR}/lib
)

target_include_directories( native
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${PROJECT_SOURCE_DIR}/../../include/runtime
    ${LIBCXX_INSTALL_DIR}/include/c++/v1
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries( native
  PRIVATE
    project_warnings
    project_options
)
