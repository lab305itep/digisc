/****************************************************************************************
 *	DANSS data analysis - extract positron info from IBD simulation			*
 ****************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TMath.h"
#include "TFile.h"
#include "TChain.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TStyle.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"
#include "TRandom.h"
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

#include "evtbuilder.h"

int main(int argc, char **argv)
{
	struct MCEventStruct	MCEvent;
	struct MCParticleStruct	MCParticle;
	struct DanssFromMC	FromMC;
	TFile *PairFile;
	TFile *MCFile;
	TFile *OutFile;
	TTree *PairTree;
	TTree *MCTree;
	TTree *OutTree;
	long long i, j, N, K;
	
	if (argc < 4) {
		printf("Usage: %s pair_file.root MC_file.root output_file.root\n", argv[0]);
		return 10;
	}

	PairFile = new TFile(argv[1]);
	MCFile   = new TFile(argv[2]);
	OutFile  = new TFile(argv[3], "RECREATE");
	if (!PairFile->IsOpen() || !MCFile->IsOpen() || !OutFile->IsOpen()) return 20; 
	
	PairTree = (TTree *) PairFile->Get("DanssPair");
	MCTree = (TTree *) MCFile->Get("DANSSParticle");
	if (!PairTree || !MCTree) return 30;
	
	PairTree->SetBranchAddress("MCEvent", &MCEvent);
	MCTree->SetBranchAddress("ParticleData", &MCParticle);

	N = PairTree->GetEntries();
	K = MCTree->GetEntries();
	if (!N || !K) return 40;

	OutFile->cd();
	OutTree = new TTree("FromMC", "FromMC");
	OutTree->Branch("FromMC", &FromMC, "MCPositronEnergy/F:MCPositronX[3]");

	for (i=0, j=0; i<N; i++) {
		PairTree->GetEntry(i);
		for (;j<K; j++) {
			MCTree->GetEntry(j);
			if (MCEvent.EventID == MCParticle.EventID && MCParticle.ID == 1) break;	// search for positron
		}
		if (j == K) {
			printf("Very strange EventID = %f not found in MC\n", MCEvent.EventID);
			break;
		}
		FromMC.MCPositronEnergy = MCParticle.ParticleEnergy;
		FromMC.MCPositronX[0] = 48.0 - MCParticle.X / 10000.0;	// um to cm
		FromMC.MCPositronX[1] = 48.0 - MCParticle.Y / 10000.0;	// um to cm
		FromMC.MCPositronX[2] = 49.5 + MCParticle.Z / 10000.0;	// um to cm
		OutTree->Fill();
	}

	OutTree->Write();
	PairFile->Close();
	MCFile->Close();
	OutFile->Close();
	return 0;
}
