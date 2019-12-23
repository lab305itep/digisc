#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Load(double d[60][64][2], char *fname)
{
	int i, j;
	FILE *f;
	char str[256];
	
	f = fopen(fname, "rt");
	if (!f) {
		printf("File %s not found!\n", fname);
		return;
	}
	for (;;) {
		if (!fgets(str, sizeof(str), f)) break;
		i = atoi(&str[8]);
		j = atoi(&str[11]);
		d[i][j][0] = atof(&str[18]);
		d[i][j][1] = 1;
//		if (i==1 && j==1) printf("1.1=%f ", d[i][j][0]);
	}
	fclose(f);
}

int main(int argc, char **argv)
{
	int i, j, k;
	double thr;
	double A[60][64][2];
	double B[60][64][2];
	int Cnt[2];
	double Ssum;
	
	if (argc < 3) return 10;
	memset(A, 0, sizeof(A));
	memset(B, 0, sizeof(B));
	Load(A, argv[1]);
	Load(B, argv[2]);
	thr = 0.3;
	if (argc > 3) thr = strtod(argv[3], NULL);
	Cnt[0] = Cnt[1] = 0;
	Ssum = 0;
	for (i=0; i<60; i++) for (j=0; j<64; j++) if (A[i][j][1] && B[i][j][1]) {
		Cnt[0]++;
		Ssum += (A[i][j][0] - B[i][j][0]) * (A[i][j][0] - B[i][j][0]);
		if (fabs(A[i][j][0] - B[i][j][0]) > thr) {
			printf("Channel = %2.2d.%2.2d  difference = %f\n", i, j, A[i][j][0] - B[i][j][0]);
			Cnt[1]++;
		}
	}
	printf("Total: %d,   Errors: %d\n", Cnt[0], Cnt[1]);
	if (Cnt[0] > 100) printf("RMS = %f\n", sqrt(Ssum/Cnt[0]));
	return 0;
}
