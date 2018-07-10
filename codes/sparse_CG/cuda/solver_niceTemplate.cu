#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "solver.h"
#include "io.h"

// ab <- a' * b
void vectorDot(const floatType* a, const floatType* b, const int n, floatType* ab){
/*	int i;
	(*ab)=0;
	for(i=0; i<n; i++){
		*ab += a[i]*b[i];
	}
*/



}

// y <- ax + y
void axpy(const floatType a, const floatType* x, const int n, floatType* y){
	int i;
	for(i=0; i<n; i++){
		y[i]=a*x[i]+y[i];
	}
}

// y <- x + ay
__global__ void xpay(const floatType* x, const floatType a, const int n, floatType* y){
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if(i<n){
		y[i]=x[i]+a*y[i];
	}
}

//z <- ax + y
__global__ void axpyz(const floatType a, const floatType* x, const floatType* y, const int n, floatType* z){
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if(i<n){
		z[i]=a*x[i]+y[i];
	}
}



// y <- A*x
__global__ void matvec(const struct MatrixCRS A, const floatType* x, floatType* y){
	int j;
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if(i<A.n){
		y[i]=0;
		for(j=A.ptr[i]; j<A.ptr[i+1]; j++){
			y[i]+=A.value[j]*x[A.index[j]];
		}
	}
}//TODO: This approach is not really load balanced. Is there a better one?


template <NrmPhase>
__global__ void nrm2_kernel0(const floatType* x, const int n, floatType* x_out){
  extern __shared__ double sdata[];

  int tid=threadIdx.x;
  int i = blockIdx.x*blockDim.x+threadIdx.x;
	
  sdata[tid]= (i<n) ? x[i] :0;
  __syncthreads();

  for(unsigned int s=1; s<blockDim.x; s*=2){
    if((tid%(2*s)) == 0){
      sdata[tid]+=sdata[tid+s];
    }
  __syncthreads();
  }

  if(tid==0) x_out[blockIdx.x]= sdata[0];
}

// Specialization for the WarmUp phase
template <>
__global__ void nrm2_kernel0<WarmUp>(const floatType* x, const int n, floatType* x_out){
  extern __shared__ double sdata[];

  int tid=threadIdx.x;
  int i = blockIdx.x*blockDim.x+threadIdx.x;
	
  sdata[tid]= (i<n) ? (x[i]*x[i]) :0;
  __syncthreads();

  for(unsigned int s=1; s<blockDim.x; s*=2){
    if((tid%(2*s)) == 0){
      sdata[tid]+=sdata[tid+s];
    }
  __syncthreads();
  }

  if(tid==0) x_out[blockIdx.x]= sdata[0];
}

//nrm <- ||x||_2
void nrm2(const floatType* x_d, const int n, floatType* nrm){
	floatType* xOut_d;
	int threads;
  int blocks;
  int s;
  size_t smem;

	getKernelConfig(n, &blocks, &threads, &smem);

	//TODO: Is it better to to allocate the memory before this point?
	//Reasons: 1.confusing interface(device/host pointers)
	//2. do this only one time not Iter times
	CUCHK(cudaMalloc((void**) &xOut_d, blocks*sizeof(floatType)));
	
  nrm2_kernel0<WarmUp><<<blocks,threads,smem>>>(x_d, n, xOut_d);
	CUCHK(cudaGetLastError());

  s=blocks;
  while(s>1){
    getKernelConfig(s, &blocks, &threads, &smem);
    nrm2_kernel0<Reduce><<<s,threads,smem>>>(xOut_d, s, xOut_d);
		CUCHK(cudaGetLastError());
    s= (s+threads-1)/(threads);
  }

	cudaMemcpy(nrm, xOut_d, sizeof(floatType),cudaMemcpyDeviceToHost); //TODO: really copy here?
	*nrm=sqrt(*nrm);

	CUCHK(cudaFree(xOut_d));
}


