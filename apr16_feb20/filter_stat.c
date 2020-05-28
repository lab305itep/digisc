#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	FILE *fIn;
	FILE *fOut;
	char str[1024];
	char strcopy[1024];
	char *ptr, *sptr;
	int num;
	int type;
	int i;
	double fNoise;
	double fLimit;	// limit noise, Hz
	
	if (argc < 4) {
		printf("Usage: %s fLimit file_stst_in file_stat_out\n", argv[0]);
		return 10;
	}
	
	fLimit = strtod(argv[1], NULL);
	fIn = fopen(argv[2], "rt");
	fOut = fopen(argv[3], "wt");
	
	if (!fIn || !fOut) {
		printf("Can not open files.\n");
		return 20;
	}
	
	fprintf(fOut, "# Limit of %6.1f Hz applied to noise\n", fLimit);
	
	for(;;) {
		ptr = fgets(str, sizeof(str), fIn);
		if (!ptr || feof(fIn)) break;
		strcpy(strcopy, str);
		ptr = strtok(str, " \t");
		if (!ptr || !isdigit(ptr[0])) {	// this is a comment
			fprintf(fOut, strcopy);
			continue;
		}
		num = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Strange record %d\n", num);
			continue;
		}
		type = strtol(ptr, NULL, 10);
		if (type <= 0) {	// the run is already tagged out
			fprintf(fOut, strcopy);
			continue;
		}
		sptr = strtok(NULL, " \t");
		for (i=0; i<6; i++) ptr = strtok(NULL, " \t");
		if (!ptr) {
			printf("Strange record %d\n", num);
			continue;
		}
		fNoise = strtod(ptr, NULL);
		if (fNoise > fLimit) type = -type;
		sptr += strcopy - str;
		fprintf(fOut, "%6d %3d %s", num, type, sptr);
	}

	fclose(fIn);
	fclose(fOut);
	return 0;
}
