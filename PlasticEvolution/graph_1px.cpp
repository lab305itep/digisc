/****************************************************************
 *  Create 1px signal graphs					*
 ****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <TFile.h>
#include <TGraph.h>
#include <TList.h>
#include <TObject.h>
#include <TString.h>
#include <TTree.h>

#define VDIR "/home/clusters/rrcmpi/alekseev/igor/dvert"
#define MAXRUN 200000

/****************************************************************/

class MyRunList {
	private:
		double *RunList;
		double date2year(int year, int month, int day);
	public:
		MyRunList(const char *stat_all);
		~MyRunList(void);
		inline double GetRunYear(int run) {		// time in years since 01.01.2016
			if (run < 0 || run >= MAXRUN) {
				printf("Out of range run number %d.\n", run);
				return -1000;
			}
			return RunList[run];
		}
};

MyRunList::MyRunList(const char *stat_all)
{
	FILE *f;
	int i, type, year, month, day, cnt;
	char str[1024];
	char *ptr;
	
	RunList = (double *) malloc(MAXRUN * sizeof(double));
	if (!RunList) {
		printf("No Memory for RunList!\n");
		throw;
	}
	
	for (i=0; i < MAXRUN; i++) RunList[i] = -1;
	
	f = fopen(stat_all, "rt");
	if (!f) {
		printf("File %s not opened. Dummy run list created.\n");
		for (i=0; i<MAXRUN; i++) RunList[i] = i/20000.0;
	} else {
		cnt = 0;
		for (i=0; i<MAXRUN; i++) RunList[i] = -1000;	// unknown files are bad
		for (;!feof(f);) {
			ptr = fgets(str, sizeof(str), f);
			if (!ptr) break;
			ptr = strtok(str, " \t");
			if (!ptr) continue;
			i = strtol(ptr, NULL, 10);
			if (i <= 0 || i >= MAXRUN) continue;
			ptr = strtok(NULL, " \t");
			if (!ptr) continue;
			type = strtol(ptr, NULL, 10);
			if (type <= 0) continue;
			ptr = strtok(NULL, " -\t");
			if (!ptr) continue;
			year = strtol(ptr, NULL, 10);
			if (year < 2016 || year > 2030) continue;
			ptr = strtok(NULL, " -\t");
			if (!ptr) continue;
			month = strtol(ptr, NULL, 10);
			if (month < 1 || month > 12) continue;
			ptr = strtok(NULL, " -\t");
			if (!ptr) continue;
			day = strtol(ptr, NULL, 10);
			if (day < 1 || day > 31) continue;
			RunList[i] = date2year(year, month, day);
			cnt++;
		}
		fclose(f);
		printf("%d good runs found\n", cnt);
	}
}

MyRunList::~MyRunList(void)
{
	if (RunList) free(RunList);
}

// approximate calculation
double MyRunList::date2year(int year, int month, int day)
{
	return ((year - 2016.0) + (month - 1) / 12.0 + (day - 1) / 365.0);
}

//	Usage: ./graph_1px fileout.root [run_begin run_end]
int main(int argc, char **argv)
{
	int runBegin = 0;
	int runEnd = MAXRUN-1;
	int i, j, N;
	char str[1024];
	TFile *fIn;
	TTree *tCalib;
	struct chanCalibStruct {
		int adc;
		int chan;
		float fConv;
		float f1px;
		float fXtalk;
	} chanCalib;
	TGraph *gr;
	double Year;

	if (argc < 2) {
		printf("Usage: %s fileout.root [run_begin run_end]\n", argv[0]);
		return 10;
	}
	
	TFile *fOut = new TFile(argv[1], "RECREATE");
	if (argc > 2) runBegin = strtol(argv[2], NULL, 10);
	if (argc > 3) runEnd = strtol(argv[3], NULL, 10);
	TList *gList = new TList();
	
	MyRunList *RunList = new MyRunList("../stat_all.txt");
	
	for (i = runBegin; i <= runEnd; i++) {
		if ((i%5000) == 0) printf("Run = %d\n", i);
		Year = RunList->GetRunYear(i);
		if (Year < 0) continue;
		sprintf(str, "%s/%3.3dxxx/vert_%6.6d.root", VDIR, i/1000, i);
		j = access(str, R_OK);
		if (j) continue;	// no file
		fIn = new TFile(str);
		if (!fIn->IsOpen()) {
			delete fIn;
			continue;
		}
		tCalib = (TTree *) fIn->Get("Calib");
		if (!tCalib) {
			printf("Tree Calib not found in %s\n", str);
			delete fIn;
			continue;
		}
		tCalib->SetBranchAddress("SiPM", &chanCalib);
		N = tCalib->GetEntries();
		for (j=0; j<N; j++) {
			tCalib->GetEntry(j);
			if (chanCalib.f1px <= 0) continue;
			sprintf(str, "g1px%2.2d_%2.2d", chanCalib.adc, chanCalib.chan);
			gr = (TGraph *) gList->FindObject(str);
			if (!gr) {
				gr = new TGraph();
				gr->SetName(str);
				gList->AddLast(gr);
			}
			gr->AddPoint(Year, chanCalib.f1px);
		}
		delete fIn;
	}

	fOut->cd();
	for(TObject *obj: *gList) obj->Write();
	fOut->Close();

	return 0;
}
