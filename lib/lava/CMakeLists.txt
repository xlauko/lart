add_library( lava INTERFACE )

target_include_directories( lava
  INTERFACE
    $<INSTALL_INTERFACE:include>
    ${PROJECT_SOURCE_DIR}/lib/include
)

target_link_libraries( lava INTERFACE lart_project_options lart_project_warnings )
