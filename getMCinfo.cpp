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
	long long j0;
	
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
	OutTree->Branch("FromMC", &FromMC, "MCPositronEnergy/F:MCPositronX[3]:MCNeutronEnergy");

	for (i=0, j=0; i<N; i++) {
		PairTree->GetEntry(i);
		j0 = j;
		for (;j<K-1; j++) {
			MCTree->GetEntry(j);
			if (MCEvent.EventID == MCParticle.EventID && ((int)MCParticle.ID) == 1) break;	// search for positron
		}
		if (j == K-1) {
			printf("Very strange EventID = %f positron not found in MC @ %Ld\n", MCEvent.EventID, j0);
			// we need to do something nevertheless !
			FromMC.MCPositronEnergy = 0;
			FromMC.MCPositronX[0] = 0;
			FromMC.MCPositronX[1] = 0;
			FromMC.MCPositronX[2] = 0;
			FromMC.MCNeutronEnergy = 0;
			OutTree->Fill();
			j = j0;
			continue;
		}
		FromMC.MCPositronEnergy = MCParticle.ParticleEnergy;
		FromMC.MCPositronX[0] = 48.0 - MCParticle.X / 10000.0;	// um to cm
		FromMC.MCPositronX[1] = 48.0 - MCParticle.Y / 10000.0;	// um to cm
		FromMC.MCPositronX[2] = 49.5 + MCParticle.Z / 10000.0;	// um to cm
		j++;
		MCTree->GetEntry(j);
		if (MCEvent.EventID != MCParticle.EventID || MCParticle.ID != 2) {	// neutron should be here !
			printf("Very strange EventID = %f neutron not found in MC @ %Ld\n", MCEvent.EventID, j0);
			FromMC.MCPositronEnergy = 0;
			FromMC.MCPositronX[0] = 0;
			FromMC.MCPositronX[1] = 0;
			FromMC.MCPositronX[2] = 0;
			FromMC.MCNeutronEnergy = 0;
			OutTree->Fill();
			j = j0;
			continue;
		}
		FromMC.MCNeutronEnergy = MCParticle.ParticleEnergy;
		OutTree->Fill();
	}

	OutTree->Write();
	PairFile->Close();
	MCFile->Close();
	OutFile->Close();
	return 0;
}
