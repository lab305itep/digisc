draw_spcmp(const char *hname)
{
	int i, j;
	const char *fname[4][3] = {
		{"apr16_jan19_old-calc.root",   "apr16_jan19_old_SiPm-calc.root",   "apr16_jan19_old_Pmt-calc.root"},
		{"apr16_jan19_nobug-calc.root", "apr16_jan19_nobug_SiPm-calc.root", "apr16_jan19_nobug_Pmt-calc.root"},
		{"apr16_jan19_1hit-calc.root",  "apr16_jan19_1hit_SiPm-calc.root",  "apr16_jan19_1hit_Pmt-calc.root"},
		{"apr16_jan19_base-calc.root",  "apr16_jan19_base_SiPm-calc.root",  "apr16_jan19_base_Pmt-calc.root"}
	};
		const char *scname[5] = {
		"apr16_jan19_base_1.00-calc.root", 
		"apr16_jan19_base_0.98-calc.root", 
		"apr16_jan19_base_0.96-calc.root", 
		"apr16_jan19_base_0.94-calc.root", 
		"apr16_jan19_base_0.92-calc.root", 
	};
	const char *plb_name = "danss_report_v4_sep17-calc.root";
	const enum EColor color[6] = {kRed, kGreen, kBlue, kMagenta, kOrange};
	const enum EMarkerStyle marker[4] = {kFullCircle, kFullSquare, kFullTriangleUp, kFullDiamond};
	const char *suffix[3] = {"", ", SiPm", ", Pmt"};
	
	TFile *f[4][3];
	TFile *fSc[5];
	TH1 *h[4][3];
	TH1 *hSc[5];
	TH1 *hr[3][3];
	TH1 *hPlb = NULL;
	TH1D *hPlb0 = NULL;
	TH1 *hRebin[4];
	TH1 *hScRebin[5];
	
	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	
	TFile *fPlb = new TFile(plb_name);
	if (fPlb->IsOpen()) switch (hname[1]) {
	case 'U':
		hPlb = (TH1 *) fPlb->Get("hUp_30");
		break;
	case 'M':
		hPlb = (TH1 *) fPlb->Get("hMid_30");
		break;
	case 'D':
		hPlb = (TH1 *) fPlb->Get("hDown_30");
		break;
	case 'S':
		hPlb = (TH1 *) fPlb->Get("hSum");
		break;
	}
	if (hPlb) {
		hPlb->SetLineColor(kBlack);
		hPlb->SetLineWidth(2);
		hPlb->SetMarkerColor(kBlack);
		hPlb->SetMarkerStyle(kOpenCircle);
		hPlb0 = new TH1D("hPlb0", "PLB with 4 extra bins", 64, 0, 16);
		for (i=1; i<=60; i++) {
			hPlb0->SetBinContent(i+4, hPlb->GetBinContent(i));
			hPlb0->SetBinError(i+4, hPlb->GetBinError(i));
		}
		hPlb->Scale(0.5);	// to be compatible with new binning
	}
	
	for (i=0; i<4; i++) for (j=0; j<3; j++) {
		f[i][j] = new TFile(fname[i][j]);
		if (!f[i][j]->IsOpen()) break;
		h[i][j] = (TH1*) f[i][j]->Get(hname);
		if (!h[i][j]) break;
		h[i][j]->SetLineColor(color[i]);
		h[i][j]->SetLineWidth(2);
		h[i][j]->SetMarkerColor(color[i]);
		h[i][j]->SetMarkerStyle(marker[i]);
	}
	if (i != 4 || j != 3) return;

	for (i=0; i<5; i++) {
		fSc[i] = new TFile(scname[i]);
		if (!fSc[i]->IsOpen()) break;
		hSc[i] = (TH1*) fSc[i]->Get(hname);
		if (!hSc[i]) break;
		hSc[i]->SetLineColor(color[i]);
	}
	if (i != 5) return;

	for (i=0; i<3; i++) for (j=0; j<3; j++) {
		hr[i][j] = (TH1 *) h[i][j]->Clone((TString(h[i][j]->GetName()) + "r").Data());
		hr[i][j]->SetTitle((TString(h[i][j]->GetTitle()) + suffix[j]).Data());
		hr[i][j]->Divide(h[3][j]);
	}

	TLegend lg(0.65, 0.7, 0.95, 0.9);
	lg.AddEntry(h[0][0], "PLB cuts", "lp");
	lg.AddEntry(h[1][0], "New cuts, buggy isolation cut", "lp");
	lg.AddEntry(h[2][0], "New cuts without 1hit cut", "lp");
	lg.AddEntry(h[3][0], "All new cuts", "lp");

	TLegend lg0(0.65, 0.65, 0.95, 0.9);
	lg0.AddEntry(h[0][0], "PLB cuts", "lp");
	lg0.AddEntry(h[1][0], "New cuts, buggy isolation cut", "lp");
	lg0.AddEntry(h[2][0], "New cuts without 1hit cut", "lp");
	lg0.AddEntry(h[3][0], "All new cuts", "lp");
	if (hPlb) lg0.AddEntry(hPlb, "PLB data", "lp");

	TLegend lg1(0.65, 0.75, 0.95, 0.9);
	lg1.AddEntry(hr[0][0], "PLB cuts to New", "lp");
	lg1.AddEntry(hr[1][0], "New cuts, buggy isolation cut to New", "lp");
	lg1.AddEntry(hr[2][0], "New cuts without 1hit cut to New", "lp");

	TLegend lgSc(0.65, 0.65, 0.95, 0.9);
	lgSc.AddEntry(hSc[0], "New calibration", "l");
	lgSc.AddEntry(hSc[1], "New calibration * 0.98", "l");
	lgSc.AddEntry(hSc[2], "New calibration * 0.96", "l");
	lgSc.AddEntry(hSc[3], "New calibration * 0.94", "l");
	lgSc.AddEntry(hSc[4], "New calibration * 0.92", "l");
	if (hPlb) lgSc.AddEntry(hPlb, "PLB data", "l");

	TLegend lgSc0(0.65, 0.65, 0.95, 0.9);
	lgSc0.AddEntry(hSc[0], "New calibration", "l");
	lgSc0.AddEntry(hSc[1], "New calibration * 0.98", "l");
	lgSc0.AddEntry(hSc[2], "New calibration * 0.96", "l");
	lgSc0.AddEntry(hSc[3], "New calibration * 0.94", "l");
	lgSc0.AddEntry(hSc[4], "New calibration * 0.92", "l");

	TCanvas *cv = new TCanvas("CV", "CV", 1200, 800);
	TString str(hname);
	str += ".pdf";
	cv->SaveAs((str + "[").Data());
	
	for (j=0; j<3; j++) {
		cv->Clear();
		h[0][j]->GetXaxis()->SetRange(9, 80);
		h[0][j]->SetTitle((TString(h[0][j]->GetTitle()) + suffix[j]).Data());
		for (i=0; i<4; i++) h[i][j]->Draw((i) ? "e,same" : "e");
		if (j == 0 && hPlb) {
			hPlb->Draw("e,same");
			lg0.Draw();
		} else {
			lg.Draw();
		}
		cv->Update();
		cv->SaveAs(str.Data());
		
		cv->Clear();
		hr[0][j]->GetXaxis()->SetRange(9, 80);
		hr[0][j]->SetMinimum(0.9);
		hr[0][j]->SetMaximum(1.5);
		for (i=0; i<3; i++) hr[i][j]->Draw((i) ? "e,same" : "e");
		lg1.Draw();
		cv->Update();
		cv->SaveAs(str.Data());
	}
	
	if (hPlb) {
		cv->Clear();
		for (i=0; i<4; i++) {
			hRebin[i] = h[i][0]->Rebin();
			hRebin[i]->Divide(hPlb0);
		}
		hRebin[0]->SetTitle("Ratio to PLB");
		hRebin[0]->SetMinimum(0.7);
		hRebin[0]->SetMaximum(1.6);
		hRebin[0]->GetXaxis()->SetRange(5, 40);
		for (i=0; i<4; i++) hRebin[i]->Draw((i) ? "e,same" : "e");
		lg.Draw();
		cv->Update();
		cv->SaveAs(str.Data());
	}
	
	cv->Clear();
	hSc[0]->GetXaxis()->SetRange(9, 64);
	hSc[0]->SetMaximum(1.25 * hSc[0]->GetMaximum());
	for(i=0; i<5; i++) hSc[i]->Draw((i) ? "hist,same" : "hist");
	if (hPlb) {
		hPlb->SetLineWidth(1);
		hPlb->Draw("hist,same");
	}
	lgSc.Draw();
	cv->Update();
	cv->SaveAs(str.Data());
	
	if (hPlb) {
		cv->Clear();
		for (i=0; i<5; i++) {
			hScRebin[i] = hSc[i]->Rebin();
			hScRebin[i]->Divide(hPlb0);
		}
		hScRebin[0]->SetTitle("Ratio to PLB");
		hScRebin[0]->SetMinimum(0.7);
		hScRebin[0]->SetMaximum(1.6);
		hScRebin[0]->GetXaxis()->SetRange(5, 32);
		for (i=0; i<5; i++) hScRebin[i]->Draw((i) ? "hist,same" : "hist");
		lgSc0.Draw();
		cv->Update();
		cv->SaveAs(str.Data());
	}
	
	cv->SaveAs((str + "]").Data());
	for (i=0; i<4; i++) for (j=0; j<3; j++) f[i][j]->Close();
	fPlb->Close();
}
