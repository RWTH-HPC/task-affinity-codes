#!/usr/local_rwth/bin/zsh
#BSUB -J "stream_task_aff_test"
#BSUB -o stream_task_aff_test.out.%J
#BSUB -e stream_task_aff_test.err.%J
#BSUB -W 01:00
#BSUB -m c24m128
#BSUB -n 1
#BSUB -x
#BSUB -a openmp
#BSUB -P jara0001
#BSUB -M 120000
#BSUB -u j.klinkenberg@itc.rwth-aachen.de
#BSUB -B
#BSUB -N

CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CUR_MEDIAN_FILE=${CUR_DIR}/median.sh

OMP_BASE_DIR=${CUR_DIR}/../stream/
PROGRAM_CMD="no_numa_balancing ./stream_task.exe"

# enable modules
module switch intel intel/18.0
module use /home/jk869269/.modules

cd ${OMP_BASE_DIR}

NUM_ITER=5

#array=( 2 4 8 16 24 )
array=( 6 12 )

export T_AFF_INVERTED=0
export T_AFF_SINGLE_CREATOR=1
export T_AFF_NUM_TASK_MULTIPLICATOR=10
export STREAM_ARRAY_SIZE=$((2**28))

module unload omp
make rel.baseline
for value in ${array[*]}
do
  echo "Running rel.baseline with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.baseline" "${PROGRAM_CMD}"
done

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

make rel.domain.round_robin
for value in ${array[*]}
do
  echo "Running rel.domain.round_robin with $value threads..."
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.domain.rr" "${PROGRAM_CMD}"
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
  ${CUR_MEDIAN_FILE} ${NUM_ITER} ${value} "rel.thread.rr" "${PROGRAM_CMD}"
done

