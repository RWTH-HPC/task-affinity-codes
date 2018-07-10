SET(MKL_FOUND FALSE)

FIND_PATH(
	MKL_INCLUDE_DIR
	mkl_spblas.h
	PATHS
	/usr/local/include
	/usr/include
)

FIND_PATH(
	MKL_LIBRARY_DIR
	libmkl_solver_lp64.so
	libmkl_solver_lp64.a
	libmkl_solver_lp64.so
	libmkl_solver_lp64.a
	libmkl_intel_lp64.so
	libmkl_intel_lp64.a
	libmkl_intel_thread.so
	libmkl_intel_thread.a
	libmkl_core.so
	libmkl_core.a
	PATHS
	/usr/local/lib
	/usr/lib
	${MKLROOT}/lib/intel64
	${MKLROOT}/lib
	${MKLROOT}/lib64
	${MKLROOT}/lib/mic
)

IF (MKL_INCLUDE_DIR AND MKL_LIBRARY_DIR)
	SET (MKL_FOUND TRUE)
ENDIF (MKL_INCLUDE_DIR AND MKL_LIBRARY_DIR)

IF (NOT MKL_FOUND)
	IF (MKL_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find MKL.")
	ENDIF (MKL_FIND_REQUIRED)
ENDIF (NOT MKL_FOUND)
