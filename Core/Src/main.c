/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
//#include "ILI9488.h"
#include "parameters.h"
#include <string.h>
//#include "ad7124.h"
//#include "ad7124_regs.h"
//#include "printf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c2;
DMA_HandleTypeDef hdma_i2c2_rx;

LPTIM_HandleTypeDef hlptim1;

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi2_rx;

MDMA_HandleTypeDef hmdma_mdma_channel0_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel1_sw_0;
SRAM_HandleTypeDef hsram1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_lcd */
osThreadId_t Task_lcdHandle;
const osThreadAttr_t Task_lcd_attributes = {
  .name = "Task_lcd",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_key */
osThreadId_t Task_keyHandle;
const osThreadAttr_t Task_key_attributes = {
  .name = "Task_key",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_dsp */
osThreadId_t Task_dspHandle;
const osThreadAttr_t Task_dsp_attributes = {
  .name = "Task_dsp",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for queue_pointer_event */
osMessageQueueId_t queue_pointer_eventHandle;
const osMessageQueueAttr_t queue_pointer_event_attributes = {
  .name = "queue_pointer_event"
};
/* Definitions for ConnectionEvent */
osMessageQueueId_t ConnectionEventHandle;
const osMessageQueueAttr_t ConnectionEvent_attributes = {
  .name = "ConnectionEvent"
};
/* Definitions for fmc_write_notification_semaphore */
osSemaphoreId_t fmc_write_notification_semaphoreHandle;
const osSemaphoreAttr_t fmc_write_notification_semaphore_attributes = {
  .name = "fmc_write_notification_semaphore"
};
/* Definitions for ad7124_notification_semaphore */
osSemaphoreId_t ad7124_notification_semaphoreHandle;
const osSemaphoreAttr_t ad7124_notification_semaphore_attributes = {
  .name = "ad7124_notification_semaphore"
};
/* Definitions for ads127l11_notification_semaphore */
osSemaphoreId_t ads127l11_notification_semaphoreHandle;
const osSemaphoreAttr_t ads127l11_notification_semaphore_attributes = {
  .name = "ads127l11_notification_semaphore"
};
/* Definitions for battery_i2c_read_notification_semaphore */
osSemaphoreId_t battery_i2c_read_notification_semaphoreHandle;
const osSemaphoreAttr_t battery_i2c_read_notification_semaphore_attributes = {
  .name = "battery_i2c_read_notification_semaphore"
};
/* USER CODE BEGIN PV */

extern void writetobackup( const void *data);
extern void readfrombackup(uint32_t wr_adress, uint8_t *readdata);

extern HAL_StatusTypeDef init_bq25883(void);
extern void bq2588x_update_status(bq2588x_t *bq);

uint8_t stk;
uint8_t text[16];
__IO uint32_t I2C_Rx_Complete_Flag = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_MDMA_Init(void);
static void MX_DMA_Init(void);
static void MX_FMC_Init(void);
static void MX_SPI1_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI2_Init(void);
static void MX_LPTIM1_Init(void);
static void MX_I2C2_Init(void);
void StartDefaultTask(void *argument);
extern void StartTask02(void *argument);
extern void StartTask03(void *argument);
extern void StartTask01(void *argument);

/* USER CODE BEGIN PFP */
static void TransferCompleteMDMA(MDMA_HandleTypeDef *hmdma);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	 HAL_DMA_MuxSyncConfigTypeDef dmamux_syncParams;
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  ///BSP_IO_Init(0, &init);
  //hmdma_mdma_channel0_sw_0.XferCpltCallback = TransferCompleteMDMA;
 // hmdma_mdma_channel0_sw_0.XferRepeatBlockCpltCallback = TransferCompleteMDMA;

//---------------------------------------

// --------------------------------------------------


  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_MDMA_Init();
  MX_DMA_Init();
  MX_FMC_Init();
  MX_SPI1_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_LPTIM1_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(ARM_PWR_GPIO_Port, ARM_PWR_Pin, 1);
  //HAL_GPIO_WritePin(GPIOC, LTE_POWERKEY_Pin, 0);

//------------------------------------------------
  //HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);

__HAL_RCC_BKPRAM_CLK_ENABLE();
HAL_PWREx_EnableBkUpReg();
//HAL_PWR_EnableBkUpAccess();
//*readdata =  *((uint32_t*)0x38800000);



 memcpy((void *)&g_demo_parameters1, (void const *)0x38800000, sizeof(demo_param_t));
 if(strcmp(g_demo_parameters.vers_sf, g_demo_parameters1.vers_sf))
 {
	 memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t)); // FLASH
 }

 if(g_demo_parameters1.magic == DEMO_PARAMETERS_MAGIC)
 {
	 //HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
	 memcpy((void *)&g_demo_parameters, (void const *)0x38800000, sizeof(demo_param_t));
 }
 if(g_demo_parameters.nMeasMode) g_demo_parameters.nMeasMode = 0;
// ----------------------------------------------


  HAL_GPIO_WritePin(GPIOC, AN_POWER_DOWN_Pin, 0);

  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);

  HAL_MDMA_RegisterCallback(&hmdma_mdma_channel0_sw_0,HAL_MDMA_XFER_CPLT_CB_ID,TransferCompleteMDMA);
  HAL_MDMA_RegisterCallback(&hmdma_mdma_channel1_sw_0,HAL_MDMA_XFER_CPLT_CB_ID,TransferCompleteMDMA);
  //HAL_I2C_RegisterCallback(&hi2c2,HAL_MDMA_XFER_CPLT_CB_ID,HAL_I2C_MemRxCpltCallback);
  //HAL_MDMA_RegisterCallback(&hmdma_mdma_channel1_sw_0,HAL_MDMA_XFER_CPLT_CB_ID,TransferCompleteMDMA);

