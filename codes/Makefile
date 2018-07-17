
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(dir $(MKFILE_PATH))
RUNTIME_DIR=${CUR_DIR}/llvm-openmp/runtime
BUILD_DIR=$(RUNTIME_DIR)/build
#CUR_COMPILER_OPTS=-DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icpc -DCMAKE_Fortran_COMPILER=ifort
CUR_COMPILER_OPTS=

#all: orig.deb orig.rel task.deb task.rel
all: task.deb task.rel

#cmake -H"$(RUNTIME_DIR)" -DCMAKE_BUILD_TYPE=debug -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=ON -B"$(BUILD_DIR)" -DLIBOMP_CFLAGS=-fsanitize=thread -DLIBOMP_CPPFLAGS=-fsanitize=thread -DLIBOMP_CXXFLAGS=-fsanitize=thread -DLIBOMP_LDFLAGS=-fsanitize=thread -DLIBOMP_TSAN_SUPPORT=ON

all_begin:
	mkdir -p $(BUILD_DIR)

orig.deb: all_begin
	cmake -H"$(RUNTIME_DIR)" -DCMAKE_BUILD_TYPE=debug -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=OFF -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) -j4
	
	cp -v $(BUILD_DIR)/src/lib* ~/install/llvm-openmp/task_aff_orig.deb/lib/
	cp -v $(BUILD_DIR)/src/omp* ~/install/llvm-openmp/task_aff_orig.deb/include/
	
orig.rel: all_begin
	cmake -H"$(RUNTIME_DIR)" -DCMAKE_BUILD_TYPE=release -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=OFF -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) -j4
	
	cp -v $(BUILD_DIR)/src/lib* ~/install/llvm-openmp/task_aff_orig.rel/lib/
	cp -v $(BUILD_DIR)/src/omp* ~/install/llvm-openmp/task_aff_orig.rel/include/

task.deb: all_begin
	cmake -H"$(RUNTIME_DIR)" ${CUR_COMPILER_OPTS} -DCMAKE_BUILD_TYPE=debug -DLIBOMP_FORTRAN_MODULES=ON -DLIBOMP_USE_STDCPPLIB=ON -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=ON -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) -j4
	
	cp -v $(BUILD_DIR)/src/lib* ~/install/llvm-openmp/task_aff.deb/lib/
	cp -v $(BUILD_DIR)/src/omp* ~/install/llvm-openmp/task_aff.deb/include/

task.rel: all_begin
	cmake -H"$(RUNTIME_DIR)" ${CUR_COMPILER_OPTS} -DCMAKE_BUILD_TYPE=release -DLIBOMP_FORTRAN_MODULES=ON -DLIBOMP_USE_STDCPPLIB=ON -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=ON -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) -j4
	
	cp -v $(BUILD_DIR)/src/lib* ~/install/llvm-openmp/task_aff.rel/lib/
	cp -v $(BUILD_DIR)/src/omp* ~/install/llvm-openmp/task_aff.rel/include/
