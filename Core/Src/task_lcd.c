#include "main.h"
#include "cmsis_os.h"
#include "ILI9488.h"
//#include "dma_pr.h"
//#include "widgets.h"
#include "parameters.h"
#include <string.h>
#include "printf.h"
//#include "cli_usart.h"


uint32_t mounted = 0;
uint32_t ffirst = 1;
uint32_t bluescreen = 0;
uint32_t flag_ftp_update = 0;
uint32_t flag_ftp_start = 0;
char tempftp[68];
int draw_file_name = 0;
//char cnt_file_name[32];

extern RTC_HandleTypeDef hrtc;

bq2588x_t bqv;
extern uint32_t init_bq25883(void);
extern void bq2588x_update_status(bq2588x_t *bq);

///extern freertos_twihs2_if freertos_twihs_master_init_twi2(Twihs *p_twihs, uint32_t interrupt_priority);

struct win_attributes  win_attributes_root;
uint32_t demo_spcial_mode[DEMO_SPEC_MODE_MAX];  //kt

/** Notification queue. */
//xQueueHandle queue_pointer_event;

/* Interrupts that get enabled to catch error conditions on the bus.  The
 * overrun error is not enabled as overruns on Rx are expected while sending. */
//#define SPI_SR_ERROR_INTERRUPTS                         (SPI_SR_MODF)
//#define SPI_IER_ERROR_INTERRUPTS                        (SPI_IER_MODF)


/** XDMAC channel HW Interface number for SPI0 transmit,refer to datasheet. */
//efine SPI0_XDMAC_TX_CH_NUM  1
/** XDMAC channel configuration. */
//atic xdmac_channel_config_t xdmac_tx_cfg;

/* Used to latch errors found during the execution of the example. */
//static uint32_t error_detected = pdFALSE;

/*
void freertos_fmc_if_init(void);
status_code_t freertos_spi_write_packet(const uint8_t *data, size_t len, portTickType block_time_ticks);
status_code_t freertos_optionally_wait_transfer_completion(	xSemaphoreHandle notification_semaphore, portTickType max_block_time_ticks);
void freertos_start_dma_transfer(const uint8_t *data, size_t len);
*/
static void win_process_events(void);
void draw_battery(bool blink, uint32_t batt_counter);  //kt

int battery_level_previous; //kt
#define BATT_PLACE_X	442 //kt
#define BATT_PLACE_Y	5   //kt

#define BATT_LEVEL_0  6200			// 6200 mV
#define BATT_LEVEL_1  6800			// 6800 mV
#define BATT_LEVEL_2  7400			// 7400 mV
#define BATT_LEVEL_3  8000			// 8000 mV


//const char* ftpstr = " wait connection ";
extern char cnt_file_name[32];
extern char txt_file_name[32];
//uint32_t dsp_start_event=0;
uint32_t ftp_toggle1=0;



/*
 * 
 * A SPI DMA interrupt handler that is called for ili9488.
 */
///void local_spi_dma_handler(void)
///{
///	portBASE_TYPE higher_priority_task_woken = pdFALSE;	
		/* If the driver is supporting multi-threading, then return the access mutex. */
		//if (tx_dma_control[spi_index].peripheral_access_sem != NULL) {
			//xSemaphoreGiveFromISR(
					//tx_dma_control[spi_index].peripheral_access_sem,
					//&higher_priority_task_woken);
		//}

		/* if the sending task supplied a notification semaphore, then
		 * notify the task that the transmission has completed. */
