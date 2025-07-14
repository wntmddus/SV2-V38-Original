/*##############################################################################
             Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.            
                Svib tech Confidential Proprietary.
                          All rights reserved.
--------------------------------------------------------------------------------
          Project name :  Lightweight Environmental Analyzer (SV2)  
--------------------------------------------------------------------------------
Filename    : SLMProcess.c

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
 15thMay2024    | 0.2  | sameer   | Fs=3000Hz, SLM algo corrections 
 25thJune2024   | 0.3  | sameer   | Fs=12000Hz, SLM algo corrections 
 20thMar2025    | 0.7  | sameer   | fs=24000Hz for certification   
##############################################################################*/

#include "SLMProcess.h"

#include <string.h>
//#include <arm_math.h>

#include "SLMCali.h"


_SLM SLMStruct;
_LPSLM SLMStructPtr = &SLMStruct;


void InitSLMProcess(struct _SLM* SLMStruct, struct _SLMCALI* SLMCaliStruct)
{
	int cn, cl;
	SLMStruct->dBReference = SLMCaliStruct->SLMReferenceLevel;
	SLMStruct->LowerLimitSoundSquaredB = (float)(pow(2.0e-5, 2) * pow(10, -30.0 / 20.0 * 2));

	if (SLMCaliStruct->fMICSensitivity <= 0)
	SLMCaliStruct->fMICSensitivity = 51.2f;  //51.2mv/pa
	if (SLMCaliStruct->AmpGain <= 0)
	SLMCaliStruct->AmpGain = 2;
	if (SLMCaliStruct->AmpGain > 64)
	SLMCaliStruct->AmpGain = 64;

	if (SLMStruct->fPercLev1 <= 0)
	SLMStruct->fPercLev1 = 1;
	if (SLMStruct->fPercLev1 >= 99)
	SLMStruct->fPercLev1 = 99;

	SLMStruct->LinearCountNum = 0;

	SLMStruct->SoundTriggerFound = 0;  //Sameer: 26th June, 2025

	if (SLMStruct->nIntegralTime < (float)((float)SLM_BUFLEN / (float)SLM_SAMPLERATE))
	{
		SLMStruct->nIntegralTime = (float)((float)SLM_BUFLEN / (float)SLM_SAMPLERATE);
	}

	switch (SLMStruct->SoundQuant)
	{
		case LEQ:
		break;
		default:
		SLMStruct->SoundQuant = LEQ;
		break;
	}
	switch (SLMStruct->WeightChoice)
	{
		case Z_WT:
		case A_WT:
		case B_WT:
		case C_WT:
		break;
		default:
		SLMStruct->WeightChoice = Z_WT;
		break;
	}
	switch (SLMStruct->TimeWeight)
	{
		case F:
		case Su:
		case I:
		break;
		default:
		SLMStruct->TimeWeight = Su;
		break;
	}
	//----------------

	SLMCaliStruct->fMICSensitivity = 50.0f;  //
	SLMCaliStruct->AmpGain = 1.0f;
	//SLMCaliStruct->fOutputCoef = 1.0f;   //Sameer: 26th June, 2025
	SLMStruct->dBReference = 94.0f;

	//Gain calculations ...
	//SLMStruct->m_Gain = (DSPTYPE)(INPUTMAXVOLTAGE * 1000.0f) / (SLMCaliStruct->fMICSensitivity * SLMCaliStruct->AmpGain * QUANTIZEMAXVALUE);

	SLMStruct->m_Gain = SLM_CALI_SCALE_CONSTANT / SLMCaliStruct->fMICSensitivity / SLMCaliStruct->AmpGain;

	SLMStruct->m_Gain = SLMStruct->m_Gain * SLMCaliStruct->fOutputCoef;// *2.35f/* * 0.28*/; //sam test

	//------------

	SLMStruct->lLAeqAvgCounter = 0;
	SLMStruct->CountIntTimeDataNum = 0;

	SLMStruct->m_TimeInterval = (float)(SLM_BUFLEN) / (float)(SLM_SAMPLERATE);
	SLMStruct->LAeqAvgTime = (float)((long)(SLMStruct->nIntegralTime / (float)SLMStruct->m_TimeInterval + 0.001f));

	SLMStruct->IntTimeDataNum = (float)((SLMStruct->nIntegralTime * (float)SLM_SAMPLERATE / (float)SLM_BUFLEN));

	SLMStruct->m_MinPeak = FLT_MAX;
	SLMStruct->m_MaxPeak = 0.0;
	SLMStruct->fLAeqAccumulate = 0.0;
	SLMStruct->fLAeqAvgValue = 0.0;
	SLMStruct->fPeakTemp = 0.0;
	SLMStruct->Olddata = 0.0;
	SLMStruct->OldPeakData = 0.0;

	//// Leq 5min
	//SLMStruct->fLAeqAccumulate_5min = 0.0f;
	//SLMStruct->fLAeqAvgValue_5min = 0.0f;
	//SLMStruct->Olddata_5min = 0.0f;
	//SLMStruct->Detector_5min = 0.0f;
	//SLMStruct->lLAeqAvgCounter_5min = 0;
	//// Leq 5min -----

	for (cn = 0; cn < 3; cn++)
	{
		SLMStruct->dlslm[cn] = 0.0f;
		for (cl = 0; cl < 3; cl++)
		{
			SLMStruct->Sounddl[cn][cl] = 0.0f;
			SLMStruct->HPF1[cn][cl] = 0.0f;
		}
	}


	switch (SLMStruct->IntegTimeStop)
	{
		case 0:
		SLMStruct->m_bStop = 0;
		break;
		case 1:
		SLMStruct->m_bStop = 1;
		break;
		default:
		SLMStruct->m_bStop = 0;
		break;
	}

	// Set Time Constant
	double TimeConst, DecayTimeConst;

#if 0  //Sam-25
		switch(SLMStruct->->TimeWeight)
		{
		case I : // 35ms, Impulse
			TimeConst = 0.035f; // sec
			break;
		case F : // 125ms, Fast
			TimeConst = 0.125f; // sec
			break;
		default : // 1sec, Slow
			TimeConst = 1.0f; //sec
			break; 
		}

		//exponential decay calculatins....
		if(TimeConst <= 0.035f)
			DecayTimeConst = 1.5f;
		else 
			DecayTimeConst = TimeConst;
#else  //Sam-25
		switch(SLMStruct->TimeWeight)
		{
		case I : // 35ms, Impulse
			TimeConst = 0.1f; // sec
			break;
		case F : // 125ms, Fast
			TimeConst = 0.5f; // sec
			break;
		default : // 1sec, Slow
			TimeConst = 1.0f; //sec
			break; 
		}

		//exponential decay calculatins....
		if(TimeConst <= 0.1f)
			DecayTimeConst = 1.5f;
		else 
			DecayTimeConst = TimeConst;
#endif 

	switch (SLMStruct->SoundQuant)
	{
		case LEQ:

		SLMStruct->Exp = (float)exp(-1.0 / ((double)SLM_SAMPLERATE * (double)SLMStruct->nIntegralTime));
		break;
	}
	SLMStruct->DecayExp = (float)exp(-1.0 / ((double)SLM_SAMPLERATE * (double)DecayTimeConst));

	// Leq 5min
	SLMStruct->Exp_5min = (float)exp(-1.0 / ((double)SLM_SAMPLERATE * (double)300.0f));
	// Leq 5min  ----


	if (SLMStruct->Exp <= 0)
	SLMStruct->Exp = 0.9;
	if (SLMStruct->Exp > 0.9999999)
	SLMStruct->Exp = 0.9;

	SLMStruct->IsInitialized = 1;

	return;
}



