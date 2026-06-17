/****************************************************************
 *	Hook class to add SiPM noise. May be something more...	*
 ****************************************************************/
#include "ReadDigiDataUserX.h"

const double NoiseTmin = 100.0;					// noise time window begin, ns
const double NoiseTmax = 400.0;					// noise time window end, ns
const double NoiseAvr = 1.0E-4 * (NoiseTmax - NoiseTmin);	// in one channel per event
const double Xtalk = 1.37;					// Average Crosstalk
const double phe2MeV = 21.21;					// Average phe per MeV

ReadDigiDataUserX::ReadDigiDataUserX(ReadDigiDataUser *u, bool MCwithNoise)
{
	int i, j;
	
	user = u;
	isNoise = MCwithNoise;
	if (!isNoise) return;
//	Copy original hits
	nOrigHits = user->nhits();
	for (i=0; i<nOrigHits; i++) {
		HitX[i].e = user->e(i);
		HitX[i].signal = user->signal(i);
		HitX[i].t0raw = user->t0raw(i);
		HitX[i].t0 = user->t0(i);
		HitX[i].npe = user->npe(i);
		HitX[i].npix = user->npix(i);
		HitX[i].t = user->t(i);
		HitX[i].t_raw = user->t_raw(i);
		HitX[i].type = user->type(i);
		HitX[i].side = user->side(i);
		HitX[i].firstCoord = user->firstCoord(i);
		HitX[i].zCoord = user->zCoord(i);
		HitX[i].indexByHit = user->indexByHit(i);
		HitX[i].chanIndex = user->chanIndex(i);
		HitX[i].adc = user->adc(i);
		HitX[i].adcChan = user->adcChan(i);
		HitX[i].t0Digi = user->t0Digi(i);
		HitX[i].t0rawDigi = user->t0rawDigi(i);
		HitX[i].t0rawRawDigi = user->t0rawRawDigi(i);
		HitX[i].extraT = user->extraT(i);
		HitX[i].nMaxima = user->nMaxima(i);
		HitX[i].chi2 = user->chi2(i);
		HitX[i].signalDigi = user->signalDigi(i);
		HitX[i].signalRawDigi = user->signalRawDigi(i);
		HitX[i].A = user->A(i);
		HitX[i].npeMc = user->npeMc(i);
		HitX[i].e_McTruth = user->e_McTruth(i);
		HitX[i].wasOvfl = user->wasOvfl(i);
		HitX[i].wasBadFit = user->wasBadFit(i);
	}
//	Generate Additional hits
	for (i=0; i<iN_Strips; i++) {
		j = rndm->Poisson(NoiseAvr);
		if (!j) continue;
		//////////////??????????????????
	}
}

/********************************
 *	from ReadDigiDataUser	*
 ********************************/

int ReadDigiDataUserX::nhits()
{
	if (!isNoise) return user->nhits();
}

float ReadDigiDataUserX::e(int iHitNum)
{
	if (!isNoise) return user->e(iHitNum);
}

float ReadDigiDataUserX::signal(int iHitNum)
{
	if (!isNoise) return user->signal(iHitNum);
}

float ReadDigiDataUserX::t0raw(int iHitNum)
{
	if (!isNoise) return user->t0raw(iHitNum);
}

float ReadDigiDataUserX::t0(int iHitNum)
{
	if (!isNoise) return user->t0(iHitNum);
}

float ReadDigiDataUserX::npe(int iHitNum, float fSignalValue)
{
	if (!isNoise) return user->npe(iHitNum, fSignalValue);
}

float ReadDigiDataUserX::npix(int iHitNum, float fSignalValue)
{
	if (!isNoise) return user->npix(iHitNum, fSignalValue);
}

float ReadDigiDataUserX::t(int iHitNum)
{
	if (!isNoise) return user->t(iHitNum);
}

float ReadDigiDataUserX::t_raw(int iHitNum)
{
	if (!isNoise) return user->t_raw(iHitNum);
}

danssAdcType ReadDigiDataUserX::type(int iHitNum)
{
	if (!isNoise) return user-> type(iHitNum);
}

