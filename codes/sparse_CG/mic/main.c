#include <stdlib.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

#include "def.h"
#include "help.h"
#include "output.h"
#include "io.h"
#include "solver.h"
#include "errorcheck.h"

// init rhs, so that the solution is one, fill x with 2.34567
void initLGS(const struct MatrixCRS* A, floatType* b, floatType* x){
	int i,j;
	#pragma omp parallel for private(i,j)
	for(i = 0; i < A->n; i++){
		x[i]=0;//2.34567;
		b[i]=0;
		for(j = A->ptr[i]; j < A->ptr[i+1]; j++){
			b[i]+=A->value[j];
		}
		//		b[i]*=1000;
	}
}

int main(int argc, char** argv){
	struct MatrixCRS A;
	struct SolverConfig sc;
	floatType* b, *x;
	floatType bnrm2, residual;
	double ioTime, solveTime, totalTime;
	int correct;
	floatType maxError;

  if (argc != 2) {
    help(argv[0]);
    return 1;
  } else if (!strcmp(argv[1], "-h")) {
    help(argv[0]);
    return 0;
  }

  init();

	totalTime=getWTime();

	ioTime = getWTime();
	parseMM(argv[1], &A);// parse and init LGS (only one processor)
	ioTime = getWTime() - ioTime;

	b = (floatType*)malloc(A.n * sizeof(floatType));
	x = (floatType*)malloc(A.n * sizeof(floatType));
	initLGS(&A, b, x);

  // Calculate the initial residuum for error checking
	bnrm2 = get_residual(&A, b, x);
 
	//printMatrix(&A);
	//set solver config
	sc.maxIter=config.maxIter;
	sc.tolerance=config.tolerance;

	//print LGS
	//printf("Intial LGS\n");
	//printMatrix(&A);
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

	// Clean up
	free(b);
	free(x);

	totalTime=getWTime()-totalTime;

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
      "Threads", 'i', omp_get_max_threads(),
#ifdef DATA_DISTRIBUTION
      "Data distribution", 's', "precalculated",
#else
      "Data distribution", 's', "static",
#endif
      "Residual", 'e', sc.residual,
      "Iterations", 'i', sc.iter,
      "MatVec time", 'f', sc.timeMatvec,
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
