void digi_statone(int num, const char *root_dir)
{
	char fname[1024];
	double tm;
	char start[64], stop[64];
	FILE *fff;
	TCut ct;
	
//	TCut cNoise("PmtCleanEnergy/SiPmCleanEnergy < 0.4 && SiPmHits > 60");
	TCut cNoise("(PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3");
	TCut cVeto("VetoCleanHits > 1 || VetoCleanEnergy > 4 || BottomLayersEnergy > 3.0");
	TCut cDVeto("PmtCleanEnergy + SiPmCleanEnergy > 40");
	
	sprintf(fname, "/home/clusters/rrcmpi/alekseev/igor/%s/%3.3dxxx/danss_%6.6d.root", root_dir, num/1000, num);
	fff = fopen(fname, "rb");
	if (!fff) {
		printf("%d 0 file %s not found\n", num, fname);
		return;
	}
	fclose(fff);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) {
		printf("%d 0 file %s not found\n", num, fname);
		return;
	}

	TTree *info = (TTree *) f->Get("DanssInfo");
	if (!info) {
		printf("%d 0 file %s - no info\n", num, fname);
		return;
	}
	info->GetEntry(0);
	tm = info->GetLeaf("gTime")->GetValueLong64() / 125.0E6;
	if (tm < -250000 && tm > -300000) tm += 281474.9767;	// workaround 45 bit counter wrapping
//	time_t tStart = info->GetLeaf("startTime")->GetValue();
	time_t tStop = info->GetLeaf("stopTime")->GetValue();
	int ipos = info->GetLeaf("position")->GetValue();
	
	TTree *evt = (TTree *) f->Get("DanssEvent");
	if (!evt) {
		printf("%d 0 file %s - no events\n", num, fname);
		return;
	}
	TTree *hits = (TTree *) f->Get("RawHits");
	if (!hits) {
		printf("RawHits tree not found in %s\n", fname);
		return;
	}
	evt->AddFriend(hits);
	evt->GetEntry(0);
	time_t tStart = evt->GetLeaf("unixTime")->GetValue();

	int N = evt->GetEntries();
	
	int noise = evt->GetEntries(cNoise.GetTitle());
	ct = (cVeto || cDVeto) && !cNoise;
	int veto = evt->GetEntries(ct.GetTitle());
	ct = cVeto && !cDVeto && !cNoise;
	int vetoOnly = evt->GetEntries(ct.GetTitle());
	ct = !cVeto && cDVeto && !cNoise;
	int danssOnly = evt->GetEntries(ct.GetTitle());
	ct = TCut("PmtCleanEnergy + SiPmCleanEnergy > 2") && !cNoise;
	int gt1MeV = evt->GetEntries(ct.GetTitle());
	ct = TCut("PmtCleanEnergy + SiPmCleanEnergy > 6") && !cNoise;
	int gt3MeV = evt->GetEntries(ct.GetTitle());
	ct = TCut("PmtCleanEnergy + SiPmCleanEnergy > 40") && !cNoise;
	int gt20MeV = evt->GetEntries(ct.GetTitle());
	ct = TCut("PositronEnergy > 1 && PositronEnergy < 8") && !cNoise;
	int positrons = evt->GetEntries(ct.GetTitle());
	ct = TCut("PmtCleanEnergy + SiPmCleanEnergy > 6 && PmtCleanEnergy + SiPmCleanEnergy < 30 && SiPmCleanHits > 2") && !cNoise;
	int neutrons = evt->GetEntries(ct.GetTitle());
//	
	strftime(start, sizeof(start), "%F %R", localtime(&tStart));
	strftime(stop , sizeof(stop) , "%R", localtime(&tStop));
	printf("%6d %2d %s %s %5.0f   %8d %6.1f %6.1f %6.1f %6.1f %6.2f %6.1f %6.1f %6.1f %6.1f %6.1f\n", 
		num, ipos, start, stop, tm, N, N/tm, noise/tm, veto/tm, vetoOnly/tm, danssOnly/tm, gt1MeV/tm, 
			gt3MeV/tm, gt20MeV/tm, positrons/tm, neutrons/tm);

	f->Close();
}

void digi_stattitle(void)
{
	printf("Run   Pos Start           Stop   len, s   Events   Trig   PickUp Veto  NoPMT NoVeto  >1MeV  >3MeV >20MeV     e+     n\n");
}

void digi_stat(int first, int last, const char *root_dir = "root6")
{
	int i;
	
	digi_stattitle();
	for (i=first; i<=last; i++) digi_statone(i, root_dir);
}
