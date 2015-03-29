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
// TX library anchor and init/terminate functions
// file-logging facilities
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXCON.C

#include <txlib.h>
#include <txwpriv.h>                            // private window interface

// Needs to be in a non-windowing component for the OEM versions
TXW_ANCHOR txwanchor =
{
   #if   defined (DEV32)
      PT_WINDOWABLEVIO,                         // sessiontype default (VIO/FS)
      FALSE,                                    // KBD reader thread kill
   #elif defined (DOS32)
      NULL,                                     // DPMI parameter  ptr,  512
      NULL,                                     // DPMI parameter  ptr,  512
      0,
      0,
   #endif
   FALSE,                                       // global insert-mode
   TRUE,                                        // typeahead enabled
   TXW_ARROW_STD,                               // stdandard arrow handling
   0,                                           // first auto assigned id
   0,                                           // std radix bits, all decimal
   0, 0,                                        // saved cursor position
   {0,0,79,24},                                 // default 80x25 screen
   0,                                           // no automenu header ID
   0, 0, 0,                                     // start pos and flags for drag
   TRUE,                                        // mouse enabled
   FALSE,                                       // pedantic mode
   TXHWND_NULL,                                 // no mouse capture
   TXHWND_DESKTOP,                              // default modality
   NULL,                                        // focus window
   NULL,                                        // first window in chain
   NULL,                                        // last window in chain
   0,                                           // actual window count
   NULL,                                        // SBview for quick status
   TXHWND_NULL,                                 // Entryfield marked as cmdline
   TXHWND_NULL,                                 // Canvas marked as main menu
   TRUE,                                        // show default status
   75,                                          // resize dialog, rsmaxlines
   437,                                         // codepage, std 437 compatible
   NULL,                                        // system accelerator table
   ""                                           // list match string value
};

TXW_ANCHOR *txwa = &txwanchor;                  // private anchor (HAB)


/*****************************************************************************/
// Initialize TX-libary, after parsing EXE parameters and switches
/*****************************************************************************/
void TxLibraryInit
(
   BOOL                screenOff                // IN    start with screen off
)
{
   #if defined (DEV32)
      TIB             *tib;
      PIB             *pib;
   #endif

   ENTER();

   #if defined (DEV32)
      if (DosGetInfoBlocks( &tib, &pib) == NO_ERROR)
      {
         txwa->session = pib->pib_ultype;
      }
   #endif

   if (screenOff)
   {
      TxScreenState(DEVICE_OFF);
      TxSetAnsiMode( A_OFF);
   }
   TxFsSetCritErrHandler( TRUE);
   if (TxaExeSwitchUnSet( TXA_O_MOUSE))         // don't use the mouse
   {
      txwa->useMouse = FALSE;
   }
   if (TxaExeSwitch( 'p'))                      // pedantic warnings/errors
   {
      txwa->pedantic = TRUE;
   }
   #if defined (DOS32)
      if (TxaExeSwitchSet( TXA_O_KEYB))         // keyb switch
      {
         TxSetNlsKeyboard( TxaExeSwitchStr( TXA_O_KEYB, "", "us"), "");
      }
      txwa->dpmi1 = txDpmiAlloc( 1, 512, &(txwa->dpmi1Selector));
      txwa->dpmi2 = txDpmiAlloc( 1, 512, &(txwa->dpmi2Selector));
   #endif
   TxPrintInit( TxaExeSwitch('a'),
   #if defined (UNIX)
               !TxaExeSwitchUnSet('7'));        // default 7-bit only on Unix
   #else
                TxaExeSwitch('7'));             // and full 8-bit on others
   #endif
   #if defined (USEWINDOWING)
      TxInputInitialize();                      // Init windowed KBD/MOU input
   #endif
   VRETURN ();
}                                               // end 'TxLibraryInit'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate TX-libary, after parsing EXE parameters and switches
/*****************************************************************************/
void TxLibraryExit
(
   void
)
{
   TxAppendToLogFile("", TRUE);
   TxPrintTerminate();                          // stop printf redirection
   TxFsSetCritErrHandler( FALSE);

   #if defined (USEWINDOWING)
     TxInputTerminate();                        // stop windowed KBD/MOU input
   #endif

   TxScreenTerminate();

   #if defined (DOS32)
      txDpmiFree( txwa->dpmi1Selector);
      txDpmiFree( txwa->dpmi2Selector);
   #endif
}                                               // end 'TxLibraryExit'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate TxPrint environment
/*****************************************************************************/
void TxPrintTerminate
(
   void
)
{
   //- nothing to do yet ...
}                                               // end 'TxPrintTerminate'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Initialise Ansi environment
/*****************************************************************************/
void TxPrintInit
(
   BOOL                ansiOff,                 // IN    don't use ANSI colors
   BOOL                ascii7                   // IN    Convert to 7-bit ASCII
)
{
   #if defined (DOS32)
      static BOOL      force_ansi_probe = FALSE;
   #endif

   if (ansiOff)
   {
      TxSetAnsiMode( A_OFF);                    // so do not use it
   }
   else
   {
      TxSetAnsiMode( A_ON);                     // Allow ANSI to start with

      #if   defined (DOS32)                     // but conditionally for DOS
      if ((TxaExeSwitchUnSet('w')) ||           // when non-windowed to avoid
          (force_ansi_probe))                   // on-screen garbage strings
      {                                         // if ANSI.SYS not loaded
         short         col = TxCursorCol();

         printf("%s", CNN);                     // Reset color attribute
         fflush( stdout);                       // make sure it is done!
         if (col != TxCursorCol())              // cursor has moved!
         {                                      // ANSI not working
            TxSetAnsiMode( A_OFF);              // so do not use it
            printf( "    ");            // try to undo the probe
         }
      }
      else
      {
         force_ansi_probe = TRUE;               // always probe after first
      }                                         // time (non EXE startup)
      #endif
   }
   TRACES(( "ASCII7 set 2: %s\n", (ascii7) ? "TRUE" : "FALSE"));
   TxSetAscii7Mode( ascii7);

   TxClaimKeyboard();

   TxSetBrightBgMode( TRUE);                    // no blinking, use bright BG
}                                               // end 'TxPrintInit'
/*---------------------------------------------------------------------------*/

