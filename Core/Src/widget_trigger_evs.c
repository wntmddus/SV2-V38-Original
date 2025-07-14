/*
 * widget_trigger_evs.c
 *
 * Created: 3/14/2023 6:12:43 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"

/*
//to parameters.h
extern void trigger_evs_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_trigger_evs(void);
*/

#define TRIGG_EVS_MAX_ID	5		//count of option's line in the screen

static const char* opt_items[] = {
	"TriggerLevel(VibrationRMS):       dB",
	"         Use Sound Trigger:",
	"       Sound Trigger Level:       dB",
	"            Recording Time:",
	"    DelayTime(Contin.Mode):"
};

//see in parameters.h:
static float opt_Val_0[]={45.0f, 46.0f, 47.0f, 48.0f, 49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f, 57.0f, 58.0f, 59.0f, 60.0f};
static int   opt_Val_1[]={0, 1};
static float opt_Val_2[]={60.0f, 61.0f, 62.0f, 63.0f, 64.0f, 65.0f, 66.0f, 67.0f, 68.0f, 69.0f, 70.0f, 71.0f, 72.0f, 73.0f, 74.0f, 75.0f};
static float opt_Val_3[]={1.0f, 2.0f, 3.0f, 5.0f, 10.0f, 60.0f, 300.0f, 3600.0f}; //
static int   opt_Val_4[]={0, 1, 5, 10};
	
int opt_top[TRIGG_EVS_MAX_ID];
int opt_val_indx[TRIGG_EVS_MAX_ID][2];	//second ind: [0]-count of value in opt_Val_x; [1] - index of value in opt_Val_x for every option;
static int opt_ind=0;					//ind of current option
static bool sub_opt_mode=0;				//0-moving across options line,  1-moving inside selected option


//=================================================================
static const char* GetOptText(int line)
{ 
	const char* opt_text_0[]=   { "  45   ", "  46   ","  47   ","  48   ","  49   ","  50   ","  51   ","  52   ","  53   ","  54   ","  55   ","  56   ","  57   ","  58   ","  59   ","  60   "};
	const char* opt_text_1[]=	{ "  NO   ", "  YES  " };
	const char* opt_text_2[]=   { "  60   ", "  61   ","  62   ","  63   ","  64   ","  65   ","  66   ","  67   ","  68   ","  69   ","  70   ","  71   ","  72   ","  73   ","  74   ","  75   "};
	const char* opt_text_3[]=	{ "  1 s  ", "  2 s  ","  3 s  ","  5 s  ","  10 s ","  1 min","  5 min"," 1 hour"};  //
	const char* opt_text_4[]=	{ "  none ", "  1 s  ","  5 s  ","  10 s "    };

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
	for (i=0;i<TRIGG_EVS_MAX_ID;i++)	{	opt_val_indx[i][0]=0;	opt_val_indx[i][1]=0;	}
	
	opt_val_indx[0][0]=sizeof(opt_Val_0)/sizeof(float);
	for (i=0;i<opt_val_indx[0][0];i++)
	{	if (g_demo_parameters.opt_trig[0].fLevel==opt_Val_0[i])	opt_val_indx[0][1]=i;	}
	
	opt_val_indx[1][0]=sizeof(opt_Val_1)/sizeof(int);
		for (i=0;i<opt_val_indx[1][0];i++)
		{	if (g_demo_parameters.opt_trig[0].nUseSoundTrigger==opt_Val_1[i]) opt_val_indx[1][1]=i;	}

	opt_val_indx[2][0]=sizeof(opt_Val_2)/sizeof(int);
		for (i=0;i<opt_val_indx[2][0];i++)
		{	if (g_demo_parameters.opt_trig[0].fLevelSoundTrigger==opt_Val_2[i])	opt_val_indx[2][1]=i;	}

	opt_val_indx[3][0]=sizeof(opt_Val_3)/sizeof(int);
	for (i=0;i<opt_val_indx[3][0];i++)
	{	if (g_demo_parameters.opt_trig[0].fPostTrigTime==opt_Val_3[i]) opt_val_indx[3][1]=i;	}
	
	opt_val_indx[4][0]=sizeof(opt_Val_4)/sizeof(int);
	for (i=0;i<opt_val_indx[4][0];i++)
	{	if (g_demo_parameters.opt_trig[0].nDelay==opt_Val_4[i])	opt_val_indx[4][1]=i;	}
}

static void set_opt_sel(int indx , bool set, RgbStr clr)
{
	if (!set)	{		clr.color=clr.bcolor;	}
	
	int x=0, y=0;
	if (indx<TRIGG_EVS_MAX_ID)			{	x=10; y=opt_top[indx];	}
	else if (indx==TRIGG_EVS_MAX_ID)		{	x=182; y=230;	}
	else if (indx==(TRIGG_EVS_MAX_ID+1))	{	x=260; y=230;	}
	ILI9488_WriteString(x, y, "@", Font_11x18, &clr);
}

