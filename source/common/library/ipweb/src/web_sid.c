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
*  09.03.2019  mifi  First Version.
**************************************************************************/
#define __WEB_SID_C__

/*=======================================================================*/
/*  Includes                                                             */
/*=======================================================================*/

#include <string.h>
#include <stdlib.h>
#include "tal.h"
#include "tcts.h"
#include "ipweb.h"
#include "xmem.h"

#if (_IP_WEB_SID_SUPPORT >= 1)

#include "mbedtls/sha256.h"

/*=======================================================================*/
/*  Extern                                                               */
/*=======================================================================*/

/*=======================================================================*/
/*  All Structures and Common Constants                                  */
/*=======================================================================*/

#define sha2_context    mbedtls_sha256_context
#define sha2_start(_a)  mbedtls_sha256_starts_ret(_a, 0)
#define sha2_update     mbedtls_sha256_update_ret
#define sha2_finish     mbedtls_sha256_finish_ret


#define LOGIN_ERROR_CNT_MAX   3
#define LOGIN_TIMEOUT_SEC     60

#define SID_TIMEOUT_SEC       (1*60)
#define SID_START             "sid="
#define SID_LEN               32
#define SID_NONCE_LEN         16
#define SID_COOKIE_SIZE       64

#define USER_SIZE             16

#define SALT_SIZE             8
#define SHA2_HASH_SIZE        32

#define USER_LIST_CNT         8
#define SID_LIST_CNT          8

typedef struct _user_
{
   char      User[USER_SIZE+1];
   uint8_t   Salt[SALT_SIZE];
   uint8_t   Hash[SHA2_HASH_SIZE];
   uint32_t dPermission;
} user_t;

typedef struct _sid_
{
   int      nUserIndex;
   uint32_t dIPAddr;
   //uint32_t dLastAccessTimeSec;
   uint32_t dLastAccessHTTPTimeSec;
   uint32_t dLastAccessCGITimeSec;
   int      nAccessGranted;
   uint32_t dPermission;
   char      StrSID[SID_LEN+1];           /* Add 1 for zero termination */    
   char      StrNonce[SID_NONCE_LEN+1];   /* Add 1 for zero termination */
} sid_t;

typedef struct _sid_data_
{
   uint32_t dIPAddr;
   uint32_t dTimeSec;
   uint32_t dTimeTick;
} sid_data_t;

/*=======================================================================*/
/*  Definition of all global Data                                        */
/*=======================================================================*/

/*=======================================================================*/
/*  Definition of all local Data                                         */
/*=======================================================================*/

static user_t UserList[USER_LIST_CNT];
static sid_t  SIDList[SID_LIST_CNT];

static int      nLoginErrorCnt         = 0;
static int      nLoginBlocked          = 0;
static uint32_t dLoginBlockedStartTime = 0;

/*=======================================================================*/
/*  Definition of all local Procedures                                   */
/*=======================================================================*/

/*************************************************************************/
/*  CreateSalt                                                           */
/*                                                                       */
/*  In    : pSalt                                                        */
/*  Out   : pSalt                                                        */
/*  Return: none                                                         */
/*************************************************************************/
static void CreateSalt (uint8_t *pSalt)
{
   tal_CPURngHardwarePoll(pSalt, SALT_SIZE);

} /* CreateSalt */

/*************************************************************************/
/*  CreateHashBySalt                                                     */
/*                                                                       */
/*  In    : pSalt, pHash, pStr, bStrLen                                  */
/*  Out   : pHash                                                        */
/*  Return: none                                                         */
/*************************************************************************/
static void CreateHashBySalt (uint8_t *pSalt, uint8_t *pHash, char *pStr, size_t StrLen)
{
   static sha2_context HASHctx;

   sha2_start(&HASHctx);
   sha2_update(&HASHctx, (uint8_t*)pSalt, SALT_SIZE);
   sha2_update(&HASHctx, (uint8_t*)pStr, StrLen);
   sha2_finish(&HASHctx, pHash);

} /* CreateHashBySalt */

