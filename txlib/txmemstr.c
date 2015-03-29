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
// Generic TX string functions, memory string (not null terminated)
//
// Author: J. van Wijk
//
// JvW  20-08-2005 Initial version, split off from TXSTRING

#include <txlib.h>


/*****************************************************************************/
// Find occurence of a string in a memory region (with embedded NUL)
/*****************************************************************************/
char *TxMemStr                                  // RET   ptr to string or NULL
(
   void               *mem,                     // IN    memory region
   char               *str,                     // IN    string to find
   int                 len                      // IN    length of mem region
)
{
   char               *rc = NULL;
   char               *mr = (char *) mem;
   int                 at = 0;                 // current position from start
   int                 sl = strlen( str);
   char               *hit;

   while ((rc == NULL) && ((hit = memchr( mr + at, str[0], len -at)) != NULL))
   {
      if (strncmp( hit, str, sl) == 0)         // found a match
      {
         rc = hit;
      }
      else
      {
         at = hit - mr +1;
      }
   }
   TRACES(("TxMemStr: mem:%8.8lx, str:'%s' rc:%8.8lx\n", mem, str, rc));
   return (rc);
}                                               // end 'TxMemStr'
/*---------------------------------------------------------------------------*/

