#include <stdio.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TCut.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLegend.h>
#include <TLeaf.h>
#include <TLine.h>
#include <TRandom2.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TText.h>

/***************************************************************************************
*			Runs with sources
* Runs		Source	Date		Position	Nruns
* 12198-12303	none	2017-02-03
* 12306-12346	60Co	2017-02-04	center		41
* 12348-12361	60Co	2017-02-05	edge		14
* 12364-12373	22Na	2017-02-05	edge		10
* 12376-12407	22Na	2017-02-05	center		32
* 12411-12547	none	2017-02-06
* 50750-50873	none	2018-11-18
* 50949-50997	60Co	2018-11-20	center		49
* 50999-51034	60Co	2018-11-21	edge		36
* 51036-51095	22Na	2018-11-21	edge		60
* 51099-51161	22Na	2018-11-22	center		63
* 51167-51267	none	2018-11-23
***************************************************************************************/

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

void draw_Exp(TChain *tExpA, TChain *tExpB, TChain *tInfoA, TChain *tInfoB, 
	const char *name, const char *fname, TCut cXY, TCut cZ, double eScale, double Efit[2], int version)
{
	char str[256];
	long long gtA, gtB;
	double timeA, timeB;
	int i;
	
	gStyle->SetOptStat("i");
	gStyle->SetOptFit(1);
//	gStyle->SetOptStat(0);
//	gStyle->SetOptFit(0);
	gStyle->SetTitleXSize(0.05);
	gStyle->SetTitleYSize(0.05);
	gStyle->SetLabelSize(0.05);
	gStyle->SetPadLeftMargin(0.15);
	gStyle->SetPadBottomMargin(0.15);
//	gStyle->SetLineWidth(4);
	
	TH2D *hXY = new TH2D("hXY", "XY distribution of gamma flash center;X, cm;Y, cm", 25, 0, 100, 25, 0, 100);
	sprintf(str, "DANSS energy deposit in %s decay;E, MeV", name);
	TH1D *hExpA = new TH1D("hExpA", str, 70, 0, 7);
	TH1D *hExpB = new TH1D("hExpB", str, 70, 0, 7);
	TH1D *hExpC = new TH1D("hExpC", str, 70, 0, 7);
	sprintf(str, "SiPM energy deposit in %s decay;E, MeV", name);
	TH1D *hExpSiPMA = new TH1D("hExpSiPMA", str, 70, 0, 7);
	TH1D *hExpSiPMB = new TH1D("hExpSiPMB", str, 70, 0, 7);
	TH1D *hExpSiPMC = new TH1D("hExpSiPMC", str, 70, 0, 7);
	sprintf(str, "PMT energy deposit in %s decay;E, MeV", name);
	TH1D *hExpPMTA = new TH1D("hExpPMTA", str, 70, 0, 7);
	TH1D *hExpPMTB = new TH1D("hExpPMTB", str, 70, 0, 7);
	TH1D *hExpPMTC = new TH1D("hExpPMTC", str, 70, 0, 7);
	sprintf(str, "Number of hits from %s decay", name);
	TH1D *hHitsA = new TH1D("hHitsA", str, 20, 0, 20);
	TH1D *hHitsB = new TH1D("hHitsB", str, 20, 0, 20);
	TH1D *hHitsC = new TH1D("hHitsC", str, 20, 0, 20);

	TCut cxyz("NeutronX[0] >= 0 && NeutronX[1] >= 0 && NeutronX[2] >= 0");
	TCut cVeto("VetoCleanHits < 2 && VetoCleanEnergy < 4");
	TCut cn("SiPmCleanHits > 2");
	TCut cNoise("((PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3) && VetoCleanHits == 0");
//	TCut cNoise("(PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3");
	TCut cSel = cxyz && cVeto && !cNoise;
	
	tExpA->Project("hXY", "NeutronX[1]+2:NeutronX[0]+2", cSel && cZ && cn);
	if (version == 72) {
		sprintf(str, "(SiPmCleanEnergy+PmtCleanEnergy-0.13)*%5.3f/2.0", eScale);	// 0.13 - SiPM residual noise energy
	} else {
		sprintf(str, "(SiPmCleanEnergy+PmtCleanEnergy)*%5.3f/2.0", eScale);		// new nosie cut
	}
	tExpA->Project("hExpA", str, cSel && cZ && cXY && cn);
	tExpB->Project("hExpB", str, cSel && cZ && cXY && cn);
	if (version == 72) {
		sprintf(str, "(SiPmCleanEnergy-0.13)*%5.3f", eScale);				// 0.13 - SiPM residual noise energy
	} else {
		sprintf(str, "(SiPmCleanEnergy)*%5.3f", eScale);
	}
	tExpA->Project("hExpSiPMA", str, cSel && cZ && cXY && cn);
	tExpB->Project("hExpSiPMB", str, cSel && cZ && cXY && cn);
	sprintf(str, "PmtCleanEnergy*%5.3f", eScale);
	tExpA->Project("hExpPMTA", str, cSel && cZ && cXY && cn);
	tExpB->Project("hExpPMTB", str, cSel && cZ && cXY && cn);
	tExpA->Project("hHitsA", "SiPmCleanHits", cSel && cZ && cXY);
	tExpB->Project("hHitsB", "SiPmCleanHits", cSel && cZ && cXY);
	
	gtA = gtB = 0;
	for(i=0; i<tInfoA->GetEntries(); i++) {
		tInfoA->GetEntry(i);
		gtA += tInfoA->GetLeaf("gTime")->GetValueLong64();
	}
	for(i=0; i<tInfoB->GetEntries(); i++) {
		tInfoB->GetEntry(i);
		gtB += tInfoB->GetLeaf("gTime")->GetValueLong64();
	}
	timeA = gtA / 1.25E8;
	timeB = gtB / 1.25E8;
	
//	printf("A = %6.1fs    B = %6.1fs\n", timeA, timeB);
	
	hExpA->Sumw2();
	hExpB->Sumw2();
	hExpSiPMA->Sumw2();
	hExpSiPMB->Sumw2();
	hExpPMTA->Sumw2();
	hExpPMTB->Sumw2();
	hHitsA->Sumw2();
	hHitsB->Sumw2();
	
	hExpC->Add(hExpA, hExpB, 1.0/timeA, -1.0/timeB);
	hExpSiPMC->Add(hExpSiPMA, hExpSiPMB, 1.0/timeA, -1.0/timeB);
	hExpPMTC->Add(hExpPMTA, hExpPMTB, 1.0/timeA, -1.0/timeB);
	hHitsC->Add(hHitsA, hHitsB, 1.0/timeA, -1.0/timeB);

	hXY->GetXaxis()->SetLabelSize(0.045);
	hXY->GetYaxis()->SetLabelSize(0.045);
	hXY->GetZaxis()->SetLabelSize(0.045);
	hExpC->GetYaxis()->SetLabelSize(0.05);
	hExpC->SetLineWidth(2);
	hExpC->SetLineColor(kBlue);
	hExpSiPMC->GetYaxis()->SetLabelSize(0.05);
	hExpPMTC->GetYaxis()->SetLabelSize(0.05);
	hHitsC->GetYaxis()->SetLabelSize(0.05);
	hHitsC->SetLineColor(kBlue);
	hHitsC->SetMarkerColor(kBlue);
	hHitsC->SetMarkerStyle(kFullSquare);

	hHitsC->SetStats(0);
	
	TCanvas *cExpA = new TCanvas("cExpA", "Data", 1200, 800);
	cExpA->Divide(2, 2);
	cExpA->cd(1);
	hExpC->Fit("gaus", "", "", Efit[0], Efit[1]);
	cExpA->cd(2);
	hExpSiPMC->Fit("gaus", "", "", Efit[0], Efit[1]);
	cExpA->cd(3);
	hExpPMTC->Fit("gaus", "", "", Efit[0], Efit[1]);
	cExpA->cd(4);
	hHitsC->Draw();

	sprintf(str, "%s.pdf(", fname);
	cExpA->SaveAs(str);

	TCanvas *cExpB = new TCanvas("cExpB", "Hits", 1200, 800);
	hXY->Draw("colz");
	sprintf(str, "%s.pdf)", fname);
	cExpB->SaveAs(str);

	sprintf(str, "%s.root", fname);
	TFile *f = new TFile(str, "RECREATE");
	if (f->IsOpen()) {
		f->cd();
		hXY->Write();
		hExpA->Write();
		hExpB->Write();
		hExpC->Write();
		hExpSiPMA->Write();
		hExpSiPMB->Write();
		hExpSiPMC->Write();
		hExpPMTA->Write();
		hExpPMTB->Write();
		hExpPMTC->Write();
		hHitsA->Write();
		hHitsB->Write();
		hHitsC->Write();
		f->Close();
	}
}

