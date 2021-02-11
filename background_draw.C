#define NHISTS   31
#define THISTS    5
#define OPTHISTS 14

void background_draw(const char *rootname, const char *mcname, double muFraction = 0.068)
{
	char strs[128];
	char strl[1024];
	const char *titles[] = {
		"gtDiff", "R1",   "R2",   "RZ",   "PX",    "PY",  "PZ",  "NX",  "NY",  "NZ", 
		"NE",     "NE10", "NE20", "NE30", "NE40",  "NH",  "PH",  "AH",  "AE",  "AM", 
		"AMO",    "P2AZ", "AH1",  "AE1",  "P2AZ1", "PX1", "PY1", "PZ1", "PXN", "PYN", 
		"PZN"};
        const char *ttitles[] = {"TSHOWER", "TMUON", "TBEFORE", "TAFTER", "TAFTERP"};
	const char *suffix[] = {"A-rand", "A-diff", "B-diff", "C-diff"};
	const Color_t color[4] = {kGreen+2, kBlue, kRed, kOrange};
	const int marker[4] = {kOpenCircle, kFullCircle, kOpenSquare, kOpenStar};
	const float Cut[NHISTS][2] = {
		{1.0, -50.0}, {-50.0, 40.0},  {-50.0, 48.0},  {-50.0, -50.0}, {4.0, 96.0},
		{4.0, 96.0},  {4.0, 96.0},    {-50.0, -50.0}, {-50.0, -50.0}, {-50.0, -50.0},
		{1.5, 9.5},   {1.5, 9.5},     {1.5, 9.5},     {1.5, 9.5},     {1.5, 9.5},
		{3.0, 20.0},  {1.0, 8.0},     {0.0, 7.0},     {0.0, 1.2},     {-50.0, 0.8},
		{-50.0, 0.8}, {-50.0, -50.0}, {1.0, 7.0},     {0.1, 1.2},     {-50.0, -50.0}, 
		{4.0, 96.0},  {4.0, 96.0},    {4.0, 96.0},    {4.0, 96.0},    {4.0, 96.0}, 
		{4.0, 96.0}};
	const int toOpt[] = {0, 100, 101, 102, 10, 11, 12, 13, 14, 118, 22, 122, 23, 123};
	TH1D *h[NHISTS][4];
	TH1D *hMC[NHISTS];
	TH1D *hT[THISTS];
	TH1D *hOpt[OPTHISTS];
	int i, j, k1, k2, num, ul;
	double hMax, hMin;
	int iMax;
	char pdfname[1024];
	char *ptr;
	TLine ln;
	double upTime;
	TH1 *hConst;
	TLatex txt;
	double N_diff, N_rand, N_cosmA, N_cosmB, err;

	strcpy(pdfname, rootname);
	ptr = strstr(pdfname, ".root");
	if (ptr) {
		strcpy(ptr, ".pdf");
	} else {
		strcat(pdfname, ".pdf");
	}
	
	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	
	gStyle->SetTitleXSize(0.05);
	gStyle->SetTitleYSize(0.05);
	gStyle->SetLabelSize(0.05);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadBottomMargin(0.16);
	
	ln.SetLineWidth(3);
	ln.SetLineColor(kBlue);
	
	TFile *fRoot = new TFile(rootname);
	if (!fRoot->IsOpen()) {
		printf("root-file %s not found  - run background_calc() first!\n", rootname);
		return;
	}
	hConst = (TH1 *) fRoot->Get("hConst");
	if (!hConst) {
		printf("Strange root-file: %s - hConst not found - run background_calc() first!\n", rootname);
		delete fRoot;
		return;
	}
	i = hConst->GetXaxis()->FindBin("gTime");
	if (i <= 0) {
		printf("Strange root-file: %s - gTime not found in hConst - run background_calc() first!\n", rootname);
		delete fRoot;
		return;
	}
	upTime = hConst->GetBinContent(i);
	
	for (i=0; i<NHISTS; i++) {
		for (j=0; j<4; j++) {
			sprintf(strs, "h%s%s", titles[i], suffix[j]);
			h[i][j] = (TH1D*) fRoot->Get(strs);
			if (!h[i][j]) {
				printf("%s not found  - rerun background_calc() to create all hists!\n", strs);
				fRoot->Close();
				return;
			}
			h[i][j]->Scale(24*3600.0/upTime);
			h[i][j]->GetYaxis()->SetTitle("Evt/day/bin");
			h[i][j]->GetYaxis()->SetLabelSize(0.05);
			h[i][j]->SetLineWidth(2);
			h[i][j]->SetLineColor(color[j]);
			h[i][j]->SetMarkerColor(color[j]);
			h[i][j]->SetMarkerStyle(marker[j]);
			h[i][j]->SetMinimum(0.0);
		}
	}

	memset(hMC, 0, sizeof(hMC));
	if (mcname) {
		TFile *fMC = new TFile(mcname);
		if (fMC->IsOpen()) {
			for (i=0; i<NHISTS; i++) {
				sprintf(strs, "h%s_MC", titles[i]);
				hMC[i] = (TH1D*) fMC->Get(strs);
				if (!hMC[i]) continue;
				hMC[i]->Scale(h[i][1]->Integral() / hMC[i]->Integral());
				hMC[i]->SetLineColor(kBlack);
			}
		}
	}

	for (i=0; i<THISTS; i++) {
		sprintf(strs, "%s-sig", ttitles[i]);
		hT[i] = (TH1D*) fRoot->Get(strs);
		if (!hT[i]) {
			printf("%s not found - rerun background_calc() to create all hists!\n", strs);
			fRoot->Close();
			return;
		}
		hT[i]->Scale(24*3600.0/upTime);
		hT[i]->GetYaxis()->SetTitle("Evt/day/bin");
		hT[i]->GetYaxis()->SetLabelSize(0.05);
		hT[i]->SetLineWidth(2);
		hT[i]->SetLineColor(kBlue);
		hT[i]->SetMarkerColor(kBlue);
		hT[i]->SetMarkerStyle(kFullCircle);
		hT[i]->SetMinimum(0.0);
	}

	TCanvas *cv = new TCanvas("CV", "Background plots", 1200, 900);
	TLegend *lg = new TLegend(0.7, 0.8, 0.95, 0.95);
	lg->AddEntry(h[0][0], "Random", "LP");
	lg->AddEntry(h[0][1], "Neutrino", "LP");
	lg->AddEntry(h[0][2], "Cosmic-A", "LP");
	lg->AddEntry(h[0][3], "Cosmic-B", "LP");
	if (hMC[0]) lg->AddEntry(hMC[0], "Monte-Carlo", "L");
	lg->AddEntry(&ln, "Cut(s)", "L");
	
	sprintf(strl, "%s[", pdfname);
	cv->SaveAs(strl);
	
	txt.SetTextSize(0.05);
	for (i=0; i<NHISTS; i++) {
		cv->Clear();
		hMax = 0;
		iMax = 0;
		for (j=0; j<3; j++) if (h[i][j]->GetMaximum() > hMax) {
			hMax = h[i][j]->GetMaximum();
			iMax = j;
		}
		h[i][iMax]->Draw();
		for (j=0; j<4; j++) if (j != iMax) h[i][j]->Draw("same");
		if (hMC[i]) hMC[i]->Draw("same,hist");
		for (j=0; j<2; j++) if (Cut[i][j] > -1.0) ln.DrawLine(Cut[i][j], 0, Cut[i][j], hMax);
		lg->Draw();
		k1 = (Cut[i][0] > -1.0) ? h[i][0]->GetXaxis()->FindBin(Cut[i][0] + 0.00001) : 1;
		k2 = (Cut[i][1] > -1.0) ? h[i][0]->GetXaxis()->FindBin(Cut[i][1] - 0.00001) : h[i][0]->GetXaxis()->GetNbins();
		for (j=0; j<4; j++) {
			txt.SetTextColor(color[j]);
			sprintf(strl, "[ %5.1f %%]", 100.0 * h[i][j]->Integral(k1, k2) / h[i][j]->Integral());
			txt.DrawLatexNDC(0.5, 0.65 - 0.05*j, strl);
		}
		if (hMC[i]) {
			txt.SetTextColor(kBlack);
			sprintf(strl, "[ %5.1f %%]", 100.0 * hMC[i]->Integral(k1, k2) / hMC[i]->Integral());
			txt.DrawLatexNDC(0.5, 0.45, strl);
		}
		txt.SetTextColor(kBlue);
		err = sqrt(h[i][0]->Integral(k1, k2) + h[i][1]->Integral(k1, k2) + muFraction * (h[i][2]->Integral(k1, k2) + h[i][3]->Integral(k1, k2)));
		err *= 100.0 / h[i][1]->Integral(k1, k2);
		sprintf(strl, "#sigma/N_{cut}=%4.1f%%", err);
		txt.DrawLatexNDC(0.5, 0.4, strl);
		err = sqrt(h[i][0]->Integral() + h[i][1]->Integral() + muFraction * (h[i][2]->Integral() + h[i][3]->Integral()));
		err *= 100.0 / h[i][1]->Integral();
		sprintf(strl, "#sigma/N_{nocut}=%4.1f%%", err);
		txt.DrawLatexNDC(0.5, 0.35, strl);
		cv->SaveAs(pdfname);
	}
	
	for (i=0; i<OPTHISTS; i++) {
		num = toOpt[i] % 100;
		ul  = toOpt[i] / 100;
		sprintf(strs, "hOpt_%s%c", titles[num], (ul) ? 'U' : 'L');
		hOpt[i] = (TH1D*) h[num][1]->Clone(strs);
		sprintf(strl, "%s - %s cut;%s;%%", h[num][1]->GetTitle(), (ul) ? "upper" : "lower",
			h[num][1]->GetXaxis()->GetTitle());
		hOpt[i]->SetTitle(strl);
		hOpt[i]->Reset();
		hOpt[i]->SetLineColor(kBlack);
		k1 = (Cut[num][0] > -1.0) ? hOpt[i]->GetXaxis()->FindBin(Cut[num][0] + 0.00001) : 1;
		k2 = (Cut[num][1] > -1.0) ? hOpt[i]->GetXaxis()->FindBin(Cut[num][1] - 0.00001) : hOpt[i]->GetXaxis()->GetNbins();
		hMin = 10000.0;
		for (j=1; j<=hOpt[i]->GetXaxis()->GetNbins(); j++) {
			if (ul) {	// Upper cut
				if (j < k1) continue;
				N_rand  = h[num][0]->Integral(k1, j);
				N_diff  = h[num][1]->Integral(k1, j);
				N_cosmA = h[num][2]->Integral(k1, j);
				N_cosmB = h[num][3]->Integral(k1, j);
			} else {		// Lower cut
				if (j > k2) continue;
				N_rand  = h[num][0]->Integral(j, k2);
				N_diff  = h[num][1]->Integral(j, k2);
				N_cosmA = h[num][2]->Integral(j, k2);
				N_cosmB = h[num][3]->Integral(j, k2);
			}
			if (N_diff <= 0) continue;
			err = 100.0 * sqrt(N_rand + N_diff + muFraction * (N_cosmA + N_cosmB)) / N_diff;
			hOpt[i]->SetBinContent(j, err);
			if (err < hMin) hMin = err;
		}
		hOpt[i]->SetMinimum(0.9*hMin);
		hOpt[i]->SetMaximum(5*hMin);
		hOpt[i]->Draw();
		for (j=0; j<2; j++) if (Cut[num][j] > -1.0) ln.DrawLine(Cut[num][j], hOpt[i]->GetMinimum(), Cut[num][j], hOpt[i]->GetMaximum());
		cv->SaveAs(pdfname);
	}
	
	TF1 *fExpo = new TF1("fExpo", "expo", 0, 500);
	TF1 *fExpo2 = new TF1("fExpo2", "expo(0)+expo(2)", 0, 500);
	txt.SetTextSize(0.07);
//		Draw time hists
	for (i=0; i<THISTS; i++) {
		switch(i) {
		case 0:	// TSHOWER
			hT[i]->Fit("fExpo", "Q", "", 65, 200);
			sprintf(strl, "#tau = %5.1f us", -1.0/fExpo->GetParameter(1));
			txt.DrawLatexNDC(0.65, 0.7, strl);
			break;
		case 1:	// TMUON
			hMax = hT[i]->GetMaximum();
			fExpo2->SetParameters(TMath::Log(hMax), -1/12., TMath::Log(hMax/10.), -1./1000);
			hT[i]->Fit("fExpo2", "Q", "", 15, 200);
			sprintf(strl, "#tau = %5.1f us", -1.0/fExpo2->GetParameter(1));
			txt.DrawLatexNDC(0.65, 0.7, strl);
			break;
		case 2:	// TBEFORE
			hT[i]->Fit("fExpo2", "Q", "", 10, 200);
			sprintf(strl, "#tau = %5.1f us", -1.0/fExpo2->GetParameter(1));
			txt.DrawLatexNDC(0.65, 0.7, strl);
			break;
		case 3:	// TAFTER
			hT[i]->Fit("fExpo2", "Q", "", 5, 200);
			sprintf(strl, "#tau = %5.1f us", -1.0/fExpo2->GetParameter(1));
			txt.DrawLatexNDC(0.65, 0.7, strl);
			break;
		case 4:	// TAFTERP
			hT[i]->Fit("fExpo2", "Q", "", 50, 200);
			sprintf(strl, "#tau = %5.1f us", -1.0/fExpo2->GetParameter(1));
			txt.DrawLatexNDC(0.65, 0.7, strl);
			break;
		}
		txt.DrawLatexNDC(0.65, 0.7, strl);
		cv->SaveAs(pdfname);
	}
//		Draw moderation and capture
	TF1 *fExpoGt = new TF1("fExpoGt", "expo", 0, 50);
	fExpoGt->SetLineColor(kRed);
	TF1 *fExpoMC = new TF1("fExpoMC", "expo", 0, 50);
	txt.SetTextSize(0.04);

	h[0][1]->Fit("fExpoGt", "Q", "", 16, 50);
	cv->Update();
	TPaveStats *pv = (TPaveStats *) h[0][1]->FindObject("stats");
	double y1 = pv->GetY1NDC();
	double y2 = pv->GetY2NDC();
	pv->SetY1NDC(2 * y1 - y2);
	pv->SetY2NDC(y1);
	pv->SetLineColor(kBlue);
	pv->SetTextColor(kBlue);
	sprintf(strl, "#tau_{EXP} = %5.2f us", -1.0/fExpoGt->GetParameter(1));
	txt.SetTextColor(kBlue);
	txt.DrawLatexNDC(0.7, 0.5, strl);
	hMC[0]->Fit("fExpoMC", "Q", "hist,sames", 16, 50);
	sprintf(strl, "#tau_{MC} = %5.2f us", -1.0/fExpoMC->GetParameter(1));
	txt.SetTextColor(kBlack);
	txt.DrawLatexNDC(0.7, 0.43, strl);
	cv->Update();
	cv->SaveAs(pdfname);
	
	sprintf(strl, "%s]", pdfname);
	cv->SaveAs(strl);
	delete cv;
	fRoot->Close();
}
