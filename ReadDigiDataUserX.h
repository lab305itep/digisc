#ifndef READDIGIDATAUSERX_H
#define READDIGIDATAUSERX_H

#include "TRandom2.h"
#include "readDigiData.h"

struct HitStructX {
	float e;
	float signal;
	float t0raw;
	float t0;
	float npe;
	float npix;
	float t;
	float t_raw;
	danssAdcType type;
	char side;
	int firstCoord;
	int zCoord;
	int indexByHit;
	int chanIndex;
	int adc;
	int adcChan;
	float t0Digi;
	float t0rawDigi;
	float t0rawRawDigi;
	int extraT;
	int nMaxima;
	float chi2;
	float signalDigi;
	float signalRawDigi;
	float A;
	float npeMc;
	float e_McTruth;
	bool wasOvfl;
	bool wasBadFit;
};

class ReadDigiDataUserX {
private:
	ReadDigiDataUser *user;
	int nOrigHits;
	int nTotalHits;
	struct HitStructX HitX[iMaxDataElements];
	TRandom2 *rndm;
	bool isNoise;
public:
	ReadDigiDataUserX(ReadDigiDataUser *u, bool MCwithNoise = false);
	void BeginEvent(void);
	//	from class ReadDigiDataUser
	int nhits();
	float e(int iHitNum);
	float signal(int iHitNum);
	float t0raw(int iHitNum);
	float t0(int iHitNum);
	float npe(int iHitNum, float fSignalValue = iUnPhysicalSetting);
	float npix(int iHitNum, float fSignalValue = iUnPhysicalSetting);
	float t(int iHitNum);
	float t_raw(int iHitNum);
	danssAdcType type(int iHitNum);
	int coords(int iHitNum, char &rcSide, int &riFirstCoord, int &riZCoord);
	char side(int iHitNum);
	int firstCoord(int iHitNum);
	int zCoord(int iHitNum);
	int indexByHit(int iHitNum);
//	int cellIndicesByHit(int iHitNum, int *piPlaceToStoreIndices);
	bool isBadChannel(int iAdcChannelIndex);
	bool isBadMip(int iAdcChannelIndex);
	bool isBadMipFull(int iAdcChannelIndex);
	//	from class DanssSpcDigiEvent
	int absTime();
	long long int globalTime();
	danssMasterTrgTypes masterTriggerType();
	int chanIndex(int iHitNum);
	int adc(int iHitNum);
	int adcChan(int iHitNum);
	float t0Digi(int iHitNum);
	float t0rawDigi(int iHitNum);
	float t0rawRawDigi(int iHitNum);
	int extraT(int iHitNum);
	int nMaxima(int iHitNum);
	float chi2(int iHitNum);
	float signalDigi(int iHitNum);
	float signalRawDigi(int iHitNum);
	float A(int iHitNum);
	float npeMc(int iHitNum);
	float e_McTruth(int iHitNum);
	bool wasOvfl(int iHitNum);
	bool wasBadFit(int iHitNum);
};

#endif	// READDIGIDATAUSERX_H
