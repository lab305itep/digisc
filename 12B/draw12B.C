TRandom2 rnd;

class MyRandom {
    public:
	inline MyRandom(void) {;};
	inline ~MyRandom(void) {;};
	static inline double Gaus(double mean = 0, double sigma = 1) {
		return rnd.Gaus(mean, sigma);
	};
	static inline double GausAdd(double val, double sigma) {
		return rnd.Gaus(val, sqrt(val)*sigma);
	};
	static inline double GausAdd2(double val, double sigma) {
		return rnd.Gaus(val, val*sigma);
	};
};

TChain *create_chain(const char *name, int from, int to)
{
	TChain *ch;
	char str[1024];
	int i;
	FILE *f_stat;
	int *rc_stat;
	char *ptr;
	int num;
	
	f_stat = fopen("stat_all.txt", "rt");
	if (!f_stat) {
		printf("Can not open stat file!\n");
		return NULL;
	}
	
	rc_stat = (int *) malloc((to - from + 1) * sizeof(int));
	if (!rc_stat) {
		printf("No memory !\n");
		return NULL;
	}
	memset(rc_stat, 0, (to - from + 1) * sizeof(int));
	for (;;) {
		ptr = fgets(str, sizeof(str), f_stat);
		if (!ptr) break;
		ptr = strtok(str, " \t");
		if (!ptr) continue;
		if (!isdigit(ptr[0])) continue;
		num = strtol(ptr, NULL, 10);
		if (num < from || num > to) continue;
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		if (!isdigit(ptr[0])) continue;
		rc_stat[num - from] = strtol(ptr, NULL, 10);
	}
	fclose(f_stat);
	
	ch = new TChain(name, name);
	for (i=from; i<=to; i++) {
		if (rc_stat[i - from] != 2 && rc_stat[i - from] != 3 && rc_stat[i - from] != 4 && rc_stat[i - from] != 5 && rc_stat[i - from] != 16) continue;
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/muon7n7/%3.3dxxx/muon_%6.6d.root", i/1000, i);
		num = access(str, R_OK);	// R_OK = 4 - test read access
		if (num) continue;
		ch->AddFile(str, 0);
	}
	printf("%Ld entries found.\n", ch->GetEntries());
	
	free(rc_stat);
	
	return ch;
}

