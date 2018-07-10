#ifndef __IO_H__
#define __IO_H__

#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif

int getMinNNZ(struct MatrixCRS* A);
int getMaxNNZ(struct MatrixCRS* A);
int getMaxN(struct MatrixCRS* A);
int getMinN(struct MatrixCRS* A);
size_t get_pagesize();
void parseRHS(char *filename, floatType *b);
void first_touch_vector(floatType* vector, int n, const struct MatrixCRS* const A);
void parseMTXDump(const char *filename, struct MatrixCRS *A, int num_tasks);
void parseMM(const char *filename, struct MatrixCRS *A);
void parseMM_(const char *filename, struct MatrixCRS *A, int num_tasks);
void parseVector(const char *filename, floatType *x, int n);
void printVector(const floatType *x, int n);
void printMatrix(const struct MatrixCRS *A);
void destroyMatrix(struct MatrixCRS *A);
# ifdef DATA_DISTRIBUTION
void calc_data_dist(struct MatrixCRS *A);
void calc_data_dist_(struct MatrixCRS *A, int num_tasks);
# endif
#ifdef __cplusplus
}
#endif

#endif
