PROG=sort
FILE_ENDING=.c
CUR_COMPILER=$(CC)
FLAGS_COMPILER=-Wall -mcmodel=medium
COMMON_DIR=../../common


BOT_ADDITIONAL_PARAMS=-I../../../01_util
#BOT_ADDITIONAL_PARAMS=-I../../../01_util -DUSE_TASK_AFF_ONLY_AT_CUTOFF=1
#BOT_ADDITIONAL_PARAMS=-I../../../01_util -qopt-streaming-stores=always
#BOT_ADDITIONAL_PARAMS=-DMANUAL_CUTOFF
#BOT_ADDITIONAL_PARAMS=-DIF_CUTOFF

pre-build:
	@$(CUR_COMPILER) $(FLAGS_OPENMP) -c -I$(COMMON_DIR) -O3 $(COMMON_DIR)/bots_common.c -o bots_common.o
	@$(CUR_COMPILER) $(FLAGS_OPENMP) -c -I$(COMMON_DIR) -O3 -I. -o main.o ${BOT_ADDITIONAL_PARAMS} $(COMMON_DIR)/bots_main.c

pre-build-deb:
	@$(CUR_COMPILER) $(FLAGS_OPENMP) -c -I$(COMMON_DIR) -g -O0 $(COMMON_DIR)/bots_common.c -o bots_common.o
	@$(CUR_COMPILER) $(FLAGS_OPENMP) -c -I$(COMMON_DIR) -g -O0 -I. -o main.o ${BOT_ADDITIONAL_PARAMS} $(COMMON_DIR)/bots_main.c


rel.affinity: AFFINITY_FLAG=-DTASK_AFFINITY ./../../../task_affinity_support/task_affinity_support.c -D_FILTER_EXEC_TIMES
rel.affinity: rel

deb.affinity: AFFINITY_FLAG=-DTASK_AFFINITY ./../../../task_affinity_support/task_affinity_support.c -D_FILTER_EXEC_TIMES
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

rel: FLAGS_OPTIMIZATION=-O3 #-g
rel: generic

generic:
	$(CUR_COMPILER) $(FLAGS_COMPILER) $(FLAGS_OPENMP) $(FLAGS_OPTIMIZATION) -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS}  ${PROG}${FILE_ENDING} -o ${PROG}.exe ${AFFINITY_FLAG}

clean:
	rm -f *.exe *.o *~ bla* nr_* output* *output.txt pushed_* evol* starts* task_exe*
