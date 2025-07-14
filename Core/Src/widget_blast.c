/*
 * widget_blast.c
 *
 * Created: 2/22/2023 6:15:42 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include <math.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"

void draw_blast_static(void);
void DrawGrid_bls(void);
void DrawInGrid_bls(int row, int col, float val);
void ini_bls_algo(void);

/*
//to parameters.h
extern void blast_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_blast(void);
void draw_blast_update(float val, _BLS_OUT bls_out);
*/

/* called from task_key  */
void blast_win_KEY_handler(uint32_t key_pad)	//to parameters.h as extern
{
	static uint8_t 	vld_pressed = 0;
	
	switch (key_pad)
	{
		case UP: // 
			vld_pressed = 1;	break;
		case F2: // 
			vld_pressed = 2;	break;
		case BOT: //	
			vld_pressed = 3;	break;	
		case F5: // menu
			vld_pressed = 5;	break;
		case FREC: //rec
			vld_pressed = 9;	break;
		break;
		
	}

	if (vld_pressed) {
		struct win_pointer_event win_touch_event;
		win_touch_event.buttons = vld_pressed;
		
		vld_pressed = 0;

		osMessageQueuePut(queue_pointer_eventHandle,&win_touch_event, 0, 0);//xQueueSend(queue_pointer_event, &win_touch_event, 0);
	}
}


/* only for task_lcd  */
static void widget_blast_command_handler(struct win_pointer_event const *event)
{
	if (event->buttons == F5) // menu
	{
		if (get_special_mode_status(DEMO_MODE_BLAST))
		{
			//if call menu-> need to stop recording
			if (flag_arm_rec)
			{
				return; //user must stop recording
			}
			
			//
/*			adc_stop();*/
			set_special_mode_status(DEMO_MODE_BLAST, 0);
			app_widget_launch_menu_main();
			set_special_mode_status(DEMO_MENU_MAIN, 1);
		}
	}
	if (event->buttons == FREC) // rec
	{
		if (get_special_mode_status(DEMO_MODE_BLAST))
		{
			if (!mounted)
			{
				flag_arm_rec=0;				
				///set_rec_button();
			}
			else
				flag_arm_rec=!flag_arm_rec;
		}
	}
}

/**
 * \brief Widget start
 */
void app_widget_launch_blast(void)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_blast_command_handler;

/*	adc_start();*/
	//ini algo for evs
	ini_bls_algo();
	
	draw_blast_static();
}

/**
 * \brief rendering on the LCD.
 */
void draw_blast_static(void)
{
	ILI9488_FillRectangle(0, 32, ILI9488_WIDTH, 258,White);
	
	RgbStr clr = {RGB565(128,0,0),White};
	ILI9488_WriteString(2, 2, "BLS  ", Font_16x26, &clr);

	//ILI9488_FillRectangle(0, 30, ILI9488_WIDTH, 2, 0, 0, 0); //lines
	//ILI9488_FillRectangle(0, 290, ILI9488_WIDTH, 2, 0, 0, 0);
	
	DrawGrid_bls();
}

void draw_blast_update(float val, _BLS_OUT bls_out)	//to parameters.h
{
	//sound
	DrawInGrid_bls(2, 0, bls_out.SoundOutput[0]);	//"LeqIn"
	DrawInGrid_bls(2, 1, bls_out.SoundOutput[1]);	//"LeqAv"
	DrawInGrid_bls(2, 2, bls_out.SoundOutput[2]);	//"Lmax"
	DrawInGrid_bls(2, 3, bls_out.SoundOutput[3]);	//"Lmin"
	
	//vibro
	for (int j=1;j<4;j++)	DrawInGrid_bls(5, j, bls_out.VelocityPeakInst[j-1]);	//"VelPeak(Inst)"
	for (int j=1;j<4;j++)	DrawInGrid_bls(6, j, bls_out.VelocityPeakHold[j-1]);	//"VelPeak(Hold)"  //13.11
	for (int j=1;j<4;j++)	DrawInGrid_bls(7, j, bls_out.fLmaxBLS[j-1]);	//Lmax
	for (int j=1;j<4;j++)	DrawInGrid_bls(8, j, bls_out.LnOutBLS[j-1]);	//L10
	
	////time
	//char text[16];
	//RgbStr clr = {0x00,0x00,0x00,0xff,0xff,0xff};
	//snprintf(text, 16, "%8.2f",val);
	//ILI9488_WriteString(10, 270, text, Font_11x18, &clr);
}

