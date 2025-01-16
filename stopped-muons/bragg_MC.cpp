//	Create MC Bragg peak histogram
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

int main(int argc, char ** argv)
{
	struct StoppedMuonStruct Muon;
	float MCDepth;
	int i, j, N, K;
	double scale;
	double L, dedx;
	char str[1024];
	
	if (argc < 2) {
		printf("Usage: %s prc_dir\n", argv[0]);
		return 10;
	}
	
	TChain *chIn = new TChain("StoppedMuons", "StoppedMuons");
	TChain *chDepth = new TChain("Depth", "Depth");
	for (i=0; i<5; i++) for (j=1; j<=16; j++) {
		sprintf(str, "%s/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d.root", argv[1], i, j);
		chIn->AddFile(str);
		sprintf(str, "%s/mc_Muons_indLY_transcode_rawProc_pedSim_%2.2d_%2.2d-depth.root", argv[1], i, j);
		chDepth->AddFile(str);
	}
	TString fname(argv[1]);
	TFile *fOut = new TFile((fname + "-hist.root").Data(), "RECREATE");
	if (!fOut->IsOpen()) return 20;
	
	chIn->SetBranchAddress("Stopped", &Muon);
	chDepth->SetBranchAddress("Depth", &MCDepth);
	
	N = chIn->GetEntries();
	K = chDepth->GetEntries();
	if (N != K) {
		printf("Chain lengths mismatch %d != %d\n", N, K);
		return 40;
	}
	chIn->AddFriend(chDepth);

	TH2D *hEL = new TH2D("hEL", "dE/dx versus L;cm;MeV/cm", 600, 0, 60, 500, 0, 25);
	
	for (i=0; i<N; i++) {
		if (MCDepth <= 0 && MCDepth > 1.1) continue;
		chIn->GetEntry(i);
		scale = sqrt(1 + tan(Muon.thetaX) * tan(Muon.thetaX) + tan(Muon.thetaY) * tan(Muon.thetaY));
		dedx = Muon.Ehit[0] / (MCDepth * scale);
		L = MCDepth * scale / 2;
		hEL->Fill(L, dedx);
		for (j = 1; j < Muon.NHits; j++) if (Muon.Ehit[j] > 0) {
			dedx = Muon.Ehit[j] / scale;
			L = (j + MCDepth - 0.5) * scale;
			hEL->Fill(L, dedx);
		}
	}
	
	auto hELProfX = hEL->ProfileX("hEL_profX");
	
	fOut->cd();
	hEL->Write();
	hELProfX->Write();
	fOut->Close();
	return 0;
}
