void draw_mu_calib(const char *fname)
{
	char str[1024];
	char *ptr;
	float rSiPM, kSiPM, cSiPM;
	float rPMT, kPMT, cPMT;
	float chi2SiPM, chi2PMT;
	
	FILE *fIn = fopen(fname, "rt");
	if (!fIn) {
		printf("%s: %m.\n", fname);
		return;
	}
	strcpy(str, fname);
	ptr = strrchr(str, '.');
	if (ptr) *ptr = '\0';
	strcat(str, ".root");
	TFile fOut(str, "RECREATE");
	if (!fOut.IsOpen()) return;
	TH2D *hSiPM = new TH2D("hSiPM", "SiPM scan;scale;smearing", 21, 0.9475, 1.0525, 21, 0.0475, 0.1525);
	TH2D *hPMT  = new TH2D("hPMT",  "PMT scan;scale;smearing",  21, 0.9475, 1.0525, 21, 0.0075, 0.1125);
	for(;;) {
		if(!fgets(str, sizeof(str), fIn)) break;
		ptr = str;
		for(ptr = str; *ptr; ptr++) if (isdigit(*ptr)) break;
		if(!*ptr) continue;
		sscanf(ptr, "%f %f %f %f %f %f : %f %f", 
			&rSiPM, &rPMT, &kSiPM, &kPMT, &cSiPM, &cPMT, &chi2SiPM, &chi2PMT);
		hSiPM->Fill(rSiPM, cSiPM, chi2SiPM);
		hPMT->Fill(rPMT, cPMT, chi2PMT);
	}
	hSiPM->Write();
	hPMT->Write();
	fOut.Close();
}
