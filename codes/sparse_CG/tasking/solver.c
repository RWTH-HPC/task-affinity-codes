#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include "solver.h"
#include "io.h"

#ifdef TASK_DISTRIBUTION
int NUMREMOTEPERFORM=0;
#ifndef PAR_PRODUCER
int *TASK_CONSUMERS;
#endif
#endif


// ab <- a' * b
void vectorDot(const floatType* a, const floatType* b, const int n, floatType* ab){
	int i;
	floatType sum;
	sum=0;
	#pragma omp parallel for private(i) reduction(+:sum)
	for(i=0; i<n; i++){
		sum += a[i]*b[i];
	}
	(*ab)=sum;

}

// y <- ax + y
void axpy(const floatType a, const floatType* x, const int n, floatType* y){
	int i;
	#pragma omp parallel for private(i)
	for(i=0; i<n; i++){
		y[i]=a*x[i]+y[i];
	}
}

// y <- x + ay
void xpay(const floatType* x, const floatType a, const int n, floatType* y){
	int i;
	#pragma omp parallel for private(i)
	for(i=0; i<n; i++){
		y[i]=x[i]+a*y[i];
	}
}

// y <- A*x
void
matvec(struct SolverConfig* sc, const struct MatrixCRS* A, const floatType* x,
    floatType* y, const int chunkSize)
{
	int i;
#ifdef TASK_DISTRIBUTION
	struct crtCsm {
		int creater;
		int consumer;
	};

	typedef struct crtCsm CC;
	CC *cc = (CC *)malloc(sizeof(CC) * sc->numTasks);

#pragma omp parallel for
	for (i = 0; i < sc->numTasks; i++) {
		cc[i].creater = 0;
		cc[i].consumer = 0;
	}
#endif
	int j, is, ie, j0, cs, ce;
	double y0;
	const int* index = A->index;
	const floatType* value = A->value;
	int chunkID;

	#pragma omp parallel private(chunkID)
	{
	int c;
#ifndef PAR_PRODUCER
	#pragma omp single
	{
#else
	#pragma omp for private(i,j,is,ie,j0,y0,c,cs,ce)
#endif
#ifdef DATA_DISTRIBUTION
	for(chunkID=0; chunkID<sc->numTasks; chunkID++){
		cs=A->blockptr[chunkID];
		ce=A->blockptr[chunkID+1];
#ifdef TASK_AFFINITY
      // get start position
      //int tmp = A->ptr[(int)((ce+cs)/2)];
      int tmp = A->ptr[ce];
      kmpc_set_task_affinity((void *)&value[tmp]);
      //kmpc_set_task_affinity((void *)&y[tmp]);
#endif
#else
	// generate all tasks
	for(c=0; c<A->n; c+=chunkSize){
	//for(c=A->n-1; c >=0; c-=chunkSize){
	  chunkID = c / chunkSize;
#ifdef TASK_AFFINITY
      // get start position
      //int tmp = A->ptr[c+(int)(chunkSize / 2)];
      int tmp = A->ptr[c];
      kmpc_set_task_affinity((void *)&value[tmp]);
      //kmpc_set_task_affinity((void *)&y[tmp]);
#endif
#endif
#ifdef TASK_DISTRIBUTION
		cc[chunkID].creater = omp_get_thread_num();
		#pragma omp task private(i,j,is,ie,j0,y0) firstprivate(c,cs,ce,chunkID) shared(A, index, value, x, y, cc) default(none)
		{
		cc[chunkID].consumer = omp_get_thread_num();
#else
		#pragma omp task private(i,j,is,ie,j0,y0) firstprivate(c,cs,ce,chunkID) shared(A, index, value, x, y) default(none)
		{
#endif
		//iterage the rows which belong to the chunk
#ifdef DATA_DISTRIBUTION
		for(i=cs; i<ce; i++){
#else
		for(i=c; i<c+chunkSize && i<A->n; i++){
#endif
			y0=0;
			is=A->ptr[i];
			ie=A->ptr[i+1];
			//#pragma vector always
			// iterate one row
			for(j=is; j<ie; j++){
				j0=index[j];
				y0+=value[j]*x[j0];
			}
			y[i]=y0;
		}
		}//task end
	}
	#ifndef PAR_PRODUCER
	}//single end
	#endif
	}//parallel end

#ifdef TASK_DISTRIBUTION
#ifndef PAR_PRODUCER
    for(i=0; i<sc->numTasks; i++) {
        TASK_CONSUMERS[cc[i].consumer] ++;
    }
#endif
    for(i=0; i<sc->numTasks; i++) {
        if(cc[i].creater != cc[i].consumer) {
            NUMREMOTEPERFORM++;
        }
    }
    free(cc);
#endif
}

//nrm <- ||x||_2
void nrm2(const floatType* x, const int n, floatType* nrm){
	int i;
	floatType sum=0;
	#pragma omp parallel for private(i) reduction(+:sum)
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
   q(k)      = A * p(k)
   dot_pq    = <p(k),q(k)>
1  alpha     = rho(k) / dot_pq
2  x(k+1)    = x(k) + alpha*p(k)
3  r(k+1)    = r(k) - alpha*q(k)
   check convergence ||r(k+1)||_2 < eps
	 rho(k+1)  = <r(k+1), r(k+1)>
4  beta      = rho(k+1) / rho(k)
5  p(k+1)    = r(k+1) + beta*p(k)
***************************************/

void
cg(const struct MatrixCRS* A, const floatType* b, floatType* x,
    struct SolverConfig* sc)
{
	floatType* r, *p, *q;
	floatType alpha, beta, rho, rho_old, dot_pq, bnrm2;
	int iter, n = A->n, nnz = A->nnz;
	double timeMatvec_s;
	double timeMatvec = 0;
	const int chunkSize = sc->chunkSize;

#ifdef TASK_AFF_DOMAIN_FIRST
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_first, kmp_task_aff_map_type_domain);
#endif
#ifdef TASK_AFF_DOMAIN_RAND
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_random, kmp_task_aff_map_type_domain);
#endif
#ifdef TASK_AFF_DOMAIN_LOWEST
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_lowest_wl, kmp_task_aff_map_type_domain);
#endif
#ifdef TASK_AFF_DOMAIN_PRIVATE
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_private, kmp_task_aff_map_type_domain);
#endif
#ifdef TASK_AFF_DOMAIN_RR
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_round_robin, kmp_task_aff_map_type_domain);
#endif
#ifdef TASK_AFF_THREAD_FIRST
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_first, kmp_task_aff_map_type_thread);
#endif
#ifdef TASK_AFF_THREAD_RAND
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_random, kmp_task_aff_map_type_thread);
#endif
#ifdef TASK_AFF_THREAD_LOWEST
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_lowest_wl, kmp_task_aff_map_type_thread);
#endif
#ifdef TASK_AFF_THREAD_RR
  kmpc_task_affinity_init(kmp_task_aff_init_thread_type_round_robin, kmp_task_aff_map_type_thread);
