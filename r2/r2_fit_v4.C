//All indexes go Section (U, M, D) Position (U, M, D) SP: UU, UM, UD, MU, MM, MD, DU, DM, DD
struct R2Data {
	double dist[9];		// distances 
	double counts[9];	// counrts
	double ecounts[9];	// errors
	double sDist;		// global shift error (0 - free)
	double sSize;		// effectiv core size error (0 - free)
	double Size;		// nominal effectiv reactor core size
} MyData;

void R2DataInit(double *r, double *cnt, double *ecnt)
{
	memcpy(MyData.dist, r, sizeof(MyData.dist));
	memcpy(MyData.counts, cnt, sizeof(MyData.counts));
	memcpy(MyData.ecounts, ecnt, sizeof(MyData.ecounts));
	MyData.sDist = 0;
	MyData.sSize = 0;
	MyData.Size = 3.5;
}

void R2SetErrors(double sigmaD, double sigmaS)
{
	MyData.sDist = sigmaD;
	MyData.sSize = sigmaS;
}

void R2SetSize(double S)
{
	MyData.Size = S;
}

// par[0] - Constant
// par[1] - shift
// par[2] - effective size
// par[3,4] - scaling relative of the Up and Down sections
static void R2Chi2(int &npar, double *gin, double &f, double *par,  int  iflag)
{
	double fun;
	double cnt[9], err[9];
	int i;

	switch (iflag) {
	case 1: // Initialization
		break;
	case 2: //Compute derivatives
		break;
	case 3: // after the fit is finished
		break;
	default:
		for(i=0; i<3; i++) {
			cnt[i] = par[3] * MyData.counts[i];
			err[i] = par[3] * MyData.ecounts[i];
			cnt[3+i] = MyData.counts[3+i];
			err[3+i] = MyData.ecounts[3+i];
			cnt[6+i] = par[4] * MyData.counts[6+i];
			err[6+i] = par[4] * MyData.ecounts[6+i];
		}
		f = 0;
		for (i=0; i<9; i++) {
			fun = par[0] / ((MyData.dist[i] - par[1]) * (MyData.dist[i] - par[1]) - par[2] * par[2] / 4);
			f += (fun - cnt[i]) * (fun - cnt[i]) / (err[i] * err[i]);
		}
		if (MyData.sDist) f += par[1] * par[1] / (MyData.sDist * MyData.sDist);
		if (MyData.sSize) f += (par[2] - MyData.Size) * (par[2] - MyData.Size) / (MyData.sSize * MyData.sSize);
	}
}

void r2_fit(const char *signame, double RangeMin, double RangeMax)
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
	double x[9], ex[9], cntcorr[9], ecntcorr[9], y[9], ey[9], res[9], eres[9];
	int iPart, iPos, k;
	TGraphErrors *gr;
	TGraphErrors *grres;
	char str[256];
	double C, eC, Shift, eShift, Size, eSize, sUp, esUp, sDown, esDown, chi2;
	double fedm, errdef;
	int npari, nparx, irc;

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

	memset(ex, 0, sizeof(ex));
	for (iPart=0; iPart<3; iPart++) for(iPos=0; iPos<3; iPos++) {
		x[3*iPart+iPos] = R[iPos] + dR[iPart];
		cntcorr[3*iPart+iPos] += (dead[iPos] - odead[iPos]) * xccnt[3*iPart+iPos] / (1 - odead[iPos]);	// correction for wrong off subtraction
		cntcorr[3*iPart+iPos] = cnt[3*iPart+iPos] / (1 - dead[iPos])  - (off[3*iPart+iPos] - coff[3*iPart+iPos]) / (1 - odead[iPos]);
		ecntcorr[3*iPart+iPos] = sqrt(ecnt[3*iPart+iPos]*ecnt[3*iPart+iPos] / ((1 - dead[iPos])*(1 - dead[iPos])) +
			+ (eoff[3*iPart+iPos]*eoff[3*iPart+iPos] + ecoff[3*iPart+iPos]*ecoff[3*iPart+iPos]) / ((1 - odead[iPos])*(1 - odead[iPos])));
	}

	R2DataInit(x, cntcorr, ecntcorr);
	R2SetSize(3.5);
	R2SetErrors(0.05, 0.15);
	TMinuit *MyMinuit = new TMinuit(5);
	MyMinuit->SetFCN(R2Chi2);
	MyMinuit->DefineParameter(0, "Const", 10000, 100, 0, 1E10);
	MyMinuit->DefineParameter(1, "Shift", 0, 0.1, -10, 10);
	MyMinuit->DefineParameter(2, "Size", 3.6, 1, 0, 10);
	MyMinuit->DefineParameter(3, "rUp", 1, 0.1, 0.1, 10);
	MyMinuit->DefineParameter(4, "rDown", 1, 0.1, 0.1, 10);
