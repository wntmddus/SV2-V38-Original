/**
 * \file
 *
 */

#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include "parameters.h"
#include "ad7124.h"
#include "ad7124_regs.h"
#include "printf.h"
#include "arm_math.h"
//#include "sd.h"
/* FatFs includes component */
#include "ff_gen_drv.h"
#include "bsp_driver_sd.h"
#include "fatfs.h"

#include "ads_core.h"


#define SSC_MEM_SIZE		(VIB_BUFFER_SIZE*2*3)

#define LENGTH_SAMPLES  SSC_BUFFER_SIZE
__IO   uint32_t DMA_TransferErrorFlag = 0;
//int pwr_bt_first = 0;

//static void HAL_TransferError(DMA_HandleTypeDef *hdma);
//static void HAL_TransferStop(DMA_HandleTypeDef *hdma);
//static void HAL_TransferHalfStop(DMA_HandleTypeDef *hdma);
extern LPTIM_HandleTypeDef hlptim1;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern osThreadId_t Task_dspHandle;
float32_t mytrg1;
float32_t mytrg2;


void vibro(void);
void soundf(void);
/** Receiver buffer content. */
//memory - here is twice(ping-pong)  - these are data from adc
//__attribute__((__aligned__(4), __section__(".data_TCM")))
/*
__attribute__((__aligned__(32),  __section__(".buf20"))) static uint32_t adc_rx_buff[11][SSC_MEM_SIZE];

__attribute__((__aligned__(32),  __section__(".buf20")))
static int32_t vib1_rx_buff[SSC_BUFFER_SIZE];		//

__attribute__((__aligned__(32),  __section__(".buf20")))
static int32_t vib2_rx_buff[SSC_BUFFER_SIZE];		//

__attribute__((__aligned__(32),  __section__(".buf20")))
static int32_t vib3_rx_buff[SSC_BUFFER_SIZE];		//

__attribute__((__aligned__(32),  __section__(".buf20")))
static uint16_t sound_rx_buff[SSC_BUFFER_SIZE];		// 1 buffers (short type) for 1 audio channel
*/
//__attribute__((__aligned__(32),  __section__(".buf20")))
__attribute__((__aligned__(32))) uint8_t soundmicS[4];
__attribute__((__aligned__(32))) uint8_t soundmicR[2][2400*3];
__attribute__((__aligned__(32))) int16_t sd_rx_buff[164*3];		// 3 buffers  for 3 vibro channels
__attribute__((__aligned__(32))) int32_t test3ch_buff[2][3][164];
//__attribute__((__aligned__(32))) uint8_t samp24[2][164][3*3];
__attribute__((__aligned__(32))) int16_t m_data_calibr[3][EVS_BUFLEN];

__attribute__((__aligned__(32))) int16_t soundTest[2400];
__attribute__((__aligned__(32))) long sound_rx_buff[2400];
//__attribute__((__aligned__(32))) uint16_t samp16[164*3];

uint8_t* pcm_ptr = &soundmicR[0][0];

extern RTC_HandleTypeDef hrtc;
extern SPI_HandleTypeDef hspi2;

int file_duration=0, old_file_duration=0;
short isRecPress;
bool  isRecToFile=0; //!!! for cont & single rec.modes it's not same to isRecPress
short flag_arm_rec_old;
uint32_t redraw_lcd;
void close_txt_file(float ntime);

/* Private define ------------------------------------------------------------*/
#define ENAB true
enum {
  TRANSFER_WAIT,
  TRANSFER_COMPLETE,
  TRANSFER_ERROR
};

#define AD7124_CHANNEL_COUNT 3


/* Private variables ---------------------------------------------------------*/
//extern UART_HandleTypeDef huart3;
uint8_t data0[]={'0','0','\n', '\r'};
uint8_t data01[]={'0','1','\n', '\r'};
uint8_t data02[]={'0','2','\n', '\r'};
uint8_t data03[]={'0','3','\n', '\r'};
uint8_t data1[]={'1','0','\n', '\r'};
uint8_t data11[]={'1','1','\n', '\r'};
uint8_t data12[]={'1','2','\n', '\r'};
uint8_t data13[]={'1','3','\n', '\r'};
uint8_t data2[]={'2','0','\n', '\r'};

struct ad7124_dev *dev = &ad7124_device;
int32_t reti;
uint32_t		timeout = 1000;	/* Number of tries before a function times out */
int32_t			sample;		/* Stores raw value read from the ADC */
uint8_t setup_index;
uint8_t ch_index;
int32_t	error_spi1;

int32_t	doutrdy = 0;
int32_t	tg = 0;
uint32_t reg_test1 = 10;

/* Buffer used for transmission */
const uint32_t aTxBuffer[8] = {0x42000000,0,0,0,0,0,0,0};
/* Buffer used for reception */
ALIGN_32BYTES(uint32_t aRxBuffer[8]);

/* transfer state */
__IO uint32_t wTransferState = TRANSFER_WAIT;


int32_t	ch1_samples[2][164];
int32_t	ch2_samples[2][164];
int32_t	ch3_samples[2][164];
int32_t	ping_pong = 0;
int32_t	 ch_samples_count;
uint8_t channel_read;

int32_t	count_error;

static uint32_t wavsizeh, wavsize1ch, wavsize3ch;
struct DateTime dt_dsp;

//FIL file_test3ch;     /* File object */
FIL file_wave3ch;     /* File object */
FIL file_wave1ch;     /* File object */
FIL file_txt;     /* File object */


typedef enum {
  CARD_CONNECTED,
  CARD_DISCONNECTED,
  CARD_STATUS_CHANGED,
}SD_ConnectionStateTypeDef;

extern const Diskio_drvTypeDef  SD_Driver;

uint8_t isInitialized = 0;
//static uint8_t isFsCreated = 0;
static __IO uint8_t statusChanged = 0;

char cnt_file_name[24]; //,cnt_file_name_3ch[32],cnt_file_name_1ch[32];
char str_start_rec[24];   //for saving info about start recording for result file
char txt_file_name[24];


//uint8_t workBuffer[2 * _MAX_SS];
/*
 * ensure that the read buffer 'rtext' is 32-Byte aligned in address and size
 * to guarantee that any other data in the cache won't be affected when the 'rtext'
 * is being invalidated.
 */
ALIGN_32BYTES(uint8_t rtext[64]);

int tyu = 0x800000;

RTC_TimeTypeDef sTimest = {0};
RTC_DateTypeDef sDatest = {0};

/* Private function prototypes -----------------------------------------------*/
static void PB4EXTI_Init(void);
void MY_SPI_TxRxCpltCallback();
static void MY_SPI_CloseTransfer();
void MY_SPI_ErrorCallback();

extern FRESULT create_header(FIL* file, short nChannels, int nSamplesPerSecond);
extern FRESULT update_header(FIL* file, short nChannels, DWORD size);
///static void FS_FileOperations(void);
static void close_wave_file(void);
static FRESULT create_files(void);

void LPTIM_Config(void);

/* ADS127L11 Register 'CONFIG1' Value */
#define ADS127L11_CONFIG1_REG                    \
{                                                \
    {                                            \
        .ainn_buff = 1, /* AINN Buffer enable */ \
        .ainp_buff = 1, /* AINP Buffer enable */ \
        .refp_buf  = 1, /* REFP Buffer enable */ \
        .vcm       = 1, /* VCM Output enable */ \
        .inp_rng   = 0, /* 1x Input Range     */ \
        .ref_rng   = 1, /* 4.096V High-reference range*/ \
    }                                            \
}

/* ADS127L11 Register 'CONFIG2' Value */
#define ADS127L11_CONFIG2_REG                          \
{                                                      \
    {                                                  \
        .pwdn       = 0, /* Normal Operation        */ \
        .stby_mode  = 0, /* Idle Mode               */ \
        .speed_mode = 0, /* High-Speed Mode         */ \
        .start_mode = 0, /* Start-Stop Control Mode */ \
        .sdo_mode   = 0, /* Data Output Only        */ \
        .ext_rng    = 0, /* Standard Input Range    */ \
    }                                                  \
}

