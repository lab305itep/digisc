#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	FILE *f;
	char str[1024];
	double sum;
	int N;
	int K;
	
	if (argc < 3) return 10;
	K = strtol(argv[1], NULL, 0);
	f = fopen(argv[2], "rt");
	if (!f) return 20;
	
	N = 0;
	sum = 0;
	for(;;) {
		if (!fgets(str, sizeof(str), f)) break;
		if (!isdigit(str[1])) continue;
		sum += strtod(&str[K], NULL);
		if (N < 3) printf("%s", &str[K]);
		N++;
	}
	
	fclose(f);
	printf("Average of %d entries = %f\n", N, sum / N);
	return 0;
}
