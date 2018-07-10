#!/usr/local_rwth/bin/zsh
#BSUB -J "mergesort_smp"
#BSUB -o merge.out.%J
#BSUB -e merge.err.%J
#BSUB -W 03:00
#BSUB -m c24m128
##BSUB -m c144m1024
#BSUB -n 1
#BSUB -x
#BSUB -a openmp
#BSUB -P jara0001
#BSUB -M 120000
#BSUB -u j.klinkenberg@itc.rwth-aachen.de
#BSUB -B
#BSUB -N

OMP_BASE_DIR=/home/jk869269/repos/openmp/mergesort

# enable modules
module switch intel intel/18.0
module use /home/jk869269/.modules

# first necessary to rebuild runtimes on SMP machine
cd /home/jk869269/repos/openmp/
#make all

cd ${OMP_BASE_DIR}

./do.sh
