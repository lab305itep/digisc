//	Compare different cuts
#define BASEDIR "/home/clusters/rrcmpi/alekseev/igor/pair8n2/MC/IBD/"

TChain *make_MCchain(void)
{
	TChain *ch = new TChain("DanssPair");
	
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_00_01.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_00_02.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_00_03.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_00_04.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_01_01.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_01_02.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_01_03.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_01_04.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_02_01.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_02_02.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_02_03.root");
	ch->AddFile(BASEDIR "235U/mc_IBD_indLY_transcode_rawProc_pedSim_235U_02_04.root");

	ch->AddFile(BASEDIR "238U/mc_IBD_indLY_transcode_rawProc_pedSim_238U_00_01.root");
	ch->AddFile(BASEDIR "238U/mc_IBD_indLY_transcode_rawProc_pedSim_238U_00_02.root");
	ch->AddFile(BASEDIR "238U/mc_IBD_indLY_transcode_rawProc_pedSim_238U_00_03.root");
	ch->AddFile(BASEDIR "238U/mc_IBD_indLY_transcode_rawProc_pedSim_238U_00_04.root");

	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_00_01.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_00_02.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_00_03.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_00_04.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_01_01.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_01_02.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_01_03.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_01_04.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_02_01.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_02_02.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_02_03.root");
	ch->AddFile(BASEDIR "239Pu/mc_IBD_indLY_transcode_rawProc_pedSim_239Pu_02_04.root");

	ch->AddFile(BASEDIR "241Pu/mc_IBD_indLY_transcode_rawProc_pedSim_241Pu_00_01.root");
	ch->AddFile(BASEDIR "241Pu/mc_IBD_indLY_transcode_rawProc_pedSim_241Pu_00_02.root");
	ch->AddFile(BASEDIR "241Pu/mc_IBD_indLY_transcode_rawProc_pedSim_241Pu_00_03.root");
	ch->AddFile(BASEDIR "241Pu/mc_IBD_indLY_transcode_rawProc_pedSim_241Pu_00_04.root");

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
/*	Cuts 03.93.2923
export SPECTR_WHAT="PositronEnergy*0.994"
export SPECTR_MUCUT="gtFromVeto > 90"
export SPECTR_CUT00="gtFromPrevious > 50 && gtToNext > 80 && EventsBetween == 0"
export SPECTR_CUT01="gtFromShower > 120 || ShowerEnergy < 800"
export SPECTR_CUT02="PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)"
export SPECTR_CUT03="PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)"
export SPECTR_CUT04="PositronX[2] > 3.5 && PositronX[2] < 95.5"
export SPECTR_CUT05="gtDiff > 1"
export SPECTR_CUT06="AnnihilationEnergy < 1.2 && AnnihilationGammas < 12"
export SPECTR_CUT07="PositronEnergy*0.994 > 0.5"
export SPECTR_CUT08="(PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || (Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy))"
export SPECTR_CUT09="Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)"
export SPECTR_CUT10="NeutronEnergy > 1.5 + 2.6 * exp(-0.15 * PositronEnergy*PositronEnergy)"
export SPECTR_CUT11="NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20"
export SPECTR_CUT12="!(PositronHits == 1 && (AnnihilationGammas < 1 || AnnihilationEnergy < 0.1))"
export SPECTR_CUT13="PositronHits < 8"
*/
	const char *Name[] = {"Base", "Fiducial", "Positron", "Distance", "Neutron", "Single"};
	TH1D *h[6];
	char strs[128], strl[256];
	int i;

	TCut cVeto("gtFromVeto > 90 && gtFromPrevious > 50 && gtToNext > 80 && EventsBetween == 0 && (gtFromShower > 120 || ShowerEnergy < 800)");
	TCut cBase("gtDiff > 1 && PositronEnergy*0.994 > 0.5");
	cBase = cVeto && cBase;
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cXYZ = cX && cY && cZ;
	TCut cPositron("PositronHits < 8 && AnnihilationEnergy < 1.2 && AnnihilationGammas < 12");
	TCut cR2("(PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0) || (Distance < 40 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy))");
	TCut cR3("Distance < 48 - 17 * exp(-0.13 * PositronEnergy*PositronEnergy)");
	TCut cR = cR2 && cR3;
	TCut cN("NeutronEnergy > 1.5 + 2.6 * exp(-0.15 * PositronEnergy*PositronEnergy) && NeutronEnergy < 9.5 && NeutronHits >= 3 && NeutronHits < 20");
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
		sprintf(strl, "Spectrum with cuts;E_{e+}, MeV");
		h[i] = new TH1D(strs, strl, 128, 0, 16);
		ch->Project(h[i]->GetName(), "PositronEnergy*0.994", cc[i]);
		h[i]->Write();
	}
	f.Close();
}

void plot_cuts(void)
{
	const double N=16E6;
	const char *Name[] = {"Base", "Fiducial", "Positron", "Distance", "Neutron", "Single"};
	const int color[] = {kBlack, kRed, kGreen, kBlue, kOrange, kMagenta};
	TH1D *h[6];
	char str[256];
	int i;

	gStyle->SetOptStat(0);
	TFile *f = new TFile("draw_cuts.root");

	TLegend *lg = new TLegend(0.5, 0.5, 0.89, 0.89);
	for (i=0; i<6; i++) {
		sprintf(str, "h%s", Name[i]);
		h[i] = (TH1D *) f->Get(str);
		if (!h[i]) {
			printf("No %s\n", str);
			return;
		}
		h[i]->SetLineWidth(2);
		h[i]->SetLineColor(color[i]);
		h[i]->Draw((i) ? "same" : "");
		sprintf(str, "%s: %6.4f [0.75-8]", Name[i], h[i]->Integral(7, 64) / N);
		lg->AddEntry(h[i], str, "l");
	}
	lg->Draw();
}

