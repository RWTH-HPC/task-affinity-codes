#!/bin/zsh

#SBATCH --account=jara0001
#SBATCH --partition=c16s
#SBARCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=STREAM_TASK_AFFINITY_TEST
#SBATCH --output=sbatch_output.txt
#SBATCH --time=00:10:00
#SBATCH --exclusive


PROG_CMD=./stream_task.exe
PROG_VERSION=rel
#PROG_VERSION=deb
NAME=$PROG_VERSION

export KMP_TASK_STEALING_CONSTRAINT=0
#export KMP_A_DEBUG=2
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=64
#export OMP_NUM_THREADS=284

export T_AFF_INVERTED=0
export THIRD_INVERTED=0
export KMP_AFFINITY=verbose
export T_AFF_SINGLE_CREATOR=1
export T_AFF_NUM_TASK_MULTIPLICATOR=16
#export STREAM_ARRAY_SIZE=$((2**27))
export STREAM_ARRAY_SIZE=$((2**31))

export TASK_AFF_THREAD_SELECTION_STRATEGY=-1
export TASK_AFF_AFFINITY_MAP_MODE=-1
export TASK_AFF_PAGE_SELECTION_MODE=-1
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=-1
export TASK_AFF_NUMBER_OF_AFFINITIES=1


thread_selection_mode=( first random lowest_wl round_robin private )
map_mode=(thread domain combined)
page_selection_strategy=(first_page_of_first_affinity divide_in_n every_n_th first_and_last binary first)
page_weight_strategy=(first majority by_affinity size)


module switch intel intel/18.0

function compile {
  echo "Compiling affinity $1"
  make ${PROG_VERSION}"$1"
}

function run {
  no_numa_balancing "${PROG_CMD}" &> output_${NAME}.txt
  sed -i 's/\./\,/g' output_${NAME}.txt
  grep "Elapsed time" output_${NAME}.txt
  grep ": corr_domain" output_${NAME}.txt > output_corr_domain_${NAME}.txt
  grep ": in_corr_domain" output_${NAME}.txt > output_in_corr_domain_${NAME}.txt
  #grep "_affinity_schedule" output_${NAME}.txt
  #grep "combined_map_strat" output_${NAME}.txt > combined_map_stats_${NAME}.txt
  #grep "T#0" output_${NAME}.txt > stats_${NAME}.txt
  #grep "combined_map" stats_${NAME}.txt
}


function compile_and_run {
  compile "$1"
  run "$1"
}

function set_up_affinity {
  export TASK_AFF_THREAD_SELECTION_STRATEGY=$1
  echo "Thread selection strategy:\t ${thread_selection_mode[$1+1]}"
  export TASK_AFF_AFFINITY_MAP_MODE=$2
  echo "Map mode:\t\t\t ${map_mode[$2+1]}"
  export TASK_AFF_PAGE_SELECTION_MODE=$3
  echo "Page selection strategy:\t ${page_selection_strategy[$3+1]}"
  export TASK_AFF_PAGE_WEIGHTING_STRATEGY=$4
  echo "Page weight strategy:\t\t ${page_weight_strategy[$4+1]}"

  NAME=${PROG_VERSION}___${5}${1}${2}${3}${4}${6}${7}___${thread_selection_mode[$TASK_AFF_THREAD_SELECTION_STRATEGY + 1]}___${map_mode[$TASK_AFF_AFFINITY_MAP_MODE+1]}___${page_selection_strategy[$TASK_AFF_PAGE_SELECTION_MODE+1]}___${page_weight_strategy[$TASK_AFF_PAGE_WEIGHTING_STRATEGY+1]}___THREADS-$OMP_NUM_THREADS
  echo "${NAME}"
}

make clean
module unload omp
compile_and_run ".baseline"
echo "\nrun task without affinity"
#compile_and_run ""

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}

#run with default config
compile ".affinity"
# echo "\nrun default"
# run ".affinity"
# echo ""

set_up_affinity 2 0 0 0 64               #thread(2)=lowest, page_sel(0)=first_of_every_page, page_weight(1)=Majority
#for i in {0..1}
#do
  run ".affinity"
#done
echo "\n"
set_up_affinity 2 1 0 0 64
#for i in {0..1}
#do
  run ".affinity"
#done
echo "\n"

#for threshold in {0..10}
#do
#  export TASK_AFF_THRESHOLD=$(($threshold/10.0))
#  echo "$TASK_AFF_THRESHOLD"
  #set_up_affinity 0 2 0 0 64 ${threshold}
  #for i in {0..10}
  #do
  #    run ".affinity"
  #done
#  grep "combined_map" stats_${NAME}.txt
#  echo "\n--------\n\n"
#done




##-------------------------------------------------------##
##            
##-------------------------------------------------------##
  
  # for t in {16..16}                  #number of threads
  # do
  #   export OMP_NUM_THREADS=$t
  #   echo "Number of threads:\t\t $t"
  #   for tsm in {0..4}               #Thread selecetion mode
  #   do
  #     for pss in {0..5}           #Page sellection mode
  #     do
  #       for pws in {0..3}         #Page  Weight mode
  #       do
  #         for mm in {0..1}              #Map Mode
  #         do
  #           set_up_affinity $tsm $mm $pss $pws $t
  #           run ".affinity"
  #           echo "\n\nrun with combined map:"
  #         done
  #         echo "\n----------\n"
  #       done
  #     done
  #   done
  # done
