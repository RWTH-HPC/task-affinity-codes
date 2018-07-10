/*-----------------------------------------------------------------------*/
/* Program: STREAM                                                       */
/* Revision: $Id: stream.c,v 5.10 2013/01/17 16:01:06 mccalpin Exp mccalpin $ */
/* Original code developed by John D. McCalpin                           */
/* Programmers: John D. McCalpin                                         */
/*              Joe R. Zagar                                             */
/*                                                                       */
/* This program measures memory transfer rates in MB/s for simple        */
/* computational kernels coded in C.                                     */
/*-----------------------------------------------------------------------*/
/* Copyright 1991-2013: John D. McCalpin                                 */
/*-----------------------------------------------------------------------*/
/* License:                                                              */
/*  1. You are free to use this program and/or to redistribute           */
/*     this program.                                                     */
/*  2. You are free to modify this program for your own use,             */
/*     including commercial use, subject to the publication              */
/*     restrictions in item 3.                                           */
/*  3. You are free to publish results obtained from running this        */
/*     program, or from works that you derive from this program,         */
/*     with the following limitations:                                   */
/*     3a. In order to be referred to as "STREAM benchmark results",     */
/*         published results must be in conformance to the STREAM        */
/*         Run Rules, (briefly reviewed below) published at              */
/*         http://www.cs.virginia.edu/stream/ref.html                    */
/*         and incorporated herein by reference.                         */
/*         As the copyright holder, John McCalpin retains the            */
/*         right to determine conformity with the Run Rules.             */
/*     3b. Results based on modified source code or on runs not in       */
/*         accordance with the STREAM Run Rules must be clearly          */
/*         labelled whenever they are published.  Examples of            */
/*         proper labelling include:                                     */
/*           "tuned STREAM benchmark results"                            */
/*           "based on a variant of the STREAM benchmark code"           */
/*         Other comparable, clear, and reasonable labelling is          */
/*         acceptable.                                                   */
/*     3c. Submission of results to the STREAM benchmark web site        */
/*         is encouraged, but not required.                              */
/*  4. Use of this program or creation of derived works based on this    */
/*     program constitutes acceptance of these licensing restrictions.   */
/*  5. Absolutely no warranty is expressed or implied.                   */
/*-----------------------------------------------------------------------*/
# include <stdio.h>
# include <unistd.h>
# include <math.h>
# include <float.h>
# include <limits.h>
# include <sys/time.h>
#include <sys/mman.h>

#include <numa.h>
#include <numaif.h>

#include <errno.h>

/*-----------------------------------------------------------------------
 * INSTRUCTIONS:
 *
 *	1) STREAM requires different amounts of memory to run on different
 *           systems, depending on both the system cache size(s) and the
 *           granularity of the system timer.
 *     You should adjust the value of 'STREAM_ARRAY_SIZE' (below)
 *           to meet *both* of the following criteria:
 *       (a) Each array must be at least 4 times the size of the
 *           available cache memory. I don't worry about the difference
 *           between 10^6 and 2^20, so in practice the minimum array size
 *           is about 3.8 times the cache size.
 *           Example 1: One Xeon E3 with 8 MB L3 cache
 *               STREAM_ARRAY_SIZE should be >= 4 million, giving
 *               an array size of 30.5 MB and a total memory requirement
 *               of 91.5 MB.  
 *           Example 2: Two Xeon E5's with 20 MB L3 cache each (using OpenMP)
 *               STREAM_ARRAY_SIZE should be >= 20 million, giving
 *               an array size of 153 MB and a total memory requirement
 *               of 458 MB.  
 *       (b) The size should be large enough so that the 'timing calibration'
 *           output by the program is at least 20 clock-ticks.  
 *           Example: most versions of Windows have a 10 millisecond timer
 *               granularity.  20 "ticks" at 10 ms/tic is 200 milliseconds.
 *               If the chip is capable of 10 GB/s, it moves 2 GB in 200 msec.
 *               This means the each array must be at least 1 GB, or 128M elements.
 *
 *      Version 5.10 increases the default array size from 2 million
 *          elements to 10 million elements in response to the increasing
 *          size of L3 caches.  The new default size is large enough for caches
 *          up to 20 MB. 
 *      Version 5.10 changes the loop index variables from "register int"
 *          to "ssize_t", which allows array indices >2^32 (4 billion)
 *          on properly configured 64-bit systems.  Additional compiler options
 *          (such as "-mcmodel=medium") may be required for large memory runs.
 *
 *      Array size can be set at compile time without modifying the source
 *          code for the (many) compilers that support preprocessor definitions
 *          on the compile line.  E.g.,
 *                gcc -O -DSTREAM_ARRAY_SIZE=100000000 stream.c -o stream.100M
 *          will override the default size of 10M with a new size of 100M elements
 *          per array.
 */
