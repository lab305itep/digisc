#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Load(double d[50][64][2], char *fname)
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

void Save(double d[50][64][2], char *fname)
{
	int i, j;
	FILE *f;
	
	f = fopen(fname, "wt");
	if (!f) return;
	for (i=0; i<50; i++) for (j=0; j<64; j++) if (d[i][j][1] > 0)
		fprintf(f, "Channel=%2.2d.%2.2d  DT=%6.1f\n", i, j, d[i][j][0]);
	fclose(f);
}

int main(int argc, char **argv)
{
	int i, j, k;
	double A[50][64][2];
	double B[50][64][2];
	
	if (argc < 4) return 10;
	memset(A, 0, sizeof(A));
	memset(B, 0, sizeof(B));
	Load(A, argv[1]);
	Load(B, argv[2]);
//	printf("%f/%f %f/%f ", A[1][1][0], A[1][1][1], B[1][1][0], B[1][1][1]);
	for (i=0; i<50; i++) for (j=0; j<64; j++) {
		A[i][j][0] += B[i][j][0];
		A[i][j][1] += B[i][j][1];
	}
//	printf("%f\n", A[1][1][0]);	
	Save(A, argv[3]);
	return 0;
}
