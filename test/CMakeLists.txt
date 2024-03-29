include( CTest )

# add_subdirectory( unittests )

set( LART_TEST_SHLIBEXT "${CMAKE_SHARED_LIBRARY_SUFFIX}" )

set( LART_TEST_SITE_CFG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in" )
set( LART_TEST_CFG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py.in" )
set( LART_TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )

set( LART_FILECHECK_ARGS "" )

set( LIT_SITE_CFG_IN_HEADER
    "## Autogenerated from ${LART_TEST_SITE_CFG_INPUT}\n## Do not edit!" )

configure_file( "${LART_TEST_SITE_CFG_INPUT}"
  "${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py" @ONLY
)

configure_file( "scripts/testrun.sh.in"
    "${CMAKE_CURRENT_BINARY_DIR}/scripts/testrun.sh" @ONLY
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

file( COPY "include" DESTINATION "." )

add_test(
    NAME lit
    COMMAND
        lit -v "${CMAKE_CURRENT_BINARY_DIR}"
)

set_tests_properties( lit
    PROPERTIES
        ENVIROMENT "LARTCC_LIBCXX_PATH=${LIBCXX_INSTALL_DIR}"
)
