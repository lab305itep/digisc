#include <sys/stat.h>
TChain *create_chain(const char *name, int from, int to)
{
	TChain *ch;
	char str[1024];
	int i;
	
	ch = new TChain(name, name);
	for (i=from; i<=to; i++) {
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/muon/%3.3dxxx/muon_%6.6d.root", i/1000, i);
		ch->AddFile(str, 0);
	}
	printf("%d entries found.\n", ch->GetEntries());
	return ch;
}
