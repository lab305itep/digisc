TRandom2 rnd;

class MyRandom {
    public:
	inline MyRandom(void) {;};
	inline ~MyRandom(void) {;};
	static inline double Gaus(double mean = 0, double sigma = 1) {
		return rnd.Gaus(mean, sigma);
	};
	static inline double GausAdd(double val, double sigma = 0, double csigma = 0) {
		return rnd.Gaus(val, sqrt(val*sigma*sigma + val*val*csigma*csigma));
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
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/muon8n1/%3.3dxxx/muon_%6.6d.root", i/1000, i);
		num = access(str, R_OK);	// R_OK = 4 - test read access
		if (num) continue;
		ch->AddFile(str, 0);
	}
	printf("%Ld entries found.\n", ch->GetEntries());
	
	free(rc_stat);
	
	return ch;
}

void draw12B(int from, int to)
{
	const double RndmSqe = 0.12;
	const double Scale = 1.0;
	const double RndmC = 0.04;
	const double SiPMRndmSqe = 0.12;
	const double SiPMScale = 1.0;
	const double SiPMRndmC = 0.125;
	const double PMTRndmSqe = 0.12;
	const double PMTScale = 1.0;
	const double PMTRndmC = 0.06;
//	const char *mcname = "/home/clusters/rrcmpi/alekseev/igor/root6n8/MC/DataTakingPeriod01/12B/mc_12B_glbLY_transcode_rawProc_pedSim.root";
	const char *mcname = "/home/clusters/rrcmpi/alekseev/igor/root8n1/MC/RadSources/mc_12B_indLY_transcode_rawProc_pedSim.root";

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

	sprintf(str, "Experiment with ^{12}B cuts, %s*%6.4f;MeV", "ClusterEnergy", Scale);
	TH1D *hExp = new TH1D("hExp12B", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, %s*%6.4f;MeV", "ClusterSiPMEnergy", SiPMScale);
	TH1D *hExpSiPM = new TH1D("hExp12BSiPM", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, %s*%6.4f;MeV", "ClusterPMTEnergy", PMTScale);
	TH1D *hExpPMT = new TH1D("hExp12BPMT", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, random, %s*%6.4f;MeV", "ClusterEnergy", Scale);
	TH1D *hRndm = new TH1D("hRndm12B", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, random, %s*%6.4f;MeV", "ClusterSiPMEnergy", SiPMScale);
	TH1D *hRndmSiPM = new TH1D("hRndm12BSiPM", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, random, %s*%6.4f;MeV", "ClusterPMTEnergy", PMTScale);
	TH1D *hRndmPMT = new TH1D("hRndm12BPMT", str, 80, 0, 20);
	sprintf(str, "MC for ^{12}B decay, %s + Random (%6.4f/#sqrt{E} #oplus %6.4f);MeV", "PositronEnergy", RndmSqe, RndmC);
	TH1D *hMC = new TH1D("hMC12B", str, 80, 0, 20);
	sprintf(str, "MC for ^{12}B decay, %s + Random (%6.4f/#sqrt{E} #oplus %6.4f);MeV", "PositronSiPMEnergy", SiPMRndmSqe, SiPMRndmC);
	TH1D *hMCSiPM = new TH1D("hMC12BSiPM", str, 80, 0, 20);
	sprintf(str, "MC for ^{12}B decay, %s + Random (%6.4f/#sqrt{E} #oplus %6.4f);MeV", "PositronPMTEnergy", PMTRndmSqe, PMTRndmC);
	TH1D *hMCPMT = new TH1D("hMC12BPMT", str, 80, 0, 20);
	TH1D *hExpT = new TH1D("hExp12BT", "Time from muon, experiment;ms", 99, 1, 100);
	TH1D *hRndmT = new TH1D("hRndm12BT", "Time from muon, MC;ms", 99, 1, 100);

	sprintf(str, "ClusterEnergy * %6.4f", Scale);
	chA->Project(hExp->GetName(), str, "gtDiff > 500");
	chR->Project(hRndm->GetName(), str, "gtDiff > 500");
	sprintf(str, "ClusterSiPmEnergy * %6.4f", SiPMScale);
	chA->Project(hExpSiPM->GetName(), str, "gtDiff > 500");
	chR->Project(hRndmSiPM->GetName(), str, "gtDiff > 500");
	sprintf(str, "ClusterPmtEnergy * %6.4f", PMTScale);
	chA->Project(hExpPMT->GetName(), str, "gtDiff > 500");
	chR->Project(hRndmPMT->GetName(), str, "gtDiff > 500");
	sprintf(str, "MyRandom::GausAdd(PositronEnergy, %6.4f, %6.4f)", RndmSqe, RndmC);
	tMC->Project(hMC->GetName(), str);
	sprintf(str, "MyRandom::GausAdd(PositronSiPmEnergy, %6.4f, %6.4f)", SiPMRndmSqe, SiPMRndmC);
	tMC->Project(hMCSiPM->GetName(), str);
	sprintf(str, "MyRandom::GausAdd(PositronPmtEnergy, %6.4f, %6.4f)", PMTRndmSqe, PMTRndmC);
	tMC->Project(hMCPMT->GetName(), str);
	chA->Project(hExpT->GetName(), "gtDiff / 1000.0");
	chR->Project(hRndmT->GetName(), "gtDiff / 1000.0");
	
	hExp->Sumw2();
	hRndm->Sumw2();
	hMC->Sumw2();
	hExpSiPM->Sumw2();
	hRndmSiPM->Sumw2();
	hMCSiPM->Sumw2();
	hExpPMT->Sumw2();
	hRndmPMT->Sumw2();
	hMCPMT->Sumw2();
	hExpT->Sumw2();
	hRndmT->Sumw2();
	
	hExp->Add(hRndm, -1.0/16);
	hExpSiPM->Add(hRndmSiPM, -1.0/16);
	hExpPMT->Add(hRndmPMT, -1.0/16);
	hExpT->Add(hRndmT, -1.0/16);
	hMC->Scale(hExp->Integral(16, 36) / hMC->Integral(16, 36));
	hMCSiPM->Scale(hExpSiPM->Integral(16, 36) / hMCSiPM->Integral(16, 36));
	hMCPMT->Scale(hExpPMT->Integral(16, 36) / hMCPMT->Integral(16, 36));
	
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetMarkerColor(kRed);
	hExp->SetLineColor(kRed);
	hExpSiPM->SetMarkerStyle(kFullCircle);
	hExpSiPM->SetMarkerColor(kRed);
	hExpSiPM->SetLineColor(kRed);
	hExpPMT->SetMarkerStyle(kFullCircle);
	hExpPMT->SetMarkerColor(kRed);
	hExpPMT->SetLineColor(kRed);
	hExpT->SetMarkerStyle(kFullCircle);
	hExpT->SetMarkerColor(kGreen);
	hExpT->SetLineColor(kGreen);
	hMC->SetLineWidth(2);
	hMC->SetLineColor(kBlue);
	hMCSiPM->SetLineWidth(2);
	hMCSiPM->SetLineColor(kBlue);
	hMCPMT->SetLineWidth(2);
	hMCPMT->SetLineColor(kBlue);
	
	TCanvas *cv = new TCanvas("CV", "12B", 1400, 800);
	sprintf(str, "12B_81_rndm_%5.3f_%5.3f_scale_%5.3f", RndmSqe, RndmC, Scale);
	TString oname(str);
	cv->SaveAs((oname+".pdf[").Data());

	cv->Divide(2, 1);
	cv->cd(1);
	hExp->Draw();
	hMC->Draw("hist,same");
	TLegend *lg = new TLegend(0.6, 0.75, 0.9, 0.9);
	sprintf(str, "Exp * %6.4f", Scale);
	lg->AddEntry(hExp, str, "pe");
	sprintf(str, "MC + %6.4f/#sqrt{E} #oplus %6.4f", RndmSqe, RndmC);
	lg->AddEntry(hMC, str, "l");
	lg->Draw();
	cv->cd(2);
	hExpT->Fit("expo");
	cv->SaveAs((oname+".pdf").Data());

	cv->Clear();
	cv->Divide(2,1);
	cv->cd(1);
	hExpSiPM->Draw();
	hMCSiPM->Draw("hist,same");
	lg = new TLegend(0.6, 0.75, 0.9, 0.9);
	sprintf(str, "Exp * %6.4f", SiPMScale);
	lg->AddEntry(hExp, str, "pe");
	sprintf(str, "MC + %6.4f/#sqrt{E} #oplus %6.4f", SiPMRndmSqe, SiPMRndmC);
	lg->AddEntry(hMC, str, "l");
	lg->Draw();
	cv->cd(2);
	hExpPMT->Draw();
	hMCPMT->Draw("hist,same");
	lg = new TLegend(0.6, 0.75, 0.9, 0.9);
	sprintf(str, "Exp * %6.4f", PMTScale);
	lg->AddEntry(hExp, str, "pe");
	sprintf(str, "MC + %6.4f/#sqrt{E} #oplus %6.4f", PMTRndmSqe, PMTRndmC);
	lg->AddEntry(hMC, str, "l");
	lg->Draw();
	cv->SaveAs((oname+".pdf").Data());
	cv->SaveAs((oname+".pdf]").Data());
	
	TFile *fOut = new TFile((oname+".root").Data(), "RECREATE");
	fOut->cd();
	hExp->Write();
	hMC->Write();
	hExpSiPM->Write();
	hMCSiPM->Write();
	hExpPMT->Write();
	hMCPMT->Write();
	hExpT->Write();
	hRndm->Write();
	hRndmSiPM->Write();
	hRndmPMT->Write();
	hRndmT->Write();
	fOut->Close();
}

double chi2Diff(const TH1D *hA, const TH1D *hB, int binMin, int binMax)
{
	double sum;
	int i;
	for (i = binMin; i <= binMax; i++) sum += 
		(hA->GetBinContent(i) - hB->GetBinContent(i)) * (hA->GetBinContent(i) - hB->GetBinContent(i)) /
		(hA->GetBinError(i) * hA->GetBinError(i) + hB->GetBinError(i) * hB->GetBinError(i));
	return sum;
}

TH2D *makeScan(const char *pattern = "12B_78_rndm_%5.3f_scale_%5.3f.root", const char *suffix = "")
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
	char str[1024];
	int binMin, binMax;
	double chi2;
	
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
		binMin = hExp->FindBin(3.501);
		binMax = hExp->FindBin(12.999);
		hMC->Scale(hExp->Integral(binMin, binMax) / hMC->Integral(binMin, binMax));
		chi2 = chi2Diff(hExp, hMC, binMin, binMax);
		h2d->SetBinContent(i+1, j+1, chi2);
		f->Close();
		delete f;
	}
	
	sprintf(str, "12B-scan-%s.root", suffix);
	f = new TFile(str, "RECREATE");
	f->cd();
	h2d->Write();
	f->Close();
	
	return h2d;
}

// Draw SiPM to PMT comparison
void sipm2pmt(const char *fname)
{
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	TH1D *hExp12BSiPM = (TH1D *) f->Get("hExp12BSiPM");
	TH1D *hExp12BPMT  = (TH1D *) f->Get("hExp12BPMT");
	TH1D *hMC12BSiPM  = (TH1D *) f->Get("hMC12BSiPM");
	TH1D *hMC12BPMT   = (TH1D *) f->Get("hMC12BPMT");
	hMC12BPMT->Scale(hMC12BSiPM->Integral() / hMC12BPMT->Integral());
	hExp12BSiPM->SetLineColor(kRed);
	hExp12BPMT->SetLineColor(kBlue);
	hMC12BSiPM->SetLineColor(kRed);
	hMC12BPMT->SetLineColor(kBlue);
	hExp12BSiPM->SetLineWidth(2);
	hExp12BPMT->SetLineWidth(2);
	hMC12BSiPM->SetLineWidth(2);
	hMC12BPMT->SetLineWidth(2);
	hExp12BSiPM->SetTitle("^{12}B - Experiment, SiPM and PMT");
	hMC12BSiPM->SetTitle("^{12}B - MC, SiPM and PMT");
	
	TCanvas *cv = new TCanvas("CV", "CV", 1600, 1200);
	cv->Divide(2, 1);
	cv->cd(1);
	TLegend *lg = new TLegend(0.7, 0.85, 0.9, 0.93);
	lg->AddEntry(hExp12BSiPM, "SiPM", "l");
	lg->AddEntry(hExp12BPMT, "PMT", "l");
	hExp12BSiPM->Draw("hist");
	hExp12BPMT->Draw("hist,same");
	lg->Draw();
	cv->cd(2);
	hMC12BSiPM->Draw("hist");
	hMC12BPMT->Draw("hist,same");
	lg->Draw();
}
