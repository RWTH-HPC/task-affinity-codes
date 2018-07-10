#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "def.h"

void cg(const struct MatrixCRS* A, const floatType* b,floatType* x, struct SolverConfig* sc);
void matvec(const struct MatrixCRS* A, const floatType* x, floatType* y);
void xpay(const floatType* x, const floatType a, const int n, floatType* y);

#endif
