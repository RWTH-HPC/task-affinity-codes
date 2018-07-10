# Find Lis related things
# Author: Tim Cramer
# This module defines:
# LIS_INCLUDE_DIR, where to find lis.h
# LIS_LIBRARY_DIR, where to find liblis.a or liblis.so
# LIS_LIBRARIES, the shared lib of lis
# LIS_STATIC_LIBRARIES, the static lib of lis

SET(LIS_FOUND FALSE)

#####################################################################
# find dirs and libs
#####################################################################
FIND_PATH(
	LIS_INCLUDE_DIR
	lis.h
	PATHS
	$ENV{LIS_INCLUDE_DIR}
	/usr/local/include
	/usr/include
	DOC "Location of lis.h."
)

FIND_PATH(
	LIS_LIBRARY_DIR
	liblis.a
	liblis.so
	PATHS
	$ENV{LIS_LIBRARY_DIR}
	/usr/local/lib
	/usr/lib
	DOC "Location of liblis.so or liblis.a."
)

#####################################################################
# check success
#####################################################################
IF (LIS_INCLUDE_DIR AND LIS_LIBRARY_DIR)
	SET (LIS_FOUND TRUE)
ENDIF (LIS_INCLUDE_DIR AND LIS_LIBRARY_DIR)

IF (NOT LIS_FOUND)
	IF (LIS_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find lis Library.")
	ENDIF (LIS_FIND_REQUIRED)
ENDIF (NOT LIS_FOUND)