void DrawGrid_bls(void)
{
	//parameters must be the same in function DrawInGrid() !
	int left=2;
	int width=ILI9488_WIDTH-4;
	int top=34;
	int heigh=250;
	int Nrow=10;
	int Ncol=4;
	
	ILI9488_FillRectangle(left+1, top+1*(heigh/Nrow)+1, width-2, heigh/Nrow-2, RGB565(180,200,220));
	ILI9488_FillRectangle(left+1, top+4*(heigh/Nrow)+1, width-2, heigh/Nrow-2, RGB565(180,200,220));
	ILI9488_FillRectangle(left+1, top+5*(heigh/Nrow)+1, (width/Ncol)-2, 4*heigh/Nrow-2, RGB565(180,200,220));
	
	for (int r=0;r<Nrow;r++)
	ILI9488_FillRectangle(left, top+r*(heigh/Nrow), width, 1, Black);
	
	for (int c=0;c<=Ncol;c++)
	ILI9488_FillRectangle(left+c*(width/Ncol), top, 1, (Nrow-1)*(heigh/Nrow), Black);
	
	ILI9488_FillRectangle(left+1, top+0*(heigh/Nrow)+1, width-2, heigh/Nrow-1, White);
	ILI9488_FillRectangle(left+1, top+3*(heigh/Nrow)+1, width-2, heigh/Nrow-1, White);
	
	RgbStr clr = {Black, White};
	ILI9488_WriteString(width/2-40, top+0*(heigh/Nrow)+4, "Sound(dB)",			Font_11x18, &clr);
	ILI9488_WriteString(width/2-80, top+3*(heigh/Nrow)+4, "Vibration(mm/sec)",	Font_11x18, &clr);
	
	//clr.br=180;
	//clr.bg=200;
	//clr.bb=220;
	clr.bcolor =  RGB565(180,200,220);
	int c=left+36;
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"LeqIn",	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"LeqAv",	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"Lmax" ,	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"Lmin" ,	Font_11x18, &clr);
	
	c=left+(width/Ncol)+60;
	ILI9488_WriteString(c, top+4*(heigh/Nrow)+4,"X" ,	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+4*(heigh/Nrow)+4,"Y" ,	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+4*(heigh/Nrow)+4,"Z" ,	Font_11x18, &clr);
	
	c=left+2;
	int r=top+4*(heigh/Nrow)+4;
	ILI9488_WriteString(c, r,"VelPeak" ,	Font_11x18, &clr);	r=r+(heigh/Nrow);	//-
	ILI9488_WriteString(c+20, r,"-Instant" ,	Font_11x18, &clr);	r=r+(heigh/Nrow);	//"VelPeak(Inst)"
	ILI9488_WriteString(c+20, r,"-Hold",	Font_11x18, &clr);	r=r+(heigh/Nrow);		//"VelPeak(Hold)"
	ILI9488_WriteString(c, r,"Lmax" ,	Font_11x18, &clr);	r=r+(heigh/Nrow);	//
	ILI9488_WriteString(c, r,"L10" ,	Font_11x18, &clr);	r=r+(heigh/Nrow);	//-
}

void DrawInGrid_bls(int row, int col, float val)
{
	//parameters must be the same in function DrawGrid() !
	int left=2;
	int width=ILI9488_WIDTH-4;
	int top=34;
	int heigh=250;
	int Nrow=10;
	int Ncol=4;
	
	if (col>=Ncol) col=Ncol-1;
	if (row>=Nrow) row=Nrow-1;
	
	char text[16];
	RgbStr clr = {Black, White};
	snprintf_(text, 16, "%8.2f",val);
	
	int x=left+20+(width/Ncol)*col;
	int y=top+6+(heigh/Nrow)*row;
	ILI9488_WriteString(x, y, text, Font_11x18, &clr);
	
}