//  bq2588x_t bqv;

 if( init_bq25883() == 0) HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);

// bq2588x_update_status(&bqv);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of fmc_write_notification_semaphore */
  fmc_write_notification_semaphoreHandle = osSemaphoreNew(1, 0, &fmc_write_notification_semaphore_attributes);

  /* creation of ad7124_notification_semaphore */
  ad7124_notification_semaphoreHandle = osSemaphoreNew(1, 0, &ad7124_notification_semaphore_attributes);

  /* creation of ads127l11_notification_semaphore */
  ads127l11_notification_semaphoreHandle = osSemaphoreNew(1, 0, &ads127l11_notification_semaphore_attributes);

  /* creation of battery_i2c_read_notification_semaphore */
  battery_i2c_read_notification_semaphoreHandle = osSemaphoreNew(1, 0, &battery_i2c_read_notification_semaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */

  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queue_pointer_event */
  queue_pointer_eventHandle = osMessageQueueNew (10, sizeof(win_pointer_event_t), &queue_pointer_event_attributes);

  /* creation of ConnectionEvent */
  ConnectionEventHandle = osMessageQueueNew (10, sizeof(uint16_t), &ConnectionEvent_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Task_lcd */
  Task_lcdHandle = osThreadNew(StartTask02, NULL, &Task_lcd_attributes);

  /* creation of Task_key */
  Task_keyHandle = osThreadNew(StartTask03, NULL, &Task_key_attributes);

  /* creation of Task_dsp */
  Task_dspHandle = osThreadNew(StartTask01, NULL, &Task_dsp_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 12;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_SPI2
                              |RCC_PERIPHCLK_SPI1;
  PeriphClkInitStruct.PLL2.PLL2M = 16;
  PeriphClkInitStruct.PLL2.PLL2N = 128;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x0090216E;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */


  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief LPTIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV2;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_ENDOFPERIOD;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2){
	  RTC_DateTypeDef  sdatestructure;
	  RTC_TimeTypeDef  stimestructure;

	  /*##-1- Configure the Date #################################################*/
	  /* Set Date: Tuesday February 2nd 2017 */
	  sdatestructure.Year = 0x17;
	  sdatestructure.Month = RTC_MONTH_FEBRUARY;
	  sdatestructure.Date = 0x2;
	  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;

	  if(HAL_RTC_SetDate(&hrtc,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
	  {
	    /* Initialization Error */
	    Error_Handler();
	  }

	  /*##-2- Configure the Time #################################################*/
	  /* Set Time: 02:20:00 */
	  stimestructure.Hours = 0x02;
	  stimestructure.Minutes = 0x20;
	  stimestructure.Seconds = 0x00;
	  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
	  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

	  if(HAL_RTC_SetTime(&hrtc,&stimestructure,RTC_FORMAT_BCD) != HAL_OK)
	  {
	    /* Initialization Error */
	    Error_Handler();
	  }
	  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
	}
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 5;
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 9, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
  /* DMAMUX1_OVR_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMAMUX1_OVR_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(DMAMUX1_OVR_IRQn);

}

/**
  * Enable MDMA controller clock
  * Configure MDMA for global transfers
  *   hmdma_mdma_channel0_sw_0
  *   hmdma_mdma_channel1_sw_0
  */
static void MX_MDMA_Init(void)
{

  /* MDMA controller clock enable */
  __HAL_RCC_MDMA_CLK_ENABLE();
  /* Local variables */

  /* Configure MDMA channel MDMA_Channel0 */
  /* Configure MDMA request hmdma_mdma_channel0_sw_0 on MDMA_Channel0 */
  hmdma_mdma_channel0_sw_0.Instance = MDMA_Channel0;
  hmdma_mdma_channel0_sw_0.Init.Request = MDMA_REQUEST_SW;
  hmdma_mdma_channel0_sw_0.Init.TransferTriggerMode = MDMA_REPEAT_BLOCK_TRANSFER;
  hmdma_mdma_channel0_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
  hmdma_mdma_channel0_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
  hmdma_mdma_channel0_sw_0.Init.SourceInc = MDMA_SRC_INC_HALFWORD;
  hmdma_mdma_channel0_sw_0.Init.DestinationInc = MDMA_DEST_INC_HALFWORD;
  hmdma_mdma_channel0_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_HALFWORD;
  hmdma_mdma_channel0_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_HALFWORD;
  hmdma_mdma_channel0_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
  hmdma_mdma_channel0_sw_0.Init.BufferTransferLength = 128;
  hmdma_mdma_channel0_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
  hmdma_mdma_channel0_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
  hmdma_mdma_channel0_sw_0.Init.SourceBlockAddressOffset = 0;
  hmdma_mdma_channel0_sw_0.Init.DestBlockAddressOffset = 0;
  if (HAL_MDMA_Init(&hmdma_mdma_channel0_sw_0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure MDMA channel MDMA_Channel1 */
  /* Configure MDMA request hmdma_mdma_channel1_sw_0 on MDMA_Channel1 */
  hmdma_mdma_channel1_sw_0.Instance = MDMA_Channel1;
  hmdma_mdma_channel1_sw_0.Init.Request = MDMA_REQUEST_SW;
  hmdma_mdma_channel1_sw_0.Init.TransferTriggerMode = MDMA_REPEAT_BLOCK_TRANSFER;
  hmdma_mdma_channel1_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
  hmdma_mdma_channel1_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
  hmdma_mdma_channel1_sw_0.Init.SourceInc = MDMA_SRC_INC_DISABLE;
  hmdma_mdma_channel1_sw_0.Init.DestinationInc = MDMA_DEST_INC_HALFWORD;
  hmdma_mdma_channel1_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_HALFWORD;
  hmdma_mdma_channel1_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_HALFWORD;
  hmdma_mdma_channel1_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
  hmdma_mdma_channel1_sw_0.Init.BufferTransferLength = 128;
  hmdma_mdma_channel1_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
  hmdma_mdma_channel1_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
  hmdma_mdma_channel1_sw_0.Init.SourceBlockAddressOffset = 0;
  hmdma_mdma_channel1_sw_0.Init.DestBlockAddressOffset = 0;
  if (HAL_MDMA_Init(&hmdma_mdma_channel1_sw_0) != HAL_OK)
  {
    Error_Handler();
  }

  /* MDMA interrupt initialization */
  /* MDMA_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MDMA_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(MDMA_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 5;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 15;
  Timing.BusTurnAroundDuration = 5;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GAIN1_0_Pin|GAIN1_1_Pin|AN_POWER_DOWN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NSSSPI1_Pin|LED0_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ARM_PWR_GPIO_Port, ARM_PWR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : F1_Pin F2_Pin F4_Pin */
  GPIO_InitStruct.Pin = F1_Pin|F2_Pin|F4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : F5_Pin DOWN_Pin */
  GPIO_InitStruct.Pin = F5_Pin|DOWN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : RIGHT_Pin */
  GPIO_InitStruct.Pin = RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(RIGHT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_KEY_PWR_Pin */
  GPIO_InitStruct.Pin = INT_KEY_PWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT_KEY_PWR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GAIN1_0_Pin GAIN1_1_Pin AN_POWER_DOWN_Pin */
  GPIO_InitStruct.Pin = GAIN1_0_Pin|GAIN1_1_Pin|AN_POWER_DOWN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : NSSSPI1_Pin LED0_Pin */
  GPIO_InitStruct.Pin = NSSSPI1_Pin|LED0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : UP_Pin ENT_Pin LEFT_Pin */
  GPIO_InitStruct.Pin = UP_Pin|ENT_Pin|LEFT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SD_DETECT_Pin DRDY_Pin */
  GPIO_InitStruct.Pin = SD_DETECT_Pin|DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ARM_PWR_Pin */
  GPIO_InitStruct.Pin = ARM_PWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ARM_PWR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : REC_Pin */
  GPIO_InitStruct.Pin = REC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(REC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RST_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	if (battery_i2c_read_notification_semaphoreHandle != NULL) {
		osSemaphoreRelease(battery_i2c_read_notification_semaphoreHandle);
		//HAL_GPIO_TogglePin(GPIOB, V_TX_Pin);
		HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
	}
  I2C_Rx_Complete_Flag = 1;
}

static void TransferCompleteMDMA(MDMA_HandleTypeDef *hmdma)
{
	if (fmc_write_notification_semaphoreHandle != NULL) {
		osSemaphoreRelease(fmc_write_notification_semaphoreHandle);
		//HAL_GPIO_TogglePin(GPIOB, V_TX_Pin);
	}
}

/**
  * @brief  Main routine for Mass Storage Class
  * @param  None
  * @retval None
  */
/*static void FS_FileOperations(void)
{
  FRESULT res;                                          // FatFs function common result code
  uint32_t byteswritten, bytesread;                     // File write/read counts
  uint8_t wtext[] = "This is STM32 working with FatFs uSD + FreeRTOS"; // File write buffer

  // Register the file system object to the FatFs module
  if(f_mount(&SDFatFS, (TCHAR const*)SDPath, 0) == FR_OK)
  {
    // check whether the FS has been already created
    if (isFsCreated == 0)
    {
      if(f_mkfs(SDPath, FM_ANY, 0, workBuffer, sizeof(workBuffer)) != FR_OK)
      {
        //BSP_LED_On(LED3);
        return;
      }
      isFsCreated = 1;
    }
    // Create and Open a new text file object with write access
    if(f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {
      // Write data to the text file
      res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

      if((byteswritten > 0) && (res == FR_OK))
      {
        // Close the open text file
        f_close(&MyFile);

        // Open the text file object with read access
        if(f_open(&MyFile, "STM32.TXT", FA_READ) == FR_OK)
        {
          // Read data from the text file
          res = f_read(&MyFile, rtext, sizeof(rtext), (void *)&bytesread);

          if((bytesread > 0) && (res == FR_OK))
          {
            // Close the open text file
            f_close(&MyFile);

            // Compare read data with the expected data/
            if((bytesread == byteswritten))
            {
              // Success of the demo: no error occurrence
              //BSP_LED_On(LED1);
              return;
            }
          }
        }
      }
    }
  }
  // Error
  //BSP_LED_On(LED3);
}*/
/*
static void SD_Initialize(void)
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
*/
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
//  for(;;)
//  {
	  //HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
//    osDelay(1000);
//  }
	//uint16_t res = 0;
	//osStatus_t r_state;
	//uint8_t x=0;

	  /* Infinite Loop */
	  for( ;; )
	  {
		 // memcpy((void *)0x38800000, (void *)&g_demo_parameters,sizeof(demo_param_t));
		 // HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
		 // osDelay(2000);

		 // memcpy((void *)&g_demo_parameters1, (void const *)0x38800000, sizeof(demo_param_t));
		  //if(g_demo_parameters1.magic == DEMO_PARAMETERS_MAGIC) HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
		  osDelay(2000);

	  }
  /* USER CODE END 5 */
}

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x60000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32B;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x60100000;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0x38800000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4KB;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
