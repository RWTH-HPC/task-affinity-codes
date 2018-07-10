#!/bin/bash

#PROG_CMD="./fft1.exe -n $((2**25))"
#PROG_CMD="./fft1.exe -n 134217728"
PROG_CMD="./fft.exe"
#PROG_VERSION=deb
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
export OMP_PLACES=cores 
export OMP_PROC_BIND=spread 
#export OMP_NUM_THREADS=8
export OMP_NUM_THREADS=24
#export KMP_A_DEBUG=30 

module switch intel intel/18.0

function eval_run {
  curname=$1
  if ! [ -z "$2" ]; then
    curname="${curname}.$2"
  fi
  echo "Executing affinity ${curname}"
  make ${PROG_VERSION}."$1"
  
  echo "${PROG_CMD}"
  #no_numa_balancing ${PROG_CMD} &> output_${curname}.txt
  ${PROG_CMD} &> output_${curname}.txt
  #${PROG_CMD} &> output_abc.txt
  #grep "Elapsed time" output_${curname}.txt
  #grep "TASK AFFINITY:" output_${curname}.txt > bla_${curname}
  #grep "TASK_EXECUTION_TIME"  output_${curname}.txt > task_execution_times_${curname} 
}

make clean
module load omp/task_aff.${PROG_VERSION}
eval_run "llvm"
eval_run "domain.lowest"
#eval_run "domain.rand"
#eval_run "domain.round_robin"
#eval_run "thread.lowest"
#eval_run "thread.rand"
#eval_run "thread.round_robin"



