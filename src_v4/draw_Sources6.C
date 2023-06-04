#include <stdio.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TCut.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLatex.h>
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
* 127280-127379	none	2022-06-19
* 127389-127456	22Na	2022-06-21	center		68
* 127513-127580	24Na	2022-06-23	center, not seen, strong pickup (?)
* 127658-127719	24Na	2022-06-25	center, not seen
* 127720-127772	248Cm	2022-06-26	center		53
* 127774-127837	60Co	2022-06-27	center		64
* 127838-127900	none	2022-06-28
* 127958-128016	22Na	2022-06-30	center, UP	59
* 128020-128119	none	2022-07-01
***************************************************************************************
* The channel for teflon tube to introduce sources is machined in left low part of 
* strip X=12 Z=49. So strips X=11,12 Z=49 and Y=12 Z=48 could be afected by beta
* reaching it through the capsule. We remove events with these strips hit
***************************************************************************************/
struct DanssEventStruct7 {
//		Common parameters
	long long	globalTime;		// time in terms of 125 MHz
	long long	number;			// event number in the file
	int		unixTime;		// linux time, seconds
	float		fineTime;		// fine time of the event (for hit selection)
	int		trigType;		// type of the trigger
//		Veto parameters
	int		VetoCleanHits;		// hits above threshold and in time window
	float		VetoCleanEnergy;	// Energy Sum of clean hits
	float		BottomLayersEnergy;	// Energy in the two bottom SiPM layers to be used in Veto
//		PMT parameters
	int		PmtCleanHits;
	float		PmtCleanEnergy;
//		SiPM parameters
	int		SiPmHits;
	float		SiPmEnergy;
	int		SiPmCleanHits;
	float		SiPmCleanEnergy;
	int		SiPmEarlyHits;		// to understand random background
	float		SiPmEarlyEnergy;
//		"positron cluster" parameters
	int		PositronHits;		// SiPm hits in the cluster
	float		PositronEnergy;		// Energy sum of the cluster, longitudinally corrected (SiPM+PMT)
	float		TotalEnergy;		// Event full energy  longitudinally correctd (SiPM+PMT)
	float		PositronSiPmEnergy;	// SiPM energy in the  longitudinally cluster, corrected
	float		PositronPmtEnergy;	// PMT energy in the  longitudinally cluster, corrected
	float		PositronX[3];		// cluster position
	int		AnnihilationGammas;	// number of possible annihilation gammas
	float		AnnihilationEnergy;	// Energy in annihilation gammas
	float		AnnihilationMax;	// Energy in the maximum annihilation hit
	float		MinPositron2GammaZ;	// Z-distance to the closest gamma
//		"neutron" parameters
	float		NeutronX[3];		// center of gammas position
	int		NHits;			// Number of hits
	float		NeutronEnergy;		// Total energy corrected for signal attenuation
	int		NeutronHits;		// Hits SiPm + PMT (where no SiPm)
	int		NXYSiPmRaw;		// number of SiPm raw hits X - Y, raw hits
	int		NXYSiPmClean;		// number of SiPm raw hits X - Y, clean hits
	int		NXYPmt;			// number of Pmt raw hits X - Y
};

struct HitTypeStruct {
	char 	type;
	char	z;
	char	xy;
	char	flag;
};

//		Raw hits information
struct RawHitInfoStruct {
	unsigned short PmtCnt;
	unsigned short VetoCnt;
	unsigned short SiPmCnt;
};

