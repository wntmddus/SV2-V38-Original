/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : SLMProcess.h

Author(s)   : Sameer Ahamed P.S

Description : Data processing of SLM

Context     :

Caution     : None

*****************************MODIFICATION HISTORY*******************************
				|      |          |
 Date			| Ver  | Author   | Description
----------------|------|----------|---------------------------------------------
				|      |          |
 1stJune2023	| 0.1  | sameer   | Fs=22050Hz, SLM algo creation
##############################################################################*/
#pragma once
#include "stdlib.h"

#include "define_1634HzFs.h"
#include "SLMCali.h"

#define _HAVE_STDC

typedef enum SoundProcessMode {
	LEQ, //0
} SOUNDPROCESSMODE;

typedef enum SoundWeightChoice {
	Z_WT,
	A_WT,
	B_WT,
	C_WT
} SOUNDWEIGHT;

typedef enum SoundtIMEWeightChoice {
	F,
	Su,
	I
} SOUNDTIMEWEIGHT;

typedef struct _SLM
{
	float SoundOutput[7];  //Leq inst,Leq Avg,Leq MAX,Leq MIN,Peak Sound,lLAeqAvgCounter,LE
		
	int SoundQuant;
	int WeightChoice;
	int TimeWeight;
	int IntegTimeStop;
	int m_bStop;
	float /*long*/ LinearCountNum;
	long lLAeqAvgCounter;
	long CountIntTimeDataNum;
	float Reference;
	float fPercLev1;
	float nIntegralTime;
	float m_TimeInterval;
	float LAeqAvgTime;
	float IntTimeDataNum;
	float m_MinPeak;
	float m_MaxPeak;
	float fLAeqAccumulate;
	float fLAeqAvgValue;
	float fPeakTemp;
	float Olddata;
	float OldPeakData;
	float dlslm[3];
	float Sounddl[3][3];
	float HPF1[3][3];
	float Exp;
	float DecayExp;
	float IsInitialized;
	float Detector;
	float RMSPeakOldData;
	float recRecordTime;
	float m_Gain;
	float LowerLimitSoundSquaredB; 
	float dBReference;
	float fPeakSoundTime;
	float DataWriteFlag;
	float SLMfeedin[SLM_MAX_DATA_LEN];
	short SLMfeedinS[SLM_MAX_DATA_LEN];

	long lLAeqAvgCounter_5min;
	long CountIntTimeDataNum_5min;
	float LAeqAvgTime_5min;
	float IntTimeDataNum_5min;
	float fLAeqAccumulate_5min;
	float fLAeqAvgValue_5min;
	float Olddata_5min;
	float Detector_5min;
	float Exp_5min;


	float *LnInput;  // Ln input
	float fPercLev;
	float LnOut[10];

	float   SoundTriggerLevel;
	int   	UseSroundTrigger;  //true (1) or false (0)
	short   SoundTriggerFound;  //True(1) of False(0)   //Sameer: 26th June, 2025


}  _SLM, *_LPSLM;

extern _LPSLM SLMStructPtr;  

void SLMProcess(struct _SLM *SLMStruct);
void InitSLMProcess(struct  _SLM *SLMStruct, struct _SLMCALI *SLMCaliStruct);
void SoundHPF(struct _SLM *SLMStruct, float *out);
void SoundWeighting(struct _SLM *SLMStruct);
void SoundRMSDetector(struct _SLM *SLMStruct);
void SoundPeakDetector(struct _SLM *SLMStruct);
void SoundUpdateCounters(struct _SLM *SLMStruct);
void Sound_TriggerFound_ResetAlgo(struct _SLM *SLMStruct);
void SoundConvertUnit(struct _SLM *SLMStruct);
void SoundMaxMinDetector(struct _SLM *SLMStruct);
void SoundAccumulateDetector(struct _SLM *SLMStruct); 


