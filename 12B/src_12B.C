/*
    A collection of functions for 12B calibration
*/
#include "evtbuilder.h"
const int Nbins = 80;

struct ExpDataStruct {
	double *ExpSignal;
	double *ExpRandom;
	int NExpSignal;
	int NExpRandom;
};

struct MCDataStruct {
	TMatrixD *CentralMatrix[3];
	TMatrixD *BirksMatrix[3];
	TMatrixD *CherMatrix[3];
};

struct BinRangeStruct {
	int From;
	int Till;
};

struct FitParametersStruct {
//		12C(n,p)12B
//	Experiment:
	struct ExpDataStruct ExpIA;
//	MC:
	struct MCDataStruct MCIA;
//		12C(mu-,nu)12B
//	Experiment:
	struct ExpDataStruct ExpAY;
//	MC:
	struct MCDataStruct MCAY;
//	Fit
	TVectorD *Spectrum;
	int iClusterEnergySelection;
	double FunMap[7][7][7];
	int iIA;
	int iAY;
	double Sigma[6];
	struct BinRangeStruct rangeIA;
	struct BinRangeStruct rangeAY;
} FitPar = {
	.Spectrum = NULL,
	.iClusterEnergySelection = -1,
};

//	Signal tree
	/*****************************************************************************
	*	Tree from Sasha Yakovleva					     *
	*        :SummaryBranch : event_id/L:muon_event_id/L:time/D:sipm_energy/D:   *
	*         | sipm_energy_xz/D:sipm_energy_yz/D:sipm_energy_attn/D:            *
	*         | sipm_energy_attn_xz/D:sipm_energy_attn_yz/D:pmt_energy/D:        *
	*         | pmt_energy_xz/D:pmt_energy_yz/D:pmt_energy_attn/D:               *
	*         | pmt_energy_attn_xz/D:pmt_energy_attn_yz/D:mix_energy/D:          *
	*         | mix_energy_attn/D:max_sipm_energy/D:max_pmt_energy/D:            *
	*         | total_hits/L:closest_strip_energy/D:cluster_energy/D:            *
	*         | cluster_energy_attn/D:pmt_cluster_energy/D:                      *
	*         | pmt_cluster_energy_attn/D:birth_coord_x/D:birth_coord_y/D:       *
	*         | birth_coord_z/D:prev_event_sipm_energy/D:prev_event_pmt_energy/D:*
	*         | prev_event_time/D:next_event_sipm_energy/D:                      *
	*         | next_event_pmt_energy/D:next_event_time/D:veto_sipm_energy/D:    *
	*         | veto_pmt_energy/D:veto_signal/D:time_from_veto/D:                *
	*         | has_nearest_strip/I:was_decay/I                                  *
	******************************************************************************/
struct BorumInfo {
	Long64_t event_id;
	Long64_t muon_event_id;
	Double_t time;
	Double_t sipm_energy;
	Double_t sipm_energy_xz;
	Double_t sipm_energy_yz;
	Double_t sipm_energy_attn;
	Double_t sipm_energy_attn_xz;
	Double_t sipm_energy_attn_yz;
	Double_t pmt_energy;
	Double_t pmt_energy_xz;
	Double_t pmt_energy_yz;
	Double_t pmt_energy_attn;
	Double_t pmt_energy_attn_xz;
	Double_t pmt_energy_attn_yz;
	Double_t mix_energy;
	Double_t mix_energy_attn;
	Double_t max_sipm_energy;
	Double_t max_pmt_energy;
	Long64_t total_hits;
	Double_t closest_strip_energy;
	Double_t cluster_energy;
	Double_t cluster_energy_attn;
	Double_t pmt_cluster_energy;
	Double_t pmt_cluster_energy_attn;
//	Double_t mix_cluster_energy;
//	Double_t mix_cluster_energy_attn;
	Double_t birth_coord_x;
	Double_t birth_coord_y;
	Double_t birth_coord_z;
//	Double_t total_hits_in_cluster;
//	Double_t max_energy_hit_in_cluster;
	Double_t prev_event_sipm_energy;
	Double_t prev_event_pmt_energy;
	Double_t prev_event_time;
	Double_t next_event_sipm_energy;
	Double_t next_event_pmt_energy;
	Double_t next_event_time;
	Double_t veto_sipm_energy;
	Double_t veto_pmt_energy;
	Double_t veto_signal;
	Double_t time_from_veto;
	Int_t has_nearest_strip;
	Int_t was_decay;
};
//	Background tree
	/*****************************************************************************
	*Br    0 :SummaryBranch : event_id/L:muon_event_id/L:time/D:sipm_energy/D:   *
	*         | sipm_energy_attn/D:pmt_energy/D:pmt_energy_attn/D:mix_energy/D:  *
	*         | mix_energy_attn/D:max_sipm_energy/D:max_pmt_energy/D:            *
	*         | total_hits/L:closest_strip_energy/D:cluster_energy/D:            *
	*         | cluster_energy_attn/D:pmt_cluster_energy/D:                      *
	*         | pmt_cluster_energy_attn/D:birth_coord_x/D:birth_coord_y/D:       *
	*         | birth_coord_z/D:prev_event_sipm_energy/D:prev_event_pmt_energy/D:*
	*         | prev_event_time/D:next_event_sipm_energy/D:                      *
	*         | next_event_pmt_energy/D:next_event_time/D:veto_sipm_energy/D:    *
	*         | veto_pmt_energy/D:veto_signal/D:time_from_veto/D:                *
	*         | has_nearest_strip/I:was_decay/I                                  *
	******************************************************************************/
struct BorumInfoBgnd {
	Long64_t event_id;
	Long64_t muon_event_id;
	Double_t time;
	Double_t sipm_energy;
	Double_t sipm_energy_attn;
	Double_t pmt_energy;
	Double_t pmt_energy_attn;
	Double_t mix_energy;
	Double_t mix_energy_attn;
	Double_t max_sipm_energy;
	Double_t max_pmt_energy;
	Long64_t total_hits;
	Double_t closest_strip_energy;
	Double_t cluster_energy;
	Double_t cluster_energy_attn;
	Double_t pmt_cluster_energy;
	Double_t pmt_cluster_energy_attn;
	Double_t birth_coord_x;
	Double_t birth_coord_y;
	Double_t birth_coord_z;
	Double_t prev_event_sipm_energy;
	Double_t prev_event_pmt_energy;
	Double_t prev_event_time;
	Double_t next_event_sipm_energy;
	Double_t next_event_pmt_energy;
	Double_t next_event_time;
	Double_t veto_sipm_energy;
	Double_t veto_pmt_energy;
	Double_t veto_signal;
	Double_t time_from_veto;
	Int_t has_nearest_strip;
	Int_t was_decay;
};

/*
	Delete object by name if exists
*/
void delete_if_exist(const char *name)
{
	auto tmp = gROOT->FindObject(name);
	if (tmp) delete tmp;
}

