/****************************************************************
 *	Hook class to add SiPM noise. May be something more...	*
 ****************************************************************/
#include "ReadDigiDataUserX.h"

const double NoiseTmin = 100.0;					// noise time window begin, ns
const double NoiseTmax = 400.0;					// noise time window end, ns
const double NoiseAvr = 5.9E-5 * (NoiseTmax - NoiseTmin);	// in one channel per event
const double Xtalk = 1.37;					// Average Crosstalk
const double phe2MeV = 21.21;					// Average phe per MeV

ReadDigiDataUserX::ReadDigiDataUserX(ReadDigiDataUser *u, bool MCwithNoise)
{
	user = u;
	isNoise = MCwithNoise;
	rndm = new TRandom2();
}

void ReadDigiDataUserX::BeginEvent(void)
{
	int i, j, k;
	double r, t;
	
	if (!isNoise) return;
//	Copy original hits
	nTotalHits = nOrigHits = user->nhits();
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
		j = rndm->Poisson(NoiseAvr);	// number of hits in the channel
		if (!j) continue;
		r = j * Xtalk;
		k = rndm->Poisson(r);		// number of pixels 
		if (!k) continue;
		t = NoiseTmin + (NoiseTmax - NoiseTmin) * rndm->Rndm();
		for (j=0; j < nOrigHits; j++) if (i == HitX[j].chanIndex) break;
		if (j < nOrigHits) {	// Add to existing hit. Keep oroiginal time, type etc.
			HitX[j].e += k * 1.0 / (Xtalk * phe2MeV);
			HitX[j].signal += k * 1.0 / (Xtalk * phe2MeV);
			HitX[j].npe += k / Xtalk;
			HitX[j].npix += k;
			HitX[j].A += 12.0 * k;
		} else {		// Create new hit
			HitX[nTotalHits].e = k * 1.0 / (Xtalk * phe2MeV);
			HitX[nTotalHits].signal = k * 1.0 / (Xtalk * phe2MeV);
			HitX[nTotalHits].t0raw = t;	// we will put t in all times
			HitX[nTotalHits].t0 = t;	// we will put t in all times
			HitX[nTotalHits].npe = k / Xtalk;
			HitX[nTotalHits].npix = k;
			HitX[nTotalHits].t = t;	// we will put t in all times
			HitX[nTotalHits].t_raw = t;	// we will put t in all times
			HitX[nTotalHits].type = bSiPm;
			user->coordsByCellIndex(i, HitX[nTotalHits].side, 
				HitX[nTotalHits].firstCoord, HitX[nTotalHits].zCoord); 
			HitX[nTotalHits].indexByHit = i;	// no difference ?
			HitX[nTotalHits].chanIndex = i;		// no difference ?
			user->adcParsByCellIndex(i, HitX[nTotalHits].adc, HitX[nTotalHits].adcChan);
			HitX[nTotalHits].t0Digi = t;	// we will put t in all times
			HitX[nTotalHits].t0rawDigi = t;	// we will put t in all times
			HitX[nTotalHits].t0rawRawDigi = t;	// we will put t in all times
			HitX[nTotalHits].extraT = (int) (t/8);	// we will put t in all times
			HitX[nTotalHits].nMaxima = (int) ((t + 20.0) / 8); // we will put t in all times
			HitX[nTotalHits].chi2 = 1.0;	// something
			HitX[nTotalHits].signalDigi = 0;	// to be done
			HitX[nTotalHits].signalRawDigi = 0;	// to be done
			HitX[nTotalHits].A = 12.0 * k;		// something close to amplitude
			HitX[nTotalHits].npeMc = 0;		// this is noise
			HitX[nTotalHits].e_McTruth = 0;		// this is noise
			HitX[nTotalHits].wasOvfl = false;
			HitX[nTotalHits].wasBadFit = false;
			nTotalHits++;
		}
	}
}

/********************************
 *	from ReadDigiDataUser	*
 ********************************/

int ReadDigiDataUserX::nhits()
{
	if (!isNoise) return user->nhits();
	return nTotalHits;
}

float ReadDigiDataUserX::e(int iHitNum)
{
	if (!isNoise) return user->e(iHitNum);
	return HitX[iHitNum].e;
}

float ReadDigiDataUserX::signal(int iHitNum)
{
	if (!isNoise) return user->signal(iHitNum);
	return HitX[iHitNum].signal;
}

float ReadDigiDataUserX::t0raw(int iHitNum)
{
	if (!isNoise) return user->t0raw(iHitNum);
	return HitX[iHitNum].t0raw;
}

float ReadDigiDataUserX::t0(int iHitNum)
{
	if (!isNoise) return user->t0(iHitNum);
	return HitX[iHitNum].t0;
}

