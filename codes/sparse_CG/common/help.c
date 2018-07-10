#include <stdio.h>

void
help(const char *argv0)
{
#ifdef LIS
	printf("Usage: %s matrix [lis options]\n"
#else
	printf("Usage: %s matrix\n"
#endif
	    "\n"
	    "matrix can be either a matrix market or an mtxdump file.\n"
	    "mtxdump files can be generated using mtxdump.\n"
	    "\n"
	    "Environment variables:\n"
	    "\tCG_MAX_ITER\tmaximum number of iterations\n"
	    "\tCG_TOLERANCE\tallowed tolerance after which to stop\n"
	    "\tCG_OUTPUT_FILE\twrite result x in outputfile\n"
	    "\tCG_RHS\tThe right hand side of the equation\n"
#ifdef TASKING
	    "\tCG_NUM_TASKS\tnumber of tasks to use\n"
#endif
	    "\tCG_PRECON\tthe preconditioner to use\n"
	    "The defaults are:\n"
	    "\tCG_MAX_ITER\t1000\n"
	    "\tCG_TOLERANCE\t0.0000001\n"
#ifdef TASKING
	    "\tCG_NUM_TASKS\tnumber of processors\n"
#endif
	    "\n", argv0);
}
