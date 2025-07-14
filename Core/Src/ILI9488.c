/*
 * ILI9488.c
 *
 *  Created on: Oct 28, 2024
 *      Author: ryzen
 */
#include "ILI9488.h"
#include "main.h"
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include "parameters.h"

#include "string.h"

#define  SB (16384)
__attribute__((__aligned__(32))) uint16_t lcddata[SB];

extern MDMA_HandleTypeDef hmdma_mdma_channel0_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel1_sw_0;


// ===========================================
// LCD WRITE COMMAND
// ===========================================
void ILI9488_WriteCommand(unsigned char cmd)
{
	*(unsigned short *)(LCD_REG) = cmd;
}

// ===========================================
// LCD WRITE DATA
// ===========================================
void ILI9488_DataWrite (unsigned short data)
{
	*(unsigned short *)(LCD_DATA)= data;
};

void ILI9488_WriteDataBig(uint16_t* buff, size_t buff_size) {

//uint32_t status;

///	status = HAL_MDMA_Start_IT(&hmdma_mdma_channel0_sw_0,(uint32_t) buff, (uint32_t) 0x60100000, buff_size*2, 1);

}

void ILI9488_WriteDataSmall(size_t buff_size) {

uint32_t iz;

	if (buff_size <= SB)
	{
		for (iz=0; iz < buff_size; iz++)
		{
			ILI9488_DataWrite(lcddata[iz]);
		}
	}
}

//status = HAL_MDMA_Start_IT(&hmdma_mdma_channel40_sw_0,(uint32_t) &pixel, (uint32_t) 0x60100000, 2*2, 1);

void ILI9488_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	// column address set
	ILI9488_WriteCommand(0x2A); // CASET
	{
		ILI9488_DataWrite((x0 >> 8) & 0xFF);
		ILI9488_DataWrite(x0 & 0xFF);
		ILI9488_DataWrite((x1 >> 8) & 0xFF);
		ILI9488_DataWrite(x1 & 0xFF);
	}
	// row address set
	ILI9488_WriteCommand(0x2B); // RASET
	{
		ILI9488_DataWrite((y0 >> 8) & 0xFF);
		ILI9488_DataWrite(y0 & 0xFF);
		ILI9488_DataWrite((y1 >> 8) & 0xFF);
		ILI9488_DataWrite(y1 & 0xFF);
	}
	// write to RAM
	ILI9488_WriteCommand(0x2C); // RAMWR
}
void ILI9488_Reset(void)
{
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 0); //LCD_RST = 0;
	osDelay(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, 1); //LCD_RST = 1;
	osDelay(100);
}

void ILI9488_Init(void) {
	ILI9488_Reset();

	// command list is based on https://github.com/martnak/STM32-ILI9488

	ILI9488_WriteCommand(0x01);

		ILI9488_WriteCommand(0xE0);
		{
			uint8_t data[] = { 0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F };
			memcpy(lcddata, data, sizeof(data));
			ILI9488_WriteDataSmall(sizeof(data));
		}


		ILI9488_WriteCommand(0xE1);
		{
			uint8_t data[] = { 0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F };
			memcpy(lcddata, data, sizeof(data));
			ILI9488_WriteDataSmall(sizeof(data));
		}
  
		ILI9488_WriteCommand(0xC0);
		{
			ILI9488_DataWrite(0x17);
			ILI9488_DataWrite(0x15);
		}
  
		ILI9488_WriteCommand(0xC1);
		{
			ILI9488_DataWrite(0x41);
		}
 
		ILI9488_WriteCommand(0xC5);
		{
			ILI9488_DataWrite(0x00);
			ILI9488_DataWrite(0x12);
			ILI9488_DataWrite(0x80);
			ILI9488_DataWrite(0x15);
		}

uint8_t madctl = 0xE8; //ILI9488_ROTATION; //0x28 rotate left, 0x48 default orientation

		ILI9488_WriteCommand(0x36);
		{
			ILI9488_DataWrite(madctl);
		}

 		ILI9488_WriteCommand(0x3A);
		{
			ILI9488_DataWrite(0x55);//16BIT/PIXEL
		}
 
		ILI9488_WriteCommand(0xB0);
		{
			ILI9488_DataWrite(0);
		}
  
		ILI9488_WriteCommand(0xB1);
		{
			ILI9488_DataWrite(0XA0);
		}
 
		ILI9488_WriteCommand(0xB4);
		{
			ILI9488_DataWrite(2);
		}
  
		ILI9488_WriteCommand(0xB6);
		{
			ILI9488_DataWrite(2);
			ILI9488_DataWrite(2);
		}
 
		ILI9488_WriteCommand(0xE9);
		{
			ILI9488_DataWrite(0);
		}
 
		ILI9488_WriteCommand(0xF7);
		{
			ILI9488_DataWrite(0xA9);
			ILI9488_DataWrite(0x51);
			ILI9488_DataWrite(0x2C);
			ILI9488_DataWrite(0x82);
		}

		ILI9488_WriteCommand(0x11);
		osDelay(120);

		ILI9488_WriteCommand(0x29);
		osDelay(10);

}

