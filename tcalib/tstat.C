void tstat(int first, int last, const char *hname = "hDT01c", const char *dir = "/home/clusters/rrcmpi/alekseev/igor/root8n2")
{
	int num, i, j;
	TFile *f;
	TH1 *h;
	TTree *t;
	char str[4096];
	TFile *fOut;
	TTree *tOut;
	struct StatStruct {
		int Run;
		int Chan;
		float Mean;
		float RMS;
	} Stat;
	
	sprintf(str, "mkdir -p %s/tstat", dir);
	system(str);
	sprintf(str, "%s/tstat/%s_%6.6d_%6.6d.root", dir, hname, first, last);
	fOut = new TFile(str, "RECREATE");
	if (!fOut->IsOpen()) return;
	tOut = new TTree("tStat", "tStat");
	tOut->Branch("Stat", &Stat, "Run/I:Chan/I:Mean/F:RMS/F");
	num = 0;
	try {
		for (num = first; num <= last; num++) {
			sprintf(str, "%s/%3.3dxxx/danss_%6.6d.root", dir, num/1000, num);
			f = new TFile(str);
			if (!f->IsOpen()) continue;
			t = (TTree *) f->Get("DanssEvent");
			if (!t || t->GetEntries() < 500000) continue;
			Stat.Run = num;
			for (i=0; i<64; i++) {
				sprintf(str, "%s%2.2d", hname, i);
				h = (TH1 *) f->Get(str);
				if (h && h->GetEntries() > 1000) {
					for (j=0; j<5; j++) h->GetXaxis()->SetRangeUser( // 5 iteration to cut tails
						h->GetMean() - 2.0*h->GetRMS(), h->GetMean() + 2.0*h->GetRMS());
					Stat.Chan = i;
					Stat.Mean = h->GetMean();
					Stat.RMS  = h->GetRMS();
					tOut->Fill();
				}
			}
			f->Close();
			delete f;
		}
	} catch (...) {
		printf("Exception at num = %d\n", num);
	}
	fOut->cd();
	tOut->Write();
	fOut->Close();
}

void DrawStat(int first, int last, int chfirst, int chlast, const char *fname)
{
	char strs[128];
	char strl[1024];
	int i, color, nx;
	
	TTree *t =new TTree("T", "T");
	t->ReadFile(fname, "Run/I:D[64]/F:Cnt/I");
	for(i=chfirst; i<=chlast; i++) {
		sprintf(strs, "hDT_%d", i);
		sprintf(strl, "Time differencies for channels %d-%d;Run;#Delta t, ns", chfirst, chlast);
		nx = last - first + 1;
		while (nx > 2000) nx /= 2;
		TH2D *h = new TH2D(strs, strl, nx, first, last, 200, -10, 10);
		color = (i > chfirst + 8) ? i - chfirst + 20 : i - chfirst + 1;
		h->SetLineColor(color);
		h->SetMarkerColor(color);
		h->SetMarkerStyle(kFullDotMedium);
		sprintf(strl, "D[%d]:Run", i);
		t->Project(strs, strl);
		h->DrawCopy((i == chfirst) ? "P" : "P,SAME");
		delete h;
	}
}

void DrawAllStat(void)
{
	char str[1024];
	int i;
	
	TCanvas *cv = new TCanvas("CV", "CV", 2000, 800);
	cv->SaveAs("tstat_all.pdf[");
	for (i=1; i<=51; i++) {
		sprintf(str, "hstat_hDT%2.2d.txt_l", i);
		DrawStat(2210, 42300, 0, 63, str);
	cv->SaveAs("tstat_all.pdf");
	}
	cv->SaveAs("tstat_all.pdf]");
}
