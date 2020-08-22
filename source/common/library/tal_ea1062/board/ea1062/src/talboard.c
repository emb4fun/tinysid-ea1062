/**************************************************************************
*  This file is part of the TAL project (Tiny Abstraction Layer)
*
*  Copyright (c) 2020 by Michael Fischer (www.emb4fun.de).
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright 
*     notice, this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the 
*     documentation and/or other materials provided with the distribution.
*
*  3. Neither the name of the author nor the names of its contributors may 
*     be used to endorse or promote products derived from this software 
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
*  SUCH DAMAGE.
*
***************************************************************************
*  History:
*
*  08.06.2020  mifi  First Version for the iMX RT1062 Developer’s Kit.
**************************************************************************/
#if defined(USE_BOARD_EA1062)
#define __TALBOARD_C__

/*=======================================================================*/
/*  Include                                                              */
/*=======================================================================*/
#include <string.h>
#include "tal.h"

#include "fsl_iomuxc.h"
#include "fsl_lpi2c.h"

/*=======================================================================*/
/*  All Structures and Common Constants                                  */
/*=======================================================================*/

#define I2C_MAC_SLAVE_ADDR_7BIT 0x53

/*=======================================================================*/
/*  Definition of all global Data                                        */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Data                                         */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Procedures                                   */
/*=======================================================================*/

/*=======================================================================*/
/*  All code exported                                                    */
/*=======================================================================*/

/* 
 * Read the pre-programmed EUI-48 node address. It is globally unique
 * and is read only.
 *
 *  Obtained from Embedded Artirst who provided this under BSD license.
 */
static status_t read_mac(uint8_t* addr)
{
   lpi2c_master_transfer_t xfer = {0};
   uint8_t reg = 0xfa;
   status_t status;

   /* Write 1 byte to select the address to read from (0xfa) */
   xfer.slaveAddress = I2C_MAC_SLAVE_ADDR_7BIT;
   xfer.direction = kLPI2C_Write;
   xfer.subaddress = 0;
   xfer.subaddressSize = 0;
   xfer.data = &reg;
   xfer.dataSize = 1;
   xfer.flags = kLPI2C_TransferDefaultFlag;

   status = LPI2C_MasterTransferBlocking(LPI2C1, &xfer);
   if (status != kStatus_Success) {
      return status;
   }

   // wait

   /* Read the data, 6 bytes */
   xfer.slaveAddress = I2C_MAC_SLAVE_ADDR_7BIT;
   xfer.direction = kLPI2C_Read;
   xfer.subaddress = 0;
   xfer.subaddressSize = 0;
   xfer.data = addr;
   xfer.dataSize = 6;
   xfer.flags = kLPI2C_TransferDefaultFlag;

   return LPI2C_MasterTransferBlocking(LPI2C1, &xfer);
}


#if defined(MEMORY_INIT)
/*************************************************************************/
/*  MemoryInit                                                           */
/*                                                                       */
/*  If MEMORY_INIT is defined, the MemoryInit() function will be called. */
/*  By default  MemoryInit() is called after SystemInit() to enable an   */
/*  external memory controller.                                          */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void MemoryInit (void)
{
   /*
    * SDRAM is still by evkmimxrt1020_sdram_init.jlinkscript
    */
   
} /* MemoryInit */
#endif

/*************************************************************************/
/*  tal_BoardEnableCOMx                                                  */
/*                                                                       */
/*  Board and hardware depending functionality to enable the COM port.   */
/*  If this port is not supported by the board, return an error.         */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: TAL_OK / error cause                                         */
/*************************************************************************/
TAL_RESULT tal_BoardEnableCOM1 (void)
{
   CLOCK_EnableClock(kCLOCK_Iomuxc);         /* iomuxc clock (iomuxc_clk_enable): 0x03u */

   IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_B0_12_LPUART1_TX,       /* GPIO_AD_B0_12 is configured as LPUART1_TX */
      0U);                                   /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_B0_13_LPUART1_RX,       /* GPIO_AD_B0_13 is configured as LPUART1_RX */
      0U);                                   /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_B0_12_LPUART1_TX,       /* GPIO_AD_B0_12 PAD functional properties : */
      0x10B0u);                              /* Slew Rate Field: Slow Slew Rate
                                                Drive Strength Field: R0/6
                                                Speed Field: medium(100MHz)
                                                Open Drain Enable Field: Open Drain Disabled
                                                Pull / Keep Enable Field: Pull/Keeper Enabled
                                                Pull / Keep Select Field: Keeper
                                                Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_B0_13_LPUART1_RX,       /* GPIO_AD_B0_13 PAD functional properties : */
      0x10B0u);                              /* Slew Rate Field: Slow Slew Rate
                                                Drive Strength Field: R0/6
                                                Speed Field: medium(100MHz)
                                                Open Drain Enable Field: Open Drain Disabled
                                                Pull / Keep Enable Field: Pull/Keeper Enabled
                                                Pull / Keep Select Field: Keeper
                                                Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                Hyst. Enable Field: Hysteresis Disabled */


   /* Configure UART divider to default */
   CLOCK_SetMux(kCLOCK_UartMux, 0); /* Set UART source to PLL3 80M */
   CLOCK_SetDiv(kCLOCK_UartDiv, 0); /* Set UART divider to 1 */

   return(TAL_OK);
} /* tal_BoardEnableCOM1 */

