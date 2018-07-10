PROG=health
FILE_ENDING=.c
CUR_COMPILER=$(CC)
COMMON_DIR=../../common


#BOT_ADDITIONAL_PARAMS=-I../../../01_util 
BOT_ADDITIONAL_PARAMS=-I../../../01_util -DMANUAL_CUTOFF
#BOT_ADDITIONAL_PARAMS=-I../../../01_util -DIF_CUTOFF
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

deb: deb.domain.lowest

deb.domain.rand: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RAND

deb.domain.first: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_FIRST

deb.domain.lowest: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_LOWEST

deb.domain.round_robin: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RR

deb.thread.rand: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RAND

deb.thread.first: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_FIRST

deb.thread.lowest: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_LOWEST

deb.thread.round_robin: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RR

deb.llvm: pre-build-deb
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

deb.gcc: pre-build-deb
	 $(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

rel: rel.domain.lowest

rel.domain.rand: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RAND

rel.domain.first: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_FIRST

rel.domain.lowest: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_LOWEST

rel.domain.round_robin: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RR

rel.thread.rand: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RAND

rel.thread.first: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_FIRST

rel.thread.lowest: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_LOWEST

rel.thread.round_robin: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RR

rel.llvm: pre-build
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

rel.gcc: pre-build
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 -I$(COMMON_DIR) bots_common.o  main.o ${BOT_ADDITIONAL_PARAMS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

clean:
	rm -f *.exe *.o *~ bla* nr_* output* pushed_* evol* starts* task_exe*
