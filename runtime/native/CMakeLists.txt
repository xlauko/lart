add_library( lart-runtime INTERFACE )
target_include_directories( lart-runtime
    INTERFACE
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library( native
  STATIC
    svcomp.cpp
    config.cpp
    choose.cpp
    shadow.cpp
    stash.cpp
    lart.cpp
    trace.cpp
    fault.cpp
)

set_property( TARGET native PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET native PROPERTY CXX_STANDARD 20 )

target_include_directories( native
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/runtime
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property( TARGET llvmsc PROPERTY POSITION_INDEPENDENT_CODE ON )

target_link_libraries( native
  PRIVATE
    project_warnings
    project_options
    llvmsc::llvmsc
)

install (TARGETS native
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

