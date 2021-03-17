IF (Z3_INCLUDE_DIR)
  SET(Z3_FIND_QUIETLY TRUE)
ENDIF (Z3_INCLUDE_DIR)

FIND_PATH(Z3_INCLUDE_DIR z3.h z3++.h PATH_SUFFIXES z3)

SET(Z3_NAMES z3 libz3 ltz3 libz3 lz3)
FIND_LIBRARY(Z3_LIBRARY NAMES ${Z3_NAMES} )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Z3 DEFAULT_MSG Z3_LIBRARY Z3_INCLUDE_DIR)

IF(Z3_FOUND)
  SET(Z3_LIBRARIES ${Z3_LIBRARY})
  SET(Z3_INCLUDE_DIRS ${Z3_INCLUDE_DIR})
ELSE(Z3_FOUND)
  SET(Z3_LIBRARIES)
ENDIF(Z3_FOUND)

MARK_AS_ADVANCED(Z3_LIBRARY Z3_INCLUDE_DIR)