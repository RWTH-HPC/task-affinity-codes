// =======================================
// This tool generates a symmetrix laplace
// matrix in matrix market format.
//
// Author: Tim Cramer, RWTH Aachen
// =======================================


#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>
#include <stdlib.h>

// This n results in matix with size of 6 GB (with 5 entries per row)
//int n = 100000000;
int n = 1024000;
int nnz = 0;

void addNNZ(std::ofstream& out, int x, int x_dist, int val){
	const std::string space = "      ";
	int y = x + x_dist;
	if(y > 0 && y <= n) {
		out << x << " " << y << space << val << std::endl;
		nnz++;
	}

	return;
}

int main(){
	std::string fname = "./out.mtx";
	std::ofstream out;
	out.open(fname.c_str());

	// the map defines for the value (double) for the
	// the entry on the a position relative to the entry
	// on the main diagonale. 
	std::map<int,double> indexMap;
	std::map<int,double>::iterator it;

	indexMap[-2000]  = -1.0;
	indexMap[   -1]  = -1.0;
	indexMap[    0]  =  4.0;
	indexMap[    1]  = -1.0;
	indexMap[ 2000]  = -1.0;

	assert(n>2000);

	// compute expected NNZ
	int nnzExpected = n * indexMap.size();
	for(it = indexMap.begin(); it != indexMap.end(); it++){
		nnzExpected -= abs(it->first);
	}

	// Write matrix market header
	out << "%%MatrixMarket matrix coordinate real general" << std::endl;
	out << n << " " << n << " " << nnzExpected << std::endl;

	for(int i=1; i<=n; i++) {
		for(it = indexMap.begin(); it != indexMap.end(); it++){
			addNNZ(out,i,it->first,it->second);
		}
	}

	out.close();

	std::cout << "Matrix (nnz=" << nnz << ", n=" << n 
	          << ") written to " << fname << std::endl;

	assert(nnz==nnzExpected);

	return 0;
}
