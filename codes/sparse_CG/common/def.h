#ifndef __DEF_H__
#define __DEF_H__

#include <stddef.h>
#include <errno.h>

/*
 * Only use schedule(runtime) for compilers which are known to work well with
 * it.
 *
 * It should be noted here that GCC as of 4.7 has serious problems when using
 * schedule(runtime) and no schedule has been set in the environment.
 */
#if defined(__ICC) || defined(__ADD_OTHER_COMPILERS_HERE__)
//# define RUNTIME_SCHEDULE schedule(runtime)
# define RUNTIME_SCHEDULE schedule(static)
#else
# define RUNTIME_SCHEDULE
#endif

#ifndef NDEBUG
# define COUT(x) { printf("[DEBUG:] "); printf(x); fflush(stdout); }
# define COUT1(x, a) { printf("[DEBUG:] "); printf(x, a); fflush(stdout); }
# define COUT2(x, a, b) { printf("[DEBUG:] "); printf(x, a, b); fflush(stdout); }
# define COUT3(x, a, b, c) { printf("[DEBUG:] "); printf(x, a, b, c); fflush(stdout); }
#else
# define COUT(x)
# define COUT1(x, a)
# define COUT2(x, a, b)
# define COUT3(x, a, b, c)
#endif

#define CHKERR(b,s) \
	if (b) { \
		printf("%s failed: %s:%i %s\n", s, __FILE__, __LINE__, \
		    strerror(errno)); \
		exit(1); \
	}


#ifdef USE_CUDA
# define BLOCK_DIM 512  // TODO: DO THIS GENERIC
// TODO: Do this also for non debug mode?
# define CUCHK(x)							    \
	if (x != cudaSuccess) {						    \
		printf("CUDA-ERROR %d: %s:%i: %s\n", x, __FILE__, __LINE__, \
		    cudaGetErrorString(x));				    \
		exit(1);						    \
	}
#endif

typedef double floatType;

#ifdef __cplusplus
extern "C" {
#endif
extern struct config {
	int maxIter;
	floatType tolerance;
	const char *outputFile;
	const char *rhs;
#ifdef TASKING
	int numTasks;
#endif
	const char *precon;
} config;
#ifdef __cplusplus
}
#endif

struct MatrixCRS {
	char mmapped;
	int n;
	int nnz;
	int *ptr;
	int *index;
	floatType *value;
#ifdef DATA_DISTRIBUTION
	int *blockptr;
#endif
};

struct SolverConfig {
	floatType tolerance;
	int iter;
	int maxIter;
#ifdef TASKING
	int numTasks;
	int chunkSize;
#endif
	floatType residual;
	floatType timeMatvec;
	floatType timeXpay;
	floatType timeNrm2;
	floatType timeAxpy;
	floatType timeVectorDot;
};

enum NrmPhase {
	WarmUp,
	Reduce
};

enum ReduceOperation {
	Nrm2,
	VectorDot
};

#ifdef __cplusplus
extern "C" {
#endif
extern void init(void);
#ifdef __GNUC__
//__attribute__((target(mic)))
#endif
double getWTime(void);
extern unsigned int nextPow2(unsigned int x);

# ifdef USE_CUDA
void getKernelConfig(const int size, int *blocks, int *threads, size_t *smem);
# endif
#ifdef __cplusplus
}
#endif

#endif
