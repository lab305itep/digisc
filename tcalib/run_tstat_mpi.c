#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

// argv[1] - hist prefix
int main(int argc, char **argv)
{
	int serial;
	char str[4096];
	time_t t0, t1;
	int irc;
	const char *prefix = "hDT";
	
	t0 = time(NULL);
	
	if (argc > 1) prefix = argv[1];
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
//		The run itself
	sprintf(str, "root -l -b -q \"tstat.C(2210, 71210, \\\"%s%2.2dc\\\")\" > tstat_%s%2.2d.txt",
		prefix, serial + 1, prefix, serial + 1);
	irc = system(str);
	if (irc) printf("Run for module %d: error %d returned: %m\n", serial, irc);
//		time and print
	t1 = time(NULL);
	printf("Module %d: elapsed time %d s:\n", serial, t1 - t0);
fin:
	MPI_Finalize();
}
