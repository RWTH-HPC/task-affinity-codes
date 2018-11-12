/*
*  This file is part of Christian's OpenMP Task-parallel QuickSort
*
*  Copyright (C) 2013 by Christian Terboven <christian@terboven.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
*/

#include <iostream>
#include <algorithm>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
 /* Not technically required, but needed on some UNIX distributions */
#include <sys/types.h>
#include <sys/stat.h>

#include <cmath>
#include <ctime>
#include <cstring>

#include <errno.h>
#include <numaif.h>

#include <omp.h>

#include <no_huge_page_alloc.h>

#define KMP_PRINT_MSG 0
#define TASK_AFF_UNTIED 0
#define PAGE_ALIGN 0

#ifndef T_AFF_NUMBER_TASKS_MULTIPLICATOR
  #define T_AFF_NUMBER_TASKS_MULTIPLICATOR 5
#endif

#if PAGE_ALIGN
# include <malloc.h>
#endif

int FileExists(const size_t size)
{
  char buf[255];
  sprintf(buf, "input__%lu", size);
  if(access(buf, F_OK) != -1)
    return 1;
  else
    return 0;
}

template <class T>
void ReadFile(T array[], const size_t size)
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

  int *tmp_arr = (int*) mem;
  size_t i;
  for(i = 0; i < size; i++)
  {
    array[i] = tmp_arr[i];
  }

  munmap(mem, st.st_size);
  close(fd);
}


template <class T>
void WriteFile(T array[], const size_t size, bool binary)
{
  size_t pagesize = sysconf(_SC_PAGESIZE);
#define PAGES(size) ((size + (pagesize - 1)) & ~(pagesize - 1))
  char buf[255];
  if(binary)
    sprintf(buf, "input__%lu", size);
  else
    sprintf(buf, "input__%lu_pure", size);

  if(binary){
  int* value = (int*)malloc(PAGES(sizeof(int) * size));
  memset(value, 0, PAGES(sizeof(int) * size));
  memcpy(value, array, sizeof(int) * size);

  FILE *f;
  f = fopen(buf, "wb");
  fwrite(value, 1, PAGES(sizeof(int) * size), f);
  fclose(f);
  free(value);
  } else {
    FILE *f;
    f = fopen(buf, "wb");
    size_t i;
    for(i = 0; i < size; i++)
    {
      fprintf(f, "%d ", array[i]);
    }
    fclose(f);
  }
}

/**
  * helper routine: check whether array is sorted
  */
template <class T>
bool isSorted(T array[], const size_t size){
	for (size_t idx = 1; idx < size; ++idx){
		if (array[idx - 1] > array[idx]){
			return false;
		}
	}
	return true;
}


/**
  * sequential merge step (straight-forward implementation)
  */
template <class T>
void MsMergeSequential(T out[], T in[], long begin1, long end1, long begin2, long end2, long outBegin) {
	long left = begin1;
	long right = begin2;

	long idx = outBegin;

	while (left < end1 && right < end2) {
		if (in[left] <= in[right]) {
			out[idx] = in[left];
			left++;
		} else {
			out[idx] = in[right];
			right++;
		}
		idx++;
	}

	while (left < end1) {
		out[idx] = in[left];
		left++, idx++;
	}

	while (right < end2) {
		out[idx] = in[right];
		right++, idx++;
	}
}


/**
  * sequential MergeSort
  * used in cut-off, or for timing reference and result verification
  */
template <class T>
void MsSequential(T array[], T tmp[], bool inplace, long begin, long end) {
	if (begin < (end - 1)) {
		const long half = (begin + end) / 2;
		MsSequential(array, tmp, !inplace, begin, half);
		MsSequential(array, tmp, !inplace, half, end);
		if (inplace) {
			MsMergeSequential(array, tmp, begin, half, half, end, begin);
		} else {
			MsMergeSequential(tmp, array, begin, half, half, end, begin);
		}
	} else if (!inplace) {
		tmp[begin] = array[begin];
	}
}

/**
  * parallel merge step (straight-forward implementation)
  */
