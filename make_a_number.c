#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - the first digi file number
//	Prints a number: argv[1] + serial
//	Prints "NaN" on error
int main(int argc, char *argv[]) 
{
	int serial;
	int fnum;
	
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 2) {
		printf("NaN\n");
		goto fin;
	}
	fnum = strtol(argv[1], NULL, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	fnum += serial;
	printf("%6.6d\n", fnum);
fin:
	MPI_Finalize();
}
