/*
 * widget_opt_analyz_evs.c
 *
 * Created: 2/27/2023 11:47:07 AM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"

/*
//to parameters.h
extern void opt_an_evs_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_an_evs(void);
void draw_...._update(float val);
*/

#define OPT_EVS_MAX_ID	5		//count of option's line in the screen

static const char* opt_items[] = {
	"   Sound Integr.Time:", 
	"Sound Freq.Weighting:", 
	"Sound Time Weighting:", 
	"   Vibro Integr.Time:", 
	"     Acc dBReference:"
};

//see in parameters.h:
float opt_Val_0[]={0.100f, 1.0f};  //22.05  {0.125f, 1.0f};
int   opt_Val_1[]={1, 0};
int   opt_Val_2[]={0, 1};
float opt_Val_3[]={0.100f, 1.0f};   //22.05  {0.125f, 1.0f};
float opt_Val_4[]={1.0f, 10.0f};
//	bool  bCalcLmax;		//for Blast only: calculate Lmax&L10(for z-axis)

static int opt_top[OPT_EVS_MAX_ID];///
static int opt_val_indx[OPT_EVS_MAX_ID][2];///   //second ind: [0]-count of value in opt_Val_x; [1] - index of value in opt_Val_x for every option;
static int opt_ind/*=0*/;						//ind of current option
static bool sub_opt_mode/*=0*/;	///			//0-moving across options line,  1-moving inside selected option

static const char* GetOptText(int line)
{
	const char* opt_text_0[]={ " 0.1 s ", " 1.0 s "};
	const char* opt_text_1[]={ " A     ", " Z     "};
	const char* opt_text_2[]={ " F     ", " S     " };   //in algo F=0, S=1, I=2  // in algo SLMStruct->TimeWeight = 0.5  and 1.0
	const char* opt_text_3[]={ " 0.1 s ", " 1.0 s "};
	const char* opt_text_4[]={ " 1   "  , " 10  "};
		
	switch (line)
	{
		case 0:	return	opt_text_0[	opt_val_indx[line][1]];	break;
		case 1:	return	opt_text_1[	opt_val_indx[line][1]];	break;
		case 2:	return	opt_text_2[	opt_val_indx[line][1]];	break;
		case 3:	return	opt_text_3[	opt_val_indx[line][1]];	break;
		case 4:	return	opt_text_4[	opt_val_indx[line][1]];	break;
		default:return	opt_text_0[0];
	}
}

static void getDefValIndex(void)
{
	int i;
	for (i=0;i<OPT_EVS_MAX_ID;i++)	{	opt_val_indx[i][0]=0;	opt_val_indx[i][1]=0;	}
	
	opt_val_indx[0][0]=sizeof(opt_Val_0)/sizeof(float);
	for (i=0;i<opt_val_indx[0][0];i++)
	{	if (g_demo_parameters.opt_analyz[0].fIntegrTime==opt_Val_0[i])	opt_val_indx[0][1]=i;	}
		
	opt_val_indx[1][0]=sizeof(opt_Val_1)/sizeof(int);
	for (i=0;i<opt_val_indx[1][0];i++)
	{	if (g_demo_parameters.opt_analyz[0].nFreqWeight==opt_Val_1[i])	opt_val_indx[1][1]=i;	}
		
	opt_val_indx[2][0]=sizeof(opt_Val_2)/sizeof(int);
	for (i=0;i<opt_val_indx[2][0];i++)
	{	if (g_demo_parameters.opt_analyz[0].nTimeWeight==opt_Val_2[i])	opt_val_indx[2][1]=i;	}
		
	opt_val_indx[3][0]=sizeof(opt_Val_3)/sizeof(float);
	for (i=0;i<opt_val_indx[3][0];i++)
	{	if (g_demo_parameters.opt_analyz[0].fVibroIntegrTime==opt_Val_3[i])	opt_val_indx[3][1]=i;	}
		
	opt_val_indx[4][0]=sizeof(opt_Val_4)/sizeof(float);
	for (i=0;i<opt_val_indx[4][0];i++)
	{	if (g_demo_parameters.opt_analyz[0].fDbRefAcc==opt_Val_4[i])	opt_val_indx[4][1]=i;	}
}

