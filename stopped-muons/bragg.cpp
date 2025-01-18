//	Create Bragg peak histogram. No depth information available
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TChain.h>
#include <TFile.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TString.h>
#include <TTree.h>

#define MAXZ	100

#pragma pack(push,1)
struct StoppedMuonStruct {
        int index;              // Index in the DanssEvent tree
        long long globalTime;   // global time 125 MHz ticks
        int Z;                  // z of the stopping strip
        int XY;                 // xy of the stopping strip
        float thetaX;           // angle of the track in ZX, 0 - vertical
        float thetaY;           // angle of the track in ZY, 0 - vertical
        int NHits;              // number of hits including empty hits
        float Ehit[MAXZ];       // hit energy
};
#pragma pack(pop)

void ProjectEL(TH2D* h, const char *fname)
{
	double scale;
	double L, dedx;
	struct StoppedMuonStruct Muon;
	int i, j, N;
	
	i = access(fname, R_OK);
	if (i) return;
	
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	
	TTree *t = (TTree *) f->Get("StoppedMuons");
	if (!t) return;
	
	t->SetBranchAddress("Stopped", &Muon);
	
	N = t->GetEntries();
	
	for (i=0; i<N; i++) {
		t->GetEntry(i);
		scale = sqrt(1 + tan(Muon.thetaX) * tan(Muon.thetaX) + tan(Muon.thetaY) * tan(Muon.thetaY));
		dedx = 2 * Muon.Ehit[0] / scale;
		L = scale / 4;
		h->Fill(L, dedx);
		for (j = 1; j < Muon.NHits; j++) if (Muon.Ehit[j] > 0) {
			dedx = Muon.Ehit[j] / scale;
			L = (j + 0.25) * scale;
			h->Fill(L, dedx);
		}
	}
	f->Close();
	delete f;
}

int main(int argc, char ** argv)
{
	int i, j;
	char str[1024];
	const char *dirname = NULL;
	int run_begin, run_end;
	const char *expdir = "/home/clusters/rrcmpi/alekseev/igor/stopped8n2";
	char fname[1024];
	
	if (argc < 2) {
		printf("Usage: %s prc_dir | run_begin run_end [expdir]\n", argv[0]);
		return 10;
	}
	
	if (isdigit(argv[1][0])) {
		if (argc < 3) {
			printf("Usage: %s prc_dir | run_begin run_end [expdir]\n", argv[0]);
			return 20;
		}
		run_begin = strtol(argv[1], NULL, 10);
		run_end = strtol(argv[2], NULL, 10);
		if (argc > 3) expdir = argv[3];
		sprintf(fname, "bragg_%6.6d_%6.6d.root", run_begin, run_end);
	} else {
		dirname = argv[1];
		sprintf(fname, "%s-bragg.root", dirname);
	}
	TFile *fOut = new TFile(fname, "RECREATE");
	if (!fOut->IsOpen()) return 30;
	TH2D *hEL = new TH2D("hEL", "dE/dx versus L;cm;MeV/cm", 600, 0, 60, 500, 0, 25);
	if (dirname) {
		for (i=0; i<5; i++) for (j=1; j<=16; j++) {
			sprintf(fname, "%s/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d.root", dirname, i, j);
			ProjectEL(hEL, fname);
		}
	} else {
		for (i=run_begin; i<=run_end; i++) {
			sprintf(fname, "%s/%3.3dxxx/stopped_%6.6d.root", expdir, i/1000, i);
			ProjectEL(hEL, fname);
		}
	}
	
	auto hELProfX = hEL->ProfileX("hEL_profX");
	
	fOut->cd();
	hEL->Write();
	hELProfX->Write();
	fOut->Close();
	return 0;
}
