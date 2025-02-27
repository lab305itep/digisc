TChain *make_UP(void)
{
//	Top position, eff=0.3439; deadtime=0.1294; time=689090.6
	char str[1024];
	int i;
	const char *exdir = "/home/clusters/rrcmpi/alekseev/igor/exotics8n2";
	
	TChain *ch = new TChain("Exotics", "Up");
	for (i=89704; i<=90194; i++) {
		sprintf(str, "%s/%3.3dxxx/exotics_%6.6d.root", exdir, i/1000, i);
		if (access(str, R_OK)) continue;	// no file
		ch->AddFile(str);
	}
	return ch;
}

TChain *make_DOWN(void)
{
//	Bottom position, eff=0.3439; deadtime=0.1422; time=523430.4
	char str[1024];
	int i;
	const char *exdir = "/home/clusters/rrcmpi/alekseev/igor/exotics8n2";
	
	TChain *ch = new TChain("Exotics", "Down");
	for (i=90195; i<=90588; i++) {
		sprintf(str, "%s/%3.3dxxx/exotics_%6.6d.root", exdir, i/1000, i);
		if (access(str, R_OK)) continue;	// no file
		ch->AddFile(str);
	}
	return ch;
}

TChain *make_OFF(void)
{
//	OFF: top position, eff=0.3465; deadtime=0.0922; time=516941.1
	char str[1024];
	int i;
	const char *exdir = "/home/clusters/rrcmpi/alekseev/igor/exotics8n2";
	
	TChain *ch = new TChain("Exotics", "Off");
	for (i=88566; i<=88859; i++) {
		sprintf(str, "%s/%3.3dxxx/exotics_%6.6d.root", exdir, i/1000, i);
		if (access(str, R_OK)) continue;	// no file
		ch->AddFile(str);
	}
	return ch;
}

