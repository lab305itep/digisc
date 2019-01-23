#include <stdio.h>
#include <string.h>

int main(int argc, char ** argv)
{
	FILE *f;
	char str[0x10000];
	int NCnt, SCnt, JCnt;
	char *ptr;
	int flag;
	
	if (argc < 2) {
		printf("Usage: %s <keyword>\n", argv[0]);
		return -100;
	}
	
	f = popen("showq", "r");
	if (!f) {
		printf("Fatal error: %m\n");
		return 100;
	}
	
	NCnt = SCnt = JCnt = 0;
	flag = 0;
	
	for (;;) {
		if (!fgets(str, sizeof(str), f)) break;
//		printf("%s\n", str);
		ptr = strstr(str, "JOBS-");
		if (ptr) {
			flag++;
			continue;
		}
		if (flag != 2) continue;
		ptr = strstr(str, argv[1]);
		if (ptr) {
			SCnt++;
		} else if (SCnt) {
			JCnt++;
		} else {
			NCnt++;
		}
	}
	pclose(f);
	
	printf("%d before %d jobs, %d after\n", NCnt, SCnt, JCnt);
	
	return 0;
}
