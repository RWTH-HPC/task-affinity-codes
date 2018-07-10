#include <iostream>

#include "viennacl/compressed_matrix.hpp"
#include "viennacl/linalg/cg.hpp"
#include "viennacl/linalg/jacobi_precond.hpp"

#include "def.h"
#include "io.h"
#include "output.h"

typedef viennacl::compressed_matrix<floatType, 1> matrix_t;
typedef viennacl::vector<floatType, 1> vector_t;

const unsigned maxIter = 10000;
const double tolerance = 1e-6;

int
main(int argc, char *argv[])
{
	struct MatrixCRS mm;

	if (argc != 2) {
		std::cout << "Syntax: " << argv[0] << " file" << std::endl;
		return 1;
	}

	init();

	std::cout << "[DEBUG] Parsing matrix" << std::endl;

	double totalTime = getWTime();
	double ioTime = getWTime();

	parseMM(argv[1], &mm);

	ioTime = getWTime() - ioTime;

	matrix_t mat(mm.n, mm.n, mm.nnz);
	std::cout << "[DEBUG] Uploading matrix to GPU" << std::endl;
	mat.set((unsigned int*)mm.ptr, (unsigned int*)mm.index, mm.value, mm.n,
	    mm.nnz);

	std::cout << "[DEBUG] calculating b" << std::endl;
	std::vector<floatType> cpuvec(mm.n, 0);

	for (int i = 0; i < mm.n; i++)
		for (int j = mm.ptr[i]; j < mm.ptr[i + 1]; j++)
			cpuvec[i] += mm.value[j];

	std::cout << "[DEBUG] Uploading b to GPU" << std::endl;
	vector_t vec(mm.n);
	viennacl::copy(cpuvec.begin(), cpuvec.end(), vec.begin());

	std::cout << "[DEBUG] Solving" << std::endl;

	double solveTime = getWTime();

	viennacl::linalg::cg_tag tag(config.tolerance, config.maxIter);
	vector_t res = viennacl::linalg::solve(mat, vec, tag);

	std::vector<floatType> x(mm.n);
	viennacl::copy(res.begin(), res.end(), x.begin());
	solveTime = getWTime() - solveTime;

	std::cout << "(";
	for (int i = 0; i < x.size(); i++)
		std::cout << i << ":" << x[i] << "' ";
	std::cout << ")" << std::endl;

	write_result(&x[0], x.size());

	totalTime = getWTime() - totalTime;

	output(
	    argv,
	    "N", 'i', mm.n,
	    "NNZ", 'i', mm.nnz,
	    "Max. iterations", 'i', maxIter,
	    "Tolerance", 'e', tolerance,
	    "Residual", 'e', tag.error(),
	    "Iterations", 'i', tag.iters(),
	    "IO time", 'f', ioTime,
	    "Solve time", 'f', solveTime,
	    "Total time", 'f', totalTime,
	    (const char*)NULL
	);

	return 0;
}
