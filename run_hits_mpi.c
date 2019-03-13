#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

//	argv[1] - the first run number
//	argv[2] - runs per process
//	We will process runs argv[1] + k*argv[2], ..., argv[1] + (k+1)*argv[2] - 1
int main(int argc, char *argv[]) 
{
	int serial;
	char str[4096];
	int fnum, pnum;
	time_t t0, t1;
	int irc;
	int i;
	char *ptr;
	const char *src_dir = "/home/clusters/rrcmpi/alekseev/igor/root6n1";
	const char *tgt_dir = "/home/clusters/rrcmpi/alekseev/igor/root6n2";
	const char *hits_dir = "/home/clusters/rrcmpi/alekseev/igor/hitcheck";
	
	t0 = time(NULL);

	ptr = getenv("SRC_DIR");
	if (ptr) src_dir = ptr;
	ptr = getenv("TGT_DIR");
	if (ptr) tgt_dir = ptr;
	ptr = getenv("HITS_DIR");
	if (ptr) hits_dir = ptr;

//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 3) {
		printf("Usage %s first_run_number runs_per_process\n", argv[0]);
		goto fin;
	}
	fnum = strtol(argv[1], NULL, 0);
	pnum = strtol(argv[2], NULL, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	fnum += serial*pnum;
//		The run itself
	for (i = 0; i<pnum; i++) {
//	ROOTNAME=`printf "%3.3dxxx/danss_%6.6d.root" $j $i`
//	HITNAME=`printf "%3.3dxxx/hits_%6.6d.txt.bz2" $j $i`
//	if [ -f $SRCDIR/$ROOTNAME ] ; then

		sprintf(str, "%s/%3.3dxxx/danss_%6.6d.root", src_dir, (fnum + i)/1000, fnum+i);
		if (access(str, R_OK)) {
			printf("Run %d: file %s not found\n", fnum+i, str);
			continue;
		}
//		Copy
//		cp --remove-destination $SRCDIR/$ROOTNAME $TGTDIR/$ROOTNAME 
		sprintf(str, "cp --remove-destination %s/%3.3dxxx/danss_%6.6d.root %s/%3.3dxxx/danss_%6.6d.root",
			src_dir, (fnum+i)/1000, fnum+i, tgt_dir, (fnum+i)/1000, fnum+i);
		irc = system(str);
		if (irc) {
			printf("Run %d: copy error [%s]: %m\n", fnum+i, str);
			continue;
		}
//		./hittree $TGTDIR/$ROOTNAME $HITDIR/$HITNAME
		sprintf(str, "./hittree %s/%3.3dxxx/danss_%6.6d.root %s/%3.3dxxx/hits_%6.6d.txt.bz2",
			tgt_dir, (fnum+i)/1000, fnum+i, hits_dir, (fnum+i)/1000, fnum+i);
			irc = system(str);
		if (irc) {
			printf("Run %d: processing error [%s]: %m\n", fnum+i, str);
			continue;
		}
	}
//		time and print
	t1 = time(NULL);
	printf("Runs %d-%d: elapsed time %d s:\n", fnum, fnum+pnum-1, t1 - t0);
fin:
	MPI_Finalize();
}
