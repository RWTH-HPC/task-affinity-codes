#!/bin/zsh

#SBATCH --account=jara0001
#SBATCH --partition=c16s
#SBARCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=STREAM_TASK_AFFINITY_TEST
#SBATCH --output=sbatch_output.txt
#SBATCH --time=00:30:00
#SBATCH --exclusive

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
export OMP_NUM_THREADS=64

# additionally specify number of tasks
export CG_NUM_TASKS=$(($OMP_NUM_THREADS * 16))//16
export PAR_PRODUCER=1


export TASK_AFF_THREAD_SELECTION_STRATEGY=-1
export TASK_AFF_AFFINITY_MAP_MODE=-1
export TASK_AFF_PAGE_SELECTION_MODE=-1
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=-1
export TASK_AFF_NUMBER_OF_AFFINITIES=1


module switch intel intel/18.0

thread_selection_mode=( first random lowest_wl round_robin private )
map_mode=(thread domain combined)
page_selection_strategy=(first_page_of_first_affinity divide_in_n every_n_th first_and_last binary first)
page_weight_strategy=(first majority by_affinity size)

function compile {
  echo "Compiling affinity $1"
  make ${PROG_VERSION}"$1"
}

function run {
  no_numa_balancing "${PROG_CMD}" &> ${NAME}_$2_output.txt
  sed -i 's/\./\,/g' ${NAME}_$2_output.txt
  grep "Elapsed time" ${NAME}_$2_output.txt
  #grep ": corr_domain" output_${NAME}.txt > output_corr_domain_${NAME}.txt
  #grep ": in_corr_domain" output_${NAME}.txt > output_in_corr_domain_${NAME}.txt
  #grep -e "count_overall" -e "count_task" output_${NAME}.txt > output_stats_${NAME}.txt
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

  #NAME=${PROG_VERSION}___${5}${1}${2}${3}${4}${6}${7}___${thread_selection_mode[$TASK_AFF_THREAD_SELECTION_STRATEGY + 1]}___${map_mode[$TASK_AFF_AFFINITY_MAP_MODE+1]}___${page_selection_strategy[$TASK_AFF_PAGE_SELECTION_MODE+1]}___${page_weight_strategy[$TASK_AFF_PAGE_WEIGHTING_STRATEGY+1]}___THREADS-$OMP_NUM_THREADS
  

  NAME=Sparce_CG_${map_mode[$2+1]}
  echo "${NAME}"
}

make clean
module unload omp
#eval_run "gcc"
#eval_run "llvm" "" "intel"
#eval_run "baseline"

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}
#make -C ~ task.${PROG_VERSION}

compile ".affinity"
set_up_affinity 2 0 0 0 64 #thread
for i in {0..10}
do
  run ".affinity" ${i}
done
echo "\n"

set_up_affinity 2 1 0 0 64 #domain
for i in {0..10}
do
  run ".affinity" ${i}
done
echo "\n"

for threshold in {4..4}
do
  export TASK_AFF_THRESHOLD=$(($threshold/10.0))
  set_up_affinity 2 2 0 0 64 ${threshold}
  for i in {0..10}
  do
      run ".affinity" ${i}
  done
  #grep "combined_map" stats_${NAME}.txt
  echo "\n--------\n\n"
done