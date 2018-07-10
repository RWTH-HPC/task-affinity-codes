add_definitions(-DUSE_CUDA)

cuda_add_executable(
	cg_cusp
	cg.cu
	../common/def.cu
	../common/output.c
)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_cusp DESTINATION bin)
