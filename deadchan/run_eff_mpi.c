#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

#define TMPDIR "/home/clusters/rrcmpi/alekseev/igor/tmp/"

int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	time_t t0, t1;
	int irc;
	char *rootdir;
	char *pairdir;
	char *fuellist;
	char *vardir;
	int start_run = 0;
	
	t0 = time(NULL);
	
	if (argc > 1) start_run = strtol(argv[1], NULL, 10);
//		Get our run number
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	if (argc > 1) start_run = strtol(argv[1], NULL, 10);
	serial += start_run;
	
	rootdir = getenv("ROOT_DIR");
	if (!rootdir) rootdir = "/home/clusters/rrcmpi/alekseev/igor/root8n1/MC/Eff/";
	pairdir = getenv("PAIR_DIR");
	if (!pairdir) pairdir = "/home/clusters/rrcmpi/alekseev/igor/pair8n1/MC/Eff/";
	fuellist = getenv("FUEL_LIST");
	if (!fuellist) fuellist = "fuel.list";
	vardir = getenv("VAR_DIR");
	if (!vardir) vardir = "varlists/";
	
	sprintf(str, "%s/%d.list", vardir, serial);
	irc = access(str, R_OK);
	if (irc) goto fin;
	
	sprintf(str, "mkdir -p %s", rootdir);
	irc = system(str);
	if (irc) {
		printf("DIGI Run %d: mkdir %s error %d returned: %m\n", serial, rootdir, irc);
		goto fin;
	}

	sprintf(str, "mkdir -p %s", pairdir);
	irc = system(str);
	if (irc) {
		printf("DIGI Run %d: mkdir %s error %d returned: %m\n", serial, rootdir, irc);
		goto fin;
	}
	
	setenv("DANSSRAWREC_HOME", "../lib_v3.2", 1);
//			Run digi
	if (strcmp(fuellist, "none")) {
		sprintf(str, "../digi_evtbuilder6_v3 -no_hit_tables -file %s -output %s/var_%d_fuel.root -flag 0x860000 -mcdata -deadlist %s/%d.list", 
			fuellist, rootdir, serial, vardir, serial);
		irc = system(str);
		if (irc) {
			printf("DIGI Run %d: error %d returned: %m\n", serial, irc);
			goto fin;
		}
	}
//			Run pair
	sprintf(str, "../pairbuilder8 %s/var_%d_fuel.root %s/var_%d_fuel.root",
		rootdir, serial, pairdir, serial);
	irc = system(str);
	if (irc) {
		printf("PAIR Run %d: error %d returned: %m\n", serial, irc);
		goto fin;
	}
//			Plot Spectra
	sprintf(str, "root -b -l -q \"mcfuelspectra.C(\\\"%s/var_%d_fuel.root\\\", \\\"%s/var_%d_spfuel.root\\\")\"",
		pairdir, serial, vardir, serial);
	irc = system(str);
	if (irc) {
		printf("SPECTRA Run %d: error %d returned: %m\n", serial, irc);
		goto fin;
	}
//		time and print
	t1 = time(NULL);
	printf("Run %d: elapsed time %d s:\n", serial, t1 - t0);
fin:
	MPI_Finalize();
	return 0;
}
