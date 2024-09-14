/*
    A collection of functions for 12B calibration
*/
#include "../evtbuilder.h"
const int Nbins = 80;

/*
	Calculate chi2 of two histograms difference
*/
double chi2Diff(const TH1D *hA, const TH1D *hB, int binMin, int binMax)
{
	double sum;
	int i;
	sum = 0;
	for (i = binMin; i <= binMax; i++) sum += 
		(hA->GetBinContent(i) - hB->GetBinContent(i)) * (hA->GetBinContent(i) - hB->GetBinContent(i)) /
		(hA->GetBinError(i) * hA->GetBinError(i) + hB->GetBinError(i) * hB->GetBinError(i));
	return sum;
}

/*
	Create tree chain. file "stat_all.txt" is used for the valid raun selection.
	name - name of the chain to be created
	from - the first run
	to - the last run
	format - pattern for muon pair file names
*/
TChain *create_chain(const char *name, int from, int to, const char *format = "/home/clusters/rrcmpi/alekseev/igor/muon8n2/%3.3dxxx/muon_%6.6d.root")
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
		sprintf(str, format, i/1000, i);
		num = access(str, R_OK);	// R_OK = 4 - test read access
		if (num) continue;
		ch->AddFile(str, 0);
	}
	printf("%Ld entries found.\n", ch->GetEntries());
	
	free(rc_stat);
	
	return ch;
}

