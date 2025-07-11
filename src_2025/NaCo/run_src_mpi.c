#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

//	argv[1] root command pattern
//	Example: "draw_Sources6.C(1031, \"Full_decay_center_Chikuma\", %6.4f, 30)"
int main(int argc, char *argv[]) 
{
	int serial;
	char str[1024];
	char cmd[2048];
	int irc;
	double scale;
//		Get our run number
	MPI_Init(&argc, &argv);
	if (argc < 2) {
		printf("Usage %s \"root command pattern\"\n", argv[0]);
		goto fin;
	}
	MPI_Comm_rank(MPI_COMM_WORLD, &serial);
	scale  = 0.9 + 0.005*serial;
//		The run itself
	sprintf(str, argv[1], scale);
	sprintf(cmd, "root -p -q -b \'%s\'", str);
	irc = system(cmd);
	if (irc) printf("Scale %6.4f: processing error [%s]: %m\n", scale, cmd);
//		time and print
fin:
	MPI_Finalize();
}
