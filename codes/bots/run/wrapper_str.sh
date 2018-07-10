#!/bin/bash

#TMP_VERS=base domain.lowest
TMP_VERS="omp-tasks omp-tasks-domain.lowest"

for ver in $(echo $TMP_VERS | tr , ' '); do
  case $ver in
    "omp-tasks")
    ;;
    *)
    module load omp/task_aff.rel
    ;;
  esac

  # set arg value and run
  #ARG_VERSIONS=$ver
  ./run-strassen.sh -v $ver &> "result_$ver.txt"

  case $ver in
    "omp-tasks")
    ;;
    *)
    module unload omp
    ;;
  esac

done
 $(echo $ARG_CLASSES | tr , ' ')
