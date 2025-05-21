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
* 12198-12303	none	2017-02-03	down
* 12306-12346	60Co	2017-02-04	center		41
* 12348-12361	60Co	2017-02-05	edge		14
* 12364-12373	22Na	2017-02-05	edge		10
* 12376-12407	22Na	2017-02-05	center		32
* 12411-12421	none	2017-02-06	down		later position was changed to up
* 50750-50873	none	2018-11-18	down
* 50949-50997	60Co	2018-11-20	center		49
* 50999-51034	60Co	2018-11-21	edge		36
* 51036-51095	22Na	2018-11-21	edge		60
* 51099-51161	22Na	2018-11-22	center		63
* 51167-51267	none	2018-11-23	down
* 127280-127379	none	2022-06-19	down
* 127389-127456	22Na	2022-06-21	center		68
* 127462-127512	none	2022-06-22	down
* 127513-127580	24Na	2022-06-23	center, not seen, strong pickup (?)
* 127581-127657	none	2022-06-24	down
* 127658-127719	24Na	2022-06-25	center, not seen
* 127720-127772	248Cm	2022-06-26	center		53
* 127774-127837	60Co	2022-06-27	center		64
* 127838-127900	none	2022-06-28	down
* 127901-127957	none	2022-06-29	up
* 127958-128016	22Na	2022-06-30	center, UP	59
* 128020-128119	none	2022-07-01	up
***************************************************************************************
* Center position (50, 50, 50)
* Edge position   (50, 90, 50)
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
void do_projections(TChain *chain, TH1D *hSum, TH1D *hSiPM, TH1D *hPMT, TH1D *hHits, double X, double Y, double Z, double RMAX, double scale = 1)
{
	long long i, N, N1, N2, N3, N4, N5, N6, N7;
	int j, k;
	struct DanssEventStruct7 DanssEvent;
	struct HitTypeStruct HitType[2600];
	float HitE[2600];
	struct RawHitInfoStruct RawHits;
	double r2, E;
	int east, west, north, south, up, down;
	int noRaw;
	double SiPmMax, PmtMax;

	chain->SetBranchAddress("Data", &DanssEvent);
	chain->SetBranchAddress("HitType", &HitType);
	chain->SetBranchAddress("HitE", &HitE);
	noRaw = chain->SetBranchAddress("RawHits", &RawHits);
	N = chain->GetEntries();
	N1 = N2 = N3 = N4 = N5 = N6 = N7 = 0;
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
		if (!noRaw && (1.0*DanssEvent.PmtCleanHits/RawHits.PmtCnt <= 0.3 || 1.0*DanssEvent.SiPmHits/RawHits.SiPmCnt <= 0.3)) continue;
		N4++;
//	Cut over r2
		r2  = (DanssEvent.NeutronX[0] - X) * (DanssEvent.NeutronX[0] - X);
		r2 += (DanssEvent.NeutronX[1] - Y) * (DanssEvent.NeutronX[1] - Y);
		r2 += (DanssEvent.NeutronX[2] - Z) * (DanssEvent.NeutronX[2] - Z);
		if (r2 > RMAX*RMAX) continue;		// 30 cm
		N5++;
//	Remove event if beta-afected strip hit
		k = 0;
		for (j=0; j<DanssEvent.NHits; j++) {
			if (HitType[j].type != 0) continue;	// Select SiPM
			if (HitType[j].z == 48 && HitType[j].xy == 12) k++;
			if (HitType[j].z == 49 && (HitType[j].xy == 11 || HitType[j].xy == 12)) k++;
		}
		if (k != 0) continue;
		N6++;
//	Request opposite hits in at least one projection
		east = west = north = south = up = down = 0;
		for (j=0; j<DanssEvent.NHits; j++) {
			if (HitType[j].type != 0) continue;	// Select SiPM
			if (HitType[j].z < 49) down++;
			if (HitType[j].z > 49) up++;
			if ((HitType[j].z & 1) && HitType[j].xy < 12) east++; 
			if ((HitType[j].z & 1) && HitType[j].xy > 12) west++; 
			if ((HitType[j].z & 1) == 0 && HitType[j].xy < 12) north++; 
			if ((HitType[j].z & 1) == 0 && HitType[j].xy > 12) south++; 
		}
// don't use		if (!((east && west) || (north && south) || (up && down))) continue;
		N7++;
//	Find maximum hits
		SiPmMax = PmtMax = 0;
		for (j=0; j<DanssEvent.NHits; j++) {
			switch (HitType[j].type) {
			case 0:	// SiPM
				if (HitE[j] > SiPmMax) SiPmMax = HitE[j];
				break;
			case 1:	// PMT
				if (HitE[j] > PmtMax) PmtMax = HitE[j];
				break;
			}
		}
//	Fill
		E = (DanssEvent.SiPmCleanEnergy+DanssEvent.PmtCleanEnergy)/2.0;
		hSum->Fill(E * scale);
		hSiPM->Fill(DanssEvent.SiPmCleanEnergy * scale);
		hPMT->Fill(DanssEvent.PmtCleanEnergy * scale);
//		hSiPM->Fill(SiPmMax * scale);
//		hPMT->Fill(PmtMax * scale);
		if (E > 1.0 && E < 2.6) hHits->Fill(DanssEvent.SiPmCleanHits);
	}
	printf("Cut rejection: %Ld(total) %Ld(xyz) %Ld(veto) %Ld(hits) %Ld(noise) %Ld(r2) %Ld(strips) %Ld(opposite)\n", 
		N, N1, N2, N3, N4, N5, N6, N7);
}

