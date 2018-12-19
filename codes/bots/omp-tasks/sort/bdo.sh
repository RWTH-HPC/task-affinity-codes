#!/bin/bash
#BSUB -P thes0466
#BSUB -W 15:00
#BSUB -m c144m1024
#BSUB -a openmp
#BSUB -n 1
#BSUB -x
#BSUB -o batch_%J/sort_%J
#BSUB -J "bots_sort"
#BSUB -M 1020000

lscpu
mkdir batch_$LSB_JOBID
cd batch_$LSB_JOBID

#PROG_CMD="./strassen.exe -n 2048"
#PROG_CMD="./strassen.exe -n 4096"
#PROG_CMD="./sort.exe -n 33554432"
#PROG_CMD="./sort.exe -n $((2**31))"
PROG_CMD="../sort.exe -n $((2**31))"
#PROG_CMD="./sort.exe -n 33554432"
#PROG_VERSION=deb
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=60
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=64
#export OMP_NUM_THREADS=24

module switch intel intel/18.0

function eval_run {
    if [ -n "$2" ] && [ -n "$3" ]; then
      curname="$1.$4.$3"
  else
      curname=$1
    fi
  echo "Executing affinity ${curname}"
  make -C ../ ${PROG_VERSION}."$1" sched=$2 num=$3
  no_numa_balancing "${PROG_CMD}" &>> output_${curname}.txt #-c for verification
  grep "Elapsed" output_${curname}.txt
  tac output_${curname}.txt | grep -m 1 "Elapsed" >> time_${curname}.txt
  #grep "Verification" output_${curname}.txt
  #grep "TASK AFFINITY:" output_$1.txt > bla_$1
  #grep "stole task" output_$1.txt > nr_steals_$1
  #grep "TASK_SUCCESSFULLY_PUSHED" output_$1.txt > pushed_$1
  #grep "task_aff_stats" output_$1.txt > evol_$1
  #grep "__kmp_task_start(enter_aff)" output_$1.txt > starts_$1
  #grep "TASK_EXECUTION_TIME"  output_$1.txt > task_execution_times_$1
}

make clean
module unload omp

# for i in {1..10}; do
# for i in 008 0016 032 048 064 080 104 128 144; do
# eval_run "gcc"
# eval_run "llvm"
# done
# done

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}
make -C ~ task.${PROG_VERSION}
if [[ $? -ne 0 ]] ; then
    exit 1
fi

#eval_run "llvm"
#eval_run "gcc"
#eval_run "domain.lowest"
#eval_run "domain.rand"
#eval_run "domain.round_robin"
#eval_run "thread.lowest"
#eval_run "thread.rand"
#eval_run "thread.round_robin"

#STRATS NAME TO NUMBER CONVERTER
first1=0
first=5
divn=1
divn2=12
step=2
fal=3
bin=4

first1=00
none=01
aff=02
size=03
#divn 1, step 2, fal 3, first 0
#none 1, aff 2, size 3, first 0

for j in {1..10}; do

export OMP_NUM_THREADS=64
#

for i in 008 0016 032 048 064 080 104 128 144; do
export OMP_NUM_THREADS=$i

eval_run "domain.lowest" $first1$first1 1 "D_baseline_T$i"
eval_run "thread.lowest" $first1$first1 1 "T_baseline_T$i"
eval_run "domain.lowest" $fal$none 2 "D_F&L_T$i"
eval_run "thread.lowest" $fal$none 2 "T_F&L_T$i"
eval_run "domain.lowest" $bin$none 10 "D_Bin_T$i"
eval_run "domain.lowest" $step$none 512 "D_Step(512)_T$i"
eval_run "thread.lowest" $step$none 512 "T_Step(512)_T$i"

done

done
