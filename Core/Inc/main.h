/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define F1_Pin GPIO_PIN_4
#define F1_GPIO_Port GPIOE
#define F2_Pin GPIO_PIN_5
#define F2_GPIO_Port GPIOE
#define F4_Pin GPIO_PIN_6
#define F4_GPIO_Port GPIOE
#define F5_Pin GPIO_PIN_13
#define F5_GPIO_Port GPIOC
#define RIGHT_Pin GPIO_PIN_1
#define RIGHT_GPIO_Port GPIOH
#define INT_KEY_PWR_Pin GPIO_PIN_1
#define INT_KEY_PWR_GPIO_Port GPIOC
#define GAIN1_0_Pin GPIO_PIN_2
#define GAIN1_0_GPIO_Port GPIOC
#define GAIN1_1_Pin GPIO_PIN_3
#define GAIN1_1_GPIO_Port GPIOC
#define NSSSPI1_Pin GPIO_PIN_4
#define NSSSPI1_GPIO_Port GPIOA
#define UP_Pin GPIO_PIN_5
#define UP_GPIO_Port GPIOA
#define ENT_Pin GPIO_PIN_6
#define ENT_GPIO_Port GPIOA
#define LEFT_Pin GPIO_PIN_7
#define LEFT_GPIO_Port GPIOA
#define DOWN_Pin GPIO_PIN_4
#define DOWN_GPIO_Port GPIOC
#define AN_POWER_DOWN_Pin GPIO_PIN_5
#define AN_POWER_DOWN_GPIO_Port GPIOC
#define SD_DETECT_Pin GPIO_PIN_0
#define SD_DETECT_GPIO_Port GPIOB
#define ARM_PWR_Pin GPIO_PIN_1
#define ARM_PWR_GPIO_Port GPIOB
#define LED0_Pin GPIO_PIN_10
#define LED0_GPIO_Port GPIOA
#define REC_Pin GPIO_PIN_3
#define REC_GPIO_Port GPIOD
#define LCD_RST_Pin GPIO_PIN_6
#define LCD_RST_GPIO_Port GPIOD
#define DRDY_Pin GPIO_PIN_7
#define DRDY_GPIO_Port GPIOB
#define LCD_BL_Pin GPIO_PIN_1
#define LCD_BL_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
