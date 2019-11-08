#define MAXBRD	60
#define MAXCHAN	64

TH1D *hSum[MAXBRD][MAXCHAN];

void Print(int first, int last, const char *dirname)
{
	char str[1024];
	const char *ptr;
	int irc; 
	int i, j, cnt;
	FILE *fRep;

	TCanvas *cv = new TCanvas("CVT", "CVT", 1500, 1200);
	ptr = strrchr(dirname, '/');
	if (!ptr) {
		ptr = dirname;
	} else {
		ptr++;
	}
	sprintf(str, "%s_%d_%d", ptr, first, last);
	TString s(str);
	fRep = fopen((s+".log").Data(), "wt");
	if (!fRep) {
		printf("Can not open rep file %s: %m\n", (s+",log").Data());
		return;
	}
	cv->Print((s+".pdf[").Data());
	
	for (i=0; i<MAXBRD; i++) for (j=0; j<MAXCHAN; j++) {
		if (!(j & 15)) {
			cv->Clear();
			cv->Divide(4, 4);
			cnt = 0;
		}
		if (hSum[i][j]->GetEntries() > 1000) {
			cv->cd((j & 15) + 1);
			hSum[i][j]->Draw();
			fprintf(fRep, "Channel=%2.2d.%2.2d  DT=%6.1f\n", i, j, hSum[i][j]->GetMean());
			cnt++;
		}
		if ((j & 15) == 15 && cnt > 0) cv->Print((s+".pdf").Data());
	}
	cv->Print((s+".pdf]").Data());
	fclose(fRep);
}

void AddFromFile(int num, const char *dirname)
{
	char str[1024];
	TH1D *h;
	int i, j;
	
	sprintf(str, "%s/%3.3dxxx/danss_%6.6d.root", dirname, num/1000, num);
	TFile *f = new TFile(str);
	if (!f->IsOpen()) return;
	for (i=0; i<MAXBRD; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(str, "hDTP%2.2dc%2.2d", i, j);
		h = (TH1D*) f->Get(str);
		if (!h) continue;
		hSum[i][j]->Add(h);
	}
	f->Close();
}

void MakeHists(void)
{
	int i, j;
	char strs[128];
	char strl[1024];
	for (i=0; i<MAXBRD; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(strs, "hDTP%2.2dc%2.2dSum", i, j);
		sprintf(strl, "Channel %2.2d.%2.2d time difference to the average PMT time;ns", i, j);
		hSum[i][j] = new TH1D(strs, strl, 250, -25, 25);
	}
}

//	Sum hDTPUUcNN hists in the range and make .log and .pdf
void drawtimehists2(int first, int last, const char *dirname)
{
	int i;
	gStyle->SetOptStat(1110);
	MakeHists();
	for (i=first; i<=last; i++) AddFromFile(i, dirname);
	Print(first, last, dirname);
}