/*************************************************************************/
/*  FindSIDEntry                                                         */
/*                                                                       */
/*  Find the session id for the given address, or a free entry.          */
/*                                                                       */
/*  In    : dIPAddr                                                      */
/*  Out   : none                                                         */
/*  Return: pSIDEntry                                                    */
/*************************************************************************/
static sid_t *FindSIDEntry (uint32_t dIPAddr, int *pIndex)
{
   sid_t *pSIDEntry = NULL;
   int    nIndex;
   
   /* Check if the ip address is used */
   for(nIndex = 0; nIndex < SID_LIST_CNT; nIndex++)
   {
      if (dIPAddr == SIDList[nIndex].dIPAddr)
      {
         *pIndex   = nIndex;
         pSIDEntry = &SIDList[nIndex];
         break;
      }
   }
   
   /* Check for a free entry search */
   if (NULL == pSIDEntry)
   {
      for(nIndex = 0; nIndex < SID_LIST_CNT; nIndex++)
      {
         if (0 == SIDList[nIndex].dIPAddr)
         {
            *pIndex   = nIndex;
            pSIDEntry = &SIDList[nIndex];
            break;
         }
      }
   }

   return(pSIDEntry);
} /* FindSIDEntry */

/*************************************************************************/
/*  CheckUserPassword                                                    */
/*                                                                       */
/*  Check if User/Password is valid.                                     */
/*                                                                       */
/*  In    : pSIDEntry, pUser, pPassword, pPermission                     */
/*  Out   : pPermission                                                  */
/*  Return: -1 == invalid / all other = valid                            */
/*************************************************************************/
static int CheckUserPassword (sid_t *pSIDEntry, char *pUser, char *pPassword, uint32_t *pPermission)
{
   int           nValid = -1;
   int           nIndex;
   static uint8_t Hash[SHA2_HASH_SIZE];  /* Use static here, because stack should not be used */
   
   (void)pSIDEntry;
   
   *pPermission = 0;
   
   /* Check for valid user/password combination */
   for(nIndex = 0; nIndex < USER_LIST_CNT; nIndex++)
   {
      /* Check if user match with our "database" */
      if (0 == strcmp(UserList[nIndex].User, pUser))
      {
         /* Create hash from password */
         CreateHashBySalt(UserList[nIndex].Salt, Hash, pPassword, strlen(pPassword));
      
         if (0 == memcmp(UserList[nIndex].Hash, Hash, SHA2_HASH_SIZE))
         {
            nValid       = nIndex;
            *pPermission = UserList[nIndex].dPermission;
            break;
         }
         else
         {
            /* Wrong password */
            break;
         }
      }
   }

   
   /* In case of an error increase the error counter */   
   if ((-1 == nValid) && (0 == nLoginBlocked) && (nLoginErrorCnt < LOGIN_ERROR_CNT_MAX))
   {
      nLoginErrorCnt++;
      if (LOGIN_ERROR_CNT_MAX == nLoginErrorCnt)
      {
         /* To many login error, block login for the next LOGIN_TIMEOUT_SEC */
         nLoginBlocked = 1;
         dLoginBlockedStartTime = OS_TimeGetSeconds();
      }
   }
   
   /* For security, if login is blocked, invalidate the result again */
   if (1 == nLoginBlocked)
   {
      nValid = -1;
   }
   
   /* If no login error, clear error count */
   if (nValid != -1)
   {
      nLoginErrorCnt = 0;
   }
   
   return(nValid);
} /* CheckUserPassword */

