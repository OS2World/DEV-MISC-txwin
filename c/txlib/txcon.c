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
// Screen output using ANSI X3.64 / XTERM terminal escape definitions
// and file-logging facilities
//
// Author: J. van Wijk
//
// JvW  19-07-2017 Updated ANSI support with rxvt 88 and xterm 256 color palette
// JvW  24-01-2003 Removed thread-id from default hex-dump display
// JvW  01-02-2002 Added 7 bit ASCII filtering using switch '-7'
// JvW  10-12-2001 Added TxStrListAdd
// JvW  29-08-2001 Rename to TXCON
// JvW  17-01-2000 Added UNICODE modus to FormatMixedStr function
// JvW  08-03-1998 Added WIN32 ANSI expansion function (JvD)
// JvW  20-04-1997 Ported to WIN32
// JvW  27-01-1996 Made more generic, for LPTIO tests
// JvW  15-02-1996 Added ddl global debugging variable
// JvW  09-03-1996 Added trace macro's and implementation
// JvW  26-12-1996 Added RetBool support for 64 threads
// JvW  17-06-1995 Initial version, split off from DHPFS.C

#include <txlib.h>
#include <txwpriv.h>                            // private window interface
#include <txtpriv.h>                            // private text   interface

static  DEVICE_STATE   screen_act  = DEVICE_ON;
static  DEVICE_STATE   logfile_act = DEVICE_ON;

#if defined (USEWINDOWING)
static  char txConfirmTitle[] = " Confirmation request ";
static  char txMsgWarnTitle[] = " Notification or warning message ";
#endif

                                                // Hook control structures
        TXH_INFO   *txh_clean = NULL;           // clean TxPrint handler
        TXH_INFO   *txh_raw   = NULL;           // raw ansi TxPrint handler

static  char       *txm_buff  = NULL;           // temporary buffer for TxPrint

static  BOOL        txk_Abort = FALSE;          // TX KB-system abort notify
static  ULONG       txkThread = 0;              // TX KB-system owning thread

#define ANSI_S_TIMEOUT  10000                   // 10 seconds timeout on claim

static  BOOL  txc_ascii7 = FALSE;               // ASCII filtering off
static  char  txc_ansi   = A_ON;

//- logfile cycling and size control, first size includes closing message ...
static  ULONG      log_written = 50;            // bytes written on this file
static  ULONG      log_maxsize = 0;             // maximum size per logfile
static  ULONG      log_seq_num = 0;             // sequence number 0..n
static  ULONG      log_retain  = 0;             // log recycle retain count

//- Note that all 'diagonal' colors (uictest) where fg == bg, are changes to grey (8) or White (15)

//- Note: 256 color strings sorted in default PC order, standard 16x16 colors, original PC standard
ANSISTRING  ansi_col_16[ ANSICOLORS] =
{
   "[1;40;30m",                                // Black / very dark-Grey
   "[0;40;34m",
   "[0;40;32m",
   "[0;40;36m",
   "[0;40;31m",
   "[0;40;35m",
   "[0;40;33m",
   "[0;40;37m",
   "[1;40;30m",
   "[1;40;34m",
   "[1;40;32m",
   "[1;40;36m",
   "[1;40;31m",
   "[1;40;35m",
   "[1;40;33m",
   "[1;40;37m",

   "[0;44;30m",                                // Blue
   "[1;44;30m",                                // grey
   "[0;44;32m",
   "[0;44;36m",
   "[0;44;31m",
   "[0;44;35m",
   "[0;44;33m",
   "[0;44;37m",
   "[1;44;30m",
   "[1;44;34m",
   "[1;44;32m",
   "[1;44;36m",
   "[1;44;31m",
   "[1;44;35m",
   "[1;44;33m",
   "[1;44;37m",

   "[0;42;30m",                                // Green
   "[0;42;34m",
   "[1;42;30m",                                // grey
   "[0;42;36m",
   "[0;42;31m",
   "[0;42;35m",
   "[0;42;33m",
   "[0;42;37m",
   "[1;42;30m",
   "[1;42;34m",
   "[1;42;32m",
   "[1;42;36m",
   "[1;42;31m",
   "[1;42;35m",
   "[1;42;33m",
   "[1;42;37m",

   "[0;46;30m",                                // Cyan
   "[0;46;34m",
   "[0;46;32m",
   "[1;46;30m",                                // grey
   "[0;46;31m",
   "[0;46;35m",
   "[0;46;33m",
   "[0;46;37m",
   "[1;46;30m",
   "[1;46;34m",
   "[1;46;32m",
   "[1;46;36m",
   "[1;46;31m",
   "[1;46;35m",
   "[1;46;33m",
   "[1;46;37m",

   "[0;41;30m",                                // Red
   "[0;41;34m",
   "[0;41;32m",
   "[0;41;36m",
   "[1;41;30m",                                // grey
   "[0;41;35m",
   "[0;41;33m",
   "[0;41;37m",
   "[1;41;30m",
   "[1;41;34m",
   "[1;41;32m",
   "[1;41;36m",
   "[1;41;31m",
   "[1;41;35m",
   "[1;41;33m",
   "[1;41;37m",

   "[0;45;30m",                                // Magenta
   "[0;45;34m",
   "[0;45;32m",
   "[0;45;36m",
   "[0;45;31m",
   "[1;45;30m",                                // grey
   "[0;45;33m",
   "[0;45;37m",
   "[1;45;30m",
   "[1;45;34m",
   "[1;45;32m",
   "[1;45;36m",
   "[1;45;31m",
   "[1;45;35m",
   "[1;45;33m",
   "[1;45;37m",

   "[0;43;30m",                                // Brown / dark-Yellow
   "[0;43;34m",
   "[0;43;32m",
   "[0;43;36m",
   "[0;43;31m",
   "[0;43;35m",
   "[1;43;30m",                                // grey
   "[0;43;37m",
   "[1;43;30m",
   "[1;43;34m",
   "[1;43;32m",
   "[1;43;36m",
   "[1;43;31m",
   "[1;43;35m",
   "[1;43;33m",
   "[1;43;37m",

   "[0;47;30m",                                // White
   "[0;47;34m",
   "[0;47;32m",
   "[0;47;36m",
   "[0;47;31m",
   "[0;47;35m",
   "[0;47;33m",
   "[1;47;30m",                                // grey
   "[1;47;30m",
   "[1;47;34m",
   "[1;47;32m",
   "[1;47;36m",
   "[1;47;31m",
   "[1;47;35m",
   "[1;47;33m",
   "[1;47;37m",

   "[5;40;30m",                                // Grey
   "[5;40;34m",
   "[5;40;32m",
   "[5;40;36m",
   "[5;40;31m",
   "[5;40;35m",
   "[5;40;33m",
   "[5;40;37m",
   "[5;40;1;37m",                              // White
   "[5;40;1;34m",
   "[5;40;1;32m",
   "[5;40;1;36m",
   "[5;40;1;31m",
   "[5;40;1;35m",
   "[5;40;1;33m",
   "[5;40;1;37m",

   "[5;44;30m",                                // LBlue
   "[5;44;34m",
   "[5;44;32m",
   "[5;44;36m",
   "[5;44;31m",
   "[5;44;35m",
   "[5;44;33m",
   "[5;44;37m",
   "[5;44;1;30m",
   "[5;44;1;30m",                              // grey
   "[5;44;1;32m",
   "[5;44;1;36m",
   "[5;44;1;31m",
   "[5;44;1;35m",
   "[5;44;1;33m",
   "[5;44;1;37m",

   "[5;42;30m",                                // LGreen
   "[5;42;34m",
   "[5;42;32m",
   "[5;42;36m",
   "[5;42;31m",
   "[5;42;35m",
   "[5;42;33m",
   "[5;42;37m",
   "[5;42;1;30m",
   "[5;42;1;34m",
   "[5;42;1;30m",                              // grey
   "[5;42;1;36m",
   "[5;42;1;31m",
   "[5;42;1;35m",
   "[5;42;1;33m",
   "[5;42;1;37m",

   "[5;46;30m",                                // LCyan
   "[5;46;34m",
   "[5;46;32m",
   "[5;46;36m",
   "[5;46;31m",
   "[5;46;35m",
   "[5;46;33m",
   "[5;46;37m",
   "[5;46;1;30m",
   "[5;46;1;34m",
   "[5;46;1;32m",
   "[5;46;1;30m",                              // grey
   "[5;46;1;31m",
   "[5;46;1;35m",
   "[5;46;1;33m",
   "[5;46;1;37m",

   "[5;41;30m",                                // LRed
   "[5;41;34m",
   "[5;41;32m",
   "[5;41;36m",
   "[5;41;31m",
   "[5;41;35m",
   "[5;41;33m",
   "[5;41;37m",
   "[5;41;1;30m",
   "[5;41;1;34m",
   "[5;41;1;32m",
   "[5;41;1;36m",
   "[5;41;1;30m",                              // grey
   "[5;41;1;35m",
   "[5;41;1;33m",
   "[5;41;1;37m",

   "[5;45;30m",                                // LMagenta
   "[5;45;34m",
   "[5;45;32m",
   "[5;45;36m",
   "[5;45;31m",
   "[5;45;35m",
   "[5;45;33m",
   "[5;45;37m",
   "[5;45;1;30m",
   "[5;45;1;34m",
   "[5;45;1;32m",
   "[5;45;1;36m",
   "[5;45;1;31m",
   "[5;45;1;30m",                              // grey
   "[5;45;1;33m",
   "[5;45;1;37m",

   "[5;43;30m",                                // Yellow
   "[5;43;34m",
   "[5;43;32m",
   "[5;43;36m",
   "[5;43;31m",
   "[5;43;35m",
   "[5;43;33m",
   "[5;43;37m",
   "[5;43;1;30m",
   "[5;43;1;34m",
   "[5;43;1;32m",
   "[5;43;1;36m",
   "[5;43;1;31m",
   "[5;43;1;35m",
   "[5;43;1;30m",                              // grey
   "[5;43;1;37m",

   "[5;47;30m",                                // LWhite
   "[5;47;34m",
   "[5;47;32m",
   "[5;47;36m",
   "[5;47;31m",
   "[5;47;35m",
   "[5;47;33m",
   "[5;47;37m",
   "[5;47;1;30m",
   "[5;47;1;34m",
   "[5;47;1;32m",
   "[5;47;1;36m",
   "[5;47;1;31m",
   "[5;47;1;35m",
   "[5;47;1;33m",
   "[5;47;1;30m"                               // grey
};

