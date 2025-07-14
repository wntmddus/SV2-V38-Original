/*
 * widget_ftp.c
 *
 * Created: 4/10/2023 7:37:52 PM
  *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"

/*
//to parameters.h
extern void opt_ftp_win_KEY_handler(uint32_t key_pad);
void app_widget_launch_opt_ftp(void);
*/
#define OPT_FTP_MAX_ID	4		//count of option's line in the screen

static const char* opt_items[] = {
	"      IP:",
	"    Port:",
	"UserName:",
	"Password:"
};

opt_ftp_connect	opt_ftp_temp;

static int opt_ind;						//ind of current option
static int opt_top[OPT_FTP_MAX_ID];
static char out_buffer[32];


static void set_opt_sel(int indx , bool set, RgbStr clr)
{
	if (!set)	{		clr.color=clr.bcolor;	}
	
	int x=0, y=0;
	if (indx<OPT_FTP_MAX_ID)			{	x=10; y=opt_top[indx];	}
	else if (indx==OPT_FTP_MAX_ID)		{	x=182; y=230;	}
	else if (indx==(OPT_FTP_MAX_ID+1))	{	x=260; y=230;	}
	ILI9488_WriteString(x, y, "@", Font_11x18, &clr);
}

static const char* GetOptText(int line)
{	
	switch (line)
	{
		case 0:	snprintf_(out_buffer, 32, "%3d.%3d.%3d.%3d", opt_ftp_temp.ip[0], opt_ftp_temp.ip[1], opt_ftp_temp.ip[2], opt_ftp_temp.ip[3]);
				return out_buffer;
				break;
		case 1:	snprintf_(out_buffer, 32, "%d", opt_ftp_temp.port);
				return out_buffer;
				break;
		case 2:	return	opt_ftp_temp.user_name;	break;
		case 3:	return	opt_ftp_temp.password;	break;
		//case 4:	return	opt_ftp_temp.path;	break;
		default:return	"---";
	}
}



/* called from task_key  */
void opt_ftp_win_KEY_handler(uint32_t key_pad)
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

