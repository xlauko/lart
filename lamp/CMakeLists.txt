cmake_minimum_required( VERSION 3.16 )

include( cmake/lamp.cmake )

register_domain( unit )
register_domain( tracing-unit )
register_domain( interval )
register_domain( tracing-interval )
register_domain( constant )
register_domain( trivial )
register_domain( term )
register_domain( tracing-term )
register_domain( pa )
register_domain( zero )
register_domain( tracing-zero )
register_domain( floats )
register_domain( tracing-floats )


add_library( lamp-api INTERFACE )
target_include_directories( lamp-api
    INTERFACE
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties( lamp-api PROPERTIES PUBLIC_HEADER
    "include/lamp.h;include/lamp-pointers.h;include/svcomp.h"
)

install(TARGETS lamp-api
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)