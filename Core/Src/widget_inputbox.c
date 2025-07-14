/*
 * widget_inputbox.c
 *
 * Created: 3/21/2023 3:53:38 PM
 *  Author: Tatsiana Katovich
 */ 

#include <string.h>
#include <math.h>
#include "parameters.h"
#include "ili9488.h"

/*
//to parameters.h
extern void inputbox_KEY_handler(uint32_t key_pad);
void app_widget_launch_inputbox(demo_special_mode_id id,uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* str, int max_pos_n, int type);
*/
static int  POS_INDX_MAX=6;  //max count of chars in the input box, input parameter

static char m_chars[68]={ ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};  //the list for selection						//for digit type
static int  DIGIT_INDX_MAX=12;															//count of chars in the list for selection		//for digit type

static int m_pos_indx=0;
static int m_digit_indx=0;
static rect inbox_rect;
static char input_str[32];  //POS_INDX_MAX
static demo_special_mode_id m_id;


/* only for task_key  */
void inputbox_KEY_handler(uint32_t key_pad)
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

static void set_position(int indx , bool set)
{
	RgbStr clr = {White,RGB565(0x80,0x80,0x80)};
		
	if (set)	{	clr.bcolor = RGB565(0xff,0x96,0xff);	}
		
	char temp[2];
	temp[0]=input_str[indx];
	temp[1]='\0';	
		
	ILI9488_WriteString(inbox_rect.x+indx*Font_11x18.width, inbox_rect.y, temp, Font_11x18, &clr);
	
	if (set) //find digit index in the list of selection
	{
		/*if (temp[0]>=48 && temp[0]<=57)  //'0','1',....'9'
			m_digit_indx=temp[0]-48+1;
		else if (temp[0]==46)             //'.'
			m_digit_indx=10+1;
		else m_digit_indx=0;		      //' '
		*/
		
		m_digit_indx=0;
		for (int i=0;i<DIGIT_INDX_MAX;i++)
			if (temp[0]==m_chars[i])
				m_digit_indx=i;
	}
}

static void set_digit(int indx)
{
	RgbStr clr = {White, RGB565(0xff,0x96,0xff)};

	char temp[2];
	temp[0]=input_str[indx];
	temp[1]='\0';
	
	ILI9488_WriteString(inbox_rect.x+indx*Font_11x18.width, inbox_rect.y, temp, Font_11x18, &clr);
}

//left&right trim if input_str
static void trim_string(void)
{
	//trim string : left side
	int j=0,len=POS_INDX_MAX,i=0;
	for (i=0;i<POS_INDX_MAX;i++)
	{
		if (input_str[i]==' ')		j++;
		else i=POS_INDX_MAX;//stop
	}
	len=len-j;
	
	if (j>0)
	{
		for (i=0;i<len;i++)
			input_str[i]=input_str[i+j];
	}
	//trim string : right side
	len=strlen(input_str);
	j=0;
	for (i=len-1;i>=0;i--)
	{
		if (input_str[i]==' ')
		j++;
		else i=-1;//stop
	}
	len=len-j;
	input_str[len]='\0';
	for (i=len;i<POS_INDX_MAX;i++)
	input_str[i]=0;	
}

