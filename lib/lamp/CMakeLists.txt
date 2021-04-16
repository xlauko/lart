cmake_minimum_required( VERSION 3.16 )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake;"
)

include( lamp )

register_domain( unit )
register_domain( constant )
register_domain( trivial )
register_domain( term )
register_domain( pa )