static void set_opt_sel(int indx , bool set, RgbStr clr)
{
	if (!set)	{		clr.color=clr.bcolor;	}
	
	int x=0, y=0;
	if (indx<OPT_EVS_MAX_ID)			{	x=10; y=opt_top[indx];	}
	else if (indx==OPT_EVS_MAX_ID)		{	x=182; y=230;	}  
	else if (indx==(OPT_EVS_MAX_ID+1))	{	x=260; y=230;	} 
	ILI9488_WriteString(x, y, "@", Font_11x18, &clr);
}

static void set_opt_val_sel(int indx , bool set, RgbStr clr)
{
	if (set)	{	clr.color = Black;   
					clr.bcolor = RGB565(0xFF,0x96,0xFF);}  //magenta box
	ILI9488_WriteString(270, opt_top[indx], GetOptText(indx), Font_11x18, &clr);
}

/**
 * \brief rendering on the LCD.
 */
static void draw_static(void)
{
	opt_ind=0;
	
	ILI9488_FillRectangle(0, 32, ILI9488_WIDTH, 258,White);
	
	RgbStr clr = {RGB565(0,0x82,0),White};
	ILI9488_WriteString(140, 34, "EVS Analyzing Options:", Font_11x18, &clr);	//title
	
	clr.color = Black;
	ILI9488_WriteString(200, 230, "OK     Cancel", Font_11x18, &clr);			//ok, cancel
	ILI9488_FillRectangle(0, 262, 480, 1,Black);
	ILI9488_FillRectangle(0, 263, 480, 27,RGB565(180,200,220));
	
	getDefValIndex();

	int i;
	for (i=0;i<OPT_EVS_MAX_ID;i++)
	{
		opt_top[i]=68+24*i;
		ILI9488_WriteString(30,  opt_top[i], opt_items[i], Font_11x18, &clr);
		ILI9488_WriteString(270, opt_top[i], GetOptText(i), Font_11x18, &clr);
	}
	
	ILI9488_WriteString(330, opt_top[i-1],"[*10 m/s ]"  , Font_11x18, &clr);
	ILI9488_WriteString(372, opt_top[i-1]-4,"-6", Font_7x10, &clr);
	ILI9488_WriteString(416, opt_top[i-1]-4,"2", Font_7x10, &clr);
	
	set_opt_sel(opt_ind,1,clr);
	
	//notes
	clr.bcolor = RGB565(180,200,220);
	ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select   ", Font_11x18, &clr);
}

/* called from task_key  */
void opt_an_evs_win_KEY_handler(uint32_t key_pad)
{
	static uint8_t 	vld_pressed = 0;
	
	switch (key_pad)
	{
		case UP: // 
			vld_pressed = 1;			break;
		case F2: // 
			vld_pressed = 2;			break;
		case BOT: //	
			vld_pressed = 3;			break;	
		case F5: // menu
			vld_pressed = 5;			break;
		case F6: // <-
			vld_pressed = 6;			break;
		case F7: // ->
			vld_pressed = 7;			break;
		break;
	}

	if (vld_pressed) {
		struct win_pointer_event win_touch_event;
		win_touch_event.buttons = vld_pressed;
		
		vld_pressed = 0;

		osMessageQueuePut(queue_pointer_eventHandle,&win_touch_event, 0, 0);//xQueueSend(queue_pointer_event, &win_touch_event, 0);
	}
}

