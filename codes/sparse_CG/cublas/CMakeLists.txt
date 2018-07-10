add_definitions(-DUSE_CUDA)

cuda_add_executable(
	cg_cublas
	solver.cu
	../common/def.cu
	../common/errorcheck.cu
	../common/help.cu
	../common/io.c
	../common/main.cu
	../common/mmio.c
	../common/output.c
)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_cublas DESTINATION bin)