///		if (spi_write_notification_semaphore != NULL) {
///			xSemaphoreGiveFromISR(
///					spi_write_notification_semaphore,
///					&higher_priority_task_woken);
///		}

	//if ((spi_status & SR_ERROR_INTERRUPTS) != 0) {
		///* An mode error occurred in either a transmission or reception.  Abort.
		//Stop the transmission, disable interrupts used by the peripheral, and
		//ensure the peripheral access mutex is made available to tasks.  As this
		//peripheral is half duplex, only the Tx peripheral access mutex exits. */
		//spi_disable_interrupt(spi_port, SPI_IDR_ENDTX);
		//spi_disable_interrupt(spi_port, SPI_IDR_ENDRX);

		//if (tx_dma_control[spi_index].peripheral_access_sem != NULL) {
			//xSemaphoreGiveFromISR(
					//tx_dma_control[spi_index].peripheral_access_sem,
					//&higher_priority_task_woken);
		//}

		///* The SPI port will have been disabled, re-enable it. */
		//spi_enable(spi_port);
	//}
		/* If giving a semaphore caused a task to unblock, and the unblocked task
		has a priority equal to or higher than the currently running task (the task
		this ISR interrupted), then higher_priority_task_woken will have
		automatically been set to pdTRUE within the semaphore function.
		portEND_SWITCHING_ISR() will then ensure that this ISR returns directly to
		the higher priority unblocked task. */
///		portEND_SWITCHING_ISR(higher_priority_task_woken);	

///}

/*-----------------------------------------------------------*/
// void freertos_start_dma_transfer(const uint8_t *data, size_t len)
// {
	// uint32_t xdmaint;
	
	// /* Ensure the notification semaphore starts in the expected state in case
	// the previous DMA transfer didn't complete for any reason. */
	// if (spi_write_notification_semaphore !=	NULL) {
		// xSemaphoreTake(spi_write_notification_semaphore,0);
	// }
	// SCB_CleanDCache_by_Addr((uint32_t*)data, len);

		// xdmaint = (XDMAC_CIE_BIE |
		// XDMAC_CIE_DIE   |
		// XDMAC_CIE_FIE   |
		// XDMAC_CIE_RBIE  |
		// XDMAC_CIE_WBIE  |
		// XDMAC_CIE_ROIE);

		// /* Initialize channel config for transmitter */
		// xdmac_tx_cfg.mbr_ubc = (uint32_t) len;
		// xdmac_tx_cfg.mbr_sa = (uint32_t) data;
		// xdmac_tx_cfg.mbr_da = (uint32_t)&(SPI_MASTER_BASE->SPI_TDR);
		// xdmac_tx_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
		// XDMAC_CC_MBSIZE_SINGLE |
		// XDMAC_CC_DSYNC_MEM2PER |
		// XDMAC_CC_CSIZE_CHK_1 |
		// XDMAC_CC_DWIDTH_BYTE |
		// XDMAC_CC_SIF_AHB_IF0 |
		// XDMAC_CC_DIF_AHB_IF1 |
		// XDMAC_CC_SAM_INCREMENTED_AM |
		// XDMAC_CC_DAM_FIXED_AM |
		// XDMAC_CC_PERID(SPI0_XDMAC_TX_CH_NUM);

		// xdmac_tx_cfg.mbr_bc = 0;
		// xdmac_tx_cfg.mbr_ds =  0;
		// xdmac_tx_cfg.mbr_sus = 0;
		// xdmac_tx_cfg.mbr_dus = 0;

		// xdmac_configure_transfer(XDMAC, XDMAC_SPITX_CH, &xdmac_tx_cfg);

		// xdmac_channel_set_descriptor_control(XDMAC, XDMAC_SPITX_CH, 0);
		// xdmac_channel_enable_interrupt(XDMAC, XDMAC_SPITX_CH, xdmaint);
		// xdmac_channel_enable(XDMAC, XDMAC_SPITX_CH);
		// xdmac_enable_interrupt(XDMAC, XDMAC_SPITX_CH);	

// }	
				
/*-----------------------------------------------------------*/
/*
 * For internal use only.
 * If notification_semaphore is not NULL then the user has supplied their own
 * notification semaphore, indicating that they do not wish to wait for the end
 * of the PDC transfer.  In that case, this function will exit immediately and
 * always return STATUS_OK.
 * If notification_semaphore is NULL, but there is a semaphore associated with
 * the PDC, then the driver is using its own internal notification semaphore and
 * this function should not return until either the semaphore is 'taken', or
 * max_block_time_ticks RTOS ticks have passed.  STATUS_OK is returned if the
 * semaphore is taken.  ERR_TIMEOUT is returned if the wait for the semaphore
 * timed out.
 */
