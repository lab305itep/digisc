#define NHISTS 7
void drawAbsEff(const char *fname)
{
	const char *hname[NHISTS] = {"hBasic_M", "hXYZ_M", "hPH_M", "hNE_M", "hAH_M", "hR_M", "h1_M"};
	const enum EColor clr[NHISTS] = {kRed, kOrange, kMagenta, kCyan, kBlue, kGreen, kRed - 7};
	const char *htitle[NHISTS] = {"Basic cuts only", "Fiducial volume", "Positron cuts", "Neutron energy",
		"Annihilation gammas", "Distance", "Single hit clusters"};
	TH1D *hPrcM[NHISTS];
	char str[256];
	int i;
	
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	TH1D *hOrig = (TH1D*) f->Get("hOrig");
	if (!hOrig) {
		printf("Hist hOrig not found in %s.\n", fname);
		return;
	}
	hOrig->SetLineColor(kBlack);
	hOrig->SetLineWidth(2);
	hOrig->SetTitle("Applying cuts one after another, MC energy");
	hOrig->GetYaxis()->SetTitleOffset(1);
	hOrig->GetXaxis()->SetRange(1, 56);
	
	for (i=0; i<NHISTS; i++) {
		hPrcM[i] = (TH1D*) f->Get(hname[i]);
		if (!hPrcM[i]) {
			printf("Hist %s not found in %s.\n", hname[i], fname);
			return;
		}
		hPrcM[i]->SetLineColor(clr[i]);
		hPrcM[i]->SetLineWidth(2);
	}
	
	TCanvas *cv = new TCanvas("CV1", "CV1", 1300, 1000);
	hOrig->Draw("hist");
	for (i=0; i<NHISTS; i++) hPrcM[i]->Draw("hist,same");
	TLegend *lg = new TLegend(0.6, 0.55, 0.98, 0.9);
	lg->AddEntry(hOrig, "15 mln. MC events", "L");
	for (i=0; i<NHISTS; i++) {
		sprintf(str, "%s - %5.2f mln. events", htitle[i], hPrcM[i]->Integral(1,56)/1.0E6);
		lg->AddEntry(hPrcM[i], str, "L");
	}
	lg->Draw();
	cv->SaveAs("MCAbs.png");
}

void drawRatios(const char *fname)
{
	const char *hname[NHISTS] = {"hBasic_M", "hXYZ_M", "hPH_M", "hNE_M", "hAH_M", "hR_M", "h1_M"};
	const enum EColor clr[NHISTS] = {kRed, kOrange, kMagenta, kCyan, kBlue, kGreen, kRed - 7};
	const char *htitle[NHISTS] = {"Basic cuts only", "Fiducial volume", "Positron cuts", "Neutron energy",
		"Annihilation gammas", "Distance", "Single hit clusters"};
	TH1D *hPrcM[NHISTS];
	char str[256];
	int i;
	
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	TH1D *hOrig = (TH1D*) f->Get("hOrig");
	if (!hOrig) {
		printf("Hist hOrig not found in %s.\n", fname);
		return;
	}
	hOrig->SetLineColor(kBlack);
	
	for (i=0; i<NHISTS; i++) {
		hPrcM[i] = (TH1D*) f->Get(hname[i]);
		if (!hPrcM[i]) {
			printf("Hist %s not found in %s.\n", hname[i], fname);
			return;
		}
		hPrcM[i]->SetLineColor(clr[i]);
		hPrcM[i]->SetLineWidth(2);
		hPrcM[i]->Divide(hOrig);
	}
	hPrcM[0]->SetTitle("Applying cuts one after another, MC energy");
	hPrcM[0]->GetYaxis()->SetTitle("N/N_{0}");
	hPrcM[0]->GetYaxis()->SetTitleOffset(1);
	hPrcM[0]->GetXaxis()->SetRange(1, 56);
	
	TCanvas *cv = new TCanvas("CV2", "CV2", 1300, 1000);
	hPrcM[0]->Draw("hist");
	for (i=1; i<NHISTS; i++) hPrcM[i]->Draw("hist,same");
	TLegend *lg = new TLegend(0.4, 0.15, 0.85, 0.55);
	for (i=0; i<NHISTS; i++) lg->AddEntry(hPrcM[i], htitle[i], "L");
	lg->Draw();
	cv->SaveAs("MCRat.png");
}

