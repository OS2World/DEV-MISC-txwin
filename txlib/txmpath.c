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
// TxLib filesystem functions, make std and 8.3 paths
//

#include <txlib.h>                              // TxLib interface

#include <sys/stat.h>                           // for low level stuff


/*****************************************************************************/
// Create path in a recursive fashion
/*****************************************************************************/
ULONG TxMakePath                                // RET   result
(
   char               *filename                 // IN    relative/absolute path
)
{
   ULONG               rc = NO_ERROR;           // return value
   #if defined (WIN32)
      char            *p = NULL;
      ULONG            ulAttr;
   #elif defined (DOS32)
      char            *p = NULL;
      unsigned         usAttr;                  // attributes (dos.h)
   #elif defined (UNIX)
      //- to be refined
   #else
      char            *p = NULL;
      FILESTATUS3      fstat;
   #endif

   ENTER();

   #if defined (WIN32)
      ulAttr = GetFileAttributes(filename);
      if (ulAttr != TXINVALID)
      {
         if ((ulAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
         {
            TxPrint("\nCannot create directory, a file '%s' exists  ", filename);
            rc = ERROR_FILE_EXISTS;             // file with same name exists
         }
      }
      else                                      // not existing
      {
         rc = NO_ERROR;
         if (strcmp(filename+1, ":"))           // no bare drive specification
         {
            if ((p = TxStripBaseName(filename)) != NULL) // last component
            {
               rc = TxMakePath(filename);
               *(--p) = FS_PATH_SEP;            // join last part again
            }
            if (rc == NO_ERROR)
            {
               if (!CreateDirectory(filename, NULL)) // New, no security
               {
                  rc = TX_INVALID_PATH;
               }
            }
         }
      }
   #elif defined (DOS32)
      rc = (ULONG) _dos_getfileattr(filename, &usAttr);
      TRACES(("get attr, RC %lu, attr %hx on: '%s'\n", rc, usAttr, filename));
      if (rc == NO_ERROR)
      {
         if ((usAttr & _A_SUBDIR) == 0)         // it is an existing file
         {
            TxPrint("\nCannot create directory, a file '%s' exists  ", filename);
            rc = ERROR_FILE_EXISTS;             // file with same name exists
         }
      }
      else                                      // not existing
      {
         rc = NO_ERROR;
         if (strcmp(filename+1, ":"))           // no bare drive specification
         {
            if ((p = TxStripBaseName(filename)) != NULL) // find last component
            {
               rc = TxMakePath(filename);
               *(--p) = FS_PATH_SEP;            // join last part again
            }
            if (rc == NO_ERROR)
            {
               if (mkdir( filename) != 0)
               {
                  rc = TX_INVALID_PATH;
               }
            }
         }
      }
   #elif defined (UNIX)
      //- to be refined
   #else
      rc = DosQPathInfo(filename,               // Path string
                        FIL_STANDARD,           // Path data required
                        &fstat,                 // Path data buffer
                        sizeof(fstat));         // Path data buffer size
      TRACES(("DosQPathInfo, RC %lu on: '%s'\n", rc, filename));
      if (rc == NO_ERROR)
      {
         if ((fstat.attrFile & FILE_DIRECTORY) == 0)
         {
            TxPrint("\nCannot create directory, a file '%s' exists  ", filename);
            rc = ERROR_FILE_EXISTS;             // file with same name exists
         }
      }
      else
      {
         rc = NO_ERROR;
         if (strcmp(filename+1, ":"))           // no bare drive specification
         {
            if ((p = TxStripBaseName(filename)) != NULL) // find last component
            {
               rc = TxMakePath(filename);
               *(--p) = FS_PATH_SEP;            // join last part again
            }
            if (rc == NO_ERROR)
            {
               rc = DosMkDir(filename, NULL);   // Create New, without EA's
            }
         }
      }
   #endif
   RETURN (rc);
}                                               // end 'TxMakePath'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Make an 8-dot-3 compatible copy of supplied path/filename string (no spaces)
/*****************************************************************************/
ULONG TxMake8dot3                               // RET   length converted name
(
   char               *filename,                // IN    relative/absolute path
   char               *fname8d3                 // OUT   8dot3 compatible
)
{
   ULONG               rc = 0;                  // return value
   char               *p  = filename;           // ptr in source name
   char               *d  = fname8d3;           // ptr in converted name
   ULONG               cs = 0;                  // checksum for segment
   ULONG               i  = 0;                  // length converted part

   ENTER();

   TRACES(("filename : '%s'\n", filename));
   do
   {
      if ((*p == '\\') || (*p == '/') ||        // end of any segment
          (*p == ':')  || (*p == 0))
      {
         if (*p == ':')                         // end of drive part
         {
            cs = 0;                             // no cs on drive part
         }
         else if ((cs != 0) && (i > 8))         // add cs to converted ?
         {                                      // . plus 3 upcase hex digits
            cs = ((cs ^ (cs >> 24)) &0xfff);    // XOR and shuffle a bit
            sprintf( d, ".%3.3lX", cs);
            d += strlen(d);                     // advance to end of string
         }
         *d++ = *p;                             // copy character, convert space
         i    = 0;                              // reset count on next segment
         cs   = 0;                              // reset checksum
      }
      else                                      // other characters
      {
         if (i < 8)
         {
            if (*p == '.')                      // dot in dir/file name
            {
               if (strcspn( p+1, "\\/") <= 3)   // will be valid if total
               {                                // stays 12 or less (8+1+3)
                  *d = '.';                     // so keep the original dot
                  i  = 4;                       // avoid conversion on next 3
               }
               else                             // translate to '_' because
               {                                // .XXX will be added for cs
                  *d = '_';
                  i++;
               }
            }
            else
            {
               *d = *p;                         // copy character
               i++;
            }
            d++;                                // advance counter & dest
         }
         else                                   // conversion will happen
         {
            if ((i == 8) && (*p == '.'))        // dot in dir/file name
            {
               if (strcspn( p+1, "\\/") <= 3)   // will be valid if total
               {                                // stays 12 or less (8+1+3)
                  *d++ = '.';                   // so keep the original dot
                  i    = 4;                     // avoid conversion on next 3
               }
               else                             // translate to '_' because
               {                                // .XXX will be added for cs
                  *(d-3) = *(d-2);              // shift last 2 chars back
                  *(d-2) = *(d-1);
                  if (*p == '.')
                  {
                     *(d-1) = '_';
                  }
                  else
                  {
                     *(d-1) = *p;
                  }

                  i++;
                  rc = 1;                       // signal conversion made
               }
            }
            else
            {
               *(d-3) = *(d-2);                 // shift last 2 chars back
               *(d-2) = *(d-1);
               if (*p == '.')
               {
                  *(d-1) = '_';
               }
               else
               {
                  *(d-1) = *p;
               }

               i++;
               rc = 1;                          // signal conversion made
            }
         }
         cs *= 2;                               // multiply by 2
         cs += *p;                              // add char to checksum
      }
      if (*p != 0)
      {
         p++;
      }
   } while ((*p != 0) || (i >= 8));             // until end and cs added
   *d = 0;                                      // always terminated

   TxRepl( fname8d3, ' ', '_');                 // translate spaces
   TxRepl( fname8d3, '*', '-');                 // translate wildcard '*'
   TxRepl( fname8d3, '?', '-');                 // translate wildcard '?'
   TxRepl( fname8d3, '&', '!');                 // translate illegal  '&'
   TxRepl( fname8d3, '=', '!');                 // translate illegal  '='
   TxRepl( fname8d3, '+', '!');                 // translate illegal  '+'
   if (rc != 0)                                 // conversion made ?
   {
      rc = strlen( fname8d3);
   }
   TRACES(("fname out: '%s'\n", fname8d3));
   RETURN (rc);
}                                               // end 'TxMake8dot3'
/*---------------------------------------------------------------------------*/

