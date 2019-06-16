//	Global parameters
#define NEBINS	128
#include "positions.h"
TFile *fData;
// Fast neutron corrections 
const char *NeutronCorrN = "0.003475-0.000152*x";
const char *NeutronCorrC = "0.01138-0.000437*x";
const double OtherBlockFraction = 0.0060;	// Neutrino fraction of other blocks. Distances to other reactors: 160, 336 and 478 m
const double ERange[2] = {1.001, 6.999};

void change_file_suffix(char *to, int len, const char *from, const char *where, const char *what)
{
	char *ptr;
	
	strncpy(to, from, len - strlen(what));
	ptr = strstr(to, where);
	if (ptr) *ptr = '\0';
	strcat(to, what);
}

//	Makes an integer out of date.
//	A string dd.mm.yy expected.
//	result = dd + 31*mm + 366*yy
//	We don't care if numbers are not sequential
int date2int(const char *str)
{
	int d, m, y;
	d = 10 * (str[0] - '0') + str[1] - '0';
	m = 10 * (str[3] - '0') + str[4] - '0';
	y = 10 * (str[6] - '0') + str[7] - '0';	
	return d + 31*m + 366*y;
}

// Check if name like "xxxx_dd.mm.yy" is in the range from-to
// "from" is included and "to" is not included
int is_in_date_range(const char *name, const char *from, const char *to)
{
	int iname, ifrom, ito;
	const char *ptr;
	
	if (!(from && to)) return true;
	ifrom = date2int(from);
	ito = date2int(to);
	ptr = strrchr(name, '_');
	if (!ptr) {
		ptr = name;
	} else {
		ptr++;
	}
	iname = date2int(ptr);
	return (iname >= ifrom && iname < ito);
}

//  Make sum of spectra over large number of points
int sum_of_spectra(TH1D *hSum, TH1D *hSub, const char *posmask, int permask, double bgScale = 1.0, double *days = NULL)
{
	int N;
	TH1D *hSig;
	TH1D *hConst;
	TH1D *hBgnd;
	int i;
	char str[1024];
	double tSum;
	double dt;
	char *ptr;
	int Cnt;
	TF1 fBgndN("fBgndN", NeutronCorrN, 0, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0, 100);

	N = sizeof(positions) / sizeof(positions[0]);
	hSum->Reset();
	hSub->Reset();
	tSum = 0;
	for (i=0; i<N; i++) {
		ptr = strchr(posmask, positions[i].name[0]);
		if (!(ptr && ((1 << positions[i].period) & permask))) continue;
		sprintf(str, "%s_hSig-diff", positions[i].name);
		hSig = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hConst", positions[i].name);
		hConst = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hCosm-diff", positions[i].name);
		hBgnd = (TH1D*) fData->Get(str);
		if (!(hSig && hConst && hBgnd)) continue;
		dt = hConst->GetBinContent(1) / 1000.0;	// seconds * 10^3
		tSum += dt;
		hSum->Add(hSig);
		hSub->Add(hBgnd, positions[i].bgnd * bgScale);
		hSub->Add(&fBgndN, dt);
		hSub->Add(&fBgndC, -dt * positions[i].bgnd * bgScale);
	}
	hSum->Add(hSub, -1);
	if (days) *days = tSum / 86.4;
	if (tSum == 0) return 0;
	Cnt = hSum->Integral();
	hSum->Scale(86.4 / tSum);
	hSub->Scale(86.4 / tSum);
	return Cnt;
}

int sum_of_spectral(TH1D *hSum, TH1D *hSub, const char *posmask, const char *pfrom, const char *pto, double bgScale = 1.0, double *days = NULL)
{
	int N;
	TH1D *hSig;
	TH1D *hConst;
	TH1D *hBgnd;
	int i;
	char str[1024];
	double tSum;
	double dt;
	char *ptr;
	int Cnt;
	TF1 fBgndN("fBgndN", NeutronCorrN, 0, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0, 100);

	N = sizeof(positions) / sizeof(positions[0]);
	hSum->Reset();
	hSub->Reset();
	tSum = 0;
	for (i=0; i<N; i++) if (is_in_date_range(positions[i].name, pfrom, pto) && positions[i].period) {	// ignore reactor not @100%
		ptr = strchr(posmask, positions[i].name[0]);
		if (!(ptr && positions[i].period)) continue;
		sprintf(str, "%s_hSig-diff", positions[i].name);
		hSig = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hConst", positions[i].name);
		hConst = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hCosm-diff", positions[i].name);
		hBgnd = (TH1D*) fData->Get(str);
		if (!(hSig && hConst && hBgnd)) continue;
		dt = hConst->GetBinContent(1) / 1000.0;	// seconds * 10^3
		tSum += dt;
		hSum->Add(hSig);
		hSub->Add(hBgnd, positions[i].bgnd * bgScale);
		hSub->Add(&fBgndN, dt);
		hSub->Add(&fBgndC, -dt * positions[i].bgnd * bgScale);
	}
	hSum->Add(hSub, -1);
	if (days) *days = tSum / 86.4;
	if (tSum == 0) return 0;
	Cnt = hSum->Integral();
	hSum->Scale(86.4 / tSum);
	hSub->Scale(86.4 / tSum);
	return Cnt;
}

void sum_of_raw(TH1D *hSumSig, TH1D *hSumBgnd, const char *posmask, int permask, const char *pfrom = NULL, const char *pto = NULL)
{
	int N;
	TH1D *hSig;
	TH1D *hConst;
	TH1D *hBgnd;
	int i;
	char str[1024];
	double tSum;
	double dt;
	char *ptr;

	N = sizeof(positions) / sizeof(positions[0]);
	hSumSig->Reset();
	hSumBgnd->Reset();
	tSum = 0;
	for (i=0; i<N; i++) {
		ptr = strchr(posmask, positions[i].name[0]);
		if (!(is_in_date_range(positions[i].name, pfrom, pto) && ptr && ((1 << positions[i].period) & permask))) continue;
		sprintf(str, "%s_hSig-diff", positions[i].name);
		hSig = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hConst", positions[i].name);
		hConst = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hCosm-diff", positions[i].name);
		hBgnd = (TH1D*) fData->Get(str);
		if (!(hSig && hConst && hBgnd)) continue;
		dt = hConst->GetBinContent(1) / 1000.0;	// seconds * 10^3
		tSum += dt;
		hSumSig->Add(hSig);
		hSumBgnd->Add(hBgnd);
	}
	
	if (tSum == 0) return;
	hSumSig->Scale(86.4 / tSum);
	hSumBgnd->Scale(86.4 / tSum);
}


