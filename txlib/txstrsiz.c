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
// TX size value to string or TxPrint
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL

#include <txlib.h>


/*****************************************************************************/
// Format (file) size in 7.1 value + Byte/KiB/MiB/GiB (11); to TxPrint output
/*****************************************************************************/
void txPrtSize64
(
   char               *text,                    // IN    leading string
   LLONG               data,                    // IN    size data
   char               *trail                    // IN    trailing text
)
{
   TXLN                string;

   strcpy( string, "");
   TxPrint("%s", txStrSize64( text, string, data, trail));
}                                               // end 'txPrtSize64'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Format (file) size in 7.1 value + Byte/KiB/MiB/GiB (11); append to string
/*****************************************************************************/
char *txStrSize64                               // RET   resulting string
(
   char               *str,                     // OUT   resulting string
   char               *text,                    // IN    leading string
   LLONG               data,                    // IN    size data
   char               *trail                    // IN    trailing text
)
{
   TXTS                form;

   strcat( str, text);
   if (data <= 99999)                           // express as a byte-value
   {
      sprintf( form, "%5lld bytes", data);
   }
   else                                         // use KiB/MiB/GiB/TiB
   {
      double value = ((double) data) / 1024.0;  // start with KiB

      if      (value <= 4999.9)                 // arbitrary, but 7.8 GiB
      {                                         // works out OK then :-)
         sprintf( form, "%7.1lf KiB", value);   // consistent with DFSee
      }
      else
      {
         value /= 1024.0;                       // convert to MiB
         if (value <= 49999.9)
         {
            sprintf(form, "%7.1lf MiB", value);
         }
         else
         {
            value /= 1024.0;                    // convert to GiB
            if (value <= 49999.9)
            {
               sprintf(form, "%7.1lf GiB", value);
            }
            else
            {
               value /= 1024.0;                 // convert to TiB
               sprintf(form, "%7.1lf TiB", value);
            }
         }
      }
   }
   strcat( str, form);
   strcat( str, trail);
   return( str);
}                                               // end 'txStrSize64'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Format time in seconds in an HHH:MM:SS string (Elapsed/ETA)
/*****************************************************************************/
char *txStrSec2hms                              // RET   resulting string
(
   char               *str,                     // OUT   resulting string
   char               *text,                    // IN    leading string
   ULONG               sec,                     // IN    seconds
   char               *trail                    // IN    trailing text
)
{
   TXTS                form    = {0};
   ULONG               hours   = (sec / 3600);
   ULONG               minutes = (sec % 3600) / 60;
   ULONG               seconds = (sec % 60);

   strcat( str, text);

   if (hours)
   {
      sprintf( form, "%lu:", hours);
   }
   strcat( str, form);

   sprintf( form, "%lu:%02lu", minutes, seconds);
   if (minutes < 10)
   {
      strcat( str, (hours) ? "0" : " ");
   }
   strcat( str, form);
   strcat( str, trail);
   return( str);
}                                               // end 'txStrSec2hms'
/*---------------------------------------------------------------------------*/

