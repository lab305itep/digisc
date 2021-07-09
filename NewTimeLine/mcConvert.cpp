#include <stdio.h>
#include <TFile.h>
#include <TTree.h>

#define SLICE	1000
#define TIMELINESHIFT	1000.0
#define MAXNAME	1000

/****************************************************************
 *			Structures				*
 ****************************************************************/
#pragma pack(push, 1)
//		DANSSParticle
	struct ParticleDataStruct {
		double EventID;
		double ID;
		double ParentID;
		double ParticleEnergy;
		double X, Y, Z, DirX, DirY, DirZ;
		double Time;
		double KillingFlag;
	} ParticleData;
	char ParticleName[MAXNAME];
	char ParticleMaterialName[MAXNAME];
	char ParticleCreatorProcessName[MAXNAME];
//		DANSSPrimary
	struct PrimaryDataStruct {
		double EventID;
		double ID;
		double Energy;
		double X, Y, Z;
	} PrimaryData;
	char PrimaryName[MAXNAME];
	char PrimaryMaterialName[MAXNAME];
//		DANSSEvent
	struct EventDataNewStruct {
		double EventID;
		double ParticleEnergy;
		double EnergyLoss;
		double DetectorEnergyLoss;
		double CopperEnergyLoss;
		double GdCoverEnergyLoss;
		double X, Y, Z, DirX, DirY, DirZ;
		char   FluxFlag;
	} EventDataNew;
	struct EventDataOldStruct {
		double EventID;
		double ParticleEnergy;
		double EnergyLoss;
		double DetectorEnergyLoss;
		double CopperEnergyLoss;
		double GdCoverEnergyLoss;
		double X, Y, Z, DirX, DirY, DirZ;
		double TimelineShift;
		char   FluxFlag;
	} EventDataOld;
//		DANSSSignal
	struct SiPMSignalDataStruct {	// the same old and new
		double SiPMSumSignal;
		double NSiPMFired;
		double SiPMSignal[25][100];
	} SiPMSignalData;
	struct PMTSignalDataStruct {
		double PMTSumSignal;
		double PMTSignalOdd[5][5];
		double PMTSignalEven[5][5];	// end of new
		double PMTTimelineOdd[5][5][25000];
		double PMTTimelineEven[5][5][25000];
	} PMTSignalData;
	struct RealSignalDataNewStruct {
		double EventID;
		double RealSumSignal;
		double RealSignal[25][100];
		double RealSignalDist[25][100];
	} RealSignalDataNew;
	struct RealSignalDataOldStruct {
		double EventID;
		double RealSumSignal;
		double RealSumSignalTimeline[25000];
		double RealSignal[25][100];
		double RealSignalDist[25][100];
	} RealSignalDataOld;
//		DANSSVeto
	struct VetoSignalDataStruct {
		double EventID;
		double RealSumSignal;
		double RealCornerSumSignal;
		double RealSignal[6][8];
		double SumSignal;
		double CornerSumSignal;
		double Signal[6][8];		// end of new
		double Timeline[6][8][25000];
	} VetoSignalData;
//		DANSSSiPMHits
	struct SiPMHitBranchStruct {
		long EventID;
		int Column;
		int Row;
		double Time;
		long Signal;
	} SiPMHitBranch;
//		DANSSPMTHits
	struct PMTHitBranchStruct {
		long EventID;
		int Column;
		int Row;
		double Time;
		long Signal;
		char PMTEvenGroupFlag;
	} PMTHitBranch;
//		DANSSVetoHits
	struct VetoHitBranchStruct {
		long EventID;
		int Panel;
		int Slab;
		double Time;
		long Signal;
	} VetoHitBranch;
//		DANSSSiPMTimeline
	struct SiPMTimelineBranchStruct {
		double EventID;
		double Column;
		double Row;
		double SiPMTimeline[25000];
	} SiPMTimelineBranch;
#pragma pack(pop)