#ifndef STREAM_ARRAY_SIZE
#   define STREAM_ARRAY_SIZE	260000000
#endif

/*  2) STREAM runs each kernel "NTIMES" times and reports the *best* result
 *         for any iteration after the first, therefore the minimum value
 *         for NTIMES is 2.
 *      There are no rules on maximum allowable values for NTIMES, but
 *         values larger than the default are unlikely to noticeably
 *         increase the reported performance.
 *      NTIMES can also be set on the compile line without changing the source
 *         code using, for example, "-DNTIMES=7".
 */
#ifdef NTIMES
#if NTIMES<=1
#   define NTIMES	10
#endif
#endif
#ifndef NTIMES
#   define NTIMES	10
#endif

/*  Users are allowed to modify the "OFFSET" variable, which *may* change the
 *         relative alignment of the arrays (though compilers may change the 
 *         effective offset by making the arrays non-contiguous on some systems). 
 *      Use of non-zero values for OFFSET can be especially helpful if the
 *         STREAM_ARRAY_SIZE is set to a value close to a large power of 2.
 *      OFFSET can also be set on the compile line without changing the source
 *         code using, for example, "-DOFFSET=56".
 */
#ifndef OFFSET
#   define OFFSET	0
#endif

/*
 *	3) Compile the code with optimization.  Many compilers generate
 *       unreasonably bad code before the optimizer tightens things up.  
 *     If the results are unreasonably good, on the other hand, the
 *       optimizer might be too smart for me!
 *
 *     For a simple single-core version, try compiling with:
 *            cc -O stream.c -o stream
 *     This is known to work on many, many systems....
 *
 *     To use multiple cores, you need to tell the compiler to obey the OpenMP
 *       directives in the code.  This varies by compiler, but a common example is
 *            gcc -O -fopenmp stream.c -o stream_omp
 *       The environment variable OMP_NUM_THREADS allows runtime control of the 
 *         number of threads/cores used when the resulting "stream_omp" program
 *         is executed.
 *
 *     To run with single-precision variables and arithmetic, simply add
 *         -DSTREAM_TYPE=float
 *     to the compile line.
 *     Note that this changes the minimum array sizes required --- see (1) above.
 *
 *     The preprocessor directive "TUNED" does not do much -- it simply causes the 
 *       code to call separate functions to execute each kernel.  Trivial versions
 *       of these functions are provided, but they are *not* tuned -- they just 
 *       provide predefined interfaces to be replaced with tuned code.
 *
 *
 *	4) Optional: Mail the results to mccalpin@cs.virginia.edu
 *	   Be sure to include info that will help me understand:
 *		a) the computer hardware configuration (e.g., processor model, memory type)
 *		b) the compiler name/version and compilation flags
 *      c) any run-time information (such as OMP_NUM_THREADS)
 *		d) all of the output from the test case.
 *
 * Thanks!
 *
 *-----------------------------------------------------------------------*/

# define HLINE "-------------------------------------------------------------\n"

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

#ifndef SIMULTAN
#define SIMULTAN 0
#endif

// static STREAM_TYPE	a[STREAM_ARRAY_SIZE+OFFSET],
// 			b[STREAM_ARRAY_SIZE+OFFSET],
// 			c[STREAM_ARRAY_SIZE+OFFSET];

static double	avgtime[4] = {0}, maxtime[4] = {0},
		mintime[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};

static char	*label[4] = {"Copy:      ", "Scale:     ",
    "Add:       ", "Triad:     "};

// static double	bytes[4] = {
//     2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
//     2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
//     3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
//     3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE
//     };

