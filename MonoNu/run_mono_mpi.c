#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	time_t t0, t1;
	int irc;
	char *myarg;
	
	t0 = time(NULL);
	
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	
	sprintf(str, "MONO_ARG%2.2d", serial);
	myarg = getenv(str);
	if (!myarg) goto fin;
	
	sprintf(str, "./do_flat_mc.sh %s", myarg);
	irc = system(str);
	if (irc) {
		printf("Run %d: error %d returned: %m\n", serial, irc);
		goto fin;
	}
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", serial, t1 - t0);
fin:
	MPI_Finalize();
	return 0;
}
