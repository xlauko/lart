include( CTest )

add_executable( larttests )

target_sources( larttests
    PRIVATE
    src/main.cpp
    src/annotations.cpp
)

target_link_libraries( larttests PRIVATE cc )

target_include_directories( larttests
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)


target_compile_features( larttests PRIVATE cxx_std_20 )

target_compile_options(
    larttests
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/std:c++20>
            $<$<CXX_COMPILER_ID:MSVC>:/W4>
            $<$<CXX_COMPILER_ID:MSVC>:/WX>
            $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall>
            $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Werror>
            $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-pedantic>
)

# add `-DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE to switch to runtime only checks
# target_compile_definitions(SI_detail_tests PRIVATE
# CATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

add_test( NAME tests COMMAND tests )
