#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "def.h"

void cg(const struct MatrixCRS* A, const floatType* b,floatType* x, struct SolverConfig* sc);
__global__ void matvec(const struct MatrixCRS* A, const floatType* x, floatType* y);
__global__ void xpay(const floatType* x, const floatType a, const int n, floatType* y);

#endif
