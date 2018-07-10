#!/usr/local_rwth/bin/zsh
#BSUB -J "cg"
#BSUB -o batch_cg_smp.out.aff.cg.%J
#BSUB -e batch_cg_smp.err.aff.cg.%J
#BSUB -W 02:30
##BSUB -m c24m128
#BSUB -m c144m1024
#BSUB -n 1
#BSUB -x
#BSUB -a openmp
#BSUB -P jara0001
##BSUB -M 120000
#BSUB -M 1000000
#BSUB -u j.klinkenberg@itc.rwth-aachen.de
#BSUB -B
#BSUB -N

CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
OMP_BASE_DIR=${CUR_DIR}/../sparse_CG/tasking/

# enable modules
module switch intel intel/18.0
module use /home/jk869269/.modules

cd ${OMP_BASE_DIR}

./do.sh
