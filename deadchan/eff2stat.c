#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXEFF	100
#define MAXSTR	4096

int main(int argc, char **argv)
{
	char str[MAXSTR];
	struct EffStruct {
		double eff;
		int from;
		int to;
	} Eff[MAXEFF];
	char *ptr;
	FILE *fEff;
	FILE *fIn;
	FILE *fOut;
	int num;
	int type;
	int cnt, cntr, i;
	double current;
	
	memset(Eff, 0, sizeof(Eff));
	if (argc < 3) {
		printf("Usage %s file_eff.txt file_stat.txt file_out.txt\n", argv[0]);
		return 10;
	}
	
	fEff = fopen(argv[1], "rt");
	fIn = fopen(argv[2], "rt");
	fOut = fopen(argv[3], "wt");
	if (!fEff || !fIn || !fOut) {
		printf("We need to open all files: eff: %s  stat: %s  out: %s\n", argv[1], argv[2], argv[3]);
		return 20;
	}

//		Read ranges and efficiencies
	cnt = 0;
	for (;;) {
		ptr = fgets(str, sizeof(str), fEff);
		if (!ptr) break;	// EOF
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		Eff[cnt].from = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		Eff[cnt].to = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		Eff[cnt].eff = strtod(ptr, NULL);
		cnt++;
		if (cnt >= MAXEFF) break;	// too many entries
	}
	printf("%s: %d efficiency ranges found\n", argv[1], cnt);
	for (i=0; i<cnt; i++) {
		printf("%d - %d: %f\n", Eff[i].from, Eff[i].to, Eff[i].eff);
	}
	printf("Press <enter>\n");
	getchar();
	
//		Read stat and write the output
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
		for (i=0; i<cnt; i++) if (num >= Eff[i].from && num <= Eff[i].to) break;
		if (i == cnt) {
			printf("Efficiency range for run %d not found\n", num);
			continue;
		}
		fprintf(fOut, "%s %7.5f\n", str, Eff[i].eff);
		cnt++;
	}

	printf("%s: %d runs written\n", argv[3], cnt);
	
	fclose(fEff);
	fclose(fIn);
	fclose(fOut);
}
