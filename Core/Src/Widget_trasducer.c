/*
 * Widget_trasducer.c
 *
 * Created: 3/15/2023 3:04:13 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"

/*
//to parameters.h
extern void transd_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_transd(void);
void draw_...._update(float val);
*/


#define TABLE_LINE	5	
#define TABLE_COLUMN	4

typedef struct
{
	int line;
	int col;
} opt_cell;

typedef struct
{
	int x;
	int y;
	int w;
	int h;
} opt_coordinate;

static opt_transducer	opt_transd_temp[4];
static opt_cell cur_cell;
static opt_coordinate opt_coord[TABLE_LINE+1][TABLE_COLUMN];

static void set_opt_sel(opt_cell cell , bool set, RgbStr clr);
static void do_calibration(int ch);

static void transd_exit(void)
{
	if (g_demo_parameters.nMeasMode ==0)
	{
		set_special_mode_status(DEMO_OPT_TRANSD, 0);
		app_widget_launch_evs();
		set_special_mode_status(DEMO_MODE_EVS, 1);
	}
	else if (g_demo_parameters.nMeasMode ==1)
	{
		set_special_mode_status(DEMO_OPT_TRANSD, 0);
		app_widget_launch_blast();
		set_special_mode_status(DEMO_MODE_BLAST, 1);
	}
}



static void set_text_(int line, int col )
{
	int x= opt_coord[line][col].x+Font_11x18.width;
	int y= opt_coord[line][col].y;
	int w= opt_coord[line][col].w-Font_11x18.width;
	int h= opt_coord[line][col].h-1;
	
	ILI9488_FillRectangle(x, y-5, w-1, h,White);
	
	RgbStr clr = {Black,White};
		
	char temp[16];
	switch (line)
	{
		case 0:		snprintf_(temp, 16, "%6.0f",	opt_transd_temp[col].fAmplGain);		break;
		
		case 1:		if (opt_transd_temp[col].fSenset<999.99f)
						snprintf_(temp, 16, "%6.2f", opt_transd_temp[col].fSenset);			
					else if (opt_transd_temp[col].fSenset<9999.99f)
						snprintf_(temp, 16, "%6.1f", opt_transd_temp[col].fSenset);
					else
						snprintf_(temp, 16, "%6.0f", opt_transd_temp[col].fSenset);
					break;
					
		case 2:		if (col==3)	snprintf_(temp, 16, " ");
					else snprintf_(temp, 16, "%6.2f", opt_transd_temp[col].fNormRMS);
					break;
					
		case 3:		if (col==3)	snprintf_(temp, 16, " ");
					else snprintf_(temp, 16, "%6.2f", opt_transd_temp[col].fNormFreq);
					break;
					
		case 4:		snprintf_(temp, 16, "%6.2f", opt_transd_temp[col].fCalibr);			
					break;
					
	}	
	ILI9488_WriteString(x ,y, temp, Font_11x18, &clr);
}

static void set_text(opt_cell cell )
{
	set_text_(cell.line, cell.col);
}

/**
 * \brief rendering on the LCD.
 */
