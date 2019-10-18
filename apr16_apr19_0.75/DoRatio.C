TH1D *DoRatio(TFile *f, const char *suffix)
{
	char str[1024];
	sprintf(str, "hUp_%s", suffix);
	TH1D *hUp = (TH1D*) f->Get(str);
	sprintf(str, "hDown_%s", suffix);
	TH1D *hDown = (TH1D*) f->Get(str);
	sprintf(str, "hConst_%s", suffix);
	TH1D *hC = (TH1D*) f->Get(str);
	
	if (!hUp || !hDown || !hC) return NULL;
	
	double PwrUp = hC->GetBinContent(7);
	double PwrDown = hC->GetBinContent(9);
	double DeadUp = hC->GetBinContent(10);
	double DeadDown = hC->GetBinContent(12);
	double EffUp = hC->GetBinContent(13);
	double EffDown = hC->GetBinContent(15);
	
	sprintf(str, "hRatio_%s", suffix);
	TH1D *hR = (TH1D*) hUp->Clone(str);
	sprintf(str, "Ratio Down / Up for %s", suffix);
	hR->SetTitle(str);
	hR->Divide(hDown, hUp);
	double k = PwrUp * EffUp * (1 - DeadUp) / (PwrDown * EffDown * (1 - DeadDown));
	printf("Scale = %f\n", k);
	hR->Scale(k);
	
	return hR;
}
