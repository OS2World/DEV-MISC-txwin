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
// TX string number handling functions
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL

#include <txlib.h>


/*****************************************************************************/
// Test if string represents a valid hex number, allow 0x prefix and ,unit
/*****************************************************************************/
BOOL txIsValidHex                               // RET   string is valid hex
(
   char               *num                      // IN    candidate string
)
{
   BOOL                rc = TRUE;               // function return
   char               *s  = num;

   if (strncasecmp( s, "0x", 2) == 0)
   {
      s += 2;
   }
   while ((*s) && (*s != ','))                  // until end or unit separator
   {
      if (isxdigit(*s))
      {
         s++;
      }
      else
      {
         rc = FALSE;
         break;
      }
   }
   return (rc);
}                                               // end 'txIsValidHex'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test if string represents a valid decimal nr, allow 0t/0n prefix and ,unit
/*****************************************************************************/
BOOL txIsValidDec                               // RET   string is not decimal
(
   char               *num                      // IN    candidate string
)
{
   BOOL                rc = TRUE;               // function return
   char               *s  = num;

   if ((strncasecmp( s, "0t", 2) == 0) ||
       (strncasecmp( s, "0n", 2) == 0)  )
   {
      s += 2;
   }
   while ((*s) && (*s != ','))                  // until end or unit separator
   {
      if (isdigit(*s))
      {
         s++;
      }
      else
      {
         rc = FALSE;
         break;
      }
   }
   return (rc);
}                                               // end 'txIsValidDec'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test if string starts with a valid MCS prefix 0x 0n 0t 0x (case insensitive)
/*****************************************************************************/
BOOL txHasMcsPrefix                             // RET   string has MCS prefix
(
   char               *num                      // IN    candidate string
)
{
   BOOL                rc = FALSE;              // function return

   ENTER();
   TRACES(("num string: '%s'\n", num));

   if (num[0] == '0')
   {
      if (strchr("NnOoTtXx", num[1]) != NULL)
      {
         rc = TRUE;
      }
   }
   BRETURN (rc);
}                                               // end 'txHasMcsPrefix'
/*---------------------------------------------------------------------------*/

