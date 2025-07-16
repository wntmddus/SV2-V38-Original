/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : BLSProcess.c

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
 6thJuly2023	| 0.2  | sameer   | Fs=2205Hz, output structures separated
 9thNov2023     | 0.3  | sameer   | Fs=2756Hz, calibration corrections
 25thNov2023    | 0.4  | sameer   | Trigger flag setting added in BLS mode 
 4thDec2023     | 0.5  | sameer   | L10, Lmax, Integration time added in BLS mode 
 15thMay2024    | 0.6  | sameer   | fs=3kHz 
 20thMar2025    | 0.7  | sameer   | fs=1634Hz for certification  
##############################################################################*/
#include "BLSProcess.h"

//#include <arm_math.h>

_BLS BLSStruct;
_LPBLS BLSStructPtr = &BLSStruct;


_BLS_OUT BLSOUTStruct;
_LPBLS_OUT BLSOUTStructPtr = &BLSOUTStruct;



 //  // it fs = 3kHz
//const  double fltcoef[3][6] =
//{
//	//0.4 Hz HPF
//	{1.0f, -2.0f,  1.0f,  1.0f, -1.999565736900130730191449401900172233582f,  0.999566438586479755201708030654117465019f},
//	{1.0f, -2.0f,  1.0f,  1.0f, -1.998815231354954313758298667380586266518f, 0.998815932777936454911582586646545678377f},
//	{1.0f, -2.0f,  1.0f,  1.0f, -1.998382183251714350191718949645292013884f,  0.998382884522731606224965617002453655005}
//};
//const float BiQCoef = 0.9974f;

  // it fs = 1634Hz
const  double fltcoef[3][6] =
{
	//0.4 Hz HPF
	{1.0f, -2.0f,  1.0f,  1.0f, -1.999201767220130987467996419582050293684f,  0.999204132065095951276134655927307903767f},
	{1.0f, -2.0f,  1.0f,  1.0f, -1.997824782771508322909426169644575566053f,  0.997827145987645769409368767810519784689f},
	{1.0f, -2.0f,  1.0f,  1.0f, -1.99703064379811334738690220547141507268f,   0.997033006074868111134890114044537767768f}
};
const float BiQCoef = 0.9974f;




const  float HP1p5HzCoef[2][6] =  //---> make it fs = 3 kHz
{
	{1.0f,  -2.0f,  1.0f,  1.0f,  -1.9987156322823489f,  0.99871843783693404f},
	{1.0f,  -2.0f,  1.0f,  1.0f,  -1.9969060334468174f,  0.99690883646130679f}
};
const float HP1p5HzCoef_Coef = 0.9946f;

//short intBLSProcess(struct _BLS* Strptr)
short intBLSProcess(struct _BLS  *Strptr, struct _SLM* SLMStruct, struct _SLMCALI* SLMCaliStruct, struct _BLS_OUT* blsout)
{
	int i;

	SLMStruct->SoundTriggerLevel = Strptr->SoundTriggerLevel;
	SLMStruct->UseSroundTrigger  = Strptr->UseSroundTrigger;
	blsout->SoundTriggerFound=0;
	InitSLMProcess(SLMStruct, SLMCaliStruct); //One time call before SLM algo process

	Strptr->CoefVel_1 = 0.1f / ((float)BLS_SAMPLERATE * 2);
	Strptr->CoefVel_2 = 0.9f / ((float)BLS_SAMPLERATE * 2);
	Strptr->CoefDisp_1 = 0.1f / (float)BLS_SAMPLERATE;
	Strptr->CoefDisp_2 = 0.9f / (float)BLS_SAMPLERATE;

	for (i = 0; i < 3; i++)
	{
		Strptr->fCaliFactor[i] = (VLM_CALI_SCALE_CONSTANT / Strptr->Sensitivity[i] / Strptr->AmpGain[i]) * Strptr->CaliCoef[i]*1000.0f;
		Strptr->GerbdBReference[i] = 2.54 * pow(10.0, -8.0);//2.54 x 10-8
		Strptr->Exp[i] = 0.95f;
		Strptr->AccdBReferenceBLS[i] = 1.0e-6f;
		Strptr->refreciprocalBLS[i] = 1.0f / Strptr->AccdBReferenceBLS[i];
		Strptr->IntTimeDataNum[i] = (unsigned int)(Strptr->IntegTime[i] * (float)BLS_SAMPLERATE / (float)BLS_BUFLEN);//Trigger, L10, Lmax and Integration time
	}

	blsout->VibChTrigFound[0] = 0;//Trigger, L10, Lmax and Integration time
	blsout->VibChTrigFound[1] = 0;//Trigger, L10, Lmax and Integration time
	blsout->VibChTrigFound[2] = 0;//Trigger, L10, Lmax and Integration time
	
	blsout->VibrationTriggerFound=0;	//kt 2024.01
	
	ResetBLSAlgo(Strptr, blsout);

	return 1;
}

