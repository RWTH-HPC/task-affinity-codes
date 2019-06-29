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
#export T_AFF_NUM_TASK_MULTIPLICATOR=4
export T_AFF_NUM_TASK_MULTIPLICATOR=16
#export STREAM_ARRAY_SIZE=$((2**21))
export STREAM_ARRAY_SIZE=$((2**27))

module switch intel intel/18.0

#source /home/jk869269/util/bash/ompGetCoresForSpread.sh
#TMP_CORES=$(ompGetCoresForSpread ${OMP_NUM_THREADS})
#module load likwid

function eval_run {
  if [ -n "$2" ] && [ -n "$3" ] &&  [ -n "$4" ]; then
    curname="$1.$5.$4"
else
    curname=$1
  fi
  echo "Executing affinity ${curname}"
  make ${PROG_VERSION}."$1" page_select_strategy=$2 page_weight_strategy=$3 num=$4

  # { timex -v likwid-perfctr -f -g NUMA -c ${TMP_CORES} -O -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  # { timex -v likwid-perfctr -f -g TASKAFFINITY -c ${TMP_CORES} -O -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  # { timex -v likwid-perfctr -f -g QPI -c ${TMP_CORES} -O -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  # { timex -v likwid-perfctr -f -g CYCLE_ACTIVITY -c ${TMP_CORES} -O -o likwid_cycle_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  #sed -i 's/,/\t/g' likwid_${curname}.csv
  #sed -i 's/\./,/g' likwid_${curname}.csv
  #sed -i 's/,/\t/g' likwid_cycle_${curname}.csv
  #sed -i 's/\./,/g' likwid_cycle_${curname}.csv

  #no_numa_balancing advixe-cl -collect roofline -project-dir TestRoofline -- "${PROG_CMD}" &> output_${curname}.txt
  no_numa_balancing "${PROG_CMD}" &> output_${curname}.txt
  grep "Elapsed time" output_${curname}.txt
  #rm stats_*
  grep "T#0" output_${curname}.txt > stats_${curname}.txt
  # grep "count_max_data_affinity_len_Ofpages" output_${curname}.txt >> stats_${curname}.txt
  # grep "map_find" output_${curname}.txt >> stats_${curname}.txt
  # grep "map_insert" output_${curname}.txt >> stats_${curname}.txt
  # grep "map_overall" output_${curname}.txt >> stats_${curname}.txt
  # grep "count_map_found" output_${curname}.txt >> stats_${curname}.txt
  # grep "count_map_not_found" output_${curname}.txt >> stats_${curname}.txt
  # grep "time_identify_physical_location" output_${curname}.txt >> stats_${curname}.txt
  # grep "time_strategy1" output_${curname}.txt >> stats_${curname}.txt
  # grep "time_strategy2" output_${curname}.txt >> stats_${curname}.txt
  #grep "TASK AFFINITY:" output_${curname}.txt > bla_${curname}
  #grep "stole task" output_${curname}.txt > nr_steals_${curname}
  #grep "TASK_SUCCESSFULLY_PUSHED" output_${curname}.txt > pushed_${curname}
  #grep "task_aff_stats" output_${curname}.txt > evol_${curname}
  #grep "__kmp_task_start(enter_aff)" output_${curname}.txt > starts_${curname}
  #grep "TASK_EXECUTION_TIME"  output_${curname}.txt > task_execution_times_${curname}
}

make clean
module unload omp
#eval_run "baseline"
#eval_run "llvm" "" "intel"

# module switch intel gcc/7
# eval_run "gcc"
# module switch gcc intel/18.0

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}
#make -C ~ task.${PROG_VERSION}
if [[ $? -ne 0 ]] ; then
    exit 1
fi

#eval_run "llvm"
#eval_run "domain.lowest"
#eval_run "domain.private"
#eval_run "domain.rand"
#eval_run "domain.round_robin"
#eval_run "thread.lowest"
#eval_run "thread.rand"
#eval_run "thread.round_robin"

#STRATS NAME TO NUMBER CONVERTER
first1=0
divn=1
step=2
fal=3
bin=4
first=5

first1=0
none=1
aff=2
size=3
#divn 1, step 2, fal 3, first 0
#none 1, aff 2, size 3, first 0

eval_run "domain.lowest" $first1 $first1 1 "first1_first1"
eval_run "thread.lowest" $first1 $first1 1 "first1_first1"
eval_run "domain.lowest" $first $none 1 "first_none"
eval_run "thread.lowest" $first $none 1 "first_none"
# eval_run "domain.lowest" $bin$none 10 "bin_none"

# eval_run "domain.lowest" $divn$first 2 "divn_first"
# eval_run "domain.lowest" $divn$none 4 "divn_none"
# eval_run "domain.lowest" $divn$aff 4 "divn_aff"
# eval_run "domain.lowest" $divn$size 4 "divn_size"
# eval_run "domain.lowest" $divn$size 12 "divn_size"
eval_run "domain.lowest" $divn $none 12 "divn_none"
eval_run "thread.lowest" $divn $none 12 "divn_none"
#
#
# eval_run "thread.lowest" $divn2$aff2 12 "divn2_aff2"
# eval_run "domain.lowest" $fal$size 12 "fal_size"
#
# eval_run "domain.lowest" $step$first 100 "step_first"
eval_run "domain.lowest" $step $none 100 "step_none"
eval_run "thread.lowest" $step $none 100 "step_none"
# eval_run "domain.lowest" $step$aff 100 "step_aff"
# eval_run "domain.lowest" $step$size 100 "step_size"
#
# eval_run "domain.lowest" $divn2$first 100 "divn2_first"
# eval_run "domain.lowest" $divn2$none 100 "divn2_none"
# eval_run "domain.lowest" $divn2$aff 100 "divn2_aff"
# eval_run "domain.lowest" $divn2$size 100 "divn2_size"

# eval_run "domain.lowest" $fal$first 2 "fal_first"
eval_run "domain.lowest" $fal $none 2 "fal_none"
eval_run "thread.lowest" $fal $none 2 "fal_none"
# eval_run "domain.lowest" $fal$aff 10 "fal_aff"
# eval_run "domain.lowest" $fal$size 10 "fal_size"