static void draw_static(void)
{
	cur_cell.line=TABLE_LINE+1;  //buttons line, ok
	cur_cell.col =0;
	
	ILI9488_FillRectangle(0, 32, ILI9488_WIDTH, 258,White);
	
	RgbStr clr = {RGB565(0x0,0x82,0x0), White};
	ILI9488_WriteString(140, 34, "Transducer Options:", Font_11x18, &clr);	//title
	
	clr.color = Black;
	//kt25 ILI9488_WriteString(85, 240, "Calibrate", Font_11x18, &clr);			//calibrate


	ILI9488_WriteString(350, 240, "OK   Cancel", Font_11x18, &clr);		//ok, cancel
	ILI9488_FillRectangle(0, 262, 480, 1,Black);
	ILI9488_FillRectangle(0, 263, 480, 27,RGB565(180,200,220));
	
	//-----------------------------------------------
	int top=60;
	int hh=25;
	int Nrow=6;	
	int wid[]={2,85,166, 247,328, 361,442,480};
	int i=0;
	int j=0;
	for (i=0;i<TABLE_LINE;i++)
	{
		for (j=0;j<TABLE_COLUMN-1;j++)
		{
			opt_coord[i][j].x=wid[j+1];
			opt_coord[i][j].y=top + 6 + hh*(i+1);
			opt_coord[i][j].w=wid[2]-wid[1];
			opt_coord[i][j].h=hh;
			set_text_(i,j);
		}		
		opt_coord[i][TABLE_COLUMN-1].x=wid[5];
		opt_coord[i][TABLE_COLUMN-1].y=top + 6 + hh*(i+1);
		opt_coord[i][TABLE_COLUMN-1].w=wid[6]-wid[5];
		opt_coord[i][TABLE_COLUMN-1].h=25;
		set_text_(i,TABLE_COLUMN-1);
	}
	//calibration buttons:
	i=TABLE_LINE;
	for (j=0;j<TABLE_COLUMN-1;j++) //vib
	{
		opt_coord[i][j].x=wid[j+1];
		opt_coord[i][j].y=top + 6 + hh*(i+1);
		opt_coord[i][j].w=wid[2]-wid[1];
		opt_coord[i][j].h=hh;
		ILI9488_WriteString(opt_coord[i][j].x+10, opt_coord[i][j].y, "Calibr", Font_11x18, &clr);
	}
	j=TABLE_COLUMN-1;
	opt_coord[i][j].x=wid[5];  //sound
	opt_coord[i][j].y=top + 6 + hh*(i+1);
	opt_coord[i][j].w=wid[6]-wid[5];
	opt_coord[i][j].h=25;
	ILI9488_WriteString(opt_coord[i][j].x+10, opt_coord[i][j].y, "Calibr", Font_11x18, &clr);
	
	//
	ILI9488_FillRectangle(0, top, 480, hh,RGB565(180,200,220));
	ILI9488_FillRectangle(0, top+hh, wid[1]-1, hh*(Nrow-1),RGB565(180,200,220));
	
	for (int r=0;r<=Nrow;r++)	//=
		ILI9488_FillRectangle(0, top+r*hh, 480, 1, Black);
	
	for (i=1;i<7;i++)			//||
		ILI9488_FillRectangle(wid[i]-1, top, 1, hh*Nrow,Black);

	
	clr.bcolor = RGB565(180,200,220);
	int h=top+6;
	ILI9488_WriteString(wid[1], h, " ch-V1 ", Font_11x18, &clr); i++;
	ILI9488_WriteString(wid[2], h, " ch-V2 ", Font_11x18, &clr); i++;
	ILI9488_WriteString(wid[3], h, " ch-V3 ", Font_11x18, &clr); i++;
	ILI9488_WriteString(wid[5], h, " Sound ", Font_11x18, &clr); i++;
	
	i=0;
	h=h+hh;
	ILI9488_WriteString(wid[0],	h,	"AmGain",  Font_11x18, &clr);h=h+hh;
	ILI9488_WriteString(wid[0],	h,	"Senset.",  Font_11x18, &clr);h=h+hh;
	ILI9488_WriteString(wid[0],	h,	"NormRMS",  Font_11x18, &clr);h=h+hh;
	ILI9488_WriteString(wid[0],	h,	"NormFrq",  Font_11x18, &clr);h=h+hh;
	ILI9488_WriteString(wid[0],	h,	"CalCoef",  Font_11x18, &clr);h=h+hh;
	
	clr.bcolor = White;
	h=top+hh*2+6;
	ILI9488_WriteString(wid[4]+1,	h+6,   "mV/g", Font_7x10, &clr);	ILI9488_WriteString(wid[6]+1,	h+6,   "mV/pa", Font_7x10, &clr);	h=h+hh;
	ILI9488_WriteString(wid[4]+1,	h+6,   "m/ss", Font_7x10, &clr);	h=h+hh;
	ILI9488_WriteString(wid[4]+1,	h+6,   " Hz", Font_7x10, &clr);		h=h+hh;
	
	ILI9488_WriteString(opt_coord[2][3].x,opt_coord[2][3].y , "   -",  Font_11x18, &clr);
	ILI9488_WriteString(opt_coord[3][3].x,opt_coord[3][3].y , "   -",  Font_11x18, &clr);
	
/*	//kt25
	clr.color = Black;
	h=top+6+hh*6+2;
	ILI9488_WriteString(wid[1]+11, h, "Calibr", Font_11x18, &clr);			//calibrate
	ILI9488_WriteString(wid[2]+11, h, "Calibr", Font_11x18, &clr);			//calibrate
	ILI9488_WriteString(wid[3]+11, h, "Calibr", Font_11x18, &clr);			//calibrate
	ILI9488_WriteString(wid[5]+11, h, "Calibr", Font_11x18, &clr);			//calibrate
*/
	set_opt_sel(cur_cell,1,clr);
	
	//notes
	clr.bcolor = RGB565(180,200,220);
	ILI9488_WriteString(10, 266, "<^> Navigation   ok-Select   ", Font_11x18, &clr);
}