/* ADS127L11 Register 'CONFIG3' Value */
//#define ADS127L11_CONFIG3_REG_FILTER_VAL ADS127L11_FILTER_SINC3_OSR_32000_SINC1_OSR_5
#define ADS127L11_CONFIG3_REG                        \
{                                                    \
    {                                                \
        .filter = ADS127L11_FILTER_WIDEBAND_OSR_512,  \
        .delay  = 0, /* No delay */                  \
    }                                                \
}

/* ADS127L11 Register 'CONFIG4' Value */
#define ADS127L11_CONFIG4_REG                                        \
{                                                                    \
    {                                                                \
        .status  = 0, /* Status added to the output data */ \
        .reg_crc = 0, /* CRC Disabled        */                      \
        .spi_crc = 0, /* SPI_CRC Disabled        */                      \
        .data    = 0, /* 24 bit Resolution   */                      \
        .out_drv = 0, /* Full Drive Strength */                      \
        .clk_div = 0, /* No Clock Divide     */                      \
        .clk_sel = 1, /* MY Use External Clock*/                     \
    }                                                                \
}
/*  'CONFIG4' Value BEFORE*/
#define BEFORE_CONFIG4_REG                                        \
{                                                                    \
    {                                                                \
        .status  = 0, /* Status added to the output data */ \
        .reg_crc = 0, /* CRC Disabled        */                      \
        .spi_crc = 0, /* SPI_CRC Disabled        */                      \
        .data    = 0, /* 24 bit Resolution   */                      \
        .out_drv = 0, /* Full Drive Strength */                      \
        .clk_div = 0, /* No Clock Divide     */                      \
        .clk_sel = 1, /* MY Use External Clock*/                     \
    }                                                                \
}
ads127l11_t  ads =
{
    .cfg4_reg   = ( (ads127l11_config4_reg_t)ADS127L11_CONFIG4_REG ),
    .reg = {0}
};


/**
 * \brief task core function.
 *
 * \param pvParameters Junk parameter.
 */
void StartTask01(void *pvParameters)
{
	/* Just to avoid compiler warnings. */
	UNUSED(pvParameters);


	
//	osEvent event;

	  PB4EXTI_Init();

	  HAL_GPIO_WritePin(GPIOC, AN_POWER_DOWN_Pin, 1);

	  g_flagCalibr=0;

///	    if(BSP_SD_IsDetected())
///	    {
///	    	osMessageQueuePut(ConnectionEventHandle, CARD_CONNECTED, 0, osWaitForever);
///	    }



//int8_t workBuffer[1024];
/* sin for sound -----------------------------------------------*/
		  for (int jj = 0; jj < SLM_BUFLEN; jj++)
		{
			//g_algo.SLMStruct.SLMfeedin[jj] = (short)(16000*sin(M_PI_2*(jj/(float)SLM_BUFLEN)));
			 // g_algo.SLMStruct.SLMfeedin[jj] = 0;
		}
/* end sin for sound -----------------------------------------------*/

		cnt_file_name[0] = 0x32;//2
		cnt_file_name[1] = 0x30;//0
		cnt_file_name[8] = 0x2D;//-
		cnt_file_name[15] = 0x5F;//_
		cnt_file_name[16] = 0x33;//3
		cnt_file_name[17] = 0x63;//c
		cnt_file_name[18] = 0x68;//h
		cnt_file_name[19] = 0x2E;//.
		cnt_file_name[20] = 0x77;//w
		cnt_file_name[21] = 0x61;//a
		cnt_file_name[22] = 0x76;//v
		cnt_file_name[23] = 0;//0

		txt_file_name[0] = 0x32;//2
		txt_file_name[1] = 0x30;//0
		txt_file_name[8] = 0x2D;//-
		txt_file_name[15] = 0x2E;//.
		txt_file_name[16] = 0x74;//t
		txt_file_name[17] = 0x78;//x
		txt_file_name[18] = 0x74;//t
		txt_file_name[19] = 0;//0

		str_start_rec[0] = 0x32;
		str_start_rec[1] = 0x30;
		str_start_rec[4] = 0x2D;
		str_start_rec[7] = 0x2D;
		str_start_rec[10] = 0x20;
		str_start_rec[13] = 0x3A;
		str_start_rec[16] = 0x3A;
		str_start_rec[19] = 0;//0

	    if(BSP_SD_IsDetected())
	    {
		  /* Register the file system object to the FatFs module */
		  if(f_mount(&SDFatFS, (TCHAR const*)SDPath, 0) == FR_OK)
		  {
			    // check whether the FS has been already created
//			    if (isFsCreated == 0)
//			    {
//			      if(f_mkfs(SDPath, FM_ANY, 0, workBuffer, sizeof(workBuffer)) != FR_OK)
//			      {
			        //BSP_LED_On(LED3);
//			        return;
//			      }
//			      isFsCreated = 1;
//			    }


				  mounted = 1;
				  //HAL_UART_Transmit(&huart3, data0, 4, 2);

		  }
	    }


	  HAL_GPIO_WritePin(GPIOC, GAIN1_0_Pin|GAIN1_1_Pin, 0); //1 gain 8


	  osDelay(1000);

	    // Start ADS127L11
	    bool setup_res = ads127l11_setup
	    (
	        &ads,
	        (ads127l11_config1_reg_t)ADS127L11_CONFIG1_REG,
	        (ads127l11_config2_reg_t)ADS127L11_CONFIG2_REG,
	        (ads127l11_config3_reg_t)ADS127L11_CONFIG3_REG,
	        (ads127l11_config4_reg_t)ADS127L11_CONFIG4_REG
	    );

	    // Check ADS127L11 initialization
	    //if(setup_res) HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);



		reti = ad7124_setup(dev); //////
		if (reti)
			error_spi1 = 1;

	// Set config 0
		setup_index = 0;

		reti = setConfigFilter (dev, setup_index, 2, 1, 0, 0, 0);
		if (reti)
			error_spi1 = 1;
		reti = ad7124_set_polarity(dev,1,setup_index); //True in case of Bipolar
		if (reti)
			error_spi1 = 1;
		reti = ad7124_set_reference_source(dev,INTERNAL_REF, setup_index, ENAB); ///////
		if (reti)
			error_spi1 = 1;
		reti = ad7124_enable_buffers(dev,1,1,1,setup_index); //Enable Input BufferP. Disable Input BufferM. Enable reference Buffer.
		if (reti)
			error_spi1 = 1;
	// Config ADC
		reti = ad7124_set_adc_mode(dev, AD7124_CONTINUOUS);//////
		if (reti)
			error_spi1 = 1;
		reti = ad7124_set_power_mode(dev, AD7124_HIGH_POWER);//////
		if (reti)
			error_spi1 = 1;
	// Set channel 0
		ch_index = 0;

		reti = ad7124_connect_analog_input(dev, ch_index, AD7124_AIN6, AD7124_AIN15);
		if (reti)
			error_spi1 = 1;

		reti = ad7124_assign_setup(dev, ch_index, 0); //setup_index = 0;
		if (reti)
			error_spi1 = 1;

		reti = ad7124_set_channel_status(dev,ch_index,ENAB);
		if (reti)
			error_spi1 = 1;

		// Set channel 1
			ch_index = 1;

			reti = ad7124_connect_analog_input(dev, ch_index, AD7124_AIN7, AD7124_AIN15);
			if (reti)
				error_spi1 = 1;

			reti = ad7124_assign_setup(dev, ch_index, 0); //setup_index = 0;
			if (reti)
				error_spi1 = 1;

			reti = ad7124_set_channel_status(dev,ch_index,ENAB);
			if (reti)
				error_spi1 = 1;

			// Set channel 2
				ch_index = 2;

				reti = ad7124_connect_analog_input(dev, ch_index, AD7124_AIN8, AD7124_AIN15);
				if (reti)
					error_spi1 = 1;

				reti = ad7124_assign_setup(dev, ch_index, 0); //setup_index = 0;
				if (reti)
					error_spi1 = 1;

				reti = ad7124_set_channel_status(dev,ch_index,ENAB);
				if (reti)
					error_spi1 = 1;

		//VBIAS
		reti = setBiasPins (dev, AD7124_8_IO_CTRL2_REG_GPIO_VBIAS15);
		if (reti)
			error_spi1 = 1;


		//if(error_spi1 > 0)
		//	HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);


		  doutrdy = 0;
		  while(HAL_NVIC_GetPendingIRQ(EXTI4_IRQn)) {
		    EXTI->PR1 = (EXTI_PR1_PR4);
		    HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
		  }
		  HAL_NVIC_EnableIRQ(EXTI4_IRQn);


		    /* SPI1 interrupt Init */
		    HAL_NVIC_SetPriority(SPI1_IRQn, 6, 0);
		    HAL_NVIC_EnableIRQ(SPI1_IRQn);

		  HAL_GPIO_WritePin(GPIOA, NSSSPI1_Pin, 0);

		  flag_arm_rec_old=flag_arm_rec; //keep previous state of recording/arming

		  /*Start the synchronized transmission process*/
		  /*##-3- Configure and start the LPTIM1 used for DMA transfer Synchronization#*/
		  /* LPTIM1 is configured with 24000Hz */
		  /* Select Callbacks functions called after Transfer complete and Transfer error */


/*			HAL_DMA_XFER_CPLT_CB_ID:
			HAL_DMA_XFER_HALFCPLT_CB_ID:
			HAL_DMA_XFER_M1CPLT_CB_ID:
			HAL_DMA_XFER_M1HALFCPLT_CB_ID:
			HAL_DMA_XFER_ERROR_CB_ID:
			HAL_DMA_XFER_ABORT_CB_ID:
*/
		  //HAL_DMA_RegisterCallback(&hdma_spi2_rx, HAL_DMA_XFER_HALFCPLT_CB_ID, HAL_TransferHalfStop);
		  //HAL_DMA_RegisterCallback(&hdma_spi2_rx, HAL_DMA_XFER_CPLT_CB_ID, HAL_TransferStop);
		  //HAL_DMA_RegisterCallback(&hdma_spi2_rx, HAL_DMA_XFER_ERROR_CB_ID, HAL_TransferError);
		  /* Polling on ADS Ready signal */
		  int timout = 10000;
		   while((HAL_GPIO_ReadPin(GPIOB, DRDY_Pin) == 0) && (timout > 0))
		   {
		     timout--;
		   }
		   timout = 100;
		   while((HAL_GPIO_ReadPin(GPIOB, DRDY_Pin) == 1) && (timout > 0))
		   {
		     timout--;
		   }
		  LPTIM_Config();
		  /*##-6- Start the synchronized transmission process #####################################*/
		  if(HAL_SPI_TransmitReceive_DMA(&hspi2, (uint8_t*)soundmicS, (uint8_t*)soundmicR, 2*2400*3))
		    {
		      /* Transfer error in transmission process */
		      Error_Handler();
		    }

		  //pwr_bt_first = 1;

	for (;;) {

		///osDelay(50);

		isRecPress=flag_arm_rec;  //keep in isRecPress, because global var flag_arm_rec can be changed during "while"
		//check changing of recording state:

		if (isRecPress!=flag_arm_rec_old)  //status is changing: create wave-file of close
		{
			//HAL_UART_Transmit(&huart3, data01, 4, 2);

			//manual recording
			if (isRecPress && g_demo_parameters.opt_rec.nRecordMode ==0)  //start rec->create file
			{
				//HAL_UART_Transmit(&huart3, data02, 4, 2);
				if (create_files()!=FR_OK)
				{
					flag_arm_rec=0;
					isRecToFile=0;
				}
				else
				{
					//HAL_UART_Transmit(&huart3, data03, 4, 2);
					isRecToFile=1;
					old_file_duration=0;

				}
			}
			else if (isRecToFile)//close file  //for all recording types (by FREC button)
			{
				if (g_demo_parameters.opt_rec.file_type==0 || g_demo_parameters.opt_rec.file_type==2){
					close_wave_file();
					//HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
				}
				if (g_demo_parameters.opt_rec.file_type==1 || g_demo_parameters.opt_rec.file_type==2)
					close_txt_file((float)wavsize3ch/(float)(1634*3*2));  //pop
				isRecToFile=0;
				draw_file_name = 2;//("");//duplicate for guarantee
			}
			flag_arm_rec_old = isRecPress;

			///set_rec_button();
		}

		//if (dat_kol == 10){
		//	dat_kol = 11;
		//	update_header(&Myfile,3,wavsize3ch);
		//	f_close(&Myfile);
		//	HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
		//}

		//Get Data
		/* Wait until notified by the ISR that transmission is complete. */


				//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 1);
				///HAL_GPIO_WritePin(F1_GPIO_Port, F1_Pin,1);
				///HAL_GPIO_WritePin(F4_GPIO_Port, F4_Pin,1);
		switch(osThreadFlagsWait(0x7, osFlagsWaitAny,1000))
		{
		case 0x1: //ads Half
			///HAL_GPIO_WritePin(F1_GPIO_Port, F1_Pin, 0);
			soundf();

		break;
		case 0x2: //ads Complit
			///HAL_GPIO_WritePin(F1_GPIO_Port, F1_Pin, 0);
			soundf();
		break;
		case 0x4: // ad71
			///HAL_GPIO_WritePin(F4_GPIO_Port, F4_Pin, 0);
			vibro();
		break;
		}
	}

}

/* HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); Configure the External Interrupt or event for the current IO */
static void PB4EXTI_Init(void)
{
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB;
	/* SET Falling edge configuration */
	EXTI->FTSR1 |= EXTI_FTSR1_TR4;
	/* SET EXTI line configuration */
	EXTI->IMR1 |= EXTI_IMR1_IM4;
	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
	HAL_NVIC_DisableIRQ(EXTI4_IRQn);

}
uint32_t spiState;
uint32_t spiErrorCode = HAL_SPI_ERROR_NONE;
uint32_t spiTxXferCount;
uint32_t spiRxXferCount;
uint32_t imdect;
int32_t sam_count;
/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
 if(((EXTI->PR1) & (EXTI_PR1_PR4)) != 0){
	EXTI->PR1 = EXTI_PR1_PR4;
		//HAL_GPIO_WritePin(GPIOA, NSSSPI1_Pin, 0);
	//HAL_GPIO_WritePin(GPIOB, V_TX_Pin, 1);
	if(doutrdy == 0){

		//HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 1);

		  /* Set the transaction information */
		  spiState       = HAL_SPI_STATE_BUSY_TX_RX;
		  spiErrorCode   = HAL_SPI_ERROR_NONE;

		 /* SPIx CFG1 Configuration */
		  //WRITE_REG(hspi->Instance->CFG1, (hspi->Init.BaudRatePrescaler | hspi->Init.CRCCalculation | crc_length | hspi->Init.FifoThreshold | hspi->Init.DataSize));
		 /* 32BIT DataSize */
		    SPI1->CFG1 |= SPI_DATASIZE_32BIT;

		/* Set the number of data at current transfer */
		SPI1->CR2 = 1;

		/* Enable SPI peripheral */
		SPI1->CR1 |= SPI_CR1_SPE;

		/* Fill in the TxFIFO */
	    /* Transmit data in 32 Bit mode */
	    *((__IO uint32_t *)&(SPI1->TXDR)) = *((const uint32_t *) &(aTxBuffer[0]));

	    /* Enable EOT, DXP, UDR, OVR, FRE, MODF and TSERF interrupts */
	    //SPI1->IER |= (SPI_IT_EOT | SPI_IT_DXP | SPI_IT_UDR | SPI_IT_OVR | SPI_IT_FRE | SPI_IT_MODF | SPI_IT_TSERF);
	    SPI1->IER |= (SPI_IT_EOT); // | SPI_IT_DXP | SPI_IT_UDR | SPI_IT_OVR | SPI_IT_FRE | SPI_IT_MODF | SPI_IT_TSERF);

	    /* Start Master transfer */
	    SPI1->CR1 |= SPI_CR1_CSTART;

		//HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 0);

/*
		HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 1);
			HAL_SPI_TransmitReceive_DMA(dev->spi_desc, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, 4);
		HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 0);
*/
	}

	else if(doutrdy == 1){

		//..HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 1);

		/* Set the transaction information */
		  spiState       = HAL_SPI_STATE_BUSY_TX_RX;
		  spiErrorCode   = HAL_SPI_ERROR_NONE;

				/* Set the number of data at current transfer */
				SPI1->CR2 = 1;

				/* Enable SPI peripheral */
				SPI1->CR1 |= SPI_CR1_SPE;

				/* Fill in the TxFIFO */
			    /* Transmit data in 32 Bit mode */
			    *((__IO uint32_t *)&(SPI1->TXDR)) = *((const uint32_t *) &(aTxBuffer[0]));

			    /* Enable EOT, DXP, UDR, OVR, FRE, MODF and TSERF interrupts */
			    //SPI1->IER |= (SPI_IT_EOT | SPI_IT_DXP | SPI_IT_UDR | SPI_IT_OVR | SPI_IT_FRE | SPI_IT_MODF | SPI_IT_TSERF);
			    SPI1->IER |= (SPI_IT_EOT); // | SPI_IT_DXP | SPI_IT_UDR | SPI_IT_OVR | SPI_IT_FRE | SPI_IT_MODF | SPI_IT_TSERF);

			    /* Start Master transfer */
			    SPI1->CR1 |= SPI_CR1_CSTART;
		//..HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 0);

	}

	doutrdy = 1;
	//HAL_NVIC_DisableIRQ(EXTI4_IRQn);
	//HAL_GPIO_WritePin(GPIOB, V_TX_Pin, 0);

	EXTI->IMR1 &= ~EXTI_IMR1_IM4;
 }
}