//short BLSProcess(struct _BLS* Strptr)
short BLSProcess(struct _BLS* Strptr, struct _SLM* SLMStruct, struct _SLMCALI* SLMCaliStruct, struct _BLS_OUT* blsout)
{

	int j, ch, cn, stage, in1;
	float DetectedValue, fv, maxfv;
	float 	peakacc, peakdisp,peakvelinst,peakvelhold,	Npeakvel;
	float 	localmax = 0;

	//in case of VibChTrigFound Or Sound Trigger Found ------------------
	if (Strptr->TriggerFoundResetAlgo)//Trigger, L10, Lmax and Integration time
	{
		ResetBLSAlgo(Strptr, blsout);
		Sound_TriggerFound_ResetAlgo(SLMStruct);
		SLMStruct->SoundTriggerFound = 0;

		blsout->VibChTrigFound[0] = 0;
		blsout->VibChTrigFound[1] = 0;
		blsout->VibChTrigFound[2] = 0;
		
		blsout->VibrationTriggerFound= 0;	//kt 2024.01
		Strptr->TriggerFoundResetAlgo = 0; 	//kt 2024.01 - I think Sameer forgot it
	}
	//End of VibChTrigFound reset ---------------

	//Sound Processing
		SLMProcess(SLMStruct);
		blsout->SoundTriggerFound = SLMStruct->SoundTriggerFound;  //Sameer: 26th June, 2025
		for (cn = 0; cn < 7; cn++)
		{
			blsout->SoundOutput[cn] = SLMStruct->SoundOutput[cn]; //transfer the sound output to struct
		}

	//Vib Processing
		Strptr->TotalCount++;
		for (ch = 0; ch < 3; ch++)
		{
			blsout->m_lFeedCount[ch]++;//Trigger, L10, Lmax and Integration time

			//Apply calibration coeff
			for (cn = 0; cn < BLS_BUFLEN; cn++)
			{
				//Strptr->fCaliInput[ch][cn] = (double)(Strptr->input[ch][cn]) * 1.666666666666667e-4;// 0.000030517578125;//  *Strptr->fCaliFactor[ch]; //Calibration gain applied  //sam test
				Strptr->fCaliInput[ch][cn] = (double)(Strptr->input[ch][cn]) * Strptr->fCaliFactor[ch]; //Calibration gain applied  //sam test
			}

#if 1
			////DC removal filter ------
			if (Strptr->firstframe[ch] < 2)
			{
				Strptr->firstframe[ch]++;
				localmax = 0;
				for (cn = 0; cn < EVS_BUFLEN; cn++)   //I
				{
					if (localmax < (float)(Strptr->fCaliInput[ch][cn]))
					{
						localmax = (float)(Strptr->fCaliInput[ch][cn]);
					}
				}

				for (cn = 0; cn < EVS_BUFLEN; cn++)   //I  fs = 1634Hz
				{
					Strptr->HPFBLS[ch][0][2] = Strptr->HPFBLS[ch][0][1];
					Strptr->HPFBLS[ch][0][1] = Strptr->HPFBLS[ch][0][0];
					Strptr->HPFBLS[ch][0][0] = (float)(Strptr->fCaliInput[ch][cn]) - Strptr->HPFBLS[ch][0][1] * (-1.992418154101028626712377445073798298836f) - Strptr->HPFBLS[ch][0][2] * (0.992432884306420626252531747013563290238f);
					Strptr->fCaliInput[ch][cn] = Strptr->HPFBLS[ch][0][0] + Strptr->HPFBLS[ch][0][1] * (-2.0f) + Strptr->HPFBLS[ch][0][2];
					Strptr->fCaliInput[ch][cn] = Strptr->fCaliInput[ch][cn] * (1 - exp(-(double)cn / (double)(2*EVS_BUFLEN * localmax)));
				}
			}
			else
			{
				for (cn = 0; cn < EVS_BUFLEN; cn++)   //I  fs = 1634Hz
				{
					Strptr->HPFBLS[ch][0][2] = Strptr->HPFBLS[ch][0][1];
					Strptr->HPFBLS[ch][0][1] = Strptr->HPFBLS[ch][0][0];
					Strptr->HPFBLS[ch][0][0] = (float)(Strptr->fCaliInput[ch][cn]) - Strptr->HPFBLS[ch][0][1] * (-1.992418154101028626712377445073798298836f) - Strptr->HPFBLS[ch][0][2] * (0.992432884306420626252531747013563290238f);
					Strptr->fCaliInput[ch][cn] = Strptr->HPFBLS[ch][0][0] + Strptr->HPFBLS[ch][0][1] * (-2.0f) + Strptr->HPFBLS[ch][0][2];
					//Strptr->fCaliInput[ch][cn] = Strptr->fCaliInput[ch][cn];
				}
			}
			//End of DC removal filter -------
#endif

			//Apply BLS filtering
			//--- BL_FLA, 10-100,  fs = 1634Hz
			for (int cn = 0; cn < BLS_BUFLEN; cn++)   //I
			{
				Strptr->fBPF[ch][0][2] = Strptr->fBPF[ch][0][1];
				Strptr->fBPF[ch][0][1] = Strptr->fBPF[ch][0][0];
				Strptr->fBPF[ch][0][0] = (float)(Strptr->fCaliInput[ch][cn]) - Strptr->fBPF[ch][0][1] * (-1.616408792608432065662782406434416770935f) - Strptr->fBPF[ch][0][2] * ( 0.73433096369627248733991109475027769804f);
				Strptr->ACCOut[ch][cn] = Strptr->fBPF[ch][0][0] - Strptr->fBPF[ch][0][2];
			};
			for (int cn = 0; cn < BLS_BUFLEN; cn++)   //II
			{
				Strptr->fBPF[ch][1][2] = Strptr->fBPF[ch][1][1];
				Strptr->fBPF[ch][1][1] = Strptr->fBPF[ch][1][0];
				Strptr->fBPF[ch][1][0] = Strptr->ACCOut[ch][cn] - Strptr->fBPF[ch][1][1] * (-1.965274140178845829751708151889033615589f) - Strptr->fBPF[ch][1][2] * ( 0.966839684973849089466568784700939431787f);
				Strptr->ACCOut[ch][cn] = Strptr->fBPF[ch][1][0] - Strptr->fBPF[ch][1][2];
			};
			for (int cn = 0; cn < BLS_BUFLEN; cn++)    //III
			{
				Strptr->fBPF[ch][2][2] = Strptr->fBPF[ch][2][1];
				Strptr->fBPF[ch][2][1] = Strptr->fBPF[ch][2][0];
				Strptr->fBPF[ch][2][0] = Strptr->ACCOut[ch][cn] - Strptr->fBPF[ch][2][1] * (-1.689740711999942135435048840008676052094f) - Strptr->fBPF[ch][2][2] * ( 0.70243843570490627215008316852618008852f);
				Strptr->ACCOut[ch][cn] = Strptr->fBPF[ch][2][0] - Strptr->fBPF[ch][2][2];
				Strptr->ACCOut[ch][cn] = Strptr->ACCOut[ch][cn] * 0.003f;   //0.019536076658758023
			};
			// BLS filter ends here

			//EVS filter begins -----
			//fc=1Hz fs = 1634Hz
			for (cn = 0; cn < EVS_BUFLEN; cn++)   //I
			{
				Strptr->fHPF2BLS[ch][0][2] = Strptr->fHPF2BLS[ch][0][1];
				Strptr->fHPF2BLS[ch][0][1] = Strptr->fHPF2BLS[ch][0][0];
				Strptr->fHPF2BLS[ch][0][0] = (float)(Strptr->fCaliInput[ch][cn]) - Strptr->fHPF2BLS[ch][0][1] * (-1.998782880736332812077193921140860766172f) - Strptr->fHPF2BLS[ch][0][2] * ( 0.998797657996972843008620657201390713453f);
				Strptr->EVSOutBLS[ch][cn] = Strptr->fHPF2BLS[ch][0][0] + Strptr->fHPF2BLS[ch][0][1] * (-2.0f) + Strptr->fHPF2BLS[ch][0][2];
			};
			for (cn = 0; cn < EVS_BUFLEN; cn++)  // II
			{
				Strptr->fHPF2BLS[ch][1][2] = Strptr->fHPF2BLS[ch][1][1];
				Strptr->fHPF2BLS[ch][1][1] = Strptr->fHPF2BLS[ch][1][0];
				Strptr->fHPF2BLS[ch][1][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fHPF2BLS[ch][1][1] * (-1.996499892698631040133250280632637441158f) - Strptr->fHPF2BLS[ch][1][2] * ( 0.996514653080844858834552724147215485573f);
				Strptr->EVSOutBLS[ch][cn] = Strptr->fHPF2BLS[ch][1][0] + Strptr->fHPF2BLS[ch][1][1] * (-2.0f) + Strptr->fHPF2BLS[ch][1][2];
			};
			for (cn = 0; cn < EVS_BUFLEN; cn++)   //III
			{
				Strptr->fHPF2BLS[ch][2][2] = Strptr->fHPF2BLS[ch][2][1];
				Strptr->fHPF2BLS[ch][2][1] = Strptr->fHPF2BLS[ch][2][0];
				Strptr->fHPF2BLS[ch][2][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fHPF2BLS[ch][2][1] * (-1.994561968134844764932722682715393602848f) - Strptr->fHPF2BLS[ch][2][2] * ( 0.994576714189731481141620861308183521032f);
				Strptr->EVSOutBLS[ch][cn] = Strptr->fHPF2BLS[ch][2][0] + Strptr->fHPF2BLS[ch][2][1] * (-2.0f) + Strptr->fHPF2BLS[ch][2][2];
			};
			for (cn = 0; cn < EVS_BUFLEN; cn++)   //IV
			{
				Strptr->fHPF2BLS[ch][3][2] = Strptr->fHPF2BLS[ch][3][1];
				Strptr->fHPF2BLS[ch][3][1] = Strptr->fHPF2BLS[ch][3][0];
				Strptr->fHPF2BLS[ch][3][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fHPF2BLS[ch][3][1] * (-1.993156341484074944858662092883605509996f) - Strptr->fHPF2BLS[ch][3][2] * ( 0.993171077146981717831408786878455430269f);
				Strptr->EVSOutBLS[ch][cn] = Strptr->fHPF2BLS[ch][3][0] + Strptr->fHPF2BLS[ch][3][1] * (-2.0f) + Strptr->fHPF2BLS[ch][3][2];
			};
			for (cn = 0; cn < EVS_BUFLEN; cn++)    //V
			{
				Strptr->fHPF2BLS[ch][4][2] = Strptr->fHPF2BLS[ch][4][1];
				Strptr->fHPF2BLS[ch][4][1] = Strptr->fHPF2BLS[ch][4][0];
				Strptr->fHPF2BLS[ch][4][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fHPF2BLS[ch][4][1] * (-1.992418154101028626712377445073798298836f) - Strptr->fHPF2BLS[ch][4][2] * ( 0.992432884306420626252531747013563290238f);
				Strptr->EVSOutBLS[ch][cn] = Strptr->fHPF2BLS[ch][4][0] + Strptr->fHPF2BLS[ch][4][1] * (-2.0f) + Strptr->fHPF2BLS[ch][4][2];
				Strptr->EVSOutBLS[ch][cn] = Strptr->EVSOutBLS[ch][cn] * (0.00615f); // (0.0021f);   //0.019536076658758023

			};

			//--------------End of HPF --------------------------------
#if 0
			if (ch != 2)  //horizontal filters
			{
				for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 1634Hz: EVS filter: XY
				{
					Strptr->fDLine2BLS[ch][2] = Strptr->fDLine2BLS[ch][1];
					Strptr->fDLine2BLS[ch][1] = Strptr->fDLine2BLS[ch][0];
					Strptr->fDLine2BLS[ch][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fDLine2BLS[ch][1] * (-1.305298990588446006455569659010507166386f) - Strptr->fDLine2BLS[ch][2] * (0.310630199707513043883011505386093631387f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->fDLine2BLS[ch][0] + Strptr->fDLine2BLS[ch][1] * (0.312301961904086489152376771016861312091f) + Strptr->fDLine2BLS[ch][2] * (0.07495721129246385450173306708165910095f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->EVSOutBLS[ch][cn] * (0.006539141258164309425393589947361761006f);   //0.019536076658758023
				}
			}
			else  //Vertical filter
			{
				for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 1634Hz: EVS filter: Vertical
				{
					Strptr->fDLine2BLS[ch][2] = Strptr->fDLine2BLS[ch][1];
					Strptr->fDLine2BLS[ch][1] = Strptr->fDLine2BLS[ch][0];
					Strptr->fDLine2BLS[ch][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fDLine2BLS[ch][1] * (-1.693516727352923023630637544556520879269f) - Strptr->fDLine2BLS[ch][2] * ( 0.716999475051233625677582494972739368677f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->fDLine2BLS[ch][0] + Strptr->fDLine2BLS[ch][1] * (0.969706090760794947591705295053543522954f) + Strptr->fDLine2BLS[ch][2] * (0.090742150086967260369519294727069791406f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->EVSOutBLS[ch][cn] * (-0.009005615557375818910901266178825608222f);   //0.019536076658758023
				}
			}
#else
			if (ch != 2)  //horizontal filters
			{
				for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 1634Hz: EVS filter: XY
				{
					Strptr->fDLine2BLS[ch][2] = Strptr->fDLine2BLS[ch][1];
					Strptr->fDLine2BLS[ch][1] = Strptr->fDLine2BLS[ch][0];
					Strptr->fDLine2BLS[ch][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fDLine2BLS[ch][1] * (-1.137906224053234716109272994799539446831f) - Strptr->fDLine2BLS[ch][2] * (0.144533023010725097945439188151794951409f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->fDLine2BLS[ch][0] + Strptr->fDLine2BLS[ch][1] * (0.549654277640906574475820889347232878208f) + Strptr->fDLine2BLS[ch][2] * (0.055370757431011559390743315134386648424f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->EVSOutBLS[ch][cn] * (-0.007025308853920264433312592444735855679f);//(0.006539141258164309425393589947361761006f);   //0.019536076658758023
				}
			}
			else  //Vertical filter
			{
				for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 1634Hz: EVS filter: Vertical
				{
					Strptr->fDLine2BLS[ch][2] = Strptr->fDLine2BLS[ch][1];
					Strptr->fDLine2BLS[ch][1] = Strptr->fDLine2BLS[ch][0];
					Strptr->fDLine2BLS[ch][0] = (float)(Strptr->EVSOutBLS[ch][cn]) - Strptr->fDLine2BLS[ch][1] * (-1.970435134614317185253185016335919499397f) - Strptr->fDLine2BLS[ch][2] * ( 0.970861268291070222957728219626005738974f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->fDLine2BLS[ch][0] + Strptr->fDLine2BLS[ch][1] * (-1.021264910669738235071690723998472094536f) + Strptr->fDLine2BLS[ch][2] * (0.027505405627287152908744261026185995433f);
					Strptr->EVSOutBLS[ch][cn] = Strptr->EVSOutBLS[ch][cn] * 0.030522574328333405835333991262814379297f;//(-0.009005615557375818910901266178825608222f);   //0.019536076658758023
				}
			}
			//-----End of EVS weighting-----
#endif
			//-----End of EVS weighting-----
			//EVS filter ends here ----


			//EVS RMS & Lv calculations begins ----		 //Trigger, L10, Lmax and Integration time
			DetectedValue = 0.0f;
			switch (Strptr->LinearCountNumBLS[ch] == 0.0f ? 0 : 1)
			{
				case 0:
					DetectedValue = 0.0f;
					//ACC
					for (cn = 0; cn < EVS_BUFLEN; cn++)
						DetectedValue += (Strptr->EVSOutBLS[ch][cn] * Strptr->EVSOutBLS[ch][cn]);
					DetectedValue /= (float)EVS_BUFLEN;
					Strptr->ACCOlddata[ch] = DetectedValue;
					break;
				default:
					break;
			}

			Strptr->LinearCountNumBLS[ch]++;
			Strptr->ACCAveragedValue[ch] = 0;

			for (cn = 0; cn < EVS_BUFLEN; cn++)
			{
				//ACC
				DetectedValue = Strptr->Exp[ch] * Strptr->ACCOlddata[ch] + (1.0f - Strptr->Exp[ch]) * (Strptr->EVSOutBLS[ch][cn] * Strptr->EVSOutBLS[ch][cn]);
				Strptr->ACCOlddata[ch] = DetectedValue; // Squared RMS
				Strptr->ACCAveragedValue[ch] += DetectedValue;
			}
			Strptr->ACCRMSLOCAL[ch] = sqrtf(Strptr->ACCAveragedValue[ch] / (float)EVS_BUFLEN);

			//Integration time
			if (blsout->m_lFeedCount[ch] == Strptr->IntTimeDataNum[ch])
			{
				Strptr->tempRMS[ch] = Strptr->ACCRMSLOCAL[ch];
				blsout->m_lFeedCount[ch] = 0;
			}

			fv = Strptr->tempRMS[ch] * Strptr->refreciprocalBLS[ch];
			blsout->fLveqBLS[ch] = fv > 11.22f ? (20.0f * (float)log10(fv)) : 21.0f; //20log10(11.22) = 21.0dB;

			//Max and Min of Lv
			maxfv = blsout->fLmaxBLS[ch];

			if (maxfv <= blsout->fLveqBLS[ch])
			    maxfv = blsout->fLveqBLS[ch];

			blsout->fLmaxBLS[ch] = maxfv > 21.0f ? maxfv : 21.0f;

			
			//EVS RMS & Lv calculations ends here ---- //Trigger, L10, Lmax and Integration time


			//Call Ln detector to get L10 single value ----- //Trigger, L10, Lmax and Integration time
			Strptr->LnInputBLS[ch] = blsout->fLveqBLS[ch];
			LnDetectorCalledFromBLS(Strptr, blsout,ch);
			
			//EVS RMS & Lv calculations ends here ----- //Trigger, L10, Lmax and Integration time



			//Channel Integration to get velocity  ...
			for (cn = 0; cn < BLS_BUFLEN; cn++)
			{
				Strptr->VelDLine[ch][0][2] = Strptr->VelDLine[ch][0][1];
				Strptr->VelDLine[ch][0][1] = Strptr->VelDLine[ch][0][0];
				Strptr->VelDLine[ch][0][0] = Strptr->ACCOut[ch][cn] - Strptr->VelDLine[ch][0][1] * fltcoef[0][4] - Strptr->VelDLine[ch][0][2] * fltcoef[0][5];
				Strptr->midbuf[ch][cn] = Strptr->VelDLine[ch][0][0] * fltcoef[0][0] + Strptr->VelDLine[ch][0][1] * fltcoef[0][1] + Strptr->VelDLine[ch][0][2] * fltcoef[0][2];
			}

			for (cn = 0; cn < BLS_BUFLEN; cn++)
			{
				Strptr->VelDLine[ch][1][2] = Strptr->VelDLine[ch][1][1];
				Strptr->VelDLine[ch][1][1] = Strptr->VelDLine[ch][1][0];
				Strptr->VelDLine[ch][1][0] = Strptr->midbuf[ch][cn] - Strptr->VelDLine[ch][1][1] * fltcoef[1][4] - Strptr->VelDLine[ch][1][2] * fltcoef[1][5];
				Strptr->midbuf[ch][cn] = Strptr->VelDLine[ch][1][0] * fltcoef[1][0] + Strptr->VelDLine[ch][1][1] * fltcoef[1][1] + Strptr->VelDLine[ch][1][2] * fltcoef[1][2];
			}

			for (cn = 0; cn < BLS_BUFLEN; cn++)
			{
				Strptr->VelDLine[ch][2][2] = Strptr->VelDLine[ch][2][1];
				Strptr->VelDLine[ch][2][1] = Strptr->VelDLine[ch][2][0];
				Strptr->VelDLine[ch][2][0] = Strptr->midbuf[ch][cn] - Strptr->VelDLine[ch][2][1] * fltcoef[2][4] - Strptr->VelDLine[ch][2][2] * fltcoef[2][5];
				Strptr->VELOut[ch][cn] = Strptr->VelDLine[ch][2][0] * fltcoef[2][0] + Strptr->VelDLine[ch][2][1] * fltcoef[2][1] + Strptr->VelDLine[ch][2][2] * fltcoef[2][2];
			}
			//End of multistage bi-quad ...

			//Integral
			for (in1 = 0; in1 < BLS_BUFLEN; in1++)   //---> fs = 1634Hz
			{
				Strptr->IntgrationDL[ch][0][1] = Strptr->IntgrationDL[ch][0][0];
				Strptr->IntgrationDL[ch][0][0] = Strptr->VELOut[ch][in1] + Strptr->IntgrationDL[ch][0][1];
				Strptr->VELOut[ch][in1] = Strptr->CoefVel_1 * Strptr->IntgrationDL[ch][0][0] + Strptr->CoefVel_2 * Strptr->IntgrationDL[ch][0][1];
				Strptr->VELOut[ch][in1] = (0.00081f) * Strptr->VELOut[ch][in1];  //Sameer:  25th June, 2025
			}
			//End of ACC integral ---> Vel ----------------

			//Channel Integration to get displacement  ---------------------  //fs = 1634Hz
			for (cn = 0; cn < BLS_BUFLEN; cn++)
			{
				Strptr->DispDLine[ch][0][2] = Strptr->DispDLine[ch][0][1];
				Strptr->DispDLine[ch][0][1] = Strptr->DispDLine[ch][0][0];
				Strptr->DispDLine[ch][0][0] = Strptr->VELOut[ch][cn] - Strptr->DispDLine[ch][0][1] * HP1p5HzCoef[0][4] - Strptr->DispDLine[ch][0][2] * HP1p5HzCoef[0][5];
				Strptr->midbuf[ch][cn] = Strptr->DispDLine[ch][0][0] * HP1p5HzCoef[0][0] + Strptr->DispDLine[ch][0][1] * HP1p5HzCoef[0][1] + Strptr->DispDLine[ch][0][2] * HP1p5HzCoef[0][2];
			}
			for (stage = 1; stage < 2; stage++)
			{
				for (cn = 0; cn < BLS_BUFLEN; cn++)
				{
					Strptr->DispDLine[ch][stage][2] = Strptr->DispDLine[ch][stage][1];
					Strptr->DispDLine[ch][stage][1] = Strptr->DispDLine[ch][stage][0];
					Strptr->DispDLine[ch][stage][0] = Strptr->midbuf[ch][cn] - Strptr->DispDLine[ch][stage][1] * HP1p5HzCoef[stage][4] - Strptr->DispDLine[ch][stage][2] * HP1p5HzCoef[stage][5];
					Strptr->midbuf[ch][cn] = Strptr->DispDLine[ch][stage][0] * HP1p5HzCoef[stage][0] + Strptr->DispDLine[ch][stage][1] * HP1p5HzCoef[stage][1] + Strptr->DispDLine[ch][stage][2] * HP1p5HzCoef[stage][2];
				}
			}
			for (cn = 0; cn < BLS_BUFLEN; cn++)
			{
				Strptr->DISPOut[ch][cn] = HP1p5HzCoef_Coef * Strptr->midbuf[ch][cn];
			}
			//End of multistage bi-quad ...

			//Integral
			for (in1 = 0; in1 < BLS_BUFLEN; in1++)   //fs = 1634Hz
			{
				Strptr->IntgrationDL[ch][1][1] = Strptr->IntgrationDL[ch][1][0];
				Strptr->IntgrationDL[ch][1][0] = Strptr->DISPOut[ch][in1] + Strptr->IntgrationDL[ch][1][1];
				Strptr->DISPOut[ch][in1] = Strptr->CoefVel_1 * Strptr->IntgrationDL[ch][1][0] + Strptr->CoefVel_2 * Strptr->IntgrationDL[ch][1][1];
				Strptr->DISPOut[ch][in1] = Strptr->DISPOut[ch][in1] * 0.0058f;
			}
			//End of Vel integral ---> Disp  ----------------

#if 0 //Sameer: 26th June, 2925
			//RMS calculations (ACC & VEL) ----------------------------------------
			
			//Calculate RMS of ACC & VELhere
			DetectedValue = 0.0f;
			if (Strptr->TotalCount ==  1)
			{
				//ACC
				for (cn = 0; cn < BLS_BUFLEN; cn++)
				DetectedValue += (Strptr->ACCOut[ch][cn] * Strptr->ACCOut[ch][cn]);
				DetectedValue /= (float)BLS_BUFLEN;
				Strptr->ACCOlddata[ch] = DetectedValue;
				
				//VEL
				for (cn = 0; cn < BLS_BUFLEN; cn++)
				DetectedValue += (Strptr->VELOut[ch][cn] * Strptr->VELOut[ch][cn]);
				DetectedValue /= (float)BLS_BUFLEN;
				Strptr->VELOlddata[ch] = DetectedValue;
			}
			
			for (cn = 0; cn < BLS_BUFLEN; cn++)
			{
				//ACC
				DetectedValue = Strptr->Exp[ch] * Strptr->ACCOlddata[ch] + (1.0f - Strptr->Exp[ch]) * (Strptr->ACCOut[ch][cn] * Strptr->ACCOut[ch][cn]);
				Strptr->ACCOlddata[ch] = DetectedValue; // Squared RMS
				Strptr->ACCAveragedValue[ch] += DetectedValue;
				//VEL
				DetectedValue = Strptr->Exp[ch] * Strptr->VELOlddata[ch] + (1.0f - Strptr->Exp[ch]) * (Strptr->VELOut[ch][cn] * Strptr->VELOut[ch][cn]);
				Strptr->VELOlddata[ch] = DetectedValue; // Squared RMS
				Strptr->VELAveragedValue[ch] += DetectedValue;
			}
			blsout->ACCRMS[ch] = sqrtf(Strptr->ACCAveragedValue[ch] / (float)BLS_BUFLEN);
			blsout->VELRMS[ch] = (sqrtf(Strptr->VELAveragedValue[ch] / (float)BLS_BUFLEN)) * 1000.0f; // mm/s

			blsout->VELdB[ch] = (float)(20.0 * log10(blsout->VELRMS[ch])) - (float)(20.0 * log10(Strptr->GerbdBReference[ch]));//vel in mm
			if (blsout->VELdB[ch] < 30.0f)
			{
				blsout->VELdB[ch] = 30.0f;
			}
			//RMS Calculations (ACC & VEL) ends here-------------------------------------------
#endif

			//Band pass filtering before parameter calculations
			// 10Hz to 100 Hz, fs = 1634Hz
			for (int cn = 0; cn < BLS_BUFLEN; cn++)   //I
			{
				Strptr->fBPFVEL1[ch][0][2] = Strptr->fBPFVEL1[ch][0][1];
				Strptr->fBPFVEL1[ch][0][1] = Strptr->fBPFVEL1[ch][0][0];
				Strptr->fBPFVEL1[ch][0][0] = (float)(Strptr->VELOut[ch][cn]) - Strptr->fBPFVEL1[ch][0][1] * (-1.616408792608432065662782406434416770935f) - Strptr->fBPFVEL1[ch][0][2] * ( 0.73433096369627248733991109475027769804f);
				Strptr->VELOut[ch][cn] = Strptr->fBPFVEL1[ch][0][0] - Strptr->fBPFVEL1[ch][0][2];
			};
			for (int cn = 0; cn < BLS_BUFLEN; cn++)   //II
			{
				Strptr->fBPFVEL1[ch][1][2] = Strptr->fBPFVEL1[ch][1][1];
				Strptr->fBPFVEL1[ch][1][1] = Strptr->fBPFVEL1[ch][1][0];
				Strptr->fBPFVEL1[ch][1][0] = Strptr->VELOut[ch][cn] - Strptr->fBPFVEL1[ch][1][1] * (-1.965274140178845829751708151889033615589f) - Strptr->fBPFVEL1[ch][1][2] * ( 0.966839684973849089466568784700939431787f);
				Strptr->VELOut[ch][cn] = Strptr->fBPFVEL1[ch][1][0] - Strptr->fBPFVEL1[ch][1][2];
			};
			for (int cn = 0; cn < BLS_BUFLEN; cn++)    //III
			{
				Strptr->fBPFVEL1[ch][2][2] = Strptr->fBPFVEL1[ch][2][1];
				Strptr->fBPFVEL1[ch][2][1] = Strptr->fBPFVEL1[ch][2][0];
				Strptr->fBPFVEL1[ch][2][0] = Strptr->VELOut[ch][cn] - Strptr->fBPFVEL1[ch][2][1] * (-1.689740711999942135435048840008676052094f) - Strptr->fBPFVEL1[ch][2][2] * ( 0.70243843570490627215008316852618008852f);
				Strptr->VELOut[ch][cn] = Strptr->fBPFVEL1[ch][2][0] - Strptr->fBPFVEL1[ch][2][2];
				////Strptr->VELOut[ch][cn] = Strptr->VELOut[ch][cn] * (0.0008f * 1.3f) * 1000.0f; //to give the velociy in mm/s
				//Strptr->VELOut[ch][cn] = Strptr->VELOut[ch][cn] * (0.00081f) * 1000.0f; //to give the velociy in mm/s
				Strptr->VELOut[ch][cn] = Strptr->VELOut[ch][cn] *  1000.0f; //to give the velociy in mm/s


				//Trigger found!!!  //Trigger, L10, Lmax and Integration time
				if ((fabs(Strptr->VELOut[ch][cn]) >= Strptr->VibTriggerLevel))
				{
					blsout->VibChTrigFound[ch] = 1;
				}
				
			};


			//BLS params calculation -----------------------------------------
			peakacc = 0.0f, peakdisp = 0.0f;
			peakvelinst = 0.0;// = (float)(m_mob[ChannelNum].chout.VelocityPeakInst);//velpeak in m/s
			peakvelhold = (float)(blsout->VelocityPeakHold[ch]);//velpeak in m/s
			Npeakvel = 0.0;

			for (j = 0; j < BLS_BUFLEN; j++)  //calculate local maxima
			{
				if (Strptr->ACCOut[ch][j] > peakacc)
				peakacc = Strptr->ACCOut[ch][j];
				if (fabs(Strptr->VELOut[ch][j]) > peakvelinst)  // +ve,-ve in peak calculations...
				peakvelinst = fabs(Strptr->VELOut[ch][j]);
				if (Strptr->VELOut[ch][j] < Npeakvel)
				Npeakvel = Strptr->VELOut[ch][j];
				if (Strptr->DISPOut[ch][j] > peakdisp)
				peakdisp = Strptr->DISPOut[ch][j];

				Strptr->VelocityPVS[ch][j] = Strptr->VELOut[ch][j];//(BLSOut[j] * BLSOut[j]); //1 m/s --> 1000 mm/s: store the square value to get Peak vector sum ...
			}

			if (peakvelhold <= peakvelinst)       //calculate global maxima
			{
				blsout->VelocityPeakHold[ch] = peakvelinst;
				blsout->TimeOfVelPeak[ch] = (float)((float)Strptr->TotalCount/ (float)8.0f);// frame count/number of frames per second
			}

			blsout->AccPeak[ch] = peakacc;  //m/s^2
			blsout->VelocityPeakInst[ch] =peakvelinst; //1 m/s --> 1000 mm/s
			blsout->VelocityP2P[ch] = peakvelinst + fabs(Npeakvel);
			blsout->DispPeak[ch] = peakdisp * 1000000.0f;// in um unit
			//Strptr->zcf[ch] is calcualted by below function
			ZeroCrossingFrequency(Strptr,ch, blsout);
			//--End of BLS params calculations -------------------------------

		}//End of CH for loop
		
		//if (blsout->VibChTrigFound[0] == 1  && blsout->VibChTrigFound[1] == 1  && blsout->VibChTrigFound[2] == 1)	//kt 2024.01
		if (blsout->VibChTrigFound[0] == 1  || blsout->VibChTrigFound[1] == 1  || blsout->VibChTrigFound[2] == 1)
		blsout->VibrationTriggerFound= 1;

		//PVS calculations ---------------------  //Trigger, L10, Lmax and Integration time
		for (int i = 0; i < BLS_BUFLEN; i++)
		{
			Strptr->VectSum[i] = sqrtf(Strptr->VelocityPVS[0][i] * Strptr->VelocityPVS[0][i]
			+ Strptr->VelocityPVS[1][i] * Strptr->VelocityPVS[1][i]
			+ Strptr->VelocityPVS[2][i] * Strptr->VelocityPVS[2][i]);
		}

		float maxPVS = blsout->PeakVectorSum;
		for (int i = 0; i < BLS_BUFLEN; i++)
		{
			if (maxPVS <= Strptr->VectSum[i])
			{
				maxPVS = Strptr->VectSum[i];
				blsout->PeakVectSumTime = (float)((float)Strptr->TotalCount / (float)8.0f);// frame count/number of frames per second
			}
		}
		blsout->PeakVectorSum = maxPVS;
		//End of PVS calculations ---------------

		return 1;

	}

	short ZeroCrossingFrequency(struct _BLS* Strptr, int ch,struct _BLS_OUT* blsout)
	{
		int i;
		long count = Strptr->ZCFreqCount[ch];//input
		
		for (i = 0; i < BLS_BUFLEN-1; i++)
		{
			if((Strptr->VELOut[ch][i]) > 0 && (Strptr->VELOut[ch][i+1] < 0)) // -ve going
			count = count + 1;
			
			if((Strptr->VELOut[ch][i] < 0) && (Strptr->VELOut[ch][i + 1] > 0)) // +ve going
			count = count + 1;
		}

		blsout->zcf[ch] = (count / 2.0) * (BLS_SAMPLERATE/(float)(BLS_BUFLEN* Strptr->TotalCount));
		Strptr->ZCFreqCount[ch] = count;

		//if (blsout->zcf[ch] < 2.0f)
		//blsout->zcf[ch] = 2.0f;
		//if (blsout->zcf[ch] > ((float)BLS_SAMPLERATE/2.0f))
		//blsout->zcf[ch] = (float)BLS_SAMPLERATE / 2.0f;
		
		blsout->zcf[ch] = Strptr->ZCFreqCount[ch];  //test

		return 1;
	}


	short ResetBLSAlgo(struct _BLS* Strptr, struct _BLS_OUT* blsout)
	{
		int i, j, k;

		Strptr->TotalCount = 0;
		blsout->PeakVectorSum = 0;
		blsout->PeakVectSumTime = 0;
		/*Strptr->VibChTrigFound_found = 0;*/

		for (i = 0; i < 3; i++)
		{
			Strptr->LnInputBLS[i] = 0.0f;  // Ln input  //Trigger, L10, Lmax and Integration time
			Strptr->ZCFreqCount[i] = 0;
			blsout->zcf[i] = 0;
			blsout->DispPeak[i] = 0;
			blsout->VelocityP2P[i] = 0;
			blsout->VelocityPeakInst[i] = 0;
			blsout->AccPeak[i] = 0;
			blsout->TimeOfVelPeak[i] = 0;
			blsout->VelocityPeakHold[i] = 0;
			Strptr->VELOlddata[i] = 0;
			Strptr->ACCOlddata[i] = 0;
			Strptr->ACCAveragedValue[i] = 0;
			Strptr->VELAveragedValue[i] = 0;
			blsout->ACCRMS[i] = 0; //Trigger, L10, Lmax and Integration time
			blsout->VELRMS[i] = 0; //Trigger, L10, Lmax and Integration time
			blsout->VELdB[i] = 0; //Trigger, L10, Lmax and Integration time
			Strptr->LinearCountNumBLS[i] = 0; //Trigger, L10, Lmax and Integration time
			blsout->LnOutBLS[i] = 0.0f; //Trigger, L10, Lmax and Integration time
			blsout->m_lFeedCount[i] = 0; //Trigger, L10, Lmax and Integration time
			Strptr->tempRMS[i] = 0.0f;
			blsout->fLmaxBLS[i] =  0.0f;
		}

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				for (k = 0; k < 3; k++)
				{
					Strptr->DispDLine[i][j][k] = 0;
					Strptr->VelDLine[i][j][k] = 0;
					Strptr->IntgrationDL[i][j][k] = 0;
					Strptr->fBPF[i][j][k] = 0;
					Strptr->fBPFVEL1[i][j][k] = 0;
					Strptr->IntgrationDL[i][j][k] = 0;
				}
			}
		}

		for (i = 0; i < 3; i++)
		{
			Strptr->firstframe[i] = 0;
			for (j = 0; j < 3; j++)
			{
				Strptr->fDLine2BLS[i][j] = 0;
			}
		}

		for (i = 0; i < 3; i++)
		{
			Strptr->firstframe[i] = 0;// sam new
			for (j = 0; j < 5; j++)
			{
				for (k = 0; k < 3; k++)
				{
					Strptr->fHPF2BLS[i][j][k] = 0;
					Strptr->HPFBLS[i][j][k] = 0;   //sam new
					Strptr->LPFBLS[i][j][k] = 0;   //sam new
				}
			}
		}
		
		for (i = 0; i < 7; i++)
		{
			blsout->SoundOutput[i] = 0.0f;
		}

		
		//Strptr->fPercLev = 0.0f;
		
		for (i = 0; i < 3; i++) //Trigger, L10, Lmax and Integration time
		{
			for (j = 0; j < 140; j++)
			{
				Strptr->countervalueBLS[i][j] = 0;
				Strptr->levelsBLS[i][j] = (float)(j);
				Strptr->cumulativesumsBLS[i][j] = 0.0f;
			}
		}

		return 1;
	}

	//Trigger, L10, Lmax and Integration time
	//L10 calculations for BLS mode
	short LnDetectorCalledFromBLS(struct _BLS* Strptr, struct _BLS_OUT* blsout, short ch)
	{
		float takevalue = 0.0f;
		float localsum = 0.0f;
		long totalmeasuremnets = 0;

		takevalue = (ROUND(Strptr->LnInputBLS[ch]));

		for (int j = 0; j < 140; j++)
		{
			if (takevalue == Strptr->levelsBLS[ch][j])
			{
				Strptr->countervalueBLS[ch][j] = Strptr->countervalueBLS[ch][j] + 1;
			}
		}

		for (int j = 0; j < 140; j++)
		{
			totalmeasuremnets = totalmeasuremnets + Strptr->countervalueBLS[ch][j];
		}

		for (int k = 0; k < 140; k++)
		{
			localsum = 0;
			for (int j = k; j < 140; j++)
			{
				localsum = localsum + Strptr->countervalueBLS[ch][j];
			}
			Strptr->cumulativesumsBLS[ch][k] = ROUND((localsum / totalmeasuremnets) * 100.0f);
		}

		for (int m = 0; m < 140; m++)
		{
			if (Strptr->cumulativesumsBLS[ch][m] < 10)
			{
				blsout->LnOutBLS[ch] = (float)m - 0.29;  //L10
				break;
			}
		}
		//--------------- End of Percentile Calculations --------------------
		return 1;

	}
