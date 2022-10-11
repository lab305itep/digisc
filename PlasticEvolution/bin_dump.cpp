#include <stdio.h>

int main(int argc, char **argv)
{
	int irc, cnt;
	struct HeadStruct {
		int adc;
		int chan;
		int num;
		int run_begin;
		int run_step;
		int run_end;
	} head;

	if (argc < 2) {
		printf("Usage: %s fname.bin\n", argv[0]);
		return 10;
	}
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		printf("Can not open the file %s\n", argv[1]);
		return 30;
	}

	cnt = 0;
	for (;;) {
		irc = fread(&head, sizeof(head), 1, f);
		if (irc != 1) break;
		printf("%2d.%2.2d: %d entries (begin: %d end: %d step: %d\n", 
			head.adc, head.chan, head.num, head.run_begin, head.run_step, head.run_end);
		irc = fseek(f, head.num * sizeof(double), SEEK_CUR);
		if (irc) {
			printf("Unexpected file read error %m\n");
			break;
		}
		cnt++;
	}
	fclose(f);
	printf("%d channels\n", cnt);

	return 0;
}

