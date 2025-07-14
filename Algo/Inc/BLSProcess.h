/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : BLSProcess.h

Author(s)   : Sameer Ahamed P.S

Description : Data processing of BLS MODE

Context     :

Caution     : None

*****************************MODIFICATION HISTORY*******************************
				|      |          |
 Date			| Ver  | Author   | Description
----------------|------|----------|---------------------------------------------
				|      |          |
 1stJune2023	| 0.1  | sameer   | Fs=2205Hz, BLS algo creation
 6thJuly2023	| 0.2  | sameer   | Fs=2205Hz, output structures seprated
 9thNov2023     | 0.3  | sameer   | Fs=2756Hz, calibration corrections
 25thNov2023    | 0.4  | sameer   | Trigger flag setting added in BLS mode
 4thDec2023     | 0.5  | sameer   | L10, Lmax, Integration time added in BLS mode
##############################################################################*/

#ifndef BLSCODE_H_INCLUDED
#define BLSCODE_H_INCLUDED


#include <define_1634HzFs.h>
#include <stdio.h>
#include <stdlib.h>

#include <SLMCali.h>
#include <SLMProcess.h>


//#define BLS_BUFFER_LENGTH 275  //64 //4096  
#define BLS_NUMBER_OF_CHANNELS 3

typedef struct _BLS_OUT
{
	float SoundOutput[7];  //Leq inst,Leq Avg,Leq MAX,Leq MIN,Peak Sound,lLAeqAvgCounter,LE
	float PeakVectorSum;
	float PeakVectSumTime;
	float zcf[3];
	float DispPeak[3];
	float VelocityP2P[3];
	float VelocityPeakInst[3];
	float AccPeak[3];
	float TimeOfVelPeak[3];
	float   ACCRMS[3];
	float   VELRMS[3];
	float   VELdB[3];
	float VelocityPeakHold[3]; 

	//new output: EVS related
	float   fLveqBLS[3]; //Trigger, L10, Lmax and Integration time
	float   fLmaxBLS[3];//Trigger, L10, Lmax and Integration time
	float	LnOutBLS[3];//L10 for all three channels //Trigger, L10, Lmax and Integration time
	short	VibChTrigFound[3]; //Trigger, L10, Lmax and Integration time
	short   m_lFeedCount[3];//Trigger, L10, Lmax and Integration time
	short	VibrationTriggerFound;	//kt 2024.01: unfortunately Mr.Sameer does not support logic behavior for VibChTrigFound function,
						//here is my modification to transfer VibChTrigFound event to system part
						//=1 if all channels crossed VibChTrigFound level

	short   SoundTriggerFound;  //True(1) of False(0)   //Sameer: 26th June, 2025
}  _BLS_OUT, * _LPBLS_OUT;

extern _LPBLS_OUT BLSOUTStructPtr;
	
typedef struct _BLS
{
	short   firstframe[3];
	float	TotalCount;
	float	CoefVel_2;
	float	CoefVel_1;
	float	CoefDisp_1;
	float	CoefDisp_2;
	float   GerbdBReference[3];
	float   VELOlddata[3];
	float   ACCOlddata[3];
	float	ACCAveragedValue[3];
	float   VELAveragedValue[3];
	float   Exp[3];
	long    ZCFreqCount[3]; 
	float   fHPF2BLS[3][5][3];
	float   EVSOutBLS[3][EVS_BUFLEN];
	float   fDLine2BLS[3][3];
	float   LinearCountNumBLS[3];
	float	cumulativesumsBLS[3][140];
	float   refreciprocalBLS[3];
	float   AccdBReferenceBLS[3];
	float   ACCRMSLOCAL[3];
	float   tempRMS[3];
	
	float   HPFBLS[3][5][3];  //sam new
	float   LPFBLS[3][5][3];  //sam new

	float	Sensitivity[3];
	float	AmpGain[3];
	float	CaliCoef[3];
	float	fCaliFactor[3];
	float	DispDLine[3][3][3];
	float	VelDLine[3][3][3];
	float	IntgrationDL[3][3][3];
	float	fBPF[3][3][3];
	float	fBPFVEL1[3][3][3];
	float	VectSum[BLS_BUFLEN];
	float	midbuf[3][BLS_BUFLEN];
	float	fCaliInput[3][BLS_BUFLEN];
	float	input[3][BLS_BUFLEN];
	float	VelocityPVS[3][BLS_BUFLEN];
	float	ACCOut[3][BLS_BUFLEN];
	float	VELOut[3][BLS_BUFLEN];
	float	DISPOut[3][BLS_BUFLEN];
	
	//New parameters: EVS related
	float	LnInputBLS[3];//Trigger, L10, Lmax and Integration time
	float   ACCRMS[3];//Trigger, L10, Lmax and Integration time
	float	levelsBLS[3][140];//Trigger, L10, Lmax and Integration time
	float	countervalueBLS[3][140];//Trigger, L10, Lmax and Integration time
	float   IntTimeDataNum[3];//Trigger, L10, Lmax and Integration time
	float   IntegTime[3];//Trigger, L10, Lmax and Integration time
	float   VibTriggerLevel;//Trigger, L10, Lmax and Integration time
	short	TriggerFoundResetAlgo;//Trigger, L10, Lmax and Integration time

	float   SoundTriggerLevel;
	int   UseSroundTrigger;  //true (1) or false (0)


	//--------------------------
}   _BLS, * _LPBLS;

extern _LPBLS BLSStructPtr;

short BLSProcess(struct _BLS* abc, struct _SLM* SLMStruct, struct _SLMCALI* abc1, struct _BLS_OUT *xyz);

short intBLSProcess(struct _BLS* abc, struct _SLM* SLMStruct, struct _SLMCALI* abc1, struct _BLS_OUT* xyz);

short ResetBLSAlgo(struct _BLS* abc, struct _BLS_OUT* xyz);

short ZeroCrossingFrequency(struct _BLS* abc, int ch, struct _BLS_OUT* xyz);

short LnDetectorCalledFromBLS(struct _BLS* Strptr, struct _BLS_OUT* blsout, short ch);//Trigger, L10, Lmax and Integration time

#endif // BLSCODE_H_INCLUDED
