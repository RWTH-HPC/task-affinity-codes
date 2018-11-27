#!/bin/bash
#BSUB -P thes0466
#BSUB -W 01:00
#BSUB -m c144m1024
#BSUB -a openmp
#BSUB -n 1
#BSUB -x
#BSUB -o output_batch
#BSUB -J spmxvB
#BSUB -M 524288

#PROG_CMD="./solver.exe ../DATA/G3_circuit.mtxdump"
#PROG_CMD="./solver.exe ../DATA/Serena.mtxdump"
#PROG_CMD="./solver.exe ../DATA/mat235M.mtxdump"
PROG_CMD="./solver.exe ../DATA/2.mtx"
#PROG_CMD="./solver.exe ../DATA/HV15R.mtxdump"
#PROG_VERSION=deb
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
#export KMP_A_DEBUG=30
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
#export OMP_NUM_THREADS=12
export OMP_NUM_THREADS=4

# additionally specify number of tasks
export CG_NUM_TASKS=$(($OMP_NUM_THREADS * 16))//16
export PAR_PRODUCER=1

module switch intel intel/18.0

#source /home/jk869269/util/bash/ompGetCoresForSpread.sh
#TMP_CORES=$(ompGetCoresForSpread ${OMP_NUM_THREADS})
#module load likwid

function eval_run {
  make ${PROG_VERSION}."$1" sched=$2 num=$3

  curname=$1
  if [ -n "$2" ] && [ -n "$3" ]; then
    curname="$1.$4.$3"
  fi
  echo "Executing affinity ${curname}"

  #{ timex -v likwid-perfctr -f -g NUMA -c ${TMP_CORES} -O -f -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  #{ timex -v likwid-perfctr -f -g CYCLE_ACTIVITY -c ${TMP_CORES} -O -f -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  #sed -i 's/,/\t/g' likwid_${curname}.csv
  #sed -i 's/\./,/g' likwid_${curname}.csv

  #no_numa_balancing numamem -s 1 "${PROG_CMD}" &> output_${curname}.txt
  no_numa_balancing "${PROG_CMD}" &> output_${curname}.txt
  grep "Elapsed time" output_${curname}.txt
  #grep "TASK AFFINITY:" output_${curname}.txt > bla_${curname}
  #grep "stole task" output_${curname}.txt > nr_steals_${curname}
  #grep "TASK_SUCCESSFULLY_PUSHED" output_${curname}.txt > pushed_${curname}
  #grep "task_aff_stats" output_${curname}.txt > evol_${curname}
  #grep "__kmp_task_start(enter_aff)" output_${curname}.txt > starts_${curname}
  #grep "TASK_EXECUTION_TIME"  output_${curname}.txt > task_execution_times_${curname}
}

make clean
module unload omp
#eval_run "gcc"
#eval_run "llvm" "" "intel"
#eval_run "baseline"

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}
#make -C ~ task.${PROG_VERSION}
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

eval_run "domain.lowest" $divn3$none 5 "divn3_size"
eval_run "domain.lowest" $fal$none 2 "fal_none"
#eval_run "domain.lowest" $first0$first 1 "first0_first"
#eval_run "domain.lowest" $divn2$size 20 "divn2_size"

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
