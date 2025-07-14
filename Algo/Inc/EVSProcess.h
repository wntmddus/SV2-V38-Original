/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : EVSProcess.h

Author(s)   : Sameer Ahamed P.S

Description : Data processing of EVS MODE

Context     :

Caution     : None

*****************************MODIFICATION HISTORY*******************************
				|      |          |
 Date			| Ver  | Author   | Description
----------------|------|----------|---------------------------------------------
				|      |          |
 1stJune2023	| 0.1  | sameer   | Fs=2205Hz, EVS algo creation
 6thJuly2023	| 0.2  | sameer   | Fs=2205Hz, output structures seprated
 9thNov2023     | 0.3  | sameer   | Fs=2756Hz, calibration corrections
 25thNov2023    | 0.4  | sameer   | Trigger flag setting added in BLS mode
 4thDec2023     | 0.5  | sameer   | L10, Lmax, Integration time added in BLS mode
##############################################################################*/

#include <stdio.h>
#include <stdlib.h>

#include "define_1634HzFs.h"
#include "SLMCali.h"
#include "SLMProcess.h"
//kt #include "SLMCali.h"

//#define EVS_BUFFER_LENGTH 275  //64 //4096  
#define EVS_NUMBER_OF_CHANNELS 3

typedef struct _EVS_OUT
{
	float SoundOutput[7];  //Leq inst,Leq Avg,Leq MAX,Leq MIN,Peak Sound,lLAeqAvgCounter,LE
	float   ACCRMS[3];
	float   VELRMS[3];
	float   DISPRMS[3];
	float   fLveq[3];
	float   fLmax[3];
	float   fLmin[3];
	float   fLvAvg[3];
	float	LnOut[10];
	short   m_lFeedCount[3];  
	short	VibChTrigFound[3];  //Trigger, L10, Lmax and Integration time
	short	VibrationTriggerFound;	//kt 2024.01: unfortunately Mr.Sameer does not support logic behavior for VibChTrigFound function,
						//here is my modification to transfer VibChTrigFound event to system part
						//=1 if all channels crossed VibChTrigFound level
	short   SoundTriggerFound;  //True(1) of False(0)   //Sameer: 26th June, 2025

}  _EVS_OUT, * _LPEVS_OUT;

extern _LPEVS_OUT EVSOUTStructPtr;

typedef struct _EVS  //kt:  size=1+17*3+140*3+9+45+  4*3*275+9 =3835*4byte=15340
{		
	short   firstframe[3];
	
	float	LnInput;  
	float   AccdBReference[3];
	float   Exp[3];	
	float	ACCAveragedValue[3];
	float   VELAveragedValue[3];
	float   DISPAveragedValue[3];	
	float 	DISPOlddata[3];
	float   refreciprocal[3];
	float   LvAccumulator[3];
	float   LinearAveCount[3];
	float   VELOlddata[3];
	float   ACCOlddata[3];
	float   LinearCountNum[3];
	float	Sensitivity[3];
	float	AmpGain[3];
	float	CaliCoef[3];
	float	fCaliFactor[3];	
	long	countervalue[140];   
	float	levels[140];   
	float	cumulativesums[140];  
	float   fDLine2[3][3];
	float   fHPF2[3][5][3];
	
	float   HPFEVS[3][5][3];  //sam new
	
	float	ACCOut[3][EVS_BUFLEN];
	float	fCaliInput[3][EVS_BUFLEN];
	float	input[3][EVS_BUFLEN];
	float   EVSOut[3][EVS_BUFLEN];
	
	float   IntTimeDataNum[3];  //Trigger, L10, Lmax and Integration time
	float   IntegTime[3];  //Trigger, L10, Lmax and Integration time
	float   ACCRMSLOCAL[3];  
	float   VibTriggerLevel;  //Trigger, L10, Lmax and Integration time
	short	TriggerFoundResetAlgo;  //Trigger, L10, Lmax and Integration time
	
	float   SoundTriggerLevel;
	int   UseSroundTrigger;  //true (1) or false (0)

		
}   _EVS, * _LPEVS;

extern _LPEVS_OUT EVSOUTStructPtr;

extern _LPEVS EVSStructPtr;

short EVSProcess(struct _EVS* abc, struct _SLM* SLMStruct, struct _SLMCALI* abc1, struct _EVS_OUT* xyz);

short intEVSProcess(struct _EVS* abc, struct _SLM* SLMStruct, struct _SLMCALI* abc1, struct _EVS_OUT* xyz);

short ResetEVSAlgo(struct _EVS* abc, struct _EVS_OUT* xyz);  

short EVSLnDetector(struct _EVS* abc, struct _EVS_OUT* xyz);

