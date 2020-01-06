#PROG=stream_task2
PROG=solver
FILE_ENDING=.c
FLAGS_COMPILER=-Wall -mcmodel=medium
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

rel.affinity: AFFINITY_FLAG=-DTASK_AFFINITY ./../../task_affinity_support/task_affinity_support.c -D_FILTER_EXEC_TIMES
rel.affinity: rel

deb.affinity: AFFINITY_FLAG=-DTASK_AFFINITY ./../../task_affinity_support/task_affinity_support.c -D_FILTER_EXEC_TIMES
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

deb.baseline:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS_OMP} ../common/solver.c -o ${PROG}.exe

deb.optimal:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -g -O0 ${ADD_COMPILE_FLAGS_OMP_DD} ../common/solver.c -o ${PROG}.exe

rel.baseline:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS_OMP} ../common/solver.c -o ${PROG}.exe

rel.optimal:
	$(CUR_COMPILER) -Wall -mcmodel=medium $(FLAGS_OPENMP) -O3 ${ADD_COMPILE_FLAGS_OMP_DD} ../common/solver.c -o ${PROG}.exe

generic:
	$(CUR_COMPILER) $(FLAGS_COMPILER) $(FLAGS_OPENMP) $(FLAGS_OPTIMIZATION)  ${ADD_COMPILE_FLAGS} ${PROG}${FILE_ENDING} -o ${PROG}.exe ${AFFINITY_FLAG}

clean:
	rm -f *.exe *.o *~ bla* nr_* output* pushed_* evol* starts* task_exe* likwid_*
