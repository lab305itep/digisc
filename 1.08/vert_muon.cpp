#include <stdio.h>
#include <TFile.h>
#include <TTree.h>
#include "../evtbuilder.h"

#define iMaxDataElements 2600
#define bSiPm 0
#define bPmt  1

double PMTYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.00147;
	double rez;
	rez = (0.987387*exp(-0.0016*(x-48)) + 0.023973*exp(-0.0877*(x-48)) - 0.0113581*exp(-0.1042*(x-48))
	        -2.30972E-6*exp(0.2214*(x-48))) / FuncAverage;
	return rez;
}

double SiPMYAverageLightColl(double x)
{
    //<func(x)=1>
	const double FuncAverage = 1.02208;
	double rez;
	rez = (0.00577381*exp(-0.1823*(x-48)) + 0.999583*exp(-0.0024*(x-48)) - 8.095E-13*exp(0.5205*(x-48))
	        -0.00535714*exp(-0.1838*(x-48))) / FuncAverage;
	return rez;
}


int main(int argc, char **argv)
{
	int i, j, k, Nevents, Nhits;
	int CntTrk;
	double xUp, yUp, xDown, yDown;
	double ExUp, EyUp, ExDown, EyDown;
	double r2;
	double Lcorr, Ccorr;
	struct DanssEventStruct7 DanssEvent;
	struct HitStruct {
		float E[iMaxDataElements];
		float T[iMaxDataElements];
		struct HitTypeStruct type[iMaxDataElements];
	} HitArray;
	struct MuonHitStruct {
		float E;
		int N;
	} MuonHit;
	struct MuonEventStruct {
		int MCNum;
		int NHits;
		float xUp;
		float yUp;
		float xDown;
		float yDown;
		float r2;
	} MuonEvent;
	double XZ[50][25], YZ[50][25];

	if (argc < 3) {
		printf("Usage: %s input_file.root output_file.root\n", argv[0]);
		return 10;
	}
	
	TFile *fIn = new TFile(argv[1]);
	TFile *fOut = new TFile(argv[2], "RECREATE");
	if (!fIn->IsOpen() || !fOut->IsOpen()) return 20;
	TTree *tIn = (TTree *) fIn->Get("DanssEvent");
	if (!tIn) {
		printf("Can not open the tree.\n");
		return 30;
	}
	tIn->SetBranchAddress("Data", &DanssEvent);
	tIn->SetBranchAddress("HitE", HitArray.E);
	tIn->SetBranchAddress("HitT", HitArray.T);
	tIn->SetBranchAddress("HitType", HitArray.type);
	fOut->cd();
	TTree *tOut = new TTree("MuonHits", "Muon hits in SiPm and PMT");
	tOut->Branch("Hits", &MuonHit, "E/F:N/I");
	TTree *tEvt = new TTree("MuonEvents", "Selected muon events");
	tEvt->Branch("Events", &MuonEvent, "MCNum/I:NHits/I:xUp/F:yUp/F:xDown/F:yDown/F:r2/F");
	Nevents = tIn->GetEntries();
	CntTrk = 0;
	
	for (i=0; i<Nevents; i++) {
		tIn->GetEntry(i);
		// Fill XYZ
		memset(XZ, 0, sizeof(XZ));
		memset(YZ, 0, sizeof(YZ));
		for (j=0; j<DanssEvent.NHits; j++) {
			switch(HitArray.type[j].type) {
			case bSiPm:
				if (HitArray.type[j].z & 1) {	// X
					XZ[HitArray.type[j].z / 2][HitArray.type[j].xy] = HitArray.E[j];
				} else {			// Y
					YZ[HitArray.type[j].z / 2][HitArray.type[j].xy] = HitArray.E[j];
				}
				break;
			case bPmt:
				break;
			}
		}
		// Find the track up and down crossings
		xUp = yUp = xDown = yDown = ExUp = EyUp = ExDown = EyDown = 0;
		for(k=0; k<2; k++) for(j=0; j<25; j++) {
			if (XZ[k][j] > 0) {
				xDown += (4*j + 2.0) * XZ[k][j];
				ExDown += XZ[k][j];
			}
			if (YZ[k][j] > 0) {
				yDown += (4*j + 2.0) * YZ[k][j];
				EyDown += YZ[k][j];
			}
			if (XZ[k+48][j] > 0) {
				xUp += (4*j + 2.0) * XZ[k+48][j];
				ExUp += XZ[k+48][j];
			}
			if (YZ[k+48][j] > 0) {
				yUp += (4*j + 2.0) * YZ[k+48][j];
				EyUp += YZ[k+48][j];
			}
		}
		if (ExUp < 0.1 || EyUp < 0.1 || ExDown < 0.1 || EyDown < 0.1) continue;
		xUp /= ExUp;
		xDown /= ExDown;
		yUp /= EyUp;
		yDown /= EyDown;
		r2 = (xUp - xDown) * (xUp - xDown) + (yUp - yDown) * (yUp - yDown);
//		if (i < 100) printf("%d: Up(%5.1f, %5.1f) Down(%5.1f, %5.1f) sqrt(r2) = %5.1f\n",
//			i, xUp, yUp, xDown, yDown, sqrt(r2));
		if (r2 > 400) continue;		// this corresponds to 2% increase in the track length
		CntTrk++;
		Nhits = 0;
		// Store good muon hits
		for(k=0; k<50; k++) for(j=0; j<25; j++) if (XZ[k][j] > 0) {
			// cuts for no leakage
			if (j > 0 && XZ[k][j-1] > 0) continue;
			if (j < 24 && XZ[k][j+1] > 0) continue;
			// cuts for strips above and below
			if (k > 0 && XZ[k-1][j] < 0.1) continue;
			if (k < 49 && XZ[k+1][j] < 0.1) continue;
			// Good hit found - fill the tree
			Lcorr = yDown + (yUp - yDown) * k / 48.0;
			Ccorr = SiPMYAverageLightColl(Lcorr);
			MuonHit.E = XZ[k][j] * Ccorr;
			MuonHit.N = 10000 + 100 * k + j;
			tOut->Fill();
			Nhits++;
		}
		for(k=0; k<50; k++) for(j=0; j<25; j++) if (YZ[k][j] > 0) {
			// cuts for no leakage
			if (j > 0 && YZ[k][j-1] > 0) continue;
			if (j < 24 && YZ[k][j+1] > 0) continue;
			// cuts for strips above and below
			if (k > 0 && YZ[k-1][j] < 0.1) continue;
			if (k < 49 && YZ[k+1][j] < 0.1) continue;
			// Good hit found - fill the tree
			Lcorr = xDown + (xUp - xDown) * (k - 1) / 48.0;
			Ccorr = SiPMYAverageLightColl(Lcorr);
			MuonHit.E = YZ[k][j] * Ccorr;
			MuonHit.N = 100 * k + j;
			tOut->Fill();
			Nhits++;
		}
		MuonEvent.MCNum = DanssEvent.unixTime;
		MuonEvent.NHits = Nhits;
		MuonEvent.xUp = xUp;
		MuonEvent.yUp = yUp;
		MuonEvent.xDown = xDown;
		MuonEvent.yDown = yDown;
		MuonEvent.r2 = r2;
		tEvt->Fill();
	}
	
	fOut->cd();
	tOut->Write();
	tEvt->Write();
	fOut->Close();
	fIn->Close();
	
	printf("%d events processed - %d vertical tracks found\n", Nevents, CntTrk);
	
	return 0;
}
