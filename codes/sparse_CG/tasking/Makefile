#PROG=stream_task2
PROG=solver
FILE_ENDING=.c
CUR_COMPILER=$(CC)

INCL_FLAGS=-I./../common -I./../../01_util -DPAGE_ALIGN -DNDEBUG
#ADD_COMPILE_FLAGS=-I/home/jk869269/repos/openmp/ompt_lib/

ADD_COMPILE_FLAGS_OMP_DD=${INCL_FLAGS} -DDATA_DISTRIBUTION -DUSE_OMP ../common/def.c ../common/errorcheck.c ../common/kernel.c ../common/help.c ../common/io.c ../common/main.c ../common/mmio.c ../common/output.c -rdynamic -lm
ADD_COMPILE_FLAGS_OMP=${INCL_FLAGS} -DUSE_OMP ../common/def.c ../common/errorcheck.c ../common/kernel.c ../common/help.c ../common/io.c ../common/main.c ../common/mmio.c ../common/output.c -rdynamic -lm

#ADD_COMPILE_FLAGS=${INCL_FLAGS} -DTASKING -DUSE_OMP -DDATA_DISTRIBUTION ../common/def.c ../common/errorcheck.c ../common/help.c ../common/io.c ../common/main.c ../common/mmio.c ../common/output.c -rdynamic -lm
#ADD_COMPILE_FLAGS=${INCL_FLAGS} -DTASKING -DUSE_OMP -DDATA_DISTRIBUTION -DPAR_PRODUCER ../common/def.c ../common/errorcheck.c ../common/help.c ../common/io.c ../common/main.c ../common/mmio.c ../common/output.c -rdynamic -lm
ADD_COMPILE_FLAGS=${INCL_FLAGS} -DTASKING -DUSE_OMP -DSTATIC_INIT ../common/def.c ../common/errorcheck.c ../common/help.c ../common/io.c ../common/main.c ../common/mmio.c ../common/output.c -rdynamic -lm
ifdef PAR_PRODUCER
	ADD_COMPILE_FLAGS := -DPAR_PRODUCER=1 ${ADD_COMPILE_FLAGS}
	ADD_COMPILE_FLAGS_OMP := -DPAR_PRODUCER=1 ${ADD_COMPILE_FLAGS_OMP}
endif
#ADD_COMPILE_FLAGS=${INCL_FLAGS} -DTASKING -DUSE_OMP -DSTATIC_INIT -DPAR_PRODUCER=1 ../common/def.c ../common/errorcheck.c ../common/help.c ../common/io.c ../common/main.c ../common/mmio.c ../common/output.c -rdynamic -lm

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

deb.gcc: 
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

deb.baseline: 
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS_OMP} ../common/solver.c -o ${PROG}.exe

deb.optimal: 
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS_OMP_DD} ../common/solver.c -o ${PROG}.exe

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

rel.gcc:
	$(CUR_COMPILER) -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe

rel.baseline: 
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS_OMP} ../common/solver.c -o ${PROG}.exe

rel.optimal: 
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS_OMP_DD} ../common/solver.c -o ${PROG}.exe

clean:
	rm -f *.exe *.o *~ bla* nr_* output* pushed_* evol* starts* task_exe* likwid_*
