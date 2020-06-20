void r2_simple(const char *signame, double RangeMin, double RangeMax)
{
	const char *fname[3] = {
		"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_up-calc.root",
		"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_mid-calc.root",
		"/home/clusters/rrcmpi/alekseev/igor/apr16_feb20/base_v3f2_down-calc.root"
	};
//	const char *signame = "MainA";
	const char *offname = "Off";
	const double Range[2] = {RangeMin, RangeMax};
	const double CntMax = 2500;
//			Up, Mid, Down Position
	const double R[3] = {10.90, 11.89, 12.87};
//			Up, Mid, Down Part
	const double dR[3] = {-0.305, 0, 0.305};
	const char *posname[3] = {"Up", "Mid", "Down"};
	const char poschar[3] = {'u', 'm', 'd'};
//				      Up,     Mid,    Down Position
//	const double dead[3]  = {0.13008, 0.13615, 0.14532};
//	const double odead[3] = {0.09323, 0.09187, 0.09214};
//	Part, position: UU, UM, UD, MU, MM, MD, DU, DM, DD
//	const double cnt[9] = {1506.2, 1246.5, 1059.3, 1519.6, 1271.2, 1068.2, 1164.1, 977.1, 821.9};
//		[0.75 - 8]
//	Part, position:            UU,     UM,     UD,     MU,     MM,     MD,     DU,     DM,     DD
//	const double cnt[9] = {1812.6, 1514.6, 1269.7, 1815.2, 1519.7, 1268.9, 1471.4, 1229.3, 1032.5};
//	const double ecnt[9] = {  2.9,    3.7,    2.0,    2.8,    3.6,    2.0,    2.5,    3.2,    1.7};
//	const double off[9] = {  34.2,   42.1,   55.4,   34.2,   31.8,   34.3,   51.0,   46.3,   47.2};
//	const double eoff[9] = {  3.3,    3.6,    3.9,    3.3,    3.4,    3.6,    3.0,    3.1,    3.3};
//	const double coff[9] = { 44.3,   46.0,   46.3,   35.7,   36.7,   37.2,   40.7,   41.4,   41.4};
//	const double ecoff[9] = { 0.3,    0.5,    0.3,    0.2,    0.2,    0.2,    0.2,    0.3,    0.3};
	double dead[3], odead[3], cnt[9], ecnt[9], off[9], eoff[9], coff[9], ecoff[9];
	double xccnt[9];
	TFile *f;
	TH1D *hf;
	double x[9], ex[9], cntcorr[9], ecntcorr[9], res[9], eres[9];
	int iPart, iPos, k;
	TGraphErrors *gr;
	TGraphErrors *grres;
	double scale;
	char str[256];

//		Get data from files
	for (iPart=0; iPart<3; iPart++) {
		f = new TFile(fname[iPart]);
		if (!f->IsOpen()) return;
//	cnt
		for (iPos=0; iPos<3; iPos++) {
			sprintf(str, "h%s_%s", posname[iPos], signame);
			hf = (TH1D *) f->Get(str);
			if (!hf) {
				printf("Hist %s not found.\n", str);
				return;
			}
			cnt[3*iPart + iPos] = hf->IntegralAndError(hf->FindBin(Range[0] + 0.0001), hf->FindBin(Range[1] - 0.0001), ecnt[3*iPart + iPos]);
		}
//	xccnt
		for (iPos=0; iPos<3; iPos++) {
			sprintf(str, "h%sSub_%s", posname[iPos], signame);
			hf = (TH1D *) f->Get(str);
			if (!hf) {
				printf("Hist %s not found.\n", str);
				return;
			}
			xccnt[3*iPart + iPos] = hf->Integral(hf->FindBin(Range[0] + 0.0001), hf->FindBin(Range[1] - 0.0001));
		}
//	dead (we need it only once)
		if (iPart == 0) {
			sprintf(str, "hConst_%s", signame);
			hf = (TH1D *) f->Get(str);
			if (!hf) {
				printf("Hist %s not found.\n", str);
				return;
			}
			for (iPos=0; iPos<3; iPos++) dead[iPos] = hf->GetBinContent(10 + iPos);
			sprintf(str, "hConst_%s", offname);
			hf = (TH1D *) f->Get(str);
			if (!hf) {
				printf("Hist %s not found.\n", str);
				return;
			}
			for (iPos=0; iPos<3; iPos++) odead[iPos] = hf->GetBinContent(10 + iPos);
		}
//	Off
		for (iPos=0; iPos<3; iPos++) {
			sprintf(str, "hOff_%s_%c", offname, poschar[iPos]);
			hf = (TH1D *) f->Get(str);
			if (!hf) {
				printf("Hist %s not found.\n", str);
				return;
			}
			off[3*iPart + iPos] = hf->IntegralAndError(hf->FindBin(Range[0] + 0.0001), hf->FindBin(Range[1] - 0.0001), eoff[3*iPart + iPos]);
		}
		
//	Cosmic Off
		for (iPos=0; iPos<3; iPos++) {
			sprintf(str, "hOffCosm_%s_%c", offname, poschar[iPos]);
			hf = (TH1D *) f->Get(str);
			if (!hf) {
				printf("Hist %s not found.\n", str);
				return;
			}
			coff[3*iPart + iPos] = hf->IntegralAndError(hf->FindBin(Range[0] + 0.0001), hf->FindBin(Range[1] - 0.0001), ecoff[3*iPart + iPos]);
		}
		
		f->Close();
	}

	printf("Dead: %6.4f %6.4f %6.4f   OffDead %6.4f %6.4f %6.4f\n", dead[0], dead[1], dead[2], odead[0], odead[1], odead[2]);
	for (iPart=0; iPart<3; iPart++) for (iPos=0; iPos<3; iPos++) printf("%s %s : %7g+-%5g - (%6g+-%5g  - %6g+-%5g) + x * %6g\n",
		posname[iPart], posname[iPos], cnt[3*iPart + iPos], ecnt[3*iPart + iPos], off[3*iPart + iPos], eoff[3*iPart + iPos], 
		coff[3*iPart + iPos], ecoff[3*iPart + iPos], xccnt[3*iPart + iPos]);
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 800);
	cv->Divide(2, 2);
	TH1D *h = new TH1D("hR2", "R2;r, m; Events/day", 100, 10, 13.3);
	h->SetMinimum(-300);
	h->SetMaximum(CntMax);
	TF1 *fR2 = new TF1("fR2", "[0] / (x * x - 1.8 * 1.8)", 2.0, 20.0);
	fR2->SetLineColor(kRed);
	memset(ex, 0, sizeof(ex));
	for (iPart=0; iPart<3; iPart++) for(iPos=0; iPos<3; iPos++) {
		x[3*iPart+iPos] = R[iPos] + dR[iPart];
		cntcorr[3*iPart+iPos] += (dead[iPos] - odead[iPos]) * xccnt[3*iPart+iPos] / (1 - odead[iPos]);	// correction for wrong off subtraction
		cntcorr[3*iPart+iPos] = cnt[3*iPart+iPos] / (1 - dead[iPos])  - (off[3*iPart+iPos] - coff[3*iPart+iPos]) / (1 - odead[iPos]);
		ecntcorr[3*iPart+iPos] = sqrt(ecnt[3*iPart+iPos]*ecnt[3*iPart+iPos] / ((1 - dead[iPos])*(1 - dead[iPos])) +
			+ (eoff[3*iPart+iPos]*eoff[3*iPart+iPos] + ecoff[3*iPart+iPos]*ecoff[3*iPart+iPos]) / ((1 - odead[iPos])*(1 - odead[iPos])));
	}
	
	for (iPart=0; iPart<3; iPart++) {
		cv->cd(iPart+1);
		printf("%d : %f:%f +- %f  %f:%f +- %f  %f:%f +- %f\n", iPart, 
			x[3*iPart],   cntcorr[3*iPart],   ecntcorr[3*iPart], 
			x[3*iPart+1], cntcorr[3*iPart+1], ecntcorr[3*iPart+1],
			x[3*iPart+2], cntcorr[3*iPart+2], ecntcorr[3*iPart+2]);
		gr = new TGraphErrors(3, &x[3*iPart], &cntcorr[3*iPart], ex, &ecntcorr[3*iPart]);
		sprintf(str, "R2: %s part", posname[iPart]);
		h->SetTitle(str);
		h->DrawCopy();
		gr->SetLineColor(kBlue);
		gr->SetMarkerColor(kBlue);
		gr->SetMarkerStyle(kFullCircle);
		gr->Fit(fR2, "q");
		gr->Draw("p");
		fR2->DrawCopy("same");
		
		for (k=0; k<3; k++) {
			res[k] = 20 * (cntcorr[3*iPart + k] - fR2->Eval(x[3*iPart + k]));
			eres[k] = 20 * ecntcorr[3*iPart + k];
		}
		grres = new TGraphErrors(3, &x[3*iPart] , res, ex, eres);
		grres->SetLineColor(kGreen);
		grres->SetMarkerColor(kGreen);
		grres->SetMarkerStyle(kFullCircle);
		grres->Draw("p");
		
		if (iPart == 0) {
			scale = fR2->GetParameter(0);
		} else {
			for (iPos=0; iPos<3; iPos++) {
				cntcorr[3*iPart+iPos]  *= scale / fR2->GetParameter(0);
				ecntcorr[3*iPart+iPos] *= scale / fR2->GetParameter(0);
			}
		}
	}

	TF1 *fR22 = new TF1("fR22", "[0] / ((x-[1]) * (x-[1]) - 1.8 * 1.8)", 2.0, 20.0);
	fR22->SetLineColor(kRed);

	cv->cd(4);
	h->SetTitle("R2: 9 points");
	h->DrawCopy();
	h->SetStats(0);
	gr = new TGraphErrors(9, x, cntcorr, ex, ecntcorr);
	gr->SetLineColor(kBlue);
	gr->SetMarkerColor(kBlue);
	gr->SetMarkerStyle(kFullCircle);
	gr->Draw("p");
	gr->Fit(fR22, "q");
	fR22->DrawCopy("same");
	
	for (k=0; k<9; k++) {
		res[k] = 20 * (cntcorr[k] - fR22->Eval(x[k]));
		eres[k] = 20 * ecntcorr[k];
		printf("%f: %f +- %f   delta = %f +- %f\n", x[k], cntcorr[k], ecntcorr[k], res[k], eres[k]);
	}

	grres = new TGraphErrors(9, x, res, ex, eres);
	grres->SetLineColor(kGreen);
	grres->SetMarkerColor(kGreen);
	grres->SetMarkerStyle(kFullCircle);
	grres->Draw("p");
	
	sprintf(str, "r2_%s_%5.3f-%5.3f.png", signame, Range[0], Range[1]);
	cv->SaveAs(str);
}

void many_plots(void)
{
	r2_simple("MainA", 0.75, 8);
	r2_simple("MainA", 1.0, 7);
	r2_simple("MainA", 1.5, 6);
	r2_simple("Main", 1.5, 6);
	r2_simple("MainA1", 1.5, 6);
	r2_simple("PLB", 1.5, 6);
	r2_simple("PPLB", 1.5, 6);
	r2_simple("Stg1", 1.5, 6);
	r2_simple("Stg2", 1.5, 6);
	r2_simple("Cmp4", 1.5, 6);
	r2_simple("Cmp5", 1.5, 6);
	r2_simple("Cmp6", 1.5, 6);
	r2_simple("Pos3", 1.5, 6);
	r2_simple("Pos3", 0.75, 8);
	r2_simple("Pos3", 0.75, 3);
	r2_simple("Pos3", 3, 8);
}