/*
	Make experimental histograms for 12B decay
	from - the first run
	to - the last run
	format - pattern for muon pair file names
*/
void src_12B(int from, int to, const char *format = "/home/clusters/rrcmpi/alekseev/igor/muon8n2/%3.3dxxx/muon_%6.6d.root")
{
	char str[1024];

	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);

	TChain *chA = create_chain("MuonPair", from, to, format);
	TChain *chR = create_chain("MuonRandom", from, to, format);
	if (!chA || !chR) return;

	sprintf(str, "Experiment with ^{12}B cuts, %s;MeV;Events", "ClusterEnergy");
	TH1D *hExp = new TH1D("hExp12B", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, %s;MeV;Events", "ClusterSiPMEnergy");
	TH1D *hExpSiPM = new TH1D("hExp12BSiPM", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, %s;MeV;Events", "ClusterPMTEnergy");
	TH1D *hExpPMT = new TH1D("hExp12BPMT", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, random, %s;MeV;Events", "ClusterEnergy");
	TH1D *hRndm = new TH1D("hRndm12B", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, random, %s;MeV;Events", "ClusterSiPMEnergy");
	TH1D *hRndmSiPM = new TH1D("hRndm12BSiPM", str, 80, 0, 20);
	sprintf(str, "Experiment with ^{12}B cuts, random, %s;MeV;Events", "ClusterPMTEnergy");
	TH1D *hRndmPMT = new TH1D("hRndm12BPMT", str, 80, 0, 20);
	TH1D *hExpT = new TH1D("hExp12BT", "Time from muon, experiment;ms;Events", 99, 1, 100);
	TH1D *hRndmT = new TH1D("hRndm12BT", "Time from muon, random;ms;Events", 99, 1, 100);

	chA->Project(hExp->GetName(), "ClusterEnergy", "gtDiff > 500 && OffClusterEnergy < 0.06");
	chR->Project(hRndm->GetName(), "ClusterEnergy", "gtDiff > 500 && OffClusterEnergy < 0.06");
	chA->Project(hExpSiPM->GetName(), "ClusterSiPmEnergy", "gtDiff > 500 && OffClusterEnergy < 0.06");
	chR->Project(hRndmSiPM->GetName(), "ClusterSiPmEnergy", "gtDiff > 500 && OffClusterEnergy < 0.06");
	chA->Project(hExpPMT->GetName(), "ClusterPmtEnergy", "gtDiff > 500 && OffClusterEnergy < 0.06");
	chR->Project(hRndmPMT->GetName(), "ClusterPmtEnergy", "gtDiff > 500 && OffClusterEnergy < 0.06");
	chA->Project(hExpT->GetName(), "gtDiff / 1000.0", "ClusterEnergy> 4 && OffClusterEnergy < 0.06");
	chR->Project(hRndmT->GetName(), "gtDiff / 1000.0", "ClusterEnergy> 4 && OffClusterEnergy < 0.06");
	
	hExp->Sumw2();
	hRndm->Sumw2();
	hExpSiPM->Sumw2();
	hRndmSiPM->Sumw2();
	hExpPMT->Sumw2();
	hRndmPMT->Sumw2();
	hExpT->Sumw2();
	hRndmT->Sumw2();
	
	hRndm->Scale(1.0/16);
	hRndmSiPM->Scale(1.0/16);
	hRndmPMT->Scale(1.0/16);
	hRndmT->Scale(1.0/16);
	
	TH1D *hDiff = (TH1D *) hExp->Clone("hDiff12B");
	TH1D *hDiffSiPM = (TH1D *) hExpSiPM->Clone("hDiff12BSiPM");
	TH1D *hDiffPMT = (TH1D *) hExpPMT->Clone("hDiff12BPMT");
	TH1D *hDiffT = (TH1D *) hExpT->Clone("hDiff12BT");
	hDiff->SetTitle("Cluster energy for ^{12}B decay;MeV;Events");
	hDiffSiPM->SetTitle("Cluster energy for ^{12}B decay, SiPM;MeV;Events");
	hDiffPMT->SetTitle("Cluster energy for ^{12}B decay, PMT;MeV;Events");
	hDiffT->SetTitle("Time from muon event;ms;Events");
	
	hDiff->Add(hRndm, -1.0);
	hDiffSiPM->Add(hRndmSiPM, -1.0);
	hDiffPMT->Add(hRndmPMT, -1.0);
	hDiffT->Add(hRndmT, -1.0);
	
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
	hExpT->SetMarkerColor(kRed);
	hExpT->SetLineColor(kRed);

	hRndm->SetMarkerStyle(kOpenCircle);
	hRndm->SetMarkerColor(kGreen);
	hRndm->SetLineColor(kGreen);
	hRndmSiPM->SetMarkerStyle(kOpenCircle);
	hRndmSiPM->SetMarkerColor(kGreen);
	hRndmSiPM->SetLineColor(kGreen);
	hRndmPMT->SetMarkerStyle(kOpenCircle);
	hRndmPMT->SetMarkerColor(kGreen);
	hRndmPMT->SetLineColor(kGreen);
	hRndmT->SetMarkerStyle(kOpenCircle);
	hRndmT->SetMarkerColor(kGreen);
	hRndmT->SetLineColor(kGreen);

	hDiff->SetMarkerStyle(kFullSquare);
	hDiff->SetMarkerColor(kBlue);
	hDiff->SetLineColor(kBlue);
	hDiffSiPM->SetMarkerStyle(kFullSquare);
	hDiffSiPM->SetMarkerColor(kBlue);
	hDiffSiPM->SetLineColor(kBlue);
	hDiffPMT->SetMarkerStyle(kFullSquare);
	hDiffPMT->SetMarkerColor(kBlue);
	hDiffPMT->SetLineColor(kBlue);
	hDiffT->SetMarkerStyle(kFullSquare);
	hDiffT->SetMarkerColor(kBlue);
	hDiffT->SetLineColor(kBlue);
	
	TCanvas *cv = new TCanvas("CV", "12B", 1400, 1000);
	sprintf(str, "12B_exp82_%d_%d", from, to);
	TString oname(str);

	cv->Divide(2, 2);
	cv->cd(1);
	hExp->Draw();
	hRndm->Draw("same");
	hDiff->Draw("same");
	TLegend *lg = new TLegend(0.6, 0.7, 0.85, 0.85);
	lg->AddEntry(hExp, "All", "pe");
	lg->AddEntry(hRndm, "Acc. bgnd.", "pe");
	lg->AddEntry(hDiff, "Signal", "pe");
	lg->Draw();
	cv->cd(2);
	hExpSiPM->Draw();
	hRndmSiPM->Draw("same");
	hDiffSiPM->Draw("same");
	lg->Draw();
	cv->cd(3);
	hExpPMT->Draw();
	hRndmPMT->Draw("same");
	hDiffPMT->Draw("same");
	lg->Draw();
	cv->cd(4);
	hExpT->SetMinimum(0);
	hExpT->Draw();
	hRndmT->Draw("same");
	TF1 *fExpo = new TF1("fExpo", "[0]*exp(-x/[1])", 0, 100);
	fExpo->SetParNames("Const.", "#tau");
	fExpo->SetParameters(100, 20);
	hDiffT->Fit(fExpo, "", "sames");
	cv->SaveAs((oname+".pdf").Data());
	
	TFile *fOut = new TFile((oname+".root").Data(), "RECREATE");
	fOut->cd();
	hExp->Write();
	hExpSiPM->Write();
	hExpPMT->Write();
	hExpT->Write();
	hRndm->Write();
	hRndmSiPM->Write();
	hRndmPMT->Write();
	hRndmT->Write();
	hDiff->Write();
	hDiffSiPM->Write();
	hDiffPMT->Write();
	hDiffT->Write();
	fOut->Close();
}

/*
	Make MC histograms for 12B decay. A matrix over scale and shift is createed.
	scale [0.9; 1.0] with step 0.005
	shift [-0.15; 0.15] with step 0.025 MeV		no shift
	mcname - file(s) with MC generated 12B decays
*/
void src_12BMC(const char *mcname, const char *mchist)
{
	char strs[256], strl[1024];
	int i, j;
	double scale, shift;
	char *ptr;
	char *mclist;

	mclist = strdup(mcname);
	ptr = strtok(mclist, " \t,");
	TChain *tMC = new TChain("DanssEvent", "DanssEvent");
	for(;;) {
		if (!ptr || !strlen(ptr)) break;
		tMC->AddFile(ptr);
		ptr = strtok(NULL, " \t,");
	}
	if (!tMC->GetEntries()) {
		printf("Bad file(s) %s\n", mcname);
		return;
	}
	
	TFile *fOut = new TFile(mchist, "RECREATE");
	if (!fOut->IsOpen()) return;

	TH1D *hMCT = new TH1D("hMC12BT", "Time from muon, MC;ms;Events", 99, 1, 100);
	tMC->Project(hMCT->GetName(), "TimelineShift / 1000000.0", "AnnihilationEnergy < 0.25 && PositronEnergy > 4.0");
	hMCT->Sumw2();
	hMCT->Write();
	j = 6;
	for (i=0; i<41; i++) {
		scale = 0.9 + 0.005*i;
		shift = -0.15 + 0.025 * j;
		
		sprintf(strs, "hMC12B_%2.2d_%2.2d", i, j);
		sprintf(strl, "MC of ^{12}B decay, E_{MC}=%5.3f*E+(%6.3f), SiPM+PMT;MeV;Events", scale, shift);
		TH1D *hMC = new TH1D(strs, strl, 80, 0, 20);
		sprintf(strs, "hMC12BSiPM_%2.2d_%2.2d", i, j);
		sprintf(strl, "MC of ^{12}B decay, E_{MC}=%5.3f*E+(%6.3f), SiPM;MeV;Events", scale, shift);
		TH1D *hMCSiPM = new TH1D(strs, strl, 80, 0, 20);
		sprintf(strs, "hMC12BPMT_%2.2d_%2.2d", i, j);
		sprintf(strl, "MC of ^{12}B decay, E_{MC}=%5.3f*E+(%6.3f), PMT;MeV;Events", scale, shift);
		TH1D *hMCPMT = new TH1D(strs, strl, 80, 0, 20);
		
		sprintf(strs, "PositronEnergy*%5.3f + (%6.4f)", scale, shift);
		tMC->Project(hMC->GetName(), strs, "AnnihilationEnergy < 0.06 && PositronEnergy > 3.0");
		sprintf(strs, "PositronSiPmEnergy*%5.3f + (%6.4f)", scale, shift);
		tMC->Project(hMCSiPM->GetName(), strs, "AnnihilationEnergy < 0.06 && PositronEnergy > 3.0");
		sprintf(strs, "PositronPmtEnergy*%5.3f + (%6.4f)", scale, shift);
		tMC->Project(hMCPMT->GetName(), strs, "AnnihilationEnergy < 0.06 && PositronEnergy > 3.0");

		hMC->Sumw2();
		hMCSiPM->Sumw2();
		hMCPMT->Sumw2();
	
		hMC->Write();
		hMCSiPM->Write();
		hMCPMT->Write();
	}
	fOut->Close();
}

/*
	Draw a comparison of the experimental and MC histgrams.
	expname - file with experimental histograms
	mcname - file with MC histograms
	iScale - scale = 0.9 + 0.005 * iScale
	iShift - shift = -0.15 + 0.025 * iShift
*/
void draw_12B(const char *expname, const char *mcname, int iScale = 20, int iShift = 6)
{
	char str[256];
	double scale = 0.9 + 0.005 * iScale;
	double shift = -0.15 + 0.025 * iShift;
	
	TFile *fExp = new TFile(expname);
	TFile *fMC  = new TFile(mcname);
	if (!fExp->IsOpen() || !fMC->IsOpen()) return;
	
	TH1D *hExp = (TH1D *) fExp->Get("hDiff12B");
	TH1D *hExpSiPM = (TH1D *) fExp->Get("hDiff12BSiPM");
	TH1D *hExpPMT = (TH1D *) fExp->Get("hDiff12BPMT");
	TH1D *hExpT = (TH1D *) fExp->Get("hDiff12BT");
	sprintf(str, "hMC12B_%2.2d_%2.2d", iScale, iShift);
	TH1D *hMC = (TH1D *) fMC->Get(str);
	sprintf(str, "hMC12BSiPM_%2.2d_%2.2d", iScale, iShift);
	TH1D *hMCSiPM = (TH1D *) fMC->Get(str);
	sprintf(str, "hMC12BPMT_%2.2d_%2.2d", iScale, iShift);
	TH1D *hMCPMT = (TH1D *) fMC->Get(str);
	TH1D *hMCT = (TH1D *) fMC->Get("hMC12BT");
	if (!hExp || !hMC || !hExpSiPM || !hMCSiPM || !hExpPMT || !hMCPMT || !hExpT || !hMCT) {
		printf("Not all histograms found in %s and %s.\n", expname, mcname);
		return;
	}

	sprintf(str, "^{12}B decay, ClusterEnergy E_{MC}=%5.3f*E+(%6.3f), SiPM+PMT", scale, shift);
	hExp->SetTitle(str);
	sprintf(str, "^{12}B decay, ClusterEnergy E_{MC}=%5.3f*E+(%6.3f), SiPM", scale, shift);
	hExpSiPM->SetTitle(str);
	sprintf(str, "^{12}B decay, ClusterEnergy E_{MC}=%5.3f*E+(%6.3f), PMT", scale, shift);
	hExpPMT->SetTitle(str);
	hExpT->SetTitle("^{12}B decay time");

	hExp->SetLineWidth(2);
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetLineColor(kRed);
	hExp->SetMarkerColor(kRed);
	hExpSiPM->SetLineWidth(2);
	hExpSiPM->SetMarkerStyle(kFullCircle);
	hExpSiPM->SetLineColor(kRed);
	hExpSiPM->SetMarkerColor(kRed);
	hExpPMT->SetLineWidth(2);
	hExpPMT->SetMarkerStyle(kFullCircle);
	hExpPMT->SetLineColor(kRed);
	hExpPMT->SetMarkerColor(kRed);
	hExpT->SetLineWidth(2);
	hExpT->SetMarkerStyle(kFullCircle);
	hExpT->SetLineColor(kRed);
	hExpT->SetMarkerColor(kRed);
	hMC->SetLineWidth(2);
	hMC->SetLineColor(kBlue);
	hMCSiPM->SetLineWidth(2);
	hMCSiPM->SetLineColor(kBlue);
	hMCPMT->SetLineWidth(2);
	hMCPMT->SetLineColor(kBlue);
	hMCT->SetLineWidth(2);
	hMCT->SetLineColor(kBlue);

	hMC->Scale(hExp->Integral(17, 48) / hMC->Integral(17, 48));
	hMCSiPM->Scale(hExpSiPM->Integral(17, 48) / hMCSiPM->Integral(17, 48));
	hMCPMT->Scale(hExpPMT->Integral(17, 48) / hMCPMT->Integral(17, 48));
	hMCT->Scale(hExpT->Integral(2, 80) / hMCT->Integral(2, 80));

	TF1 *fExpo = new TF1("fExpo", "[0]*exp(-x/[1])", 0, 100);
	fExpo->SetParNames("Const.", "#tau");
	fExpo->SetParameters(100, 20);
	fExpo->SetLineColor(kBlack);
	
	gStyle->SetOptFit();
	gStyle->SetOptStat(0);
	
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	cv->Clear();
	cv->Divide(2, 2);
	
	cv->cd(1);
	hExp->DrawCopy();
	hMC->DrawCopy("hist,same");
	TLegend *lg = new TLegend(0.65, 0.55, 0.89, 0.7);
	lg->AddEntry(hExp, "Data", "pe");
	lg->AddEntry(hMC, "MC", "l");
	lg->Draw();
	cv->cd(2);
	hExpSiPM->DrawCopy();
	hMCSiPM->DrawCopy("hist,same");
	lg->Draw();
	cv->cd(3);
	hExpPMT->DrawCopy();
	hMCPMT->DrawCopy("hist,same");
	lg->Draw();
	cv->cd(4);
	hExpT->Fit(fExpo, "q");
	hExpT->DrawCopy();
	hMCT->DrawCopy("hist,same");
	TLegend *lg1 = new TLegend(0.65, 0.55, 0.89, 0.75);
	lg1->AddEntry(hExp, "Data", "pe");
	lg1->AddEntry(hMC, "MC", "l");
	lg1->AddEntry(fExpo, "Fit to Data", "l");
	lg1->Draw();
	
	sprintf(str, "12B_82_exp2mc_scale_%5.3f_shift_%6.3f.png", scale, shift);
	cv->SaveAs(str);
	fExp->Close();
	fMC->Close();
}

/*
	Scan the difference between the experimental and MC histgrams.
	expname - file with experimental histograms
	mcname - file with MC histograms
*/
void scan_12B(const char *expname, const char *mcname)
{
	char str[256];
	TH1D *hMC;
	TH1D *hMCSiPM;
	TH1D *hMCPMT;
	int iMinX, iMinY, iMinXSiPM, iMinYSiPM, iMinXPMT, iMinYPMT, dummy;
	int i, j;
	double xmin;
	TLatex txt;
	
	TH2D *hScan = new TH2D("hScan12B", "#chi^{2} difference between MC and data, ^{12}B, SiPM+PMT;Scale;Shift;#chi^{2}", 
		41, 0.9 - 0.0025, 1.1 + 0.0025, 13, -0.15 - 0.0125, 0.15 + 0.0125);
	TH2D *hScanSiPM = new TH2D("hScan12BSiPM", "#chi^{2} difference between MC and data, ^{12}B, SiPM;Scale;Shift;#chi^{2}", 
		41, 0.9 - 0.0025, 1.1 + 0.0025, 13, -0.15 - 0.0125, 0.15 + 0.0125);
	TH2D *hScanPMT = new TH2D("hScan12BPMT", "#chi^{2} difference between MC and data, ^{12}B, PMT;Scale;Shift;#chi^{2}", 
		41, 0.9 - 0.0025, 1.1 + 0.0025, 13, -0.15 - 0.0125, 0.15 + 0.0125);
	
	TFile *fExp = new TFile(expname);
	TFile *fMC  = new TFile(mcname);
	if (!fExp->IsOpen() || !fMC->IsOpen()) return;
	
	TH1D *hExp = (TH1D *) fExp->Get("hDiff12B");
	TH1D *hExpSiPM = (TH1D *) fExp->Get("hDiff12BSiPM");
	TH1D *hExpPMT = (TH1D *) fExp->Get("hDiff12BPMT");
	TH1D *hExpT = (TH1D *) fExp->Get("hDiff12BT");
	if (!hExp || !hExpSiPM || !hExpPMT) {
		printf("Not all histograms found in %s.\n", expname);
		return;
	}
	j = 6;	// no shift
	for (i=0; i<41; i++) {
		sprintf(str, "hMC12B_%2.2d_%2.2d", i, j);
		hMC = (TH1D *) fMC->Get(str);
		sprintf(str, "hMC12BSiPM_%2.2d_%2.2d", i, j);
		hMCSiPM = (TH1D *) fMC->Get(str);
		sprintf(str, "hMC12BPMT_%2.2d_%2.2d", i, j);
		hMCPMT = (TH1D *) fMC->Get(str);
		if (!hMC || !hMCSiPM || !hMCPMT) {
			printf("Not all histograms found in %s.\n", mcname);
			return;
		}
		hMC->Scale(hExp->Integral(17, 48) / hMC->Integral(17, 48));
		hMCSiPM->Scale(hExpSiPM->Integral(17, 48) / hMCSiPM->Integral(17, 48));
		hMCPMT->Scale(hExpPMT->Integral(17, 48) / hMCPMT->Integral(17, 48));
		hScan->SetBinContent(i+1, j+1, chi2Diff(hExp, hMC, 17, 48));
		hScanSiPM->SetBinContent(i+1, j+1, chi2Diff(hExpSiPM, hMCSiPM, 17, 48));
		hScanPMT->SetBinContent(i+1, j+1, chi2Diff(hExpPMT, hMCPMT, 17, 48));
	}
	
	hScan->GetMinimumBin(iMinX, iMinY, dummy);
	hScanSiPM->GetMinimumBin(iMinXSiPM, iMinYSiPM, dummy);
	hScanPMT->GetMinimumBin(iMinXPMT, iMinYPMT, dummy);
	
	iMinX = iMinXSiPM = iMinXPMT = 21;
	iMinY = iMinYSiPM = iMinYPMT = 7;
	
	TF1 *fpol2 = new TF1("fpol2", "pol2", -10, 10);
	
	gStyle->SetOptStat(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	cv->Clear();
	cv->Divide(3, 3);
	cv->cd(1);
	hScan->DrawCopy("colorz");
	cv->cd(2);
	hScanSiPM->DrawCopy("colorz");
	cv->cd(3);
	hScanPMT->DrawCopy("colorz");
	cv->cd(4);
	TH1D *hScanX = hScan->ProjectionX("_px", iMinY, iMinY);
	hScanX->SetTitle("Profile at Shift = 0");
	hScanX->Fit(fpol2, "q");
	hScanX->DrawCopy();
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);
	cv->cd(5);
	TH1D *hScanSiPMX = hScanSiPM->ProjectionX("_px", iMinYSiPM, iMinYSiPM);
	hScanSiPMX->SetTitle("Profile at Shift = 0");
	hScanSiPMX->Fit(fpol2, "q");
	hScanSiPMX->DrawCopy();
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);
	cv->cd(6);
	TH1D *hScanPMTX = hScanPMT->ProjectionX("_px", iMinYPMT, iMinYPMT);
	hScanPMTX->SetTitle("Profile at Shift = 0");
	hScanPMTX->Fit(fpol2, "q");
	hScanPMTX->DrawCopy();
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Scale=%5.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);
	cv->cd(7);
	TH1D *hScanY = hScan->ProjectionY("_py", iMinX, iMinX);
	hScanY->SetTitle("Profile at Scale = 1");
	hScanY->Fit(fpol2, "q");
	hScanY->DrawCopy();
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Shift=%6.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);
	cv->cd(8);
	TH1D *hScanSiPMY = hScanSiPM->ProjectionY("_py", iMinXSiPM, iMinXSiPM);
	hScanSiPMY->SetTitle("Profile at Scale = 1");
	hScanSiPMY->Fit(fpol2, "q");
	hScanSiPMY->DrawCopy();
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Shift=%6.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);
	cv->cd(9);
	TH1D *hScanPMTY = hScanPMT->ProjectionY("_py", iMinXPMT, iMinXPMT);
	hScanPMTY->SetTitle("Profile at Scale = 1");
	hScanPMTY->Fit(fpol2, "q");
	hScanPMTY->DrawCopy();
	xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
	sprintf(str, "Shift=%6.3f", xmin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#Chi^{2}_{min}=%6.1f", fpol2->Eval(xmin));
	txt.DrawLatexNDC(0.4, 0.72, str);
	TString resname(mcname);
	resname.ReplaceAll(".root", "");
	
	cv->SaveAs((resname + "-scan.png").Data());
	
	TFile *fOut = new TFile((resname + "-scan.root").Data(), "RECREATE");
	if (!fOut->IsOpen()) return;
	hScan->Write();
	hScanSiPM->Write();
	hScanPMT->Write();
	fOut->Close();
	fExp->Close();
	fMC->Close();
}

