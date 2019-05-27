#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv)
{
	FILE *f;
	char str[0x10000];
	int NCnt, SCnt, JCnt;
	char *ptr;
	
	f = popen("pbsnodes", "r");
	if (!f) {
		printf("Fatal error: %m\n");
		return 100;
	}
	
	NCnt = SCnt = JCnt = 0;
	
	for (;;) {
		if (!fgets(str, sizeof(str), f)) break;
//		printf("%s\n", str);
		ptr = strstr(str, "sessions=");
		if (ptr) {
			ptr += strlen("sessions=");
			SCnt += strtol(ptr, NULL, 0);
			NCnt++;
		} else {
			ptr = strstr(str, "jobs =");
			if (ptr) {
				for (;;) {
					ptr = strchr(ptr, '/');
					if (!ptr) break;
					ptr++;
					JCnt++;
				}
			}
		}
	}
	pclose(f);
	
	printf("%d sessions, %d jobs, %d nodes\n", SCnt, JCnt, NCnt);
	
	return 0;
}
