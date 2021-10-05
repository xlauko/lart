add_subdirectory( native )

add_library( runtime INTERFACE )

set_target_properties( runtime PROPERTIES PUBLIC_HEADER
    "include/runtime/lart.h;include/runtime/shadowmeta.h"
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
