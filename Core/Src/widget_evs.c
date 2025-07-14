/*
 * widget_evs.c
 *
 * Created: 2/22/2023 6:15:04 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include <math.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"
#include "arm_math.h"

void draw_evs_static(void);
void DrawGrid_evs(void);
void DrawInGrid_evs(int row, int col, float val);
void ini_evs_algo(void);

extern float32_t mytrg1;
extern float32_t mytrg2;

/*
//to parameters.h
extern void evs_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_evs(void);
void draw_evs_update(float val);
*/

/* called from task_key  */
void evs_win_KEY_handler(uint32_t key_pad)
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
static void widget_evs_command_handler(struct win_pointer_event const *event)
{
//	char hole1[64];
//	RgbStr clr = {0x0,0x0,0x0,0xff,0xff,0xff};	
/*	if (event->buttons == F2) // hold 
	{
		snprintf(hole1, 64, "F2");
		ILI9488_WriteString(10, 50, hole1, Font_16x26, &clr);
	}
	if (event->buttons == F3) // zero
	{		
	snprintf(hole1, 64, "F3");
	ILI9488_WriteString(10, 50, hole1, Font_16x26, &clr);
	}*/
	
	if (event->buttons == F5) // menu
	{		
		if (get_special_mode_status(DEMO_MODE_EVS))
		{
			//if call menu-> need to stop recording
			if (flag_arm_rec)
			{
				return; //user must stop recording
			}
			
			//
			/*adc_stop();*/
			set_special_mode_status(DEMO_MODE_EVS, 0);
			app_widget_launch_menu_main();
			set_special_mode_status(DEMO_MENU_MAIN, 1);
		}
	}
	if (event->buttons == FREC) // rec
	{
		if (get_special_mode_status(DEMO_MODE_EVS))
		{		
			//if (g_demo_parameters.opt_rec.nRecordMode==0) //is not supported for Single & cont modes because Sameer didn't support trigger in EVS
			//	flag_arm_rec=!flag_arm_rec;	
			//else   set_rec_button();	
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
void app_widget_launch_evs(void)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_evs_command_handler;
	
	/*adc_start();*/
	//ini algo for evs
	ini_evs_algo();

	draw_evs_static();
}

/**
 * \brief rendering on the LCD.
 */
void draw_evs_static(void)
{
	ILI9488_FillRectangle(0, 32, ILI9488_WIDTH, 258,White);
	
	RgbStr clr = {RGB565(0,0x82,0),White};
	ILI9488_WriteString(2, 2, "EVS  ", Font_16x26, &clr);

//	ILI9488_FillRectangle(0, 30, ILI9488_WIDTH, 2, 0, 0, 0); //lines
//	ILI9488_FillRectangle(0, 290, ILI9488_WIDTH, 2, 0, 0, 0);
	
	DrawGrid_evs();
}

void draw_evs_update(float val, _EVS_OUT evs_out)	//to parameters.h
{
	
	//sound
	DrawInGrid_evs(2, 0, evs_out.SoundOutput[0]);	//"LeqIn"mytrg1);
	DrawInGrid_evs(2, 1, evs_out.SoundOutput[1]);	//"LeqAv"mytrg2);
	DrawInGrid_evs(2, 2, evs_out.SoundOutput[2]);	//"Lmax" 
	DrawInGrid_evs(2, 3, evs_out.SoundOutput[3]);	//"Lmin" 
	
	//vibro
	for (int j=1;j<4;j++)	DrawInGrid_evs(5, j, evs_out.fLmax[j-1]);	//"Lmax" 
	for (int j=1;j<4;j++)	DrawInGrid_evs(6, j, evs_out.fLveq[j-1]);	//"Lv(In)"
	for (int j=1;j<4;j++)	DrawInGrid_evs(7, j, evs_out.fLvAvg[j-1]);	//"Lv(Av)"
	//for (int j=1;j<4;j++)	DrawInGrid_evs(8, j, 0.0f);		//"L10"   //13.11
							DrawInGrid_evs(8, 3, evs_out.LnOut[3]);		//"L10"   //13.11  -Ln is only for the vertical (z) channel
	
	////time
	//char text[16];
	//RgbStr clr = {0x00,0x00,0x00,0xff,0xff,0xff};
	//snprintf(text, 16, "%8.2f",val);
	//ILI9488_WriteString(10, 270, text, Font_11x18, &clr);
}

void DrawGrid_evs(void)
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
	ILI9488_WriteString(width/2-40, top+0*(heigh/Nrow)+4, "Sound(dB)",	Font_11x18, &clr);
	ILI9488_WriteString(width/2-60, top+3*(heigh/Nrow)+4, "Vibration(dB)",	Font_11x18, &clr);
	

	clr.bcolor = RGB565(180,200,220);
	int c=left+36;
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"LeqIn",	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"LeqAv",	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"Lmax" ,	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+1*(heigh/Nrow)+4,"Lmin" ,	Font_11x18, &clr);	
	
	c=left+(width/Ncol)+60;
	ILI9488_WriteString(c, top+4*(heigh/Nrow)+4,"X" ,	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+4*(heigh/Nrow)+4,"Y" ,	Font_11x18, &clr);	c=c+(width/Ncol);
	ILI9488_WriteString(c, top+4*(heigh/Nrow)+4,"Z" ,	Font_11x18, &clr);	
	
	c=left+10;
	int r=top+5*(heigh/Nrow)+4;
	ILI9488_WriteString(c, r,"Lmax" ,	Font_11x18, &clr);	r=r+(heigh/Nrow);
	ILI9488_WriteString(c, r,"Lv(In)",	Font_11x18, &clr);	r=r+(heigh/Nrow);
	ILI9488_WriteString(c, r,"Lv(Av)",	Font_11x18, &clr);	r=r+(heigh/Nrow);
	ILI9488_WriteString(c, r,"L10(Z)" ,	Font_11x18, &clr);	//13.11
	
}

