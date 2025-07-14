/*
 * ILI9488.h
 *
 *  Created on: Oct 30, 2024
 *      Author: ryzen
 */

#ifndef INC_ILI9488_H_
#define INC_ILI9488_H_

#include <stdint.h>
#include <stddef.h>
#include "fonts.h"

#define LCD_REG 0x60000000
#define LCD_DATA 0x60100000

#define LCD_XSIZE_TFT    480
#define LCD_YSIZE_TFT    320

/* LCD color */
#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

#define RGB565(r,g,b) ((r >> 3) << 11 | (g >> 2) << 5 | ( b >> 3))

typedef struct stcolor{
	uint16_t color;
	uint16_t bcolor;
} RgbStr;

// rotate left
#define ILI9488_WIDTH  480
#define ILI9488_HEIGHT 320
#define ILI9488_MADCTL_MV  0x20
#define ILI9488_MADCTL_BGR 0x08
#define ILI9488_ROTATION (ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR)


// ===========================================
// RESET
// ===========================================
//void ILI9488_SW_Reset(void);
void ILI9488_HW_Reset(void);

 void ILI9488_Init(void);
 void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
 void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, RgbStr *clr);
 //void ILI9488_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
 void ILI9488_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PicDef pic);


#endif /* INC_ILI9488_H_ */