static void opt_analyz_evs_exit(void)
{
	if (g_demo_parameters.nMeasMode ==0) //must be
	{
		set_special_mode_status(DEMO_OPT_ANALYZ_EVS, 0);
		app_widget_launch_evs();
		set_special_mode_status(DEMO_MODE_EVS, 1);
	}	
}
/* only for task_lcd  */
static void widget_opt_an_evs_command_handler(struct win_pointer_event const *event)
{
	RgbStr clr = {Black,White};	
	
	//------------------ TOP ---------------------------
	if (event->buttons == UP)
	{
		if (!sub_opt_mode)	//moving across options line
		{			
			if (opt_ind==0)
			{
				set_opt_sel(opt_ind,0,clr);
				opt_ind=OPT_EVS_MAX_ID+1;
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
			opt_val_indx[opt_ind][1]--;
			if (opt_val_indx[opt_ind][1]<0)
			opt_val_indx[opt_ind][1]=opt_val_indx[opt_ind][0]-1;
			set_opt_val_sel(opt_ind,1,clr);
		}
	}
	
	//------------------ BOTTOM ---------------------------
	if (event->buttons == BOT) 
	{	
		if (!sub_opt_mode)	//moving across options line
		{
			if (opt_ind==(OPT_EVS_MAX_ID+1))
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
			opt_val_indx[opt_ind][1]++;
			if (opt_val_indx[opt_ind][1]>=opt_val_indx[opt_ind][0])
			opt_val_indx[opt_ind][1]=0;
			set_opt_val_sel(opt_ind,1,clr);
		}	
	}
	
	//------------------ OK ---------------------------
	if (event->buttons == F2) 
	{
		if (opt_ind<OPT_EVS_MAX_ID)											//options line
		{
			sub_opt_mode=!sub_opt_mode;
			set_opt_val_sel(opt_ind , sub_opt_mode, clr);
			
			if (sub_opt_mode)	//notes
			{				
				clr.bcolor = RGB565(180,200,220);
				ILI9488_WriteString(10, 266, "^ Prev/Next  ok-Confirm", Font_11x18, &clr);
			}
			else
			{
				clr.bcolor = RGB565(180,200,220);
				ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select   ", Font_11x18, &clr);
			}
		}
		else if (opt_ind==OPT_EVS_MAX_ID)									//ok ->save current options, back to main menu
		{
			g_demo_parameters.opt_analyz[0].fIntegrTime		=opt_Val_0[opt_val_indx[0][1]];
			g_demo_parameters.opt_analyz[0].nFreqWeight		=opt_Val_1[opt_val_indx[1][1]];	
			g_demo_parameters.opt_analyz[0].nTimeWeight		=opt_Val_2[opt_val_indx[2][1]];	
			g_demo_parameters.opt_analyz[0].fVibroIntegrTime=opt_Val_3[opt_val_indx[3][1]];	
			g_demo_parameters.opt_analyz[0].fDbRefAcc		=opt_Val_4[opt_val_indx[4][1]];
			memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t)); // FLASH
			
			opt_analyz_evs_exit();
		}
		else if (opt_ind==(OPT_EVS_MAX_ID+1))								//cancel, just back to main menu
		{
			opt_analyz_evs_exit();			
		}
	}
	
	//------------------ MENU ---------------------------
	if (event->buttons == F5) // menu
	{
		opt_analyz_evs_exit();
	}

	//------------------ LEFT-RIGHT ---------------------------
	if (event->buttons == F6) // <=
	{
//		snprintf_(hole1, 64, "F6");		ILI9488_WriteString(20, 296, hole1, Font_11x18, &clr);
	}
	if (event->buttons == F7) // =>
	{
//		snprintf_(hole1, 64, "F7");		ILI9488_WriteString(20, 296, hole1, Font_11x18, &clr);
	}
}

/**
 * \brief Widget start
 */
void app_widget_launch_opt_an_evs(void)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_opt_an_evs_command_handler;
	
	opt_ind=0;						//ind of current option
	sub_opt_mode=0;

	draw_static();
}
