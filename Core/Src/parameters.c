/**
 * \file
 *
 * \brief SAM toolkit demo parameter initialization and storage.
 *
 */

#include <string.h>
#include <stdio.h>
#include "parameters.h"
#include "stm32h7xx.h"
//#include "flash_efc.h"
//#include "task.h"

/** Parameter offset */
//const int __offset_parameters__	= (IFLASH_ADDR + IFLASH_SIZE - IFLASH_PAGE_SIZE*256);
//static int g_page_index = 0;    /* 0 ~ 255 */

/** Parameters used in the demo */
demo_param_t g_demo_parameters1;

demo_param_t g_demo_parameters = {
	.magic = DEMO_PARAMETERS_MAGIC,

	.nMeasMode=0,	//BLS
	.nFS_vib=1634,//.nFS_vib=2756,
	//-------------  for EVS  --------------------
	.opt_analyz[0].nSamplFreqVibro	=1634,//=1024,
	.opt_analyz[0].nBufSizeVibro	=164,
	.opt_analyz[0].nSamplFreqSound	=0,//=16384,
	.opt_analyz[0].nBufSizeSound	=0,
	.opt_analyz[0].fIntegrTime=0.100f,   //22.05   0.125,		//F
	.opt_analyz[0].nFreqWeight=1,			//A_WT			
	.opt_analyz[0].nTimeWeight=0,			//F
	.opt_analyz[0].fVibroIntegrTime=1.0f,   //22.05  0.125,	//F
	.opt_analyz[0].fDbRefAcc=10.0f,
//	.opt_analyz[0].bCalcLmax=0,				//for Blast only: calculate Lmax&L10(for z-axis)
	//-------------  for BLAST  ------------------
	.opt_analyz[1].nSamplFreqVibro	=1634,//=1024,
	.opt_analyz[1].nBufSizeVibro	=164,
	.opt_analyz[1].nSamplFreqSound	=0,//=16384,
	.opt_analyz[1].nBufSizeSound	=0,
	.opt_analyz[1].fIntegrTime=0.100f,    //22.05  0.125f,		//F
	.opt_analyz[1].nFreqWeight=1,			//A_WT
	.opt_analyz[1].nTimeWeight=0,			//F
	.opt_analyz[1].fVibroIntegrTime=1.0f,   //22.05   0.125f,	//F - not used in blast
	.opt_analyz[1].fDbRefAcc=10.0f,
//	.opt_analyz[1].bCalcLmax=1,				//for Blast only: calculate Lmax&L10(for z-axis)
	//------------- TRANSD: VIBRO  ------------------
	.opt_transd[0].fAmplGain = 1.0f,
	.opt_transd[0].fSenset   = 800.0f,
	.opt_transd[0].fNormRMS  = 9.8f,
	.opt_transd[0].fNormFreq = 60.0f,
	.opt_transd[0].fCalibr	= 1.0f,	
	.opt_transd[1].fAmplGain = 1.0f,
	.opt_transd[1].fSenset   = 800.0f,
	.opt_transd[1].fNormRMS  = 9.8f,
	.opt_transd[1].fNormFreq = 60.0f,
	.opt_transd[1].fCalibr	= 1.0f,
	.opt_transd[2].fAmplGain = 1.0f,
	.opt_transd[2].fSenset   = 800.0f,
	.opt_transd[2].fNormRMS  = 9.8f,
	.opt_transd[2].fNormFreq = 60.0f,
	.opt_transd[2].fCalibr	= 1.0f,
	//------------- TRANSD: SOUND  ------------------
	.opt_transd[3].fAmplGain = 1.0f,
	.opt_transd[3].fSenset   = 50.0f,
	.opt_transd[3].fNormRMS  = 94.0f,  //6.937f,   //Sameer: 27th June, 2024
	.opt_transd[3].fNormFreq = 1000.0f,//60.0f,    //Sameer: 27th June, 2024
	.opt_transd[3].fCalibr	= 1.0f,  //0.7f         //Sameer: 27th June, 2024
	//---------------  TRIGGER EVS ------------------
	.opt_trig[0].fLevel = 51.0f,
	.opt_trig[0].fPostTrigTime=5.0f,		//1,2,3,5,10s,1,5m,1h  timer in continuous and single modes
	.opt_trig[0].nDelay = 5,				//0, 1, 5, 10
	.opt_trig[0].nUseSoundTrigger=0,		//0-1
	.opt_trig[0].fLevelSoundTrigger=65.0f,  //60-75 db
	//---------------  TRIGGER BLS ------------------
	.opt_trig[1].fLevel = 0.3f,
	.opt_trig[1].fPostTrigTime=3.0f,		//1,2,3,5,10s,1,5m,1h  timer in continuous and single modes
	.opt_trig[1].nDelay = 5,				//0, 1, 5, 10
	.opt_trig[1].nUseSoundTrigger=0,		//0-1
	.opt_trig[1].fLevelSoundTrigger=65.0f,  //60-75 db
	//-----------------  FTP ------------------------
	.opt_ftp.ip[0]=222,
	.opt_ftp.ip[1]=110,
	.opt_ftp.ip[2]=27,
	.opt_ftp.ip[3]=181,
	.opt_ftp.port=2321,
	.opt_ftp.user_name="SVD",
	.opt_ftp.password="svd",
	.opt_ftp.path="GT1DATABASE",
	//-----------------  RECORDING ------------------------
	.opt_rec.file_type=1,  //0-wav, 1-txt, 2-wav+txt
	.opt_rec.timer=0,
	.opt_rec.nRecordMode=0, 	//0-manual;  1-Single;  2- Cont
	//-----------------  common information ------------------------
	.opt_info.str_sernumber="SN-2025-05",
	.opt_info.str_model="Model-STM",
	.opt_info.str_operator="123 test",
	//----------------------------------------------

	.ser_numb=0,
	.ser_numb_const[0]='S', // �SV12CNT XXXXX�
	.ser_numb_const[1]='V',
	.ser_numb_const[2]='1',
	.ser_numb_const[3]='2',
	.ser_numb_const[4]='C',
	.ser_numb_const[5]='N',
	.ser_numb_const[6]='T',
	
	.vers_sf="3.8",           /*sameer: July 7th, 2025*/
};

