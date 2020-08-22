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
#define __TALCPU_C__

/*=======================================================================*/
/*  Include                                                              */
/*=======================================================================*/
#include <stdlib.h>
#include "tal.h"

#include "board.h"
#include "clock_config.h"

#include "fsl_common.h"
#include "fsl_cache.h"
#include "fsl_trng.h"
#include "fsl_iomuxc.h"
#include "fsl_wdog.h"

/*=======================================================================*/
/*  All Structures and Common Constants                                  */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Data                                         */
/*=======================================================================*/

static uint32_t dHiResPeriod   = 0;
static uint32_t dHiResPeriod16 = 0;

static trng_config_t trngConfig;

/*=======================================================================*/
/*  Definition of all local Procedures                                   */
/*=======================================================================*/

/*************************************************************************/
/*  InitWatchdog                                                         */
/*                                                                       */
/*  Initialize the hardware output signal of the watchdog.               */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
static void InitWatchdog (void)
{
   CLOCK_EnableClock(kCLOCK_Iomuxc);
   
   IOMUXC_SetPinMux(IOMUXC_GPIO_B1_13_WDOG1_B, 0U);

   /*
    * IOMUXC_GPIO_B1_13_WDOG1_B PAD functional properties :
    * Slew Rate Field: Slow Slew Rate
    * Drive Strength Field: R0/6
    * Speed Field: medium(100MHz)
    * Open Drain Enable Field: Open Drain Disabled
    * Pull / Keep Enable Field: Pull/Keeper Enabled
    * Pull / Keep Select Field: Keeper
    * Pull Up / Down Config. Field: 100K Ohm Pull Down
    * Hyst. Enable Field: Hysteresis Disabled
    */
   IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_13_WDOG1_B, 0x10B0u);

} /* InitWatchdog */

/*************************************************************************/
/*  NewSysTick_Config                                                    */
/*                                                                       */
/*  Based on the "core_cm4.h" version, but an AHB clock divided by 8 is  */
/*  used for the SysTick clock source.                                   */
/*                                                                       */
/*  The function initializes the System Timer and its interrupt, and     */
/*  starts the System Tick Timer. Counter is in free running mode to     */
/*  generate periodic interrupts.                                        */
/*                                                                       */
/*  In    : ticks                                                        */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
static uint32_t NewSysTick_Config (uint32_t ticks)
{
   /* Reload value impossible */
   if ((ticks - 1) > SysTick_LOAD_RELOAD_Msk)  return (1);

   /* Set reload register */
   SysTick->LOAD = ticks - 1;
  
   /* Set Priority for Systick Interrupt */
   NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIO);
  
   /* Load the SysTick Counter Value */
   SysTick->VAL = 0;
   
   /* 
    * SysTick IRQ and SysTick Timer must be
    * enabled with tal_CPUSysTickStart later.
    */
    
   return(ticks);
} /* NewSysTick_Config */

/*=======================================================================*/
/*  All code exported                                                    */
/*=======================================================================*/

/*************************************************************************/
/*  tal_CPUInit                                                          */
/*                                                                       */
/*  "Initialize" the CPU.                                                */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPUInit (void)
{
   BOARD_ConfigMPU();
   BOARD_BootClockRUN();

   /* Update clock info */
   SystemCoreClockUpdate(); 
   
   /* 
    * Init SysTick 
    */
   dHiResPeriod = NewSysTick_Config(SystemCoreClock / OS_TICKS_PER_SECOND);
   
   /* 
    * dHiResPeriod value must be a 16bit count, but here it is
    * bigger. Therefore dHiResPeriod must be divided by 16.
    */
   dHiResPeriod16 = dHiResPeriod / 16;

   /* Configure Priority Grouping (core_cm4.h) */
   NVIC_SetPriorityGrouping(7);
} /* tal_CPUInit */

/*************************************************************************/
/*  tal_CPUSysTickStart                                                  */
/*                                                                       */
/*  Start the SysTick.                                                   */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPUSysTickStart (void)
{
   /* Enable SysTick IRQ and SysTick Timer */
   SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
} /* tal_CPUSysTickStart */

/*************************************************************************/
/*  tal_CPUIrqEnable                                                     */
/*                                                                       */
/*  Enable the given IRQ.                                                */
/*                                                                       */
/*  In    : IRQ                                                          */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPUIrqEnable (int IRQ)
{
   NVIC_EnableIRQ((IRQn_Type)IRQ);
} /* tal_CPUIrqEnable */

/*************************************************************************/
/*  tal_CPUIrqDisable                                                    */
/*                                                                       */
/*  Disable the given IRQ.                                               */
/*                                                                       */
/*  In    : IRQ                                                          */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPUIrqDisable (int IRQ)
{
   NVIC_DisableIRQ((IRQn_Type)IRQ);
} /* tal_CPUIrqDisable */

/*************************************************************************/
/*  tal_CPUIrqDisableAll                                                 */
/*                                                                       */
/*  Disable all interrupts.                                              */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPUIrqDisableAll (void)
{
   /*lint +rw(_to_semi) */
   /*lint -d__disable_irq=_to_semi */

   __disable_irq();
   
} /* tal_CPUIrqDisableAll */

