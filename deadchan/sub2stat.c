/*
    Change efficiency information in stat file.
    Position 22, starting with 1.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXRUN	100000
#define MAXSTR	1024

int main(int argc, char **argv)
{
	char str[MAXSTR];
	double eff[MAXRUN];
	double current;
	char *ptr;
	FILE *fStat;
	FILE *fDead;
	FILE *fOut;
	int num;
	int cnt;
	int type;
	int i, j;
	int sp;
	
	memset(eff, 0, sizeof(eff));
	if (argc < 4) {
		printf("Usage file_stat.txt file_eff.txt file_out.txt\n");
		return 10;
	}
	
	fStat = fopen(argv[1], "rt");
	fDead = fopen(argv[2], "rt");
	fOut = fopen(argv[3], "wt");
	if (!fStat || !fDead || !fOut) {
		printf("Can not open all files: %s %s %s\n", argv[1], argv[2], argv[3]);
		return 20;
	}

//		Read dead times
	cnt = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fDead);
		if (!ptr) break;	// EOF
		if (strlen(str) < 1) continue;
		str[strlen(str)-1] = '\0';	// remove '\n'
		num = strtol(str, &ptr, 10);
		if (num <= 0) continue;
		if (num >= MAXRUN) {
			printf("MAXRUN = %d is too small ! (Run %d met in %s)\n", MAXRUN, num, argv[2]);
			continue;
		}
		i = strtol(ptr, &ptr, 10);	// dummy read - we don't need this number
		eff[num] = strtod(ptr, NULL);
		cnt++;
	}
	printf("%s: %d runs found\n", argv[2], cnt);
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
		for (i=0;str[i];i++) {
			if (!sp && (str[i] == ' ' || str[i] == '\t')) {
				j++;
				if (j == 21) str[i] = '\0';
				if (j == 22) {
					ptr = &str[i];
					break;
				}
			}
			sp = (str[i] == ' ' || str[i] == '\t') ? 1 : 0;
		}
		fprintf(fOut, "%s %7.5f%s\n", str, eff[num], ptr);
		cnt++;
	}

	printf("%s: %d runs written\n", argv[3], cnt);
	
	fclose(fStat);
	fclose(fDead);
	fclose(fOut);
}
