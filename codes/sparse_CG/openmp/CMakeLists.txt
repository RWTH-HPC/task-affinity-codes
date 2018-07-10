add_definitions(-DUSE_OMP)

find_package(Likwid)

if (SERIAL_INIT)
	add_definitions(-DSERIAL_INIT)
endif (SERIAL_INIT)

if (PAGE_ALIGN)
	add_definitions(-DPAGE_ALIGN)
endif (PAGE_ALIGN)

if (LIKWID_MARKER_API)
	add_definitions(-DLIKWID_MARKER_API)
	include_directories(${LIKWID_INCLUDE_DIR})
	link_directories(${LIKWID_LIBRARY_DIR})
endif (LIKWID_MARKER_API)

if (VECTOR_TIME)
	add_definitions(-DVECTOR_TIME)
endif (VECTOR_TIME)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qopenmp")
MESSAGE( STATUS "CMAKE_C_FLAGS:         " ${CMAKE_C_FLAGS} )

if (OPENMP)
	add_executable(
		cg_omp
		../common/def.c
		../common/help.c
		../common/io.c
		../common/kernel.c
		../common/main.c
		../common/mmio.c
		../common/output.c
		../common/solver.c
		../common/errorcheck.c
	)
	if (LIKWID_MARKER_API)
		target_link_libraries(cg_omp likwid m)
	else (LIKWID_MARKER_API)
		target_link_libraries(cg_omp m)
	endif (LIKWID_MARKER_API)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_omp DESTINATION bin)
endif (OPENMP)

if (OPENMP_DD)
	add_executable(
		cg_omp_dd
		../common/def.c
		../common/help.c
		../common/io.c
		../common/kernel.c
		../common/main.c
		../common/mmio.c
		../common/output.c
		../common/solver.c
		../common/errorcheck.c
	)
	if (STATIC_INIT)
		target_compile_definitions(
			cg_omp_dd
			PUBLIC
			"STATIC_INIT"
			"DATA_DISTRIBUTION"
		)
	else (STATIC_INIT)
 		set_target_properties(
			cg_omp_dd
			PROPERTIES
			COMPILE_DEFINITIONS
			"DATA_DISTRIBUTION"
		)
	endif (STATIC_INIT)
	if (LIKWID_MARKER_API)
		target_link_libraries(cg_omp_dd likwid m)
	else (LIKWID_MARKER_API)
		target_link_libraries(cg_omp_dd m)
	endif (LIKWID_MARKER_API)
	install(
		PROGRAMS
		${CMAKE_CURRENT_BINARY_DIR}/cg_omp_dd
		DESTINATION
		bin
	)
endif (OPENMP_DD)
