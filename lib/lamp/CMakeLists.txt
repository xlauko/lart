cmake_minimum_required( VERSION 3.17 )

project( lamp-project )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake;"
)

macro( register_domain domain )
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
      -fsanitize-blacklist=${CMAKE_CURRENT_SOURCE_DIR}/dataflow-blacklist.txt
      -nostdinc++
    )

    target_link_options( ${domain}
      PUBLIC
        -Wl,-rpath,${LLVM_INSTALL_DIR}/lib
        -L${LIBCXX_INSTALL_DIR}/lib
        -Wl,-rpath,${LIBCXX_INSTALL_DIR}/lib
    )


    target_include_directories( ${domain}
      PUBLIC
        $<INSTALL_INTERFACE:include>
        ${PROJECT_SOURCE_DIR}/../include
        ${LIBCXX_INSTALL_DIR}/include/c++/v1
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endif()
endmacro()

register_domain( unit )
register_domain( constant )
register_domain( trivial )

register_domain( term )
if ( term_build )
  add_library( libz3 STATIC IMPORTED )
  set_target_properties( libz3 PROPERTIES
    IMPORTED_LOCATION ${Z3_INSTALL_DIR}/lib/libz3.a
    INTERFACE_INCLUDE_DIRECTORIES ${Z3_INSTALL_DIR}/include
  )
  target_link_libraries( term PUBLIC libz3 )
endif()

register_domain( pa )
if ( pa_build )
  target_link_libraries( pa PUBLIC libz3 )
endif()