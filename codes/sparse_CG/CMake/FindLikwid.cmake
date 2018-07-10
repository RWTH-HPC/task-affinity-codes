# Find likwid related things
# Author: Tim Cramer
# This module defines:
# LIKWID_INCLUDE_DIR, where to find likwid.h
# LIKWID_LIBRARY_DIR, where to find liblikwid

SET(LIKWID_FOUND FALSE)

#####################################################################
# find dirs and libs
#####################################################################
FIND_PATH(
	LIKWID_INCLUDE_DIR
	likwid.h
	PATHS
	$ENV{LIKWID_INCLUDE_DIR}
	/usr/local/include
	/usr/include
	/opt/likwid/include
	DOC "Location of likwid.h."
)

FIND_PATH(
	LIKWID_LIBRARY_DIR
	liblikwid.a
	liblikwid.so
	PATHS
	$ENV{LIKWID_LIBRARY_DIR}
	/usr/local/lib
	/usr/lib
	/opt/likwid/lib
	DOC "Location of liblikwid."
)

#####################################################################
# check success
#####################################################################
IF (LIKWID_INCLUDE_DIR AND LIKWID_LIBRARY_DIR)
	SET (LIKWID_FOUND TRUE)
ENDIF (LIKWID_INCLUDE_DIR AND LIKWID_LIBRARY_DIR)

IF (NOT LIKWID_FOUND)
	IF (LIKWID_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find likwid Library.")
	ENDIF (LIKWID_FIND_REQUIRED)
ENDIF (NOT LIKWID_FOUND)