// status_code_t freertos_optionally_wait_transfer_completion(	xSemaphoreHandle notification_semaphore, portTickType max_block_time_ticks)
// {
	// status_code_t return_value = STATUS_OK;

	// if (notification_semaphore == NULL) {
		// if (spi_write_notification_semaphore !=	NULL) {
			// /* Wait until notified by the ISR that transmission is
			// complete. */
			// if (xSemaphoreTake(spi_write_notification_semaphore,
					// max_block_time_ticks) != pdPASS) {
				// return_value = ERR_TIMEOUT;
				// //ioport_toggle_pin_level(LED0_GPIO);
			// }
			// //ioport_toggle_pin_level(LED1_GPIO);
		// }
	// }

	// return return_value;
// }


// status_code_t freertos_fmc_write_packet(const uint8_t *data, size_t len, portTickType block_time_ticks)
// {
//	 status_code_t return_value;

	// freertos_start_dma_transfer(data, len);
	
	// /* Catch the end of transmission so the access mutex can be
	// returned, and the task notified (if it supplied a notification
	// semaphore).  The DMA interrupt can be enabled here  */

//	 return_value = freertos_optionally_wait_transfer_completion(
//			 NULL,
//			 block_time_ticks);
			
					
	// return return_value;
// }


// static void ili_spi_write(uint8_t *buffer, uint32_t number_of_bytes)
// {
	
	// /* The blocking API is being used.  Other tasks will execute while the
	// SPI write is in progress. */
	// if (freertos_spi_write_packet(buffer, number_of_bytes,
	// 200UL / portTICK_RATE_MS) != STATUS_OK) {
		// error_detected = pdTRUE;
		// //ioport_set_pin_level(LED0_GPIO, LED0_INACTIVE_LEVEL);
	// }
// }


/*-----------------------------------------------------------*/


///freertos_fmc_if freertos_fmc_init(void)
///{

////	freertos_spi_if return_value;
	
	/* Configure SPI as master, set up SPI clock. */
////	spi_master_initialize();


	/* Create any required peripheral access mutexes and transaction complete
	semaphores.  This peripheral is half duplex so only a single access
	mutex is required. */
		
	/* If the tx driver is to be thread aware then create an access control
	semaphore.  An Rx access mutex is not created in this function as half duplex
	peripherals need only use a single access mutex, and the Tx semaphore is used
	for the purpose.  Full duplex peripherals have extra configuration steps
	that are performed separately. 
	if ((options_flags & USE_TX_ACCESS_SEM) != 0) {
		vSemaphoreCreateBinary(
				tx_dma_control->peripheral_access_sem);
		configASSERT(tx_dma_control->peripheral_access_sem);
	*/	
		/* Ensure the binary semaphore starts with equal to 1
		xSemaphoreGive(tx_dma_control->peripheral_access_sem);
	} */

	/* If the transmit function is only going to return once the transmit is
	complete then create an internal notification semaphore.
	if ((options_flags & WAIT_TX_COMPLETE) != 0) {
		vSemaphoreCreateBinary(
				tx_dma_control->transaction_complete_notification_semaphore);
		configASSERT(
				tx_dma_control->transaction_complete_notification_semaphore);
	*/
		/* Ensure the semaphore starts in the expected state.  A block time
		of zero can be used here as the semaphore is guaranteed to be
		available (it has only just been created).
		xSemaphoreTake(
				tx_dma_control->transaction_complete_notification_semaphore,
				0);
	*/
////		vSemaphoreCreateBinary(spi_write_notification_semaphore);
////		configASSERT(spi_write_notification_semaphore);
////		xSemaphoreTake(spi_write_notification_semaphore, 0);

	/* Error interrupts are always enabled. */
////	spi_enable_interrupt(SPI_MASTER_BASE, SPI_IER_ERROR_INTERRUPTS);		

	/* Configure and enable the TWIHS interrupt in the interrupt controller. */
	//configure_interrupt_controller(

///		NVIC_ClearPendingIRQ(SPI0_IRQn);
///		NVIC_SetPriority(SPI0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+2);
///		NVIC_EnableIRQ(SPI0_IRQn);
		
