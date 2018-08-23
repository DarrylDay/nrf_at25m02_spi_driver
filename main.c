// SPI Drivers
#include "nrf_drv_spi.h"

// General NRF Drivers
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "app_error.h"

// Log Drivers
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// LSM6DS3 Drivers
#include "at25m02_driver.h"

// Additional SPI Pins
#define IMU_CHIP_SELECT 12

// SPI Instance and Variables
#define SPI_INSTANCE  0                                                          /**< SPI instance index. */
static nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);                   /**< SPI instance. */

// Main Loop
int main(void)
{

    // Configure Log
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("\033[2J\033[;H");
    NRF_LOG_FLUSH();

    // Configure Write/Hold Pins
    nrf_gpio_cfg_output(EEPROM_WRITE_PROTECT_PIN);
    nrf_gpio_cfg_output(EEPROM_HOLD_PIN);
    nrf_gpio_cfg_output(IMU_CHIP_SELECT);
    nrf_gpio_pin_set(EEPROM_WRITE_PROTECT_PIN);
    nrf_gpio_pin_set(EEPROM_HOLD_PIN);
    nrf_gpio_pin_set(IMU_CHIP_SELECT);

    // Configure SPI
    nrf_drv_spi_config_t spi_config = AT25M02_GET_SPI_CONFIG();
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));

    NRF_LOG_INFO("== AT25M02 Test ==\n");
    NRF_LOG_FLUSH();

    nrf_delay_ms(500);

    // // *** Read Status Register Example ***
    // // By Default the Status Register == 0x00
    // uint8_t status_reg = 0;
    // while(1){
    //     AT25M02_READ_STATUS_REG(&spi, &status_reg);
    //     NRF_LOG_INFO("Status Reg: %X", status_reg);
    //     NRF_LOG_FLUSH();
    //     nrf_delay_ms(200);
    // }

    // // *** Read Memory Example ***
    // uint32_t addr = 0x0001ABCD;
    // uint8_t buffer[] = {0,0,0,0,0};
    // uint16_t nBytes = 5;
    // while(1) {
    //     AT25M02_READ_ADDR(&spi, addr, buffer, nBytes);
    //     for(size_t i = 0; i < nBytes; i++)
    //     {
    //         NRF_LOG_INFO("Addr: %X, Value: %X", addr+i, buffer[i]);
    //     }
    //     NRF_LOG_FLUSH();
    //     nrf_delay_ms(5000);
    // }
    
    // *** Write Memory Example ***
    uint32_t addr = 0x0001ABCD;
    uint8_t buffer[] = {0xEE, 0xCA, 0xDD, 0x32, 0x55};
    while(1) {
        NRF_LOG_INFO("Writing to EEPROM...");
        AT25M02_WRITE_ADDR(&spi, addr, buffer, 5, true);
        NRF_LOG_FLUSH();
        nrf_delay_ms(5000);
    }
    

}
