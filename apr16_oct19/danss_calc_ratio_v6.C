//	Global parameters
#define NEBINS 128
#define MAXPOSITIONS 1000
//#include "positions.h"
struct PositionsStruct {
	char name[32];
	int first;
	int last;
	int period;
	char chpower;
	double power;
	double bgnd;
	double total;
	double dead;
	double eff;
} positions[MAXPOSITIONS];
int PosEntries;
struct StatStruct {
	double Cnt;
	double days;
	double power;
	double dead;
	double eff;
};

#define MAXRANGE 5
struct PeriodsStruct {
	const char *name;
	const char *title;
	int pwr;	// 0 - ZERO, 1 - FULL, 2 - doesn't matter
	const int range[MAXRANGE][2];
};

const struct PeriodsStruct PeriodList[] = {
	{"All", "April 16 - Oct 19 (everything)", 1, {{2307, 71210}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Main", "Oct 16 - Oct 19 (main)", 1, {{6010, 71210}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Main1", "Oct 16 - Oct 19 without Apr - Jul 17 (main1)", 1, {{6010, 17848}, {24776, 71210}, {0, 0}, {0, 0}, {0, 0}}},
	{"Cmp4", "Oct 16 - Jul 17 (campaign 4)", 1, {{6010, 23059}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Cmp5", "Aug 17 - Jan 19 (campaign 5)", 1, {{24776, 54595}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Cmp6", "Feb 19 - Oct 19 (campaign 6)", 1, {{56626, 71210}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"PLB", "April 16 - Sep 17 (PLB)", 1, {{2307, 27061}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"PPLB", "Oct 17 - Oct 19 (post PLB)", 1, {{27062, 71210}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Stg1", "Oct 16 - Mar 18 (before grounding)", 1, {{6010, 36402}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Stg2", "May 18 - Oct 19 (after grounding)", 1, {{38048, 71210}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Off1", "Jul - Aug 17 (reactor off 1)", 0, {{21188, 24705}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Off2", "Jan - Feb 19 (reactor off 2)", 0, {{54782, 56290}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Off", "Jul - Aug 17 and Jan - Feb 19 (reactor off)", 0, {{21188, 24705}, {54782, 56290}, {0, 0}, {0, 0}, {0, 0}}},
	{"End4", "Apr - Jul 17 (end of campaign 4)", 1, {{17849, 23059}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Beg5", "Sep - Dec 17 (begin of campaign 5)", 1, {{26437, 31697}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"End5", "Oct 18 - Jan 19 (end of campaign 5)", 1, {{48438, 54595}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Beg6", "Mar - Jun 19 (begin of campaign 6)", 1, {{58344, 64261}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"End", "Apr - Jul 17 and Oct 18 - Jan 19 (ends of campaigns)", 1, {{17849, 23059}, {48438, 54595}, {0, 0}, {0, 0}, {0, 0}}},
	{"Beg", "Sep - Dec 17 and Mar - Jun 19 (begins of campaigns)", 1, {{26437, 31697}, {58344, 64261}, {0, 0}, {0, 0}, {0, 0}}},
	{"xMar17", "Main - Mar 17", 1, {{6010, 14359}, {17849, 71210}, {0, 0}, {0, 0}, {0, 0}}},
	{"xApr17", "Main - Apr 17", 1, {{6010, 17848}, {19639, 71210}, {0, 0}, {0, 0}, {0, 0}}},
	{"xMay17", "Main - May 17", 1, {{6010, 19638}, {21412, 71210}, {0, 0}, {0, 0}, {0, 0}}},
	{"xJun17", "Main - Jun 17", 1, {{6010, 21411}, {23060, 71210}, {0, 0}, {0, 0}, {0, 0}}},
	{"Mar17", "Mar 17", 1, {{14360, 17848}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Apr17", "Apr 17", 1, {{17849, 19638}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"May17", "May 17", 1, {{19639, 21411}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{"Jun17", "Jun 17", 1, {{21412, 23059}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}}
};

TFile *fData;
// Fast neutron corrections 
const char *NeutronCorrN = "0.003373-0.000144*x";
const char *NeutronCorrC = "0.01078-0.000417*x";
const double rOffCosmCorr = 0.91;
const double OtherBlockFraction = 0.0060;	// Neutrino fraction of other blocks. Distances to other reactors: 160, 336 and 478 m
const double ERange[2] = {0.751, 6.999};

int GlobalPeriod(int run)
{
	if (run < 5000) return 1;		// April-June 2016
	if (run < 24000) return 2;		// October 2016 - July 2017: till the end of Campaign 4
	if (run < 37000) return 3;		// August 2017 - March 2018: Campaign 5 till detector repair
	if (run < 56000) return 4;		// May 2018 - January 2019: after detector repair till the end of  Campaign 5
	return 5;				// Campaign 6
}

int readPositions(const char *fname)
{
	FILE *f;
	char str[1024];
	char *ptr;
	
	PosEntries = 0;
	f = fopen(fname, "rt");
	if (!f) return 0;
	for(;;) {
		ptr = fgets(str, sizeof(str), f);
		if (!ptr) break;
		ptr = strtok(str, " \t");
		if (!ptr || ptr[0] == '*') continue;
		if (!strchr("udmrs", ptr[0])) continue;
		strncpy(positions[PosEntries].name, ptr, sizeof(positions[PosEntries].name));
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].first = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].last = strtol(ptr, NULL, 10);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].chpower = ptr[0];
		positions[PosEntries].period = (ptr[0] == 'F') ? GlobalPeriod(positions[PosEntries].first) : 0;
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].power = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].bgnd = 1.55 * strtod(ptr, NULL);	// we use this coef to keep to the old definition
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].total = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].dead = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].eff = strtod(ptr, NULL);
		PosEntries++;
		if (PosEntries == MAXPOSITIONS) {
			printf("Warning ! MAXPOSITIONS = %d reached\n", MAXPOSITIONS);
			break;
		}
	}
	fclose(f);
	return PosEntries;
}

int GetPrefix(const char *name)
{
	int i;
	const char *List[] = {"hUp_", "hMid_", "hDown_"};
	for (i = 0; i < sizeof(List) / sizeof(List[0]); i++) if (!strncmp(name, List[i], strlen(List[i]))) break;
	if (i == sizeof(List) / sizeof(List[0])) return 0;
	return i+1;
}

//	Try to find scale correction from power, efficiency, dead time for
//	histogramms ratio A/B. Return 1.0 if this is not an option.
double GetEffScale(const char *nameA, const char *nameB)
{
	char str[1024];
	char * ptr;
	int iA, iB;
	
	iA = GetPrefix(nameA);
	if (!iA) return 1.0;
	iB = GetPrefix(nameB);
	if (!iB) return 1.0;
	ptr = strchr(nameA, '_');
	if (!ptr) return 1.0;
	sprintf(str, "hConst%s", ptr);
	TH1D *hCA = (TH1D*) gROOT->FindObject(str);
	if (!hCA) return 1.0;
	ptr = strchr(nameB, '_');
	if (!ptr) return 1.0;
	sprintf(str, "hConst%s", ptr);
	TH1D *hCB = (TH1D*) gROOT->FindObject(str);
	if (!hCB) return 1.0;
	
	double PwrA = hCA->GetBinContent(6 + iA);
	double PwrB = hCB->GetBinContent(6 + iB);
	double DeadA = hCA->GetBinContent(9 + iA);
	double DeadB = hCB->GetBinContent(9 + iB);
	double EffA = hCA->GetBinContent(12 + iA);
	double EffB = hCB->GetBinContent(12 + iB);
	
	double k = PwrB * EffB * (1 - DeadB) / (PwrA * EffA * (1 - DeadA));
	return k;
}

void change_file_suffix(char *to, int len, const char *from, const char *where, const char *what)
{
	char *ptr;
	
	strncpy(to, from, len - strlen(what));
	ptr = strstr(to, where);
	if (ptr) *ptr = '\0';
	strcat(to, what);
}

int is_position_in_period(struct PositionsStruct *pos, const struct PeriodsStruct *period)
{
	int i;
	if (!period) return 1;
	if (period->pwr == 0 && pos->chpower != 'Z') return 0;
	if (period->pwr == 1 && pos->chpower != 'F') return 0;
	for (i=0; i<MAXRANGE; i++) {
		if (pos->first >= period->range[i][0] && pos->last <= period->range[i][1]) return 1;
	}
	return 0;
}

const struct PeriodsStruct *period_by_name(const char *name)
{
	int i;
	for (i=0; i < sizeof(PeriodList) / sizeof(PeriodList[0]); i++) 
		if (!strcmp(name, PeriodList[i].name)) return &PeriodList[i];
	return NULL;
}

//  Make sum of spectra over large number of points
double sum_of_spectra(TH1D *hSum, TH1D *hSub, const char *posmask, const struct PeriodsStruct *period, 
	double bgScale = 1.0, double Nscale = 1.0, struct StatStruct *stat = NULL)
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
	double Cnt;
	double days;
	double power;
	double total;
	double dead;
	double eff;
	TF1 fBgndN("fBgndN", NeutronCorrN, 0, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0, 100);

	power = total = dead = eff = 0;
	hSum->Reset();
	hSub->Reset();
	tSum = 0;
	if (stat) memset(stat, 0, sizeof(struct StatStruct));
	for (i=0; i<PosEntries; i++) {
		ptr = strchr(posmask, positions[i].name[0]);
		if (!(ptr && is_position_in_period(&positions[i], period))) continue;
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
		hSub->Add(&fBgndN, dt * Nscale);
		hSub->Add(&fBgndC, -dt * positions[i].bgnd * bgScale * Nscale);
		power += positions[i].power * positions[i].total;
		eff += positions[i].eff * positions[i].total;
		total += positions[i].total;
		dead += positions[i].dead;
	}
	hSum->Add(hSub, -1);
	days = tSum / 86.4;
	if (tSum == 0) return 0;
	Cnt = hSum->Integral();
	hSum->Scale(86.4 / tSum);
	hSub->Scale(86.4 / tSum);
	if (total > 0 && stat) {
		stat->days = days;
		stat->Cnt = Cnt;
		stat->power = power / total;
		stat->dead = dead / total;
		stat->eff = eff / total;
	}
	return Cnt;
}

void sum_of_raw(TH1D *hSumSig, TH1D *hSumBgnd, const char *posmask, const struct PeriodsStruct *period)
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
		if (!(ptr && is_position_in_period(&positions[i], period))) continue;
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

void draw_spectra_page(TCanvas *cv, const struct PeriodsStruct *period, double bgScale, double Nscale = 1.0)
{
	TLegend *lg;
	char strs[128];
	char strl[1024];
	double val, err;
	double valb[3], errb[3];
	double Cnt;
	int i;
	struct StatStruct stat[3];
	const char *ConstLabels[] = {
		"UpCnt", "MidCnt", "DownCnt", "UpDays", "MidDays", "DownDays",
		"UpPwr", "MidPwr", "DownPwr", "UpDead", "MidDead", "DownDead",
		"UpEff", "MidEff", "DownEff"
	};
	int NL;
	TLatex txt;
	TH1D *hTmp;

	cv->Clear();
	lg = new TLegend(0.35, 0.65, 0.9, 0.9);
	sprintf(strs, "hUp_%s", period->name);
	sprintf(strl, "Positron spectrum %s, UP;Positron energy, MeV;Events / (day * 125 keV)", period->title);
	TH1D *hUp = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hUpSub_%s", period->name);
	sprintf(strl, "Background spectrum %s, UP;Positron energy, MeV;Events / (day * 125 keV)", period->title);
	TH1D *hUpSub = new TH1D(strs, strl, NEBINS, 0, 16);
	Cnt = sum_of_spectra(hUp, hUpSub, "u", period, bgScale, Nscale, &stat[0]);
	if (period->pwr == 1) {
		hTmp = (TH1D *) hUp->Clone("hTmp");	// keep to subtract blocks #321
	} else {
		hTmp = (TH1D *) gROOT->FindObject("hUp_Main");
		if (hTmp) hTmp =(TH1D *) hTmp->Clone("hTmp");
	}
	if (!hTmp) printf("Can not subtract OtherBlockFraction - no hUp_Main!\n");
	hUp->Add(hTmp, -OtherBlockFraction);
	stat[0].Cnt -= OtherBlockFraction * hTmp->Integral() * stat[0].days;
	Cnt -= OtherBlockFraction * hTmp->Integral() * stat[0].days;
	hUp->Write();
	hUpSub->Write();
	hUp->SetLineColor(kRed);
	hUp->SetFillColor(kRed-10);
	hUp->GetYaxis()->SetLabelSize(0.08);
	hUp->SetTitle(period->title);
	hUp->Draw("hist,e");
	val = hUp->IntegralAndError(hUp->FindBin(1.001), hUp->FindBin(7.999), err);
		OtherBlockFraction * hTmp->Integral(hUp->FindBin(1.001), hUp->FindBin(7.999));
	sprintf(strs, "  Up: %7.0f events %6.1f #pm%5.1f / day", stat[0].Cnt, val, err);
	lg->AddEntry(hUp, strs, "l");

	sprintf(strs, "hMid_%s", period->name);
	sprintf(strl, "Positron spectrum %s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", period->title);
	TH1D *hMid = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hMidSub_%s", period->name);
	sprintf(strl, "Background spectrum %s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", period->title);
	TH1D *hMidSub = new TH1D(strs, strl, NEBINS, 0, 16);
	Cnt += sum_of_spectra(hMid, hMidSub, "m", period, bgScale, Nscale, &stat[1]);
	hMid->Add(hTmp, -OtherBlockFraction);
	stat[1].Cnt -= OtherBlockFraction * hTmp->Integral() * stat[1].days;
	Cnt -= OtherBlockFraction * hTmp->Integral() * stat[1].days;
	hMid->Write();
	hMidSub->Write();
	hMid->SetLineColor(kGreen);
	hMid->SetFillColor(kGreen-10);
	hMid->Draw("same,hist,e");
	val = hMid->IntegralAndError(hUp->FindBin(1.001), hMid->FindBin(7.999), err);
	sprintf(strs, " Mid: %7.0f events %6.1f #pm%5.1f / day", stat[1].Cnt, val, err);
	lg->AddEntry(hMid, strs, "l");

	sprintf(strs, "hDown_%s", period->name);
	sprintf(strl, "Positron spectrum %s, DOWN;Positron energy, MeV;Events / (day * 125 keV)", period->title);
	TH1D *hDown = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hDownSub_%s", period->name);
	sprintf(strl, "Background spectrum %s, DOWN;Positron energy, MeV;Events / (day * 125 keV)", period->title);
	TH1D *hDownSub = new TH1D(strs, strl, NEBINS, 0, 16);
	Cnt += sum_of_spectra(hDown, hDownSub, "d", period, bgScale, Nscale, &stat[2]);
	hDown->Add(hTmp, -OtherBlockFraction);
	stat[2].Cnt -= OtherBlockFraction * hTmp->Integral() * stat[2].days;
	Cnt -= OtherBlockFraction * hTmp->Integral() * stat[2].days;
	hDown->Write();
	hDownSub->Write();
	hDown->SetLineColor(kBlue);
	hDown->SetFillColor(kBlue-10);
	hDown->Draw("same,hist,e");
	val = hDown->IntegralAndError(hUp->FindBin(1.001), hDown->FindBin(7.999), err);
	sprintf(strs, "Down: %7.0f events %6.1f #pm%5.1f / day", stat[2].Cnt, val, err);
	lg->AddEntry(hDown, strs, "l");

	hUpSub->SetLineColor(kBlack);
	hUpSub->SetFillColor(kGray);
	hUpSub->Draw("same,hist,e");
	valb[0] = hUpSub->IntegralAndError(hUpSub->FindBin(1.001), hUpSub->FindBin(7.999), errb[0]);
	valb[1] = hMidSub->IntegralAndError(hMidSub->FindBin(1.001), hMidSub->FindBin(7.999), errb[1]);
	valb[2] = hDownSub->IntegralAndError(hDownSub->FindBin(1.001), hDownSub->FindBin(7.999), errb[2]);
	sprintf(strs, "Bgnd Up: %5.1f #pm%3.1f (Mid: %5.1f #pm%3.1f, Down: %5.1f #pm%3.1f)", 
		valb[0], errb[0], valb[1], errb[1], valb[2], errb[2]);
	lg->AddEntry(hUpSub, strs, "l");

	hUp->Draw("axis,same");
	lg->Draw();
	sprintf(strs, "%7.0f events", Cnt);
	txt.DrawLatexNDC(0.6, 0.5, strs);
	cv->Update();
	
	sprintf(strs, "hConst_%s", period->name);
	sprintf(strl, "Statistics: %s", period->title);
	NL = sizeof(ConstLabels)/sizeof(char *);
	TH1D *hConst = new TH1D(strs, strl, NL, 0, NL);
	for (i=0; i<NL; i++) hConst->GetXaxis()->SetBinLabel(i+1, ConstLabels[i]);
	for (i=0; i<3; i++) {
		hConst->SetBinContent(i+1, stat[i].Cnt);
		hConst->SetBinContent(i+4, stat[i].days);
		hConst->SetBinContent(i+7, stat[i].power);
		hConst->SetBinContent(i+10, stat[i].dead);
		hConst->SetBinContent(i+13, stat[i].eff);
	}
	hConst->Write();

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
	double val[2], err[2];

	lg = new TLegend(0.35, 0.65, 0.60, 0.9);
	sprintf(strs, "hTailN_%s", posmask);
	sprintf(strl, "Positron raw spectrum %s;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hN = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hTailC_%s", posmask);
	sprintf(strl, "Muon raw spectrum %s;Positron energy, MeV;Events / (day * 125 keV)", title);
	TH1D *hC = new TH1D(strs, strl, NEBINS, 0, 16);
	sum_of_raw(hN, hC, posmask, period_by_name("Main"));	// main period
	hN->Write();
	hC->Write();
	hC->SetLineColor(kRed);
	hC->GetYaxis()->SetLabelSize(0.08);
	hC->SetTitle(title);
	hC->SetTitleSize(0.08);
	hC->GetXaxis()->SetRange(hC->FindBin(8.001), hC->FindBin(15.999));
	hC->SetMinimum(0);
	hC->Fit("pol1", "", "", 10, 16);
	val[1] = hC->GetFunction("pol1")->Integral(1, 8) / hC->GetBinWidth(1);
	err[1] = hC->GetFunction("pol1")->IntegralError(1, 8) / hC->GetBinWidth(1);
	hN->SetLineColor(kBlue);
	hN->Fit("pol1", "", "sames", 10, 16);
	val[0] = hN->GetFunction("pol1")->Integral(1, 8) / hN->GetBinWidth(1);
	err[0] = hN->GetFunction("pol1")->IntegralError(1, 8) / hN->GetBinWidth(1);
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
	sprintf(strs, "Neutrino 1-8 MeV=%5.1f#pm%3.1f evt/day", val[0], err[0]);
	txt.SetTextColor(kBlue);
	txt.DrawLatex(11, 0.5*hC->GetMaximum(), strs);
	sprintf(strs, "Cosmic 1-8 MeV=%5.1f#pm%3.1f evt/day", val[1], err[1]);
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
	hAB->Scale(GetEffScale(nameA, nameB));
	hAB->SetLineColor(kBlue);
	hAB->Write();	
	hAB->SetMinimum(min);
	hAB->SetMaximum(max);
	hAB->GetYaxis()->SetLabelSize(0.08);
	hAB->GetXaxis()->SetRange(hAB->FindBin(ERange[0]), hAB->FindBin(ERange[1]));
	hAB->GetYaxis()->SetTitle("");
	hAB->Fit("pol0", "", "", ERange[0], ERange[1]);
}

void draw_single_period_ratio(const char *prefix, const char *nameA, const char *nameB)
{
	const struct PeriodsStruct *periodA;
	const struct PeriodsStruct *periodB;
	char histA[256];
	char histB[256];
	char name[256];
	char title[256];
	TH1 * hA;
	TH1 * hB;

	periodA = period_by_name(nameA);
	periodB = period_by_name(nameB);
	if (!periodA || !periodB) {
		printf("Can not find period %s or/and %s.\n", nameA, nameB);
		return;
	}

	sprintf(histA, "%s_%s", prefix, nameA);
	sprintf(histB, "%s_%s", prefix, nameB);
	sprintf(name, "%s_%s_2_%s", prefix, nameA, nameB);

	hA = (TH1D*) gROOT->FindObject(histA);
	hB = (TH1D*) gROOT->FindObject(histB);
	if (!hA || !hB) {
		printf("Can not find histogram %s or/and %s.\n", histA, histB);
		return;
	}
	if (hA->Integral() < 10 || hB->Integral() < 10) return;	// nothing to draw
	
	sprintf(title, "Ratio %s: %s / %s;Positron energy, MeV;#frac{N_{%s}}{N_{%s}}", 
		prefix, periodA->title, periodB->title, nameA, nameB);
	draw_single_ratio(histA, histB, name, title, 0.75, 1.25);
}

void draw_normalized_ratio(const struct PeriodsStruct *periodA, const struct PeriodsStruct *periodB,
	const char *name, const char *title, double min=0.6, double max=1.2, double bgScale = 1.0, double Nscale = 1.0)
{
	char strs[128];
	struct StatStruct daysAU, daysAM, daysAD, daysBU, daysBM, daysBD;
//	int i;
//	const char posmask[3][3] = {"u", "m", "d"};
	const double Nfactor[3] = {1.0, (11.88*11.88)/(10.89*10.89), (12.88*12.88)/(10.89*10.89)};
//	const double OtherBlockFraction = 0.0060;	// Distances to other reactors: 160, 336 and 478 m
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
	sum_of_spectra(hAU, hAUb, "u", periodA, bgScale, Nscale, &daysAU);
	sum_of_spectra(hAM, hAMb, "m", periodA, bgScale, Nscale, &daysAM);
	sum_of_spectra(hAD, hADb, "d", periodA, bgScale, Nscale, &daysAD);
	sum_of_spectra(hBU, hBUb, "u", periodB, bgScale, Nscale, &daysBU);
	sum_of_spectra(hBM, hBMb, "m", periodB, bgScale, Nscale, &daysBM);
	sum_of_spectra(hBD, hBDb, "d", periodB, bgScale, Nscale, &daysBD);
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
	hTmpA->Add(hAU, Nfactor[0]*daysAU.days);
	hTmpA->Add(hAM, Nfactor[1]*daysAM.days);
	hTmpA->Add(hAD, Nfactor[2]*daysAD.days);
	hTmpA->Scale(1.0 / (daysAU.days + daysAM.days + daysAD.days));
	hTmpB->Reset();
	hTmpB->Add(hBU, Nfactor[0]*daysBU.days);
	hTmpB->Add(hBM, Nfactor[1]*daysBM.days);
	hTmpB->Add(hBD, Nfactor[2]*daysBD.days);
	hTmpB->Scale(1.0 / (daysBU.days + daysBM.days + daysBD.days));
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
	delete hAUb;
	delete hAMb;
	delete hADb;
	delete hBUb;
	delete hBDb;
	delete hBMb;
	delete hTmpA;
	delete hTmpB;
}

void draw_period_ratios(const char *suffixA, const char* suffixB, const char *title, double min=0.7, double max=1.2)
{
	char nameA[64];
	char nameB[64];
	char nameR[64];
	TH1D *hRUp;
	TH1D *hRMid;
	TH1D *hRDown;
	TH1D *hA;
	TH1D *hB;
	
	sprintf(nameA, "hUp_%s", suffixA);
	sprintf(nameB, "hUp_%s", suffixB);
	hA = (TH1D *) gROOT->FindObject(nameA);
	hB = (TH1D *) gROOT->FindObject(nameB);
	if (!hA || !hB) {
		printf("Can not find hist: %s or/and %s. Step %s\n", nameA, nameB, title);
		return;
	}
	sprintf(nameR, "hRUp_%s_%s", suffixA, suffixB);
	hRUp = (TH1D*) hA->Clone(nameR);
	hRUp->SetTitle(title);
	hRUp->Divide(hA, hB);
	hRUp->SetLineColor(kRed);
	hRUp->SetMarkerColor(kRed);
	hRUp->SetMarkerSize(2);
	hRUp->SetMarkerStyle(kFullTriangleUp);
	
	sprintf(nameA, "hMid_%s", suffixA);
	sprintf(nameB, "hMid_%s", suffixB);
	hA = (TH1D *) gROOT->FindObject(nameA);
	hB = (TH1D *) gROOT->FindObject(nameB);
	if (!hA || !hB) {
		printf("Can not find hist: %s or/and %s. Step %s\n", nameA, nameB, title);
		return;
	}
	sprintf(nameR, "hRMid_%s_%s", suffixA, suffixB);
	hRMid = (TH1D*) hA->Clone(nameR);
	hRMid->SetTitle(title);
	hRMid->Divide(hA, hB);
	hRMid->SetLineColor(kGreen);
	hRMid->SetMarkerColor(kGreen);
	hRMid->SetMarkerSize(2);
	hRMid->SetMarkerStyle(kFullCircle);
	
	sprintf(nameA, "hDown_%s", suffixA);
	sprintf(nameB, "hDown_%s", suffixB);
	hA = (TH1D *) gROOT->FindObject(nameA);
	hB = (TH1D *) gROOT->FindObject(nameB);
	if (!hA || !hB) {
		printf("Can not find hist: %s or/and %s. Step %s\n", nameA, nameB, title);
		return;
	}
	sprintf(nameR, "hRDown_%s_%s", suffixA, suffixB);
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

double CalcHistRatio(TH1 *hA, TH1 *hB, int binFirst, int binLast, double &err)
{
//	We ignore error on hB here
	double Sab, Sb2;
	double a, b, ea;
	int i;
	
	if (hA->GetNbinsX() != hB->GetNbinsX()) {
		printf("CalcHistRatio: histogramms %s and %s have different number of bins !\n",
			hA->GetTitle(), hB->GetTitle());
		err = 0;
		return -1;
	}
	Sab = Sb2 = 0;
	for (i=binFirst; i<=binLast; i++) {
		a = hA->GetBinContent(i+1);
		b = hB->GetBinContent(i+1);
		ea = hA->GetBinError(i+1);
		Sab += a * b / (ea * ea);
		Sb2 += b * b / (ea * ea);
	}
	err = 1 / sqrt(Sb2);
//	printf("Sab = %f, Sb2 = %f, err = %f, val = %f\n", Sab, Sb2, err, Sab / Sb2);
	return Sab / Sb2;
}

void draw_signal_spectra(const struct PeriodsStruct *period, double bgScale = 5.6/2.5, 
	double Nscale = 1.0, const char *posmask = "umd")
{
	char strs[128];
	char strl[1024];
	TLatex txt;
	double val, err;

	sprintf(strs, "hOff_%s_%s", period->name, posmask);
	sprintf(strl, "Positron raw spectrum %s (%s);Positron energy, MeV;Events / (day * 125 keV)", 
		period->title, posmask);
	TH1D *hN = new TH1D(strs, strl, NEBINS, 0, 16);
	sprintf(strs, "hOffCosm_%s_%s", period->name, posmask);
	sprintf(strl, "Positron raw spectrum (cosmic) %s (%s);Positron energy, MeV;Events / (day * 125 keV)", 
		period->title, posmask);
	TH1D *hC = new TH1D(strs, strl, NEBINS, 0, 16);
	sum_of_raw(hN, hC, posmask, period);
	hN->Add((TH1D*)gROOT->FindObject("hUp_Main"), -OtherBlockFraction);		// subtract blocks 1-3
	TF1 fBgndN("fBgndN", NeutronCorrN, 0, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0, 100);
	sprintf(strs, "hOff_%s_%s_c", period->name, posmask);
	TH1D *hNC = (TH1D*) hN->Clone(strs);
	sprintf(strs, "hOffCosm_%s_%s_c", period->name, posmask);
	TH1D *hCC = (TH1D*) hC->Clone(strs);
	hC->Add(&fBgndC, -86.4 * Nscale * rOffCosmCorr);
	hC->Scale(bgScale * 0.025);	// fixed 0.025 here
	hC->Add(&fBgndN, 86.4 * Nscale * rOffCosmCorr);
	hNC->Add(&fBgndN, -86.4 * Nscale * rOffCosmCorr);
	hCC->Add(&fBgndC, -86.4 * Nscale * rOffCosmCorr);
	hN->Write();
	hC->Write();
	hNC->Write();
	hCC->Write();
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
	val = CalcHistRatio(hNC, hCC, hNC->FindBin(1.001), hNC->FindBin(7.999), err);
	sprintf(strs, "Signal/Cosmics = %6.4f #pm %6.4f", val, err);
	txt.SetTextColor(kGreen);
	txt.DrawLatex(5, 0.79*hN->GetMaximum(), strs);
	sprintf(strs, "Detector position(s): %s", posmask);
	txt.SetTextColor(kBlack);
	txt.DrawLatex(5, 0.67*hN->GetMaximum(), strs);
	val = hN->IntegralAndError(hN->FindBin(1.001), hN->FindBin(6.999), err);
	sprintf(strs, "Integral 1-7 MeV = %5.1f #pm %3.1f events/day", val, err);
	txt.SetTextColor(kBlue);
	txt.DrawLatex(5, 0.55*hN->GetMaximum(), strs);
	val = hC->IntegralAndError(hN->FindBin(1.001), hN->FindBin(6.999), err);
	sprintf(strs, "Integral 1-7 MeV = %5.1f #pm %3.1f  events/day", val, err);
	txt.SetTextColor(kRed);
	txt.DrawLatex(5, 0.43*hN->GetMaximum(), strs);
}

void draw_off_ratio(const char *name, const char *title)
{
	char str[1024];
	
	sprintf(str, "hOff_%s_c", name);
	TH1 *h = (TH1 *)gROOT->FindObject(str);
	sprintf(str, "hOffCosm_%s_c", name);
	TH1 *hc = (TH1 *)gROOT->FindObject(str);
	if (!h || !hc) {
		printf("Histogramm(s) hOff*%s_c not found!\n", name);
		return;
	}
	sprintf(str, "hOffRatio_%s_c", name);
	TH1* hR = (TH1*) h->Clone(str);
	hR->SetTitle(title);
	hR->Divide(h, hc);
	hR->GetXaxis()->SetRange(hR->FindBin(0.001), hR->FindBin(8.999));
	hR->GetYaxis()->SetLabelSize(0.06);
	hR->SetMinimum(-0.05);
	hR->SetMaximum(0.2);
	hR->Fit("pol0", "", "", 1, 8);
	hR->Write();
}

int sum_accidental(TH1D *h, const char *posmask = "umdrs", const struct PeriodsStruct *period = NULL)
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
	for (i=0; i<N; i++) if (is_position_in_period(&positions[i], period)) {
		ptr = strchr(posmask, positions[i].name[0]);
		if (!(ptr && is_position_in_period(&positions[i], period))) continue;
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

void draw_accidental(const struct PeriodsStruct *period = NULL)
{
	char strl[1024];
	char strs[128];
	double val, err;
	int n;

	if (!period) period = period_by_name("Main");

	sprintf(strs, "AccUp_%s", period->name);
	sprintf(strl, "Accidental background spectrum %s, UP;Positron energy, MeV;Events / (day * 125 keV)", 
		period->title);
	TH1D *hUp = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_accidental(hUp, "u", period);
	hUp->Write();
	hUp->SetLineColor(kRed);
	hUp->SetFillColor(kRed-10);
	hUp->GetYaxis()->SetLabelSize(0.08);
	sprintf(strl, "Accidental background spectrum %s", period->title);
	hUp->SetTitle(strl);
	hUp->Draw("hist,e");
	val = hUp->IntegralAndError(hUp->FindBin(1.001), hUp->FindBin(7.999), err);
	sprintf(strs, "  Up: %d events %6.1f #pm%4.1f / day", n, val, err);
	TLegend *lg = new TLegend(0.35, 0.65, 0.9, 0.9);
	lg->AddEntry(hUp, strs, "l");

	sprintf(strs, "hAccMid_%s", period->name);
	sprintf(strl, "Accidental background spectrum %s, MIDDLE;Positron energy, MeV;Events / (day * 125 keV)", 
		period->title);
	TH1D *hMid = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_accidental(hMid, "m", period);
	hMid->Write();
	hMid->SetLineColor(kGreen);
	hMid->SetFillColor(kGreen-10);
	hMid->Draw("same,hist,e");
	val = hMid->IntegralAndError(hMid->FindBin(1.001), hMid->FindBin(7.999), err);
	sprintf(strs, " Mid: %d events %6.1f #pm%4.1f / day", n, val, err);
	lg->AddEntry(hMid, strs, "l");

	sprintf(strs, "hAccDown_%s", period->name);
	sprintf(strl, "Accidental background  spectrum %s, DOWN;Positron energy, MeV;Events / (day * 125 keV)",
		period->title);
	TH1D *hDown = new TH1D(strs, strl, NEBINS, 0, 16);
	n = sum_accidental(hDown, "d", period);
	hDown->Write();
	hDown->SetLineColor(kBlue);
	hDown->SetFillColor(kBlue-10);
	hDown->Draw("same,hist,e");
	val = hDown->IntegralAndError(hDown->FindBin(1.001), hDown->FindBin(7.999), err);
	sprintf(strs, "Down: %d events %6.1f #pm%4.1f / day", n, val, err);
	lg->AddEntry(hDown, strs, "l");
	
	lg->Draw();
}

void danss_calc_ratio_v6(const char *fname, double bgScale = 5.6/2.5, double Nscale = 1.0)
{
	TCanvas *cv;
	TFile *f;
	TFile *fOut;
	char str[1024], strs[128];
	char strl[1024], strm[128];
	TLatex *txt;
	double val, err;
	char pname[1024];
	char rname[2014];
	TLegend *lg;
	int i, j;
	TH1D *hSum;
	TH1D *hSumSub;
	const char *spname[] = {"End4", "Mar17", "Apr17", "May17", "Jun17"};
	const char *spprefix[] = {"hUp", "hMid", "hDown", "hSum"};
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1111);
	gStyle->SetTitleXSize(0.08);
	gStyle->SetTitleYSize(0.08);
	gStyle->SetLabelSize(0.08);
	gStyle->SetPadLeftMargin(0.2);
	gStyle->SetPadBottomMargin(0.2);
//	gStyle->SetLineWidth(2);

	readPositions("stat_pos.txt");
	
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
	
//	Spectra pages
	for (i = 0; i < sizeof(PeriodList) / sizeof(PeriodList[0]); i++) {
		draw_spectra_page(cv, &PeriodList[i], bgScale, Nscale);
		cv->Print(pname);
	}
//	Sum total Spectra
	for (i = 0; i < sizeof(PeriodList) / sizeof(PeriodList[0]); i++) {
		sprintf(strs, "hSum_%s", PeriodList[i].name);
		sprintf(str, "Positron spectrum %s;Positron energy, MeV;Events / (day * 125 keV)", PeriodList[i].title);
		hSum = new TH1D(strs, str, NEBINS, 0, 16);
		sprintf(strs, "hSumSub_%s", PeriodList[i].name);
		sprintf(str, "Background spectrum %s;Positron energy, MeV;Events / (day * 125 keV)", PeriodList[i].title);
		hSumSub = new TH1D(strs, str, NEBINS, 0, 16);
		sum_of_spectra(hSum, hSumSub, "umdrs", &PeriodList[i], bgScale, Nscale);
		hSum->Write();
		hSumSub->Write();
	}
//	Draw selected collection
//	Main
	((TH1D*) gROOT->FindObject("hSum_Main"))->Draw("e");
	cv->Print(pname);
//	Cmp4,5,6
	((TH1D*) gROOT->FindObject("hSum_Cmp4"))->SetLineColor(kRed);
	((TH1D*) gROOT->FindObject("hSum_Cmp5"))->SetLineColor(kGreen);
	((TH1D*) gROOT->FindObject("hSum_Cmp6"))->SetLineColor(kBlue);
	((TH1D*) gROOT->FindObject("hSum_Cmp4"))->Draw("e");
	((TH1D*) gROOT->FindObject("hSum_Cmp5"))->Draw("e,same");
	((TH1D*) gROOT->FindObject("hSum_Cmp6"))->Draw("e,same");
	lg = new TLegend(0.6, 0.7, 0.9, 0.8);
	lg->AddEntry(gROOT->FindObject("hSum_Cmp4"), period_by_name("Cmp4")->title, "LE");
	lg->AddEntry(gROOT->FindObject("hSum_Cmp5"), period_by_name("Cmp5")->title, "LE");
	lg->AddEntry(gROOT->FindObject("hSum_Cmp6"), period_by_name("Cmp6")->title, "LE");
	lg->Draw();
	cv->Update();
	cv->Print(pname);
//	PLB, PPLB
	((TH1D*) gROOT->FindObject("hSum_PLB"))->SetLineColor(kRed);
	((TH1D*) gROOT->FindObject("hSum_PPLB"))->SetLineColor(kBlue);
	((TH1D*) gROOT->FindObject("hSum_PLB"))->Draw("e");
	((TH1D*) gROOT->FindObject("hSum_PPLB"))->Draw("e,same");
	lg->Clear();
	lg->AddEntry(gROOT->FindObject("hSum_PLB"), period_by_name("PLB")->title, "LE");
	lg->AddEntry(gROOT->FindObject("hSum_PPLB"), period_by_name("PPLB")->title, "LE");
	lg->Draw();
	cv->Update();
	cv->Print(pname);
//	Stg1,2
	((TH1D*) gROOT->FindObject("hSum_Stg1"))->SetLineColor(kRed);
	((TH1D*) gROOT->FindObject("hSum_Stg2"))->SetLineColor(kBlue);
	((TH1D*) gROOT->FindObject("hSum_Stg1"))->Draw("e");
	((TH1D*) gROOT->FindObject("hSum_Stg2"))->Draw("e,same");
	lg->Clear();
	lg->AddEntry(gROOT->FindObject("hSum_Stg1"), period_by_name("Stg1")->title, "LE");
	lg->AddEntry(gROOT->FindObject("hSum_Stg2"), period_by_name("Stg2")->title, "LE");
	lg->Draw();
	cv->Update();
	cv->Print(pname);
//	Beg, End
	((TH1D*) gROOT->FindObject("hSum_Beg"))->SetLineColor(kRed);
	((TH1D*) gROOT->FindObject("hSum_End"))->SetLineColor(kBlue);
	((TH1D*) gROOT->FindObject("hSum_Beg"))->Draw("e");
	((TH1D*) gROOT->FindObject("hSum_End"))->Draw("e,same");
	lg->Clear();
	lg->AddEntry(gROOT->FindObject("hSum_Beg"), period_by_name("Beg")->title, "LE");
	lg->AddEntry(gROOT->FindObject("hSum_End"), period_by_name("End")->title, "LE");
	lg->Draw();
	cv->Update();
	cv->Print(pname);

//goto fin;

//	Ratios Down/Up
	for (i = 0; i < sizeof(PeriodList) / sizeof(PeriodList[0]); i++) {
		if (PeriodList[i].pwr != 1) continue;
		sprintf(strs, "hDown_%s", PeriodList[i].name);
		sprintf(strm, "hUp_%s", PeriodList[i].name);
		sprintf(strl, "hDownUp_%s", PeriodList[i].name);
		sprintf(str, "Ratio Down/Up %s;Positron energy, MeV;#frac{N_{DOWN}}{N_{UP}}", PeriodList[i].title);
		draw_single_ratio(strs, strm, strl, str, 0.6, 0.9);
		cv->Update();
		cv->Print(pname);
	}

//	Ratios	Mid/Up
	for (i = 0; i < sizeof(PeriodList) / sizeof(PeriodList[0]); i++) {
		if (PeriodList[i].pwr != 1) continue;
		sprintf(strs, "hMid_%s", PeriodList[i].name);
		sprintf(strm, "hUp_%s", PeriodList[i].name);
		sprintf(strl, "hMidUp_%s", PeriodList[i].name);
		if (((TH1D*) gROOT->FindObject(strs))->Integral() < 10) continue;
		sprintf(str, "Ratio Mid/Up %s;Positron energy, MeV;#frac{N_{MID}}{N_{UP}}", PeriodList[i].title);
		draw_single_ratio(strs, strm, strl, str, 0.6, 0.9);
		cv->Update();
		cv->Print(pname);
	}

//	Ratios	Down/Mid
	for (i = 0; i < sizeof(PeriodList) / sizeof(PeriodList[0]); i++) {
		if (PeriodList[i].pwr != 1) continue;
		sprintf(strs, "hDown_%s", PeriodList[i].name);
		sprintf(strm, "hMid_%s", PeriodList[i].name);
		sprintf(strl, "hDownMid_%s", PeriodList[i].name);
		if (((TH1D*) gROOT->FindObject(strm))->Integral() < 10) continue;
		sprintf(str, "Ratio Down/Mid %s;Positron energy, MeV;#frac{N_{DOWN}}{N_{MID}}", PeriodList[i].title);
		draw_single_ratio(strs, strm, strl, str, 0.6, 0.9);
		cv->Update();
		cv->Print(pname);
	}

//	Ratios for questionable periods
	for (i = 0; i < sizeof(spname) / sizeof(spname[0]); i++) {
		cv->Clear();
		cv->Divide(2, 2);
		for (j = 0; j < sizeof(spprefix) / sizeof(spprefix[0]); j++) {
			cv->cd(j + 1);
			draw_single_period_ratio(spprefix[j], spname[i], "Main1");
		}
		cv->Update();
		cv->Print(pname);
	}

//	after shutdown / before shutdown - use proper periods.
	cv->Clear();
	draw_normalized_ratio(period_by_name("Beg5"), period_by_name("End4"), "hNRatioBeg5_2_End4", 
		"Normalized ratio begin campaign 5 / end campaign 4;Positron energy, MeV", 0.9, 1.4, bgScale, Nscale);
	cv->Update();
	cv->Print(pname);

	draw_normalized_ratio(period_by_name("Beg6"), period_by_name("End5"), "hNRatioBeg6_2_End5", 
		"Normalized ratio begin campaign 6 / end campaign 5;Positron energy, MeV", 0.9, 1.4, bgScale, Nscale);
	cv->Update();
	cv->Print(pname);

	draw_normalized_ratio(period_by_name("Beg5"), period_by_name("End5"), "hNRatioBeg5_2_End5", 
		"Normalized ratio begin campaign 5 / end campaign 5;Positron energy, MeV", 0.9, 1.4, bgScale, Nscale);
	cv->Update();
	cv->Print(pname);

	draw_normalized_ratio(period_by_name("Beg"), period_by_name("End"), "hNRatioBeg_2_end", 
		"Normalized ratio after shutdown 5,6 / before shutdown 4,5;Positron energy, MeV", 0.9, 1.4, bgScale, Nscale);
	cv->Update();
	cv->Print(pname);

//	Tails - without background subtraction
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

//		Reactor Off 1
	cv->Clear();
	draw_signal_spectra(period_by_name("Off1"), bgScale, Nscale);
	cv->Update();
	cv->Print(pname);

//		Reactor Off 2
	draw_signal_spectra(period_by_name("Off2"), bgScale, Nscale);
	cv->Update();
	cv->Print(pname);

//		Reactor Off 1+2
	draw_signal_spectra(period_by_name("Off"), bgScale, Nscale);
	cv->Update();
	cv->Print(pname);

//		Reactor Off 1+2 up
	cv->Clear();
	draw_signal_spectra(period_by_name("Off"), bgScale, Nscale, "u");
	cv->Update();
	cv->Print(pname);
//		Reactor Off 1+2 mid
	draw_signal_spectra(period_by_name("Off"), bgScale, Nscale, "m");
	cv->Update();
	cv->Print(pname);
//		Reactor Off 1+2 down
	draw_signal_spectra(period_by_name("Off"), bgScale, Nscale, "d");
	cv->Update();
	cv->Print(pname);
	
//		Illustrate Off fits by cosmics
	cv->Clear();
	cv->Divide(2, 2);
	cv->cd(1);
	draw_off_ratio("Off_umd", "Neutrino to visible cosmics ratio, reactor off;MeV;R");
	cv->cd(2);
	draw_off_ratio("Off_u", "Neutrino to visible cosmics ratio UP, reactor off;MeV;R");
	cv->cd(3);
	draw_off_ratio("Off_m", "Neutrino to visible cosmics ratio MIDDLE, reactor off;MeV;R");
	cv->cd(4);
	draw_off_ratio("Off_d", "Neutrino to visible cosmics ratio DOWN, reactor off;MeV;R");
	cv->Update();
	cv->Print(pname);

//		Accidental background
	cv->Clear();
	draw_accidental();
	cv->Update();
	cv->Print(pname);

//****************	Write file	*****************//
fin:
	sprintf(str, "%s]", pname);
	cv->Print(str);
	fData->Close();
	delete cv;
	fOut->Close();
}