//=======================================================
/* called from task_key  */
void transd_win_KEY_handler(uint32_t key_pad)
{
	static uint8_t 	vld_pressed = 0;
	
	switch (key_pad)
	{
		case UP: // 
			vld_pressed = 1;
			break;
		case F2: // 
			vld_pressed = 2;
		
			break;
		case BOT: //	
			vld_pressed = 3;
			
			break;	
		case F5: // menu
			vld_pressed = 5;
			
			break;
		case F6: // <-
			vld_pressed = 6;
			
			break;
		case F7: // ->
			vld_pressed = 7;
			
			break;
		break;
	}

	if (vld_pressed) {
		struct win_pointer_event win_touch_event;
		win_touch_event.buttons = vld_pressed;
		
		vld_pressed = 0;

		osMessageQueuePut(queue_pointer_eventHandle,&win_touch_event, 0, 0);//xQueueSend(queue_pointer_event, &win_touch_event, 0);
	}
}

static void set_opt_sel(opt_cell cell , bool set, RgbStr clr)
{
	if (!set)	{		clr.color=clr.bcolor;	}
	
	if (cell.line ==TABLE_LINE+1)
	{
		int x=0, y=240;
		switch (cell.col)
		{
			case 0:	x=340;  break;  //ok
			case 1: x=394; break;    //cancel
			default:x=394; break;
		}
		ILI9488_WriteString(x, y, "@", Font_11x18, &clr);		
	}
	else
		ILI9488_WriteString(opt_coord[cell.line][cell.col].x , opt_coord[cell.line][cell.col].y, "@", Font_11x18, &clr);
	
}

