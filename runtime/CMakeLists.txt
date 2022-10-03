add_subdirectory( native )

add_library( runtime INTERFACE )

set( RUNTIME_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/native PARENT_SCOPE )

set_target_properties( runtime PROPERTIES PUBLIC_HEADER
    "include/runtime/lart.h;include/runtime/shadow.hpp"
)

target_include_directories( runtime
    INTERFACE
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS runtime
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