extern double mysecond();
#ifdef _OPENMP
extern int omp_get_num_threads();
#endif
int
main()
    {
    int			quantum, checktick();
    int			BytesPerWord;
    int			k;
    ssize_t		j;
    STREAM_TYPE		scalar;
    double		t, times[4][NTIMES];
    double t_overall;
	double t_tmp;

	STREAM_TYPE	* a;
	STREAM_TYPE	* b;
	STREAM_TYPE	* c;

	size_t tmp_size = sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE;

	// a = malloc(STREAM_ARRAY_SIZE * sizeof(STREAM_TYPE));
	// b = malloc(STREAM_ARRAY_SIZE * sizeof(STREAM_TYPE));
	// c = malloc(STREAM_ARRAY_SIZE * sizeof(STREAM_TYPE));

	a = (STREAM_TYPE*)memalign(getpagesize(), tmp_size);
	b = (STREAM_TYPE*)memalign(getpagesize(), tmp_size);
	c = (STREAM_TYPE*)memalign(getpagesize(), tmp_size);
	madvise(a, tmp_size, MADV_NOHUGEPAGE);
	madvise(b, tmp_size, MADV_NOHUGEPAGE);
	madvise(c, tmp_size, MADV_NOHUGEPAGE);

    //int res = set_mempolicy(MPOL_PREFERRED, NULL, 0);
    //printf("set_mempolicy: %d (errno=%d)\n", res, errno);

    /* --- SETUP --- determine precision and check timing --- */

    printf(HLINE);
    printf("STREAM version $Revision: 5.10 $\n");
    printf(HLINE);
    BytesPerWord = sizeof(STREAM_TYPE);
    printf("This system uses %d bytes per array element.\n", BytesPerWord);

    printf(HLINE);
#ifdef N
    printf("*****  WARNING: ******\n");
    printf("      It appears that you set the preprocessor variable N when compiling this code.\n");
    printf("      This version of the code uses the preprocesor variable STREAM_ARRAY_SIZE to control the array size\n");
    printf("      Reverting to default value of STREAM_ARRAY_SIZE=%llu\n",(unsigned long long) STREAM_ARRAY_SIZE);
    printf("*****  WARNING: ******\n");
#endif

    printf("Array size = %llu (elements), Offset = %d (elements)\n" , (unsigned long long) STREAM_ARRAY_SIZE, OFFSET);
    printf("Memory per array = %.1f MiB (= %.1f GiB).\n", 
	BytesPerWord * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.0),
	BytesPerWord * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.0/1024.0));
    printf("Total memory required = %.1f MiB (= %.1f GiB).\n",
	(3.0 * BytesPerWord) * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.),
	(3.0 * BytesPerWord) * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024./1024.));
    printf("Each kernel will be executed %d times.\n", NTIMES);
    printf(" The *best* time for each kernel (excluding the first iteration)\n"); 
    printf(" will be used to compute the reported bandwidth.\n");

#ifdef _OPENMP
    printf(HLINE);
#pragma omp parallel 
    {
#pragma omp master
	{
	    k = omp_get_num_threads();
	    printf ("Number of Threads requested = %i\n",k);
        }
    }
#endif

#ifdef _OPENMP
	k = 0;
#pragma omp parallel
#pragma omp atomic 
		k++;
    printf ("Number of Threads counted = %i\n",k);
#endif

    /* Get initial value for system clock. */

	// parallel data initialization
	fprintf(stderr, "Initializing data (parallel) ...\n");
#pragma omp parallel for
    for (j=0; j<STREAM_ARRAY_SIZE; j++) {
	    a[j] = 1.0;
	    b[j] = 2.0;
	    c[j] = 0.0;
	}

    printf(HLINE);

    if  ( (quantum = checktick()) >= 1) 
	printf("Your clock granularity/precision appears to be "
	    "%d microseconds.\n", quantum);
    else {
	printf("Your clock granularity appears to be "
	    "less than one microsecond.\n");
	quantum = 1;
    }

	printf("PAGESIZE of the current system seems to be %d\n", getpagesize());
    
	// DEBUG to see if data is distributed correctly with lower array sizes
	sleep(3);

    /*	--- MAIN LOOP --- repeat test cases NTIMES times --- */

    scalar = 3.0;

	// !!!! init arrays for access times (Threads are assumed to be pinned, one thread on each socket) !!!!
	double access_times_copy[omp_get_max_threads()][omp_get_max_threads()];
	double access_times_scale[omp_get_max_threads()][omp_get_max_threads()];
	double access_times_add[omp_get_max_threads()][omp_get_max_threads()];
	double access_times_triad[omp_get_max_threads()][omp_get_max_threads()];
	
	int tmp_num_threads = omp_get_max_threads();
	long step = STREAM_ARRAY_SIZE / tmp_num_threads;
    	if(STREAM_ARRAY_SIZE % tmp_num_threads != 0)
      		step++;

