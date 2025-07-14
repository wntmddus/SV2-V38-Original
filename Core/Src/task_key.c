/**
 * \file
 *
 */

#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include "parameters.h"
//#include "sd.h"


//uint32_t bluescreen;

//uint32_t buttonlongpress;
//uint32_t buttonpresstime;

//void StartTask03(void *pvParameters);

//volatile uint32_t g_button_event = 0;
//volatile portTickType btn_down_time = 0;
volatile portTickType btn_down_time = 0;
void power_off_mode(void);
uint32_t  pwr_bt_first;


/*
F1 4
F2 5
F4 6
F5 13
RIGHT 1
UP 5
ENT 6
LEFT 7
DOWN 4
REC 3
*/

/* F1 */
uint8_t pres_F1;
uint8_t prev_F1;
uint8_t cnt_F1;
/* F2 */
uint8_t pres_F2;
uint8_t prev_F2;
uint8_t cnt_F2;
/* F4 */
uint8_t pres_F4;
uint8_t prev_F4;
uint8_t cnt_F4;
/* F5 */
uint8_t pres_F5;
uint8_t prev_F5;
uint8_t cnt_F5;
/* RIGHT */
uint8_t pres_RIGHT;
uint8_t prev_RIGHT;
uint8_t cnt_RIGHT;
/* UP */
uint8_t pres_UP;
uint8_t prev_UP;
uint8_t cnt_UP;
/* ENT */
uint8_t pres_ENT;
uint8_t prev_ENT;
uint8_t cnt_ENT;
/* LEFT */
uint8_t pres_LEFT;
uint8_t prev_LEFT;
uint8_t cnt_LEFT;
/* DOWN */
uint8_t pres_DOWN;
uint8_t prev_DOWN;
uint8_t cnt_DOWN;
/* REC */
uint8_t pres_REC;
uint8_t prev_REC;
uint8_t cnt_REC;
/* POWER */
uint8_t pres_POWER;
uint8_t prev_POWER;
uint8_t cnt_POWER;
uint8_t release_POWER;

/* Other */
uint32_t counter;

