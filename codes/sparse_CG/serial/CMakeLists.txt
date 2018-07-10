add_executable(
	cg_ser
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
target_link_libraries(cg_ser m)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_ser DESTINATION bin)
