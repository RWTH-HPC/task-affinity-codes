#!/bin/zsh

#SBATCH --account=jara0001
#SBATCH --partition=c16s
#SBARCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=STREAM_TASK_AFFINITY_TEST
#SBATCH --output=sbatch.txt
#SBATCH --time=05:30:00
#SBATCH --exclusive

PROG_VERSION=rel
#PROG_VERSION=deb


export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=1
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=24
#export OMP_NUM_THREADS=64

export T_AFF_INVERTED=0
export THIRD_INVERTED=0
export T_AFF_SINGLE_CREATOR=1
export T_AFF_NUM_TASK_MULTIPLICATOR=16

export TASK_AFF_THREAD_SELECTION_STRATEGY=-1
export TASK_AFF_AFFINITY_MAP_MODE=-1
export TASK_AFF_PAGE_SELECTION_MODE=-1
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=-1
export TASK_AFF_NUMBER_OF_AFFINITIES=15

#export MATRIX_SIZE=40000
export MATRIX_SIZE=15000
export TITLE_SIZE=500
export CHECK_RESULTS=0

PROG_CMD="./ch_intel_aff ${MATRIX_SIZE} ${TITLE_SIZE} ${CHECK_RESULTS}"

thread_selection_mode=( first random lowest_wl RoundRobin private )
map_mode=(thread domain combined)
page_selection_strategy=(FirstPageOfFirstAffinity DivideInN EveryNTh FirstAndLast binary first)
page_weight_strategy=(first majority ByAffinity size)

function run {
  #no_numa_balancing "${PROG_CMD}" &> output-files/${NAME}_$2_output.txt
  #no_numa_balancing numamem -s 1 "${PROG_CMD}" &> output-files/${NAME}_$2_output.txt
  ${PROG_CMD} &> output-files/${NAME}_$2_output.txt
  sed -i 's/\./\,/g' output-files/${NAME}_$2_output.txt
  grep "Elapsed time" output-files/${NAME}_$2_output.txt
  #grep ": corr_domain" output_${NAME}.txt > output_corr_domain_${NAME}.txt
  #grep ": in_corr_domain" output_${NAME}.txt > output_in_corr_domain_${NAME}.txt
  #grep -e "count_overall" -e "count_task" output_${NAME}.txt > output_stats_${NAME}.txt
  #grep "_affinity_schedule" output_${NAME}.txt
  #grep "combined_map_strat" output_${NAME}.txt > combined_map_stats_${NAME}.txt
  #grep "T#0" output_${NAME}.txt > stats_${NAME}.txt
  #grep "combined_map" stats_${NAME}.txt
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

  #NAME=${PROG_VERSION}___${5}${1}${2}${3}${4}${6}${7}___${thread_selection_mode[$TASK_AFF_THREAD_SELECTION_STRATEGY + 1]}___${map_mode[$TASK_AFF_AFFINITY_MAP_MODE+1]}___${page_selection_strategy[$TASK_AFF_PAGE_SELECTION_MODE+1]}___${page_weight_strategy[$TASK_AFF_PAGE_WEIGHTING_STRATEGY+1]}___THREADS-$OMP_NUM_THREADS
  
  NAME=CHOLESKY_${TASK_AFF_NUMBER_OF_AFFINITIES}-${page_selection_strategy[$3+1]}-${page_weight_strategy[$4+1]}-${map_mode[$2+1]}${6}
  echo "${NAME}"
}

make clean
mkdir output-files

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}

echo "running regular..\n"
./ch_intel ${MATRIX_SIZE} ${TITLE_SIZE} ${CHECK_RESULTS} &> output-files/regular_0_output.txt
grep "Elapsed time" output-files/regular_0_output.txt
echo "\n"

# set_up_affinity 2 0 0 1
# run ".affinity" 24
# echo "\n"
# set_up_affinity 2 1 0 1
# run ".affinity" 24
# echo "\n"
# set_up_affinity 2 2 0 1
# run ".affinity" 24 9
# echo "\n"


 for affinities in {10..10}
 do
   export TASK_AFF_NUMBER_OF_AFFINITIES=${affinities}
   echo "Number of affinities:\t\t ${affinities}"
   for page_mode in {0..0} #first_page_of_first_affinity, devide_in_n, first_and last
   do
     for page_weight in {3..3}  #majority, by_affinity
     do
       set_up_affinity 2 1 ${page_mode} ${page_weight} 64 #thread
       for i in {0..9}
       do
         run ".affinity" ${i}
       done
       echo "\n"

       set_up_affinity 2 0 ${page_mode} ${page_weight} 64 #domain
       for i in {0..9}
       do
         run ".affinity" ${i}
       done
       echo "\n"

       for threshold in {8..8}
       do
         export TASK_AFF_THRESHOLD=$(($threshold/10.0))
         set_up_affinity 2 2 ${page_mode} ${page_weight} 64 -${threshold}0%
         for i in {0..9}
         do
             run ".affinity" ${i}
         done
         #grep "combined_map" stats_${NAME}.txt
         echo "\n--------\n\n"
       done
     done
   done
 done
