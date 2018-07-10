#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#ifdef PAGE_ALIGN
# include <malloc.h>
#endif

#ifndef _WIN32
# include <fcntl.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/mman.h>
#endif

#ifndef O_BINARY
# define O_BINARY 0
#endif

#define PAGES(size) ((size + (pagesize - 1)) & ~(pagesize - 1))
//#define MAP_HUGETLB  0x40000

#include "def.h"
#include "io.h"
#include "mmio.h"

#ifdef USE_OMP
# include <omp.h>
#endif


//#####################################################################
// calc_data_dist
//#####################################################################
#ifdef DATA_DISTRIBUTION
void
calc_data_dist(struct MatrixCRS *A)
{
	calc_data_dist_(A, omp_get_max_threads());
}

void
calc_data_dist_(struct MatrixCRS *A, int numTasks)
{
	int i;
	int nnz = 0;
	int row = 0;
	int block_size = A->nnz / numTasks;

	COUT1("Precalculate data distribution for %d threads.\n", numTasks);

	if ((A->blockptr = malloc(sizeof(int) * (numTasks + 1))) == NULL) {
		puts("Out of memory");
		exit(1);
	}

	for (i = 0; i < numTasks; i++) {
		while (nnz < block_size * i) {
			nnz += A->ptr[row+1] - A->ptr[row];
			row++;
		}

		A->blockptr[i] = row;
		COUT2("blockptr[%d] = %d.\n", i, row);
	}

	A->blockptr[numTasks] = A->n;

	COUT("Precalculate data distribution done.\n");
}
#endif


//#####################################################################
// getMinNNZ
//#####################################################################
//Return the minimum number of NNZ per thread
int getMinNNZ(struct MatrixCRS* A){
	int min_nnz = INT_MAX;
	int local_nnz;
#ifdef USE_OMP
# ifdef DATA_DISTRIBUTION
#  pragma omp parallel private(local_nnz) shared(min_nnz)

	{
		int thread, bs, be;
		thread = omp_get_thread_num();
		bs = A->blockptr[thread];
		be = A->blockptr[thread + 1];
		local_nnz = A->ptr[be] - A->ptr[bs];

#   pragma omp critical
		{
		if (local_nnz < min_nnz)
			min_nnz = local_nnz;
		}//end critical
	}//end parallel

// No data distribution
# else
# pragma omp parallel private(local_nnz) shared(min_nnz)

	{
	int i;
	local_nnz = 0;
#  pragma omp for RUNTIME_SCHEDULE

		for (i=0; i<A->n; i++){
			local_nnz += A->ptr[i+1] - A->ptr[i];
		}
#  pragma omp critical
		{
			if (local_nnz < min_nnz)
				min_nnz = local_nnz;
		}//end critical
	}//end parallel

# endif

//No OMP
#else
	min_nnz = A->nnz;
#endif

	return min_nnz;
}


//#####################################################################
// getMaxNNZ
//#####################################################################
//Return the maximum number of NNZ per thread
int getMaxNNZ(struct MatrixCRS* A){
	int max_nnz = 0;
	int local_nnz;
#ifdef USE_OMP
# ifdef DATA_DISTRIBUTION
#  pragma omp parallel private(local_nnz) shared(max_nnz)
	{
		int thread, bs, be;
		thread = omp_get_thread_num();
		bs = A->blockptr[thread];
		be = A->blockptr[thread + 1];
		local_nnz = A->ptr[be] - A->ptr[bs];

#   pragma omp critical
		{
		if (local_nnz > max_nnz)
			max_nnz = local_nnz;
		}//end critical
	}//end parallel

// No data distribution
# else
# pragma omp parallel private(local_nnz ) shared(max_nnz)

	{
	int i;
	local_nnz = 0;
#  pragma omp for RUNTIME_SCHEDULE
		for (i=0; i<A->n; i++){
			local_nnz += A->ptr[i+1] - A->ptr[i];
		}
#  pragma omp critical
		{
			if (local_nnz > max_nnz)
				max_nnz = local_nnz;
		}//end critical
	}//end parallel

# endif

//No OMP
#else
	max_nnz = A->nnz;
#endif

	return max_nnz;
}


