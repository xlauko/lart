cmake_minimum_required( VERSION 3.16 )

if ( DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE )
  set( CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "" )
endif()

project( lart VERSION 0.0.0
	DESCRIPTION "LLVM Abstraction and Refinement Tool"
  HOMEPAGE_URL "https://github.com/xlauko/lart"
)

option( BUILD_LART_CC "Build LART Clang Plugin" OFF )
option( BUILD_LART_RUNTIME "Build LART Runtime" OFF )

if ( BUILD_LART_CC )
  add_subdirectory( external )
  add_subdirectory( lartcc )
endif()

if ( BUILD_LART_RUNTIME )
  add_subdirectory( runtime )
  add_subdirectory( lava )
  add_subdirectory( lamp )
endif()


# if ( ENABLE_TESTING )
#   enable_testing()
#   add_subdirectory( test )
# endif()
