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
// TxLib filesystem functions, Critical-error handling
//

#include <txlib.h>                              // TxLib interface
#include <txtpriv.h>                            // private interface


#if defined (DOS32)
   ULONG            TxAutoFail   = 1;           // start in AutoFail for DOS
#else
   ULONG            TxAutoFail   = 0;           // but NOT for OS/2 and WIN!
#endif                                          // to assure first autofail
                                                // call to succeeed


/*****************************************************************************/
// Set critical error handling to auto-fail (versus interactive Abort, retry .)
// Calls to this may be nested, autofail is ON until matching fail==false call
/*****************************************************************************/
void TxFsAutoFailCriticalErrors
(
   BOOL                fail                     // IN    autofail on CritErr
)
{
   ENTER();
   TRACES(( "TxAutoFail: %lu, fail: %s\n", TxAutoFail, (fail) ? "TRUE" : "NO"));

   if ((fail == FALSE) && (TxAutoFail != 0))
   {
      if (--TxAutoFail == 0)                    // 0 after decrement
      {
         #if defined   (WIN32)
            SetErrorMode( 0);
         #elif defined (DEV32)
            DosError( FERR_ENABLEHARDERR);
         #endif
      }
   }
   else if (fail)
   {
      if (TxAutoFail++ == 0)                    // 0 before increment
      {
         #if defined   (WIN32)
            SetErrorMode( SEM_FAILCRITICALERRORS);
         #elif defined (DEV32)
            DosError( FERR_DISABLEHARDERR);
         #endif
      }
   }
   //- Note: DOS TX-handler will obey the TxAutoFail level directly ...
   VRETURN();
}                                           // end 'TxFsAutoFailCriticalErrors'
/*---------------------------------------------------------------------------*/