void Debounce(uint8_t *pres_px, uint8_t *cnt_px, uint8_t *prev_px, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) /// uint32_t GPIOx_IDR, uint32_t GPIO_IDR_IDx)
{
	uint8_t cur_px;
	/* Read current state */
	//uint8_t cur_px = (~GPIOx_IDR & GPIO_IDR_IDx) != 0;
	cur_px = !HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
	//if(cur_px == 0) HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);

	/* If level has changed  */
	if (cur_px != *prev_px) {
	/* Increase counter */
		(*cnt_px)++;
		/* If consecutive 4*20 = 80ms approved */
		if (*cnt_px >= 4) {
			/* The button have not bounced for four checks, change state */
			*prev_px = cur_px;
			/* If the button was pressed (not released), tell main so */
			if (cur_px != 0)
			{
				*pres_px = 1;
				//HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			}
			(*cnt_px) = 0;
		}
	}
	else {
		/* Reset counter */
		*cnt_px = 0;
	}
}

void DebouncePower(uint8_t *pres_px, uint8_t *cnt_px, uint8_t *prev_px,uint8_t *release_px, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) /// uint32_t GPIOx_IDR, uint32_t GPIO_IDR_IDx)
{
	uint8_t cur_px;
	/* Read current state */
	//uint8_t cur_px = (~GPIOx_IDR & GPIO_IDR_IDx) != 0;
	cur_px = !HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
	//if(cur_px == 0) HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);

	/* If level has changed  */
	if (cur_px != *prev_px) {
	/* Increase counter */
		(*cnt_px)++;
		/* If consecutive 50*20 = 1000ms approved */
		if (*cnt_px >= 50) {
			/* The button have not bounced for four checks, change state */
			*prev_px = cur_px;
			/* If the button was pressed (not released), tell main so */
			if (cur_px != 0)
			{
				*pres_px = 1;
				HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
			}
			(*cnt_px) = 0;
			/* If the button was released, tell power */
			if (cur_px == 0)
			{
				*release_px = 1;
				HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
			}
		}
	}
	else {
		/* Reset counter */
		*cnt_px = 0;
	}
}

/**
 * \brief KEY task core function.
 *
 * \param pvParameters Junk parameter.
 */
void StartTask03(void *pvParameters)
{
	/* Just to avoid compiler warnings. */
	UNUSED(pvParameters);
	//uint32_t current_ticks;
	uint32_t key_pad=0;
	//portTickType current_ticks;
	//timeMutex = xSemaphoreCreateMutex();
	pwr_bt_first = 0;

	for (;;) {

			//Debounce(&pres_F1, &cnt_F1, &prev_F1, F1_GPIO_Port, F1_Pin);
			//Debounce(&pres_F2, &cnt_F2, &prev_F2, F2_GPIO_Port, F2_Pin);
			//Debounce(&pres_F4, &cnt_F4, &prev_F4, F4_GPIO_Port, F4_Pin);
			Debounce(&pres_F5, &cnt_F5, &prev_F5, F5_GPIO_Port, F5_Pin);
			Debounce(&pres_RIGHT, &cnt_RIGHT, &prev_RIGHT, RIGHT_GPIO_Port, RIGHT_Pin);
			Debounce(&pres_UP, &cnt_UP, &prev_UP, UP_GPIO_Port, UP_Pin);
			Debounce(&pres_ENT, &cnt_ENT, &prev_ENT, ENT_GPIO_Port, ENT_Pin);
			Debounce(&pres_LEFT, &cnt_LEFT, &prev_LEFT, LEFT_GPIO_Port, LEFT_Pin);
			Debounce(&pres_DOWN, &cnt_DOWN, &prev_DOWN, DOWN_GPIO_Port, DOWN_Pin);
			Debounce(&pres_REC, &cnt_REC, &prev_REC, REC_GPIO_Port, REC_Pin);
			DebouncePower(&pres_POWER, &cnt_POWER, &prev_POWER, &release_POWER, INT_KEY_PWR_GPIO_Port,INT_KEY_PWR_Pin);

			//if (release_POWER && !flag_arm_rec)  //to prevent power off during recording
			//{
			//	release_POWER = 0;
			//	power_off_mode();
			//}
			if(pres_POWER && !flag_arm_rec){
				pres_POWER = 0;
				if(pwr_bt_first > 250)
				{
					power_off_mode();
				}

			}

			//if(pres_F1){
			//	pres_F1 = 0;
			//	key_pad = 0;
				//HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);

			//	counter++;
			//}
			//if(pres_F2){
			//	pres_F2 = 0;
			//	key_pad = 0;
			//}
			//if(pres_F4){
			//	pres_F4 = 0;
			//	key_pad = 0;
			//}
			if(pres_LEFT){
				pres_LEFT = 0;
				key_pad = 6;
			}
			if(pres_RIGHT){
				pres_RIGHT = 0;
				key_pad = 7;
			}
			if(pres_UP){
				pres_UP = 0;
				key_pad = 1;
			}
			if(pres_DOWN){
				pres_DOWN = 0;
				key_pad = 3;
			}
			if(pres_ENT){
				pres_ENT = 0;
				key_pad = 2;
			}
			if(pres_F5){
				pres_F5 = 0;
				key_pad = 5;
				//HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			}
			if(pres_REC){
				pres_REC = 0;
				key_pad = 9;

			}

			if(key_pad != 0){

				if (get_special_mode_status(DEMO_MODE_EVS))   evs_win_KEY_handler( key_pad);
				if (get_special_mode_status(DEMO_MODE_BLAST)) blast_win_KEY_handler( key_pad);
				if (get_special_mode_status(DEMO_MENU_MAIN))  menu_main_win_KEY_handler( key_pad); //kt

			if (get_special_mode_status(DEMO_OPT_ANALYZ_EVS)) opt_an_evs_win_KEY_handler( key_pad);
	 		if (get_special_mode_status(DEMO_OPT_ANALYZ_BLAST)) opt_an_bls_win_KEY_handler( key_pad); 

		 	if (get_special_mode_status(DEMO_TRIGGER_EVS)) trigger_evs_win_KEY_handler( key_pad);
			if (get_special_mode_status(DEMO_TRIGGER_BLAST)) trigger_bls_win_KEY_handler( key_pad);

			if (get_special_mode_status(DEMO_OPT_TRANSD)) transd_win_KEY_handler( key_pad);
			if (get_special_mode_status(DEMO_OPT_RECORD)) opt_rec_win_KEY_handler( key_pad);
			if (get_special_mode_status(DEMO_OPT_FTP)) opt_ftp_win_KEY_handler( key_pad);
			//if (get_special_mode_status(DEMO_SEND_FILE)) send_file_win_KEY_handler( key_pad);
				if (get_special_mode_status(DEMO_OPT_COMMON)) opt_common_win_KEY_handler( key_pad);

			if (get_special_mode_status(DEMO_INPUTBOX)) inputbox_KEY_handler( key_pad);
			if (get_special_mode_status(DEMO_SELECTBOX)) selectbox_KEY_handler( key_pad);

				key_pad = 0;
			}
			osDelay(20);
			pwr_bt_first++;
	}
}

void power_off_mode(void)
{
	bluescreen = 1;

	if ( mounted) {
		//sd_update_chunkinfo();
		//f_close(&file_wave);

		//f_mount(LUN_ID_SD_MMC_0_MEM, NULL);
		f_mount(NULL, (TCHAR const*)SDPath, 0);
		mounted = 0;
		//sd_mmc_deinit();
		//deconfig_hsmci_pins();
	}
	osDelay(1000);

	//ioport_set_pin_level(AN_POWER_DOWN, IOPORT_PIN_LEVEL_LOW);
    HAL_GPIO_WritePin(GPIOC, AN_POWER_DOWN_Pin, 0);
	
	//ioport_set_pin_level(ARM_PWR_LATCH, IOPORT_PIN_LEVEL_LOW);
    HAL_GPIO_WritePin(ARM_PWR_GPIO_Port, ARM_PWR_Pin, 0);
	while(1);
}



