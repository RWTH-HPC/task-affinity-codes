#include <stdlib.h>
#include <string.h>

#include <lis.h>

#include "def.h"
#include "io.h"
#include "errorcheck.h"
#include "help.h"
#include "output.h"

#define CHECK(x)							\
	do {								\
		int retcode = x;					\
		if (retcode != LIS_SUCCESS) {				\
			printf("Error in line %d:\n%s\nError: %s\n",	\
			    __LINE__, #x, retcode2str(retcode));	\
			abort();					\
		}							\
	} while(0)

static char*
retcode2str(int x)
{
	switch (x) {
	case 0:
		return "LIS_SUCCESS";
	case 1:
		return "LIS_ILL_OPTION";
	case 2:
		return "LIS_BREAKDOWN";
	case 3:
		return "LIS_OUT_OF_MEMORY";
	case 4:
		return "LIS_MAXITER";
	case 5:
		return "LIS_NOT_IMPLEMENTED";
	case 6:
		return "LIS_ERR_FILE_IO";
	default:
		return "unknown";
	}
}

static void
parse_args(LIS_SOLVER solver, int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		char *arg;

		if (argv[i][0] != '-')
			continue;

		arg = strdup(argv[i]);

		if (i + 1 < argc && argv[i + 1][0] != '-') {
			size_t len1 = strlen(arg);
			size_t len2 = strlen(argv[i + 1]);
			size_t len = len1 + 1 + len2;

			if ((arg = realloc(arg, len + 1)) == NULL) {
				fputs("Out of memory!\n", stderr);
				exit(1);
			}

			arg[len1] = ' ';
			memcpy(arg + len1 + 1, argv[i + 1], len2);
			arg[len] = 0;
		}

		printf("lis_set_option(\"%s\", solver)\n", arg);
		CHECK(lis_solver_set_option(arg, solver));

		free(arg);
	}
}

int
main(int argc, char *argv[])
{
	double ioTime, solveTime, totalTime;
	int fake_argc = 1;
	char *fake_argv[1] = { argv[0] };
	floatType bnrm2, residual_end;
	int correct;
	floatType maxError;
	struct MatrixCRS A_chk;	// Use CRS struct for error checking

	totalTime = getWTime();

	if (argc < 2) {
		help(argv[0]);
		return 1;
	} else if (!strcmp(argv[1], "-h")) {
		help(argv[0]);
		return 0;
	}

	init();

	CHECK(lis_initialize(&fake_argc, (char***)&fake_argv));

	int i, j, n;

  ioTime = getWTime();
	LIS_MATRIX A;
	CHECK(lis_matrix_create(0, &A));
#ifdef USE_LIS_IO
	CHECK(lis_input_matrix(A, argv[1]));
#else
	struct MatrixCRS A_;

	parseMM(argv[1], &A_);

	CHECK(lis_matrix_set_size(A, 0, A_.n));
	CHECK(lis_matrix_set_csr(A_.nnz, A_.ptr, A_.index, A_.value, A));
#endif
	ioTime = getWTime() - ioTime;
	CHECK(lis_matrix_assemble(A));
	CHECK(lis_matrix_get_size(A, &n, &n));

	LIS_VECTOR b;
	CHECK(lis_vector_create(0, &b));
	CHECK(lis_vector_set_size(b, 0, n));

	LIS_VECTOR x;
	CHECK(lis_vector_create(0, &x));
	CHECK(lis_vector_set_size(x, 0, n));

	// TODO: First touch
	//       (How? Access Lis's internal data structure?)
	for (i = 0; i < n; i++) {
		lis_vector_set_value(LIS_INS_VALUE, i, 0, b);
		lis_vector_set_value(LIS_INS_VALUE, i, 0, x);

		if (config.rhs == NULL)
			for (j = A->ptr[i]; j < A->ptr[i + 1]; j++)
				b->value[i] += A->value[j];
	}

	if (config.rhs != NULL)
		parseVector(config.rhs, b->value, n);

	// for error checking fill own data struct
	A_chk.nnz = A_.nnz;
	A_chk.n = A_.n;
	A_chk.ptr = A_.ptr;
	A_chk.index = A_.index;
	A_chk.value = A_.value;

	// Calculate the initial residuum for error checking
	bnrm2 = get_residual(&A_chk, b->value, x->value);

	LIS_SOLVER solver;
	CHECK(lis_solver_create(&solver));
	CHECK(lis_solver_set_option("-i cg", solver));

	if (!strcasecmp(config.precon, "jacobi"))
		CHECK(lis_solver_set_option("-p jacobi", solver));
	else if (!strcmp(config.precon, "ssor"))
		CHECK(lis_solver_set_option("-p ssor", solver));
	else {
		config.precon = "none";
		CHECK(lis_solver_set_option("-p none", solver));
	}

	//CHECK(lis_solver_set_option("-i cg -p ssor -print out", solver));
	char *maxIter;
	/* Without -1, lis does one too much for some reason */
	asprintf(&maxIter, "-maxiter %d", config.maxIter - 1);
	CHECK(lis_solver_set_option(maxIter, solver));
	free(maxIter);
	char *tol;
	asprintf(&tol, "-tol %e", config.tolerance);
	CHECK(lis_solver_set_option(tol, solver));
	free(tol);

	parse_args(solver, argc, argv);

	solveTime = getWTime();
	CHECK(lis_solve(A, b, x, solver));
	solveTime = getWTime()-solveTime;

//	lis_vector_print(x);
	printVector(x->value, 10);

	int iters;
	CHECK(lis_solver_get_iters(solver, &iters));

	LIS_REAL residual;
	CHECK(lis_solver_get_residualnorm(solver, &residual));

	// Check error
	residual_end = get_residual(&A_chk, b->value, x->value);
	correct = check_error(bnrm2, residual_end, config.tolerance);
	maxError = get_max_error(x->value, n);

	write_result(x->value, n);

	totalTime = getWTime() - totalTime;

	output(
	    argv,
	    "Max. iterations", 'i', config.maxIter,
	    "Tolerance", 'e', config.tolerance,
	    "Preconditioner", 's', config.precon,
	    "Residual", 'e', residual,
	    "Iterations", 'i', iters,
	    "IO time", 'f', ioTime,
	    "Solve time", 'f', solveTime,
	    "Total time", 'f', totalTime,
	    "Max Error", 'e', maxError,
	    "RESULT CHECK", 's', correct == 0 ? "ERROR" : "OK",
	    (const char*)NULL
	);

	return 0;
}
