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
// CRC32 implementation, compatible with OS/2 LVM data-structure checksums
//
// Author: J. van Wijk
//
//

#include <txlib.h>                              // TxLib interface

#define DFS_LVM_CRC_RANGE  256
#define DFS_LVM_CRC_POLY   0xEDB88320L
#define DFS_LVM_CRC_INIT   0xFFFFFFFFL

static ULONG          *crcTable = NULL;         // for LVM type CRC

// Allocate and prepare the CRC-table needed for LVM CRC calculation
static void TxCreateLvmCrcTable
(
   void
);


/*****************************************************************************/
// Allocate and prepare the CRC-table needed for LVM CRC calculation
/*****************************************************************************/
static void TxCreateLvmCrcTable
(
   void
)
{
   crcTable = TxAlloc(DFS_LVM_CRC_RANGE, sizeof(ULONG));
   if (crcTable != NULL)
   {
      ULONG            i;
      ULONG            j;
      ULONG            value;

      for ( i = 0; i < DFS_LVM_CRC_RANGE ; i++ )
      {
        value = i;

        for ( j = 8 ; j > 0; j-- )
        {
           if (value & 1)
           {
              value = (value >> 1) ^ DFS_LVM_CRC_POLY;
           }
           else
           {
              value >>= 1;
           }
        }
        crcTable[i] = value;
      }
   }
}                                               // end 'TxCreateLvmCrcTable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Calculate 32-bit CRC value using LVM compatible algorithm and polynom
/*****************************************************************************/
ULONG TxCalculateLvmCrc
(
   BYTE               *area,                    // IN    data area needing CRC
   ULONG               size                     // IN    size of the data area
)
{
   ULONG               rc = DFS_LVM_CRC_INIT;   // function return, initial

   if (crcTable == NULL)
   {
      TxCreateLvmCrcTable();
   }
   if (crcTable != NULL)
   {
      BYTE            *this = area;             // current byte handled
      ULONG            t1;
      ULONG            t2;                      // intermediate values
      ULONG            i;

      for (this = area, i = 0; i < size; i++, this++)
      {
        t1 = (rc >> 8) & 0xffffff;
        t2 = crcTable[ (rc ^ (ULONG) *this) & (ULONG) 0xff];
        rc = t1 ^ t2;
      }
   }
   return (rc);
}                                               // end 'TxCalculateLvmCrc'
/*---------------------------------------------------------------------------*/

