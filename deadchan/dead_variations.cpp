#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXDIFF	5
#define MAXVAR	200
#define MAXSTR	4096
#define VECLEN	3200

class DetVector {
private:
	char array[VECLEN];
public:
	DetVector(char *str = NULL, int run = 0);
	int Diff(DetVector *vec);
	void Dump(const char *fname);
};

DetVector::DetVector(char *str, int run)
{
	char *tok;
	int mod, chan, num;
	memset(array, 0, sizeof(array));
	if (str != NULL) {
		tok = strtok(str, " \t");
		tok = strtok(NULL, " \t");
		for (;;) {
			tok = strtok(NULL, " \t");
			if (!tok) break;
			if (tok[2] != '.') {
				printf("Run %d: Format error: channel = %s\n", run, tok);
				continue;
			}
			mod = 10*(tok[0]-'0') + tok[1] - '0';
			chan = 10*(tok[3]-'0') + tok[4] - '0';
			num = 64*mod + chan;
			if (num < 0 || num >= VECLEN) {
				printf("Run %d: Channel %s = %d is out of range [0 - %d]\n", run, tok, num, VECLEN-1);
				continue;
			}
			array[num] = 1;
		}
	}
}

int DetVector::Diff(DetVector *vec)
{
	int i;
	int res;
	
	res = 0;
	for (i=0; i<VECLEN; i++) if (array[i] != vec->array[i]) res++;
	return res;
}

void DetVector::Dump(const char *fname)
{
	int i;
	FILE *f = fopen(fname, "wt");
	if (!f) {
		printf("Can not open file %s: %m\n", fname);
		return;
	}
	for (i=0; i<VECLEN; i++) if (array[i]) fprintf(f ,"%2.2d.%2.2d\n", i / 64, i % 64);
	fclose(f);
}

int main(int argc, char **argv)
{
	char buf[MAXSTR];
	char bcopy[MAXSTR];
	DetVector *voc[MAXVAR];
	DetVector *cur;
	FILE *fIn;
	FILE *fOut;
	int i;
	int fnum;
	char *ptr;
	int Vcnt;
	
	if (argc < 4) {
		printf("Usage: %s deadlist.txt runlist.txt varlists_dir\n", argv[0]);
		return 10;
	}
	
	fIn = fopen(argv[1], "rt");
	fOut = fopen(argv[2], "wt");
	
	if (!fIn || !fOut) {
		printf("Can not open files!\n");
		return 20;
	}
	
	Vcnt = 0;
	for (;;) {
		if (feof(fIn)) break;
		if (!fgets(buf, sizeof(buf), fIn)) break;
		if (!strlen(buf)) continue;
		fnum = strtol(buf, &ptr, 10);
		if (!fnum) continue;
		strcpy(bcopy, ptr);
		cur = new DetVector(bcopy, fnum);
		for (i=0; i<Vcnt; i++) if (cur->Diff(voc[i]) <= MAXDIFF) break;
		if (i == Vcnt) {
			if (Vcnt < MAXVAR) {
				voc[Vcnt] = cur;
				Vcnt++;
			} else {
				printf("Can not store configuration for file %d. Maximum of %d cells filled.\n", fnum, MAXVAR);
				break;
			}
		} else {
			delete cur;
		}
		fprintf(fOut, "%6d %4d\n", fnum, i);
	}
	
	sprintf(buf, "mkdir -p %s", argv[3]);
	i = system(buf);
	if (i) {
		printf("Can not create directory %s : %m\n", argv[3]);
	} else {
		for (i=0; i<Vcnt; i++) {
			sprintf(buf, "%s/%d.list", argv[3], i);
			voc[i]->Dump(buf);
		}
	}
	
	printf("%d variations found\n", Vcnt);
	
	fclose(fIn);
	fclose(fOut);
	
	return 0;
}
