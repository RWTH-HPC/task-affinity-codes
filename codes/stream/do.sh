#!/bin/bash

PROG_CMD=./stream_task.exe
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=2
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=64
#export OMP_NUM_THREADS=284

export T_AFF_INVERTED=0
export THIRD_INVERTED=0
export T_AFF_SINGLE_CREATOR=1
export T_AFF_NUM_TASK_MULTIPLICATOR=16
export STREAM_ARRAY_SIZE=$((2**27))

export THREAD_SELECTION_STRATEGY=-1
export AFFINITY_MAP_MODE=-1
export PAGE_SELCTION_MODE=-1
export PAGE_WEIGHTING_STRATEGY=-1
export NUMBER_OF_AFFINITIES=-1


module switch intel intel/18.0

#source /home/jk869269/util/bash/ompGetCoresForSpread.sh
#TMP_CORES=$(ompGetCoresForSpread ${OMP_NUM_THREADS})
#module load likwid

function eval_run {
  echo "Executing affinity ${curname}"
  make ${PROG_VERSION}"$1"
  no_numa_balancing "${PROG_CMD}" &> output_${curname}.txt
  grep "Elapsed time" output_${curname}.txt
  grep "T#0" output_${curname}.txt > stats_${curname}.txt
}

make clean
module unload omp
eval_run ".baseline"

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}

eval_run ""
