add_definitions(-DUSE_OMP)

if (SERIAL_INIT)
	add_definitions(-DSERIAL_INIT)
endif (SERIAL_INIT)

find_package(MKL)

include_directories(${MKL_INCLUDE_DIR})
link_directories(${MKL_LIBRARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")

add_executable(
	cg_mkl
	kernel.c
	../common/def.c
	../common/help.c
	../common/io.c
	../common/main.c
	../common/mmio.c
	../common/output.c
	../common/solver.c
	../common/errorcheck.c
)
target_link_libraries(
	cg_mkl
#	mkl_solver_lp64
	mkl_intel_lp64
	mkl_intel_thread
	mkl_core
	m
)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_mkl DESTINATION bin)
