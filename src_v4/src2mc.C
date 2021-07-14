double chi2Hist(TH1 *hA, TH1* hB, int firstbin, int lastbin)
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

void src2mc(const char *exp, const char *mc, const char *fname = NULL)
{
	int i;
	char str[128];
	TObject *fun;
	const char *expHists[4] = {"hExpC", "hExpSiPMC", "hExpPMTC", "hHitsC"};
	const char *mcHists[4] = {"hMc", "hMcSiPM", "hMcPMT", "hMcHits"};
		
	TFile *fExp = new TFile(exp);
	TFile *fMc = new TFile(mc);
	if (!fExp->IsOpen() || !fMc->IsOpen()) return;
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	
	TH1 *hExp[4];
	TH1 *hMc[4];
	
	for (i=0; i<4; i++) {
		hExp[i] = (TH1 *) fExp->Get(expHists[i]);
		hMc[i] = (TH1 *) fMc->Get(mcHists[i]);
		if (!hExp[i] || !hMc[i]) {
			printf("i=%d\n", i);
			return;
		}
		hExp[i]->SetMarkerColor(kBlue);
		hExp[i]->SetMarkerStyle(kFullCircle);
		hExp[i]->SetLineColor(kBlue);
		hExp[i]->SetMarkerSize(1.2);
		hExp[i]->SetStats(0);
		fun = hExp[i]->FindObject("gaus");
		if (fun) fun->Delete();
		hMc[i]->SetLineColor(kBlack);
		hMc[i]->SetLineWidth(2);
		hMc[i]->SetStats(0);
		hMc[i]->Scale(hExp[i]->Integral() * hMc[i]->GetNbinsX() / (hMc[i]->Integral() * hExp[i]->GetNbinsX()));
	}
	
	TLegend *lg = new TLegend(0.65, 0.75, 0.9, 0.9);
	lg->AddEntry(hExp[3], "Experiment", "pl");
	lg->AddEntry(hMc[3], "Monte Carlo", "l");
	TLatex *txt = new TLatex();
	
	TCanvas *cv = new TCanvas("CV", "Exp versus MC", 1200, 900);
	cv->Divide(2, 2);
	for (i=0; i<4; i++) {
		cv->cd(i+1);
		hMc[i]->Draw("hist");
		hExp[i]->Draw("e,same");
		lg->Draw();
		if (i != 3) {
			sprintf(str, "#chi^{2}/n.d.f.=%5.1f / %d", chi2Hist(hExp[i], hMc[i], 10, 30), 30 - 10);
			txt->DrawLatexNDC(0.5, 0.65, str);
		}
	}
	if (fname) cv->SaveAs(fname);
}
