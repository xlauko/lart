add_library( lart
    driver.cpp
    pass.cpp
    annotations.cpp
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines" )

target_include_directories( lart
  PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/include
)

target_link_libraries( lart PRIVATE llvmsc )

target_compile_features( lart PRIVATE cxx_std_20 )

if( NOT LLVM_ENABLE_RTTI )
  set_target_properties( lart PROPERTIES COMPILE_FLAGS "-fno-rtti" )
endif()

configure_file("lartcc.in" "lartcc" @ONLY)
configure_file("lartcc++.in" "lartcc++" @ONLY)
