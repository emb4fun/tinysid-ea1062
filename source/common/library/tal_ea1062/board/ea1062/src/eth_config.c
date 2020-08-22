/**************************************************************************
*  Copyright (c) 2020 by Michael Fischer (www.emb4fun.de).
*  All rights reserved.
*
*  Based on an example from ST. Therefore partial copyright:
*  Copyright (c) 2016 STMicroelectronics
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
*  07.06.2020  mifi  First Version.
**************************************************************************/
#define __ETH_CONFIG_C__

/*
 * Copyright 2017-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * The hardware naming for the MACs are ENET0 and ENET2.
 * But the logical names are enet1 and enet2.
 */

/*=======================================================================*/
/*  Include                                                              */
/*=======================================================================*/
#include <stdint.h>
#include "tal.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_gpio.h"


#if !defined(FSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE)
   Error: FSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE must be definedby the preprocessor;
#endif   

/*=======================================================================*/
/*  Extern                                                               */
/*=======================================================================*/

/*=======================================================================*/
/*  All Structures and Common Constants                                  */
/*=======================================================================*/

#if !defined(ETH_MAX_IFACE)
#define ETH_MAX_IFACE   1
#endif

/*=======================================================================*/
/*  Definition of all local Data                                         */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Procedures                                   */
/*=======================================================================*/

/*************************************************************************/
/*  init_module_clock                                                    */
/*                                                                       */
/*  Enable PLL output for ENET1 and ENET2.                               */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
static void init_module_clock (void)
{
   clock_enet_pll_config_t config;
   
   memset(&config, 0x00, sizeof(config));
   
   config.enableClkOutput    = true;
   config.loopDivider        = 1;
   config.enableClkOutput25M = false;
   
   CLOCK_InitEnetPll(&config);
   
} /* init_module_clock */

/*************************************************************************/
/*  enet1_pin_mux                                                        */
/*                                                                       */
/*  Configures pin routing and optionally pin electrical features.       */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
static void enet1_pin_mux (void)
{
   CLOCK_EnableClock(kCLOCK_Iomuxc);           /* iomuxc clock (iomuxc_clk_enable): 0x03u */

   IOMUXC_SetPinMux(
       IOMUXC_GPIO_AD_B0_09_GPIO1_IO09,        /* GPIO_AD_B0_09 is configured as GPIO1_IO09 */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_AD_B0_10_GPIO1_IO10,        /* GPIO_AD_B0_10 is configured as GPIO1_IO10 */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
      
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_04_ENET_RX_DATA00,       /* GPIO_B1_04 is configured as ENET_RX_DATA00 */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_05_ENET_RX_DATA01,       /* GPIO_B1_05 is configured as ENET_RX_DATA01 */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_06_ENET_RX_EN,           /* GPIO_B1_06 is configured as ENET_RX_EN */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_07_ENET_TX_DATA00,       /* GPIO_B1_07 is configured as ENET_TX_DATA00 */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_08_ENET_TX_DATA01,       /* GPIO_B1_08 is configured as ENET_TX_DATA01 */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_09_ENET_TX_EN,           /* GPIO_B1_09 is configured as ENET_TX_EN */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_10_ENET_REF_CLK,         /* GPIO_B1_10 is configured as ENET_REF_CLK */
       1U);                                    /* Software Input On Field: Force input path of pad GPIO_B1_10 */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_B1_11_ENET_RX_ER,           /* GPIO_B1_11 is configured as ENET_RX_ER */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_EMC_40_ENET_MDC,            /* GPIO_EMC_40 is configured as ENET_MDC */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
   IOMUXC_SetPinMux(
       IOMUXC_GPIO_EMC_41_ENET_MDIO,           /* GPIO_EMC_41 is configured as ENET_MDIO */
       0U);                                    /* Software Input On Field: Input Path is determined by functionality */
      
      
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_AD_B0_09_GPIO1_IO09,        /* GPIO_AD_B0_09 PAD functional properties : */
       0xB0A9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: medium(100MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_AD_B0_10_GPIO1_IO10,        /* GPIO_AD_B0_10 PAD functional properties : */
       0xB0A9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: medium(100MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_04_ENET_RX_DATA00,       /* GPIO_B1_04 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_05_ENET_RX_DATA01,       /* GPIO_B1_05 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_06_ENET_RX_EN,           /* GPIO_B1_06 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_07_ENET_TX_DATA00,       /* GPIO_B1_07 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_08_ENET_TX_DATA01,       /* GPIO_B1_08 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_09_ENET_TX_EN,           /* GPIO_B1_09 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_10_ENET_REF_CLK,         /* GPIO_B1_10 PAD functional properties : */
       0x31u);                                 /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/6
                                                  Speed Field: low(50MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Disabled
                                                  Pull / Keep Select Field: Keeper
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_B1_11_ENET_RX_ER,           /* GPIO_B1_11 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_EMC_40_ENET_MDC,            /* GPIO_EMC_40 PAD functional properties : */
       0xB0E9u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: max(200MHz)
                                                  Open Drain Enable Field: Open Drain Disabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */
   IOMUXC_SetPinConfig(
       IOMUXC_GPIO_EMC_41_ENET_MDIO,           /* GPIO_EMC_41 PAD functional properties : */
       0xB829u);                               /* Slew Rate Field: Fast Slew Rate
                                                  Drive Strength Field: R0/5
                                                  Speed Field: low(50MHz)
                                                  Open Drain Enable Field: Open Drain Enabled
                                                  Pull / Keep Enable Field: Pull/Keeper Enabled
                                                  Pull / Keep Select Field: Pull
                                                  Pull Up / Down Config. Field: 100K Ohm Pull Up
                                                  Hyst. Enable Field: Hysteresis Disabled */

} /* enet1_pin_mux */

/*************************************************************************/
/*  enet1_reset_phy                                                      */
/*                                                                       */
/*  Reset the PHY of enet1 (ENET0).                                      */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
static void enet1_reset_phy (void)
{
   gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
   
   /*
    * Code from the lwip_iperf_bm.c example
    * <sdk>\boards\evkmimxrt1060\lwip_examples\lwip_iperf\bm
    */
   
   GPIO_PinInit(GPIO1, 9, &gpio_config);
   GPIO_PinInit(GPIO1, 10, &gpio_config);
   /* pull up the ENET_INT before RESET. */
   GPIO_WritePinOutput(GPIO1, 10, 1);
   GPIO_WritePinOutput(GPIO1, 9, 0);
   OS_TimeDly(100);
   GPIO_WritePinOutput(GPIO1, 9, 1);   

} /* enet1_reset */

/*=======================================================================*/
/*  All code exported                                                    */
/*=======================================================================*/

/*************************************************************************/
/*  BoardETHConfig                                                       */
/*                                                                       */
/*  Configures pin routing and optionally pin electrical features.       */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void BoardETHConfig (void)
{
   /* 
    * Init ENET0 and ENET2 clock 
    */
   init_module_clock();

   
   /* 
    * Cconfigure ENET0
    */
   enet1_pin_mux();
   IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true); /*lint !e747*/
   enet1_reset_phy();

} /* BoardETHConfig */

/*************************************************************************/
/*  BoardGetPhyAddress                                                   */
/*                                                                       */
/*  Return PHY address depending of the iface.                           */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
uint32_t BoardGetPhyAddress (int iface)
{
   uint32_t addr = 2;
   
   /* Here iface can be 1 or 2 */
   if (1 == iface)
   {
      addr = 1;
   }
   
   return(addr);
} /* BoardGetPhyAddress */

/*** EOF ***/