int main(int argc, char **argv)
{
/****************************************************************
 *			Local variables				*
 ****************************************************************/
	long Nevents;
	long EventID;
	long nMax;
	long iFirst;
	struct SiPMHitBranchStruct *SiPMHits;
	struct PMTHitBranchStruct *PMTHits;
	struct VetoHitBranchStruct *VetoHits;
	long KSiPMHits, LSiPMHits, PtrSiPMHits, NSiPMHits;
	long KPMTHits, LPMTHits, PtrPMTHits, NPMTHits;
	long KVetoHits, LVetoHits, PtrVetoHits, NVetoHits;
	long NPrimary, NParticle;
	double minTime;
	double *SiPMTimeline;
	char SiPMArray[25][100];
	long i;
	int j;
/****************************************************************
 *			Files and arguments			*
 ****************************************************************/
	if (argc < 3) {
		printf("Usage %s new_format.root old_format.root [Nentries [FirstEntry]]--- convert MC new root files to old format\n", argv[0]);
		printf("Nentries - copy Nentries only.  FirstEntry - first entry to copy.\n");
		return 10;
	}
	
	nMax = TTree::kMaxEntries;
	iFirst = 0;
//	TFile *fIn = new TFile(argv[1]);
	TFile *fOut = new TFile(argv[2], "RECREATE");
	if (argc > 3) nMax = strtol(argv[3], NULL, 10);
	if (argc > 4) iFirst = strtol(argv[4], NULL, 10);
	if (!fOut->IsOpen()) return 20;
/****************************************************************
 *		Input file trees				*
 ****************************************************************/
//		DANSSParticle - exact copy
	TFile *fInParticle = new TFile(argv[1]);
	TTree *tInParticle = (TTree *) fInParticle->Get("DANSSParticle");
	if (!tInParticle) {
		printf("Tree DANSSParticle not found.\n");
		return 30;
	}
	tInParticle->SetBranchAddress("ParticleData", &ParticleData);
	tInParticle->SetBranchAddress("ParticleName", ParticleName);
	tInParticle->SetBranchAddress("ParticleMaterialName", ParticleMaterialName);
	tInParticle->SetBranchAddress("ParticleCreatorProcessName", ParticleCreatorProcessName);
//		DANSSPrimary - exact copy
	TFile *fInPrimary = new TFile(argv[1]);
	TTree *tInPrimary = (TTree *) fInPrimary->Get("DANSSPrimary");
	if (!tInPrimary) {
		printf("Tree DANSSPrimary not found.\n");
		return 31;
	}
	tInPrimary->SetBranchAddress("PrimaryData", &PrimaryData);
	tInPrimary->SetBranchAddress("PrimaryName", PrimaryName);
	tInPrimary->SetBranchAddress("PrimaryMaterialName", PrimaryMaterialName);
//		DANSSRun - exact copy
	TFile *fInRun = new TFile(argv[1]);
	TTree *tInRun = (TTree *) fInRun->Get("DANSSRun");
	if (!tInRun) {
		printf("Tree DANSSRun not found.\n");
		return 32;
	}
//		DANSSEvent - Open new
	TFile *fInEvent = new TFile(argv[1]);
	TTree *tInEvent = (TTree *) fInEvent->Get("DANSSEvent");
	if (!tInEvent) {
		printf("Tree DANSSEvent not found.\n");
		return 33;
	}
	tInEvent->SetBranchAddress("EventData", &EventDataNew);
//		DANSSSignal - Open new
	TFile *fInSignal = new TFile(argv[1]);
	TTree *tInSignal = (TTree *) fInSignal->Get("DANSSSignal");
	if (!tInSignal) {
		printf("Tree DANSSSignal not found.\n");
		return 34;
	}
	tInSignal->SetBranchAddress("SiPMSignalData", &SiPMSignalData);
	tInSignal->SetBranchAddress("PMTSignalData", &PMTSignalData);
	tInSignal->SetBranchAddress("RealSignalData",&RealSignalDataNew);
//		DANSSVeto - Open new
	TFile *fInVeto = new TFile(argv[1]);
	TTree *tInVeto = (TTree *) fInVeto->Get("DANSSVeto");
	if (!tInVeto) {
		printf("Tree DANSSVeto not found.\n");
		return 35;
	}
	tInVeto->SetBranchAddress("VetoSignalData", &VetoSignalData);
//		DANSS*Hit - new only
	TFile *fInSiPMHit = new TFile(argv[1]);
	TTree *tInSiPMHit = (TTree *) fInSiPMHit->Get("DANSSSiPMHit");
	if (!tInSiPMHit) {
		printf("Tree DANSSSiPMHit not found.\n");
		return 36;
	}
	tInSiPMHit->SetBranchAddress("SiPMHitBranch", &SiPMHitBranch);
	
	TFile *fInPMTHit = new TFile(argv[1]);
	TTree *tInPMTHit = (TTree *) fInPMTHit->Get("DANSSPMTHit");
	if (!tInPMTHit) {
		printf("Tree DANSSPMTHit not found.\n");
		return 37;
	}
	tInPMTHit->SetBranchAddress("PMTHitBranch", &PMTHitBranch);
	
	TFile *fInVetoHit = new TFile(argv[1]);
	TTree *tInVetoHit = (TTree *) fInVetoHit->Get("DANSSVetoHit");
	if (!tInVetoHit) {
		printf("Tree DANSSVetoHit not found.\n");
		return 38;
	}
	tInVetoHit->SetBranchAddress("VetoHitBranch", &VetoHitBranch);
/****************************************************************
 *		Set event range					*
 ****************************************************************/
	Nevents = tInEvent->GetEntries();
	if (iFirst >= Nevents) {
		printf("FirstEntry (%Ld) >= Nentries (%Ld) - nothing to do.\n", iFirst, Nevents);
		return 39;
	}
	if (iFirst + nMax > Nevents) nMax = Nevents - iFirst;
/****************************************************************
 *		Outputfile trees				*
 ****************************************************************/
	fOut->cd();
	TTree *tOutRun = tInRun->CloneTree(-1, "fast");		// always just a copy
	TTree *tOutParticle = tInParticle->CloneTree(0);	// create tree only
	TTree *tOutPrimary = tInPrimary->CloneTree(0);		// create tree only
	TTree *tOutEvent = new TTree("DANSSEvent", "DANSS event tree");
	tOutEvent->Branch("EventData", &EventDataOld, 
		"EventID/D:ParticleEnergy:EnergyLoss:DetectorEnergyLoss:CopperEnergyLoss:GdCoverEnergyLoss:X:Y:Z:DirX:DirY:DirZ:TimelineShift:FluxFlag/B");
	TTree *tOutSignal = new TTree("DANSSSignal", "DANSS strip signal tree");
	tOutSignal->Branch("SiPMSignalData", &SiPMSignalData, "SiPMSumSignal/D:NSiPMFired:SiPMSignal[25][100]/D");
	tOutSignal->Branch("PMTSignalData", &PMTSignalData, 
		"PMTSumSignal/D:PMTSignalOdd[5][5]/D:PMTSignalEven[5][5]:PMTTimelineOdd[5][5][25000]/D:PMTTimelineEven[5][5][25000]/D");
	tOutSignal->Branch("RealSignalData", &RealSignalDataOld, 
		"EventID/D:RealSumSignal/D:RealSumSignalTimeline[25000]/D:RealSignal[25][100]/D:RealSignalDist[25][100]/D");
	memset(RealSignalDataOld.RealSumSignalTimeline, 0, sizeof(RealSignalDataOld.RealSumSignalTimeline));	// just erase - no information in new MC
	TTree *tOutVeto = new TTree("DANSSVeto", "DANSS veto signal tree");
	tOutVeto->Branch("VetoSignalData", &VetoSignalData, 
		"EventID/D:RealSumSignal/D:RealCornerSumSignal/D:RealSignal[6][8]/D:SumSignal/D:CornerSumSignal/D:Signal[6][8]/D:Timeline[6][8][25000]/D");
	TTree *tOutSiPMTimeline = new TTree("DANSSSiPMTimeline", "DANSS SiPM timeline tree");
	tOutSiPMTimeline->Branch("SiPMTimelineBranch", &SiPMTimelineBranch, "EventID/D:Column:Row:SiPMTimeline[25000]");
	SiPMTimeline = (double *) malloc(25*100*25000*sizeof(double));
	if (!SiPMTimeline) {
		printf("Fatal - no memory for SiPMTimeline\n");
		return 40;
	}
	TTree::SetMaxTreeSize(400000000000LL);
/****************************************************************
 *		Process DANSSParticle and DANSSPrimary		*
 ****************************************************************/
	NPrimary = tInPrimary->GetEntries();
	for(i=0; i < NPrimary; i++) {
		tInPrimary->GetEntry(i);
		if (PrimaryData.EventID < iFirst + 1) continue;
		if (PrimaryData.EventID > iFirst + nMax) break;
		tOutPrimary->Fill();
	}
	NParticle = tInParticle->GetEntries();
	for(i=0; i < NParticle; i++) {
		tInParticle->GetEntry(i);
		if (ParticleData.EventID < iFirst + 1) continue;
		if (ParticleData.EventID > iFirst + nMax) break;
		tOutParticle->Fill();
	}
/****************************************************************
 *		Process events					*
 ****************************************************************/
	NSiPMHits = tInSiPMHit->GetEntries();
	NPMTHits = tInPMTHit->GetEntries();
	NVetoHits = tInVetoHit->GetEntries();
	PtrSiPMHits = PtrPMTHits = PtrVetoHits = 0;
	LSiPMHits = LPMTHits = LVetoHits = 0;
	SiPMHits = NULL;
	PMTHits = NULL;
	VetoHits = NULL;
	
	for (EventID = iFirst + 1; EventID <= iFirst + nMax; EventID++) {
//		Read Event data
		tInEvent->GetEntry(EventID - 1);
		tInSignal->GetEntry(EventID - 1);
		tInVeto->GetEntry(EventID - 1);
//		Read hits
		KSiPMHits = KPMTHits = KVetoHits = 0;
		for(;PtrSiPMHits < NSiPMHits; PtrSiPMHits++) {
			tInSiPMHit->GetEntry(PtrSiPMHits);
			if(SiPMHitBranch.EventID < EventID) continue;
			if(SiPMHitBranch.EventID > EventID) break;
			if (KSiPMHits >= LSiPMHits) {
				LSiPMHits += SLICE;
				SiPMHits = (struct SiPMHitBranchStruct *) realloc(SiPMHits, LSiPMHits * sizeof(struct SiPMHitBranchStruct));
				if (!SiPMHits) {
					printf("Fatal - no memory for SiPMHits[%d]\n", LSiPMHits);
					return 41;
				}
			}
			memcpy(&SiPMHits[KSiPMHits], &SiPMHitBranch, sizeof(struct SiPMHitBranchStruct));
			KSiPMHits++;
		}
		for(;PtrPMTHits < NPMTHits; PtrPMTHits++) {
			tInPMTHit->GetEntry(PtrPMTHits);
			if(PMTHitBranch.EventID < EventID) continue;
			if(PMTHitBranch.EventID > EventID) break;
			if (KPMTHits >= LPMTHits) {
				LPMTHits += SLICE;
				PMTHits = (struct PMTHitBranchStruct *) realloc(PMTHits, LPMTHits * sizeof(struct PMTHitBranchStruct));
				if (!PMTHits) {
					printf("Fatal - no memory for PMTHits[%d]\n", LPMTHits);
					return 42;
				}
			}
			memcpy(&PMTHits[KPMTHits], &PMTHitBranch, sizeof(struct PMTHitBranchStruct));
			KPMTHits++;
		}
		for(;PtrVetoHits < NVetoHits; PtrVetoHits++) {
			tInVetoHit->GetEntry(PtrVetoHits);
			if(VetoHitBranch.EventID < EventID) continue;
			if(VetoHitBranch.EventID > EventID) break;
			if (KVetoHits >= LVetoHits) {
				LVetoHits += SLICE;
				VetoHits = (struct VetoHitBranchStruct *) realloc(VetoHits, LVetoHits * sizeof(struct VetoHitBranchStruct));
				if (!VetoHits) {
					printf("Fatal - no memory for VetoHits[%d]\n", LVetoHits);
					return 43;
				}
			}
			memcpy(&VetoHits[KVetoHits], &VetoHitBranch, sizeof(struct VetoHitBranchStruct));
			KVetoHits++;
		}
//		Find the first hit in time
		minTime = 1.0e99;	// something large
		for(i=0; i<KSiPMHits; i++) if (SiPMHits[i].Time < minTime) minTime = SiPMHits[i].Time;
		for(i=0; i<KPMTHits; i++) if (PMTHits[i].Time < minTime) minTime = PMTHits[i].Time;
		for(i=0; i<KVetoHits; i++) if (VetoHits[i].Time < minTime) minTime = VetoHits[i].Time;
//		Old DANSSEvent, DANSSSignal, DANSSVeto
		memcpy(&EventDataOld, &EventDataNew, sizeof(EventDataNew));
		EventDataOld.TimelineShift = minTime;
		EventDataOld.FluxFlag = EventDataNew.FluxFlag;
		memset(PMTSignalData.PMTTimelineOdd, 0, sizeof(PMTSignalData.PMTTimelineOdd));
		memset(PMTSignalData.PMTTimelineEven, 0, sizeof(PMTSignalData.PMTTimelineEven));
		memset(VetoSignalData.Timeline, 0, sizeof(VetoSignalData.Timeline));
		RealSignalDataOld.EventID = RealSignalDataNew.EventID;
		RealSignalDataOld.RealSumSignal = RealSignalDataNew.RealSumSignal;
		memcpy(RealSignalDataOld.RealSignal, RealSignalDataNew.RealSignal, sizeof(RealSignalDataOld.RealSignal));
		memcpy(RealSignalDataOld.RealSignalDist, RealSignalDataNew.RealSignalDist, sizeof(RealSignalDataOld.RealSignalDist));
		for(i=0; i<KPMTHits; i++) {
			j = (PMTHits[i].Time - minTime + TIMELINESHIFT) / 8;
			if (j < 0 || j >= 25000) continue;
			if (PMTHits[i].PMTEvenGroupFlag) {
				PMTSignalData.PMTTimelineEven[PMTHits[i].Column][PMTHits[i].Row][j] += PMTHits[i].Signal;
			} else {
				PMTSignalData.PMTTimelineOdd[PMTHits[i].Column][PMTHits[i].Row][j] += PMTHits[i].Signal;
			}
		}
		for(i=0; i<KVetoHits; i++) {
			j = (VetoHits[i].Time - minTime + TIMELINESHIFT) / 8;
			if (j < 0 || j >= 25000) continue;
			VetoSignalData.Timeline[VetoHits[i].Panel][VetoHits[i].Slab][j] += VetoHits[i].Signal;
		}
		tOutEvent->Fill();
		tOutSignal->Fill();
		tOutVeto->Fill();
//		Old SiPMTimeline
		memset(SiPMArray, 0, sizeof(SiPMArray));
		for(i=0; i<KSiPMHits; i++) {
			j = (SiPMHits[i].Time - minTime + TIMELINESHIFT) / 8;
			if (j < 0 || j >= 25000) continue;
			if(!SiPMArray[SiPMHits[i].Column][SiPMHits[i].Row]) {
				memset(&SiPMTimeline[(SiPMHits[i].Column * 100 + SiPMHits[i].Row) * 25000], 0, 25000 * sizeof(double));
				SiPMArray[SiPMHits[i].Column][SiPMHits[i].Row] = 1;
			}
			SiPMTimeline[(SiPMHits[i].Column * 100 + SiPMHits[i].Row) * 25000 + j] += SiPMHits[i].Signal;
		}
		for (i=0; i<25; i++) for (j=0; j<100; j++) if (SiPMArray[i][j]) {
			SiPMTimelineBranch.EventID = EventID;
			SiPMTimelineBranch.Column = i;
			SiPMTimelineBranch.Row = j;
			memcpy(SiPMTimelineBranch.SiPMTimeline, &SiPMTimeline[(i * 100 + j) * 25000], 25000 * sizeof(double));
			tOutSiPMTimeline->Fill();
		}
	}
/****************************************************************
 *		Write and close					*
 ****************************************************************/
	fOut->cd();
	tOutParticle->Write();
	tOutPrimary->Write();
	tOutRun->Write();
	tOutEvent->Write();
	tOutSignal->Write();
	tOutVeto->Write();
	tOutSiPMTimeline->Write();
	fOut->Close();
	fInParticle->Close();
	fInPrimary->Close();
	fInEvent->Close();
	fInRun->Close();
	fInSignal->Close();
	fInVeto->Close();
	fInSiPMHit->Close();
	fInPMTHit->Close();
	fInVetoHit->Close();
	if (SiPMHits) free(SiPMHits);
	if (PMTHits) free(PMTHits);
	if (VetoHits) free(VetoHits);
	free(SiPMTimeline);
	return 0;
}
