/**********************************************************************************************/
/*  This program is part of the Barcelona OpenMP Tasks Suite                                  */
/*  Copyright (C) 2009 Barcelona Supercomputing Center - Centro Nacional de Supercomputacion  */
/*  Copyright (C) 2009 Universitat Politecnica de Catalunya                                   */
/*                                                                                            */
/*  This program is free software; you can redistribute it and/or modify                      */
/*  it under the terms of the GNU General Public License as published by                      */
/*  the Free Software Foundation; either version 2 of the License, or                         */
/*  (at your option) any later version.                                                       */
/*                                                                                            */
/*  This program is distributed in the hope that it will be useful,                           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                             */
/*  GNU General Public License for more details.                                              */
/*                                                                                            */
/*  You should have received a copy of the GNU General Public License                         */
/*  along with this program; if not, write to the Free Software                               */
/*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA            */
/**********************************************************************************************/

/*
 *  Original code from the Cilk project
 *
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 */

/*
 * this program uses an algorithm that we call `cilksort'.
 * The algorithm is essentially mergesort:
 *
 *   cilksort(in[1..n]) =
 *       spawn cilksort(in[1..n/2], tmp[1..n/2])
 *       spawn cilksort(in[n/2..n], tmp[n/2..n])
 *       sync
 *       spawn cilkmerge(tmp[1..n/2], tmp[n/2..n], in[1..n])
 *
 *
 * The procedure cilkmerge does the following:
 *
 *       cilkmerge(A[1..n], B[1..m], C[1..(n+m)]) =
 *          find the median of A \union B using binary
 *          search.  The binary search gives a pair
 *          (ma, mb) such that ma + mb = (n + m)/2
 *          and all elements in A[1..ma] are smaller than
 *          B[mb..m], and all the B[1..mb] are smaller
 *          than all elements in A[ma..n].
 *
 *          spawn cilkmerge(A[1..ma], B[1..mb], C[1..(n+m)/2])
 *          spawn cilkmerge(A[ma..m], B[mb..n], C[(n+m)/2 .. (n+m)])
 *          sync
 *
 * The algorithm appears for the first time (AFAIK) in S. G. Akl and
 * N. Santoro, "Optimal Parallel Merging and Sorting Without Memory
 * Conflicts", IEEE Trans. Comp., Vol. C-36 No. 11, Nov. 1987 .  The
 * paper does not express the algorithm using recursion, but the
 * idea of finding the median is there.
 *
 * For cilksort of n elements, T_1 = O(n log n) and
 * T_\infty = O(log^3 n).  There is a way to shave a
 * log factor in the critical path (left as homework).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bots.h"
#include "app-desc.h"

#include <no_huge_page_alloc.h>

#ifndef USE_UNTIED_VER
	#define USE_UNTIED_VER 0
#endif
#ifndef USE_TASK_AFF_ONLY_AT_CUTOFF
	#define USE_TASK_AFF_ONLY_AT_CUTOFF 0
#endif

ELM *array, *tmp;

static unsigned long rand_nxt = 0;

static int FileExists(const size_t size)
{
  char buf[255];
  sprintf(buf, "input__%lu", size);
  if(access(buf, F_OK) != -1)
    return 1;
  else
    return 0;
}

static void ReadFile(ELM *array, const size_t size)
{
  char buf[255];
  sprintf(buf, "input__%lu", size);

  int fd = open(buf, O_RDONLY);
  if (fd == -1){
    printf("Error open file\n");
    exit(1);
  }

  struct stat st;
  fstat(fd, &st);

  void *mem = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, fd, 0);
  if(mem == MAP_FAILED){
    printf("Error reading file\n");
    exit(1);
  }

  long *tmp_arr = (long*) mem;
  size_t i;
  for(i = 0; i < size; i++)
  {
    array[i] = tmp_arr[i];
  }

  munmap(mem, st.st_size);
  close(fd);
}

static void WriteFile(ELM *array, const size_t size, int binary)
{
  size_t pagesize = sysconf(_SC_PAGESIZE);
#define PAGES(size) ((size + (pagesize - 1)) & ~(pagesize - 1))
  char buf[255];
  if(binary)
    sprintf(buf, "input__%lu", size);
  else
    sprintf(buf, "input__%lu_pure", size);

  if(binary){
  long* value = (long*)malloc(PAGES(sizeof(long) * size));
  memset(value, 0, PAGES(sizeof(long) * size));
  memcpy(value, array, sizeof(long) * size);

  FILE *f;
  f = fopen(buf, "wb");
  fwrite(value, 1, PAGES(sizeof(long) * size), f);
  fclose(f);
  free(value);
  } else {
    FILE *f;
    f = fopen(buf, "wb");
    size_t i;
    for(i = 0; i < size; i++)
    {
      fprintf(f, "%ld ", array[i]);
    }
    fclose(f);
  }
}

static inline unsigned long my_rand(void)
{
     rand_nxt = rand_nxt * 1103515245 + 12345;
     return rand_nxt;
}

static inline void my_srand(unsigned long seed)
{
     rand_nxt = seed;
}

static inline ELM med3(ELM a, ELM b, ELM c)
{
     if (a < b) {
	  if (b < c) {
	       return b;
	  } else {
	       if (a < c)
		    return c;
	       else
		    return a;
	  }
     } else {
	  if (b > c) {
	       return b;
	  } else {
	       if (a > c)
		    return c;
	       else
		    return a;
	  }
     }
}

/*
 * simple approach for now; a better median-finding
 * may be preferable
 */
