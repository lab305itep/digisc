#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - the first run number
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	char fin[1024];
	char fout[1024];
	int fnum;
	time_t t0, t1;
	int irc;
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 2) {
		printf("Usage %s run_number\n", argv[0]);
		goto fin;
	}
	fnum = strtol(argv[1], NULL, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	fnum += serial;
//		check that file exists
	sprintf(fin, "/home/clusters/rrcmpi/alekseev/igor/root6n/%3.3dxxx/danss_%6.6d.root", fnum/1000, fnum);
	sprintf(fout, "/home/clusters/rrcmpi/alekseev/igor/pair7n/%3.3dxxx/pair_%6.6d.root", fnum/1000, fnum);
	irc = access(fin, R_OK);
	if (irc) {
		printf("Run %6.6d not found at %s\n", fnum, fin);
		goto fin;
	}
//		The run itself
	sprintf(str, "/home/itep/alekseev/igor/pairbuilder7 %s %s", fin, fout);
	irc = system(str);
	if (irc) printf("Run %d: error %d returned: %m\n", fnum, irc);
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", fnum, t1 - t0);
fin:
	MPI_Finalize();
}
