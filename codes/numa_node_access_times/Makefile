PROG=numa_node_access_times
FILE_ENDING=.c
CUR_COMPILER=$(CC)

#ADD_COMPILE_FLAGS=-I/home/jk869269/repos/openmp/ompt_lib/
ADD_COMPILE_FLAGS=-lnuma
ifdef STREAM_ARRAY_SIZE
	ADD_COMPILE_FLAGS += -DSTREAM_ARRAY_SIZE=${STREAM_ARRAY_SIZE}
endif

deb: deb.domain.lowest

deb.domain.rand:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RAND

deb.domain.first:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_FIRST

deb.domain.lowest:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_LOWEST

deb.domain.private:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_PRIVATE

deb.domain.round_robin:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RR

deb.thread.rand:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RAND

deb.thread.first:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_FIRST

deb.thread.lowest:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_LOWEST

deb.thread.round_robin:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RR

deb.llvm:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

rel: rel.domain.lowest

rel.domain.rand:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RAND

rel.domain.first:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_FIRST

rel.domain.lowest:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_LOWEST

rel.domain.private:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_PRIVATE

rel.domain.round_robin:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_DOMAIN_RR

rel.thread.rand:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RAND

rel.thread.first:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_FIRST

rel.thread.lowest:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_LOWEST

rel.thread.round_robin:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe -DTASK_AFFINITY -DTASK_AFF_THREAD_RR

rel.llvm:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

clean:
	rm -f *.exe *.o *~ bla* nr_* output* pushed_* evol* starts* task_exe* likwid_*