int ReadDigiDataUserX::coords(int iHitNum, char &rcSide, int &riFirstCoord, int &riZCoord)
{
	if (!isNoise) return user->coords(iHitNum, rcSide, riFirstCoord, riZCoord);
}

char ReadDigiDataUserX::side(int iHitNum)
{
	if (!isNoise) return user->side(iHitNum);
}

int ReadDigiDataUserX::firstCoord(int iHitNum)
{
	if (!isNoise) return user->firstCoord(iHitNum);
}

int ReadDigiDataUserX::zCoord(int iHitNum)
{
	if (!isNoise) return user->zCoord(iHitNum);
}

int ReadDigiDataUserX::indexByHit(int iHitNum)
{
	if (!isNoise) return user->indexByHit(iHitNum);
}

// int ReadDigiDataUserX::cellIndicesByHit(int iHitNum, int *piPlaceToStoreIndices)
// {
//	return user->cellIndicesByHit(iHitNum, piPlaceToStoreIndices);
// }

bool ReadDigiDataUserX::isBadChannel(int iAdcChannelIndex)
{
	return user->isBadChannel(iAdcChannelIndex);
}

bool ReadDigiDataUserX::isBadMip(int iAdcChannelIndex)
{
	return user->isBadMip(iAdcChannelIndex);
}

bool ReadDigiDataUserX::isBadMipFull(int iAdcChannelIndex)
{
	return user->isBadMipFull(iAdcChannelIndex);
}

/********************************
 *	from DanssSpcDigiEvent	*
 ********************************/
int ReadDigiDataUserX::absTime()
{
	return user->absTime();
}

long long int ReadDigiDataUserX::globalTime()
{
	return user->globalTime();
}

danssMasterTrgTypes ReadDigiDataUserX::masterTriggerType()
{
	return user->masterTriggerType();
}

int ReadDigiDataUserX::chanIndex(int iHitNum)
{
	if (!isNoise) return user->chanIndex(iHitNum);
}

int ReadDigiDataUserX::adc(int iHitNum)
{
	if (!isNoise) return user->adc(iHitNum);
}

int ReadDigiDataUserX::adcChan(int iHitNum)
{
	if (!isNoise) return user->adcChan(iHitNum);
}

float ReadDigiDataUserX::t0Digi(int iHitNum)
{
	if (!isNoise) return user->t0Digi(iHitNum);
}

float ReadDigiDataUserX::t0rawDigi(int iHitNum)
{
	if (!isNoise) return user->t0rawDigi(iHitNum);
}

float ReadDigiDataUserX::t0rawRawDigi(int iHitNum)
{
	if (!isNoise) return user->t0rawRawDigi(iHitNum);
}

int ReadDigiDataUserX::extraT(int iHitNum)
{
	if (!isNoise) return user->extraT(iHitNum);
}

int ReadDigiDataUserX::nMaxima(int iHitNum)
{
	if (!isNoise) return user->nMaxima(iHitNum);
}

float ReadDigiDataUserX::chi2(int iHitNum)
{
	if (!isNoise) return user->chi2(iHitNum);
}

float ReadDigiDataUserX::signalDigi(int iHitNum)
{
	if (!isNoise) return user->signalDigi(iHitNum);
}

float ReadDigiDataUserX::signalRawDigi(int iHitNum)
{
	if (!isNoise) return user->signalRawDigi(iHitNum);
}

float ReadDigiDataUserX::A(int iHitNum)
{
	if (!isNoise) return user->A(iHitNum);
}

float ReadDigiDataUserX::npeMc(int iHitNum)
{
	if (!isNoise) return user->npeMc(iHitNum);
}

float ReadDigiDataUserX::e_McTruth(int iHitNum)
{
	if (!isNoise) return user->e_McTruth(iHitNum);
}

bool ReadDigiDataUserX::wasOvfl(int iHitNum)
{
	if (!isNoise) return user->wasOvfl(iHitNum);
}

bool ReadDigiDataUserX::wasBadFit(int iHitNum)
{
	if (!isNoise) return user->wasBadFit(iHitNum);
}
