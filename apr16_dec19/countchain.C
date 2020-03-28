TChain *makechain(const char *where, const char *what, const char *name, int first, int last)
{
	char str[1024];
	int i;
	TChain *ch = new TChain(name);
	for (i=first; i<=last; i++) {
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/%3.3dxxx/%s_%6.6d.root", where, i/1000, what, i);
		ch->AddFile(str);
	}
	return ch;
}

void count(int first, int last)
{
	TChain *ch;
	int cnt;
	
	ch = makechain("pair7n9", "pair", "DanssPair", first, last);
	cnt = ch->GetEntries();
	printf("pair7n9: DanssPair = %d\n", cnt); 
	delete ch;
	ch = makechain("pair7n9", "pair", "DanssRandom", first, last);
	cnt = ch->GetEntries();
	printf("pair7n9: DanssRandom = %d\n", cnt); 
	delete ch;
	ch = makechain("pair7n10", "pair", "DanssPair", first, last);
	cnt = ch->GetEntries();
	printf("pair7n10: DanssPair = %d\n", cnt); 
	delete ch;
	ch = makechain("pair7n10", "pair", "DanssRandom", first, last);
	cnt = ch->GetEntries();
	printf("pair7n10: DanssRandom = %d\n", cnt); 
	delete ch;
}
