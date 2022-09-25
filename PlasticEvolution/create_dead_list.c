#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	int bad[60][64];
	FILE *fIn;
	FILE *fOut;
	int chan;
	int adc;
	int irc;
	
	if (argc < 3) {
		printf("Usage: %s filein.txt fileout.txt\n", argv[0]);
		return 10;
	}
	fIn = fopen(argv[1], "rt");
	fOut = fopen(argv[2], "wt");
	if (!fIn || !fOut) {
		printf("Check file names %s %s, please", argv[1], argv[2]);
		return 20;
	}
	memset(bad, 0, sizeof(bad));
	
	for(;!feof(fIn);) {
		irc = fscanf(fIn, "%d.%d", &adc, &chan);
		if (irc == 2) {
			bad[adc][chan] = 1;
		}
	}
	
	for(adc=0; adc<60; adc++) for(chan=0; chan<64; chan++) if (bad[adc][chan]) fprintf(fOut, "%2d.%2.2d\n", adc, chan);

	fclose(fIn);
	fclose(fOut);
}