static inline ELM choose_pivot(ELM *low, ELM *high)
{
     return med3(*low, *high, low[(high - low) / 2]);
}

static ELM *seqpart(ELM *low, ELM *high)
{
     ELM pivot;
     ELM h, l;
     ELM *curr_low = low;
     ELM *curr_high = high;

     pivot = choose_pivot(low, high);

     while (1) {
	  while ((h = *curr_high) > pivot)
	       curr_high--;

	  while ((l = *curr_low) < pivot)
	       curr_low++;

	  if (curr_low >= curr_high)
	       break;

	  *curr_high-- = l;
	  *curr_low++ = h;
     }

     /*
      * I don't know if this is really necessary.
      * The problem is that the pivot is not always the
      * first element, and the partition may be trivial.
      * However, if the partition is trivial, then
      * *high is the largest element, whence the following
      * code.
      */
     if (curr_high < high)
	  return curr_high;
     else
	  return curr_high - 1;
}

#define swap(a, b) \
{ \
  ELM tmp;\
  tmp = a;\
  a = b;\
  b = tmp;\
}

static void insertion_sort(ELM *low, ELM *high)
{
     ELM *p, *q;
     ELM a, b;

     for (q = low + 1; q <= high; ++q) {
	  a = q[0];
	  for (p = q - 1; p >= low && (b = p[0]) > a; p--)
	       p[1] = b;
	  p[1] = a;
     }
}

/*
 * tail-recursive quicksort, almost unrecognizable :-)
 */
void seqquick(ELM *low, ELM *high)
{
     ELM *p;

     while (high - low >= bots_app_cutoff_value_2) {
	  p = seqpart(low, high);
	  seqquick(low, p);
	  low = p + 1;
     }

     insertion_sort(low, high);
}

void seqmerge(ELM *low1, ELM *high1, ELM *low2, ELM *high2,
	      ELM *lowdest)
{
     ELM a1, a2;

     /*
      * The following 'if' statement is not necessary
      * for the correctness of the algorithm, and is
      * in fact subsumed by the rest of the function.
      * However, it is a few percent faster.  Here is why.
      *
      * The merging loop below has something like
      *   if (a1 < a2) {
      *        *dest++ = a1;
      *        ++low1;
      *        if (end of array) break;
      *        a1 = *low1;
      *   }
      *
      * Now, a1 is needed immediately in the next iteration
      * and there is no way to mask the latency of the load.
      * A better approach is to load a1 *before* the end-of-array
      * check; the problem is that we may be speculatively
      * loading an element out of range.  While this is
      * probably not a problem in practice, yet I don't feel
      * comfortable with an incorrect algorithm.  Therefore,
      * I use the 'fast' loop on the array (except for the last
      * element) and the 'slow' loop for the rest, saving both
      * performance and correctness.
      */

     if (low1 < high1 && low2 < high2) {
	  a1 = *low1;
	  a2 = *low2;
	  for (;;) {
	       if (a1 < a2) {
		    *lowdest++ = a1;
		    a1 = *++low1;
		    if (low1 >= high1)
			 break;
	       } else {
		    *lowdest++ = a2;
		    a2 = *++low2;
		    if (low2 >= high2)
			 break;
	       }
	  }
     }
     if (low1 <= high1 && low2 <= high2) {
	  a1 = *low1;
	  a2 = *low2;
	  for (;;) {
	       if (a1 < a2) {
		    *lowdest++ = a1;
		    ++low1;
		    if (low1 > high1)
			 break;
		    a1 = *low1;
	       } else {
		    *lowdest++ = a2;
		    ++low2;
		    if (low2 > high2)
			 break;
		    a2 = *low2;
	       }
	  }
     }
     if (low1 > high1) {
	  memcpy(lowdest, low2, sizeof(ELM) * (high2 - low2 + 1));
     } else {
	  memcpy(lowdest, low1, sizeof(ELM) * (high1 - low1 + 1));
     }
}

