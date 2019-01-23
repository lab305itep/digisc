void drawrawbgnd(const char *fname)
{
	const char *names[18] = {"hAllc", "hAllt", "hVetoc", "hVetot", "hNoVetoc", "hNoVetot", "h20c", "h20t", "hNo20c", "hNo20t",
		"h2topc", "h2topt", "hNo2topc", "hNo2topt", "h2edgec", "h2edget", "hNo2edgec", "hNo2edget"};
	int i;
	char str[1024];
	char *ptr;
	TVirtualPad *pd;
	TH1D *h;
	TFile *f;
	
	f = new TFile(fname);
	if (!f->IsOpen()) return;
	gStyle->SetOptStat(0);
	strcpy(str, fname);
	ptr = strchr(str, '.');
	if (!ptr) ptr = &str[strlen(str)];
	strcpy(ptr, ".pdf[");
	TCanvas cv("CV", "CV", 1200, 1000);
	cv.SaveAs(str);
	ptr[4] = '\0';
	for (i=0; i < sizeof(names) / sizeof(names[0]); i++) {
		h = (TH1D*) f->Get(names[i]);
		if (!h) continue;
		cv.Clear();
		cv.Divide(2,2);
		
		pd = cv.cd(1);
		pd->SetLogy(0);
		h->DrawCopy();
		pd = cv.cd(2);
		pd->SetLogy(1);
		h->DrawCopy();

		h->GetXaxis()->SetRange(3,100);
		pd = cv.cd(3);
		pd->SetLogy(0);
		h->DrawCopy();
		pd = cv.cd(4);
		pd->SetLogy(1);
		h->DrawCopy();
		
		cv.SaveAs(str);
	}
	ptr[4] = ']';
	cv.SaveAs(str);
}
