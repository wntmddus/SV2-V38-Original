/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : SLMCali.c

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
 20thMar2025    | 0.7  | sameer   | fs=24kHz for certification   
##############################################################################*/


//#include "stdafx.h"
#include "SLMCali.h"


_SLMCALI SLMCaliStruct;
_LPSLMCALI SLMCaliStrucPtr = &SLMCaliStruct;

//kt - really not used          extern long shared_buffer[SLM_CALIBRATION_SECS*SLM_MAX_DATA_FEED_LENGTH];

const  float SLMCaliHPF10Hz[3][6] =
{
	{1.0f, - 2.0f,  1.0f,  1.0f, - 1.9970218925781167f,  0.99705432364590407f},
	{1.0f, - 2.0f,  1.0f,  1.0f, - 1.9919403917769984f,  0.99197274032265736f},
	{1.0f, - 2.0f,  1.0f,  1.0f, - 1.9890183438588338f,  0.98905064495126527f}
};
const double  SLMCaliHPF10HzGain = 0.9891f;

/////////////////////////////////////////////////////////////////////////////////////////////
int SLMCalibrationInit( struct _SLMCALI *SLMCaliStruct )
{	
	SLMCaliStruct->nCaliOver   =  0;//calibration not done
	SLMCaliStruct->nInputCount = 0;
	SLMCaliStruct->iCalibrationReturnValue=0;
	SLMCaliStruct->nCaliInputState=0;
	SLMCaliStruct->fScaleCoef  = SLM_CALI_SCALE_CONSTANT / SLMCaliStruct->fMICSensitivity / SLMCaliStruct->AmpGain ;
	SLMCaliStruct->fCaliCoef[0]=1.0f;  
	SLMCaliStruct->fCaliCoef[1]=1.0f;
	SLMCaliStruct->fCaliCoef[2]=1.0f;
	SLMCaliStruct->fOutputCoef = 1.0f;
	//SLMCaliStruct->iInputSignal = (long *)shared_buffer;

	SLMCaliStruct->fNorminalRms = (float)SLMCaliStruct->dBRef * (float)pow(10.0f, SLMCaliStruct->SLMReferenceLevel / 20.0f);

	//-----------------Gain calculations------------------------------------
	//SLMCaliStruct->fScaleCoef = (DSPTYPE)(INPUTMAXVOLTAGE * 1000.0f / (SLMCaliStruct->fMICSensitivity*(DSPTYPE)SLMCaliStruct->AmpGain*QUANTIZEMAXVALUE));


	//-------------End of Gain Calculations---------------------------------

	// DC removal filter
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
			SLMCaliStruct->SLMCALIHPFDL[i][j] = 0.0f;
	}
	
	return 1;
}
double temp[SLM_MAX_DATA_FEED_LENGTH];
int SLMCalibrationByChannel(struct _SLMCALI* SLMCaliStruct, short* ch0_input) //Sameer, 2023.11.09
{

	//HPF to remove DC offset and low frequeny from cali signal
	// DC removal filter 
	for (int cn = 0; cn<SLM_MAX_DATA_FEED_LENGTH; cn++)   //I
	{
		SLMCaliStruct->SLMCALIHPFDL[1][2] = SLMCaliStruct->SLMCALIHPFDL[1][1];
		SLMCaliStruct->SLMCALIHPFDL[1][1] = SLMCaliStruct->SLMCALIHPFDL[1][0];
        SLMCaliStruct->SLMCALIHPFDL[1][0] = (float)(ch0_input[cn])-SLMCaliStruct->SLMCALIHPFDL[1][1] * SLMCaliHPF10Hz[0][4] - SLMCaliStruct->SLMCALIHPFDL[1][2] * SLMCaliHPF10Hz[0][5];
		temp[cn] = SLMCaliStruct->SLMCALIHPFDL[1][0] * SLMCaliHPF10Hz[0][0] + SLMCaliStruct->SLMCALIHPFDL[1][1] * SLMCaliHPF10Hz[0][1] + SLMCaliStruct->SLMCALIHPFDL[1][2] * SLMCaliHPF10Hz[0][2];
	};

	for (int cn = 0; cn<SLM_MAX_DATA_FEED_LENGTH; cn++)  // II
	{
		SLMCaliStruct->SLMCALIHPFDL[0][2] = SLMCaliStruct->SLMCALIHPFDL[0][1];
		SLMCaliStruct->SLMCALIHPFDL[0][1] = SLMCaliStruct->SLMCALIHPFDL[0][0];
		SLMCaliStruct->SLMCALIHPFDL[0][0] = (float)(temp[cn]) - SLMCaliStruct->SLMCALIHPFDL[0][1] * SLMCaliHPF10Hz[1][4] - SLMCaliStruct->SLMCALIHPFDL[0][2] * SLMCaliHPF10Hz[1][5];
		temp[cn] = SLMCaliStruct->SLMCALIHPFDL[0][0] * SLMCaliHPF10Hz[1][0] + SLMCaliStruct->SLMCALIHPFDL[0][1] * SLMCaliHPF10Hz[1][1] + SLMCaliStruct->SLMCALIHPFDL[0][2] * SLMCaliHPF10Hz[1][2];
	};

	for (int cn = 0; cn<SLM_MAX_DATA_FEED_LENGTH; cn++)  // III
	{
		SLMCaliStruct->SLMCALIHPFDL[2][2] = SLMCaliStruct->SLMCALIHPFDL[2][1];
		SLMCaliStruct->SLMCALIHPFDL[2][1] = SLMCaliStruct->SLMCALIHPFDL[2][0];
		SLMCaliStruct->SLMCALIHPFDL[2][0] = (float)(temp[cn]) - SLMCaliStruct->SLMCALIHPFDL[2][1] * SLMCaliHPF10Hz[2][4] - SLMCaliStruct->SLMCALIHPFDL[2][2] * SLMCaliHPF10Hz[2][5];
		temp[cn] = SLMCaliStruct->SLMCALIHPFDL[2][0] * SLMCaliHPF10Hz[2][0] + SLMCaliStruct->SLMCALIHPFDL[2][1] * SLMCaliHPF10Hz[2][1] + SLMCaliStruct->SLMCALIHPFDL[2][2] * SLMCaliHPF10Hz[2][2];
		ch0_input[cn] =(temp[cn] * SLMCaliHPF10HzGain);
	};


	SLMCaliStruct->nInputCount++;
	if(SLMCaliStruct->nInputCount<=SLM_SKIP_COUNT)
		return 0;
	if(SLMCaliStruct->nInputCount>(SLM_CALIBRATION_SECS+ SLM_SKIP_COUNT))
		return 0;

	int nStart = (SLMCaliStruct->nInputCount-(SLM_SKIP_COUNT+1))*SLM_MAX_DATA_FEED_LENGTH;
	
	for(int cn=0; cn<SLM_MAX_DATA_FEED_LENGTH; cn++)
	{ 
		SLMCaliStruct->iInputSignal[nStart+cn]=ch0_input[cn]; 
		if(abs(ch0_input[cn])>sOVER_RANGE_LIMIT) 
			SLMCaliStruct->nCaliInputState  = -1;
	}
	SLMCaliStruct->iCalibrationReturnValue++; 
	if(SLMCaliStruct->nInputCount==SLM_CALIBRATION_SECS+ SLM_SKIP_COUNT)
		SLMCalibrationOver(SLMCaliStruct);
	return 1;
}