/*************************************************************************/
/*  CreateSID                                                            */
/*                                                                       */
/*  Create a SID for he given IP address.                                */
/*                                                                       */
/*  In    : dIPAddr                                                      */
/*  Out   : none                                                         */
/*  Return: pSessionID                                                   */
/*************************************************************************/
static char *CreateSID (uint32_t dIPAddr)
{
   char      *pSessionID = NULL;
   sid_t     *pSIDEntry;
   sid_data_t  Data;
   int        nIndex;
   char        Hex[3];
   
   static sha2_context HASHctx;
   static uint8_t      HashValue[SHA2_HASH_SIZE];
   
   pSIDEntry = FindSIDEntry(dIPAddr, &nIndex);
   if (pSIDEntry != NULL)
   {   
      /*
       * Create SID
       */
      Data.dIPAddr   = dIPAddr;
      Data.dTimeSec  = OS_TimeGetSeconds();
      Data.dTimeTick = OS_TimeGet();

      sha2_start(&HASHctx);
      sha2_update(&HASHctx, (uint8_t*)&Data, sizeof(Data));
      sha2_finish(&HASHctx, HashValue);
      
      pSIDEntry->dIPAddr            = dIPAddr;
      //pSIDEntry->dLastAccessTimeSec = OS_TimeGetSeconds();
      pSIDEntry->dLastAccessHTTPTimeSec = pSIDEntry->dLastAccessCGITimeSec = OS_TimeGetSeconds();
      pSIDEntry->nAccessGranted     = 0;

      /* Save index in the first 2 digits */
      sprintf(pSIDEntry->StrSID, "%02X", nIndex);
      
      /* Save the rest of the hash result */
      for(nIndex = 0; nIndex < 15; nIndex++)
      {
         sprintf(Hex, "%02X", HashValue[nIndex]);
         strcat(pSIDEntry->StrSID, Hex);
      }
      
      pSessionID = pSIDEntry->StrSID;
   }
      
   return(pSessionID);
} /* CreateSID */

/*=======================================================================*/
/*  All code exported                                                    */
/*=======================================================================*/

/*************************************************************************/
/*  WebSidInit                                                           */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void WebSidInit (void)
{
   memset(UserList, 0x00, sizeof(UserList));
   memset(SIDList,  0x00, sizeof(SIDList));

   /*
    * Setup admin
    */
   sprintf(UserList[0].User, "admin");
   CreateSalt(UserList[0].Salt);
   CreateHashBySalt(UserList[0].Salt, UserList[0].Hash, "admin", 5);
   UserList[0].dPermission = 0xFFFFFFFF;

   /*
    * Setup guest
    */
   sprintf(UserList[1].User, "guest");
   CreateSalt(UserList[1].Salt);
   CreateHashBySalt(UserList[1].Salt, UserList[1].Hash, "guest", 5);
   UserList[1].dPermission = 0;

   /*
    * Setup user1
    */
   sprintf(UserList[2].User, "user1");
   CreateSalt(UserList[2].Salt);
   CreateHashBySalt(UserList[2].Salt, UserList[2].Hash, "user1", 5);
   UserList[2].dPermission = 0x00000001;

   /*
    * Setup user2
    */
   sprintf(UserList[3].User, "user2");
   CreateSalt(UserList[3].Salt);
   CreateHashBySalt(UserList[3].Salt, UserList[3].Hash, "user2", 5);
   UserList[3].dPermission = 0x00000002;
   
} /* WebSidInit */

/*************************************************************************/
/*  WebSidParseCookie                                                    */
/*                                                                       */
/*  Parse the cookie to retrieve the SID.                                */
/*                                                                       */
/*  In    : pCookie                                                      */
/*  Out   : none                                                         */
/*  Return: pSessionID                                                   */
/*************************************************************************/
char *WebSidParseCookie (char *pCookie)
{
   char *pStart;
   char *pSessionID = NULL;
   
   if (pCookie != NULL)
   {
      pStart = strstr(pCookie, SID_START);
      if (pStart != NULL)
      {
         pStart += strlen(SID_START);
      
         pSessionID = xcalloc(XM_ID_WEB, 1, SID_LEN+1);         
         if (pSessionID != NULL)
         {
            memcpy(pSessionID, pStart, SID_LEN);
         }   
      }
   }      
   
   return(pSessionID);
} /* WebSidParseCookie */

