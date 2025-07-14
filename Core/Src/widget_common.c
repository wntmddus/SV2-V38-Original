/*
 * widget_common.c
 *
 * Created: 11/7/2023 6:42:11 PM
 *  Author: Tatsiana
 */ 

#include <string.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"
#include "main.h"

/*
//to parameters.h
extern void opt_common_win_KEY_handler(uint32_t key_pad); 
void app_widget_launch_opt_common(void);
*/
extern RTC_HandleTypeDef hrtc;
#define OPT_COMMON_MAX_ID	4		//count of option's line in the screen

static const char* opt_items[] = {
	" Date/Time:",
	"     Model:",
	"Ser.Number:",
	"  Operator:"
};

opt_common opt_info_temp;

static int opt_top[OPT_COMMON_MAX_ID];///
static int opt_ind/*=0*/;			//ind of current option
static int sub_opt_ind=0;		//ind of current sub-option inside line with index opt_ind.  0-not selected, 1-year,2-month, 3-day, 4-hour, 5-min, 6-sec


static int opt_Val_0[6]; //for data-time
static int opt_x[6]	; //x for data-time
//static uint16_t sub_opt_mode; //ind of current sub-option inside line with index opt_ind.  0-not selected, 1-year,2-month, 3-day, 4-hour, 5-min, 6-sec
static char m_temp_text[8];

static const char* GetOptText(int line, int sub_line)
{
	switch (line)
	{
		case 0:	
			if (sub_line==1) snprintf_(m_temp_text,8,"%4d",opt_Val_0[sub_line-1]);  
			else snprintf_(m_temp_text,8,"%2d",opt_Val_0[sub_line-1]); 
		    return m_temp_text;	
			break;
		case 1:	return	opt_info_temp.str_model ;	break;
		case 2:	return	opt_info_temp.str_sernumber;		break;
		case 3:	return	opt_info_temp.str_operator;		break;
		
		default:return	m_temp_text;
	}
}

static void set_opt_sel(int indx , bool set, RgbStr clr)
{
	if (!set)	{		clr.color=clr.bcolor;	}
	
	int x=0, y=0;
	if (indx<OPT_COMMON_MAX_ID)			{	x=10; y=opt_top[indx];	}
	else if (indx==OPT_COMMON_MAX_ID)		{	x=182; y=230;	}
	else if (indx==(OPT_COMMON_MAX_ID+1))	{	x=260; y=230;	}
	ILI9488_WriteString(x, y, "@", Font_11x18, &clr);
}

static void set_opt_val_sel(int indx , int sub_indx, bool set, RgbStr clr)
{
	if (set)	{	clr.color = Black;
	clr.bcolor = RGB565(0xFF,0x96,0xFF);}  //magenta box
	ILI9488_WriteString(opt_x[sub_indx-1], opt_top[indx], GetOptText(indx,sub_indx), Font_11x18, &clr);
}

static void common_exit(void)
{
	set_special_mode_status(DEMO_OPT_COMMON, 0);
	if (g_demo_parameters.nMeasMode ==0)
	{
		app_widget_launch_evs();
		set_special_mode_status(DEMO_MODE_EVS, 1);
	}
	if (g_demo_parameters.nMeasMode ==1)
	{
		app_widget_launch_blast();
		set_special_mode_status(DEMO_MODE_BLAST, 1);
	}
	
}

/* called from task_key  */
void opt_common_win_KEY_handler(uint32_t key_pad)
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
			HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			
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

