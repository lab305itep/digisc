//	Compare MC to IBD spectrum
#define BASEDIR "/home/clusters/rrcmpi/alekseev/igor/pair7n15/MC/DataTakingPeriod01/"

TChain *make_MCchain(int isotope)
{
	TChain *ch = new TChain("DanssPair");
	
	switch (isotope) {
	case 235:
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_00.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_01.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_02.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_03.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_04.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_05.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_06.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_07.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_08.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_09.root");
		break;
	case 238:
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_00.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_01.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_02.root");
		break;
	case 239:
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_00.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_01.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_02.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_03.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_04.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_05.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_06.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_07.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_08.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_09.root");
		break;
	case 241:
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_00.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_01.root");
		ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_02.root");
		break;
	default:
		delete ch;
		return NULL;
	}
	return ch;
}

void exp2mc(const char *file_exp, const char *name_exp = "hSum_Main", double fPu239 = 34.0, 
	double kScale = 1.00, double kShift = 0, const char *cAux = NULL, const char *sAux = NULL)
{
	char strs[128], strl[1024];
	const int a_fuel[4] = {235, 238, 239, 241};
	const char *name_fuel[4] = {"235U", "238U", "239Pu", "241Pu"};
	double frac_fuel[4];// = {53, 7.3, 34, 5.7};		// some average for Apr16-Jan 19. 
	const double cross_fuel[4] = {6.69, 10.10, 4.36, 6.04};
	const int nfiles[4]={11, 4, 11, 4};
	const int color[4] = {kRed, kBlue, kGreen, kOrange};
	int k1, k2, k3, k4;
	TH1D *hExp;
	TH1D *hFuel[4];
	TH1D *hMC;
	TFile *fExp;
	TChain *tFuel[4];
	int i;
	
	//  RENO 2200 days, near detector: arXiv:1806.00248
	// prompt energy
	const double renoeShift = 1.022;    // Mev, subtract to convert to positron energy
	double renoep[] = {1.7,  1.9,  2.1,  2.3,  2.5,  2.7,  2.9,  3.1,  3.3,  3.5,  3.7,	// Point at 1.7 added by Lex
				 3.9,  4.1,  4.3,  4.5,  4.7,  4.9,  5.1,  5.3,  5.5,  5.8, 
				 6.2,  6.7,  7.7};
	double renoepe[] = {0.1, 0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.1, 0.1,
				 0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.2, 
				 0.2,  0.3,  0.7};
	                    // (Exp-MC)/MC in pixels
	const double renoconv = 2970.;    // pixels per unity
	double renobump[] = {-7.,  -53., -28., -35., -38., -10., -11.,  14.,  51., 32.,  11., 
				   -15.,  88., 167., 278., 352., 404., 428., 385., 305., 292., 
				   230.,  22., -120.};
	double renoebump[] = {25.,  25.,  25.,  25.,  25.,  25.,  25.,  25.,  25.,  25.,  25.,  
				    25.,  25.,  25.,  25.,  25.,  25.,  25.,  25.,  25.,  25., 
				    25.,  35.,   83.};
	                                
	for (i=0; i<sizeof(renoep)/sizeof(renoep[0]); i++) {
		renoep[i] -= renoeShift;
		renobump[i] /= renoconv;
		renoebump[i] /= renoconv;
		renobump[i] += 1.0;
	}
	TGraphErrors * gReno;
	gReno = new TGraphErrors(sizeof(renoep)/sizeof(renoep[0]), renoep, renobump, renoepe, renoebump);
	gReno->SetLineColor(kBlack);
	gReno->SetMarkerColor(kBlack);
	gReno->SetMarkerStyle(kFullSquare);
	gReno->SetMarkerSize(1.5);
	// Last points repeated to get regular grid, errors multiplied by sqrt(n)
	double reno[34] = {
		  -7.,  -53.,  -28.,  -35., -38., -10., -11.,   14.,   51.,   32.,  
		  11.,  -15.,   88.,  167., 278., 352., 404.,  428.,  385.,  305., 
		 292.,  292.,  230.,  230.,  22.,  22.,  22., -120., -120., -120., 
		-120., -120., -120., -120.};
	double renoe[34] = {
		  25.,   25.,   25.,   25.,  25.,  25.,  25.,   25.,   25.,   25.,  
		  25.,   25.,   25.,   25.,  25.,  25.,  25.,   25.,   25.,   25.,  
		  35.,   35.,   35.,   35.,  60.,  60.,  60.,  220.,  220.,  220., 
		 220.,  220.,  220.,  220.};

	TH1D *hReno = new TH1D("hReno", "Reno with data smoothed with DANSS energy resolution", 34, renoep[0] - 0.1, 6.7 + renoep[0]);
	for (i=0; i<34; i++) {
		reno[i] /= renoconv;
		renoe[i] /= renoconv;
		hReno->SetBinContent(i+1, 1);
		hReno->SetBinError(i+1, renoe[i]);
	}

        TF1 *fGaus = new TF1("fGaus", "gaus", -20.0, 20.0);
        for (i=0; i<34; i++) {
		fGaus->SetParameter(0, 1);      // amplitude
		fGaus->SetParameter(1, hReno->GetXaxis()->GetBinCenter(i+1)); // position
		fGaus->SetParameter(2, 0.34*sqrt(hReno->GetXaxis()->GetBinCenter(i+1)));      // DANSS resolution: 34%/sqrt(E)
		hReno->Add(fGaus, reno[i]*0.2/fGaus->Integral(-20.0, 20.0));
	}

	frac_fuel[0] = 104.8 - 1.55 * fPu239;
	frac_fuel[2] = fPu239;
	frac_fuel[3] = 0.463 * fPu239 - 9.5;
	frac_fuel[1] = 100.0 - frac_fuel[0] - frac_fuel[2] - frac_fuel[3];
	
	TCut cVeto("gtFromVeto > 60");
	TCut cIso("EventsBetween == 0");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 1");
	TCut cGamma("AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cPh("PositronHits < 8");
	TCut cR2("Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR3("Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR = cR3 && (cRXY || cR2);
	TCut cNH("NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cNE("NeutronEnergy > 1.5 + 3 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cN = cNH && cNE;
        TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))");

	TCut cSel = cX && cY && cZ && c20 && cPh && cR && cN && cSingle;
	if (cAux) cSel = cSel && TCut(cAux);
	
	gStyle->SetOptStat(0);
	sprintf(strl, "ibd_v3_239Pu_%4.1f_%s_scale_%5.3f_%5.3f_%s", fPu239, name_exp, kScale, kShift, (sAux) ? sAux : "");
	TString pdfName(strl);
	
//	printf("file: %s\n", file_exp);
	fExp = new TFile(file_exp);
//	printf("ff\n");
	if (!fExp->IsOpen()) return;
	hExp = (TH1D*) fExp->Get(name_exp);
	if (!hExp) return;
	hExp->GetXaxis()->SetLabelSize(0.04);
	hExp->GetXaxis()->SetTitleSize(0.04);
	hExp->GetXaxis()->SetTitleOffset(1.0);
	hExp->GetYaxis()->SetLabelSize(0.04);
	hExp->GetYaxis()->SetTitleSize(0.04);
	hExp->GetYaxis()->SetTitleOffset(1.2);
	
	for (i=0; i<4; i++) {
		tFuel[i] = make_MCchain(a_fuel[i]);
		if (!tFuel[i]) return;
		sprintf(strs, "hMC%s", name_fuel[i]);
		hFuel[i] = (TH1D*) hExp->Clone(strs);
		hFuel[i]->Reset();
		hFuel[i]->SetLineColor(color[i]);
		hFuel[i]->SetLineWidth(2);
		sprintf(strl,"MC IBD for %s;MeV", name_fuel[i]);
		hFuel[i]->SetTitle(strl);
		sprintf(strl, "%6.4f * PositronEnergy - (%5.3f)", kScale, kShift);
		tFuel[i]->Project(hFuel[i]->GetName(), strl, cSel);
		hFuel[i]->Scale(1.0/nfiles[i]);
	}

	TCanvas *cv = new TCanvas("MC", "MC", 1000, 800);
	cv->Print((pdfName+".pdf[").Data());
	hMC = (TH1D*)hExp->Clone("hMC");
	hMC->SetTitle("MC middle of compaign fuel mixture;E_{e+}, MeV");
	hMC->Reset();
	hMC->SetLineWidth(2);
	for (i=0; i<4;i++) hMC->Add(hFuel[i], frac_fuel[i] * cross_fuel[i]);
	k1 = hExp->FindBin(1.501);
	k2 = hExp->FindBin(2.999);
	k3 = hExp->FindBin(0.751);
	k4 = hExp->FindBin(6.999);
	hMC->Scale(hExp->Integral(k1, k2) / hMC->Integral(k1, k2));
	
	hExp->SetLineWidth(2);
	hExp->SetLineColor(kRed);
	hExp->GetXaxis()->SetRange(k3, k4);
	hExp->Draw();
	hMC->Draw("same");
	TLegend *lg = new TLegend(0.65, 0.77, 0.89, 0.89);
	lg->AddEntry(hExp, "Experiment", "le");
	sprintf(strl, "MC * %5.3f - %5.3f", kScale, kShift);
	lg->AddEntry(hMC, strl, "le");
	lg->Draw();

	cv->Print((pdfName + ".pdf").Data());

	TH1D *hRatio = (TH1D*) hExp->Clone("hRatio");
	hRatio->SetTitle("Experiment to MC ratio;E_{e+}, scaled, MeV;N_{EXP}/N_{MC}");
	hRatio->Divide(hMC);
	hRatio->SetMaximum(1.19);
	hRatio->SetMinimum(0.87);
	hRatio->Draw();

	hReno->SetMarkerColor(kBlue);
	hReno->SetMarkerSize(2.0);
	hReno->SetMarkerStyle(kStar);
	hReno->Draw("same,e");
	gReno->Draw("ep");
	
	TLegend *lg1 = new TLegend(0.65, 0.77, 0.89, 0.89);
	lg1->AddEntry(hRatio, "DANSS", "le");
	lg1->AddEntry(gReno, "RENO arXiv:1806.00248", "lpe");
	lg1->AddEntry(hReno, "RENO #oplus 34%/#sqrt{E}", "lpe");
	lg1->Draw();

	cv->Print((pdfName + ".pdf").Data());
	cv->Print((pdfName+".pdf]").Data());
	
	TFile *fOut = new TFile((pdfName + ".root").Data(), "RECREATE");
	fOut->cd();
	for (i=0; i<4; i++) hFuel[i]->Write();
	hExp->Write();
	hMC->Write();
	hRatio->Write();
	hReno->Write();
	fOut->Close();
}
