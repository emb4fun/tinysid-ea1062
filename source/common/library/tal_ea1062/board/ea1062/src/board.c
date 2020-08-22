/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "board.h"
#include "fsl_iomuxc.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_SD_Pin_Config(uint32_t speed, uint32_t strength)
{
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_USDHC1_CMD,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_USDHC1_CLK,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(0) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_USDHC1_DATA0,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_USDHC1_DATA1,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_04_USDHC1_DATA2,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_05_USDHC1_DATA3,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
}


/*************************************************************************/
/*  CalculateMPURegion                                                   */
/*                                                                       */
/*  Calculate the MPU region size for by the given memory size.          */
/*                                                                       */
/*  In    : dSize                                                        */
/*  Out   : none                                                         */
/*  Return: MPURegionSize                                                */
/*************************************************************************/
static uint8_t CalculateMPURegion (uint32_t dSize)
{
   uint32_t i;

   i = 0;
   while ((dSize >> i) > 0x1U)
   {
      i++;
   }
   i = i - 1;
   
   return(i);   
} /* CalculateMPURegion */

/*************************************************************************/
/*  BOARD_ConfigMPU                                                      */
/*                                                                       */
/*  Configure MPU.                                                       */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void BOARD_ConfigMPU (void)
{
   /* Disable I cache and D cache */
   if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
   {
      SCB_DisableICache();
   }
   if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
   {
      SCB_DisableDCache();
   }

   /* Disable MPU */
   ARM_MPU_Disable();

   /* MPU configure:
    * Use ARM_MPU_RASR(DisableExec, AccessPermission, TypeExtField, IsShareable, IsCacheable, IsBufferable,
    * SubRegionDisable, Size)
    * API in mpu_armv7.h.
    * param DisableExec       Instruction access (XN) disable bit,0=instruction fetches enabled, 1=instruction fetches
    * disabled.
    * param AccessPermission  Data access permissions, allows you to configure read/write access for User and
    * Privileged mode.
    *      Use MACROS defined in mpu_armv7.h:
    * ARM_MPU_AP_NONE/ARM_MPU_AP_PRIV/ARM_MPU_AP_URO/ARM_MPU_AP_FULL/ARM_MPU_AP_PRO/ARM_MPU_AP_RO
    * Combine TypeExtField/IsShareable/IsCacheable/IsBufferable to configure MPU memory access attributes.
    *  TypeExtField  IsShareable  IsCacheable  IsBufferable   Memory Attribtue    Shareability        Cache
    *     0             x           0           0             Strongly Ordered    shareable
    *     0             x           0           1              Device             shareable
    *     0             0           1           0              Normal             not shareable   Outer and inner write
    * through no write allocate
    *     0             0           1           1              Normal             not shareable   Outer and inner write
    * back no write allocate
    *     0             1           1           0              Normal             shareable       Outer and inner write
    * through no write allocate
    *     0             1           1           1              Normal             shareable       Outer and inner write
    * back no write allocate
    *     1             0           0           0              Normal             not shareable   outer and inner
    * noncache
    *     1             1           0           0              Normal             shareable       outer and inner
    * noncache
    *     1             0           1           1              Normal             not shareable   outer and inner write
    * back write/read acllocate
    *     1             1           1           1              Normal             shareable       outer and inner write
    * back write/read acllocate
    *     2             x           0           0              Device              not shareable
    *  Above are normal use settings, if your want to see more details or want to config different inner/outter cache
    * policy.
    *  please refer to Table 4-55 /4-56 in arm cortex-M7 generic user guide <dui0646b_cortex_m7_dgug.pdf>
    * param SubRegionDisable  Sub-region disable field. 0=sub-region is enabled, 1=sub-region is disabled.
    * param Size              Region size of the region to be configured. use ARM_MPU_REGION_SIZE_xxx MACRO in
    * mpu_armv7.h.
    */

   /* MPU Configuration
   The M7 Core has 16 entires: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0489d/Chdecfea.html
   Compare chapter "Chapter 32 ARM Cortex M7 Platform (M7)"

   The M4 Core has 8 entries: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439b/Chdecfea.html
   compare chapter "33.5.4 Memory Protection Unit (MPU)" */

   // Region 0 setting: Memory with Device type, not shareable, non-cacheable.           - ITCM + ROM
   MPU->RBAR = ARM_MPU_RBAR(0, 0x00000000U);
   MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_1GB);

   // Region 1 setting: Memory with Normal type, not shareable, outer/inner write back   - ITCM
   extern const uint32_t __ITCM_segment_start__;
   extern const uint32_t __ITCM_segment_size__;
   extern const uint32_t __ITCM_segment_end__;
   MPU->RBAR = ARM_MPU_RBAR(1, (uint32_t)&__ITCM_segment_start__);
   MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, CalculateMPURegion((uint32_t)&__ITCM_segment_size__) );

   // Region 2 setting: Memory with Normal type, not shareable, outer/inner write back   - DTCM
   extern const uint32_t __DTCM_segment_start__;
   extern const uint32_t __DTCM_segment_size__;
   extern const uint32_t __DTCM_segment_end__;
   MPU->RBAR = ARM_MPU_RBAR(2, (uint32_t)&__DTCM_segment_start__);
   MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, CalculateMPURegion((uint32_t)&__DTCM_segment_size__) );

   // Region 3 setting: Memory with Normal type, not shareable, outer/inner write back   - OCRAM
   extern const uint32_t __OCRAM_segment_start__;
   extern const uint32_t __OCRAM_segment_size__;
   extern const uint32_t __OCRAM_segment_end__;
   MPU->RBAR = ARM_MPU_RBAR(3, (uint32_t)&__OCRAM_segment_start__);
   MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, CalculateMPURegion((uint32_t)&__OCRAM_segment_size__) );

   // Region 4 setting: Setting Memory with Device type, not shareable, non-cacheable.   - FlexSPI
   extern const uint32_t __FlexSPI_segment_start__;
   extern const uint32_t __FlexSPI_segment_size__;
   extern const uint32_t __FlexSPI_segment_end__;
   MPU->RBAR = ARM_MPU_RBAR(4, (uint32_t)&__FlexSPI_segment_start__);
   MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_RO, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_4MB);

   // Region 5 setting: Memory with Normal type, not shareable, outer/inner write back   - SDRAM
   extern const uint32_t __SDRAM_segment_start__;
   extern const uint32_t __SDRAM_segment_size__;
   extern const uint32_t __SDRAM_segment_end__;
   MPU->RBAR = ARM_MPU_RBAR(5, (uint32_t)&__SDRAM_segment_start__);
   MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_32MB);

   // Region 6 setting: Memory with Normal type, not shareable, non-cacheable            - SDRAM (NonCacheable)
   extern const uint32_t __NonCache_segment_start__;
   extern const uint32_t __NonCache_segment_size__;
   extern const uint32_t __NonCache_segment_end__;
   MPU->RBAR = ARM_MPU_RBAR(6, (uint32_t)&__NonCache_segment_start__);
   MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, CalculateMPURegion((uint32_t)&__NonCache_segment_size__) );

   /* Enable MPU */
   ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

   /* Enable I cache and D cache */
   SCB_EnableDCache();
   SCB_EnableICache();

} /* BOARD_ConfigMPU */

/*** EOF ***/

