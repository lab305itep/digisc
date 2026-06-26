void cmpwithnoise(int what)
{
	const char *dirFuso = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/Fuso";
	const char *dirNoise = "/home/clusters/rrcmpi/alekseev/igor/root8n7/MC/FusoWithNoise";
	const char *MC[] = {
		"22Na/Full_decay_center_Fuso/mc_22Na_indLY_transcode_rawProc_pedSim_Center1.root",
		"60Co/Center_Fuso/mc_60Co_indLY_transcode_rawProc_pedSim_Center1.root",
		"248Cm/Center_Fuso/mc_248Cm_indLY_transcode_rawProc_pedSim_Center1.root",
		"12B/DB_spectrum_Fuso/mc_12B-DB_indLY_transcode_rawProc_pedSim_DBspectrum1.root"
	};
	TFile *fC;
	TFile *fN;
	TTree *tC;
	TTree *tN;
	TH1D *hC;
	TH1D *hN;
	TPave *pC;
	TPave *pN;
	double height;
	TLegend *lg;
	
	fC = new TFile(Form("%s/%s", dirFuso, MC[what]));
	fN = new TFile(Form("%s/%s", dirNoise, MC[what]));
	if (!fC->IsOpen() || !fN->IsOpen()) return;
	tC = (TTree *) fC->Get("DanssEvent");
	tN = (TTree *) fN->Get("DanssEvent");
	if (!tC || !tN) {
		printf("No trees\n");
		return;
	}
	
	gStyle->SetOptStat(1110);
	gROOT->cd();
	switch (what) {
	case 0:	// 22Na
		hC = new TH1D("h22NaC", "^{22}Na;MeV", 40, 0, 4);
		hN = new TH1D("h22NaN", "^{22}Na;MeV", 40, 0, 4);
		tC->Project(hC->GetName(), "SiPmCleanEnergy");
		tN->Project(hN->GetName(), "SiPmCleanEnergy");
		break;
	case 1:	// 60Co
		hC = new TH1D("h60CoC", "^{60}Co;MeV", 40, 0, 4);
		hN = new TH1D("h60CoN", "^{60}Co;MeV", 40, 0, 4);
		tC->Project(hC->GetName(), "SiPmCleanEnergy");
		tN->Project(hN->GetName(), "SiPmCleanEnergy");
		break;
	case 2:	// 248Cm
		hC = new TH1D("h248CmC", "^{248}Cm;MeV", 50, 0, 10);
		hN = new TH1D("h248CmN", "^{248}Cm;MeV", 50, 0, 10);
		tC->Project(hC->GetName(), "SiPmCleanEnergy", "((globalTime/125) % 1000000) > 2 && ((globalTime/125) % 1000000) < 100");
		tN->Project(hN->GetName(), "SiPmCleanEnergy", "((globalTime/125) % 1000000) > 2 && ((globalTime/125) % 1000000) < 100");
		break;
	case 3:	// 12B
		hC = new TH1D("h12BC", "^{12}B;MeV", 50, 0, 20);
		hN = new TH1D("h12BN", "^{12}B;MeV", 50, 0, 20);
		tC->Project(hC->GetName(), "SiPmCleanEnergy");
		tN->Project(hN->GetName(), "SiPmCleanEnergy");
		break;
	}
	hC->SetLineColor(kBlue);
	hC->SetLineWidth(2);
	hN->SetLineColor(kRed);
	hN->SetLineWidth(2);
	hN->SetMarkerStyle(kFullSquare);
	hN->SetMarkerSize(1.01);
	hN->SetMarkerColor(kRed);
	hC->Draw("hist");
	hN->Draw("pex0,sames");
	gPad->Update();
	pN = (TPave *) hN->FindObject("stats");
	pC = (TPave *) hC->FindObject("stats");
	if (!pN || !pC) {
		printf("pave(s) were not found\n");
		return;
	}
	pN->SetLineColor(kRed);
	pN->SetLineWidth(2);
	pN->Draw();
	pC = (TPave *) hC->FindObject("stats");
	pC->SetLineColor(kBlue);
	pC->SetLineWidth(2);
	height = pC->GetY2NDC() - pC->GetY1NDC();
	pC->SetY2NDC(pN->GetY1NDC() - 0.03);
	pC->SetY1NDC(pN->GetY1NDC() - 0.03 - height);
	pC->Draw();
	lg = new TLegend(0.55, 0.75, 0.75, 0.88);
	lg->AddEntry(hN, "With Noise", "lpe");
	lg->AddEntry(hC, "No Noise", "l");
	lg->Draw();
}
