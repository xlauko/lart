
macro( register_domain domain )
  option( ${domain}_build "Build domain ${domain}" ON )

  if ( ${domain}_build )
    # general domain build properties/setup
    add_library( ${domain}-prop INTERFACE )

    target_include_directories( ${domain}-prop
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_library( ${domain} STATIC ${domain}.cpp )
    target_link_libraries( ${domain}
      PUBLIC ${domain}-prop
      PRIVATE lava runtime llvmsc::llvmsc
    )

    set_property( TARGET ${domain} PROPERTY POSITION_INDEPENDENT_CODE ON )
    set_property( TARGET ${domain} PROPERTY CXX_STANDARD 20 )

    install (TARGETS ${domain}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
    )

  endif()
endmacro()
