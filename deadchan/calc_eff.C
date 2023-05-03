void calc_eff(int NVAR, const char *outname, const char *vardir, int MCTotal = 2000000)
{
	int i;
	char str[1024];
	double eff[NVAR];
	TH1D *h;
	TFile *f;
	FILE *fEff;
	FILE *fOut;
	int run, var;
	char *tok;
	
	memset(eff, 0, sizeof(eff));
	for (i=0; i<NVAR; i++) {
		sprintf(str, "%s/var_%d_spfuel.root", vardir, i);
		f = new TFile(str);
		if (!f->IsOpen()) continue;
		h = (TH1D *) f->Get("hMC");
		if (!h) {
			printf("hMC not found in %s\n", str);
			continue;
		}
		eff[i] = h->Integral(9, 64) / MCTotal;
	}
	
	fEff = fopen(outname, "wt");
	if (!fEff) {
		printf("can not open file %s: %m\n", outname);
		return;
	}
	for (i=0; i<NVAR; i++) fprintf(fEff, "%2d %f\n", i, eff[i]);
	fclose(fEff);
}