void drawIncrements(const char *fname)
{
	const char *hname[NHISTS] = {"hBasic_M", "hXYZ_M", "hPH_M", "hNE_M", "hAH_M", "hR_M", "h1_M"};
	const enum EColor clr[NHISTS] = {kRed, kOrange, kMagenta, kCyan, kBlue, kGreen, kRed - 7};
	const char *htitle[NHISTS] = {"Basic cuts only", "Fiducial volume", "Positron cuts", "Neutron energy",
		"Annihilation gammas", "Distance", "Single hit clusters"};
	TH1D *hPrcM[NHISTS];
	char str[256];
	int i;
	
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	TH1D *hOrig = (TH1D*) f->Get("hOrig");
	if (!hOrig) {
		printf("Hist hOrig not found in %s.\n", fname);
		return;
	}
	hOrig->SetLineColor(kBlack);
	
	for (i=0; i<NHISTS; i++) {
		hPrcM[i] = (TH1D*) f->Get(hname[i]);
		if (!hPrcM[i]) {
			printf("Hist %s not found in %s.\n", hname[i], fname);
			return;
		}
		hPrcM[i]->SetLineColor(clr[i]);
		hPrcM[i]->SetLineWidth(2);
	}
	for (i=NHISTS-1; i>0; i--) hPrcM[i]->Divide(hPrcM[i-1]);
	hPrcM[0]->Divide(hOrig);
	hPrcM[0]->SetTitle("Cut to previous cut, MC energy");
	hPrcM[0]->GetYaxis()->SetTitle("N/N_{PREV}");
	hPrcM[0]->GetYaxis()->SetTitleOffset(1);
	hPrcM[0]->GetXaxis()->SetRange(1, 56);
	hPrcM[0]->SetMinimum(0);
	hPrcM[0]->SetMaximum(1);
	
	TCanvas *cv = new TCanvas("CV3", "CV3", 1300, 1000);
	hPrcM[0]->Draw("hist");
	for (i=1; i<NHISTS; i++) hPrcM[i]->Draw("hist,same");
	TLegend *lg = new TLegend(0.4, 0.15, 0.85, 0.55);
	for (i=0; i<NHISTS; i++) lg->AddEntry(hPrcM[i], htitle[i], "L");
	lg->Draw();
	cv->SaveAs("MCInc.png");
}

void drawRecovered(const char *fname)
{
	const char *hname[NHISTS] = {"hBasic_R", "hXYZ_R", "hPH_R", "hNE_R", "hAH_R", "hR_R", "h1_R"};
	const enum EColor clr[NHISTS] = {kRed, kOrange, kMagenta, kCyan, kBlue, kGreen, kRed - 7};
	const char *htitle[NHISTS] = {"Basic cuts only", "Fiducial volume", "Positron cuts", "Neutron energy",
		"Annihilation gammas", "Distance", "Single hit clusters"};
	TH1D *hPrcM[NHISTS];
	char str[256];
	int i;
	
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	hOrig->SetTitle("Applying cuts one after another");
	hOrig->GetYaxis()->SetTitleOffset(1);
	hOrig->GetXaxis()->SetRange(1, 64);

	for (i=0; i<NHISTS; i++) {
		hPrcM[i] = (TH1D*) f->Get(hname[i]);
		if (!hPrcM[i]) {
			printf("Hist %s not found in %s.\n", hname[i], fname);
			return;
		}
		hPrcM[i]->SetLineColor(clr[i]);
		hPrcM[i]->SetLineWidth(2);
	}
	hPrcM[0]->SetTitle("Applying cuts one after another");
	hPrcM[0]->GetYaxis()->SetTitleOffset(1);
	hPrcM[0]->GetXaxis()->SetRange(1, 56);
	
	TCanvas *cv = new TCanvas("CV4", "CV4", 1300, 1000);
	hPrcM[0]->Draw("hist");
	for (i=1; i<NHISTS; i++) hPrcM[i]->Draw("hist,same");
	TLegend *lg = new TLegend(0.6, 0.55, 0.98, 0.9);
	for (i=0; i<NHISTS; i++) {
		sprintf(str, "%s - %5.2f mln. events", htitle[i], hPrcM[i]->Integral(1,56)/1.0E6);
		lg->AddEntry(hPrcM[i], str, "L");
	}
	lg->Draw();
	cv->SaveAs("RecAbs.png");
}

