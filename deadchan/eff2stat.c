#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXRUN	100000
#define MAXSTR	1024

int main(int argc, char **argv)
{
	char str[MAXSTR];
	double eff[MAXRUN];
	char *ptr;
	FILE *fEff;
	FILE *fIn;
	FILE *fOut;
	int num;
	int type;
	int cnt;
	double current;
	
	memset(eff, 0, sizeof(eff));
	if (argc < 4) {
		printf("Usage %s file_eff.txt file_stat.txt file_out.txt\n", argv[0]);
		return 10;
	}
	
	fEff = fopen(argv[1], "rt");
	fIn = fopen(argv[2], "rt");
	fOut = fopen(argv[3], "wt");
	if (!fEff || !fIn || !fOut) {
		printf("Can not open all files: %s %s %s\n", argv[1], argv[2], argv[3]);
		return 20;
	}

//		Read efficiencies
	cnt = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fEff);
		if (!ptr) break;	// EOF
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		num = strtol(ptr, NULL, 10);
		if (num <= 0) continue;
		if (num >= MAXRUN) {
			printf("MAXRUN = %d is too small ! (Run %d met in %s)\n", MAXRUN, num, argv[1]);
			continue;
		}
		strtok(NULL, " \t");
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		eff[num] = strtod(ptr, NULL);
		cnt++;
	}
	printf("%s: %d runs found\n", argv[1], cnt);
//		Fill gaps
	current = 0;
	for (num = 0; num < MAXRUN; num++) if (eff[num] == 0) {
		eff[num] = current;
	} else {
		current = eff[num];
	}
//		Read stat and write the output
	cnt = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fIn);
		if (!ptr) break;	// EOF
		if (strlen(str) < 1) continue;
		str[strlen(str)-1] = '\0';
		num = strtol(str, &ptr, 10);
		if (num <= 0) {			// comment etc
			fprintf(fOut, "%s\n", str);
			continue;
		}
		if (num >= MAXRUN) {
			printf("MAXRUN = %d is too small ! (Run %d met in %s)\n", MAXRUN, num, argv[2]);
			continue;
		}
		fprintf(fOut, "%s %7.5f\n", str, eff[num]);
		cnt++;
	}

	printf("%s: %d runs written\n", argv[3], cnt);
	
	fclose(fEff);
	fclose(fIn);
	fclose(fOut);
}
