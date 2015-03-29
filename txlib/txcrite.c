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


#if defined (DOS32)                             // DOS specific local stuff

#define DOS_CRITERR_INT   0x24                  // DOS critical error interrupt


typedef void (_interrupt _far * TXFSINT)        // DOS std interrupt handler
(
   void
);


typedef int      (far * TXFS_CRITHAND)          // crit-err handler
(
   unsigned            deverr,                  // IN    device code
   unsigned            doserr,                  // IN    dos error code
   unsigned     far   *hdr                      // IN    device header
);


// Handle critical errors using a TXWIN MsgBox popup or automatic-FAIL
int TxCriticalErrorHandler
(
   unsigned            deverr,                  // IN    device code
   unsigned            doserr,                  // IN    dos error code
   unsigned     far   *hdr                      // IN    device header
);

static TXFSINT         TxOldHandler = NULL;     // old handler, crit errors



/*****************************************************************************/
// Handle critical errors by presenting an Abort, Retry, Ignore MessageBox
/*****************************************************************************/
int TxCriticalErrorHandler                      // Tx critical error handler
(
   unsigned            deverr,                  // IN    device code
   unsigned            doserr,                  // IN    dos error code
   unsigned     far   *hdr                      // IN    device header
)
{
   #if defined (USEWINDOWING)
   if (    (TxAutoFail != 0)                    // mode set to AutoFail
      #if defined (DUMP)
        || (TxTrLevel  != 0)                    // or tracing is active
      #endif
      )
   {
      _hardresume( (int) _HARDERR_FAIL);
   }
   else
   {
      TXTM          text;

      if (deverr & 0x8000)                      // error on a device ?
      {
         sprintf( text, "Error on device %8.8s - ", ((char *) hdr) + 11);
      }
      else                                      // error on a drive
      {
         sprintf( text, "Error %sing Drive %c: - ",
                         (deverr & 0x100) ? "writ" : "read",
                         (char) (deverr & 0xff) + 'A');
      }
      switch (doserr)
      {
         case  0: strcat( text, "Disk is write protected" ); break;
         case  1: strcat( text, "Unknown drive or unit"   ); break;
         case  2: strcat( text, "Drive is not ready"      ); break;
         case  3: strcat( text, "Unknown device command"  ); break;
         case  4: strcat( text, "CRC error"               ); break;
         case  5: strcat( text, "Bad request format"      ); break;
         case  6: strcat( text, "Seek error"              ); break;
         case  7: strcat( text, "Media not recognized"    ); break;
         case  8: strcat( text, "Sector not found"        ); break;
         case  9: strcat( text, "Printer out of paper"    ); break;
         case 10: strcat( text, "General write fault"     ); break;
         case 11: strcat( text, "General read fault"      ); break;
         default: strcat( text, "General failure"         ); break;
      }

      //- Note: handler is instable on FreeDOS (and perhaps others)
      //-       probably because it uses too much stack ...

      switch (txwMessageBox( TXHWND_DESKTOP, TXHWND_DESKTOP,
                             text, "DOS Critical error",
                             TXWD_RETRYFAIL,
                             TXMB_RETRYFAIL | TXMB_HELP    |
                             TXMB_MOVEABLE  | TXMB_HCENTER | TXMB_VCENTER) )
      {
         case TXMBID_RETRY:
            _hardresume( (int) _HARDERR_RETRY);
            break;

         case TXMBID_FAIL:                      // Fail selected,
         default:                               // or <F4> or <Esc>
            _hardresume( (int) _HARDERR_FAIL);
            break;
      }
   }
   #else
      _hardresume( (int) _HARDERR_FAIL);
   #endif
   return 0;
}                                               // end 'TxCriticalErrorHandler'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get number of physical diskette drives from BIOS
/*****************************************************************************/
USHORT TxPhysDisketteDrives                     // RET   nr of diskette drives
(
   void
)
{
   USHORT              rc = 0;                  // function return
   USHORT              eq;                      // equipment list

   ENTER();

   eq = TxxBiosWord(TXX_EQUIPMENT);             // get the equipment BIOS word
   if (eq & 0x0001)                             // some drives present
   {
      rc = ((eq & 0x00c0) > 6) +1;
   }
   RETURN (rc);
}                                               // end 'TxPhysDisketteDrives'
/*---------------------------------------------------------------------------*/
#endif


/*****************************************************************************/
// Set critical error handling to use an interactive MessageBox (for DOS only)
/*****************************************************************************/
void TxFsSetCritErrHandler
(
   BOOL                enable                   // IN    MsgBox on CritErr
)
{
   ENTER();
   #if   defined (WIN32)
      //- Note: Windows provides its own popups
   #elif defined (DOS32)
      if (enable)
      {
         if (TxOldHandler == NULL)              // not set yet ?
         {
            TxOldHandler = (TXFSINT) _dos_getvect( DOS_CRITERR_INT);
            _harderr((TXFS_CRITHAND) TxCriticalErrorHandler); // install our own handler
         }                                      // using C-runtime stubs
      }
      else
      {
         if (TxOldHandler != NULL)            // if set, re-install old handler
         {
            _dos_setvect( DOS_CRITERR_INT, TxOldHandler);
            TxOldHandler = NULL;
         }
      }
      TRACES(("Old handler:%8.8lx\n", TxOldHandler));
   #elif defined (UNIX)
      //- to be refined
   #else
      //- Note: OS/2 provides its own popups in PM
      //-       Without PM, to be refined

   #endif
   VRETURN();
}                                               // end 'TxFsSetCritErrHandler'
/*---------------------------------------------------------------------------*/

