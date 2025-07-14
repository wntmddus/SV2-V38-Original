/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : VLMCali.c

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

#include "VLMCali.h"
double tempcalich[BLS_BUFLEN];

_VLMCALI VLMCaliStruct;
_LPVLMCALI VLMCaliStrucPtr = &VLMCaliStruct;

//kt - really not used          extern long shared_buffer[VLM_CALIBRATION_SECS * VLM_MAX_DATA_FEED_LENGTH];

const  float VLMCaliHPF10Hz[3][6] =
{
	{1.0f, -2.0f,  1.0f,  1.0f, -1.9970218925781167f,  0.99705432364590407f},
	{1.0f, -2.0f,  1.0f,  1.0f, -1.9919403917769984f,  0.99197274032265736f},
	{1.0f, -2.0f,  1.0f,  1.0f, -1.9890183438588338f,  0.98905064495126527f}
};
const double  VLMCaliHPF10HzGain = 0.9891f;

/////////////////////////////////////////////////////////////////////////////////////////////
int VLMCalibrationInit(struct _VLMCALI* VLMCaliStruct, int ch)
{
	VLMCaliStruct->nCaliOver[ch] = 0;//calibration not done
	VLMCaliStruct->nInputCount[ch] = 0;
	VLMCaliStruct->iCalibrationReturnValue[ch] = 0;
	VLMCaliStruct->nCaliInputState[ch] = 0;
	//VLMCaliStruct->fScaleCoef  = VLM_TMSG_SCALE_CONSTANT / VLMCaliStruct->fMICSensitivity / VLMCaliStruct->AmpGain ;
	VLMCaliStruct->fCaliCoef[ch][0] = 1.0f;
	VLMCaliStruct->fCaliCoef[ch][1] = 1.0f;
	VLMCaliStruct->fCaliCoef[ch][2] = 1.0f;
	VLMCaliStruct->fOutputCoef[ch] = 1.0f;
	//VLMCaliStruct->iInputSignal = (long *)shared_buffer;

	VLMCaliStruct->fNorminalRms[ch] = 10.0f;// (float)VLMCaliStruct->dBRef[ch] * (float)pow(10.0f, VLMCaliStruct->VLMReferenceLevel[ch] / 20.0f);  //Sameer 2023.11.09

	//-----------------Gain calculations------------------------------------
	//VLMCaliStruct->fScaleCoef[ch] = (DSPTYPE)(INPUTMAXVOLTAGE * 1000.0f / (VLMCaliStruct->fVibSensitivity[ch] * (DSPTYPE)VLMCaliStruct->AmpGain[ch] * QUANTIZEMAXVALUE));
	VLMCaliStruct->fScaleCoef[ch] = (DSPTYPE)(VLM_CALI_SCALE_CONSTANT / (VLMCaliStruct->fVibSensitivity[ch] * (DSPTYPE)VLMCaliStruct->AmpGain[ch]))*1000.0f;

	//-------------End of Gain Calculations---------------------------------

	
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
			VLMCaliStruct->CALIHPFDL[ch][i][j] = 0.0f;
	}

	return 1;
}

