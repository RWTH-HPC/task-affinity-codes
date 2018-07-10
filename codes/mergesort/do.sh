#!/bin/bash

#ARR_SIZE=120000000
ARR_SIZE=$((2**24))
#ARR_SIZE=$((2**30))
#ARR_SIZE=$((2**33))
PROG_CMD="./MergeSort.exe ${ARR_SIZE}"
#PROG_VERSION=deb
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
#export KMP_A_DEBUG=30 
export OMP_PLACES=cores 
export OMP_PROC_BIND=spread 
export OMP_NUM_THREADS=12
#export OMP_NUM_THREADS=128
export T_AFF_NUMBER_TASKS_MULTIPLICATOR=6

module switch intel intel/18.0

#source /home/jk869269/util/bash/ompGetCoresForSpread.sh
#TMP_CORES=$(ompGetCoresForSpread ${OMP_NUM_THREADS})
#module load likwid

function eval_run {
  make ${PROG_VERSION}."$1"
  
  curname=$1
  if ! [ -z "$2" ]; then
    curname="${curname}.$2"
  fi  
  echo "Executing affinity $1"
  
  #{ timex -v no_numa_balancing likwid-perfctr -g NUMA -c ${TMP_CORES} -f -O -o likwid_${curname}.csv "${PROG_CMD}" ; } &> output_${curname}.txt
  #{ timex -v no_numa_balancing likwid-perfctr -g TASKAFFINITY -c ${TMP_CORES} -O -o likwid_${curname}.csv "${PROG_CMD}" ; } &> output_${curname}.txt
  #{ timex -v no_numa_balancing likwid-perfctr -g QPI -c ${TMP_CORES} -f -O -o likwid_${curname}.csv "${PROG_CMD}" ; } &> output_${curname}.txt
  #{ timex -v no_numa_balancing likwid-perfctr -g CYCLE_ACTIVITY -c ${TMP_CORES} -f -O -o likwid_${curname}.csv "${PROG_CMD}" ; } &> output_${curname}.txt
  #{ timex -v no_numa_balancing likwid-perfctr -g FALSE_SHARE -c ${TMP_CORES} -f -O -o likwid_${curname}.csv "${PROG_CMD}" ; } &> output_${curname}.txt
  #{ timex -v no_numa_balancing likwid-perfctr -g FLOPS_DP -c 0,3,6,9,12,15,18,21 -f -O -o likwid_${curname}.csv "${PROG_CMD}" ; } &> output_${curname}.txt
  #{ timex -v no_numa_balancing likwid-perfctr -g MEM_DP -c 0,3,6,9,12,15,18,21 -f -O -o likwid_${curname}.csv "${PROG_CMD}" ; } &> output_${curname}.txt
  #sed -i 's/,/\t/g' likwid_${curname}.csv
  #sed -i 's/\./,/g' likwid_${curname}.csv
  
  #no_numa_balancing advixe-cl -collect roofline -project-dir TestRoofline -- "${PROG_CMD}" &> output_${curname}.txt
  no_numa_balancing "${PROG_CMD}" &> output_${curname}.txt
  grep "Elapsed time" output_${curname}.txt
  grep "TASK AFFINITY:" output_${curname}.txt > bla_${curname}
  #grep "stole task" output_${curname}.txt > nr_steals_${curname}
  #grep "TASK_SUCCESSFULLY_PUSHED" output_${curname}.txt > pushed_${curname}
  #grep "task_aff_stats" output_${curname}.txt > evol_${curname}
  #grep "__kmp_task_start(enter_aff)" output_${curname}.txt > starts_${curname}
  grep "TASK_EXECUTION_TIME"  output_${curname}.txt > task_execution_times_${curname}
}

make clean
module load omp/task_aff.${PROG_VERSION}
eval_run "llvm"
eval_run "domain.lowest"
#eval_run "domain.private"
#eval_run "domain.rand"
#eval_run "domain.round_robin"
#eval_run "thread.lowest"
#eval_run "thread.rand"
#eval_run "thread.round_robin"