/*
	Fill histograms excluding events with beta-afected strips
*/
void do_projections(TChain *chain, TH1D *hSum, TH1D *hSiPM, TH1D *hPMT, TH1D *hHits)
{
	long long i, N, N1, N2, N3, N4, N5;
	int j, k;
	struct DanssEventStruct7 DanssEvent;
	struct HitTypeStruct HitType[2600];
	struct RawHitInfoStruct RawHits;

	chain->SetBranchAddress("Data", &DanssEvent);
	chain->SetBranchAddress("HitType", &HitType);
	chain->SetBranchAddress("RawHits", &RawHits);
	N = chain->GetEntries();
	N1 = N2 = N3 = N4 = N5 = 0;
	for (i=0; i<N; i++) {
		chain->GetEntry(i);
//	TCut cxyz("NeutronX[0] >= 0 && NeutronX[1] >= 0 && NeutronX[2] >= 0");
		if (DanssEvent.NeutronX[0] < 0) continue;
		if (DanssEvent.NeutronX[1] < 0) continue;
		if (DanssEvent.NeutronX[2] < 0) continue;
		N1++;
//	TCut cVeto("VetoCleanHits < 2 && VetoCleanEnergy < 4");
		if (DanssEvent.VetoCleanHits >= 2 || DanssEvent.VetoCleanEnergy >= 4) continue;
		N2++;
//	TCut cn("SiPmCleanHits > 2");
		if (DanssEvent.SiPmCleanHits <= 2) continue;
		if (DanssEvent.PmtCleanHits < 1) continue;
		N3++;
//	TCut cNoise("((PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3) && VetoCleanHits == 0");
		if (1.0*DanssEvent.PmtCleanHits/RawHits.PmtCnt <= 0.3 || 1.0*DanssEvent.SiPmHits/RawHits.SiPmCnt <= 0.3) continue;
		N4++;
//	Remove event if beta-afected strip hit
		k = 0;
		for (j=0; j<DanssEvent.NHits; j++) {
			if (HitType[j].type != 0) continue;	// Select SiPM
			if (HitType[j].z == 48 && HitType[j].xy == 12) k++;
			if (HitType[j].z == 49 && (HitType[j].xy == 11 || HitType[j].xy == 12)) k++;
		}
		if (k != 0) continue;
		N5++;
//	Fill
		hSum->Fill((DanssEvent.SiPmCleanEnergy+DanssEvent.PmtCleanEnergy)/2.0);
		hSiPM->Fill(DanssEvent.SiPmCleanEnergy);
		hPMT->Fill(DanssEvent.PmtCleanEnergy);
		hHits->Fill(DanssEvent.SiPmCleanHits);
	}
	printf("Cut rejection: %Ld %Ld %Ld %Ld %Ld %Ld\n", N, N1, N2, N3, N4, N5);
}