void DrawInGrid_evs(int row, int col, float val)
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

void ini_evs_algo()
{
	//----Take following parameters values from Calibration strucute after Calibration has been performed//	
	g_algo.SLMCaliStruct.AmpGain			= g_demo_parameters.opt_transd[3].fAmplGain;		//User Option:
	g_algo.SLMCaliStruct.fMICSensitivity	= g_demo_parameters.opt_transd[3].fSenset;			//50.0 mv/pa //User options
	g_algo.SLMCaliStruct.Frequency			= g_demo_parameters.opt_transd[3].fNormFreq;		// User Option: Fixed	(was 1000 )  - ???
	g_algo.SLMCaliStruct.SLMReferenceLevel	= g_demo_parameters.opt_transd[3].fNormRMS;			// 94.0;	114		//User Option: -???
	g_algo.SLMCaliStruct.fOutputCoef		= g_demo_parameters.opt_transd[3].fCalibr;			// get from sound Calibration mode SOUNDCALIBRATOR
	g_algo.SLMCaliStruct.dBRef				= 2e-5f;											//User Options: 20*1-^-6 Pa pressure  - ???
	
	
	//-------------
	g_algo.SLMStruct.SoundQuant		= 0;											//Fixed//	0 - LEQ, 1- SPL, 2 -LMAX, 3 - LMIN, 4 - LE,  5-Ln
	g_algo.SLMStruct.WeightChoice	= g_demo_parameters.opt_analyz[0].nFreqWeight;	//User option// 	0 - Z_WT,1 - A_WT, 2 -B_WT, 3 - C_WT      //User options
	g_algo.SLMStruct.TimeWeight		= g_demo_parameters.opt_analyz[0].nTimeWeight;	// 	0 - F, 1 - S, 2 - I     //User options
	g_algo.SLMStruct.IntegTimeStop = 0;												//fixed in algo to 0; there is no in GUI //   Fixed//0 - NO Stop, 1 - Stop   //User options

	g_algo.SLMStruct.nIntegralTime = 1200;   //User option//Units in second, for example if user selects recording time as 5 min, plz give input as 300 seconds.
	//If user selects 3 times per hour, i.e 20 min interval, you need to feed as 20*60 = 1200 sec.

	for (int i = 0; i < 3; i++)
	{
		g_algo.EVSStruct.AmpGain[i]			= g_demo_parameters.opt_transd[i].fAmplGain;
		g_algo.EVSStruct.Sensitivity[i]		= g_demo_parameters.opt_transd[i].fSenset;
		g_algo.EVSStruct.CaliCoef[i]		= g_demo_parameters.opt_transd[i].fCalibr;
		g_algo.EVSStruct.IntegTime[i]		= g_demo_parameters.opt_analyz[0].fVibroIntegrTime;	//User option in seconds		
		g_algo.EVSStruct.AccdBReference[i]	= g_demo_parameters.opt_analyz[0].fDbRefAcc;
	}
	g_algo.EVSStruct.VibTriggerLevel			= g_demo_parameters.opt_trig[0].fLevel;		//

	// 2025.06.20  for Sameer:
	// new options (use sound trigger and level of sound trigger) were requested by SV
	// add them to BLSStruct and ini in this place
	// from UI use g_demo_parameters.opt_trig[0].nUseSoundTrigger  and g_demo_parameters.opt_trig[0].fLevelSoundTrigger
	// after algo processing  use same (as for vibro) "evsout->isTrigger" variable for information UI to start recording

	g_algo.EVSStruct.SoundTriggerLevel			= g_demo_parameters.opt_trig[0].fLevelSoundTrigger;//Sameer: 26th June, 2025
	g_algo.EVSStruct.UseSroundTrigger			= g_demo_parameters.opt_trig[0].nUseSoundTrigger;//Sameer: 26th June, 2025


	g_algo.EVSStruct.TriggerFoundResetAlgo	= 0;										//0 or 1: Input from system part

	intEVSProcess(&(g_algo.EVSStruct), &(g_algo.SLMStruct), &(g_algo.SLMCaliStruct), &(g_algo.EVSOUTStruct));//One time call before EVS algo process
}
