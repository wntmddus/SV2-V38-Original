/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : SLMCali.h

Author(s)   : Sameer Ahamed P.S

Description : Data processing of sound calibration 

Context     :

Caution     : None

*****************************MODIFICATION HISTORY*******************************
				|      |          |
 Date			| Ver  | Author   | Description
----------------|------|----------|---------------------------------------------
				|      |          |
 1stJune2023	| 0.1  | sameer   | Fs=22050Hz, SLM CALI algo creation
##############################################################################*/


#pragma once
#include <define_1634HzFs.h>
#include "stdlib.h"

#define SLM_SKIP_COUNT       10   //After 100/10 = 10 seconds algo uses the stable calibration signal  //Sameer, 2023.11.09
#define SLM_CALIBRATION_SECS 1      //Around 0.8 second caliration signal is used after stability of calibration signal 

//   
// bit  1  the input is too small 
// bit  2  unknown erro	system error	 
// bit  3  input overload
// bit  4  calculation error
// bit  6  the reference frequency maybe wrong 


typedef struct _SLMCALI
{
		long	iInputSignal[SLM_CALIBRATION_SECS*SLM_MAX_DATA_FEED_LENGTH];	//memory - sameer
		short	nCaliOver;					//0-calibration not done, 1-Calibration Over
		int		nDataFeedLength;
		int		nInputCount;
		int		nCaliInputState;			// -1, overload   -2, underrange  0, normal
		int		DataFeedLength;				// added to make the buffer length ansd sampling rate changable. 
		int		AmpGain;					//  add for independence from the iso8041std branch.
		int		iCalibrationReturnValue;	// > 0   result correct.
		float	fMICSensitivity;			// Mic sensitivity (MIC)
		float	fNorminalRms;				// the normial rms value of each sensor
		float	Frequency;					// the normial frequency value of each sensor     
		float	fCaliCoef[3];				// extented coef aray set     (no need to show in ui.)
		float	fOutputCoef;				// final result of each axis  (this three values need to show in ui.)
		float	fScaleCoef;
		float   SLMCALIHPFDL[3][3];			//DC and low frequency removal HPF  
		float	SLMReferenceLevel;
		float   dBRef;
}_SLMCALI, *_LPSLMCALI;

extern _LPSLMCALI SLMCaliStrucPtr;

int		SLMCalibrationInit( struct _SLMCALI *abc );
int		SLMCalibrationByChannel(struct _SLMCALI* abc, short* ch0_input);   //Sameer, 2023.11.09
float	SLMChannelCaliRms(struct _SLMCALI *abc ,const int len);
int		SLMCalibrationOver(struct _SLMCALI *abc );
void	SLMGetCrossCorrelation(struct _SLMCALI *abc , float *tx,  const int len,  const float omg );
void	SLMGetPseudoInverseCov(struct _SLMCALI *abc ,float t[][2],  const int len,  const float omg );
float	SLMLms_Curve_Fitting(struct _SLMCALI *abc , const int len);