#define swap_indices(a, b) \
{ \
  ELM *tmp;\
  tmp = a;\
  a = b;\
  b = tmp;\
}

ELM *binsplit(ELM val, ELM *low, ELM *high)
{
     /*
      * returns index which contains greatest element <= val.  If val is
      * less than all elements, returns low-1
      */
     ELM *mid;

     while (low != high) {
	  mid = low + ((high - low + 1) >> 1);
	  if (val <= *mid)
	       high = mid - 1;
	  else
	       low = mid;
     }

     if (*low > val)
	  return low - 1;
     else
	  return low;
}


void cilkmerge_par(ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest)
{
     /*
      * Cilkmerge: Merges range [low1, high1] with range [low2, high2]
      * into the range [lowdest, ...]
      */

     ELM *split1, *split2;	/*
				 * where each of the ranges are broken for
				 * recursive merge
				 */
     long int lowsize;		/*
				 * total size of lower halves of two
				 * ranges - 2
				 */

     /*
      * We want to take the middle element (indexed by split1) from the
      * larger of the two arrays.  The following code assumes that split1
      * is taken from range [low1, high1].  So if [low1, high1] is
      * actually the smaller range, we should swap it with [low2, high2]
      */

     if (high2 - low2 > high1 - low1) {
	  swap_indices(low1, low2);
	  swap_indices(high1, high2);
     }
     if (high2 < low2) {
	  /* smaller range is empty */
	  memcpy(lowdest, low1, sizeof(ELM) * (high1 - low1));
	  return;
     }
     if (high2 - low2 < bots_app_cutoff_value ) {
	  seqmerge(low1, high1, low2, high2, lowdest);
	  return;
     }
     /*
      * Basic approach: Find the middle element of one range (indexed by
      * split1). Find where this element would fit in the other range
      * (indexed by split 2). Then merge the two lower halves and the two
      * upper halves.
      */

     split1 = ((high1 - low1 + 1) / 2) + low1;
     split2 = binsplit(*split1, low2, high2);
     lowsize = split1 - low1 + split2 - low2;

     /*
      * directly put the splitting element into
      * the appropriate location
      */
     *(lowdest + lowsize + 1) = *split1;
#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (split1 - 1 - low1 < bots_app_cutoff_value )
#endif
      //kmpc_set_task_affinity(lowdest);
	  int size = sizeof(ELM);
	  int lenL1 = (split1 - low1 - 1)*size;
	  int lenL2 = (split2 - low2)*size;
	  int lenL12 = lenL1 + lenL2;
	  kmpc_set_task_affinity(low1, lenL1);
	  kmpc_set_task_affinity(low2, lenL2);
	  kmpc_set_task_affinity(lowdest, lenL12);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilkmerge_par(low1, split1 - 1, low2, split2, lowdest);

#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (high1 - (split1 + 1) < bots_app_cutoff_value )
#endif
      //kmpc_set_task_affinity(lowdest + lowsize + 2);
	  int lenH1 = (high1 - split1 - 1)*size;
	  int lenH2 = (high2 - split2 - 1)*size;
	  int lenH12 = lenH1 + lenH2;
	  kmpc_set_task_affinity(split1+1,lenH1);
	  kmpc_set_task_affinity(split2+1,lenH2);
	  kmpc_set_task_affinity(lowdest+lowsize+2,lenH12);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilkmerge_par(split1 + 1, high1, split2 + 1, high2,
		     lowdest + lowsize + 2);

#pragma omp taskwait

     return;
}

