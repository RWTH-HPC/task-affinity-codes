#include <stdlib.h>
#include "io.h"
#include "mmio.h"

//void matConvMM2CRS()

void parseMM(char* filename, struct MatrixCRS* A){
	int M,N, nnz;
	int i,j;
	int* I, *J, *offset;
	floatType* V;
	FILE* f;
	int temp;

	COUT("Start Matrix parse.\n");

	MM_typecode matcode;

		if((f=fopen(filename, "r"))==NULL){
		printf("ERROR: Cant open file!\n");
		exit(1);
	}

	if (mm_read_banner(f, &matcode) != 0){
		printf("ERROR: Could not process Matrix Market banner.\n");
		exit(1);
	}

	/*  This is how one can screen matrix types if their application */
	/*  only supports a subset of the Matrix Market data types.      */
	if (mm_is_complex(matcode) && mm_is_matrix(matcode) &&  mm_is_sparse(matcode) ){
		printf("ERROR: Sorry, this application does not support ");
		printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
		exit(1);
	}

	/* find out size of sparse matrix .... */
	if (mm_read_mtx_crd_size(f, &M, &N, &nnz) !=0){
		printf("ERROR: Could not read matrix size!\n");
		exit(1);
	}

	if(N!=M){
		printf("ERROR: Naahhh. Come on, give me a NxN matrix!\n");
		exit(1);
	}

	COUT("Start memory allocation.\n");

	/* memory for CRS Matrix */
	A->value = (floatType*)malloc(sizeof(floatType)*nnz);
	A->index = (int*)malloc(sizeof(int)*nnz);
	A->ptr  = (int*)malloc(sizeof(int)*(N+1));

	#pragma omp parallel for private(i)
	for(i=0; i<N+1; i++)
		A->ptr[i]=0;

	A->nnz=nnz;
	A->n=N;

	/* memory for MM Matrix */
	I=(int*)malloc(sizeof(int)*nnz);
	J=(int*)malloc(sizeof(int)*nnz);
	V=(floatType*)malloc(sizeof(floatType)*nnz);

	/* temporay memory */
	offset  = (int*)malloc(sizeof(int)*nnz);
	memset(offset,0,nnz*sizeof(int));

	COUT("Read from file.\n");

	for (i=0; i<nnz; i++){
		fscanf(f, "%d %d %lg\n", &I[i], &J[i], &V[i]);

		// count entries in one line
		(A->ptr[I[i]])++;

		I[i]--;  /* adjust from 1-based to 0-based */
		J[i]--;

	}

	COUT("Start converting from MM to CRS.\n");


	// convert MM to CRS //TODO: This seems to be really inefficient...
	for (i=0; i<N; i++){
		A->ptr[i+1]+=A->ptr[i]; /* calculate row ptr with the saved offset */
	}

	//init to avoid ccNUMA Probs
  #pragma omp parallel for private(i,j) schedule(runtime)
  for(i=0; i<N; i++){
    for(j=A->ptr[i]; j<A->ptr[i+1]; j++){
			A->value[j] = 0.0;
			A->index[j] = 0.0;
		}
	}

		for (j=0; j<nnz; j++){
				//TODO: need the cols of one row to be sorted?
				temp=A->ptr[I[j]];
				A->index[temp+offset[temp]]=J[j];
				A->value[temp+offset[temp]]=V[j];
				offset[temp]++;
			//all values inserted for this line?
			//if(hit==A->ptr[i+1]-A->ptr[i]){
				//break;
			//}
		}

	//COUT(("Breaks: %d\n",temp));
	COUT("MM Parse done.\n");

	free(offset);
	free(I);
	free(J);
	free(V);
	fclose(f);
}

void destroyMatrix(struct MatrixCRS* A){
	free(A->value);
	free(A->index);
	free(A->ptr);
}

void printVector(const floatType* x, const int n){
	int i;
	printf("(");
	for(i = 0; i < n; i++){
		printf("%d:%e' ",i,x[i]);
	}
	printf(")\n");
}

void printMatrix(const struct MatrixCRS* A){
	int i;
	int rowIndex=0;
  for(i=0; i< A->nnz; i++){
    if (i==0)
      printf("Row %d: [", 0);
    // We have reached the next row if the actual iteration
    // is bigger than the next row pointer
    if( i >= A->ptr[rowIndex+1] ){
      rowIndex++;
      printf("]\nRow %d: [", rowIndex);
    }
    printf("%d:", A->index[i]);
    printf("%f' ",A->value[i]);
  }
  printf("]\n");

}