//#####################################################################
// getMinN
//#####################################################################
//Return the minimum number of lines per thread
int getMinN(struct MatrixCRS* A){
	int min_n = INT_MAX;
	int local_n;
#ifdef USE_OMP
# ifdef DATA_DISTRIBUTION
#  pragma omp parallel private(local_n) shared(min_n)
	{
		int thread, bs, be;
		thread = omp_get_thread_num();
		bs = A->blockptr[thread];
		be = A->blockptr[thread + 1];
		local_n = be-bs;

#   pragma omp critical
		{
		if (local_n < min_n)
			min_n = local_n;
		}//end critical
	}//end parallel

// No data distribution
# else
# pragma omp parallel private(local_n) shared(min_n)

	{
	int i;
	local_n = 0;
#  pragma omp for RUNTIME_SCHEDULE
		for (i=0; i<A->n; i++){
			local_n++;
		}
#  pragma omp critical
		{
			if (local_n < min_n)
				min_n = local_n;
		}//end critical
	}//end parallel

# endif

//No OMP
#else
	min_n = A->n;
#endif

	return min_n;
}


//#####################################################################
// getMaxN
//#####################################################################
//Return the maximum number of lines per thread
int getMaxN(struct MatrixCRS* A){
	int max_n = 0;
	int local_n;
#ifdef USE_OMP
# ifdef DATA_DISTRIBUTION
#  pragma omp parallel private(local_n) shared(max_n)
	{
		int thread, bs, be;
		thread = omp_get_thread_num();
		bs = A->blockptr[thread];
		be = A->blockptr[thread + 1];
		local_n = be-bs;

#   pragma omp critical
		{
		if (local_n > max_n)
			max_n = local_n;
		}//end critical
	}//end parallel

// No data distribution
# else
# pragma omp parallel private(local_n) shared(max_n)

	{
	int i;
	local_n = 0;
#  pragma omp for RUNTIME_SCHEDULE
		for (i=0; i<A->n; i++){
			local_n++;
		}
#  pragma omp critical
		{
			if (local_n > max_n)
				max_n = local_n;
		}//end critical
	}//end parallel

# endif

//No OMP
#else
	max_n = A->n;
#endif

	return max_n;
}

