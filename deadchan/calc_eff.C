#define NVAR	173
void calc_eff(const char *run_file = NULL)
{
	const int MCTotal = 2000000;	// MC events played
	const char * vardir = "vars_4.0"; 
	int i;
	char str[1024];
	double eff[NVAR];
	TH1D *h;
	TFile *f;
	FILE *fEff;
	FILE *fOut;
	int run, var;
	char *tok;
	
//	for (i=0; i<63; i++) {
//		sprintf(str, ".x mcfuelspectra.C(\"Eff/var_%d_fuel.root\", \"Eff/var_%d_spfuel.root\")", i, i);
//		gROOT->ProcessLine(str);
//	}
	for (i=0; i<NVAR; i++) {
		sprintf(str, "%s/var_%d_spfuel.root", vardir, i);
		f = new TFile(str);
		h = (TH1D *) f->Get("hMC");
		if (!h) {
			printf("hMC not found in %s\n", str);
			return;
		}
		eff[i] = h->Integral(9, 64) / MCTotal;
	}
	
	fEff = fopen("eff_4.0.txt", "wt");
	if (!fEff) {
		printf("can not open file eff.txt: %m\n");
		return;
	}
	for (i=0; i<NVAR; i++) fprintf(fEff, "%2d %f\n", i, eff[i]);
	fclose(fEff);
	
	if (run_file) {
		fEff = fopen(run_file, "rt");
		if (!fEff) {
			printf("can not open file %s: %m\n", run_file);
			return;
		}
		fOut = fopen("run_eff_4.0.txt", "wt");
		if (!fOut) {
			printf("can not open file run_eff.txt: %m\n");
			return;
		}
	
		for(;;) {
			if (!fgets(str, sizeof(str), fEff)) break;	// EOF
			tok =strtok(str, " \t");
			if (!tok) continue;
			run = strtol(tok, NULL, 10);
			tok =strtok(NULL, " \t");
			if (!tok) continue;
			var = strtol(tok, NULL, 10);
			if (var <0 || var >= NVAR) continue;
			fprintf(fOut, "%6d %2d %7.5f\n", run, var, eff[var]);
		}
		fclose(fEff);
		fclose(fOut);
	}
}
