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
		L[k] = (0.5 + i) * scale;
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