/**
  * @brief This function handles SPI1 global interrupt.
  */
void SPI1_IRQHandler(void)
{
	uint32_t State = spiState;

	//..HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 1);
	  /* SPI in mode Transmitter and Receiver no overrun- no underrun- Both TxFIFO has space for write and RxFIFO contains for read a single packet at least--*/
//	  if (HAL_IS_BIT_CLR(trigger, SPI_FLAG_OVR) && HAL_IS_BIT_CLR(trigger, SPI_FLAG_UDR) && HAL_IS_BIT_SET(trigger, SPI_FLAG_DXP))
//	  {
//	    hspi->TxISR(hspi);
//	    hspi->RxISR(hspi);
//	    handled = 1UL;
//	  }

//	  if (handled != 0UL)
//	  {
//	    return;
//	  }

	  /* SPI End Of Transfer: DMA or IT based transfer */
	  //if (HAL_IS_BIT_SET(trigger, SPI_FLAG_EOT))
	if(SPI1->SR & SPI_FLAG_EOT)
	  {
	    /* Clear EOT/TXTF/SUSP flag */
		SPI1->IFCR |= SPI_IFCR_EOTC | SPI_IFCR_TXTFC | SPI_IFCR_SUSPC;
	    //__HAL_SPI_CLEAR_EOTFLAG(hspi);
	    //__HAL_SPI_CLEAR_TXTFFLAG(hspi);
	    //__HAL_SPI_CLEAR_SUSPFLAG(hspi);

	    /* Disable EOT interrupt */
	    //__HAL_SPI_DISABLE_IT(hspi, SPI_IT_EOT);
		SPI1->IER  &= ~(SPI_IT_EOT);

	    /* For the IT based receive extra polling maybe required for last packet */

	      /* Pooling remaining data */
//	      while (hspi->RxXferCount != 0UL)
//	      {
	        /* Receive data in 32 Bit mode */

//	          *((uint32_t *)hspi->pRxBuffPtr) = *((__IO uint32_t *)&hspi->Instance->RXDR);
//	          hspi->pRxBuffPtr += sizeof(uint32_t);

//	        hspi->RxXferCount--;
//	      }

		*((uint32_t *)&(aRxBuffer[0])) = *((__IO uint32_t *)&(SPI1->RXDR));
		//aRxBuffer[0] = tyu;
		//tyu += 0x100;
	    /* Call SPI Standard close procedure */
	    MY_SPI_CloseTransfer();

	    spiState = HAL_SPI_STATE_READY;
	    if (spiErrorCode != HAL_SPI_ERROR_NONE)
	    {

	      MY_SPI_ErrorCallback();
		  //..HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 0);
	      return;
	    }

	    /* Call appropriate user callback */
	    if (State == HAL_SPI_STATE_BUSY_TX_RX)
	    {
	      MY_SPI_TxRxCpltCallback();
	    }
	    else
	    {
	      /* End of the appropriate call */
	    }
		  //..HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 0);
	    return;
	  }

	  /* SPI in Error Treatment --------------------------------------------------*/
	  if (((SPI1->IER & SPI1->SR) & (SPI_FLAG_MODF | SPI_FLAG_OVR | SPI_FLAG_FRE | SPI_FLAG_UDR)) != 0UL)
	  {
	    /* SPI Overrun error interrupt occurred ----------------------------------*/
	    if (((SPI1->IER & SPI1->SR) & SPI_FLAG_OVR) != 0UL)
	    {
	      SET_BIT(spiErrorCode, HAL_SPI_ERROR_OVR);
	      //__HAL_SPI_CLEAR_OVRFLAG(hspi);
	      SPI1->IFCR |= SPI_IFCR_OVRC;
	    }

	    /* SPI Mode Fault error interrupt occurred -------------------------------*/
	    if (((SPI1->IER & SPI1->SR) & SPI_FLAG_MODF) != 0UL)
	    {
	      SET_BIT(spiErrorCode, HAL_SPI_ERROR_MODF);
	      //__HAL_SPI_CLEAR_MODFFLAG(hspi);
	      SPI1->IFCR |= SPI_IFCR_MODFC;
	    }

	    /* SPI Frame error interrupt occurred ------------------------------------*/
	    if (((SPI1->IER & SPI1->SR) & SPI_FLAG_FRE) != 0UL)
	    {
	      SET_BIT(spiErrorCode, HAL_SPI_ERROR_FRE);
	      //__HAL_SPI_CLEAR_FREFLAG(hspi);
	      SPI1->IFCR |= SPI_IFCR_TIFREC;
	    }

	    /* SPI Underrun error interrupt occurred ------------------------------------*/
	    if (((SPI1->IER & SPI1->SR) & SPI_FLAG_UDR) != 0UL)
	    {
	      SET_BIT(spiErrorCode, HAL_SPI_ERROR_UDR);
	      //__HAL_SPI_CLEAR_UDRFLAG(hspi);
	      SPI1->IFCR |= SPI_IFCR_UDRC;
	    }

	    if (spiErrorCode != HAL_SPI_ERROR_NONE)
	    {
	      /* Disable SPI peripheral */
	      //__HAL_SPI_DISABLE(hspi);
	    	SPI1->CR1  &= ~(SPI_CR1_SPE);

	      /* Disable all interrupts */
	      //__HAL_SPI_DISABLE_IT(hspi, (SPI_IT_EOT | SPI_IT_RXP | SPI_IT_TXP | SPI_IT_MODF |
	      //                            SPI_IT_OVR | SPI_IT_FRE | SPI_IT_UDR));
	      SPI1->IER &= ~((SPI_IT_EOT | SPI_IT_RXP | SPI_IT_TXP | SPI_IT_MODF | SPI_IT_OVR | SPI_IT_FRE | SPI_IT_UDR));

	        /* Restore hspi->State to Ready */
	        spiState = HAL_SPI_STATE_READY;

	        /* Call user error callback */
	        MY_SPI_ErrorCallback();
	    }
		 // HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 0);
	    return;
	  }
	  //HAL_GPIO_WritePin(GPIOB, VCP_TX_Pin, 0);
}


