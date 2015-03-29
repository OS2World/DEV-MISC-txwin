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
// Filesystem attribute handling functions
//
// Author: J. van Wijk
//
// JvW  13-06-2002 Initial version, split off from TXCON

#include <txlib.h>


// REXX value = d2x(((369*365+89)*86400)*10000000)
#define  TXNTM_LO    ((ULONG) 0xd53e8000)       // delta between 1-1-1601 base
#define  TXNTM_HI    ((ULONG) 0x019db1de)       // and ANSI 1-1-1970 (100 ns)

#define  TXNTM_HI_M  ((double) 65536 * (double) (65536))
#define  TXNTM_NS_D  ((double) 10000000)        // 100ns slots per second


/*****************************************************************************/
// Translate attributes (FAT) to 6-position string format
/*****************************************************************************/
void txFileAttr2String
(
   ULONG               data,                    // IN    data
   char               *attrib                   // OUT   attribute string[6]
)
{
   strcpy( attrib, "-----");
   if (data & FATTR_ARCHIVED)  attrib[0] = 'A';
   if (data & FATTR_DIRECTORY) attrib[1] = 'D';
   if (data & FATTR_HIDDEN)    attrib[2] = 'H';
   if (data & FATTR_READONLY)  attrib[3] = 'R';
   if (data & FATTR_SYSTEM)    attrib[4] = 'S';
}                                               // end 'txFileAttr2String'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert Windows-NT/W2K/XP filetime (64 bit) to compiler time_t value
/*****************************************************************************/
time_t txWinFileTime2t                          // RET   time_t value
(
   NTIME              *nt,                      // IN    ptr NT time value
   LONG                TimeZone                 // IN    TZ offset to GMT (min)
)
{
   ULONG               lo;
   ULONG               hi;
   double              tstamp;

   //- ENTER();
   //- TRACES(("NT time value 64bit: %8.8lx %8.8lx\n", nt->hi, nt->lo));

   lo = nt->lo - TXNTM_LO;
   hi = nt->hi - TXNTM_HI;                      // subtract fixed 1-1-1970
   if ( nt->lo < TXNTM_LO)
   {
      hi--;                                     // adjust for borrow-1
   }
   tstamp  = ((double) lo + ((double) hi * TXNTM_HI_M)) / TXNTM_NS_D;
   tstamp += ((double) TimeZone * 60);          // correct for timezone

   //- TRACES(("Seconds since 1970 : %lf\n", tstamp));
   //- RETURN( (time_t) tstamp);
   return( (time_t) tstamp);
}                                               // end 'txWinFileTime2t'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert compiler time_t to Windows-NT/W2K/XP filetime (64 bit) value
/*****************************************************************************/
void txCt2WinFileTime
(
   time_t              ct,                      // IN    C standard time value
   NTIME              *nt,                      // OUT   NT time value
   LONG                TimeZone                 // IN    TZ offset to GMT (min)
)
{
   LLONG               tstamp;                  // 64bit NT time in nsec ...

   //- ENTER();
   //- TRACES(("Seconds since 1970 : %lu\n", ct));

   tstamp = (ct * TXNTM_NS_D) + (((LLONG) TXNTM_HI) << 32) + TXNTM_LO;

   nt->lo = tstamp &  0xffffffff;
   nt->hi = tstamp >> 32;

   //- TRACES(("NT time value 64bit: %8.8lx %8.8lx\n", nt->hi, nt->lo));
   //- VRETURN();
   return;
}                                               // end 'txCt2WinFileTime'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert DOS, OS/2 and eCS filetime (2 * USHORT) to compiler time_t value
/*****************************************************************************/
time_t txOS2FileTime2t                          // RET   time_t value
(
   USHORT             *pd,                      // IN    USHORT OS2 coded date
   USHORT             *pt                       // IN    USHORT OS2 coded time
)
{
   double              tstamp = 0;
   S_DATE             *date   = (S_DATE *) pd;
   S_TIME             *time   = (S_TIME *) pt;
   long                md = 0;                  // days difference from 1970
   long                y,m;                     // year and month numbers
   long                leaps;

   //- ENTER();
   //- TRACES(("OS2 date %4.4hx time %4.4hx\n", *pd, *pt));

   //- could be recoded to use mktime() and use intermediate struct tm

   y = date->year + 1980;                       // real year
   leaps = (y - 1970) / 4 -1;                   // nr of leap-years since ...
   md = ((y - 1970) * 365) + leaps + date->day; // days since 1-1-1970
   for (m = 1; m < date->month; m++)
   {
      switch (m)
      {
         case 2:                           md += ((y%4)==0) ? 29 : 28; break;
         case 4: case 6: case 9: case 11:  md +=                   30; break;
         default:                          md +=                   31; break;
      }
   }
   tstamp = ((double) ((((md * 24) +  time->hours)  * 60) + time->minutes) *
             (double) 60) + (double) (time->twosecs * 2);

   //- TRACES(("Seconds since 1970 : %lf\n", tstamp));
   //- RETURN( (time_t) tstamp);
   return( (time_t) tstamp);
}                                               // end 'txOS2FileTime2t'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Convert compiler time_t to DOS, OS/2 and eCS filetime (2 * USHORT) value
/*****************************************************************************/
void txCt2OS2FileTime                           // RET   time_t value
(
   time_t              ct,                      // IN    C standard time value
   USHORT             *pd,                      // OUT   USHORT OS2 coded date
   USHORT             *pt                       // OUT   USHORT OS2 coded time
)
{
   struct tm          *tstamp;
   S_DATE             *date   = (S_DATE *) pd;
   S_TIME             *time   = (S_TIME *) pt;

   //- ENTER();
   //- TRACES(("Seconds since 1970 : %lu\n", ct));

   tstamp = gmtime( &ct);

   time->twosecs = tstamp->tm_sec / 2;
   time->minutes = tstamp->tm_min;
   time->hours   = tstamp->tm_hour;

   date->day     = tstamp->tm_mday;
   date->month   = tstamp->tm_mon;
   date->year    = tstamp->tm_year - 80;

   //- TRACES(("OS2 date %4.4hx time %4.4hx\n", *pd, *pt));
   //- VRETURN();
   return;
}                                               // end 'txCt2OS2FileTime'
/*---------------------------------------------------------------------------*/

