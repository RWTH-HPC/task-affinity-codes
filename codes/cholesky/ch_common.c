
#define MAIN

#include "ch_common.h"
#include "cholesky.h"
#include "timing.h"
#include "./../task_affinity_support/task_affinity_support.h"


#if (defined(DEBUG) || defined(USE_TIMING))
_Atomic int cnt_pdotrf = 0;
_Atomic int cnt_trsm = 0;
_Atomic int cnt_gemm = 0;
_Atomic int cnt_syrk = 0;
#endif

static void get_block_rank(int *block_rank, int nt);

void omp_potrf(double * SPEC_RESTRICT const A, int ts, int ld)
{
#if (defined(DEBUG) || defined(USE_TIMING))
    cnt_pdotrf++;
    START_TIMING(TIME_POTRF);
#endif
    static int INFO;
    static const char L = 'L';
    dpotrf_(&L, &ts, A, &ld, &INFO);
#if (defined(DEBUG) || defined(USE_TIMING))
    END_TIMING(TIME_POTRF);
#endif
}

void omp_trsm(double * SPEC_RESTRICT A, double * SPEC_RESTRICT B, int ts, int ld)
{
#if (defined(DEBUG) || defined(USE_TIMING))
    cnt_trsm++;
    START_TIMING(TIME_TRSM);
#endif
    static char LO = 'L', TR = 'T', NU = 'N', RI = 'R';
    static double DONE = 1.0;
    dtrsm_(&RI, &LO, &TR, &NU, &ts, &ts, &DONE, A, &ld, B, &ld );
#if (defined(DEBUG) || defined(USE_TIMING))
    END_TIMING(TIME_TRSM);
#endif
}

void omp_gemm(double * SPEC_RESTRICT A, double * SPEC_RESTRICT B, double * SPEC_RESTRICT C, int ts, int ld)
{
#if (defined(DEBUG) || defined(USE_TIMING))
    cnt_gemm++;
    START_TIMING(TIME_GEMM);
#endif
    static const char TR = 'T', NT = 'N';
    static double DONE = 1.0, DMONE = -1.0;
    dgemm_(&NT, &TR, &ts, &ts, &ts, &DMONE, A, &ld, B, &ld, &DONE, C, &ld);
#if (defined(DEBUG) || defined(USE_TIMING))
    END_TIMING(TIME_GEMM);
#endif
}

void omp_syrk(double * SPEC_RESTRICT A, double * SPEC_RESTRICT B, int ts, int ld)
{
#if (defined(DEBUG) || defined(USE_TIMING))
    cnt_syrk++;
    START_TIMING(TIME_SYRK);
#endif
    static char LO = 'L', NT = 'N';
    static double DONE = 1.0, DMONE = -1.0;
    dsyrk_(&LO, &NT, &ts, &ts, &DMONE, A, &ld, &DONE, B, &ld );
#if (defined(DEBUG) || defined(USE_TIMING))
    END_TIMING(TIME_SYRK);
#endif
}

void cholesky_regular(const int ts, const int nt, double* A[nt][nt])
{
#pragma omp parallel
{
#pragma omp single
{
    for (int k = 0; k < nt; k++) {
#pragma omp task depend(out: A[k][k])
{
        omp_potrf(A[k][k], ts, ts);
#ifdef DEBUG
        printf("potrf:out:A[%d][%d]\n", k, k);
#endif
}
        for (int i = k + 1; i < nt; i++) {
#pragma omp task depend(in: A[k][k]) depend(out: A[k][i])
{
            omp_trsm(A[k][k], A[k][i], ts, ts);
#ifdef DEBUG
        printf("trsm :in:A[%d][%d]:out:A[%d][%d]\n", k, k, k, i);
#endif
}
        }
        for (int i = k + 1; i < nt; i++) {
            for (int j = k + 1; j < i; j++) {
#pragma omp task depend(in: A[k][i], A[k][j]) depend(out: A[j][i])
{
                omp_gemm(A[k][i], A[k][j], A[j][i], ts, ts);
#ifdef DEBUG
                printf("gemm :in:A[%d][%d]:A[%d][%d]:out:A[%d][%d]\n", k, i, k, j, j, i);
#endif
}
            }
#pragma omp task depend(in: A[k][i]) depend(out: A[i][i])
{
            omp_syrk(A[k][i], A[i][i], ts, ts);
#ifdef DEBUG
            printf("syrk :in:A[%d][%d]:out:A[%d][%d]\n", k, i, i, i);
#endif
}
        }
    }
#pragma omp taskwait
}
}
}