void draw_MC(TChain *tMc, const char *name, const char *fname, TCut cXY, TCut cZ, double Efit[2], double kRndm = 0.0, double cRndm = 0.0)
{
	char str[256];
	double rAB;
	long NA, NB;
	
	gStyle->SetOptStat("i");
	gStyle->SetOptFit(1);
//	gStyle->SetOptStat(0);
//	gStyle->SetOptFit(0);
	gStyle->SetTitleXSize(0.05);
	gStyle->SetTitleYSize(0.05);
	gStyle->SetLabelSize(0.05);
	gStyle->SetPadLeftMargin(0.15);
	gStyle->SetPadBottomMargin(0.15);
//	gStyle->SetLineWidth(4);
	
	sprintf(str, "Monte Carlo energy deposit in %s decay;E, MeV", name);
	TH1D *hMc = new TH1D("hMc", str, 140, 0, 7);
	sprintf(str, "Monte Carlo SiPM energy deposit in %s decay;E, MeV", name);
	TH1D *hMcSiPM = new TH1D("hMcSiPM", str, 35, 0, 7);
	sprintf(str, "Monte Carlo PMT energy deposit in %s decay;E, MeV", name);
	TH1D *hMcPMT = new TH1D("hMcPMT", str, 35, 0, 7);
	sprintf(str, "Monte Carlo number of hits from %s decay", name);
	TH1D *hMcHits = new TH1D("hMcHits", str, 20, 0, 20);

	TCut cxyz("NeutronX[0] >= 0 && NeutronX[1] >= 0 && NeutronX[2] >= 0");
	TCut cVeto("VetoCleanHits < 2 && VetoCleanEnergy < 4");
	TCut cn("SiPmCleanHits > 2");
	TCut cSel = cxyz && cVeto;
	
	sprintf(str, "MyRandom::GausAdd((SiPmCleanEnergy+PmtCleanEnergy)/2.0, %6.4f, %6.4f)", kRndm, cRndm);
	tMc->Project("hMc", str, cSel && cZ && cXY && cn);
	sprintf(str, "MyRandom::GausAdd(SiPmCleanEnergy, %6.4f, %6.4f)", kRndm, cRndm);
	tMc->Project("hMcSiPM", str, cSel && cZ && cXY && cn);
	sprintf(str, "MyRandom::GausAdd(PmtCleanEnergy, %6.4f, %6.4f)", kRndm, cRndm);
	tMc->Project("hMcPMT", str, cSel && cZ && cXY && cn);
	tMc->Project("hMcHits", "SiPmCleanHits", cSel && cZ && cXY);

	hMc->Sumw2();
	hMcSiPM->Sumw2();
	hMcPMT->Sumw2();
	hMcHits->Sumw2();
	
	hMc->GetYaxis()->SetLabelSize(0.05);
	hMcSiPM->GetYaxis()->SetLabelSize(0.05);
	hMcPMT->GetYaxis()->SetLabelSize(0.05);
	hMcHits->GetYaxis()->SetLabelSize(0.05);
	hMcHits->SetLineColor(kRed);
	hMcHits->SetMarkerColor(kRed);
	hMcHits->SetMarkerStyle(kFullCircle);
	hMcHits->SetStats(0);
	
	TCanvas *cMc = new TCanvas("cMc", "Monte Carlo", 1200, 800);
	cMc->Divide(2, 2);
	cMc->cd(1);
	hMc->Fit("gaus", "", "", Efit[0], Efit[1]);
	cMc->cd(2);
	hMcSiPM->Fit("gaus", "", "", Efit[0], Efit[1]);
	cMc->cd(3);
	hMcPMT->Fit("gaus", "", "", Efit[0], Efit[1]);
	cMc->cd(4);
	hMcHits->Draw("hist");
	sprintf(str, "%s.pdf", fname);
	cMc->SaveAs(str);
	
	sprintf(str, "%s.root", fname);
	TFile *f = new TFile(str, "RECREATE");
	if (f->IsOpen()) {
		f->cd();
		hMc->Write();
		hMcSiPM->Write();
		hMcPMT->Write();
		hMcHits->Write();
		f->Close();
	}
}

