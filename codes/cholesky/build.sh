#!/bin/bash

PROG_VERSION=rel

for target in intel 
do
  # load default modules
  module purge
  module load DEVELOP

  # load target specific compiler and libraries
  while IFS='' read -r line || [[ -n "$line" ]]; do
    if  [[ $line == LOAD_COMPILER* ]] || [[ $line == LOAD_LIBS* ]] ; then
      eval "$line"
    fi
  done < "flags_${target}.def"
  module load ${LOAD_COMPILER}
  module load ${LOAD_LIBS}
  module li

  # make corresponding targets
  module use -a ~/.modules
  module load omp/task_aff.${PROG_VERSION}
  TARGET=${target} make affinity
  
  TARGET=${target} make
  
done
