/**************************************************************************
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
*  04.07.2020  mifi  First Version for the iMX RT1062 Developer’s Kit.
**************************************************************************/

/*=======================================================================*/
/*  Includes                                                             */
/*=======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tal.h"
#include "ff.h"
#include "diskio.h"

#include "fsl_sd.h"
#include "fsl_cache.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"

#if !defined(FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL)
   Error: FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL must be defined to 1;
#endif   

/*=======================================================================*/
/*  Global                                                               */
/*=======================================================================*/


/*=======================================================================*/
/*  Extern                                                               */
/*=======================================================================*/

void USDHC1_DriverIRQHandler (void);
void BOARD_InitPins (void);

/*=======================================================================*/
/*  All Structures and Common Constants                                  */
/*=======================================================================*/

#define BLOCK_SIZE   512

#define SD_LED_ON()
#define SD_LED_OFF()

/*=======================================================================*/
/*  Definition of all global Data                                        */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Data                                         */
/*=======================================================================*/

/*
 * sd card descriptor 
 */
static sd_card_t SDCard; 


static DSTATUS DiskStatus = STA_NOINIT;
static OS_SEMA FSSema;


/*! @brief SDMMC host detect card configuration */
static const sdmmchost_detect_card_t s_sdCardDetect = 
{
   .cdType = kSDMMCHOST_DetectCardByGpioCD,
   .cdTimeOut_ms = (~0U),
};

/*=======================================================================*/
/*  Definition of all local Procedures                                   */
/*=======================================================================*/

/*=======================================================================*/
/*  All code exported                                                    */
/*=======================================================================*/

/*************************************************************************/
/*  ff_memalloc                                                          */
/*                                                                       */
/*  Allocate a memory block                                              */
/*                                                                       */
/*  In    : msize                                                        */
/*  Out   : none                                                         */
/*  Return: Returns pointer to the allocated memory block                */
/*************************************************************************/
void *ff_memalloc (UINT msize)
{
   return(xmalloc(XM_ID_FS, msize));
} /* ff_memalloc */

/*************************************************************************/
/*  ff_memfree                                                           */
/*                                                                       */
/*  Free a memory block                                                  */
/*                                                                       */
/*  In    : mblock                                                       */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void ff_memfree (void *mblock)
{
   xfree(mblock);
} /* ff_memfree */

/*************************************************************************/
/*  ff_cre_syncobj                                                       */
/*                                                                       */
/*  Create a Synchronization Object                                      */
/*                                                                       */
/*  In    : vol,  corresponding logical drive being processed.           */
/*          sobj, pointer to return the created sync object.             */
/*  Out   : none                                                         */
/*  Return: 0 = Error / 1 = OK                                           */
/*************************************************************************/
int ff_cre_syncobj (BYTE vol, FF_SYNC_t *sobj)
{
   (void)vol;
   
   /* Init the semaphore */
   OS_SemaCreate(&FSSema, 1, 1);
   *sobj = &FSSema;

   return(1);
} /* ff_cre_syncobj */

/*************************************************************************/
/*  ff_del_syncobj                                                       */
/*                                                                       */
/*  Delete a Synchronization Object                                      */
/*                                                                       */
/*  In    : sobj, sync object tied to the logical drive to be deleted.   */
/*  Out   : none                                                         */
/*  Return: 0 = Error / 1 = OK                                           */
/*************************************************************************/
int ff_del_syncobj (FF_SYNC_t sobj)
{
   /* Reset the semaphore */
   OS_SemaDelete(sobj);
   
   return(1);
} /* ff_del_syncobj */

/*************************************************************************/
/*  ff_req_grant                                                         */
/*                                                                       */
/*  Request Grant to Access the Volume                                   */
/*                                                                       */
/*  In    : sobj, sync object to wait.                                   */
/*  Out   : none                                                         */
/*  Return: 1 = Got a grant / 0 =  Could not get a grant                 */
/*************************************************************************/
int ff_req_grant (FF_SYNC_t sobj)
{
   OS_SemaWait(sobj, OS_WAIT_INFINITE);
   
   return(1);
} /* ff_req_grant */

