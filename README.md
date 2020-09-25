# task-affinity-codes
Benchmarks and scripts to test the experimental implementation for task affinity

## 1. List of benchmarks tested
The following benchmarks have been tested with and without affinity extension for the OpenMP task construct.

* **./codes/stream**                  
    - STREAM version that uses tasks instead of work sharing construct
* **./codes/bots/omp-tasks/sort**     
    - Parallel merge sort algorithm from bots benchmark suite
* **./codes/bots/omp-tasks/health**   
    - health benchmark from bots benchmark suite
* **./codes/sparse_CG/tasking**
    - CG solver that uses SPMXV

## 2. How to compile and run

#### 2.1 Clone repository that contains the customized runtime and switch to branch **task-affinity**  
```bash
git clone https://github.com/RWTH-HPC/openmp.git
cd openmp  
git checkout task-affinity  
```

#### 2.2 Move back to folder that contains **openmp** repo and clone this repository now  
```bash
cd ..
git clone https://github.com/RWTH-HPC/task-affinity-codes.git
cd task-affinity-codes
```

#### 2.3 Set the following environment variables. Then build the runtime  
```bash
export RUNTIME_DIR=$(pwd)/../openmp
export RUNTIME_AFF_REL_DIR=$(pwd)/runtime_affinity_rel
export RUNTIME_AFF_DEB_DIR=$(pwd)/runtime_affinity_deb
export RUNTIME_ORG_REL_DIR=$(pwd)/runtime_original_rel
export RUNTIME_ORG_DEB_DIR=$(pwd)/runtime_original_deb
make
```

#### 2.4 Set your C compiler either to intel or clang (Should be a compiler that is compatible with LLVM OpenMP runtime)
```bash
export CC=icc
```

#### 2.5 Change to the desired OpenMP runtime with or without task affinity (here with task affinity)
```bash
# make sure custom llvm library will be executed (set explicitly or use module system)
export LD_LIBRARY_PATH=${RUNTIME_AFF_REL_DIR}/lib:${LD_LIBRARY_PATH}
export LIBRARY_PATH=${RUNTIME_AFF_REL_DIR}/lib:${LIBRARY_PATH}

export INCLUDE=${RUNTIME_AFF_REL_DIR}/include:${INCLUDE}
export CPATH=${RUNTIME_AFF_REL_DIR}/include:${CPATH}
export C_INCLUDE_PATH=${RUNTIME_AFF_REL_DIR}/include:${C_INCLUDE_PATH}
export CPLUS_INCLUDE_PATH=${RUNTIME_AFF_REL_DIR}/include:${CPLUS_INCLUDE_PATH}
```

#### 2.6 Move to desired benchmark folder, compile and run the benchmark (Example here: Stream)
```bash
cd codes/stream
# this script will build and compare multiple versions
bash run_test.sh
```

**Note:** If you have linker issues complaining about task-affinity related symbols, you might need to add `-L${RUNTIME_AFF_REL_DIR}/lib` to your compile flags.