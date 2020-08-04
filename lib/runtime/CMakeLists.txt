cmake_minimum_required( VERSION 3.17 )

add_library( lart-runtime MODULE
    config.cpp
)

set( LART_RUNTIME_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE STRING "Lart Runtime Directory")