static void set_opt_val_sel(int indx , bool set, RgbStr clr)
{
	if (set)	//magenta box
	{	
		clr.color = Black;
		clr.bcolor = Magenta;
	}  
	ILI9488_WriteString(325, opt_top[indx], GetOptText(indx), Font_11x18, &clr);
}

/**
 * \brief rendering on the LCD.
 */
static void draw_static(void)
{
	opt_ind=0;
	
	ILI9488_FillRectangle(0, 32, ILI9488_WIDTH, 258,White);
	
	RgbStr clr = {RGB565(0,0x82,0),White};
	ILI9488_WriteString(140, 34, "Blast Trigger Options:", Font_11x18, &clr);	//title
	
	clr.color = Black;
	ILI9488_WriteString(200, 230, "OK     Cancel", Font_11x18, &clr);			//ok, cancel
	ILI9488_FillRectangle(0, 262, 480, 1,Black);
	ILI9488_FillRectangle(0, 263, 480, 27,RGB565(180,200,220));
	
	getDefValIndex();

	int i;
	for (i=0;i<TRIGG_EVS_MAX_ID;i++)
	{
		opt_top[i]=68+24*i;
		ILI9488_WriteString(30,  opt_top[i], opt_items[i], Font_11x18, &clr);
		ILI9488_WriteString(325, opt_top[i], GetOptText(i), Font_11x18, &clr);
	}
	
	set_opt_sel(opt_ind,1,clr);
	
	//notes
	clr.bcolor = RGB565(180,200,220);
	ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select   ", Font_11x18, &clr);
}


/* called from task_key  */
void trigger_evs_win_KEY_handler(uint32_t key_pad)
{
	static uint8_t 	vld_pressed = 0;
	
	switch (key_pad)
	{
		case UP: // up
			vld_pressed = 1;	break;
		case F2: // ok
			vld_pressed = 2;	break;
		case BOT: //	bottom
			vld_pressed = 3;	break;	
		case F5: //	bottom
			vld_pressed = 5;	break;
	}

	if (vld_pressed) {
		struct win_pointer_event win_touch_event;
		win_touch_event.buttons = vld_pressed;		
		vld_pressed = 0;
		osMessageQueuePut(queue_pointer_eventHandle,&win_touch_event, 0, 0);//xQueueSend(queue_pointer_event, &win_touch_event, 0);
	}
}

static void trigger_evs_exit()
{
	if (g_demo_parameters.nMeasMode ==0) //must be
	{
		set_special_mode_status(DEMO_TRIGGER_EVS, 0);
		app_widget_launch_evs();
		set_special_mode_status(DEMO_MODE_EVS, 1);
	}
}

/* only for task_lcd  */
static void widget_trig_evs_command_handler(struct win_pointer_event const *event)
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
				opt_ind=TRIGG_EVS_MAX_ID+1;
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
			opt_val_indx[opt_ind][1]++;
			if (opt_val_indx[opt_ind][1]>=opt_val_indx[opt_ind][0])
			opt_val_indx[opt_ind][1]=0;
			set_opt_val_sel(opt_ind,1,clr);
		}
	}
	
	//------------------ BOTTOM ---------------------------
	if (event->buttons == BOT) 
	{		
		if (!sub_opt_mode)	//moving across options line
		{
			if (opt_ind==(TRIGG_EVS_MAX_ID+1))
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
			opt_val_indx[opt_ind][1]--;
			if (opt_val_indx[opt_ind][1]<0)
			opt_val_indx[opt_ind][1]=opt_val_indx[opt_ind][0]-1;
			set_opt_val_sel(opt_ind,1,clr);
		}	
	}
	
	//------------------ OK ---------------------------
	if (event->buttons == F2) 
	{
		if (opt_ind<TRIGG_EVS_MAX_ID)											//options line
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
		else if (opt_ind==TRIGG_EVS_MAX_ID)									//ok ->save current options, back to main menu
		{
			g_demo_parameters.opt_trig[0].fLevel			=opt_Val_0[opt_val_indx[0][1]];
			g_demo_parameters.opt_trig[0].nUseSoundTrigger	=opt_Val_1[opt_val_indx[1][1]];
			g_demo_parameters.opt_trig[0].fLevelSoundTrigger=opt_Val_2[opt_val_indx[2][1]];
			g_demo_parameters.opt_trig[0].fPostTrigTime		=opt_Val_3[opt_val_indx[3][1]];
			g_demo_parameters.opt_trig[0].nDelay			=opt_Val_4[opt_val_indx[4][1]];
			memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t)); // FLASH
			trigger_evs_exit();
		}
		else if (opt_ind==(TRIGG_EVS_MAX_ID+1))								//cancel, just back to main menu
		{
			trigger_evs_exit();		
		}
	}
	
	//------------------ MENU ---------------------------
	if (event->buttons == F5) // menu
	{
		trigger_evs_exit();
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
void app_widget_launch_trigger_evs(void)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_trig_evs_command_handler;

	draw_static();
}