void draw_Exp(TChain *tExpA, TChain *tExpB, TChain *tInfoA, TChain *tInfoB, const char *name, const char *fname)
{
	char str[256];
	long long gtA, gtB;
	double timeA, timeB;
	int i;
	
	gStyle->SetOptStat("i");
	gStyle->SetOptFit(1);
	gStyle->SetTitleXSize(0.05);
	gStyle->SetTitleYSize(0.05);
	gStyle->SetLabelSize(0.05);
	gStyle->SetPadLeftMargin(0.15);
	gStyle->SetPadBottomMargin(0.15);
	
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
	TCut cn("SiPmCleanHits > 2 && PmtCleanHits > 0");
	TCut cNoise("((PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3) && VetoCleanHits == 0");
	TCut cSel = cxyz && cVeto && cn && !cNoise;
	
	tExpA->Project("hXY", "NeutronX[1]+2:NeutronX[0]+2", cSel);
	do_projections(tExpA, hExpA, hExpSiPMA, hExpPMTA, hHitsA);
	do_projections(tExpB, hExpB, hExpSiPMB, hExpPMTB, hHitsB);
	
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
	hExpC->Draw();
	cExpA->cd(2);
	hExpSiPMC->Draw();
	cExpA->cd(3);
	hExpPMTC->Draw();
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

void draw_MC(TChain *tMc, const char *name, const char *fname, double scale)
{
	char str[1024];
	double rAB;
	long NA, NB;
	
	gStyle->SetOptStat("i");
	gStyle->SetOptFit(1);
	gStyle->SetTitleXSize(0.05);
	gStyle->SetTitleYSize(0.05);
	gStyle->SetLabelSize(0.05);
	gStyle->SetPadLeftMargin(0.15);
	gStyle->SetPadBottomMargin(0.15);
	
	TH2D *hMcXY = new TH2D("hMcXY", "XY distribution of gamma flash center;X, cm;Y, cm", 25, 0, 100, 25, 0, 100);
	sprintf(str, "Monte Carlo energy deposit in %s decay;E, MeV", name);
	TH1D *hMc = new TH1D("hMc", str, 70, 0, 7);
	sprintf(str, "Monte Carlo SiPM energy deposit in %s decay;E, MeV", name);
	TH1D *hMcSiPM = new TH1D("hMcSiPM", str, 70, 0, 7);
	sprintf(str, "Monte Carlo PMT energy deposit in %s decay;E, MeV", name);
	TH1D *hMcPMT = new TH1D("hMcPMT", str, 70, 0, 7);
	sprintf(str, "Monte Carlo number of hits from %s decay", name);
	TH1D *hMcHits = new TH1D("hMcHits", str, 20, 0, 20);

	TCut cxyz("NeutronX[0] >= 0 && NeutronX[1] >= 0 && NeutronX[2] >= 0");
	TCut cVeto("VetoCleanHits < 2 && VetoCleanEnergy < 4");
	TCut cn("SiPmCleanHits > 2 && PmtCleanHits > 0");
	TCut cSel = cxyz && cVeto && cn;
	
	tMc->Project("hMcXY", "NeutronX[1]+2:NeutronX[0]+2", cSel);
	sprintf(str, "%5.3f*(SiPmCleanEnergy+PmtCleanEnergy)/2.0", scale);
	tMc->Project("hMc", str, cSel);
	sprintf(str, "%5.3f*SiPmCleanEnergy", scale);
	tMc->Project("hMcSiPM", str, cSel);
	sprintf(str, "%5.3f*PmtCleanEnergy", scale);
	tMc->Project("hMcPMT", str, cSel);
	tMc->Project("hMcHits", "SiPmCleanHits", cSel);

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
	hMcXY->SetStats(0);
	
	TCanvas *cMc = new TCanvas("cMc", "Monte Carlo", 1200, 800);
	cMc->Divide(2, 2);
	cMc->cd(1);
	hMc->Draw();
	cMc->cd(2);
	hMcSiPM->Draw();
	cMc->cd(3);
	hMcPMT->Draw();
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
		hMcXY->Write();
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

void draw_Sources6(int iser, const char *rootdir = "root8n2", double scale = 1.0, int max_files = 0)
{
	const char *name = "";
	char fname[1024];
	char str[1024];
	TCut cXY;
	TCut cZ;
	int code;
	int i;
	
	TChain *tMc = new TChain("DanssEvent");
	TChain *tExpA = new TChain("DanssEvent");
	TChain *tExpB = new TChain("DanssEvent");
	TChain *tInfoA = new TChain("DanssInfo");
	TChain *tInfoB = new TChain("DanssInfo");
	TChain *tRawA = new TChain("RawHits");
	TChain *tRawB = new TChain("RawHits");
	tExpA->AddFriend(tRawA);
	tExpB->AddFriend(tRawB);
	
	code = iser / 1000;
	switch (iser) {
	case 1:		// Na Feb 17, center
		Add2Chain(tExpA, 12376, 12407, rootdir, max_files);
		Add2Chain(tRawA, 12376, 12407, rootdir, max_files);
		Add2Chain(tInfoA, 12376, 12407, rootdir, max_files);
		Add2Chain(tExpB, 12411, 12547, rootdir, max_files);
		Add2Chain(tRawB, 12411, 12547, rootdir, max_files);
		Add2Chain(tInfoB, 12411, 12547, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_feb17_center_%s", rootdir);
		break;
	case 2:		// Na Feb 17, edge
		Add2Chain(tExpA, 12364, 12373, rootdir, max_files);
		Add2Chain(tRawA, 12364, 12373, rootdir, max_files);
		Add2Chain(tInfoA, 12364, 12373, rootdir, max_files);
		Add2Chain(tExpB, 12411, 12547, rootdir, max_files);
		Add2Chain(tRawB, 12411, 12547, rootdir, max_files);
		Add2Chain(tInfoB, 12411, 12547, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_feb17_edge_%s", rootdir);
		break;
	case 11:		// Na Nov 18, center
		Add2Chain(tExpA, 51099, 51161, rootdir, max_files);
		Add2Chain(tRawA, 51099, 51161, rootdir, max_files);
		Add2Chain(tInfoA, 51099, 51161, rootdir, max_files);
		Add2Chain(tExpB, 51167, 51267, rootdir, max_files);
		Add2Chain(tRawB, 51167, 51267, rootdir, max_files);
		Add2Chain(tInfoB, 51167, 51267, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_nov18_center_%s", rootdir);
		break;
	case 12:		// Na Nov 18, edge
		Add2Chain(tExpA, 51036, 51095, rootdir, max_files);
		Add2Chain(tRawA, 51036, 51095, rootdir, max_files);
		Add2Chain(tInfoA, 51036, 51095, rootdir, max_files);
		Add2Chain(tExpB, 51167, 51267, rootdir, max_files);
		Add2Chain(tRawB, 51167, 51267, rootdir, max_files);
		Add2Chain(tInfoB, 51167, 51267, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_nov18_edge_%s", rootdir);
		break;
	case 21:		// Na June 22, center DOWN
		Add2Chain(tExpA, 127389, 127456, rootdir, max_files);
		Add2Chain(tRawA, 127389, 127456, rootdir, max_files);
		Add2Chain(tInfoA, 127389, 127456, rootdir, max_files);
		Add2Chain(tExpB, 127280, 127379, rootdir, max_files);
		Add2Chain(tRawB, 127280, 127379, rootdir, max_files);
		Add2Chain(tInfoB, 127280, 127379, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_jun22_center_%s", rootdir);
		break;
	case 31:		// Na June 22, center UP
		Add2Chain(tExpA, 127958, 128016, rootdir, max_files);
		Add2Chain(tRawA, 127958, 128016, rootdir, max_files);
		Add2Chain(tInfoA, 127958, 128016, rootdir, max_files);
		Add2Chain(tExpB, 128020, 128119, rootdir, max_files);
		Add2Chain(tRawB, 128020, 128119, rootdir, max_files);
		Add2Chain(tInfoB, 128020, 128119, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_jun22_centerUP_%s", rootdir);
		break;
	case 101:		// Co Feb 17, center
		Add2Chain(tExpA, 12306, 12346, rootdir, max_files);
		Add2Chain(tRawA, 12306, 12346, rootdir, max_files);
		Add2Chain(tInfoA, 12306, 12346, rootdir, max_files);
		Add2Chain(tExpB, 12198, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12198, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12198, 12303, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_feb17_center_%s", rootdir);
		break;
	case 102:		// Co Feb 17, edge
		Add2Chain(tExpA, 12348, 12361, rootdir, max_files);
		Add2Chain(tRawA, 12348, 12361, rootdir, max_files);
		Add2Chain(tInfoA, 12348, 12361, rootdir, max_files);
		Add2Chain(tExpB, 12198, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12198, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12198, 12303, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_feb17_edge_%s", rootdir);
		break;
	case 111:		// Co Nov 18, center
		Add2Chain(tExpA, 50949, 50997, rootdir, max_files);
		Add2Chain(tRawA, 50949, 50997, rootdir, max_files);
		Add2Chain(tInfoA, 50949, 50997, rootdir, max_files);
		Add2Chain(tExpB, 50750, 50873, rootdir, max_files);
		Add2Chain(tRawB, 50750, 50873, rootdir, max_files);
		Add2Chain(tInfoB, 50750, 50873, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_nov18_center_%s", rootdir);
		break;
	case 112:		// Co Nov 18, edge
		Add2Chain(tExpA, 50999, 51034, rootdir, max_files);
		Add2Chain(tRawA, 50999, 51034, rootdir, max_files);
		Add2Chain(tInfoA, 50999, 51034, rootdir, max_files);
		Add2Chain(tExpB, 50750, 50873, rootdir, max_files);
		Add2Chain(tRawB, 50750, 50873, rootdir, max_files);
		Add2Chain(tInfoB, 50750, 50873, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_nov18_edge_%s", rootdir);
		break;
	case 121:		// Co June 22, center
		Add2Chain(tExpA, 127774, 127837, rootdir, max_files);
		Add2Chain(tRawA, 127774, 127837, rootdir, max_files);
		Add2Chain(tInfoA, 127774, 127837, rootdir, max_files);
		Add2Chain(tExpB, 127838, 127900, rootdir, max_files);
		Add2Chain(tRawB, 127838, 127900, rootdir, max_files);
		Add2Chain(tInfoB, 127838, 127900, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_jun22_center_%s", rootdir);
		break;
		
	case 1001:	// Na MC, center
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources/mc_22Na_indLY_transcode_rawProc_pedSim.root");
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "22Na_MC_center_%s_S%5.3f", rootdir, scale);
		break;
	case 1002:	// Na MC, edge
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources/mc_22Na_92_5_cmPos_indLY_transcode_rawProc_pedSim.root");
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "22Na_MC_edge_%s_S%5.3f", rootdir, scale);
		break;
	case 1101:	// Co MC, center
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources/mc_60Co_indLY_transcode_rawProc_pedSim.root");
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "60Co_MC_center_%s_S%5.3f", rootdir, scale);
		break;
	case 1102:	// Co MC, edge
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources/mc_60Co_92_5_cmPos_indLY_transcode_rawProc_pedSim.root");
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "60Co_MC_edge_%s_S%5.3f", rootdir, scale);
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
		printf("21  - June 22, center\n");
		printf("31  - June 22, center, UP\n");
		printf("DD - for MC:\n");
		printf("1  - center (50, 50, 50) position\n");
		printf("2  - edge (50, 90, 50) position\n");
		printf("The whole list: 1 2 11 12 21 31 101 102 111 112 121 1001 1002 1101 1102\n");
		code = -1;
		break;
	}

	switch (code) {
	case 0:	// Experiment
		draw_Exp(tExpA, tExpB, tInfoA, tInfoB, name, fname);
		break;
	case 1:	// MC
		draw_MC(tMc, name, fname, scale);
		break;
	default:
		break;
	}

	delete tMc;
	delete tExpA;
	delete tExpB;
	delete tRawA;
	delete tRawB;
	delete tInfoA;
	delete tInfoB;
}

void make_pattern(int from, int to, TH2D **hXZ, TH2D **hYZ, const char *rootdir = "root8n2")
{
	long long gtA, gtB;
	double timeA, timeB;
	int i;
	
	TChain *tExpA = new TChain("DanssEvent");
	TChain *tExpB = new TChain("DanssEvent");
	TChain *tRawA = new TChain("RawHits");
	TChain *tRawB = new TChain("RawHits");
	TChain *tInfoA = new TChain("DanssInfo");
	TChain *tInfoB = new TChain("DanssInfo");
	tExpA->AddFriend(tRawA);
	tExpB->AddFriend(tRawB);
	
	Add2Chain(tExpA, from, to, rootdir);
	Add2Chain(tRawA, from, to, rootdir);
	Add2Chain(tInfoA, from, to, rootdir);
	Add2Chain(tExpB, from, to, rootdir);
	Add2Chain(tRawB, from, to, rootdir);
	Add2Chain(tInfoB, from, to, rootdir);
	
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
	
	*hXZ = new TH2D("hXZ", "XZ SiPM plane;x;z", 25, 0, 25, 50, 0, 100);
	*hYZ = new TH2D("hYZ", "YZ SiPM plane;24-y;z", 25, 0, 25, 50, 0, 100);
	
	TCut cx("(HitType & 0xFF) == 0 && ((HitType >> 8) & 1) == 1");
	TCut cy("(HitType & 0xFF) == 0 && ((HitType >> 8) & 1) == 0");
	TCut cxyz("NeutronX[0] >= 0 && NeutronX[1] >= 0 && NeutronX[2] >= 0");
	TCut cVeto("VetoCleanHits < 2 && VetoCleanEnergy < 4");
	TCut cn("SiPmCleanHits > 2");
	TCut cNoise("((PmtCnt > 0 && PmtCleanHits/PmtCnt < 0.3) || SiPmHits/SiPmCnt < 0.3) && VetoCleanHits == 0");
	TCut cSel = cxyz && cVeto && !cNoise;
	
	tExpA->Project((*hXZ)->GetName(), "((HitType >> 8) & 0xFE):((HitType >> 16) & 0xFF)", cSel && cx);
	tExpA->Project((*hYZ)->GetName(), "((HitType >> 8) & 0xFE):24 - ((HitType >> 16) & 0xFF)", cSel && cy); 
}
