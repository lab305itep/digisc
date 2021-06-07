#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - top directory
//	We will process runs in /argv[1]/"serial"xxx/
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	char fname[4000];
	time_t t0, t1;
	int i, irc;
	int Cnt[2];
	
	t0 = time(NULL);

//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 2) {
		printf("Usage %s directory\n", argv[0]);
		goto fin;
	}
	
	memset(Cnt, 0, sizeof(Cnt));
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	sprintf(fname, "%s/%3.3dxxx", argv[1], serial);
	if (!access(fname, R_OK | X_OK)) {		// check if the directory exists
		printf("Checking: %s\n", fname);
//		The run itself
		for (i = 0; i<1000; i++) {
			sprintf(fname, "%s/%3.3dxxx/danss_data_%6.6d_phys.digi.bz2", argv[1], serial, serial * 1000 + i);
			if (!access(fname, R_OK)) {	// check if the file exists
				Cnt[0]++;
				sprintf(str, "bzip2 -t %s", fname);
				irc = system(str);
				if (irc) {
					Cnt[1]++;
					printf("%d - bad digi.\n", serial * 1000 + i);
				}
			}
		}
	} 
//		time and print
	t1 = time(NULL);
	printf("Runs %d-%d: %d bad out of %d elapsed time %d s:\n", serial*1000, serial*1000+999, Cnt[1], Cnt[0], t1 - t0);
fin:
	MPI_Finalize();
}
