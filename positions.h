const struct PositionsStruct {
	char name[32];
	int first;
	int last;
	int period;
	double bgnd;
} positions[] = {
#include "pos_apr21.h"
};
const char periods[][30] = {
	"April-June 16", 
	"October 16 - July 17", 
	"August 17 - March 18", 
	"May 18 - January 19", 
	"March 19 - July 20",
	"August 20 - April 21"
};
