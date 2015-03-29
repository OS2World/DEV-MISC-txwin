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
// TxLib filesystem functions, filename manipulations, base name
//

#include <txlib.h>                              // TxLib interface


/*****************************************************************************/
// Get base-name part from a path+filename string
/*****************************************************************************/
char *TxGetBaseName                             // RET   ptr to basename
(
   char               *fname                    // IN    path+filename string
)
{
   char               *rc;

   if      ((rc = strrchr( fname, FS_PATH_SEP)) != NULL)
   {
      rc++;                                     // start of basename
   }
   else if ((rc = strrchr( fname, ':')) != NULL) // bare drive spec (x:file)
   {
      rc++;
   }
   else
   {
      rc = fname;                               // no path, use whole name
   }
   return( rc);
}                                               // end 'TxGetBaseName'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Strip basename from path+filename string, leaving the PATH component only
/*****************************************************************************/
char *TxStripBaseName                           // RET   BaseName location,
(                                               //       or NULL if not there
   char               *fname                    // IN    path+filename string
)                                               // OUT   path only string
{
   char               *rc = strrchr( fname, FS_PATH_SEP);

   if (rc != NULL)
   {
      *(rc++) = 0;                              // terminate at PATH_SEP loc
   }                                            // and advance to BaseName
   return( rc);
}                                               // end 'TxStripBaseName'
/*---------------------------------------------------------------------------*/

