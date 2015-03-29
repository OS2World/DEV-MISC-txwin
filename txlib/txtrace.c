//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2005 Fsys Software and Jan van Wijk
//
// ==========================================================================
//
// This file contains Original Code and/or Modifications of Original Code as
// defined in and that are subject to the GNU Lesser General Public License.
// You may not use this file except in compliance with the License.
// BY USING THIS FILE YOU AGREE TO ALL TERMS AND CONDITIONS OF THE LICENSE.
// A copy of the License is provided with the Original Code and Modifications,
// and is also available at http://www.dfsee.com/txwin/lgpl.htm
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License,
// or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; (lgpl.htm) if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// Questions on TxWin licensing can be directed to: txwin@fsys.nl
//
// ==========================================================================
//
// Trace functions for module entry/exit
//
// Author: J. van Wijk
//
// JvW  26-12-1996   Split-off from ansilog
// JvW  11-10-1999   No timestamp when tracelevel >= 900

#define DUMP 1                                  // tracecode always available

#include <txlib.h>
#include <txtpriv.h>

        ULONG     TxTrLevel      = 0;           // trace level
        ULONG     TxTrSlowDown   = 0;           // xx ms pause per traceline
        BOOL      TxTrLogOnly    = TRUE;        // default to log only
        BOOL      TxTrTstamp     = FALSE;       // default no timestamp

        ULONG     TxTrIndent[TXTHREADS] = {0};  // indent per thread

static  ULONG     TxTrMainThread = 1;           // TID of main() thread


/*****************************************************************************/
// Init trace level and destinations at startup, to be called from main()
/*****************************************************************************/
void TxTraceInitMain
(
   int                *pargc,                   // INOUT argument count
   char              **pargv[],                 // INOUT array of arg values
   char               *envname,                 // IN    trace env-var name
   char               *prefix                   // IN    tracefile prefix
)
{
   char               *tr;                      // trace specification
   int                 argc = *pargc;
   char              **argv = *pargv;
   char               *s;

   TxTrMainThread = TXTHREADID;                 // remember TID of main()

   if ((          argc     >  1 ) &&            // there is a parameter
       (strlen(   argv[1]) >  2 ) &&            // 3 or more characters
       (         *argv[1] == '-') &&            // and it is a switch
       (isdigit(*(argv[1] +1) ) ) &&            // and starts numerical
       (isdigit(*(argv[1] +2) ) )  )            // with at least two digits
   {
      tr = argv[1] +1;                          // start trace specification

      *pargc = argc -1;                         // shift rest of arguments
      *pargv = argv +1;
   }
   else
   {
      if ((tr = getenv( envname)) != NULL)      // trace in environment ?
      {
         int           i;

         for (i = 1; i < argc; i++)
         {
            if (strncmp(argv[i], "-l", 2) == 0) // there is a -l switch!
            {
               tr = NULL;                       // NO trace now, to avoid
               break;                           // clobbering real logfile
            }
         }
      }
   }
   if (tr != NULL)
   {
      TxTrLevel = atol( tr);                    // numeric trace level
      if (strchr( tr, 's') != NULL)             // screen output too
      {
         TxTrLogOnly  = FALSE;
      }
      if (strchr( tr, 'r') != NULL)             // close/reopen logfile
      {
         TxSetLogReOpen( TRUE);
      }
      if ((s = strchr( tr, 'd')) != NULL)       // 100ms pause per traceline
      {
         if ((TxTrSlowDown = atol( s+1)) == 0)  // or specific value
         {
            TxTrSlowDown = 100;
         }
      }
      if (strchr( tr, 't') != NULL)             // timestamps included
      {
         TxTrTstamp   = TRUE;
      }
      if (strchr( tr, 'l') == NULL)             // auto-log allowed
      {
         TXTM           trcfile;

         sprintf( trcfile, "%s-%lu", prefix, TxTrLevel);
         TxAppendToLogFile( trcfile, TRUE);
      }
   }
}                                               // end 'TxTraceInitMain'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle entry for a routine
/*****************************************************************************/
void TxTraceEnter
(
   char              *mod                       // IN    module/function name
)
{
   TxTraceLeader();
   TxPrint( "%sEnter func%s : %s%s%s\n", CNM, CNN, CNC, mod, CNN);
   TxTrIndent[ TXTHREADID]++;
}                                               // end 'TxTraceEnter'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle return of a routine
/*****************************************************************************/
void TxTraceReturn
(
   char              *mod,                      // IN    module/function name
   ULONG              rc                        // IN    return value
)
{
   if (TxTrIndent[TXTHREADID])
   {
      TxTrIndent[TXTHREADID]--;
   }
   TxTraceLeader();
   TxPrint( "%sRet%s:%s% 8.8lX %s%s%s\n",
            CNM, CNN, ((rc) ? CBR : CBG), rc, CNC, mod, CNN);
}                                               // end 'TxTraceReturn'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle boolean return of a routine
/*****************************************************************************/
void TxTraceRetBool
(
   char              *mod,                      // IN    module/function name
   BOOL               rc                        // IN    return value
)
{
   if (TxTrIndent[TXTHREADID])
   {
      TxTrIndent[TXTHREADID]--;
   }
   TxTraceLeader();
   TxPrint( "%sRet%s:%s%s %s%s%s\n", CNM, CNN,
            ((rc) ?  CBG       :  CBR),
            ((rc) ? "True    " : "False   "),
              CNC,   mod,         CNN);
}                                               // end 'TxTraceRetBool'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle return of a void routine
/*****************************************************************************/
void TxTraceRetVoid
(
   char              *mod                       // IN    module/function name
)
{
   if (TxTrIndent[TXTHREADID])
   {
      TxTrIndent[ TXTHREADID]--;
   }
   TxTraceLeader();
   TxPrint( "%sReturn void%s: %s%s%s\n", CNM, CNN, CNC, mod, CNN);
}                                               // end 'TxTraceRetVoid'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Timestamp, thread-id and indent for trace-line
/*****************************************************************************/
void TxTraceLeader
(
   void
)
{
   TXTS                lti = {0};
   TXTT                ltm = {0};
   ULONG               tid = TXTHREADID;

   if (TxTrSlowDown)
   {
      TxSleep( TxTrSlowDown);
   }
   if (TxTrTstamp)
   {
      sprintf( ltm, "%s%7.7lu ", CBZ, clock());
   }
   if ((tid != TxTrMainThread) || (TxTrTstamp))
   {
      sprintf( lti, "%2.2lu ", tid);
   }
   TxPrint( "%s%s%s%*.*s",  ltm, lti, CNN, (int) TxTrIndent[tid],
                                            (int) TxTrIndent[tid], "");
}                                               // end 'TxTraceLeader'
/*---------------------------------------------------------------------------*/

