#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_OMP
# include <omp.h>
#endif

#ifdef LIKWID_MARKER_API
# include <likwid.h>
#endif

#include "solver.h"
#include "kernel.h"
#include "output.h"

static double timeMatvec = 0;
static double timeVectorDot = 0;
static double timeAxpy = 0;
static double timeXpay = 0;
static double timeNrm2 = 0;

void
vectorDot(const floatType *a, const floatType *b, const int n, floatType *ab)
{
	double timeVectorDot_s = getWTime();
	vectorDot_kernel(a, b, n, ab);
	timeVectorDot += getWTime() - timeVectorDot_s;
}

void
axpy(const floatType a, const floatType *x, const int n, floatType *y)
{
	double timeAxpy_s = getWTime();
	axpy_kernel(a, x, n, y);
	timeAxpy += getWTime() - timeAxpy_s;
}

void
xpay(const floatType *x, const floatType a, const int n, floatType *y)
{
	double timeXpay_s = getWTime();
	xpay_kernel(x, a, n, y);
	timeXpay += getWTime() - timeXpay_s;
}

void
matvec(const struct MatrixCRS *A, const floatType *x, floatType *y)
{
	double timeMatvec_s = getWTime();
	matvec_kernel(A, x, y);
	timeMatvec += getWTime() - timeMatvec_s;
}

void
nrm2(const floatType *x, const int n, floatType *nrm)
{
	double timeNrm2_s = getWTime();
	nrm2_kernel(x, n, nrm);
	timeNrm2 += getWTime() - timeNrm2_s;
}

static void
no_precon(const struct MatrixCRS *A, const floatType *r, floatType *z)
{
}

static void
jacobi(const struct MatrixCRS *A, const floatType *r, floatType *z)
{
	int i, j;

#if defined(USE_OMP)
# pragma omp parallel for private(i, j) RUNTIME_SCHEDULE
#endif
	for (i = 0; i < A->n; i++) {
		for (j = A->ptr[i]; A->index[j] < i; j++);

		if (A->index[j] != i) {
			puts("No value on diagonal!");
			abort();
		}

		if (A->value[j] == 0) {
			puts("Zero on diagonal! Don't use Jacobi!");
			abort();
		}

		// @Jonathan: Felix sagt man muss nur 1/diagonalElement rechnen
		z[i] = r[i] / A->value[j];
	}
}

// From lis:
/***************************************
 * Preconditioned Conjugate Gradient   *
 ***************************************
 r(0)    = b - Ax(0)
 rho(-1) = 1
 p(0)    = (0,...,0)^T
 ***************************************
 for k=1,2,...
   z(k-1)    = M^-1 * r(k-1)
   rho(k-1)  = <r(k-1),z(k-1)>
   beta      = rho(k-1) / rho(k-2)
   p(k)      = z(k-1) + beta*p(k-1)
   q(k)      = A * p(k)
   dot_pq    = <p(k),q(k)>
   alpha     = rho(k-1) / dot_pq
   x(k)      = x(k-1) + alpha*p(k)
   r(k)      = r(k-1) - alpha*q(k)
 ***************************************/
void
cg(const struct MatrixCRS *A, const floatType *b, floatType *x,
    struct SolverConfig *sc)
{
	floatType *r, *p, *q, *z;
	floatType alpha, beta, rho, rho_old, dot_pq, bnrm2;
	int i, iter, n, nnz;
#ifdef _OPENACC
	int *ptr, *index;
	floatType *value;
#endif
	size_t pagesize;
	void (*precon)(const struct MatrixCRS*, const floatType*, floatType*);

	n = A->n;
	nnz = A->nnz;
#ifdef _OPENACC
	ptr = A->ptr;
	index = A->index;
	value = A->value;
#endif

#ifdef PAGE_ALIGN
	pagesize = get_pagesize();
	r = memalign(pagesize, n * sizeof(floatType));
	CHKERR(x == NULL, "memalign()");
	p = memalign(pagesize, n * sizeof(floatType));
	CHKERR(x == NULL, "memalign()");
	q = memalign(pagesize, n * sizeof(floatType));
	CHKERR(x == NULL, "memalign()");
	z = memalign(pagesize, n * sizeof(floatType));
	CHKERR(x == NULL, "memalign()");
	printf("_rpq_%lx_%lx_%lx_\n", r, p, q);
#else
	// allocate memory
	r = malloc(n * sizeof(floatType));
	p = malloc(n * sizeof(floatType));
	q = malloc(n * sizeof(floatType));
#endif

	if (!strcasecmp(config.precon, "jacobi")) {
		precon = jacobi;
		z = malloc(n * sizeof(floatType));
		first_touch_vector(z, n, A);
	} else {
		config.precon = "none";
		precon = no_precon;
		z = r;
	}


	first_touch_vector(r, n, A);
	first_touch_vector(p, n, A);
	first_touch_vector(q, n, A);

#ifdef _OPENACC
# pragma acc data copyin(ptr[0:n+1], index[0:nnz], value[0:nnz], b[0:n]) \
		  copy(x[0:n]) create(r[0:n], p[0:n], q[0:n])
	{
#endif
	
	
	// r(0)    = b - Ax(0)
	matvec(A, x, r);
	xpay(b, -1.0, n, r);

	// calculate initial residuum
	nrm2(r, n, &bnrm2);

	bnrm2 = 1.0 / bnrm2;
	printf("bnrm %e\n", bnrm2);

	beta = 0;
	rho = 0;
	// p = 0
	memset(p, 0, n * sizeof(floatType));
#ifdef _OPENACC
# pragma acc update device(p[0:n])
#endif

	rho_old = 1;

	for (iter = 0; iter < sc->maxIter; iter++) {
		precon(A, r, z);

		// rho(k + 1) = <r(k + 1), z(k + 1)>
		vectorDot(r, z, n, &rho);

		// beta = rho(k + 1) / rho(k)
		beta = rho / rho_old;

		// p(k + 1) = z(k + 1) + beta * p(k)
		xpay(z, beta, n, p);

		// q(k) = A * p(k)
		matvec(A, p, q);

		// dot_pq = <p(k), q(k)>
		vectorDot(p, q, n, &dot_pq);

		// alpha = rho(k) / dot_pq
		alpha = rho / dot_pq;

		// x(k + 1) = x(k) + alpha * p(k)
		// r(k + 1) = r(k) - alpha * q(k)
		axpy(alpha, p, n, x);
		axpy(-alpha, q, n, r);

		// check convergence ||r(k + 1)||_2 < eps
		// printVector(r, n);
		// printVector(x, n);

		// Calculate relative residuum
		nrm2(r, n, &sc->residual);
		sc->residual *= bnrm2;

		COUT2("res_%d=%e\n", iter + 1, sc->residual);
		if (sc->residual <= sc->tolerance)
			break;

		rho_old = rho;
	}
#ifdef _OPENACC
	}
#endif

	sc->iter = iter;
	sc->timeMatvec = timeMatvec;

	sc->timeXpay = timeXpay;
	sc->timeNrm2 = timeNrm2;
	sc->timeAxpy = timeAxpy;
	sc->timeVectorDot = timeVectorDot;

	// printf("x=");
	// printVector(x,n);
	// printf("r=");
	// printVector(r,n);

	free(r);
	free(p);
	free(q);
  printf("Elapsed time for program\t%lf\tsec\n",timeMatvec);

	if (z != r)
		free(z);
}