/* only for task_lcd  */
static void widget_opt_common_command_handler(struct win_pointer_event const *event)
{
	RgbStr clr = {Black, White};
	RgbStr clr2 = {Black,RGB565(180,200,220)};
	//------------------ TOP ---------------------------
	if (event->buttons == UP)
	{
		if (sub_opt_ind==0)	//moving across options line
		{			
			if (opt_ind==0)
			{
				set_opt_sel(opt_ind,0,clr);
				opt_ind=OPT_COMMON_MAX_ID+1;
				set_opt_sel(opt_ind,1,clr);
			}
			else
			{
				set_opt_sel(opt_ind,0,clr);
				opt_ind--;
				set_opt_sel(opt_ind,1,clr);
			}
		}
		else //moving inside selected option
		{
			int max=0,min=0;
			switch (sub_opt_ind)
			{
				case 1:	max=2050;	min=2020;	break;//year
				case 2:	max=12;		min=1;		break;//month
				case 3:	max=31;		min=1;		break;//day
				case 4:	max=23;		min=0;		break;//h
				case 5:	max=59;		min=0;		break;//min
				case 6:	max=59;		min=0;		break;//sec				
			}
			opt_Val_0[sub_opt_ind-1]++;
			if (opt_Val_0[sub_opt_ind-1]>max)
				opt_Val_0[sub_opt_ind-1]=min;
				
			set_opt_val_sel(opt_ind,sub_opt_ind,1,clr);
		}
	}
	
	//------------------ BOTTOM ---------------------------
	if (event->buttons == BOT) 
	{	
		if (sub_opt_ind==0)	//moving across options line
		{
			if (opt_ind==(OPT_COMMON_MAX_ID+1))
			{
				set_opt_sel(opt_ind,0,clr);
				opt_ind=0;
				set_opt_sel(opt_ind,1,clr);
			}
			else
			{
				set_opt_sel(opt_ind,0,clr);
				opt_ind++;
				set_opt_sel(opt_ind,1,clr);
			}
		}	
		else //moving inside selected option
		{
			int max=0,min=0;
			switch (sub_opt_ind)
			{
				case 1:	max=2050;	min=2020;	break;//year
				case 2:	max=12;		min=1;		break;//month
				case 3:	max=31;		min=1;		break;//day
				case 4:	max=23;		min=0;		break;//h
				case 5:	max=59;		min=0;		break;//min
				case 6:	max=59;		min=0;		break;//sec
			}
			opt_Val_0[sub_opt_ind-1]--;
			if (opt_Val_0[sub_opt_ind-1]<min)
			opt_Val_0[sub_opt_ind-1]=max;
			
			set_opt_val_sel(opt_ind,sub_opt_ind,1,clr);
		}	
	}
	
	//------------------ OK ---------------------------
	if (event->buttons == F2) 
	{
		if (opt_ind<OPT_COMMON_MAX_ID)											//options line
		{
			//notes
			//RgbStr clr2 = {0x00,0x00,0x00,180,200,220};
			ILI9488_WriteString(10, 266, "<>Position  ^Digit  ok-Save  F5-Cancel", Font_11x18, &clr2);
			
			if (opt_ind==0)  //data-time
			{			
				if (sub_opt_ind==0)
				{
					sub_opt_ind=1; //set to 'year'
					set_opt_val_sel(opt_ind,sub_opt_ind,1,clr);					
					
					ILI9488_WriteString(10, 266, "<>Option  ^Value  ok-Exit", Font_11x18, &clr2);
				}
				else
				{
					set_opt_val_sel(opt_ind,sub_opt_ind,0,clr);
					sub_opt_ind=0; //deselect
					
					ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select     ", Font_11x18, &clr2);
				}
			}
			
			if (1<=opt_ind && opt_ind<=3)  //model  //sernumber  //operator
			{
				set_special_mode_status(DEMO_OPT_COMMON, 0);
				app_widget_launch_inputbox(DEMO_OPT_COMMON, 170,  opt_top[opt_ind],28*Font_11x18.width,24, GetOptText(opt_ind,0),28,1);
				set_special_mode_status(DEMO_INPUTBOX, 1);
			}
		}
		else if (opt_ind==OPT_COMMON_MAX_ID)									//ok ->save current options, back to main menu
		{
			
			  RTC_DateTypeDef  csdatestructure;
			  RTC_TimeTypeDef  cstimestructure;

			  /*##-1- Configure the Date #################################################*/
			  csdatestructure.Year = opt_Val_0[0] - 2000;
			  csdatestructure.Month = opt_Val_0[1];
			  csdatestructure.Date = opt_Val_0[2];
			  csdatestructure.WeekDay = 2;

			  if(HAL_RTC_SetDate(&hrtc,&csdatestructure,RTC_FORMAT_BIN) != HAL_OK)
			  {
			      Error_Handler();
			  }

			  /*##-2- Configure the Time #################################################*/
			  cstimestructure.Hours = opt_Val_0[3];
			  cstimestructure.Minutes = opt_Val_0[4];
			  cstimestructure.Seconds = opt_Val_0[5];
			  cstimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
			  cstimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
			  cstimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

			  if(HAL_RTC_SetTime(&hrtc,&cstimestructure,RTC_FORMAT_BIN) != HAL_OK)
			  {
			    Error_Handler();
			  }


			refresh_system_time();

			snprintf_(g_demo_parameters.opt_info.str_model		, 32, "%s",	opt_info_temp.str_model);
			snprintf_(g_demo_parameters.opt_info.str_sernumber	, 32, "%s",	opt_info_temp.str_sernumber );
			snprintf_(g_demo_parameters.opt_info.str_operator	, 32, "%s",	opt_info_temp.str_operator );
			memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t)); // FLASH
			
			common_exit();
		}
		else if (opt_ind==(OPT_COMMON_MAX_ID+1))								//cancel, just back to main menu
		{
			common_exit();		
		}
	}
	
	//------------------ MENU ---------------------------
	if (event->buttons == F5) // menu
	{
		common_exit();
	}

	//------------------ LEFT-LEFT ---------------------------
	if (event->buttons == F6) // <=
	{
		if (opt_ind==0 && sub_opt_ind>0)  //data-time
		{
			set_opt_val_sel(opt_ind,sub_opt_ind,0,clr);
			sub_opt_ind--; //next option
			if (sub_opt_ind<1)
				sub_opt_ind=6;
			set_opt_val_sel(opt_ind,sub_opt_ind,1,clr);
		}
	}
	if (event->buttons == F7) // =>
	{
		if (opt_ind==0 && sub_opt_ind>0)  //data-time
		{
			set_opt_val_sel(opt_ind,sub_opt_ind,0,clr);
			sub_opt_ind++; //next option
			if (sub_opt_ind>=7)
			sub_opt_ind=1;
			set_opt_val_sel(opt_ind,sub_opt_ind,1,clr);
		}
	}
}