//#####################################################################
// first_touch_vector
// TODO: Add TASK_INIT
//#####################################################################
// init parallel to avoid ccNUMA probs for the vectors
void first_touch_vector(floatType* vector, int n, const struct MatrixCRS* const A){
	int i;

#ifdef USE_OMP
# ifndef DATA_DISTRIBUTION
#  ifndef SERIAL_INIT
#   pragma omp parallel for RUNTIME_SCHEDULE
#  endif
	for (i = 0; i < n; i++){
# else
  fprintf(stderr, "Vector first touch = data dist\n");
  int thread, bs, be;
#  pragma omp parallel private(thread, bs, be, i)
  {
    thread = omp_get_thread_num();
    bs = A->blockptr[thread];
    be = A->blockptr[thread + 1];

    for (i = bs; i < be; i++) {
# endif
			vector[i] = 0.0;
	}
#endif

#ifdef DATA_DISTRIBUTION
}
#endif

}


//#####################################################################
// first_touch_mtx
//#####################################################################
// init parallel to avoid ccNUMA probs for mtx files
void
first_touch_mtx(struct MatrixCRS *A, int numTasks)
{
	int do_distr_init = 1;
	int chunkSize;
	int c, i, j;

#ifdef TASK_INIT
	chunkSize = N / numTasks;
	if (N % numTasks != 0)
		chunkSize++;
# pragma omp parallel
	{
# ifndef PAR_PRODUCER
#  pragma omp single
	{
# else
#  pragma omp for private(i, j) RUNTIME_SCHEDULE
# endif
		for (c = 0; c < A->n; c += chunkSize) {
# pragma omp task
			{
				for (i = c; i < c + chunkSize &&
				    i < A->n; i++) {
					for (j = A->ptr[i];
					    j < A->ptr[i + 1]; j++) {
						A->value[j] = 0.0;
						A->index[j] = 0.0;
					}
				}
			}
		} //task
# ifndef PAR_PRODUCER
	} // single
# endif
	} // parallel

// Start non TASK versions
#else

# ifdef DATA_DISTRIBUTION
//TODO:FIX THIS. FOR TOUCH DUMP THIS IS OK
	if(do_distr_init)
	{
		calc_data_dist_(A, omp_get_max_threads());

		int thread, bs, be;
#pragma omp parallel private(thread, bs, be, i, j)
  		{
		thread = omp_get_thread_num();
		bs = A->blockptr[thread];
		be = A->blockptr[thread + 1];

		COUT2("First touch blockptr[%d] = %d\n", thread, bs);
		for (i = bs; i < be; i++) {
			for (j = A->ptr[i]; j < A->ptr[i + 1]; j++) {
				A->index[j] = 0;
				A->value[j] = 0.0;
			}
		}
		}
	} else {
		printf("ERROR: DATA_DISTRIBUTION IS NOT WORKING CORRECTLY FOR MTX FILES. PLEASE USE MTXDUMP\n");exit(1);
	}
# endif

# if defined(CHUNK_INIT)
#  pragma omp parallel for private(i, j) schedule(dynamic, chunkSize) RUNTIME_SCHEDULE
# elif defined(SERIAL_INIT)
	// Do not use any pragma for serial case, so this is empty
# elif defined(USE_OMP)
#  pragma omp parallel for private(i, j) RUNTIME_SCHEDULE
# endif
	for (i = 0; i < A->n; i++) {
		for (j = A->ptr[i]; j < A->ptr[i + 1]; j++) {
			A->value[j] = 0.0;
			A->index[j] = 0.0;
		}
	}
#endif
}

//#####################################################################
// first_touch_dump
//#####################################################################
// init parallel to avoid ccNUMA probs for mtxdump files
void
first_touch_dump(struct MatrixCRS *A, int numTasks)
{
	int i, j;
#ifdef TASK_INIT
	int chunkSize = A->n / numTasks;
		if (A->n % numTasks != 0)
			chunkSize++;
# pragma omp parallel
		{
# ifndef PAR_PRODUCER
#  pragma omp single
		{
# else
#  pragma omp for private(i, j) RUNTIME_SCHEDULE
# endif
		int c;
			for (c = 0; c < A->n; c += chunkSize) {
# pragma omp task
				{
					for (i = c; i < c + chunkSize &&
					    i < A->n; i++) {
						for (j = A->ptr[i];
						    j < A->ptr[i + 1]; j++) {
							volatile floatType
							    *value = A->value;
							value[j] *= -1;
							value[j] *= -1;
						volatile int
							    *index = A->index;
							index[j] ^= 1;
							index[j] ^= 1;
						}
					}
				}
			} //task
# ifndef PAR_PRODUCER
		} // single
# endif
		} // parallel

// Start non TASK versions
#else
# if defined(DATA_DISTRIBUTION) && !defined(STATIC_INIT)
  printf("First touch with data distribution\n");
  int thread, bs, be;
#  pragma omp parallel private(thread, bs, be, i, j)
  {
		thread = omp_get_thread_num();
		bs = A->blockptr[thread];
		be = A->blockptr[thread + 1];

		COUT2("First touch blockptr[%d] = %d\n", thread, bs);
		for (i = bs; i < be; i++) {
# else
#  if defined(CHUNK_INIT)
#   pragma omp parallel for private(i, j) schedule(static, chunkSize)
#  elif defined(SERIAL_INIT)
		// Do not use any pragma for serial case, so this is empty
#  elif defined(USE_OMP)
#   ifdef STATIC_INIT
	printf("First touch with static init over rows...\n");
	//printf("Warning: Using static init\n");
#   endif
#   pragma omp parallel for private(i, j) RUNTIME_SCHEDULE
#  endif
	for (i = 0; i < A->n; i++) {
# endif
		for (j = A->ptr[i]; j < A->ptr[i + 1]; j++) {
			volatile floatType *value = A->value;
			value[j] *= -1;
			value[j] *= -1;
			volatile int *index = A->index;
			index[j] ^= 1;
			index[j] ^= 1;
		}
	}
#endif

#if defined(DATA_DISTRIBUTION) && !defined(STATIC_INIT)
}
#endif

}


//#####################################################################
// parseRHS
//#####################################################################
void parseRHS(char *filename, floatType *b){
  FILE* fp;
  char line[256];
  int i;

  COUT("Start reading RHS.\n");
  if ((fp = fopen(filename, "r")) == NULL) {
    printf("ERROR: Cant open file!\n");
    exit(1);
  }

  i=0;
  while (fgets(line, 256, fp)){
    b[i] = strtod(line, NULL);
    ++i;
  }

  printVector(b, 10);
}

//#####################################################################
// get_pagesize
//#####################################################################
size_t
get_pagesize()
{
	size_t pagesize;
#ifdef _WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	pagesize = si.dwPageSize;
#else
	if ((pagesize = sysconf(_SC_PAGESIZE)) < 1)
		pagesize = 4096;
#endif
	return pagesize;
}

//#####################################################################
// parseMTXDump
//#####################################################################
void
parseMTXDump(const char *filename, struct MatrixCRS *A, int numTasks)
{
	size_t pagesize;
	int fd = open(filename, O_RDONLY | O_BINARY);
	CHKERR(fd == -1, "open()");

	struct stat st;
	fstat(fd, &st);

	pagesize = get_pagesize();

	//TODO: REMOVE QUICK HACK for huge pages
	//pagesize = 64*2*1024*1024; //128MB
	COUT1("Pagesize = %ld\n", pagesize);
	COUT1("st.st_size = %ld\n", st.st_size);
	char *mem = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE,
	    MAP_FILE | MAP_PRIVATE, fd, 0);

	//void *mem2 = mmap(0, pagesize, PROT_READ | PROT_WRITE,
	//    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
	CHKERR(mem == MAP_FAILED, "mmap()");
	//CHKERR(mem2 == MAP_FAILED, "mmap()");

	struct {
		int n, nnz;
	} *info = (void*)mem;

	A->n = info->n;
	A->nnz = info->nnz;
	A->mmapped = 1;

	mem += pagesize;

	A->ptr = (int*)mem;
	mem += PAGES(sizeof(int) * (A->n + 1));

	A->index = (int*)mem;
	mem += PAGES(sizeof(int) * A->nnz);

	A->value = (floatType*)mem;

#ifdef DATA_DISTRIBUTION
# ifdef TASKING
	calc_data_dist_(A, numTasks);
# else
	calc_data_dist(A);
# endif
#endif

	first_touch_dump(A, numTasks);

	return;
}


//#####################################################################
// parseMM
//#####################################################################
void
parseMM(const char *filename, struct MatrixCRS *A)
{
	parseMM_(filename, A, 0);
}

void
parseMM_(const char *filename, struct MatrixCRS *A, int numTasks)
{
	int M,N, nnz;
	int i,j;
	int temp;
	int *I, *J, *offset;
	floatType *V;
	FILE *fp;
	size_t pagesize;

	char *tmp = strrchr(filename, '.');
	pagesize=get_pagesize();
	if (tmp != NULL && !strcmp(tmp, ".mtxdump"))
		parseMTXDump(filename, A, numTasks);
	else {
		A->mmapped = 0;

		COUT("Start Matrix parse.\n");

		if ((fp = fopen(filename, "r")) == NULL) {
			printf("ERROR: Cant open file!\n");
			exit(1);
		}

		MM_typecode matcode;
		if (mm_read_banner(fp, &matcode) != 0) {
			printf("ERROR: Could not process Matrix Market banner.\n");
			exit(1);
		}

		/*
		 * This is how one can screen matrix types if their application only
		 * supports a subset of the Matrix Market data types.
		 */
		if (mm_is_complex(matcode) && mm_is_matrix(matcode) &&
		    mm_is_sparse(matcode)) {
			printf("ERROR: Sorry, this application does not support ");
			printf("Market Market type: [%s]\n",
		    mm_typecode_to_str(matcode));
			exit(1);
		}

		/* find out size of sparse matrix .... */
		if (mm_read_mtx_crd_size(fp, &M, &N, &nnz) != 0) {
			printf("ERROR: Could not read matrix size!\n");
			exit(1);
		}

		if (N != M) {
			printf("ERROR: Naahhh. Come on, give me a NxN matrix!\n");
			exit(1);
		}

		if (mm_is_symmetric(matcode)){
			printf("Warning: Matrix is stored in symmetric style, but allocated in full CRS\n");

			// store upper and lower triangular
			nnz = 2 * nnz - N;
		}

		COUT("Start memory allocation.\n");

		/* memory for CRS Matrix */
#ifdef PAGE_ALIGN
		A->value = (floatType*)memalign(pagesize, sizeof(floatType) * nnz);
		CHKERR(A->value == NULL, "memalign()");
		A->index = (int*)memalign(pagesize, sizeof(int) * nnz);
		CHKERR(A->value == NULL, "memalign()");
		A->ptr = (int*)memalign(pagesize, sizeof(int) * (N + 1));
		CHKERR(A->value == NULL, "memalign()");
#else
		A->value = (floatType*)malloc(sizeof(floatType) * nnz);
		CHKERR(A->value == NULL, "malloc()");
		A->index = (int*)malloc(sizeof(int) * nnz);
		CHKERR(A->index == NULL, "malloc()");
		A->ptr = (int*)malloc(sizeof(int) * (N + 1));
		CHKERR(A->ptr == NULL, "malloc()");
#endif


//TODO: This first touch might not be optimal for DATA_DIST
#ifdef USE_OMP
# pragma omp parallel for private(i) RUNTIME_SCHEDULE
#endif
	for (i = 0; i < N + 1; i++)
		A->ptr[i] = 0;

	A->nnz = nnz;
	A->n = N;

	/* memory for MM Matrix */
	I = (int*)malloc(sizeof(int) * nnz);
	J = (int*)malloc(sizeof(int) * nnz);
	V = (floatType*)malloc(sizeof(floatType) * nnz);

	if (I == NULL || J == NULL || V == NULL) {
		puts("Out of memory!");
		exit(1);
	}

	/* temporay memory */
	if ((offset = (int*)malloc(sizeof(int) * nnz)) == NULL) {
		puts("Out of memory!");
		exit(1);
	}
	memset(offset, 0, nnz * sizeof(int));

	COUT("Read from file.\n");

	for (i = 0; i < nnz; i++) {
		fscanf(fp, "%d %d %lg\n", &I[i], &J[i], &V[i]);


		// count double if entry is not on diag and in symmetric file format
		if (I[i] != J[i] && mm_is_symmetric(matcode)){
			(A->ptr[I[i]])++;
			i++;
			I[i] = J[i-1];
			J[i] = I[i-1];

			I[i-1]--;  /* adjust from 1-based to 0-based */
			J[i-1]--;

			V[i] = V[i-1];
		}

		// count entries in one line
		(A->ptr[I[i]])++;


		I[i]--;  /* adjust from 1-based to 0-based */
		J[i]--;
	}

	COUT("Start converting from MM to CRS.\n");

	// convert MM to CRS
	for (i = 0; i < N; i++){
		/* calculate row ptr with the saved offset */
		A->ptr[i + 1] += A->ptr[i];
	}

	first_touch_mtx(A, numTasks);

	for (j = 0; j < nnz; j++){
		temp = A->ptr[I[j]];
		A->index[temp + offset[temp]] = J[j];
		A->value[temp + offset[temp]] = V[j];
		offset[temp]++;
	}

#ifdef DATA_DISTRIBUTION
# ifdef TASKING
	calc_data_dist_(A,numTasks);
# else
	calc_data_dist(A);
#endif
#endif

	COUT("MM Parse done.\n");

	free(offset);
	free(I);
	free(J);
	free(V);
	fclose(fp);
	}
}


//#####################################################################
// destroyMatrix
//#####################################################################
void
destroyMatrix(struct MatrixCRS *A)
{
	if (A->mmapped) {
		/* FIXME */
	} else {
		free(A->value);
		free(A->index);
		free(A->ptr);
#ifdef DATA_DISTRIBUTION
		free(A->blockptr);
#endif
	}
}

void
parseVector(const char *filename, floatType *x, int n)
{
	FILE *fp;
	char line[64];
	int i = 0;

	if ((fp = fopen(filename, "r")) == NULL) {
		printf("Failed to open %s!\n", filename);
		exit(1);
	}

	while (i < n && fgets(line, 64, fp) != NULL)
		x[i++] = strtod(line, NULL);

	if (i < n) {
		puts("RHS too small!");
		exit(1);
	}

	fclose(fp);
}

//#####################################################################
// printVector
//#####################################################################
void
printVector(const floatType *x, const int n)
{
	int i;

	printf("(");
	for (i = 0; i < n; i++)
		printf("%d:%e' ", i, x[i]);
	printf(")\n");
}


//#####################################################################
// printMatrix
//#####################################################################
void
printMatrix(const struct MatrixCRS *A)
{
	int i;
	int rowIndex = 0;

	for (i = 0; i < A->nnz; i++) {
		if (i == 0)
			printf("Row %d: [", 0);

		/*
		 * We have reached the next row if the actual iteration is
		 * bigger than the next row pointer
		 */
		if (i >= A->ptr[rowIndex + 1]) {
			rowIndex++;
			printf("]\nRow %d: [", rowIndex);
		}

		printf("%d:", A->index[i]);
		printf("%f' ", A->value[i]);
	}

	printf("]\n");
}