/*************************************************************************/
/*  ff_rel_grant                                                         */
/*                                                                       */
/*  Request Grant to Access the Volume                                   */
/*                                                                       */
/*  In    : sobj, Sync object to be signaled.                            */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void ff_rel_grant (FF_SYNC_t sobj)
{
   OS_SemaSignal(sobj);
} /* ff_rel_grant */

/*************************************************************************/
/*  disk_initialize                                                      */
/*                                                                       */
/*  Initialize a Drive                                                   */
/*                                                                       */
/*  In    : pdrv, physical drive nmuber to identify the drive            */
/*  Out   : none                                                         */
/*  Return: Status of Disk Functions                                     */
/*************************************************************************/
DSTATUS disk_initialize (uint8_t pdrv)
{
   static int InitDone = FALSE;
   
   if (DiskStatus & STA_NODISK) 
   {
      /* No card in the socket */
      return(DiskStatus);
   } 
   
   SD_LED_ON();

   if (0 == pdrv)
   {
      if (FALSE == InitDone)
      { 
         InitDone = TRUE;
         
         BOARD_InitPins();
         
         /* Configure system pll PFD0 fractional divider to 24, output clock is 528MHZ * 18 / 24 = 396 MHZ */
         CLOCK_InitSysPfd(kCLOCK_Pfd0, 24U);
         
         /* Configure USDHC clock source and divider */
         CLOCK_SetDiv(kCLOCK_Usdhc1Div, 0U);
         CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);
         
         /* Clear data first */
         memset(&SDCard, 0x00, sizeof(SDCard));
                           
         DiskStatus |= STA_NOINIT;
                        
      } /* end if (FALSE == InitDone) */
      

      /*
       * Initialize host interface
       */      
      if (false == SDCard.isHostReady)
      {
         /* Save host information. */
         SDCard.host.base           = SD_HOST_BASEADDR;
         SDCard.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
         
         /* Card detect type */
         SDCard.usrParam.cd = &s_sdCardDetect;
         
         /* Set timing mode, SDR104 is to high for the EA board, therefore use the SDR50 mode */
         SDCard.currentTiming = kSD_TimingSDR50Mode; /* SDR50 mode produce a clock of 99MHz */
         
         SD_HostInit(&SDCard);
         SD_PowerOffCard(SDCard.host.base, SDCard.usrParam.pwr);
      }
      
      
      if (DiskStatus & STA_NOINIT)
      {
         if (true == SD_IsCardPresent(&SDCard))
         {
            /* Reset host */
            SD_HostReset(&(SDCard.host));
            SD_PowerOnCard(SDCard.host.base, SDCard.usrParam.pwr);
         
            if (kStatus_Success != SD_CardInit(&SDCard))
            {
               SD_CardDeinit(&SDCard);
               memset(&SDCard, 0U, sizeof(SDCard));
                 
               DiskStatus |= STA_NOINIT;
            }
            else
            {
               term_printf("SD busClock Hz: %d\r\n\r\n", SDCard.busClock_Hz);
               DiskStatus &= ~STA_NOINIT;
            }
         }
      }
      
   } /* end if (0 == pdrv) */

   SD_LED_OFF();

   return(DiskStatus);
} /* disk_initialize */

/*************************************************************************/
/*  disk_removed                                                         */
/*                                                                       */
/*  Medium was removed                                                   */
/*                                                                       */
/*  In    : pdrv, physical drive nmuber to identify the drive            */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void disk_removed (BYTE pdrv)
{
   if (0 == pdrv)
   {
      /* Remove power first */
      SD_PowerOffCard(SDCard.host.base, SDCard.usrParam.pwr);
   
      /* DeInit card */
      SD_CardDeinit(&SDCard);
      memset(&SDCard, 0x00, sizeof(SDCard));

      DiskStatus |= (STA_NODISK | STA_NOINIT);
   }
         
} /* disk_removed */

/*************************************************************************/
/*  disk_status                                                          */
/*                                                                       */
/*  Get Drive Status                                                     */
/*                                                                       */
/*  In    : pdrv, physical drive nmuber to identify the drive            */
/*  Out   : none                                                         */
/*  Return: Status of Disk Functions                                     */
/*************************************************************************/
DSTATUS disk_status (uint8_t pdrv)
{
   if (0 == pdrv)
   {
      DiskStatus |= STA_NODISK;
      
      if (true == SD_IsCardPresent(&SDCard))
      {
         DiskStatus &= ~STA_NODISK;
      }
   }
   else
   {
      return(STA_NOINIT);
   } 

   return(DiskStatus);
} /* disk_status */

