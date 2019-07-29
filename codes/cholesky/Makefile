
ifneq ($(TARGET),)
include ./flags_$(TARGET).def
else
TARGET=generic
CFLAGS=-fopenmp
LDFLAGS=-fopenmp
endif

# parameters and settings for execution
RUN_SETTINGS    = OMP_PLACES=cores OMP_PROC_BIND=spread
PROG_EXE        ?= ch_$(TARGET)
MATRIX_SIZE     ?= 15360
BLOCK_SIZE      ?= 512
BOOL_CHECK      ?= 1
AFFINITY_FLAG=
#CFLAGS          += -DTRACE -I$(VT_ROOT)/include
#LDFLAGS         += -trace

task_aff: AFFINITY_FLAG=-DTASK_AFFINITY ./../task_affinity_support/task_affinity_support.c
task_aff: all

all : ch_$(TARGET) ch_$(TARGET)_timing

ch_$(TARGET):
	$(CC) $(CFLAGS) -o $@ ch_common.c -O3 -g $(LDFLAGS) ${AFFINITY_FLAG}

ch_$(TARGET)_timing:
	$(CC) $(CFLAGS) -o $@ ch_common.c -O3 -g $(LDFLAGS) -DUSE_TIMING ${AFFINITY_FLAG}

run:
	$(RUN_SETTINGS)  ./$(PROG_EXE) $(MATRIX_SIZE) $(BLOCK_SIZE) $(BOOL_CHECK)

run-timing:
	$(RUN_SETTINGS) ./$(PROG_EXE)_timing $(MATRIX_SIZE) $(BLOCK_SIZE) $(BOOL_CHECK)

clean:
	rm -f *.o ch_$(TARGET) ch_$(TARGET)_timing
