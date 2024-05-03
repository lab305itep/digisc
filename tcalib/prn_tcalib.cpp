/***
 *
 * Version:       2.0
 *
 * Package:       DANSS time calibration pass 4
 *
 * Description:  Make calibration file
 *
 ***/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/***********************	Definitions	****************************/
#define MYVERSION	"2.00"
#define MAXCHAN		6000
#define NBRK		37

using namespace std;

void Help(void)
{
	printf("\tDANSS time calibration pass 4. Version %s\n", MYVERSION);
	printf("Print calibration file to be used in the analysis\n");
	printf("\tUsage:\n");
	printf("\t./prn_tcalib run_dir\n");
}

void ReadCalib(const char *dirname, int run_first, int run_last, float *calib, int len)
{
	FILE *f;
	char name[1024];
	int irc;
	
	memset(calib, 0, len);
	sprintf(name, "%s/hist/calib_%6.6d_%6.6d.bin", dirname, run_first, run_last);
	irc = access(name, R_OK);
	if (irc) {
		printf("Can not access calib file %s: %m.\n", name);
		return;
	}
	f = fopen(name, "rb");
	if (!f) {
		printf("Can not open calib file %s: %m.\n", name);
		return;
	}
	fread(calib, 1, len, f);
	fclose(f);
}

int main(int argc, const char **argv)
{
	const int run_first = 2210;
	const int brk_points[NBRK] = 
		{ 5000,   6836,   7639,   9247,  11579,  15999,  17444,  19257,  25474,  26552, 
		 26958,  27865,  29522,  31117,  32184,  36999,  64045,  69764,  70770,  98999, 
		 99797, 100303, 100600, 102850, 104996, 105474, 105650, 105979, 106300, 107999, 
		109600, 110801, 113968, 134000, 135484, 151872, 155000};
	float Calib[NBRK][MAXCHAN];
	int i, j;
	int ranges[NBRK][2];
	char str[1024];
	const char *run_dir;
	
	if (argc < 2) {
		Help();
		return 100;
	}
	run_dir = argv[1];
	
	memset(Calib, 0, sizeof(Calib));
	
	for(i=0; i<NBRK; i++) {
		if (!i) {
			ranges[i][0] = run_first;
		} else {
			ranges[i][0] = brk_points[i-1]+1;
		}
		ranges[i][1] = brk_points[i];
	}
	
	for(i=0; i<NBRK; i++) ReadCalib(run_dir, ranges[i][0], ranges[i][1], Calib[i], sizeof(Calib[0]));
	
	sprintf(str, "%s/hist/calib_sorted_%6.6d_%6.6d.txt", run_dir, run_first, brk_points[NBRK-1]);
	FILE *fOut = fopen(str, "wt");
	if (!fOut) {
		printf("Can not open file %s for writing the result: %m.\n", str);
		return 10;
	}
	
	for (i=0; i<MAXCHAN; i++) {
		for(j=0; j<NBRK; j++) if (Calib[j][i]) break;
		if (j == NBRK) continue;
		fprintf(fOut, "%2.2d.%2.2d %6.6d %6.6d %6.2f\n", i/100, i%100, 0, run_first-1, 0);
		for(j=0; j<NBRK; j++) if (Calib[j][i]) {
			fprintf(fOut, "%2.2d.%2.2d %6.6d %6.6d %6.2f\n", i/100, i%100, 
				ranges[j][0], ranges[j][1], Calib[j][i]);
		}
	}
	fclose(fOut);
	
	return 0;
}
