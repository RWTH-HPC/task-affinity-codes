#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef _WIN32
# include <unistd.h>
#else
# include <windows.h>
#endif

#include "io.h"

static size_t pagesize;

#define PAGES(size) ((size + (pagesize - 1)) & ~(pagesize - 1))

static void
error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	puts("");
	abort();
}

static void
dump(const char *path, struct MatrixCRS *A)
{
	FILE *fp;
	struct {
		int n, nnz;
	} *info;
	int *ptr, *index;
	floatType *value;

	if ((fp = fopen(path, "wb")) == NULL)
		error("Can't open '%s' for reading", path);

	/* Dump matrix info */
	if ((info = (void*)malloc(pagesize)) == NULL)
		error("Can't allocate memory for info");

	memset(info, 0, pagesize);
	info->n = A->n;
	info->nnz = A->nnz;

	fwrite(info, 1, pagesize, fp);

	free(info);

	/* Dump ptr */
	if ((ptr = (int*)malloc(PAGES(sizeof(int) * (A->n + 1)))) == NULL)
		error("Can't allocate memory for ptr");

	memset(ptr, 0, PAGES(sizeof(int) * (A->n + 1)));
	memcpy(ptr, A->ptr, sizeof(int) * (A->n + 1));
	fwrite(ptr, 1, PAGES(sizeof(int) * (A->n + 1)), fp);
	free(ptr);

	/* Dump index */
	if ((index = (int*)malloc(PAGES(sizeof(int) * A->nnz))) == NULL)
		error("Can't allocate memory for index");

	memset(index, 0, PAGES(sizeof(int) * A->nnz));
	memcpy(index, A->index, sizeof(int) * A->nnz);
	fwrite(index, 1, PAGES(sizeof(int) * A->nnz), fp);
	free(index);

	/* Dump value */
	value = (floatType*)malloc(PAGES(sizeof(floatType) * A->nnz));
	if (value == NULL)
		error("Can't allocate memory for value");

	memset(value, 0, PAGES(sizeof(floatType) * A->nnz));
	memcpy(value, A->value, sizeof(floatType) * A->nnz);
	fwrite(value, 1, PAGES(sizeof(floatType) * A->nnz), fp);
	free(value);

	fclose(fp);
}

static char*
get_output_path(const char *path)
{
	size_t len = strlen(path);
	char *out, *tmp;

	if (len < 4 || memcmp(path + len - 4, ".mtx", 5))
		return "matrix.mtxdump";

	if ((out = malloc(len + 5)) == NULL)
		error("Can't allocate memory for output path");

	memcpy(out, path, len);
	memcpy(out + len, "dump", 5);

	if ((tmp = strrchr(out, '/')) != NULL)
		return tmp + 1;

	return out;
}

int
main(int argc, char *argv[])
{
	struct MatrixCRS A;

#ifdef _WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	pagesize = si.dwPageSize;
#else
	if ((pagesize = sysconf(_SC_PAGESIZE)) < 1)
		pagesize = 4096;
#endif

	parseMM(argv[1], &A);

	dump(get_output_path(argv[1]), &A);

	return 0;
}
