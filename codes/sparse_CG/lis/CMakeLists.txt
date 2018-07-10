add_definitions(-DLIS)
add_definitions(-DUSE_OMP)

find_package(Lis)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")

include_directories(${LIS_INCLUDE_DIR})
link_directories(${LIS_LIBRARY_DIR})

option(USE_LIS_IO "Use lis for IO in the lis version" OFF)

if (USE_LIS_IO)
	add_definitions(-DUSE_LIS_IO)
endif (USE_LIS_IO)

add_executable(
	cg_lis
	main.c
	../common/def.c
	../common/errorcheck.c
	../common/help.c
	../common/io.c
	../common/mmio.c
	../common/output.c
)
target_link_libraries(cg_lis lis)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cg_lis DESTINATION bin)
