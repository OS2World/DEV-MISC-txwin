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
// TxLib Unicode string handling
//

#include <txlib.h>                              // TxLib interface

/*****************************************************************************/
// Append translated Mac-Unicode string-fragment to an ASCII string
/*****************************************************************************/
char *TxMacUniAppend                            // RET   ptr to ASCII string
(
   S_MACUNI_STRING    *macuni,                  // IN    Mac-Unicode string part
   char               *asc                      // INOUT ASCII string
)
{
   char               *pos = asc + strlen(asc); // append position
   USHORT              len = TxBE16( macuni->length);
   USHORT              i;
   USHORT              uch;                     // unicode character

   for ( i = 0;
        (i < len) && ((uch = TxBE16( macuni->unicode[i])) != 0);
         i++)
   {
      if (uch > 0xff)
      {
         if (uch != 0xffff)
         {
            *pos++ = '*';                       // to be refined
         }
      }
      else
      {
         *pos++ = (char) (uch & 0xff);
      }
   }
   *pos = '\0';                                 // terminate the string
   return (asc);
}                                               /* end TxMacUniAppend        */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Append translated Unicode string-fragment to an ASCII string
/*****************************************************************************/
char *TxUnicAppend                              // RET   ptr to ASCII string
(
   USHORT             *uni,                     // IN    Unicode string part
   char               *asc,                     // INOUT ASCII string
   USHORT              len                      // IN    maximum Unicode length
)
{
   char               *pos = asc + strlen( asc); // append position
   USHORT              i;

   for (i = 0; i < len && uni[i] != 0; i++)     // until len or zero-unicode
   {
      if (uni[i] > 0xff)
      {
         if (uni[i] != 0xffff)
         {
            *pos++ = '*';                       // to be refined
         }
      }
      else
      {
         *pos++ = (char) (uni[i] & 0xff);
      }
   }
   *pos = '\0';                                 // terminate the string
   return (asc);
}                                               /* end TxUnicAppend          */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Detect if input is Unicode or ASCII, Append fragment to an ASCII string
/*****************************************************************************/
char *TxUnicAsciiAppend                         // RET   ptr to ASCII string
(
   USHORT             *uni,                     // IN    Unicode or ASCII string
   char               *asc,                     // INOUT ASCII string
   USHORT              len                      // IN    maximum Unicode length
)
{
   if (TxUnicAsciiLength(     uni, len) > 5)
   {
      TXTM    txt;
      TxCopy( txt, (char *) uni, len  * 2);
      strcat( asc, txt);
   }
   else
   {
      TxUnicAppend( uni, asc,    len);
   }
   return (asc);
}                                               /* end TxUnicAsciiAppend     */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Perform a Unicode to ASCII case-sensitive stringcompare (aka strncmp)
/*****************************************************************************/
int TxUnicStrncmp                               // RET   compare result
(
   char               *uni,                     // IN    Unicode string
   char               *asc,                     // IN    ASCII   string
   size_t              len                      // IN    max ASCII comp-length
)
{
   size_t              i;
   int                 rc = 0;

   for ( i = 0; (rc == 0) && (i < len) && *asc; i++, asc++, uni += 2)
   {
      rc = *uni - *asc;
   }
   return (rc);
}                                               /* end TxUnicStrncmp         */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Perform a Unicode to ASCII case-insensitive stringcompare (aka strncasecmp)
/*****************************************************************************/
int TxUnicStrnicmp                              // RET   compare result
(
   char               *uni,                     // IN    Unicode string
   char               *asc,                     // IN    ASCII   string
   size_t              len                      // IN    max ASCII comp-length
)
{
   size_t              i;
   int                 rc = 0;

   for ( i = 0; (rc == 0) && (i < len) && *asc; i++, asc++, uni += 2)
   {
      rc = ((*uni) & 0xdf) - ((*asc) & 0xdf);
   }
   return (rc);
}                                               /* end TxUnicStrnicmp        */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get length of possible Unicode string that is PURE ASCIIT, no zeroes etc
// Useful to determine how to display an unknown string (test if > 5 or so)
/*****************************************************************************/
int TxUnicAsciiLength                           // RET   length pure ASCII
(
   USHORT             *uni,                     // IN    Unicode string part
   USHORT              len                      // IN    maximum Unicode length
)
{
   char               *s = (char *) uni;
   int                 i = 0;

   while ((*s) && (i < (2 * len)))
   {
      if (TxIsPureAscii( *s++))
      {
         i++;
      }
      else
      {
         break;
      }
   }
   return (i);
}                                               /* end TxUnicAsciiLength     */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Translate Unicode string-fragment to an ASCII string (unicode little endian)
/*****************************************************************************/
char *TxUnic2Ascii                              // RET   ptr to ASCII string
(
   USHORT             *uni,                     // IN    Unicode string part
   char               *asc,                     // INOUT ASCII string
   USHORT              len                      // IN    maximum Unicode length
)
{
   asc[0] = 0;                                  // start with empty result
   return( TxUnicAppend( uni, asc, len));
}                                               /* end TxUnic2Ascii          */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Translate ASCII string to Unicode string             (unicode little endian)
/*****************************************************************************/
USHORT *TxAscii2Unic                            // RET   ptr to Unicode string
(
   char               *asc,                     // IN    ASCII string
   USHORT             *uni,                     // INOUT Unicode string part
   USHORT              len                      // IN    maximum Unicode length
)
{
   int           i;
   char          ch;

   for (i = 0; i < len; i++)
   {
      ch = asc[ i];                             // ASCII character from name
      uni[ i] = (USHORT) ch;                    // write as UNICODE

      if (ch == 0)                              // end of ASCII string
      {
         break;
      }
   }
   return( uni);
}                                               /* end TxUnic2Ascii          */
/*---------------------------------------------------------------------------*/


