add_library( cc MODULE
    driver.cpp
    pass.cpp
)

if( NOT LLVM_ENABLE_RTTI )
  set_target_properties( instrumentation PROPERTIES COMPILE_FLAGS "-fno-rtti" )
endif()

configure_file("lartcc.in" "lartcc" @ONLY)
configure_file("lartcc++.in" "lartcc++" @ONLY)