void ini_bls_algo()
{
	//----Take following parameters values from Calibration strucute after Calibration has been performed//
	g_algo.SLMCaliStruct.AmpGain			= g_demo_parameters.opt_transd[3].fAmplGain;		//User Option:
	g_algo.SLMCaliStruct.fMICSensitivity	= g_demo_parameters.opt_transd[3].fSenset;			//50.0 mv/pa //User options
	g_algo.SLMCaliStruct.Frequency			= g_demo_parameters.opt_transd[3].fNormFreq;		// User Option: Fixed	(was 1000 )  - ???
	g_algo.SLMCaliStruct.SLMReferenceLevel	= g_demo_parameters.opt_transd[3].fNormRMS;			// 94.0;	114		//User Option: -???
	g_algo.SLMCaliStruct.fOutputCoef		= g_demo_parameters.opt_transd[3].fCalibr;			// get from sound Calibration mode SOUNDCALIBRATOR
	g_algo.SLMCaliStruct.dBRef				= 2e-5f;											//User Options: 20*1-^-6 Pa pressure  - ???
	
	
	//-------------
	g_algo.SLMStruct.SoundQuant		= 0;												//Fixed in algo = 0 (LEQ)			//	0 - LEQ, 1- SPL, 2 -LMAX, 3 - LMIN, 4 - LE,  5-Ln
	g_algo.SLMStruct.WeightChoice	= g_demo_parameters.opt_analyz[1].nFreqWeight;		//User option: 	0 - Z_WT,1 - A_WT, 2 -B_WT, 3 - C_WT      //User options
	g_algo.SLMStruct.TimeWeight		= g_demo_parameters.opt_analyz[1].nTimeWeight;		//in GUI only F  // 	0 - F, 1 - S, 2 - I     //User options , sameer used 1(slow), in EnvDevice only Fast
	g_algo.SLMStruct.IntegTimeStop	= 0;												//fixed in algo to 0; there is no in GUI //0 - NO Stop, 1 - Stop   //User options

	g_algo.SLMStruct.nIntegralTime = 1200;  //User option, Sameer uses 1200 as example, but there is no in GUI
											//Units in second, 
											//for example if user selects recording time as 5 min, plz give input as 300 seconds.
											//If user selects 3 times per hour, i.e 20 min interval, you need to feed as 20*60 = 1200 sec.

	for (int i = 0; i < 3; i++)
	{
		g_algo.BLSStruct.AmpGain[i]		= g_demo_parameters.opt_transd[i].fAmplGain;
		g_algo.BLSStruct.Sensitivity[i] = g_demo_parameters.opt_transd[i].fSenset;
		g_algo.BLSStruct.CaliCoef[i]	= g_demo_parameters.opt_transd[i].fCalibr;
		
		//g_algo.BLSStruct.GerbdBReference[i]	= g_demo_parameters.opt_analyz[1].fDbRefAcc;
		g_algo.BLSStruct.IntegTime[i]		= 1;	// 0.25;    //User option in seconds		//kt: actually there is no this user option for BLS, vibro(similar to SV1)
	}
	g_algo.BLSStruct.VibTriggerLevel			= g_demo_parameters.opt_trig[1].fLevel;		//User input; units: mm/s

	// 2025.06.20  for Sameer:
	// new options (use sound trigger and level of sound trigger) were requested by SV
	// add them to BLSStruct and ini in this place
	// from UI use g_demo_parameters.opt_trig[1].nUseSoundTrigger  and g_demo_parameters.opt_trig[1].fLevelSoundTrigger
	// after algo processing  use same (as for vibro) "blsout->isTrigger" variable for information UI to start recording

	g_algo.BLSStruct.SoundTriggerLevel			= g_demo_parameters.opt_trig[1].fLevelSoundTrigger;//Sameer: 26th June, 2025
	g_algo.BLSStruct.UseSroundTrigger			= g_demo_parameters.opt_trig[1].nUseSoundTrigger;//Sameer: 26th June, 2025

	g_algo.BLSStruct.TriggerFoundResetAlgo	= 0;										//0 or 1: Input from system part 

	intBLSProcess(&(g_algo.BLSStruct), &(g_algo.SLMStruct), &(g_algo.SLMCaliStruct), &(g_algo.BLSOUTStruct));//One time call before EVS algo process
}
