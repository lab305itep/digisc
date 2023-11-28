void drawMuonEvent(const char *fname, int num)
{
	struct MuonEventStruct {
		int MCNum;
		int NHits;
		float xUp;
		float yUp;
		float xDown;
		float yDown;
		float r2;
	} Event;
	float XZ[50][25], YZ[50][25];
	int i, j;
	char str[1024];
	
	gStyle->SetOptStat(0);
	gStyle->SetPadRightMargin(0.15);
	
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	TTree *t = (TTree *) f->Get("Event");
	if (!t) {
		printf("Tree Event not found in %s\n", fname);
		delete f;
		return;
	}
	
	t->SetBranchAddress("Data", &Event);
	t->SetBranchAddress("XZ", XZ);
	t->SetBranchAddress("YZ", YZ);
	
	TH2F *hxz = new TH2F("hxz", "ZX-plane;x, cm;z, cm;E, MeV", 25, 0, 100, 50, 0, 100);
	TH2F *hyz = new TH2F("hyz", "ZY-plane;y, cm;z, cm;E, MeV", 25, 0, 100, 50, 0, 100);
	hxz->SetMinimum(0);
	hyz->SetMinimum(0);
	hxz->SetMaximum(6.0);
	hyz->SetMaximum(6.0);
	t->GetEntry(num);
	
	for (i=0; i<50; i++) for (j=0; j<25; j++) {
		hxz->SetBinContent(j+1, i+1, XZ[i][j]);
		hyz->SetBinContent(j+1, i+1, YZ[i][j]);
	}
	
	TLine *ln = new TLine();
	ln->SetLineWidth(2);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CVEvent");
	if (cv) {
		cv->Clear();
	} else {
		cv = new TCanvas("CVEvent", "Event", 1600, 800);
	}
	cv->Divide(2, 1);
	cv->cd(1);
	hxz->DrawCopy("colorz");
	ln->DrawLine(Event.xDown, 4, Event.xUp, 96);
	cv->cd(2);
	hyz->DrawCopy("colorz");
	ln->DrawLine(Event.yDown, 4, Event.yUp, 96);
	delete hxz;
	delete hyz;
	delete ln;
	
	sprintf(str, "muon_event_%d.png", num);
	cv->SaveAs(str);
	delete f;
}

/****************************************************************/
/*
 * Caculate histogram median in the range [firstbin, lastbin]
 * The default includes underflow and overflow
 * Error = binwidth * sqrt(N) / 2 / binheight
 * Histogram could be scaled
 * Unit weights (just events) assumed during the fill
 */
double Median(TH1 *h, double *err = NULL)
{
	int firstbin, lastbin;
	double half;
	double sum;
	double val;
	double x;
	int i;
	
	firstbin = 0;			// underflow
	lastbin = h->GetNbinsX() + 1;	// overflow
	half = 0.5 * h->Integral(firstbin, lastbin);
	sum = 0;
	for (i = firstbin; i <= lastbin; i++) {
		val = h->GetBinContent(i);
		if (val <= 0) continue;
		if (sum + val < half) {
			sum += val;
		} else {
			x = h->GetXaxis()->GetBinLowEdge(i) + h->GetXaxis()->GetBinWidth(i) * (half - sum) / val;
			break;
		}
	}
	if (err) *err = h->GetXaxis()->GetBinWidth(i) * half / val / sqrt(h->GetEntries());
	return x;
}


void drawMedian(const char *fname, int num)
{
	char str[1024];
	double val;
	
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	sprintf(str, "hPhe_%6.6d", num);
	TH1D *h = (TH1D*) f->Get(str);
	if (!h) {
		printf("%s not found in %s\n", str, fname);
		delete f;
		return;
	}
	gStyle->SetOptStat(10);
	h->SetTitle(";ph.c.;Hits");
	TCanvas *cv = new TCanvas("CVMedian", "Median", 800, 800);
	h->DrawCopy();
	TLine *ln = new TLine();
	TLatex *txt = new TLatex();
	
	val = h->GetMean();
	sprintf(str, "Mean = %5.2f", val);
	txt->SetTextColor(kBlack);
	txt->DrawLatexNDC(0.4, 0.7, str);
	ln->SetLineColor(kBlack);
	ln->SetLineWidth(2);
	ln->DrawLine(val, 0, val, 0.85*h->GetMaximum());

	val = Median(h);
	sprintf(str, "Median = %5.2f", val);
	txt->SetTextColor(kBlue);
	txt->DrawLatexNDC(0.4, 0.77, str);
	ln->SetLineColor(kBlue);
	ln->SetLineWidth(2);
	ln->DrawLine(val, 0, val, 0.95*h->GetMaximum());
	
	delete f;
}