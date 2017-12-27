//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2017 Fsys Software and Jan van Wijk
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
// Timer SET and EXPIRATION-test functions that works on ELAPSED time,
// not on spent-CPU time like clock() does on some platforms, causing
// time measurement to fail (time-stand-still when thread blocked)
// This caused problem with mouse-DBLCLK detect, and progress status updating
//
// Interface will use 'nanoseconds-since-programstart' as its base unit,
// and implements that with a resolution as high as supported by the OS
//
//
// Author: J. van Wijk
//
// JvW  03-06-2017 Initial version, 'best' resolution ELAPSED time functions

#include <txlib.h>
#include <txtpriv.h>                            // for the TxTrTstamp flag


#if   defined (UNIX)                            // gettimeofday is used
   #include <sys/time.h>
#endif

#define TXTMR_NS_PER_SEC  ((TXTIMER) 1000000000)
#define TXTMR_US_PER_SEC  ((TXTIMER) 1000000)

static TXTIMER         txtmr_base;


// Get nanoseconds (elapsed) time value from OS, with best resolution possible
static TXTIMER TxTmrGetNanoSecStamp             // RET   nanosecond value
(
   void
);


//- Note: can NOT use any trace, since it is used for trace-timestamping too!


/*****************************************************************************/
// Determine and set the BASE timer-value, resulting in 0 nsec since startup
/*****************************************************************************/
void TxTmrInitializeBase                        // call once, at program start
(
   void
)
{
   #if   defined (UNIX)                        // gettimeofday is used
      TRACES(("Resolution nsec: unknown, minimum: 1000\n"));
   #elif defined (WIN32)
      TRACES(("Resolution nsec: unknown, minimum: 100\n"));
   #else                                        // probably DOS or OS/2, uses clock()
      TRACES(("Resolution nsec: %llu\n", TXTMR_NS_PER_SEC / CLOCKS_PER_SEC));
   #endif

   txtmr_base = TxTmrGetNanoSecStamp();
}                                               // end 'TxTmrInitializeBase'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set a timer variable to expire after the given duration in nanoseconds
/*****************************************************************************/
TXTIMER TxTmrSetTimer                           // RET   timer expiration value
(
   TXTIMER             duration                 // IN    Duration in nanoseconds
)
{
   TXTIMER             rc = 0;                  // function return
   #if defined (DUMP)
      BOOL             ts = TxTrTstamp;         // save the flag
      TxTrTstamp = FALSE;                       // avoid recursion (TraceLeader)
   #endif

   rc = TxTmrGetNanoSecStamp() - txtmr_base + duration;

   #if defined (DUMP)
      TxTrTstamp = ts;                          // restore flag
   #endif
   return (rc);
}                                               // end 'TxTmrSetTimer'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set a timer variable to expire after the given duration in nanoseconds
/*****************************************************************************/
BOOL TxTmrTimerExpired                          // RET   timer has expired
(
   TXTIMER             timer                    // IN    timer expiration value
)
{
   BOOL                rc;

   rc = ((TxTmrGetNanoSecStamp() - txtmr_base) > timer);

   return (rc);
}                                               // end 'TxTmrInitializeBase'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get nanoseconds (elapsed) time value from OS, with best resolution possible
/*****************************************************************************/
static TXTIMER TxTmrGetNanoSecStamp             // RET   nanosecond value
(
   void
)
{
   TXTIMER             rc = 0;                  // function return

   #if   defined (UNIX)                         // gettimeofday is used
      struct timeval   tv;
   #elif defined (WIN32)
      FILETIME         ft;
   #else                                        // probably DOS or OS/2, uses clock()
   #endif

   #if   defined (UNIX)                         // gettimeofday is used, 1 uSec resolution
      if (gettimeofday( &tv, NULL) == 0)        // hi part is seconds, low is uSec
      {
         rc = ((TXTIMER) tv.tv_sec * TXTMR_NS_PER_SEC) + tv.tv_usec * 1000;
         TRLEVX(456,("             TxTmr RAW sec:%8.8lx usec:%8.8lx  rc:%llx\n", tv.tv_sec, tv.tv_usec, rc));
      }
      else
      {
         TRLEVX(456,( "gettimeofday error: %s\n", strerror(errno)));
      }
   #elif defined (WIN32)
      GetSystemTimeAsFileTime( &ft);            // uses 100 nsec time units in 64-bits (hi+lo)
      rc = (((TXTIMER) ft.dwHighDateTime << 32) + ft.dwLowDateTime) * 100;
      TRLEVX(456,("             TxTmr RAW sec:%8.8lx 100n:%8.8lx  rc:%llx\n", ft.dwHighDateTime, ft.dwLowDateTime, rc));

   #else                                        // probably DOS or OS/2, uses clock()
      rc = ((TXTIMER) clock() * TXTMR_NS_PER_SEC) / CLOCKS_PER_SEC;
      TRLEVX(456,("             TxTmr RAW clock:%8.8lx  rc:%llx\n", clock(), rc));
   #endif

   return (rc);
}                                               // end 'TxTmrGetNanoSecStamp'
/*---------------------------------------------------------------------------*/