/*************************************************************************/
/*  WebSidCreateCookie                                                   */
/*                                                                       */
/*  Create a cookie for the given session.                               */
/*                                                                       */
/*  In    : hs                                                           */
/*  Out   : none                                                         */
/*  Return: pCookie                                                      */
/*************************************************************************/
char *WebSidCreateCookie (HTTPD_SESSION *hs)
{
   char    *pCookie;
   char    *pSessionID;
   uint32_t dSrcIP;
   
   /* Get source IP address */   
   dSrcIP = hs->s_stream->strm_caddr.sin_addr.s_addr;
   
   pCookie = xmalloc(XM_ID_WEB, SID_COOKIE_SIZE);
   if (pCookie != NULL)
   {
      pSessionID = CreateSID(dSrcIP);
      if (pSessionID != NULL)
      {
         snprintf(pCookie, SID_COOKIE_SIZE, "%s%s; HttpOnly; Path=/",
                  SID_START, pSessionID);
      }
      else
      {
         xfree(pCookie);
         pCookie = NULL;
      }                      
   }   

   return(pCookie);
} /* WebSidCreateCookie */

/*************************************************************************/
/*  WebSidCreateNonce                                                    */
/*                                                                       */
/*  Create a NONCE for the given session.                                */
/*                                                                       */
/*  In    : hs                                                           */
/*  Out   : none                                                         */
/*  Return: pNonce                                                       */
/*************************************************************************/
char *WebSidCreateNonce (HTTPD_SESSION *hs)
{
   char        *pNonce = NULL;
   int          nIndex;
   int          nSIDEntry;
   sid_t       *pSIDEntry;
   uint8_t       Nonce[(SID_NONCE_LEN/2)];   
   char          Hex[3];
   HTTP_REQUEST *req = &hs->s_req;

   /* Check first if the SID is valid */
   nSIDEntry = WebSidCheck(hs, req->req_sid, WEB_SID_HTTP); 
   if (nSIDEntry != -1)   
   {
      pSIDEntry = &SIDList[nSIDEntry];

      tal_CPURngHardwarePoll(Nonce, sizeof(Nonce));

      pSIDEntry->StrNonce[0] = 0;
      for(nIndex = 0; nIndex < (SID_NONCE_LEN/2); nIndex++)
      {
         sprintf(Hex, "%02X", Nonce[nIndex]);
         strcat(pSIDEntry->StrNonce, Hex);
      }
      pNonce = pSIDEntry->StrNonce;
   }

   return(pNonce);
} /* WebSidCreateNonce */

/*************************************************************************/
/*  WebSidCheck                                                          */
/*                                                                       */
/*  Check if the SessionID is valid.                                     */
/*                                                                       */
/*  Return -1 if the SID is invalid, otherwise return the position       */ 
/*  in the SIDList.                                                      */
/*                                                                       */
/*  In    : hs, pSessionID, nIsHttp                                      */
/*  Out   : none                                                         */
/*  Return: -1 == invalid / nSIDEntry                                    */
/*************************************************************************/
int WebSidCheck (HTTPD_SESSION *hs, char *pSessionID, int nIsHttp)
{
   int      nSIDEntry = -1;
   int      nIndex;
   sid_t   *pSIDEntry = NULL;
   uint32_t dIPAddr;
   char      Hex[3];
   
   /* Get source IP address */   
   dIPAddr = hs->s_stream->strm_caddr.sin_addr.s_addr;
   
   if (pSessionID != NULL)
   {
      Hex[0] = pSessionID[0];
      Hex[1] = pSessionID[1];
      Hex[2] = 0;
      nIndex = atoi(Hex);
      
      if (nIndex < SID_LIST_CNT)
      {
         pSIDEntry = &SIDList[nIndex];
         
         if( (dIPAddr == pSIDEntry->dIPAddr)                    &&
             (0 == memcmp(pSIDEntry->StrSID, pSessionID, SID_LEN)) )
         {      
            if (WEB_SID_HTTP == nIsHttp)
            {
               /* Check HTTP timeout */       
               if (OS_TEST_TIMEOUT(OS_TimeGetSeconds(), pSIDEntry->dLastAccessHTTPTimeSec, SID_TIMEOUT_SEC))
               {
                  /* Timeout => not valid anymore */
                  nSIDEntry = -1;
                  pSIDEntry->nAccessGranted = 0;
               }
               else
               {
                  /* Retrigger => valid */
                  pSIDEntry->dLastAccessHTTPTimeSec = OS_TimeGetSeconds();
               
                  nSIDEntry = nIndex;
               }
            }
            else
            {
               /* Check CGI timeout */       
               if (OS_TEST_TIMEOUT(OS_TimeGetSeconds(), pSIDEntry->dLastAccessCGITimeSec, SID_TIMEOUT_SEC))
               {
                  /* Timeout => not valid anymore */
                  nSIDEntry = -1;
                  pSIDEntry->nAccessGranted = 0;
               }
               else
               {
                  /* Retrigger => valid */
                  pSIDEntry->dLastAccessCGITimeSec = OS_TimeGetSeconds();
               
                  nSIDEntry = nIndex;
               }
            }               
         }             
      }
   }
      
   return(nSIDEntry);
} /* WebSidCheck */