static void ftp_exit(void)
{
	set_special_mode_status(DEMO_OPT_FTP, 0);
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

/* only for task_lcd  */
static void widget_opt_ftp_command_handler(struct win_pointer_event const *event)
{
	RgbStr clr = {Black, White};	
	
	//------------------ TOP ---------------------------
	if (event->buttons == UP)
	{
//		if (!sub_opt_mode)	//moving across options line
//		{			
			if (opt_ind==0)
			{
				set_opt_sel(opt_ind,0,clr);
				opt_ind=OPT_FTP_MAX_ID+1;
				set_opt_sel(opt_ind,1,clr);
			}
			else
			{
				set_opt_sel(opt_ind,0,clr);
				opt_ind--;
				set_opt_sel(opt_ind,1,clr);
			}
		//}
		//else //moving inside selected option
		//{
			//opt_val_indx[opt_ind][1]--;
			//if (opt_val_indx[opt_ind][1]<0)
			//opt_val_indx[opt_ind][1]=opt_val_indx[opt_ind][0]-1;
			//set_opt_val_sel(opt_ind,1,clr);
		//}
	}
	
	//------------------ BOTTOM ---------------------------
	if (event->buttons == BOT) 
	{	
/*		if (!sub_opt_mode)	//moving across options line
		{*/
			if (opt_ind==(OPT_FTP_MAX_ID+1))
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
/*		}	
		else //moving inside selected option
		{
			opt_val_indx[opt_ind][1]++;
			if (opt_val_indx[opt_ind][1]>=opt_val_indx[opt_ind][0])
			opt_val_indx[opt_ind][1]=0;
			set_opt_val_sel(opt_ind,1,clr);
		}	*/
	}
	
	//------------------ OK ---------------------------
	if (event->buttons == F2) 
	{
		if (opt_ind<OPT_FTP_MAX_ID)											//options line
		{
			//notes
			RgbStr clr2 = {Black,RGB565(180,200,220)};
			ILI9488_WriteString(10, 266, "<>Position  ^Digit  ok-Save  F5-Cancel", Font_11x18, &clr2);
			
			if (opt_ind==0)  //ip-address
			{
				//snprintf_(temp, 32, "%3d.%3d.%3d.%3d",	opt_ftp_temp.ip[0],	opt_ftp_temp.ip[1],	opt_ftp_temp.ip[2],	opt_ftp_temp.ip[3]);
				set_special_mode_status(DEMO_OPT_FTP, 0);
				app_widget_launch_inputbox(DEMO_OPT_FTP, 146,  opt_top[opt_ind],15*Font_11x18.width,24, GetOptText(opt_ind),15,0);
				set_special_mode_status(DEMO_INPUTBOX, 1);
			}
			
			if (opt_ind==1)  //port
			{				
				//snprintf_(temp, 16, "% 6d",	opt_ftp_temp.port);
				set_special_mode_status(DEMO_OPT_FTP, 0);
				app_widget_launch_inputbox(DEMO_OPT_FTP, 146,  opt_top[opt_ind],6*Font_11x18.width,24, GetOptText(opt_ind),6,0);
				set_special_mode_status(DEMO_INPUTBOX, 1);
			}
			
			if (opt_ind==2)  //username
			{
				//snprintf_(temp, 32, "%s",	opt_ftp_temp.user_name);
				set_special_mode_status(DEMO_OPT_FTP, 0);
				app_widget_launch_inputbox(DEMO_OPT_FTP, 146,  opt_top[opt_ind],30*Font_11x18.width,24, GetOptText(opt_ind),30,1);
				set_special_mode_status(DEMO_INPUTBOX, 1);
			}
			
			if (opt_ind==3)  //password
			{
				//snprintf_(temp, 32, "%s",	opt_ftp_temp.password);
				set_special_mode_status(DEMO_OPT_FTP, 0);
				app_widget_launch_inputbox(DEMO_OPT_FTP, 146,  opt_top[opt_ind],30*Font_11x18.width,24, GetOptText(opt_ind),30,1);
				set_special_mode_status(DEMO_INPUTBOX, 1);
			}
			
		/*	sub_opt_mode=!sub_opt_mode;
			set_opt_val_sel(opt_ind , sub_opt_mode, clr);
			
			if (sub_opt_mode)	//notes
			{				
				clr.br=180;	clr.bg=200;	clr.bb=220;
				ILI9488_WriteString(10, 266, "^ Prev/Next  ok-Confirm", Font_11x18, &clr);
			}
			else
			{
				clr.br=180;	clr.bg=200;	clr.bb=220;
				ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select   ", Font_11x18, &clr);
			}*/
		}
		else if (opt_ind==OPT_FTP_MAX_ID)									//ok ->save current options, back to main menu
		{
			for (int k=0;k<4;k++) g_demo_parameters.opt_ftp.ip[k] =opt_ftp_temp.ip[k];
			g_demo_parameters.opt_ftp.port=opt_ftp_temp.port;
			snprintf_(g_demo_parameters.opt_ftp.user_name, 32, "%s",	opt_ftp_temp.user_name);
			snprintf_(g_demo_parameters.opt_ftp.password , 32, "%s",	opt_ftp_temp.password );
			memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t)); // FLASH
			
			ftp_exit();
		}
		else if (opt_ind==(OPT_FTP_MAX_ID+1))								//cancel, just back to main menu
		{
			ftp_exit();		
		}
	}
	
	//------------------ MENU ---------------------------
	if (event->buttons == F5) // menu
	{
		ftp_exit();
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
	
	RgbStr clr = {RGB565(0,0x82,0),White};
	ILI9488_WriteString(140, 34, "FTP Options:", Font_11x18, &clr);	//title
	
	clr.color = Black;
	ILI9488_WriteString(200, 230, "OK     Cancel", Font_11x18, &clr);			//ok, cancel
	ILI9488_FillRectangle(0, 262, 480, 1,Black);
	ILI9488_FillRectangle(0, 263, 480, 27,RGB565(180,200,220));
	
	//getDefValIndex();

	int i;
	for (i=0;i<OPT_FTP_MAX_ID;i++)
	{
		opt_top[i]=68+24*i;
		ILI9488_WriteString(30,  opt_top[i], opt_items[i], Font_11x18, &clr);
		ILI9488_WriteString(146, opt_top[i], GetOptText(i), Font_11x18, &clr);
	}
	
	char tmp[64];
	ILI9488_WriteString(30,  opt_top[OPT_FTP_MAX_ID-1]+24, "    Path:", Font_11x18, &clr);   
	snprintf_(tmp, 64, "%s\\%s",opt_ftp_temp.path,	opt_ftp_temp.user_name);
	ILI9488_WriteString(146, opt_top[OPT_FTP_MAX_ID-1]+24, tmp, Font_11x18, &clr);
	
	set_opt_sel(opt_ind,1,clr);
	
	//notes
	clr.bcolor = RGB565(180,200,220);
	ILI9488_WriteString(10, 266,         "^ Up/Down  ok-Select                  ", Font_11x18, &clr);
}


