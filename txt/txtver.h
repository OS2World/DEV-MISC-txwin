//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2014 Fsys Software and Jan van Wijk
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
// TXTest version and name definitions
//
// Author: J. van Wijk
//
#ifndef    TXTVER_H
   #define TXTVER_H

#define TXT_INTROD     20030707L  // introduction date
#define TXT_EXPIRE           33L  // 33 days until expiration for unregistred
#define TXT_COMPILE    __DATE__   // compile date, as base for _EXPIRE

#define TXT_V   "5.00 13-06-2014" // Minor update for TXLib 2.0
//efine TXT_V   "4.00 01-06-2007" // Added MAC OS X (Darwin) naming and fixes
//efine TXT_V   "3.09 20-06-2006" // Completed Hex-Editor test dialog
//efine TXT_V   "3.08 29-11-2005" // Removed REG stuff for TxWin 1.02 (Netlabs)
//efine TXT_V   "3.07 29-09-2005" // Minor updates for Open TxWindows 1.0
//efine TXT_V   "3.06 24-06-2005" // Fixed getch() hang for OS2, use kbhit()
//efine TXT_V   "3.05 14-06-2005" // Updated for mouse tests
//efine TXT_V   "3.04 24-01-2005" // Updated to test TXWIDGETS and dialog
//efine TXT_V   "3.03 22-04-2004" // Updated to use new window colorschemes
//efine TXT_V   "3.02 11-04-2004" // Working OpenFile/SaveAs dialogs on Linux
//efine TXT_V   "3.01 24-03-2004" // Added CBOXES, CHARSET and COLORS to menu
//efine TXT_V   "3.00 12-03-2004" // Added Linux project naming
//efine TXT_V   "2.03 23-08-2003" // Added accelerator keys and menu-help
//efine TXT_V   "2.02 17-08-2003" // Added mainmenu with MenuBar
//efine TXT_V   "2.01 20-07-2003" // Added first list test stuff
//efine TXT_V   "2.00 08-07-2003" // Initial TXTest specific version

#if   defined (WIN32)
   #if defined (__WATCOMC__)
      #define TXT_N "TXTst wNT"
      #define TXT_X "TXTWIN"
   #else
      #define TXT_N "TXTst-wNT"
      #define TXT_X "TXTNT"
   #endif
#elif defined (DOS32)
   #define TXT_N "TXTst Dos"
   #define TXT_X "TXTDOS"
#elif defined (LINUX)
   #define TXT_N "TXT Linux"
   #define TXT_X "TXTLIN"
#elif defined (DARWIN)
   #define TXT_N "TxtDarwin"
   #define TXT_X "TXTMAC"
#else
   #if defined (__WATCOMC__)
      #define TXT_N "TXTst OS2"
      #define TXT_X "TXTOS2"
   #else
      #define TXT_N "TXTst-OS2"
      #define TXT_X "TXT"
   #endif
#endif
#define TXT_C "(c) 2000-2014: Jan van Wijk"

#endif
