#!/bin/bash

PROG_CMD=./stream_task.exe
PROG_VERSION=deb
NAME=$PROG_VERSION

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=3
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=3
#export OMP_NUM_THREADS=284

export T_AFF_INVERTED=0
export THIRD_INVERTED=0
export T_AFF_SINGLE_CREATOR=1
export T_AFF_NUM_TASK_MULTIPLICATOR=16
export STREAM_ARRAY_SIZE=$((2**27))

export THREAD_SELECTION_STRATEGY=-1
export AFFINITY_MAP_MODE=-1
export PAGE_SELECTION_MODE=-1
export PAGE_WEIGHTING_STRATEGY=-1
export NUMBER_OF_AFFINITIES=-1

thread_selection_mode=( first random lowest_wl round_robin private )
map_mode=(thread domain)
page_selection_strategy=(first_page_of_first_affinity divide_in_n every_n_th first_and_last binary first)
page_weight_strategy=(first majority by_affinity size)


module switch intel intel/18.0

function compile {
  echo "Compiling affinity ${curname}"
  make ${PROG_VERSION}"$1"
}

function run {
  no_numa_balancing "${PROG_CMD}" &> output_${NAME}.txt
  grep "Elapsed time" output_${NAME}.txt
  grep "T#0" output_${NAME}.txt > stats_${NAME}.txt
}


function compile_and_run {
  compile "$1"
  run "$1"
}

function set_up_affinity {
  #echo ""
  THREAD_SELECTION_STRATEGY=$1
  echo "Thread selection strategy:\t ${thread_selection_mode[$1+1]}"
  AFFINITY_MAP_MODE=$2
  echo "Map mode:\t\t\t ${map_mode[$2+1]}"
  PAGE_SELECTION_MODE=$3
  echo "Page selection strategy:\t ${page_selection_strategy[$3+1]}"
  PAGE_WEIGHTING_STRATEGY=$4
  echo "Page weight strategy:\t\t ${page_weight_strategy[$4+1]}"

  NAME=${PROG_VERSION}___${thread_selection_mode[$THREAD_SELECTION_STRATEGY + 1]}___${map_mode[$AFFINITY_MAP_MODE+1]}___${page_selection_strategy[$PAGE_SELECTION_MODE+1]}___${page_weight_strategy[$PAGE_WEIGHTING_STRATEGY+1]}___THREADS-$OMP_NUM_THREADS
  echo "${NAME}"
}

make clean
module unload omp
compile_and_run ".baseline"

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}
#run with default config
compile ".affinity"
echo "\n run on default"
run ".affinity"

#set_up_affinity 0 0 0 3
#run ".affinity"
#no_numa_balancing ./stream_task.exe

for t in {2..32}
do
    export OMP_NUM_THREADS=$t
    echo ""
    echo "Number of threads:\t\t $t"
    set_up_affinity 3 0 0 2
    run ".affinity"
done

for tsm in {0..4}
do
  for mm in {0..1}
    do
    for pss in {0..5}
    do
      for pws in {0..3}
      do
        #set_up_affinity $tsm $mm $pss $pws
        #run ".affinity"
	#no_numa_balancing ./stream_task.exe
      done
    done
  done
done
