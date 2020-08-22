/*
 * Obtained from Embedded Artirst who provided this under BSD license.
 * The Clear BSD License
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_common.h"
#include "fsl_lpi2c.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_iomuxc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#define LPI2C_MASTER_CLOCK_FREQUENCY LPI2C_CLOCK_FREQUENCY


#define PCA6416_I2C  LPI2C1

#define PCA6416_I2C_SLAVE_ADDR_7BIT 0x20

#define PTA6416_INPUT_PORT0_REG   0x00
#define PTA6416_INPUT_PORT1_REG   0x01
#define PTA6416_OUTPUT_PORT0_REG  0x02
#define PTA6416_OUTPUT_PORT1_REG  0x03
#define PTA6416_POLINV_PORT0_REG  0x04
#define PTA6416_POLINV_PORT1_REG  0x05
#define PTA6416_CONFIG_PORT0_REG  0x06
#define PTA6416_CONFIG_PORT1_REG  0x07


/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint16_t shadow = 0x0000;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void init_pins(void)
{
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_B1_00_LPI2C1_SCL,        /* GPIO_AD_B1_00 is configured as LPI2C1_SCL */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AD_B1_00 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_B1_01_LPI2C1_SDA,        /* GPIO_AD_B1_01 is configured as LPI2C1_SDA */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AD_B1_01 */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_B1_00_LPI2C1_SCL,        /* GPIO_AD_B1_00 PAD functional properties : */
      0xD8B0u);                               /* Slew Rate Field: Slow Slew Rate
                                                 Drive Strength Field: R0/6
                                                 Speed Field: medium(100MHz)
                                                 Open Drain Enable Field: Open Drain Enabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 22K Ohm Pull Up
                                                 Hyst. Enable Field: Hysteresis Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_B1_01_LPI2C1_SDA,        /* GPIO_AD_B1_01 PAD functional properties : */
      0xD8B0u);                               /* Slew Rate Field: Slow Slew Rate
                                                 Drive Strength Field: R0/6
                                                 Speed Field: medium(100MHz)
                                                 Open Drain Enable Field: Open Drain Enabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 22K Ohm Pull Up
                                                 Hyst. Enable Field: Hysteresis Disabled */
}

static void i2c_init()
{
    lpi2c_master_config_t masterConfig = {0};

    /*
    * masterConfig.debugEnable = false;
    * masterConfig.ignoreAck = false;
    * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
    * masterConfig.baudRate_Hz = 100000U;
    * masterConfig.busIdleTimeout_ns = 0;
    * masterConfig.pinLowTimeout_ns = 0;
    * masterConfig.sdaGlitchFilterWidth_ns = 0;
    * masterConfig.sclGlitchFilterWidth_ns = 0;
    */
    LPI2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Hz = 100000U;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(LPI2C1, &masterConfig, LPI2C_CLOCK_FREQUENCY);
}

static void i2c_deinit()
{
    LPI2C_MasterDeinit(LPI2C1);
}

/* Write to both port registers. */
static status_t i2c_write(uint8_t port0reg, uint16_t val)
{
    lpi2c_master_transfer_t xfer = {0};
    uint8_t data[] = { val&0xff, (val>>8) };

    xfer.slaveAddress = PCA6416_I2C_SLAVE_ADDR_7BIT;
    xfer.direction = kLPI2C_Write;
    xfer.subaddress = port0reg;
    xfer.subaddressSize = 1;
    xfer.data = data;
    xfer.dataSize = 2;
    xfer.flags = kLPI2C_TransferDefaultFlag;

    return LPI2C_MasterTransferBlocking(LPI2C1, &xfer);
}

status_t PCA6416_Init(void)
{
    static bool already_initialized = false;
    status_t status;

    if (already_initialized) {
    	return kStatus_Success;
    }

    init_pins();
    i2c_init();

    status = i2c_write(PTA6416_OUTPUT_PORT0_REG, 0x0000);
    if (status != kStatus_Success) {
        return status;
    }
    status = i2c_write(PTA6416_CONFIG_PORT0_REG, 0x0000);
    if (status != kStatus_Success) {
        return status;
    }
    shadow = 0x0000;
    already_initialized = true;
    return status;
}

status_t PCA6416_SetAllPins(uint16_t values)
{
    shadow = values;
    return i2c_write(PTA6416_OUTPUT_PORT0_REG, values);
}
status_t PCA6416_ClearPins(uint16_t mask)
{
	shadow = shadow & ~mask;
    return i2c_write(PTA6416_OUTPUT_PORT0_REG, shadow);
}
status_t PCA6416_SetPins(uint16_t mask)
{
	shadow = shadow | mask;
    return i2c_write(PTA6416_OUTPUT_PORT0_REG, shadow);
}
