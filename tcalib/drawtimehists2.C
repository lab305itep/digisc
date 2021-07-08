#define MAXBRD	60
#define MAXCHAN	64

TH1D *hSum[MAXBRD][MAXCHAN];
TF1 *fPol1;
TF1 *fPolGaus;

void Print(int first, int last, const char *dirname, const char *hist)
{
	char str[1024];
	const char *ptr;
	int irc; 
	int i, j, cnt;
	FILE *fRep;
	TLine *lnRed;
	TLine *lnBlue;
	double delta;
	TVirtualPad *pd;
	double y1, y2;

	TCanvas *cv = new TCanvas("CVT", "CVT", 1500, 1200);
	strncpy(str, dirname, sizeof(str));
	if (str[strlen(str) - 1] == '/')  str[strlen(str) - 1] = '\0';
	ptr = strrchr(str, '/');
	if (!ptr) {
		ptr = str;
	} else {
		ptr++;
	}
	sprintf(str, "%s_%s_%d_%d", hist, ptr, first, last);
	TString s(str);
	fRep = fopen((s+".log").Data(), "wt");
	if (!fRep) {
		printf("Can not open rep file %s: %m\n", (s+".log").Data());
		return;
	}
	cv->Print((s+".pdf[").Data());
	lnRed = new TLine();
	lnRed->SetLineColor(kRed);
	lnRed->SetLineWidth(2);
	lnBlue = new TLine();
	lnBlue->SetLineColor(kBlue);
	lnBlue->SetLineWidth(2);
	
	for (i=0; i<MAXBRD; i++) for (j=0; j<MAXCHAN; j++) {
		if (!(j & 15)) {
			cv->Clear();
			cv->Divide(4, 4);
			cnt = 0;
		}
		if (hSum[i][j]->GetEntries() > 1000) {
			pd = cv->cd((j & 15) + 1);
			hSum[i][j]->UseCurrentStyle();
			hSum[i][j]->Fit(fPol1, "Q0", "");
			fPolGaus->SetParameters(fPol1->GetParameter(0), fPol1->GetParameter(1), 
				hSum[i][j]->GetMaximum() - fPol1->GetParameter(0), 0, 3.0);
			hSum[i][j]->Fit(fPolGaus, "Q", "");
			y1 = 0;
			y2 = 1.05*hSum[i][j]->GetMaximum();
			lnBlue->DrawLine(0, y1, 0, y2);
			if (i != 1 && i != 3) {
				lnRed->DrawLine(-10, y1, -10, 0.5*(y1 + y2));
				lnRed->DrawLine( 10, y1,  10, 0.5*(y1 + y2));
			}
			fprintf(fRep, "Channel= %2.2d.%2.2d  DT= %6.1f\n", i, j, fPolGaus->GetParameter(3));
			cnt++;
		}
		if ((j & 15) == 15 && cnt > 0) {
			cv->Update();
			cv->Print((s+".pdf").Data());
		}
	}
	cv->Print((s+".pdf]").Data());
	fclose(fRep);
}

void AddFromFile(int num, const char *dirname, const char *hist)
{
	char str[1024];
	TH1D *h;
	int i, j;
	
	sprintf(str, "%s/%3.3dxxx/danss_%6.6d.root", dirname, num/1000, num);
	TFile *f = new TFile(str);
	if (!f->IsOpen()) return;
	for (i=0; i<MAXBRD; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(str, "%s%2.2dc%2.2d", hist, i, j);
		h = (TH1D*) f->Get(str);
		if (!h) continue;
		hSum[i][j]->Add(h);
	}
	f->Close();
}

void MakeHists(const char *hist)
{
	int i, j;
	char strs[128];
	char strl[1024];
	for (i=0; i<MAXBRD; i++) for (j=0; j<MAXCHAN; j++) {
		sprintf(strs, "%s%2.2dc%2.2dSum", hist, i, j);
		sprintf(strl, "Channel %2.2d.%2.2d versus PMT;ns", i, j);
		hSum[i][j] = new TH1D(strs, strl, 250, -25, 25);
	}
}

//	Sum hDTPUUcNN hists in the range and make .log and .pdf
void drawtimehists2(int first, int last, const char *dirname, const char *hist)
{
	int i;
//	gStyle->SetOptStat(1001110);
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	gStyle->SetStatFontSize(0.05);
	gStyle->SetStatH(0.4);
	gStyle->SetStatW(0.4);
	gStyle->SetTitleFontSize(0.08);
	fPol1 = new TF1("fPol1", "pol1", -50, 50);
	fPolGaus = new TF1("fPolGaus", "pol1(0) + gaus(2)", -50, 50);
	MakeHists(hist);
	for (i=first; i<=last; i++) AddFromFile(i, dirname, hist);
	Print(first, last, dirname, hist);
}
