#include <stdio.h>
#include <TCut.h>
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>
#include "HPainter2.h"

/****************************************
 *	Calculate various cuts		*
 ****************************************/

#define NHISTS 17
void background_calc(const char *fname, int run_first, int run_last, TCut cAux = (TCut) "")
{
	char strs[128];
	char strl[1024];
	const char titles[NHISTS][16] = {"gtDiff", "R1", "R2", "RZ", "PX", "PY", "PZ", "NX", "NY", "NZ", "NE", "NH", "PH", "AH", "AE", "AM", "AMO"};
	const char titlel[NHISTS][256] = {
		"Time from positron to neutron;us",
		"Distance between positron and neutron, 2D;cm", "Distance between positron and neutron, 3D;cm", "Distance between positron and neutron, Z;cm", 
		"Positron vertex X;cm", "Positron vertex Y;cm", "Positron vertex Z;cm", 
		"Neutron vertex X;cm", "Neutron vertex Y;cm", "Neutron vertex Z;cm", 
		"Neutron capture energy;MeV", "Neutron capture SiPM hits",
		"Number of SiPM hits in positron cluster", 
		"Number of SiPM hits out of the cluster", "Energy out of the cluster;MeV", "The most energetic hit out of the cluster;MeV",
		"The most energetic hit out of the cluster - other cuts applied;MeV"
	};
	TH1D *h[NHISTS][3];
	int i, j;
	
//		Main cuts
	TCut cVeto("gtFromVeto > 60");
	TCut cMuonA("gtFromVeto == 0");
	TCut cMuonB("gtFromVeto > 0 && gtFromVeto <= 60");
	TCut cIso("(gtFromPrevious > 45 && gtToNext > 80 && EventsBetween == 0) || (gtFromPrevious == gtFromVeto)");
	TCut cShower("gtFromVeto > 200 || DanssEnergy < 300");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 2");
        TCut cGamma("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10");
	TCut cGammaMax("AnnihilationMax < 0.8");
        TCut cPe("PositronEnergy > 1");
        TCut cR1("Distance < 45");
        TCut cR2("Distance < 55");
//        TCut cRZ("fabs(DistanceZ) < 40");
        TCut cR = cR2 && (cRXY || cR1) && cRZ;
        TCut cN("NeutronEnergy > 3.5");
        TCut ct;
	TCut cv[3];

	cv[0] = cVeto;	// neutrino
	cv[1] = cMuonA; // fast neutron
	cv[2] = cMuonB; // pair of neutrons

	TFile *fRoot = new TFile(fname, "RECREATE");
	for (i=0; i<NHISTS; i++) for (j=0; j<3; j++) {
		sprintf(strs, "h%s%c", titles[i], 'A' + j);
		switch(i) {
		case 0:		// gtDiff
			h[i][j] = new TH1D(strs, titlel[i], 50, 0, 50.0);
			break;
		case 1:		// R1, R2
		case 2:
			h[i][j] = new TH1D(strs, titlel[i], 40, 0, 160.0);
			break;
		case 3:		// RZ
			h[i][j] = new TH1D(strs, titlel[i], 100, -50.0, 50.0);
			break;
		case 4:		// PX, PY, NX, NY
		case 5:
		case 7:
		case 8:
			h[i][j] = new TH1D(strs, titlel[i], 25, 0, 100.0);
			break;
		case 6:		// PZ, NZ
		case 9:
			h[i][j] = new TH1D(strs, titlel[i], 100, 0, 100.0);
			break;
		case 10:	// NE
			h[i][j] = new TH1D(strs, titlel[i], 45, 3.0, 12.0);
			break;
		case 11:	// NH
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 12:	// PH
			h[i][j] = new TH1D(strs, titlel[i], 10, 0, 10.0);
			break;
		case 13:	// AH
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 20.0);
			break;
		case 14:	// AE, AM, AMO
		case 15:
		case 16:
			h[i][j] = new TH1D(strs, titlel[i], 20, 0, 4.0);
			break;
		}
	}

	HPainter2 *hp = new HPainter2(0x801E, run_first, run_last);
	hp->SetFile(fRoot);

	for (j=0; j<3; j++) {
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax && cN;
		hp->Project(h[0][j], "gtDiff", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cGamma && cGammaMax && cN && !cRXY;
		hp->Project(h[1][j], "Distance", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cGamma && cGammaMax && cN && cRXY;
		hp->Project(h[2][j], "Distance", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cPe && cGamma && cGammaMax && cN;
		hp->Project(h[3][j], "DistanceZ", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cGamma && cGammaMax && cN && "PositronX[0] >= 0";
		hp->Project(h[4][j], "PositronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cGamma && cGammaMax && cN && "PositronX[1] >= 0";
		hp->Project(h[5][j], "PositronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cGamma && cGammaMax && cN && "PositronX[2] >= 0";
		hp->Project(h[6][j], "PositronX[2] + 0.5", ct && cv[j] && cAux);
		ct = cIso && cShower && cY && cZ && cR && cPe && cGamma && cGammaMax && cN && "NeutronX[0] >= 0";
		hp->Project(h[7][j], "NeutronX[0] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cZ && cR && cPe && cGamma && cGammaMax && cN && "NeutronX[1] >= 0";
		hp->Project(h[8][j], "NeutronX[1] + 2.0", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cR && cPe && cGamma && cGammaMax && cN && "NeutronX[2] >= 0";
		hp->Project(h[9][j], "NeutronX[2] + 0.5", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax;
		hp->Project(h[10][j], "NeutronEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax;
		hp->Project(h[11][j], "NeutronHits", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cGamma && cGammaMax;
		hp->Project(h[12][j], "PositronHits", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN;
		hp->Project(h[13][j], "AnnihilationGammas", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN;
		hp->Project(h[14][j], "AnnihilationEnergy", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN;
		hp->Project(h[15][j], "AnnihilationMax", ct && cv[j] && cAux);
		ct = cIso && cShower && cX && cY && cZ && cR && cPe && cN && cGamma;
		hp->Project(h[16][j], "AnnihilationMax", ct && cv[j] && cAux);
	}
	
	fRoot->cd();
	for (i=0; i<NHISTS; i++) for (j=0; j<3; j++) h[i][j]->Write();
	delete hp;
	fRoot->Close();
}


void background_draw_all(const char *rootname = "background_plots2.root")
{
	char strs[128];
	char strl[1024];
	const char titles[NHISTS][16] = {"gtDiff", "R1", "R2", "RZ", "PX", "PY", "PZ", "NX", "NY", "NZ", "NE", "NH", "PH", "AH", "AE", "AM", "AMO"};
	const char suffix[4][10] = {"A-rand", "A-diff", "B-diff", "C-diff"};
	const Color_t color[4] = {kGreen+2, kBlue, kRed, kOrange};
	const int marker[4] = {kOpenCircle, kFullCircle, kOpenSquare, kOpenStar};
	TH1D *h[NHISTS][4];
	int i, j;
	double hMax;
	int iMax;
	char pdfname[1024];
	char *ptr;

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
	
	TFile *fRoot = new TFile(rootname);
	if (!fRoot->IsOpen()) {
		printf("root-file %s not found  - run background_calc() first!\n", rootname);
		return;
	}
	for (i=0; i<NHISTS; i++) {
		for (j=0; j<4; j++) {
			sprintf(strs, "h%s%s", titles[i], suffix[j]);
			h[i][j] = (TH1D*) fRoot->Get(strs);
			if (!h[i][j]) {
				printf("%s not found  - rerun background_calc() to create all hists!\n", strs);
				fRoot->Close();
				return;
			}
			h[i][j]->SetLineWidth(2);
			h[i][j]->SetLineColor(color[j]);
			h[i][j]->SetMarkerColor(color[j]);
			h[i][j]->SetMarkerStyle(marker[j]);
			h[i][j]->GetYaxis()->SetLabelSize(0.05);
			h[i][j]->SetMinimum(0);
			h[i][j]->GetYaxis()->SetTitle("");
		}
	}
	
	TCanvas *cv = new TCanvas("CV", "Background plots", 1200, 900);
	TLegend *lg = new TLegend(0.7, 0.8, 0.95, 0.95);
	lg->AddEntry(h[0][0], "Random", "LP");
	lg->AddEntry(h[0][1], "Neutrino", "LP");
	lg->AddEntry(h[0][2], "Cosmic-A", "LP");
	lg->AddEntry(h[0][3], "Cosmic-B", "LP");
	
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
		lg->Draw();
		cv->SaveAs(pdfname);
	}
	
	sprintf(strl, "%s]", pdfname);
	cv->SaveAs(strl);
	fRoot->Close();
}

/****************************************
 *	Calc and draw gtDiff 		*
 ****************************************/

void background_calcgt(const char *fname = "background_plotsgt.root", int run_first = 5808, int run_last = 15028)
{
	char strs[128];
	char strl[1024];
	TH1D *h[3][2];
	int i, j;
	
//		Main cuts
	TCut cVeto("gtFromVeto > 60");
	TCut cIso("(gtFromPrevious > 45 && gtToNext > 80 && EventsBetween == 0) || (gtFromPrevious == gtFromVeto)");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 2");
        TCut cGamma("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10 && AnnihilationMax < 0.8" );
        TCut cPe("PositronEnergy > 1");
        TCut cR60("Distance < 60");
        TCut cR1("Distance < 45");
        TCut cR2("Distance < 55");
        TCut cRZ("fabs(DistanceZ) < 40");
        TCut cR = cR2 && (cRXY || cR1) && cRZ;
        TCut cN("NeutronEnergy > 3.5");
        TCut cN3("NeutronEnergy > 3.0");
        TCut ct;

	TFile *fRoot = new TFile(fname, "RECREATE");
	for (i=0; i<3; i++) for (j=0; j<2; j++) {
		sprintf(strs, "hgtDiff%c%c", 'A'+i, (j) ? 'C' : 'N');
		sprintf(strl, "Time from positron to neutron: %s;us", (j) ? "Cosmic" : "Neutrino");
		h[i][j] = new TH1D(strs, strl, 50, 0, 50.0);
	}

	HPainter2 *hp = new HPainter2(0x801E, run_first, run_last);
	hp->SetFile(fRoot);

	for (j=0; j<2; j++) {
		ct = cIso && cX && cY && cZ && cR60 && cRZ && cPe && cGamma && cN3;
		hp->Project(h[0][j], "gtDiff", (j) ? (ct && !cVeto) : (ct && cVeto));
		ct = cIso && cX && cY && cZ && cR && cPe && cGamma && cN3;
		hp->Project(h[1][j], "gtDiff", (j) ? (ct && !cVeto) : (ct && cVeto));
		ct = cIso && cX && cY && cZ && cR && cPe && cGamma && cN;
		hp->Project(h[2][j], "gtDiff", (j) ? (ct && !cVeto) : (ct && cVeto));
	}
	
	fRoot->cd();
	for (i=0; i<3; i++) for (j=0; j<2; j++) h[i][j]->Write();
	delete hp;
	fRoot->Close();
}

void background_draw_gt(const char *rootname = "background_plotsgt.root")
{
	char strs[128];
	char strl[1024];
	char title[3][128] = {
		"Old cuts: R < 60 cm && NeutronEnergy > 3.0 MeV",
		"Diatance < 45 cm for 2D and Distance < 55 cm for 3 D, NeutronEnergy > 3.0 MeV",
		"Diatance < 45 cm for 2D and Distance < 55 cm for 3 D, NeutronEnergy > 3.5 MeV"
	};
	const char suffix[3][10] = {"N-rand", "N-diff", "C-diff"};
	const Color_t color[3] = {kGreen+2, kBlue, kRed};
	const int marker[3] = {kFullStar, kFullCircle, kFullCross};
	TH1D *h[3][3];
	int i, j, k, kl, ku;
	double hMax;
	int iMax;
	double total, totale;
	double frac, frace;
	char pdfname[1024];
	char *ptr;

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
	
	TFile *fRoot = new TFile(rootname);
	if (!fRoot->IsOpen()) {
		printf("root-file %s not found  - run background_calc() first!\n", rootname);
		return;
	}
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++) {
			sprintf(strs, "hgtDiff%c%s", 'A'+i, suffix[j]);
			h[i][j] = (TH1D*) fRoot->Get(strs);
			if (!h[i][j]) {
				printf("%s not found  - rerun background_calc() to create all hists!\n", strs);
				fRoot->Close();
				return;
			}
			h[i][j]->SetLineWidth(2);
			h[i][j]->SetLineColor(color[j]);
			h[i][j]->SetMarkerColor(color[j]);
			h[i][j]->SetMarkerStyle(marker[j]);
			h[i][j]->GetYaxis()->SetLabelSize(0.05);
			h[i][j]->SetMinimum(0);
			h[i][j]->GetYaxis()->SetTitle("");
			h[i][j]->SetTitle(title[i]);
		}
	}
	TCanvas *cv = new TCanvas("CV", "Background plots", 1200, 900);
	TLegend *lg = new TLegend(0.7, 0.8, 0.95, 0.95);
	lg->AddEntry(h[0][0], "Random", "LP");
	lg->AddEntry(h[0][1], "Neutrino", "LP");
	lg->AddEntry(h[0][2], "Cosmic", "LP");
	
	sprintf(strl, "%s[", pdfname);
	cv->SaveAs(strl);
	
	for (i=0; i<3; i++) {
		cv->Clear();
		hMax = 0;
		iMax = 0;
		for (j=0; j<3; j++) if (h[i][j]->GetMaximum() > hMax) {
			hMax = h[i][j]->GetMaximum();
			iMax = j;
		}
		h[i][iMax]->Draw();
		for (j=0; j<3; j++) if (j != iMax) h[i][j]->Draw("same");
		lg->Draw();
		
		cv->SaveAs(pdfname);
	}
	
	sprintf(strl, "%s]", pdfname);
	cv->SaveAs(strl);
	fRoot->Close();
}

/****************************************
 *	Calc and draw positron energy 	*
 ****************************************/

void background_calc_pe(const char *fname = "background_plotspe2.root", int run_first = 5808, int run_last = 15028)
{
	char strs[128];
	char strl[1024];
	TH1D *h[3];
	int i;

	
//		Main cuts
	TCut cVeto("gtFromVeto > 60");
	TCut cMuonA("gtFromVeto == 0");
	TCut cMuonB("gtFromVeto > 0 && gtFromVeto <= 60");
	TCut cIso("(gtFromPrevious > 45 && gtToNext > 80 && EventsBetween == 0) || (gtFromPrevious == gtFromVeto)");
	TCut cShower("gtFromVeto > 200 || DanssEnergy < 300");
	TCut cX("PositronX[0] < 0 || (PositronX[0] > 2 && PositronX[0] < 94)");
	TCut cY("PositronX[1] < 0 || (PositronX[1] > 2 && PositronX[1] < 94)");
	TCut cZ("PositronX[2] > 3.5 && PositronX[2] < 95.5");
	TCut cRXY("PositronX[0] >= 0 && PositronX[1] >= 0 && NeutronX[0] >= 0 && NeutronX[1] >= 0");
	TCut c20("gtDiff > 2");
        TCut cGamma("AnnihilationEnergy < 1.8 && AnnihilationGammas <= 10");
	TCut cGammaMax("AnnihilationMax < 0.8");
        TCut cPe("PositronEnergy > 1");
        TCut cR1("Distance < 45");
        TCut cR2("Distance < 55");
        TCut cRZ("fabs(DistanceZ) < 40");
        TCut cR = cR2 && (cRXY || cR1) && cRZ;
        TCut cN("NeutronEnergy > 3.5");

        TCut ct = cIso && cShower && cX && cY && cZ && c20 && cR && cPe && cGamma && cGammaMax && cN;

	TFile *fRoot = new TFile(fname, "RECREATE");
	for (i=0; i<3; i++) {
		sprintf(strs, "hPosEnergy%c", 'A'+i);
		sprintf(strl, "Positron Energy;MeV");
		h[i] = new TH1D(strs, strl, 60, 1.0, 16.0);
	}

	HPainter2 *hp = new HPainter2(0x801E, run_first, run_last);
	hp->SetFile(fRoot);

	hp->Project(h[0], "PositronEnergy", ct && cVeto);	// Neutrino
	hp->Project(h[1], "PositronEnergy", ct && cMuonA);	// Muon - immediate
	hp->Project(h[2], "PositronEnergy", ct && cMuonB);	// Muon - delayed
	
	fRoot->cd();
	for (i=0; i<3; i++)h[i]->Write();
	delete hp;
	fRoot->Close();
}

void background_draw_pe(const char *rootname = "background_plotspe2.root")
{
	char strs[256];
	const Color_t color[3] = {kGreen+2, kBlue, kRed};
	const int marker[3] = {kFullStar, kFullCircle, kFullCross};
	const char title[3][16] = {"Neutrino", "Cosmic A", "Cosmic B"};
	TH1D *h[3];
	TH1D *hz[3];
	int i;
	char pdfname[1024];
	char *ptr;
	TPad *pd;
	TVirtualPad *pv;
	double hMax;
	int iMax;
	TPaveStats *stat;

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
	
	gStyle->SetTitleXSize(0.06);
	gStyle->SetTitleYSize(0.06);
	gStyle->SetLabelSize(0.06);
	gStyle->SetPadLeftMargin(0.12);
	gStyle->SetPadBottomMargin(0.15);
	
	TFile *fRoot = new TFile(rootname);
	if (!fRoot->IsOpen()) {
		printf("root-file %s not found  - run background_calc() first!\n", rootname);
		return;
	}
	for (i=0; i<3; i++) {
		sprintf(strs, "hPosEnergy%c", 'A'+i);
		h[i] = (TH1D*) fRoot->Get(strs);
		if (!h[i]) {
			printf("%s not found  - rerun background_calcpe() to create all hists!\n", strs);
			fRoot->Close();
			return;
		}
		h[i]->SetTitle("Positron energy;MeV;mHz");
		h[i]->SetLineWidth(2);
		h[i]->SetLineColor(color[i]);
		h[i]->SetMarkerColor(color[i]);
		h[i]->SetMarkerStyle(marker[i]);
		h[i]->GetXaxis()->SetTitleSize(0.06);
		h[i]->GetYaxis()->SetTitleSize(0.06);
		h[i]->GetXaxis()->SetLabelSize(0.06);
		h[i]->GetYaxis()->SetLabelSize(0.06);
		h[i]->SetMinimum(0);
		sprintf(strs, "%s_z", h[i]->GetName());
		hz[i] = (TH1D*)h[i]->Clone(strs);
		hz[i]->SetTitle("");
		hz[i]->GetXaxis()->SetRange(hz[i]->FindBin(7.001), hz[i]->GetNbinsX());
	}
	TCanvas *cv = new TCanvas("CV", "Positron Energy", 1200, 900);
	TLegend *lg = new TLegend(0.5, 0.8, 0.95, 0.95);
	for (i=0; i<3; i++) {
		sprintf(strs, "%s = %5.1f mHz", title[i], h[i]->Integral());
		lg->AddEntry(h[i], strs, "LPE");
	}
	lg->SetTextSize(0.04);
	
	pv = cv->cd();
	pv->SetFillStyle(4000);

	hMax = 0;
	iMax = 0;
	for (i=0; i<3; i++) if (h[i]->GetMaximum() > hMax) {
		hMax = h[i]->GetMaximum();
		iMax = i;
	}
	h[iMax]->Draw();
	for (i=0; i<3; i++) if (i != iMax) h[i]->Draw("same");
	lg->Draw();

	pv->cd();
	pd = new TPad("PD", "", 0.35, 0.35, 0.95, 0.79);
	pd->SetTopMargin(0);
	pd->SetRightMargin(0);
	pd->Draw();
	pd->cd();
	hMax = 0;
	iMax = 0;
	for (i=0; i<3; i++) if (hz[i]->GetMaximum() > hMax) {
		hMax = hz[i]->GetMaximum();
		iMax = i;
	}
	hz[iMax]->Fit("pol1", "", "", 11, 16);
	for (i=0; i<3; i++) if (i != iMax) hz[i]->Fit("pol1", "", "sames", 11, 16);
	pd->Update();
	for (i=0; i<3; i++) {
		stat = (TPaveStats *) hz[i]->FindObject("stats");
		if (!stat) {
			printf("stat[%d] = NULL\n", i);
			continue;
		}
		stat->SetTextColor(color[i]);
		stat->SetLineColor(color[i]);
		stat->SetY1NDC(0.5  + 0.15*i);
		stat->SetY2NDC(0.65 + 0.15*i);
		stat->Draw();
	}
	pv->SetFillStyle(4000);
	pd->Draw();

	cv->SaveAs(pdfname);
	
	fRoot->Close();
}