/* only for task_lcd  */
static void widget_transd_command_handler(struct win_pointer_event const *event)
{
	if (g_flagCalibr==1) //Calibration is processing
		return;
		
	RgbStr clr = {Black,White};	
	
	//------------------ TOP ---------------------------
	if (event->buttons == UP)
	{		
		set_opt_sel(cur_cell,0,clr);			
		if (cur_cell.line==0)
		{				
			cur_cell.line=TABLE_LINE+1;  //kt25
			cur_cell.col=0;				//kt25 - ok
		}
		else
		{
			cur_cell.line--;
			if ( (cur_cell.line ==2  || cur_cell.line ==3 ) && cur_cell.col==3  )
				cur_cell.col=2;
		}			
		set_opt_sel(cur_cell,1,clr);
	}
	
	//------------------ BOTTOM ---------------------------
	if (event->buttons == BOT) 
	{	
		set_opt_sel(cur_cell,0,clr);
		if (cur_cell.line==(TABLE_LINE+1))  //kt25
		{				
			cur_cell.line=0;
			cur_cell.col=0;
		}
		else
		{
			cur_cell.line++;
			if (cur_cell.line==TABLE_LINE+1)  //kt25
				cur_cell.col=0;
			if ( (cur_cell.line ==2  || cur_cell.line ==3 ) && cur_cell.col==3  )
				cur_cell.col=2;
		}
		set_opt_sel(cur_cell,1,clr);
	}
	
	//------------------ LEFT ---------------------------
	if (event->buttons == F6) // <=
	{
		set_opt_sel(cur_cell,0,clr);
		if (cur_cell.col==0)
		{				
			if ( (cur_cell.line ==2)  || (cur_cell.line ==3)  )
				cur_cell.col=2;
			else if (  (cur_cell.line == TABLE_LINE+1) )  //ok, cancel
							cur_cell.col=1;
			else
				cur_cell.col=TABLE_COLUMN-1;
		}
		else
		{
			cur_cell.col--;
		}			
		set_opt_sel(cur_cell,1,clr);
	}
	
	//------------------ RIGHT ---------------------------
	if (event->buttons == F7) // =>
	{
		set_opt_sel(cur_cell,0,clr);
		if (cur_cell.col==(TABLE_COLUMN-1))
		{				
			cur_cell.col=0;
		}
		else
		{
			cur_cell.col++;
			if ( (cur_cell.col==3) && (cur_cell.line ==2  || cur_cell.line ==3   ))  //kt25
				cur_cell.col=0;
			if ( (cur_cell.col==2) && (cur_cell.line ==TABLE_LINE+1  ))  //ok, cancel
				cur_cell.col=0;
		}			
		set_opt_sel(cur_cell,1,clr);
	}
	
	//------------------ OK ---------------------------
	if (event->buttons == F2) 
	{
		if (cur_cell.line>0 && cur_cell.line<TABLE_LINE)			//options line except amp.gain
		{
			//notes
			RgbStr clr2 = {Black,RGB565(180,200,220)};
			ILI9488_WriteString(10, 266, "<>Position  ^Digit  ok-Save  F5-Cancel", Font_11x18, &clr2);
			
			char temp[16];
			switch (cur_cell.line)
			{
				case 0:		snprintf_(temp, 16, "%6.0f",	opt_transd_temp[cur_cell.col].fAmplGain);		break;
				case 1:		snprintf_(temp, 16, "%6.2f", opt_transd_temp[cur_cell.col].fSenset);			break;
				case 2:		if (cur_cell.col==3)	snprintf_(temp, 16, " ");
							else snprintf_(temp, 16, "%6.2f", opt_transd_temp[cur_cell.col].fNormRMS);
							break;
				case 3:		if (cur_cell.col==3)	snprintf_(temp, 16, " ");
							else snprintf_(temp, 16, "%6.2f", opt_transd_temp[cur_cell.col].fNormFreq);
							break;
				case 4:		snprintf_(temp, 16, "%6.2f", opt_transd_temp[cur_cell.col].fCalibr);			break;
			}
			
			//
			set_special_mode_status(DEMO_OPT_TRANSD, 0);
			app_widget_launch_inputbox(DEMO_OPT_TRANSD, opt_coord[cur_cell.line][cur_cell.col].x +Font_11x18.width, opt_coord[cur_cell.line][cur_cell.col].y,opt_coord[cur_cell.line][cur_cell.col].w-Font_11x18.width,opt_coord[cur_cell.line][cur_cell.col].h,temp,6,0);
			set_special_mode_status(DEMO_INPUTBOX, 1);
		}
		else if (cur_cell.line==0)	//amp.gain
		{
			//notes
			RgbStr clr3 = {Black,RGB565(180,200,220)};
			ILI9488_WriteString(10, 266, "^Prev/Next Value   ok-Save   F5-Cancel", Font_11x18, &clr3);
			
			set_special_mode_status(DEMO_OPT_TRANSD, 0);
			app_widget_launch_selectbox(opt_coord[cur_cell.line][cur_cell.col].x +Font_11x18.width, opt_coord[cur_cell.line][cur_cell.col].y,opt_coord[cur_cell.line][cur_cell.col].w-Font_11x18.width,opt_coord[cur_cell.line][cur_cell.col].h,opt_transd_temp[cur_cell.col].fAmplGain);
			set_special_mode_status(DEMO_SELECTBOX, 1);
		}
		else if (cur_cell.line==TABLE_LINE)  //calibr   //kt25
		{
			if (cur_cell.col >=0  && cur_cell.col <=3)	//Calibration buttons
			{
				do_calibration(cur_cell.col);
			}
		}
		else //ok, cancel buttons
		{

			if (cur_cell.col ==0)	//ok ->save current options, back to main menu
			{								
				memcpy(g_demo_parameters.opt_transd,  opt_transd_temp,    sizeof(opt_transducer)*4);
				memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t)); // FLASH
				transd_exit();
			}
			if (cur_cell.col==1)	//cancel, just back to main menu
			{
				transd_exit();		
			}
		}
	}
	
	//------------------ MENU ---------------------------
	if (event->buttons == F5) // menu
	{		
		transd_exit();	
	}
}

