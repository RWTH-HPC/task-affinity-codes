#!/usr/local_rwth/bin/zsh
#BSUB -J "merge_task_aff_test"
#BSUB -o merge_task_aff_test.out.%J
#BSUB -e merge_task_aff_test.err.%J
#BSUB -W 05:00
#BSUB -m c24m128
#BSUB -n 1
#BSUB -x
#BSUB -a openmp
#BSUB -P jara0001
##BSUB -P hpclab
#BSUB -M 120000
#BSUB -u j.klinkenberg@itc.rwth-aachen.de
#BSUB -B
#BSUB -N

CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CUR_MEDIAN_FILE=${CUR_DIR}/median.sh

OMP_BASE_DIR=${CUR_DIR}/../tmp_mergesort/
ARR_SIZE=$((2**31))
PROGRAM_CMD="no_numa_balancing ./MergeSort.exe ${ARR_SIZE}"

# enable modules
module switch intel intel/18.0
module use /home/jk869269/.modules

cd ${OMP_BASE_DIR}

NUM_ITER=10

array=( 8 12 16 24 )
#array=($(seq 1 1 24))
#array=($(seq 1 1 3))

module unload omp
: << END
make rel.llvm
for value in ${array[*]}
do
  echo "Running rel.intel with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.intel" "${PROGRAM_CMD}"
done
END

module load omp/task_aff.rel
make rel.llvm
for value in ${array[*]}
do
  echo "Running rel.llvm with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.llvm" "${PROGRAM_CMD}"
done

make rel.domain.rand
for value in ${array[*]}
do
  echo "Running rel.domain.rand with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.domain.rand" "${PROGRAM_CMD}"
done

make rel.domain.lowest
for value in ${array[*]}
do
  echo "Running rel.domain.lowest with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.domain.lowest" "${PROGRAM_CMD}"
done

make rel.domain.private
for value in ${array[*]}
do
  echo "Running rel.domain.private with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.domain.private" "${PROGRAM_CMD}"
done

make rel.domain.round_robin
for value in ${array[*]}
do
  echo "Running rel.domain.round_robin with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.domain.round_robin" "${PROGRAM_CMD}"
done

make rel.thread.rand
for value in ${array[*]}
do
  echo "Running rel.thread.rand with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.thread.rand" "${PROGRAM_CMD}"
done

make rel.thread.lowest
for value in ${array[*]}
do
  echo "Running rel.thread.lowest with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.thread.lowest" "${PROGRAM_CMD}"
done

make rel.thread.round_robin
for value in ${array[*]}
do
  echo "Running rel.thread.round_robin with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.thread.round_robin" "${PROGRAM_CMD}"
done