//- Note: 256 color strings sorted in default PC order, 256-color palette, as used with xterm-256
ANSISTRING  ansi_col256[ ANSICOLORS] =
{
   "[48;5;16;38;5;8m",                         // Black / very dark-Grey
   "[48;5;16;38;5;4m",
   "[48;5;16;38;5;2m",
   "[48;5;16;38;5;6m",
   "[48;5;16;38;5;1m",
   "[48;5;16;38;5;5m",
   "[48;5;16;38;5;3m",
   "[48;5;16;38;5;7m",
   "[48;5;16;38;5;8m",
   "[48;5;16;38;5;12m",
   "[48;5;16;38;5;10m",
   "[48;5;16;38;5;14m",
   "[48;5;16;38;5;9m",
   "[48;5;16;38;5;13m",
   "[48;5;16;38;5;11m",
   "[48;5;16;38;5;15m",

   "[48;5;18;38;5;8m",                         // Blue, with black-on-blue made lighter!
   "[48;5;18;38;5;8m",                         // grey
   "[48;5;18;38;5;2m",
   "[48;5;18;38;5;6m",
   "[48;5;18;38;5;1m",
   "[48;5;18;38;5;5m",
   "[48;5;18;38;5;3m",
   "[48;5;18;38;5;7m",
   "[48;5;18;38;5;8m",
   "[48;5;18;38;5;12m",
   "[48;5;18;38;5;10m",
   "[48;5;18;38;5;14m",
   "[48;5;18;38;5;9m",
   "[48;5;18;38;5;13m",
   "[48;5;18;38;5;11m",
   "[48;5;18;38;5;15m",

   "[48;5;28;38;5;0m",                         // Green
   "[48;5;28;38;5;4m",
   "[48;5;28;38;5;22m",                        // darker green
   "[48;5;28;38;5;6m",
   "[48;5;28;38;5;1m",
   "[48;5;28;38;5;5m",
   "[48;5;28;38;5;3m",
   "[48;5;28;38;5;7m",
   "[48;5;28;38;5;8m",
   "[48;5;28;38;5;12m",
   "[48;5;28;38;5;10m",
   "[48;5;28;38;5;14m",
   "[48;5;28;38;5;9m",
   "[48;5;28;38;5;13m",
   "[48;5;28;38;5;11m",
   "[48;5;28;38;5;15m",

   "[48;5;37;38;5;0m",                         // Cyan
   "[48;5;37;38;5;4m",
   "[48;5;37;38;5;28m",                        // darker green
   "[48;5;37;38;5;8m",                         // grey
   "[48;5;37;38;5;1m",
   "[48;5;37;38;5;5m",
   "[48;5;37;38;5;3m",
   "[48;5;37;38;5;7m",
   "[48;5;37;38;5;8m",
   "[48;5;37;38;5;12m",
   "[48;5;37;38;5;28m",                        // fg 28 instead of 10 (darker green)
   "[48;5;37;38;5;14m",
   "[48;5;37;38;5;9m",
   "[48;5;37;38;5;13m",
   "[48;5;37;38;5;11m",
   "[48;5;37;38;5;15m",

   "[48;5;88;38;5;0m",                         // Red
   "[48;5;88;38;5;4m",
   "[48;5;88;38;5;2m",
   "[48;5;88;38;5;6m",
   "[48;5;88;38;5;8m",                         // grey
   "[48;5;88;38;5;5m",
   "[48;5;88;38;5;3m",
   "[48;5;88;38;5;7m",
   "[48;5;88;38;5;8m",
   "[48;5;88;38;5;12m",
   "[48;5;88;38;5;10m",
   "[48;5;88;38;5;14m",
   "[48;5;88;38;5;9m",
   "[48;5;88;38;5;13m",
   "[48;5;88;38;5;11m",
   "[48;5;88;38;5;15m",

   "[48;5;127;38;5;0m",                        // Magenta
   "[48;5;127;38;5;4m",
   "[48;5;127;38;5;2m",
   "[48;5;127;38;5;6m",
   "[48;5;127;38;5;1m",
   "[48;5;127;38;5;8m",                        // grey
   "[48;5;127;38;5;3m",
   "[48;5;127;38;5;7m",
   "[48;5;127;38;5;8m",
   "[48;5;127;38;5;12m",
   "[48;5;127;38;5;10m",
   "[48;5;127;38;5;14m",
   "[48;5;127;38;5;9m",
   "[48;5;127;38;5;13m",
   "[48;5;127;38;5;11m",
   "[48;5;127;38;5;15m",

   "[48;5;142;38;5;0m",                        // Brown / dark-Yellow
   "[48;5;142;38;5;4m",
   "[48;5;142;38;5;2m",
   "[48;5;142;38;5;6m",
   "[48;5;142;38;5;1m",
   "[48;5;142;38;5;5m",
   "[48;5;142;38;5;8m",                        // grey
   "[48;5;142;38;5;7m",
   "[48;5;142;38;5;8m",
   "[48;5;142;38;5;12m",
   "[48;5;142;38;5;10m",
   "[48;5;142;38;5;14m",
   "[48;5;142;38;5;9m",
   "[48;5;142;38;5;13m",
   "[48;5;142;38;5;11m",
   "[48;5;142;38;5;15m",

   "[48;5;252;38;5;0m",                        // White
   "[48;5;252;38;5;4m",
   "[48;5;252;38;5;2m",
   "[48;5;252;38;5;6m",
   "[48;5;252;38;5;1m",
   "[48;5;252;38;5;5m",
   "[48;5;252;38;5;3m",
   "[48;5;252;38;5;8m",                        // grey
   "[48;5;252;38;5;8m",
   "[48;5;252;38;5;12m",
   "[48;5;252;38;5;10m",
   "[48;5;252;38;5;14m",
   "[48;5;252;38;5;9m",
   "[48;5;252;38;5;13m",
   "[48;5;252;38;5;11m",
   "[48;5;252;38;5;15m",

   "[48;5;244;38;5;0m",                        // 244 is 'medium grey' background, 0..15 are the
   "[48;5;244;38;5;4m",                        // FG sorted in the same order as the rest (PC)
   "[48;5;244;38;5;2m",
   "[48;5;244;38;5;6m",                        // NOTE: 'new' ANSI strings need the matching
   "[48;5;244;38;5;1m",                        // decoding in TXWSBUF.C txwpParseAnsiArgs() !!!
   "[48;5;244;38;5;53m",                       // Magenta, darkened
   "[48;5;244;38;5;3m",
   "[48;5;244;38;5;7m",
   "[48;5;244;38;5;15m",                       // White
   "[48;5;244;38;5;12m",
   "[48;5;244;38;5;10m",
   "[48;5;244;38;5;14m",
   "[48;5;244;38;5;9m",
   "[48;5;244;38;5;127m",                      // LMagenta, darkened
   "[48;5;244;38;5;11m",
   "[48;5;244;38;5;15m",

   "[48;5;21;38;5;0m",                         // LBlue
   "[48;5;21;38;5;4m",
   "[48;5;21;38;5;2m",
   "[48;5;21;38;5;6m",
   "[48;5;21;38;5;1m",
   "[48;5;21;38;5;5m",
   "[48;5;21;38;5;3m",
   "[48;5;21;38;5;7m",
   "[48;5;21;38;5;8m",
   "[48;5;21;38;5;8m",                         // grey
   "[48;5;21;38;5;10m",
   "[48;5;21;38;5;14m",
   "[48;5;21;38;5;9m",
   "[48;5;21;38;5;13m",
   "[48;5;21;38;5;11m",
   "[48;5;21;38;5;15m",

   "[48;5;40;38;5;0m",                         // LGreen
   "[48;5;40;38;5;4m",
   "[48;5;40;38;5;2m",
   "[48;5;40;38;5;6m",
   "[48;5;40;38;5;1m",
   "[48;5;40;38;5;5m",
   "[48;5;40;38;5;3m",
   "[48;5;40;38;5;7m",
   "[48;5;40;38;5;8m",
   "[48;5;40;38;5;12m",
   "[48;5;40;38;5;8m",                         // grey
   "[48;5;40;38;5;14m",
   "[48;5;40;38;5;9m",
   "[48;5;40;38;5;13m",
   "[48;5;40;38;5;11m",
   "[48;5;40;38;5;15m",

   "[48;5;50;38;5;0m",                         // LCyan
   "[48;5;50;38;5;4m",
   "[48;5;50;38;5;2m",
   "[48;5;50;38;5;6m",
   "[48;5;50;38;5;1m",
   "[48;5;50;38;5;5m",
   "[48;5;50;38;5;3m",
   "[48;5;50;38;5;7m",
   "[48;5;50;38;5;8m",
   "[48;5;50;38;5;12m",
   "[48;5;50;38;5;10m",
   "[48;5;50;38;5;8m",                         // grey
   "[48;5;50;38;5;9m",
   "[48;5;50;38;5;127m",                       // Magenta instead of LMagenta
   "[48;5;50;38;5;11m",
   "[48;5;50;38;5;15m",

   "[48;5;196;38;5;0m",                        // LRed
   "[48;5;196;38;5;4m",
   "[48;5;196;38;5;2m",
   "[48;5;196;38;5;6m",
   "[48;5;196;38;5;1m",
   "[48;5;196;38;5;5m",
   "[48;5;196;38;5;3m",
   "[48;5;196;38;5;7m",
   "[48;5;196;38;5;8m",
   "[48;5;196;38;5;12m",
   "[48;5;196;38;5;10m",
   "[48;5;196;38;5;14m",
   "[48;5;196;38;5;8m",                        // grey
   "[48;5;196;38;5;93m",                       // darker magenta
   "[48;5;196;38;5;11m",
   "[48;5;196;38;5;15m",

   "[48;5;201;38;5;0m",                        // LMagenta
   "[48;5;201;38;5;4m",
   "[48;5;201;38;5;2m",
   "[48;5;201;38;5;6m",
   "[48;5;201;38;5;1m",
   "[48;5;201;38;5;5m",
   "[48;5;201;38;5;3m",
   "[48;5;201;38;5;7m",
   "[48;5;201;38;5;8m",
   "[48;5;201;38;5;12m",
   "[48;5;201;38;5;10m",
   "[48;5;201;38;5;14m",
   "[48;5;201;38;5;9m",
   "[48;5;201;38;5;8m",                        // grey
   "[48;5;201;38;5;11m",
   "[48;5;201;38;5;15m",

   "[48;5;226;38;5;0m",                        // Yellow
   "[48;5;226;38;5;4m",
   "[48;5;226;38;5;2m",
   "[48;5;226;38;5;6m",
   "[48;5;226;38;5;1m",
   "[48;5;226;38;5;5m",
   "[48;5;226;38;5;3m",
   "[48;5;226;38;5;7m",
   "[48;5;226;38;5;8m",
   "[48;5;226;38;5;12m",
   "[48;5;226;38;5;10m",
   "[48;5;226;38;5;14m",
   "[48;5;226;38;5;9m",
   "[48;5;226;38;5;13m",
   "[48;5;226;38;5;8m",                        // grey
   "[48;5;226;38;5;249m",                      // lighter grey

   "[48;5;255;38;5;0m",                        // LWhite
   "[48;5;255;38;5;4m",
   "[48;5;255;38;5;2m",
   "[48;5;255;38;5;6m",
   "[48;5;255;38;5;1m",
   "[48;5;255;38;5;5m",
   "[48;5;255;38;5;3m",
   "[48;5;255;38;5;7m",
   "[48;5;255;38;5;8m",
   "[48;5;255;38;5;12m",
   "[48;5;255;38;5;10m",
   "[48;5;255;38;5;14m",
   "[48;5;255;38;5;9m",
   "[48;5;255;38;5;13m",
   "[48;5;255;38;5;214m",                      // darker yellow
   "[48;5;255;38;5;8m",                        // grey
};


