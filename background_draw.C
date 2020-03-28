#define NHISTS 24
#define THISTS 5

void background_draw(const char *rootname, const char *mcname)
{
	char strs[128];
	char strl[1024];
	const char *titles[] = {
		"gtDiff", "R1",  "R2",  "RZ", "PX", "PY", "PZ",  "NX",   "NY",  "NZ", 
                "NE",     "NH",  "PH",  "AH", "AE", "AM", "AMO", "P2AZ", "AH1", "AE1", 
                "P2AZ1",  "PX1", "PY1", "PZ1"};
        const char *ttitles[] = {"TSHOWER", "TMUON", "TBEFORE", "TAFTER", "TAFTERP"};
	const char *suffix[] = {"A-rand", "A-diff", "B-diff", "C-diff"};
	const Color_t color[4] = {kGreen+2, kBlue, kRed, kOrange};
	const int marker[4] = {kOpenCircle, kFullCircle, kOpenSquare, kOpenStar};
	const float Cut[NHISTS][2] = {
		{2.0, -50.0},  {45.0, -50.0}, {55.0, -50.0},  {-50.0, -50.0}, {4.0, 96.0},
		{4.0, 96.0},   {4.0, 96.0},   {-50.0, -50.0}, {-50.0, -50.0}, {-50.0, -50.0},
		{3.5, 9.5},    {3.0, 20.0},   {1.0, 7.0},     {0.0, 9.0},     {0.0, 1.8},
		{0.8, -50.0},  {0.8, -50.0},  {-50.0, -50.0}, {2.0, 9.0},     {0.2, 1.8},
		{15.0, -50.0}, {4.0, 96.0},   {4.0, 96.0},    {4.0, 96.0}
	};
	TH1D *h[NHISTS][4];
	TH1D *hMC[NHISTS];
	TH1D *hT[THISTS];
	int i, j;
	double hMax;
	int iMax;
	char pdfname[1024];
	char *ptr;
	TLine ln;
	double upTime;
	TH1 *hConst;
	TLatex txt;

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
		cv->SaveAs(pdfname);
	}
	
	TF1 *fExpo = new TF1("fExpo", "expo", 0, 500);
	TF1 *fExpo2 = new TF1("fExpo2", "expo(0)+expo(2)", 0, 500);
	txt.SetTextSize(0.07);
	
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
	
	sprintf(strl, "%s]", pdfname);
	cv->SaveAs(strl);
	delete cv;
	fRoot->Close();
}
