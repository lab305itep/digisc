//	Compare different cuts
#define BASEDIR "/home/clusters/rrcmpi/alekseev/igor/pair7n15/MC/DataTakingPeriod01/"

TChain *make_MCchain(void)
{
	TChain *ch = new TChain("DanssPair");
	
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

	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U.root");
	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_00.root");
	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_01.root");
	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_238U_02.root");

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

	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu.root");
	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_00.root");
	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_01.root");
	ch->AddFile(BASEDIR "FuelGd/mc_IBD_glbLY_transcode_rawProc_pedSim_241Pu_02.root");

	return ch;
}

void paint_cuts(void)
{
	const char *Name[] = {"hBase", "hFiducial", "hPositron", "hDistance", "hNeutron", "hSingle"};
	const enum EColor clr[] = {kBlack, kRed, kGreen, kBlue, kOrange, kViolet};
	TH1D *h[6], *hr[6];
	char strs[128];
	int i;
	
	gStyle->SetOptStat(0);
	TCanvas *cv = new TCanvas("CV", "CV", 1200, 800);
	cv->Divide(2, 1);
	TFile f("draw_cuts.root");
	cv->cd(1);
	TLegend *lg  = new TLegend(0.5, 0.55, 0.98, 0.92);
	for (i=0; i<6; i++) {
		h[i] = (TH1D*) f.Get(Name[i]);
		h[i]->SetLineColor(clr[i]);
		if (!i) {
			h[0]->GetXaxis()->SetRangeUser(0.5, 8);
			h[0]->SetTitle("Spectrum with cuts");
			h[0]->DrawCopy();
		} else {
			h[i]->DrawCopy("same");
		}
		sprintf(strs, "%s: %d", Name[i], (int) h[i]->GetEntries());
		lg->AddEntry(h[i], strs, "l");
	}
	lg->Draw();
	cv->cd(2);
	lg  = new TLegend(0.5, 0.15, 0.98, 0.52);
	for (i=0; i<6; i++) {
		sprintf(strs, "%s_r", Name[i]);
		hr[i] = (TH1D*) h[i]->Clone(strs);
		hr[i]->Divide(h[i], h[(i>0) ? i-1 : 0]);
		if (!i) {
			hr[0]->GetXaxis()->SetRangeUser(0.5, 8);
			hr[0]->SetMaximum(1.05);
			hr[0]->SetMinimum(0.5);
			hr[0]->SetTitle("Effect of each cut");
			hr[0]->DrawCopy();
		} else {
			hr[i]->DrawCopy("same");
		}
		sprintf(strs, "%s: %6.4f", Name[i], hr[i]->Integral(5, 64) / hr[0]->Integral(5, 64));
		lg->AddEntry(h[i], strs, "l");
	}
	lg->Draw();
	cv->SaveAs("draw_cuts.png");
	f.Close();
}

void draw_cuts(void)
{
	const char *Name[] = {"Base", "Fiducial", "Positron", "Distance", "Neutron", "Single"};
	TH1D *h[6];
	char strs[128], strl[256];
	int i;

	TCut cVeto("gtFromVeto > 90 && gtFromPrevious > 50 && gtToNext > 80 && EventsBetween == 0 && (gtFromShower > 120 || ShowerEnergy < 800)");
	TCut cBase("gtDiff > 1 && PositronEnergy > 0.5");
	cBase = cVeto && cBase;
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cXYZ = cX && cY && cZ;
	TCut cPositron("PositronHits < 8 && AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cR2("(PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || (Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy))");
	TCut cR3("Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR = cR2 && cR3;
	TCut cN("NeutronEnergy > 1.5 + 3 * exp(-0.13 * PositronEnergy*PositronEnergy) && NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
	TCut cSingle("!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))");
	TCut cc[6];
	cc[0] = cBase;
	cc[1] = cc[0] && cXYZ;
	cc[2] = cc[1] && cPositron;
	cc[3] = cc[2] && cR;
	cc[4] = cc[3] && cN;
	cc[5] = cc[4] && cSingle;
	
	TChain *ch = make_MCchain();
	TFile f("draw_cuts.root", "RECREATE");
	for (i=0; i<6; i++) {
		sprintf(strs, "h%s", Name[i]);
		sprintf(strl, "Spectrum with cuts: %s;E_{e+}, MeV", Name[i]);
		h[i] = new TH1D(strs, strl, 128, 0, 16);
		ch->Project(h[i]->GetName(), "PositronEnergy", cc[i]);
		h[i]->Write();
	}
	f.Close();
}