void Add2Chain(TChain *ch, int from, int to, const char *rootdir, int max_files = 0)
{
	int i;
	char str[1024];
	
	for(i=from; i<=to && (max_files <= 0 || i < from+max_files); i++) {
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/%3.3dxxx/danss_%6.6d.root", 
			rootdir, i/1000, i);
		ch->AddFile(str);
	}
}

void draw_Sources2(int iser, double Rndm_or_scale, int version = 74, double cRndm = 0, int max_files = 0)
{
	const char *name = "";
	char fname[1024];
	char str[1024];
	double Efit[2] = {1.0, 3.5};
	TCut cXY;
	TCut cZ;
	int code;
	int i;
	char *rootdir = (char *)"root6n5";
	
	TChain *tMc = new TChain("DanssEvent");
	TChain *tExpA = new TChain("DanssEvent");
	TChain *tExpB = new TChain("DanssEvent");
	TChain *tInfoA = new TChain("DanssInfo");
	TChain *tInfoB = new TChain("DanssInfo");
	TChain *tRawA = new TChain("RawHits");
	TChain *tRawB = new TChain("RawHits");
	tExpA->AddFriend(tRawA);
	tExpB->AddFriend(tRawB);
	
	switch (version) {
	case 72:
		rootdir = (char *)"root6n2";
		break;
	case 74:
		rootdir = (char *)"root6n4";
		break;
	case 76:
		rootdir = (char *)"root6n6";
		break;
	case 78:
		rootdir = (char *)"root6n8";
		break;
	default:
		rootdir = (char *)"root6n4";
		break;
	}
	
	code = iser / 1000;
	switch (iser) {
	case 1:		// Na Feb 17, center
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 48) * (NeutronX[1] - 48) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 12376, 12407, rootdir, max_files);
		Add2Chain(tRawA, 12376, 12407, rootdir, max_files);
		Add2Chain(tInfoA, 12376, 12407, rootdir, max_files);
		Add2Chain(tExpB, 12411, 12474, rootdir, max_files);
		Add2Chain(tRawB, 12411, 12474, rootdir, max_files);
		Add2Chain(tInfoB, 12411, 12474, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_feb17_center_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
	case 2:		// Na Feb 17, edge
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 88) * (NeutronX[1] - 88) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 12364, 12373, rootdir, max_files);
		Add2Chain(tRawA, 12364, 12373, rootdir, max_files);
		Add2Chain(tInfoA, 12364, 12373, rootdir, max_files);
		Add2Chain(tExpB, 12411, 12430, rootdir, max_files);
		Add2Chain(tRawB, 12411, 12430, rootdir, max_files);
		Add2Chain(tInfoB, 12411, 12430, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_feb17_edge_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
	case 11:		// Na Nov 18, center
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 48) * (NeutronX[1] - 48) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 51099, 51161, rootdir, max_files);
		Add2Chain(tRawA, 51099, 51161, rootdir, max_files);
		Add2Chain(tInfoA, 51099, 51161, rootdir, max_files);
		Add2Chain(tExpB, 51167, 51267, rootdir, max_files);
		Add2Chain(tRawB, 51167, 51267, rootdir, max_files);
		Add2Chain(tInfoB, 51167, 51267, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_nov18_center_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
	case 12:		// Na Nov 18, edge
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 88) * (NeutronX[1] - 88) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 51036, 51095, rootdir, max_files);
		Add2Chain(tRawA, 51036, 51095, rootdir, max_files);
		Add2Chain(tInfoA, 51036, 51095, rootdir, max_files);
		Add2Chain(tExpB, 51167, 51267, rootdir, max_files);
		Add2Chain(tRawB, 51167, 51267, rootdir, max_files);
		Add2Chain(tInfoB, 51167, 51267, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_nov18_edge_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
	case 101:		// Co Feb 17, center
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 48) * (NeutronX[1] - 48) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 12306, 12346, rootdir, max_files);
		Add2Chain(tRawA, 12306, 12346, rootdir, max_files);
		Add2Chain(tInfoA, 12306, 12346, rootdir, max_files);
		Add2Chain(tExpB, 12198, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12198, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12198, 12303, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_feb17_center_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
	case 102:		// Co Feb 17, edge
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 88) * (NeutronX[1] - 88) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 12348, 12361, rootdir, max_files);
		Add2Chain(tRawA, 12348, 12361, rootdir, max_files);
		Add2Chain(tInfoA, 12348, 12361, rootdir, max_files);
		Add2Chain(tExpB, 12260, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12260, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12260, 12303, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_feb17_edge_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
	case 111:		// Co Nov 18, center
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 48) * (NeutronX[1] - 48) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 50949, 50997, rootdir, max_files);
		Add2Chain(tRawA, 50949, 50997, rootdir, max_files);
		Add2Chain(tInfoA, 50949, 50997, rootdir, max_files);
		Add2Chain(tExpB, 50750, 50873, rootdir, max_files);
		Add2Chain(tRawB, 50750, 50873, rootdir, max_files);
		Add2Chain(tInfoB, 50750, 50873, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_nov18_center_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
	case 112:		// Co Nov 18, edge
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 88) * (NeutronX[1] - 88) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
		Add2Chain(tExpA, 50999, 51034, rootdir, max_files);
		Add2Chain(tRawA, 50999, 51034, rootdir, max_files);
		Add2Chain(tInfoA, 50999, 51034, rootdir, max_files);
		Add2Chain(tExpB, 50750, 50873, rootdir, max_files);
		Add2Chain(tRawB, 50750, 50873, rootdir, max_files);
		Add2Chain(tInfoB, 50750, 50873, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_nov18_edge_%5.3f_%s", Rndm_or_scale, rootdir);
		break;
		
	case 1001:	// Na MC, center
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 48) * (NeutronX[1] - 48) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
//		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_22Na_glbLY_transcode_rawProc_pedSim.root", rootdir);
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_22Na_glbLY_transcode_rawProc_pedSim.root", rootdir);
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "22Na_MC_center_rndm_%4.2f_%4.2f_%s", Rndm_or_scale, cRndm, rootdir);
		break;
	case 1002:	// Na MC, edge
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 88) * (NeutronX[1] - 88) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
//		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_22Na_glbLY_transcode_rawProc_pedSim_90cm.root", rootdir);
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_22Na_90cmPos_glbLY_transcode_rawProc_pedSim.root", rootdir);
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "22Na_MC_edge_rndm_%4.2f_%4.2f_%s", Rndm_or_scale, cRndm, rootdir);
		break;
	case 1101:	// Co MC, center
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 48) * (NeutronX[1] - 48) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
//		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_60Co_glbLY_transcode_rawProc_pedSim.root", rootdir);
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_60Co_glbLY_transcode_rawProc_pedSim.root", rootdir);
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "60Co_MC_center_rndm_%4.2f_%4.2f_%s", Rndm_or_scale, cRndm, rootdir);
		break;
	case 1102:	// Co MC, edge
		cXY = (TCut) "(NeutronX[0] - 48) * (NeutronX[0] - 48) + (NeutronX[1] - 88) * (NeutronX[1] - 88) + (NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 400";
		cZ = (TCut) "(NeutronX[2] - 49.5) * (NeutronX[2] - 49.5) < 100";