#if !SIMULTAN
	int cur_executor;
	for(cur_executor = 0; cur_executor < tmp_num_threads; cur_executor++)
	{
#endif
#pragma omp parallel private(t_tmp)
		{
#if !SIMULTAN
			if(omp_get_thread_num() == cur_executor)
			{
#endif
				// determine current numa node
				long tmp_idx_start_cur = omp_get_thread_num() * step;
				long tmp_idx_end_cur = MIN((omp_get_thread_num()+1)*step-1,STREAM_ARRAY_SIZE);
				int exec_data_domain = -1;
				//void * exec_pointer = &c[(int)((tmp_idx_start_cur+tmp_idx_end_cur)/2)];
				void * exec_pointer = &c[tmp_idx_start_cur];
				int exec_err = move_pages(0 /*self memory */, 1, &exec_pointer, NULL, &exec_data_domain, 0);
				printf(HLINE);
				printf("Currently executing thread = %d on domain %d\n", omp_get_thread_num(), exec_data_domain);

				int iThread = 0;

				// COPY --------------------------------------------------------------
				printf(HLINE);
				for(iThread = 0; iThread < tmp_num_threads; iThread++){
					
					// TODO: need to clear caches

					long tmp_idx_start = iThread * step;
					long tmp_idx_end = MIN((iThread+1)*step-1,STREAM_ARRAY_SIZE);

					long i;
					t_tmp = mysecond();
					for(i = tmp_idx_start; i <= tmp_idx_end; i++){
						c[i] = a[i];
					}
					t_tmp = mysecond() - t_tmp;

					int current_data_domain = -1;
					//void * tmp_pointer = &c[(int)((tmp_idx_start+tmp_idx_end)/2)];
					void * tmp_pointer = &c[tmp_idx_start];
					//printf("Adress of array element %ld is %p\n", (tmp_idx_start+tmp_idx_end)/2, tmp_pointer);
					int tmp_err = move_pages(0 /*self memory */, 1, &tmp_pointer, NULL, &current_data_domain, 0);				
					printf("COPY\tAccess from domain\t%d\tto domain\t%d\ttook\t%f\tsec\n", exec_data_domain, current_data_domain, t_tmp);
					access_times_copy[exec_data_domain][current_data_domain] = t_tmp;
				}

				// SCALE --------------------------------------------------------------
				printf(HLINE);
				for(iThread = 0; iThread < tmp_num_threads; iThread++){
					long tmp_idx_start = iThread * step;
					long tmp_idx_end = MIN((iThread+1)*step-1,STREAM_ARRAY_SIZE);

					long i;
					t_tmp = mysecond();
					for(i = tmp_idx_start; i <= tmp_idx_end; i++){
						b[i] = scalar*c[i];
					}
					t_tmp = mysecond() - t_tmp;

					int current_data_domain = -1;
					//void * tmp_pointer = &b[(int)((tmp_idx_start+tmp_idx_end)/2)];
					void * tmp_pointer = &b[tmp_idx_start];
					int tmp_err = move_pages(0 /*self memory */, 1, &tmp_pointer, NULL, &current_data_domain, 0);				
					printf("SCALE\tAccess from domain\t%d\tto domain\t%d\ttook\t%f\tsec\n", exec_data_domain, current_data_domain, t_tmp);
					access_times_scale[exec_data_domain][current_data_domain] = t_tmp;
				}

				// ADD --------------------------------------------------------------
				printf(HLINE);
				for(iThread = 0; iThread < tmp_num_threads; iThread++){
					long tmp_idx_start = iThread * step;
					long tmp_idx_end = MIN((iThread+1)*step-1,STREAM_ARRAY_SIZE);

					long i;
					t_tmp = mysecond();
					for(i = tmp_idx_start; i <= tmp_idx_end; i++){
						c[i] = a[i]+b[i];
					}
					t_tmp = mysecond() - t_tmp;

					int current_data_domain = -1;
					//void * tmp_pointer = &c[(int)((tmp_idx_start+tmp_idx_end)/2)];
					void * tmp_pointer = &c[tmp_idx_start];
					int tmp_err = move_pages(0 /*self memory */, 1, &tmp_pointer, NULL, &current_data_domain, 0);				
					printf("ADD\tAccess from domain\t%d\tto domain\t%d\ttook\t%f\tsec\n", exec_data_domain, current_data_domain, t_tmp);
					access_times_add[exec_data_domain][current_data_domain] = t_tmp;
				}

				// TRIAD --------------------------------------------------------------
				printf(HLINE);
				for(iThread = 0; iThread < tmp_num_threads; iThread++){
					long tmp_idx_start = iThread * step;
					long tmp_idx_end = MIN((iThread+1)*step-1,STREAM_ARRAY_SIZE);

					long i;
					t_tmp = mysecond();
					for(i = tmp_idx_start; i <= tmp_idx_end; i++){
						a[i] = b[i]+scalar*c[i];
					}
					t_tmp = mysecond() - t_tmp;

					int current_data_domain = -1;
					//void * tmp_pointer = &a[(int)((tmp_idx_start+tmp_idx_end)/2)];
					void * tmp_pointer = &a[tmp_idx_start];
					int tmp_err = move_pages(0 /*self memory */, 1, &tmp_pointer, NULL, &current_data_domain, 0);				
					printf("TRIAD\tAccess from domain\t%d\tto domain\t%d\ttook\t%f\tsec\n", exec_data_domain, current_data_domain, t_tmp);
					access_times_triad[exec_data_domain][current_data_domain] = t_tmp;
				}
#if !SIMULTAN
			}
#endif
		}
#if !SIMULTAN
	}