void MY_SPI_TxRxCpltCallback()
{
	int32_t stemmp;
  /*  Transfer in transmission process is complete */

  /* Transfer in reception process is complete */
	EXTI->IMR1 |= EXTI_IMR1_IM4;//setting bits

//	*((uint32_t *)&(aRxBuffer[0])) = *((__IO uint32_t *)&(SPI1->RXDR));

	///HAL_GPIO_TogglePin(F2_GPIO_Port, F2_Pin);

	/* Build the result */

		stemmp = (aRxBuffer[0]) - 0x800000;

		//samp24[ping_pong][ch_samples_count][channel_read*3] =  (int8_t)((stemmp >> 24) & 0xFF);
		//samp24[ping_pong][ch_samples_count][channel_read*3+1] =  (int8_t)((stemmp >> 16) & 0xFF);
		//samp24[ping_pong][ch_samples_count][channel_read*3+2] =  (int8_t)((stemmp >> 8) & 0xFF);
		if (channel_read == 0) {
			ch1_samples[ping_pong][ch_samples_count] = stemmp;
			///test3ch_buff[ping_pong][0][ch_samples_count] = aRxBuffer[0];
		}
		else if (channel_read == 1) {
			ch2_samples[ping_pong][ch_samples_count] = stemmp;
			///test3ch_buff[ping_pong][1][ch_samples_count] = aRxBuffer[0];
		}
		else if (channel_read == 2) {
			///test3ch_buff[ping_pong][2][ch_samples_count] = aRxBuffer[0];
			ch3_samples[ping_pong][ch_samples_count++] = stemmp;

		}


	if(channel_read < 2) channel_read++;
	else channel_read = 0;

	if (ch_samples_count == 164) {
			ch_samples_count = 0;
			ping_pong ^= 1;
			 //HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 0);
			 osThreadFlagsSet(Task_dspHandle,0x4);
			//if (ad7124_notification_semaphoreHandle != NULL) {
			//	osSemaphoreRelease(ad7124_notification_semaphoreHandle);
				//HAL_GPIO_TogglePin(GPIOB, V_TX_Pin);
			//}
	}



	  while(HAL_NVIC_GetPendingIRQ(EXTI4_IRQn)) {
	    EXTI->PR1 = (EXTI_PR1_PR4);
	    HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
	  }
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  wTransferState = TRANSFER_COMPLETE;

}

/**
  * @brief  Close Transfer and clear flags.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval HAL_ERROR: if any error detected
  *         HAL_OK: if nothing detected
  */
static void MY_SPI_CloseTransfer()
{
  //uint32_t itflag = SPI1->SR;

  //__HAL_SPI_CLEAR_EOTFLAG(hspi);
  //__HAL_SPI_CLEAR_TXTFFLAG(hspi);
  SPI1->IFCR |= SPI_IFCR_EOTC | SPI_IFCR_TXTFC;

  /* Disable SPI peripheral */
  //__HAL_SPI_DISABLE(hspi);
  SPI1->CR1  &= ~(SPI_CR1_SPE);

  /* Disable ITs */
   SPI1->IER &= ~((SPI_IT_EOT | SPI_IT_TXP | SPI_IT_RXP | SPI_IT_DXP | SPI_IT_UDR | SPI_IT_OVR | SPI_IT_FRE | SPI_IT_MODF));


  /* Report UnderRun error for non RX Only communication */
/*  if (spiState != HAL_SPI_STATE_BUSY_RX)
  {
    if ((itflag & SPI_FLAG_UDR) != 0UL)
    {
      SET_BIT(spiErrorCode, HAL_SPI_ERROR_UDR);
      //__HAL_SPI_CLEAR_UDRFLAG(hspi);
      SPI1->IFCR |= SPI_IFCR_UDRC;
    }
  }
*/
  /* Report OverRun error for non TX Only communication */
/*  if (spiState != HAL_SPI_STATE_BUSY_TX)
  {
    if ((itflag & SPI_FLAG_OVR) != 0UL)
    {
      SET_BIT(spiErrorCode, HAL_SPI_ERROR_OVR);
      //__HAL_SPI_CLEAR_OVRFLAG(hspi);
      SPI1->IFCR |= SPI_IFCR_OVRC;
    }
  }
*/
  /* SPI Mode Fault error interrupt occurred -----------------------------??*/
/*  if ((itflag & SPI_FLAG_MODF) != 0UL)
  {
    SET_BIT(spiErrorCode, HAL_SPI_ERROR_MODF);
    //__HAL_SPI_CLEAR_MODFFLAG(hspi);
    SPI1->IFCR |= SPI_IFCR_MODFC;
  }
*/
  /* SPI Frame error interrupt occurred ----------------------------------??*/
/*  if ((itflag & SPI_FLAG_FRE) != 0UL)
  {
    SET_BIT(spiErrorCode, HAL_SPI_ERROR_FRE);
    //__HAL_SPI_CLEAR_FREFLAG(hspi);
    SPI1->IFCR |= SPI_IFCR_TIFREC;
  }
*/
  //spiTxXferCount = (uint16_t)0UL;
  //spiRxXferCount = (uint16_t)0UL;
}



/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of DMA TxRx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//{
  /*  Transfer in transmission process is complete */

  /* Transfer in reception process is complete */
//	EXTI->IMR1 |= EXTI_IMR1_IM4;//setting bits

	/* Build the result
	res_value = 0;
	for(imd = 1; imd < 3 + 1; imd++) {
		res_value <<= 8;
		res_value += aRxBuffer[imd];
	}
	channel_read = aRxBuffer[5] & 0x0F;
	if (channel_read < AD7124_CHANNEL_COUNT) {
		if (channel_read == 0) {
			if (ch1_samples_count < 160) ch1_samples[ping_pong][ch1_samples_count++] = res_value;
			else count_error++;
		}
		if (channel_read == 1) {
			if (ch2_samples_count < 160) ch2_samples[ping_pong][ch2_samples_count++] = res_value;
			else count_error++;
		}
		if (channel_read == 2) {
			if (ch3_samples_count < 160) ch3_samples[ping_pong][ch3_samples_count++] = res_value;
			else count_error++;
		}
	} else {
			//"Channel Read was %d, which is not < AD7124_CHANNEL_COUNT\r\n"
	}

	if (ch3_samples_count == 160) {
		if ((ch3_samples_count == ch2_samples_count) && (ch3_samples_count == ch1_samples_count)) {
			ch3_samples_count = 0;
			ch2_samples_count = 0;
			ch1_samples_count = 0;
			ping_pong ^= 1;
		} else count_error++;

	}
*/


//	  while(HAL_NVIC_GetPendingIRQ(EXTI4_IRQn)) {
//	    EXTI->PR1 = (EXTI_PR1_PR4);
//	    HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
//	  }
//	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
//  wTransferState = TRANSFER_COMPLETE;

//}
/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void MY_SPI_ErrorCallback()
{
  wTransferState = TRANSFER_ERROR;
}

