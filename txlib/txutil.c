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
// Generic TX utility functions
//
// Author: J. van Wijk
//
// JvW  13-06-2002 Initial version, split off from TXCON

#include <txlib.h>

static  char  txc_ascii7_t[] =                  // translate table 128..255

        "CueaaaaceeeiiiAAEaAooouuyOUcLYPfaiounNao?[]24i<>-=*|++++++|+++++"
// 128  "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿"

        "++++-++++++++=+++++++++++++*=||=abcpE_ut08OSmfeU=+GL[]:~o..vn2. ";
// 192  "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"



/*****************************************************************************/
// Test if area consists completely of the empty byte/character ==> is empty
/*****************************************************************************/
BOOL TxAreaEmpty                                // RET   area is empty
(
   char               *area,                    // IN    area to check
   int                 size,                    // IN    size of area, bytes
   char                empty                    // IN    the empty char
)
{
   BOOL                rc = TRUE;

   while (size--)
   {
      if (*area != empty)
      {
         rc = FALSE;
         break;
      }
      area++;
   }
   return (rc);
}                                               // end 'TxAreaEmpty'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Filter 8-bit ASCII, strip to 7-bit, translate, or replace by specified char
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
void TxAscii827                                 // translate 8-bit to 7-bit
(
   char               *str,                     // INOUT string to convert
   unsigned char       new                      // IN    new char, or 0x00
)
{
   char                ch;

   while ((ch = *str) != 0)
   {
      if ((ch & 0x80) != 0)                     // 8th bit is set
      {
         switch (new)
         {
            case TXASCII827_STRIP:
               *str = (char)(ch & 0x7f);        // simple strip off
               break;

            case TXASCII827_TRANS:
               *str = txc_ascii7_t[ch & 0x7f]; // translate using table
               break;

            default:
               *str = new;                      // replace by specified
               break;
         }
      }
      str++;
   }
}                                               // end 'TxAscii827'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Copy string to destination, replacing non printables, clip and terminate it
/*****************************************************************************/
void TxCopy
(
   char               *dest,                    // OUT   destination string
   char               *source,                  // IN    source string
   int                 len                      // IN    max length incl \0
)
{
   if (len > 0)
   {
      while ((*source) && (--len))
      {
         *dest++ = TxPrintable(*source++);
      }
      *dest = 0;
   }
}                                               // end 'TxCopy'
/*---------------------------------------------------------------------------*/