int SLMCalibrationOver(struct _SLMCALI *SLMCaliStruct)
{
	float rmsCh;
	//int   calichno = SLMCaliStruct->ChNo;

#if 1
	float fLmsRms = SLMLms_Curve_Fitting(SLMCaliStruct, SLM_CALIBRATION_SECS*SLM_MAX_DATA_FEED_LENGTH);
	rmsCh = SLMChannelCaliRms(SLMCaliStruct,SLM_CALIBRATION_SECS*SLM_MAX_DATA_FEED_LENGTH);
	SLMCaliStruct->fCaliCoef[0] = SLMCaliStruct->fNorminalRms/rmsCh;
	SLMCaliStruct->fCaliCoef[1] = SLMCaliStruct->fNorminalRms/fLmsRms;
	SLMCaliStruct->fOutputCoef = SLMCaliStruct->fNorminalRms/rmsCh;
#else
	float fLmsRms = 7.75f * Lms_Curve_Fitting(SLMCaliStruct, SLM_CALIBRATION_SECS*SLM_MAX_DATA_FEED_LENGTH);
	rmsCh = ChannelCaliRms(SLMCaliStruct, SLM_CALIBRATION_SECS*SLM_MAX_DATA_FEED_LENGTH);
	SLMCaliStruct->fCaliCoef[0] = SLMCaliStruct->fNorminalRms / rmsCh;
	SLMCaliStruct->fCaliCoef[1] = SLMCaliStruct->fNorminalRms / fLmsRms;
	SLMCaliStruct->fOutputCoef = fLmsRms / rmsCh;
#endif 
 	int correct_return_code = SLMCaliStruct->iCalibrationReturnValue;
	SLMCaliStruct->iCalibrationReturnValue = 0;                                        // reinitialization of return code
	
	float   judge=SLMCaliStruct->fOutputCoef;								 // final check
	float   judge1=fabsf((fLmsRms-rmsCh)/fLmsRms);

	if(judge<=0)					
		SLMCaliStruct->iCalibrationReturnValue |= CALIBARION_ERROR_SYSTEM_ERROR;	          // -2    unknown erro	system error		 
	if(judge>5.0f)					
		SLMCaliStruct->iCalibrationReturnValue |= CALIBARION_ERROR_INPUT_TOO_SAMLL;         // -1    the input is too small   
	else 
		if(judge>2.0f)             
			SLMCaliStruct->iCalibrationReturnValue |= CALIBARION_ERROR_CACULATION_ERROR;        // -8    calculation error
	
	if(judge1>=6.28f)			    
		SLMCaliStruct->iCalibrationReturnValue |= CALIBARION_ERROR_REFERENCE_FREQUENCY;     // -32   the ref frequency  maybe wrong  

	if(SLMCaliStruct->nCaliInputState==-1)			
		SLMCaliStruct->iCalibrationReturnValue |= CALIBARION_ERROR_OVERLOAD;                // -4     input overload
	
	if(SLMCaliStruct->iCalibrationReturnValue)  
		SLMCaliStruct->iCalibrationReturnValue = -SLMCaliStruct->iCalibrationReturnValue;
	else									  
		SLMCaliStruct->iCalibrationReturnValue = correct_return_code;

	/*if(SLMCaliStruct->hWnidowToNotify!=NULL)	  
		SendNotifyMessage(SLMCaliStruct->hWnidowToNotify, CALIBRATION_OVER, 1, 0);*/

	SLMCaliStruct->nCaliOver = 1;	//Calibraitn Over
	return 1;
}