template <class T>
void MsMergeParallel(T out[], T in[], long begin1, long end1, long begin2, long end2, long outBegin, int deep) {
  int nextDeep = deep - 1;
	if (deep) {
			long half1, half2, tmp, count, step;
			if ((end1 - begin1) < (end2 - begin2)) {
				half2 = (begin2 + end2) / 2;
				// find in[half2] in [begin1, end1) (std::upper_bound)
				half1 = begin1, count = (end1 - begin1);
				while (count > 0) {
					step = count / 2;
					tmp = half1 + step;
					if (in[tmp] <= in[half2]) {
						tmp++;
						half1 = tmp;
						count -= step + 1;
					} else {
						count = step;
					}
				}
			} else {
				half1 = (begin1 + end1) / 2;
				// find in[half1] in [begin2, end2) (std::lower_bound)
				half2 = begin2, count = (end2 - begin2);
				while (count > 0) {
					step = count / 2;
					tmp = half2 + step;
					if (in[tmp] < in[half1]) {
						tmp++;
						half2 = tmp;
						count -= step + 1;
					} else {
						count = step;
					}
				}
			}
#ifdef TASK_AFFINITY
#if KMP_PRINT_MSG
      char buf[50];
#endif
      if(nextDeep){
#if KMP_PRINT_MSG
        sprintf(buf, "MsMergeParallel %d", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
      }
      else{
#if KMP_PRINT_MSG
        sprintf(buf, "MsMergeParallel %d (seq)", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
        //kmpc_set_task_affinity(&out[begin1]);
        int len1 = half1 - begin1;
        int len2 = half2 - begin2;
        int len12 = len1+len2;
        kmpc_set_task_affinity(&out[begin1],&len12);
        kmpc_set_task_affinity(&in[begin1],&len1);
        kmpc_set_task_affinity(&in[begin2],&len2);
      }
#endif
#if TASK_AFF_UNTIED
			#pragma omp task default(shared) untied
#else
			#pragma omp task default(shared)
#endif
			{
				MsMergeParallel(out, in, begin1, half1, begin2, half2, outBegin, deep - 1);
			}

			long outBegin2 = outBegin + (half1 - begin1) + (half2 - begin2);
#ifdef TASK_AFFINITY
      if(nextDeep){
#if KMP_PRINT_MSG
        sprintf(buf, "MsMergeParallel %d", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
      }
      else{
#if KMP_PRINT_MSG
        sprintf(buf, "MsMergeParallel %d (seq)", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
        //kmpc_set_task_affinity(&out[half1]);
        int len1 = end1 - half1;
        int len2 = end2 - half2;
        int len12 = len1+len2;
        kmpc_set_task_affinity(&out[half1],&len12);
        kmpc_set_task_affinity(&in[half1],&len1);
        kmpc_set_task_affinity(&in[half2],&len2);
      }
#endif
#if TASK_AFF_UNTIED
			#pragma omp task default(shared) untied
#else
			#pragma omp task default(shared)
#endif
			{
				MsMergeParallel(out, in, half1, end1, half2, end2, outBegin2, deep - 1);
			}

      #pragma omp taskwait
	} else {
		MsMergeSequential(out, in, begin1, end1, begin2, end2, outBegin);
	}
}

/**
  * OpenMP Task-parallel MergeSort
  */
template <class T>
void MsParallel(T array[], T tmp[], bool inplace, long begin, long end, int deep) {
	if (begin < (end - 1)) {
		long half = (begin + end) / 2;
    int nextDeep = deep-1;

		if (deep){

#ifdef TASK_AFFINITY
#if KMP_PRINT_MSG
      char buf[50];
#endif
      if(nextDeep){
#if KMP_PRINT_MSG
        sprintf(buf, "MsParallel %d", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
      }
      else{
#if KMP_PRINT_MSG
        sprintf(buf, "MsParallel %d (seq)", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
        //kmpc_set_task_affinity(&tmp[begin]);
        int len = half - begin;
        kmpc_set_task_affinity(&array[begin],&len);
        kmpc_set_task_affinity(&tmp[begin],&len);
      }
#endif
#if TASK_AFF_UNTIED
			#pragma omp task default(shared) untied
#else
			#pragma omp task default(shared)
#endif
			{
				MsParallel(array, tmp, !inplace, begin, half, deep - 1);
			}
#ifdef TASK_AFFINITY
      if(nextDeep){
#if KMP_PRINT_MSG
        sprintf(buf, "MsParallel %d", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
      }
      else{
#if KMP_PRINT_MSG
        sprintf(buf, "MsParallel %d (seq)", nextDeep);
        kmpc_task_affinity_set_msg(buf);
#endif
        //kmpc_set_task_affinity(&tmp[half]);
        int len = end- half;
        kmpc_set_task_affinity(&array[half],&len);
        kmpc_set_task_affinity(&tmp[half],&len);
      }
#endif
#if TASK_AFF_UNTIED
			#pragma omp task default(shared) untied
#else
			#pragma omp task default(shared)
#endif
			{
				MsParallel(array, tmp, !inplace, half, end, deep - 1);
			}
    #pragma omp taskwait
		}
		else
    {
			MsSequential(array, tmp, !inplace, begin, half);
			MsSequential(array, tmp, !inplace, half, end);
		}

		if (inplace) {
			MsMergeParallel(array, tmp, begin, half, half, end, begin, deep);
		} else {
			MsMergeParallel(tmp, array, begin, half, half, end, begin, deep);
		}
	} else if (!inplace) {
		tmp[begin] = array[begin];
	}
}


/**
  * OpenMP Task-parallel MergeSort
  * startup routine containing the Parallel Region
  */
template <class T>
void MsParallelOmp(T array[], T tmp[], const size_t size) {

	// compute cut-off recursion level
	const int iMinTask = (omp_get_max_threads() * T_AFF_NUMBER_TASKS_MULTIPLICATOR);
  // fprintf(stderr, "iMinTasks = %d\n", iMinTask);
	std::cout << "Running with iMinTasks=" << iMinTask << std::endl;
	int deep = 0;
	while ((1 << deep) < iMinTask) deep += 1;

#ifdef T_AFF_CUTOFF_DEPTH
  deep = T_AFF_CUTOFF_DEPTH;
#endif
	std::cout << "Running with deep=" << deep << std::endl;

#pragma omp parallel
#pragma omp master
	{
		MsParallel(array, tmp, true, 0, size, deep);
	}
}

#define xstr(s) str(s)
#define str(s) #s
#define ELEMENT_T int

/** @brief program entry point
  */
int main(int argc, char* argv[]) {
	/*
  int res = set_mempolicy(MPOL_PREFERRED, NULL, 0);
	if (res) {
		std::cerr << "set_mempolicy failed (errno=" << errno << ")" << std::endl;
		return -1;
	}
  */

	// measure the time
	double t1, t2;

    // choose which policy you want to use by specifying FLAG during compile process
    //also SCHEDULE_TYPE and SCHEDULE_NUM can be specified for other strategies
    #ifndef SCHEDULE_TYPE
    #   define SCHEDULE_TYPE 101
    #endif
    #ifndef SCHEDULE_NUM
    #   define SCHEDULE_NUM 20
    #endif

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
    #endif

	// expect one command line arguments: array size
	if (argc != 2) {
		std::cout << "Usage: MergeSort.exe <array size> " << std::endl;
		std::cout << std::endl;
		return -1;
	}
	else {
		const size_t stSize = strtol(argv[1], NULL, 10);
		//ELEMENT_T *data = new ELEMENT_T[stSize];
		//ELEMENT_T *tmp = new ELEMENT_T[stSize];

    ELEMENT_T *data = (ELEMENT_T *) alloc(stSize * sizeof(ELEMENT_T));
    ELEMENT_T *tmp = (ELEMENT_T *) alloc(stSize * sizeof(ELEMENT_T));

#if PAGE_ALIGN
    size_t pagesize;
    if ((pagesize = sysconf(_SC_PAGESIZE)) < 1)
      pagesize = 4096;

		std::cout << "Use memory alignment with pagesize = " << pagesize << std::endl;
    data = (ELEMENT_T*) memalign(pagesize, sizeof(ELEMENT_T) * stSize);
    tmp = (ELEMENT_T*) memalign(pagesize, sizeof(ELEMENT_T) * stSize);
#endif

		// first touch
		#pragma omp parallel for schedule(static)
		for (size_t idx = 0; idx < stSize; ++idx){
			data[idx] = 0;
			tmp[idx] = 0;
		}

    std::cout << "Initialization..." << std::endl;
    if(FileExists(stSize))
    {
      ReadFile(data, stSize);
    }
    else
    {
      srand(95);
      for (size_t idx = 0; idx < stSize; ++idx){
        data[idx] = ELEMENT_T(stSize * (double(rand()) / RAND_MAX));
      }
      WriteFile(data, stSize, true);
    }
	  fprintf (stderr, "Initialization done...\n");

		double dSize = (stSize * sizeof(ELEMENT_T)) / 1024 / 1024;
		std::cout << "Sorting " << stSize << " elements of type " xstr(ELEMENT_T) " (" << dSize << " MiB)..." << std::endl;

		t1 = omp_get_wtime();
		MsParallelOmp<ELEMENT_T>(data, tmp, stSize);
		t2 = omp_get_wtime() - t1;
    printf("Elapsed time for program\t%lf\tsec\n",t2);
		std::cout << "done, took " << t2 << " sec. Verification...";
		if (isSorted<ELEMENT_T>(data, stSize)) {
			std::cout << " successful." << std::endl;
		}
		else {
			std::cout << " FAILED." << std::endl;
		}

		delete[] data;
		delete[] tmp;
	}

	return 0;
}