void draw_Exp(TChain *tExpA, TChain *tExpB, TChain *tInfoA, TChain *tInfoB, const char *name, const char *fname, double X, double Y, double Z, double RMAX)
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
	TCut cE("(SiPmCleanEnergy+PmtCleanEnergy)/2.0 > 1.0 && (SiPmCleanEnergy+PmtCleanEnergy)/2.0 < 2.6");
	TCut cSel = cxyz && cVeto && cn && !cNoise;
	
	tExpA->Project("hXY", "NeutronX[1]+2:NeutronX[0]+2", cSel);
	do_projections(tExpA, hExpA, hExpSiPMA, hExpPMTA, hHitsA, X, Y, Z, RMAX, 1.0);
	do_projections(tExpB, hExpB, hExpSiPMB, hExpPMTB, hHitsB, X, Y, Z, RMAX, 1.0);
	
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
	
	hExpA->Scale(1.0/timeA);
	hExpB->Scale(1.0/timeB);
	hExpSiPMA->Scale(1.0/timeA);
	hExpSiPMB->Scale(1.0/timeB);
	hExpPMTA->Scale(1.0/timeA);
	hExpPMTB->Scale(1.0/timeB);
	hHitsA->Scale(1.0/timeA);
	hHitsB->Scale(1.0/timeB);
	
	hExpC->Add(hExpA, hExpB, 1.0, -1.0);
	hExpSiPMC->Add(hExpSiPMA, hExpSiPMB, 1.0, -1.0);
	hExpPMTC->Add(hExpPMTA, hExpPMTB, 1.0, -1.0);
	hHitsC->Add(hHitsA, hHitsB, 1.0, -1.0);

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

void draw_MC(TChain *tMc, const char *name, const char *fname, double scale, double X, double Y, double Z, double RMAX)
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
	sprintf(str, "(NeutronX[0]+2-%5.1f)*(NeutronX[0]+2-%5.1f) + (NeutronX[1]+2-%5.1f)*(NeutronX[1]+2-%5.1f) + "
		"(NeutronX[2]+0.5-%5.1f)*(NeutronX[2]+0.5-%5.1f) < %5.1f*%5.1f", X, X, Y, Y, Z, Z, RMAX, RMAX);
	TCut cR2(str);
	TCut cE("(SiPmCleanEnergy+PmtCleanEnergy)/2.0 > 1.0 && (SiPmCleanEnergy+PmtCleanEnergy)/2.0 < 2.6");
	
	tMc->Project("hMcXY", "NeutronX[1]+2:NeutronX[0]+2", cSel && cE);
	do_projections(tMc, hMc, hMcSiPM, hMcPMT, hMcHits, X, Y, Z, RMAX, scale);
