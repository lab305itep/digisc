#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "positions.h"

#define MAXRUNS	100000

int check_date(const char *date, const char *posname)
{
	int len;
	len  = strlen(posname);
//	printf("[%c %c %c %c %c %c] [%c %c %c %c %c %c]\n",
//		date[0], date[1], date[3], date[4], date[8], date[9], 
//		posname[len-8], posname[len-7], posname[len-5], posname[len-4], posname[len-2], posname[len-1]);
	if (date[0] != posname[len-8]) return 1;
	if (date[1] != posname[len-7]) return 1;
	if (date[3] != posname[len-5]) return 1;
	if (date[4] != posname[len-4]) return 1;
	if (date[8] != posname[len-2]) return 1;
	if (date[9] != posname[len-1]) return 1;
	return 0;
}


void translate_date(char *outdate, const char *indate)
{
	outdate[0] = indate[8];
	outdate[1] = indate[9];
	outdate[2] = '.';
	outdate[3] = indate[5];
	outdate[4] = indate[6];
	outdate[5] = '.';
	outdate[6] = indate[0];
	outdate[7] = indate[1];
	outdate[8] = indate[2];
	outdate[9] = indate[3];
	outdate[10] = '\0';
}

int main()
{
	int i, N;
	FILE *fIn;
	FILE *fOut;
	struct StatStruct {
		int pos;
		char date[16];
		char begin[8];
		char end[8];
	} *statall;
	char str[1024];
	char *ptr;
	int num, first, last;
	char begin_date[16];
	char end_date[16];
	
	statall = (struct StatStruct *) malloc(MAXRUNS * sizeof(struct StatStruct));
	if (!statall) return 50;
	memset(statall, 0, MAXRUNS * sizeof(struct StatStruct));
	
	fIn = fopen("stat_all.txt", "rt");
	if (!fIn) return 100;
	for(;;) {
		if (!fgets(str, sizeof(str), fIn)) break;
		if (!strlen(str)) continue;
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		num = strtol(ptr, NULL, 10);
		if (num > 0 && num < MAXRUNS) {
			ptr = strtok(NULL, " \t");
			if (!ptr) continue;
			statall[num].pos = strtol(ptr, NULL, 10);
			if (!statall[num].pos) continue;
			ptr = strtok(NULL, " \t");
			if(!ptr) {
				printf("Strange record for run %d\n", num);
				statall[num].pos = 0;
				continue;
			}
			strncpy(statall[num].date, ptr, sizeof(statall[num].date));
			ptr = strtok(NULL, " \t");
			if(!ptr) {
				printf("Strange record for run %d\n", num);
				statall[num].pos = 0;
				continue;
			}
			strncpy(statall[num].begin, ptr, sizeof(statall[num].begin));
			ptr = strtok(NULL, " \t");
			if(!ptr) {
				printf("Strange record for run %d\n", num);
				statall[num].pos = 0;
				continue;
			}
			strncpy(statall[num].end, ptr, sizeof(statall[num].end));
		}
	}
	fclose(fIn);
	
	fOut = fopen("group_list.txt", "wt");
	if (!fOut) return 200;

	N = sizeof(positions) / sizeof(positions[0]);
	for (i=0; i<N; i++) {
		switch (positions[i].name[0]) {
		case 'd' : num = 2; break;
		case 'm' : num = 3; break;
		case 'u' : num = 4; break;
		case 's' : num = 5; break;
		case 'r' : num = 16; break;
		}
//		search for the first usable run
		for (first = positions[i].first; first <= positions[i].last; first++) 
			if (statall[first].pos == num) break;
		if (first > positions[i].last) {
			printf("No usable runs found for position %s: %d - %d\n",
				positions[i].name, positions[i].first, positions[i].last);
			continue;
		}
		for (last = positions[i].last; last >= positions[i].first; last--) 
			if (statall[last].pos == num) break;
		translate_date(begin_date, statall[first].date);
		translate_date(end_date, statall[last].date);
		if (check_date(begin_date, positions[i].name)) {
			printf("positions.h date [%s] does not match actual date [%s]\n",
				positions[i].name, begin_date);
			continue;
		}
		
		fprintf(fOut, "%c %s %s - %s %s -\n", positions[i].name[0], 
			begin_date, statall[first].begin, end_date, statall[last].end); 
	}
	fclose(fOut);
	return 0;
}
