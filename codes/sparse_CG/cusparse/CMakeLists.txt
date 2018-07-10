add_definitions(-DUSE_CUDA)

add_executable(
	cg_cusparse
	solver.c
	../common/def.c
	../common/errorcheck.c
	../common/help.c
	../common/io.c
	../common/main.c
	../common/mmio.c
	../common/output.c
)
target_link_libraries(cg_cusparse cudart cusparse cublas)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_cusparse DESTINATION bin)
