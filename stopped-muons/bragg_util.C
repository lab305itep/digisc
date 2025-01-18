#define MAXZ 100

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

//	Draw a single track number num from a tree tIn
void draw_one(int num, TTree *tIn)
{
	struct StoppedMuonStruct Muon;
	double L[MAXZ], dedx[MAXZ];
	double scale;
	int i, k;
	
	tIn->SetBranchAddress("Stopped", &Muon);
	if (!tIn->GetEntry(num)) return;
	
	scale = sqrt(1 + tan(Muon.thetaX)*tan(Muon.thetaX) + tan(Muon.thetaY)*tan(Muon.thetaY));
	k = 0;
	printf("Scale = %f\n", scale);
	for (i=0; i<Muon.NHits; i++) if (Muon.Ehit[i] > 0) {
		L[k] = (0.25 + i) * scale;
		dedx[k] = (i) ? Muon.Ehit[i] / scale : 2 * Muon.Ehit[i] / scale;
		printf("%2d  %f MeV => %f cm %f MeV/cm\n",
			i, Muon.Ehit[i], L[k], dedx[k]);
		k++;
	}
	TGraph *gr = new TGraph(k, L, dedx);
	gr->SetMarkerStyle(kFullSquare);
	gr->SetMarkerColor(kBlue);
	gr->SetMarkerSize(0.9);
	gr->Draw("AP");
}

//	sum hEL histogramms
void sum_hEL(void)
{
	int i;
	char str[1024];
	TH2D *h;
	TH2D *hSum;
	TFile *fIn;
	
	hSum = NULL;
	for (i=0; i<167309; i+=20000) {
		sprintf(str, "bragg_%6.6d_%6.6d.root", i, i + 19999);
		fIn = new TFile(str);
		if (!fIn->IsOpen()) continue;
		h = (TH2D*) fIn->Get("hEL");
		if (!h) {
			printf("hEL not found in %s\n", str);
			continue;
		}
		if (hSum) {
			hSum->Add(h);
		} else {
			gROOT->cd();
			hSum = (TH2D*) h->Clone("hSum");
		}
		fIn->Close();
	}
	hSum->SetName("hEL");
	
	auto hELProfX = hSum->ProfileX("hEL_profX");
	
	TFile *fOut = new TFile("bragg_002210_167308.root", "RECREATE");
	hSum->Write();
	hELProfX->Write();
	fOut->Close();
}

//	Make MC delta files
void make_deltas(void)
{
	char str[1024];
	const char *mcdir = "/home/clusters/rrcmpi/alekseev/igor/stopped8n2/MC/Chikuma/Muons";
	const char *fnames[] = {
		"Hit_checker_cutted_Chikuma-bragg.root",
		"Hit_checker_cutted_Chikuma_Birks_el_0_0108-bragg.root",
		"Hit_checker_cutted_Chikuma_Birks_el_0_0308-bragg.root",
		"Hit_checker_cutted_Chikuma_Cher_coeff_0_033-bragg.root",
		"Hit_checker_cutted_Chikuma_Cher_coeff_0_233-bragg.root",
		"Hit_checker_cutted_Chikuma_main_Birks_0_0108-bragg.root",
		"Hit_checker_cutted_Chikuma_main_Birks_0_0308-bragg.root",
		"Hit_checker_cutted_Chikuma_paint_0_15-bragg.root",
		"Hit_checker_cutted_Chikuma_paint_0_45-bragg.root"
	};
	int i;
	TFile *fIn[9];
	TH1D *hIn[9];
	
	for (i=0; i<9; i++) {
		sprintf(str, "%s/%s", mcdir, fnames[i]);
		fIn[i] = new TFile(str);
		if (!fIn[i]->IsOpen()) return;
		hIn[i] = (TH1D *) fIn[i]->Get("hEL_profX");
		if (!hIn[i]) {
			printf("Hist hEL_profX not found in %s\n", str);
			return;
		}
	}
	TFile *fOut = new TFile("MC-bragg.root", "RECREATE");
	
	TH1D *hCenter = (TH1D *) hIn[0]->Clone("hCenter");
	TH1D *hDBirks = (TH1D *) hIn[0]->Clone("hDBirks");
	TH1D *hDCher = (TH1D *) hIn[0]->Clone("hDCher");
	TH1D *hDMBirks = (TH1D *) hIn[0]->Clone("hDMBirks");
	TH1D *hDPaint = (TH1D *) hIn[0]->Clone("hDPaint");
	
	hDBirks->Add(hIn[1], hIn[2], 0.5, -0.5);
	hDCher->Add(hIn[3], hIn[4], 0.5, -0.5);
	hDMBirks->Add(hIn[5], hIn[6], 0.5, -0.5);
	hDPaint->Add(hIn[7], hIn[8], 0.5, -0.5);
	
	hDBirks->Write();
	hDCher->Write();
	hDMBirks->Write();
	hDPaint->Write();
	hCenter->Write();
	fOut->Close();
	for (i=0; i<9; i++) fIn[i]->Close();
}
