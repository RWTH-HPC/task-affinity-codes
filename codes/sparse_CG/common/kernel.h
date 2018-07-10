#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "def.h"

// ab <- a' * b
extern void vectorDot_kernel(const floatType *a, const floatType *b,
    const int n, floatType *ab);
// y <- ax + y
extern void axpy_kernel(const floatType a, const floatType *x, const int n,
    floatType *y);
// y <- x + ay
extern void xpay_kernel(const floatType *x, const floatType a, const int n,
    floatType *y);
// y <- A * x
extern void matvec_kernel(const struct MatrixCRS *A, const floatType *x,
    floatType *y);
// nrm <- ||x||_2
extern void nrm2_kernel(const floatType *x, const int n, floatType *nrm);

#endif