/**
 * \brief rendering on the LCD.
 */
static void draw_static(void)
{
//	opt_ind=0;
	RTC_TimeTypeDef scTimest = {0};
	RTC_DateTypeDef scDatest = {0};
	uint32_t cyear, cmonth, cday, chour, cminute, csecond;
	
	ILI9488_FillRectangle(0, 32, ILI9488_WIDTH, 258,White);
	
	RgbStr clr = {RGB565(0,0x82,0),White};
	ILI9488_WriteString(140, 34, "Common Options:", Font_11x18, &clr);	//title
	
	clr.color = Black;
	ILI9488_WriteString(200, 230, "OK     Cancel", Font_11x18, &clr);			//ok, cancel
	ILI9488_FillRectangle(0, 262, 480, 1,Black);
	ILI9488_FillRectangle(0, 263, 480, 27,RGB565(180,200,220));
	
	int i;
	for (i=0;i<OPT_COMMON_MAX_ID;i++)
	{
		opt_top[i]=68+24*i;
		ILI9488_WriteString(30,  opt_top[i], opt_items[i], Font_11x18, &clr);
		if (i!=0)
			ILI9488_WriteString(170, opt_top[i], GetOptText(i,0), Font_11x18, &clr);
	}
	
	//======================================================================================
	//                                     data-time	

	HAL_RTC_GetTime(&hrtc, &scTimest, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &scDatest, RTC_FORMAT_BIN);

	cyear = scDatest.Year + 2000;
	cmonth = scDatest.Month;
	cday = scDatest.Date;
	chour = scTimest.Hours;
	cminute = scTimest.Minutes;
	csecond = scTimest.Seconds;

	
	opt_Val_0[0]=cyear;
	opt_Val_0[1]=cmonth;
	opt_Val_0[2]=cday;
	opt_Val_0[3]=chour;
	opt_Val_0[4]=cminute;
	opt_Val_0[5]=csecond;
	
	char text[8];
	int x=170;
	//ILI9488_WriteString(30, opt_top[0], "Date/Time: ", Font_11x18, &clr);
	
	opt_x[0]=x;
	snprintf_(text,8,"%4d.",(int)cyear);
	ILI9488_WriteString(x, opt_top[0], text, Font_11x18, &clr);	x=x+50;
	
	opt_x[1]=x;
	snprintf_(text,8,"%2d.",(int)cmonth);
	ILI9488_WriteString(x, opt_top[0], text, Font_11x18, &clr);	x=x+30;
	
	opt_x[2]=x;
	snprintf_(text,8,"%2d",(int)cday);
	ILI9488_WriteString(x, opt_top[0], text, Font_11x18, &clr);	x=x+60;
	
	opt_x[3]=x;
	snprintf_(text,8,"%2d:",(int)chour);
	ILI9488_WriteString(x, opt_top[0], text, Font_11x18, &clr);	x=x+30;
	
	opt_x[4]=x;
	snprintf_(text,8,"%2d:",(int)cminute);
	ILI9488_WriteString(x, opt_top[0], text, Font_11x18, &clr);	x=x+30;
	
	opt_x[5]=x;
	snprintf_(text,8,"%2d",(int)csecond);
	ILI9488_WriteString(x, opt_top[0], text, Font_11x18, &clr);	x=x+30;
	
	//                              data-time
	//==========================================================================

	set_opt_sel(opt_ind,1,clr);
	
	//notes
	clr.bcolor = RGB565(180,200,220);
	ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select     ", Font_11x18, &clr);
}