int VLMCalibrationByChannel(struct _VLMCALI* VLMCaliStruct, short* ch0_input, int ch)
{

	//HPF to remove DC offset and low frequeny from cali signal
	// DC removal filter 
	for (int cn = 0; cn < BLS_BUFLEN; cn++)   //I
	{
		VLMCaliStruct->CALIHPFDL[ch][1][2] = VLMCaliStruct->CALIHPFDL[ch][1][1];
		VLMCaliStruct->CALIHPFDL[ch][1][1] = VLMCaliStruct->CALIHPFDL[ch][1][0];
		VLMCaliStruct->CALIHPFDL[ch][1][0] = (float)(ch0_input[cn]) - VLMCaliStruct->CALIHPFDL[ch][1][1] * VLMCaliHPF10Hz[0][4] - VLMCaliStruct->CALIHPFDL[ch][1][2] * VLMCaliHPF10Hz[0][5];
		tempcalich[cn] = VLMCaliStruct->CALIHPFDL[ch][1][0] * VLMCaliHPF10Hz[0][0] + VLMCaliStruct->CALIHPFDL[ch][1][1] * VLMCaliHPF10Hz[0][1] + VLMCaliStruct->CALIHPFDL[ch][1][2] * VLMCaliHPF10Hz[0][2];
	};

	for (int cn = 0; cn < BLS_BUFLEN; cn++)  // II
	{
		VLMCaliStruct->CALIHPFDL[ch][0][2] = VLMCaliStruct->CALIHPFDL[ch][0][1];
		VLMCaliStruct->CALIHPFDL[ch][0][1] = VLMCaliStruct->CALIHPFDL[ch][0][0];
		VLMCaliStruct->CALIHPFDL[ch][0][0] = (float)(tempcalich[cn]) - VLMCaliStruct->CALIHPFDL[ch][0][1] * VLMCaliHPF10Hz[1][4] - VLMCaliStruct->CALIHPFDL[ch][0][2] * VLMCaliHPF10Hz[1][5];
		tempcalich[cn] = VLMCaliStruct->CALIHPFDL[ch][0][0] * VLMCaliHPF10Hz[1][0] + VLMCaliStruct->CALIHPFDL[ch][0][1] * VLMCaliHPF10Hz[1][1] + VLMCaliStruct->CALIHPFDL[ch][0][2] * VLMCaliHPF10Hz[1][2];
	};

	for (int cn = 0; cn < BLS_BUFLEN; cn++)  // III
	{
		VLMCaliStruct->CALIHPFDL[ch][2][2] = VLMCaliStruct->CALIHPFDL[ch][2][1];
		VLMCaliStruct->CALIHPFDL[ch][2][1] = VLMCaliStruct->CALIHPFDL[ch][2][0];
		VLMCaliStruct->CALIHPFDL[ch][2][0] = (float)(tempcalich[cn]) - VLMCaliStruct->CALIHPFDL[ch][2][1] * VLMCaliHPF10Hz[2][4] - VLMCaliStruct->CALIHPFDL[ch][2][2] * VLMCaliHPF10Hz[2][5];
		tempcalich[cn] = VLMCaliStruct->CALIHPFDL[ch][2][0] * VLMCaliHPF10Hz[2][0] + VLMCaliStruct->CALIHPFDL[ch][2][1] * VLMCaliHPF10Hz[2][1] + VLMCaliStruct->CALIHPFDL[ch][2][2] * VLMCaliHPF10Hz[2][2];
		ch0_input[cn] =(tempcalich[cn] * VLMCaliHPF10HzGain);
	};


	VLMCaliStruct->nInputCount[ch]++;
	if (VLMCaliStruct->nInputCount[ch] <= VLM_SKIP_COUNT)
		return 0;
	if (VLMCaliStruct->nInputCount[ch] > (VLM_CALIBRATION_SECS + VLM_SKIP_COUNT))
		return 0;

	int nStart = (VLMCaliStruct->nInputCount[ch] - (VLM_SKIP_COUNT + 1)) * VLM_MAX_DATA_FEED_LENGTH;

	for (int cn = 0; cn < BLS_BUFLEN; cn++)
	{
		VLMCaliStruct->iInputSignal[ch][nStart + cn] = ch0_input[cn];
		if (abs(ch0_input[cn]) > sOVER_RANGE_LIMIT)
			VLMCaliStruct->nCaliInputState[ch] = -1;
	}
	VLMCaliStruct->iCalibrationReturnValue[ch]++;
	if (VLMCaliStruct->nInputCount[ch] == VLM_CALIBRATION_SECS + VLM_SKIP_COUNT)
		VLMCalibrationOver(VLMCaliStruct, ch);
	return 1;
}

int VLMCalibrationOver(struct _VLMCALI* VLMCaliStruct, int ch)
{
	float rmsCh;
	//int   calichno = VLMCaliStruct->ChNo;

#if 1
	float fLmsRms = VLMLms_Curve_Fitting(VLMCaliStruct, VLM_CALIBRATION_SECS * BLS_BUFLEN, ch);
	rmsCh = VLMChannelCaliRms(VLMCaliStruct, VLM_CALIBRATION_SECS * BLS_BUFLEN, ch);
	VLMCaliStruct->fCaliCoef[ch][0] = VLMCaliStruct->fNorminalRms[ch] / rmsCh;
	VLMCaliStruct->fCaliCoef[ch][1] = VLMCaliStruct->fNorminalRms[ch] / fLmsRms;
	VLMCaliStruct->fOutputCoef[ch] = VLMCaliStruct->fNorminalRms[ch] / rmsCh;
#else
	float fLmsRms = 7.75f * Lms_Curve_Fitting(VLMCaliStruct, VLM_CALIBRATION_SECS * VLM_MAX_DATA_FEED_LENGTH);
	rmsCh = ChannelCaliRms(VLMCaliStruct, VLM_CALIBRATION_SECS * VLM_MAX_DATA_FEED_LENGTH);
	VLMCaliStruct->fCaliCoef[0] = VLMCaliStruct->fNorminalRms / rmsCh;
	VLMCaliStruct->fCaliCoef[1] = VLMCaliStruct->fNorminalRms / fLmsRms;
	VLMCaliStruct->fOutputCoef = fLmsRms / rmsCh;
#endif 
	int correct_return_code = VLMCaliStruct->iCalibrationReturnValue[ch];
	VLMCaliStruct->iCalibrationReturnValue[ch] = 0;                                        // reinitialization of return code

	float   judge = VLMCaliStruct->fOutputCoef[ch];								 // final check
	float   judge1 = fabsf((fLmsRms - rmsCh) / fLmsRms);

	if (judge <= 0)
		VLMCaliStruct->iCalibrationReturnValue[ch] |= CALIBARION_ERROR_SYSTEM_ERROR;	          // -2    unknown erro	system error		 
	if (judge > 5.0f)
		VLMCaliStruct->iCalibrationReturnValue[ch] |= CALIBARION_ERROR_INPUT_TOO_SAMLL;         // -1    the input is too small   
	else
		if (judge > 2.0f)
			VLMCaliStruct->iCalibrationReturnValue[ch] |= CALIBARION_ERROR_CACULATION_ERROR;        // -8    calculation error

	if (judge1 >= 6.28f)
		VLMCaliStruct->iCalibrationReturnValue[ch] |= CALIBARION_ERROR_REFERENCE_FREQUENCY;     // -32   the ref frequency  maybe wrong  

	if (VLMCaliStruct->nCaliInputState[ch] == -1)
		VLMCaliStruct->iCalibrationReturnValue[ch] |= CALIBARION_ERROR_OVERLOAD;                // -4     input overload

	if (VLMCaliStruct->iCalibrationReturnValue[ch])
		VLMCaliStruct->iCalibrationReturnValue[ch] = -VLMCaliStruct->iCalibrationReturnValue[ch];
	else
		VLMCaliStruct->iCalibrationReturnValue[ch] = correct_return_code;

	/*if(VLMCaliStruct->hWnidowToNotify!=NULL)
		SendNotifyMessage(VLMCaliStruct->hWnidowToNotify, CALIBRATION_OVER, 1, 0);*/

	VLMCaliStruct->nCaliOver[ch] = 1;	//Calibraitn Over
	return 1;
}