void draw_spectra_page(TCanvas *cv, const char *title, int periodmask, double bgScale)
{
	TLegend *lg;
	char strs[128];
	char strl[1024];
	double val, err;
	int Cnt, n;
	TLatex txt;

	cv->Clear();
	lg = new TLegend(0.35, 0.65, 0.9, 0.9);
	sprintf(strs, "hUp_%d", periodmask);
	sprintf(strl, "Positron spectrum %s, UP;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hUp = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hUpSub_%d", periodmask);
	sprintf(strl, "Background spectrum %s, UP;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hUpSub = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_of_spectra(hUp, hUpSub, "u", periodmask, bgScale);
	Cnt = n;
	TH1D *hTmp = (TH1D *) hUp->Clone("hTmp");	// keep to subtract block #3
	hUp->Add(hTmp, -OtherBlockFraction);
	hUp->Write();
	hUpSub->Write();
	hUp->SetLineColor(kRed);
	hUp->SetFillColor(kRed-10);
	hUp->GetYaxis()->SetLabelSize(0.08);
	hUp->SetTitle(title);
	hUp->Draw("hist,e");
	val = hUp->IntegralAndError(hUp->FindBin(1.001), hUp->FindBin(7.999), err);
	sprintf(strs, "  Up: %d events %6.1f #pm%5.1f / day", n, val, err);
	lg->AddEntry(hUp, strs, "l");

	sprintf(strs, "hMid_%d", periodmask);
	sprintf(strl, "Positron spectrum %s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hMid = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hMidSub_%d", periodmask);
	sprintf(strl, "Background spectrum %s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hMidSub = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_of_spectra(hMid, hMidSub, "m", periodmask, bgScale);
	Cnt += n;
	hMid->Add(hTmp, -OtherBlockFraction);
	hMid->Write();
	hMidSub->Write();
	hMid->SetLineColor(kGreen);
	hMid->SetFillColor(kGreen-10);
	hMid->Draw("same,hist,e");
	val = hMid->IntegralAndError(hUp->FindBin(1.001), hMid->FindBin(7.999), err);
	sprintf(strs, " Mid: %d events %6.1f #pm%5.1f / day", n, val, err);
	lg->AddEntry(hMid, strs, "l");

	sprintf(strs, "hDown_%d", periodmask);
	sprintf(strl, "Positron spectrum %s, DOWN;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hDown = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hDownSub_%d", periodmask);
	sprintf(strl, "Background spectrum %s, DOWN;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hDownSub = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_of_spectra(hDown, hDownSub, "d", periodmask, bgScale);
	Cnt += n;
	hDown->Add(hTmp, -OtherBlockFraction);
	hDown->Write();
	hDownSub->Write();
	hDown->SetLineColor(kBlue);
	hDown->SetFillColor(kBlue-10);
	hDown->Draw("same,hist,e");
	val = hDown->IntegralAndError(hUp->FindBin(1.001), hDown->FindBin(7.999), err);
	sprintf(strs, "Down: %d events %6.1f #pm%5.1f / day", n, val, err);
	lg->AddEntry(hDown, strs, "l");

	hUpSub->SetLineColor(kBlack);
	hUpSub->SetFillColor(kGray);
	hUpSub->Draw("same,hist,e");
	lg->AddEntry(hUpSub, "Background for Up position", "l");

	hUp->Draw("axis,same");
	lg->Draw();
	sprintf(strs, "%d events", Cnt);
	txt.DrawLatexNDC(0.6, 0.5, strs);
	cv->Update();
	delete hTmp;
}

void draw_spectra_pagel(TCanvas *cv, const char *title, const char *pfrom, const char *pto, double bgScale)
{
	TLegend *lg;
	char strs[128];
	char strl[1024];
	double val, err;
	int Cnt, n;
	TLatex txt;
	const double OtherBlockFraction = 0.0060;	// Distances to other reactors: 160, 336 and 478 m

	cv->Clear();
	lg = new TLegend(0.35, 0.65, 0.9, 0.9);
	sprintf(strs, "hUp_%s_%s", pfrom, pto);
	sprintf(strl, "Positron spectrum %s, UP;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hUp = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hUpSub_%s_%s", pfrom, pto);
	sprintf(strl, "Background spectrum %s, UP;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hUpSub = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_of_spectral(hUp, hUpSub, "u", pfrom, pto, bgScale);
	Cnt = n;
	TH1D *hTmp = (TH1D *) hUp->Clone("hTmp");	// keep to subtract block #3
	hUp->Add(hTmp, -OtherBlockFraction);
	hUp->Write();
	hUpSub->Write();
	hUp->SetLineColor(kRed);
	hUp->SetFillColor(kRed-10);
	hUp->GetYaxis()->SetLabelSize(0.08);
	hUp->SetTitle(title);
	hUp->Draw("hist,e");
	val = hUp->IntegralAndError(hUp->FindBin(1.001), hUp->FindBin(7.999), err);
	sprintf(strs, "  Up: %d events %6.1f #pm%5.1f / day", n, val, err);
	lg->AddEntry(hUp, strs, "l");

	sprintf(strs, "hMid_%s_%s", pfrom, pto);
	sprintf(strl, "Positron spectrum %s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hMid = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hMidSub_%s_%s", pfrom, pto);
	sprintf(strl, "Background spectrum %s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hMidSub = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_of_spectral(hMid, hMidSub, "m", pfrom, pto, bgScale);
	Cnt += n;
	hMid->Add(hTmp, -OtherBlockFraction);
	hMid->Write();
	hMidSub->Write();
	hMid->SetLineColor(kGreen);
	hMid->SetFillColor(kGreen-10);
	hMid->Draw("same,hist,e");
	val = hMid->IntegralAndError(hUp->FindBin(1.001), hMid->FindBin(7.999), err);
	sprintf(strs, " Mid: %d events %6.1f #pm%5.1f / day", n, val, err);
	lg->AddEntry(hMid, strs, "l");

	sprintf(strs, "hDown_%s_%s", pfrom, pto);
	sprintf(strl, "Positron spectrum %s, DOWN;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hDown = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hDownSub_%s_%s", pfrom, pto);
	sprintf(strl, "Background spectrum %s, DOWN;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hDownSub = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_of_spectral(hDown, hDownSub, "d", pfrom, pto, bgScale);
	Cnt += n;
	hDown->Add(hTmp, -OtherBlockFraction);
	hDown->Write();
	hDown->SetLineColor(kBlue);
	hDown->SetFillColor(kBlue-10);
	hDown->Draw("same,hist,e");
	val = hDown->IntegralAndError(hUp->FindBin(1.001), hDown->FindBin(7.999), err);
	sprintf(strs, "Down: %d events %6.1f #pm%5.1f / day", n, val, err);
	lg->AddEntry(hDown, strs, "l");

	hUpSub->SetLineColor(kBlack);
	hUpSub->SetFillColor(kGray);
	hUpSub->Draw("same,hist,e");
	lg->AddEntry(hUpSub, "Background for Up position", "l");

	hUp->Draw("axis,same");
	lg->Draw();
	sprintf(strs, "%d events", Cnt);
	txt.DrawLatexNDC(0.6, 0.5, strs);
	cv->Update();
	delete hTmp;
}

void draw_tail_hist(const char *title, const char *posmask)
{
	TLegend *lg;
	char strs[128];
	char strl[1024];
	TPaveStats *pv;
	double y1, y2;
	TLatex txt;

	lg = new TLegend(0.35, 0.65, 0.60, 0.9);
	sprintf(strs, "hTailN_%s", posmask);
	sprintf(strl, "Positron raw spectrum %s;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hN = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hTailC_%s", posmask);
	sprintf(strl, "Muon raw spectrum %s;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hC = new TH1D(strs, strl, NEBINS, 0, 16);
	sum_of_raw(hN, hC, posmask, 30);
	hN->Write();
	hC->Write();
	hC->SetLineColor(kRed);
	hC->GetYaxis()->SetLabelSize(0.08);
	hC->SetTitle(title);
	hC->SetTitleSize(0.08);
	hC->GetXaxis()->SetRange(hC->FindBin(8.001), hC->FindBin(15.999));
	hC->SetMinimum(0);
	hC->Fit("pol1", "", "", 10, 16);
	hN->SetLineColor(kBlue);
	hN->Fit("pol1", "", "sames", 10, 16);
	gPad->Update();
	pv = (TPaveStats *)hC->FindObject("stats");
	pv->SetLineColor(kRed);
	pv->SetTextColor(kRed);
	y1 = pv->GetY1NDC();
	y2 = pv->GetY2NDC();
	pv->SetY1NDC(2 * y1 - y2);
	pv->SetY2NDC(y1);
	pv->Draw();
	pv = (TPaveStats *)hN->FindObject("stats");
	pv->SetLineColor(kBlue);
	pv->SetTextColor(kBlue);
	pv->Draw();
	lg->AddEntry(hN, "Neutrino", "l");
	lg->AddEntry(hC, "Cosmic", "l");
	lg->Draw();
	sprintf(strs, "Neutrino 1-7 MeV = %5.1f events/day", hN->GetFunction("pol1")->Integral(1, 7) / hN->GetBinWidth(1));
	txt.SetTextColor(kBlue);
	txt.DrawLatex(11, 0.5*hC->GetMaximum(), strs);
	sprintf(strs, "Cosmic 1-7 MeV = %5.1f events/day", hC->GetFunction("pol1")->Integral(1, 7) / hC->GetBinWidth(1));
	txt.SetTextColor(kRed);
	txt.DrawLatex(11, 0.42*hC->GetMaximum(), strs);
}

void draw_single_ratio(const char *nameA, const char *nameB, const char *name, const char *title, double min=0.6, double max=1.2)
{
	TH1D *hA = (TH1D *) gROOT->FindObject(nameA);
	TH1D *hB = (TH1D *) gROOT->FindObject(nameB);
	if (!hA || !hB) {
		printf("Can not find hist: %s or/and %s. Step %s\n", nameA, nameB, title);
		return;
	}
	TH1D *hAB = (TH1D*) hA->Clone(name);
	hAB->SetName(name);
	hAB->SetTitle(title);
	hAB->Divide(hA, hB);
	hAB->SetLineColor(kBlue);
	hAB->Write();	
	hAB->SetMinimum(min);
	hAB->SetMaximum(max);
	hAB->GetYaxis()->SetLabelSize(0.08);
	hAB->GetXaxis()->SetRange(hAB->FindBin(ERange[0]), hAB->FindBin(ERange[1]));
	hAB->GetYaxis()->SetTitle("");
	hAB->Fit("pol0", "", "", ERange[0], ERange[1]);
}

void draw_normalized_ratio(const char* beginA, const char *endA, const char* beginB, const char *endB, 
	const char *name, const char *title, double min=0.6, double max=1.2, double bgScale = 1.0)
{
	char strs[128];
	double daysAU, daysAM, daysAD, daysBU, daysBM, daysBD;
//	int i;
//	const char posmask[3][3] = {"u", "m", "d"};
	const double Nfactor[3] = {1.0, (11.7*11.7)/(10.7*10.7), (12.7*12.7)/(10.7*10.7)};
	const double OtherBlockFraction = 0.0060;	// Distances to other reactors: 160, 336 and 478 m
//		Book
	TH1D *hAU = new TH1D("hNRAU", "", NEBINS, 0, 16);
	TH1D *hAM = new TH1D("hNRAM", "", NEBINS, 0, 16);
	TH1D *hAD = new TH1D("hNRAD", "", NEBINS, 0, 16);
	TH1D *hBU = new TH1D("hNRBU", "", NEBINS, 0, 16);
	TH1D *hBM = new TH1D("hNRBM", "", NEBINS, 0, 16);
	TH1D *hBD = new TH1D("hNRBD", "", NEBINS, 0, 16);
	TH1D *hAUb = new TH1D("hNRAUb", "", NEBINS, 0, 16);
	TH1D *hAMb = new TH1D("hNRAMb", "", NEBINS, 0, 16);
	TH1D *hADb = new TH1D("hNRADb", "", NEBINS, 0, 16);
	TH1D *hBUb = new TH1D("hNRBUb", "", NEBINS, 0, 16);
	TH1D *hBMb = new TH1D("hNRBMb", "", NEBINS, 0, 16);
	TH1D *hBDb = new TH1D("hNRBDb", "", NEBINS, 0, 16);
	TH1D *hAB = new TH1D(name, title, NEBINS, 0, 16);
//		Make initial sum
	sum_of_spectral(hAU, hAUb, "u", beginA, endA, bgScale, &daysAU);
	sum_of_spectral(hAM, hAMb, "m", beginA, endA, bgScale, &daysAM);
	sum_of_spectral(hAD, hADb, "d", beginA, endA, bgScale, &daysAD);
	sum_of_spectral(hBU, hBUb, "u", beginB, endB, bgScale, &daysBU);
	sum_of_spectral(hBM, hBMb, "m", beginB, endB, bgScale, &daysBM);
	sum_of_spectral(hBD, hBDb, "d", beginB, endB, bgScale, &daysBD);
//		Subtract other blocks
	TH1D *hTmpA = (TH1D *) hAU->Clone("hTmpA");
	TH1D *hTmpB = (TH1D *) hBU->Clone("hTmpB");
	hAU->Add(hTmpA, -OtherBlockFraction);
	hAM->Add(hTmpA, -OtherBlockFraction);
	hAD->Add(hTmpA, -OtherBlockFraction);
	hBU->Add(hTmpB, -OtherBlockFraction);
	hBM->Add(hTmpB, -OtherBlockFraction);
	hBD->Add(hTmpB, -OtherBlockFraction);
//		Add spectra with R^2 weights
	hTmpA->Reset();
	hTmpA->Add(hAU, Nfactor[0]*daysAU);
	hTmpA->Add(hAM, Nfactor[1]*daysAM);
	hTmpA->Add(hAD, Nfactor[2]*daysAD);
	hTmpA->Scale(1.0 / (daysAU + daysAM + daysAD));
	hTmpB->Reset();
	hTmpB->Add(hBU, Nfactor[0]*daysBU);
	hTmpB->Add(hBM, Nfactor[1]*daysBM);
	hTmpB->Add(hBD, Nfactor[2]*daysBD);
	hTmpB->Scale(1.0 / (daysBU + daysBM + daysBD));
//		Draw
	hAB->Divide(hTmpA, hTmpB);
	hAB->SetLineColor(kBlue);
	hAB->Write();	
	hAB->SetMinimum(min);
	hAB->SetMaximum(max);
	hAB->GetYaxis()->SetLabelSize(0.08);
	hAB->GetXaxis()->SetRange(hAB->FindBin(ERange[0]), hAB->FindBin(ERange[1]));
	hAB->Fit("pol1", "", "", ERange[0], ERange[1]);
//		Clean
	delete hAU;
	delete hAM;
	delete hAD;
	delete hBU;
	delete hBD;
	delete hBM;
	delete hTmpA;
	delete hTmpB;
}

void draw_period_ratios(int m1, int m2, const char *title, double min=0.7, double max=1.2)
{
	char nameA[64];
	char nameB[64];
	char nameR[64];
	TH1D *hRUp;
	TH1D *hRMid;
	TH1D *hRDown;
	TH1D *hA;
	TH1D *hB;
	
	sprintf(nameA, "hUp_%d", 1 << m1);
	sprintf(nameB, "hUp_%d", 1 << m2);
	hA = (TH1D *) gROOT->FindObject(nameA);
	hB = (TH1D *) gROOT->FindObject(nameB);
	if (!hA || !hB) {
		printf("Can not find hist: %s or/and %s. Step %s\n", nameA, nameB, title);
		return;
	}
	sprintf(nameR, "hRUp_%d_%d", 1 << m1, 1 << m2);
	hRUp = (TH1D*) hA->Clone(nameR);
	hRUp->SetTitle(title);
	hRUp->Divide(hA, hB);
	hRUp->SetLineColor(kRed);
	hRUp->SetMarkerColor(kRed);
	hRUp->SetMarkerSize(2);
	hRUp->SetMarkerStyle(kFullTriangleUp);
	
	sprintf(nameA, "hMid_%d", 1 << m1);
	sprintf(nameB, "hMid_%d", 1 << m2);
	hA = (TH1D *) gROOT->FindObject(nameA);
	hB = (TH1D *) gROOT->FindObject(nameB);
	if (!hA || !hB) {
		printf("Can not find hist: %s or/and %s. Step %s\n", nameA, nameB, title);
		return;
	}
	sprintf(nameR, "hRMid_%d_%d", 1 << m1, 1 << m2);
	hRMid = (TH1D*) hA->Clone(nameR);
	hRMid->SetTitle(title);
	hRMid->Divide(hA, hB);
	hRMid->SetLineColor(kGreen);
	hRMid->SetMarkerColor(kGreen);
	hRMid->SetMarkerSize(2);
	hRMid->SetMarkerStyle(kFullCircle);
	
	sprintf(nameA, "hDown_%d", 1 << m1);
	sprintf(nameB, "hDown_%d", 1 << m2);
	hA = (TH1D *) gROOT->FindObject(nameA);
	hB = (TH1D *) gROOT->FindObject(nameB);
	if (!hA || !hB) {
		printf("Can not find hist: %s or/and %s. Step %s\n", nameA, nameB, title);
		return;
	}
	sprintf(nameR, "hRDown_%d_%d", 1 << m1, 1 << m2);
	hRDown = (TH1D*) hA->Clone(nameR);
	hRDown->SetTitle(title);
	hRDown->Divide(hA, hB);
	hRDown->SetLineColor(kBlue);
	hRDown->SetMarkerColor(kBlue);
	hRDown->SetMarkerSize(2);
	hRDown->SetMarkerStyle(kFullTriangleDown);
	
	hRUp->SetMinimum(min);
	hRUp->SetMaximum(max);
	hRUp->GetXaxis()->SetRange(hRUp->FindBin(ERange[0]), hRUp->FindBin(ERange[1]));
	hRUp->DrawCopy();
	hRMid->DrawCopy("same");
	hRDown->DrawCopy("same");
	TLegend *lg = new TLegend(0.3, 0.7, 0.5, 0.85);
	lg->AddEntry(hRUp, "Up", "LP");
	lg->AddEntry(hRMid, "Middle", "LP");
	lg->AddEntry(hRDown, "down", "LP");
	
	delete hRUp;
	delete hRMid;
	delete hRDown;
}

void draw_signal_spectra(const char *from, const char *to, double bgScale = 5.6/2.5)
{
	char strs[128];
	char strl[1024];
	TLatex txt;

	sprintf(strs, "hOff_%s_%s", from, to);
	sprintf(strl, "Positron raw spectrum %s-%s;Positron energy, MeV;Events / (day * 125 keV)", from, to);
	TH1D *hN = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hOffCosm_%s_%s", from, to);
	sprintf(strl, "Positron raw spectrum (cosmic) %s-%s;Positron energy, MeV;Events / (day * 125 keV)", from, to);
	TH1D *hC = new TH1D(strs, strl, NEBINS, 0, 16);
	sum_of_raw(hN, hC, "umdrs", 31, from, to);
	hN->Add((TH1D*)gROOT->FindObject("hUp_28"), -0.006);		// subtract blocks 1-3
	TF1 fBgndN("fBgndN", NeutronCorrN, 0, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0, 100);
	hC->Add(&fBgndC, -86.4);
	hC->Scale(bgScale * 0.025);
	hC->Add(&fBgndN, 86.4);
	hN->Write();
	hC->Write();
	hC->SetLineColor(kRed);
	hC->GetYaxis()->SetLabelSize(0.08);
	hC->SetTitleSize(0.08);
	hC->SetMinimum(0);
	hN->SetLineColor(kBlue);
	hN->Draw();
	hC->Draw("same");
	TLegend *lg = new TLegend(0.65, 0.75, 0.90, 0.9);
	lg->AddEntry(hN, "Neutrino", "le");
	sprintf(strs, "Cosmic * %4.1f%%", bgScale * 2.5);
	lg->AddEntry(hC, strs, "le");
	lg->Draw();
	sprintf(strs, "Integral 1-7 MeV = %5.1f events/day", hN->Integral(hN->FindBin(1.001), hN->FindBin(6.999)));
	txt.SetTextColor(kBlue);
	txt.DrawLatex(6, 0.55*hN->GetMaximum(), strs);
	sprintf(strs, "Integral 1-7 MeV = %5.1f events/day", hC->Integral(hN->FindBin(1.001), hN->FindBin(6.999)));
	txt.SetTextColor(kRed);
	txt.DrawLatex(6, 0.45*hN->GetMaximum(), strs);
}

int sum_accidental(TH1D *h, const char *posmask, const char *pfrom = NULL, const char *pto = NULL)
{
	int N;
	TH1D *hConst;
	TH1D *hBgnd;
	int i;
	char str[256];
	double tSum;
	double dt;
	char *ptr;
	int n;

	N = sizeof(positions) / sizeof(positions[0]);
	
	h->Reset();
	tSum = 0;
	for (i=0; i<N; i++) if (is_in_date_range(positions[i].name, pfrom, pto)) {
		ptr = strchr(posmask, positions[i].name[0]);
		if (!(ptr && positions[i].period)) continue;
		sprintf(str, "%s_hSig-rand", positions[i].name);
		hBgnd = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hConst", positions[i].name);
		hConst = (TH1D*) fData->Get(str);
		if (!(hConst && hBgnd)) continue;
		dt = hConst->GetBinContent(1) / 1000.0;	// seconds * 10^3
		tSum += dt;
		h->Add(hBgnd);
	}
	if (tSum == 0) return 0;
	n = h->Integral();
	h->Scale(86.4 / tSum);
	return n;
}

void draw_accidental(const char *pfrom = NULL, const char *pto = NULL)
{
	char strl[1024];
	char strs[128];
	double val, err;
	int n;

	sprintf(strs, "AccUp_%s_%s", pfrom, pto);
	sprintf(strl, "Accidental background spectrum %s-%s, UP;Positron energy, MeV;Events / (day * 125 keV)", 
		pfrom, pto);
	TH1D *hUp = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_accidental(hUp, "u", pfrom, pto);
	hUp->Write();
	hUp->SetLineColor(kRed);
	hUp->SetFillColor(kRed-10);
	hUp->GetYaxis()->SetLabelSize(0.08);
	sprintf(strl, "Accidental background spectrum %s-%s", pfrom, pto);
	hUp->SetTitle(strl);
	hUp->Draw("hist,e");
	val = hUp->IntegralAndError(hUp->FindBin(1.001), hUp->FindBin(7.999), err);
	sprintf(strs, "  Up: %d events %5.0f #pm%4.0f / day", n, val, err);
	TLegend *lg = new TLegend(0.35, 0.65, 0.9, 0.9);
	lg->AddEntry(hUp, strs, "l");

	sprintf(strs, "hAccMid_%s_%s", pfrom, pto);
	sprintf(strl, "Accidental background spectrum %s-%s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", 
		pfrom, pto);
	TH1D *hMid = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_accidental(hMid, "m", pfrom, pto);
	hMid->Write();
	hMid->SetLineColor(kGreen);
	hMid->SetFillColor(kGreen-10);
	hMid->Draw("same,hist,e");
	val = hMid->IntegralAndError(hMid->FindBin(1.001), hMid->FindBin(7.999), err);
	sprintf(strs, " Mid: %d events %5.0f #pm%4.0f / day", n, val, err);
	lg->AddEntry(hMid, strs, "l");

	sprintf(strs, "hAccDown_%s_%s", pfrom, pto);
	sprintf(strl, "Accidental background  spectrum %s-%s, DOWN;Positron energy, MeV;Events / (day * 125 keV)",
		pfrom, pto);
	TH1D *hDown = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_accidental(hDown, "d", pfrom, pto);
	hDown->Write();
	hDown->SetLineColor(kBlue);
	hDown->SetFillColor(kBlue-10);
	hDown->Draw("same,hist,e");
	val = hDown->IntegralAndError(hDown->FindBin(1.001), hDown->FindBin(7.999), err);
	sprintf(strs, "Down: %d events %5.0f #pm%4.0f / day", n, val, err);
	lg->AddEntry(hDown, strs, "l");
	
	lg->Draw();
}

void danss_calc_ratio_v5(const char *fname, double bgScale = 5.6/2.5)
{
	TCanvas *cv;
	TFile *f;
	TFile *fOut;
	char str[1024];
	TLatex *txt;
	double val, err;
	char pname[1024];
	char rname[2014];
	TLegend *lg;
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1111);
	gStyle->SetTitleXSize(0.08);
	gStyle->SetTitleYSize(0.08);
	gStyle->SetLabelSize(0.08);
	gStyle->SetPadLeftMargin(0.2);
	gStyle->SetPadBottomMargin(0.2);
//	gStyle->SetLineWidth(2);
	
	cv = new TCanvas("CV", "Results", 2400, 1200);
	fData = new TFile(fname);
	if (!fData->IsOpen()) return;
	change_file_suffix(pname, sizeof(pname), fname, ".root", "-calc.pdf");
	change_file_suffix(rname, sizeof(pname), fname, ".root", "-calc.root");
	sprintf(str, "%s[", pname);
	cv->Print(str);
	fOut = new TFile(rname, "RECREATE");
	fOut->cd();
	txt = new TLatex();
	
	printf("Spectra pages\n");
//	Page 1:	Spectra All
	draw_spectra_page(cv, "Apr 16-Jan 19", 0x1E, bgScale);
	cv->Print(pname);
	
//	Page 1a:	Spectra All but April-June 16.
	draw_spectra_page(cv, "Oct 16-Jan 19", 0x1C, bgScale);
	cv->Print(pname);

//	Page 1b:	before Sep17
	draw_spectra_pagel(cv, "Apr 16-Sep 17", "01.04.16", "30.09.17", bgScale);
	cv->Print(pname);

//	Page 1c:	After Sep17
	draw_spectra_pagel(cv, "Sep 17-Jan19", "01.10.17", "09.01.19", bgScale);
	cv->Print(pname);
	
//	3 months Before reactor shut down
	draw_spectra_pagel(cv, "Apr-July 17", "05.04.17", "07.07.17", bgScale);
	cv->Print(pname);

//	3 months After reactor on + 1 month
	draw_spectra_pagel(cv, "Oct-Dec 17", "20.09.17", "20.12.17", bgScale);
	cv->Print(pname);

//	Page 2:	Spectra April-June
	draw_spectra_page(cv, "April-June 16", 2, bgScale);
	cv->Print(pname);

//	Page 3:	Spectra October-to the end of campaign
	draw_spectra_page(cv, "Oct 16-Jul 17", 4, bgScale);
	cv->Print(pname);

//	Page 4:	Spectra Campaign 5 till upgrade
	draw_spectra_page(cv, "Aug 17-Mar 18", 8, bgScale);
	cv->Print(pname);

//	Page 5:	Spectra March 18-January 19
	draw_spectra_page(cv, "May-Jan 19", 0x10, bgScale);
	cv->Print(pname);

//********* two-months periods spectra pages (scan...)
	draw_spectra_pagel(cv, "Apr-Jun 16", "21.04.16", "07.06.16", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Oct-Nov 16", "30.09.16", "01.12.16", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Dec 16-Jan 17", "02.12.16", "01.02.17", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Feb-Mar 17", "06.02.17", "01.04.17", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Apr-May 17", "03.04.17", "01.06.17", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Jun-Jul 17", "02.06.17", "08.07.17", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Aug-Oct 17", "19.08.17", "31.10.17", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Nov-Dec 17", "01.11.17", "31.12.17", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Jan-Mar 18", "01.01.18", "08.03.18", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "May-Jun 18", "04.05.18", "01.07.18", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Jul-Aug 18", "02.07.18", "01.09.18", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Sep-Oct 18", "03.09.18", "01.11.18", bgScale);
	cv->Print(pname);
	draw_spectra_pagel(cv, "Nov 18-Jan 19", "02.11.18", "06.01.19", bgScale);
	cv->Print(pname);
//*********

	printf("Sum spectra page\n");
//	Page 6: Total Spectrum (all positions)
	cv->Clear();
	cv->Divide(3, 2);

	cv->cd(1);
	TH1D *hSum = new TH1D("hSum", "Positron spectrum Apr 16 - Jan 19;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	TH1D *hSumSub = new TH1D("hSumSub", "Background spectrum Apr 16 - Jan 19;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	sum_of_spectra(hSum, hSumSub, "umdrs", 0x1E, bgScale);
	hSum->Write();
	hSumSub->Write();
	hSum->Draw("e");

	cv->cd(2);
	TH1D *hSum1 = new TH1D("hSum1", "Positron spectrum Apr-Jun 16;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	TH1D *hSum1Sub = new TH1D("hSumAfterSub", "Background spectrum Oct - Dec 17;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	sum_of_spectra(hSum1, hSum1Sub, "umdrs", 2, bgScale);
	hSum1->Write();
	hSum1Sub->Write();
	hSum1->Draw("e");

	cv->cd(3);
	TH1D *hSum2 = new TH1D("hSum2", "Positron spectrum Oct 16-Jul 17;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	TH1D *hSum2Sub = new TH1D("hSum2Sub", "Background spectrum Oct 16-Jul 17;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	sum_of_spectra(hSum2, hSum2Sub, "umdrs", 4, bgScale);
	hSum2->Write();
	hSum2Sub->Write();
	hSum2->Draw("e");

	cv->cd(4);
	TH1D *hSum3 = new TH1D("hSum3", "Positron spectrum Aug 17-Mar 18;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	TH1D *hSum3Sub = new TH1D("hSum3Sub", "Background spectrum Aug 17-Mar 18;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	sum_of_spectra(hSum3, hSum3Sub, "umdrs", 8, bgScale);
	hSum3->Write();
	hSum3Sub->Write();
	hSum3->Draw("e");

	cv->cd(5);
	TH1D *hSum4 = new TH1D("hSum4", "Positron spectrum May-Nov 18;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	TH1D *hSum4Sub = new TH1D("hSum4Sub", "Background spectrum May-Nov 18;Positron energy, MeV;Events / (day * 125 keV)", NEBINS, 0, 16);
	sum_of_spectra(hSum4, hSum4Sub, "umdrs", 0x10, bgScale);
	hSum4->Write();
	hSum4Sub->Write();
	hSum4->Draw("e");

	cv->cd(6);
	hSum2->SetLineColor(kRed);
	hSum3->SetLineColor(kBlue);
	lg = new TLegend(0.6, 0.7, 0.9, 0.8);
	lg->AddEntry(hSum2, "Oct 16 - July 17", "LE");
	lg->AddEntry(hSum3, "Aug 17 - Mar 18", "LE");
	hSum2->Draw("e");
	hSum3->Draw("e,same");
	lg->Draw();

	cv->Update();
	cv->Print(pname);
	
	printf("Ratio pages\n");
//	Page 7:	Down/Up
	cv->Clear();
	cv->Divide(2, 2);

	cv->cd(1);
	draw_single_ratio("hDown_2", "hUp_2", "hDownUp_2", "Ratio Down/Up Apr-Jun 16;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->cd(2);
	draw_single_ratio("hDown_4", "hUp_4", "hDownUp_4", "Ratio Down/Up Oct 16-Jul 17;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->cd(3);
	draw_single_ratio("hDown_8", "hUp_8", "hDownUp_8", "Ratio Down/Up Aug 17-Mar 18;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->cd(4);
	draw_single_ratio("hDown_16", "hUp_16", "hDownUp_16", "Ratio Down/Up May 18-Jan 19;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->Update();
	cv->Print(pname);
	
	cv->Clear();
	cv->Divide(2, 2);

	cv->cd(1);
	draw_single_ratio("hDown_30", "hUp_30", "hDownUp_30", "Ratio Down/Up Apr 16-Jan 19 (Everything);Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->cd(2);
	draw_single_ratio("hDown_28", "hUp_28", "hDownUpAll", "Ratio Down/Up Oct 16-Jan 19 (ALL-Apr16);Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->cd(3);
	draw_single_ratio("hDown_01.04.16_30.09.17", "hUp_01.04.16_30.09.17", "hDownUp_01.04.16_30.09.17", 
		"Ratio Down/Up Apr 16-Sep 17 (PLB period);Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->cd(4);
	draw_single_ratio("hDown_01.10.17_09.01.19", "hUp_01.10.17_09.01.19", "hDownUp_01.10.17_09.01.19", 
		"Ratio Down/Up Oct 17-Jan 19 (After PLB);Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);

	cv->Update();
	cv->Print(pname);

//	Page 7a: Main plot
	cv->Clear();
	draw_single_ratio("hDown_28", "hUp_28", "hDownUp_28", "Ratio Down/Up Oct 16-Jan 19 (ALL-Apr16);Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.65, 0.8);
	cv->Update();
	cv->Print(pname);

//********* two-months periods spectra pages (scan...)
	cv->Clear();
	cv->Divide(2, 2);
	cv->cd(1);
	draw_single_ratio("hDown_21.04.16_07.06.16", "hUp_21.04.16_07.06.16", "hDownUp_21.04.16_07.06.16", 
		"Ratio Down/Up Apr-Jun 16;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(2);
	draw_single_ratio("hDown_30.09.16_01.12.16", "hUp_30.09.16_01.12.16", "hDownUp_30.09.16_01.12.16", 
		"Ratio Down/Up Oct-Nov 16;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(3);
	draw_single_ratio("hDown_02.12.16_01.02.17", "hUp_02.12.16_01.02.17", "hDownUp_02.12.16_01.02.17", 
		"Ratio Down/Up Dec 16-Jan 17;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(4);
	draw_single_ratio("hDown_06.02.17_01.04.17", "hUp_06.02.17_01.04.17", "hDownUp_06.02.17_01.04.17", 
		"Ratio Down/Up Feb-Mar 17;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->Update();
	cv->Print(pname);

	cv->Clear();
	cv->Divide(2, 2);
	cv->cd(1);
	draw_single_ratio("hDown_03.04.17_01.06.17", "hUp_03.04.17_01.06.17", "hDownUp_03.04.17_01.06.17", 
		"Ratio Down/Up Apr-May 17;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(2);
	draw_single_ratio("hDown_02.06.17_08.07.17", "hUp_02.06.17_08.07.17", "hDownUp_02.06.17_08.07.17", 
		"Ratio Down/Up Jun-Jul 17;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(3);
	draw_single_ratio("hDown_19.08.17_31.10.17", "hUp_19.08.17_31.10.17", "hDownUp_19.08.17_31.10.17", 
		"Ratio Down/Up Aug-Oct 17;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(4);
	draw_single_ratio("hDown_01.11.17_31.12.17", "hUp_01.11.17_31.12.17", "hDownUp_01.11.17_31.12.17", 
		"Ratio Down/Up Nov-Dec 17;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->Update();
	cv->Print(pname);

	cv->Clear();
	cv->Divide(2, 2);
	cv->cd(1);
	draw_single_ratio("hDown_01.01.18_08.03.18", "hUp_01.01.18_08.03.18", "hDownUp_01.01.18_08.03.18", 
		"Ratio Down/Up Jan-Mar 18;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(2);
	draw_single_ratio("hDown_04.05.18_01.07.18", "hUp_04.05.18_01.07.18", "hDownUp_04.05.18_01.07.18", 
		"Ratio Down/Up May-Jun 18;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(3);
	draw_single_ratio("hDown_02.07.18_01.09.18", "hUp_02.07.18_01.09.18", "hDownUp_02.07.18_01.09.18", 
		"Ratio Down/Up Jul-Aug 18;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->cd(4);
	draw_single_ratio("hDown_03.09.18_01.11.18", "hUp_03.09.18_01.11.18", "hDownUp_03.09.18_01.11.18", 
		"Ratio Down/Up Sep-Oct 18;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->Update();
	cv->Print(pname);

	cv->Clear();
	cv->Divide(2, 2);
	cv->cd(1);
	draw_single_ratio("hDown_02.11.18_06.01.19", "hUp_02.11.18_06.01.19", "hDownUp_02.11.18_06.01.19", 
		"Ratio Down/Up Nov 18-Jan 19;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", 0.6, 0.9);
	cv->Update();
	cv->Print(pname);
//*********

//	Page 8:	Mid/Up
	cv->Clear();
	cv->Divide(2, 2);

	cv->cd(1);
	draw_single_ratio("hMid_2", "hUp_2", "hMidUp_2", "Ratio Mid/Up Apr-Jun 16;Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->cd(2);
	draw_single_ratio("hMid_4", "hUp_4", "hMidUp_4", "Ratio Mid/Up Oct 16-Jul 17;Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->cd(3);
	draw_single_ratio("hMid_8", "hUp_8", "hMidUp_8", "Ratio Mid/Up Aug 17-Mar 18;Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->cd(4);
	draw_single_ratio("hMid_16", "hUp_16", "hMidUp_16", "Ratio Mid/Up May 18-Jan 19;Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->Update();
	cv->Print(pname);
	
	cv->Clear();
	cv->Divide(2, 2);

	cv->cd(1);
	draw_single_ratio("hMid_30", "hUp_30", "hMidUp_30", "Ratio Mid/Up Apr 16-Jan 19 (Everything);Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->cd(2);
	draw_single_ratio("hMid_28", "hUp_28", "hMidUpAll", "Ratio Mid/Up Oct 16-Jan 19 (ALL-Apr16);Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->cd(3);
	draw_single_ratio("hMid_01.04.16_30.09.17", "hUp_01.04.16_30.09.17", "hMidUp_01.04.16_30.09.17", 
		"Ratio Mid/Up Apr 16-Sep 17 (PLB period);Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->cd(4);
	draw_single_ratio("hMid_01.10.17_09.01.19", "hUp_01.10.17_09.01.19", "hMidUp_01.10.17_09.01.19", 
		"Ratio Mid/Up Oct 17-Jan 19 (After PLB);Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", 0.6, 0.9);

	cv->Update();
	cv->Print(pname);

//	Double ratios
	cv->Clear();
	cv->Divide(3, 2);

	cv->cd(1);
	draw_single_ratio("hDownUp_2", "hDownUp_30", "hDownUp2_2_30", "Double ratio Down/Up Apr-Jun 16 / All;Positron energy, MeV", 0.7, 1.3);

	cv->cd(2);
	draw_single_ratio("hDownUp_4", "hDownUp_30", "hDownUp2_4_30", "Double ratio Down/Up Oct 16 - Jul 17 / All;Positron energy, MeV", 0.7, 1.3);

	cv->cd(3);
	draw_single_ratio("hDownUp_8", "hDownUp_30", "hDownUp2_8_30", "Double ratio Down/Up Aug 17 - Mar 18 / All;Positron energy, MeV", 0.7, 1.3);

	cv->cd(4);
	draw_single_ratio("hDownUp_16", "hDownUp_30", "hDownUp2_16_30", "Double ratio Down/Up May 18 - Jan 19 / All;Positron energy, MeV", 0.7, 1.3);

	cv->cd(5);
	draw_single_ratio("hDownUp_01.04.16_30.09.17", "hDownUp_30", "hDownUp2_PLB_30", "Double ratio Down/Up PLB / All;Positron energy, MeV", 0.7, 1.3);

	cv->cd(6);
	draw_single_ratio("hDownUp_01.04.16_30.09.17", "hDownUp_01.10.17_09.01.19", "hDownUp2_PLB_postPLB", "Double ratio Down/Up PLB / post PLB;Positron energy, MeV", 0.7, 1.3);

	cv->Update();
	cv->Print(pname);

//	Page 9: Period ratios
	cv->Clear();
	cv->Divide(2, 2);

	cv->cd(1);
	draw_single_ratio("hSum2", "hSum4", "hRatio_24", "Unnormalized Ratio Oct 16-Jul 17/May 18 -Jan 19;Positron energy, MeV", 0.8, 1.4);

	cv->cd(2);
	draw_single_ratio("hSum3", "hSum4", "hRatio_34", "Unnormalized Ratio Aug 17-Mar 18/May-Jan 19;Positron energy, MeV", 0.8, 1.4);

	cv->cd(3);
	draw_single_ratio("hSum2", "hSum3", "hRatio_23", "Unnormalized Ratio Oct 16-Jul 17/Aug 17-Mar 18;Positron energy, MeV", 0.8, 1.4);

	cv->cd(4);
	draw_single_ratio("hSum4", "hSum", "hRatio_4All", "Unnormalized Ratio May 18-Jan 19/apr 16-Jan 19;Positron energy, MeV", 0.8, 1.4);

	cv->Update();
	cv->Print(pname);

//	Page 9a: after shutdown / before shutdown - use proper periods.
	cv->Clear();
	draw_normalized_ratio("20.09.17", "20.12.17", "05.04.17", "07.07.17", "hNRatioAfter2Before", 
		"Normalized ratio after shutdown / before shutdown;Positron energy, MeV", 0.9, 1.4, bgScale);
	cv->Update();
	cv->Print(pname);

//	Page 9b: after shutdown / before shutdown
	cv->Clear();
	draw_period_ratios(2, 3, "Ratio after shutdown / before shutdown;Positron energy, MeV");
	cv->Update();
	cv->Print(pname);

//	Page 9c: after shutdown / before shutdown
	cv->Clear();
	draw_period_ratios(4, 3, "Ratio after upgrade / before upgrade;Positron energy, MeV");
	cv->Update();
	cv->Print(pname);

	printf("Raw pages\n");
//	Page 10: Without background subtraction
	cv->Clear();
	cv->Divide(2, 2);

	cv->cd(1);
	draw_tail_hist("all", "umd");
	
	cv->cd(2);
	draw_tail_hist("up", "u");

	cv->cd(3);
	draw_tail_hist("middle", "m");

	cv->cd(4);
	draw_tail_hist("down", "d");
	
	cv->Update();
	cv->Print(pname);

//		Reactor Off
	cv->Clear();
	draw_signal_spectra("10.07.17", "18.08.17", bgScale);
	cv->Update();
	cv->Print(pname);

//		Accidental background
	cv->Clear();
	draw_accidental();
	cv->Update();
	cv->Print(pname);

//****************	Write file	*****************//
	sprintf(str, "%s]", pname);
	cv->Print(str);
	fData->Close();
	delete cv;
	fOut->Close();
}