//		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_60Co_glbLY_transcode_rawProc_pedSim_90cm.root", rootdir);
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/%s/MC/DataTakingPeriod01/RadSources/mc_60Co_90cmPos_glbLY_transcode_rawProc_pedSim.root", rootdir);
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "60Co_MC_edge_rndm_%4.2f_%4.2f_%s", Rndm_or_scale, cRndm, rootdir);
		break;
	default:
		printf("%d - unknown\n", iser);
		printf("Available valuse for iser = MIDD:\n");
		printf("M - MC (1) or experiment (0);\n");
		printf("I - isotope: 60Co (1) or 22Na (0);\n");
		printf("DD - serial for experiment:\n");
		printf("1  - February 17, center\n");
		printf("2  - February 17, edge\n");
		printf("11  - November 18, center\n");
		printf("12  - November 18, edge\n");
		printf("DD - for MC:\n");
		printf("1  - center (50, 50, 50) position\n");
		printf("2 - edge (50, 90, 50) position\n");
		code = -1;
		break;
	}

	switch (code) {
	case 0:	// Experiment
		draw_Exp(tExpA, tExpB, tInfoA, tInfoB, name, fname, cXY, cZ, Rndm_or_scale, Efit, version);
		break;
	case 1:	// MC
		draw_MC(tMc, name, fname, cXY, cZ, Efit, Rndm_or_scale, cRndm);
		break;		
	default:
		break;
	}

	delete tMc;
	delete tExpA;
	delete tExpB;
}

