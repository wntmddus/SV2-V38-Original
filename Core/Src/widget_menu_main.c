/*
 * widget_menu_main.c
 *
 * Created: 2/21/2023 11:06:45 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include <math.h>
#include "parameters.h"
#include "ili9488.h"

#define OPT_MENU_MAX_ID	6  //2025  8		//count of option's line in the screen

const char* menu_items[] = {
	"EVS / BLAST" 
//	,"Send To Server"  //2025
	,"Recording Options" 
	,"Trigger Options" 
	,"Analyzing Options" 
	,"Transducers Options" 
//	,"FTP Options"   //"About",  //2025
	,"Common"
};
static int opt_top[OPT_MENU_MAX_ID];///
int opt_ind_menu=0;	
bool inside_opt_evs_blast=false;

static void set_opt_sel_menu(int indx , bool set, RgbStr clr)
{
	if (!set)	{		clr.color=clr.bcolor;	}
	
	int x=0, y=0;
	if (indx<OPT_MENU_MAX_ID)			{	x=120; y=opt_top[indx];	}
	ILI9488_WriteString(x, y, "@", Font_11x18, &clr);
}

static void menu_indx0_sel(int sel, RgbStr clr) //sel=-1;  0;  1
{	
	switch (sel)
	{
		case -1: //nothing
		default:
			ILI9488_WriteString(140, opt_top[0], "EVS / BLAST", Font_11x18, &clr);
			ILI9488_WriteString(115, 266, "  ^ Up/Down  ok-Select ", Font_11x18, &clr);
			break;
			
		case 0://evs is selected
			ILI9488_WriteString(115, 266, " <> Switch   ok-Select ", Font_11x18, &clr);
			ILI9488_WriteString(140, opt_top[0], "    / BLAST", Font_11x18, &clr);
			clr.bcolor = RGB565(0xff,0x96,0xff);
			ILI9488_WriteString(140, opt_top[0], "EVS ", Font_11x18, &clr);
			break;
			
		case 1://blast is selected
			ILI9488_WriteString(115, 266, " <> Switch   ok-Select ", Font_11x18, &clr);
			ILI9488_WriteString(140, opt_top[0], "EVS /", Font_11x18, &clr);
			clr.bcolor = RGB565(0xff,0x96,0xff);
			ILI9488_WriteString(140+5*11, opt_top[0], " BLAST", Font_11x18, &clr);
			
			break;
	}
}

/* only for task_key  */
void menu_main_win_KEY_handler(uint32_t key_pad)
{
	static uint8_t 	vld_pressed = 0;
	
	switch (key_pad)
	{
		case UP: //up
			vld_pressed = 1;		break;
		case F2: //ok
			vld_pressed = 2;		break;
		case BOT: //bottom
			vld_pressed = 3;		break;
		case F6: //left
			vld_pressed = 6;		break;
		case F7: //right
			vld_pressed = 7;		break;
		case F5: // menu
			vld_pressed = 5;		break;
	}

	if (vld_pressed) {
		struct win_pointer_event win_touch_event;
		win_touch_event.buttons = vld_pressed;
		
		vld_pressed = 0;

		osMessageQueuePut(queue_pointer_eventHandle,&win_touch_event, 0, 0);//xQueueSend(queue_pointer_event, &win_touch_event, 0);
	}
}

static void main_menu_exit(void)
{
	if (g_demo_parameters.nMeasMode ==0)
	{
		set_special_mode_status(DEMO_MENU_MAIN, 0);
		app_widget_launch_evs();
		set_special_mode_status(DEMO_MODE_EVS, 1);
	}
	else if (g_demo_parameters.nMeasMode ==1)
	{
		set_special_mode_status(DEMO_MENU_MAIN, 0);
		app_widget_launch_blast();
		set_special_mode_status(DEMO_MODE_BLAST, 1);
	}
}

/**
 * \brief Move over widget (Frame handler for the LEQ widget.)
 */