/*************************************************************************/
/*  WebSidCheckAccessGranted                                             */
/*                                                                       */
/*  Check if the access is granted.                                      */
/*  Return -1 if the SID is invalid, otherwise return the position       */ 
/*  in the SIDList.                                                      */
/*                                                                       */
/*  In    : hs, pSessionID, nIsHttp                                      */
/*  Out   : none                                                         */
/*  Return: 1 == granted / otherwise not granted                         */
/*************************************************************************/
int WebSidCheckAccessGranted (HTTPD_SESSION *hs, char *pSessionID, int nIsHttp)
{
   int      nGranted = 0;
   int      nIndex;
   sid_t   *pSIDEntry = NULL;
   uint32_t dIPAddr;
   char      Hex[3];
   
   /* Get source IP address */   
   dIPAddr = hs->s_stream->strm_caddr.sin_addr.s_addr;
   
   if (pSessionID != NULL)
   {
      Hex[0] = pSessionID[0];
      Hex[1] = pSessionID[1];
      Hex[2] = 0;
      nIndex = atoi(Hex);
      
      if (nIndex < SID_LIST_CNT)
      {
         pSIDEntry = &SIDList[nIndex];
         
         if( (dIPAddr == pSIDEntry->dIPAddr)                       &&
             (0 == memcmp(pSIDEntry->StrSID, pSessionID, SID_LEN)) &&
             (1 == pSIDEntry->nAccessGranted)                      )
         {      
            if (WEB_SID_HTTP == nIsHttp)
            {
               /* Check HTTP timeout */       
               if (OS_TEST_TIMEOUT(OS_TimeGetSeconds(), pSIDEntry->dLastAccessHTTPTimeSec, SID_TIMEOUT_SEC))
               {
                  /* Timeout => not valid anymore */
                  pSIDEntry->nAccessGranted = 0;
               }
               else
               {
                  nGranted = 1;
               }
            }
            else
            {
               /* Check CGI timeout */       
               if (OS_TEST_TIMEOUT(OS_TimeGetSeconds(), pSIDEntry->dLastAccessCGITimeSec, SID_TIMEOUT_SEC))
               {
                  /* Timeout => not valid anymore */
                  pSIDEntry->nAccessGranted = 0;
               }
               else
               {
                  nGranted = 1;
               }
            }               
         }             
      }
   }
      
   return(nGranted);
} /* WebSidCheckAccessGranted */

