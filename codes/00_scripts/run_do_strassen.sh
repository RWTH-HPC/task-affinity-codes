#!/usr/local_rwth/bin/zsh
#BSUB -J "cg"
#BSUB -o out.aff.cg.%J
#BSUB -e err.aff.cg.%J
#BSUB -W 02:00
##BSUB -m c24m128
#BSUB -m c144m1024
#BSUB -n 1
#BSUB -x
#BSUB -a openmp
#BSUB -P jara0001
##BSUB -P hpclab
#BSUB -M 120000
#BSUB -u j.klinkenberg@itc.rwth-aachen.de
#BSUB -B
#BSUB -N

#PROGRAM_CMD=""
OMP_BASE_DIR=/home/jk869269/repos/openmp/bots/omp-tasks/strassen/

# enable modules
module switch intel intel/18.0
module use /home/jk869269/.modules

# first necessary to rebuild runtimes on SMP machine
cd /home/jk869269/repos/openmp/
#make all

cd ${OMP_BASE_DIR}

./do.sh
