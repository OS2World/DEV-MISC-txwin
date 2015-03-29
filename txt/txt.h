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
// TXT navigation and general function interface
//
// Author: J. van Wijk
//
// JvW  08-07-2003 Initial version, derived from DFS.H
//
#ifndef    TXT_H
   #define TXT_H

#define TXT_CMD_FAILED    ((USHORT) 901)        // Generic cmd failure code
#define TXT_ALLOC_ERROR   ((USHORT) 906)        // memory allocation error
#define TXT_PENDING       ((USHORT) 907)        // function pending
#define TXT_QUIT          ((USHORT) 909)        // quit TXT interactive


typedef struct txtinf                           // information anchor block
{
   BOOL                batch;                   // batch-mode active
   BOOL                dialogs;                 // dialogs will be used
   ULONG               verbosity;               // output verbosity
   int                 eStrategy;               // error strategy
   ULONG               retc;                    // overall return-code
   USHORT              sbWidth;                 // visible scroll-buffer width
   USHORT              sbLength;                // visible scroll-buffer length
   BOOL                autoquit;                // auto quit after fdisk/setb
   BOOL                regconfirm;              // registration conf required
   BOOL                nowindow;                // running in classic mode
   TXCELL             *sbbuf;                   // actual scroll buffer
   ULONG               sbsize;                  // actual scrollbuf size
   ULONG               sblwidth;                // actual sb linelength
   TXWHANDLE           sbwindow;                // scroll-buffer window
   TXWHANDLE           menuOwner;               // menu handling window
   BOOL                automenu;                // automatic menu activation
   BOOL                autodrop;                // automatic pulldown drop
   ULONG               menuopen;                // default drop-down menu
   ULONG               worklevel;               // when 0, activate menu
   TXSELIST           *slSchemes;               // selection list, Color schemes
   #if defined (DOS32)
      BOOL             win9x;                   // Windows-9x DosBox detected
   #endif
} TXTINF;                                       // end of struct "txtinf"

extern  TXTINF     *txta;                       // TXT anchor block

extern  char       *switchhelp[];
extern  char       *cmdhelptxt[];

#define TXT_PROFILE     "profile.txt"

// scroll to end (output), cancel-abort and execute a new command
#define txtExecEnd(c) txwSendMsg( hwnd, TXWM_CHAR, 0, TXc_END),              \
                      TxCancelAbort(),                                       \
                      txtMultiCommand((c), 0, TRUE, TRUE, FALSE)

// cancel-abort and execute a new command
#define txtExecCmd(c) TxCancelAbort(),                                       \
                      txtMultiCommand((c), 0, TRUE, TRUE, FALSE)


// cancel-abort and execute a new command, NO echo or prompting
#define txtExecSilent(c) TxCancelAbort(),                                    \
                         txtMultiCommand((c), 0, FALSE, FALSE, TRUE)

//- WARNING: Never execute two txtExec... from the menu, that will
//-          cause the menu to restart after the 1st, causing hangs
//-          Use a real "multiple command" in one go instead.

#define txtBEGINWORK() {txta->worklevel++;}
#define txtENDWORK()   {                                                     \
                          if (txta->worklevel)                               \
                          {                                                  \
                             if ((--(txta->worklevel) == 0) &&               \
                                    (txta->menuOwner  != 0)  )               \
                             {                                               \
                                txwPostMsg( txta->menuOwner,                 \
                                            TXWM_COMMAND, TXTM_DEFAULT, 0);  \
                             }                                               \
                          }                                                  \
                       }


// Print TXTst logo+status, do startup checks, run startup commands + profile
ULONG txtStartupLogo                            // RET   Checks and firstcmd RC
(
   char               *firstcmd                 // IN    initial command
);


// Execute multiple txt-commands separated by # characters
ULONG txtMultiCommand
(
   char               *cmdstring,               // IN    multiple command
   ULONG               lnr,                     // IN    linenumber or 0
   BOOL                echo,                    // IN    Echo  before execute
   BOOL                prompt,                  // IN    prompt after execute
   BOOL                quiet                    // IN    screen-off during cmd
);

#endif
