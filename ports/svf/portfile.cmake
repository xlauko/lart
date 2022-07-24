vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO xlauko/SVF
  REF 76e14b2a51db9f6ece094975f049cfbbbc58347a
  SHA512 ec2fd7d48ac64610f681ce24298c0208ecd0612ee8b0f663feca58f50a030ff9c8faa853d77a0b15165a995d041c6c6552cc8dce6fba170751b70f348986662e
  HEAD_REF main
)

# Set this to a valid LLVM installation dir
if ( DEFINED ENV{LLVM_INSTALL_DIR} AND NOT DEFINED LLVM_INSTALL_DIR )
  set( LLVM_INSTALL_DIR "$ENV{LLVM_INSTALL_DIR}" )
endif()

set( LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory" )
message( STATUS "Using LLVM_INSTALL_DIR: ${LLVM_INSTALL_DIR}" )

# A bit of a sanity checking
set( LLVM_INCLUDE_DIR "${LLVM_INSTALL_DIR}/include/llvm" )
if( NOT EXISTS "${LLVM_INCLUDE_DIR}" )
  message( FATAL_ERROR " LLVM_INCLUDE_DIR (${LLVM_INCLUDE_DIR}) is invalid." )
endif()

set( LLVM_CMAKE_FILE "${LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake" )
if( NOT EXISTS "${LLVM_CMAKE_FILE}" )
  message(FATAL_ERROR " LLVM_CMAKE_FILE (${LLVM_CMAKE_FILE}) is invalid.")
endif()

set( LLVM_DIR "${LLVM_INSTALL_DIR}/lib/cmake/llvm/" CACHE PATH "LLVM config path" )
message( STATUS "Using LLVM_DIR: ${LLVM_DIR}" )

# Set this to a valid Z3 installation dir
if ( DEFINED ENV{Z3_DIR} AND NOT DEFINED Z3_DIR )
  set( Z3_DIR "$ENV{Z3_DIR}" )
endif()

set( Z3_DIR "" CACHE PATH "Z3 installation directory" )
message( STATUS "Using Z3_DIR: ${Z3_DIR}" )

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DLLVM_DIR=${LLVM_DIR}
    -DZ3_DIR=${Z3_DIR}
    -DCMAKE_CXX_FLAGS=-Wno-unused-variable
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(
  PACKAGE_NAME "svf"
  CONFIG_PATH lib/cmake/SVF
)

file( REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" )

file(
  INSTALL "${SOURCE_PATH}/LICENSE.TXT"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
  RENAME copyright
)

if ( EXISTS "${CMAKE_CURRENT_LIST_DIR}/${lower_package}_usage" )
  file(
    INSTALL "${CMAKE_CURRENT_LIST_DIR}/${lower_package}_usage"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${lower_package}"
    RENAME usage
  )
endif()