/*************************************************************************/
/*  tal_CPUIrqSetPriority                                                */
/*                                                                       */
/*  Set priority of the given IRQ.                                       */
/*                                                                       */
/*  In    : IRQ, Priority                                                */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPUIrqSetPriority (int IRQ, int Priority)
{
   NVIC_SetPriority((IRQn_Type)IRQ, (uint32_t)Priority);
} /* tal_CPUIrqSetPriority */

/*************************************************************************/
/*  tal_CPUStatGetHiResPeriod                                            */
/*                                                                       */
/*  Return the HiResPeriod value.                                        */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: HiResPeriod                                                  */
/*************************************************************************/
uint32_t tal_CPUStatGetHiResPeriod (void)
{
   return(dHiResPeriod16);
} /* tal_CPUStatGetHiResPeriod */

/*************************************************************************/
/*  tal_CPUStatGetHiResCnt                                               */
/*                                                                       */
/*  Return the HiRes counter.                                            */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: HiRes counter                                                */
/*************************************************************************/
uint32_t tal_CPUStatGetHiResCnt (void)
{
   uint32_t dValue;
   
   /* Get milliseconds */
   dValue  = (OS_TimeGet() << 16);
   
   /* The SysTick counts down from HiResPeriod, therefore HiResPeriod - X */
   /* HiResPeriod is used, therefore divide the time by 16 too */
   dValue |= (uint16_t)((dHiResPeriod - SysTick->VAL) / 16);
   
   return(dValue);
} /* tal_CPUStatGetHiResCnt */

/*************************************************************************/
/*  tal_CPUGetFrequencyCPU                                               */
/*                                                                       */
/*  Return the clock frequency of the CPU in MHz.                        */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: Frequency                                                    */
/*************************************************************************/
uint32_t tal_CPUGetFrequencyCPU (void)
{
   return( CLOCK_GetFreq(kCLOCK_CpuClk) );
} /* tal_CPUGetFrequencyCPU */

uint32_t tal_CPUGetFrequencyUSB (void)
{
   return( CLOCK_GetFreq(kCLOCK_Usb1PllClk) );
}

uint32_t tal_CPUGetFrequencyAHB (void)
{
   return( CLOCK_GetFreq(kCLOCK_AhbClk) );
}

uint32_t tal_CPUGetFrequencyIPG (void)
{
   return( CLOCK_GetFreq(kCLOCK_IpgClk) );
}

uint32_t tal_CPUGetFrequencyOSC (void)
{
   return( CLOCK_GetFreq(kCLOCK_OscClk) );
}

/*************************************************************************/
/*  tal_CPURngInit                                                       */
/*                                                                       */
/*  Initialize the random number generator.                              */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPURngInit (void)
{
   TRNG_GetDefaultConfig(&trngConfig);
   
   /* Set sample mode of the TRNG ring oscillator to Von Neumann, for better random data.*/
   /* Initialize TRNG */
   TRNG_Init(TRNG, &trngConfig);

} /* tal_CPURngInit */

/*************************************************************************/
/*  tal_CPURngDeInit                                                     */
/*                                                                       */
/*  DeInitialize the random number generator.                            */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPURngDeInit (void)
{
} /* tal_CPURngDeInit */

/*************************************************************************/
/*  tal_CPURngHardwarePoll                                               */
/*                                                                       */
/*  Generates a 32-bit random number.                                    */
/*                                                                       */
/*  In    : pData, dSize                                                 */
/*  Out   : pData                                                        */
/*  Return: TALOK / TAL_ERROR                                            */
/*************************************************************************/
TAL_RESULT tal_CPURngHardwarePoll (uint8_t *pData, uint32_t dSize)
{
   TAL_RESULT  Error = TAL_ERROR;
   status_t    Status;
   
   Status = TRNG_GetRandomData(TRNG, pData, dSize);
   if (kStatus_Success == Status)
   {
      Error = TAL_OK;
   }

   return(Error);   
} /* tal_CPURngHardwarePoll */  

/*************************************************************************/
/*  tal_CPUReboot                                                        */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void tal_CPUReboot (void)
{
   term_printf("\r\n*** Reboot ***\r\n");
   
#if defined(__FLASH__) || defined(__FLASH_2_SDRAM__) || defined(__SDRAM_BOOT__)  
{
   wdog_config_t config;
   
   InitWatchdog();   
   
   WDOG_GetDefaultConfig(&config);
   config.timeoutValue = 0x00;   /* Timeout value is (0x00 + 1)/2 = 0.5 sec. */
   config.enableTimeOutAssert = true;
   WDOG_Init(WDOG1, &config);
}   
#endif   

   OS_TimeDly(500);
   while (1)
   {
      __asm__ ("nop");
   }
   
} /* tal_CPUReboot */

/*************************************************************************/
/*  SysTick_Handler                                                      */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void SysTick_Handler (void)
{
   TAL_CPU_IRQ_ENTER();
   
   OS_TimerCallback();
   
   TAL_CPU_IRQ_EXIT();
} /* SysTick_Handler */

/*** EOF ***/
