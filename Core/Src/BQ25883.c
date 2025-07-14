
#include "parameters.h"
#include "bq2588x_reg.h"

extern I2C_HandleTypeDef hi2c2;
//xSemaphoreHandle battery_twi_read_notification_semaphore = NULL;
//static const portTickType max_block_time_ticks3 = 200UL / portTICK_RATE_MS;
//static portBASE_TYPE higher_priority_task_woken3 = pdFALSE;
HAL_StatusTypeDef init_bq25883(void);
void bq2588x_update_status(bq2588x_t *bq);

extern uint32_t I2C_Rx_Complete_Flag;


/* Charger */

/* The address for BQ25883 */
#define BQ25883_ADDRESS		0xD6
#define BQ25883_MEM_ADDR_LENGTH	1

uint32_t statusR, statusW;


static int bq2588x_read_16(uint8_t reg, uint16_t *data)
{
	HAL_StatusTypeDef otv;
	uint8_t rx_buffer[2];
	
	otv = HAL_I2C_Mem_Read_DMA(&hi2c2, (uint16_t)BQ25883_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)rx_buffer, 2);
  if ( otv != HAL_OK)
  {
    /* Reading process Error */
    return 1;
  }

	 if (battery_i2c_read_notification_semaphoreHandle !=	NULL) {
		 /* Wait until notified by the ISR that transmission is complete. */
		 //HAL_GPIO_WritePin(GPIOB, V_RX_Pin, 0);
		 if (osSemaphoreAcquire(battery_i2c_read_notification_semaphoreHandle,	 osWaitForever) == osOK) {

			 //return_value = ERR_TIMEOUT;
		 }
	 }


   /* Wait for the end of the reception signaled using the callback HAL_I2C_MemRxCpltCallback*/
//  while((I2C_Rx_Complete_Flag == 0))
//  {
    /* wait until I2C Reception ends or error */
//  }
	//TWIHS2->TWIHS_IER = TWIHS_IER_RXRDY | TWIHS_IER_TXCOMP | TWIHS_IER_NACK;
	
	/* Send a START Condition */
	//TWIHS2->TWIHS_CR = TWIHS_CR_START;
	


	
	*data = rx_buffer[1] + ((uint16_t)rx_buffer[0] << 8);
	
	return 0;	
}



#define ADC_RES_REG_BASE	0x17
enum {
	ADC_IBUS,
	ADC_ICHG,
	ADC_VBUS,
	ADC_VBAT,
	ADC_VSYS,
	ADC_TS,
	ADC_TDIE,
	ADC_MAX_NUM
};



void bq2588x_update_status(bq2588x_t *bq)
{
	int ret;
	uint16_t reg_val;
	
		ret=bq2588x_read_16(BQ2588X_REG_VBAT_ADC_1, &reg_val);
		if (ret == 0)
		bq->vbat_volt = reg_val;

}



HAL_StatusTypeDef init_bq25883(void) {

	uint8_t tx_buffer[2];
	HAL_StatusTypeDef otv;
		//tx_buffer[0] = 0x42;	/* WD_RST */
		//packet_tx.addr[0] = BQ2588X_REG_CHG_CTRL3;
		tx_buffer[0] = 0x8D;	/* WD_DISABLE */
		otv = HAL_I2C_Mem_Write(&hi2c2 , (uint16_t)BQ25883_ADDRESS, BQ2588X_REG_CHG_CTRL1, I2C_MEMADD_SIZE_8BIT, (uint8_t*)(tx_buffer), 1, 100);
	    if(otv != HAL_OK)
	    {
	      /* Writing process Error */
	    	return otv;
	    }
//		if (twihs_master_write(TWIHS2, &packet_tx) != TWIHS_SUCCESS) {
//			return -1;
//		}

			//if (twihs_master_read(TWIHS2, &packet_rx) != TWIHS_SUCCESS) {
				//return -1;
			//}

		tx_buffer[0] = 0x80;	/* ADC_EN, ADC_CONT, 15BIT */
		otv = HAL_I2C_Mem_Write(&hi2c2 , (uint16_t)BQ25883_ADDRESS, BQ2588X_REG_ADC_CTRL, I2C_MEMADD_SIZE_8BIT, (uint8_t*)(tx_buffer), 1, 100);
	    if(otv != HAL_OK)
	    {
	      /* Writing process Error */
	    	return otv;
	    }
//		if (twihs_master_write(TWIHS2, &packet_tx) != TWIHS_SUCCESS) {
//			return -1;
//		}
		
		return otv;
			
}

/*-----------------------------------------------------------*/