void cilksort_par(ELM *low, ELM *tmp, long size)
{
     /*
      * divide the input in four parts of the same size (A, B, C, D)
      * Then:
      *   1) recursively sort A, B, C, and D (in parallel)
      *   2) merge A and B into tmp1, and C and D into tmp2 (in parallel)
      *   3) merge tmp1 and tmp2 into the original array
      */
     long quarter = size / 4;
     ELM *A, *B, *C, *D, *tmpA, *tmpB, *tmpC, *tmpD;

     if (size < bots_app_cutoff_value_1 ) {
	  /* quicksort when less than 1024 elements */
	  seqquick(low, low + size - 1);
	  return;
     }
     A = low;
     tmpA = tmp;
     B = A + quarter;
     tmpB = tmpA + quarter;
     C = B + quarter;
     tmpC = tmpB + quarter;
     D = C + quarter;
     tmpD = tmpC + quarter;

#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (quarter < bots_app_cutoff_value_1 )
#endif
      //kmpc_set_task_affinity(&tmpA[quarter/2]);
      //kmpc_set_task_affinity(tmpA);
	  int len = quarter;
	  kmpc_set_task_affinity(tmpA, len);
	  kmpc_set_task_affinity(A, len);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilksort_par(A, tmpA, quarter);

#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (quarter < bots_app_cutoff_value_1 )
#endif
      //kmpc_set_task_affinity(&tmpB[quarter/2]);
      //kmpc_set_task_affinity(tmpB);
	  //int len = quarter;
	  kmpc_set_task_affinity(tmpB, len);
	  kmpc_set_task_affinity(B, len);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilksort_par(B, tmpB, quarter);

#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (quarter < bots_app_cutoff_value_1 )
#endif
      //kmpc_set_task_affinity(&tmpC[quarter/2]);
      //kmpc_set_task_affinity(tmpC);
	  //int len = quarter;
	  kmpc_set_task_affinity(tmpC, len);
	  kmpc_set_task_affinity(C, len);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilksort_par(C, tmpC, quarter);

#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (quarter < bots_app_cutoff_value_1 )
#endif
      //kmpc_set_task_affinity(&tmpD[quarter/2]);
      //kmpc_set_task_affinity(tmpD);
	  //int len = quarter;
	  kmpc_set_task_affinity(tmpD, len);
	  kmpc_set_task_affinity(D, len);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilksort_par(D, tmpD, size - 3 * quarter);

#pragma omp taskwait

#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (quarter < bots_app_cutoff_value )
#endif
      //kmpc_set_task_affinity(&A[quarter/2]);
      //kmpc_set_task_affinity(tmpA);
	  //int len = quarter;
	  kmpc_set_task_affinity(tmpA, len);
	  kmpc_set_task_affinity(A, len);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilkmerge_par(A, A + quarter - 1, B, B + quarter - 1, tmpA);

#ifdef TASK_AFFINITY
#if USE_TASK_AFF_ONLY_AT_CUTOFF
	if (quarter < bots_app_cutoff_value )
#endif
      //kmpc_set_task_affinity(&C[quarter/2]);
      //kmpc_set_task_affinity(tmpC);
	  //int len = quarter;
	  kmpc_set_task_affinity(tmpC, len);
	  kmpc_set_task_affinity(C, len);
#endif
#if USE_UNTIED_VER
#pragma omp task untied
#else
#pragma omp task
#endif
     cilkmerge_par(C, C + quarter - 1, D, low + size - 1, tmpC);

#pragma omp taskwait

     cilkmerge_par(tmpA, tmpC - 1, tmpC, tmpA + size - 1, A);
}

void scramble_array( ELM *array )
{
     unsigned long i;
     unsigned long j;

     for (i = 0; i < bots_arg_size; ++i) {
	  j = my_rand();
	  j = j % bots_arg_size;
	  swap(array[i], array[j]);
     }
}

void fill_array( ELM *array )
{
     unsigned long i;

     my_srand(1);
     /* first, fill with integers 1..size */
#pragma omp parallel for schedule(static)
     for (i = 0; i < bots_arg_size; ++i) {
	      array[i] = i;
     }
}