//	sprintf(str, "%5.3f*(SiPmCleanEnergy+PmtCleanEnergy)/2.0", scale);
//	tMc->Project("hMc", str, cSel && cR2);
//	sprintf(str, "%5.3f*SiPmCleanEnergy", scale);
//	tMc->Project("hMcSiPM", str, cSel && cR2);
//	sprintf(str, "%5.3f*PmtCleanEnergy", scale);
//	tMc->Project("hMcPMT", str, cSel && cR2);
//	tMc->Project("hMcHits", "SiPmCleanHits", cSel && cR2 && cE);

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

void draw_Sources6(int iser, const char *rootdir = "root8n2", double scale = 1.0, double RMAX = 30, int max_files = 0)
{
	const char *name = "";
	char fname[1024];
	char str[1024];
	TCut cXY;
	TCut cZ;
	int code;
	int i;
	double X, Y, Z;
	
	TChain *tMc = new TChain("DanssEvent");
	TChain *tExpA = new TChain("DanssEvent");
	TChain *tExpB = new TChain("DanssEvent");
	TChain *tInfoA = new TChain("DanssInfo");
	TChain *tInfoB = new TChain("DanssInfo");
	TChain *tRawA = new TChain("RawHits");
	TChain *tRawB = new TChain("RawHits");
	tExpA->AddFriend(tRawA);
	tExpB->AddFriend(tRawB);
	
	X = Z = 50;
	code = iser / 1000;
	switch (iser) {
	case 1:		// Na Feb 17, center
		Add2Chain(tExpA, 12376, 12407, rootdir, max_files);
		Add2Chain(tRawA, 12376, 12407, rootdir, max_files);
		Add2Chain(tInfoA, 12376, 12407, rootdir, max_files);
		Add2Chain(tExpB, 12204, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12204, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12204, 12303, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_feb17_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 2:		// Na Feb 17, edge
		Add2Chain(tExpA, 12364, 12373, rootdir, max_files);
		Add2Chain(tRawA, 12364, 12373, rootdir, max_files);
		Add2Chain(tInfoA, 12364, 12373, rootdir, max_files);
		Add2Chain(tExpB, 12204, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12204, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12204, 12303, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_feb17_edge_%s_R%4.1f", rootdir, RMAX);
		Y = 90;
		break;
	case 11:		// Na Nov 18, center
		Add2Chain(tExpA, 51099, 51161, rootdir, max_files);
		Add2Chain(tRawA, 51099, 51161, rootdir, max_files);
		Add2Chain(tInfoA, 51099, 51161, rootdir, max_files);
		Add2Chain(tExpB, 51168, 51267, rootdir, max_files);
		Add2Chain(tRawB, 51168, 51267, rootdir, max_files);
		Add2Chain(tInfoB, 51168, 51267, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_nov18_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 12:		// Na Nov 18, edge
		Add2Chain(tExpA, 51036, 51095, rootdir, max_files);
		Add2Chain(tRawA, 51036, 51095, rootdir, max_files);
		Add2Chain(tInfoA, 51036, 51095, rootdir, max_files);
		Add2Chain(tExpB, 51168, 51267, rootdir, max_files);
		Add2Chain(tRawB, 51168, 51267, rootdir, max_files);
		Add2Chain(tInfoB, 51168, 51267, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_nov18_edge_%s_R%4.1f", rootdir, RMAX);
		Y = 90;
		break;
	case 21:		// Na June 22, center DOWN
		Add2Chain(tExpA, 127389, 127456, rootdir, max_files);
		Add2Chain(tRawA, 127389, 127456, rootdir, max_files);
		Add2Chain(tInfoA, 127389, 127456, rootdir, max_files);
		Add2Chain(tExpB, 127280, 127379, rootdir, max_files);
		Add2Chain(tRawB, 127280, 127379, rootdir, max_files);
		Add2Chain(tInfoB, 127280, 127379, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_jun22_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 31:		// Na June 22, center UP
		Add2Chain(tExpA, 127958, 128016, rootdir, max_files);
		Add2Chain(tRawA, 127958, 128016, rootdir, max_files);
		Add2Chain(tInfoA, 127958, 128016, rootdir, max_files);
		Add2Chain(tExpB, 128020, 128119, rootdir, max_files);
		Add2Chain(tRawB, 128020, 128119, rootdir, max_files);
		Add2Chain(tInfoB, 128020, 128119, rootdir, max_files);
		name = "22Na";
		sprintf(fname, "22Na_jun22_centerUP_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 101:		// Co Feb 17, center
		Add2Chain(tExpA, 12306, 12346, rootdir, max_files);
		Add2Chain(tRawA, 12306, 12346, rootdir, max_files);
		Add2Chain(tInfoA, 12306, 12346, rootdir, max_files);
		Add2Chain(tExpB, 12198, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12198, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12198, 12303, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_feb17_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 102:		// Co Feb 17, edge
		Add2Chain(tExpA, 12348, 12361, rootdir, max_files);
		Add2Chain(tRawA, 12348, 12361, rootdir, max_files);
		Add2Chain(tInfoA, 12348, 12361, rootdir, max_files);
		Add2Chain(tExpB, 12198, 12303, rootdir, max_files);
		Add2Chain(tRawB, 12198, 12303, rootdir, max_files);
		Add2Chain(tInfoB, 12198, 12303, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_feb17_edge_%s_R%4.1f", rootdir, RMAX);
		Y = 90;
		break;
	case 111:		// Co Nov 18, center
		Add2Chain(tExpA, 50949, 50997, rootdir, max_files);
		Add2Chain(tRawA, 50949, 50997, rootdir, max_files);
		Add2Chain(tInfoA, 50949, 50997, rootdir, max_files);
		Add2Chain(tExpB, 50750, 50873, rootdir, max_files);
		Add2Chain(tRawB, 50750, 50873, rootdir, max_files);
		Add2Chain(tInfoB, 50750, 50873, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_nov18_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 112:		// Co Nov 18, edge
		Add2Chain(tExpA, 50999, 51034, rootdir, max_files);
		Add2Chain(tRawA, 50999, 51034, rootdir, max_files);
		Add2Chain(tInfoA, 50999, 51034, rootdir, max_files);
		Add2Chain(tExpB, 50750, 50873, rootdir, max_files);
		Add2Chain(tRawB, 50750, 50873, rootdir, max_files);
		Add2Chain(tInfoB, 50750, 50873, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_nov18_edge_%s_R%4.1f", rootdir, RMAX);
		Y = 90;
		break;
	case 121:		// Co June 22, center
		Add2Chain(tExpA, 127774, 127837, rootdir, max_files);
		Add2Chain(tRawA, 127774, 127837, rootdir, max_files);
		Add2Chain(tInfoA, 127774, 127837, rootdir, max_files);
		Add2Chain(tExpB, 127838, 127900, rootdir, max_files);
		Add2Chain(tRawB, 127838, 127900, rootdir, max_files);
		Add2Chain(tInfoB, 127838, 127900, rootdir, max_files);
		name = "60Co";
		sprintf(fname, "60Co_jun22_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 221:		// 24Na June 22, center, run 1
		Add2Chain(tExpA, 127513, 127580, rootdir, max_files);
		Add2Chain(tRawA, 127513, 127580, rootdir, max_files);
		Add2Chain(tInfoA, 127513, 127580, rootdir, max_files);
		Add2Chain(tExpB, 127581, 127657, rootdir, max_files);
		Add2Chain(tRawB, 127581, 127657, rootdir, max_files);
		Add2Chain(tInfoB, 127581, 127657, rootdir, max_files);
		name = "24Na";
		sprintf(fname, "24Na_jun22a_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 223:		// 24Na June 22, center, run 2
		Add2Chain(tExpA, 127658, 127719, rootdir, max_files);
		Add2Chain(tRawA, 127658, 127719, rootdir, max_files);
		Add2Chain(tInfoA, 127658, 127719, rootdir, max_files);
		Add2Chain(tExpB, 127838, 127900, rootdir, max_files);
		Add2Chain(tRawB, 127838, 127900, rootdir, max_files);
		Add2Chain(tInfoB, 127838, 127900, rootdir, max_files);
		name = "24Na";
		sprintf(fname, "24Na_jun22b_center_%s_R%4.1f", rootdir, RMAX);
		Y = 50;
		break;
	case 1001:	// Na MC, center
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/mc_22Na_indLY_transcode_rawProc_pedSim_Center1.root");
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "v10/22Na_MC_center_%s_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	case 1002:	// Na MC, edge
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/mc_22Na_indLY_transcode_rawProc_pedSim_92_5_cm1.root");
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "v10/22Na_MC_edge_%s_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 90;
		break;
	case 1011:	// Na MC - full model, center
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/mc_22Na_indLY_transcode_rawProc_pedSim_Full_decay_Center1.root");
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "v10/22Na_MCF_center_%s_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	case 1012:	// Na MC - full model, edge
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/mc_22Na_indLY_transcode_rawProc_pedSim_Full_decay_92_5_cm1.root");
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "v10/22Na_MCF_edge_%s_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 90;
		break;
	case 1021:	// Na MC, center, full model, suffix to /home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Akagi/22Na
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Akagi/22Na/%s/mc_22Na_indLY_transcode_rawProc_pedSim_Center1.root", rootdir);
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "Akagi/22Na/%s/MC_center_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	case 1031:	// Na MC, center, full model, suffix to /home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Chikuma/22Na
//		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Chikuma/22Na/%s/mc_22Na_indLY_transcode_rawProc_pedSim_Center1.root", rootdir);
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Chikuma/22Na/%s/mc_22Na_indLY_transcode_rawProc_pedSim_Center1.root", rootdir);
		tMc->AddFile(str);
		name = "22Na";
		sprintf(fname, "Chikuma/22Na/%s/MC_center_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	case 1041:	// Na MC with SiPM noise
		for (i=0; i<40; i++) {
			sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Chikuma/22Na/Full_decay_center_Chikuma_xzmap/SiPmNoise/mc_22Na_indLY_transcode_rawProc_pedSim_1p%d.root", i);
			tMc->AddFile(str);
		}
		name = "22Na";
		sprintf(fname, "Chikuma/22Na/%s_noise/MC_center_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	case 1101:	// Co MC, center
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/mc_60Co_indLY_transcode_rawProc_pedSim_Center1.root");
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "v10/60Co_MC_center_%s_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	case 1102:	// Co MC, edge
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/RadSources_v10/mc_60Co_indLY_transcode_rawProc_pedSim_92_5_cm1.root");
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "v10/60Co_MC_edge_%s_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 90;
		break;
	case 1121:	// Co MC, center, Akagi, suffix to /home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Akagi/60Co
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Akagi/60Co/%s/mc_60Co_indLY_transcode_rawProc_pedSim_Center1.root", rootdir);
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "Akagi/60Co/%s/MC_center_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	case 1131:	// Co MC, center, chikuma, suffix to /home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Chikuma/60Co
		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n2/MC/Chikuma/60Co/%s/mc_60Co_indLY_transcode_rawProc_pedSim_Center1.root", rootdir);
//		sprintf(str, "/home/clusters/rrcmpi/alekseev/igor/root8n4/MC/Chikuma/60Co/%s/mc_60Co_indLY_transcode_rawProc_pedSim_Center1.root", rootdir);
		tMc->AddFile(str);
		name = "60Co";
		sprintf(fname, "Chikuma/60Co/%s/MC_center_S%5.3f_R%4.1f", rootdir, scale, RMAX);
		Y = 50;
		break;
	default:
		printf("%d - unknown\n", iser);
		printf("Available valuse for iser = MIDD:\n");
		printf("M  - MC (1) or experiment (0);\n");
		printf("I  - isotope: 24Na (2), 60Co (1) or 22Na (0);\n");
		printf("DD - serial for experiment:\n");
		printf("1  - February 17, center\n");
		printf("2  - February 17, edge\n");
		printf("11 - November 18, center\n");
		printf("12 - November 18, edge\n");
		printf("21 - June 22, center\n");
		printf("23 - June 22, center, 24Na second run\n");
		printf("31 - June 22, center, UP\n");
		printf("DD - for MC:\n");
		printf("1  - center (50, 50, 50) position\n");
		printf("2  - edge (50, 90, 50) position\n");
		printf("11 - center (50, 50, 50) position, full decay\n");
		printf("12 - edge (50, 90, 50) position, full decay\n");
		printf("21 - center (50, 50, 50) position, Akagi. Use rootdir to set suffix\n");
		printf("The whole list: 1 2 11 12 21 31 101 102 111 112 121 221 223 1001 1002 1011 1012 1021 1101 1102 1121\n");
		code = -1;
		break;
	}

	switch (code) {
	case 0:	// Experiment
		draw_Exp(tExpA, tExpB, tInfoA, tInfoB, name, fname, X, Y, Z, RMAX);
		break;
	case 1:	// MC
		draw_MC(tMc, name, fname, scale, X, Y, Z, RMAX);
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

/*
	make strip load histograms
*/
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
	Draw scale scan
	what = "22Na" or "60Co"
	when = "feb17", "nov18" or "jun22"
	where = "center", "centerUP" or "edge"
	version = "Full_decay_center_Akagi" etc.
	RMAX - maximum distance from the source ()
*/
void draw_scale_scan(const char *what, const char *when, const char *where, const char *version, 
	double RMAX = 30)
{
	const char *exppattern = "%s_%s_%s_root8n2_R%4.1f.root"; 		// what, when, where, RMAX
//	const char *exppattern = "%s_%s_%s_ODroot8n4_R%4.1f.root"; 		// what, when, where, RMAX
//	const char *MCpattern = "v10/%s_%s_%s_%s_S%5.3f_R%4.1f.root";	// what, mcsuffix, whereMC, version, scale, RMAX
//	const char *MCpattern = "Akagi/%s/%s/MC_%s_S%5.3f_R%4.1f.root";	// what, versionMC, whereMC, scale, RMAX
	const char *MCpattern = "Chikuma/%s/%s/MC_%s_S%5.3f_R%4.1f.root";	// what, versionMC, whereMC, scale, RMAX
	const int binMin = 11;
	const int binMax = 26;
	char expname[1024];
	char MCname[1024];
	char str[256];
	int i;
	double scale;
	double sMin, sMinSiPM, sMinPMT;
	TFile *fMC;
	TH1D *hMC;
	TH1D *hMCSiPM;
	TH1D *hMCPMT;
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	
	sprintf(str, "Scan over scale for %s, %s, %s;Scale;#chi^{2}", what, when, where);
	TH1D *hScan = new TH1D("hScan", str, 41, 0.8975, 1.1025);
	sprintf(str, "Scan over scale for %s, %s, %s, SiPM;Scale;#chi^{2}", what, when, where);
	TH1D *hScanSiPM = new TH1D("hScanSiPM", str, 41, 0.8975, 1.1025);
	sprintf(str, "Scan over scale for %s, %s, %s, PMT;Scale;#chi^{2}", what, when, where);
	TH1D *hScanPMT = new TH1D("hScanPMT", str, 41, 0.8975, 1.1025);
	hScan->SetMarkerStyle(kFullCircle);
	hScanSiPM->SetMarkerStyle(kFullCircle);
	hScanPMT->SetMarkerStyle(kFullCircle);
	hScan->SetMarkerSize(1.5);
	hScanSiPM->SetMarkerSize(1.5);
	hScanPMT->SetMarkerSize(1.5);

	sprintf(expname, exppattern, what, when, where, RMAX);
	TFile *fExp = new TFile(expname);
	if (!fExp->IsOpen()) return;
	TH1D *hExp = (TH1D *) fExp->Get("hExpC");
	TH1D *hExpSiPM = (TH1D *) fExp->Get("hExpSiPMC");
	TH1D *hExpPMT = (TH1D *) fExp->Get("hExpPMTC");
	if (!hExp || !hExpSiPM || !hExpPMT) {
		printf("Not all histograms found in %s\n", expname);
		return;
	}

	hExp->SetLineColor(kRed);
	hExp->SetMarkerColor(kRed);
	hExp->SetMarkerStyle(kFullCircle);
	hExpSiPM->SetLineColor(kRed);
	hExpSiPM->SetMarkerColor(kRed);
	hExpSiPM->SetMarkerStyle(kFullCircle);
	hExpPMT->SetLineColor(kRed);
	hExpPMT->SetMarkerColor(kRed);
	hExpPMT->SetMarkerStyle(kFullCircle);
	
	char *whereMC = strdup(where);
	if (!whereMC) return;
	if (strlen(whereMC) > strlen("center")) whereMC[strlen("center")] = '\0';
	
	for (i=0; i<41; i++) {
		scale = 0.9 + 0.005*i;
		sprintf(MCname, MCpattern, what, version, whereMC, scale, RMAX);
		fMC = new TFile(MCname);
		if (!fMC->IsOpen()) return;
		hMC = (TH1D *) fMC->Get("hMc");
		hMCSiPM = (TH1D *) fMC->Get("hMcSiPM");
		hMCPMT = (TH1D *) fMC->Get("hMcPMT");
		if (!hMC || !hMCSiPM || !hMCPMT) {
			printf("Not all histograms found in %s\n", MCname);
			return;
		}
		hMC->Scale(hExp->Integral(binMin, binMax) / hMC->Integral(binMin, binMax));
		hMCSiPM->Scale(hExpSiPM->Integral(binMin, binMax) / hMCSiPM->Integral(binMin, binMax));
		hMCPMT->Scale(hExpPMT->Integral(binMin, binMax) / hMCPMT->Integral(binMin, binMax));
		hScan->SetBinContent(i+1, chi2Diff(hExp, hMC, binMin, binMax));
		hScanSiPM->SetBinContent(i+1, chi2Diff(hExpSiPM, hMCSiPM, binMin, binMax));
		hScanPMT->SetBinContent(i+1, chi2Diff(hExpPMT, hMCPMT, binMin, binMax));
		fMC->Close();
	}
	
	TF1 *fPol2 = new TF1("fPol2", "pol2", 0.1, 10);
	TLatex txt;
	
	TCanvas *cv = new TCanvas("CV", "CV", 1440, 1000);
	cv->Divide(3, 2);
	
	cv->cd(4);
	hScan->Fit(fPol2, "q");
	sMin = -fPol2->GetParameter(1) / (2 * fPol2->GetParameter(2));
	hScan->Fit(fPol2, "q", "", sMin - 0.05, sMin + 0.05);
	hScan->DrawCopy();
	sMin = -fPol2->GetParameter(1) / (2 * fPol2->GetParameter(2));
	sprintf(str, "scale = %5.3f", sMin);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#chi^{2}_{min} = %6.1f", fPol2->Eval(sMin));
	txt.DrawLatexNDC(0.4, 0.72, str);

	cv->cd(5);
	hScanSiPM->Fit(fPol2, "q");
	sMinSiPM = -fPol2->GetParameter(1) / (2 * fPol2->GetParameter(2));
	hScanSiPM->Fit(fPol2, "q", "", sMinSiPM - 0.05, sMinSiPM + 0.05);
	hScanSiPM->DrawCopy();
	sMinSiPM = -fPol2->GetParameter(1) / (2 * fPol2->GetParameter(2));
	sprintf(str, "scale = %5.3f", sMinSiPM);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#chi^{2}_{min} = %6.1f", fPol2->Eval(sMinSiPM));
	txt.DrawLatexNDC(0.4, 0.72, str);

	cv->cd(6);
	hScanPMT->Fit(fPol2, "q");
	sMinPMT = -fPol2->GetParameter(1) / (2 * fPol2->GetParameter(2));
	hScanPMT->Fit(fPol2, "q", "", sMinPMT - 0.05, sMinPMT + 0.05);
	hScanPMT->DrawCopy();
	sMinPMT = -fPol2->GetParameter(1) / (2 * fPol2->GetParameter(2));
	sprintf(str, "scale = %5.3f", sMinPMT);
	txt.DrawLatexNDC(0.4, 0.8, str);
	sprintf(str, "#chi^{2}_{min} = %6.1f", fPol2->Eval(sMinPMT));
	txt.DrawLatexNDC(0.4, 0.72, str);
	
	i = (sMin - 0.8975) / 0.005;
	if (i < 0) i = 0;
	if (i > 41) i = 41;
	scale = 0.9 + 0.005*i;
	sprintf(MCname, MCpattern, what, version, whereMC, scale, RMAX);
	fMC = new TFile(MCname);
	if (!fMC->IsOpen()) return;
	hMC = (TH1D *) fMC->Get("hMc");
	hMCSiPM = (TH1D *) fMC->Get("hMcSiPM");
	hMCPMT = (TH1D *) fMC->Get("hMcPMT");
	if (!hMC || !hMCSiPM || !hMCPMT) {
		printf("Not all histograms found in %s\n", MCname);
		return;
	}
	hMC->Scale(hExp->Integral(binMin, binMax) / hMC->Integral(binMin, binMax));
	hMCSiPM->Scale(hExpSiPM->Integral(binMin, binMax) / hMCSiPM->Integral(binMin, binMax));
	hMCPMT->Scale(hExpPMT->Integral(binMin, binMax) / hMCPMT->Integral(binMin, binMax));

	cv->cd(1);
	hExp->DrawCopy();
	hMC->DrawCopy("same,hist");
	TLegend *lg = new TLegend(0.5, 0.6, 0.75, 0.75);
	lg->AddEntry(hExp, "Data", "pe");
	lg->AddEntry(hMC, "MC", "l");
	lg->Draw();

	cv->cd(2);
	hExpSiPM->DrawCopy();
	hMCSiPM->DrawCopy("same,hist");
	lg->Draw();

	cv->cd(3);
	hExpPMT->DrawCopy();
	hMCPMT->DrawCopy("same,hist");
	lg->Draw();

	sprintf(str, "%s_%s_%s_%s_R%4.1f_scan.png", what, when, where, version, RMAX);
	cv->SaveAs(str);

	sprintf(str, "%s_%s_%s_%s_R%4.1f_scan.root", what, when, where, version, RMAX);
	TFile *fOut = new TFile(str, "RECREATE");
	if (!fOut->IsOpen()) return;
	hExp->Write();
	hMC->Write();
	hScan->Write();
	hExpSiPM->Write();
	hMCSiPM->Write();
	hScanSiPM->Write();
	hExpPMT->Write();
	hMCPMT->Write();
	hScanPMT->Write();
	fOut->Close();

	fExp->Close();
	fMC->Close();
}

/*
        Draw nHits
*/
void draw_nHits(const char *what, const char *when = "jun22", const char *where = "center", const char *version = "root8n2", double RMAX = 30)
{
	const char *exppattern = "%s_%s_%s_%s_R%4.1f.root"; 		// what, when, where, version, RMAX
	const char *MCpattern = "%s_MC_%s_%s_S%5.3f_R%4.1f.root";	// what, whereMC, version, scale, RMAX
	char expname[1024];
	char MCname[1024];
	char str[1024];
	
	sprintf(expname, exppattern, what, when, where, version, RMAX);
	TFile *fExp = new TFile(expname);
	if (!fExp->IsOpen()) return;
	TH1D *hHits = (TH1D *) fExp->Get("hHitsC");
	
	char *whereMC = strdup(where);
	if (!whereMC) return;
	if (strlen(whereMC) > strlen("center")) whereMC[strlen("center")] = '\0';
	sprintf(MCname, MCpattern, what, whereMC, version, 1.0, RMAX);
	TFile *fMC = new TFile(MCname);
	if (!fMC->IsOpen()) return;
	TH1D *hMcHits = (TH1D *) fMC->Get("hMcHits");
	
	hHits->SetLineColor(kRed);
	hHits->SetMarkerColor(kRed);
	hHits->SetMarkerStyle(kFullCircle);
	hHits->SetStats(0);
	hMcHits->SetLineWidth(2);
	hMcHits->SetLineColor(kBlue);
	
	hMcHits->Scale(hHits->Integral() / hMcHits->Integral());
	hMcHits->DrawCopy("hist");
	hHits->DrawCopy("same");
	
	sprintf(str, "%s_%s_%s_%s_R%4.1f_nHits.png", what, when, where, version, RMAX);
	gPad->SaveAs(str);

	sprintf(str, "%s_%s_%s_%s_R%4.1f_nHits.root", what, when, where, version, RMAX);
	TFile *fOut = new TFile(str, "RECREATE");
	if (!fOut->IsOpen()) return;
	hHits->Write();
	hMcHits->Write();
	fOut->Close();
	
	fExp->Close();
	fMC->Close();
}
