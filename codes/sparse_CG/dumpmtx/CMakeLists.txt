add_executable(
	cg_dumpmtx
	main.c
	../common/mmio.c
	../common/io.c
)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_dumpmtx DESTINATION bin)