// Updates the frame counter, avg counter and call to output write to file...
void SoundUpdateCounters(struct _SLM* SLMStruct)
{
	SLMStruct->CountIntTimeDataNum++;
	SLMStruct->LinearCountNum += SLMStruct->m_TimeInterval;

	SLMStruct->lLAeqAvgCounter++;
	//   This part of code is to reset the statistical parameters at the end of measurement
	if ((float)SLMStruct->lLAeqAvgCounter == SLMStruct->LAeqAvgTime)
	{
		SLMStruct->lLAeqAvgCounter = 0;
		SLMStruct->fLAeqAccumulate = 0.0;
	}

	SLMStruct->DataWriteFlag = 0;
	if ((float)SLMStruct->CountIntTimeDataNum == SLMStruct->IntTimeDataNum)
	{
		//WriteDataToFile();
		SLMStruct->DataWriteFlag = 1;
		SLMStruct->CountIntTimeDataNum = 0;
	}

	return;
}

void SLMProcess(struct _SLM* SLMStruct)
{

	switch (SLMStruct->SoundQuant)
	{
		case 0://Leq
		default:
		SoundWeighting(SLMStruct);
		SoundRMSDetector(SLMStruct);
		//SoundPeakDetector(SLMStruct);
		SoundConvertUnit(SLMStruct);
		SoundMaxMinDetector(SLMStruct);
		SoundAccumulateDetector(SLMStruct);
		SLMStruct->SoundOutput[0] = SLMStruct->Detector;		// Leq inst
		SLMStruct->SoundOutput[1] = SLMStruct->fLAeqAvgValue;	// Leq Avg
		SLMStruct->SoundOutput[2] = SLMStruct->m_MaxPeak;		// Leq MAX
		SLMStruct->SoundOutput[3] = SLMStruct->m_MinPeak;		// Leq MIN
		SLMStruct->SoundOutput[4] = SLMStruct->fPeakTemp;       // Peak Sound
		SLMStruct->SoundOutput[5] = (float)SLMStruct->lLAeqAvgCounter;	//

		break;
	}

	SoundUpdateCounters(SLMStruct);

	//Sound trigger checks:  //Sameer: 26th June, 2025
	if(SLMStruct->UseSroundTrigger == 1)
	{
		if(SLMStruct->Detector >= SLMStruct->SoundTriggerLevel)
		{
			SLMStruct->SoundTriggerFound = 1;
		}
	}

	return;
}


