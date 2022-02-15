/***
 *
 * Version:       2.0
 *
 * Package:       DANSS time calibration pass 3
 *
 * Description:   draw calibration as a function of run number.
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

void Help(void)
{
	printf("\tDANSS time calibration pass 3. Version %s\n", MYVERSION);
	printf("Draw calibration as a function of run number.\n");
	printf("\tUsage:\n");
	printf("\t./draw_tcalib run_first run_last run_step run_dir\n");
}

void ReadCalib(const char *dirname, int run, int step, float *calib, int len)
{
	FILE *f;
	char name[1024];
	int irc;
	
	memset(calib, 0, len);
	sprintf(name, "%s/hist/calib_%6.6d_%6.6d.bin", dirname, run, run+step-1);
	irc = access(name, R_OK);
	if (irc) return;
	f = fopen(name, "rb");
	if (!f) {
		printf("Can not open calib file %s: %m.\n", name);
		return;
	}
	fread(calib, 1, len, f);
	fclose(f);
}

int main(int argc, const char **argv)
{
	const int color[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 28, 30, 34, 38, 40, 41, 46};
	int i, j, k, n, chan, irc, ibin;
	char strs[128], strl[1024];
	int run, run_first, run_last, run_step;
	const char *run_dir;
	TH1D *hTrace[MAXCHAN];
	float Calib[MAXCHAN];
	double rmin, rmax;
	
	if (argc < 5) {
		Help();
		return 100;
	}
	run_first = strtol(argv[1], NULL, 10);
	run_last = strtol(argv[2], NULL, 10);
	run_step = strtol(argv[3], NULL, 10);
	run_dir = argv[4];
	
	memset(hTrace, 0, sizeof(hTrace));
	
	for (run=run_first; run<run_last; run+=run_step) {
		ReadCalib(run_dir, run, run_step, Calib, sizeof(Calib));
		for (i=0; i<MAXCHAN; i++) {
			if (!Calib[i]) continue;
			if (!hTrace[i]) {
				sprintf(strs, "hTr%2.2dc%2.2d", i / 100, i % 100);
				sprintf(strl, "Time calibration chan %2d.%2.2d;run;ns", i / 100, i % 100);
				hTrace[i] = new TH1D(strs, strl, (run_last - run_first) / run_step + 1, run_first, run_last);
			}
			ibin = hTrace[i]->FindBin(run);
			hTrace[i]->SetBinContent(ibin, Calib[i]);
			hTrace[i]->SetBinError(ibin, 0.001);	// no errors
		}
	}
	
	sprintf(strl, "%s/hist/trace_%6.6d_%6.6d.pdf", run_dir, run_first, run_last);
	gStyle->SetOptStat(0);
	TCanvas *cv = new TCanvas("CV", "CV", 1600, 1200);
	TString pdf(strl);
	cv->SaveAs((pdf + "[").Data());
	for (i=1; i<MAXCHAN/100; i++) for (j=0; j<4; j++) {	// cycle over modules and pages
		cv->Clear();
		n = 0;
		for (k=0; k<16; k++) {				// cycle over 16 channels
			chan = 100*i + 16*j + k;
			if (!hTrace[chan]) continue;
			hTrace[chan]->SetMarkerColor(color[k]);
			hTrace[chan]->SetMarkerStyle(kFullDotMedium);
			if (!n) {
				rmin = hTrace[chan]->GetMinimum();
				if (!rmin) rmin = hTrace[chan]->GetMinimum(0.001);
				if (rmin > 1000) rmin = 0;
				rmax = hTrace[chan]->GetMaximum();
				if (!rmax) rmax = hTrace[chan]->GetMaximum(-0.001);
				if (rmax < -1000) rmax = 0;
				hTrace[chan]->SetMinimum(rmin - 10);
				hTrace[chan]->SetMaximum(rmax + 10);
				hTrace[chan]->Draw("P");
			} else {
				hTrace[chan]->Draw("same,P");
			}
			n++;
		}
		if (n) cv->SaveAs(pdf);
		
	}

	cv->SaveAs((pdf + "]").Data());

	sprintf(strl, "%s/hist/trace_%6.6d_%6.6d.root", run_dir, run_first, run_last);
	TFile *fOut = new TFile(strl, "RECREATE");
	fOut->cd();
	for (i=0; i<MAXCHAN; i++) if (hTrace[i]) hTrace[i]->Write();
	fOut->Close();

	return 0;
}