//- Note: color strings sorted in default PC order, 88-color palette, as used with rxvt
ANSISTRING  ansi_col_88[ ANSICOLORS] =
{
   "[48;5;16;38;5;8m",                         // Black / very dark-Grey
   "[48;5;16;38;5;4m",
   "[48;5;16;38;5;2m",
   "[48;5;16;38;5;6m",
   "[48;5;16;38;5;1m",
   "[48;5;16;38;5;5m",
   "[48;5;16;38;5;3m",
   "[48;5;16;38;5;7m",
   "[48;5;16;38;5;8m",
   "[48;5;16;38;5;12m",
   "[48;5;16;38;5;10m",
   "[48;5;16;38;5;14m",
   "[48;5;16;38;5;9m",
   "[48;5;16;38;5;13m",
   "[48;5;16;38;5;11m",
   "[48;5;16;38;5;15m",

   "[48;5;17;38;5;8m",                         // Blue, with black-on-blue made lighter!
   "[48;5;17;38;5;8m",                         // grey
   "[48;5;17;38;5;2m",
   "[48;5;17;38;5;6m",
   "[48;5;17;38;5;1m",
   "[48;5;17;38;5;5m",
   "[48;5;17;38;5;3m",
   "[48;5;17;38;5;7m",
   "[48;5;17;38;5;8m",
   "[48;5;17;38;5;12m",
   "[48;5;17;38;5;10m",
   "[48;5;17;38;5;14m",
   "[48;5;17;38;5;9m",
   "[48;5;17;38;5;13m",
   "[48;5;17;38;5;11m",
   "[48;5;17;38;5;15m",

   "[48;5;20;38;5;0m",                         // Green
   "[48;5;20;38;5;4m",
   "[48;5;20;38;5;8m",                         // grey
   "[48;5;20;38;5;6m",
   "[48;5;20;38;5;1m",
   "[48;5;20;38;5;5m",
   "[48;5;20;38;5;3m",
   "[48;5;20;38;5;7m",
   "[48;5;20;38;5;8m",
   "[48;5;20;38;5;12m",
   "[48;5;20;38;5;10m",
   "[48;5;20;38;5;14m",
   "[48;5;20;38;5;9m",
   "[48;5;20;38;5;13m",
   "[48;5;20;38;5;11m",
   "[48;5;20;38;5;15m",

   "[48;5;26;38;5;0m",                         // Cyan
   "[48;5;26;38;5;4m",
   "[48;5;26;38;5;20m",                        // darker green
   "[48;5;26;38;5;8m",                         // grey
   "[48;5;26;38;5;1m",
   "[48;5;26;38;5;5m",
   "[48;5;26;38;5;3m",
   "[48;5;26;38;5;7m",
   "[48;5;26;38;5;8m",
   "[48;5;26;38;5;12m",
   "[48;5;26;38;5;20m",        // fg 10 instead of 10 (darker green)
   "[48;5;26;38;5;14m",
   "[48;5;26;38;5;9m",
   "[48;5;26;38;5;13m",
   "[48;5;26;38;5;11m",
   "[48;5;26;38;5;15m",

   "[48;5;32;38;5;0m",                         // Red
   "[48;5;32;38;5;4m",
   "[48;5;32;38;5;2m",
   "[48;5;32;38;5;6m",
   "[48;5;32;38;5;8m",                         // grey
   "[48;5;32;38;5;5m",
   "[48;5;32;38;5;3m",
   "[48;5;32;38;5;7m",
   "[48;5;32;38;5;8m",
   "[48;5;32;38;5;12m",
   "[48;5;32;38;5;10m",
   "[48;5;32;38;5;14m",
   "[48;5;32;38;5;9m",
   "[48;5;32;38;5;13m",
   "[48;5;32;38;5;11m",
   "[48;5;32;38;5;15m",

   "[48;5;33;38;5;0m",                         // Magenta
   "[48;5;33;38;5;4m",
   "[48;5;33;38;5;2m",
   "[48;5;33;38;5;6m",
   "[48;5;33;38;5;1m",
   "[48;5;33;38;5;8m",                         // grey
   "[48;5;33;38;5;3m",
   "[48;5;33;38;5;7m",
   "[48;5;33;38;5;85m",                        // light grey
   "[48;5;33;38;5;12m",
   "[48;5;33;38;5;10m",
   "[48;5;33;38;5;14m",
   "[48;5;33;38;5;9m",
   "[48;5;33;38;5;13m",
   "[48;5;33;38;5;11m",
   "[48;5;33;38;5;15m",

   "[48;5;36;38;5;0m",                         // Brown / dark-Yellow
   "[48;5;36;38;5;4m",
   "[48;5;36;38;5;2m",
   "[48;5;36;38;5;6m",
   "[48;5;36;38;5;1m",
   "[48;5;36;38;5;5m",
   "[48;5;36;38;5;8m",                         // grey
   "[48;5;36;38;5;7m",
   "[48;5;36;38;5;8m",
   "[48;5;36;38;5;12m",
   "[48;5;36;38;5;10m",
   "[48;5;36;38;5;14m",
   "[48;5;36;38;5;9m",
   "[48;5;36;38;5;13m",
   "[48;5;36;38;5;11m",
   "[48;5;36;38;5;15m",

   "[48;5;86;38;5;0m",                         // White
   "[48;5;86;38;5;4m",
   "[48;5;86;38;5;2m",
   "[48;5;86;38;5;6m",
   "[48;5;86;38;5;1m",
   "[48;5;86;38;5;5m",
   "[48;5;86;38;5;3m",
   "[48;5;86;38;5;8m",                         // grey
   "[48;5;86;38;5;8m",
   "[48;5;86;38;5;12m",
   "[48;5;86;38;5;10m",
   "[48;5;86;38;5;14m",
   "[48;5;86;38;5;9m",
   "[48;5;86;38;5;13m",
   "[48;5;86;38;5;11m",
   "[48;5;86;38;5;15m",

   "[48;5;83;38;5;0m",                         // 83 is 'medium grey' background, 0..15 are the
   "[48;5;83;38;5;4m",                         // FG sorted in the same order as the rest (PC)
   "[48;5;83;38;5;2m",
   "[48;5;83;38;5;6m",                         // NOTE: 'new' ANSI strings need the matching
   "[48;5;83;38;5;1m",                         // decoding in TXWSBUF.C txwpParseAnsiArgs() !!!
   "[48;5;83;38;5;33m",                        // Magenta, darkened
   "[48;5;83;38;5;3m",
   "[48;5;83;38;5;7m",
   "[48;5;83;38;5;15m",                        // White
   "[48;5;83;38;5;8m",                         // grey
   "[48;5;83;38;5;10m",
   "[48;5;83;38;5;14m",
   "[48;5;83;38;5;9m",
   "[48;5;83;38;5;50m",                        // LMagenta, darkened
   "[48;5;83;38;5;11m",
   "[48;5;83;38;5;15m",

   "[48;5;18;38;5;0m",                         // LBlue
   "[48;5;18;38;5;80m",                        // dark grey
   "[48;5;18;38;5;2m",
   "[48;5;18;38;5;6m",
   "[48;5;18;38;5;1m",
   "[48;5;18;38;5;33m",                        // Magenta, darkened
   "[48;5;18;38;5;3m",
   "[48;5;18;38;5;7m",
   "[48;5;18;38;5;8m",
   "[48;5;18;38;5;8m",                         // grey
   "[48;5;18;38;5;10m",
   "[48;5;18;38;5;14m",
   "[48;5;18;38;5;9m",
   "[48;5;18;38;5;50m",                        // LMagenta, darkened
   "[48;5;18;38;5;11m",
   "[48;5;18;38;5;15m",

   "[48;5;28;38;5;0m",                         // LGreen
   "[48;5;28;38;5;4m",
   "[48;5;28;38;5;2m",
   "[48;5;28;38;5;6m",
   "[48;5;28;38;5;1m",
   "[48;5;28;38;5;5m",
   "[48;5;28;38;5;3m",
   "[48;5;28;38;5;85m",                        // light grey
   "[48;5;28;38;5;8m",
   "[48;5;28;38;5;12m",
   "[48;5;28;38;5;8m",                         // grey
   "[48;5;28;38;5;14m",
   "[48;5;28;38;5;9m",
   "[48;5;28;38;5;13m",
   "[48;5;28;38;5;11m",
   "[48;5;28;38;5;15m",

   "[48;5;31;38;5;0m",                         // LCyan
   "[48;5;31;38;5;4m",
   "[48;5;31;38;5;2m",
   "[48;5;31;38;5;6m",
   "[48;5;31;38;5;1m",
   "[48;5;31;38;5;5m",
   "[48;5;31;38;5;3m",
   "[48;5;31;38;5;85m",                        // light grey
   "[48;5;31;38;5;8m",
   "[48;5;31;38;5;12m",
   "[48;5;31;38;5;10m",
   "[48;5;31;38;5;8m",                         // grey
   "[48;5;31;38;5;9m",
   "[48;5;31;38;5;33m",                        // Magenta instead of LMagenta
   "[48;5;31;38;5;11m",
   "[48;5;31;38;5;15m",

   "[48;5;64;38;5;0m",                         // LRed
   "[48;5;64;38;5;4m",
   "[48;5;64;38;5;2m",
   "[48;5;64;38;5;6m",
   "[48;5;64;38;5;1m",
   "[48;5;64;38;5;5m",
   "[48;5;64;38;5;3m",
   "[48;5;64;38;5;7m",
   "[48;5;64;38;5;8m",
   "[48;5;64;38;5;12m",
   "[48;5;64;38;5;10m",
   "[48;5;64;38;5;14m",
   "[48;5;64;38;5;8m",                         // grey
   "[48;5;64;38;5;13m",
   "[48;5;64;38;5;11m",
   "[48;5;64;38;5;15m",

   "[48;5;66;38;5;0m",                         // LMagenta
   "[48;5;66;38;5;4m",
   "[48;5;66;38;5;2m",
   "[48;5;66;38;5;6m",
   "[48;5;66;38;5;1m",
   "[48;5;66;38;5;5m",
   "[48;5;66;38;5;3m",
   "[48;5;66;38;5;7m",
   "[48;5;66;38;5;8m",
   "[48;5;66;38;5;12m",
   "[48;5;66;38;5;10m",
   "[48;5;66;38;5;14m",
   "[48;5;66;38;5;9m",
   "[48;5;66;38;5;8m",                         // grey
   "[48;5;66;38;5;11m",
   "[48;5;66;38;5;15m",

   "[48;5;76;38;5;0m",                         // Yellow
   "[48;5;76;38;5;4m",
   "[48;5;76;38;5;2m",
   "[48;5;76;38;5;6m",
   "[48;5;76;38;5;1m",
   "[48;5;76;38;5;5m",
   "[48;5;76;38;5;3m",
   "[48;5;76;38;5;85m",                        // light grey
   "[48;5;76;38;5;8m",
   "[48;5;76;38;5;12m",
   "[48;5;76;38;5;10m",
   "[48;5;76;38;5;14m",
   "[48;5;76;38;5;9m",
   "[48;5;76;38;5;13m",
   "[48;5;76;38;5;8m",                         // grey
   "[48;5;76;38;5;15m",

   "[48;5;87;38;5;0m",                         // LWhite
   "[48;5;87;38;5;4m",
   "[48;5;87;38;5;2m",
   "[48;5;87;38;5;6m",
   "[48;5;87;38;5;1m",
   "[48;5;87;38;5;5m",
   "[48;5;87;38;5;3m",
   "[48;5;87;38;5;85m",                        // light grey
   "[48;5;87;38;5;8m",
   "[48;5;87;38;5;12m",
   "[48;5;87;38;5;10m",
   "[48;5;87;38;5;14m",
   "[48;5;87;38;5;9m",
   "[48;5;87;38;5;13m",
   "[48;5;87;38;5;72m",
   "[48;5;87;38;5;8m",                         // grey
};