//	MyMinuit->FixParameter(1);
//	MyMinuit->FixParameter(2);
	MyMinuit->Migrad();
	MyMinuit->GetParameter(0, C, eC);
	MyMinuit->GetParameter(1, Shift, eShift);
	MyMinuit->GetParameter(2, Size, eSize);
	MyMinuit->GetParameter(3, sUp, esUp);
	MyMinuit->GetParameter(4, sDown, esDown);
	MyMinuit->mnstat(chi2, fedm, errdef, npari, nparx, irc);

	for (iPos=0; iPos<3; iPos++) {
		y[iPos] = sUp * cntcorr[iPos];
		ey[iPos] = sUp * ecntcorr[iPos];
		y[iPos+3] = cntcorr[iPos+3];
		ey[iPos+3] = ecntcorr[iPos+3];
		y[iPos+6] = sDown * cntcorr[iPos+6];
		ey[iPos+6] = sDown * ecntcorr[iPos+6];
	}
	sprintf(str, "%f / ((x - %f) * (x - %f) - %f * %f / 4)", C, Shift, Shift, Size, Size);
	TF1 *fR2 = new TF1("fR2", str, 2.0, 20.0);
	fR2->SetLineColor(kRed);
	gr = new TGraphErrors(9, x, y, ex, ey);

	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	TCanvas *cv = new TCanvas("CV", "CV", 1000, 800);
	TH1D *h = new TH1D("hR2", "R2 for 9 points;r, m; Events/day", 100, 10, 13.3);
	h->SetMinimum(-300);
	h->SetMaximum(CntMax);
	h->DrawCopy();
	gr->SetLineColor(kBlue);
	gr->SetMarkerColor(kBlue);
	gr->SetMarkerStyle(kFullCircle);
	gr->Draw("p");
	fR2->DrawCopy("same");
	
	for (k=0; k<9; k++) {
		res[k] = 20 * (y[k] - fR2->Eval(x[k]));
		eres[k] = 20 * ey[k];
	}
	grres = new TGraphErrors(9, x, res, ex, eres);
	grres->SetLineColor(kGreen);
	grres->SetMarkerColor(kGreen);
	grres->SetMarkerStyle(kFullCircle);
	grres->Draw("p");
	
	TLegend *lg  = new TLegend(0.7, 0.75, 0.98, 0.92);
	lg->AddEntry(gr, "Data", "pe");
	lg->AddEntry(grres, "Difference x20", "pe");
	lg->AddEntry(fR2, "Fit", "l");
	lg->Draw();

	TLatex *txt = new TLatex();
	txt->SetTextSize(0.03);
	sprintf(str, "#chi^{2} / ndf = %7.3f / 6", chi2);
	txt->DrawLatexNDC(0.15, 0.45, str);
	sprintf(str, "Const. = %6.0f #pm %5.0f", C, eC);
	txt->DrawLatexNDC(0.15, 0.4, str);
	sprintf(str, "sUp    = %6.4f #pm %6.4f   sDown = %6.4f #pm %6.4f", sUp, esUp, sDown, esDown);
	txt->DrawLatexNDC(0.15, 0.35, str);
	sprintf(str, "Shift  = %6.4f #pm %6.4f   Size  = %6.4f #pm %6.4f", Shift, eShift, Size, eSize);
	txt->DrawLatexNDC(0.15, 0.3, str);

	sprintf(str, "r2_%s_%5.3f-%5.3f.png", signame, Range[0], Range[1]);
	cv->SaveAs(str);
}

void many_plots(void)
{
	r2_fit("MainA", 0.75, 8);
	r2_fit("MainA", 1.0, 7);
	r2_fit("MainA", 1.5, 6);
	r2_fit("Main", 1.5, 6);
	r2_fit("MainA1", 1.5, 6);
	r2_fit("PLB", 1.5, 6);
	r2_fit("PPLB", 1.5, 6);
	r2_fit("Stg1", 1.5, 6);
	r2_fit("Stg2", 1.5, 6);
	r2_fit("Cmp4", 1.5, 6);
	r2_fit("Cmp5", 1.5, 6);
	r2_fit("Cmp6", 1.5, 6);
	r2_fit("Pos3", 1.5, 6);
	r2_fit("Pos3", 0.75, 8);
	r2_fit("Pos3", 0.75, 3);
	r2_fit("Pos3", 3, 8);
}
