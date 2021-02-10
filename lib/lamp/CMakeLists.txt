add_library( trivial
    trivial.cpp
)

target_link_libraries( trivial lava )
target_include_directories( trivial
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${PROJECT_SOURCE_DIR}/lib/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/include/lamp/lamp.h
    ${PROJECT_BINARY_DIR}/include/lamp.h
    COPYONLY
)