/*
	Scan the difference between the experimental and MC histgrams in several energy bins
	expname - file with experimental histograms
	mcname - file with MC histograms
*/
void scanm_12B(const char *expname, const char *mcname, int nBins = 4)
{
	const int firstBin = 13;	// 3 Mev
//	const int iWidth = 8;		// 2 MeV
//	const int nBins = 4;		// 3 - 11 MeV
//	const int iWidth = 6;		// 1.5 MeV
//	const int nBins = 6;		// 3 - 12 MeV
	const int iShift = 6;		// no shift
	char str[256], strl[1024];
	TH1D *hMC[41];
	int iMinX, iLeft, iRight;
	int i, j, k;
	double xmin, diff, err, err2;
	TLatex txt;
	TLine ln;
	TString pname(mcname);
	TH1D *hScan[nBins];
	int iWidth;
	
	pname.ReplaceAll(".root", "");
	pname += "-scan";
	if (nBins == 4) {
		iWidth = 8;
		pname += "4";
	} else if (nBins == 6) {
		iWidth = 6;
		pname += "6";
	} else {
		printf("Bad nBins: only 4 and 6 are supported\n");
		return;
	}
	
	for (i = 0; i < nBins; i++) {
		sprintf(str, "hScan12B%d", i);
//		sprintf(strl, "#chi^{2} difference between MC and data, ^{12}B, SiPM+PMT for E = [%4.1f - %4.4f] MeV;Scale;#chi^{2}",
		sprintf(strl, "SiPM+PMT, E = [%4.1f - %4.4f] MeV;Scale;#chi^{2}",
			(firstBin + i * iWidth - 1) * 0.25, (firstBin + (i + 1) * iWidth - 1) * 0.25);
		hScan[i] = new TH1D(str, strl, 41, 0.9 - 0.0025, 1.1 + 0.0025);
	}
	TFile *fExp = new TFile(expname);
	TFile *fMC  = new TFile(mcname);
	if (!fExp->IsOpen() || !fMC->IsOpen()) return;
	
	TH1D *hExp = (TH1D *) fExp->Get("hDiff12B");
	if (!hExp) {
		printf("Not all histograms found in %s.\n", expname);
		return;
	}
	for (i=0; i<41; i++) {
		sprintf(str, "hMC12B_%2.2d_%2.2d", i, iShift);
		hMC[i] = (TH1D *) fMC->Get(str);
		if (!hMC[i]) {
			printf("Not all histograms found in %s.\n", mcname);
			return;
		}
		hMC[i]->Scale(hExp->Integral(13, 80) / hMC[i]->Integral(13, 80));
		for (j = 0; j < nBins; j++) hScan[j]->SetBinContent(i + 1, chi2Diff(hExp, hMC[i], firstBin + j * iWidth, firstBin + (j + 1) * iWidth - 1));
	}
	
	TF1 *fpol2 = new TF1("fpol2", "pol2", -10, 10);
	TH1D *hMCScaled = (TH1D*) hExp->Clone("hMCScaled");	// MC with different scale in bins
	hMCScaled->Reset();
	hMCScaled->SetLineWidth(2);
	hMCScaled->SetLineColor(kBlue);
	TH1D *hPulls = (TH1D*) hExp->Clone("hPulls");	// Pulls
	hPulls->Reset();
	hPulls->SetLineWidth(2);
	hPulls->SetLineColor(kBlue);
	hPulls->SetTitle("Pulls;MeV;(#Delta/#sigma)^{2}");
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	TCanvas *cv = (TCanvas *) gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1400, 1000);
	cv->Clear();
	cv->Divide((nBins + 1) / 2, 2);
	for (i = 0; i < nBins; i++) {
		cv->cd(i + 1);
		iMinX = hScan[i]->GetMinimumBin();
		iLeft = iMinX - 10;
		if (iLeft < 1) iLeft = 1;
		iRight = iMinX + 10;
		if (iRight > 41) iRight = 41;
		hScan[i]->Fit(fpol2, "", "", hScan[i]->GetBinCenter(iLeft), hScan[i]->GetBinCenter(iRight));
		xmin = -fpol2->GetParameter(1) / (2 * fpol2->GetParameter(2));
		err = 1 / sqrt(fpol2->GetParameter(2));
		sprintf(str, "Scale=%5.3f #pm %5.3f", xmin, err);
		txt.DrawLatexNDC(0.2, 0.8, str);
		sprintf(str, "#Chi^{2}_{min} / n.d.f.=%6.1f / %d", fpol2->Eval(xmin), iWidth - 1);
		txt.DrawLatexNDC(0.2, 0.72, str);
		// Get chunk-scaled histogram
		k = (xmin - 0.9 + 0.0025) / 0.005;
		if (k < 0) k = 0;
		if (k > 40) k = 40;
		printf("xmin = %f ==> k = %d\n", xmin, k);
		for (j = firstBin + i * iWidth; j < firstBin + (i + 1) * iWidth; j++) {
			hMCScaled->SetBinContent(j, hMC[k]->GetBinContent(j));
			hMCScaled->SetBinError(j, hMC[k]->GetBinError(j));
		}
	}
	cv->SaveAs((pname + ".pdf[").Data());
	cv->SaveAs((pname + ".pdf").Data());
	cv->Clear();
	cv->Divide(1, 2);
	cv->cd(1);
	hExp->SetLineColor(kRed);
	hExp->SetMarkerColor(kRed);
	hExp->Draw();
	hMCScaled->Draw("same,hist");
	ln.SetLineColor(kMagenta);
