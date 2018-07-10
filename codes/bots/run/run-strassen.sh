#!/bin/bash
#defaults

#DEF_INPUTS="1024 2048"
DEF_INPUTS="2048"
VERBOSE=1
ARG_CPUS="4"
#ARG_VERSIONS=$1
#ARG_VERSIONS="omp-tasks omp-tasks-domain.lowest"
ARG_VERSIONS="omp-tasks omp-tasks-if_clause omp-tasks-manual"
#don't modify from here

BASE_DIR=$(dirname $0)/..
source $BASE_DIR/run/run.common 

parse_args $*
set_values
exec_all_sizes

