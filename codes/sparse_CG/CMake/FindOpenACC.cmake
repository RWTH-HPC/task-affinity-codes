if (CMAKE_COMPILER_IS_GNUCC)
	set(OPENACC_FLAGS "-fopenacc")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "PGI")
	set(OPENACC_TARGET_ACCELERATOR
		nvidia
		CACHE
		STRING
		"The target accelerator to use for OpenACC"
	)
	set(OPENACC_CUDA_COMPATIBILITY
		""
		CACHE
		STRING
		"The (optional) CUDA compatibility version (eg. 5.0)"
	)
	set(OPENACC_COMPUTE_CAPABILITY
		""
		CACHE
		STRING
		"The compute capability to target (e.g. 35)"
	)
	set(OPENACC_EXTRA_TA_FLAGS
		""
		CACHE
		STRING
		"Extra flag to pass to -ta="
	)
	set(OPENACC_FLAGS "-Minfo=accel -acc -ta=${OPENACC_TARGET_ACCELERATOR}")
	if (OPENACC_CUDA_COMPATIBILITY)
		set(OPENACC_FLAGS
			"${OPENACC_FLAGS},cuda${OPENACC_CUDA_COMPATIBILITY}"
		)
	endif ()
	if (OPENACC_COMPUTE_CAPABILITY)
		set(OPENACC_FLAGS
			"${OPENACC_FLAGS},cc${OPENACC_COMPUTE_CAPABILITY}"
		)
	endif ()
	if (OPENACC_EXTRA_TA_FLAGS)
		set(OPENACC_FLAGS "${OPENACC_FLAGS},${OPENACC_EXTRA_TA_FLAGS}")
	endif ()
endif ()
