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
// TxPrint hook mechanism handling
//
// Author: J. van Wijk
//
// JvW  19-08-2005 Initial version, split off from TXCON.C

#include <txlib.h>
#include <txtpriv.h>


/*****************************************************************************/
// Perform operations on TxPrint HOOK chain
/*****************************************************************************/
BOOL TxPrintHook                                // RET   success
(
   TXH_TYPE            type,                    // IN    type of handler
   TXH_OPERATION       operation,               // IN    requested operation
   TXH_INFO           *hinfo                    // IN    handler info
)
{
   BOOL                rc = TRUE;
   TXH_INFO          **root;                    // root of handler chain
   TXH_INFO           *this;                    // existing handler if found
   TXH_INFO           *last;                    // insertion point in chain

   ENTER();
   TRARGS(("type: %s, oper %s, hinfo: %8.8lX\n",
          (type      == TXH_T_RAW   ) ? "RAW"      : "CLEAN",
          (operation == TXH_REGISTER) ? "REGISTER" : "DEREGISTER", hinfo));

   switch (type)
   {
      case TXH_T_RAW: root = &txh_raw;   break;
      default:        root = &txh_clean; break;
   }
   for (last = NULL, this  = *root;
                     this != NULL;
        last = this, this  = this->next)
   {
      if (this == hinfo)                        // handler already present
      {
         break;
      }
   }
   switch (operation)
   {
      case TXH_REGISTER:
         if (this == NULL)                      // new handler
         {
            if (last == NULL)                   // first handler
            {
               *root = hinfo;
               hinfo->prev = NULL;
            }
            else                                // another handler
            {
               last->next  = hinfo;
               hinfo->prev = last;
            }
            hinfo->next    = NULL;
         }
         else                                   // already registred
         {
            rc = FALSE;
         }
         break;

      case TXH_DEREGISTER:
         if (this != NULL)                      // existing handler ?
         {
            if (this == *root)                  // first in chain
            {
               *root = this->next;
            }
            else
            {
               this->prev->next = this->next;
            }
            if (this->next != NULL)             // not last in chain ?
            {
               this->next->prev = this->prev;
            }
            hinfo->prev = NULL;
            hinfo->next = NULL;
         }
         else                                   // not registred
         {
            rc = FALSE;
         }
         break;

      default:
         TRACES(("Invalid hook operation: %lu\n", operation));
         break;
   }
   BRETURN(rc);
}                                               // end 'TxPrintHook'
/*---------------------------------------------------------------------------*/

