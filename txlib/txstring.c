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
// Generic TX string functions
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL

#include <txlib.h>

/*****************************************************************************/
// Replace specfied character by another in whole string, count replacements
/*****************************************************************************/
int TxRepl                                      // RET   nr of replacements
(
   char               *str,                     // INOUT string to convert
   char                old,                     // IN    old char, to replace
   char                new                      // IN    new char
)
{
   int                 rc = 0;

   while (*str)
   {
      if (*str == old)
      {
         *str = new;
         rc++;
      }
      str++;
   }
   return (rc);
}                                               // end 'TxRepl'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Copy string, and replace specfied character by a string
/*****************************************************************************/
int TxRepStr                                    // RET   nr of replacements
(
   char               *str,                     // IN    string to convert
   char                old,                     // IN    old char, to replace
   char               *new,                     // IN    new string
   char               *dest,                    // OUT   destination string
   int                 len                      // IN    max length destination
)
{
   int                 rc = 0;
   char               *s;
   int                 l;

   for (s = dest, l = 0; *str && (l < len); str++)
   {
      if (*str == old)
      {
         l += strlen(    new);                  // length to be
         if (l < len)
         {
            *s = '\0';                          // terminate for concat
            strcat(  s,  new);                  // and concat new str
            s += strlen( new);                  // at string end ...
            rc++;
         }
      }
      else
      {
         *s++ = *str;                           // just copy one char
         l++;                                   // and count ...
      }
   }
   *s = '\0';                                   // make sure it is terminated
   return (rc);
}                                               // end 'TxRepStr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Strip leading/trailing characters from a string, dest and source can be same
/*****************************************************************************/
int TxStrip                                     // RET   nr of stripped chars
(
   char               *dest,                    // OUT   destination string
   char               *source,                  // IN    source string
   char                lead,                    // IN    leading  chars to strip
   char                trail                    // IN    trailing chars to strip
)
{
   int                 rc = 0;
   char               *s = dest;

   while ((*source) && (*source == lead))
   {
      source++;
      rc++;
   }
   while (*source)
   {
      *s++ = *source++;
   }
   *s = 0;
   while ((s-- > dest) && (*s == trail))
   {
      *s = 0;
      rc++;
   }
   return (rc);
}                                               // end 'TxStrip'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Pad/Clip a string to specified length, concatenating or removing characters
/*****************************************************************************/
int TxPClip                                     // RET   +:padded -:removed
(
   char               *str,                     // INOUT destination string
   int                 size,                    // IN    requested length
   char                pad                      // IN    trailing chars to pad
)
{
   int                 pos = strlen(str);
   int                 rc  = size - pos;

   str[ size] = 0;                              // terminate at wanted length

   while (pos < size)
   {
      str[pos] = pad;
      pos++;
   }
   return (rc);
}                                               // end 'TxPClip'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert string to all uppercase (like non-possix strupr function)
/*****************************************************************************/
char *TxStrToUpper                              // RET   Updated string
(
   char               *str                      // INOUT destination string
)
{
   char               *s;

   for (s = str; *s; s++)
   {
      *s = toupper( *s);
   }
   return (str);
}                                               // end 'TxStrToUpper'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert string to all lowercase (like non-possix strlwr function)
/*****************************************************************************/
char *TxStrToLower                              // RET   Updated string
(
   char               *str                      // INOUT destination string
)
{
   char               *s;

   for (s = str; *s; s++)
   {
      *s = tolower( *s);
   }
   return (str);
}                                               // end 'TxStrToLower'
/*---------------------------------------------------------------------------*/

