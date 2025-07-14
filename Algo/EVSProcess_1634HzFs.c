/*##############################################################################
			 Copyright(C) 2023-24, Svib Software technologies Pvt Ltd, INDIA.
				Svib tech Confidential Proprietary.
						  All rights reserved.
--------------------------------------------------------------------------------
		  Project name :  Lightweight Environmental Analyzer (SV2)
--------------------------------------------------------------------------------
Filename    : EVSProcess.c

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
 6thJuly2023	| 0.2  | sameer   | Fs=2205Hz, output structures separated
 9thNov2023     | 0.3  | sameer   | Fs=2756Hz, calibration corrections
 25thNov2023    | 0.4  | sameer   | Trigger flag setting added in BLS mode
 4thDec2023     | 0.5  | sameer   | L10, Lmax, Integration time added in BLS mode
 20thMar2025    | 0.7  | sameer   | fs=1634Hz for certification  
##############################################################################*/
#include "EVSProcess.h"

//#include <arm_math.h>

_EVS EVSStruct;
_LPEVS EVSStructPtr = &EVSStruct;

_EVS_OUT EVSOUTStruct;
_LPEVS_OUT EVSOUTStructPtr = &EVSOUTStruct;

//short intEVSProcess(struct _EVS* Strptr)
short intEVSProcess(struct _EVS* Strptr, struct _SLM* SLMStruct, struct _SLMCALI* SLMCaliStruct,struct _EVS_OUT* evsout)
{
	int i;

	SLMStruct->SoundTriggerLevel = Strptr->SoundTriggerLevel;
	SLMStruct->UseSroundTrigger  = Strptr->UseSroundTrigger;
	evsout->SoundTriggerFound=0;
	InitSLMProcess(SLMStruct, SLMCaliStruct /*kt , evsout*/); //One time call before SLM algo process


	for (i = 0; i < 3; i++)
	{
		Strptr->fCaliFactor[i] = (VLM_CALI_SCALE_CONSTANT / Strptr->Sensitivity[i] / Strptr->AmpGain[i]) * Strptr->CaliCoef[i]*1000.0f;
		Strptr->AccdBReference[i] = 1.0e-6f;
		Strptr->refreciprocal[i] = 1.0f / Strptr->AccdBReference[i];
		Strptr->Exp[i] = 0.95f;
		//Strptr->IntTimeDataNum[i] = (unsigned int)(Strptr->IntegTime[i] * (float)EVS_SAMPLERATE / (float)EVS_BUFLEN); //Trigger, L10, Lmax and Integration time
		Strptr->IntTimeDataNum[i] = (unsigned int)ceilf((float)Strptr->IntegTime[i]* (float)EVS_SAMPLERATE / (float)EVS_BUFLEN); //Trigger, L10, Lmax and Integration time
	
	}

	evsout->VibChTrigFound[0] = 0;  //Trigger, L10, Lmax and Integration time
	evsout->VibChTrigFound[1] = 0;  //Trigger, L10, Lmax and Integration time
	evsout->VibChTrigFound[2] = 0;	 //Trigger, L10, Lmax and Integration time
	
	evsout->VibrationTriggerFound=0;	//kt 2024.01
	
	ResetEVSAlgo(Strptr, evsout);

	return 1;
}

