/***
 *
 * Version:       2.0
 *
 * Package:       DANSS time calibration pass 2
 *
 * Description:   Gets a tree with muon events and produce histogramms and time calibration
 *                We select muons which have single hits only
 *                in each of top and bottom layers and 80-150 hits total
 *
 ***/
#include <stdio.h>
#include <string.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TMath.h"
#include "TFile.h"
#include "TChain.h"
#include "TNetFile.h"
#include "TRandom2.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TStyle.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"
#include "TDirectory.h"
#include "TProcessID.h"
#include "TObject.h"
#include "TClonesArray.h"
#include "TRefArray.h"
#include "TRef.h"
#include "TKey.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"

/***********************	Definitions	****************************/
#define MYVERSION	"2.00"
#define MAXCHAN		6000

using namespace std;

// Globals:
TChain	*InputTree;
TH1D	*hDT[MAXCHAN];
float	tShift[MAXCHAN];
TH1D	*hDelta;

struct MuonEventStruct {
	long long number;
	int unixTime;
	int topX;
	int topY;
	int bottomX;
	int bottomY;
	int NHits;
} MuonEvent;

#define iMaxDataElements 200 // actually we limit number of hits at 150 at the event selection stage

struct HitStruct {
	int	type[iMaxDataElements];	// 0 - SiPM, 1 - PMT, 2 - Veto
	int	chan[iMaxDataElements];	// ADC*100 + chan
	int	z[iMaxDataElements];	// odd - X, even - Y
	int	xy[iMaxDataElements];	// X or Y
	float	t[iMaxDataElements];
}	HitArray;

void DrawHists(const char *name)
{
	const double width = 15;	// ns
	int i, j, k, n;
	double rmin, rmax;
	double umin, umax;
	double mean;
	int chan;
	
	gStyle->SetOptStat(1110);
	TCanvas *cv = new TCanvas("CV", "CV", 1600, 1200);
	TString str(name);
	cv->SaveAs((str + "[").Data());
	for (i=1; i<MAXCHAN/100; i++) for (j=0; j<4; j++) {	// cycle over modules and pages
		cv->Clear();
		cv->Divide(4, 4);
		n = 0;
		for (k=0; k<16; k++) {				// cycle over 16 channels
			chan = 100*i + 16*j + k;
			if (!hDT[chan]) continue;
			rmin = hDT[chan]->GetXaxis()->GetBinLowEdge(1);
			rmax = hDT[chan]->GetXaxis()->GetBinUpEdge(hDT[chan]->GetXaxis()->GetNbins());
			mean = hDT[chan]->GetMean();
			umin = mean - width;
			umax = mean + width;
			if (umin < rmin) umin = rmin;
			if (umax > rmax) umax = rmax;
			hDT[chan]->GetXaxis()->SetRangeUser(umin, umax);
			cv->cd(k+1);
			hDT[chan]->Draw();
			tShift[chan] += mean;
			hDelta->Fill(mean);
			n++;
		}
		if (n) cv->SaveAs(str);
	}
	cv->Clear();
	hDelta->Draw();
	cv->SaveAs(str);
	cv->SaveAs((str + "]").Data());
	delete cv;
}