#endif

	// allocate memory
	r=(floatType*)malloc(n*sizeof(floatType));
	p=(floatType*)malloc(n*sizeof(floatType));
	q=(floatType*)malloc(n*sizeof(floatType));

	//TODO: Think about best first touch
	first_touch_vector(r, n, A);
	first_touch_vector(p, n, A);
	first_touch_vector(q, n, A);

#ifdef TASK_DISTRIBUTION
#ifndef PAR_PRODUCER
        int i, max_threads=omp_get_max_threads();
        TASK_CONSUMERS=(int*)malloc(max_threads*sizeof(int));
        for(i=0; i < max_threads; i++) {
            TASK_CONSUMERS[i] = 0;
        }
#endif
#endif

	//p(0)    = b - Ax(0)
	timeMatvec_s=getWTime();
	matvec(sc,A,p,q,chunkSize);
	timeMatvec+=getWTime()-timeMatvec_s;

	xpay(b, -1.0, n, p);

	//calculate initial residuum
	nrm2(p,n,&bnrm2);
	bnrm2 = 1.0 /bnrm2;
	printf("bnrm %e\n", bnrm2);

	//r(0)    = p(0)
	memcpy(r, p, n*sizeof(floatType));

	//rho(0)    =  <r(0),r(0)>
	vectorDot(r,r,n,&rho);
	COUT1("rho_0=%e\n", rho);

	for(iter = 0; iter < sc->maxIter; iter++){
		//q(k)      = A * p(k)
		timeMatvec_s=getWTime();
		matvec(sc,A,p,q,chunkSize);
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
		nrm2(r, n, &(sc->residual));
		sc->residual *= bnrm2; /* I am not sure why to correct the residuum like this, but LIS does it */
		COUT2("res_%d=%e\n",iter+1, sc->residual);
		if(sc->residual <= sc->tolerance)
			break;


		//beta      = rho(k+1) / rho(k)
		beta = rho / rho_old;

		//p(k+1)    = r(k+1) + beta*p(k)
		xpay(r, beta, n, p);

	}

	sc->iter = iter;
	sc->timeMatvec = timeMatvec;

#ifdef TASK_DISTRIBUTION
#ifndef PAR_PRODUCER
    double average=100/max_threads, max=0, min=100, variance;
    double *rate=(double *)malloc(sizeof(double) * max_threads);
    for(i=0; i<max_threads; i++){
        rate[i] = (double)(TASK_CONSUMERS[i]/(float)((iter+1)*sc->numTasks))*100;
        variance += pow(rate[i]-average, 2);

        if(rate[i]>max) {
            max=rate[i];
        }

        if(rate[i]<min) {
            min=rate[i];
        }

    }

    printf("Min Rate: %g, Max Rate: %g, VARIANCE: %g\n", min, max, variance);
    free(rate);
    free(TASK_CONSUMERS);
#endif
    printf("NUMBER OF REMOTE COMPUTING : %8d\n", NUMREMOTEPERFORM);
    printf("PERCENTAGE OF REMOTE COMPUTING: %f%%\n", (float)NUMREMOTEPERFORM/(float)((iter+1)*sc->numTasks)*100);
#endif
	//printf("x=");
	//printVector(x,n);
	//printf("r=");
	//printVector(r,10);


	free(r);
	free(p);
	free(q);
  printf("Elapsed time for program\t%lf\tsec\n",timeMatvec);
}