void src2mc(const char *expfile, const char *mcfile, const char *newname)
{
	int i;

	TFile *fExp = new TFile(expfile);
	TFile *fMC = new TFile(mcfile);
	TH1 *hExpE = (TH1 *) fExp->Get("hExpC");
	TH1 *hExpH = (TH1 *) fExp->Get("hHitsC");
	TH1 *hMCE = (TH1 *) fMC->Get("hMc");
	TH1 *hMCH = (TH1 *) fMC->Get("hMcHits");
	if (!(hExpE && hExpH && hMCE && hMCH)) {
		printf("Something is wrong.\n");
		return;
	}
	hExpE->RecursiveRemove(hExpE->GetFunction("gaus"));
	hMCE->RecursiveRemove(hMCE->GetFunction("gaus"));
	TCanvas *cv = new TCanvas("CV", "SRC", 1200, 900);
	cv->Divide(2, 1);
	TVirtualPad * pd = cv->cd(1);
	pd->SetLeftMargin(0.21);
	pd->SetRightMargin(0.03);
	hExpE->SetLineColor(kBlack);
	hExpE->SetLineWidth(3);
	hExpE->SetStats(0);
	hExpE->GetXaxis()->SetTitle("Energy, MeV");
	hExpE->GetYaxis()->SetTitle("Events/100 keV");
	hExpE->GetYaxis()->SetTitleOffset(2.0);
	hExpE->Draw("e");
	hMCE->Scale(hExpE->Integral() / hMCE->Integral());
	hMCE->SetLineColor(kBlue);
	hMCE->SetStats(0);
	hMCE->Draw("same,hist");
	TLegend *lg = new TLegend(0.65, 0.8, 0.97, 0.9);
	lg->AddEntry(hExpE, "Experiment", "LE");
	lg->AddEntry(hMCE, "MC", "L");
	lg->Draw();

	pd = cv->cd(2);
	pd->SetLeftMargin(0.13);
	pd->SetRightMargin(0.03);
	hMCH->Scale(hExpH->Integral() / hMCH->Integral());
	hMCH->SetLineColor(kBlue);
	hMCH->SetStats(0);
	hMCH->GetXaxis()->SetTitle("SiPM hits");
	hMCH->GetYaxis()->SetTitle("Events/100 keV");
	hMCH->GetYaxis()->SetTitleOffset(1.3);
	hMCH->Draw("hist");
	hExpH->SetLineColor(kBlack);
	hExpH->SetLineWidth(3);
	hExpH->SetStats(0);
	hExpH->Draw("e,same");
	lg->Draw();
	cv->Update();
	cv->SaveAs(newname);
	
	printf("%s - experiment energy:\nEnergy     Events\n", newname);
	for (i=0; i<hExpE->GetNbinsX(); i++) printf("%5.3f-%5.3f    %f +- %f\n",
		hExpE->GetBinLowEdge(i+1), hExpE->GetBinLowEdge(i+1) + hExpE->GetBinWidth(i+1),
		hExpE->GetBinContent(i+1), hExpE->GetBinError(i+1));
	printf("%s - experiment SiPM hits:\nN     Events\n", newname);
	for (i=0; i<hExpH->GetNbinsX(); i++) printf("%d    %f +- %f\n", i,
		hExpH->GetBinContent(i+1), hExpH->GetBinError(i+1));
	printf("%s - MC energy:\nEnergy     Events\n", newname);
	for (i=0; i<hMCE->GetNbinsX(); i++) printf("%5.3f-%5.3f    %f +- %f\n",
		hMCE->GetBinLowEdge(i+1), hMCE->GetBinLowEdge(i+1) + hMCE->GetBinWidth(i+1),
		hMCE->GetBinContent(i+1), hMCE->GetBinError(i+1));
	printf("%s - MC SiPM hits:\nN     Events\n", newname);
	for (i=0; i<hMCH->GetNbinsX(); i++) printf("%d    %f +- %f\n", i,
		hMCH->GetBinContent(i+1), hMCH->GetBinError(i+1));

	fExp->Close();
	fMC->Close();
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

TH2D *src_scan(const char *src, const char *period, double cRndm)
{
	const double Rrndm[2] = {0.02, 0.22};
	const int Nrndm = 5;
	double Drndm = (Rrndm[1] - Rrndm[0]) / (Nrndm - 1);
	const double Rscale[2] = {0.98, 1.04};
	const int Nscale = 13;
	double Dscale = (Rscale[1] - Rscale[0]) / (Nscale - 1);
	double kRndm, kScale;
	int i, j;
	TH2D *h2d;
	TFile *f;
	char strs[128], strl[1024];
	TH1D *hExp[Nscale];
	TH1D *hMC[Nrndm];
	TH1D *tmp;
	double Emin, Emax;
	int binMin, binMax;
	double chi2;

	gStyle->SetOptStat(0);

	TLegend lg(0.6, 0.75, 0.98, 0.88);
	TLatex txt;
	TLine ln;
	ln.SetLineColor(kCyan);
	ln.SetLineStyle(kDashed);
	ln.SetLineWidth(2);


	sprintf(strl, "%s-%s-%4.2f-scan.", src, period, cRndm);
	TString name(strl);
	
	TFile *fOut = new TFile((name + "root").Data(), "RECREATE");
	if (!fOut->IsOpen()) return NULL;
	
	sprintf(strs, "h%sscan", src);
	sprintf(strl, "Scan rndm and scale for %s, const=%4.2f;Scale;Rndm", src, cRndm);
	h2d = new TH2D(strs, strl, Nscale, Rscale[0] - Dscale/2, Rscale[1] + Dscale/2, Nrndm, Rrndm[0] - Drndm/2, Rrndm[1] + Drndm/2);
	for (i=0; i<Nscale; i++) {
		kScale = Rscale[0] + Dscale * i;
		sprintf(strl, "%s_%s_center_%5.3f_root6n8.root", src, period, kScale);
		f = new TFile(strl);
		if (!f->IsOpen()) return NULL;
		tmp = (TH1D*)f->Get("hExpC");
		if (!tmp) return NULL;
		sprintf(strs, "hExp%s_%d", src, i);
		fOut->cd();
		hExp[i] = (TH1D*)tmp->Clone(strs);
		f->Close();
		delete f;
		sprintf(strl, "%s @ %s center * %5.3f", src, period, kScale);
		hExp[i]->SetTitle(strl);
		hExp[i]->SetLineColor(kRed);
		hExp[i]->SetMarkerColor(kRed);
		hExp[i]->SetMarkerStyle(kFullStar);
	}
	for (j=0; j<Nrndm; j++) {
		kScale = Rscale[0] + Dscale * i;
		kRndm = Rrndm[0] + Drndm * j;
		sprintf(strl, "%s_MC_center_rndm_%4.2f_%4.2f_root6n8.root", src, kRndm, cRndm);
		f = new TFile(strl);
		if (!f->IsOpen()) return NULL;
		tmp = (TH1D*)f->Get("hMc");
		if (!tmp) return NULL;
		sprintf(strs, "hMC%s_%d", src, j);
		fOut->cd();
		hMC[j] = (TH1D*)tmp->Rebin(2, strs);
		f->Close();
		delete f;
		sprintf(strl, "%s @ %s center #oplus %5.3f/#sqrt{E} #oplus %5.3f", src, period, kRndm, cRndm);
		hMC[j]->SetTitle(strl);
		hMC[j]->SetLineColor(kBlue);
		hMC[j]->SetLineWidth(2);
	}

	TCanvas cv("CV", "CV", 1280, 1600);
	cv.SaveAs((name + "pdf[").Data());

	if (src[0] == '2') {	// 22Na
//		hRat->Fit("pol0", "Q0", "", 0.6, 2.6);
		Emin = 0.6;
		Emax = 2.6;
	} else {		// 60Co
//		hRat->Fit("pol0", "Q0", "", 0.8, 3.0);
		Emin = 0.8;
		Emax = 3.0;
	}
	binMin = hExp[0]->FindBin(Emin + 0.001);
	binMax = hExp[0]->FindBin(Emax - 0.001);

	for (i=0; i<Nscale; i++) for (j=0; j<Nrndm; j++) {
		hMC[j]->Scale(hExp[i]->Integral(binMin, binMax) / hMC[j]->Integral(binMin, binMax));
		chi2 = chi2Diff(hExp[i], hMC[j], binMin, binMax);
		h2d->SetBinContent(i+1, j+1, chi2);
		hExp[i]->Draw("e");
		hMC[j]->Draw("hist,same");
		lg.Clear();
		lg.AddEntry(hExp[i], hExp[i]->GetTitle(), "lpe");
		lg.AddEntry(hMC[j], hMC[j]->GetTitle(), "l");
		lg.Draw();
		sprintf(strl, "#chi^{2}/N.d.f. = %6.2g / %d", chi2, binMax - binMin);
		txt.DrawLatex(2, hExp[i]->GetMaximum() / 10.0, strl);
		ln.DrawLine(Emin, 0, Emin, hExp[i]->GetMaximum() * 0.6);
		ln.DrawLine(Emax, 0, Emax, hExp[i]->GetMaximum() * 0.6);
		cv.Update();
		cv.SaveAs((name + "pdf").Data());
	}

	h2d->Draw("box");
	cv.SaveAs((name + "pdf").Data());
	h2d->Draw("colorz");
	cv.SaveAs((name + "pdf").Data());
	h2d->Draw("lego2");
	cv.SaveAs((name + "pdf").Data());
	TH1D * hPrjX = h2d->ProjectionX("__X", 2, 2);
	hPrjX->SetTitle("^{248}Cm scan profile at Rndm = 7%/#sqrt{E}");
	hPrjX->Draw("e");
	cv.SaveAs((name + "pdf").Data());
	TH1D * hPrjY = h2d->ProjectionY("__Y", 7, 7);
	hPrjY->SetTitle("^{248}Cm scan profile at scale = 1");
	hPrjY->Draw("e");
	cv.SaveAs((name + "pdf").Data());

	cv.SaveAs((name + "pdf]").Data());
	fOut->cd();
	h2d->Write();
	for (i=0; i<Nscale; i++) hExp[i]->Write();
	for (j=0; j<Nrndm; j++) hMC[j]->Write();
	fOut->Close();
	
	return h2d;
}
