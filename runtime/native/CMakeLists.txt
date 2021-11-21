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
    taint.cpp
    lart.cpp
    trace.cpp
    fault.cpp
)

set_property( TARGET native PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET native PROPERTY CXX_STANDARD 20 )

target_compile_options( native PUBLIC
  -fno-exceptions -fno-rtti -fsanitize=dataflow
  -nostdinc++
)

target_link_options( native
  PUBLIC
    -Wl,-rpath,${LLVM_INSTALL_DIR}/lib
    -L${LIBCXX_INSTALL_DIR}/lib
    -Wl,-rpath,${LIBCXX_INSTALL_DIR}/lib
)

target_include_directories( native
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/runtime
    ${LIBCXX_INSTALL_DIR}/include/c++/v1
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries( native
  PRIVATE
    project_warnings
    project_options
)

install (TARGETS native
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)


add_library( native-concrete
  STATIC
    fault.cpp
    config.cpp
    concrete.cpp
)

set_property( TARGET native-concrete PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET native-concrete PROPERTY CXX_STANDARD 20 )

target_compile_options( native-concrete PUBLIC
  -fno-exceptions -fno-rtti
)

target_include_directories( native-concrete
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/runtime
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries( native-concrete
  PRIVATE
    project_warnings
    project_options
)

install (TARGETS native-concrete
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

add_library( native-concrete-i386
  STATIC
    fault.cpp
    config.cpp
    concrete.cpp
)

set_property( TARGET native-concrete-i386 PROPERTY POSITION_INDEPENDENT_CODE ON )
set_property( TARGET native-concrete-i386 PROPERTY CXX_STANDARD 20 )

target_compile_options( native-concrete-i386 PUBLIC
  -fno-exceptions -fno-rtti -m32
)

target_include_directories( native-concrete-i386
  PUBLIC
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/runtime
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries( native-concrete-i386
  PRIVATE
    project_warnings
    project_options
)

install (TARGETS native-concrete-i386
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
