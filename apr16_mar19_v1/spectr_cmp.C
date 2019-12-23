TCanvas *cv;

void draw_spcmp(const char *hname, const char *nameA, const char *nameB)
{
	char str[1024];
	double val, err;
	
	sprintf(str, "apr16_mar19_%s-calc.root", nameA);
	TFile *fA = new TFile(str);
	sprintf(str, "apr16_mar19_%s-calc.root", nameB);
	TFile *fB = new TFile(str);
	if (!fA->IsOpen() || !fB->IsOpen()) return;
	TH1 *hA = (TH1 *) fA->Get(hname);
	TH1 *hB = (TH1 *) fB->Get(hname);
	if (!hA || !hB) return;
	
	hA->GetXaxis()->SetRange(hA->FindBin(1.001), hA->FindBin(7.999));
	hA->SetTitleSize(0.1);
	hA->GetXaxis()->SetLabelSize(0.05);
	hA->GetXaxis()->SetTitleSize(0.05);
	hA->GetYaxis()->SetLabelSize(0.05);
	hA->GetYaxis()->SetTitleSize(0.05);
	hA->GetYaxis()->SetTitleOffset(1.2);
	hB->GetXaxis()->SetRange(hB->FindBin(1.001), hB->FindBin(7.999));
	hB->SetTitleSize(0.1);
	hB->GetXaxis()->SetLabelSize(0.05);
	hB->GetXaxis()->SetTitleSize(0.05);
	hB->GetYaxis()->SetLabelSize(0.05);
	hB->GetYaxis()->SetTitleSize(0.05);
	hB->GetYaxis()->SetTitleOffset(1.2);
	TH1 *hD = (TH1 *) hA->Clone("Difference");
	TH1 *hR = (TH1 *) hA->Clone("Ratio");
	hD->Add(hA, hB, 1, -1);
	hR->Divide(hA, hB);
	
	hA->SetLineColor(kRed);
	hB->SetLineColor(kBlue);
	hD->SetTitle("Difference");
	hR->SetTitle("Ratio;Positron energy, MeV;");
	
	cv->Clear();
	cv->Divide(2, 1);
	cv->cd(1);
	if (hA->GetMaximum() > hB->GetMaximum()) {
		hA->DrawCopy();
		hB->DrawCopy("same");
	} else {
		hB->DrawCopy();
		hA->DrawCopy("same");
	}
	TLegend *lg = new TLegend(0.35, 0.8, 0.99, 0.93);
	lg->SetFillColor(kWhite);
	val = hA->IntegralAndError(hA->FindBin(1.001), hA->FindBin(7.999), err);
	sprintf(str, "%s: %6.1f #pm %4.1f /day", nameA, val, err);
	lg->AddEntry(hA, str, "le");
	val = hB->IntegralAndError(hB->FindBin(1.001), hB->FindBin(7.999), err);
	sprintf(str, "%s: %6.1f #pm %4.1f /day", nameB, val, err);
	lg->AddEntry(hB, str, "le");
	lg->Draw();
	TVirtualPad *pd = cv->cd(2);
	pd->Divide(1, 2);
	pd->cd(1);
	hD->DrawCopy();
	pd->cd(2);
	hR->Fit("pol1", "Q", "0");
	hR->DrawCopy();
	cv->Update();
	fA->Close();
	fB->Close();
}

void spectr_cmp(void)
{
	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	gStyle->SetPadLeftMargin(0.13);
	gStyle->SetPadRightMargin(0.03);
	cv = new TCanvas("CV", "CV", 1300, 1000);
	cv->SaveAs("spcmp.pdf[");
	draw_spcmp("hUp_01.04.16_30.09.17", "7.2_base", "7.2_neut");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.2_base", "7.2_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.2_neut", "7.2_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.4_base", "7.4_neut");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.4_base", "7.4_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.4_neut", "7.4_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.2_base", "7.4_base");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.2_gamma", "7.4_base");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.2_base", "7.4_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_01.04.16_30.09.17", "7.2_neut", "7.4_neut");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.2_base", "7.2_neut");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.2_base", "7.2_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.2_neut", "7.2_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.4_base", "7.4_neut");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.4_base", "7.4_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.4_neut", "7.4_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.2_base", "7.4_base");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.2_gamma", "7.4_base");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.2_base", "7.4_gamma");
	cv->SaveAs("spcmp.pdf");

	draw_spcmp("hUp_62", "7.2_neut", "7.4_neut");
	cv->SaveAs("spcmp.pdf");
	
	cv->SaveAs("spcmp.pdf]");
}