void sort_init ( void )
{
     /* Checking arguments */
     if (bots_arg_size < 4) {
        bots_message("%s can not be less than 4, using 4 as a parameter.\n", BOTS_APP_DESC_ARG_SIZE );
        bots_arg_size = 4;
     }

     if (bots_app_cutoff_value < 2) {
        bots_message("%s can not be less than 2, using 2 as a parameter.\n", BOTS_APP_DESC_ARG_CUTOFF);
        bots_app_cutoff_value = 2;
     }
     else if (bots_app_cutoff_value > bots_arg_size ) {
        bots_message("%s can not be greather than vector size, using %ld as a parameter.\n", BOTS_APP_DESC_ARG_CUTOFF, bots_arg_size);
        bots_app_cutoff_value = bots_arg_size;
     }

     if (bots_app_cutoff_value_1 > bots_arg_size ) {
        bots_message("%s can not be greather than vector size, using %ld as a parameter.\n", BOTS_APP_DESC_ARG_CUTOFF_1, bots_arg_size);
        bots_app_cutoff_value_1 = bots_arg_size;
     }
     if (bots_app_cutoff_value_2 > bots_arg_size ) {
        bots_message("%s can not be greather than vector size, using %ld as a parameter.\n", BOTS_APP_DESC_ARG_CUTOFF_2, bots_arg_size);
        bots_app_cutoff_value_2 = bots_arg_size;
     }

     if (bots_app_cutoff_value_2 > bots_app_cutoff_value_1) {
        bots_message("%s can not be greather than %s, using %d as a parameter.\n",
		BOTS_APP_DESC_ARG_CUTOFF_2,
		BOTS_APP_DESC_ARG_CUTOFF_1,
		bots_app_cutoff_value_1
	);
        bots_app_cutoff_value_2 = bots_app_cutoff_value_1;
     }

     array = (ELM *) alloc(bots_arg_size * sizeof(ELM));
     tmp = (ELM *) alloc(bots_arg_size * sizeof(ELM));

    // first touch
    int i;
 #pragma omp parallel for schedule(static)
     for (i = 0; i < bots_arg_size; ++i) {
	      array[i] = 0;
     }

 #pragma omp parallel for schedule(static)
     for (i = 0; i < bots_arg_size; ++i) {
	      tmp[i] = 0;
     }

    // check for existing file otherwise write
    if(FileExists(bots_arg_size))
    {
      ReadFile(array, bots_arg_size);
    }
    else
    {
      fill_array(array);
      scramble_array(array);
      WriteFile(array, bots_arg_size, 1);
    }

     //fill_array(array);
     //fill_array(tmp);
     //scramble_array(array);
}

void sort_par ( void )
{
	#ifndef SCHEDULE_TYPE
	#   define SCHEDULE_TYPE 101
	#endif
	#ifndef SCHEDULE_NUM
	#   define SCHEDULE_NUM 20
	#endif

     init_task_affinity();
     /*
	#ifdef TASK_AFF_DOMAIN_FIRST
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_first, kmp_task_aff_map_type_domain, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_DOMAIN_RAND
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_random, kmp_task_aff_map_type_domain, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_DOMAIN_LOWEST
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_lowest_wl, kmp_task_aff_map_type_domain, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_DOMAIN_PRIVATE
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_private, kmp_task_aff_map_type_domain, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_DOMAIN_RR
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_round_robin, kmp_task_aff_map_type_domain, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_THREAD_FIRST
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_first, kmp_task_aff_map_type_thread, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_THREAD_RAND
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_random, kmp_task_aff_map_type_thread, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_THREAD_LOWEST
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_lowest_wl, kmp_task_aff_map_type_thread, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif
	#ifdef TASK_AFF_THREAD_RR
	  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_round_robin, kmp_task_aff_map_type_thread, SCHEDULE_TYPE , SCHEDULE_NUM);
	#endif*/

	#ifdef _FILTER_EXEC_TIMES
	  kmpc_task_affinity_taskexectimes_set_enabled(0);
	#endif

	double t_overall;
	t_overall = omp_get_wtime();
	bots_message("Computing multisort algorithm (n=%ld) ", bots_arg_size);
	#pragma omp parallel
	#pragma omp single nowait
#if USE_UNTIED_VER
	#pragma omp task untied
#else
	#pragma omp task
#endif
	     cilksort_par(array, tmp, bots_arg_size);
	bots_message(" completed!\n");
	t_overall = omp_get_wtime() - t_overall;
	printf("Elapsed time for program\t%lf\tsec\n",t_overall);

  if(sort_verify() == BOTS_RESULT_SUCCESSFUL)
    printf("Sort successfull\n");
  else
    printf("Sort failed\n");
}

int sort_verify ( void )
{
     int i, success = 1;
     for (i = 0; i < bots_arg_size; ++i)
	  if (array[i] != i) {
	       success = 0;
		   break;
	  }

     return success ? BOTS_RESULT_SUCCESSFUL : BOTS_RESULT_UNSUCCESSFUL;
}
