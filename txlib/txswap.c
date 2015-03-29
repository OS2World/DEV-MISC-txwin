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
// Binary value SWAP low/high parts
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL

#include <txlib.h>


/*****************************************************************************/
// Convert USHORT value from big to little endian or vice-versa
/*****************************************************************************/
USHORT txSwapUS                                 // RET   converted value
(
   USHORT              val                      // IN    value
)
{
   USHORT              rc;

   rc  = (val & 0x00ff) << 8;
   rc += (val & 0xff00) >> 8;

   return (rc);
}                                               // end 'txSwapUS'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert ULONG value from big to little endian or vice-versa
/*****************************************************************************/
ULONG txSwapUL                                  // RET   converted value
(
   ULONG               val                      // IN    value
)
{
   ULONG               rc;

   rc  = (val & 0x000000ff) << 24;
   rc += (val & 0x0000ff00) <<  8;
   rc += (val & 0x00ff0000) >>  8;
   rc += (val & 0xff000000) >> 24;

   return (rc);
}                                               // end 'txSwapUL'
/*---------------------------------------------------------------------------*/

