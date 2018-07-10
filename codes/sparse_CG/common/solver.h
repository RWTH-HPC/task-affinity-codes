#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "def.h"

extern void cg(const struct MatrixCRS *A, const floatType *b, floatType *x,
    struct SolverConfig *sc);
#endif
