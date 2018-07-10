#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <cusparse.h>
#include <cublas.h>
#include "solver.h"
#include "io.h"


/***************************************
 *         Conjugate Gradient          *
 ***************************************
 p(0)    = b - Ax(0)
 r(0)    = p(0)
 rho(o)    =  <r(o),r(o)>                //TODO: need to be calculated in loop for preconditioning
 ***************************************
 for k=0,1,2,...,n-1
   q(k)      = A * p(k)                   O(n^2)
   dot_pq    = <p(k),q(k)>                O(n)
1  alpha     = rho(k) / dot_pq
2  x(k+1)    = x(k) + alpha*p(k)          O(n)
3  r(k+1)    = r(k) - alpha*q(k)          O(n)
   check convergence ||r(k+1)||_2 < eps   O(n)
	 rho(k+1)  = <r(k+1), r(k+1)>           O(n)
4  beta      = rho(k+1) / rho(k)
5  p(k+1)    = r(k+1) + beta*p(k)         O(n)
***************************************/

void cg(const struct MatrixCRS* A, const floatType* b, floatType* x, struct SolverConfig* sc){
	//floatType/** r, *p,*/ *q;
	floatType alpha, beta, rho, rho_old, dot_pq, bnrm2;
	int iter,n, nnz;
	double timeMatvec_s;
	double timeMatvec=0;

	// The device pointer
	struct MatrixCRS A_d;
	floatType* p_d;
	floatType* r_d;
	floatType* q_d;
	floatType* x_d;
	floatType* b_d;

	n=A->n;
	nnz=A->nnz;
	A_d.n=n;
	A_d.nnz=nnz;

	// allocate memory
	//r=(floatType*)malloc(n*sizeof(floatType));
	//p=(floatType*)malloc(n*sizeof(floatType));
	//q=(floatType*)malloc(n*sizeof(floatType));

	// allocate device memory
	CUCHK(cudaMalloc((void**)&(A_d.value), sizeof(floatType)*nnz));
	CUCHK(cudaMalloc((void**)&(A_d.index), sizeof(int)*nnz));
	CUCHK(cudaMalloc((void**)&(A_d.ptr), sizeof(int)*(n+1)));
	CUCHK(cudaMalloc((void**)&(x_d), sizeof(floatType)*n));
	CUCHK(cudaMalloc((void**)&(p_d),sizeof(floatType)*n));
	CUCHK(cudaMalloc((void**)&(r_d),sizeof(floatType)*n));
	CUCHK(cudaMalloc((void**)&(q_d),sizeof(floatType)*n));
	CUCHK(cudaMalloc((void**)&(b_d),sizeof(floatType)*n));

	// copy device memory
	CUCHK(cudaMemcpy(A_d.value, A->value,nnz*sizeof(floatType), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(A_d.index, A->index,nnz*sizeof(int), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(A_d.ptr, A->ptr,(n+1)*sizeof(int), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(x_d, x,n*sizeof(floatType), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(b_d, b,n*sizeof(floatType), cudaMemcpyHostToDevice));

	//cusparse warm up
	cusparseHandle_t handle = 0;
	cusparseStatus_t status;
	status = cusparseCreate(&handle);
	if(status != CUSPARSE_STATUS_SUCCESS){
		printf("ERROR: cusparse initialization failed\n");
		exit(1);
	}

	cusparseMatDescr_t descr = 0;
	status = cusparseCreateMatDescr(&descr);
	if(status != CUSPARSE_STATUS_SUCCESS){
		printf("ERROR: cusparse CreateMatDescr failed\n");
		exit(1);
	}

	//p(0)    = b - Ax(0)
	timeMatvec_s=getWTime();
	cusparseDcsrmv(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, n, n, -1.0, descr, A_d.value, A_d.ptr, A_d.index, x_d, 0.0, p_d);
	timeMatvec+=getWTime()-timeMatvec_s;
	cublasDaxpy(n, 1.0, b_d, 1, p_d, 1);

	//calculate initial residuum
	bnrm2 = 1.0 / cublasDnrm2(n, p_d, 1);
	COUT1("bnrm %e\n", bnrm2);

	//r(0)    = p(0)
	CUCHK(cudaMemcpy(r_d, p_d, n*sizeof(floatType), cudaMemcpyDeviceToDevice));


	//rho(0)    =  <r(0),r(0)>
	//vectorDot(r_d,r_d,n,&rho);
	rho = cublasDdot(n, r_d, 1, r_d, 1);

	COUT1("rho_0=%e\n", rho);

	for(iter = 0; iter < sc->maxIter; iter++){
		//q(k)      = A * p(k)
		timeMatvec_s=getWTime();
		cusparseDcsrmv(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, n, n, 1.0, descr, A_d.value, A_d.ptr, A_d.index, p_d, 0.0, q_d);
		timeMatvec+=getWTime()-timeMatvec_s;

		//dot_pq    = <p(k),q(k)>
		dot_pq = cublasDdot(n, p_d,1, q_d, 1);

		//alpha     = rho(k) / dot_pq
		alpha = rho / dot_pq;

		//x(k+1)    = x(k) + alpha*p(k)
		cublasDaxpy(n, alpha, p_d, 1, x_d, 1);

		//r(k+1)    = r(k) - alpha*q(k)
		cublasDaxpy(n, -alpha, q_d, 1, r_d, 1);

   	//check convergence ||r(k+1)||_2 < eps
		//printVector(r,n);
		//printVector(x,n);
		//nrm2(r_d, n, &(sc->residual));
		sc->residual = cublasDnrm2(n, r_d, 1);

		sc->residual*=bnrm2; // I am not sure why to correct the residuum like this, but LIS does it
		COUT2("res_%d=%e\n",iter+1, sc->residual);
		if(sc->residual <= sc->tolerance)
			break;

		rho_old=rho;

		//rho(k+1)  = <r(k+1), r(k+1)>
		rho = cublasDdot(n, r_d, 1, r_d, 1);


		//beta      = rho(k+1) / rho(k)
		beta = rho / rho_old;

		//p(k+1)    = r(k+1) + beta*p(k)
		//TODO: Is there a better way to do this (in one function?)
		cublasDscal(n, beta, p_d, 1),
		cublasDaxpy(n, 1.0, r_d, 1, p_d, 1);
		//xpay<<<dimGrid, dimBlock>>>(r_d, beta, n, p_d);

	}

	cudaMemcpy(x, x_d,n*sizeof(floatType), cudaMemcpyDeviceToHost);

	//printf("x=");
	//printVector(x,n);
	//printf("r=");
	//printVector(r,n);

	sc->iter = iter;
	sc->timeMatvec = timeMatvec;

	//free(r);
	//free(p);
	//free(q);

	// clean up the device
	CUCHK(cudaFree(A_d.value));
	CUCHK(cudaFree(A_d.index));
	CUCHK(cudaFree(A_d.ptr));
	CUCHK(cudaFree(x_d));
	CUCHK(cudaFree(b_d));
	CUCHK(cudaFree(p_d));
	CUCHK(cudaFree(r_d));
	CUCHK(cudaFree(q_d));
}