// First 256-colors part filled in on ScreenInit(), rest stays the same
ANSISTRING  ansi[ NUMBER_ANSIS] =
{
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",

                                                // 0x100, non-palette dependant codes
   "[0m",                                      // NORMAL
   "[1m",                                      // INTENSE
   "[1A",                                      // CURSOR_UP1
   "[2A",                                      // CURSOR_UP2
   "[4A",                                      // CURSOR_UP4
   "[8A",                                      // CURSOR_UP8
   "[1B",                                      // CURSOR_DOWN1
   "[2B",                                      // CURSOR_DOWN2
   "[4B",                                      // CURSOR_DOWN4
   "[8B",                                      // CURSOR_DOWN8
   "[1C",                                      // CURSOR_RIGHT1
   "[2C",                                      // CURSOR_RIGHT2
   "[4C",                                      // CURSOR_RIGHT4
   "[8C",                                      // CURSOR_RIGHT8
   "[1D",                                      // CURSOR_LEFT1
   "[2D",                                      // CURSOR_LEFT2
   "[4D",                                      // CURSOR_LEFT4
   "[8D",                                      // CURSOR_LEFT8
   "[s",                                       // CURSOR_SAVEP
   "[u",                                       // CURSOR_RESTP
   "[K",                                       // CLEAR_TO_EOL
   "[1;70H",                                   // CURS_GO_1_70
   "[1;75H",                                   // CURS_GO_1_75
   "[1;1H",                                    // CURS_GO_1_01
   "[2;1H",                                    // CURS_GO_2_01
   "[23;1H",                                   // CURS_GO23_01
   "[24;1H"                                    // CURS_GO24_01
};