static FRESULT create_wav_file()
{
	FRESULT res = FR_NOT_READY;//FR_INVALID_PARAMETER;

	if (mounted)
	{

		dt_dsp.second++;
		if(dt_dsp.second == 60) dt_dsp.second = 0;

/*		if (file_test3ch.obj.fs == 0)
		{
			//HAL_UART_Transmit(&huart3, data1, 4, 2);
			//char test_file_name[32] = "0:data.txt";
			HAL_RTC_GetTime(&hrtc, &sTimest, RTC_FORMAT_BCD);
			HAL_RTC_GetDate(&hrtc, &sDatest, RTC_FORMAT_BCD);

			cnt_file_name[2] = ((sDatest.Year >> 4) & 0xF)+0x30;
			cnt_file_name[3] = (sDatest.Year & 0xF)+0x30;
			cnt_file_name[4] = ((sDatest.Month >> 4) & 0xF)+0x30;
			cnt_file_name[5] = (sDatest.Month & 0xF)+0x30;
			cnt_file_name[6] = ((sDatest.Date >> 4) & 0xF)+0x30;
			cnt_file_name[7] = (sDatest.Date & 0xF)+0x30;
			cnt_file_name[9] = ((sTimest.Hours >> 4) & 0xF)+0x30;
			cnt_file_name[10] = (sTimest.Hours & 0xF)+0x30;
			cnt_file_name[11] = ((sTimest.Minutes >> 4) & 0xF)+0x30;
			cnt_file_name[12] = (sTimest.Minutes & 0xF)+0x30;
			cnt_file_name[13] = ((sTimest.Seconds >> 4) & 0xF)+0x30;
			cnt_file_name[14] = (sTimest.Seconds & 0xF)+0x30;
			cnt_file_name[16] = 0x36;//3


					res = f_open(&file_test3ch, cnt_file_name, FA_OPEN_ALWAYS | FA_WRITE);
					if (res == FR_OK)
					{
						//HAL_UART_Transmit(&huart3, data12, 4, 2);
						f_sync(&file_test3ch);//alex

					}
					else if (res==FR_NOT_READY)  //no SD
					{
						mounted=0;
						return 1;

						//DrawSD();
					}

		}
*/
		if (file_wave3ch.obj.fs == 0)
		{
			//HAL_UART_Transmit(&huart3, data1, 4, 2);
			//char test_file_name[32] = "0:data.txt";
			HAL_RTC_GetTime(&hrtc, &sTimest, RTC_FORMAT_BCD);
			HAL_RTC_GetDate(&hrtc, &sDatest, RTC_FORMAT_BCD);

			cnt_file_name[2] = ((sDatest.Year >> 4) & 0xF)+0x30;
			cnt_file_name[3] = (sDatest.Year & 0xF)+0x30;
			cnt_file_name[4] = ((sDatest.Month >> 4) & 0xF)+0x30;
			cnt_file_name[5] = (sDatest.Month & 0xF)+0x30;
			cnt_file_name[6] = ((sDatest.Date >> 4) & 0xF)+0x30;
			cnt_file_name[7] = (sDatest.Date & 0xF)+0x30;
			cnt_file_name[9] = ((sTimest.Hours >> 4) & 0xF)+0x30;
			cnt_file_name[10] = (sTimest.Hours & 0xF)+0x30;
			cnt_file_name[11] = ((sTimest.Minutes >> 4) & 0xF)+0x30;
			cnt_file_name[12] = (sTimest.Minutes & 0xF)+0x30;
			cnt_file_name[13] = ((sTimest.Seconds >> 4) & 0xF)+0x30;
			cnt_file_name[14] = (sTimest.Seconds & 0xF)+0x30;
			cnt_file_name[16] = 0x33;//3


					res = f_open(&file_wave3ch, cnt_file_name, FA_OPEN_ALWAYS | FA_WRITE);
					if (res == FR_OK)
					{
						//HAL_UART_Transmit(&huart3, data12, 4, 2);
						res=create_header(&file_wave3ch,3,SamplFreqVibro);
						f_sync(&file_wave3ch);//alex
						draw_file_name = 1;//(cnt_file_name);
					}
					else if (res==FR_NOT_READY)  //no SD
					{
						mounted=0;
						return 1;

						//DrawSD();
					}

		}
		if (file_wave1ch.obj.fs == 0)
		{
			//HAL_UART_Transmit(&huart3, data1, 4, 2);
			//char test_file_name[32] = "0:data.txt";
			HAL_RTC_GetTime(&hrtc, &sTimest, RTC_FORMAT_BCD);
			HAL_RTC_GetDate(&hrtc, &sDatest, RTC_FORMAT_BCD);

			cnt_file_name[2] = ((sDatest.Year >> 4) & 0xF)+0x30;
			cnt_file_name[3] = (sDatest.Year & 0xF)+0x30;
			cnt_file_name[4] = ((sDatest.Month >> 4) & 0xF)+0x30;
			cnt_file_name[5] = (sDatest.Month & 0xF)+0x30;
			cnt_file_name[6] = ((sDatest.Date >> 4) & 0xF)+0x30;
			cnt_file_name[7] = (sDatest.Date & 0xF)+0x30;
			cnt_file_name[9] = ((sTimest.Hours >> 4) & 0xF)+0x30;
			cnt_file_name[10] = (sTimest.Hours & 0xF)+0x30;
			cnt_file_name[11] = ((sTimest.Minutes >> 4) & 0xF)+0x30;
			cnt_file_name[12] = (sTimest.Minutes & 0xF)+0x30;
			cnt_file_name[13] = ((sTimest.Seconds >> 4) & 0xF)+0x30;
			cnt_file_name[14] = (sTimest.Seconds & 0xF)+0x30;
			cnt_file_name[16] = 0x31;//1

			res = f_open(&file_wave1ch, cnt_file_name, FA_OPEN_ALWAYS | FA_WRITE);
			if (res == FR_OK)
			{

				res=create_header(&file_wave1ch,1,SamplFreqSound);
				f_sync(&file_wave1ch);//alex

			}
			else if (res==FR_NOT_READY)  //no SD
			{
				mounted=0;
				return 1;
				//DrawSD();
			}

		}

	}

	return res;
}

static void close_wave_file(void)
{
/*	if (file_test3ch.obj.fs != 0)
	{
    	f_close(&file_test3ch);


	}
*/
	if (file_wave3ch.obj.fs != 0)
	{
    	update_header(&file_wave3ch,3,wavsize3ch);
    	f_close(&file_wave3ch);

    	draw_file_name = 2;//("");
    	draw_rec_time(0);
	}
	if (file_wave1ch.obj.fs != 0)
	{
    	update_header(&file_wave1ch,1,wavsize1ch);
    	f_close(&file_wave1ch);

 	}
	//ioport_set_pin_level(LED0_GPIO, IOPORT_PIN_LEVEL_HIGH);
}

static FRESULT create_txt_file(void)
{
	FRESULT res = FR_NOT_READY;//FR_INVALID_PARAMETER;

	if (mounted)
	{
		if (file_txt.obj.fs == 0)
		{
			//HAL_UART_Transmit(&huart3, data1, 4, 2);
			//char test_file_name[32] = "0:data.txt";
			HAL_RTC_GetTime(&hrtc, &sTimest, RTC_FORMAT_BCD);
			HAL_RTC_GetDate(&hrtc, &sDatest, RTC_FORMAT_BCD);

			txt_file_name[2] = ((sDatest.Year >> 4) & 0xF)+0x30;
			txt_file_name[3] = (sDatest.Year & 0xF)+0x30;
			txt_file_name[4] = ((sDatest.Month >> 4) & 0xF)+0x30;
			txt_file_name[5] = (sDatest.Month & 0xF)+0x30;
			txt_file_name[6] = ((sDatest.Date >> 4) & 0xF)+0x30;
			txt_file_name[7] = (sDatest.Date & 0xF)+0x30;
			txt_file_name[9] = ((sTimest.Hours >> 4) & 0xF)+0x30;
			txt_file_name[10] = (sTimest.Hours & 0xF)+0x30;
			txt_file_name[11] = ((sTimest.Minutes >> 4) & 0xF)+0x30;
			txt_file_name[12] = (sTimest.Minutes & 0xF)+0x30;
			txt_file_name[13] = ((sTimest.Seconds >> 4) & 0xF)+0x30;
			txt_file_name[14] = (sTimest.Seconds & 0xF)+0x30;

			str_start_rec[2] = txt_file_name[2];
			str_start_rec[3] = txt_file_name[3];
			str_start_rec[5] = txt_file_name[4];
			str_start_rec[6] = txt_file_name[5];
			str_start_rec[8] = txt_file_name[6];
			str_start_rec[9] = txt_file_name[7];
			str_start_rec[11] = txt_file_name[9];
			str_start_rec[12] = txt_file_name[10];
			str_start_rec[14] = txt_file_name[11];
			str_start_rec[15] = txt_file_name[12];
			str_start_rec[17] = txt_file_name[13];
			str_start_rec[18] = txt_file_name[14];

			res = f_open(&file_txt, txt_file_name, FA_OPEN_ALWAYS | FA_WRITE);
			if (res == FR_OK)
			{
				char tmp[32];
				UINT bw;
				snprintf_(tmp,32,"File: %s\r\n",txt_file_name);
				 f_write(&file_txt, tmp, strlen(tmp), &bw);

				f_sync(&file_txt);//alex
				if (g_demo_parameters.opt_rec.file_type==1) //only txt
					draw_file_name = 3;//(cnt_file_name);

			}
			else if (res==FR_NOT_READY)  //no SD
			{
				mounted=0;
				//DrawSD();
			}

		}
	}
	return res;
}

void close_txt_file(float ntime)
{
	write_result(&file_txt, ntime);

	//
	f_close(&file_txt);

	draw_file_name = 2;// ("");
	draw_rec_time(0);
	//ioport_set_pin_level(LED0_GPIO, IOPORT_PIN_LEVEL_HIGH);
}

