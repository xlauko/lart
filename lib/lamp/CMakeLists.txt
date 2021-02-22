set( CMAKE_C_COMPILER CLANG_BINARY )
set( CMAKE_CXX_COMPILER CLANGPP_BINARY )

# link against sanitized libc++
add_compile_options( "-fsanitize=dataflow;-fno-exceptions;-fno-rtti" )
add_link_options( "-fsanitize=dataflow;-stdlib=libc++;-Wl,-rpath,${LLVM_INSTALL_DIR}/lib" )
add_link_options( "-stdlib=libc++;-Wl,-rpath,${LLVM_INSTALL_DIR}/lib" )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=dataflow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-blacklist=${PROJECT_BINARY_DIR}/runtime/dfsan-blacklist.txt")

add_library( unit
  OBJECT
    unit.cpp
)

set_property( TARGET unit PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET unit PROPERTY CXX_STANDARD 20 )

target_link_libraries( unit lava )
target_include_directories( unit
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${PROJECT_SOURCE_DIR}/lib/include
    $<BUILD_INTERFACE:${LLVM_INSTALL_DIR}/include>
    $<BUILD_INTERFACE:${LLVM_INSTALL_DIR}/include/c++/v1>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/include/lamp/lamp.h
    ${PROJECT_BINARY_DIR}/include/lamp.h
    COPYONLY
)

add_library( unit-runtime STATIC $<TARGET_OBJECTS:unit> $<TARGET_OBJECTS:lart-exec> )
