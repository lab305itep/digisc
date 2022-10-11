/****************************************************************
 * Find common correction coefficient				*
 * k - run index						*
 * i - strip number						*
 * j - event number						*
 * s[k][i][j] - event energy deposit in a strip			*
 * r[k][i] - strip relative phe size deviation			*
 * M - expected average						*
 * chi^2 = SUM (s[k][i][j]*(1+alpha*r[k][i]) - M)^2		*
 * S = <s[k][i][j]>						*
 * SR = <s[k][i][j] * r[k][i]>					*
 * S2R = <s[k][i][j]^2 * r[k][i]>				*
 * S2R2 = <s[k][i][j]^2 * r[k][i]^2>				*
 * alpha = (S * SR - S2R) / (S2R2 - SR^2)			*
 ****************************************************************
 * Part B: calculate alpha					*
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	double sumS, sumSR, sumS2R, sumS2R2;
	int cnt;
	int run_begin, run_end;
	int run, num;
	double S, SR, S2R, S2R2;
	double alpha;
	FILE *fIn;
	
	if (argc < 3) {
		printf("Usage: %s run_begin run_end\n", argv[0]);
		return 10;
	}
	run_begin = strtol(argv[1], NULL, 10);
	run_end = strtol(argv[2], NULL, 10);
	fIn = fopen("/home/clusters/rrcmpi/alekseev/igor/dvert/corr/corr_all.txt", "rt");
	sumS = sumSR = sumS2R = sumS2R2 = cnt = 0;
	for(;!feof(fIn);) {
		fscanf(fIn, "%d: %d %f %f %f %f", &run, &num, &S, &SR, &S2R, &S2R2);
		if (run < run_begin) continue;
		if (run > run_end) break;
		sumS += num * S;
		sumSR += num * SR;
		sumS2R += num * S2R;
		sumS2R2 += num * S2R2;
		cnt += num;
	}
	if (cnt > 0) {
		sumS /= cnt;
		sumSR /= cnt;
		sumS2R /= cnt;
		sumS2R2 /= cnt;
		alpha = (sumS * sumSR - sumS2R) / (sumS2R2 - sumSR * sumSR);
	} else {
		alpha = 0;
	}
	fclose(fIn);
	printf("Runs %d-%d   alpha = %12.5f\n", run_begin, run_end, alpha);
}
