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

	t0 = time(NULL);
	
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	
	sprintf(str, "./process_flat.sh %d %d %c", serial/25, ((serial/5) %5), 'A' + (serial % 5));
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
