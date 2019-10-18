/*
	An example how to get sum over several periods..
	Difference will not work, because of wrong error propagation
*/
void GetCombo(TFile *f, TH1D **hUp, TH1D **hMid, TH1D **hDown)
{
	const char *month[] = {
		"20.04.16_10.06.16", "10.06.16_01.12.16", "01.12.16_01.01.17", "01.01.17_01.02.17",
		"01.02.17_01.03.17", "01.03.17_01.04.17", "01.04.17_01.05.17", "01.05.17_01.06.17",
		"01.06.17_01.07.17", "01.07.17_01.09.17", "01.09.17_01.10.17", "01.10.17_01.11.17",
		"01.11.17_01.12.17", "01.12.17_01.01.18", "01.01.18_01.02.18", "01.02.18_01.04.18",
		"01.04.18_01.06.18", "01.06.18_01.07.18", "01.07.18_01.08.18", "01.08.18_01.09.18",
		"01.09.18_01.10.18", "01.10.18_01.11.18", "01.11.18_01.12.18", "01.12.18_08.01.19",
		"08.01.19_02.03.19"};
	int i;
	TH1D *hConstTmp;
	TH1D *hUpTmp;
	TH1D *hMidTmp;
	TH1D *hDownTmp;
	double dUp, dMid, dDown;
	double dSumUp, dSumMid, dSumDown;
	char str[1024];
	
	sprintf(str, "hUp_%s", month[0]);
	hUpTmp = (TH1D *) f->Get(str);
	*hUp = (TH1D *) hUpTmp->Clone("hUpPeriod");
	(*hUp)->SetTitle("Up counts for some period");
	(*hUp)->Reset();
	*hMid = (TH1D *) hUpTmp->Clone("hMidPeriod");
	(*hMid)->SetTitle("Up counts for some period");
	(*hMid)->Reset();
	*hDown = (TH1D *) hUpTmp->Clone("hDownPeriod");
	(*hDown)->SetTitle("Down counts for some period");
	(*hDown)->Reset();
	
	dSumUp = dSumMid = dSumDown = 0;
	for (i=0; i<sizeof(month)/sizeof(month[0]); i++) {
		sprintf(str, "hConst_%s", month[i]);
		hConstTmp = (TH1D *) f->Get(str);
		sprintf(str, "hUp_%s", month[i]);
		hUpTmp = (TH1D *) f->Get(str);
		sprintf(str, "hMid_%s", month[i]);
		hMidTmp = (TH1D *) f->Get(str);
		sprintf(str, "hDown_%s", month[i]);
		hDownTmp = (TH1D *) f->Get(str);
		
		if (!hConstTmp || !hUpTmp || !hMidTmp || !hDownTmp) {
			printf("%s - some of histogramms not found.\n", month[i]);
			return;
		}
		
		dUp = hConstTmp->GetBinContent(4);
		dMid = hConstTmp->GetBinContent(5);
		dDown = hConstTmp->GetBinContent(6);
		
		(*hUp)->Add(hUpTmp, dUp);
		dSumUp += dUp;
		(*hMid)->Add(hMidTmp, dMid);
		dSumMid += dMid;
		(*hDown)->Add(hDownTmp, dDown);
		dSumDown += dDown;
		printf("Adding %s : %f %f %f\n", month[i], dUp, dMid, dDown);
	}
	
	(*hUp)->Scale(1.0 / dSumUp);
	(*hMid)->Scale(1.0 / dSumMid);
	(*hDown)->Scale(1.0 / dSumDown);
}
