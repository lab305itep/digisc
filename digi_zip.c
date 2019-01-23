#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

#define NF 500

//	argv[1] - the first digi file number
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	char fname[1024];
	int fnum;
	int irc;
	int i, fn;
	
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 2) {
		printf("Usage %s first_run_number\n", argv[0]);
		goto fin;
	}
	fn = strtol(argv[1], NULL, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	fn += serial*NF;
	for (i=0; i<NF; i++) {
//		check that file exists
	fnum = fn + i;
		sprintf(fname, "/home/clusters/rrcmpi/alekseev/igor/digi/v2.1/%3.3dxxx/danss_data_%6.6d_phys_rawrec.digi", fnum/1000, fnum);
		irc = access(fname, R_OK);
		if (irc) continue;
//		The run itself
		sprintf(str, "bzip2 -f %s", fname);
		system(str);
//		time and print
	}
fin:
	MPI_Finalize();
}
