add_library( lart
  OBJECT
    assume.cpp
    alias.cpp
    driver.cpp
    dfa.cpp
    lifter.cpp
    pass.cpp
    preprocess.cpp
    syntactic.cpp
    taint.cpp

    backend/base.cpp
    backend/native.cpp
)

set_property(TARGET lart PROPERTY POSITION_INDEPENDENT_CODE ON)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines" )

target_include_directories( lart
  PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/include
)

add_dependencies( lart svflib )

target_link_libraries( lart PRIVATE llvmsc::llvmsc svf::svf spdlog::spdlog_header_only )
target_link_libraries( lart PRIVATE lart_project_options lart_project_warnings )

if( NOT LLVM_ENABLE_RTTI )
  set_target_properties( lart PROPERTIES COMPILE_FLAGS "-fno-rtti" )
endif()

add_library( lart_module MODULE $<TARGET_OBJECTS:lart> )
target_link_libraries( lart_module PRIVATE llvmsc::llvmsc svf::svf )
target_link_libraries( lart_module PRIVATE lart_project_options lart_project_warnings )

add_library( lart_static STATIC $<TARGET_OBJECTS:lart> )
target_link_libraries( lart_static PUBLIC lart )

configure_file("lartcc.in" "lartcc" @ONLY)
configure_file("lartcc++.in" "lartcc++" @ONLY)
