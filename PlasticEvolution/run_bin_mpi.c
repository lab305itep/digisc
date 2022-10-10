#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

#define MINADC 2
#define PHEDIR "/home/clusters/rrcmpi/alekseev/igor/dvert/phe"

//	argv[1] - the adc number
int main(int argc, char *argv[]) 
{
	struct FileListStruct {
		const char *name;
		int begin;
		int end;
	} FileList[] = {
		{"phe_2000_12999.root",      2000,  12999},
		{"phe_13000_22999.root",    13000,  22999},
		{"phe_23000_32999.root",    23000,  32999},
		{"phe_33000_42999.root",    33000,  42999},
		{"phe_43000_52999.root",    43000,  52999},
		{"phe_53000_62999.root",    53000,  62999},
		{"phe_63000_82999.root",    63000,  82999},
		{"phe_83000_102999.root",   83000, 102999},
		{"phe_103000_121999.root", 103000, 121999}
	};
	int serial;
	int N = sizeof(FileList) / sizeof(FileList[0]);
	char str[4096];
	time_t t0, t1;
	int irc;
	int i;
	
	t0 = time(NULL);
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	//		Check if run from list is required and get its number
//		The run itself
	for (i=0; i<N; i++) {
		sprintf(str, "./make_rel_coef_bin %s/%s %d %d %d 50", 
			PHEDIR, FileList[i].name, serial + MINADC, FileList[i].begin, FileList[i].end);
		irc = system(str);
		if (irc) printf("Run \"%s\": error %d returned: %m\n", str, irc);
	}
//		time and print
	t1 = time(NULL);
	printf("Run adc %d: elapsed time %d s:\n", serial + MINADC, t1 - t0);
fin:
	MPI_Finalize();
}