/*
	Calculate chi2 of two histograms difference
*/
double chi2Diff(const TH1D *hA, const TH1D *hB, int binMin, int binMax)
{
	double sum;
	double err;
	int i;
	sum = 0;
	for (i = binMin; i <= binMax; i++) {
		err = hA->GetBinError(i) * hA->GetBinError(i) + hB->GetBinError(i) * hB->GetBinError(i);
		if (err <= 0) {
			err += 10000;	// like 100 sigma
		} else {
			sum += (hA->GetBinContent(i) - hB->GetBinContent(i)) * (hA->GetBinContent(i) - hB->GetBinContent(i)) / err;
		}
	}
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
void src_12B(int from, int to, const char *format = "/home/clusters/rrcmpi/alekseev/igor/muon8n7/%3.3dxxx/muon_%6.6d.root")
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

void Chikuma_All_MC(void)
{
	const char *MCvar[] = { "DB_spectrum_Chikuma", 
		"DB_spectrum_Chikuma_Birks_el_0_0108",
		"DB_spectrum_Chikuma_Birks_el_0_0308",
		"DB_spectrum_Chikuma_Cher_coeff_0_033",
		"DB_spectrum_Chikuma_Cher_coeff_0_233",
		"DB_spectrum_Chikuma_main_Birks_0_0108",
		"DB_spectrum_Chikuma_main_Birks_0_0308",
		"DB_spectrum_Chikuma_paint_0_15",
		"DB_spectrum_Chikuma_paint_0_45"};
	const char *MCdir = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/Chikuma/12B";
	char namein[4096];
	char nameout[4096];
	int i;
	
	for (i=0; i < sizeof(MCvar)/sizeof(MCvar[0]); i++) {
		sprintf(namein,
			"%s/%s/mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum1.root %s/%s/mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum2.root",
			MCdir, MCvar[i], MCdir, MCvar[i]);
		sprintf(nameout, "MC12B_8.7-%s.hist.root", MCvar[i]);
		src_12BMC(namein, nameout);
	}
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

void scan_all87(void)
{
	const char *MCvar[] = { "DB_spectrum_Chikuma", 
		"DB_spectrum_Chikuma_Birks_el_0_0108",
		"DB_spectrum_Chikuma_Birks_el_0_0308",
		"DB_spectrum_Chikuma_Cher_coeff_0_033",
		"DB_spectrum_Chikuma_Cher_coeff_0_233",
		"DB_spectrum_Chikuma_main_Birks_0_0108",
		"DB_spectrum_Chikuma_main_Birks_0_0308",
		"DB_spectrum_Chikuma_paint_0_15",
		"DB_spectrum_Chikuma_paint_0_45"};
	char nameMC[4096];
	int i;
	
	for (i=0; i < sizeof(MCvar)/sizeof(MCvar[0]); i++) {
		sprintf(nameMC, "MC12B-%s.hist.root", MCvar[i]);
		scan_12B("12B_exp87_2210_175528.root", nameMC);
	}
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
	printf("Input trees of %lld and %lld events\n", tMC->GetEntries(), tMCorig->GetEntries());
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
 *	fname - output file name	*
 *	cuts - event selection cuts	*
 ****************************************/
void MakeChikumaMatrixes(const char *fname, const char *cuts)
{
	const char *ChikumaDir = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/Chikuma/12B";
	const char *MCRawDir = "/home/clusters/rrcmpi/danss/MC_RAW/Chikuma/12B";
	const char *mcnames[] = {
		"DB_spectrum_Chikuma", 
		"DB_spectrum_Chikuma_Birks_el_0_0108", 
		"DB_spectrum_Chikuma_Birks_el_0_0308", 
		"DB_spectrum_Chikuma_Cher_coeff_0_033",
		"DB_spectrum_Chikuma_Cher_coeff_0_233",
		"DB_spectrum_Chikuma_main_Birks_0_0108", 
		"DB_spectrum_Chikuma_main_Birks_0_0308",
		"DB_spectrum_Chikuma_paint_0_15",
		"DB_spectrum_Chikuma_paint_0_45"};
	const char *filelist[] = {"mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum1.root", 
		"mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum2.root"};
//	const char *rawlist[] = {"DANSSmod0_1.root", "DANSSmod0_2.root"};
	const char *rawlist[] = {"DANSS0_1.root", "DANSS0_2.root"};
	const char *varlist[] = {"PositronEnergy", "PositronSiPmEnergy", "PositronPmtEnergy"};
	const char *mixname[] = {"Birks_el", "Cher_coeff", "main_Birks", "paint"};
	int i, j;
	char str[4096];
	char raw[4096];
	char name[256];
	TMatrixD *M[17][3];
	
	TFile *fOut = new TFile(fname, "RECREATE");
	if (!fOut->IsOpen()) return;
	
	for (i = 0; i < sizeof(mcnames) / sizeof(mcnames[0]); i++) {
		str[0] = '\0';
		for (j = 0; j < sizeof(filelist) / sizeof(filelist[0]); j++) sprintf(&str[strlen(str)], "%s/%s/%s ", ChikumaDir, mcnames[i], filelist[j]);
		raw[0] = '\0';
		for (j = 0; j < sizeof(rawlist) / sizeof(rawlist[0]); j++) sprintf(&raw[strlen(raw)], "%s/%s/Ready/%s ", MCRawDir, mcnames[i], rawlist[j]);
		for (j = 0; j < sizeof(varlist) / sizeof(varlist[0]); j++) {
			M[i][j] = MCmatrix(str, varlist[j], cuts, raw);
			if (!M[i][j]) continue;
			sprintf(name, "m12B_%s_%s", mcnames[i], varlist[j]);
			fOut->cd();
			M[i][j]->Write(name);
		}
	}
	for (j = 0; j < sizeof(varlist) / sizeof(varlist[0]); j++) {
		for (i=9; i<17; i++) M[i][j] = new TMatrixD(Nbins, Nbins);
		*(M[9][j]) = (*(M[2][j]) - *(M[1][j])) * 0.5;
		*(M[10][j]) = (*(M[2][j]) + *(M[1][j])) * 0.5 - *(M[0][j]);
		*(M[11][j]) = (*(M[4][j]) - *(M[3][j])) * 0.5;
		*(M[12][j]) = (*(M[4][j]) + *(M[3][j])) * 0.5 - *(M[0][j]);
		*(M[13][j]) = (*(M[6][j]) - *(M[5][j])) * 0.5;
		*(M[14][j]) = (*(M[6][j]) + *(M[5][j])) * 0.5 - *(M[0][j]);
		*(M[15][j]) = (*(M[8][j]) - *(M[7][j])) * 0.5;
		*(M[16][j]) = (*(M[8][j]) + *(M[7][j])) * 0.5 - *(M[0][j]);
		for (i=0; i<8; i++) {
			sprintf(name, "m12B_%s_%s_%s", (i & 1) ? "Residual" : "Delta", mixname[i/2], varlist[j]);
			fOut->cd();
			M[9+i][j]->Write(name);
		}
	}
	
	raw[0] = '\0';
	for (j = 0; j < sizeof(rawlist) / sizeof(rawlist[0]); j++) sprintf(&raw[strlen(raw)], "%s/%s/Ready/%s ", MCRawDir, mcnames[0], rawlist[j]);
	auto S = MCvector(raw);
	fOut->cd();
	S->Write("v12B_ParticleEnergy");

	fOut->Close();
}

/********************************************************
 *	Make matrixes for Chikuma: both analyses	*
 ********************************************************/
void MakeAllChikumaMatrixes(void)
{
	MakeChikumaMatrixes("12B_Chikuma_matrixes_IA_v7.root", "AnnihilationEnergy < 0.06 && PositronEnergy > 3");
	MakeChikumaMatrixes("12B_Chikuma_matrixes_AY_v7.root", "AnnihilationEnergy < 0.06 && "
		"((PositronX[0] > 10 && PositronX[0] < 86) || PositronX[0] < 0) && "
		"((PositronX[1] > 10 && PositronX[1] < 86) || PositronX[1] < 0) && "
		"PositronX[2] > 11.5 && PositronX[2] < 87.5");
}

/****************************************************************
 *	Compare Delta and Residual - check for linearity	*
 *	fname - file name with matrixes				*
 *	suffix - the Delta  matrix name after m12B_Delta_	*
 ****************************************************************/
void DrawMatrixLinearity(const char *fname, const char *suffix)
{
	char str[1024];
	int i, j;

	TFile *fIn = new TFile(fname);
	if (!fIn->IsOpen()) return;
	
	sprintf(str, "m12B_Delta_%s", suffix);
	auto mD = (TMatrixD *) fIn->Get(str);
	sprintf(str, "m12B_Residual_%s", suffix);
	auto mR = (TMatrixD *) fIn->Get(str);
	if (!mD || !mR) {
		printf("Suffix %s not found in %s\n", suffix, fname);
		fIn->Close();
		return;
	}
	gROOT->cd();
	TVectorD *sD = new TVectorD(Nbins);
	TVectorD *sR = new TVectorD(Nbins);
	
	for (i=0; i<Nbins; i++) {
		(*sD)[i] = 0;
		(*sR)[i] = 0;
		for (j=0; j<Nbins; j++) {
			(*sD)[i] += fabs((*mD)[j][i]);
			(*sR)[i] += fabs((*mR)[j][i]);
		}
	}
	sD->Draw();
	sR->Draw("same");
	
	TLatex txt;
	txt.SetTextSize(0.05);
	sprintf(str, "File %s", fname);
	txt.DrawLatexNDC(0.12, 0.83, str);
	sprintf(str, "Matrix %s", suffix);
	txt.DrawLatexNDC(0.12, 0.76, str);
	
	fIn->Close();
}

/****************************************************************
 *	Compare Delta and Residual - check for linearity	*
 *	for all possible matrixes				*
 *	pdfname - pdf file name					*
 ****************************************************************/
void DrawMatrixLinearityAll(const char *pdfname)
{
	const char *fnames[] = {"12B_Chikuma_matrixes_IA.root", "12B_Chikuma_matrixes_AY.root"};
	const char *varlist[] = {"PositronEnergy", "PositronSiPmEnergy", "PositronPmtEnergy"};
	const char *mixname[] = {"Birks_el", "Cher_coeff", "main_Birks"};
	char str[1024];
	int i, j, k;
	
	gStyle->SetOptStat(0);
	TCanvas *cv = new TCanvas("CV", "CV", 800, 800);
	TString tstr(pdfname);
	cv->SaveAs((tstr + "[").Data());
	
	for (i = 0; i < sizeof(fnames) / sizeof(fnames[0]); i++) 
		for (j = 0; j < sizeof(mixname) / sizeof(mixname[0]); j++) 
		for (k = 0; k < sizeof(varlist) / sizeof(varlist[0]); k++) {
		sprintf(str, "%s_%s", mixname[j], varlist[k]);
		DrawMatrixLinearity(fnames[i], str);
		cv->SaveAs(tstr.Data());
	}
	cv->SaveAs((tstr + "]").Data());
	delete cv;
}

/****************************************************************
 *	To increase speed of calculations an experimental tree	*
 *	with only three energies is created			*
 *	from - the first run					*
 *	to - the last run					*
 ****************************************************************/
void MakeShortTree(int from, int to)
{
	const char *format = "/home/clusters/rrcmpi/alekseev/igor/muon8n7/%3.3dxxx/muon_%6.6d.root";
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
	
	sprintf(str, "12B_events_8n7_%6.6d_%6.6d.root", from, to);
	TFile *fOut = new TFile(str, "RECREATE");
	TTree *tOut = new TTree("MuonPair", "MuonPair");
	tOut->Branch("Pair", &event.ClusterEnergy, LeafList);
	TTree *tOutR = new TTree("MuonRandom", "MuonRandom");
	tOutR->Branch("Pair", &event.ClusterEnergy, LeafList);
	
	N = chA->GetEntries();
	for (i=0; i<N; i++) {
		chA->GetEntry(i);
		if (event.gtDiff < 500) continue;	// cut at 500 us
		if (event.ClusterEnergy < 3) continue;	// cut at 3 MeV
		tOut->Fill();
	}

	N = chR->GetEntries();
	for (i=0; i<N; i++) {
		chR->GetEntry(i);
		if (event.gtDiff < 500) continue;	// cut at 500 us
		if (event.ClusterEnergy < 3) continue;	// cut at 3 MeV
		tOutR->Fill();
	}
	
	fOut->cd();
	tOut->Write();
	tOutR->Write();
	fOut->Close();
}

/****************************************************************
 *	Add files from directory to chain of Sasha's data	*
 *	chain - chain to add files				*
 *	dir - directory to add					*
 ****************************************************************/
void add_to_chainAY(TChain *chain, const char *dir)
{
	TSystemDirectory *d = new TSystemDirectory("dir", dir);
	for (auto f : *d->GetListOfFiles()) {
		TString fname(f->GetName());
		if (fname.EndsWith(".root")) {
			TString path(d->GetTitle());
			path += "/";
			path += fname;
			chain->AddFile(path);
		}
	}
	delete d;
}

/****************************************************************
 *	Apply Sasha's cuts					*
 *	info - event information				*
 *	Return true if event is OK				*
 ****************************************************************/
int AY_cuts(struct BorumInfo *info)
{
//	Geometry cut: 12 cm from the edges
	if (!(info->birth_coord_x > 12 && info->birth_coord_x < 88 &&
		info->birth_coord_y > 12 && info->birth_coord_y < 88 &&
		info->birth_coord_z > 12 && info->birth_coord_z < 92)) return 0;
//	Energy beyond cluster 60 keV
	if (info->sipm_energy_attn - info->cluster_energy_attn > 0.06) return 0;
//	??
	if (info->has_nearest_strip > 0) return 0;
//	Time from Veto (300 us)
	if (info->time_from_veto < 300 * 125) return 0;
//	Decay cut
	if (info->was_decay) return 0;
	return 1;
}

/****************************************************************
 *	Apply Sasha's cuts to random background			*
 *	info - event information				*
 *	Return true if event is OK				*
 ****************************************************************/
int AY_cuts(struct BorumInfoBgnd *info)
{
//	Geometry cut: 12 cm from the edges
	if (!(info->birth_coord_x > 12 && info->birth_coord_x < 88 &&
		info->birth_coord_y > 12 && info->birth_coord_y < 88 &&
		info->birth_coord_z > 12 && info->birth_coord_z < 92)) return 0;
//	Energy beyond cluster 60 keV
	if (info->sipm_energy_attn - info->cluster_energy_attn > 0.06) return 0;
//	??
	if (info->has_nearest_strip > 0) return 0;
//	Time from Veto (300 us)
	if (info->time_from_veto < 300 * 125) return 0;
//	Decay cut
	if (info->was_decay) return 0;
	return 1;
}

/****************************************************************
 *	To increase speed of calculations an experimental tree	*
 *	with only three energies is created. AY version		*
 ****************************************************************/
void MakeShortTreeAY(void)
{
	const char *datadir = "/home/clusters/02/n_skrobova/yakovleva/calibration/boron_decays/fit/data_storage";
	const char *bgnd_1 = "/home/clusters/02/n_skrobova/yakovleva/calibration/boron_decays/fit/bg_1";
	const char *bgnd_2 = "/home/clusters/02/n_skrobova/yakovleva/calibration/boron_decays/fit/bg_2";
	char str[1024];
	struct {
		float mix_energy;
		float SiPm_energy;
		float Pmt_energy;
	} event;
	const char *LeafList = 
		"ClusterEnergy/F:"	// Energy sum of the cluster (SiPM)
		"ClusterSiPmEnergy/F:"	// SiPM energy in the cluster, corrected
		"ClusterPmtEnergy/F";	// PMT energy in the cluster, corrected
	long i, N;
	struct BorumInfo info;
	struct BorumInfoBgnd bgnd;

	TChain *chA = new TChain("BorumTree", "BorumPair");
	TChain *chR = new TChain("BorumTree", "BorumRandom");
	add_to_chainAY(chA, datadir);
	add_to_chainAY(chR, bgnd_1);
	add_to_chainAY(chR, bgnd_2);
	chA->SetBranchAddress("SummaryBranch", &info);
	chR->SetBranchAddress("SummaryBranch", &bgnd);
	
	TFile *fOut = new TFile("12B_events_AY.root", "RECREATE");
	TTree *tOut = new TTree("MuonPair", "MuonPair");
	tOut->Branch("Pair", &event, LeafList);
	TTree *tOutR = new TTree("MuonRandom", "MuonRandom");
	tOutR->Branch("Pair", &event, LeafList);
	
	N = chA->GetEntries();
	printf("%ld entries in signal chain\n", N);
	for (i=0; i<N; i++) {
		chA->GetEntry(i);
		if (!AY_cuts(&info)) continue;
		event.SiPm_energy = info.cluster_energy_attn;
		event.Pmt_energy = info.pmt_cluster_energy_attn;
		event.mix_energy = (event.SiPm_energy + event.Pmt_energy) / 2;
		tOut->Fill();
	}
	printf("%lld entries in output signal tree\n", tOut->GetEntries());

	N = chR->GetEntries();
	printf("%ld entries in background chain\n", N);
	for (i=0; i<N; i++) {
		chR->GetEntry(i);
		if (!AY_cuts(&bgnd)) continue;
		event.SiPm_energy = bgnd.cluster_energy_attn;
		event.Pmt_energy = bgnd.pmt_cluster_energy_attn;
		event.mix_energy = (event.SiPm_energy + event.Pmt_energy) / 2;
		tOutR->Fill();
	}
	printf("%lld entries in output background tree\n", tOutR->GetEntries());
	
	fOut->cd();
	tOut->Write();
	tOutR->Write();
	fOut->Close();
}

/****************************************************************
 *	Create experimental distribution with given scale	*
 *	Kscale - energy scale					*
 *	return 1D histogram with Signal - Random/16		*
 ****************************************************************/
TH1D *GetExperiment(double Kscale, struct ExpDataStruct *data, const char *name, const double r_fraction)
{
	int i;
	delete_if_exist(name);
	TH1D *hS = new TH1D(name, "", Nbins, 0, 20);
	TH1D *hR = new TH1D("__hRandom", "", Nbins, 0, 20);
	for(i=0; i<data->NExpSignal; i++) hS->Fill(Kscale * data->ExpSignal[i]);
	for(i=0; i<data->NExpRandom; i++) hR->Fill(Kscale * data->ExpRandom[i]);
	hS->Sumw2();
	hR->Sumw2();
	hS->Add(hR, -r_fraction);
	delete hR;
	return hS;
}

/****************************************************************
 *	Create a MC histogram in linear approximation for	*
 *	Kbirks and Kcher					*
 *	Kbirks - difference from central Birks coef / 0.01	*
 *	Kcher - difference from central Cherenkov coef / 0.1	*
 *	Mmain - central response matrix				*
 *	Mbirks - derivative on Kbirks response matrix		*
 *	Mcher - derivative on Kcher response matrix		*
 *	S - true electron spectrum				*
 *	name - name of the histogram				*
 ****************************************************************/
TH1D *GetMC(double Kbirks, double Kcher, TMatrixD *Mmain, TMatrixD *Mbirks, TMatrixD *Mcher, TVectorD *S, const char *name)
{
	int i;
	TVectorD R(Nbins);
	TMatrixD A(Nbins, Nbins);
	A = (*Mmain) + Kbirks * (*Mbirks) + Kcher * (*Mcher);
	R = A * (*S);
	delete_if_exist(name);
	TH1D *h = new TH1D(name, "", Nbins, 0, 20);
	for (i=0; i<Nbins; i++) {
		h->SetBinContent(i+1, R[i]);
		h->SetBinError(i+1, 0);
	}
	return h;
}

/*
    SiPM coefficients for other sources
    Kb - Birks for e+-, coef for 0.01 variation, Kb0 = 0.0208 cm/MeV
    Kc - Cherenkov, coef for 0.1 variation, Kc0 = 0.133 MeV/cm
    Kh - Birks for heavy, coef for 0.01 variation, Kh0 = 0.0208 cm/MeV
	mu    = 0.961 + 0.005*Kb - 0.032*Kc + 0.014*Kh
	12B   = 0.942 + 0.021*Kb - 0.034*Kc + 0*Kh
	22Na  = 0.999 + 0.056*Kb - 0.009*Kc + 0*Kh
	60Co  = 0.982 + 0.046*Kb - 0.012*Kc + 0*Kh
	248Cm = 0.966 + 0.033*Kb - 0.020*Kc + 0*Kh
	mudec = 0.934 + 0.016*Kb - 0.027*Kc + 0*Kh
	Bragg = 0.932 + 0.005*Kb - 0.015*Kc + 0.021*Kh
*/
double RFunction(double Kb, double Kc, double Kh, int num)
{
		// Kb, Kc, Kh, K0
	const double A[6][4] = {
		{0.005, -0.032, 0.014, 0.961},	// mu
//		{0.021, -0.034, 0, 0.942},	// 12B	- we use different approach to boron
		{0.056, -0.009, 0, 0.999},	// 22Na
		{0.046, -0.012, 0, 0.982},	// 60Co
		{0.033, -0.020, 0, 0.966},	// 248Cm
		{0.016, -0.027, 0, 0.934},	// mudec
		{0.005, -0.015, 0.021, 0.932}	// Bragg
	};
	return 1.0 / (A[num][0]*Kb + A[num][1]*Kc + A[num][2]*Kh + A[num][3]);
}


/****************************************************************
 *	Fit function as assumed for TMinuit			*
 *	x[0] - Kscale						*
 *	x[1] - Kbirks						*
 *	x[2] - Kcher						*
 ****************************************************************/
void FitFunction(int &Npar, double *gin, double &f, double *x, int iflag)
{
	static int Cnt;
	double chi2BIA, chi2BAY, chi2other, r;
	int i;

	if (iflag == 1) {
		Cnt = 0;
		return;
	}
	double Kscale = x[0];
	double Kbirks = x[1];
	double Kcher = x[2];
	Cnt++;
	chi2BIA = chi2BAY = chi2other = 0;
	if (FitPar.rangeIA.From < FitPar.rangeIA.Till) {
		TH1D *hExpIA = GetExperiment(Kscale, &FitPar.ExpIA, "__hExpIA", 1.0/16);
		TH1D *hMCIA = GetMC(Kbirks, Kcher,
			FitPar.MCIA.CentralMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCIA.BirksMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCIA.CherMatrix[FitPar.iClusterEnergySelection],
			FitPar.Spectrum, "__hMCIA");
		hMCIA->Scale(hExpIA->Integral(FitPar.rangeIA.From, FitPar.rangeIA.Till) / 
			hMCIA->Integral(FitPar.rangeIA.From, FitPar.rangeIA.Till));
		chi2BIA = chi2Diff(hExpIA, hMCIA, FitPar.rangeIA.From, FitPar.rangeIA.Till);
		delete hExpIA;
		delete hMCIA;
	}
	if (FitPar.rangeAY.From < FitPar.rangeAY.Till) {
		TH1D *hExpAY = GetExperiment(Kscale, &FitPar.ExpAY, "__hExpAY", 1.0/2);
		TH1D *hMCAY = GetMC(Kbirks, Kcher,
			FitPar.MCAY.CentralMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCAY.BirksMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCAY.CherMatrix[FitPar.iClusterEnergySelection],
			FitPar.Spectrum, "__hMCAY");
		hMCAY->Scale(hExpAY->Integral(FitPar.rangeAY.From, FitPar.rangeAY.Till) / 
			hMCAY->Integral(FitPar.rangeAY.From, FitPar.rangeAY.Till));
		chi2BAY = chi2Diff(hExpAY, hMCAY, FitPar.rangeAY.From, FitPar.rangeAY.Till);
		delete hExpAY;
		delete hMCAY;
	}
	for (i=0; i<6; i++) if (FitPar.Sigma[i] > 0) {
		r = RFunction(Kbirks, Kcher, Kbirks, i) - Kscale;
		chi2other += r * r / (FitPar.Sigma[i] * FitPar.Sigma[i]);
	}
	f = chi2BIA + chi2BAY + chi2other;
	printf("Call No %d with N=%d F=%d Ks=%f DKb=%f DKc=%f ==> %f + %f + %f = %f\n", 
		Cnt, Npar, iflag, Kscale, Kbirks, Kcher, chi2BIA, chi2BAY, chi2other, f);
}

/****************************************************************
 *	Draw fit result						*
 *	Kscale - the experiment scale				*
 *	Kbirks - Birks coef					*
 *	Kcher - Cherenkov coef					*
 *	which - 0 - IA, 1 - AY					*
 ****************************************************************/
void DrawFitRes(double Kscale, double Kbirks, double Kcher, int which)
{
	char str[128];
	double chi2;
	TH1D *hExp;
	TH1D *hMC;
	int binMin, binMax;

	if (which) {
		if (FitPar.rangeAY.From >= FitPar.rangeAY.Till) return;
		hExp = GetExperiment(Kscale, &FitPar.ExpAY, "__hExpAY", 1.0/2);
		hExp->SetTitle("^{12}B decays from #mu-capture;MeV;events");
		hMC = GetMC(Kbirks, Kcher,
			FitPar.MCAY.CentralMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCAY.BirksMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCAY.CherMatrix[FitPar.iClusterEnergySelection],
			FitPar.Spectrum, "__hMCAY");
		hMC->Scale(hExp->Integral(FitPar.rangeAY.From, FitPar.rangeAY.Till) / 
			hMC->Integral(FitPar.rangeAY.From, FitPar.rangeAY.Till));
		binMin = FitPar.rangeAY.From;
		binMax = FitPar.rangeAY.Till;
	} else {
		if (FitPar.rangeIA.From >= FitPar.rangeIA.Till) return;
		hExp = GetExperiment(Kscale, &FitPar.ExpIA, "__hExpIA", 1.0/16);
		hExp->SetTitle("^{12}B decays from charge exchange;MeV;events");
		hMC = GetMC(Kbirks, Kcher,
			FitPar.MCIA.CentralMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCIA.BirksMatrix[FitPar.iClusterEnergySelection],
			FitPar.MCIA.CherMatrix[FitPar.iClusterEnergySelection],
			FitPar.Spectrum, "__hMCIA");
		hMC->Scale(hExp->Integral(FitPar.rangeIA.From, FitPar.rangeIA.Till) / 
			hMC->Integral(FitPar.rangeIA.From, FitPar.rangeIA.Till));
		binMin = FitPar.rangeIA.From;
		binMax = FitPar.rangeIA.Till;
	}

	hExp->SetLineColor(kRed);
	hExp->SetLineWidth(2);
	hMC->SetLineColor(kBlue);
	hMC->SetLineWidth(2);
	hExp->DrawCopy();
	hMC->DrawCopy("same hist");
	
	TLine ln;
	ln.SetLineColor(kGreen);
	ln.SetLineWidth(2);
	ln.DrawLine((binMin-1)/4.0, 0, (binMin-1)/4.0, hExp->GetMaximum());
	ln.DrawLine(binMax/4.0, 0, binMax/4.0, hExp->GetMaximum());
	
	TLatex lt;
	chi2 = chi2Diff(hExp, hMC, binMin, binMax);
	sprintf(str, "#chi^{2} = %5.1f", chi2);
	lt.DrawLatexNDC(0.4, 0.82, str);
}

/****************************************************************
 *	Fit function as assumed for TMinuit to fit chi2 map	*
 *	x[0] - b00						*
 *	x[1] - b11						*
 *	x[2] - b22						*
 *	x[3] - b01						*
 *	x[4] - b02						*
 *	x[5] - b12						*
 *	x[6] - const.						*
 ****************************************************************/
void MapFunction(int &Npar, double *gin, double &f, double *x, int iflag)
{
	double sum, y;
	int i, j, k;
	const int Nsteps = 7;
	double a, b, c;

	if (iflag == 1) return;
	sum = 0;
	for (i=0; i<Nsteps; i++) for (j=0; j<Nsteps; j++) for (k=0; k<Nsteps; k++) {
		a = i - 0.5 * (Nsteps - 1);
		b = j - 0.5 * (Nsteps - 1);
		c = k - 0.5 * (Nsteps - 1);
		y = x[0] * a * a + x[1] * b * b + x[2] * c * c + 
			2 * x[3] * a * b + 2 * x[4] * a * c + 2 * x[5] *b * c + x[6];
		sum += (FitPar.FunMap[i][j][k] - y) * (FitPar.FunMap[i][j][k] - y);
	}
	f = sum;
}

/****************************************************************
 *	Draw correlations					*
 *	Kscale - the experiment scale				*
 *	Kbirks - Birks coef					*
 *	Kcher - Cherenkov coef					*
 ****************************************************************
 *	Calculate chi^2 in 7^3 points and fit with sqaure	*
 *	function						*
 ****************************************************************/
void DrawCorrelations(double Kscale, double Kbirks, double Kcher)
{
	const double Scale_step = 0.005;
	const double Birks_step = 0.1;
	const double Cher_step = 0.1;
	const int Nsteps = 7;
	int i, j, k;
	double x[3];
	int n;
	double B[3][3];
	double eB[3][3];
	double C, eC;
	
	for (i=0; i<Nsteps; i++) for (j=0; j<Nsteps; j++) for (k=0; k<Nsteps; k++) {
		n = 3;
		x[0] = Kscale + (i - 0.5 * (Nsteps - 1)) * Scale_step;
		x[1] = Kbirks + (j - 0.5 * (Nsteps - 1)) * Birks_step;
		x[2] = Kcher  + (k - 0.5 * (Nsteps - 1)) * Cher_step;
		FitFunction(n, NULL, FitPar.FunMap[i][j][k], x, 0);
	}
	
	TMinuit *mn = new TMinuit();
	mn->mninit(5, 6, 7);
	mn->SetFCN(MapFunction);
	mn->DefineParameter(0, "B00", 1.0, 0.05, 0., 10000.);	// B00
	mn->DefineParameter(1, "B11", 1.0, 0.05, 0., 10000.);	// B11
	mn->DefineParameter(2, "B22", 1.0, 0.05, 0., 10000.);	// B22
	mn->DefineParameter(3, "B01", 0, 0.05, -10000., 10000.);	// B01
	mn->DefineParameter(4, "B02", 0, 0.05, -10000., 10000.);	// B02
	mn->DefineParameter(5, "B12", 0, 0.05, -10000., 10000.);	// B12
	mn->DefineParameter(6, "Const.", FitPar.FunMap[(Nsteps - 1) / 2][(Nsteps - 1) / 2][(Nsteps - 1) / 2], 
		0.05, 0., 10000.);	// Const
	
	mn->Migrad();
	
	mn->GetParameter(0, B[0][0], eB[0][0]);
	mn->GetParameter(1, B[1][1], eB[1][1]);
	mn->GetParameter(2, B[2][2], eB[2][2]);
	mn->GetParameter(3, B[0][1], eB[0][1]);
	mn->GetParameter(4, B[0][2], eB[0][2]);
	mn->GetParameter(5, B[1][2], eB[1][2]);
	mn->GetParameter(6, C, eC);
	B[1][0] = B[0][1];
	B[2][0] = B[0][2];
	B[2][1] = B[1][2];
	eB[1][0] = eB[0][1];
	eB[2][0] = eB[0][2];
	eB[2][1] = eB[1][2];
	printf("Fit quad matrix:\n");
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++) printf("%10f +- %7f   ", B[i][j], eB[i][j]);
		printf("\n");
	}
	printf("Correlation matrix:\n");
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++) printf("%10f  ", B[i][j] / sqrt(B[i][i]*B[j][j]));
		printf("\n");
	}
	printf("Errors: %f   %f   %f\n", 
		Scale_step / sqrt(B[0][0]), 0.01 * Birks_step / sqrt(B[1][1]), 0.1 * Cher_step / sqrt(B[2][2])); 
	
	TH1D *hScalePar = new TH1D("hScalePar", ";;#chi^{2}", Nsteps, -0.5*Nsteps, 0.5*Nsteps);
	TH1D *hBirksPar = new TH1D("hBirksPar", ";;#chi^{2}", Nsteps, -0.5*Nsteps, 0.5*Nsteps);
	TH1D *hCherPar = new TH1D("hCherPar", ";;#chi^{2}", Nsteps, -0.5*Nsteps, 0.5*Nsteps);
	hScalePar->SetLineWidth(2);
	hScalePar->SetLineColor(kRed);
	hScalePar->SetMarkerColor(kRed);
	hScalePar->SetMarkerSize(1.3);
	hScalePar->SetMarkerStyle(kFullCircle);
	hBirksPar->SetLineWidth(2);
	hBirksPar->SetLineColor(kGreen);
	hBirksPar->SetMarkerColor(kGreen);
	hBirksPar->SetMarkerSize(1.3);
	hBirksPar->SetMarkerStyle(kFullSquare);
	hCherPar->SetLineWidth(2);
	hCherPar->SetLineColor(kBlue);
	hCherPar->SetMarkerColor(kBlue);
	hCherPar->SetMarkerSize(1.3);
	hCherPar->SetMarkerStyle(kFullDiamond);
	for (i=0; i<Nsteps; i++) {
		hScalePar->SetBinContent(i+1, FitPar.FunMap[i][(Nsteps - 1) / 2][(Nsteps - 1) / 2]);
		hBirksPar->SetBinContent(i+1, FitPar.FunMap[(Nsteps - 1) / 2][i][(Nsteps - 1) / 2]);
		hCherPar->SetBinContent(i+1, FitPar.FunMap[(Nsteps - 1) / 2][(Nsteps - 1) / 2][i]);
	}
	TF1 *fScalePar = new TF1("fScalePar", "pol2", -0.5*Nsteps, 0.5*Nsteps);
	fScalePar->SetParameters(C, 0, B[0][0]);
	fScalePar->SetLineWidth(2);
	fScalePar->SetLineColor(kRed);
	TF1 *fBirksPar = new TF1("fBirksPar", "pol2", -0.5*Nsteps, 0.5*Nsteps);
	fBirksPar->SetParameters(C, 0, B[1][1]);
	fBirksPar->SetLineWidth(2);
	fBirksPar->SetLineColor(kGreen);
	TF1 *fCherPar = new TF1("fCherPar", "pol2", -0.5*Nsteps, 0.5*Nsteps);
	fCherPar->SetParameters(C, 0, B[2][2]);
	fCherPar->SetLineWidth(2);
	fCherPar->SetLineColor(kBlue);
	
	hScalePar->SetMinimum(0);
	hScalePar->Draw("p");
	hBirksPar->Draw("p same");
	hCherPar->Draw("p same");
	fScalePar->Draw("same");
	fBirksPar->Draw("same");
	fCherPar->Draw("same");
	
	TLegend *lg = new TLegend(0.35, 0.65, 0.7, 0.85);
	lg->AddEntry(hScalePar, "Scale scan", "P");
	lg->AddEntry(hBirksPar, "Birks scan", "P");
	lg->AddEntry(hCherPar, "Cherenkov scan", "P");
	lg->Draw();
}