void drawRecRat(const char *fname)
{
	const char *hname[NHISTS] = {"hBasic_R", "hXYZ_R", "hPH_R", "hNE_R", "hAH_R", "hR_R", "h1_R"};
	const enum EColor clr[NHISTS] = {kRed, kOrange, kMagenta, kCyan, kBlue, kGreen, kRed - 7};
	const char *htitle[NHISTS] = {"Basic cuts only", "Fiducial volume", "Positron cuts", "Neutron energy",
		"Annihilation gammas", "Distance", "Single hit clusters"};
	TH1D *hPrcM[NHISTS];
	char str[256];
	int i;
	
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	
	for (i=0; i<NHISTS; i++) {
		hPrcM[i] = (TH1D*) f->Get(hname[i]);
		if (!hPrcM[i]) {
			printf("Hist %s not found in %s.\n", hname[i], fname);
			return;
		}
		hPrcM[i]->SetLineColor(clr[i]);
		if (i > 0) hPrcM[i]->Divide(hPrcM[0]);
	}
	hPrcM[1]->SetTitle("Applying cuts one after another");
	hPrcM[1]->GetYaxis()->SetTitle("N/N_{B}");
	hPrcM[1]->GetYaxis()->SetTitleOffset(1);
	hPrcM[1]->GetXaxis()->SetRange(1, 56);
	
	TCanvas *cv = new TCanvas("CV5", "CV5", 1300, 1000);
	hPrcM[1]->Draw("hist");
	for (i=2; i<NHISTS; i++) hPrcM[i]->Draw("hist,same");
	TLegend *lg = new TLegend(0.4, 0.15, 0.85, 0.55);
	for (i=1; i<NHISTS; i++) lg->AddEntry(hPrcM[i], htitle[i], "L");
	lg->Draw();
	cv->SaveAs("RecRat.png");
}

void drawRecInc(const char *fname)
{
	const char *hname[NHISTS] = {"hBasic_R", "hXYZ_R", "hPH_R", "hNE_R", "hAH_R", "hR_R", "h1_R"};
	const enum EColor clr[NHISTS] = {kRed, kOrange, kMagenta, kCyan, kBlue, kGreen, kRed - 7};
	const char *htitle[NHISTS] = {"Basic cuts only", "Fiducial volume", "Positron cuts", "Neutron energy",
		"Annihilation gammas", "Distance", "Single hit clusters"};
	TH1D *hPrcM[NHISTS];
	char str[256];
	int i;
	
	gStyle->SetOptStat(0);
	TFile *f = new TFile(fname);
	if (!f->IsOpen()) return;
	
	for (i=0; i<NHISTS; i++) {
		hPrcM[i] = (TH1D*) f->Get(hname[i]);
		if (!hPrcM[i]) {
			printf("Hist %s not found in %s.\n", hname[i], fname);
			return;
		}
		hPrcM[i]->SetLineColor(clr[i]);
		hPrcM[i]->SetLineWidth(2);
	}
	for (i=NHISTS-1; i>0; i--) hPrcM[i]->Divide(hPrcM[i-1]);

	hPrcM[1]->SetTitle("Cut to previous cut");
	hPrcM[1]->GetYaxis()->SetTitle("N/N_{PREV}");
	hPrcM[1]->GetYaxis()->SetTitleOffset(1);
	hPrcM[1]->GetXaxis()->SetRange(1, 56);
	hPrcM[1]->SetMinimum(0);
	hPrcM[1]->SetMaximum(1);
	
	TCanvas *cv = new TCanvas("CV6", "CV6", 1300, 1000);
	hPrcM[1]->Draw("hist");
	for (i=2; i<NHISTS; i++) hPrcM[i]->Draw("hist,same");
	TLegend *lg = new TLegend(0.4, 0.15, 0.85, 0.55);
	for (i=1; i<NHISTS; i++) lg->AddEntry(hPrcM[i], htitle[i], "L");
	lg->Draw();
	cv->SaveAs("RecInc.png");
}

void drawAll(const char *fname)
{
	drawAbsEff(fname);
	drawRatios(fname);
	drawIncrements(fname);
	drawRecovered(fname);
	drawRecRat(fname);
	drawRecInc(fname);
}
