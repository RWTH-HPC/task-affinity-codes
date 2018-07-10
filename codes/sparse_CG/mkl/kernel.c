#include <math.h>
#include <omp.h>
#include <mkl_blas.h>
#include <mkl_spblas.h>

#include "kernel.h"

static char matdescra[] = { 'G', 0, 0, 'C', 0, 0 };
static double one = 1;
static double zero = 0;
static int intone = 1;

// ab <- a' * b
void
vectorDot_kernel(const floatType *a, const floatType *b, const int n,
    floatType *ab)
{
	*ab = ddot(&n, a, &intone, b, &intone);
}

// y <- ax + y
void
axpy_kernel(const floatType a, const floatType *x, const int n, floatType *y)
{
	daxpy(&n, &a, x, &intone, y, &intone);
}

// y <- x + ay
void
xpay_kernel(const floatType *x, const floatType a, const int n, floatType *y)
{
	dscal(&n, &a, y, &intone);
	daxpy(&n, &one, x, &intone, y, &intone);
}

// y <- A * x
void
matvec_kernel(const struct MatrixCRS *A, const floatType *x, floatType *y)
{
	mkl_dcsrmv("n", (int*)&A->n, (int*)&A->n, &one, matdescra, A->value,
	    A->index, A->ptr, A->ptr + 1, (floatType*)x, &zero, y);
}

// nrm <- ||x||_2
void nrm2_kernel(const floatType *x, const int n, floatType *nrm)
{
	*nrm = dnrm2(&n, x, &intone);
}
