#define MAXRUN		100000
#define MAXTOKEN	25

#define TOK_RUN	0
#define TOK_TYP	1
#define TOK_DAY	2
#define TOK_TIM	3
#define TOK_NVT	11
#define TOK_20M	14
#define TOK_TOT	17
#define TOK_DED	19
#define TOK_EFF	21
#define TOK_PWR	22

struct StatAllStruct {
	int 	type;
	time_t	start;
	double	no_veto;
	double	time_total;
	double	time_dead;
	double	eff;
	double	power;
};

void StrSplit(char *str, char **token)
{
	int i;
	const char DELIM[] = " \t\r\n";
	
	memset(token, 0, MAXTOKEN * sizeof(char *));
	for (i=0; i<MAXTOKEN; i++) {
		token[i] = strtok((i) ? NULL : str, DELIM);
		if (!token[i]) break;
	}
}

time_t ConvertTime(const char *date, const char *time)
{
	struct tm tm_time;
	
	memset(&tm_time, 0, sizeof(tm_time));
	strptime(date, "%F", &tm_time);
	strptime(time, "%H:%M", &tm_time);
	return mktime(&tm_time);
}

int ReadStat(const char *name, struct StatAllStruct *Stat)
{
	FILE *fStat;
	char str[1024];
	char *ptr;
	char *token[MAXTOKEN];
	int irc;
	int type;
	
	fStat = fopen(name, "rt");
	if (!fStat) {
		printf("Can not open stat-file %s: %m\n", name);
		return 10;
	}
	memset(Stat, 0, MAXRUN * sizeof(struct StatAllStruct));
	
	for (;;) {
		ptr = fgets(str, sizeof(str), fStat);
		if (!ptr) break;	// EOF
		StrSplit(str, token);
		//	Getting run number
		if (!token[TOK_PWR]) continue;
		irc = strtol(token[TOK_RUN], NULL, 10);
		type = strtol(token[TOK_TYP], NULL, 10);
		if (irc <= 0 || type <= 0) continue;
		if (irc >= MAXRUN) {
			printf("Run %d > MAXRUN=%d met = skipping\n", irc, MAXRUN);
			continue;
		}
		Stat[irc].type = type;
		Stat[irc].start = ConvertTime(token[TOK_DAY], token[TOK_TIM]);
		Stat[irc].no_veto = strtod(token[TOK_NVT], NULL) / strtod(token[TOK_20M], NULL);
		Stat[irc].time_total = strtod(token[TOK_TOT], NULL);
		Stat[irc].time_dead = strtod(token[TOK_DED], NULL);
		Stat[irc].eff = strtod(token[TOK_EFF], NULL);
		Stat[irc].power = strtod(token[TOK_PWR], NULL);
	}
	
	fclose(fStat);
	return 0;
}

void scan_pos(int from, int to, const char *root_dir, const char *stat_file)
{
	struct {
		int run;
		int type;
		int pos;
		float height;
	} OutData;
	char fname[1024];
	FILE *fff;
	struct StatAllStruct *Stat;
	int irc;
	int run;
	
	Stat = (struct StatAllStruct *) malloc(MAXRUN * sizeof(struct StatAllStruct));
	if (!Stat) {
		printf("No memory: %m\n");
		return;
	}
	
	irc = ReadStat(stat_file, Stat);
	if (irc) {
		printf("Can not read stat file\n");
		return;
	}
	
	sprintf(fname, "/home/clusters/rrcmpi/alekseev/igor/%s/height_%6.6d_%6.6d.root", root_dir, from, to);
	TFile *fOut = new TFile(fname, "RECREATE");
	if (!fOut->IsOpen()) return;
	TTree *tOut = new TTree("H", "Height");
	tOut->Branch("Data", &OutData, "Run/I:Type/I:Pos/I:H/F");
	
	for (run = from; run <= to && run < MAXRUN; run++) {
		sprintf(fname, "/home/clusters/rrcmpi/alekseev/igor/%s/%3.3dxxx/danss_%6.6d.root", root_dir, run/1000, run);
		fff = fopen(fname, "rb");
		if (!fff) continue;
		fclose(fff);
		TFile *f = new TFile(fname);
		if (!f->IsOpen()) {
			printf("%6d   0 file %s not opened\n", run, fname);
			continue;
		}

		TTree *info = (TTree *) f->Get("DanssInfo");
		if (!info) {
			printf("%6d   0 file %s - no info tree\n", run, fname);
			continue;
		}
		info->GetEntry(0);
		OutData.run = run;
		OutData.type = Stat[run].type;
		OutData.pos = info->GetLeaf("position")->GetValue();
		OutData.height = info->GetLeaf("height")->GetValue();
		tOut->Fill();
		f->Close();
	}
	fOut->cd();
	tOut->Write();
	fOut->Close();
}
