#ifndef __IO_H__
#define __IO_H__

#include "def.h"

void parseMM(char* filename, struct MatrixCRS* A);
void printVector(const floatType* x, int n);
void printMatrix(const struct MatrixCRS* A);
void destroyMatrix(struct MatrixCRS* A);

#endif
