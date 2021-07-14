double chi2Hist(TH1D *hA, TH1D* hB, int firstbin, int lastbin)
{
	double sum;
	int i;
	double val, err2;
	
	sum = 0;
	for (i=firstbin; i<= lastbin; i++)
	{
		val = hA->GetBinContent(i) - hB->GetBinContent(i);
		err2 = hA->GetBinError(i) * hA->GetBinError(i) + hB->GetBinError(i) * hB->GetBinError(i);
		sum += val * val / err2;
	}
	return sum;
}

void draw12B_v2(const char *nameExp, const char *nameMC, const char *outname)
{
	char str[1024];
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	
	TFile *fExp = new TFile(nameExp);
	TFile *fMC = new TFile(nameMC);
	if (!fExp->IsOpen() || !fMC->IsOpen()) return;
	
	TH1D *hExp     = (TH1D *) fExp->Get("hExp12B");
	TH1D *hExpSiPM = (TH1D *) fExp->Get("hExp12BSiPM");
	TH1D *hExpPMT  = (TH1D *) fExp->Get("hExp12BPMT");
	TH1D *hExpT    = (TH1D *) fExp->Get("hExp12BT");
	TH1D *hMC      = (TH1D *) fMC->Get("hMC12B");
	TH1D *hMCSiPM  = (TH1D *) fMC->Get("hMC12BSiPM");
	TH1D *hMCPMT   = (TH1D *) fMC->Get("hMC12BPMT");
	TH1D *hMCT     = (TH1D *) fMC->Get("hMC12BT");

	hMC->Scale(hExp->Integral(16, 36) / hMC->Integral(16, 36));			// 4-9 MeV
	hMCSiPM->Scale(hExpSiPM->Integral(16, 36) / hMCSiPM->Integral(16, 36));
	hMCPMT->Scale(hExpPMT->Integral(16, 36) / hMCPMT->Integral(16, 36));
	hMCT->Scale(hExpT->Integral() / hMCT->Integral());				// 1-100 ms
	
	hExp->SetMarkerStyle(kFullCircle);
	hExp->SetMarkerColor(kRed);
	hExp->SetLineColor(kRed);
	hExpSiPM->SetMarkerStyle(kFullCircle);
	hExpSiPM->SetMarkerColor(kRed);
	hExpSiPM->SetLineColor(kRed);
	hExpPMT->SetMarkerStyle(kFullCircle);
	hExpPMT->SetMarkerColor(kRed);
	hExpPMT->SetLineColor(kRed);
	hExpT->SetMarkerStyle(kFullCircle);
	hExpT->SetMarkerColor(kGreen);
	hExpT->SetLineColor(kGreen);
	hMC->SetLineWidth(2);
	hMC->SetLineColor(kBlue);
	hMCSiPM->SetLineWidth(2);
	hMCSiPM->SetLineColor(kBlue);
	hMCPMT->SetLineWidth(2);
	hMCPMT->SetLineColor(kBlue);
	hMCT->SetLineWidth(2);
	hMCT->SetLineColor(kBlue);
	TLatex *txt = new TLatex();
	
	TCanvas *cv = new TCanvas("CV", "12B", 1600, 900);
	TString oname(outname);
	cv->SaveAs((oname+".pdf[").Data());

	cv->Divide(2, 1);
	cv->cd(1);
	hExp->SetTitle("^{12}B decay, cluster energy, SiPM+PMT;MeV;N");
	hExp->Draw();
	hMC->Draw("hist,same");
	TLegend *lg = new TLegend(0.6, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp, "Experiment", "pe");
	lg->AddEntry(hMC, "Monte-Carlo", "l");
	lg->Draw();
	sprintf(str, "#chi^{2}/n.d.f.=%5.1f / %d", chi2Hist(hExp, hMC, 16, 36), 36 - 16);
	txt->DrawLatexNDC(0.5, 0.65, str);
	cv->cd(2);
	hExpT->SetTitle("^{12}B decay time;ms;N");
	hExpT->Fit("expo");
	hMCT->Draw("hist,same");
	cv->SaveAs((oname+".pdf").Data());

	cv->Clear();
	cv->Divide(2,1);
	cv->cd(1);
	hExpSiPM->SetTitle("^{12}B decay, cluster energy, SiPM;MeV;N");
	hExpSiPM->Draw();
	hMCSiPM->Draw("hist,same");
	lg->Draw();
	sprintf(str, "#chi^{2}/n.d.f.=%5.1f / %d", chi2Hist(hExpSiPM, hMCSiPM, 16, 36), 36 - 16);
	txt->DrawLatexNDC(0.5, 0.65, str);
	cv->cd(2);
	hExpPMT->SetTitle("^{12}B decay, cluster energy, PMT;MeV;N");
	hExpPMT->Draw();
	hMCPMT->Draw("hist,same");
	lg->Draw();
	sprintf(str, "#chi^{2}/n.d.f.=%5.1f / %d", chi2Hist(hExpPMT, hMCPMT, 16, 36), 36 - 16);
	txt->DrawLatexNDC(0.5, 0.65, str);
	cv->SaveAs((oname+".pdf").Data());

	cv->Clear();
	cv->Divide(2, 1);
	hExpSiPM->SetLineColor(kRed);
	hExpPMT->SetLineColor(kBlue);
	hMCSiPM->SetLineColor(kRed);
	hMCPMT->SetLineColor(kBlue);
	hExpSiPM->SetLineWidth(2);
	hExpPMT->SetLineWidth(2);
	hMCSiPM->SetLineWidth(2);
	hMCPMT->SetLineWidth(2);
	hExpPMT->SetTitle("^{12}B - Experiment, SiPM and PMT");
	hMCPMT->SetTitle("^{12}B - MC, SiPM and PMT");
	lg = new TLegend(0.7, 0.85, 0.9, 0.93);
	lg->AddEntry(hExpSiPM, "SiPM", "l");
	lg->AddEntry(hExpPMT, "PMT", "l");
	
	cv->cd(1);
	hExpPMT->Draw("hist");
	hExpSiPM->Draw("hist,same");
	lg->Draw();
	cv->cd(2);
	hMCPMT->Draw("hist");
	hMCSiPM->Draw("hist,same");
	lg->Draw();
	cv->SaveAs((oname+".pdf").Data());

	cv->SaveAs((oname+".pdf]").Data());
}
