const struct PositionsStruct {
	char name[32];
	int first;
	int last;
	int period;
	double bgnd;
} positions[] = {
#include "positions/pos_2210_175528.h"
};
const char periods[][30] = {
	"April-June 16", 
	"October 16 - July 17", 
	"August 17 - March 18", 
	"May 18 - January 19", 
	"March 19 - July 20",
	"August 20 - January 21",
	"March 21 - January 22",
	"February 22 - June 23",
	"July 23 - January 25",
	"February 25 - May 25"
};
