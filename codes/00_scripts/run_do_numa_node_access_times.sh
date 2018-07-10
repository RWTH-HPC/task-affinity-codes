#!/usr/local_rwth/bin/zsh
#BSUB -J "numa_node_access_times"
#BSUB -o numa_node_access_times.out.%J
#BSUB -e numa_node_access_times.err.%J
#BSUB -W 00:10
##BSUB -m c24m128
#BSUB -m c144m1024
#BSUB -n 1
#BSUB -x
#BSUB -a openmp
#BSUB -P jara0001
#BSUB -M 1000000  
#BSUB -u j.klinkenberg@itc.rwth-aachen.de
#BSUB -B
#BSUB -N

OMP_BASE_DIR=/home/jk869269/repos/openmp/numa_node_access_times/

# enable modules
module switch intel intel/18.0
module use /home/jk869269/.modules

cd ${OMP_BASE_DIR}

./do.sh
