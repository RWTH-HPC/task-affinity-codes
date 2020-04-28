#!/usr/local_rwth/bin/zsh
#SBATCH --account=jara0001
#SBATCH --partition=c16s
#SBATCH --exclusive
#SBARCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=144
#SBATCH --job-name=CHOLESKY_TASK_AFFINITY_TEST
#SBATCH --output=sbatch.txt
#SBATCH --time=05:30:00

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=4
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=64

export TASK_AFF_THREAD_SELECTION_STRATEGY=2
export TASK_AFF_AFFINITY_MAP_MODE=1
#export TASK_AFF_PAGE_SELECTION_MODE=3
export TASK_AFF_PAGE_SELECTION_MODE=5
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=1
export TASK_AFF_NUMBER_OF_AFFINITIES=15

export MATRIX_SIZE=45000
export TITLE_SIZE=500
export CHECK_RESULTS=0

module use -a ~/.modules
module load omp/task_aff.rel
#module load intelixe
#module load ddt

#no_numa_balancing 
#ddt ch_intel_aff ${MATRIX_SIZE} ${TITLE_SIZE} ${CHECK_RESULTS}
no_numa_balancing ch_intel ${MATRIX_SIZE} ${TITLE_SIZE} ${CHECK_RESULTS} &> res_intel
no_numa_balancing ch_intel_aff ${MATRIX_SIZE} ${TITLE_SIZE} ${CHECK_RESULTS} &> res_intel_aff
#inspxe-gui
