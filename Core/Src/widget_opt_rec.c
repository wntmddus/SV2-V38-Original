/*
 * widget_opt_rec.c
 *
 * Created: 4/5/2023 3:39:40 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"

/*
//to parameters.h
extern void opt_rec_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_rec(void);
*/

#define OPT_REC_MAX_ID	3		//count of option's line in the screen

static int opt_ind/*=0*/;						//ind of current option

static const char* opt_items[] = {
	"Save:",
	"RecType:",
	"Use Timer:"
};

//see in parameters.h:
static uint16_t opt_Val_0[]={0,1,2};  //2025      {0};
static uint16_t opt_Val_1[]={0,1,2};  //2025      {0};
static int   opt_Val_2[]={0, 1, 2, 5, 10, 15, 30, 60,300,3600 };


static int opt_top[OPT_REC_MAX_ID];///
static int opt_val_indx[OPT_REC_MAX_ID][2];//second ind: [0]-count of value in opt_Val_x; [1] - index of value in opt_Val_x for every option;

static bool sub_opt_mode/*=0*/;	///			//0-moving across options line,  1-moving inside selected option



static const char* GetOptText(int line)
{
	const char* opt_text_0[]={"Wave File          ", "Result File        ", "Wave & Result Files"};  // {"Wave File          "};   //2025
	const char* opt_text_1[]={"Manual      ", "Single      ", "Continuous  "};                       //  {"Manual      "};			//2025
	const char* opt_text_2[]={ "  NO", " 1 s", " 2 s", " 5 s", "10 s", "15 s", "30 s", " 1 m", " 5 m", " 1 h"};
	
	switch (line)
	{
		case 0:	return	opt_text_0[	opt_val_indx[line][1]];	break;
		case 1:	return	opt_text_1[	opt_val_indx[line][1]];	break;
		case 2:	return	opt_text_2[	opt_val_indx[line][1]];	break;
		default:return	opt_text_0[0];
	}
}

static void getDefValIndex(void)
{
	int i;
	for (i=0;i<OPT_REC_MAX_ID;i++)	{	opt_val_indx[i][0]=0;	opt_val_indx[i][1]=0;	}
	
	opt_val_indx[0][0]=sizeof(opt_Val_0)/sizeof(uint16_t);
	for (i=0;i<opt_val_indx[0][0];i++)
	{	if (g_demo_parameters.opt_rec.file_type==opt_Val_0[i])	opt_val_indx[0][1]=i;	}
		
	opt_val_indx[1][0]=sizeof(opt_Val_1)/sizeof(uint16_t);
	for (i=0;i<opt_val_indx[1][0];i++)
	{	if (g_demo_parameters.opt_rec.nRecordMode==opt_Val_1[i])	opt_val_indx[1][1]=i;	}
	
	opt_val_indx[2][0]=sizeof(opt_Val_2)/sizeof(int);
	for (i=0;i<opt_val_indx[2][0];i++)
	{	if (g_demo_parameters.opt_rec.timer==opt_Val_2[i])	opt_val_indx[2][1]=i;	}
	
}

static void set_opt_sel(int indx , bool set, RgbStr clr)
{
	if (!set)	{		clr.color=clr.bcolor;	}
	
	int x=0, y=0;
	if (indx<OPT_REC_MAX_ID)			{	x=10; y=opt_top[indx];	}
	else if (indx==OPT_REC_MAX_ID)		{	x=182; y=230;	}
	else if (indx==(OPT_REC_MAX_ID+1))	{	x=260; y=230;	}
	ILI9488_WriteString(x, y, "@", Font_11x18, &clr);
}

static void set_opt_val_sel(int indx , bool set, RgbStr clr)
{
	if (set)	{	clr.color = Black;  
	clr.bcolor = RGB565(0xFF,0x96,0xFF);}  //magenta box
	ILI9488_WriteString(170, opt_top[indx], GetOptText(indx), Font_11x18, &clr);
}

static void record_exit(void)
{
	set_special_mode_status(DEMO_OPT_RECORD, 0);
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
void opt_rec_win_KEY_handler(uint32_t key_pad)
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

/* only for task_lcd  */
static void widget_opt_rec_command_handler(struct win_pointer_event const *event)
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
				opt_ind=OPT_REC_MAX_ID+1;
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
			if (opt_ind==(OPT_REC_MAX_ID+1))
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
		if (opt_ind<OPT_REC_MAX_ID)											//options line
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
		else if (opt_ind==OPT_REC_MAX_ID)									//ok ->save current options, back to main menu
		{
			g_demo_parameters.opt_rec.file_type	=opt_Val_0[opt_val_indx[0][1]];  //0;   //2025
			g_demo_parameters.opt_rec.nRecordMode=opt_Val_0[opt_val_indx[1][1]]; // 0;   //2025
			g_demo_parameters.opt_rec.timer		=opt_Val_2[opt_val_indx[2][1]];
			memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t)); // FLASH
			
			record_exit();			
			///set_rec_button(); //to hide REC-ERR  if Single-Cont modes was set
		}
		else if (opt_ind==(OPT_REC_MAX_ID+1))								//cancel, just back to main menu
		{
			record_exit();		
		}
	}
	
	//------------------ MENU ---------------------------
	if (event->buttons == F5) // menu
	{
		record_exit();
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
 * \brief rendering on the LCD.
 */
static void draw_static(void)
{
//	opt_ind=0;
	
	ILI9488_FillRectangle(0, 32, ILI9488_WIDTH, 258,White);
	
	RgbStr clr = {RGB565(0x0,0x82,0x0),White};
	ILI9488_WriteString(140, 34, "Recording Options:", Font_11x18, &clr);	//title
	
	clr.color = Black;
	ILI9488_WriteString(200, 230, "OK     Cancel", Font_11x18, &clr);			//ok, cancel
	ILI9488_FillRectangle(0, 262, 480, 1,Black);
	ILI9488_FillRectangle(0, 263, 480, 27,RGB565(180,200,220));
	
	getDefValIndex();

	int i;
	for (i=0;i<OPT_REC_MAX_ID;i++)
	{
		opt_top[i]=68+24*i;
		ILI9488_WriteString(30,  opt_top[i], opt_items[i], Font_11x18, &clr);
		ILI9488_WriteString(170, opt_top[i], GetOptText(i), Font_11x18, &clr);
	}
	ILI9488_WriteString(250, opt_top[2], "(for Manual Rec.)", Font_11x18, &clr);
	
	set_opt_sel(opt_ind,1,clr);
	
	//notes
	clr.bcolor =  RGB565(180,200,220);
	ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select   ", Font_11x18, &clr);
}


/**
 * \brief Widget start
 */
void app_widget_launch_opt_rec(void)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_opt_rec_command_handler;
	
	opt_ind=0;						//ind of current option
	sub_opt_mode=0;

	draw_static();
}
