//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2017 Fsys Software and Jan van Wijk
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
// Questions on TxWin licensing can be directed to: jvw@fsys.nl
//
// ==========================================================================
//
// TXTest windowed user interface
//
// Author: J. van Wijk
//
// 1.00 08-07-2003    Initial version
//
#ifndef    TXTWIN
   #define TXTWIN

#define TXTH_GENERIC     0
#define TXTH_CONFIRM   5000

//- enable or mark menu-items by ID
//- enable or mark menu-items by ID
#define txtMiEnable(i,c,r) txwMiEnable(txtGetMainMenu(),(i),(c),(r))
#define txtMiMarked(i,c)   txwMiMarked(txtGetMainMenu(),(i),(c),NULL)


//- menu (equal to help) and accelerator codes
#define TXTH_MENUS     3000                     // menu help base
#define TXTM_BAR       3010
#define TXTM_DEFAULT   3030
#define TXTM_AUTOMENU  3040

#define TXTM_FILE      3100
#define TXTC_OPEN      3110
#define TXTC_SAVE      3120
#define TXTC_RUNS      3130
#define TXTC_EXIT      3140

#define TXTM_TEST      3200
#define TXTC_DIALOG    3210
#define TXTC_MSGBOX    3220
#define TXTC_PROMPT    3230
#define TXTC_HEXED     3235
#define TXTC_LIST      3240
#define TXTC_DISABL    3250
#define TXTC_WIDGET    3260
#define TXTC_PRWIDG    3261
#define TXTC_SUBMEN    3280

#define TXTM_SHOW      3300
#define TXTC_VOLUMES   3310
#define TXTC_DISPLAY   3320
#define TXTC_DIRALL    3330
#define TXTC_DIRFILE   3340
#define TXTC_DIRSUBS   3350
#define TXTC_DIRTREE   3360
#define TXTC_COLORR    3394
#define TXTC_UICTEST   3396

#define TXTM_SETT      3400
#define TXTC_EXPERT    3402
#define TXTC_SCHEME    3405
#define TXTC_INVSCR    3410
#define TXTC_BRTSCR    3420
#define TXTC_B2BSCR    3430
#define TXTC_ASCII7    3440
#define TXTC_COLTXT    3450
#define TXTC_AUTOMB    3460
#define TXTC_AUTODR    3470
#define TXTC_LOGAUTO   3480

#define TXTM_HELP      3900
#define TXTC_UIHELP    3930
#define TXTC_ABOUT     3940
#define TXTC_PREGREP   3960
#define TXTC_H_KEYBD   3961
#define TXTC_H_MOUSE   3962
#define TXTC_H_CLIPB   3963
#define TXTC_H_MENUS   3964
#define TXTC_HSCRIPT   3970
#define TXTC_HS_RUNU   3972
#define TXTC_HSYNTAX   3973
#define TXTC_HS_VARS   3975
#define TXTC_CMDLINE   3979
#define TXTC_OPTHELP   3980
#define TXTC_CMDHELP   3981
#define TXTC_SW_HELP   3984
#define TXTC_TXCHELP   3993
#define TXTC_TXSHELP   3994
#define TXTC_H_SECTS   3997
#define TXTC_H_GREP    3998

//- allow enough space here for all color schemes!
#define TXTB_SCHEME    4480



// Start and maintain TXTest interactive text-based windowed user-interface
ULONG txtWindowed
(
   char               *initial                  // IN    initial TXTest cmd
);

// Return pointer to current main-menu (can be BASIC or EXPERT or ...)
TXS_MENUBAR *txtGetMainMenu
(
   void
);

// Check if dialog is wanted and possible, message otherwise
BOOL txtDialogAppropriate
(
   void
);


// Setup test dialog
ULONG txtTestDialog                             // RET   result
(
   void
);

// Simple test dialog with a selection-list (LISTBOX) client control
ULONG txtListBoxDialog                          // RET   result
(
   void
);

// Present LOG/TRACE options dialog and execute resulting command
ULONG txtLogDialog
(
   char               *logname,                 // IN    default name or NULL
   ULONG               helpid,                  // IN    specific help-id
   BOOL                reopen,                  // IN    reopen logfile
   char               *message                  // IN    extra message or NULL
);

// Present Run-script file-dialog with options and execute resulting command
ULONG txtRunScriptDialog
(
   char               *firstParam,              // IN    path/scriptname, or empty
   char               *scriptInfo               // OUT   scriptname + parameters
);

#endif
