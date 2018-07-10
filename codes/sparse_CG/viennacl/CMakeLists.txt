add_executable(
	cg_viennacl
	main.cxx
	../common/def.c
	../common/io.c
	../common/mmio.c
	../common/output.c
)
include_directories("${PROJECT_SOURCE_DIR}/viennacl")
target_link_libraries(
	cg_viennacl
	${OPENCL_LIBRARIES}
)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_viennacl DESTINATION bin)
