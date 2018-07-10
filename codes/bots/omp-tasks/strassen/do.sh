#!/bin/bash

#PROG_CMD="./strassen.exe -n 64"
PROG_CMD="./strassen.exe -n 4096"
#PROG_CMD="./strassen.exe -n 8192"
#PROG_CMD="./strassen.exe -n 16384"
#PROG_VERSION=deb
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=30 
export OMP_PLACES=cores 
export OMP_PROC_BIND=spread 
export OMP_NUM_THREADS=8
#export OMP_NUM_THREADS=32

module switch intel intel/18.0

function eval_run {
  echo "Executing affinity $1"
  make ${PROG_VERSION}."$1"
  no_numa_balancing "${PROG_CMD}" &> output_$1.txt
  grep "Elapsed" output_$1.txt
  grep "TASK AFFINITY:" output_$1.txt > bla_$1
  grep "stole task" output_$1.txt > nr_steals_$1
  grep "TASK_SUCCESSFULLY_PUSHED" output_$1.txt > pushed_$1
  grep "task_aff_stats" output_$1.txt > evol_$1
  grep "__kmp_task_start(enter_aff)" output_$1.txt > starts_$1
  grep "TASK_EXECUTION_TIME"  output_$1.txt > task_execution_times_$1
}

make clean
module load omp/task_aff.${PROG_VERSION}
eval_run "llvm"
eval_run "domain.lowest"
eval_run "domain.private"
eval_run "domain.rand"
eval_run "domain.round_robin"
eval_run "thread.lowest"
eval_run "thread.rand"
eval_run "thread.round_robin"



