add_library( lava INTERFACE )

target_include_directories( lava
  INTERFACE
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries( lava INTERFACE lart_project_options lart_project_warnings )