///		spi_enable(SPI_MASTER_BASE);
		
	/* \typedef freertos_twihs_if
	* \brief Type returned from a call to freertos_twihs_master_init(), and then used
	* to reference a TWIHS port in calls to FreeRTOS peripheral control functions.
	*/		
///	return_value = (freertos_spi_if)p_spi;				
			
///	return return_value;

///}
/*-----------------------------------------------------------*/



void DrawSD(void)
{
	if (mounted){
		ILI9488_DrawImage(410, 4, 22, 22, SD_22x22_YES);
	}
	else
		ILI9488_DrawImage(410/*380*/, 4, 22, 22, SD_22x22_NO);
}

void draw_rec_time(int sec)
{
	RgbStr clr2 = {Red,White};
	if (sec>0)
	{
		//char text[32];
		char text[32] = {1,2,3,4,5};
		snprintf_(text, 16, "%5d sec", sec);

		ILI9488_WriteString(370, 270 , text, Font_11x18, &clr2);
	}
	else
		ILI9488_FillRectangle(370, 270, 98, 20, White);
}

void set_rec_button(void)
{
	RgbStr clr2 = {Red,White};
	RgbStr clr = {Black,White};
	if (flag_arm_rec)
	{

		ILI9488_WriteString(300, 2, "REC", Font_16x26, &clr2);
		if (g_demo_parameters.opt_rec.nRecordMode==1)
		{
			ILI9488_WriteString(300+48+3, 2+8, "s", Font_11x18, &clr2);
		}
		else if (g_demo_parameters.opt_rec.nRecordMode==2)
		{
			ILI9488_WriteString(300+48+3, 2+8, "c", Font_11x18, &clr2);
		}
	}
	else
	{

		ILI9488_WriteString(300, 2, "    ", Font_16x26, &clr);
	}
}

void refresh_system_time()
{
	 //int ret=10;
		RTC_TimeTypeDef stTimest = {0};
		RTC_DateTypeDef stDatest = {0};

		HAL_RTC_GetTime(&hrtc, &stTimest, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &stDatest, RTC_FORMAT_BIN);
	// struct DateTime dt_sip;
	// if(xSemaphoreTake(timeMutex, 10)==pdTRUE) {
		// ret = read_time_date(&dt_sip.hour, &dt_sip.minute, &dt_sip.second, &dt_sip.year, &dt_sip.month, &dt_sip.day, &dt_sip.weekday, NULL);
		// xSemaphoreGive(timeMutex);
	// } else {
		// //failed to lock mutex
		// return;		
	// }
	
	// if(ret == 0)
	// {
		 RgbStr clr = {Black,White};
		 char text[32];
		 snprintf_(text, 32, "   %02lu:%02lu", stTimest.Hours, stTimest.Minutes); //redraw time
		 ILI9488_WriteString(408, 296, text, Font_7x10, &clr);
		
		 {
			 snprintf_(text, 32, "%04lu-%02lu-%02lu", (stDatest.Year + 2000), stDatest.Month, stDatest.Date);
			 ILI9488_WriteString(408, 306, text, Font_7x10, &clr);
		 }
	// }



}