/*************************************************************************/
/*  disk_read                                                            */
/*                                                                       */
/*  Read Sector(s)                                                       */
/*                                                                       */
/*  In    : pdrv,   physical drive nmuber to identify the drive          */
/*          buff,   data buffer to store read data                       */
/*          sector, sector address in LBA                                */
/*          count,  number of sectors to read                            */
/*  Out   : none                                                         */
/*  Return: Results of Disk Functions                                    */
/*************************************************************************/
DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
   DRESULT  Result = RES_ERROR;
   uint32_t startAddr = (uint32_t)buff;


   if (0 == pdrv)
   {
      /* Check drive status */
      if (DiskStatus & STA_NOINIT)
      {
         return(RES_NOTRDY);
      }  
      
      SD_LED_ON();

      if (kStatus_Success == SD_ReadBlocks(&SDCard, buff, sector, count))
      {
         Result = RES_OK;
      }

      SD_LED_OFF();
   }

   return(Result);
} /* disk_read */

/*************************************************************************/
/*  disk_write                                                           */
/*                                                                       */
/*  Write Sector(s)                                                      */
/*                                                                       */
/*  In    : pdrv,   physical drive nmuber to identify the drive          */
/*          buff,   data to be written                                   */
/*          sector, sector address in LBA                                */
/*          count,  number of sectors to write                           */
/*  Out   : none                                                         */
/*  Return: Results of Disk Functions                                    */
/*************************************************************************/
DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
   DRESULT  Result = RES_ERROR;
   status_t Status;

   if (0 == pdrv)
   {
      /* Check drive status */
      if (DiskStatus & STA_NOINIT)
      {
         return(RES_NOTRDY);
      }  
      
      SD_LED_ON();

      if (kStatus_Success == SD_WriteBlocks(&SDCard, buff, sector, count))
      {
         Result = RES_OK;
      }

      SD_LED_OFF();
   }

   return(Result);
} /* disk_write */

/*************************************************************************/
/*  disk_ioctl                                                           */
/*                                                                       */
/*  In    : pdrv, physical drive nmuber to identify the drive            */
/*          cmd,  control code                                           */
/*          buff, buffer to send/receive control data                    */
/*  Out   : none                                                         */
/*  Return: Results of Disk Functions                                    */
/*************************************************************************/
DRESULT disk_ioctl (uint8_t pdrv, uint8_t cmd, void *buff)
{
   DRESULT Result = RES_PARERR;
  
   if ((0 == pdrv) && (buff != NULL))
   {
      /* Check drive status */
      if (DiskStatus & STA_NOINIT)
      {
         return(RES_NOTRDY);
      }
      
      SD_LED_ON();
      
      switch(cmd)
      {
         case CTRL_SYNC:         /* Make sure that no pending write process */
            Result = RES_OK;
            break;
            
         case GET_SECTOR_COUNT:  /* Get number of sectors on the disk (DWORD) */
            *(DWORD*)buff = SDCard.blockCount;
            Result = RES_OK;
            break;

         case GET_SECTOR_SIZE:   /* Get R/W sector size (WORD) */
            *(WORD*)buff = SDCard.blockSize;
            Result = RES_OK;
            break;
            
         case GET_BLOCK_SIZE:    /* Get erase block size in unit of sector (DWORD) */
            *(DWORD*)buff = SDCard.csd.eraseSectorSize;
            Result = RES_OK;
            break;

         default:
            Result = RES_PARERR;
            break;
      }
      
      SD_LED_OFF();
   }
   
   return(Result);
} /* disk_ioctl */

/*************************************************************************/
/*  get_fattime                                                          */
/*                                                                       */
/*  Gets Time from RTC                                                   */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: Time                                                         */
/*************************************************************************/
DWORD get_fattime (void)
{
   return(0);
}

void USDHC1_IRQHandler(void)
{
   USDHC1_DriverIRQHandler();
}

/*** EOF ***/