//	ln.SetLineWidth(2);
	for (i = 0; i <= nBins; i++) ln.DrawLine((firstBin + i * iWidth - 1) * 0.25, 0, (firstBin + i * iWidth - 1) * 0.25, 1.1 * hExp->GetMaximum());
	cv->cd(2);
	//	Calculate and draw pulls
	for (i = firstBin; i < firstBin + nBins * iWidth; i++) {
		diff = hExp->GetBinContent(i) - hMCScaled->GetBinContent(i);
		diff *= fabs(diff);
		err2 = hExp->GetBinError(i) * hExp->GetBinError(i) + hMCScaled->GetBinError(i) * hMCScaled->GetBinError(i);
		hPulls->SetBinContent(i, diff / err2);
	}
	hPulls->Draw();
	for (i = 0; i <= nBins; i++) ln.DrawLine((firstBin + i * iWidth - 1) * 0.25, -4, (firstBin + i * iWidth - 1) * 0.25, 4);
	cv->SaveAs((pname + ".pdf").Data());
	cv->SaveAs((pname + ".pdf]").Data());
	TFile *fOut = new TFile((pname+".root").Data(), "RECREATE");
	if (!fOut->IsOpen()) return;
	for (i = 0; i < nBins; i++) hScan[i]->Write();
	hMCScaled->Write();
	hPulls->Write();
	fOut->Close();
	fExp->Close();
	fMC->Close();
}