/*************************************************************************/
/*  WebSidCheckUserPass                                                  */
/*                                                                       */
/*  Check if User/Password combination is valid.                         */
/*                                                                       */
/*  In    : hs, pUser, pPass                                             */
/*  Out   : none                                                         */
/*  Return: 0 = blocked / 1 = not blocked                                */
/*************************************************************************/
int WebSidCheckUserPass (HTTPD_SESSION *hs, char *pUser, char *pPass)
{
   int          nValid = 0;
   int          nUserPassValid;
   uint32_t     dPermission;
   int          nSIDEntry;
   sid_t       *pSIDEntry;   
   HTTP_REQUEST *req = &hs->s_req;
   
   /* Check first if the SID is valid */
   nSIDEntry = WebSidCheck(hs, req->req_sid, FALSE); 
   if (nSIDEntry != -1)
   {
      pSIDEntry = &SIDList[nSIDEntry];
   
      /* Check if User and Password are valid */
      nUserPassValid = CheckUserPassword(pSIDEntry, pUser, pPass, &dPermission);
      if (nUserPassValid != -1)
      {
         nValid = 1;
         pSIDEntry->nUserIndex     = nUserPassValid;
         pSIDEntry->nAccessGranted = 1;
         pSIDEntry->dPermission    = dPermission;
      }
      else
      {
         /* Not valid */
         pSIDEntry->nUserIndex     = 0;
         pSIDEntry->nAccessGranted = 0;
         pSIDEntry->dPermission    = 0;
      }
   }
   
   return(nValid);
} /* WebSidCheckUserPass */

/*************************************************************************/
/*  WebSidInvalidate                                                     */
/*                                                                       */
/*  Invalidate session id if available.                                  */
/*                                                                       */
/*  In    : hs                                                           */
/*  Out   : none                                                         */
/*  Return: none                                                         */
/*************************************************************************/
void WebSidInvalidate (HTTPD_SESSION *hs)
{
   char    *pSessionID = hs->s_req.req_sid;
   sid_t   *pSIDEntry;
   uint32_t dIPAddr;
   int      nIndex;
   char      Hex[3];

   /* Get source IP address */   
   dIPAddr = hs->s_stream->strm_caddr.sin_addr.s_addr;
   
   if (pSessionID != NULL)
   {
      Hex[0] = pSessionID[0];
      Hex[1] = pSessionID[1];
      Hex[2] = 0;
      nIndex = atoi(Hex);
      
      if (nIndex < SID_LIST_CNT)
      {
         pSIDEntry = &SIDList[nIndex];
         
         if( (dIPAddr == pSIDEntry->dIPAddr)                       &&
             (0 == memcmp(pSIDEntry->StrSID, pSessionID, SID_LEN)) )
         {
            pSIDEntry->dIPAddr        = 0;
            pSIDEntry->nAccessGranted = 0;
         }             
      }
   }
   
} /* WebSidInvalidate */

/*************************************************************************/
/*  WebSidGetPermission                                                  */
/*                                                                       */
/*  Return the permission for session.                                   */
/*                                                                       */
/*  In    : nSIDEntry                                                    */
/*  Out   : none                                                         */
/*  Return: dPermission                                                  */
/*************************************************************************/
uint32_t WebSidGetPermission (int nSIDEntry)
{
   uint32_t dPermission = 0;

   if (nSIDEntry < SID_LIST_CNT)
   {
      dPermission = SIDList[nSIDEntry].dPermission;
   }
   
   return(dPermission);
} /* WebSidGetPermission */

/*************************************************************************/
/*  WebSidGetUser                                                        */
/*                                                                       */
/*  Return the user for the session.                                     */
/*                                                                       */
/*  In    : nSIDEntry                                                    */
/*  Out   : none                                                         */
/*  Return: NULL = invalid / otherwise user pointer                      */
/*************************************************************************/
char *WebSidGetUser (int nSIDEntry)
{
   char *pUser = NULL;

   if (nSIDEntry < SID_LIST_CNT)
   {
      if (SIDList[nSIDEntry].nUserIndex != -1)
      {
         pUser = xstrdup(XM_ID_WEB, UserList[SIDList[nSIDEntry].nUserIndex].User);
      }
   }
   
   return(pUser);
} /* WebSidGetUser */

