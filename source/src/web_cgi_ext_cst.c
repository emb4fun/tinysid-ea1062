/**************************************************************************
*  Copyright (c) 2019 by Michael Fischer (www.emb4fun.de).
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
*  16.03.2019  mifi  First version.
*  04.07.2019  mifi  Changed from LPC to STM. 
*  14.07.2019  mifi  First version for the BeagleBone Black.
**************************************************************************/
#define __WEB_CGI_EXT_CST_C__

/*=======================================================================*/
/*  Includes                                                             */
/*=======================================================================*/
#include <stdint.h>
#include <stdio.h>

#include "terminal.h"
#include "ipstack.h"
#include "ipweb.h"
#include "web_sid.h"

/*=======================================================================*/
/*  All extern data                                                      */
/*=======================================================================*/

/*=======================================================================*/
/*  All Structures and Common Constants                                  */
/*=======================================================================*/

#define PERMISSION_USER1   0x00000001
#define PERMISSION_USER2   0x00000002

/*=======================================================================*/
/*  Definition of all global Data                                        */
/*=======================================================================*/

int nUser1Value = 1;
int nUser2Value = 2;

/*=======================================================================*/
/*  Definition of all local Data                                         */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Procedures                                   */
/*=======================================================================*/

/*=======================================================================*/
/*  All code exported                                                    */
/*=======================================================================*/

/*************************************************************************/
/*  User1Set                                                             */
/*                                                                       */
/*  In    : hs                                                           */
/*  Out   : none                                                         */
/*  Return: 0 = OK / -1 = ERROR                                          */
/*************************************************************************/
int User1Set (HTTPD_SESSION *hs)
{
   long      Avail;
   char    *pArg;
   char    *pVal;
   char    *pRedir = NULL;
   int      nValue = 0;
   uint32_t dPermission;
   
   /* For this CGI the permission of user1 is needed */
   dPermission = hs->s_req.req_sid_perm & PERMISSION_USER1;
   
   Avail = hs->s_req.req_length;
   while (Avail) 
   {
      pArg = HttpArgReadNext(hs, &Avail);
      if (pArg != NULL)
      {
         pVal = HttpArgValue(&hs->s_req);
         if (pVal)
         {
//            term_printf("%s: %s\r\n", pArg, pVal);
         
            if      (strcmp(pArg, "value") == 0)
            {
               nValue = atoi(pVal);
            }
            else if (strcmp(pArg, "redir") == 0)
            {
               pRedir = strdup(pVal);
            }
         }            
      }
   }   

   /* Check if all required parameters are available */
   if (pRedir != NULL)
   {
      if (dPermission != 0)
      {
         nUser1Value = nValue;
         HttpSendRedirection(hs, 303, pRedir, NULL);
      }
      else
      {
         HttpSendRedirection(hs, 303, "/403.htm", NULL);
      }   
   }   
   
   free(pRedir);

   return(0);
} /* User1Set */

/*************************************************************************/
/*  User2Set                                                             */
/*                                                                       */
/*  In    : hs                                                           */
/*  Out   : none                                                         */
/*  Return: 0 = OK / -1 = ERROR                                          */
/*************************************************************************/
int User2Set (HTTPD_SESSION *hs)
{
   long      Avail;
   char    *pArg;
   char    *pVal;
   char    *pRedir = NULL;
   int      nValue = 0;
   uint32_t dPermission;
   
   /* For this CGI the permission of user2 is needed */
   dPermission = hs->s_req.req_sid_perm & PERMISSION_USER2;
   
   Avail = hs->s_req.req_length;
   while (Avail) 
   {
      pArg = HttpArgReadNext(hs, &Avail);
      if (pArg != NULL)
      {
         pVal = HttpArgValue(&hs->s_req);
         if (pVal)
         {
//            term_printf("%s: %s\r\n", pArg, pVal);
         
            if      (strcmp(pArg, "value") == 0)
            {
               nValue = atoi(pVal);
            }
            else if (strcmp(pArg, "redir") == 0)
            {
               pRedir = strdup(pVal);
            }
         }            
      }
   }   

   /* Check if all required parameters are available */
   if (pRedir != NULL)
   {
      if (dPermission != 0)
      {
         nUser2Value = nValue;
         HttpSendRedirection(hs, 303, pRedir, NULL);
      }
      else
      {
         HttpSendRedirection(hs, 303, "/403.htm", NULL);
      }   
   }   
   
   free(pRedir);

   return(0);
} /* User2Set */

/*** EOF ***/