/********************************************************************************
 *	Calculate MC matrix converting from true to MC energy			*
 *	mcname - path to processed MC root file or space separated files	*
 *	what - variable name in the processed file				*
 *	cuts - cuts to be applied						*
 *	mcorig - path to original MC file(s)					*
 *	MC response matrix is returned						*
 ********************************************************************************/
TMatrixD *MCmatrix(const char *mcname, const char *what, const char *cuts, const char *mcorig)
{
	char *ptr;
	char *mclist;
	char str[1024];
	int i, j;
	double sum;

	printf("\t=======\t\n%s\n%s\n ==> %s [%s]\n", mcname, mcorig, what, cuts);

	mclist = strdup(mcname);
	ptr = strtok(mclist, " \t,");
	TChain *tMC = new TChain("DanssEvent", "DanssEvent");
	for(;;) {
		if (!ptr || !strlen(ptr)) break;
		tMC->AddFile(ptr);
		ptr = strtok(NULL, " \t,");
	}
	if (!tMC->GetEntries()) {
		printf("Bad file(s) %s\n", mcname);
		return NULL;
	}
	free(mclist);
	
	mclist = strdup(mcorig);
	ptr = strtok(mclist, " \t,");
	TChain *tMCorig = new TChain("DANSSEvent", "DANSSEvent");
	for(;;) {
		if (!ptr || !strlen(ptr)) break;
		tMCorig->AddFile(ptr);
		ptr = strtok(NULL, " \t,");
	}
	if (!tMCorig->GetEntries()) {
		printf("Bad file(s) %s\n", mcorig);
		return NULL;
	}
	free(mclist);
	
	// Project
	TH1D *h1 = new TH1D("__htmp1", "", Nbins, 0, 20);
	TH2D *h2 = new TH2D("__htmp2", "", Nbins, 0, 20, Nbins, 0, 20);
	sprintf(str, "%s:ParticleEnergy", what);
	tMC->Project(h2->GetName(), str, cuts);
	tMCorig->Project(h1->GetName(), "ParticleEnergy");
	
	// Extract to matrix
	TMatrixD *M = new TMatrixD(Nbins, Nbins);
	for (i=0; i<Nbins; i++) for (j=0; j<Nbins; j++) (*M)[j][i] = h2->GetBinContent(i+1, j+1);
	
	// Normalize
	for (i=0; i<Nbins; i++) {
		sum = h1->GetBinContent(i+1);
		if (sum > 0) for (j=0; j<Nbins; j++) (*M)[j][i] /= sum;
	}
	
	delete tMC;
	delete tMCorig;
	delete h2;
	delete h1;
	
	return M;
}