/**
 * \brief Widget start
 */
void app_widget_launch_transd(void)	//to parameters.h
{
	g_flagCalibr=0;
	
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_transd_command_handler;
	
	memcpy(opt_transd_temp,   g_demo_parameters.opt_transd, sizeof(opt_transducer)*4);
	
	draw_static();
}

void app_widget_launch_transd_back(bool save, float out_val)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_transd_command_handler;
	
	//notes
	RgbStr clr = {Black,RGB565(180,200,220)};
	ILI9488_WriteString(10, 266, "<^> Navigation   ok-Select              ", Font_11x18, &clr);
	
	//new val
	if (save)
	{
		int col=cur_cell.col;
		
		if (out_val==0.0f) out_val=1.0f;   //normally no any parameter can be zero (to prevent zero devideing in calculation). *can be changed by request
		
		switch (cur_cell.line)
		{
			case 0:		opt_transd_temp[col].fAmplGain	= out_val;		break;
			case 1:		opt_transd_temp[col].fSenset	= out_val;		break;
			case 2:		opt_transd_temp[col].fNormRMS  = out_val;		break;
			case 3:		opt_transd_temp[col].fNormFreq	= out_val;		break;
			case 4:		opt_transd_temp[col].fCalibr	= out_val;		break;
		}
	}	
	
	//update text
	set_text(cur_cell);
}

