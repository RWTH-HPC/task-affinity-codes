#!/bin/zsh
#SBATCH --account=jara0001
#SBATCH --partition=c16s
#SBARCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=STREAM_TASK_AFFINITY_TEST
#SBATCH --output=sbatch_output.txt
#SBATCH --time=00:01:00
#SBATCH --exclusive


module unload omp
make clean
module use -a ~/.modules
module load omp/task_aff.deb
#module load omp/task_aff.rel
module load intelvtune
module load ddt

make deb.affinity
#make rel.affinity


export KMP_TASK_STEALING_CONSTRAINT=0
#export KMP_A_DEBUG=3
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=16

export T_AFF_INVERTED=0
export THIRD_INVERTED=0
export T_AFF_SINGLE_CREATOR=1
export T_AFF_NUM_TASK_MULTIPLICATOR=16
export STREAM_ARRAY_SIZE=$((2**30))

export TASK_AFF_THREAD_SELECTION_STRATEGY=2
export TASK_AFF_AFFINITY_MAP_MODE=0
export TASK_AFF_PAGE_SELECTION_MODE=0
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=1
export TASK_AFF_NUMBER_OF_AFFINITIES=20

#amplxe-gui

#./stream_task.exe &> thread.txt

export TASK_AFF_THREAD_SELECTION_STRATEGY=2
export TASK_AFF_AFFINITY_MAP_MODE=1
export TASK_AFF_PAGE_SELECTION_MODE=0
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=0
export TASK_AFF_NUMBER_OF_AFFINITIES=20
export TASK_AFF_THRESHOLD=0.3

#amplxe-gui
#./stream_task.exe &> domain.txt
ddt
