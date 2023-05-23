#!/bin/zsh
#SBATCH --account=supp0001
#SBATCH --partition=c16s
#SBATCH -C CO7
#SBARCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=144
#SBATCH --job-name=STREAM_TASK_AFFINITY_TEST
#SBATCH --time=10:00:00
#SBATCH --exclusive
#SBATCH --output=out_%j.txt

# ========================================
# === Environment: Prep & Execution Setup
# ========================================
# activate runtime
RUNTIME_BASE_DIR=${RUNTIME_BASE_DIR:-"$(pwd)/../../runtime_affinity_rel"}
export LD_LIBRARY_PATH=${RUNTIME_BASE_DIR}/lib:${LD_LIBRARY_PATH}
export LIBRARY_PATH=${RUNTIME_BASE_DIR}/lib:${LIBRARY_PATH}
export INCLUDE=${RUNTIME_BASE_DIR}/include:${INCLUDE}
export CPATH=${RUNTIME_BASE_DIR}/include:${CPATH}
export C_INCLUDE_PATH=${RUNTIME_BASE_DIR}/include:${C_INCLUDE_PATH}
export CPLUS_INCLUDE_PATH=${RUNTIME_BASE_DIR}/include:${CPLUS_INCLUDE_PATH}

RESULT_DIR="$(pwd)/$(date +"%Y-%m-%d_%H%M%S")_results"
mkdir ${RESULT_DIR}

PROG_CMD=./stream_task.exe
PROG_VERSION=rel
#PROG_VERSION=deb
NAME=$PROG_VERSION

export KMP_TASK_STEALING_CONSTRAINT=0
#export KMP_A_DEBUG=2
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=64
export N_REP=10

export T_AFF_INVERTED=0
export T_AFF_DATA_INIT_PARTIAL_REMOTE=0
#export KMP_AFFINITY=verbose
export T_AFF_SINGLE_CREATOR=0
export T_AFF_NUM_TASK_MULTIPLICATOR=20
export STREAM_ARRAY_SIZE=$((2**31))

export TASK_AFF_THREAD_SELECTION_STRATEGY=-1
export TASK_AFF_AFFINITY_MAP_MODE=-1
export TASK_AFF_PAGE_SELECTION_MODE=-1
export TASK_AFF_PAGE_WEIGHTING_STRATEGY=-1
export TASK_AFF_NUMBER_OF_AFFINITIES=1

thread_selection_mode=(first random lowest_wl RoundRobin private)
map_mode=(thread domain combined)
page_selection_strategy=(FirstPageOfFirstAffinity DivideInN EveryNTh FirstAndLast binary first)
page_weight_strategy=(first majority ByAffinity size)

# ========================================
# === Functions
# ========================================
function compile {
    echo "Compiling affinity $1"
    make ${PROG_VERSION}"$1"
}

function run {
    no_numa_balancing "${PROG_CMD}" &> ${RESULT_DIR}/${NAME}_$2_output.txt
    # sed -i 's/\./\,/g' ${RESULT_DIR}/${NAME}_$2_output.txt
    grep "Elapsed time" ${RESULT_DIR}/${NAME}_$2_output.txt
    #grep ": corr_domain" output_${NAME}.txt > output_corr_domain_${NAME}.txt
    #grep ": in_corr_domain" output_${NAME}.txt > output_in_corr_domain_${NAME}.txt
    #grep -e "count_overall" -e "count_task" output_${NAME}.txt > output_stats_${NAME}.txt
    #grep "_affinity_schedule" output_${NAME}.txt
    #grep "combined_map_strat" output_${NAME}.txt > combined_map_stats_${NAME}.txt
    #grep "T#0" output_${NAME}.txt > stats_${NAME}.txt
    #grep "combined_map" stats_${NAME}.txt
    #echo "\n"
}

function compile_and_run {
    compile "$1"
    run "$1"
}

function set_up_affinity {
    NAME=STREAM_${map_mode[$2+1]}_${page_selection_strategy[$3+1]}_${TASK_AFF_NUMBER_OF_AFFINITIES}_${page_weight_strategy[$4+1]}_${5}
    echo "${NAME}"

    export TASK_AFF_THREAD_SELECTION_STRATEGY=$1
    echo "Thread selection strategy:\t ${thread_selection_mode[$1+1]}"
    export TASK_AFF_AFFINITY_MAP_MODE=$2
    echo "Map mode:\t\t\t ${map_mode[$2+1]}"
    export TASK_AFF_PAGE_SELECTION_MODE=$3
    echo "Page selection strategy:\t ${page_selection_strategy[$3+1]}"
    export TASK_AFF_PAGE_WEIGHTING_STRATEGY=$4
    echo "Page weight strategy:\t\t ${page_weight_strategy[$4+1]}"
}

# ========================================
# === Environment: Stategies and Settings
# ========================================
# Main modes
#   0: temporal mode
#   1: domain mode
#   2: combined mode
MAIN_MODES=(1)

# Weighting strategy
#   0: first_page_only
#   1: majority
#   2: by_affinity
STRATS_WEIGHTING=(1 2)

# Page selection strategy
#   0: first_page_of_first_affinity_only
#   1: divide_in_n_pages
#   2: every_nth_page
#   3: first_and_last_page
#   4: continuous_binary_search
#   5: first_page
STRATS_PAGE_SELECTION=(0 1 2 3 5)

# Number of affinities
N_AFFINITIES=(4 8 16 32 64 128)

# clean first
make clean

# ========================================
# === Baseline Experiments
# ========================================
compile ".baseline"
NAME="baseline_${OMP_NUM_THREADS}"
for rep in {1..${N_REP}}
do
    run ".baseline" ${rep}    
done

# ========================================
# === Affinity Experiments
# ========================================
compile ".affinity"
for rep in {1..${N_REP}}
do
    for mode in "${MAIN_MODES[@]}"
    do
        for weighting in "${STRATS_WEIGHTING[@]}"
        do
            for page_selection in "${STRATS_PAGE_SELECTION[@]}"
            do
                if [ "${page_selection}" = "1" ] || [ "${page_selection}" = "2" ]; then

                    for affinities in "${N_AFFINITIES[@]}"
                    do
                        export TASK_AFF_NUMBER_OF_AFFINITIES=${affinities}
                        set_up_affinity 2 ${mode} ${page_selection} ${weighting} ${OMP_NUM_THREADS}
                        run ".affinity" ${rep}
                    done
                else
                    export TASK_AFF_NUMBER_OF_AFFINITIES=1
                    set_up_affinity 2 ${mode} ${page_selection} ${weighting} ${OMP_NUM_THREADS}
                    run ".affinity" ${rep}
                fi
            done
        done
    done
done
