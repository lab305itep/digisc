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

void draw12B(const char *mcname, int from, int to, double kRndm = 0, double kScale = 1.0)
{
	gROOT->ProcessLine(".L create_chain.C+");
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
	tMC->Project(hMC->GetName(), "PositronEnergy");
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
}