/**
 * \brief Widget start
 */
void app_widget_launch_opt_common(void)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_opt_common_command_handler;
	
	memcpy(&opt_info_temp,   &g_demo_parameters.opt_info, sizeof(opt_common));
	
	opt_ind=0;						//ind of current option
	//sub_opt_mode=0;

	draw_static();
}

static void set_text(int line)
{
	int x= 170;
	int y= opt_top[line];
	int w= Font_11x18.width;
	int h= 24;

	switch (line)
	{
		case 0:		w=w*32;	break;		
		case 1:		w=w*32;	break;		
		case 2:		w=w*32;	break;		
		case 3:		w=w*32;	break;		
	}
	
	ILI9488_FillRectangle(x, y-5, w, h,White);
	RgbStr clr = {Black, White};
	
	ILI9488_WriteString(x ,y, GetOptText(opt_ind,0), Font_11x18, &clr);
}

void app_widget_launch_common_back(bool save, float out_val, const char* str)	//to parameters.h
{
		struct win_attributes *attributes;

		attributes = &win_attributes_root;
		attributes->event_handler = widget_opt_common_command_handler;
		
		//notes
		RgbStr clr = {Black,RGB565(180,200,220)};
		ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select                  ", Font_11x18, &clr);
		
		//int dif;
		//new val
		if (save)
		{
			switch (opt_ind)
			{
				case 0: //date
					break;
				case 1:	//model
					snprintf_(opt_info_temp.str_model, 32, "%s",	str);	break;
				
				case 2:	//sernumber
					snprintf_(opt_info_temp.str_sernumber, 32, "%s",	str);	break;

				case 3:	//password
				snprintf_(opt_info_temp.str_operator , 32, "%s",	str);	break;
			}
		}
		
		//update text
		set_text(opt_ind);	
}
