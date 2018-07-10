#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef PAGE_ALIGN
# include <malloc.h>
#endif

#ifdef LIKWID_MARKER_API
# include <likwid.h>
#endif

#include "def.h"
#include "help.h"
#include "output.h"
#include "io.h"
#include "solver.h"
#include "errorcheck.h"

#ifdef USE_OMP
# include <omp.h>
#endif

#ifdef _OPENACC
# include <accel.h>
#endif
#include <no_huge_page_alloc.h>

// init rhs, so that the solution is one, fill x with 2.34567
void
initLGS(const struct MatrixCRS *A, floatType *b, floatType *x)
{
	int i, j;
#ifdef USE_OMP
	#pragma omp parallel for private(i,j)
#endif
	for (i = 0; i < A->n; i++) {
		x[i] = 0;
		b[i] = 0;

		for (j = A->ptr[i]; j < A->ptr[i + 1]; j++)
			b[i] += A->value[j];
	}
}

int
main(int argc, char *argv[])
{
	int i;
	struct MatrixCRS A;
	struct SolverConfig sc;
	floatType *b, *x;
	double ioTime, solveTime, totalTime;
#ifdef PAGE_ALIGN
	size_t pagesize;
#endif
#ifdef TASKING
	unsigned int numTasks;
#endif
	floatType bnrm2, residual, maxError;
	int correct;

	totalTime = getWTime();

#ifdef LIKWID_MARKER_API
	likwid_markerInit();
# pragma omp parallel
	likwid_markerThreadInit();
#endif

#ifdef _OPENACC
	acc_init(acc_device_nvidia);
#endif

	if (argc != 2) {
		help(argv[0]);
		return 1;
	} else if (!strcmp(argv[1], "-h")) {
		help(argv[0]);
		return 0;
	}

	init();

#ifdef TASKING
	numTasks = config.numTasks;

	// reduce number of threads for small taks numbers
	if (numTasks < omp_get_max_threads()) {
		omp_set_num_threads(numTasks);
		COUT1("WARNING: Number of threads was reduced to %d\n",
		    numTasks);
	}

	ioTime = getWTime();
	parseMM_(argv[1], &A, numTasks);
	ioTime = getWTime() - ioTime;

	sc.chunkSize = A.n / numTasks;
	if (A.n % numTasks != 0)
		sc.chunkSize++;
	sc.numTasks = numTasks;
#else
	ioTime = getWTime();
	parseMM(argv[1], &A);
	ioTime = getWTime() - ioTime;
#endif

#ifdef PAGE_ALIGN
	pagesize = get_pagesize();
	b = (floatType*)memalign(pagesize, A.n * sizeof(floatType));
	CHKERR(b==NULL, "memalign()");
	x = (floatType*)memalign(pagesize, A.n * sizeof(floatType));
	CHKERR(x==NULL, "memalign()");
	printf("_bx_%lx_%lx_\n", b, x);
#else
	b = (floatType*)malloc(A.n * sizeof(floatType));
	CHKERR(b==NULL, "malloc()");
	x = (floatType*)malloc(A.n * sizeof(floatType));
	CHKERR(x==NULL, "malloc()");
#endif

	first_touch_vector(x, A.n, &A);
	first_touch_vector(b, A.n, &A);
/*
	// proper aligned array
	struct MatrixCRS AAA;
	AAA.n = A.n;
	AAA.nnz = A.nnz;
	AAA.ptr = (int *) alloc((A.n+1) * sizeof(int));
	AAA.index = (int *) alloc(A.nnz * sizeof(int));
	AAA.value = (floatType *) alloc(A.nnz * sizeof(floatType));

	int tmp_i;
#pragma omp parallel for schedule(static)
  for(tmp_i = 0; tmp_i < A.n; tmp_i++){
		AAA.ptr[tmp_i] = 0;
	}
#pragma omp parallel for schedule(static)
  for(tmp_i = 0; tmp_i < A.nnz; tmp_i++){
		AAA.index[tmp_i] = 0;
		AAA.value[tmp_i] = 0;
	}

	for(tmp_i = 0; tmp_i < A.n; tmp_i++){
		AAA.ptr[tmp_i] = A.ptr[tmp_i];
	}	
	first_touch_mtx(&AAA, omp_get_max_threads());
	// copy into proper aligned matrix
	for(tmp_i = 0; tmp_i < A.nnz; tmp_i++){
		AAA.index[tmp_i] = A.index[tmp_i];
		AAA.value[tmp_i] = A.value[tmp_i];
	}
	// replace A with AAA
	A = AAA;
*/	
  if (config.rhs != NULL)
		parseVector(config.rhs, b, A.n);
	else
		initLGS(&A, b, x);

	// Calculate the initial residuum for error checking
	bnrm2 = get_residual(&A, b, x);

	//set solver config
	sc.maxIter = config.maxIter;
	sc.tolerance = config.tolerance;

	//print LGS
	if (A.n < 10){
		printf("Intial LGS\n");
		printMatrix(&A);
	}
	//printf("b=");
	//printVector(b, A.n);
	//printf("x=");
	//printVector(x, A.n);

	//Solving
	solveTime = getWTime();
	cg(&A, b, x, &sc);
	solveTime = getWTime()-solveTime;

	//matvec(&A, x, b);
	printVector(x, 5);

	// Check error
	residual = get_residual(&A, b, x);
	correct = check_error(bnrm2, residual, sc.tolerance);
	maxError = get_max_error(x, A.n);

	write_result(x, A.n);

	free(b);
	free(x);

#ifdef LIKWID_MARKER_API
	likwid_markerClose();
#endif

	totalTime = getWTime() - totalTime;

	output(
	    argv,
	    "NNZ", 'i', A.nnz,
	    "N", 'i', A.n,
	    "Min local NNZ", 'i', getMinNNZ(&A),
	    "Max local NNZ", 'i', getMaxNNZ(&A),
	    "Min local N", 'i', getMinN(&A),
	    "Max local N", 'i', getMaxN(&A),
	    "Max. iterations", 'i', sc.maxIter,
	    "Tolerance", 'e', sc.tolerance,
	    "Preconditioner", 's', config.precon,
#ifdef USE_OMP
	    "Threads", 'i', omp_get_max_threads(),
#endif
#ifdef TASKING
	    "Tasks", 'i', sc.numTasks,
	    "Chunk size", 'i', sc.chunkSize,
# ifdef PAR_PRODUCER
	    "Method", 's', "parallel producer",
# else
	    "Method", 's', "single producer",
# endif
#endif
#ifdef PAGE_ALIGN
	    "Page aligned", 's', "yes",
#else
	    "Page aligned", 's', "no",
#endif
#if defined(DATA_DISTRIBUTION) && !defined(STATIC_INIT)
	    "Data distribution", 's', "precalculated",
#else
	    "Data distribution", 's', "static",
#endif
#ifdef TASKING
# ifdef CHUNK_INIT
	    "Data inititialization", 's', "chunk",
# elif TASK_INIT
	    "Data inititialization", 's', "task",
# elif SERIAL_INIT
	    "Data inititialization", 's', "serial",
# else
	    "Data inititialization", 's', "static",
# endif
#endif
	    "Residual", 'e', sc.residual,
	    "Iterations", 'i', sc.iter,
	    "MatVec time", 'f', sc.timeMatvec,
	    "xpay time", 'f', sc.timeXpay,
	    "nrm2 time", 'f', sc.timeNrm2,
	    "axpy time", 'f', sc.timeAxpy,
	    "vecDot time", 'f', sc.timeVectorDot,
	    "MatVec GFLOP/s", 'f',
		1e-9 * 2 * (A.nnz + 1) * sc.iter / sc.timeMatvec,
	    "IO time", 'f', ioTime,
	    "Solve time", 'f', solveTime,
	    "Total time", 'f', totalTime,
	    "Max Error", 'e', maxError,
	    "RESULT CHECK", 's', correct == 0 ? "ERROR" : "OK",
	    (const char*)NULL
	);

	destroyMatrix(&A);

	return 0;
}