static void inputbox_command_handler(struct win_pointer_event const *event)
{
	//float val=0.0;
	//------------------ TOP ---------------------------
	if (event->buttons == UP)
	{		
		if (m_digit_indx==DIGIT_INDX_MAX-1)
			m_digit_indx=0;
		else
			m_digit_indx++;
		input_str[m_pos_indx]=m_chars[m_digit_indx];
		set_digit(m_pos_indx);
	}
	
	//------------------ BOTTOM ---------------------------
	if (event->buttons == BOT) 
	{	
		if (m_digit_indx==0)
			m_digit_indx=DIGIT_INDX_MAX-1;
		else
			m_digit_indx--;
		input_str[m_pos_indx]=m_chars[m_digit_indx];
		set_digit(m_pos_indx);
	}

	//------------------ RIGHT --------------------------- 
	if (event->buttons == F7)
	{
		set_position(m_pos_indx,0);
		if (m_pos_indx==(POS_INDX_MAX-1))
			m_pos_indx=0;
		else
			m_pos_indx++;
		set_position(m_pos_indx,1);
	}
	//------------------ LEFT --------------------------- 
	if (event->buttons == F6)
	{
		set_position(m_pos_indx,0);
		if (m_pos_indx==0)
			m_pos_indx=POS_INDX_MAX-1;
		else
			m_pos_indx--;
		set_position(m_pos_indx,1);
	}
	
	//------------------ CANCEL ---------------------------
	if (event->buttons == F5) // menu
	{		
		if (get_special_mode_status(DEMO_INPUTBOX))
		{
			set_special_mode_status(DEMO_INPUTBOX, 0);
			
			switch (m_id)
			{
				case DEMO_OPT_TRANSD:	app_widget_launch_transd_back(false, 0.0f);  break;
				case DEMO_OPT_FTP:		app_widget_launch_ftp_back(false, 0.0f, NULL);  break;
				case DEMO_OPT_COMMON:	app_widget_launch_common_back(false, 0.0f, NULL);  break;
				default: break;
			}
			set_special_mode_status(m_id, 1);
		}
	}
	//------------------ OK - SAVE ------------------------
	if (event->buttons == F2) 
	{
		if (get_special_mode_status(DEMO_INPUTBOX))
		{
			trim_string();
				
			//save
			float val=(float)atof(input_str);
			
			if (val==0.0f) val=1.0f;   //normally no any parameter can be zero (to prevent zero devideing in calculation). *can be changed by request
			//
			set_special_mode_status(DEMO_INPUTBOX, 0);
			
			switch (m_id)
			{
				case DEMO_OPT_TRANSD:	app_widget_launch_transd_back(true, val);  break;
				case DEMO_OPT_FTP:		app_widget_launch_ftp_back(true, val, input_str);  break;
				case DEMO_OPT_COMMON:	app_widget_launch_common_back(true, val, input_str);  break;
				default: break;
			}
			set_special_mode_status(m_id , 1);
		}
	}
	
}

/**
 * \brief rendering on the LCD.
 */
static void draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* str)
{	
	ILI9488_FillRectangle(x, y-5, w-1, h-1,RGB565(0x80,0x80,0x80));
	RgbStr clr = {White, RGB565(0x80,0x80,0x80)};
	ILI9488_WriteString(x, y, str, Font_11x18, &clr);
	
	m_pos_indx=0;
	set_position(m_pos_indx,1);
}


/**
 * \brief Widget start
 */
void app_widget_launch_inputbox(demo_special_mode_id id,uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* str, int max_pos_n, int type) //type=0 - digits,  =1-text
{
	struct win_attributes *attributes;

	attributes = &win_attributes_root;
	attributes->event_handler = inputbox_command_handler;
	
	m_id=id;

	inbox_rect.x=x;
	inbox_rect.y=y;
	inbox_rect.w=w;
	inbox_rect.h=h;
		
	if (max_pos_n>32)
		max_pos_n=32;
	POS_INDX_MAX=max_pos_n;
	
	int len=strlen(str);
	int j=0;
	if (len>POS_INDX_MAX)  //left trim
	{
		for (int i=0;i<len;i++)
		{
			if (str[i]==' ')		
				j++;
			else i=len;//stop
		}
	}
	len=len-j;
	
	if (len>POS_INDX_MAX)
		len=POS_INDX_MAX;
	
	int i;
	for (i=0;i<len;i++)
		input_str[i]=str[i+j];
		
	for (i=len;i<POS_INDX_MAX;i++) //fill by ' ' till and of line and move 'end of the string' to the end of new line
		input_str[i]=' ';
	input_str[i]='\0';
		
	draw(x,y,w,h,input_str);
	
	//fill selection list for text type
	if (type==1)
	{
		int k=0;
		//int i;
		for (i=0; i<26; i++)
		{
			m_chars[2*i]	=	0x61+i;  //a,.....
			m_chars[2*i+1]	=	0x41+i;	 //A,.....
			k=k+2;
		}
		for (i=0; i<10; i++)  //0-9
			m_chars[k+i]=0x30+i;
		k=k+i;
		m_chars[k]='.'; k++;//'.'
		m_chars[k]='_'; k++;//'_' 
		m_chars[k]=' '; k++;//' '
		DIGIT_INDX_MAX=k;  //already 65
	}
}
