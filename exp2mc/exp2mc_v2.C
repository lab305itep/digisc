//	Compare MC to IBD spectrum
#include <TRandom2.h>
TRandom2 rnd;

class MyRandom {
    public:
	inline MyRandom(void) {;};
	inline ~MyRandom(void) {;};
	static inline double Gaus(double mean = 0, double sigma = 1) 
	{
		return rnd.Gaus(mean, sigma);
	};
	static inline double GausAdd(double val, double sigma, double csigma)
	{
		return rnd.Gaus(val, sqrt(val*sigma*sigma + val*val*csigma*csigma));
	};
};

#define BASEDIR "/home/clusters/rrcmpi/alekseev/igor/pair7n11/MC/DataTakingPeriod01/"

TChain *make_MCchain(int isotope)
{
	TChain *ch = new TChain("DanssPair");
	
	switch (isotope) {
	case 235:
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_00.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_01.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_02.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_03.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_04.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_05.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_06.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_07.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_08.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_235U_09.root");
		break;
	case 238:
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_238U.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_00.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_01.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_02.root");
		break;
	case 239:
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_00.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_01.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_02.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_03.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_04.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_05.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_06.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_07.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_08.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_239Pu_09.root");
		break;
	case 241:
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_00.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_01.root");
		ch->AddFile(BASEDIR "Fuel/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_02.root");
		break;
	default:
		delete ch;
		return NULL;
	}
	return ch;
}

void exp2mc(const char *name_exp = "hSum_Main", int version = 780, double kRndm = 0.17, double kCRndm = 0, double kScale = 1.00)
{
	char strs[128], strl[1024];
	const char *file_exp;
	const int a_fuel[4] = {235, 238, 239, 241};
	const char *name_fuel[4] = {"235U", "238U", "239Pu", "241Pu"};
	const double frac_fuel[4] = {53, 7.3, 34, 5.7};		// some average for Apr16-Jan 19. 
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
	const double reno[20] = {
		1.00, 1.02, 0.995, 1.00, 1.00, 1.015, 1.005, 0.98,  1.02, 1.055,
		1.09, 1.12, 1.13,  1.12, 1.06, 1.05,  1.04,  1.005, 0.95, 0.98
	};
	const double renoe[20] = {
		0.01, 0.01,  0.01,  0.01,  0.01,  0.01, 0.01, 0.01,  0.01, 0.01,
		0.01, 0.015, 0.015, 0.015, 0.015, 0.02, 0.02, 0.025, 0.03, 0.03
	};
	
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cT("gtDiff > 2");
	TCut cA("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 8 && AnnihilationMax < 0.8 && PositronHits < 7");
	TCut cR("((PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || Distance < 45) && Distance < 55");
	TCut cN("NeutronEnergy > 3.5 && NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cOneHit("!(PositronHits == 1 && (AnnihilationGammas < 2 || AnnihilationEnergy < 0.2 || MinPositron2GammaZ > 15))");
	TCut cSel = cX && cY && cZ && cA && cR && cN && cOneHit;
	file_exp = NULL;
	switch (version) {
	case 7110:	// all cuts
		cSel = cX && cY && cZ && cA && cR && cN && cOneHit;
		file_exp = "/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base-calc.root";
		break;
	case 740:	// all cuts
		cSel = cX && cY && cZ && cA && cR && cN && cOneHit;
		file_exp  = "apr16_mar19_7.4_base-calc.root";
		break;
	case 741:	// no one hit extra cuts
		file_exp  = "apr16_mar19_7.4_1hit-calc.root";
		cSel = cX && cY && cZ && cA && cR && cN;
		break;
	case 742:	// no distance cut
		cSel = cX && cY && cZ && cA && cN && cOneHit;
		break;
	case 743:	// no all neutron cuts
		cSel = cX && cY && cZ && cA && cOneHit;
		break;
	case 780:
		cSel = cX && cY && cZ && cA && cR && cN && cOneHit;
		file_exp = "/home/clusters/rrcmpi/alekseev/igor/apr16_oct19/base-calc.root";
		break;
	default:
		printf("Wrong version = %d\n", version);
		return;
		break;
	}
	
	gStyle->SetOptStat(0);
	sprintf(strl, "ibd_%d_%s_rndm_%5.3f_%5.3f_scale_%5.3f", version, name_exp, kRndm, kCRndm, kScale);
	TString pdfName(strl);
	
	fExp = new TFile(file_exp);
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
		if (kRndm > 0 || kCRndm > 0) {
			sprintf(strl, "%6.3f*MyRandom::GausAdd((((PositronHits == 1)"
			" ? (PositronEnergy+0.0292)/1.043 : (PositronEnergy-0.1779)/0.9298))"
			", %8.5f, %8.54f)", kScale, kRndm, kCRndm);
		} else {
			sprintf(strl, "%6.3f * (((PositronHits == 1) ? (PositronEnergy+0.0292)/1.043"
			" : (PositronEnergy-0.1779)/0.9298))", kScale);
		}
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
	k3 = hExp->FindBin(1.001);
	k4 = hExp->FindBin(6.999);
	hMC->Scale(hExp->Integral(k1, k2) / hMC->Integral(k1, k2));
	
	hExp->SetLineWidth(2);
	hExp->SetLineColor(kRed);
	hExp->GetXaxis()->SetRange(k3, k4);
	hExp->Draw();
	hMC->Draw("same");
	TLegend *lg = new TLegend(0.65, 0.77, 0.89, 0.89);
	lg->AddEntry(hExp, "Experiment", "le");
	sprintf(strl, "MC * %5.3f #oplus %5.3f/#sqrt{E} #oplus %5.3f", kScale, kRndm, kCRndm);
	lg->AddEntry(hMC, strl, "le");
	lg->Draw();

	cv->Print((pdfName + ".pdf").Data());

	TH1D *hRatio = (TH1D*) hExp->Clone("hRatio");
	hRatio->SetTitle("Experiment to MC ratio;E_{e+}, scaled, MeV;N_{EXP}/N_{MC}");
	hRatio->Divide(hMC);
	hRatio->SetMaximum(1.19);
	hRatio->Draw();
	TH1D *hReno = new TH1D("hReno", "Reno with data smoothed with DANSS energy resolution", 20, kScale, 6*kScale);
	for (i=0; i<20; i++) {
		hReno->SetBinContent(i+1, reno[i]);
		hReno->SetBinError(i+1, renoe[i]);
	}
	hReno->SetMarkerColor(kBlue);
	hReno->SetMarkerSize(2.0);
	hReno->SetMarkerStyle(kStar);
	hReno->Draw("same,e");
	
	TLegend *lg1 = new TLegend(0.65, 0.77, 0.89, 0.89);
	lg1->AddEntry(hRatio, "DANSS", "le");
	lg1->AddEntry(hReno, "RENO", "lpe");
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
