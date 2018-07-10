find_package(OpenACC)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPENACC_FLAGS}")

add_executable(
	cg_openacc
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
target_link_libraries(cg_openacc m)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_openacc DESTINATION bin)
