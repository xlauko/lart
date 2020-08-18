add_library( cc
    driver.cpp
    pass.cpp
    annotations.cpp
)

target_include_directories( cc
  PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/include
)

target_link_libraries( cc PRIVATE llvmsc )

target_compile_features( cc PRIVATE cxx_std_20 )

if( NOT LLVM_ENABLE_RTTI )
  set_target_properties( instrumentation PROPERTIES COMPILE_FLAGS "-fno-rtti" )
endif()

configure_file("lartcc.in" "lartcc" @ONLY)
configure_file("lartcc++.in" "lartcc++" @ONLY)
