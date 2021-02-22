cmake_minimum_required( VERSION 3.17 )

project( lamp-project )

function( register_domain domain )
  option( ${domain}_build "Build domain ${domain}" ON )

  if ( ${domain}_build )
    add_library( ${domain}
      STATIC
        ${domain}.cpp
    )

    set_property( TARGET ${domain} PROPERTY POSITION_INDEPENDENT_CODE ON )
    set_property( TARGET ${domain} PROPERTY CXX_STANDARD 20 )

    target_compile_options( ${domain} PUBLIC
      -fno-exceptions -fno-rtti -fsanitize=dataflow
    )
    target_link_options( ${domain} PUBLIC -Wl,-rpath,${LLVM_INSTALL_DIR}/lib )

    target_include_directories( ${domain}
      PUBLIC
        $<INSTALL_INTERFACE:include>
        ${PROJECT_SOURCE_DIR}/../include
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endif()
endfunction()

register_domain( unit )
register_domain( constant )
