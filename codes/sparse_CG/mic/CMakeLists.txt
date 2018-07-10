add_definitions(-DUSE_OMP)

if (SERIAL_INIT)
	add_definitions(-DSERIAL_INIT)
endif (SERIAL_INIT)

if (PAGE_ALIGN)
	add_definitions(-DPAGE_ALIGN)
endif (PAGE_ALIGN)

# -offload-build is deprecated. -no-offload is OFF by default
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -offload-build -opt-report-phase:offload")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -opt-report-phase:offload")

if (OPENMP)
	add_executable(
		cg_mic
		main.c
		solver.c
		../common/def.c
		../common/help.c
		../common/io.c
		../common/mmio.c
		../common/output.c
		../common/errorcheck.c
	)
	target_link_libraries(cg_mic m)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_mic DESTINATION bin)
endif (OPENMP)

if (OPENMP_DD)
	add_executable(
		cg_mic_dd
		main.c
		solver.c
		../common/def.c
		../common/help.c
		../common/io.c
		../common/mmio.c
		../common/output.c
		../common/errorcheck.c
	)
	target_link_libraries(cg_mic_dd m)
	set_target_properties(
		cg_mic_dd
		PROPERTIES
		COMPILE_DEFINITIONS
		"DATA_DISTRIBUTION"
	)
	install(
		PROGRAMS
		${CMAKE_CURRENT_BINARY_DIR}/cg_mic_dd
		DESTINATION
		bin
	)
endif (OPENMP_DD)
