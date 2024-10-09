//	Calculate normalized spectra ratios for fuel evolution in full campaigns 5, 6, 7, 8
//	Global parameters
#define NEBINS 128		// energy bins
#define REBIN 2			// Rebin original hists
#define EMAX 16.0		// maximum energy for histograms
#define MAXPOSITIONS 1000	// maximum positions
#define CMPDIV 5		// number of campaign divisions
#define PWR0 3100.0		// nominal power
#define EFF0 0.35		// nominal efficiency

// Structure of records in positions file
struct PositionsStruct {
	char name[32];
	int first;
	int last;
	char chpower;
	double power;
	double bgnd;
	double total;
	double dead;
	double eff;
} positions[MAXPOSITIONS];
int PosEntries;

// Structure of records in stat file
struct StatStruct {
	double Cnt;
	double days;
	double power;
	double dead;
	double eff;
};

//	individual campaigns
struct Campaign {
	int begin;	// the first run
	int end;	// the last run
};


TFile *fData;
// Fast neutron corrections 
const char *NeutronCorrN = "0.5109-0.01932*x";		// neutron correction for signal per day
const char *NeutronCorrC = "2.143-0.09811*x";		// neutron correction for cosmic per day
const char *LowMuonCorr = "10.7*exp(-1.00*x)";		// low energy reactor off correction
const double OtherBlockFraction = 0.0060;	// Neutrino fraction of other blocks. Distances to other reactors: 160, 336 and 478 m
const double bgCorrUp = 0.958;			// Up background correction from the reator off
const double bgCorrMid = 0.988;			// Mid background correction from the reator off
const double bgCorrDown = 1.058;		// Down background correction from the reator off

//	Read positions file
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
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].power = strtod(ptr, NULL);
		ptr = strtok(NULL, " \t");
		if (!ptr) continue;
		positions[PosEntries].bgnd = 1.55 * strtod(ptr, NULL);	// we use this coef to keep to the old definition
		switch (positions[PosEntries].name[0]) {		// do background position correction
		case 'u' : positions[PosEntries].bgnd *= bgCorrUp; break;
		case 'm' : positions[PosEntries].bgnd *= bgCorrMid; break;
		case 'd' : positions[PosEntries].bgnd *= bgCorrDown; break;
		}
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

//	Decode position prefix
int GetPrefix(const char *name)
{
	int i;
	const char *List[] = {"up", "mid", "down"};
	char *ptr;
	
	for (i = 0; i < sizeof(List) / sizeof(List[0]); i++) if (strstr(name, List[i])) break;
	if (i == sizeof(List) / sizeof(List[0])) return 0;
	return i+1;
}

//	Try to find rate correction from power, efficiency, dead time and position
double RateScale(struct PositionsStruct *pos)
{
	char str[1024];
	char * ptr;
	int i;
	const double PosScale[3] = {1.0, 0.837, 0.712};		// distance correcion
	
	i = GetPrefix(pos->name);
	if (!i) return 0;		// we will ignore unknown positions

	double Pwr = pos->power;
	double Dead = pos->dead / pos->total;
	double Eff = pos->eff;
	
	double k = PWR0 * EFF0 * (1 - OtherBlockFraction) / (Pwr * Eff * (1 - Dead) * PosScale[i-1]);
	return k;
}

//	check that the position is in the selected range - it should be completely in it
//	Only FULL power is considered
int is_position_in_range(struct PositionsStruct *pos, int begin, int end)
{
	if (pos->first >= begin && pos->last <= end && pos->chpower == 'F') return 1;
	return 0;
}

//  Make sum of spectra over large number of points taking into accound dead time, efficiency, position etc.
TH1D *normalized_sum_of_spectra(const char *name, int begin, int end, double bgScale = 1.0)
{
	int N;
	TH1D *hSig;
	TH1D *hConst;
	TH1D *hBgnd;
	TH1D *hRBgnd;
	int i;
	char str[1024];
	double tSum;
	double dt;
	char *ptr;
	double Cnt;
	double days;
	double scale;
	TF1 fBgndN("fBgndN", NeutronCorrN, 0.5, 100);
	TF1 fBgndC("fBgndC", NeutronCorrC, 0.5, 100);
	TF1 fLowMuon("fLowMuon", LowMuonCorr, 0.5, 100);

	TH1D *hSum = new TH1D(name, "", NEBINS, 0, EMAX);
	TH1D *hSub = new TH1D("__hSub", "", NEBINS, 0, EMAX);
	tSum = 0;
	for (i=0; i<PosEntries; i++) {
		if (!(is_position_in_range(&positions[i], begin, end))) continue;
		sprintf(str, "%s_hSig-diff", positions[i].name);
		hSig = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hConst", positions[i].name);
		hConst = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hCosm-diff", positions[i].name);
		hBgnd = (TH1D*) fData->Get(str);
		sprintf(str, "%s_hCosm-murand", positions[i].name);
		hRBgnd = (TH1D*) fData->Get(str);
		if (!(hSig && hConst && hBgnd && hRBgnd)) continue;
		dt = hConst->GetBinContent(1) / 1000.0;	// seconds * 10^3
		tSum += dt;
		scale = RateScale(&positions[i]);
		hSum->Add(hSig, scale);
		hSub->Add(hBgnd, positions[i].bgnd * bgScale * scale);
		hSub->Add(hRBgnd, -positions[i].bgnd * bgScale * scale);	// subtract accidental muon to IBD coincidence
		hSub->Add(&fBgndN, dt * scale/ 86.4);
		hSub->Add(&fLowMuon, dt * scale / 86.4);
		hSub->Add(&fBgndC, -dt * positions[i].bgnd * bgScale * scale / 86.4);
	}
	days = tSum / 86.4;
	hSum->Add(hSub, -1);
	if (tSum != 0) hSum->Scale(1.0 / days);
	delete hSub;
	return hSum;
}

