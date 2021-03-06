#!/bin/bash
#BSUB -P thes0466
#BSUB -W 7:00
#BSUB -m c24m128
#BSUB -a openmp
#BSUB -n 1
#BSUB -x
#BSUB -o batch_%J_stream_%J
#BSUB -J stream_2socket
#BSUB -M 126000

lscpu
#lstopo

mkdir batch_$LSB_JOBID
cd batch_$LSB_JOBID
PROG_CMD=../stream_task.exe
#PROG_VERSION=deb
PROG_VERSION=rel

export KMP_TASK_STEALING_CONSTRAINT=0
export KMP_A_DEBUG=60
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
#export OMP_NUM_THREADS=4
export OMP_NUM_THREADS=64

export T_AFF_INVERTED=0
export THIRD_INVERTED=0
export T_AFF_SINGLE_CREATOR=1
#export T_AFF_NUM_TASK_MULTIPLICATOR=4
export T_AFF_NUM_TASK_MULTIPLICATOR=16
#export STREAM_ARRAY_SIZE=45000000000
export STREAM_ARRAY_SIZE=$((2**31))

module switch intel intel/18.0

#source /home/jk869269/util/bash/ompGetCoresForSpread.sh
#TMP_CORES=$(ompGetCoresForSpread ${OMP_NUM_THREADS})
#module load likwid

function eval_run {
  if [ -n "$2" ] && [ -n "$3" ]; then
    curname="$1.$4.$3"
else
    curname=$1
  fi
  echo "Executing affinity ${curname}"
  make -C ../ ${PROG_VERSION}."$1" sched=$2 num=$3

  # { timex -v likwid-perfctr -f -g NUMA -c ${TMP_CORES} -O -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  # { timex -v likwid-perfctr -f -g TASKAFFINITY -c ${TMP_CORES} -O -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  # { timex -v likwid-perfctr -f -g QPI -c ${TMP_CORES} -O -o likwid_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  # { timex -v likwid-perfctr -f -g CYCLE_ACTIVITY -c ${TMP_CORES} -O -o likwid_cycle_${curname}.csv no_numa_balancing "${PROG_CMD}" ; } &> output_${curname}.txt
  #sed -i 's/,/\t/g' likwid_${curname}.csv
  #sed -i 's/\./,/g' likwid_${curname}.csv
  #sed -i 's/,/\t/g' likwid_cycle_${curname}.csv
  #sed -i 's/\./,/g' likwid_cycle_${curname}.csv

  #no_numa_balancing advixe-cl -collect roofline -project-dir TestRoofline -- "${PROG_CMD}" &> output_${curname}.txt
  no_numa_balancing "${PROG_CMD}" &>> output_${curname}.txt
  echo "=== === === === === === === === === === === === === === === === === === === === === === === === === === === === === ===" >> output_${curname}.txt
  tac output_${curname}.txt | grep -m 1 "Elapsed time"
  tac output_${curname}.txt | grep -m 1 "Copy:"
  tac output_${curname}.txt | grep -m 1 "Scale:"
  tac output_${curname}.txt | grep -m 1 "Add:"
  tac output_${curname}.txt | grep -m 1 "Triad:"

  tac output_${curname}.txt | grep -m 1 "Elapsed time" >> time_${curname}.txt
  tac output_${curname}.txt | grep -m 1 "Copy:" >> time_${curname}.txt
  tac output_${curname}.txt | grep -m 1 "Scale:" >> time_${curname}.txt
  tac output_${curname}.txt | grep -m 1 "Add:" >> time_${curname}.txt
  tac output_${curname}.txt | grep -m 1 "Triad:" >> time_${curname}.txt

  #grep "TASK AFFINITY:" output_${curname}.txt > bla_${curname}
  #grep "stole task" output_${curname}.txt > nr_steals_${curname}
  #grep "TASK_SUCCESSFULLY_PUSHED" output_${curname}.txt > pushed_${curname}
  #grep "task_aff_stats" output_${curname}.txt > evol_${curname}
  #grep "__kmp_task_start(enter_aff)" output_${curname}.txt > starts_${curname}
  #grep "TASK_EXECUTION_TIME"  output_${curname}.txt > task_execution_times_${curname}
}

make clean
module unload omp
for j in {1..10}; do

for i in 02 04 06 08 10 12 16 20 24; do
export OMP_NUM_THREADS=$i

# eval_run "baseline"
eval_run "llvm"


# for i in 008 0016 032 048 064 080 104 128 144; do
# export OMP_NUM_THREADS=$i
#
# eval_run "llvm_T$i"
done
done
# done

# module switch intel gcc/7
# for i in {1..10}; do
# eval_run "gcc"
# done
# module switch gcc intel/18.0

module use -a ~/.modules
module load omp/task_aff.${PROG_VERSION}
make -C ~ task.${PROG_VERSION}
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
first=5
divn=1
divn2=12
step=2
fal=3
bin=4

first1=00
none=01
aff=02
size=03
#divn 1, step 2, fal 3, first 0
#none 1, aff 2, size 3, first 0

# for j in {1..10}; do
# export OMP_NUM_THREADS=64


# eval_run "domain.lowest" $first1$first1 1 "first1_first1"

# eval_run "domain.lowest" $first$none 1 "first_none"
# eval_run "domain.lowest" $first$aff 1 "first_aff"
# eval_run "domain.lowest" $first$size 1 "first_size"

# eval_run "domain.lowest" $fal$none 2 "fal_none"
# eval_run "domain.lowest" $fal$aff 2 "fal_aff"
# eval_run "domain.lowest" $fal$size 2 "fal_size"

# eval_run "domain.lowest" $bin$first1 20 "bin_first1"
# eval_run "domain.lowest" $bin$none 20 "bin_none"
# eval_run "domain.lowest" $bin$aff 20 "bin_aff"
# eval_run "domain.lowest" $bin$size 20 "bin_size"

# i=32
# for i in 1 2 6 12 32; do

# eval_run "domain.lowest" $divn$first1 $i "divn_first1"
# eval_run "domain.lowest" $divn$none $i "divn_none"
# eval_run "domain.lowest" $divn$aff $i "divn_aff"
# eval_run "domain.lowest" $divn$size $i "divn_size"

# eval_run "domain.lowest" $divn2$first1 $i "divn2_first1"
# eval_run "domain.lowest" $divn2$none $i "divn2_none"
# eval_run "domain.lowest" $divn2$aff $i "divn2_aff"
# eval_run "domain.lowest" $divn2$size $i "divn2_size"

# done
# i=32
# for i in 4 8 16 32 64 128 256; do

# eval_run "domain.lowest" $step$first1 $i "step_first1"
# eval_run "domain.lowest" $step$none $i "step_none"
# eval_run "domain.lowest" $step$aff $i "step_aff"
# eval_run "domain.lowest" $step$size $i "step_size"

# done

for i in 02 04 06 08 10 12 16 20 24; do
export OMP_NUM_THREADS=$i

eval_run "domain.lowest" $fal$none 2 "D_F&L_T$i"
eval_run "thread.lowest" $fal$none 2 "T_F&L_T$i"
eval_run "domain.lowest" $bin$none 10 "D_Bin_T$i"
eval_run "domain.lowest" $step$none 512 "D_Step(512)_T$i"
eval_run "thread.lowest" $step$none 512 "T_Step(512)_T$i"
done

done