//short EVSProcess(struct _EVS* Strptr)
short EVSProcess(struct _EVS* Strptr, struct _SLM* SLMStruct, struct _SLMCALI* SLMCaliStruct, struct _EVS_OUT* evsout)
{
	int ch, cn;
	float DetectedValue,fv, maxfv,minfv, Accumulate, dBinfTemp, temp1, temp2, dBinf;
	float 	localmax = 0, hpfgain = 0;


	//in case of VibChTrigFound Or Sound Trigger Found ------------------
	if (Strptr->TriggerFoundResetAlgo)  //Trigger, L10, Lmax and Integration time
	{
		ResetEVSAlgo(Strptr, evsout);
		Sound_TriggerFound_ResetAlgo(SLMStruct);
		SLMStruct->SoundTriggerFound = 0;

		evsout->VibChTrigFound[0] = 0;
		evsout->VibChTrigFound[1] = 0;
		evsout->VibChTrigFound[2] = 0;
		
		evsout->VibrationTriggerFound=0;	//kt 2024.01
		Strptr->TriggerFoundResetAlgo = 0; 	//kt 2024.01 - I think Sameer forgot it
	}

	//Sound Processing
	SLMProcess(SLMStruct);
	evsout->SoundTriggerFound = SLMStruct->SoundTriggerFound;  //Sameer: 26th June, 2025
	for (cn = 0; cn < 7; cn++)
	{
		evsout->SoundOutput[cn] = SLMStruct->SoundOutput[cn];
	}

	//Vib Processing
	for (ch = 0; ch < 3; ch++)
	{

		evsout->m_lFeedCount[ch]++; //Trigger, L10, Lmax and Integration time

		//Apply calibration coeff
		for (cn = 0; cn < BLS_BUFLEN; cn++)
		{
			//Strptr->fCaliInput[ch][cn] = (double)(Strptr->input[ch][cn]) * 1.666666666666667e-4;// * Strptr->fCaliFactor[ch]; //Calibration gain applied  //sam test
			Strptr->fCaliInput[ch][cn] = (double)(Strptr->input[ch][cn]) * Strptr->fCaliFactor[ch]; //Calibration gain applied
		}
#if 0
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

			for (cn = 0; cn < EVS_BUFLEN; cn++)   //I  fs=1634Hz
			{
				Strptr->HPFEVS[ch][0][2] = Strptr->HPFEVS[ch][0][1];
				Strptr->HPFEVS[ch][0][1] = Strptr->HPFEVS[ch][0][0];
				Strptr->HPFEVS[ch][0][0] = (float)(Strptr->fCaliInput[ch][cn]) - Strptr->HPFEVS[ch][0][1] * (-1.994561968134844764932722682715393602848f) - Strptr->HPFEVS[ch][0][2] * ( 0.994576714189731481141620861308183521032f);
				Strptr->fCaliInput[ch][cn] = Strptr->HPFEVS[ch][0][0] + Strptr->HPFEVS[ch][0][1] * (-2.0f) + Strptr->HPFEVS[ch][0][2];
				Strptr->fCaliInput[ch][cn] = Strptr->fCaliInput[ch][cn] * (1 - exp(-(double)cn/ (double)(EVS_BUFLEN * localmax)));
			}
		}
		else
		{
			for (cn = 0; cn < EVS_BUFLEN; cn++)   //I  fs=1634Hz
			{
				Strptr->HPFEVS[ch][0][2] = Strptr->HPFEVS[ch][0][1];
				Strptr->HPFEVS[ch][0][1] = Strptr->HPFEVS[ch][0][0];
				Strptr->HPFEVS[ch][0][0] = (float)(Strptr->fCaliInput[ch][cn]) - Strptr->HPFEVS[ch][0][1] * (-1.994561968134844764932722682715393602848f) - Strptr->HPFEVS[ch][0][2] * ( 0.994576714189731481141620861308183521032f);
				Strptr->fCaliInput[ch][cn] = Strptr->HPFEVS[ch][0][0] + Strptr->HPFEVS[ch][0][1] * (-2.0f) + Strptr->HPFEVS[ch][0][2];
				//Strptr->fCaliInput[ch][cn] = Strptr->fCaliInput[ch][cn];
			}
		}
		//End of DC removal filter -------
#endif
		//EVS
		//fc=1Hz , fs=1634Hz  ------------------------------------------
		for (cn = 0; cn < EVS_BUFLEN; cn++)   //I
		{
			Strptr->fHPF2[ch][0][2] = Strptr->fHPF2[ch][0][1];
			Strptr->fHPF2[ch][0][1] = Strptr->fHPF2[ch][0][0];
			Strptr->fHPF2[ch][0][0] = (float)(Strptr->fCaliInput[ch][cn]) - Strptr->fHPF2[ch][0][1] * (-1.9987828807363328120771939211408607661721f) - Strptr->fHPF2[ch][0][2] * ( 0.998797657996972843008620657201390713453f);
			Strptr->EVSOut[ch][cn] = Strptr->fHPF2[ch][0][0] + Strptr->fHPF2[ch][0][1] * (-2.0f) + Strptr->fHPF2[ch][0][2];
		};
		for (cn = 0; cn < EVS_BUFLEN; cn++)  // II
		{
			Strptr->fHPF2[ch][1][2] = Strptr->fHPF2[ch][1][1];
			Strptr->fHPF2[ch][1][1] = Strptr->fHPF2[ch][1][0];
			Strptr->fHPF2[ch][1][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fHPF2[ch][1][1] * (-1.996499892698631040133250280632637441158f) - Strptr->fHPF2[ch][1][2] * ( 0.996514653080844858834552724147215485573f);
			Strptr->EVSOut[ch][cn] = Strptr->fHPF2[ch][1][0] + Strptr->fHPF2[ch][1][1] * (-2.0f) + Strptr->fHPF2[ch][1][2];
		};
		for (cn = 0; cn < EVS_BUFLEN; cn++)   //III 
		{
			Strptr->fHPF2[ch][2][2] = Strptr->fHPF2[ch][2][1];
			Strptr->fHPF2[ch][2][1] = Strptr->fHPF2[ch][2][0];
			Strptr->fHPF2[ch][2][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fHPF2[ch][2][1] * (-1.994561968134844764932722682715393602848f) - Strptr->fHPF2[ch][2][2] * ( 0.994576714189731481141620861308183521032f);
			Strptr->EVSOut[ch][cn] = Strptr->fHPF2[ch][2][0] + Strptr->fHPF2[ch][2][1] * (-2.0f) + Strptr->fHPF2[ch][2][2];
		};
		for (cn = 0; cn < EVS_BUFLEN; cn++)   //IV
		{
			Strptr->fHPF2[ch][3][2] = Strptr->fHPF2[ch][3][1];
			Strptr->fHPF2[ch][3][1] = Strptr->fHPF2[ch][3][0];
			Strptr->fHPF2[ch][3][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fHPF2[ch][3][1] * (-1.993156341484074944858662092883605509996f) - Strptr->fHPF2[ch][3][2] * ( 0.993171077146981717831408786878455430269f);
			Strptr->EVSOut[ch][cn] = Strptr->fHPF2[ch][3][0] + Strptr->fHPF2[ch][3][1] * (-2.0f) + Strptr->fHPF2[ch][3][2];
		};
		for (cn = 0; cn < EVS_BUFLEN; cn++)    //V
		{
			Strptr->fHPF2[ch][4][2] = Strptr->fHPF2[ch][4][1];
			Strptr->fHPF2[ch][4][1] = Strptr->fHPF2[ch][4][0];
			Strptr->fHPF2[ch][4][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fHPF2[ch][4][1] * (-1.992418154101028626712377445073798298836f) - Strptr->fHPF2[ch][4][2] * ( 0.992432884306420626252531747013563290238f);
			Strptr->EVSOut[ch][cn] = Strptr->fHPF2[ch][4][0] + Strptr->fHPF2[ch][4][1] * (-2.0f) + Strptr->fHPF2[ch][4][2];
			//Strptr->EVSOut[ch][cn] = Strptr->EVSOut[ch][cn] * (0.99229f);   //0.019536076658758023   //Sameer: 25th June, 2025
			Strptr->EVSOut[ch][cn] = Strptr->EVSOut[ch][cn] * (0.00615f);   //0.019536076658758023     //Sameer: 26th June, 2025
		};

		//--------------End of HPF --------------------------------
#if 0  //Sameer: 26th June, 2025
		if (ch != 2)  //horizontal filters
		{
			for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 3 kHz: EVS filter: XY
			{
				Strptr->fDLine2[ch][2] = Strptr->fDLine2[ch][1];
				Strptr->fDLine2[ch][1] = Strptr->fDLine2[ch][0];
				Strptr->fDLine2[ch][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fDLine2[ch][1] * (-1.305298990588446006455569659010507166386f) - Strptr->fDLine2[ch][2] * ( 0.310630199707513043883011505386093631387f);
				Strptr->EVSOut[ch][cn] = Strptr->fDLine2[ch][0] + Strptr->fDLine2[ch][1] * (0.312301961904086489152376771016861312091f) + Strptr->fDLine2[ch][2] * (0.07495721129246385450173306708165910095f);
				Strptr->ACCOut[ch][cn] = Strptr->EVSOut[ch][cn] * (0.002141856806434929826749025139065452095f);   //0.019536076658758023
			}
		}
		else  //Vertical filter
		{
			for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 3 kHz: EVS filter: Vertical Z
			{
				Strptr->fDLine2[ch][2] = Strptr->fDLine2[ch][1];
				Strptr->fDLine2[ch][1] = Strptr->fDLine2[ch][0];
				Strptr->fDLine2[ch][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fDLine2[ch][1] * (-1.693516727352923023630637544556520879269f) - Strptr->fDLine2[ch][2] * ( 0.716999475051233625677582494972739368677f);
				Strptr->EVSOut[ch][cn] = Strptr->fDLine2[ch][0] + Strptr->fDLine2[ch][1] * (0.969706090760794947591705295053543522954f) + Strptr->fDLine2[ch][2] * (0.090742150086967260369519294727069791406f);
				Strptr->ACCOut[ch][cn] = Strptr->EVSOut[ch][cn] * (0.018554888221255005920085068282787688076f);   //0.019536076658758023
			}
		}
#endif
		//Sameer: July 3rd, 2025
		if (ch != 2)  //horizontal filters
		{
			for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 1634Hz: EVS filter: XY
			{
				Strptr->fDLine2[ch][2] = Strptr->fDLine2[ch][1];
				Strptr->fDLine2[ch][1] = Strptr->fDLine2[ch][0];
				Strptr->fDLine2[ch][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fDLine2[ch][1] * (-1.137906224053234716109272994799539446831f) - Strptr->fDLine2[ch][2] * ( 0.144533023010725097945439188151794951409f);
				Strptr->EVSOut[ch][cn] = Strptr->fDLine2[ch][0] + Strptr->fDLine2[ch][1] * (0.549654277640906574475820889347232878208f) + Strptr->fDLine2[ch][2] * (0.055370757431011559390743315134386648424f);
				Strptr->ACCOut[ch][cn] = Strptr->EVSOut[ch][cn] * (-0.007025308853920264433312592444735855679f);//(0.006539141258164309425393589947361761006f);   //0.019536076658758023
			}
		}
		else  //Vertical filter
		{
			for (cn = 0; cn < EVS_BUFLEN; cn++) // fs = 1634Hz: EVS filter: Vertical
			{
				Strptr->fDLine2[ch][2] = Strptr->fDLine2[ch][1];
				Strptr->fDLine2[ch][1] = Strptr->fDLine2[ch][0];
				Strptr->fDLine2[ch][0] = (float)(Strptr->EVSOut[ch][cn]) - Strptr->fDLine2[ch][1] * (-1.970435134614317185253185016335919499397f) - Strptr->fDLine2[ch][2] * (  0.970861268291070222957728219626005738974f);
				Strptr->EVSOut[ch][cn] = Strptr->fDLine2[ch][0] + Strptr->fDLine2[ch][1] * (-1.021264910669738235071690723998472094536f) + Strptr->fDLine2[ch][2] * ( 0.027505405627287152908744261026185995433f);
				Strptr->ACCOut[ch][cn] = Strptr->EVSOut[ch][cn] * 0.030522574328333405835333991262814379297f;//(-0.009005615557375818910901266178825608222f);   //0.019536076658758023
			}
		}

		//-----End of EVS weighting-----

		//Calculate RMS of ACC & VEL here


#if 1  //Sameer: Test
		DetectedValue = 0.0f;
				switch (Strptr->LinearCountNum[ch] == 0.0f ? 0 : 1)
				{
				case 0:
					DetectedValue = 0.0f;
					//ACC
					for (cn = 0; cn < EVS_BUFLEN; cn++)
						DetectedValue += (Strptr->ACCOut[ch][cn] * Strptr->ACCOut[ch][cn]);
					DetectedValue /= (float)EVS_BUFLEN;
					Strptr->ACCOlddata[ch] = DetectedValue;
					break;
				default:
					break;
				}

				Strptr->LinearCountNum[ch]++;
				Strptr->ACCAveragedValue[ch] = 0;


		for (cn = 0; cn < EVS_BUFLEN; cn++)
		{
			//ACC
			DetectedValue = Strptr->Exp[ch] * Strptr->ACCOlddata[ch] + (1.0f - Strptr->Exp[ch]) * (Strptr->ACCOut[ch][cn] * Strptr->ACCOut[ch][cn]);
			Strptr->ACCOlddata[ch] = DetectedValue; // Squared RMS
			Strptr->ACCAveragedValue[ch] += DetectedValue;
		}
#else
		DetectedValue = 0.0f;
				switch (Strptr->LinearCountNum[ch] == 0.0f ? 0 : 1)
				{
				case 0:
					DetectedValue = 0.0f;
					//ACC
					for (cn = 0; cn < EVS_BUFLEN; cn++)
						DetectedValue += (Strptr->fCaliInput[ch][cn] * Strptr->fCaliInput[ch][cn]);
					DetectedValue /= (float)EVS_BUFLEN;
					Strptr->ACCOlddata[ch] = DetectedValue;
					break;
				default:
					break;
				}

				Strptr->LinearCountNum[ch]++;
				Strptr->ACCAveragedValue[ch] = 0;


		for (cn = 0; cn < EVS_BUFLEN; cn++)
		{
			//ACC
			DetectedValue = Strptr->Exp[ch] * Strptr->ACCOlddata[ch] + (1.0f - Strptr->Exp[ch]) * (Strptr->fCaliInput[ch][cn] * Strptr->fCaliInput[ch][cn]);
			Strptr->ACCOlddata[ch] = DetectedValue; // Squared RMS
			Strptr->ACCAveragedValue[ch] += DetectedValue;
		}
#endif
		Strptr->ACCRMSLOCAL[ch] = sqrtf(Strptr->ACCAveragedValue[ch] / (float)EVS_BUFLEN);

		//Integration time  //Trigger, L10, Lmax and Integration time
		if (evsout->m_lFeedCount[ch] == Strptr->IntTimeDataNum[ch])
		{
			evsout->ACCRMS[ch] = Strptr->ACCRMSLOCAL[ch];
			evsout->m_lFeedCount[ch] = 0;
		}
	
		fv = evsout->ACCRMS[ch] * Strptr->refreciprocal[ch];
		evsout->fLveq[ch] = fv > 11.22f ? (20.0f * (float)log10(fv)) : 21.0f; //20log10(11.22) = 21.0dB;

		
		//Trigger found!!!   //Trigger, L10, Lmax and Integration time
		if ((fabs(evsout->fLveq[ch]) >= Strptr->VibTriggerLevel))
		{
			evsout->VibChTrigFound[ch] = 1;
		}
																			  
			
		//Max and Min of Lv
		maxfv = evsout->fLmax[ch];		minfv = evsout->fLmin[ch];

		if (maxfv <= evsout->fLveq[ch])
			maxfv = evsout->fLveq[ch];

		if (minfv >= evsout->fLveq[ch])
			minfv = evsout->fLveq[ch];

		evsout->fLmax[ch] = maxfv > 21.0f ? maxfv : 21.0f;
		evsout->fLmin[ch] = minfv > 21.0f ? minfv : 21.0f;
		//-----------

		// Adding linear averaging to Lv --------------------------------

		Accumulate = Strptr->LvAccumulator[ch];
		dBinfTemp = (double)(evsout->fLveq[ch]);
		temp1 = (double)pow(10.0, ((double)dBinfTemp / 10.0));
		Accumulate = (double)(Accumulate + temp1);
		temp2 = (double)(Accumulate / ((float)Strptr->LinearAveCount[ch] + 1.0));
		dBinf = (double)(10.0 * log10((double)temp2)); //  Avg
		Strptr->LvAccumulator[ch] = Accumulate;
		evsout->fLvAvg[ch] = (float)dBinf;
		Strptr->LinearAveCount[ch]++;
		//---------------------------------------------------------------
		 
		if (ch == 2)  //only vertical channel 
		{
			Strptr->LnInput = evsout->fLveq[2]; // Capture the input for Ln calculations
			EVSLnDetector(Strptr, evsout);
		}

	}
	
	//if (evsout->VibChTrigFound[0] == 1  && evsout->VibChTrigFound[1] == 1  && evsout->VibChTrigFound[2] == 1)	//kt 2024.01
	if (evsout->VibChTrigFound[0] == 1  || evsout->VibChTrigFound[1] == 1  || evsout->VibChTrigFound[2] == 1)	//kt 2024.01
		evsout->VibrationTriggerFound= 1;

	return 1;
}

short ResetEVSAlgo(struct _EVS* Strptr, struct _EVS_OUT* evsout)
{
	int i, j, k;

	for (i = 0; i < 3; i++)
	{
		Strptr->ACCAveragedValue[i] = 0.0f;
		Strptr->VELAveragedValue[i] = 0.0f;
		Strptr->DISPAveragedValue[i] = 0.0f;
		evsout->ACCRMS[i] = 0.0f;
		evsout->VELRMS[i] = 0.0f;
		evsout->DISPRMS[i] = 0.0f;
		Strptr->DISPOlddata[i] = 0.0f;
		evsout->fLveq[i] = 0.0f;
		evsout->fLmax[i] = 0.0f;
		evsout->fLmin[i] = 3000.0f;
		Strptr->LvAccumulator[i] = 0.0f;
		Strptr->LinearAveCount[i] = 0.0f;
		evsout->fLvAvg[i] = 0.0f;
		Strptr->VELOlddata[i] = 0.0f;
		Strptr->ACCOlddata[i] = 0.0f;
		Strptr->LinearCountNum[i] = 0.0f; //Trigger, L10, Lmax and Integration time
		evsout->m_lFeedCount[i] = 0;  //Trigger, L10, Lmax and Integration time
	}

	for (i = 0; i < 3; i++)
	{
		Strptr->firstframe[i] = 0; 
		for (j = 0; j < 3; j++)
		{			
			Strptr->fDLine2[i][j] = 0.0f;
		}
	}

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 5; j++)
		{
			for (k = 0; k < 3; k++)
			{
				Strptr->fHPF2[i][j][k] = 0.0f;
				Strptr->HPFEVS[i][j][k] = 0;  
			}			
		}
	}

	Strptr->LnInput = 0.0f;  // Ln input 
	//Strptr->fPercLev = 0.0f;
	for (i = 0; i < 10; i++)
		evsout->LnOut[i] = 0.0f;

	for (i = 0; i < 140; i++)
	{
		Strptr->countervalue[i] = 0;   
		Strptr->levels[i] = (float)(i);
		Strptr->cumulativesums[i] = 0.0f;  
	}

	for (i = 0; i < 7; i++)  
	{
		evsout->SoundOutput[i] = 0.0f;
	}
	return 1;
}

//Percentile Calculations: 
short EVSLnDetector(struct _EVS* Strptr, struct _EVS_OUT* evsout)
{
	float takevalue = 0.0f;
	float localsum = 0.0f;
	long totalmeasuremnets = 0;

	takevalue = (ROUND(Strptr->LnInput));

	for (int j = 0; j < 140; j++)
	{
		if (takevalue == Strptr->levels[j])
		{
			Strptr->countervalue[j] = Strptr->countervalue[j] + 1;
		}
	}

	for (int j = 0; j < 140; j++)
	{
		totalmeasuremnets = totalmeasuremnets + Strptr->countervalue[j];
	}

	for (int k = 0; k < 140; k++)
	{
		localsum = 0;
		for (int j = k; j < 140; j++)
		{
			localsum = localsum + Strptr->countervalue[j];
		}
		Strptr->cumulativesums[k] = ROUND((localsum / totalmeasuremnets) * 100.0f);
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 1)
		{
			evsout->LnOut[1] = (float)m - 0.13;  //L1
			break;
		}
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 5)
		{
			evsout->LnOut[2] = (float)m - 0.38;  //L5
			break;
		}
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 10)
		{
			evsout->LnOut[3] = (float)m - 0.29;  //L10
			break;
		}
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 30)
		{
			evsout->LnOut[8] = (float)m - 0.42;  //L30
			break;
		}
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 50)
		{
			evsout->LnOut[4] = (float)m - 0.67;  //L50
			break;
		}
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 90)
		{
			evsout->LnOut[5] = (float)m - 0.96;  //L90
			break;
		}
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 95)
		{
			evsout->LnOut[6] = (float)m - 0.81;  //L95
			break;
		}
	}

	for (int m = 0; m < 140; m++)
	{
		if (Strptr->cumulativesums[m] < 99)
		{
			evsout->LnOut[7] = (float)m - 0.72;  //L99
			break;
		}
	}

	//--------------- End of Percentile Calculations --------------------
	return 1;
}


