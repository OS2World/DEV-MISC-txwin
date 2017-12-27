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
// TX formatting function for mixed ASCII/HEX strings
// file-logging facilities
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXCON.C

#include <txlib.h>


#define               ST_SKIPW   0              // skip whitespace
#define               ST_HEX_0   1              // first HEX digit
#define               ST_HEX_1   2              // second HEX digit
#define               ST_ASCII   3              // copy plain ASCII
#define               ST_UNICO   4              // generate UNICODE from ASCII
/*****************************************************************************/
// Translate Mixed Ascii / Hex string specification to length + buffer
/*****************************************************************************/
USHORT TxFormatMixedStr                         // RET   length of buffer
(
   char               *data,                    // IN    mixed string
   char               *buf                      // OUT   formatted buffer
)
{
   char               *s;
   int                 c;
   USHORT              i;
   char                hex[3];
   int                 state = ST_SKIPW;        // parsing state, ascii/hex

   hex[2] = '\0';
   for (s=data, i=0; *s; s++)
   {
//    TxPrint("\nParse state %u, char: '%c' for buffer pos: %u", state, *s, i);
      switch (state)
      {
         case ST_SKIPW:
            switch (*s)
            {
               case TXk_SPACE:
                  break;

               default:
                  s--;                          // evaluate again in new state
                  state = ST_ASCII;
                  break;
            }
            break;

         case ST_HEX_0:
            switch (*s)
            {
               case '\'':
                  state = ST_ASCII;
                  break;

               case '0': case '1': case '2': case '3': case '4':
               case '5': case '6': case '7': case '8': case '9':
               case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
               case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                  hex[0] = *s;
                  state = ST_HEX_1;
                  break;

               case TXk_SPACE:
               case '-':
               case '}':
                  break;

               default:
                  TxPrint("\nInvalid hex char in mixed string at char: %c", *s);
                  break;
            }
            break;

         case ST_HEX_1:
            switch (*s)
            {
               case '\'':
                  TxPrint("\nSingle hex char in mixed string at char: %c", *s);
                  state = ST_ASCII;             // error, single HEX char
                  break;

               case '0': case '1': case '2': case '3': case '4':
               case '5': case '6': case '7': case '8': case '9':
               case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
               case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                  hex[1] = *s;
                  if (sscanf( hex, "%X", &c) != 0)
                  {
                     buf[i++] = (char) c;
                  }
                  state = ST_HEX_0;
                  break;

               case TXk_SPACE:
               case '-':
               case '}':
                  break;

               default:
                  TxPrint("\nInvalid hex char in mixed string at char: %c", *s);
                  break;
            }
            break;

         case ST_UNICO:
            switch (*s)
            {
               case '"':
                  state = ST_ASCII;
                  break;

               default:
                  buf[i++] = *s;
                  buf[i++] = 0;
                  break;
            }
            break;

         default:
            switch (*s)
            {
               case '\'':                       // to HEX in mixed string
               case '{':                        // to HEX for UUID format
                  state = ST_HEX_0;
                  break;

               case '"':                        // to UNICODE
                  state = ST_UNICO;
                  break;

               default:                         // just copy ASCII
                  buf[i++] = *s;
                  break;
            }
            break;
      }
   }
   return (i);
}                                               // end 'TxFormatMixedStr'
/*---------------------------------------------------------------------------*/