//Reset algo
void Sound_TriggerFound_ResetAlgo(struct _SLM* SLMStruct)
{
	int cn = 0, cl = 0;

	SLMStruct->m_MinPeak = FLT_MAX;
	SLMStruct->m_MaxPeak = 0.0f;
	SLMStruct->fLAeqAccumulate = 0.0f;
	SLMStruct->fLAeqAvgValue = 0.0f;
	SLMStruct->fPeakTemp = 0.0f;
	SLMStruct->Olddata = 0.0f;
	SLMStruct->OldPeakData = 0.0f;
	SLMStruct->Detector = 0.0f;
	SLMStruct->lLAeqAvgCounter = 0; //0.0f;      - changed by Tatsiana because it is "long" type

	for (cn = 0; cn < 7; cn++)  // Leq 5min
	{
		SLMStruct->SoundOutput[cn] = 0.0f;
	}
	for (cn = 0; cn < 3; cn++)
	{
		SLMStruct->dlslm[cn] = 0.0f;
		for (cl = 0; cl < 3; cl++)
		{
			SLMStruct->Sounddl[cn][cl] = 0.0f;
			SLMStruct->HPF1[cn][cl] = 0.0f;
		}
	}
}


/*Take2*/
float Awt[3][6] =
{
	///*Take2*/
	//1.0f,  -1.9999490587258859f,  0.99997839964349877f,    1.0f,  -1.7812991523313435f,   0.78705812258193497f,
	//1.0f,   0.1273537946905681f,  0.0036089664851138006,   1.0f,  -0.36076578625288297f,  0.061575803828994385f,
	//1.0f,   0.0f,                 0.0f,                    1.0f,   0.26222738401140078f,  0.05625970221584526f
	////Scale Values: -0.90801946094093966

	//fs=12kHz
	//{1.0f, -1.103032322769721185196090118552092462778f,  0.107171037464903431968288316511461744085f,  1.0f, -0.650856036610931387720313523459481075406f,  0.149179777998566936991764464437437709421f},
	//{1.0f, -0.306794344639041149935820840255473740399f,  0.023474764446233468973002089796864311211f,  1.0f, -1.63974939468296176947603726148372516036f,   0.658455071502160316221363700606161728501f},
	//{1.0f, -1.081705889344767079762732464587315917015f,  0.083865031545168319815353186186257516965f,  1.0f,  0.087175704445676072129600697735440917313f,  0.064414044681391099955725110248749842867}
	//float Awt_Gain = 0.9453f;

	////fs=24kHz
	//{1.0f,  -1.095045548227285614473203168017789721489f,  0.097737685817481156913544282360817305744f,  1.0f,  -1.797638514536575415903030261688400059938f,  0.802980162206703540483943015715340152383f},
	//{1.0f,  -1.078784475362205341752996901050209999084f,  0.080317490208155734032757777640654239804f,  1.0f,  -0.730801503556270937167482770746573805809f,  0.158699682274058756492607358268287498504f},
	//{1.0f,  -0.459352219838856434819263085955753922462f,  0.048764034419424215349714302192296599969f,  1.0f,  -0.144411037255032681514421710744500160217f,  0.06630953814359377695986097478453302756f}


	//fs=24kHz: July 3rd, 2025
	{	1.0f,  -1.996801218881205208788287563947960734367f,  0.996853803447629638334603896510088816285f,  1.0f,  -1.797584045984516265548336377833038568497f,  0.802234357911581374978027270117308944464f},
	{	1.0f,   0.36295286934121090416383026422408875078f,   0.019666489196987470278177312366096884944f,  1.0f,  -0.211095295519955761243124925385927781463f,  0.030468224397259959651540484060205926653f},
	{   1.0f,  0.523218697807357080442614005733048543334f,  0.0f                                       ,  1.0f,   0.799354845841034444831052496738266199827f,  0.18411695921755349969473058990843128413f}

};