void cholesky_affinity(const int ts, const int nt, double* A[nt][nt])
{
    int size = ts * ts * sizeof(double);

    #pragma omp parallel
    {
        #pragma omp master
        {
            for (int k = 0; k < nt; k++) {
                #ifdef TASK_AFFINITY
                    kmpc_set_task_affinity(&A[k][k], size);
                    //printf("set task affinity %d \n", size);
                #endif
                #pragma omp task depend(out: A[k][k])
                {
                    omp_potrf(A[k][k], ts, ts);
                #ifdef DEBUG
                    printf("potrf:out:A[%d][%d]\n", k, k);
                #endif
                }

                for (int i = k + 1; i < nt; i++) {
                    #ifdef TASK_AFFINITY
                        kmpc_set_task_affinity(&A[k][k], size);
                        kmpc_set_task_affinity(&A[k][i], size);
                    #endif
                    #pragma omp task depend(in: A[k][k]) depend(out: A[k][i])
                    {
                        omp_trsm(A[k][k], A[k][i], ts, ts);
                    #ifdef DEBUG
                        printf("trsm :in:A[%d][%d]:out:A[%d][%d]\n", k, k, k, i);
                    #endif
                    }
                }

                for (int i = k + 1; i < nt; i++) {
                    for (int j = k + 1; j < i; j++) {
                        #ifdef TASK_AFFINITY
                            kmpc_set_task_affinity(&A[k][i], size);
                            kmpc_set_task_affinity(&A[k][j], size);
                            kmpc_set_task_affinity(&A[j][i], size);
                        #endif
                        #pragma omp task depend(in: A[k][i], A[k][j]) depend(out: A[j][i])
                        {
                            omp_gemm(A[k][i], A[k][j], A[j][i], ts, ts);
                        #ifdef DEBUG
                            printf("gemm :in:A[%d][%d]:A[%d][%d]:out:A[%d][%d]\n", k, i, k, j, j, i);
                        #endif
                        }
                    }
                    #ifdef TASK_AFFINITY
                        kmpc_set_task_affinity(&A[k][i], size);
                        kmpc_set_task_affinity(&A[i][i], size);
                    #endif
                    #pragma omp task depend(in: A[k][i]) depend(out: A[i][i])
                    {
                        omp_syrk(A[k][i], A[i][i], ts, ts);
                    #ifdef DEBUG
                        printf("syrk :in:A[%d][%d]:out:A[%d][%d]\n", k, i, i, i);
                    #endif
                    }
                }
            }
        #pragma omp taskwait
        }
    }
}
/*
void cholesky_affinity(const int ts, const int nt, double* A[nt][nt])
{
    int len = 0, k = 0;
    #pragma omp parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp for private(k) schedule(static)
            for (k = 0; k < nt; k++) {
                #ifdef TASK_AFFINITY
                    kmpc_set_task_affinity(&A[k][1], 1);
                #endif
                    #pragma omp task depend(out: A[k][k])
                    {
                        omp_potrf(A[k][k], ts, ts);
                    #ifdef DEBUG
                        printf("potrf:out:A[%d][%d]\n", k, k);
                    #endif
                    }
                #ifdef TASK_AFFINITY
                    len = nt-k;
                    kmpc_set_task_affinity(&A[k], len);
                #endif
                for (int i = k + 1; i < nt; i++) {
                    #pragma omp task depend(in: A[k][k]) depend(out: A[k][i])
                    {
                        omp_trsm(A[k][k], A[k][i], ts, ts);
                    #ifdef DEBUG
                        printf("trsm :in:A[%d][%d]:out:A[%d][%d]\n", k, k, k, i);
                    #endif
                    }
                }
                #ifdef TASK_AFFINITY
                    len = nt - k;
                    kmpc_set_task_affinity(&A[k][k], len);
                #endif
                for (int i = k + 1; i < nt; i++) {
                    for (int j = k + 1; j < i; j++) {
                        #pragma omp task depend(in: A[k][i], A[k][j]) depend(out: A[j][i])
                        {
                            omp_gemm(A[k][i], A[k][j], A[j][i], ts, ts);
                        #ifdef DEBUG
                            printf("gemm :in:A[%d][%d]:A[%d][%d]:out:A[%d][%d]\n", k, i, k, j, j, i);
                        #endif
                        }
                    }
                    #pragma omp task depend(in: A[k][i]) depend(out: A[i][i])
                    {
                        omp_syrk(A[k][i], A[i][i], ts, ts);
                    #ifdef DEBUG
                        printf("syrk :in:A[%d][%d]:out:A[%d][%d]\n", k, i, i, i);
                    #endif
                    }
                }
            }
        #pragma omp taskwait
        }
    }
}
*/
int main(int argc, char *argv[])
{
    /* cholesky init */
    const char *result[3] = {"n/a","successful","UNSUCCESSFUL"};
    const double eps = BLAS_dfpinfo(blas_eps);

    if (argc < 4) {
        printf("cholesky matrix_size block_size check\n");
        exit(-1);
    }

#ifdef TASK_AFFINITY
    init_task_affinity();
#endif
    const int  n = atoi(argv[1]); // matrix size
    const int ts = atoi(argv[2]); // tile size
    int check    = atoi(argv[3]); // check result?

    const int nt = n / ts;
    printf("nt = %d, ts = %d\n", nt, ts);

    double *A_regular[nt][nt], *B, *C[nt], *A_affinity[nt][nt];

    for (int i = 0; i < nt; i++) {
        #pragma omp parallel for schedule(static,1)
        for (int j = 0; j < nt; j++) {
            if(check) {
                A_regular[i][j] = (double *) malloc(ts * ts * sizeof(double));
                assert(A_regular[i][j]);
                initialize_tile(ts, A_regular[i][j]);
            }
            
            A_affinity[i][j] = (double *) malloc(ts * ts * sizeof(double));
            assert(A_affinity[i][j]);
            if(check) {
                for (int k = 0; k < ts * ts; k++) {
                    A_affinity[i][j][k] = A_regular[i][j][k];
                }
            } else {
                initialize_tile(ts, A_affinity[i][j]);
            }            
        }

        // add to diagonal
        if (check) {
            A_regular[i][i][i*ts+i] = (double)nt;
        }
        A_affinity[i][i][i*ts+i] = (double)nt;
    }

    B = (double *) malloc(ts * ts * sizeof(double));
    #pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < nt; i++) {
        C[i] = (double *) malloc(ts * ts * sizeof(double));
        for(int j = 0; j < ts*ts; j++) {
            C[i][j] = 0.0;
        }
    }

