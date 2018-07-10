cmake_minimum_required(VERSION 2.8.5)

project(SparseCG)

option(SERIAL		"Build the serial version"			OFF)
option(OPENMP		"Build the OpenMP version"			OFF)
option(OPENMP_DD	"Build the OpenMP + data distribution version"	OFF)
option(MIC		"Build the MIC offload version"			OFF)
option(MIC_DD
	"Build the MIC offload + data distribution version"
	OFF
)
option(TASK_PP		"Build the OpenMP tasking PP version"		OFF)
option(TASK_PP_DD	"Build the OpenMP tasking PP + DD version"	OFF)
option(TASK_SP		"Build the OpenMP tasking SP version"		ON)
option(TASK_SP_AFF		"Build the OpenMP tasking SP version with affinity"		ON)
option(TASK_SP_DD	"Build the OpenMP tasking SP + DD version"	OFF)
option(MKL		"Build the MKL version"				OFF)
option(CUBLAS		"Build the cublas version"			OFF)
option(CUDA		"Build the CUDA version"			OFF)
option(CUSP		"Build the cusp version"			OFF)
option(CUSPARSE		"Build the cusparse version"			OFF)
option(VIENNACL		"Build the ViennaCL version"			OFF)
option(LIS		"Build the LIS version"				OFF)
option(OPENACC		"Build the OpenACC version"			OFF)
option(SERIAL_INIT	"Serial initialization (OPENMP, MKL, TASK)"	OFF)
option(STATIC_INIT	"Use static initialization (OPENMP, MKL, TASK)"	ON)
option(PAGE_ALIGN	"Serial initialization (OPENMP)"		OFF)
option(VECTOR_TIME	"Vector timings (OPENMP)"			OFF)
option(LIKWID_MARKER_API
	"Add instrumentation for the likwid marker api (OPENMP)"
	OFF
)
set( CMAKE_VERBOSE_MAKEFILE on )
set(CMAKE_MODULE_PATH
	"${CMAKE_SOURCE_DIR}/CMake"
	${CMAKE_MODULE_PATH}
)

if (
	OPENMP OR
	OPENMP_DD OR
	MIC OR
	MIC_DD OR
	TASK_PP OR
	TASK_PP_DD OR
	TASK_SP OR
	TASK_SP_AFF OR
	TASK_SP_DD OR
	MKL
)
	find_package(OpenMP)
endif (
	OPENMP OR
	OPENMP_DD OR
	MIC OR
	MIC_DD OR
	TASK_PP OR
	TASK_PP_DD OR
	TASK_SP OR
	TASK_SP_AFF OR
	TASK_SP_DD OR
	MKL
)

if (CUBLAS OR CUDA OR CUSPARSE)
	find_package(CUDA)
	include_directories("${CUDA_TOOLKIT_ROOT_DIR}/include")
	link_directories(
		"${CUDA_TOOLKIT_ROOT_DIR}/lib64"
		"${CUDA_TOOLKIT_ROOT_DIR}/lib"
	)
endif (CUBLAS OR CUDA OR CUSPARSE)

if (VIENNACL)
	find_package(OpenCL)
	include_directories("${OPENCL_INCLUDE_DIRS}")
endif (VIENNACL)

include_directories("${PROJECT_SOURCE_DIR}/common")

if (CUBLAS)
	add_subdirectory(cublas)
endif (CUBLAS)

if (CUDA)
	add_subdirectory(cuda)
endif (CUDA)

if (CUSP)
	add_subdirectory(cusp)
endif (CUSP)

if (CUSPARSE)
	add_subdirectory(cusparse)
endif (CUSPARSE)

if (SERIAL)
	add_subdirectory(serial)
endif (SERIAL)

#add_subdirectory(mic)

if (MKL)
	add_subdirectory(mkl)
endif (MKL)

if (OPENMP OR OPENMP_DD)
	add_subdirectory(openmp)
endif (OPENMP OR OPENMP_DD)

if (MIC OR MIC_DD)
	add_subdirectory(mic)
endif (MIC OR MIC_DD)


if (TASK_PP OR TASK_PP_DD OR TASK_SP OR TASK_SP_AFF OR TASK_SP_DD)
	add_subdirectory(tasking)
endif (TASK_PP OR TASK_PP_DD OR TASK_SP OR TASK_SP_AFF OR TASK_SP_DD)

if (VIENNACL)
	add_subdirectory(viennacl)
endif (VIENNACL)

if (LIS)
	add_subdirectory(lis)
endif (LIS)

if (OPENACC)
	add_subdirectory(openacc)
endif (OPENACC)

add_subdirectory(dumpmtx)
