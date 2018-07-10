#include <math.h>

#ifdef USE_OMP
# include <omp.h>
#endif

#include "kernel.h"

// ab <- a' * b
void
vectorDot_kernel(const floatType *a, const floatType *b, const int n,
    floatType *ab)
{
	int i;
	floatType sum = 0;

#if defined(USE_OMP)
# pragma omp parallel for private(i) reduction(+:sum) RUNTIME_SCHEDULE
#elif defined(_OPENACC)
# pragma acc kernels present(a[0:n], b[0:n]) //vector_length(BLOCK_SIZE)
# pragma acc loop gang vector(256) independent reduction(+:sum)
# pragma unroll(32)
#endif
	for (i = 0; i < n; i++)
		sum += a[i] * b[i];

	(*ab) = sum;
}

// y <- ax + y
void
axpy_kernel(const floatType a, const floatType *x, const int n, floatType *y)
{
	int i;

#if defined(USE_OMP)
# pragma omp parallel for private(i) RUNTIME_SCHEDULE
#elif defined(_OPENACC)
# pragma acc kernels present(x[0:n], y[0:n])
# pragma acc loop independent
#endif
	for (i = 0; i < n; i++)
		y[i] = a * x[i] + y[i];
}

// y <- x + ay
void
xpay_kernel(const floatType *x, const floatType a, const int n, floatType *y)
{
	int i;

#if defined(USE_OMP)
# pragma omp parallel for private(i) RUNTIME_SCHEDULE
#elif defined(_OPENACC)
# pragma acc kernels present(x[0:n], y[0:n])
# pragma acc loop independent
#endif
	for (i = 0; i < n; i++)
		y[i] = x[i] + a * y[i];
}

// y <- A * x
void
matvec_kernel(const struct MatrixCRS *A, const floatType *x, floatType *y)
{
	int i, j;
	const int *index = A->index;
	const floatType *value = A->value;
#ifdef _OPENACC
	int n = A->n;
	int nnz = A->nnz;
	const int *ptr = A->ptr;

# pragma acc kernels present(ptr[0:n+1], index[0:nnz], value[0:nnz], x[0:n], y[0:n])
# pragma acc loop independent private(i) gang, vector(32)
	for (i = 0; i < n; i++) {
		floatType tmp = 0.0;
		for (j = ptr[i]; j < ptr[i + 1]; j++)
			tmp += value[j] * x[index[j]];
		y[i] = tmp;
	}
#else

# ifdef LIKWID_MARKER_API
#  pragma omp parallel
  likwid_markerStartRegion("matvec");
# endif

int is, ie, j0;
	double y0;
# ifdef DATA_DISTRIBUTION
	int thread, bs, be;
#  pragma omp parallel private(i,j,is,ie,j0,y0, thread, bs, be)
	{

	thread = omp_get_thread_num();
	bs = A->blockptr[thread];
	be = A->blockptr[thread + 1];
	for (i = bs; i < be; i++) {
# else
#  ifdef USE_OMP
#   pragma omp parallel for private(i,j,is,ie,j0,y0) RUNTIME_SCHEDULE
#  endif
	for (i = 0; i < A->n; i++) {
# endif
		y0 = 0;
		is = A->ptr[i];
		ie = A->ptr[i + 1];
		/*
		 * forcing vectorization results in worse results on nehalem ep
		 * with aligned vector it is bit better for laplace stencil, but
		 * crashes with PMS (intel 12)
		 */
//		#pragma vector always
//		#pragma vector aligned
		for (j = is; j < ie; j++) {
			j0 = index[j];
			y0 += value[j] * x[j0];
		}
		y[i] = y0;
	}

# ifdef LIKWID_MARKER_API
	likwid_markerStopRegion("matvec");
# endif

# ifdef DATA_DISTRIBUTION
	} //end parallel
# endif
#endif

}

// nrm <- ||x||_2
void
nrm2_kernel(const floatType *x, const int n, floatType *nrm)
{
	int i;
	floatType sum = 0;

#if defined(USE_OMP)
# pragma omp parallel for private(i) reduction(+:sum) RUNTIME_SCHEDULE
#elif defined(_OPENACC)
# pragma acc kernels present(x[0:n])
# pragma acc loop independent reduction(+:sum)
#endif
	for (i = 0; i < n; i++)
		sum += x[i] * x[i];

	*nrm = sqrt(sum);
}
