/* Includes ------------------------------------------------------------------*/
#include "at25m02_driver.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/*******************************************************************************
* Function Name  : AT25M02_GET_SPI_CONFIG
* Description    : Get SPI Config
* Input          : None
* Output         : None
* Return         : nrf_drv_spi_config_t
*******************************************************************************/
nrf_drv_spi_config_t AT25M02_GET_SPI_CONFIG(){

  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin    = 11;
  spi_config.miso_pin  = 13;
  spi_config.mosi_pin  = 9;
  spi_config.sck_pin   = 8;
  spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
  spi_config.mode      = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

  return spi_config;
}

/*******************************************************************************
* Function Name  : AT25M02_READ_ADDR
* Description    : Read from EEPROM Memory
* Input          : *handle (Pointer to SPI Instance), addr (Address of Memory to Read), 
*                  nBytes (Number of Consecutive Reads, Address Increments Each Time)
* Output         : *pBuffer (Array with Memory Values)
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t AT25M02_READ_ADDR(void *handle, uint32_t addr, uint8_t *pBuffer, uint32_t nBytes){

  NRF_LOG_DEBUG("AT25M02_READ_ADDR - Start");

  // Create tx and rx buffer
  uint32_t length = nBytes+4;
  uint8_t m_tx_buf[length];
  uint8_t m_rx_buf[length];

  memset(m_tx_buf, 0, length);
  memset(m_rx_buf, 0, length);

  // Set tx buffer
  m_tx_buf[0] = AT25M02_OPCODE_READ;
  m_tx_buf[1] = (addr & 0x000000FF);
  m_tx_buf[2] = (addr & 0x0000FF00) >> 8;
  m_tx_buf[3] = (addr & 0x00FF0000) >> 16;

  NRF_LOG_DEBUG("TX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_tx_buf, length);

  ret_code_t ret = nrf_drv_spi_transfer(handle, m_tx_buf, length, m_rx_buf, length);

  if (ret != NRF_SUCCESS) {
    NRF_LOG_ERROR("AT25M02 ERROR - nrf_drv_spi_transfer did not return NRF_SUCCESS.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  NRF_LOG_DEBUG("RX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_rx_buf, length);

  for (size_t i = 4; i < length; i++) {
    pBuffer[i-4] = m_rx_buf[i];
  }

  NRF_LOG_DEBUG("AT25M02_READ_ADDR - End");

  return MEMS_SUCCESS;

}

/*******************************************************************************
* Function Name  : AT25M02_READ_STATUS_REG
* Description    : Read Status Register of EEPROM
* Input          : *handle (Pointer to SPI Instance)
* Output         : *regBuffer (Status Register Value)
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t AT25M02_READ_STATUS_REG(void *handle, uint8_t *regBuffer){

  NRF_LOG_DEBUG("AT25M02_READ_STATUS_REG - Start");

    // Create tx and rx buffer
  uint8_t m_tx_buf[] = {AT25M02_OPCODE_RDSR, 0x00};
  uint8_t m_rx_buf[] = {0x00, 0x00};

  NRF_LOG_DEBUG("TX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_tx_buf, 2);

  ret_code_t ret = nrf_drv_spi_transfer(handle, m_tx_buf, 2, m_rx_buf, 2);

  if (ret != NRF_SUCCESS) {
    NRF_LOG_ERROR("AT25M02 ERROR - nrf_drv_spi_transfer did not return NRF_SUCCESS.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  NRF_LOG_DEBUG("RX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_rx_buf, 2);

  regBuffer[0] = m_rx_buf[1];

  NRF_LOG_DEBUG("AT25M02_READ_STATUS_REG - End");

  return MEMS_SUCCESS;

}

/*******************************************************************************
* Function Name  : AT25M02_WRITE_ADDR
* Description    : Read Status Register of EEPROM
* Input          : *handle (Pointer to SPI Instance), addr (Address of Memory to Write), 
*                  nBytes (Number of Consecutive Writes, Address Increments Each Time),
*                  checkResult (Check if Data was Written Correctly) [true, false],
*                  *pBuffer (Values to Write to Memory)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t AT25M02_WRITE_ADDR(void *handle, uint32_t addr, uint8_t *pBuffer, uint16_t nBytes, bool checkResult) {

  NRF_LOG_DEBUG("AT25M02_WRITE_ADDR - Start");

  // Check if addresses are in same row of memory
  if ( (((addr+nBytes) & 0x0003FF00) != (addr & 0x0003FF00)) || nBytes > 256) {
    NRF_LOG_ERROR("AT25M02 ERROR - Bytes addressed are not in same row page or length of write operations exceed 256.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }
  
  // Set Write Enable Latch
  if(AT25M02_WRITE_ENABLE(handle) == MEMS_ERROR) {
    NRF_LOG_ERROR("AT25M02 ERROR - Write Enable Operation Returned Mems Error.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  // Create tx and rx buffer
  uint16_t length = nBytes+4;
  uint8_t m_tx_buf[length];
  uint8_t m_rx_buf[length];

  memset(m_tx_buf, 0, length);
  memset(m_rx_buf, 0, length);

  // Set tx buffer
  m_tx_buf[0] = AT25M02_OPCODE_WRITE;
  m_tx_buf[1] = (addr & 0x000000FF);
  m_tx_buf[2] = (addr & 0x0000FF00) >> 8;
  m_tx_buf[3] = (addr & 0x00FF0000) >> 16;

  // Add data to write
  for(size_t i = 4; i < length; i++)
  {
    m_tx_buf[i] = pBuffer[i-4];
  }

  NRF_LOG_DEBUG("TX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_tx_buf, length);

  ret_code_t ret = nrf_drv_spi_transfer(handle, m_tx_buf, length, m_rx_buf, length);

  if (ret != NRF_SUCCESS) {
    NRF_LOG_ERROR("AT25M02 ERROR - nrf_drv_spi_transfer did not return NRF_SUCCESS.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  // Wait Until Internal EEPROM Write Operation is Completed
  uint8_t busy = 0xFF;
  while(busy == 0xFF) {
    if(AT25M02_WRITE_POLL(handle, &busy) == MEMS_ERROR) {
      return MEMS_ERROR;
    }
  }

  // Check if write was successful
  if (checkResult == true) {
    uint8_t tempBuffer[nBytes];
    bool same = true;
    AT25M02_READ_ADDR(handle, addr, tempBuffer, nBytes);
    
    for(size_t i = 0; i < nBytes; i++)
    {
      if(tempBuffer[i] != pBuffer[i]) {
        NRF_LOG_ERROR("Memory Value = %X, Write Value = %X", tempBuffer[i], pBuffer[i]);
        same = false;
      }
    }
    
    if(same == false) {
      NRF_LOG_ERROR("AT25M02 ERROR - Write Check Failed.\n");
      NRF_LOG_FLUSH();
      return MEMS_ERROR;

    } else {
      NRF_LOG_INFO("AT25M02 Write Check Successful.");
      NRF_LOG_FLUSH();
    }
    
  }

  NRF_LOG_DEBUG("AT25M02_WRITE_ADDR - End\n");

  return MEMS_SUCCESS;

}

/*******************************************************************************
* Function Name  : AT25M02_WRITE_ENABLE
* Description    : Set WEL Status Bit to 1
* Input          : *handle (Pointer to SPI Instance)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t AT25M02_WRITE_ENABLE(void *handle) {

  NRF_LOG_DEBUG("AT25M02_WRITE_ENABLE - Start");

    // Create tx and rx buffer
  uint8_t m_tx_buf[] = {AT25M02_OPCODE_WREN};
  uint8_t m_rx_buf[] = {0x00};

  NRF_LOG_DEBUG("TX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_tx_buf, 1);

  ret_code_t ret = nrf_drv_spi_transfer(handle, m_tx_buf, 1, m_rx_buf, 1);

  if (ret != NRF_SUCCESS) {
    NRF_LOG_ERROR("AT25M02 ERROR - nrf_drv_spi_transfer did not return NRF_SUCCESS.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  NRF_LOG_DEBUG("AT25M02_WRITE_ENABLE - End");

  return MEMS_SUCCESS;

}

/*******************************************************************************
* Function Name  : AT25M02_WRITE_DISABLE
* Description    : Set WEL Status Bit to 0
* Input          : *handle (Pointer to SPI Instance)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t AT25M02_WRITE_DISABLE(void *handle) {

  NRF_LOG_DEBUG("AT25M02_WRITE_DISABLE - Start");

    // Create tx and rx buffer
  uint8_t m_tx_buf[] = {AT25M02_OPCODE_WRDI};
  uint8_t m_rx_buf[] = {0x00};

  NRF_LOG_DEBUG("TX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_tx_buf, 1);

  ret_code_t ret = nrf_drv_spi_transfer(handle, m_tx_buf, 1, m_rx_buf, 1);

  if (ret != NRF_SUCCESS) {
    NRF_LOG_ERROR("AT25M02 ERROR - nrf_drv_spi_transfer did not return NRF_SUCCESS.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  NRF_LOG_DEBUG("AT25M02_WRITE_DISABLE - End");

  return MEMS_SUCCESS;

}

/*******************************************************************************
* Function Name  : AT25M02_WRITE_POLL
* Description    : Check if EEPROM Internal Write Operation is Finished
* Input          : *handle (Pointer to SPI Instance)
* Output         : *pBuffer (0xFF == BUSY, 0x00 == DONE)
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t AT25M02_WRITE_POLL(void *handle, uint8_t *pBuffer) {

  NRF_LOG_DEBUG("AT25M02_WRITE_POLL - Start");

  // Create tx and rx buffer
  uint8_t m_tx_buf[] = {AT25M02_OPCODE_LPWP, 0x00};
  uint8_t m_rx_buf[] = {0x00, 0x00};

  NRF_LOG_DEBUG("TX Buffer: ");
  NRF_LOG_HEXDUMP_DEBUG(m_tx_buf, 2);

  ret_code_t ret = nrf_drv_spi_transfer(handle, m_tx_buf, 2, m_rx_buf, 2);

  if (ret != NRF_SUCCESS) {
    NRF_LOG_ERROR("AT25M02 ERROR - nrf_drv_spi_transfer did not return NRF_SUCCESS.\n");
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  // Check result
  if(m_rx_buf[1] != 0xFF && m_rx_buf[1] != 0x00) {
    NRF_LOG_ERROR("AT25M02 ERROR - Write Poll Result Not Correct, Value = %X.\n", m_rx_buf[1]);
    NRF_LOG_FLUSH();
    return MEMS_ERROR;
  }

  // Pass result on
  pBuffer[0] = m_rx_buf[1];

  NRF_LOG_DEBUG("AT25M02_WRITE_POLL - End");

  return MEMS_SUCCESS; 

}