/*************************************************************************/
/*  WebSidSetNewPass                                                     */
/*                                                                       */
/*  Set the new password if the old one match the session.               */
/*                                                                       */
/*  Note: pPassUser is hashed with a NONCE, and pPassNew is blowfish     */
/*        encoded with the pPassUser without the NONCE.                  */
/*                                                                       */
/*  In    : hs, pPassUser, pPassNewEncoded                               */
/*  Out   : none                                                         */
/*  Return: -1 = invalid / otherwise valid                               */
/*************************************************************************/
int WebSidSetNewPass (HTTPD_SESSION *hs, char *pPassUser, char *pPassNewEncoded)
{
   int           nValid = -1;
   int           nSIDEntry;
   int           nIndex;
   sid_t        *pSIDEntry;
   static uint8_t Hash[SHA2_HASH_SIZE];  /* Use static here, because stack should not be used */
         
   /* Check first if the SID is valid */
   nSIDEntry = WebSidCheck(hs, hs->s_req.req_sid, FALSE);
   if (nSIDEntry != -1)
   {
      pSIDEntry = &SIDList[nSIDEntry];
      
      (void)pSIDEntry;
       
      /* Check for valid user/password combination */
      for(nIndex = 0; nIndex < USER_LIST_CNT; nIndex++)
      {
         /* Check if user match with our "database" */
         if (0 == strcmp(UserList[nIndex].User, hs->s_req.req_sid_user))
         {
            /* 
             * Check if the PassUser match with the database password.
             */
            CreateHashBySalt(UserList[nIndex].Salt, Hash, pPassUser, strlen(pPassUser));
             
            /* Check existing passwords now */         
            if (0 == memcmp(UserList[nIndex].Hash, Hash, SHA2_HASH_SIZE))
            {
               /* Password is valid, create the new one */
               CreateSalt(UserList[nIndex].Salt);
               CreateHashBySalt(UserList[nIndex].Salt, UserList[nIndex].Hash, pPassNewEncoded, strlen(pPassNewEncoded));
            
               nValid = nIndex;
            }
         }
         break;      
      }   
   }   
   
   return(nValid);
} /* WebSidSetNewPass */

/*************************************************************************/
/*  WebSidLoginBlocked                                                   */
/*                                                                       */
/*  Return the info if the login is blocked.                             */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: 1 = blocked / 0 = not blocked                                */
/*************************************************************************/
int WebSidLoginBlocked (void)
{
   return(nLoginBlocked);
} /* WebSidLoginBlocked */

/*************************************************************************/
/*  WebSidLoginBlockedTime                                               */
/*                                                                       */
/*  Return the bloked timeout in seconds.                                */
/*                                                                       */
/*  In    : none                                                         */
/*  Out   : none                                                         */
/*  Return: nBlockedTime                                                 */
/*************************************************************************/
uint32_t WebSidLoginBlockedTime (void)
{
   uint32_t dBlockedTime = 0;

   if (1 == nLoginBlocked)
   {
      if (OS_TEST_TIMEOUT(OS_TimeGetSeconds(), dLoginBlockedStartTime, LOGIN_TIMEOUT_SEC))
      {
         nLoginBlocked  = 0;
         nLoginErrorCnt = 0;
      }
      else
      {
         dBlockedTime = LOGIN_TIMEOUT_SEC - (OS_TimeGetSeconds() - dLoginBlockedStartTime);
      }
   }
   
   return(dBlockedTime);
} /* WebSidLoginBlockedTime */
#else

void WebSidInit (void)
{
}
char *WebSidParseCookie (char *pCookie)
{
   (void)pCookie;
   
   return(NULL);
}
char *WebSidCreateNonce (HTTPD_SESSION *hs)
{
   (void)hs;
   
   return(NULL);
}
int  WebSidCheckUserPass (HTTPD_SESSION *hs, char *pUser, char *pPass)
{
   (void)hs;
   (void)pUser;
   (void)pPass;
   
   return(1);   
}
void WebSidInvalidate (HTTPD_SESSION *hs)
{
   (void)hs;
}
int WebSidSetNewPass (HTTPD_SESSION *hs, char *pPassUser, char *pPassNewEncoded)
{
   (void)hs;
   (void)pPassUser;
   (void)pPassNewEncoded;
   
   return(0);
} 
int WebSidLoginBlocked (void)
{
   return(0);
}
uint32_t WebSidLoginBlockedTime (void)
{
   return(0);
}

#endif /* (_IP_WEB_SID_SUPPORT >= 1) */

/*** EOF ***/
