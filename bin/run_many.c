#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	Run a command with many arguments in parrallel CPUs
//	argv[1] - command to run
//	argv[2...] - additional arguments to run in a separate CPUs
//	N-th CPU will be called with command argv[1] argv[N+2]
//	Extra CPUs will return immediately
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	time_t t0, t1;
	int irc;
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 3) {
		printf("Usage %s command argCPU0 argCPU1 ...\n", argv[0]);
		goto fin;
	}
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	if (serial > argc - 3) goto fin;	// nothing to do
//		The run itself
	sprintf(str, "%s %s", argv[1], argv[serial + 2]);
	irc = system(str);
	if (irc) printf("Run %d(%s): error %d returned: %m\n", serial, argv[serial+2], irc);
//		time and print
	t1 = time(NULL);
	printf("Run %d(%s): elapsed time %d s:\n", serial, argv[serial+2], t1 - t0);
fin:
	MPI_Finalize();
}
