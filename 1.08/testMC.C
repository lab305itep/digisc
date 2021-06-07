const double SiPMMeV = 15;
const double PMTMeV = 15;
const double SiPMCr = 0.4;
const double PMT12d = 6.0;
const double SiPMAvr = 2.0;
const double PMTAvr = 20.0;
const double SiPMLW = 0.6;
const double PMTLW = 2.0;

void FillPMT(TH1D *h, int N)
{
	TRandom rndm;
	int i;
	double s, s0, s1, s2;
	
	for (i=0; i<N; i++) {
		s = rndm.Landau(PMTAvr, PMTLW);
		s0 = rndm.Poisson(s * PMTMeV);
		s1 = rndm.Poisson(s0 * PMT12d);
		s2 = rndm.Poisson(s1 * PMT12d);
		h->Fill(s2);
	}
}

void FillSiPM(TH1D *h, int N)
{
	TRandom rndm;
	int i;
	double s, s0, s1;
	
	for (i=0; i<N; i++) {
		s = rndm.Landau(SiPMAvr, SiPMLW);
		s0 = rndm.Poisson(s * SiPMMeV);
		s1 = s0 + rndm.Poisson(s0 * SiPMCr);
		h->Fill(s1);
	}
}

void testMC(int N)
{
	TH1D *hSiPM = gROOT->FindObject("hSiPM");
	if (hSiPM) delete hSiPM;
	hSiPM = new TH1D("hSiPM", "SiPM model;Pixels", 200, 0, 200);
	FillSiPM(hSiPM, N);

	TH1D *hPMT = gROOT->FindObject("hPMT");
	if (hPMT) delete hPMT;
	hPMT = new TH1D("hPMT", "PMT model;e", 200, 0, 20000);
	FillPMT(hPMT, N);
}