float ReadDigiDataUserX::npe(int iHitNum, float fSignalValue)
{
	if (!isNoise) return user->npe(iHitNum, fSignalValue);
	return HitX[iHitNum].npe;
}

float ReadDigiDataUserX::npix(int iHitNum, float fSignalValue)
{
	if (!isNoise) return user->npix(iHitNum, fSignalValue);
	return HitX[iHitNum].npix;
}

float ReadDigiDataUserX::t(int iHitNum)
{
	if (!isNoise) return user->t(iHitNum);
	return HitX[iHitNum].t;
}

float ReadDigiDataUserX::t_raw(int iHitNum)
{
	if (!isNoise) return user->t_raw(iHitNum);
	return HitX[iHitNum].t_raw;
}

danssAdcType ReadDigiDataUserX::type(int iHitNum)
{
	if (!isNoise) return user-> type(iHitNum);
	return HitX[iHitNum].type;
}

int ReadDigiDataUserX::coords(int iHitNum, char &rcSide, int &riFirstCoord, int &riZCoord)
{
	if (!isNoise) return user->coords(iHitNum, rcSide, riFirstCoord, riZCoord);
}

char ReadDigiDataUserX::side(int iHitNum)
{
	if (!isNoise) return user->side(iHitNum);
	return HitX[iHitNum].side;
}

int ReadDigiDataUserX::firstCoord(int iHitNum)
{
	if (!isNoise) return user->firstCoord(iHitNum);
	return HitX[iHitNum].firstCoord;
}

int ReadDigiDataUserX::zCoord(int iHitNum)
{
	if (!isNoise) return user->zCoord(iHitNum);
	return HitX[iHitNum].zCoord;
}

int ReadDigiDataUserX::indexByHit(int iHitNum)
{
	if (!isNoise) return user->indexByHit(iHitNum);
	return HitX[iHitNum].indexByHit;
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
	return HitX[iHitNum].chanIndex;
}

int ReadDigiDataUserX::adc(int iHitNum)
{
	if (!isNoise) return user->adc(iHitNum);
	return HitX[iHitNum].adc;
}

int ReadDigiDataUserX::adcChan(int iHitNum)
{
	if (!isNoise) return user->adcChan(iHitNum);
	return HitX[iHitNum].adcChan;
}

float ReadDigiDataUserX::t0Digi(int iHitNum)
{
	if (!isNoise) return user->t0Digi(iHitNum);
	return HitX[iHitNum].t0Digi;
}

float ReadDigiDataUserX::t0rawDigi(int iHitNum)
{
	if (!isNoise) return user->t0rawDigi(iHitNum);
	return HitX[iHitNum].t0rawDigi;
}

float ReadDigiDataUserX::t0rawRawDigi(int iHitNum)
{
	if (!isNoise) return user->t0rawRawDigi(iHitNum);
	return HitX[iHitNum].t0rawRawDigi;
}

int ReadDigiDataUserX::extraT(int iHitNum)
{
	if (!isNoise) return user->extraT(iHitNum);
	return HitX[iHitNum].extraT;

}

int ReadDigiDataUserX::nMaxima(int iHitNum)
{
	if (!isNoise) return user->nMaxima(iHitNum);
	return HitX[iHitNum].nMaxima;
}

float ReadDigiDataUserX::chi2(int iHitNum)
{
	if (!isNoise) return user->chi2(iHitNum);
	return HitX[iHitNum].chi2;
}

float ReadDigiDataUserX::signalDigi(int iHitNum)
{
	if (!isNoise) return user->signalDigi(iHitNum);
	return HitX[iHitNum].signalDigi;
}

float ReadDigiDataUserX::signalRawDigi(int iHitNum)
{
	if (!isNoise) return user->signalRawDigi(iHitNum);
	return HitX[iHitNum].signalRawDigi;
}

float ReadDigiDataUserX::A(int iHitNum)
{
	if (!isNoise) return user->A(iHitNum);
	return HitX[iHitNum].A;
}

float ReadDigiDataUserX::npeMc(int iHitNum)
{
	if (!isNoise) return user->npeMc(iHitNum);
	return HitX[iHitNum].npeMc;
}

float ReadDigiDataUserX::e_McTruth(int iHitNum)
{
	if (!isNoise) return user->e_McTruth(iHitNum);
	return HitX[iHitNum].e_McTruth;
}

bool ReadDigiDataUserX::wasOvfl(int iHitNum)
{
	if (!isNoise) return user->wasOvfl(iHitNum);
	return HitX[iHitNum].wasOvfl;
}

bool ReadDigiDataUserX::wasBadFit(int iHitNum)
{
	if (!isNoise) return user->wasBadFit(iHitNum);
	return HitX[iHitNum].wasBadFit;
}
