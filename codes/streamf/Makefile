PROG=stream_task
FILE_ENDING=.f90
CUR_COMPILER=$(FC)

ADD_COMPILE_FLAGS=-I/home/jk869269/repos/openmp/ompt_lib/ -fpp
#ADD_COMPILE_FLAGS=
ifdef T_AFF_INVERTED
	ADD_COMPILE_FLAGS += -DT_AFF_INVERTED=${T_AFF_INVERTED}
endif
ifdef T_AFF_SINGLE_CREATOR
	ADD_COMPILE_FLAGS += -DT_AFF_SINGLE_CREATOR=${T_AFF_SINGLE_CREATOR}
endif
ifdef T_AFF_NUM_TASK_MULTIPLICATOR
	ADD_COMPILE_FLAGS += -DT_AFF_NUM_TASK_MULTIPLICATOR=${T_AFF_NUM_TASK_MULTIPLICATOR}
endif
ifdef STREAM_ARRAY_SIZE
	ADD_COMPILE_FLAGS += -DSTREAM_ARRAY_SIZE=${STREAM_ARRAY_SIZE}
endif

deb: deb.domain.lowest

deb.domain.rand:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RAND

deb.domain.first:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_FIRST

deb.domain.lowest:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_LOWEST

deb.domain.private:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_PRIVATE

deb.domain.round_robin:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RR

deb.thread.rand:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RAND

deb.thread.first:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_FIRST

deb.thread.lowest:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_LOWEST

deb.thread.round_robin:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RR

deb.llvm:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

deb.baseline:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} stream_base.c -o ${PROG}.exe

rel: rel.domain.lowest

rel.domain.rand:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RAND

rel.domain.first:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_FIRST

rel.domain.lowest:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_LOWEST

rel.domain.private:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_PRIVATE

rel.domain.round_robin:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RR

rel.thread.rand:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RAND

rel.thread.first:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_FIRST

rel.thread.lowest:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_LOWEST

rel.thread.round_robin:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RR

rel.llvm:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

rel.baseline:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} stream_base.c -o ${PROG}.exe

clean:
	rm -f *.exe *.o *~ bla* nr_* output* pushed_* evol* starts* task_exe* likwid_*
