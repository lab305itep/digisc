void calc_eff(int NVAR, const char *outname, const char *vardir, int MCTotal = 2000000)
{
	int i;
	char str[1024];
	double eff[NVAR];
	TH1D *h;
	TFile *f;
	FILE *fEff;
	FILE *fOut;
	
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

void sum_fuel(int NVAR, const char *dirbase)
{
	int i, j;
	char str[1024];
	TH1D *h;
	TH1D *hSum;
	TFile *f[4];
	TFile *fOut;
	
	for (i=0; i<NVAR; i++) {
		sprintf(str, "%s_235U/var_%d_spfuel.root", dirbase, i);
		f[0] = new TFile(str);
		if (!f[0]->IsOpen()) continue;
		sprintf(str, "%s_238U/var_%d_spfuel.root", dirbase, i);
		f[1] = new TFile(str);
		if (!f[1]->IsOpen()) continue;
		sprintf(str, "%s_239Pu/var_%d_spfuel.root", dirbase, i);
		f[2] = new TFile(str);
		if (!f[2]->IsOpen()) continue;
		sprintf(str, "%s_241Pu/var_%d_spfuel.root", dirbase, i);
		f[3] = new TFile(str);
		if (!f[3]->IsOpen()) continue;
		hSum = (TH1D *) f[0]->Get("hMC");
		if (!hSum) {
			printf("hMC not found in %s\n", f[0]->GetName());
			continue;
		}
		h = (TH1D *) f[1]->Get("hMC");
		if (!h) {
			printf("hMC not found in %s\n", f[1]->GetName());
			continue;
		}
		hSum->Add(h);
		h = (TH1D *) f[2]->Get("hMC");
		if (!h) {
			printf("hMC not found in %s\n", f[2]->GetName());
			continue;
		}
		hSum->Add(h);
		h = (TH1D *) f[3]->Get("hMC");
		if (!h) {
			printf("hMC not found in %s\n", f[3]->GetName());
			continue;
		}
		hSum->Add(h);
		sprintf(str, "%s/var_%d_spfuel.root", dirbase, i);
		fOut = new TFile(str, "RECREATE");
		if (!fOut->IsOpen()) continue;
		fOut->cd();
		hSum->Write();
		fOut->Close();
		for (j=0; j<4; j++) f[j]->Close();
	}
}