/************************************************
 *	Get vector of the original MC spectrum	*
 *	mcorig - path to original MC file(s)	*
 *	spectrum vector is returned		*
 ************************************************/
TVectorD *MCvector(const char *mcorig)
{
	char *ptr;
	char *mclist;
	int i;

	mclist = strdup(mcorig);
	ptr = strtok(mclist, " \t,");
	TChain *tMCorig = new TChain("DANSSEvent", "DANSSEvent");
	for(;;) {
		if (!ptr || !strlen(ptr)) break;
		tMCorig->AddFile(ptr);
		ptr = strtok(NULL, " \t,");
	}
	if (!tMCorig->GetEntries()) {
		printf("Bad file(s) %s\n", mcorig);
		return NULL;
	}
	free(mclist);
	
	// Project
	TH1D *h1 = new TH1D("__htmp1", "", Nbins, 0, 20);
	tMCorig->Project(h1->GetName(), "ParticleEnergy");
	// Extract to vector
	TVectorD *S = new TVectorD(Nbins);
	for (i=0; i<Nbins; i++) (*S)[i] = h1->GetBinContent(i+1);

	delete tMCorig;
	delete h1;
	
	return S;
}

/****************************************
 *	Make matrixes for Chikuma	*
 ****************************************/
void MakeChikumaMatrixes(void)
{
	const char *ChikumaDir = "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Chikuma/12B";
	const char *MCRawDir = "/home/clusters/rrcmpi/danss/MC_RAW/Chikuma/12B";
	const char *mcnames[] = {
		"DB_spectrum_Chikuma", 
		"DB_spectrum_Chikuma_Birks_el_0_0108", 
		"DB_spectrum_Chikuma_Birks_el_0_0308", 
		"DB_spectrum_Chikuma_Cher_coeff_0_033",
		"DB_spectrum_Chikuma_Cher_coeff_0_233",
		"DB_spectrum_Chikuma_main_Birks_0_0108", 
		"DB_spectrum_Chikuma_main_Birks_0_0308"};
	const char *filelist[] = {"mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum1.root", "mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum2.root"};
	const char *rawlist[] = {"DANSSmod0_1.root", "DANSSmod0_2.root"};
	const char *varlist[] = {"PositronEnergy", "PositronSiPmEnergy", "PositronPmtEnergy"};
	const char *mixname[] = {"Birks_el", "Cher_coeff", "main_Birks"};
	int i, j;
	char str[4096];
	char raw[4096];
	char name[256];
	TMatrixD *M[13][3];
	
	TFile *fOut = new TFile("12B_Chikuma_matrixes.root", "RECREATE");
	if (!fOut->IsOpen()) return;
	
	for (i = 0; i < sizeof(mcnames) / sizeof(mcnames[0]); i++) {
		str[0] = '\0';
		for (j = 0; j < sizeof(filelist) / sizeof(filelist[0]); j++) sprintf(&str[strlen(str)], "%s/%s/%s ", ChikumaDir, mcnames[i], filelist[j]);
		raw[0] = '\0';
		for (j = 0; j < sizeof(rawlist) / sizeof(rawlist[0]); j++) sprintf(&raw[strlen(raw)], "%s/%s/Ready/%s ", MCRawDir, mcnames[i], rawlist[j]);
		for (j = 0; j < sizeof(varlist) / sizeof(varlist[0]); j++) {
			M[i][j] = MCmatrix(str, varlist[j], "AnnihilationEnergy < 0.06 && PositronEnergy > 3", raw);
			if (!M[i][j]) continue;
			sprintf(name, "m12B_%s_%s", mcnames[i], varlist[j]);
			fOut->cd();
			M[i][j]->Write(name);
		}
	}
	for (j = 0; j < sizeof(varlist) / sizeof(varlist[0]); j++) {
		for (i=7; i<13; i++) M[i][j] = new TMatrixD(Nbins, Nbins);
		*(M[7][j]) = (*(M[2][j]) - *(M[1][j])) * 0.5;
		*(M[8][j]) = (*(M[2][j]) + *(M[1][j])) * 0.5 - *(M[0][j]);
		*(M[9][j]) = (*(M[4][j]) - *(M[3][j])) * 0.5;
		*(M[10][j]) = (*(M[4][j]) + *(M[3][j])) * 0.5 - *(M[0][j]);
		*(M[11][j]) = (*(M[6][j]) - *(M[5][j])) * 0.5;
		*(M[12][j]) = (*(M[6][j]) + *(M[5][j])) * 0.5 - *(M[0][j]);
		for (i=0; i<6; i++) {
			sprintf(name, "m12B_%s_%s_%s", (i & 1) ? "Residual" : "Delta", mixname[i/2], varlist[j]);
			fOut->cd();
			M[7+i][j]->Write(name);
		}
	}
	
	raw[0] = '\0';
	for (j = 0; j < sizeof(rawlist) / sizeof(rawlist[0]); j++) sprintf(&raw[strlen(raw)], "%s/%s/Ready/%s ", MCRawDir, mcnames[0], rawlist[j]);
	auto S = MCvector(raw);
	fOut->cd();
	S->Write("v12B_ParticleEnergy");

	fOut->Close();
}