/*****************************************************************************/
// Patch ANSI escape-sequences to 1/2/8/16/88/256 color palette, map to 16x16
/*****************************************************************************/
void TxSwitchAnsiPalette
(
   int                 palette                  // IN    1, 2, 8, 16, 88 or 256
)                                               //
{
   ANSISTRING         *source = NULL;           // source palette area

   ENTER();
   TRACES(("Copy palette: %d\n", palette));

   switch (palette)
   {
      case   1:                        txwa->colors =   1; break;
      case   2:                        txwa->colors =   2; break;
      case   8: source = ansi_col_16;  txwa->colors =   8; break;
      case  88: source = ansi_col_88;  txwa->colors =  88; break;
      case 256: source = ansi_col256;  txwa->colors = 256; break;
      default : source = ansi_col_16;  txwa->colors =  16; break;
   }
   if (source != NULL)                          // copy anything ?
   {
      memcpy( ansi, source, ANSICOLORS * sizeof( ANSISTRING));
   }
   else                                         // reset to NO colors
   {
      int              i;

      for (i = 0; i < ANSICOLORS; i++)
      {
         strcpy( ansi[i], ansi[ NORMAL]);       // reset all attributes (color)
      }
      strcpy( ansi [ TXaBGnZ], ansi[ INTENSE]); // set a few as 'intense'
      strcpy( ansi [ TXaBYnZ], ansi[ INTENSE]);
      strcpy( ansi [ TXaBCnZ], ansi[ INTENSE]);
      strcpy( ansi [ TXaBMnZ], ansi[ INTENSE]);
      strcpy( ansi [ TXaBWnZ], ansi[ INTENSE]);
   }
   VRETURN ();
}                                               // end 'TxSwitchAnsiPalette'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find BG+FG color values for a (color) ANSI sequence string
/*****************************************************************************/
BOOL TxReverseLookupAnsi                        // RET   TRUE when found
(
   char               *seq,                     // IN    ANSI color sequence
   BYTE               *bg,                      // OUT   BG color 0..15
   BYTE               *fg                       // OUT   FG color 0..15
)                                               // not touched when not found
{
   BOOL                rc = FALSE;              // function return
   int                 i;

   for (i = 0; i < ANSICOLORS; i++)             // walk ANSI table of strings
   {
      if (strcmp( seq, ansi[ i]) == 0)          // match
      {
         *bg = i & 0xf0;
         *fg = i & 0x0f;
         rc = TRUE;
         break;
      }
   }
   return (rc);
}                                               // end 'TxReverseLookupAnsi'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set 7-bit ASCII mode
/*****************************************************************************/
void TxSetAscii7Mode
(
   BOOL                mode                     // IN    ASCII 7-bit mode
)
{
   TRACES(( "ASCII7 set 1: %s\n", (mode) ? "TRUE" : "FALSE"));
   txc_ascii7 = mode;
}                                               // end 'TxSetAscii7Mode'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get ASCII mode, 7-bit or full
/*****************************************************************************/
BOOL TxGetAscii7Mode                              // RET   ASCII 7-bit in use
(
   void
)
{
   return( txc_ascii7);
}                                               // end 'TxGetAscii7Mode'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set ansi string definitions active or inactive
/*****************************************************************************/
void TxSetAnsiMode
(
   char                mode                     // IN    ansi mode
)
{
   int                 i;

   txc_ansi = mode;
   for (i = 0; i < NUMBER_ANSIS; i++)
   {
      ansi[i][0] = mode;
   }
}                                               // end 'TxSetAnsiMode'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get ansi string definitions active or inactive
/*****************************************************************************/
char TxGetAnsiMode                              // RET   ansi mode
(
   void
)
{
   return( txc_ansi);
}                                               // end 'TxGetAnsiMode'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test and set TxScreenState
/*****************************************************************************/
DEVICE_STATE TxScreenState                      // RET   screen active
(
   DEVICE_STATE        action                   // IN    screen action
)
{
   if (action != DEVICE_TEST)
   {
      screen_act = action;
   }
   return (screen_act);
}                                               // end 'TxScreenState'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test and set TxLogfileState, logging when state = ON and file opened
/*****************************************************************************/
DEVICE_STATE TxLogfileState                     // RET   logfile active
(
   DEVICE_STATE        action                   // IN    screen action
)
{
   if (action != DEVICE_TEST)
   {
      logfile_act = action;
   }
   return (logfile_act);
}                                               // end 'TxLogfileState'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set logfile maximum size per file
/*****************************************************************************/
void TxSetLogMaxSize
(
   ULONG               size                     // IN    maximum size, bytes
)
{
   log_maxsize = size;
}                                               // end 'TxSetLogMaxSize'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set logfile number of files to retain on cycle / rotating trace or logfiles
/*****************************************************************************/
void TxSetLogRetain
(
   ULONG               retain                   // IN    retain count
)
{
   log_seq_num = 0;                             // current = 0 == .log
   log_retain  = retain;
}                                               // end 'TxSetLogRetain'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// printf-like print on stdout, LOG filehandle and raw/clean copy hooks
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
void TxPrint
(
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   va_list             varargs;
   ULONG               size = 0;
   BOOL                log7bit;
   BOOL                logreopen;
   FILE               *log_handle = TxQueryLogFile( &log7bit, &logreopen);
   TXH_INFO           *hinfo;                   // current hook info

   va_start(varargs, fmt);
   if (txm_buff == NULL)
   {
      txm_buff = calloc( 1, TXMAX4K);
   }
   if (txm_buff != NULL)
   {
      size = (ULONG) vsprintf( txm_buff, fmt, varargs); // expanded string in buffer

      if (txc_ascii7)                           // filter to 7-bit ASCII
      {
         TxAscii827( txm_buff, TXASCII827_TRANS);
      }

      if ((screen_act    == DEVICE_ON) &&       // screen output ON
          (txh_raw       == NULL))              // and not redirected to a RAW
      {                                         // stream (like scrollbuffer)
         #if defined (WIN32)
            txNtConsoleDisplay( txm_buff);      // ANSI expand and print
         #else
            printf( "%s", txm_buff);            // just print formatted buffer
         #endif
         fflush(stdout);
      }
      for (hinfo = txh_raw; hinfo != NULL; hinfo = hinfo->next)
      {
         if ((size < hinfo->size) && (hinfo->active) &&
             ((screen_act == DEVICE_ON) ||
              (hinfo->follow_screen_toggle == FALSE)))
         {
            strcpy( hinfo->cbuf, txm_buff);
            (hinfo->copy) (hinfo->cbuf, hinfo->user);
         }
      }
      if ((log_handle != 0) || (txh_clean != NULL))
      {
         size = TxStripAnsiCodes( txm_buff);    // update size as well!
         for (hinfo = txh_clean; hinfo != NULL; hinfo = hinfo->next)
         {
            if ((size < hinfo->size) && (hinfo->active) &&
                ((screen_act == DEVICE_ON) ||
                 (hinfo->follow_screen_toggle == FALSE)))
            {
               strcpy( hinfo->cbuf, txm_buff);
               (hinfo->copy) (hinfo->cbuf, hinfo->user);
            }
         }
         if ((log_handle != 0) && (logfile_act == DEVICE_ON))
         {
            if (!txc_ascii7 && log7bit)         // 7-bit ASCII for log only ?
            {
               TxAscii827( txm_buff, TXASCII827_TRANS);
            }

            if ((log_maxsize > size) && (log_written > (log_maxsize - size)))
            {
               TXLN    fname;

               log_written = size + 150;        // start size new logfile

               fprintf( log_handle, "\nClosing logfile at size limit\n");

               TxAppendToLogFile( NULL, FALSE); // close current log, quiet
               if ((log_retain != 0) && (log_seq_num >= (log_retain - 1))) // delete one
               {
                  TxBuildLogName( log_seq_num - (log_retain -1), log_retain, fname);
                  remove( fname);               // delete a logfile (cycle)
               }
               TxBuildLogName( (log_retain) ? ++log_seq_num : 0, log_retain, fname);
               TxAppendToLogFile( fname, FALSE); // open next logfile, quiet
               log_handle  = TxQueryLogFile( &log7bit, &logreopen);
               fprintf( log_handle, "Start logfile nr %lu : '%s'\n", log_seq_num, fname);
            }
            else
            {
               log_written += size;             // maintain total size in log
            }

            fprintf( log_handle, "%s", txm_buff);
            if (logreopen)
            {
               TxCloseReopenLogFile();          // close/reopen (force flush)
            }
            else
            {
               fflush(  log_handle);            // soft flush
            }
         }
      }
   }
   va_end(varargs);
}                                               // end 'TxPrint'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Remove ANSI control code from a text-string (in place conversion)
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
ULONG TxStripAnsiCodes                          // RET   length stripped string
(                                               //       corrected for CR/LF
   char               *text                     // INOUT ANSI text to strip
)
{
   char               *rd = text;               // read-pointer in string
   char               *wr = text;               // write pointer
   ULONG               ls = 0;                  // length stripped string

   while (*rd)
   {
      if ((*rd == TXK_ESCAPE) && (*(rd+1) == '['))
      {
         rd += 2;
         while ((isdigit(*rd)) || (*rd == ';'))
         {
            rd++;
         }
      }
      else
      {
         *(wr++) = *rd;                         // copy the character

         #if !defined(UNIX)
         if (*rd == '\n')                       // when end of line
         {
            ls++;                               // correct for CR/LF char
         }
         #endif
         ls++;                                  // count for length
      }
      rd++;
   }
   *wr = '\0';                                  // terminate stripped string
   return( ls);
}                                               // end 'TxStripAnsiCodes'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Claim TX keyboard access-functions for current thread (TxAbort etc)
/*****************************************************************************/
ULONG TxClaimKeyboard                           // RET   previous owner
(
   void
)
{
   ULONG               rc = txkThread;          // function return

   ENTER();

   txkThread = TXCURTHREAD;

   RETURN (rc);
}                                               // end 'TxClaimKeyboard'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Cancel any pending abort status (as returned by TxAbort)
/*****************************************************************************/
void TxCancelAbort
(
   void
)
{
   ENTER();

   txk_Abort = FALSE;

   VRETURN ();
}                                               // end 'TxCancelAbort'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set abort status to pending, for other threads (as returned by TxAbort)
/*****************************************************************************/
void TxSetPendingAbort
(
   void
)
{
   ENTER();

   txk_Abort = TRUE;

   VRETURN ();
}                                               // end 'TxSetPendingAbort'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return kbhit status and latest More ... reply, read away the hit
/*****************************************************************************/
BOOL TxAbort                                    // RET   TRUE when abort wanted
(
   void
)
{
   BOOL                aborting = FALSE;
   ULONG               key;

   if ((txk_Abort) ||                           // already aborting
       (txkThread != TXCURTHREAD))              // or not KB owning thread
   {
      aborting = txk_Abort;
   }
   else if (kbhit())                            // no other tests until key hit
   {                                            // to avoid excessive tracing!
      if ((!TxaExeSwitch('b')) &&
          (!TxaOption('B')))                    // never abort in batch mode
      {
         #if defined (USEWINDOWING)
            key = txwGetInputEvent( FALSE, NULL);
         #else
            key = getch();
         #endif
         switch (key)
         {
            case TXK_ESCAPE:
               if ((!TxaExeSwitch('b')) && (!TxaOption('B'))) // never abort in batch mode
               {
                  aborting  = TRUE;             // abort if <Esc>
                  txk_Abort = TRUE;             // remember abort status
               }
               break;

           #if defined (DUMP)
            case TXa_SLASH:                     // toggle trace value
               switch (TxTrLevel)
               {
                  case 0:  TxTrLevel = 1;   break;
                  case 1:  TxTrLevel = 100; break;
                  default: TxTrLevel = 0;   break;
               }
               break;
           #endif

            default:                            // route some keys to focus-win
               #if defined (USEWINDOWING)
               if (txwa->desktop != NULL)       // if windowing is active
               {
                  switch (key)                  // do not send critical keys
                  {
                     case TXk_ENTER:
                        break;

                     default:
                        txwSendMsg((TXWHANDLE) txwa->focus, TXWM_CHAR, 0, key);
                        break;
                  }
               }
               #endif                           // USEWINDOWING
               break;
         }
      }
   }
   return( aborting);
}                                               // end 'TxAbort'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display message using format string and ask confirmation, Yes/No/Cancel
/*****************************************************************************/
BOOL TxConfirm                                  // RET   Confirmed
(
   ULONG               helpid,                  // IN    helpid confirmation
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   TX1K                text;
   va_list             varargs;

   va_start(varargs, fmt);
   vsprintf( text, fmt, varargs);               // format the message

   return TxConfirmWidgets( helpid, NULL, text);
}                                               // end 'TxConfirm'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display message using format string and ask confirmation, optional widgets
/*****************************************************************************/
BOOL TxwConfirm                                 // RET   Confirmed
(
   ULONG               helpid,                  // IN    helpid confirmation
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   TX1K                text;
   va_list             varargs;

   va_start(varargs, fmt);
   vsprintf( text, fmt, varargs);               // format the message

   return TxConfirmWidgets( helpid, gwdata, text);
}                                               // end 'TxwConfirm'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display message using fixed string and ask confirmation, optional widgets
/*****************************************************************************/
BOOL TxConfirmWidgets                           // RET   Confirmed
(
   ULONG               helpid,                  // IN    helpid confirmation
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *text                     // IN    message text, fixed
)
{
   BOOL                cf = TRUE;               // return value
   int                 reply = 'n';             // default NOT confirmed
   char              **mText;                   // formatted text for display
   int                 ll;                      // real max line length
   int                 lines;                   // nr of lines
   DEVICE_STATE        sa = screen_act;

   if ((ll = TxScreenCols() -12) < 20)          // ScreenCols will be 0 if
   {                                            // output is redirected!
      ll = 20;
   }
   if ((txwa->desktop != NULL) && (!txwa->pedantic))
   {
      screen_act = DEVICE_OFF;                  // no regular text if windowed
   }                                            // and not 'pedantic mode'
   else
   {
      screen_act = DEVICE_ON;                   // force screen for confirm
   }
   mText = txString2Text( text, &ll, &lines);   // convert to multi-line
   TxPrint( "\n");                              // force empty line before
   TxShowTxt(  mText);                          // display in scrollbuffer
   txFreeText( mText);                          // free the text memory

   screen_act = sa;                             // restore screen state
                                                // to allow sbView output while
                                                // MsgBox is up (other threads!)
   #if defined (USEWINDOWING)
   if (txwa->desktop != NULL)                   // there is a desktop
   {
      char            *yn;

      TxStripAnsiCodes( text);                  // remove colors for popup
      if ((yn = strstr( text, "[Y/N]")) != NULL)
      {
         *yn = '\0';                            // remove [y/n] part
      }
      switch (txwMessageBoxWidgets( TXHWND_DESKTOP, TXHWND_DESKTOP, gwdata,
                                    text, txConfirmTitle, helpid,
                                    TXMB_YESNOCANCEL | TXMB_MOVEABLE |
                                    TXMB_HCENTER     | TXMB_VCENTER) )
      {
         case TXMBID_OK:                        // F4=OK
         case TXMBID_YES:
            reply = 'y';                        // for logging only
            break;

         case TXMBID_CANCEL:
            TxSetPendingAbort();
            TxPrint( "Abort");
            reply = '!';
         default:
            cf = FALSE;                         // not confirmed
            break;
      }
   }
   else                                         // use non-windowed interface
   #endif                                       // USEWINDOWING
   {
      BOOL             replyPending = TRUE;

      do
      {
         if (TxAbort())
         {
            reply = TXK_ESCAPE;
         }
         else
         {
            reply = getch();                    // wait for next keystroke
         }
         switch (reply)
         {
            case 'y': case 'Y':
            case 'j': case 'J':
               replyPending = FALSE;
               break;

            case TXK_ESCAPE:
               TxSetPendingAbort();
               TxPrint( "Abort");
               reply = '!';
            case 'n': case 'N':
               replyPending = FALSE;
               cf = FALSE;                      // not confirmed
               break;

            default:                            // ignore other keys
               break;
         }
      } while (replyPending);                   // while reply is undetermined
   }
   if ((txwa->desktop != NULL) && (!txwa->pedantic))
   {
      screen_act = DEVICE_OFF;                  // no regular text if windowed
   }                                            // and not 'pedantic mode'
   else
   {
      screen_act = DEVICE_ON;                   // force screen for confirm
   }
   TxPrint("%c\n", reply);                      // to logfile and non-windowed
   screen_act = sa;                             // restore screen state

   return (cf);
}                                               // end 'TxConfirmWidgets'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display a message or warning using format string and wait for OK (or ESC)
/*****************************************************************************/
BOOL TxMessage                                  // RET   OK, not escaped
(
   BOOL                acknowledge,             // IN    wait for acknowledge
   ULONG               helpid,                  // IN    helpid confirmation
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   TX1K                text;
   va_list             varargs;

   va_start(varargs, fmt);
   vsprintf( text, fmt, varargs);               // format the message

   return TxMessageWidgets( acknowledge, helpid, NULL, text);
}                                               // end 'TxMessage'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display a message or warning using format string and wait for OK (or ESC)
/*****************************************************************************/
BOOL TxwMessage                                 // RET   OK, not escaped
(
   BOOL                acknowledge,             // IN    wait for acknowledge
   ULONG               helpid,                  // IN    helpid confirmation
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   TX1K                text;
   va_list             varargs;

   va_start(varargs, fmt);
   vsprintf( text, fmt, varargs);               // format the message

   return TxMessageWidgets( acknowledge, helpid, gwdata, text);
}                                               // end 'TxwMessage'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display message using fixed string and wait for [OK], optional widgets
/*****************************************************************************/
BOOL TxMessageWidgets                           // RET   OK, not escaped
(
   BOOL                acknowledge,             // IN    wait for acknowledge
   ULONG               helpid,                  // IN    helpid confirmation
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *text                     // IN    message text, fixed
)
{
   BOOL                rc = TRUE;               // return value
   char              **mText;                   // formatted text for display
   int                 ll;                      // real max line length
   int                 lines;                   // nr of lines
   DEVICE_STATE        sa = screen_act;

   if ((ll = TxScreenCols() -4) < 20)           // ScreenCols will be 0 if
   {                                            // output is redirected!
      ll = 20;
   }
   if ((txwa->desktop != NULL) && (acknowledge) && (!txwa->pedantic))
   {
      screen_act = DEVICE_OFF;                  // no regular text if windowed
   }                                            // and not 'pedantic mode'
   else if (acknowledge)
   {
      screen_act = DEVICE_ON;                   // force screen on for confirm
   }

   mText = txString2Text( text, &ll, &lines);   // convert to multi-line
   TxPrint( "\n");                              // force empty line before
   TxShowTxt(  mText);                          // display in scrollbuffer
   txFreeText( mText);                          // free the text memory
   TxPrint("\n");

   screen_act = sa;                             // restore screen state
                                                // to allow sbView output while
                                                // MsgBox is up (other threads!)
   if (acknowledge)
   {
      #if defined (USEWINDOWING)
      if (txwa->desktop != NULL)                // there is a desktop
      {
         TxStripAnsiCodes( text);               // remove colors for popup
         if (txwMessageBoxWidgets( TXHWND_DESKTOP, TXHWND_DESKTOP, gwdata,
                            text, txMsgWarnTitle, helpid,
                            TXMB_OK      | TXMB_MOVEABLE |
                            TXMB_VCENTER | TXMB_HCENTER) != TXMBID_OK)
         {
            rc = FALSE;
         }
         TxCancelAbort();                       // avoid aborting on Escape ...
      }
      else                                      // use non-windowed interface
      #endif
      {
         TxPrint( "Press [Space-bar] to continue ...\n");
         if (getch() == TXK_ESCAPE)             // wait for next keystroke
         {
            rc = FALSE;
         }
      }
   }
   return (rc);
}                                               // end 'TxMessage'
/*---------------------------------------------------------------------------*/

