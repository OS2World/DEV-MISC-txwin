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
// TX string to TX-string-list conversion and handling functions
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL

#include <txlib.h>


/*****************************************************************************/
// Add string to string-list (allocate memory)
/*****************************************************************************/
char *TxStrListAdd                              // RET   added string
(
   TXSTRLIST         **list,                    // IN    address of list (head)
   char               *str                      // IN    string to add
)
{
   char               *rc = NULL;               // function return
   TXSTRLIST         **elem = list;

   ENTER();

   if (list && str)
   {
      while (*elem != NULL)                     // find end of list
      {
         *elem = (*elem)->next;
      }
      if ((*elem = TxAlloc( 1, sizeof(TXSTRLIST))) != NULL)
      {
         if ((rc = TxAlloc( 1, strlen( str) +1)) != NULL)
         {
            strcpy( rc, str);
            (*elem)->string = rc;
         }
      }
   }
   RETURN (rc);
}                                               // end 'TxStrListAdd'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Free string-list (free memory)
/*****************************************************************************/
void  TxStrListFree
(
   TXSTRLIST         **list                     // IN    address of list (head)
)
{
   TXSTRLIST         **elem;
   TXSTRLIST         **next;

   ENTER();

   for (elem = list; *elem; elem = next)
   {
      next = &(( *elem)->next);
      TxFreeMem( *elem);
   }
   VRETURN();
}                                               // end 'TxStrListFree'
/*---------------------------------------------------------------------------*/

