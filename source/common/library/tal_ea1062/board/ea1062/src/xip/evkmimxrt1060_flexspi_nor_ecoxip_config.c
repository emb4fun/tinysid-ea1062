/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 /* This is a Serial NOR Configuration Block definition for Adesto's EcoXiP 
  * flash (ATXP032). Among other parameters it configues the system to operate
  * with:
  * - Octal-SPI
  * - Double Data Rate (DDR)
  * - SCLK = 133MHz
  */

#include "evkmimxrt1060_flexspi_nor_config.h"

/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.xip_board"
#endif


// Number of dummy cycles for non-SPI (quad/octal) modes
#define ECOXIP_READ_NON_SPI_DUMMY_CYCLES 18
#define CTRL_REG_BYTE3_VAL (((ECOXIP_READ_NON_SPI_DUMMY_CYCLES - 8) >> 1) | 0x10)

// EcoXiP command op codes
#define	EXIP_CMD_READ_STATUS_REG_BYTE1  0x05  // Read Status Register Byte 1
#define	EXIP_CMD_WRITE_ENABLE           0x06  // Write Enable
#define	EXIP_CMD_READARRAY              0x0B  // Read Array
#define	EXIP_CMD_WRITE_STAT_CTRL_REGS   0x71  // Write Status/Control Registers

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(XIP_BOOT_HEADER_ENABLE) && (XIP_BOOT_HEADER_ENABLE == 1)
#if defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__GNUC__)
__attribute__((section(".boot_hdr.conf")))
#elif defined(__ICCARM__)
#pragma location = ".boot_hdr.conf"
#endif

const flexspi_nor_config_t qspiflash_config = {
    .memConfig =
    {
        .tag = FLEXSPI_CFG_BLK_TAG,
        .version = FLEXSPI_CFG_BLK_VERSION,
        .readSampleClkSrc = kFlexSPIReadSampleClk_ExternalInputFromDqsPad,
        .csHoldTime = 3u,
        .csSetupTime = 3u,
        .columnAddressWidth = 0u,
        .deviceModeCfgEnable = 1,
		// Sequence for changing device mode. In this sequence we write to status/control regs 2-3.
        // This will switch EcoXiP to Octal-DDR mode and modify the number of dummy cycles used by it.
        .deviceModeSeq = {.seqId=14, .seqNum=1}, // index/size Status/Control Registers sequence
        .deviceModeArg = 0x88 | (CTRL_REG_BYTE3_VAL << 8), // values to be written to status/control regs 2-3
        // Enable DDR mode, Safe configuration
        .controllerMiscOption = (1u << kFlexSpiMiscOffset_DdrModeEnable) |
                                (1u << kFlexSpiMiscOffset_SafeConfigFreqEnable),
        .deviceType = kFlexSpiDeviceType_SerialNOR, // serial NOR
        .sflashPadType = kSerialFlash_8Pads,
        .serialClkFreq = kFlexSpiSerialClk_133MHz,
		.lutCustomSeqEnable = 0, // Use pre-defined LUT sequence index and number
        .sflashA1Size = 4u * 1024u * 1024u,
        .dataValidTime = {[0] = 20}, //2ns from DQS to data
        .busyOffset = 0, // busy bit in bit 0
        .busyBitPolarity = 0, // busy bit is 1 when device is busy
        .lookupTable =
            {
                // Read
                [0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_8PAD, EXIP_CMD_READARRAY, RADDR_DDR, FLEXSPI_8PAD, 0x20),
                [1] = FLEXSPI_LUT_SEQ(DUMMY_DDR, FLEXSPI_8PAD,(ECOXIP_READ_NON_SPI_DUMMY_CYCLES*2+1), READ_DDR, FLEXSPI_8PAD, 0x80),
				// Read Status
                [4] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_8PAD, EXIP_CMD_READ_STATUS_REG_BYTE1, DUMMY_DDR, FLEXSPI_8PAD, 0x08),
				[5] = FLEXSPI_LUT_SEQ(READ_DDR, FLEXSPI_8PAD, 0x01, STOP, FLEXSPI_1PAD, 0x0),
				// Write Enable
				[12] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, EXIP_CMD_WRITE_ENABLE, STOP, FLEXSPI_1PAD, 0x0),
				// Write Status/Control Registers (this specifc sequence will writes 2 bytes to status/control regs 2-3)
				[56] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, EXIP_CMD_WRITE_STAT_CTRL_REGS, CMD_SDR, FLEXSPI_1PAD, 0x02),
				[57] = FLEXSPI_LUT_SEQ(WRITE_SDR, FLEXSPI_1PAD, 0x02, STOP, FLEXSPI_1PAD, 0x0),
            },
    },
    .pageSize = 256u,
    .sectorSize = 4096u, // 4K - that's actually a block not a sector (has to match erase size)
    .ipcmdSerialClkFreq = 1, // 30MHz
    .blockSize = 4096u,
    .isUniformBlockSize = true,
};
#endif /* XIP_BOOT_HEADER_ENABLE */
