//	Make MC truth depth in the stopping strip information tree
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TFile.h>
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
struct ParticleDataStruct {
	double EventID;
	double ID;
	double ParentID;
	double ParticleEnergy;
	double X, Y, Z;
	double DirX, DirY, DirZ;
	double Time;
	double KillingFlag;
};
#pragma pack(pop)

int main(int argc, char ** argv)
{
	struct StoppedMuonStruct Muon;
	int MCEventID;
	struct ParticleDataStruct Particle;
	char MCPartName[256];
	float MCDepth;
	int i, j, k, N, K;
	
	if (argc < 3) {
		printf("Usage: %s prc_file mc_file\n", argv[0]);
		return 10;
	}
	
	TFile *fIn = new TFile(argv[1]);
	TString fname(argv[1]);
	fname.ReplaceAll(".root", "");
	TFile *fMC = new TFile(argv[2]);
	TFile *fOut = new TFile((fname + "-depth.root").Data(), "RECREATE");
	if (!fIn->IsOpen() || !fOut->IsOpen() || !fMC->IsOpen()) return 20;
	
	TTree *tIn = (TTree *) fIn->Get("StoppedMuons");
	TTree *tMC = (TTree *) fMC->Get("DANSSParticle");
	if (!tIn || !tMC) {
		printf("Can not find all trees\n");
		return 30;
	}
	tIn->SetBranchAddress("Stopped", &Muon);
	tIn->SetBranchAddress("MCEventID", &MCEventID);
	tMC->SetBranchAddress("ParticleData", &Particle);
	tMC->SetBranchAddress("ParticleName", MCPartName);
	TTree *tOut = new TTree("Depth", "Depth");
	tOut->Branch("Depth", &MCDepth, "Depth/F");
	
	N = tIn->GetEntries();
	K = tMC->GetEntries();
	k = 0;
	for (i=0; i<N; i++) {
		tIn->GetEntry(i);
		for(;k < K; k++) {
			tMC->GetEntry(k);
			if (((int) Particle.EventID) == MCEventID && ((int) Particle.ParentID) == 1 && 
				(!strcmp(MCPartName, "nu_mu") || !strcmp(MCPartName, "anti_nu_mu"))) break;
			if (((int) Particle.EventID) > MCEventID) break;
		}
		if (((int) Particle.EventID) != MCEventID) {
			printf("********** Event ID = %d not found (Pass through ?) !\n", MCEventID);
			MCDepth = 100.;
		} else {
			MCDepth = 1.04 * Muon.Z + 1.0 - (Particle.Z / 10000 + 52);
		}
//		printf("ID=%d   Strip = %d.%d   X,Y,Z = %f, %f, %f cm  delta = %f cm\n", 
//			MCEventID, Muon.Z, Muon.XY, Particle.X / 10000, Particle.Y / 10000, Particle.Z / 10000,
//			MCDepth);
		tOut->Fill();
	}
	
	fOut->cd();
	tOut->Write();
	fOut->Close();
	fIn->Close();
	fMC->Close();
	return 0;
}