/****************************************************************
 *	Load short experimental trees and matrixes		*
 *	iDet - select energy measurement:			*
 *	Return 0 on success, 1 on error				*
 ****************************************************************/
int LoadFitFiles(int iDet)
{
	const char *expFileIA = "12B_events_002210_154797.root";
	const char *expFileAY = "12B_events_AY.root";
	const char *MCFileIA = "12B_Chikuma_matrixes_IA.root";
	const char *MCFileAY = "12B_Chikuma_matrixes_AY.root";
	
	const char *mcnames[3][3] = {{
			"m12B_DB_spectrum_Chikuma_PositronEnergy", 
			"m12B_Delta_Birks_el_PositronEnergy", 
			"m12B_Delta_Cher_coeff_PositronEnergy"
		}, {
			"m12B_DB_spectrum_Chikuma_PositronSiPmEnergy", 
			"m12B_Delta_Birks_el_PositronSiPmEnergy", 
			"m12B_Delta_Cher_coeff_PositronSiPmEnergy"
		}, {
			"m12B_DB_spectrum_Chikuma_PositronPmtEnergy", 
			"m12B_Delta_Birks_el_PositronPmtEnergy", 
			"m12B_Delta_Cher_coeff_PositronPmtEnergy"
		}
	};
	int i, j, N;
	TMatrixD *M[3][3];
	float PositronEnergy[3];

//	Open files and read matrixes
	TFile *fExpIA = new TFile(expFileIA);
	TFile *fExpAY = new TFile(expFileAY);
	if (!fExpIA->IsOpen() || !fExpAY->IsOpen()) return 1;
	auto t12BExpSignalIA = (TTree *) fExpIA->Get("MuonPair");
	auto t12BExpRandomIA = (TTree *) fExpIA->Get("MuonRandom");
	auto t12BExpSignalAY = (TTree *) fExpAY->Get("MuonPair");
	auto t12BExpRandomAY = (TTree *) fExpAY->Get("MuonRandom");
	if (!t12BExpSignalIA || !t12BExpRandomIA || !t12BExpSignalAY || !t12BExpRandomAY) {
		printf("Can not get all trees\n");
		return 1;
	}
	FitPar.ExpIA.NExpSignal = t12BExpSignalIA->GetEntries();
	FitPar.ExpIA.NExpRandom = t12BExpRandomIA->GetEntries();
	FitPar.ExpAY.NExpSignal = t12BExpSignalAY->GetEntries();
	FitPar.ExpAY.NExpRandom = t12BExpRandomAY->GetEntries();
	FitPar.ExpIA.ExpSignal = (double *) malloc(FitPar.ExpIA.NExpSignal * sizeof(double));
	FitPar.ExpIA.ExpRandom = (double *) malloc(FitPar.ExpIA.NExpRandom * sizeof(double));
	FitPar.ExpAY.ExpSignal = (double *) malloc(FitPar.ExpAY.NExpSignal * sizeof(double));
	FitPar.ExpAY.ExpRandom = (double *) malloc(FitPar.ExpAY.NExpRandom * sizeof(double));
	t12BExpSignalIA->SetBranchAddress("Pair", PositronEnergy);
	t12BExpRandomIA->SetBranchAddress("Pair", PositronEnergy);
	t12BExpSignalAY->SetBranchAddress("Pair", PositronEnergy);
	t12BExpRandomAY->SetBranchAddress("Pair", PositronEnergy);
	for (i=0; i<FitPar.ExpIA.NExpSignal; i++) {
		t12BExpSignalIA->GetEntry(i);
		FitPar.ExpIA.ExpSignal[i] = PositronEnergy[iDet];
	}
	for (i=0; i<FitPar.ExpIA.NExpRandom; i++) {
		t12BExpRandomIA->GetEntry(i);
		FitPar.ExpIA.ExpRandom[i] = PositronEnergy[iDet];
	}
	for (i=0; i<FitPar.ExpAY.NExpSignal; i++) {
		t12BExpSignalAY->GetEntry(i);
		FitPar.ExpAY.ExpSignal[i] = PositronEnergy[iDet];
	}
	for (i=0; i<FitPar.ExpAY.NExpRandom; i++) {
		t12BExpRandomAY->GetEntry(i);
		FitPar.ExpAY.ExpRandom[i] = PositronEnergy[iDet];
	}
	TFile *fMCIA = new TFile(MCFileIA);
	TFile *fMCAY = new TFile(MCFileAY);
	if (!fMCIA->IsOpen() || !fMCAY->IsOpen()) return 1;
	for (i=0; i<3; i++) for (j=0; j<3; j++) {
		M[i][j] = (TMatrixD *) fMCIA->Get(mcnames[i][j]);
		if (!M[i][j]) {
			printf("Can not get all matrixes in %s\n", MCFileIA);
			return 1;
		}
	}
	for (i=0; i<3; i++) {
		FitPar.MCIA.CentralMatrix[i] = (TMatrixD *) M[i][0]->Clone();
		FitPar.MCIA.BirksMatrix[i] = (TMatrixD *) M[i][1]->Clone();
		FitPar.MCIA.CherMatrix[i] = (TMatrixD *) M[i][2]->Clone();
	}
	for (i=0; i<3; i++) for (j=0; j<3; j++) {
		M[i][j] = (TMatrixD *) fMCAY->Get(mcnames[i][j]);
		if (!M[i][j]) {
			printf("Can not get all matrixes in %s\n", MCFileAY);
			return 1;
		}
	}
	for (i=0; i<3; i++) {
		FitPar.MCAY.CentralMatrix[i] = (TMatrixD *) M[i][0]->Clone();
		FitPar.MCAY.BirksMatrix[i] = (TMatrixD *) M[i][1]->Clone();
		FitPar.MCAY.CherMatrix[i] = (TMatrixD *) M[i][2]->Clone();
	}
	FitPar.Spectrum = (TVectorD *) ((TVectorD *) fMCIA->Get("v12B_ParticleEnergy"))->Clone();
	if (!FitPar.Spectrum) {
		printf("Can not get true specturm in %s\n", MCFileIA);
		return 1;
	}
	FitPar.iClusterEnergySelection = iDet;
	printf("IA: Found %d events = %d signal - %d/16 random\n", 
		(int)(FitPar.ExpIA.NExpSignal - FitPar.ExpIA.NExpRandom / 16.0), FitPar.ExpIA.NExpSignal, FitPar.ExpIA.NExpRandom);
	printf("AY: Found %d events = %d signal - %d/2 random\n", 
		(int)(FitPar.ExpAY.NExpSignal - FitPar.ExpAY.NExpRandom / 2.0), FitPar.ExpAY.NExpSignal, FitPar.ExpAY.NExpRandom);

	fExpIA->Close();
	fMCIA->Close();
	fExpAY->Close();
	fMCAY->Close();

//	Set default parameters: 
//		sigmas for mu, 22Na, 60co, 248Cm, mudec, Bragg
	FitPar.Sigma[0] = 0.005;	// vertical muons
	FitPar.Sigma[1] = 0.005;	// 22Na decay
	FitPar.Sigma[2] = 0.005;	// 60Co decay
	FitPar.Sigma[3] = 0.005;	// 248Cm neutrons and IBD neutrons
	FitPar.Sigma[4] = 0.005;	// Electrons from mu-decay
	FitPar.Sigma[5] = 0;		// Bragg peak
//		Fit ranges
	FitPar.rangeIA.From = 13;	// 3 MeV
	FitPar.rangeIA.Till = 50;	// 12.5 MeV
	FitPar.rangeAY.From = 3;	// 0.5 MeV
	FitPar.rangeAY.Till = 50;	// 12.5 MeV
	
	return 0;
}

