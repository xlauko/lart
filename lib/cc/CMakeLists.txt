add_library( lart
  OBJECT
    assume.cpp
    alias.cpp
    driver.cpp
    dfa.cpp
    lifter.cpp
    pass.cpp
    preprocess.cpp
    syntactic.cpp
    taint.cpp

    backend/base.cpp
    backend/native.cpp
)

set_target_properties( lart PROPERTIES 
  COMPILE_FLAGS "-stdlib=libc++" 
  LINK_FLAGS "-stdlib=libc++ -lc++abi"
)

set_property(TARGET lart PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories( lart
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_dependencies( lart svflib )

find_package( spdlog CONFIG REQUIRED )
find_package( fmt CONFIG REQUIRED )
find_package( cppcoro CONFIG REQUIRED )

target_link_libraries( lart PRIVATE 
  llvmsc::llvmsc 
  svflib
  fmt::fmt-header-only
  spdlog::spdlog_header_only
  cppcoro
)

target_link_libraries( lart PRIVATE lart_project_options lart_project_warnings )

if( NOT LLVM_ENABLE_RTTI )
  set_target_properties( lart PROPERTIES COMPILE_FLAGS "-fno-rtti" )
endif()

add_library( lart_module MODULE $<TARGET_OBJECTS:lart> )
target_link_libraries( lart_module PRIVATE llvmsc::llvmsc svflib )
target_link_libraries( lart_module PRIVATE lart_project_options lart_project_warnings )

add_library( lart_static STATIC $<TARGET_OBJECTS:lart> )
target_link_libraries( lart_static PUBLIC lart )

configure_file( "lartcc.in"
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/lartcc @ONLY
)

configure_file( "lartcc++.in"
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/lartcc++ @ONLY
)

file(
    COPY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/lartcc
    DESTINATION ${PROJECT_BINARY_DIR}/bin/
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

file(
    COPY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/lartcc++
    DESTINATION ${PROJECT_BINARY_DIR}/bin/
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
