/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */
extern uint8_t isInitialized;
extern RTC_HandleTypeDef hrtc;
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  if(retSD == 0)
  {
	  if (isInitialized == 0)
	  {
		  BSP_SD_Init();

		  if(BSP_SD_IsDetected())
		  {
			  isInitialized = 1;
		  }
	  }
  }
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
	RTC_TimeTypeDef sTimeFt = {0};
	RTC_DateTypeDef sDateFt = {0};
	DWORD stimeFt;
	HAL_RTC_GetTime(&hrtc, &sTimeFt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDateFt, RTC_FORMAT_BIN);
	//2000 - 1980 = 20
	stimeFt = (((DWORD)sDateFt.Year + 20)<<25)
	| (((DWORD)sDateFt.Month)<<21)
	| (((DWORD)sDateFt.Date)<<16)
	| (((DWORD)sTimeFt.Hours)<<11)
	| (((DWORD)sTimeFt.Minutes)<<5)
	| (((DWORD)sTimeFt.Seconds)>>1);
	return stimeFt;


  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */
