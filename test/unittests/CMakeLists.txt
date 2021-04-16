find_package( Catch2 CONFIG REQUIRED )
include(CTest)
include(Catch)

# catch2 main
add_library( catch-main STATIC catch-main.cpp )
target_link_libraries( catch-main 
    PRIVATE lart_project_options
    PUBLIC  Catch2::Catch2
)

target_include_directories( catch-main
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

# test configs
add_library( test-properties INTERFACE )
target_link_libraries( test-properties
    INTERFACE 
        catch-main 
        lamp-api
        lart-runtime
        lart_project_options
        lart_project_warnings
)
target_compile_options( test-properties INTERFACE "-g" )

# test packages
add_executable( lamp-unit-tests unit.cpp )
target_link_libraries( lamp-unit-tests PRIVATE unit-src test-properties )
catch_discover_tests( lamp-unit-tests )