void do_calibration(int ch)
{
	RgbStr clr = {Red,White};
	switch (ch)
	{
	case 0: ILI9488_WriteString(2, 240, "processing ch-V1...          ", Font_11x18, &clr);	break; //214
	case 1: ILI9488_WriteString(2, 240, "processing ch-V2...          ", Font_11x18, &clr);	break; //214
	case 2: ILI9488_WriteString(2, 240, "processing ch-V3...          ", Font_11x18, &clr);	break; //214
	case 3: ILI9488_WriteString(2, 240, "processing ch-Sound...       ", Font_11x18, &clr);	break; //214
	default: return; break;	 //214
	}

	//vTaskDelay(500UL / portTICK_RATE_MS);  //alex
		
	//------------------INI------------------------

	if (ch==3)
	{
		//SOUND TRANDUCER:
		g_algo.SLMCaliStruct.fMICSensitivity	= opt_transd_temp[3].fSenset;   	//50.0 mv/pa //User options
		g_algo.SLMCaliStruct.SLMReferenceLevel	= opt_transd_temp[3].fNormRMS ;		//114.0;			//94.0	//User Option:
		g_algo.SLMCaliStruct.AmpGain			= opt_transd_temp[3].fAmplGain ;	//User Option:
		g_algo.SLMCaliStruct.Frequency			= opt_transd_temp[3].fNormFreq;		// 1000.0f;				//User Option : But u can fix it
		g_algo.SLMCaliStruct.dBRef				= 2e-5f;							//User Options: 20*1-^-6 Pa pressure: But u can fix it
		g_algo.SLMCaliStruct.fOutputCoef		= 1.0f;								//Fixed
		g_algo.SLMCaliStruct.nCaliOver			= 0;								//Fixed

		SLMCalibrationInit(&(g_algo.SLMCaliStruct));				//One time call before cali algo
	}
	else
	{
		//VIBRO TRANSDUCER:
		//for (int ch = 0; ch < 3; ch++)
		//{
			g_algo.VLMCaliStruct.fVibSensitivity[ch]	= opt_transd_temp[ch].fSenset;//800.0;			//50.0 mv/pa //User options
			g_algo.VLMCaliStruct.VLMReferenceLevel[ch]	= opt_transd_temp[ch].fNormRMS;//114.0;			//94.0	//User Option:
			g_algo.VLMCaliStruct.AmpGain[ch]			= opt_transd_temp[ch].fAmplGain;//User Option:
			g_algo.VLMCaliStruct.Frequency[ch]			= opt_transd_temp[ch].fNormFreq;//60.0					//User Option : But u can fix it

			g_algo.VLMCaliStruct.dBRef[ch]				= 0.000001f;//*g_demo_parameters.opt_analyz[1].fDbRefAcc;//User Options: 20*1-^-6 Pa pressure: But u can fix it //kt:  mistake?
			g_algo.VLMCaliStruct.fOutputCoef[ch]		= 1.0f;				//Fixed
			g_algo.VLMCaliStruct.nCaliOver[ch]			= 0;				//Fixed

			VLMCalibrationInit(&(g_algo.VLMCaliStruct), ch);				//One time call before cali algo
		//}
	}

	//-here is getting data from ADC and calculate

	/*	//Call the Calibration sound
		SLMcalibrationnByChannel(&(g_algo.SLMCaliStruct), data);

		if (g_algo.SLMCaliStruct.nCaliOver)	//Calibration Status
		{
			printf("\n Calibration Process Status:\n");
			switch (g_algo.SLMCaliStruct.iCalibrationReturnValue)
			{
				case 0:		printf("Error (0) :calculation Error \n");			break;
				case -1:	printf("Error (-1): Level of input signal is too low.\n");			break;
				case -2:	printf("Error (-2): system error, need a restart\n");			break;
				case -4:	printf("Error (-3): Level of input signal is too high.\n");			break;
				case -32:	printf("Error (-32): the ref frequency maybe not accurate\n");			break;
				default:    //printf("Error %d:",SLMCaliStrucPtr->iCalibrationReturnValue); //other  unknown error
				break;
			}
			//Calibration constant that need to be stored
			//"SLMCaliStrucPtr->fOutputCoef" must be rememberd by the device forever (i.e irrespetive of power on or off)
			//until the next call of "calibrationnByChannel( )".
			//This variable is utilized in SVLM algo processing. If this is not stored, the SVLM output will be erroneous
			printf("\ncali Coeff is: %f", g_algo.SLMCaliStruct.fOutputCoef); //Remove this line
		}*/

	g_flagCalibrChan = ch;
		g_flagCalibr=1;//start Calibration in task_dsp
		short count=0;

		while (g_flagCalibr!=0 && count<10)
		{
			//waiting
			count++;
			//vTaskDelay(500UL / portTICK_RATE_MS);  //alex
			osDelay(500);
		}
		g_flagCalibr=0; //reset

		//result
		char tmp[32];
		tmp[0]=0;

		if (ch==3) //sound
		{
			switch (g_algo.SLMCaliStruct.iCalibrationReturnValue)
			{                              //---------/---------/--------/ 29char is max
				case 0:		snprintf_(tmp,32,"Calculation error           ");		break;	//printf("Error (0) :calculation Error \n");
				case -1:	snprintf_(tmp,32,"Level of signal is too low  ");		break;	//printf("Error (-1): Level of input signal is too low.\n");
				case -2:	snprintf_(tmp,32,"System error, need a restart");		break;	//printf("Error (-2): system error, need a restart\n");
				case -4:	snprintf_(tmp,32,"Level of signal is too high ");		break;	//printf("Error (-3): Level of input signal is too high.\n");
				case -32:	snprintf_(tmp,32,"Ref.freq maybe not accurate ");		break;	//printf("Error (-32): the ref frequency maybe not accurate\n");
				default:	snprintf_(tmp,32,"Error: %d                   ",			g_algo.SLMCaliStruct.iCalibrationReturnValue);		//printf("Error %d:",VLMCaliStrucPtr->iCalibrationReturnValue); //other  unknown erro
				break;
			}

			if (g_algo.SLMCaliStruct.iCalibrationReturnValue>0)
				opt_transd_temp[ch].fCalibr=g_algo.SLMCaliStruct.fOutputCoef;
			else
				opt_transd_temp[ch].fCalibr=1.0f;  //in case of error

			set_text_(4, ch );

			if(g_algo.SLMCaliStruct.iCalibrationReturnValue>0)
				snprintf_(tmp,32,"Calibration is done");

			ILI9488_WriteString(2, 240, tmp, Font_11x18, &clr);  //214

		}
		if (ch<3)  //vibro
		{
			switch (g_algo.VLMCaliStruct.iCalibrationReturnValue[ch])
			{
				case 0:		snprintf_(tmp,32,"Calculation error           ");		break;	//printf("Error (0) :calculation Error \n");
				case -1:	snprintf_(tmp,32,"Level of signal is too low  ");		break;	//printf("Error (-1): Level of input signal is too low.\n");
				case -2:	snprintf_(tmp,32,"System error, need a restart");		break;	//printf("Error (-2): system error, need a restart\n");
				case -4:	snprintf_(tmp,32,"Level of signal is too high ");		break;	//printf("Error (-3): Level of input signal is too high.\n");
				case -32:	snprintf_(tmp,32,"Ref.freq maybe not accurate ");		break;	//printf("Error (-32): the ref frequency maybe not accurate\n");
				default:	snprintf_(tmp,32,"Error: %d                   ",			g_algo.VLMCaliStruct.iCalibrationReturnValue[ch]);		//printf("Error %d:",VLMCaliStrucPtr->iCalibrationReturnValue); //other  unknown erro
				break;
			}


			//Calibration constant that need to be stored
			//"VLMCaliStrucPtr->fOutputCoef" must be rememberd by the device forever (i.e irrespetive of power on or off)
			//until the next call of "calibrationnByChannel( )".
			//This variable is utilized in SVLM algo processing. If this is not stored, the SVLM ouput will be erroneous
			//printf("\ncali Coeff is: %f", g_algo.VLMCaliStruct.fOutputCoef[ch]); //Remove this line

			if (g_algo.VLMCaliStruct.iCalibrationReturnValue[ch]>0)
				opt_transd_temp[ch].fCalibr=g_algo.VLMCaliStruct.fOutputCoef[ch];
			else
				opt_transd_temp[ch].fCalibr=1.0f;  //in case of error

			set_text_(4, ch );

			if(g_algo.VLMCaliStruct.iCalibrationReturnValue[ch]>0 )
				snprintf_(tmp,32,"Calibration is done");

			ILI9488_WriteString(2, 240, tmp, Font_11x18, &clr); //214
		}
	
}
