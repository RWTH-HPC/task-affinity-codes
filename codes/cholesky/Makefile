
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
#MATRIX_SIZE     ?= 8000
BLOCK_SIZE      ?= 512
BOOL_CHECK      ?= 1
AFFINITY_FLAG=
AFFINITY_SUFFIX=
#CFLAGS          += -DTRACE -I$(VT_ROOT)/include
#LDFLAGS         += -trace

all : ch_$(TARGET) ch_$(TARGET)_timing

affinity: AFFINITY_FLAG=-DTASK_AFFINITY ./../task_affinity_support/task_affinity_support.c
affinity: AFFINITY_SUFFIX=_aff
affinity: ch_$(TARGET) ch_$(TARGET)_timing

ch_$(TARGET):
	$(CC) $(CFLAGS) -o $@${AFFINITY_SUFFIX} ch_common.c -O3 -g $(LDFLAGS) ${AFFINITY_FLAG}

ch_$(TARGET)_timing:
	$(CC) $(CFLAGS) -o $@${AFFINITY_SUFFIX} ch_common.c -O3 -g $(LDFLAGS) -DUSE_TIMING ${AFFINITY_FLAG}


run_affinity: PROG_EXE+=ch_$(TARGET)+=_aff
run_affinity: run

run_affinity-timing: PROG_EXE+=ch_$(TARGET)+=_aff
run_affinity-timing: run-timing

run:
	$(RUN_SETTINGS) ./$(PROG_EXE) $(MATRIX_SIZE) $(BLOCK_SIZE) $(BOOL_CHECK)

run-timing:
	$(RUN_SETTINGS) ./$(PROG_EXE)_timing $(MATRIX_SIZE) $(BLOCK_SIZE) $(BOOL_CHECK)

clean:
	rm -f *.o ch_$(TARGET) ch_$(TARGET)_timing
