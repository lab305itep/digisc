/*
    Change efficiency information in stat file.
    Position 22, starting with 1.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXRUN	200001
#define MAXSTR	1024

int main(int argc, char **argv)
{
	char str[MAXSTR];
	double eff[MAXRUN];
	double current;
	char *ptr;
	char *ptr22;
	char *ptr23;
	FILE *fStat;
	FILE *fEff;
	FILE *fOut;
	int from, to;
	int num;
	int cnt;
	int type;
	int i, j;
	int sp;
	double value;
	
	memset(eff, 0, sizeof(eff));
	if (argc < 4) {
		printf("Usage file_stat.txt file_eff.list file_out.txt\n");
		return 10;
	}
	
	fStat = fopen(argv[1], "rt");
	fEff = fopen(argv[2], "rt");
	fOut = fopen(argv[3], "wt");
	if (!fStat || !fEff || !fOut) {
		printf("Can not open all files: %s %s %s\n", argv[1], argv[2], argv[3]);
		return 20;
	}

//		Read efficiencies
	cnt = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fEff);
		if (!ptr) break;	// EOF
		if (strlen(str) < 1) continue;
		str[strlen(str)-1] = '\0';	// remove '\n'
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		if (!isdigit(ptr[0])) continue;		// this is a comment
		from = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		to = strtol(ptr, NULL, 10);
		if (from < 0 || to <= 0) continue;
		if (from > to) {
			printf("Very strtange from(%d) > to(%d).\n", from, to);
			continue;
		}
		if (to >= MAXRUN) {
			printf("MAXRUN = %d is too small ! (Run range %d-%d met in %s)\n", MAXRUN, from, to, argv[2]);
			continue;
		}
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		value = strtod(ptr, NULL);	// efficiency itself
		for (i=from; i<=to; i++) eff[i] = value;	// fill the range
		cnt++;
	}
	printf("%s: %d ranges found\n", argv[2], cnt);
//		Read stat and write the output
	cnt = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fStat);
		if (!ptr) break;	// EOF
		if (strlen(str) < 1) continue;
		str[strlen(str)-1] = '\0';
		num = strtol(str, &ptr, 10);
		if (num <= 0) {			// comment etc
			fprintf(fOut, "%s\n", str);
			continue;
		}
		type = strtol(ptr, &ptr, 10);
		if (!type) continue;	// no run
		if (num >= MAXRUN) {
			printf("MAXRUN = %d is too small ! (Run %d met in %s)\n", MAXRUN, num, argv[1]);
			continue;
		}
		if (!eff[num]) {
			printf("Run %d not found in %s\n", num, argv[2]);
			continue;
		}
		
		j = 0;
		sp = 1;
		ptr22 = NULL;
		ptr23 = NULL;
		for (i=0;str[i];i++) {
			if (!sp && (str[i] == ' ' || str[i] == '\t')) {
				j++;
				if (j == 21) ptr22 = &str[i];
				if (j == 22) ptr23 = &str[i];
			}
			sp = (str[i] == ' ' || str[i] == '\t') ? 1 : 0;
		}
		if (!ptr22 || !ptr23) {
			printf("Strange string found: %s\n", str);
		} else {
			*ptr22 = '\0';
			fprintf(fOut, "%s %7.5f%s\n", str, eff[num], ptr23);
			cnt++;
		}
	}

	printf("%s: %d runs written\n", argv[3], cnt);
	
	fclose(fStat);
	fclose(fEff);
	fclose(fOut);
}