void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	uint32_t status; //, i;
	if((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
	if((x + w - 1) >= ILI9488_WIDTH) w = ILI9488_WIDTH - x;
	if((y + h - 1) >= ILI9488_HEIGHT) h = ILI9488_HEIGHT - y;

	ILI9488_SetAddressWindow(x, y, x+w-1, y+h-1);
//	for (i=0; i < w*h; i++)
//	{
//		ILI9488_DataWrite(color);
//	}
	status = HAL_MDMA_Start_IT(&hmdma_mdma_channel1_sw_0,(uint32_t) &color, (uint32_t) 0x60100000, w*2, h);
	// if (notification_semaphore == NULL) {
		 if (fmc_write_notification_semaphoreHandle !=	NULL) {
			 /* Wait until notified by the ISR that transmission is complete. */
			 //HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 0);
			 if (osSemaphoreAcquire(fmc_write_notification_semaphoreHandle,	 1000) == osOK) {
				 //return_value = ERR_TIMEOUT;
			 }
		 }
	// }


}

void ILI9488_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PicDef pic) {
	uint32_t status;

	if((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
	if((x + w - 1) >= ILI9488_WIDTH) return;
	if((y + h - 1) >= ILI9488_HEIGHT) return;

	ILI9488_SetAddressWindow(x, y, x+w-1, y+h-1);
//	for (i=0; i < w*h; i++)
//	{
//		ILI9488_DataWrite(pic.data[i]);
//	}
	status = HAL_MDMA_Start_IT(&hmdma_mdma_channel0_sw_0,(uint32_t) pic.data, (uint32_t) 0x60100000, w*2, h);
	// if (notification_semaphore == NULL) {
		 if (fmc_write_notification_semaphoreHandle !=	NULL) {
			 /* Wait until notified by the ISR that transmission is complete. */
			 //HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 0);
			 if (osSemaphoreAcquire(fmc_write_notification_semaphoreHandle,	 1000) == osOK) {

				 //return_value = ERR_TIMEOUT;
			 }
			// //ioport_toggle_pin_level(LED1_GPIO);
		 }
	// }
}

void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, RgbStr *clr) {
uint32_t i, j, b, kt, xi, sl, ktxi, kti;
uint32_t status;
	xi=0; kt=0;
	sl = strlen(str);
	if(x + sl*font.width >= ILI9488_WIDTH) {
		return;
	}
	if(y + font.height >= ILI9488_HEIGHT) {
		return;
	}
	while(*str) {
		ktxi = xi*font.width;
		for(i = 0; i < font.height; i++) {
			b = font.data[(*str - 32) * font.height + i];
			kti = i*font.width*sl + ktxi;
			for(j = 0; j < font.width; j++) {
				kt = kti+(j);
				if (kt+2 > SB )
				{
					return;
				}
				if((b << j) & 0x8000)  {
					lcddata[kt] = clr->color;
				} else {
					lcddata[kt] = clr->bcolor;
				}

			}
		}
		str++;
		xi++;
	}
	ILI9488_SetAddressWindow(x, y, xi*(font.width)+x-1, y+font.height-1);
//	ILI9488_WriteDataSmall(font.width*font.height*xi);
	status = HAL_MDMA_Start_IT(&hmdma_mdma_channel0_sw_0,(uint32_t) lcddata, (uint32_t) 0x60100000, font.width*font.height*xi, 2);
	// if (notification_semaphore == NULL) {
		 if (fmc_write_notification_semaphoreHandle !=	NULL) {
			 /* Wait until notified by the ISR that transmission is complete. */
			 //HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 0);
			 if (osSemaphoreAcquire(fmc_write_notification_semaphoreHandle,	 1000) == osOK) {

				 //return_value = ERR_TIMEOUT;
			 }
			// //ioport_toggle_pin_level(LED1_GPIO);
		 }
	// }

}