static FRESULT create_files()
{
	FRESULT result=FR_NOT_READY;
	//cnt_file_name[0]=0;
	if (g_demo_parameters.opt_rec.file_type==0 || g_demo_parameters.opt_rec.file_type==2)
		result=create_wav_file();
	if (g_demo_parameters.opt_rec.file_type==1 || g_demo_parameters.opt_rec.file_type==2)
		result=create_txt_file();
	wavsizeh = 0; wavsize1ch = 0; wavsize3ch = 0;
	return result;
}

/**
  * @brief  Configure and start the LPTIM1 with 2sec period and 50% duty cycle.
  * @param  None
  * @retval None
  */
void LPTIM_Config(void)
{

  uint32_t periodValue;
  uint32_t pulseValue ;

  periodValue = 2047; // 98304000/(2*24000)   /* Calculate the Timer  Autoreload value for 24000Hz */
//  periodValue = 16384; // 98304000/(64*48)   /* Calculate the Timer  Autoreload value for 48Hz */
  pulseValue  = periodValue/2;        /* Set the Timer  pulse value for 50% duty cycle         */

  /* Start the timer */
  if (HAL_LPTIM_PWM_Start(&hlptim1, periodValue, pulseValue) != HAL_OK)
  {
    Error_Handler();
  }

}

static void HAL_TransferError(DMA_HandleTypeDef *hdma)
{
  DMA_TransferErrorFlag = 1;
  //HAL_GPIO_WritePin(GPIOB, V_TX_Pin, 0);
}

//static void HAL_TransferStop(DMA_HandleTypeDef *hdma)
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	 pcm_ptr = &soundmicR[1][0];
	 //HAL_GPIO_WritePin(GPIOB, V_TX_Pin, 0);
	 osThreadFlagsSet(Task_dspHandle,0x2);
//	if (ads127l11_notification_semaphoreHandle != NULL) {
//		osSemaphoreRelease(ads127l11_notification_semaphoreHandle);
//	}
}

//static void HAL_TransferHalfStop(DMA_HandleTypeDef *hdma)
void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
	 pcm_ptr = &soundmicR[0][0];
	 //HAL_GPIO_WritePin(GPIOB, V_TX_Pin, 0);
	 osThreadFlagsSet(Task_dspHandle,0x1);
//	if (ads127l11_notification_semaphoreHandle != NULL) {
//		osSemaphoreRelease(ads127l11_notification_semaphoreHandle);
//	}
}

void soundf(void)
{
	uint32_t ns = 0;
	uint32_t bw;
	FRESULT res;
	size_t size;

	int32_t sig;
	uint8_t* sig_bytes = &sig;


	for (int j = 0; j < SLM_BUFLEN; j++)	//kt
	{
		//sig_bytes[3] = pcm_ptr[j*3];
		//sig_bytes[2] = pcm_ptr[j*3+1];
		//sig_bytes[1] = pcm_ptr[j*3+2];
		sig_bytes[2] = pcm_ptr[j*3];
		sig_bytes[1] = pcm_ptr[j*3+1];
		sig_bytes[0] = pcm_ptr[j*3+2];
		if(sig_bytes[2] & 0x80) sig_bytes[3] = 0xFF;
		else  sig_bytes[3] = 0;

		soundTest[j] = ( int16_t)(sig >> 8); //16); //sound_rx_buff[j];  //Sameer; 3rd July 2024
		sound_rx_buff[j] = sig;
		//g_algo.SLMStruct.SLMfeedin[j] = (float32_t)(sig);
//		if(j == 0)
//		{
//			mytrg1 = (float32_t)(sig);
//			mytrg1 = mytrg1/256.0;
//			mytrg2 = (float32_t)(( int16_t)(sig >> 8));//16));
//		}

		//g_algo.SLMStruct.SLMfeedinS[j] = ( int16_t)(sig >> 8);//16);
		g_algo.SLMStruct.SLMfeedin[j] = ((float32_t)(sig))*0.00390625f;
	}

	if (isRecToFile)
	{

		if (g_demo_parameters.opt_rec.file_type==0 || g_demo_parameters.opt_rec.file_type==2)//to wav, wav+txt
		{
		//if(dat_kol < 10){

			if (file_wave1ch.obj.fs != 0)
			{

				ns = 0;
				size = SLM_BUFLEN*2;
				while (size > ns) {
					res = f_write(&file_wave1ch, soundTest+ns,size-ns, (void *)&bw);// pcm_ptr+ns, size-ns, (void *)&bw);
					if(res != FR_OK) break; // Alex, check
					ns += bw;
				}
				wavsize1ch += ns;
				f_sync(&file_wave1ch);
			}

		//}
		}
		else //txt file only
		{
			wavsize1ch=wavsize1ch+2400*2; // size in bite
			//wavsizeh += 164*2;
		}


	}

}

