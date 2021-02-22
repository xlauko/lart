cmake_minimum_required( VERSION 3.17 )

project( lamp-project )

add_library( unit
  STATIC
    unit.cpp
)

set_property( TARGET unit PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET unit PROPERTY CXX_STANDARD 20 )

target_compile_options( unit PUBLIC
  -fno-exceptions -fno-rtti -fsanitize=dataflow
)
target_link_options( unit PUBLIC -Wl,-rpath,${LLVM_INSTALL_DIR}/lib )

target_include_directories( unit
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${PROJECT_SOURCE_DIR}/../include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
