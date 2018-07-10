#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "def.h"


__attribute__((target(mic))) void vectorDot(const floatType* a, const floatType* b, const int n, floatType* ab);
__attribute__((target(mic))) void axpy(const floatType a, const floatType* x, const int n, floatType* y);
__attribute__((target(mic))) void xpay(const floatType* x, const floatType a, const int n, floatType* y);
__attribute__((target(mic))) void matvec(const struct MatrixCRS* A, const floatType* x, floatType* y);
__attribute__((target(mic))) void nrm2(const floatType* x, const int n, floatType* nrm);


#endif