void draw12B(int from, int to, double kRndm = 0, double kScale = 1.0)
{
	const char *mcname = "/home/clusters/rrcmpi/alekseev/igor/root6n7/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root";

//	gROOT->ProcessLine(".L create_chain.C+");
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	
	char str[1024];
	TChain *chA = create_chain("MuonPair", from, to);
	TChain *chR = create_chain("MuonRandom", from, to);
	if (!chA || !chR) return;
	TFile *fMC = new TFile(mcname);
	if (!fMC->IsOpen()) return;
	TTree *tMC = (TTree *) fMC->Get("DanssEvent");
	
	sprintf(str, "Experiment with ^{12}B cuts, %s;MeV", "ClusterEnergy");
	TH1D *hExp = new TH1D("hExp12B", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, random, %s;MeV", "ClusterEnergy");
	TH1D *hRndm = new TH1D("hRndm12B", str, 80, 0, 20);
	sprintf(str, "MC for ^{12}B decay, %s + Random (%6.4f/#sqrt{E});MeV", "PositronEnergy", kRndm);
	TH1D *hMC = new TH1D("hMC12B", str, 80, 0, 20);
	TH1D *hExpT = new TH1D("hExp12BT", "Time from muon, experiment;ms", 99, 1, 100);
	TH1D *hRndmT = new TH1D("hRndm12BT", "Time from muon, MC;ms", 99, 1, 100);
	
	sprintf(str, "ClusterEnergy * %6.4f", kScale);
	chA->Project(hExp->GetName(), str, "gtDiff > 500");
	chR->Project(hRndm->GetName(), str, "gtDiff > 500");
	sprintf(str, "MyRandom::GausAdd(PositronEnergy, %6.4f)", kRndm);
	tMC->Project(hMC->GetName(), str);
	chA->Project(hExpT->GetName(), "gtDiff / 1000.0");
	chR->Project(hRndmT->GetName(), "gtDiff / 1000.0");
	
	hExp->Sumw2();
	hRndm->Sumw2();
	hMC->Sumw2();
	hExpT->Sumw2();
	hRndmT->Sumw2();
	
	hExp->Add(hRndm, -1.0/16);
	hExpT->Add(hRndmT, -1.0/16);
	hMC->Scale(hExp->Integral(17, 64) / hMC->Integral(17, 64));
	
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetMarkerColor(kRed);
	hExp->SetLineColor(kRed);
	hExpT->SetMarkerStyle(kFullCircle);
	hExpT->SetMarkerColor(kGreen);
	hExpT->SetLineColor(kGreen);
	hMC->SetLineWidth(2);
	hMC->SetLineColor(kBlue);
	
	TCanvas *cv = new TCanvas("CV", "12B", 1400, 800);
	cv->Divide(2,1);
	cv->cd(1);
	hExp->Draw();
	hMC->Draw("hist,same");
	TLegend *lg = new TLegend(0.6, 0.75, 0.9, 0.9);
	sprintf(str, "Exp * %6.4f", kScale);
	lg->AddEntry(hExp, str, "pe");
	sprintf(str, "MC + %6.4f/#sqrt{E}", kRndm);
	lg->AddEntry(hMC, str, "l");
	lg->Draw();
	cv->cd(2);
	hExpT->Fit("expo");
	
	sprintf(str, "12B_78_rndm_%5.3f_scale_%5.3f.png", kRndm, kScale);
	cv->SaveAs(str);
	
	sprintf(str, "12B_78_rndm_%5.3f_scale_%5.3f.root", kRndm, kScale);
	TFile *fOut = new TFile(str, "RECREATE");
	fOut->cd();
	hExp->Write();
	hMC->Write();
	hExpT->Write();
	fOut->Close();
}

TH2D *makeScan(const char *pattern = "12B_74_rndm_%5.3f_scale_%5.3f.root")
{
	const double Rrndm[2] = {0.07, 0.22};
	const int Nrndm = 4;
	double Drndm = (Rrndm[1] - Rrndm[0]) / (Nrndm - 1);
	const double Rscale[2] = {0.97, 1.03};
	const int Nscale = 13;
	double Dscale = (Rscale[1] - Rscale[0]) / (Nscale - 1);
	double kRndm, kScale;
	int i, j;
	TH2D *h2d;
	TFile *f;
	char strs[128], strl[1024];
	TH1D *hExp;
	TH1D *hMC;
	TH1D *hRat;
	
	
	h2d = new TH2D("h12Bscan", "Scan rndm and scale for 12B;Scale;Rndm", 
		Nscale, Rscale[0] - Dscale/2, Rscale[1] + Dscale/2, Nrndm, Rrndm[0] - Drndm/2, Rrndm[1] + Drndm/2);
	for (i=0; i<Nscale; i++) for (j=0; j<Nrndm; j++) {
		kScale = Rscale[0] + Dscale * i;
		kRndm = Rrndm[0] + Drndm * j;
		sprintf(strl, pattern, kRndm, kScale);
		f = new TFile(strl);
		if (!f->IsOpen()) return NULL;
		hExp = (TH1D*) f->Get("hExp12B");
		hMC = (TH1D*) f->Get("hMC12B");
		if (!hExp || !hMC) return NULL;
		hRat = (TH1D*) hExp->Clone("hRat");
		hRat->Divide(hExp, hMC);
		hRat->Fit("pol0", "Q0", "", 3.5, 13);
		h2d->SetBinContent(i+1, j+1, hRat->GetFunction("pol0")->GetChisquare());
		f->Close();
		delete f;
	}
	
	f = new TFile("12B-scan.root", "RECREATE");
	f->cd();
	h2d->Write();
	f->Close();
	
	return h2d;
}
