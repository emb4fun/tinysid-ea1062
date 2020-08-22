/**************************************************************************
*  Copyright (c) 2020 by Michael Fischer (www.emb4fun.de).
*  All rights reserved.
*
*  The source is partial based on the Freescale / NXP example.
*  Therefore partial:
*
*     Copyright (c) 2015, Freescale Semiconductor, Inc.
*     Copyright 2016-2018 NXP
*     All rights reserved.
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
*  04.07.2020  mifi  First Version, tested with the i.MX RT1050-EVK board.
**************************************************************************/

/*=======================================================================*/
/*  Includes                                                             */
/*=======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tal.h"
#include "tcts.h"
#include "fsl_sdmmc_event.h"

/*=======================================================================*/
/*  Global                                                               */
/*=======================================================================*/

/*=======================================================================*/
/*  Extern                                                               */
/*=======================================================================*/

/*=======================================================================*/
/*  All Structures and Common Constants                                  */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all global Data                                        */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Data                                         */
/*=======================================================================*/

static OS_SEMA SemaTransferComplete;
static OS_SEMA SemaCardDetect;

/*=======================================================================*/
/*  Definition of all local Procedures                                   */
/*=======================================================================*/

static OS_SEMA *SDMMCEVENT_GetInstance (sdmmc_event_t eventType)
{
   OS_SEMA *pSema = NULL;
   
   switch (eventType)
   {
      case kSDMMCEVENT_TransferComplete:
      {
         pSema = &SemaTransferComplete;
         break;
      }         
            
      case kSDMMCEVENT_CardDetect:
      {
         pSema = &SemaCardDetect;
         break;
      }         
      
      default:
      {
         pSema = NULL;
         break;
      }         
   }

   return(pSema);
} /* SDMMCEVENT_GetInstance */

/*=======================================================================*/
/*  All code exported                                                    */
/*=======================================================================*/

bool SDMMCEVENT_Create (sdmmc_event_t eventType)
{
   OS_SEMA *pSema = SDMMCEVENT_GetInstance(eventType);
   
   if (pSema)
   {
      OS_SemaCreate(pSema, 0, 1);
      return(true);
   }
   
   return(false);   
} /* SDMMCEVENT_Create */


bool SDMMCEVENT_Wait (sdmmc_event_t eventType, uint32_t timeoutMilliseconds)
{
   int       rc;
   uint32_t  timeoutTicks;
   OS_SEMA *pSema = SDMMCEVENT_GetInstance(eventType);

   if ((timeoutMilliseconds != 0) && (pSema != NULL))
   {
      if (timeoutMilliseconds == ~0U)
      {
         timeoutTicks = OS_WAIT_INFINITE;
      }
      else
      {
         timeoutTicks = OS_MS_2_TICKS(timeoutMilliseconds);
      }
      
      rc = OS_SemaWait(pSema, timeoutTicks);
      if (OS_RC_OK == rc)
      {
         return(true);  /* event taken */
      }
      else
      {
         return(false); /* timeout */
      }
   }
   else
   {
      return(false);
   }
   
} /* SDMMCEVENT_Wait */   


bool SDMMCEVENT_Notify (sdmmc_event_t eventType)
{
   OS_SEMA *pSema = SDMMCEVENT_GetInstance(eventType);

   OS_SemaSignalFromInt(pSema);
   
   return(true);
} /* SDMMCEVENT_Notify */


void SDMMCEVENT_Delay (uint32_t milliseconds)
{
   OS_TimeDly(OS_MS_2_TICKS(milliseconds));
} /* SDMMCEVENT_Delay */

/*** EOF ***/