void draw_battery(bool blink, uint32_t batt_counter) //kt
{
	uint32_t status; //kt
	int level2;
	int battery_level; //kt : 0-4


	//if (batt_counter > 5)
	//{
		status = 1; 

		
		if (status & 0x1u)
		{
			bq2588x_update_status(&bqv);
			level2 = bqv.vbat_volt;

			
			if (level2 < BATT_LEVEL_0)  battery_level=0;
			else if ((level2 >= BATT_LEVEL_0) && (level2 < BATT_LEVEL_1)) battery_level=1;
			else if ((level2 >= BATT_LEVEL_1) && (level2 < BATT_LEVEL_2)) battery_level=2;
			else if ((level2 >= BATT_LEVEL_2) && (level2 < BATT_LEVEL_3)) battery_level=3;
			else if (level2 >= BATT_LEVEL_3) battery_level=4;
			else battery_level=0;
			
			if (battery_level!=battery_level_previous) //redraw if it's changed
			{
				switch (battery_level)
				{
					case 0:	ILI9488_DrawImage(BATT_PLACE_X, BATT_PLACE_Y, 36, 20, Bat_36x20_0); break;
					case 1:	ILI9488_DrawImage(BATT_PLACE_X, BATT_PLACE_Y, 36, 20, Bat_36x20_1); break;
					case 2:	ILI9488_DrawImage(BATT_PLACE_X, BATT_PLACE_Y, 36, 20, Bat_36x20_2); break;
					case 3:	ILI9488_DrawImage(BATT_PLACE_X, BATT_PLACE_Y, 36, 20, Bat_36x20_3); break;
					case 4:	ILI9488_DrawImage(BATT_PLACE_X, BATT_PLACE_Y, 36, 20, Bat_36x20_4); break;
					default:ILI9488_DrawImage(BATT_PLACE_X, BATT_PLACE_Y, 36, 20, Bat_36x20_0); break;
				}
			}
			battery_level_previous=battery_level;
			if (level2 <  6000) power_off_mode(); // 2*3000 mV

		}
	//}

}


// void create_lcd_task(void)
// {
	// static freertos_twihs0_if freertos_twi2;
	// uint32_t prb;

	// /* Initialize the FreeRTOS driver for blocking operation.  */
	// freertos_fmc_init();

	// /* Check the port was initialized successfully. */
	// //configASSERT(freertos_fmc);
	
	// /* The queue is created before scheduler started */
	// //queue_pointer_event = xQueueCreate(10, sizeof (struct win_pointer_event) );
	
	// /* Initialize the FreeRTOS driver for blocking operation.  */
	// freertos_twi2 = freertos_twihs_master_init_twi2(TWIHS2, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+2);
	
	// /* Check the port was initialized successfully. */
	// //configASSERT(freertos_twi2);
	
	// prb = init_bq25883();	

	// /* Create the task as described above. */
	// //xTaskCreate(spi_lcd_task, (const signed char *const) "LCD",
			// //stack_depth_words, (void *) freertos_spi, task_priority,
			// //&xlcd_task);
// }