float VLMChannelCaliRms(struct _VLMCALI* VLMCaliStruct, const int len, int ch)
{
	float fSquareSum, sam, rms;
	fSquareSum = 0.0f;
	for (int cn = 0; cn < len; cn++)
	{
		sam = (float)VLMCaliStruct->iInputSignal[ch][cn] * VLMCaliStruct->fScaleCoef[ch];
		fSquareSum += sam * sam;
	}
	rms = sqrtf(fSquareSum / (float)len);
	return rms;
}

float VLMLms_Curve_Fitting(struct _VLMCALI* VLMCaliStruct, const int len, int ch)
{
	float t[2][2];
	float tx[2], a[2];
	float lmsRms;
	//float fSampInterval = 1/VLMCaliStruct->Frequency;
	float fSampInterval = 1.0f / (float)EVS_SAMPLERATE;
	float omg = 2.0f * 3.14159265358f * fSampInterval * VLMCaliStruct->Frequency[ch];

	VLMGetPseudoInverseCov(VLMCaliStruct, t, len, omg,ch);
	VLMGetCrossCorrelation(VLMCaliStruct, tx, len, omg,ch);

	a[0] = t[0][0] * tx[0] + t[0][1] * tx[1];
	a[1] = t[1][0] * tx[0] + t[1][1] * tx[1];

	lmsRms = sqrtf((a[0] * a[0] + a[1] * a[1]) / 2);
	return lmsRms;
}


void VLMGetPseudoInverseCov(struct _VLMCALI* VLMCaliStruct, float t[][2], const int len, const float omg, int ch)
{
	float t00, t01, t10, t11;
	t00 = 0.0f; t01 = 0.0f; t10 = 0.0f; t11 = 0.0f;

	for (int cn = 0; cn < len; cn++)
	{
		t00 += (float)(sin(omg * cn) * sin(omg * cn));
		t01 += (float)(sin(omg * cn) * cos(omg * cn));
		t10 += (float)(cos(omg * cn) * sin(omg * cn));
		t11 += (float)(cos(omg * cn) * cos(omg * cn));
	}

	float det = t00 * t11 - t01 * t10;
	//ASSERT(det);
	t[0][0] = t00 / det;
	t[0][1] = -t01 / det;
	t[1][0] = -t10 / det;
	t[1][1] = t11 / det;
}

void VLMGetCrossCorrelation(struct _VLMCALI* VLMCaliStruct, float* tx, const int len, const float omg, int ch)
{
	float ts1, ts2, sam;
	ts1 = 0.0f;  ts2 = 0.0f;
	for (int cn = 0; cn < len; cn++)
	{
		sam = VLMCaliStruct->iInputSignal[ch][cn] * VLMCaliStruct->fScaleCoef[ch];
		ts1 += (float)(sin(omg * cn) * sam);
		ts2 += (float)(cos(omg * cn) * sam);
	}
	tx[0] = ts1;
	tx[1] = ts2;
}

/*kt
_VLMCALI VLMcalibration_block;
_LPVLMCALI VLMcali_ptr = NULL;

// Get handle to calibration block 
_LPVLMCALI VLMget_calibr_ptr(void)
{
	return &VLMcalibration_block;
}
*/
