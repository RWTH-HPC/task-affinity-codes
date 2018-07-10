#ifndef __CHECK_ERROR_H__
#define __CHECK_ERROR_H__

#include "def.h"
int check_error(const floatType bnrm2, const floatType residual, const floatType cg_tol);
floatType get_residual(const struct MatrixCRS* A, const floatType* const b, const floatType* const x);
floatType get_max_error(const floatType* const x, const int n);

#endif
