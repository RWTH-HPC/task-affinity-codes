#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <sys/time.h>
#endif

#include "def.h"

struct config config = {
	.maxIter = 1000,
	.tolerance = 0.0000001
};

void
init()
{
	const char *tmp;

	if ((tmp = getenv("CG_MAX_ITER")) != NULL)
		config.maxIter = atoi(tmp);

	if ((tmp = getenv("CG_TOLERANCE")) != NULL)
		config.tolerance = strtod(tmp, NULL);

	if ((config.precon = getenv("CG_PRECON")) == NULL)
		config.precon = "";

	config.outputFile = getenv("CG_OUTPUT_FILE");
	config.rhs = getenv("CG_RHS");

#ifdef TASKING
	if ((tmp = getenv("CG_NUM_TASKS")) != NULL)
		config.numTasks = atoi(tmp);
	else
		config.numTasks = omp_get_max_threads();
#endif
}

//__attribute__((target(mic))) double
double getWTime()
{
#ifdef USE_CUDA
	/* This is important, because kernel calls are asynchronous */
	/*
	 * TODO: Work around for tracing due to bug in cuda trace. Uncomment
	 *       this again as soon as possible!
	 */
	cudaThreadSynchronize();
#endif
	// Windows part ist stolen from CT
#if defined(_WIN32) || defined(_WIN64)
# define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + \
                      (double)((x).LowPart))
	LARGE_INTEGER time, freq;
	double dtime, dfreq, res;

	if (QueryPerformanceCounter(&time) == 0) {
		DWORD err = GetLastError();
		LPVOID buf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		    FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buf,
		    0, NULL);
		printf("QueryPerformanceCounter() failed with error %d: %s\n",
		    err, buf);
		exit(1);
	}

	if (QueryPerformanceFrequency(&freq) == 0)
	{
		DWORD err = GetLastError();
		LPVOID buf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		    FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buf,
		    0, NULL);
		printf("QueryPerformanceFrequency() failed with error %d: %s\n",
		    err, buf);
		exit(1);
	}

	dtime = Li2Double(time);
	dfreq = Li2Double(freq);
	res = dtime / dfreq;

	return res;
#else
	struct timeval tv;
	gettimeofday(&tv, (struct timezone*)0);
	return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0 );
#endif
}

unsigned int
nextPow2(unsigned int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return ++x;
}

#ifdef USE_CUDA
void
getKernelConfig(const int size, int *blocks, int *threads, size_t *smem)
{
	*threads = (size < BLOCK_DIM) ? nextPow2(size) : BLOCK_DIM;
	*blocks = (size + *threads - 1) / *threads;
	*smem = *threads * sizeof(floatType); // TODO: Do this with templates?
	// COUT3("Configured kernel with %d threads and %d blocks. smem=%d\n",
	//     *threads, *blocks, *smem);
}
#endif