/****************************************************************
 *	Try to fit MC Kbirks and Kcher to experiment		*
 *	iDet - select energy measurement:			*
 *		0: SiPM+PMT					*
 *		1: SiPM only					*
 *		2: PMT only					*
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
 *	Both 12B reactions are used and other sources added	*
 ****************************************************************/
void FitMCparameters(int iDet, const char *oname)
{
	const double Rmin = 1.00;
	const double Rmax = 1.10;
	const double Rstep = 0.005;
	int i, N;
	double R, eR, Kb, eKb, kkB, Kc, eKc, kkC;
	double x[3];
	double chi2;
	int np;
	double A, B;
	double Kscale_min;
	char str[1024];
	double Rres[6];
	const char *rnames[6] = {"Rmu", "RNa", "RCo", "RCm", "Rmu-dec", "RBragg"};

//	Prepare minimization:
	if (FitPar.iClusterEnergySelection != iDet && LoadFitFiles(iDet)) return;
	TMinuit *mn = new TMinuit();
	mn->mninit(5, 6, 7);
	mn->SetFCN(FitFunction);
	mn->DefineParameter(0, "R", 1.0, 0.05, 0.7, 1.5);	// Kscale
	mn->DefineParameter(1, "Kb", 0, 0.005, -5, 5);		// Kbirks (Delta)
	mn->DefineParameter(2, "Kc", 0, 0.05, -5, 5);		// Kcher (Delta)
	FitFunction(i, NULL, R, NULL, 1);			// Reset counter
	N = (Rmax-Rmin)/Rstep + 1;
	gROOT->cd();
	TH1D *hPar = new TH1D("hPar", "Scale scan;Scale;#chi^{2}", N, Rmin - Rstep/2, Rmax + Rstep/2);
//********************************
	printf("Ready for fit !\n");
	for (i=0; i<N; i++) {
		mn->DefineParameter(0, "R", Rmin + i*Rstep, 0.05, 0.7, 1.5);	// Kscale
		mn->FixParameter(0);
		mn->Migrad();
		mn->GetParameter(1, Kb, eKb);
		mn->GetParameter(2, Kc, eKc);
		x[0] = Rmin + i*Rstep;
		x[1] = Kb;
		x[2] = Kc;
		np = 3;
		FitFunction(np, NULL, chi2, x, 0);		// get function in the minimum found
		hPar->SetBinContent(i+1, chi2);
	}
	gStyle->SetOptStat(0);
	auto cv = (TCanvas *)gROOT->FindObject("CV");
	if (!cv) cv = new TCanvas("CV", "CV", 1200, 1200);
	cv->Clear();
	cv->Divide(2, 2);
	cv->cd(1);
	hPar->Fit("pol2", "W");
	cv->Update();
	A = hPar->GetFunction("pol2")->GetParameter(2);
	B = hPar->GetFunction("pol2")->GetParameter(1);
	R = -0.5 * B / A;
	if (R < Rmin || R > Rmax) {
		printf("R = %f is beyond the range!\n", R);
		return;
	}
	eR = 1 / sqrt(hPar->GetFunction("pol2")->GetParameter(0));
//********************************
	mn->DefineParameter(0, "R", R, 0.05, 0.7, 1.5);	// Kscale
	mn->FixParameter(0);
	mn->Migrad();
	mn->GetParameter(1, Kb, eKb);
	mn->GetParameter(2, Kc, eKc);
	
	cv->cd(3);
	DrawFitRes(R, Kb, Kc, 0);
	cv->cd(4);
	DrawFitRes(R, Kb, Kc, 1);
	cv->cd(2);
	DrawCorrelations(R, Kb, Kc);
	kkB = 0.0208 + 0.01*Kb;
	kkC = 0.133 + 0.1*Kc;

	sprintf(str, "R = %f +- %f;   Kb = %f +- %f;   Kc = %f +- %f", R, eR, kkB, eKb / 100, kkC, eKc / 10);
	printf("%s\n", str);
	TLatex lt;
	lt.SetTextSize(0.022);
	lt.DrawLatexNDC(0.05, 0.97, str);
	x[0] = R;
	x[1] = Kb;
	x[2] = Kc;
	np = 3;
	FitFunction(np, NULL, chi2, x, 0);		// get function in the minimum found
	np = -3;	// Kscale, Kbirks, Kcher
	if (FitPar.rangeIA.From < FitPar.rangeIA.Till) np += FitPar.rangeIA.Till - FitPar.rangeIA.From;
	if (FitPar.rangeAY.From < FitPar.rangeAY.Till) np += FitPar.rangeAY.Till - FitPar.rangeAY.From;
	for (i=0; i<6; i++) if (FitPar.Sigma[i] > 0) np++;
	sprintf(str, "#chi^{2} = %f / %d ndf", chi2, np);
	lt.DrawLatexNDC(0.05, 0.94, str);
	
	for (i=0; i<6; i++) printf("%s = %6.3f  ", rnames[i], RFunction(Kb, Kc, Kb, i));
	printf("\n");
	
	cv->Update();
	cv->SaveAs(oname);
}
