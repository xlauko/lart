find_package( Catch2 CONFIG REQUIRED )

add_executable( unit-tests )

add_library( catch-main STATIC catch-main.cpp )
target_link_libraries( catch-main 
    PRIVATE lart_project_options
    PUBLIC  Catch2::Catch2
)

target_include_directories( catch-main
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_sources( unit-tests
    PRIVATE
        unit.cpp
)

target_link_libraries( unit-tests 
    PRIVATE
        catch-main 
        lamp-api
        lart-runtime
        lart_project_options
        lart_project_warnings
)

include(CTest)
include(Catch)
catch_discover_tests( unit-tests )