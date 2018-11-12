#!/bin/bash
#BSUB -P thes0466
#BSUB -W 01:00
#BSUB -m c144m1024
#BSUB -a openmp
#BSUB -n 1
#BSUB -x
#BSUB -o output_batch
#BSUB -J sortB
#BSUB -M 524288

#PROG_CMD="./strassen.exe -n 2048"
#PROG_CMD="./strassen.exe -n 4096"
#PROG_CMD="./sort.exe -n 33554432"
#PROG_CMD="./sort.exe -n $((2**31))"
PROG_CMD="./sort.exe -n $((2**26))"
#PROG_CMD="./sort.exe -n 33554432"
#PROG_VERSION=deb
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=30
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=64
#export OMP_NUM_THREADS=24

module switch intel intel/18.0

function eval_run {
  curname="$1.$4.$3"
  echo "Executing affinity ${curname}"
  make ${PROG_VERSION}."$1" sched=$2 num=$3
  no_numa_balancing "${PROG_CMD}" -c &> output_${curname}.txt
  grep "Elapsed" output_${curname}.txt
  grep "Verification" output_${curname}.txt
  #grep "TASK AFFINITY:" output_$1.txt > bla_$1
  #grep "stole task" output_$1.txt > nr_steals_$1
  #grep "TASK_SUCCESSFULLY_PUSHED" output_$1.txt > pushed_$1
  #grep "task_aff_stats" output_$1.txt > evol_$1
  #grep "__kmp_task_start(enter_aff)" output_$1.txt > starts_$1
  #grep "TASK_EXECUTION_TIME"  output_$1.txt > task_execution_times_$1
}

make clean
module unload omp
eval_run "gcc"
eval_run "llvm"

make -C ~ task.${PROG_VERSION}
if [[ $? -ne 0 ]] ; then
    exit 1
fi
module use -a ~/.modules

module load omp/task_aff.${PROG_VERSION}
#eval_run "llvm"
#eval_run "gcc"
#eval_run "domain.lowest"
#eval_run "domain.rand"
#eval_run "domain.round_robin"
#eval_run "thread.lowest"
#eval_run "thread.rand"
#eval_run "thread.round_robin"

#STRATS NAME TO NUMBER CONVERTER
first=0
first0=99
divn=1
divn2=11
divn3=12
divn_old=19
step=2
step2=21
fal=3
bin=4

first=00
none=01
aff=02
size=03
size2=31
#divn 1, step 2, fal 3, first 0
#none 1, aff 2, size 3, first 0
eval_run "domain.lowest" $divn$size 10 "divn_size"
eval_run "domain.lowest" $divn3$size 10 "divn3_size"

eval_run "thread.lowest" $divn$none 10 "divn_none"
eval_run "domain.rand" $divn$none 10 "divn_none"

: << 'COMT'
eval_run "domain.lowest" $first$first 10 "first_first"
eval_run "domain.lowest" $bin$none 10 "bin.none"

eval_run "domain.lowest" $divn$first 10 "divn_first"
eval_run "domain.lowest" $divn$none 10 "divn_none"

eval_run "thread.lowest" $divn$none 10 "divn_none"
eval_run "domain.rand" $divn$none 10 "divn_none"

eval_run "domain.lowest" $divn$aff 10 "divn_aff"

eval_run "domain.lowest" $divn$size 1 "divn_size"
eval_run "domain.lowest" $divn$size 2 "divn_size"
eval_run "domain.lowest" $divn$size 4 "divn_size"
eval_run "domain.lowest" $divn$size 8 "divn_size"
eval_run "domain.lowest" $divn$size 16 "divn_size"
eval_run "domain.lowest" $divn$size 32 "divn_size"
eval_run "domain.lowest" $divn$size 64 "divn_size"
eval_run "domain.lowest" $divn$size 100 "divn_size"

eval_run "domain.lowest" $step$first 10 "step_first"
eval_run "domain.lowest" $step$none 10 "step_none"
eval_run "domain.lowest" $step$aff 10 "step_aff"
eval_run "domain.lowest" $step$size 10 "step_size"

eval_run "domain.lowest" $fal$first 10 "fal_first"
eval_run "domain.lowest" $fal$none 10 "fal_none"
eval_run "domain.lowest" $fal$aff 10 "fal_aff"
eval_run "domain.lowest" $fal$size 10 "fal_size"
COMT