//nrm <- ||x||_inf
void nrmInf(const floatType* x, const int n, floatType* nrm){
	int i;
	floatType temp;

	*nrm=0;
	for(i=0; i<n; i++){
		temp=fabs(x[i]);
		if(temp>(*nrm))
			(*nrm)=temp;
	}
}/*________ UNTESTED_____*/


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
	floatType/** r, *p,*/ *q;
	floatType alpha, beta, rho, rho_old, dot_pq, bnrm2;
	int iter,n, nnz;

	// The device pointer
	struct MatrixCRS A_d;
	floatType* p_d;
	floatType* r_d;
	floatType* x_d;
	floatType* b_d;
	
	n=A->n;
	nnz=A->nnz;
	A_d.n=n;
	A_d.nnz=nnz;	

	// allocate memory
	//r=(floatType*)malloc(n*sizeof(floatType));
	//p=(floatType*)malloc(n*sizeof(floatType));
	q=(floatType*)malloc(n*sizeof(floatType));

	// allocate device memory
	CUCHK(cudaMalloc((void**)&(A_d.value), sizeof(floatType)*nnz));
	CUCHK(cudaMalloc((void**)&(A_d.index), sizeof(int)*nnz));
	CUCHK(cudaMalloc((void**)&(A_d.ptr), sizeof(int)*(n+1)));
	CUCHK(cudaMalloc((void**)&(x_d), sizeof(floatType)*n));
	CUCHK(cudaMalloc((void**)&(p_d),sizeof(floatType)*n));
	CUCHK(cudaMalloc((void**)&(r_d),sizeof(floatType)*n));
	CUCHK(cudaMalloc((void**)&(b_d),sizeof(floatType)*n));

	// copy device memory
	CUCHK(cudaMemcpy(A_d.value, A->value,nnz*sizeof(floatType), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(A_d.index, A->index,nnz*sizeof(int), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(A_d.ptr, A->ptr,(n+1)*sizeof(int), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(x_d, x,n*sizeof(floatType), cudaMemcpyHostToDevice));
	CUCHK(cudaMemcpy(b_d, b,n*sizeof(floatType), cudaMemcpyHostToDevice));

	// thread configuration 
	dim3 dimBlock(BLOCK_DIM);//TODO: Do this in dependency of compute capability?
	dim3 dimGrid((n+dimBlock.x-1)/dimBlock.x);
	COUT2("Start %d threads in %d blocks\n", dimBlock.x, dimGrid.x);
	
	printf("SOLVING CG\n");
	printf("NNZ      : %8d\n", nnz);
	printf("N        : %8d\n", n);
	printf("MAX_ITER : %8d\n", sc->maxIter);
	printf("TOLERANCE: %8.0e\n", sc->tolerance);
	
	//p(0)    = b - Ax(0)
	matvec<<<dimGrid, dimBlock>>>(A_d,x_d,p_d);
	xpay<<<dimGrid, dimBlock>>>(b_d, -1.0, n, p_d);
	
	//calculate initial residuum
	nrm2(p_d,n,&bnrm2);
	bnrm2 = 1.0 / bnrm2;
	COUT1("bnrm %e\n", bnrm2);

	//r(0)    = p(0)
	CUCHK(cudaMemcpy(r_d, p_d, n*sizeof(floatType), CudaMemcpyDeviceToDevice));	


	//rho(0)    =  <r(0),r(0)>
	vectorDot(r,r,n,&rho);

	/*for(iter = 0; iter < sc->maxIter; iter++){
		//q(k)      = A * p(k)
		//matvec(A,p,q);

		//dot_pq    = <p(k),q(k)>
		vectorDot(p, q, n, &dot_pq);

		//alpha     = rho(k) / dot_pq
		alpha = rho / dot_pq;

		//x(k+1)    = x(k) + alpha*p(k)
		axpy(alpha, p, n, x); 

		//r(k+1)    = r(k) - alpha*q(k)
		axpy(-alpha, q, n, r);

   	//check convergence ||r(k+1)||_2 < eps
		//printVector(r,n);
		//printVector(x,n);
		nrm2(r, n, &(sc->residual));
		sc->residual*=bnrm2; // I am not sure why to correct the residuum like this, but LIS does it 
		COUT2("res_%d=%e\n",iter+1, sc->residual);
		if(sc->residual <= sc->tolerance)
			break;

		rho_old=rho;

		//rho(k+1)  = <r(k+1), r(k+1)>
		vectorDot(r,r,n,&rho);
	
		//beta      = rho(k+1) / rho(k)
		beta = rho / rho_old;

		//p(k+1)    = r(k+1) + beta*p(k)
		xpay(r, beta, n, p);

	}	*/

	cudaMemcpy(x, p_d,n*sizeof(floatType), cudaMemcpyDeviceToHost);//TODO: CORRECT THIS TO X

	printf("RESIDUAL : %8.0e\n", sc->residual);
	printf("ITER     : %8d\n", iter);

	//printf("x=");	
	//printVector(x,n);	
	//printf("r=");	
	//printVector(r,n);	


	//free(r);
	//free(p);
	free(q);

	// clean up the device
	CUCHK(cudaFree(A_d.value));
	CUCHK(cudaFree(A_d.index));
	CUCHK(cudaFree(A_d.ptr));
	CUCHK(cudaFree(x_d));
	CUCHK(cudaFree(b_d));
	CUCHK(cudaFree(p_d));
	CUCHK(cudaFree(r_d));
}
