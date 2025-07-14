
#include <stdlib.h>
#include <stdint.h>
#include "main.h"

int32_t ret_check;
int32_t ret_check1;


/**
 * @brief Write data to SPI.
 * @param  - The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t os_spi_write(SPI_HandleTypeDef *hspi,
				 uint8_t *data,
				 uint16_t bytes_number)
{
	//int32_t ret;
	uint8_t RxBuffer[8] = { 0 };

//	no_os_mutex_lock(desc->bus->mutex);
	//ret =  desc->platform_ops->write_and_read(desc, data, bytes_number);
	HAL_GPIO_WritePin(GPIOA, NSSSPI1_Pin, 0);
	ret_check = HAL_SPI_TransmitReceive(hspi, (uint8_t*)data, (uint8_t *)RxBuffer, bytes_number, 5);
	HAL_GPIO_WritePin(GPIOA, NSSSPI1_Pin, 1);
//	no_os_mutex_unlock(desc->bus->mutex);

	ret_check1++;
	return ret_check;
}

/**
 * @brief read data from SPI.
 * @param  The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t os_spi_read(SPI_HandleTypeDef *hspi,
				 uint8_t *data,
				 uint16_t bytes_number)
{
	int32_t  i;
	uint8_t RxBuffer[8] = { 0 };

//	no_os_mutex_lock(desc->bus->mutex);
	//ret =  desc->platform_ops->write_and_read(desc, data, bytes_number);
	HAL_GPIO_WritePin(GPIOA, NSSSPI1_Pin, 0);
	ret_check = HAL_SPI_TransmitReceive(hspi, (uint8_t*)data, (uint8_t *)RxBuffer, bytes_number, 5);
	HAL_GPIO_WritePin(GPIOA, NSSSPI1_Pin, 1);
//	no_os_mutex_unlock(desc->bus->mutex);

	for (i = 0; i < 8; i++) {
		data[i] = RxBuffer[i];
	}
	ret_check1++;
	return ret_check;
}