#pragma omp parallel
#pragma omp single
    num_threads = omp_get_num_threads();

    INIT_TIMING(num_threads);
    RESET_TIMINGS(num_threads);
    const float t1 = get_time();
    if (check) cholesky_regular(ts, nt, (double* (*)[nt]) A_regular);
    const float t2 = get_time() - t1;

    const float t3 = get_time();
    // TODO: implement cholesky affinity version here
#ifdef TASK_AFFINITY
    printf("\nrunning cholesky affinity...\n");
    cholesky_affinity(ts, nt, (double* (*)[nt]) A_affinity);
#else
    printf("\nrunning cholesky regular...\n");
    cholesky_regular(ts, nt, (double* (*)[nt]) A_affinity);
#endif

    const float t4 = get_time() - t3;

    /* Verification */
    if (check) {
        for (int i = 0; i < nt; i++) {
            for (int j = 0; j < nt; j++) {
                for (int k = 0; k < ts*ts; k++) {
                    // if (Ans[i][j][k] != A[i][j][k]) check = 2;
                    if (A_affinity[i][j][k] != A_regular[i][j][k]) {
                        check = 2;
                        printf("Expected: %f    Value: %f    Diff: %f\n", A_regular[i][j][k], A_affinity[i][j][k], abs(A_regular[i][j][k]-A_affinity[i][j][k]));
                        break;
                    }
                }
                if(check == 2) break;
            }
            if(check == 2) break;
        }
    }

    float time_regular      = t2;
    float gflops_regular    = (((1.0 / 3.0) * n * n * n) / ((time_regular) * 1.0e+9));
    float time_affinity     = t4;
    float gflops_affinity   = (((1.0 / 3.0) * n * n * n) / ((time_affinity) * 1.0e+9));

    printf("test:%s-%d-%d-%d:threads:%2d:result:%s:gflops_regular:%f:time:%f:gflops_affinity:%f:time_ser:%f\n", argv[0], n, ts, num_threads, num_threads, result[check], gflops_regular, t2, gflops_affinity, t4);
	printf("Elapsed time for program\t%lf\tsec\n",t4);

#if (defined(DEBUG) || defined(USE_TIMING))
    printf("count#pdotrf:%d:count#trsm:%d:count#gemm:%d:count#syrk:%d\n", cnt_pdotrf, cnt_trsm, cnt_gemm, cnt_syrk);
#endif
#ifdef USE_TIMING
	PRINT_TIMINGS(num_threads);
	FREE_TIMING();
#endif 
    for (int i = 0; i < nt; i++) {
        for (int j = 0; j < nt; j++) {
            free(A_affinity[i][j]);
            if (check)
              free(A_regular[i][j]);
        }
        free(C[i]);
    }
    free(B);
    return 0;
}