void StartTask02(void *pvParameters)
{	
	UNUSED(pvParameters);
	static uint32_t batt_counter, time_counter;	
	bool blink=true;  //kt 
	//struct DateTime dt_sip;
	//uint32_t previous_day=0;
	bool bSdMountedTemp=mounted;
	char text[32] = {1,2,3};
	RgbStr clr = {Black,White};
	RgbStr clr2 = {Red,White};

	//int ret;
	
	ILI9488_Init();
	ILI9488_FillRectangle(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT, White);
	ILI9488_FillRectangle(0, 30, ILI9488_WIDTH, 2, Black); //lines
	ILI9488_FillRectangle(0, 290, ILI9488_WIDTH, 2, Black);
	
	create_battery_icons();
	ILI9488_DrawImage(BATT_PLACE_X, BATT_PLACE_Y, 36, 20, Bat_36x20_0);
	//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 1);
	battery_level_previous=0;

	create_SD_icons();
	DrawSD();
	//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 1);
	
	snprintf_(text,16,"[Vers: %s]",g_demo_parameters.vers_sf);

	ILI9488_WriteString(4, 306, text, Font_7x10, &clr);
	//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 1);

	HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, 1);

	///osDelay(10);
	switch (g_demo_parameters.nMeasMode)
	{
		case 0: //EVS
				set_special_mode_status(DEMO_MODE_EVS, 1); //kt
				app_widget_launch_evs(); //kt
				//HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 1);
				break;
		case 1:	//BLAST
				set_special_mode_status(DEMO_MODE_BLAST, 1); //kt
				//app_widget_launch_blast(); //kt
				break;
	}


	batt_counter = 600;
	time_counter = 700;
	int fin = 0;

	for(;;)
	{

		if (bluescreen)
		{
			RgbStr clr3 = {Black,Blue};
			if(fin == 0)
			{
				fin = 1;
				ILI9488_FillRectangle(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT, Blue);
				ILI9488_WriteString(150, 160, "Shutting down ...", Font_11x18, &clr3);
			}
						
		} 
		else
		{
			// ----------------------------------------------------------
			win_process_events(); // non block 
				
			//  ------------------  BATTERY refresh: -----------------------
			blink = !blink;

			if (batt_counter++ > 500){
				batt_counter=0;
				draw_battery(blink, batt_counter);
			}
		
			//  ----------------- TIME  REFRESH: -----------------------
			if (time_counter++ > 600)
			{
				time_counter = 0;
			
				refresh_system_time();
			} 
		
			//SD draw, but currently doesn't check SD state continuously (only once after app' start)
			if (bSdMountedTemp!=mounted)
			{
				DrawSD();
				bSdMountedTemp=mounted;
			}
		
			// ----------------------------------------  SCREEN REFRESH after ALGO PROCESSING -----------------------	
			switch (g_demo_parameters.nMeasMode)
			{
				case 0:  //EVS
					if (get_special_mode_status(DEMO_MODE_EVS) && flag_lcd_update==0)
					{			
	 					draw_evs_update((float)time_counter, g_algo.EVSOUTStruct); //draw_leq();
						flag_lcd_update=-1;
					}
					break;
				case 1:	//BLS
					if (get_special_mode_status(DEMO_MODE_BLAST) && flag_lcd_update==1)
					{
	 					draw_blast_update((float)time_counter, g_algo.BLSOUTStruct); //draw_leq();
						flag_lcd_update=-1;
					}
					break;
			}
		
			// ------------------- DEMO_SEND_FILE ------------------------------
			if (get_special_mode_status(DEMO_SEND_FILE))
			{
				
				RgbStr clr = {Black,White};
				if (flag_ftp_update==2)
				{
					ILI9488_WriteString((90 + ftp_toggle1*11), 242, ".", Font_11x18, &clr);
					if (ftp_toggle1 < 32) ftp_toggle1++;
					flag_ftp_update=0;
				}

				else if (flag_ftp_update==1)
				{
					ILI9488_FillRectangle(90, 242, 480, 20, White);
					ILI9488_WriteString(90, 242, tempftp, Font_7x10, &clr);
					flag_ftp_update=0;
				}
				else if (flag_ftp_update==3)
				{
					ILI9488_FillRectangle(90, 242, 380, 20, White);
					ILI9488_WriteString(90, 242, "DONE", Font_11x18, &clr);
					flag_ftp_update=0;
				}

			}

			// ------------------- DRAW_FILE_NAME ------------------------------
			if (draw_file_name==1)
			{
				draw_file_name = 0;
				
				ILI9488_WriteString(8, 270 , cnt_file_name, Font_11x18, &clr2);
			
			}
			if (draw_file_name==3)
			{
				draw_file_name = 0;

				ILI9488_WriteString(8, 270 , txt_file_name, Font_11x18, &clr2);

			}
			if (draw_file_name==2)
			{
				draw_file_name = 0;
				ILI9488_FillRectangle(8, 270, 250, 18, White);
			}	
			// ------------------- DRAW_REC_BUTTON ------------------------------	
			
			set_rec_button();
		}
		osDelay(100);
	}
}
/*-----------------------------------------------------------*/
//__attribute__((__section__(".code_TCM")))
static void win_process_events(void)
{
	win_pointer_event_t event;
	win_event_handler_t handler = win_attributes_root.event_handler;
	
	while (osMessageQueueGet(queue_pointer_eventHandle, &event, NULL, 0) == osOK) {  //xQueueReceive(queue_pointer_event, &event, 0) == pdPASS) {
		if (handler) {
			handler(&event);
		}
	}
}


//kt  - next added:
/**
 * \brief Set special mode status.
 * \param id Special mode ID.
 * \param status special mode status.
 */
//__attribute__((__section__(".code_TCM"))) 
void set_special_mode_status( demo_special_mode_id id, uint32_t status )
{
	demo_spcial_mode[id] = status;
}

/**
 * \brief Get if the specific window is active.
 * \param id Special mode ID.
 * \return Special mode status.
 */
//__attribute__((__section__(".code_TCM"))) 
uint32_t get_special_mode_status( demo_special_mode_id id )
{
	return demo_spcial_mode[id];
}