float SLMChannelCaliRms(struct _SLMCALI *SLMCaliStruct, const int len)
{
	float fSquareSum, sam, rms;
	fSquareSum = 0.0f;
	for(int cn=0; cn<len; cn++) 
	{ 
		sam = (float) SLMCaliStruct->iInputSignal[cn]*SLMCaliStruct->fScaleCoef;
		fSquareSum += sam*sam;
	}
	rms = sqrtf( fSquareSum/(float)len );
	return rms;
}

float SLMLms_Curve_Fitting(struct _SLMCALI *SLMCaliStruct, const int len)
{
	float t[2][2];
	float tx[2], a[2];
	float lmsRms;
	//float fSampInterval = 1/SLMCaliStruct->Frequency;
	float fSampInterval = 1.0f / (float)SLM_SAMPLERATE;   
	float omg = 2.0f*3.14159265358f*fSampInterval*SLMCaliStruct->Frequency;

	SLMGetPseudoInverseCov(SLMCaliStruct,t, len, omg);
	SLMGetCrossCorrelation(SLMCaliStruct,tx, len,omg);

	a[0] = t[0][0]*tx[0] + t[0][1]*tx[1];
	a[1] = t[1][0]*tx[0] + t[1][1]*tx[1];

	lmsRms = sqrtf( (a[0]*a[0]+a[1]*a[1])/2 );
	return lmsRms;
}


void SLMGetPseudoInverseCov(struct _SLMCALI *SLMCaliStruct,float t[][2],  const int len,  const float omg )
{
	float t00, t01, t10, t11;
	t00 = 0.0f; t01 = 0.0f; t10 = 0.0f; t11=0.0f;

	for(int cn=0; cn<len; cn++)
	{
		t00 += (float)(sin(omg*cn)*sin(omg*cn));
		t01 += (float)(sin(omg*cn)*cos(omg*cn));
		t10 += (float)(cos(omg*cn)*sin(omg*cn));
		t11 += (float)(cos(omg*cn)*cos(omg*cn));
	}

	float det = t00*t11-t01*t10;
	//ASSERT(det);
	t[0][0] = t00/det;
	t[0][1] = -t01/det;
	t[1][0] = -t10/det;
	t[1][1] = t11/det;
}

void SLMGetCrossCorrelation(struct _SLMCALI *SLMCaliStruct, float *tx,  const int len,  const float omg )
{
	float ts1, ts2, sam;	
	ts1 = 0.0f;  ts2 = 0.0f;
	for(int cn=0; cn<len; cn++)
	{
		sam = SLMCaliStruct->iInputSignal[cn]*SLMCaliStruct->fScaleCoef;
		ts1 += (float)(sin(omg*cn)*sam);
		ts2 += (float)(cos(omg*cn)*sam);
	}
   tx[0] = ts1;
   tx[1] = ts2;
}

/*kt
_SLMCALI SLMcalibration_block;
_LPSLMCALI SLMcali_ptr = NULL;

// Get handle to calibration block 
_LPSLMCALI SLMget_calibr_ptr(void)
{
	return &SLMcalibration_block;
}
*/