/**
 * \brief Widget start
 */
void app_widget_launch_opt_ftp(void)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_opt_ftp_command_handler;
	
	memcpy(&opt_ftp_temp,   &g_demo_parameters.opt_ftp, sizeof(opt_ftp_connect));
	
	opt_ind=0;						//ind of current option
	//sub_opt_mode=0;

	draw_static();
}

static void set_text(int line)
{
	int x= 146;
	int y= opt_top[line];
	int w= Font_11x18.width;
	int h= 24;
	
	
	
/*	char temp[16];*/
	switch (line)
	{
		case 0:		w=w*15;	break;
		
		case 1:		w=w*6;	break;
		
		case 2:		w=w*30;	break;
		
		case 3:		w=w*30;	break;
		
	}
	
	ILI9488_FillRectangle(x, y-5, w, h,White);
	RgbStr clr = {Black, White};
	
	ILI9488_WriteString(x ,y, GetOptText(opt_ind), Font_11x18, &clr);
}

void app_widget_launch_ftp_back(bool save, float out_val, const char* str)	//to parameters.h
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = widget_opt_ftp_command_handler;
	
	//notes
	RgbStr clr = {Black,RGB565(180,200,220)};
	ILI9488_WriteString(10, 266, "^ Up/Down  ok-Select                  ", Font_11x18, &clr);
	
	int dif;
	//new val
	if (save)
	{
		switch (opt_ind)
		{
			case 0:	//ip	
				{
					char temp[16];
					int n=strlen (str);
					int p=0,i=0,start=0;
					uint16_t val;
					while (i<=n && p<4)  // !!!   i<=n  - not mistake!!!!
					{
						if (str[i]=='.' || i==n)
						{
							temp[start]='\0';
							/*val= (uint16_t)atoi(temp);*/
							if (val>255) val=255;
							opt_ftp_temp.ip[p]=val;
							p++;
							start=0;
						}
						else
						{
							temp[start]=str[i];
							start++;
						}
						i++;
					}
					if (p<4) //not correct input by user
					{
						for (int j=p;j<4;j++) 
							opt_ftp_temp.ip[j]=0;
					}
				}
				break;
			
			case 1:	//port
				opt_ftp_temp.port		= out_val;		break;
			
			case 2:	//username
				
				dif=strlen(opt_ftp_temp.user_name)-strlen(str);
				snprintf_(opt_ftp_temp.user_name, 32, "%s",	str);				
				char tmp[64];
				snprintf_(tmp, 64, "%s\\%s",opt_ftp_temp.path,	opt_ftp_temp.user_name);
				if (dif>0)
				{ 
					for (int i=0;i<dif;i++)
						snprintf_(tmp, 64, "%s ",tmp);
				}
				clr.bcolor = White;
				ILI9488_WriteString(146, opt_top[OPT_FTP_MAX_ID-1]+24, tmp, Font_11x18, &clr);
				
				break;

			case 3:	//password
				snprintf_(opt_ftp_temp.password , 32, "%s",	str);	break;
		}
	}
	
	//update text
	set_text(opt_ind);
}