/****************************************************************
 *	To increase speed of calculations an experimental tree	*
 *	with only three energies is created			*
 *	from - the first run					*
 *	to - the last run					*
 ****************************************************************/
void MakeShortTree(int from, int to)
{
	const char *format = "/home/clusters/rrcmpi/alekseev/igor/muon8n2/%3.3dxxx/muon_%6.6d.root";
	char str[1024];
	struct DanssMuonStruct event;
	const char *LeafList = 
		"ClusterEnergy/F:"	// Energy sum of the cluster (SiPM)
		"ClusterSiPmEnergy/F:"	// SiPM energy in the cluster, corrected
		"ClusterPmtEnergy/F";	// PMT energy in the cluster, corrected
	long i, N;

	TChain *chA = create_chain("MuonPair", from, to, format);
	TChain *chR = create_chain("MuonRandom", from, to, format);

	chA->SetBranchAddress("Pair", &event);
	chR->SetBranchAddress("Pair", &event);
	
	sprintf(str, "12B_events_%6.6d_%6.6d.root", from, to);
	TFile *fOut = new TFile(str, "RECREATE");
	TTree *tOut = new TTree("MuonEnergy", "MuonEnergy");
	tOut->Branch("Pair", &event.ClusterEnergy, LeafList);
	TTree *tOutR = new TTree("MuonEnergy", "MuonEnergy");
	tOutR->Branch("Pair", &event.ClusterEnergy, LeafList);
	
	N = chA->GetEntries();
	for (i=0; i<N; i++) {
		chA->GetEntry(i);
		if (event.dtDiff < 500) continue;	// cut at 500 us
		if (event.ClusterEnergy < 3) continue;	// cut at 3 MeV
		tOut->Fill();
	}

	N = chB->GetEntries();
	for (i=0; i<N; i++) {
		chB->GetEntry(i);
		if (event.dtDiff < 500) continue;	// cut at 500 us
		if (event.ClusterEnergy < 3) continue;	// cut at 3 MeV
		tOutR->Fill();
	}
	
	fOut->cd();
	tOut->Write();
	tOutR->Write();
	fOut->Close();
}

/****************************************************************
 *	Try to fit MC Kbirks and Kcher to experiment		*
 *	expname - file with experimental compressed tree	*
 *	mcmatrixes - file with MC matrixes			*
 ****************************************************************
 *	Three free parameters are used:				*
 *	Kscale - Overall scale					*
 *	Kbirks - Birks coaefficient for electrons		*
 *	Kcher - Cherenkov coefficient				*
 *	The experimental distribution is obtained by scaling	*
 *	energy with Kscale					*
 *	MC distribution is obtained using detector respond	*
 *	matrixes in linear approximation for variables		*
 *	We use only experimental errors				*
 ****************************************************************/
void FitMCparameters(const char *expname, const char *mcmatrixes)
{
}