#endif

	fflush(stdout);

	int row, col;

	printf(HLINE);
	printf("COPY\n");
	for(row = 0; row < omp_get_max_threads(); row++)
	{
		for(col = 0; col < omp_get_max_threads(); col++)
		{
			printf("%f\t", access_times_copy[row][col]);
		}
		printf("\n");
	}

	printf(HLINE);
	printf("SCALE\n");
	for(row = 0; row < omp_get_max_threads(); row++)
	{
		for(col = 0; col < omp_get_max_threads(); col++)
		{
			printf("%f\t", access_times_scale[row][col]);
		}
		printf("\n");
	}

	printf(HLINE);
	printf("ADD\n");
	for(row = 0; row < omp_get_max_threads(); row++)
	{
		for(col = 0; col < omp_get_max_threads(); col++)
		{
			printf("%f\t", access_times_add[row][col]);
		}
		printf("\n");
	}

	printf(HLINE);
	printf("TRIAD\n");
	for(row = 0; row < omp_get_max_threads(); row++)
	{
		for(col = 0; col < omp_get_max_threads(); col++)
		{
			printf("%f\t", access_times_triad[row][col]);
		}
		printf("\n");
	}

    return 0;
}

# define	M	20

int
checktick()
    {
    int		i, minDelta, Delta;
    double	t1, t2, timesfound[M];

/*  Collect a sequence of M unique time values from the system. */

    for (i = 0; i < M; i++) {
	t1 = mysecond();
	while( ((t2=mysecond()) - t1) < 1.0E-6 )
	    ;
	timesfound[i] = t1 = t2;
	}

/*
 * Determine the minimum difference between these M values.
 * This result will be our estimate (in microseconds) for the
 * clock granularity.
 */

    minDelta = 1000000;
    for (i = 1; i < M; i++) {
	Delta = (int)( 1.0E6 * (timesfound[i]-timesfound[i-1]));
	minDelta = MIN(minDelta, MAX(Delta,0));
	}

   return(minDelta);
    }



/* A gettimeofday routine to give access to the wall
   clock timer on most UNIX-like systems.  */

#include <sys/time.h>

double mysecond()
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

#ifndef abs
#define abs(a) ((a) >= 0 ? (a) : -(a))
#endif
