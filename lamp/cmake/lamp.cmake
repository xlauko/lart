
macro( register_domain domain )
  option( ${domain}_build "Build domain ${domain}" ON )

  if ( ${domain}_build )
    # general domain build properties/setup
    add_library( ${domain}-prop INTERFACE )

    target_compile_options( ${domain}-prop INTERFACE
      -fno-exceptions -fno-rtti
    )

    target_include_directories( ${domain}-prop
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # dataflow sanitized domain options
    add_library( ${domain}-dfs INTERFACE )

    # TODO create libc++ target
    target_include_directories( ${domain}-dfs
      INTERFACE
        ${LIBCXX_INSTALL_DIR}/include/c++/v1
    )

    target_link_options( ${domain}-dfs
      INTERFACE
        -Wl,-rpath,${LLVM_INSTALL_DIR}/lib
        -L${LIBCXX_INSTALL_DIR}/lib
        -Wl,-rpath,${LIBCXX_INSTALL_DIR}/lib
    )

    target_compile_options( ${domain}-dfs INTERFACE
      -nostdinc++
      -fsanitize=dataflow
      -fsanitize-blacklist=${CMAKE_CURRENT_SOURCE_DIR}/dataflow-blacklist.txt
    )

    # # domain source
    # add_library( ${domain}-src STATIC ${domain}.cpp )
    # target_link_libraries( ${domain}-src
    #   PUBLIC ${domain}-prop
    #   PRIVATE lava runtime
    # )

    # set_property( TARGET ${domain}-src PROPERTY POSITION_INDEPENDENT_CODE ON )
    # set_property( TARGET ${domain}-src PROPERTY CXX_STANDARD 20 )

    # sanitized domain library
    add_library( ${domain} STATIC ${domain}.cpp )
    target_link_libraries( ${domain}
      PUBLIC ${domain}-dfs ${domain}-prop
      PRIVATE lava runtime
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