#include "errorcheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Check if the normalized residual (which is a ration of the final and the
// inital residual) is smaller than the specified CG tolerance. 0 is returned
// in case of errors, 1 otherwise.
int
check_error(const floatType bnrm2, const floatType residual,
    const floatType cg_tol)
{
	return ((residual / bnrm2) <= cg_tol ? 1 : 0);
}

// Returns the maximum absolute error
floatType
get_max_error(const floatType *const x, const int n)
{
	floatType maxError = 0.0;
	int i;

	for (i = 0; i < n; i++) {
		// FIXME: This will only work if solution is 1
		floatType err = fabs(x[i] - 1);
		if (err > maxError)
			maxError = err;
	}

	return maxError;
}

// Calculate the current residual for error checking. You must not change this
// function, it is not used to during the algorithm. There is no need to
// parallelize it.
floatType
get_residual(const struct MatrixCRS *const A, const floatType *const b,
    const floatType *const x)
{
	int i, j;
	floatType *y;
	floatType residual;

	// Allocate residual vector
	y = (floatType*)malloc(A->n * sizeof(floatType));

	// y = A * x
	for (i = 0; i < A->n; i++) {
		y[i] = 0;

		for (j = A->ptr[i]; j < A->ptr[i + 1]; j++)
			y[i] += A->value[j] * x[A->index[j]];
	}

	// y = | b - y |
	for (i = 0; i < A->n; i++)
		y[i] = fabs(b[i]-y[i]);

	// residual = || y ||_2
	residual = 0;
	for (i = 0; i < A->n; i++)
		residual += y[i] * y[i];
	residual = sqrt(residual);

	// Clean up
	free(y);

	return residual;
}
