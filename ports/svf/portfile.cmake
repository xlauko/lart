vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO xlauko/SVF
  REF 1de995e69219e900335cb4bd09b118332e8d7b9f
  SHA512 09f43a99d9fe2395c03c834debc66c0edc2ef6b2184c476071f6219c6fc53794f35859a8f9497a67349f107b97fd5f71962c52e66d77c67a2d55101a9e3419c5
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

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DLLVM_DIR=${LLVM_DIR}
    -DCMAKE_CXX_FLAGS=-Wno-unused-variable
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(
  PACKAGE_NAME "svf"
  CONFIG_PATH lib/cmake/svf
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
