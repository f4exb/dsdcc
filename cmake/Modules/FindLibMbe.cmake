# Find libmbe

FIND_PATH(LIBMBE_INCLUDE_DIR mbelib.h)

SET(LIBMBE_NAMES ${LIBMBE_NAMES} mbe libmbe)
FIND_LIBRARY(LIBMBE_LIBRARY NAMES ${LIBMBE_NAMES} PATH)

IF (LIBMBE_INCLUDE_DIR AND LIBMBE_LIBRARY)
    SET(LIBMBE_FOUND TRUE)
ENDIF (LIBMBE_INCLUDE_DIR AND LIBMBE_LIBRARY)

IF (LIBMBE_FOUND)
    IF (NOT LibMbe_FIND_QUIETLY)
        MESSAGE (STATUS "Found LibMbe: ${LIBMBE_LIBRARY}")
    ENDIF (NOT LibMbe_FIND_QUIETLY)
ELSE (LIBMBE_FOUND)
    IF (LibMbe_FIND_REQUIRED)
        MESSAGE (FATAL_ERROR "Could not find mbe")
    ENDIF (LibMbe_FIND_REQUIRED)
ENDIF (LIBMBE_FOUND)

mark_as_advanced(LIBMBE_INCLUDE_DIR LIBMBE_LIBRARY)