/* only for task_lcd  */
static void widget_menu_main_command_handler(struct win_pointer_event const *event)
{
		RgbStr clr = {White,RGB565(0x80,0x80,0x80)};

	if (!inside_opt_evs_blast)
	{
		//------------------ TOP ---------------------------
		if (event->buttons == UP)
		{		
			if (opt_ind_menu==0)
			{
				set_opt_sel_menu(opt_ind_menu,0,clr);
				opt_ind_menu=OPT_MENU_MAX_ID-1;
				set_opt_sel_menu(opt_ind_menu,1,clr);
			}
			else
			{
				set_opt_sel_menu(opt_ind_menu,0,clr);
				opt_ind_menu--;
				set_opt_sel_menu(opt_ind_menu,1,clr);
			}
		}
	
		//------------------ BOTTOM ---------------------------
		if (event->buttons == BOT) 
		{	
			if (opt_ind_menu==(OPT_MENU_MAX_ID-1))
			{
				set_opt_sel_menu(opt_ind_menu,0,clr);
				opt_ind_menu=0;
				set_opt_sel_menu(opt_ind_menu,1,clr);
			}
			else
			{
				set_opt_sel_menu(opt_ind_menu,0,clr);
				opt_ind_menu++;
				set_opt_sel_menu(opt_ind_menu,1,clr);
			}
		}
		//------------------ EXIT MENU ---------------------------
		if (event->buttons == F5)
		{
			main_menu_exit();
			return;
		}
		
	}

	//------------------ RIGHT ---------------------------for EVS/BLAST only
	if (event->buttons == F7)
	{
		if (inside_opt_evs_blast)
		{
			g_demo_parameters.nMeasMode=!g_demo_parameters.nMeasMode;
			int sel=g_demo_parameters.nMeasMode;
			menu_indx0_sel(sel,clr);
		}
	}
	//------------------ LEFT ---------------------------for EVS/BLAST only
	if (event->buttons == F6)
	{
		if (inside_opt_evs_blast)
		{
			g_demo_parameters.nMeasMode=!g_demo_parameters.nMeasMode;
			int sel=g_demo_parameters.nMeasMode;
			menu_indx0_sel(sel,clr);
		}
	}
	
	//------------------ OK ---------------------------
	if (event->buttons == F2) 
	{
		switch (opt_ind_menu)
		{
			case 0:	//evs/blast
			{
				inside_opt_evs_blast=!inside_opt_evs_blast;
				if (inside_opt_evs_blast)
				{
					int sel=g_demo_parameters.nMeasMode;
					menu_indx0_sel(sel,clr);
				}
				else
				{  //options is selected and saved in g_demo_parameters.nMeasMode
					menu_indx0_sel(-1,clr);
					//to hide REC-ERR  if Single-Cont modes was set in EVS
					///set_rec_button(); //to hide REC-ERR  if Single-Cont modes was set in EVS					
					//and exit from menu
					main_menu_exit();
				}
			
				break;
			}
/*			case 1:	//send to server
			
			if ((g_demo_parameters.nMeasMode ==0 || g_demo_parameters.nMeasMode ==1))
			{
				set_special_mode_status(DEMO_MENU_MAIN, 0);
				app_widget_launch_send_file();
				set_special_mode_status(DEMO_SEND_FILE, 1);

			}*/
			break;
			
			//case 2:	//rec opt
			case 1:
			
				if (g_demo_parameters.nMeasMode ==0 || g_demo_parameters.nMeasMode ==1)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_opt_rec();
					set_special_mode_status(DEMO_OPT_RECORD, 1);
				}
				break;
			
			//case 3:	//trig
			case 2:
				if (g_demo_parameters.nMeasMode ==0)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_trigger_evs();
					set_special_mode_status(DEMO_TRIGGER_EVS, 1);
				}
				if (g_demo_parameters.nMeasMode ==1)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_trigger_bls();
					set_special_mode_status(DEMO_TRIGGER_BLAST, 1);
				}
				break;
			
			//case 4:	//analyz
			case 3:
				if (g_demo_parameters.nMeasMode ==0)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_opt_an_evs();
					set_special_mode_status(DEMO_OPT_ANALYZ_EVS, 1);
				}
				if (g_demo_parameters.nMeasMode ==1)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_opt_an_bls();
					set_special_mode_status(DEMO_OPT_ANALYZ_BLAST, 1);
				}			
				break;
			
			//case 5:	//transd
			case 4:
				if (g_demo_parameters.nMeasMode ==0 || g_demo_parameters.nMeasMode ==1)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_transd();
					set_special_mode_status(DEMO_OPT_TRANSD, 1);
				}
				break;				
			
	/*		case 6:	//ftp
				if (g_demo_parameters.nMeasMode ==0 || g_demo_parameters.nMeasMode ==1)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_opt_ftp();
					set_special_mode_status(DEMO_OPT_FTP, 1);
				}
				break;*/
			//case 7:	//common   //about
			case 5:
				if (g_demo_parameters.nMeasMode ==0 || g_demo_parameters.nMeasMode ==1)
				{
					set_special_mode_status(DEMO_MENU_MAIN, 0);
					app_widget_launch_opt_common();
					set_special_mode_status(DEMO_OPT_COMMON, 1);
				}
				break;
			
			case OPT_MENU_MAX_ID:  //exit
			default:
				main_menu_exit();		
				break;
		}
	}
	
}

/**
 * \brief rendering on the LCD.
 */
static void draw_menu_main_static(void)
{
	opt_ind_menu=0;
	
	ILI9488_FillRectangle(110, 32, 260, 258,RGB565(0x80,0x80,0x80)); 
	
	RgbStr clr = {White,RGB565(0x80,0x80,0x80)};
	ILI9488_WriteString(220, 36, "MENU:", Font_11x18, &clr);

	int i;
	for (i=0;i<OPT_MENU_MAX_ID;i++)
	{
		opt_top[i]=66+23*i;
		ILI9488_WriteString(140,  opt_top[i], menu_items[i], Font_11x18, &clr);
	}
	set_opt_sel_menu(opt_ind_menu,1,clr);
	
	ILI9488_FillRectangle(115, 262, 250, 1,White);
	ILI9488_WriteString(115, 266, "  ^ Up/Down  ok-Select ", Font_11x18, &clr);
}


/**
 * \brief Widget start
 */
void app_widget_launch_menu_main(void)
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_menu_main_command_handler;

	draw_menu_main_static();
}




