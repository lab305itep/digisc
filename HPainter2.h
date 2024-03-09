#ifndef HPAINTER2_H
#define HPAINTER2_H

class TChain;
class TH1;
class TH2;

class HPainter2 {
private:
	TChain *tSig;
	TChain *tRand;
	TChain *tMuRand;
	double upTime;
	unsigned int tBegin;
	unsigned int tEnd;
	int Make_file_list(int *list, int size, int mask, int run_from, int run_to);
	TFile *fRes;
	int ClosefRes;
public:
	HPainter2(int mask, int run_from, int run_to, const char *root2dir, int type = 0);
	~HPainter2(void);
	inline int IsOpen(void) { return tSig && tRand; };
	inline void SetFile(TFile *f) { fRes = f; };
	void OpenFile(const char *name);
	void Project(TH1 *hist, const char *what, TCut cut, int iZeroErrCorr = 1);
//	void Project(TH2 *hist, const char *what, TCut cut);
//	void Project(TH3 *hist, const char *what, TCut cut);
	inline unsigned int GetBeginTime(void) { return tBegin; };
	inline double GetUpTime(void) { return upTime; };
	inline void SetUpTime(double tm) { upTime = tm; };
	void SetUpTime(unsigned int t0, unsigned int t1);
	inline TChain *GetPairChain(void) { return tSig;};
	inline TChain *GetRandomChain(void) { return tRand;};
	inline TChain *GetRandomMuChain(void) { return tMuRand;};
};

#endif

