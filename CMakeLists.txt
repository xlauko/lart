cmake_minimum_required( VERSION 3.23 )

if ( DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE )
  set( CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "" )
endif()

if (NOT DEFINED VCPKG_OVERLAY_PORTS)
    set(VCPKG_OVERLAY_PORTS "${CURRENT_LIST_DIR}/ports")
endif()

project( lart VERSION 0.1.0
	DESCRIPTION "LLVM Abstraction and Refinement Tool"
  HOMEPAGE_URL "https://github.com/xlauko/lart"
)

# Globally set the required C++ standard
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_EXTENSIONS ON )

add_subdirectory( external )
add_subdirectory( lartcc )
add_subdirectory( runtime )
add_subdirectory( lava )
add_subdirectory( lamp )


if ( ENABLE_TESTING )
  enable_testing()
  add_subdirectory( test )
endif()
