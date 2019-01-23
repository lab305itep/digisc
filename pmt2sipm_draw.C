void pmt2sipm_draw(const char *name)
{
	int i;
	char str[1024];
	char strs[128];
	TObjArray array;

	gStyle->SetOptFit(1);
	sprintf(str, "%s.root", name);
	TFile *f = new TFile(str);
	if (!f->IsOpen()) return;
	TH2D *h = f->Get("hPmt2Sipm-diff");
	if (!h) return;
	TF1 *fGaus = new TF1("fGaus", "gaus", -1, 1);
	h->FitSlicesY(fGaus, 0, -1, 0, "QN", &array);
	TH1 *hpos = (TH1 *) array[1]->Clone();
	hpos->SetLineWidth(4);
	TCanvas *cv = new TCanvas("CV", "CV", 1200, 900);
	h->SetStats(0);
	hpos->SetStats(0);
	h->Draw("colorz");
	hpos->Draw("same");
	cv->Update();
	sprintf(str, "%s.pdf[", name);
	cv->SaveAs(str);
	sprintf(str, "%s.pdf", name);
	cv->SaveAs(str);
	cv->Clear();
	hpos->SetTitle("Gaus fit mean; (Sipm+Pmt)/2, MeV;2*(Sipm-Pmt)/(Sipm+Pmt)");
	hpos->Draw();
	cv->SaveAs(str);
	for (i = 0; i < h->GetNbinsX(); i++) {
		sprintf(strs, "E=%4.2f-%4.2f", 1 + 0.25*i, 1.25+0.25*i);
		h->ProjectionY(strs, i+1, i+1)->Fit("gaus");
		cv->SaveAs(str);
	}
	sprintf(str, "%s.pdf]", name);
	cv->SaveAs(str);
	f->Close();
}