opt_algo g_algo;
/*={
	.SLMStruct.SoundQuant = 0,						//Fixed//	0 - LEQ, 1- SPL, 2 -LMAX, 3 - LMIN, 4 - LE,  5-Ln
	.SLMStruct.WeightChoice = 1,						//User option// 	0 - Z_WT,1 - A_WT, 2 -B_WT, 3 - C_WT      //User options
	.SLMStruct.TimeWeight = 1,						//Fixed// 	0 - F, 1 - S, 2 - I     //User options
	.SLMStruct.IntegTimeStop = 0,					//Fixed//0 - NO Stop, 1 - Stop   //User options

	.SLMStruct.nIntegralTime = 1200,   //User option//Units in second, for example if user selects recording time as 5 min, plz give input as 300 seconds.
	//If user selects 3 times per hour, i.e 20 min interval, you need to feed as 20*60 = 1200 sec.
};*/

short flag_lcd_update=1; //-1: no update,  0-evs update, 1-bls update
short flag_arm_rec=0;		//0-arming mode
short g_flagCalibr=0;        //0-no Calibration
short g_flagCalibrChan=0;     //0-number of channel  //kt25


/*
 * \brief Commit demo parameters.
 */
void writetobackup( const void *data)
{
//	for(int i=0; i < sizeof(demo_param_t); i++ )
//	{
//		*((__IO uint8_t*)(0x38800000 + i)) = *data;
//	}
	memcpy((void *)0x38800000, data,sizeof(demo_param_t));
}
void readfrombackup(uint32_t wr_adress, uint8_t *readdata)
{
//*readdata =  *((uint8_t*)(0x38800000 + wr_adress));
	memcpy( &g_demo_parameters, (void const *)0x38800000, sizeof(demo_param_t));
}

