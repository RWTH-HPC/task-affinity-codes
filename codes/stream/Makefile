PROG=stream_task
FILE_ENDING=.c
CUR_COMPILER=$(CC)
FLAGS_COMPILER=-Wall -mcmodel=medium
ADD_COMPILE_FLAGS=-I../01_util/ -qopt-streaming-stores=always
ADD_COMPILE_FLAGS_GCC=-I../01_util/
AFFINITY_FLAG=

ifdef T_AFF_INVERTED
	ADD_COMPILE_FLAGS += -DT_AFF_INVERTED=${T_AFF_INVERTED}
	ADD_COMPILE_FLAGS_GCC += -DT_AFF_INVERTED=${T_AFF_INVERTED}
endif
ifdef T_AFF_SINGLE_CREATOR
	ADD_COMPILE_FLAGS += -DT_AFF_SINGLE_CREATOR=${T_AFF_SINGLE_CREATOR}
	ADD_COMPILE_FLAGS_GCC += -DT_AFF_SINGLE_CREATOR=${T_AFF_SINGLE_CREATOR}
endif
ifdef T_AFF_NUM_TASK_MULTIPLICATOR
	ADD_COMPILE_FLAGS += -DT_AFF_NUM_TASK_MULTIPLICATOR=${T_AFF_NUM_TASK_MULTIPLICATOR}
	ADD_COMPILE_FLAGS_GCC += -DT_AFF_NUM_TASK_MULTIPLICATOR=${T_AFF_NUM_TASK_MULTIPLICATOR}
endif
ifdef STREAM_ARRAY_SIZE
	ADD_COMPILE_FLAGS += -DSTREAM_ARRAY_SIZE=${STREAM_ARRAY_SIZE}
	ADD_COMPILE_FLAGS_GCC += -DSTREAM_ARRAY_SIZE=${STREAM_ARRAY_SIZE}
endif
ifdef THIRD_INVERTED
	ADD_COMPILE_FLAGS += -DTHIRD_INVERTED=${THIRD_INVERTED}
	ADD_COMPILE_FLAGS_GCC += -DTHIRD_INVERTED=${THIRD_INVERTED}
endif

ADD_COMPILE_FLAGS += -DNTIMES=10
#-D_FILTER_EXEC_TIMES

default: rel

rel.affinity: AFFINITY_FLAG=-DTASK_AFFINITY ./../task_affinity_support/task_affinity_support.c
rel.affinity: rel

deb.affinity: AFFINITY_FLAG=-DTASK_AFFINITY ./../task_affinity_support/task_affinity_support.c
deb.affinity: deb

rel.baseline: PROG=stream_base
rel.baseline: rel

deb.baseline: PROG=stream_base
deb.baseline: deb

rel.gcc: FLAGS_COMPILER=-mcmodel=medium
rel.gcc: rel

deb.gcc: FLAGS_COMPILER=-mcmodel=medium
deb.gcc: deb

deb: FLAGS_OPTIMIZATION=-g -O0
deb: generic

rel: FLAGS_OPTIMIZATION=-O3
rel: generic

generic:
	$(CUR_COMPILER) $(FLAGS_COMPILER) $(FLAGS_OPENMP) $(FLAGS_OPTIMIZATION) ${ADD_COMPILE_FLAGS} -o stream_task.exe ${AFFINITY_FLAG} ${PROG}${FILE_ENDING} 

clean:
	rm -f *.exe *.o *~ bla* nr_* output_* pushed_* evol* starts* task_exe* likwid_* core.* stats_*

