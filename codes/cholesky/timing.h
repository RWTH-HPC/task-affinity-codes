#ifndef _BENCH_CHOLESKY_TIMING_
#define _BENCH_CHOLESKY_TIMING_

enum {
    TIME_POTRF = 0,
    TIME_TRSM  = 1,
    TIME_GEMM  = 2,
    TIME_SYRK  = 3,
    TIME_COMM  = 4,
    TIME_CREATE = 5,
    TIME_TOTAL = 6,
    TIME_CNT
};


typedef struct perthread_timing {
    double ts[TIME_CNT];
} perthread_timing_t;

#ifdef USE_TIMING
perthread_timing_t *__timing;

#define INIT_TIMING(nthreads) \
  __timing = calloc(nthreads, sizeof(perthread_timing_t))

#define RESET_TIMINGS(nthreads) do{ \
  for (int i = 0; i < nthreads; ++i)  \
    for (int j = 0; j < TIME_CNT; ++j) \
      __timing[i].ts[j] = 0.0;    \
  } while(0)

#define ACCUMULATE_TIMINGS(nthreads, dst) do{ \
  memset(&(dst), 0, sizeof(dst)); \
  for (int i = 0; i < nthreads; ++i)  \
    for (int j = 0; j < TIME_CNT; ++j) \
      (dst).ts[j] += __timing[i].ts[j];    \
  } while(0)

#define PRINT_TIMINGS(nthreads) do { \
    perthread_timing_t acc_timings; \
    ACCUMULATE_TIMINGS(nthreads, acc_timings); \
    printf("potrf:%f:trsm:%f:gemm:%f:syrk:%f:comm:%f:create:%f:total:%f:wall:%f\n", acc_timings.ts[TIME_POTRF], acc_timings.ts[TIME_TRSM],acc_timings.ts[TIME_GEMM],acc_timings.ts[TIME_SYRK],acc_timings.ts[TIME_COMM],acc_timings.ts[TIME_CREATE],acc_timings.ts[TIME_TOTAL], acc_timings.ts[TIME_TOTAL]*nthreads); \
  } while(0)

#define FREE_TIMING() free(__timing)

#define THREAD_NUM omp_get_thread_num()

#define START_TIMING(timer) double __ts_##timer = timestamp(); int __timer = timer;
#define END_TIMING(timer) __timing[THREAD_NUM].ts[timer] += timestamp() - __ts_##timer

static double timestamp(){
    return omp_get_wtime();
}

#else 

#define INIT_TIMING(nthreads)
#define FREE_TIMING()
#define START_TIMING(timer)
#define END_TIMING(timer)
#define ACCUMULATE_TIMINGS(nthreads, dst)
#define RESET_TIMINGS(nthreads)
#define PRINT_TIMINGS(nthreads)
#endif

#endif // _BENCH_CHOLESKY_TIMING_