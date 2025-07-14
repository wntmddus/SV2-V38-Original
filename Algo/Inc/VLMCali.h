/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : VLMCali.h

Author(s)   : Sameer Ahamed P.S

Description : Data processing of VIB calibration (3-channel)

Context     :

Caution     : None

*****************************MODIFICATION HISTORY*******************************
				|      |          |
 Date			| Ver  | Author   | Description
----------------|------|----------|---------------------------------------------
				|      |          |
 1stJune2023	| 0.1  | sameer   | Fs=2205Hz/CH, VLM CALI algo creation
 6thJuly2023	| 0.2  | sameer   | Fs=2205Hz, output structures seprated
 9thNov2023     | 0.3  | sameer   | Fs=2756Hz, calibration corrections
##############################################################################*/

#pragma once
#include "stdlib.h"
#include "define_1634HzFs.h"

#define VLM_SKIP_COUNT       10   //After 100/10 = 10 seconds algo uses the stable calibration signal 
#define VLM_CALIBRATION_SECS 1    //kt  was 8 sec   //Around 0.8 second caliration signal is used after stability of calibration signal

//   
// bit  1  the input is too small 
// bit  2  unknown erro	system error	 
// bit  3  input overload
// bit  4  calculation error
// bit  6  the reference frequency maybe wrong 


typedef struct _VLMCALI
{
	long	iInputSignal[3][VLM_CALIBRATION_SECS * VLM_MAX_DATA_FEED_LENGTH];	//memory - sameer
	short	nCaliOver[3];					//0-calibration not done, 1-Calibration Over
	int		nDataFeedLength[3];
	int		nInputCount[3];
	int		nCaliInputState[3];			// -1, overload   -2, underrange  0, normal
	int		DataFeedLength[3];				// added to make the buffer length ansd sampling rate changable. 
	int		AmpGain[3];					//  add for independence from the iso8041std branch.
	int		iCalibrationReturnValue[3];	// > 0   result correct.
	float	fMICSensitivity;			// Mic sensitivity (MIC)  //2024
	float	fVibSensitivity[3];			// Vib sensor sensitivity //Sameer 2023.11.09
	float	fNorminalRms[3];				// the normial rms value of each sensor
	float	Frequency[3];					// the normial frequency value of each sensor     
	float	fCaliCoef[3][3];				// extented coef aray set     (no need to show in ui.)
	float	fOutputCoef[3];				// final result of each axis  (this three values need to show in ui.)
	float	fScaleCoef[3];
	float   CALIHPFDL[3][3][3];			//DC and low frequency removal HPF 
	float	VLMReferenceLevel[3];
	float   dBRef[3];
}_VLMCALI, * _LPVLMCALI;

extern _LPVLMCALI VLMCaliStrucPtr;

int		VLMCalibrationInit(struct _VLMCALI* abc, int ch);
int		VLMCalibrationByChannel(struct _VLMCALI* abc, short* ch0_input, int ch);
float	VLMChannelCaliRms(struct _VLMCALI* abc, const int len, int ch);
int		VLMCalibrationOver(struct _VLMCALI* abc, int ch);
void	VLMGetCrossCorrelation(struct _VLMCALI* abc, float* tx, const int len, const float omg, int ch);
void	VLMGetPseudoInverseCov(struct _VLMCALI* abc, float t[][2], const int len, const float omg, int ch);
float	VLMLms_Curve_Fitting(struct _VLMCALI* abc, const int len, int ch);