TAL_RESULT tal_BoardEnableCOM2 (void)
{
   return(TAL_ERR_COM_PORT_NOHW);
} /* tal_BoardEnableCOM2 */

TAL_RESULT tal_BoardEnableCOM3 (void)
{
   return(TAL_ERR_COM_PORT_NOHW);
} /* tal_BoardEnableCOM3 */

TAL_RESULT tal_BoardEnableCOM4 (void)
{
   return(TAL_ERR_COM_PORT_NOHW);
} /* tal_BoardEnableCOM4 */

TAL_RESULT tal_BoardEnableCOM5 (void)
{
   return(TAL_ERR_COM_PORT_NOHW);
} /* tal_BoardEnableCOM5 */

TAL_RESULT tal_BoardEnableCOM6 (void)
{
   return(TAL_ERR_COM_PORT_NOHW);
} /* tal_BoardEnableCOM6 */

TAL_RESULT tal_BoardEnableCOM7 (void)
{
   return(TAL_ERR_COM_PORT_NOHW);
} /* tal_BoardEnableCOM7 */

TAL_RESULT tal_BoardEnableCOM8 (void)
{
   return(TAL_ERR_COM_PORT_NOHW);
} /* tal_BoardEnableCOM8 */

/*************************************************************************/
/*  tal_BoardEnableCANx                                                  */
/*                                                                       */
/*  Board and hardware depending functionality to enable the CAN port.   */
/*  If this port is not supported by the board, return an error.         */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: TAL_OK / error cause                                         */
/*************************************************************************/
TAL_RESULT tal_BoardEnableCAN1 (void)
{
   return(TAL_ERR_CAN_PORT_NOHW);
} /* tal_BoardEnableCAN1 */

TAL_RESULT tal_BoardEnableCAN2 (void)
{
   return(TAL_ERR_CAN_PORT_NOHW);
} /* tal_BoardEnableCAN2 */

/*************************************************************************/
/*  tal_BoardGetMACAddress                                               */
/*                                                                       */
/*  Retrieve the MAC address of the board.                               */
/*  In case of an error, a default address will be used.                 */
/*                                                                       */
/*  In    : pAddress                                                     */
/*  Out   : pAddress                                                     */
/*  Return: TAL_OK / TAL_ERROR                                           */
/*************************************************************************/
TAL_RESULT tal_BoardGetMACAddress (uint8_t *pAddress)
{
   if (read_mac(pAddress) != kStatus_Success) 
   {
      /* Use a default address instead */
      pAddress[0] = 0x00;
      pAddress[1] = 0x1A;
      pAddress[2] = 0xF1;
      pAddress[3] = 0x99;
      pAddress[4] = 0x99;
      pAddress[5] = 0x99;
   }
   
   return(TAL_OK);
} /* tal_BoardGetMACAddress */

/*************************************************************************/
/*  tal_BoardRTCSetTM                                                    */
/*                                                                       */
/*  Set the onboard RTC time by TM                                       */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_BoardRTCSetTM (struct tm *pTM)
{
   (void)pTM;
} /* tal_BoardRTCSetTM */

/*************************************************************************/
/*  tal_BoardRTCSetUnixtime                                              */
/*                                                                       */
/*  Set the onboard RTC time by Unixtime                                 */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_BoardRTCSetUnixtime (uint32_t Unixtime)
{
   (void)Unixtime;
} /* tal_BoardRTCSetUnixtime */

/*************************************************************************/
/*  tal_BoardRTC2System                                                  */
/*                                                                       */
/*  Set the system time from the RTC.                                    */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_BoardRTC2System (void)
{
} /* tal_BoardRTC2System */

#endif /* USE_BOARD_EA1062 */

/*** EOF ***/
