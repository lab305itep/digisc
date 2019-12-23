void tstat(int first, int last, const char *hname = "hDT01c", const char *dir = "/home/clusters/rrcmpi/alekseev/igor/root6n5")
{
	int num, i, j;
	TFile *f;
	TH1 *h;
	TTree *t;
	char str[4096];
	int cnt;
	
	for (num = first; num <= last; num++) {
		sprintf(str, "%s/%3.3dxxx/danss_%6.6d.root", dir, num/1000, num);
		f = new TFile(str);
		if (!f->IsOpen()) continue;
		t = (TTree *) f->Get("DanssEvent");
		if (!t || t->GetEntries() < 500000) continue;
		printf("%6d ", num);
		cnt = 0;
		for (i=0; i<64; i++) {
			sprintf(str, "%s%2.2d", hname, i);
			h = (TH1 *) f->Get(str);
			if (!h || h->GetEntries() < 1000) {
				printf("-1000  ");
			} else {
				for (j=0; j<5; j++) h->GetXaxis()->SetRangeUser( // 5 iteration to cut tails
					h->GetMean() - 2.0*h->GetRMS(), h->GetMean() + 2.0*h->GetRMS());
				printf("%6.1f ", h->GetMean());
				cnt++;
			}
		}
		printf("%2d\n", cnt);
		f->Close();
	}
}

void DrawStat(int first, int last, int chfirst, int chlast, const char *fname)
{
	char strs[128];
	char strl[1024];
	int i, color;
	
		TTree *t =new TTree("T", "T");
	t->ReadFile(fname, "Run/I:D[64]/F:Cnt/I");
	for(i=chfirst; i<=chlast; i++) {
		sprintf(strs, "hDT_%d", i);
		sprintf(strl, "Time differencies for channels %d-%d;Run;#Delta t, ns", chfirst, chlast);
		TH2D *h = new TH2D(strs, strl, last - first + 1, first, last, 200, -10, 10);
		color = (i > chfirst + 8) ? i - chfirst + 20 : i - chfirst + 1;
		h->SetLineColor(color);
		h->SetMarkerColor(color);
		h->SetMarkerStyle(kFullDotMedium);
		sprintf(strl, "D[%d]:Run", i);
		t->Project(strs, strl);
		h->Draw((i == chfirst) ? "P" : "P,SAME");
	}
}

void DrawAllStat(int modmin, int modmax)
{
	char str[1024];
	int i;
	
	TCanvas *cv = new TCanvas("CV", "CV", 2000, 800);
	for (i=modmin; i<=modmax; i++) {
		sprintf(str, "tstat_hDT%2.2d.txt_l", i);
		DrawStat(2210, 42300, 0, 63, str);
		sprintf(str, "tstat_%d.png", i);
		cv->SaveAs(str);
	}
}