float Awt_Gain = 0.872343748251195205689612066635163500905f;

// Frequency weighting for sound level calculations ...
void SoundWeighting(struct _SLM* SLMStruct)
{
	unsigned short  cn;
	//float midbuf[SLM_MAX_DATA_LEN];

	//SoundHPF(SLMStruct, midbuf); // HPF

	//SOS Matrix :
	//	1 - 2  1  1 -1.994484692522275226522765478875953704119  0.994594057379987073197469271690351888537
	//	1 - 2  1  1 -1.98519065789626125706490711309015750885   0.985299513128214510260249880957417190075
	//	1 - 2  1  1 -1.979864079446988611721280904021114110947  0.979972642603251120618779168580658733845

	//	Scale Values :0.9891

	//HPF: 20Hz, fs=24kHz
	for (cn = 0; cn < SLM_BUFLEN; cn++)
	{
		SLMStruct->HPF1[1][2] = SLMStruct->HPF1[1][1];
		SLMStruct->HPF1[1][1] = SLMStruct->HPF1[1][0];
		//SLMStruct->HPF1[1][0] = (float)(SLMStruct->SLMfeedinS[cn]) - SLMStruct->HPF1[1][1] * (-1.997265955264412085057301737833768129349f) - SLMStruct->HPF1[1][2] * ( 0.997293333667246040263876238896045833826f);
		SLMStruct->HPF1[1][0] = (SLMStruct->SLMfeedin[cn]) - SLMStruct->HPF1[1][1] * (-1.997265955264412085057301737833768129349f) - SLMStruct->HPF1[1][2] * ( 0.997293333667246040263876238896045833826f);

		SLMStruct->SLMfeedin[cn] = SLMStruct->HPF1[1][0] + SLMStruct->HPF1[1][1] * (-2.0f) + SLMStruct->HPF1[1][2];
	};
	for (cn = 0; cn < SLM_BUFLEN; cn++)
	{
		SLMStruct->HPF1[0][2] = SLMStruct->HPF1[0][1];
		SLMStruct->HPF1[0][1] = SLMStruct->HPF1[0][0];
		SLMStruct->HPF1[0][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->HPF1[0][1] * (-1.992595228750302149833828480041120201349f) - SLMStruct->HPF1[0][2] * ( 0.992622543127095080528476955805672332644f);
		SLMStruct->SLMfeedin[cn] = SLMStruct->HPF1[0][0] + SLMStruct->HPF1[0][1] * (-2.0f) + SLMStruct->HPF1[0][2];
	};
	for (cn = 0; cn < SLM_BUFLEN; cn++)
	{
		SLMStruct->HPF1[2][2] = SLMStruct->HPF1[2][1];
		SLMStruct->HPF1[2][1] = SLMStruct->HPF1[2][0];
		SLMStruct->HPF1[2][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->HPF1[2][1] * (-1.98990851731966533755269210814731195569f) - SLMStruct->HPF1[2][2] * (0.989935794867177976108507664321223273873f);
		SLMStruct->SLMfeedin[cn] = SLMStruct->HPF1[2][0] + SLMStruct->HPF1[2][1] * (-2.0f) + SLMStruct->HPF1[2][2];
		SLMStruct->SLMfeedin[cn] = SLMStruct->SLMfeedin[cn] * (1.745f);  //Sameer: 26th June, 2026
	}



	switch (SLMStruct->WeightChoice)
	{
		case A_WT:
		{
			/*A-wt stage 1 */
			for (int cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->Sounddl[0][2] = SLMStruct->Sounddl[0][1];
				SLMStruct->Sounddl[0][1] = SLMStruct->Sounddl[0][0];
				SLMStruct->Sounddl[0][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->Sounddl[0][1] * Awt[0][4] - SLMStruct->Sounddl[0][2] * Awt[0][5];
				SLMStruct->SLMfeedin[cn] = SLMStruct->Sounddl[0][0] * Awt[0][0] + SLMStruct->Sounddl[0][1] * Awt[0][1] + SLMStruct->Sounddl[0][2] * Awt[0][2];
			};

			/*A-wt stage 2 */
			for (int cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->Sounddl[1][2] = SLMStruct->Sounddl[1][1];
				SLMStruct->Sounddl[1][1] = SLMStruct->Sounddl[1][0];
				SLMStruct->Sounddl[1][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->Sounddl[1][1] * Awt[1][4] - SLMStruct->Sounddl[1][2] * Awt[1][5];
				SLMStruct->SLMfeedin[cn] = SLMStruct->Sounddl[1][0] * Awt[1][0] + SLMStruct->Sounddl[1][1] * Awt[1][1] + SLMStruct->Sounddl[1][2] * Awt[1][2];
			};

			/*A-wt stage 3 */
			for (int cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->Sounddl[2][2] = SLMStruct->Sounddl[2][1];
				SLMStruct->Sounddl[2][1] = SLMStruct->Sounddl[2][0];
				SLMStruct->Sounddl[2][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->Sounddl[2][1] * Awt[2][4] - SLMStruct->Sounddl[2][2] * Awt[2][5];
				SLMStruct->SLMfeedin[cn] = SLMStruct->Sounddl[2][0] * Awt[2][0] + SLMStruct->Sounddl[2][1] * Awt[2][1] + SLMStruct->Sounddl[2][2] * Awt[2][2];
				SLMStruct->SLMfeedin[cn] = SLMStruct->SLMfeedin[cn] * Awt_Gain;
			};
		}
		break;
		case B_WT:
		{
			//B-weighting filter: fs=22050Hz
			//SOS Matrix :
			//1 -1.9909773067421335   0.99099785719010136   1 -1.9467674554086667   0.94747777631418884
			//1 -0.20051414296757578  0.010051482392801733  1 -0.44608493149166667  0.11835037126715722
			//	Scale Values : 0.79652530539777489

			for (cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->Sounddl[0][2] = SLMStruct->Sounddl[0][1];
				SLMStruct->Sounddl[0][1] = SLMStruct->Sounddl[0][0];
				SLMStruct->Sounddl[0][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->Sounddl[0][1] * (-1.9467674554086667f) - SLMStruct->Sounddl[0][2] * (0.94747777631418884f);
				SLMStruct->SLMfeedin[cn] = SLMStruct->Sounddl[0][0] + SLMStruct->Sounddl[0][1] * (-1.9909773067421335f) + SLMStruct->Sounddl[0][2] * (0.99099785719010136f);
			};
			for (cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->Sounddl[2][2] = SLMStruct->Sounddl[2][1];
				SLMStruct->Sounddl[2][1] = SLMStruct->Sounddl[2][0];
				SLMStruct->Sounddl[2][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->Sounddl[2][1] * (-0.44608493149166667f) - SLMStruct->Sounddl[2][2] * (0.11835037126715722f);
				SLMStruct->SLMfeedin[cn] = SLMStruct->Sounddl[2][0] + SLMStruct->Sounddl[2][1] * (-0.20051414296757578f) + SLMStruct->Sounddl[2][2] * (0.010051482392801733f);
				SLMStruct->SLMfeedin[cn] = SLMStruct->SLMfeedin[cn] * (0.81368f /*0.79652530539777489f*/);
			}
		}
		break;
		case C_WT:
		{
			//C-weighting filter: fs=22050Hz
			//SOS Matrix :
			//1  -1.012362940125006    0.012363235542931825   1  -1.2447732043747637  0.25297806660880229
			//1   0.97932861503635582  0.0059651132105978102  1   0.5676343593874823  0.097406052424605774
			//Scale Values : 0.66091492318719214

			for (cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->Sounddl[0][2] = SLMStruct->Sounddl[0][1];
				SLMStruct->Sounddl[0][1] = SLMStruct->Sounddl[0][0];
				SLMStruct->Sounddl[0][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->Sounddl[0][1] * (-1.2447732043747637f) - SLMStruct->Sounddl[0][2] * (0.25297806660880229f);
				SLMStruct->SLMfeedin[cn] = SLMStruct->Sounddl[0][0] + SLMStruct->Sounddl[0][1] * (-1.012362940125006f) + SLMStruct->Sounddl[0][2] * (0.012363235542931825f);
			};
			for (cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->Sounddl[2][2] = SLMStruct->Sounddl[2][1];
				SLMStruct->Sounddl[2][1] = SLMStruct->Sounddl[2][0];
				SLMStruct->Sounddl[2][0] = SLMStruct->SLMfeedin[cn] - SLMStruct->Sounddl[2][1] * (0.5676343593874823f) - SLMStruct->Sounddl[2][2] * (0.097406052424605774f);
				SLMStruct->SLMfeedin[cn] = SLMStruct->Sounddl[2][0] + SLMStruct->Sounddl[2][1] * (0.97932861503635582f) + SLMStruct->Sounddl[2][2] * (0.0059651132105978102f);
				SLMStruct->SLMfeedin[cn] = SLMStruct->SLMfeedin[cn] * (0.63716f /* 0.66091492318719214f */);
			}
		}
		break;
		case Z_WT:
		{
			//Z-weighting filter: fs=22050Hz
			//SOS Matrix :
			//1 - 2  1  1 -1.9919403921812773  0.99197274073958397
			//Scale Values : 0.99597828323021531

			for (cn = 0; cn < SLM_BUFLEN; cn++)
			{
				SLMStruct->dlslm[2] = SLMStruct->dlslm[1];
				SLMStruct->dlslm[1] = SLMStruct->dlslm[0];
				//SLMStruct->dlslm[0] = (float)(SLMStruct->SLMfeedinS[cn]) - SLMStruct->dlslm[1] * (-1.9919403921812773f) - SLMStruct->dlslm[2] * (0.99197274073958397f);
				SLMStruct->dlslm[0] = (SLMStruct->SLMfeedin[cn]) - SLMStruct->dlslm[1] * (-1.9919403921812773f) - SLMStruct->dlslm[2] * (0.99197274073958397f);

				SLMStruct->SLMfeedin[cn] = (SLMStruct->dlslm[0] + SLMStruct->dlslm[1] * (-2.0f) + SLMStruct->dlslm[2]);
				SLMStruct->SLMfeedin[cn] = SLMStruct->SLMfeedin[cn] * 0.99597828323021531f;
			}
		}
		break;
		default:
		{
			for (cn = 0; cn < SLM_BUFLEN; cn++)	//fft buffer size
			//SLMStruct->SLMfeedin[cn] = (float)(SLMStruct->SLMfeedinS[cn]);
			SLMStruct->SLMfeedin[cn] = (SLMStruct->SLMfeedin[cn]);
		}
	}
	return;
}

// RMS calculations ...
void SoundRMSDetector(struct _SLM* SLMStruct)
{
	int i;
	float DetectedValue = 0.0f, In = 0.0f, AveragedValue = 0.0f;

#if 0
	if (SLMStruct->LinearCountNum == 0.0)
	{
		DetectedValue = 0.0f;
		for (i = 0; i < SLM_BUFLEN; i++)
		DetectedValue += SQR(SLMStruct->SLMfeedin[i]);
		DetectedValue /= (float)SLM_BUFLEN;
		SLMStruct->Olddata = DetectedValue;
	}

	SLMStruct->Detector = 0.0f;
	for (i = 0; i < SLM_BUFLEN; i++)
	{
		DetectedValue = SLMStruct->Exp * SLMStruct->Olddata + (1.0f - SLMStruct->Exp) * SQR((SLMStruct->SLMfeedin[i]));
		SLMStruct->Olddata = DetectedValue; // Squared RMS
		AveragedValue += DetectedValue;
	}
	SLMStruct->Detector = AveragedValue / (float)SLM_BUFLEN;
#else
	AveragedValue = 0.0f;
	DetectedValue = 0.0f;
	for (i = 0; i < SLM_BUFLEN; i++)
	{
		DetectedValue = (0.9f) * DetectedValue + (0.1f) * SQR((SLMStruct->SLMfeedin[i]));
		AveragedValue += DetectedValue;
	}
	SLMStruct->Detector = AveragedValue / (float)SLM_BUFLEN;


#endif

	return;
}

// Gain and dB reference apply ...
void SoundConvertUnit(struct _SLM* SLMStruct)
{
	float TempOut = 0.0;
	float a;

	float gain = (float)SQR(SLMStruct->m_Gain);

	(SLMStruct->Detector) *= gain;

	a = (float)(10.0 * log10(SLMStruct->Detector));
	TempOut = a + 94.0f;

	if (TempOut  <= 24.0)
	{
		TempOut = 24.0f;
	}

	SLMStruct->Detector = TempOut;  //sameer test
	SLMStruct->SoundOutput[6] = TempOut;//LE


	return;
}

// Max, min calculations ....
void SoundMaxMinDetector(struct _SLM* SLMStruct)
{
	if (SLMStruct->m_MinPeak >= SLMStruct->Detector)
	{
		SLMStruct->m_MinPeak = SLMStruct->Detector;
	}
	if (SLMStruct->m_MaxPeak <= SLMStruct->Detector)
	{
		SLMStruct->m_MaxPeak = SLMStruct->Detector;
	}
	return;
}

// LEQ average calculations ...
void SoundAccumulateDetector(struct _SLM* SLMStruct)
{
	float temp1 = 0.0, temp2 = 0.0;
	float temp1_5min = 0.0, temp2_5min = 0.0;

	temp1 = (float)pow(10.0, ((double)SLMStruct->Detector / 10.0));
	SLMStruct->fLAeqAccumulate = (float)(SLMStruct->fLAeqAccumulate + temp1);
	temp2 = (float)(SLMStruct->fLAeqAccumulate / ((float)SLMStruct->lLAeqAvgCounter + 1.0));
	SLMStruct->fLAeqAvgValue = (float)(10.0 * log10((double)temp2)); // Leq Avg
	return;
}


