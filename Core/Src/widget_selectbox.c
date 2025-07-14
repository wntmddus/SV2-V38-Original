/*
 * widget_selectbox.c
 *
 * Created: 3/27/2023 6:17:30 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include "parameters.h"
#include "ili9488.h"
#include "printf.h"

/*
//to parameters.h
extern void selectbox_KEY_handler(uint32_t key_pad);
void app_widget_launch_selectbox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float val);
*/
static rect inbox_rect;
static float opt_val[]={1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f};
static int opt_val_indx[2];	///   //second ind: [0]-count of value in opt_Val; [1] - index of value in opt_Val;

void selectbox_KEY_handler(uint32_t key_pad)
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

static void set_opt_val_sel(bool set, RgbStr clr)
{
	if (set)		{	clr.color = White; clr.bcolor = RGB565(0xFF,0x96,0xFF);}  //magenta box
		
	char temp[16];
	float val=opt_val[opt_val_indx[1]];
	snprintf_(temp, 16, "%3.0f ",val	);
	
	ILI9488_WriteString(inbox_rect.x+Font_11x18.width , inbox_rect.y, temp, Font_11x18, &clr);
}

static void selectbox_command_handler(struct win_pointer_event const *event)
{
	RgbStr clr = {White,RGB565(0x80,0x80,0x80)};
	//------------------ TOP ---------------------------
	if (event->buttons == UP)
	{
		opt_val_indx[1]++;
		if (opt_val_indx[1]>=opt_val_indx[0])
		opt_val_indx[1]=0;
		set_opt_val_sel(1,clr);
	}
	
	//------------------ BOTTOM ---------------------------
	if (event->buttons == BOT)
	{
		opt_val_indx[1]--;
		if (opt_val_indx[1]<0)
		opt_val_indx[1]=opt_val_indx[0]-1;
		set_opt_val_sel(1,clr);
	}
	
	//------------------ OK ---------------------------
	if (event->buttons == F2)
	{
		if (get_special_mode_status(DEMO_SELECTBOX))
		{
			set_special_mode_status(DEMO_SELECTBOX, 0);
			app_widget_launch_transd_back(true,opt_val[opt_val_indx[1]]);//save
			set_special_mode_status(DEMO_OPT_TRANSD, 1);
		}
	}
	
	//------------------ CANCEL ---------------------------
	if (event->buttons == F5)
	{
		if (get_special_mode_status(DEMO_SELECTBOX))
		{
			set_special_mode_status(DEMO_SELECTBOX, 0);
			app_widget_launch_transd_back(false,0.0f);//don't save
			set_special_mode_status(DEMO_OPT_TRANSD, 1);
		}
	}
}

static void draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float val)
{
 	ILI9488_FillRectangle(x, y-5, w-1, h-1,RGB565(0x80,0x80,0x80));
	RgbStr clr = {White,RGB565(0x80,0x80,0x80)};
	
	set_opt_val_sel(1,clr);
}

/**
 * \brief Widget start
 */
void app_widget_launch_selectbox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float val)
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = selectbox_command_handler;

	inbox_rect.x=x;
	inbox_rect.y=y;
	inbox_rect.w=w;
	inbox_rect.h=h;
	
	opt_val_indx[0]=sizeof(opt_val)/sizeof(float);
	opt_val_indx[1]=0;
	for (int i=0;i<opt_val_indx[0];i++)
	{	if (val==opt_val[i])	opt_val_indx[1]=i;	}
	
	draw(x,y,w,h,val);
}
