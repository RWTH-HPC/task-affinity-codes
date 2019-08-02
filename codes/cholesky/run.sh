#!/bin/bash
#SBARCH --nodes=1
#SBATCH --ntasks=24
#SBATCH --job-name=STREAM_TASK_AFFINITY_TEST
#SBATCH --output=sbatch_output.txt
#SBATCH --time=00:10:00

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=3
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=16
#export OMP_NUM_THREADS=284

export T_AFF_INVERTED=0
export THIRD_INVERTED=0
export T_AFF_SINGLE_CREATOR=1
export T_AFF_NUM_TASK_MULTIPLICATOR=16

export TASK_AFF_THREAD_SELECTION_STRATEGY=-1
export TASK_AFF_AFFINITY_MAP_MODE=-1
export TASK_AFF_PAGE_SELECTION_MODE=-1
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=-1
export TASK_AFF_NUMBER_OF_AFFINITIES=20

export MATRIX_SIZE=15000
export TITLE_SIZE=500
export CHECK_RESULTS=1

thread_selection_mode=( first random lowest_wl round_robin private )
map_mode=(thread domain)
page_selection_strategy=(first_page_of_first_affinity divide_in_n every_n_th first_and_last binary first)
page_weight_strategy=(first majority by_affinity size)

function set_up_affinity {
  export TASK_AFF_THREAD_SELECTION_STRATEGY=$1
  echo "\nThread selection strategy:\t ${thread_selection_mode[$1+1]}"
  export TASK_AFF_AFFINITY_MAP_MODE=$2
  echo "Map mode:\t\t\t ${map_mode[$2+1]}"
  export TASK_AFF_PAGE_SELECTION_MODE=$3
  echo "Page selection strategy:\t ${page_selection_strategy[$3+1]}"
  export TASK_AFF_PAGE_WEIGHTING_STRATEGY=$4
  echo "Page weight strategy:\t\t ${page_weight_strategy[$4+1]}"
  echo ""
}

for t in {1..24}                  #number of threads
do
  export OMP_NUM_THREADS=$t
  echo "\nNumber of threads:\t\t $t"
  for tsm in {0..4}               #Thread selecetion mode
  do
    for mm in {0..1}              #Map Mode
    do
      for pss in {0..5}           #Page sellection mode
      do
        for pws in {0..3}         #Page  Weight mode
        do
          set_up_affinity $tsm $mm $pss $pws
          ./ch_intel_aff ${MATRIX_SIZE} ${TITLE_SIZE} ${CHECK_RESULTS}
	        echo ""
        done
      done
    done
  done
done
