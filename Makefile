
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(dir $(MKFILE_PATH))
RUNTIME_DIR?=${CUR_DIR}/../openmp
BUILD_DIR=$(RUNTIME_DIR)/build
#CUR_COMPILER_OPTS=-DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icpc -DCMAKE_Fortran_COMPILER=ifort
CUR_COMPILER_OPTS=

# directories for final runtime
RUNTIME_AFF_REL_DIR ?= ${CUR_DIR}/runtime_affinity_rel
RUNTIME_AFF_DEB_DIR ?= ${CUR_DIR}/runtime_affinity_deb
RUNTIME_ORG_REL_DIR ?= ${CUR_DIR}/runtime_original_rel
RUNTIME_ORG_DEB_DIR ?= ${CUR_DIR}/runtime_original_deb

#all: orig.deb orig.rel task.deb task.rel
all: task.deb task.rel

all_begin:
	mkdir -p $(BUILD_DIR)

	mkdir -p $(RUNTIME_AFF_REL_DIR)
	mkdir -p $(RUNTIME_AFF_DEB_DIR)
	mkdir -p $(RUNTIME_ORG_REL_DIR)
	mkdir -p $(RUNTIME_ORG_DEB_DIR)

orig.deb: all_begin
	cmake -H"$(RUNTIME_DIR)" ${CUR_COMPILER_OPTS} -DCMAKE_INSTALL_PREFIX=$(RUNTIME_ORG_DEB_DIR) -DCMAKE_BUILD_TYPE=debug -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=OFF -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) omp -j4
	make -C $(BUILD_DIR) install
	
orig.rel: all_begin
	cmake -H"$(RUNTIME_DIR)" ${CUR_COMPILER_OPTS} -DCMAKE_INSTALL_PREFIX=$(RUNTIME_ORG_REL_DIR) -DCMAKE_BUILD_TYPE=release -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=OFF -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) omp -j4
	make -C $(BUILD_DIR) install
	
task.deb: all_begin
	cmake -H"$(RUNTIME_DIR)" ${CUR_COMPILER_OPTS} -DCMAKE_INSTALL_PREFIX=$(RUNTIME_AFF_DEB_DIR) -DCMAKE_BUILD_TYPE=debug -DLIBOMP_FORTRAN_MODULES=ON -DLIBOMP_USE_STDCPPLIB=ON -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=ON -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) omp -j4
	make -C $(BUILD_DIR) install

task.rel: all_begin
	cmake -H"$(RUNTIME_DIR)" ${CUR_COMPILER_OPTS} -DCMAKE_INSTALL_PREFIX=$(RUNTIME_AFF_REL_DIR) -DCMAKE_BUILD_TYPE=release -DLIBOMP_FORTRAN_MODULES=ON -DLIBOMP_USE_STDCPPLIB=ON -DLIBOMP_OMPT_SUPPORT=ON -DLIBOMP_USE_TASK_AFFINITY=ON -B"$(BUILD_DIR)" 
	make -C $(BUILD_DIR) rebuild_cache -j4
	make -C $(BUILD_DIR) omp -j4
	make -C $(BUILD_DIR) install