#include <cusp/hyb_matrix.h>
#include <cusp/io/matrix_market.h>
#include <cusp/krylov/cg.h>

#include "def.h"
#include "output.h"

// where to perform the computation
typedef cusp::device_memory MemorySpace;
//typedef cusp::host_memory MemorySpace;

// which floating point type to use
typedef double ValueType;

//TODO: Are there inteligent iterators?
/*void initLGS2(const cusp::csr_matrix<int, ValueType, MemorySpace>* A, cusp::array1d<ValueType, MemorySpace>& b){
	int i, j;
	for (i=0; i<A->num_rows; ++i){
		for(j=A->row_offsets[i]; j<A->row_offsets[i+1]; j++)
			b[i]+=A->values[j];
	}
}*/

void initLGS(const cusp::csr_matrix<int, ValueType, cusp::host_memory>* A, cusp::array1d<ValueType, cusp::host_memory>& b){
	int i, j;
	for (i=0; i<A->num_rows; ++i){
		for(j=A->row_offsets[i]; j<A->row_offsets[i+1]; j++)
			b[i]+=A->values[j];
	}

}

void initLGS(const cusp::csr_matrix<int, ValueType, MemorySpace>* A, cusp::array1d<ValueType, MemorySpace>& b){
	int i, j;
	for (i=0; i<A->num_rows; ++i){
		for(j=A->row_offsets[i]; j<A->row_offsets[i+1]; j++)
			b[i]+=A->values[j];
	}
}

int main(int argc, char** argv){
	double ioTime, solveTime, totalTime, initTime;

	init();

	totalTime=getWTime();

	// create an empty sparse matrix structure (CSR format)
	cusp::csr_matrix<int, ValueType, cusp::host_memory> A;

	// load a matrix stored in MatrixMarket format
	ioTime=getWTime();
	cusp::io::read_matrix_market_file(A, argv[1]);
	ioTime=getWTime()-ioTime;

	// allocate storage for solution (x) and right hand side (b)
	cusp::array1d<ValueType, cusp::host_memory> x(A.num_rows, 0);
	cusp::array1d<ValueType, cusp::host_memory> b(A.num_rows, 0);
	initTime=getWTime();
	initLGS(&A, b);
	initTime=getWTime()-initTime;

	//set device memory
	cusp::csr_matrix<int, ValueType, cusp::device_memory> A_d = A;
	cusp::array1d<ValueType, cusp::device_memory> x_d=x;
	cusp::array1d<ValueType, cusp::device_memory> b_d=b;


	// set stopping criteria:
	//  iteration_limit    = 100
	//  relative_tolerance = 1e-6
	cusp::verbose_monitor<ValueType> monitor(b, config.maxIter,
	    config.tolerance);

	// set preconditioner (identity)
	cusp::identity_operator<ValueType, MemorySpace> M(A.num_rows, A.num_rows);

	// solve the linear system A * x = b with the Conjugate Gradient method
	solveTime=getWTime();
	//cusp::krylov::cg(A, x, b, monitor, M);
	cusp::krylov::cg(A_d, x_d, b_d, monitor);
	solveTime=getWTime()-solveTime;

	totalTime=getWTime()-totalTime;

	output(
	    argv,
	    "IO time   ", 'f', ioTime,
	    "Init time ", 'f', initTime,
	    "Solve time", 'f', solveTime,
	    "Total time", 'f', totalTime,
	    (const char*)NULL
	);

	return 0;
}

