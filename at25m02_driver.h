/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef AT25M02_NRF_SPI_Driver_H
#define AT25M02_NRF_SPI_Driver_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "nrf_drv_spi.h"

/* Exported common structure --------------------------------------------------------*/

#ifndef __SHARED__TYPES
#define __SHARED__TYPES

typedef enum
{
  MEMS_SUCCESS      =   0x01,
  MEMS_ERROR        =   0x00
} status_t;

#endif

/***************** EEPROM PINS ******************/

#define EEPROM_WRITE_PROTECT_PIN  10
#define EEPROM_HOLD_PIN           7

/***************** EEPROM OPCODES ******************/

#define AT25M02_OPCODE_WRSR			0x01
#define	AT25M02_OPCODE_WRITE		0x02
#define	AT25M02_OPCODE_READ			0x03
#define	AT25M02_OPCODE_WRDI			0x04
#define	AT25M02_OPCODE_RDSR			0x05
#define	AT25M02_OPCODE_WREN			0x06
#define	AT25M02_OPCODE_LPWP			0x08

/************** Functions *******************/

nrf_drv_spi_config_t AT25M02_GET_SPI_CONFIG();
status_t AT25M02_READ_ADDR(void *handle, uint32_t addr, uint8_t *pBuffer, uint32_t nBytes);
status_t AT25M02_READ_STATUS_REG(void *handle, uint8_t *regBuffer);
status_t AT25M02_WRITE_ADDR(void *handle, uint32_t addr, uint8_t *pBuffer, uint16_t nBytes, bool checkResult);
status_t AT25M02_WRITE_ENABLE(void *handle);
status_t AT25M02_WRITE_DISABLE(void *handle);
status_t AT25M02_WRITE_POLL(void *handle, uint8_t *pBuffer);

#endif