void evolution_ratio(const char *fname, double bgScale = 5.6/2.5)
{
	TCanvas *cv;
	TFile *fOut;
	char str[1024];
	TLatex *txt;
	double val, err;
	TLegend *lg;
	int i, j, begin, end;
	TH1D *h[CMPDIV];
	TH1D *hd[CMPDIV];
	TH1D *hr[CMPDIV];
//	{"Cmp5", "Aug 17 - Jan 19 (campaign 5)", 1, {{24776, 54595}
//	{"Cmp6", "Feb 19 - July 20 (campaign 6)", 1, {{56626, 87433}
//	{"Cmp7", "Aug 20 - Nov 21 (campaign 7)", 1, {{89360, 117657}
//	{"Cmp8", "Feb 22 - Jun 23 (campaign 8)", 1, {{119369, 146638}
	const struct Campaign Cmp[] = {{24776, 54595}, {56626, 87433}, {89360, 117657}, {119369, 146638}};
	const EColor color[CMPDIV] = {kBlack, kRed, kGreen, kBlue, kOrange};
	const EMarkerStyle marker[CMPDIV] = {kOpenCircle, kFullCircle, kFullSquare, kFullDiamond, kFullTriangleUp};

	gStyle->SetOptStat(0);
	gStyle->SetLineWidth(2);
	gStyle->SetPadLeftMargin(0.15);

	readPositions("pos_2210_154797_v0.txt");
	
	fData = new TFile(fname);
	if (!fData->IsOpen()) return;
	cv = new TCanvas("CV", "Results", 1200, 1200);
	TString tstr(fname);
	tstr.ReplaceAll(".root", "");
	tstr += "-norm";
	cv->Print((tstr + ".pdf[").Data());
	fOut = new TFile((tstr + ".root").Data(), "RECREATE");
	fOut->cd();
	txt = new TLatex();
	
//	Spectra pages
	lg = new TLegend(0.6, 0.65, 0.89, 0.89);
	for (i = 0; i < sizeof(Cmp) / sizeof(Cmp[0]); i++) {
		for (j=0; j<CMPDIV; j++) {
			begin = Cmp[i].begin + (Cmp[i].end - Cmp[i].begin + 1) * j / CMPDIV;
			end = Cmp[i].begin + (Cmp[i].end - Cmp[i].begin + 1) * (j + 1) / CMPDIV - 1;
			sprintf(str, "hCmp%d_%d", i+5, j+1);	// we start from campaign 5
			h[j] = normalized_sum_of_spectra(str, begin, end);
			if (REBIN != 1) h[j]->Rebin(REBIN);
			h[j]->GetXaxis()->SetRangeUser(0,8.0);
			h[j]->SetLineColor(color[j]);
			h[j]->SetMarkerColor(color[j]);
			h[j]->SetMarkerStyle(marker[j]);
			h[j]->SetMarkerSize(0.5);
			sprintf(str, "Campaign %d, Spectra;MeV;events/day/bin", i + 5);
			h[j]->SetTitle(str);
			sprintf(str, "hRCmp%d_%d", i+5, j+1);	// we start from campaign 5
			hd[j] = (TH1D *) h[j]->Clone(str);
			sprintf(str, "Campaign %d, Difference from begin;MeV;events/day/bin", i + 5);
			hd[j]->SetTitle(str);
			hd[j]->Add(h[j], h[0], 1, -1);
			hr[j] = (TH1D *) h[j]->Clone(str);
			sprintf(str, "Campaign %d, Ratio to begin;MeV;", i + 5);
			hr[j]->SetTitle(str);
			hr[j]->Divide(h[j], h[0]);
			hr[j]->SetMinimum(0.8);
			hr[j]->SetMaximum(1.2);
			
			h[j]->Draw((j) ? "same" : "");
			if(!i) {
				sprintf(str, "Period %d", j + 1);
				lg->AddEntry(h[j], str, "pl");
			}
			fOut->cd();
			h[j]->Write();
			hr[j]->Write();
			hd[j]->Write();
		}
		lg->Draw();
		cv->Print((tstr + ".pdf").Data());
		for (j=CMPDIV-1; j>0; j--) hd[j]->Draw((j != CMPDIV - 1) ? "same" : "");
		lg->Draw();
		cv->Print((tstr + ".pdf").Data());
		for (j=CMPDIV-1; j>0; j--) hr[j]->Draw((j != CMPDIV - 1) ? "same" : "");
		lg->Draw();
		cv->Print((tstr + ".pdf").Data());
	}
	
//****************	Write file	*****************//
fin:
	cv->Print((tstr + ".pdf]").Data());
	delete cv;
	fData->Close();
	fOut->Close();
}
