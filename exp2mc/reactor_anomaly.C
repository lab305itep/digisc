void reactor_anomaly(void)
{
	const double reno[20] = {
		1.00, 1.02, 0.995, 1.00, 1.00, 1.015, 1.005, 0.98,  1.02, 1.055,
		1.09, 1.12, 1.13,  1.12, 1.06, 1.05,  1.04,  1.005, 0.95, 0.98
	};
	const double renoe[20] = {
		0.01, 0.01,  0.01,  0.01,  0.01,  0.01, 0.01, 0.01,  0.01, 0.01,
		0.01, 0.015, 0.015, 0.015, 0.015, 0.02, 0.02, 0.025, 0.03, 0.03
	};
	TF1 *fGaus;
	TH1D *hReno;
	TH1D *hDanss1;
	TH1D *hDanss2;
	int i;
	
	gStyle->SetTitleXSize(0.06);
	gStyle->SetTitleYSize(0.06);
	gStyle->SetLabelSize(0.06);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadBottomMargin(0.16);
	gStyle->SetOptStat(0);
	hReno = new TH1D("hReno", "RENO data on the ratio experiment to Monte-Carlo;MeV;Experiment to Monte-Carlo", 20, 1, 6);
	hDanss1 = new TH1D("hDanss1", "RENO ratio spoiled by DANSS resolution 33%/#sqrt{E};MeV;Experiment to Monte-Carlo", 20, 1, 6);
	hDanss2 = new TH1D("hDanss2", "RENO ratio spoiled by DANSS resolution 40%/#sqrt{E};MeV;Experiment to Monte-Carlo", 20, 1, 6);
	for (i=0; i<20; i++) {
		hDanss1->SetBinContent(i+1, 1);
		hDanss2->SetBinContent(i+1, 1);
	}

	hReno->SetMarkerColor(kBlack);
	hReno->SetLineColor(kBlack);
	hReno->SetMarkerStyle(20);
	hReno->SetMarkerSize(2);
	hReno->GetYaxis()->SetLabelSize(0.06);
	hDanss1->SetMarkerColor(kBlue);
	hDanss1->SetMarkerStyle(25);
	hDanss1->SetMarkerSize(1);
	hDanss1->SetLineColor(kBlue);
	hDanss1->SetLineWidth(2);
	hDanss2->SetMarkerColor(kRed);
	hDanss2->SetMarkerStyle(21);
	hDanss2->SetMarkerSize(2);
	
	for(i=0; i<20; i++) hReno->SetBinContent(i+1, reno[i]);
	for(i=0; i<20; i++) hReno->SetBinError(i+1, renoe[i]);

	fGaus = new TF1("fGaus", "gaus", -30.0, 30.0);
	for (i=0; i<20; i++) {
		fGaus->SetParameter(0, 1);	// amplitude
		fGaus->SetParameter(1, 1.125 + 0.25*i);	// position
		fGaus->SetParameter(2, 0.33*sqrt(1.125 + 0.25*i));	// DANSS resolution: 33%/sqrt(E)
		hDanss1->Add(fGaus, (reno[i]-1)*0.25/fGaus->Integral(-30.0, 30.0));
		fGaus->SetParameter(2, 0.40*sqrt(1.125 + 0.25*i));	// DANSS resolution: 40%/sqrt(E)
		hDanss2->Add(fGaus, (reno[i]-1)*0.25/fGaus->Integral(-30.0, 30.0));
	}

	hReno->SetTitle(";MeV;");
//	hReno->SetMinimum(0.9);
//	hReno->SetMaximum(1.2);
	hReno->DrawCopy("P0");
	hDanss1->DrawCopy("P0,same,smooth");
	hDanss2->DrawCopy("P0,same");

	TLegend *lg = new TLegend(0.2, 0.75, 0.5, 0.9);
	lg->AddEntry(hReno, "RENO data", "LP");
	lg->AddEntry(hDanss1, "DANSS optimistic", "LP");
	lg->AddEntry(hDanss2, "DANSS pessimistic", "P");
	lg->Draw();
}
