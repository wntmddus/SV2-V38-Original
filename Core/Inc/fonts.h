/* vim: set ai et ts=4 sw=4: */
#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>
typedef struct {
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;
//typedef struct {
//	const uint8_t width;
//	uint8_t height;
//	const uint8_t *data;
//	const uint32_t *descriptor;
//} FontDotDef;
typedef struct {
	uint8_t width;
	uint8_t height;
	uint16_t *data;
} PicDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;
//extern FontDotDef Font_22x35;

extern PicDef Bat_36x20_0;
extern PicDef Bat_36x20_1;
extern PicDef Bat_36x20_2;
extern PicDef Bat_36x20_3;
extern PicDef Bat_36x20_4;

extern PicDef SD_22x22_YES;
extern PicDef SD_22x22_NO;

extern void create_battery_icons(void);
extern void create_SD_icons(void);

#endif // __FONTS_H__
