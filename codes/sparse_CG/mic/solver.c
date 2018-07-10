#pragma offload_attribute(push,target(mic))
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#pragma offload_attribute(pop)
#include "solver.h"

#define ALLOC alloc_if(1)
#define FREE free_if(1)
#define RETAIN free_if(0)
#define REUSE alloc_if(0)

// ab <- a' * b
__attribute__((target(mic))) void vectorDot(const floatType* a, const floatType* b, const int n, floatType* ab){
	int i;
	floatType sum=0;
//TODO: Intel BUG? ("Cannot open message catalog liboffload.cat
//offload error: buffer destroy failed (error code 14)"
//FIXME: Issue number 682534
//#pragma offload target (mic) in(a, b:length(n)) out(ab : length(1))
#pragma omp parallel for private(i), reduction(+:sum)
	for(i=0; i<n; i++){
		sum += a[i]*b[i];
	}
	(*ab)=sum;
}

// y <- ax + y
__attribute__((target(mic))) void axpy(const floatType a, const floatType* x, const int n, floatType* y){
	int i;
//#pragma offload target (mic) in(x : length(n)) inout(y : length(n))
#pragma omp parallel for private(i)
	for(i=0; i<n; i++){
		y[i]=a*x[i]+y[i];
	}
}

// y <- x + ay
__attribute__((target(mic))) void xpay(const floatType* x, const floatType a, const int n, floatType* y){
	int i;
//#pragma offload target (mic) in(x : length(n)) inout(y : length(n))
#pragma omp parallel for private(i)
	for(i=0; i<n; i++){
		y[i]=x[i]+a*y[i];
	}
}

// y <- A*x
__attribute__((target(mic))) void matvec(const struct MatrixCRS* A, const floatType* x, floatType* y){
	int i,j, is, ie, j0;
	double y0;
	const int* index = A->index;
	const int* ptr = A->ptr;
	const floatType* value = A->value;
	const int np1=A->n+1;
//#pragma offload target (mic) in(A: length(1)) in(index, value : length(A->nnz)) in(ptr : length(np1)) in(x : length(A->n)) out(y: length(A->n) )
	#pragma omp parallel for private(i,j,is,ie,j0,y0) schedule(runtime)
	for(i=0; i<A->n; i++){
		y0=0;
		is=ptr[i];
		ie=ptr[i+1];
		for(j=is; j<ie; j++){
			j0=index[j];
			y0+=value[j]*x[j0];
		}
		y[i]=y0;
	}
}

//nrm <- ||x||_2
__attribute__((target(mic))) void nrm2(const floatType* x, const int n, floatType* nrm){
	int i;
	floatType sum=0;
//#pragma offload target (mic) in(x : length(n)) out(nrm : length(1))
#pragma omp parallel for private(i), reduction(+:sum)
	for(i=0; i<n; i++){
		sum+=(x[i]*x[i]);
	}
	*nrm=sqrt(sum);
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

	const int* index = A->index;
	const int* ptr = A->ptr;
	const floatType* value = A->value;
	const int np1=A->n+1;

	floatType* r, *p, *q;
	floatType alpha, beta, rho, rho_old, dot_pq, bnrm2;
	int iter, maxIter;
	int n, nnz;
	int i;
	double tolerance, residual;
	double timeMatvec_s;
	double timeMatvec=0;

	n=A->n;
	nnz=A->nnz;

	maxIter=sc->maxIter;
	tolerance=sc->tolerance;

	// allocate memory
	r=(floatType*)malloc(n*sizeof(floatType));
	p=(floatType*)malloc(n*sizeof(floatType));
	q=(floatType*)malloc(n*sizeof(floatType));//TODO: nocopy for this fields

//#pragma offload target (mic) in(A: length(1)) in(A->index, A->value : length(A->nnz)) in(A->ptr : length(np1)) in(b, x, r, p, q : length(A->n)) out(x: length(A->n) )
	{

	//p(0)    = b - Ax(0)
	timeMatvec_s=getWTime();
	#pragma offload target(mic) in(A: length(1)) in(A->value: length(nnz) ALLOC) in(x: length(n)) out(p: length(n))
	{
	matvec(A,x,p);
	}
	timeMatvec+=getWTime()-timeMatvec_s;

	xpay(b, -1.0, n, p);

	//calculate initial residuum
	nrm2(p,n,&bnrm2);
	bnrm2 = 1.0 /bnrm2;
	printf("bnrm %e\n", bnrm2);

	//r(0)    = p(0)
	//memcpy(r, p, n*sizeof(floatType));
	#pragma omp parallel for schedule(runtime)
	for(i = 0; i < n; i++)
		r[i] = p[i];

	//rho(0)    =  <r(0),r(0)>
	vectorDot(r,r,n,&rho);
	COUT1("rho_0=%e\n", rho);

	for(iter = 0; iter < maxIter; iter++){
		//q(k)      = A * p(k)
		timeMatvec_s=getWTime();
		matvec(A,p,q);
		timeMatvec+=getWTime()-timeMatvec_s;

		//dot_pq    = <p(k),q(k)>
		vectorDot(p, q, n, &dot_pq);

		//alpha     = rho(k) / dot_pq
		alpha = rho / dot_pq;

		//x(k+1)    = x(k) + alpha*p(k)
		axpy(alpha, p, n, x);

		//r(k+1)    = r(k) - alpha*q(k)
		axpy(-alpha, q, n, r);

		rho_old=rho;

		//rho(k+1)  = <r(k+1), r(k+1)>
		vectorDot(r,r,n,&rho);

   	//check convergence ||r(k+1)||_2 < eps
		//printVector(r,n);
		//printVector(x,n);
		//nrm2(r, n, &(sc->residual));
		residual = sqrt(rho) * bnrm2; /* I am not sure why to correct the residuum like this, but LIS does it */
		COUT2("res_%d=%e\n",iter+1, residual);
		if(residual <= tolerance)
			break;


		//beta      = rho(k+1) / rho(k)
		beta = rho / rho_old;

		//p(k+1)    = r(k+1) + beta*p(k)
		xpay(r, beta, n, p);
	}
	}//pragma offload end
	sc->residual = residual;
	sc->iter = iter;
	sc->timeMatvec = timeMatvec;

	//printf("x=");
	//printVector(x,n);
	//printf("r=");
	//printVector(r,n);


	free(r);
	free(p);
	free(q);
}