void FillHists(void)
{
	const double z0 = 50;	// cm
	const double c = 30;	// cm/ns
	int i, j, N, K;
	double avrTime;
	double r;
	double z;
	double tcorr;
	int chan;
	char strs[128], strl[1024];
	
	N = InputTree->GetEntries();
	
	for (i=0; i<N; i++) {
		InputTree->GetEntry(i);
		// 1/cos(theta)
		r = sqrt((MuonEvent.topX - MuonEvent.bottomX) * (MuonEvent.topX - MuonEvent.bottomX)  +
			(MuonEvent.topY - MuonEvent.bottomY) * (MuonEvent.topY - MuonEvent.bottomY) + 4.0*4.0) / 4.0;
		K = 0;
		avrTime = 0;
		for (j=0; j<MuonEvent.NHits; j++) {
			if (HitArray.type[j] == 2) continue;	// ignore veto here
			chan = HitArray.chan[j];
			z = HitArray.z[j] + 0.5;
			if (HitArray.type[j] == 1) z *= 20;	// PMT section size
			tcorr = r * (z - z0) / c;		// time correction
			avrTime += HitArray.t[j] - tShift[chan] - tcorr;	// most of muons come from the upper side
			K++;
		}
		avrTime /= K;	// K can not be zero from the seection criteria
		
		for (j=0; j<MuonEvent.NHits; j++) {
			chan = HitArray.chan[j];
			if (!hDT[chan]) {
				sprintf(strs, "hDT%2.2dc%2.2d", chan / 100, chan % 100);
				sprintf(strl, "Time from mean time for chan %2d.%2.2d;ns", chan / 100, chan % 100);
				hDT[chan] = new TH1D(strs, strl, 400, -150, 50);
			}
			z = HitArray.z[j] + 0.5;
			if (HitArray.type[j] == 1) z *= 20;	// PMT section size
			tcorr = r * (z - z0) / c;		// time correction
			if (HitArray.type[j] == 2) tcorr = 3.3;	// VETO is about 1 m from the center of detector
			hDT[chan]->Fill(HitArray.t[j] - tShift[chan] - tcorr - avrTime);
		}
	}
}

void Help(void)
{
	printf("\tDANSS time calibration pass 2. Version %s\n", MYVERSION);
	printf("Process muon events and create histogramms.\n");
	printf("\tUsage:\n");
	printf("\t./ana_tcalib run_first run_last run_dir [tcalib.bin]\n");
}

void ReadCalib(const char *name)
{
	FILE *f;
	
	f = fopen(name, "rb");
	if (!f) {
		printf("Can not open calib file %s: %m.\n", name);
		return;
	}
	fread(tShift, 1, sizeof(tShift), f);
	fclose(f);
}

void WriteCalib(const char *name)
{
	FILE *f;
	
	f = fopen(name, "wb");
	if (!f) {
		printf("Can not open file for write %s: %m.\n", name);
		return;
	}
	fwrite(tShift, 1, sizeof(tShift), f);
	fclose(f);
}

int main(int argc, const char **argv)
{
	int i, irc;
	char str[1024];
	int run_first, run_last;
	const char *run_dir;
	
	if (argc < 4) {
		Help();
		return 100;
	}
	
	run_first = strtol(argv[1], NULL, 10);
	run_last = strtol(argv[2], NULL, 10);
	run_dir = argv[3];
	
	InputTree = new TChain("MuonEvent", "MuonEvent");
	for (i = run_first; i <= run_last; i++) {
		sprintf(str, "%s/%3.3dxxx/danss_%6.6d.root", run_dir, i/1000, i);
		irc = access(str, R_OK);
		if (!irc) InputTree->AddFile(str);
	}
	InputTree->SetBranchAddress("Data", &MuonEvent);
	InputTree->SetBranchAddress("Type", HitArray.type);
	InputTree->SetBranchAddress("Chan", HitArray.chan);
	InputTree->SetBranchAddress("Z", HitArray.z);
	InputTree->SetBranchAddress("XY", HitArray.xy);
	InputTree->SetBranchAddress("T", HitArray.t);
	hDelta = new TH1D("hDelta", "Run time shift corrections;ns", 100, -5, 5);
	
	memset(hDT, 0, sizeof(hDT));
	memset(tShift, 0, sizeof(tShift));
	if (argc > 4) ReadCalib(argv[4]);

	FillHists();
	sprintf(str, "%s/hist/hist_%6.6d_%6.6d.pdf", run_dir, run_first, run_last);
	DrawHists(str);
	sprintf(str, "%s/hist/calib_%6.6d_%6.6d.bin", run_dir, run_first, run_last);
	WriteCalib(str);

	sprintf(str, "%s/hist/hist_%6.6d_%6.6d.root", run_dir, run_first, run_last);
	TFile *fOut = new TFile(str, "RECREATE");
	fOut->cd();
	for (i=0; i<MAXCHAN; i++) if (hDT[i]) hDT[i]->Write();
	hDelta->Write();
	fOut->Close();

	return 0;
}