void vibro(void)
{

//	int16_t tch1,tch2;
	uint32_t nbrb, kv; // ks,kv1,kv2,kv3;
	FRESULT res;
	uint32_t bw;
	uint32_t nn = 0;
	size_t size;
	uint32_t dat_kol = 0;
	float fDelayForContRec=0.0f;

	nbrb = ping_pong ^ 1;
	//kv1 = 0; kv2 = 0; kv3 = 0;
	//ks = 0;


	//save to wave file:
	if (isRecToFile)
	{

		if (g_demo_parameters.opt_rec.file_type==0 || g_demo_parameters.opt_rec.file_type==2)//to wav, wav+txt
		{
/*
			if (file_test3ch.obj.fs != 0)
			{

				nn = 0;
				size = 164*3*4;//164*6;  // bytes = 3short * 2
				//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 1);
				while (size > nn) {
					res = f_write(&file_test3ch, &test3ch_buff[nbrb] + nn, size-nn, (void *)&bw);// Write the data to the file
					if(res != FR_OK) break; // Alex, check
					nn += bw;
				}

				f_sync(&file_test3ch);

				//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 0);
			}
*/
		//if(dat_kol < 10){
			if (file_wave3ch.obj.fs != 0)
			{
				kv=0; //ak1=-32760.0; ak2=32760.0; ak3=-32760.0;
				for (int j = 0; j < 164; j++)
				{
					sd_rx_buff[kv++] = ( int16_t)(ch1_samples[nbrb][j] >> 8);
					sd_rx_buff[kv++] = ( int16_t)(ch2_samples[nbrb][j] >> 8);
					sd_rx_buff[kv++] = ( int16_t)(ch3_samples[nbrb][j] >> 8);
				}

				nn = 0;
				size = 164*3*2;//164*6;  // bytes = 3short * 2
				//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 1);
				while (size > nn) {
					res = f_write(&file_wave3ch, sd_rx_buff+nn, size-nn, (void *)&bw);// Write the data to the file
					if(res != FR_OK) break; // Alex, check
					nn += bw;
				}
				wavsize3ch += nn;
				f_sync(&file_wave3ch);
				dat_kol++;
				//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 0);
			}
		//}
		}
		else //txt file only
		{
			wavsize3ch=wavsize3ch+164*3*2; // size in bite
			//wavsizeh += 164*2;
		}

		file_duration= wavsize3ch/(164*3*2)/10;  // in sec
		if (file_duration > old_file_duration)
		{
			draw_rec_time(file_duration);
		}
		old_file_duration=file_duration;

	}
	else if (g_demo_parameters.opt_rec.nRecordMode==2 && (!isRecToFile))  //to calculate delay in cont mode
	{
			wavsize3ch=wavsize3ch+164*3*2;
			file_duration= wavsize3ch/(164*3*2)/10;

	}

	// ----------------------------------------  ALGO PROCESSING AND SCREEN REFRESH -----------------------
	{

		redraw_lcd++;
		short isTrig=0;
		switch (g_demo_parameters.nMeasMode)
		{
			case 0:  //EVS
			if (get_special_mode_status(DEMO_MODE_EVS))
			{
				// sound signal for algo
	/*						for (int j = 0; j < SLM_BUFLEN; j++)	//kt
				{
					g_algo.SLMStruct.SLMfeedin[j] = (short)pcm_ptr[j]; //sound_rx_buff[j];  //Sameer; 3rd July 2024
				}
	*/
					//vibro raw data for algo
					for (int j = 0; j < EVS_BUFLEN; j++) // algo supports EVS_BUFLEN(300)
					{
						g_algo.EVSStruct.input[0][j] = ((float32_t)(ch1_samples[nbrb][j]))*0.00390625f;
						g_algo.EVSStruct.input[1][j] = ((float32_t)(ch2_samples[nbrb][j]))*0.00390625f;
						g_algo.EVSStruct.input[2][j] = ((float32_t)(ch3_samples[nbrb][j]))*0.00390625f;

					}

					EVSProcess(&(g_algo.EVSStruct), &(g_algo.SLMStruct), &(g_algo.SLMCaliStruct), &(g_algo.EVSOUTStruct));  //EVS Algo call per frame
					//isTrig= isTrig+ (short)g_algo.EVSOUTStruct.VibrationTriggerFound;
					isTrig= isTrig+ (short)g_algo.EVSOUTStruct.VibrationTriggerFound;   + (short)g_algo.EVSOUTStruct.SoundTriggerFound;  //Sameer: 26th June, 2025

				if (redraw_lcd >= 5)
				{
					flag_lcd_update=0; //redraw lcd		//draw_evs_update((float)time_counter, g_algo.EVSOUTStruct); //draw_leq();
					redraw_lcd = 0;
				}

			}
			break;

			case 1:	//BLS
			if (get_special_mode_status(DEMO_MODE_BLAST))
			{
				////// sound signal for algo
	/*						for (int j = 0; j < SLM_BUFLEN; j++)
				{
					g_algo.SLMStruct.SLMfeedin[j] = (short)sound_rx_buff[j];  //Sameer; 3rd July 2024
				}
				////
	*/
					//vibro raw data for algo
					for (int j = 0; j < BLS_BUFLEN; j++) // algo supports EVS_BUFLEN(300)
					{
						g_algo.BLSStruct.input[0][j] = ((float32_t)(ch1_samples[nbrb][j]))*0.00390625f;
						g_algo.BLSStruct.input[1][j] = ((float32_t)(ch2_samples[nbrb][j]))*0.00390625f;
						g_algo.BLSStruct.input[2][j] = ((float32_t)(ch3_samples[nbrb][j]))*0.00390625f;
					}

					BLSProcess(&(g_algo.BLSStruct), &(g_algo.SLMStruct), &(g_algo.SLMCaliStruct), &(g_algo.BLSOUTStruct ));  //BLS Alog call per frame
					//isTrig= isTrig+ (short)g_algo.BLSOUTStruct.VibrationTriggerFound;
					isTrig= isTrig+ (short)g_algo.BLSOUTStruct.VibrationTriggerFound + (short)g_algo.BLSOUTStruct.SoundTriggerFound;  //Sameer: 26th June, 2025

				if (redraw_lcd >= 5)
				{
					flag_lcd_update=1; //redraw lcd    //draw_blast_update((float)time_counter, g_algo.BLSOUTStruct); //draw_leq();
					redraw_lcd = 0;
				}

			}
			break;
		}

		//------------trigger------------------
		if ((g_demo_parameters.opt_rec.nRecordMode==1 || g_demo_parameters.opt_rec.nRecordMode==2) &&  //Single or Cont modes
			isRecPress &&		//Rec mode
			isTrig>0 &&			//trigger event
			!isRecToFile)		//still no recording to files
		{
			if ( (g_demo_parameters.opt_rec.nRecordMode==2) && (file_duration < fDelayForContRec)) //for cont mode additionally it need to wait delay
			;
			else
			{
				if (create_files()!=FR_OK)
				{
					flag_arm_rec=0;
					isRecToFile=0;
				}
				else
				{
					isRecToFile=1;
					file_duration=0;
					isTrig=0;
					if (g_demo_parameters.nMeasMode==1)			g_algo.BLSStruct.TriggerFoundResetAlgo	=1;
					else if (g_demo_parameters.nMeasMode==0)	g_algo.EVSStruct.TriggerFoundResetAlgo	=1;
				}

			}
		}

		//--------------stop by timer:--------------------
		if (isRecToFile )
		{
			//pio_toggle_pin(LED0_GPIO);
			if (g_demo_parameters.opt_rec.nRecordMode==0 && g_demo_parameters.opt_rec.timer>0  )  // Manual Mode
			{
				if (file_duration>=g_demo_parameters.opt_rec.timer)
				{
					flag_arm_rec=0;
				}
			}
			if (g_demo_parameters.opt_rec.nRecordMode==1  ) //Single Rec mode
			{
				if (file_duration>=g_demo_parameters.opt_trig[g_demo_parameters.nMeasMode].fPostTrigTime)
				{
					flag_arm_rec=0;
				}
			}
			if ( g_demo_parameters.opt_rec.nRecordMode==2  ) //Cont Rec mode
			{
				if (file_duration>=g_demo_parameters.opt_trig[g_demo_parameters.nMeasMode].fPostTrigTime)
				{
					//close current files by timer
					if (g_demo_parameters.opt_rec.file_type==0 || g_demo_parameters.opt_rec.file_type==2)
						close_wave_file();
					if (g_demo_parameters.opt_rec.file_type==1 || g_demo_parameters.opt_rec.file_type==2)
						close_txt_file( (float)wavsize3ch/(float)(1634*3*2));  //pop
					isRecToFile=0;
					draw_file_name = 2; // ("");//duplicate for guarantee

					//to prevent full stop in the circle' begin
					flag_arm_rec=1;
					flag_arm_rec_old=0;

					//in Cont mode after first recorded file it needs to make pause in fDelayForContRec (sec)
					//and start to wait trigger
					fDelayForContRec = (float)(g_demo_parameters.opt_trig[g_demo_parameters.nMeasMode].nDelay);
					wavsize1ch = 0; wavsize3ch = 0;
					file_duration = 0;
				}
			}
		}
		//-----------------------  Calibration  ----------------------------------:  //kt25
		//
		//int shift=0;
		//kt int16_t* addr=sd_rx_buff[0]; //for convenient use because sd_rx_buff is 2-dimension array
		if (get_special_mode_status(DEMO_OPT_TRANSD) && g_flagCalibr==1)
		{
			///for (int n=0;n<10;n++) //1 input buffer=1sec, 1 buffer for Calibration = 0.1s =>10 times to go through full beffer
			///{

			if (g_flagCalibrChan==3)  //sound
			{
				// sound signal for algo in sound_rx_buff	//kt
				//Call the calibration
  			SLMCalibrationByChannel(&(g_algo.SLMCaliStruct), soundTest); // sound_rx_buff);
  			//HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);

				if (g_algo.SLMCaliStruct.nCaliOver)
				{
					g_flagCalibr=0;
					///n=10;
				}
			}

			else
			{
				//vibro raw data for calibration
				if(g_flagCalibrChan == 0){
				for (int j = 0; j < EVS_BUFLEN; j++) //!!!
				  {
					m_data_calibr[g_flagCalibrChan][j] = ( int16_t)(ch1_samples[nbrb][j]>>8);	//kt
				  }
				}
				else if(g_flagCalibrChan == 1){
				for (int j = 0; j < EVS_BUFLEN; j++) //!!!
				  {
					m_data_calibr[g_flagCalibrChan][j] = ( int16_t)(ch2_samples[nbrb][j]>>8);	//kt
				  }
				}
				else if(g_flagCalibrChan == 2){
				for (int j = 0; j < EVS_BUFLEN; j++) //!!!
				  {
					m_data_calibr[g_flagCalibrChan][j] = ( int16_t)(ch3_samples[nbrb][j]>>8);	//kt
				  }
				}

				VLMCalibrationByChannel(&(g_algo.VLMCaliStruct), m_data_calibr[g_flagCalibrChan], g_flagCalibrChan);

				if (g_algo.VLMCaliStruct.nCaliOver[g_flagCalibrChan])
				{
					g_flagCalibr=0;
					///n=10;
				}
			}
			///}//end for
		} //end calibration


	}//end algo processing
}







