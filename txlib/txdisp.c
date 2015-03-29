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
// TX display functions for HEX and ASCII data structures
// file-logging facilities
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXCON.C

#include <txlib.h>

/*****************************************************************************/
// Make ascii-dump of data area on TxPrint output
/*****************************************************************************/
void TxDisplAscDump
(
   char               *lead,                    // IN    leading string
   BYTE               *data,                    // IN    data area
   ULONG               size                     // IN    size to dump
)
{
   BYTE               *s;
   ULONG               i;

   if ((data != NULL) && (size != 0) && (!TxAbort()))
   {
      TxPrint("%s", lead);
      for (s = data, i = 0; i < size && (*s != 0x1a) && !TxAbort(); s++, i++)
      {
         TxPrint("%c", (char ) *s);
      }
   }
}                                               // end 'TxDisplAscDump'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Make hex-dump of data area on TxPrint output
/*****************************************************************************/
void TxDisplHexDump
(
   BYTE               *data,                    // IN    data area
   ULONG               size                     // IN    size to dump
)
{
   TxDisplayHex( "", data, size, 0);
}                                               // end 'TxDisplHexDump'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Make hex-dump of based data area on TxPrint output, with leading text
/*****************************************************************************/
void TxDisplayHex
(
   char               *lead,                    // IN    leading text
   BYTE               *data,                    // IN    data area
   ULONG               size,                    // IN    size to dump
   ULONG               base                     // IN    base for display
)
{
   BYTE                *s;
   BYTE                c;
   ULONG               i;
   ULONG               n = 0;
   TXTM                ascii;                   // ascii part buffer
   TXTM                line;                    // accumulating buffer
   TXTS                hex;                     // temporary hex buffer

   if ((data != NULL) && (size != 0))
   {
      TxPrint("%s", lead);
      for (s=data, i=0; i < size; s++, i++)
      {
         n = i % 16;
         switch (n)
         {
            case 0:
               if (i)
               {
                  TxPrint("%s %s[%s%s%s]%s\n", line, CNC, CNN, ascii, CNC, CNN);
               }
               memset(ascii, 0, TXMAXTM);
               if (i && (i % 1024) == 0)        // every 64 lines a formfeed
               {
                  TxPrint("Byte offset: %s%lu%s\n", CBG, i, CNN);
               }
               sprintf( line, "%s-%5.5lX-%s ", CBZ, i + base, CNN);
               break;

            case 8:
               strcat( line, " ");
               break;

            default:
               break;
         }
         c = *s;
         ascii[n] = TxPrintable(c);
         sprintf( hex, " %2.2x", (int) c & 0xff);
         strcat( line, hex);
      }
      if (n < 8)
      {
         strcat( line, " ");                    // middle column
      }
      for (; n < 15; n++)
      {
         strcat( line, "   ");                  // fill rest with spaces
      }
      TxPrint( "%s %s[%s%s%s]%s\n", line, CNC, CNN, ascii, CNC, CNN);
   }
}                                               // end 'TxDisplayHex'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Make 'LONG' hex-dump of data area on TxPrint output, with 'C' comment ASCII
/*****************************************************************************/
void TxDispLongHex
(
   BYTE               *data,                    // IN    data area
   ULONG               size                     // IN    size to dump
)
{
   BYTE                *s;
   ULONG               value = 0;
   BYTE                c;
   ULONG               i;
   ULONG               n = 0;
   TXTM                ascii;                   // ascii part buffer
   TXTM                line;                    // accumulating buffer
   TXTS                hex;                     // temporary hex buffer

   if ((data != NULL) && (size != 0))
   {
      for (s=data, i=0; i < size; s++, i++)
      {
         if ((n = i % 16) == 0)
         {
            if (i)
            {
               TxPrint("%s%s// %s%s%s%s\n", line, CNC, CNN, ascii, CNC, CNN);
            }
            memset(ascii, 0, TXMAXTM);
            strcpy( line, "   ");
         }
         c = *s;
         ascii[n] = TxPrintable(c);

         switch (i % 4)
         {
            case 0: value  = c;       break;
            case 1: value |= c <<  8; break;
            case 2: value |= c << 16; break;
            case 3: value |= c << 24;
               sprintf( hex, "0x%8.8lx%c ", value, (i < (size -1)) ? ',' : ' ');
               strcat( line, hex);
               break;
         }
      }
      for (; n < 15; n += 4)
      {
         strcat( line, "            ");         // fill rest with spaces
      }
      TxPrint("%s%s// %s%s%s%s\n", line, CNC, CNN, ascii, CNC, CNN);
   }
}                                               // end 'TxDispLongHex'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return ascii printable character, conservative small reange
/*****************************************************************************/
char TxPrintable                                // RET   printable character
(
   char                c                        // IN    character
)
{
   if (isprint(c))
   {
      return (c);
   }
   else
   {
      return ('.');
   }
}                                               // end 'TxPrintable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return ascii printable character, widest safe range
/*****************************************************************************/
char TxPrintSafe                                // RET   printable character
(
   char                c                        // IN    character
)
{
   if ((c!=0x00) && (c!=0x07) && (c!=0x08) && (c!=0x09) &&
       (c!=0x0a) && (c!=0x0c) && (c!=0x0d) && (c!=0x1a) && (c!=0x1b))
   {
      return (c);
   }
   else
   {
      return ('ú');
   }
}                                               // end 'TxPrintSafe'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Output textual info in array of char-pointers, add newlines in-between
/*****************************************************************************/
void TxShowTxt
(
   char               *txt[]                    // IN    text to display
)
{
   char              **s;

   for (s  = txt; s && (*s != NULL); s++)
   {
      TxPrint( "%s%s", (s!= txt) ? "\n": "", *s);
   }
}                                               // end 'TxShowTxt'
/*---------------------------------------------------------------------------*/

