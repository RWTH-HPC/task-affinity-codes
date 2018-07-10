add_definitions(-DUSE_OMP -DTASKING)

option(TASK_DIST	"Whether to use task distribution in cg_task_*"	OFF)

if (SERIAL_INIT)
	add_definitions(-DSERIAL_INIT)
endif (SERIAL_INIT)

if (TASK_DIST)
	add_definitions(-DTASK_DISTRIBUTION)
endif (TASK_DIST)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qopenmp")
#MESSAGE( STATUS "CMAKE_C_FLAGS:         " ${CMAKE_C_FLAGS} )
if (TASK_PP)
	add_executable(
		cg_task_pp
		solver.c
		../common/def.c
		../common/errorcheck.c
		../common/help.c
		../common/io.c
		../common/main.c
		../common/mmio.c
		../common/output.c
	)
	set_target_properties(
		cg_task_pp
		PROPERTIES
		COMPILE_FLAGS
		"-DPAR_PRODUCER=1"
	)
	target_link_libraries(cg_task_pp m)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_task_pp DESTINATION bin)
endif (TASK_PP)

if (TASK_PP_DD)
	add_executable(
		cg_task_pp_dd
		solver.c
		../common/def.c
		../common/errorcheck.c
		../common/help.c
		../common/io.c
		../common/main.c
		../common/mmio.c
		../common/output.c
	)
	set_target_properties(
		cg_task_pp_dd
		PROPERTIES
		COMPILE_FLAGS
		"-DPAR_PRODUCER=1 -DDATA_DISTRIBUTION=1"
	)
	target_link_libraries(cg_task_pp_dd m)
	install(
		PROGRAMS
		${CMAKE_CURRENT_BINARY_DIR}/cg_task_pp_dd
		DESTINATION
		bin
	)
endif (TASK_PP_DD)

if (TASK_SP)
	add_executable(
		cg_task_sp
		solver.c
		../common/def.c
		../common/errorcheck.c
		../common/help.c
		../common/io.c
		../common/main.c
		../common/mmio.c
		../common/output.c
	)
	target_link_libraries(cg_task_sp m)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_task_sp DESTINATION bin)
endif (TASK_SP)

if (TASK_SP_AFF)
	add_executable(
		cg_task_sp_aff
		solver.c
		../common/def.c
		../common/errorcheck.c
		../common/help.c
		../common/io.c
		../common/main.c
		../common/mmio.c
		../common/output.c
	)
  set_target_properties(
    cg_task_sp_aff
    PROPERTIES
    COMPILE_FLAGS
    "-DTASK_AFF_DOMAIN_FIRST -DTASK_AFFINITY"
  )
	target_link_libraries(cg_task_sp_aff m)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_task_sp DESTINATION bin)
endif (TASK_SP_AFF)

if (TASK_SP_DD)
	add_executable(
		cg_task_sp_dd
		solver.c
		../common/def.c
		../common/errorcheck.c
		../common/help.c
		../common/io.c
		../common/main.c
		../common/mmio.c
		../common/output.c
	)
	set_target_properties(
		cg_task_sp_dd
		PROPERTIES
		COMPILE_FLAGS
		"-DDATA_DISTRIBUTION=1"
	)
	target_link_libraries(cg_task_sp_dd m)
	install(
		PROGRAMS
		${CMAKE_CURRENT_BINARY_DIR}/cg_task_sp_dd
		DESTINATION
		bin
	)
endif (TASK_SP_DD)
