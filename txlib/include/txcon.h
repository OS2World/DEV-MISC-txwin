#ifndef TXCON_H
#define TXCON_H
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
//
// file-logging facilities
// Screen output using ANSI X3.64 terminal escape definitions and
//
// Author: J. van Wijk
//
// JvW  05-06-2003 Rename to TXCON
// JvW  29-08-2001 Rename to TPCON
// JvW  28-07-1999 Removed ANSI_DEF_ONLY / A_DYNAMIC stuff for simplicity
// JvW  07-05-1996 Added Mutex semaphore to serialize trace output
// JvW  09-03-1996 Added trace macro's and implementation
// JvW  15-02-1996 Added ddl global debugging variable
// JvW  17-05-1996 Added TxPrint CLEAN and RAW callback hooks
// JvW  25-02-1996 Fixed "," bug in ANSI_DEF_ONLY part
// JvW  26-12-1996 Added BRETURN and EOLN / NOLN macro's
// JvW  17-06-1995 Initial version, split off from DHPFS.C


#define TXMAX4K            4096                 // max page-size buffer
#define TXMAX2K            2048                 // max huge  buffer length
#define TXMAX1K            1024                 // max large buffer length
#define TXMAXLN             320                 // max line/path length
#define TXMAXTM              84                 // max huge-word length
#define TXMAXTT              44                 // max textbuf length
#define TXMAXTS              20                 // max small text length

#define TXK_ENTER          0x0d                 // Enter  key value
#define TXK_ESCAPE         0x1b                 // Escape key value
#define TXK_SPACE          0x20                 // Space  key value

#define TXM_QUERY_ONLY     0xffff               // Query More value
#define TXM_SCREEN_SIZE    0xfffe               // More value to default
#define TXM_UNLIMITTED     0x0                  // More value 0 (no limit)

typedef char   TX4K[TXMAX4K];                   // page buffers
typedef char   TX2K[TXMAX2K];                   // large buffers
typedef char   TX1K[TXMAX1K];                   // large buffers
typedef char   TXLN[TXMAXLN];                   // line buffers
typedef char   TXTM[TXMAXTM];                   // word buffers
typedef char   TXTT[TXMAXTT];                   // text buffers
typedef char   TXTS[TXMAXTS];                   // small buffers


#if !defined (_MAX_PATH)
   #define _MAX_PATH TXMAXLN
#endif


typedef enum device_state
{
   DEVICE_ON          = 0,
   DEVICE_OFF         = 1,
   DEVICE_TEST        = 2
} DEVICE_STATE;                                 // end of enum "device_state"

// void void function pointer
typedef void (* TXVVFUNCTION)
(
   void
);

typedef enum txh_type
{
   TXH_T_CLEAN,                                 // No ANSI escape sequences
   TXH_T_RAW,                                   // include all ANSI stuff
} TXH_TYPE;                                     // end of enum "txh_type"


typedef enum txh_operation
{
   TXH_REGISTER,                                // Register a handler
   TXH_DEREGISTER,                              // Deregister a handler
} TXH_OPERATION;                                // end of enum "txh_operation"


typedef void (* TXH_CALLBACK)
(
   char               *text,                    // IN    TxPrint text to copy
   void               *data                     // IN    user data pointer
);


typedef struct txh_info                         // ansi TxPrint copy info
{
   char               *cbuf;                    // ansi copy buffer
   ULONG               size;                    // size of copy buffer
   TXH_CALLBACK        copy;                    // handler for ansi data
   void               *user;                    // user data pointer
   BOOL                active;                  // handler currently active
   BOOL                follow_screen_toggle;    // follow screen on/off
   struct txh_info    *next;                    // ptr to next handler in chain
   struct txh_info    *prev;                    // ptr to prev handler in chain
} TXH_INFO;                                     // end of struct "txh_info"


#define A_ON              ''                   // on  (escape strings)
#define A_OFF             '\0'                  // off (empty strings)

//- Single FG/BG color values and macros
#define CcZ                 0                   // black         ANSI 0
#define CcB                 1                   // blue          ANSI 4
#define CcG                 2                   // green         ANSI 2
#define CcC                 3                   // cyan          ANSI 6
#define CcR                 4                   // red           ANSI 1
#define CcM                 5                   // magenta       ANSI 5
#define CcY                 6                   // yellow        ANSI 3
#define CcW                 7                   // white         ANSI 7
#define CcI                 8                   // intense
#define Ccmax              15                   // maximum color value
#define Ccfg(x)           ((x) & 0x0f)          // foreground
#define Ccbg(x)           ((x) >> 4)            // background
#define Ccol(f,b)        (((b) << 4) + f)       // Combine FG on BG

//- ANSI color values for NORMAL backgrounds
#define TXaNZnZ           0x00
#define TXaNBnZ           0x01
#define TXaNGnZ           0x02
#define TXaNCnZ           0x03
#define TXaNRnZ           0x04
#define TXaNMnZ           0x05
#define TXaNYnZ           0x06
#define TXaNWnZ           0x07
#define TXaBZnZ           0x08
#define TXaBBnZ           0x09
#define TXaBGnZ           0x0a
#define TXaBCnZ           0x0b
#define TXaBRnZ           0x0c
#define TXaBMnZ           0x0d
#define TXaBYnZ           0x0e
#define TXaBWnZ           0x0f
#define TXaNZnB           0x10
#define TXaNBnB           0x11
#define TXaNGnB           0x12
#define TXaNCnB           0x13
#define TXaNRnB           0x14
#define TXaNMnB           0x15
#define TXaNYnB           0x16
#define TXaNWnB           0x17
#define TXaBZnB           0x18
#define TXaBBnB           0x19
#define TXaBGnB           0x1a
#define TXaBCnB           0x1b
#define TXaBRnB           0x1c
#define TXaBMnB           0x1d
#define TXaBYnB           0x1e
#define TXaBWnB           0x1f
#define TXaNZnG           0x20
#define TXaNBnG           0x21
#define TXaNGnG           0x22
#define TXaNCnG           0x23
#define TXaNRnG           0x24
#define TXaNMnG           0x25
#define TXaNYnG           0x26
#define TXaNWnG           0x27
#define TXaBZnG           0x28
#define TXaBBnG           0x29
#define TXaBGnG           0x2a
#define TXaBCnG           0x2b
#define TXaBRnG           0x2c
#define TXaBMnG           0x2d
#define TXaBYnG           0x2e
#define TXaBWnG           0x2f
#define TXaNZnC           0x30
#define TXaNBnC           0x31
#define TXaNGnC           0x32
#define TXaNCnC           0x33
#define TXaNRnC           0x34
#define TXaNMnC           0x35
#define TXaNYnC           0x36
#define TXaNWnC           0x37
#define TXaBZnC           0x38
#define TXaBBnC           0x39
#define TXaBGnC           0x3a
#define TXaBCnC           0x3b
#define TXaBRnC           0x3c
#define TXaBMnC           0x3d
#define TXaBYnC           0x3e
#define TXaBWnC           0x3f
#define TXaNZnR           0x40
#define TXaNBnR           0x41
#define TXaNGnR           0x42
#define TXaNCnR           0x43
#define TXaNRnR           0x44
#define TXaNMnR           0x45
#define TXaNYnR           0x46
#define TXaNWnR           0x47
#define TXaBZnR           0x48
#define TXaBBnR           0x49
#define TXaBGnR           0x4a
#define TXaBCnR           0x4b
#define TXaBRnR           0x4c
#define TXaBMnR           0x4d
#define TXaBYnR           0x4e
#define TXaBWnR           0x4f
#define TXaNZnM           0x50
#define TXaNBnM           0x51
#define TXaNGnM           0x52
#define TXaNCnM           0x53
#define TXaNRnM           0x54
#define TXaNMnM           0x55
#define TXaNYnM           0x56
#define TXaNWnM           0x57
#define TXaBZnM           0x58
#define TXaBBnM           0x59
#define TXaBGnM           0x5a
#define TXaBCnM           0x5b
#define TXaBRnM           0x5c
#define TXaBMnM           0x5d
#define TXaBYnM           0x5e
#define TXaBWnM           0x5f
#define TXaNZnY           0x60
#define TXaNBnY           0x61
#define TXaNGnY           0x62
#define TXaNCnY           0x63
#define TXaNRnY           0x64
#define TXaNMnY           0x65
#define TXaNYnY           0x66
#define TXaNWnY           0x67
#define TXaBZnY           0x68
#define TXaBBnY           0x69
#define TXaBGnY           0x6a
#define TXaBCnY           0x6b
#define TXaBRnY           0x6c
#define TXaBMnY           0x6d
#define TXaBYnY           0x6e
#define TXaBWnY           0x6f
#define TXaNZnW           0x70
#define TXaNBnW           0x71
#define TXaNGnW           0x72
#define TXaNCnW           0x73
#define TXaNRnW           0x74
#define TXaNMnW           0x75
#define TXaNYnW           0x76
#define TXaNWnW           0x77
#define TXaBZnW           0x78
#define TXaBBnW           0x79
#define TXaBGnW           0x7a
#define TXaBCnW           0x7b
#define TXaBRnW           0x7c
#define TXaBMnW           0x7d
#define TXaBYnW           0x7e
#define TXaBWnW           0x7f

//- ANSI color values for BRIGHT backgrounds
#define TXaNZbZ           0x80
#define TXaNBbZ           0x81
#define TXaNGbZ           0x82
#define TXaNCbZ           0x83
#define TXaNRbZ           0x84
#define TXaNMbZ           0x85
#define TXaNYbZ           0x86
#define TXaNWbZ           0x87
#define TXaBZbZ           0x88
#define TXaBBbZ           0x89
#define TXaBGbZ           0x8a
#define TXaBCbZ           0x8b
#define TXaBRbZ           0x8c
#define TXaBMbZ           0x8d
#define TXaBYbZ           0x8e
#define TXaBWbZ           0x8f
#define TXaNZbB           0x90
#define TXaNBbB           0x91
#define TXaNGbB           0x92
#define TXaNCbB           0x93
#define TXaNRbB           0x94
#define TXaNMbB           0x95
#define TXaNYbB           0x96
#define TXaNWbB           0x97
#define TXaBZbB           0x98
#define TXaBBbB           0x99
#define TXaBGbB           0x9a
#define TXaBCbB           0x9b
#define TXaBRbB           0x9c
#define TXaBMbB           0x9d
#define TXaBYbB           0x9e
#define TXaBWbB           0x9f
#define TXaNZbG           0xa0
#define TXaNBbG           0xa1
#define TXaNGbG           0xa2
#define TXaNCbG           0xa3
#define TXaNRbG           0xa4
#define TXaNMbG           0xa5
#define TXaNYbG           0xa6
#define TXaNWbG           0xa7
#define TXaBZbG           0xa8
#define TXaBBbG           0xa9
#define TXaBGbG           0xaa
#define TXaBCbG           0xab
#define TXaBRbG           0xac
#define TXaBMbG           0xad
#define TXaBYbG           0xae
#define TXaBWbG           0xaf
#define TXaNZbC           0xb0
#define TXaNBbC           0xb1
#define TXaNGbC           0xb2
#define TXaNCbC           0xb3
#define TXaNRbC           0xb4
#define TXaNMbC           0xb5
#define TXaNYbC           0xb6
#define TXaNWbC           0xb7
#define TXaBZbC           0xb8
#define TXaBBbC           0xb9
#define TXaBGbC           0xba
#define TXaBCbC           0xbb
#define TXaBRbC           0xbc
#define TXaBMbC           0xbd
#define TXaBYbC           0xbe
#define TXaBWbC           0xbf
#define TXaNZbR           0xc0
#define TXaNBbR           0xc1
#define TXaNGbR           0xc2
#define TXaNCbR           0xc3
#define TXaNRbR           0xc4
#define TXaNMbR           0xc5
#define TXaNYbR           0xc6
#define TXaNWbR           0xc7
#define TXaBZbR           0xc8
#define TXaBBbR           0xc9
#define TXaBGbR           0xca
#define TXaBCbR           0xcb
#define TXaBRbR           0xcc
#define TXaBMbR           0xcd
#define TXaBYbR           0xce
#define TXaBWbR           0xcf
#define TXaNZbM           0xd0
#define TXaNBbM           0xd1
#define TXaNGbM           0xd2
#define TXaNCbM           0xd3
#define TXaNRbM           0xd4
#define TXaNMbM           0xd5
#define TXaNYbM           0xd6
#define TXaNWbM           0xd7
#define TXaBZbM           0xd8
#define TXaBBbM           0xd9
#define TXaBGbM           0xda
#define TXaBCbM           0xdb
#define TXaBRbM           0xdc
#define TXaBMbM           0xdd
#define TXaBYbM           0xde
#define TXaBWbM           0xdf
#define TXaNZbY           0xe0
#define TXaNBbY           0xe1
#define TXaNGbY           0xe2
#define TXaNCbY           0xe3
#define TXaNRbY           0xe4
#define TXaNMbY           0xe5
#define TXaNYbY           0xe6
#define TXaNWbY           0xe7
#define TXaBZbY           0xe8
#define TXaBBbY           0xe9
#define TXaBGbY           0xea
#define TXaBCbY           0xeb
#define TXaBRbY           0xec
#define TXaBMbY           0xed
#define TXaBYbY           0xee
#define TXaBWbY           0xef
#define TXaNZbW           0xf0
#define TXaNBbW           0xf1
#define TXaNGbW           0xf2
#define TXaNCbW           0xf3
#define TXaNRbW           0xf4
#define TXaNMbW           0xf5
#define TXaNYbW           0xf6
#define TXaNWbW           0xf7
#define TXaBZbW           0xf8
#define TXaBBbW           0xf9
#define TXaBGbW           0xfa
#define TXaBCbW           0xfb
#define TXaBRbW           0xfc
#define TXaBMbW           0xfd
#define TXaBYbW           0xfe
#define TXaBWbW           0xff

#define NORMAL            0x100                 // reset attributes to default
#define CURSOR_UP1        0x101
#define CURSOR_UP2        0x102
#define CURSOR_UP4        0x103
#define CURSOR_UP8        0x104
#define CURSOR_DOWN1      0x105
#define CURSOR_DOWN2      0x106
#define CURSOR_DOWN4      0x107
#define CURSOR_DOWN8      0x108
#define CURSOR_RIGHT1     0x109
#define CURSOR_RIGHT2     0x10a
#define CURSOR_RIGHT4     0x10b
#define CURSOR_RIGHT8     0x10c
#define CURSOR_LEFT1      0x10d
#define CURSOR_LEFT2      0x10e
#define CURSOR_LEFT4      0x10f
#define CURSOR_LEFT8      0x110
#define CURSOR_SAVEP      0x111
#define CURSOR_RESTP      0x112
#define CLEAR_TO_EOL      0x113
#define CURS_GO_1_70      0x114
#define CURS_GO_1_75      0x115
#define CURS_GO_1_01      0x116
#define CURS_GO_2_01      0x117
#define CURS_GO23_01      0x118
#define CURS_GO24_01      0x119
#define NUMBER_ANSIS      0x11a

typedef char           ANSISTRING[16];
typedef ANSISTRING     ANSIDEFS[NUMBER_ANSIS];

//- ANSI color strings for NORMAL backgrounds
#define CNZnZ          ansi[TXaNZnZ]
#define CNBnZ          ansi[TXaNBnZ]
#define CNGnZ          ansi[TXaNGnZ]
#define CNCnZ          ansi[TXaNCnZ]
#define CNRnZ          ansi[TXaNRnZ]
#define CNMnZ          ansi[TXaNMnZ]
#define CNYnZ          ansi[TXaNYnZ]
#define CNWnZ          ansi[TXaNWnZ]
#define CBZnZ          ansi[TXaBZnZ]
#define CBBnZ          ansi[TXaBBnZ]
#define CBGnZ          ansi[TXaBGnZ]
#define CBCnZ          ansi[TXaBCnZ]
#define CBRnZ          ansi[TXaBRnZ]
#define CBMnZ          ansi[TXaBMnZ]
#define CBYnZ          ansi[TXaBYnZ]
#define CBWnZ          ansi[TXaBWnZ]
#define CNZnB          ansi[TXaNZnB]
#define CNBnB          ansi[TXaNBnB]
#define CNGnB          ansi[TXaNGnB]
#define CNCnB          ansi[TXaNCnB]
#define CNRnB          ansi[TXaNRnB]
#define CNMnB          ansi[TXaNMnB]
#define CNYnB          ansi[TXaNYnB]
#define CNWnB          ansi[TXaNWnB]
#define CBZnB          ansi[TXaBZnB]
#define CBBnB          ansi[TXaBBnB]
#define CBGnB          ansi[TXaBGnB]
#define CBCnB          ansi[TXaBCnB]
#define CBRnB          ansi[TXaBRnB]
#define CBMnB          ansi[TXaBMnB]
#define CBYnB          ansi[TXaBYnB]
#define CBWnB          ansi[TXaBWnB]
#define CNZnG          ansi[TXaNZnG]
#define CNBnG          ansi[TXaNBnG]
#define CNGnG          ansi[TXaNGnG]
#define CNCnG          ansi[TXaNCnG]
#define CNRnG          ansi[TXaNRnG]
#define CNMnG          ansi[TXaNMnG]
#define CNYnG          ansi[TXaNYnG]
#define CNWnG          ansi[TXaNWnG]
#define CBZnG          ansi[TXaBZnG]
#define CBBnG          ansi[TXaBBnG]
#define CBGnG          ansi[TXaBGnG]
#define CBCnG          ansi[TXaBCnG]
#define CBRnG          ansi[TXaBRnG]
#define CBMnG          ansi[TXaBMnG]
#define CBYnG          ansi[TXaBYnG]
#define CBWnG          ansi[TXaBWnG]
#define CNZnC          ansi[TXaNZnC]
#define CNBnC          ansi[TXaNBnC]
#define CNGnC          ansi[TXaNGnC]
#define CNCnC          ansi[TXaNCnC]
#define CNRnC          ansi[TXaNRnC]
#define CNMnC          ansi[TXaNMnC]
#define CNYnC          ansi[TXaNYnC]
#define CNWnC          ansi[TXaNWnC]
#define CBZnC          ansi[TXaBZnC]
#define CBBnC          ansi[TXaBBnC]
#define CBGnC          ansi[TXaBGnC]
#define CBCnC          ansi[TXaBCnC]
#define CBRnC          ansi[TXaBRnC]
#define CBMnC          ansi[TXaBMnC]
#define CBYnC          ansi[TXaBYnC]
#define CBWnC          ansi[TXaBWnC]
#define CNZnR          ansi[TXaNZnR]
#define CNBnR          ansi[TXaNBnR]
#define CNGnR          ansi[TXaNGnR]
#define CNCnR          ansi[TXaNCnR]
#define CNRnR          ansi[TXaNRnR]
#define CNMnR          ansi[TXaNMnR]
#define CNYnR          ansi[TXaNYnR]
#define CNWnR          ansi[TXaNWnR]
#define CBZnR          ansi[TXaBZnR]
#define CBBnR          ansi[TXaBBnR]
#define CBGnR          ansi[TXaBGnR]
#define CBCnR          ansi[TXaBCnR]
#define CBRnR          ansi[TXaBRnR]
#define CBMnR          ansi[TXaBMnR]
#define CBYnR          ansi[TXaBYnR]
#define CBWnR          ansi[TXaBWnR]
#define CNZnM          ansi[TXaNZnM]
#define CNBnM          ansi[TXaNBnM]
#define CNGnM          ansi[TXaNGnM]
#define CNCnM          ansi[TXaNCnM]
#define CNRnM          ansi[TXaNRnM]
#define CNMnM          ansi[TXaNMnM]
#define CNYnM          ansi[TXaNYnM]
#define CNWnM          ansi[TXaNWnM]
#define CBZnM          ansi[TXaBZnM]
#define CBBnM          ansi[TXaBBnM]
#define CBGnM          ansi[TXaBGnM]
#define CBCnM          ansi[TXaBCnM]
#define CBRnM          ansi[TXaBRnM]
#define CBMnM          ansi[TXaBMnM]
#define CBYnM          ansi[TXaBYnM]
#define CBWnM          ansi[TXaBWnM]
#define CNZnY          ansi[TXaNZnY]
#define CNBnY          ansi[TXaNBnY]
#define CNGnY          ansi[TXaNGnY]
#define CNCnY          ansi[TXaNCnY]
#define CNRnY          ansi[TXaNRnY]
#define CNMnY          ansi[TXaNMnY]
#define CNYnY          ansi[TXaNYnY]
#define CNWnY          ansi[TXaNWnY]
#define CBZnY          ansi[TXaBZnY]
#define CBBnY          ansi[TXaBBnY]
#define CBGnY          ansi[TXaBGnY]
#define CBCnY          ansi[TXaBCnY]
#define CBRnY          ansi[TXaBRnY]
#define CBMnY          ansi[TXaBMnY]
#define CBYnY          ansi[TXaBYnY]
#define CBWnY          ansi[TXaBWnY]
#define CNZnW          ansi[TXaNZnW]
#define CNBnW          ansi[TXaNBnW]
#define CNGnW          ansi[TXaNGnW]
#define CNCnW          ansi[TXaNCnW]
#define CNRnW          ansi[TXaNRnW]
#define CNMnW          ansi[TXaNMnW]
#define CNYnW          ansi[TXaNYnW]
#define CNWnW          ansi[TXaNWnW]
#define CBZnW          ansi[TXaBZnW]
#define CBBnW          ansi[TXaBBnW]
#define CBGnW          ansi[TXaBGnW]
#define CBCnW          ansi[TXaBCnW]
#define CBRnW          ansi[TXaBRnW]
#define CBMnW          ansi[TXaBMnW]
#define CBYnW          ansi[TXaBYnW]
#define CBWnW          ansi[TXaBWnW]

//- ANSI color strings for BRIGHT backgrounds
#define CNZbZ          ansi[TXaNZbZ]
#define CNBbZ          ansi[TXaNBbZ]
#define CNGbZ          ansi[TXaNGbZ]
#define CNCbZ          ansi[TXaNCbZ]
#define CNRbZ          ansi[TXaNRbZ]
#define CNMbZ          ansi[TXaNMbZ]
#define CNYbZ          ansi[TXaNYbZ]
#define CNWbZ          ansi[TXaNWbZ]
#define CBZbZ          ansi[TXaBZbZ]
#define CBBbZ          ansi[TXaBBbZ]
#define CBGbZ          ansi[TXaBGbZ]
#define CBCbZ          ansi[TXaBCbZ]
#define CBRbZ          ansi[TXaBRbZ]
#define CBMbZ          ansi[TXaBMbZ]
#define CBYbZ          ansi[TXaBYbZ]
#define CBWbZ          ansi[TXaBWbZ]
#define CNZbB          ansi[TXaNZbB]
#define CNBbB          ansi[TXaNBbB]
#define CNGbB          ansi[TXaNGbB]
#define CNCbB          ansi[TXaNCbB]
#define CNRbB          ansi[TXaNRbB]
#define CNMbB          ansi[TXaNMbB]
#define CNYbB          ansi[TXaNYbB]
#define CNWbB          ansi[TXaNWbB]
#define CBZbB          ansi[TXaBZbB]
#define CBBbB          ansi[TXaBBbB]
#define CBGbB          ansi[TXaBGbB]
#define CBCbB          ansi[TXaBCbB]
#define CBRbB          ansi[TXaBRbB]
#define CBMbB          ansi[TXaBMbB]
#define CBYbB          ansi[TXaBYbB]
#define CBWbB          ansi[TXaBWbB]
#define CNZbG          ansi[TXaNZbG]
#define CNBbG          ansi[TXaNBbG]
#define CNGbG          ansi[TXaNGbG]
#define CNCbG          ansi[TXaNCbG]
#define CNRbG          ansi[TXaNRbG]
#define CNMbG          ansi[TXaNMbG]
#define CNYbG          ansi[TXaNYbG]
#define CNWbG          ansi[TXaNWbG]
#define CBZbG          ansi[TXaBZbG]
#define CBBbG          ansi[TXaBBbG]
#define CBGbG          ansi[TXaBGbG]
#define CBCbG          ansi[TXaBCbG]
#define CBRbG          ansi[TXaBRbG]
#define CBMbG          ansi[TXaBMbG]
#define CBYbG          ansi[TXaBYbG]
#define CBWbG          ansi[TXaBWbG]
#define CNZbC          ansi[TXaNZbC]
#define CNBbC          ansi[TXaNBbC]
#define CNGbC          ansi[TXaNGbC]
#define CNCbC          ansi[TXaNCbC]
#define CNRbC          ansi[TXaNRbC]
#define CNMbC          ansi[TXaNMbC]
#define CNYbC          ansi[TXaNYbC]
#define CNWbC          ansi[TXaNWbC]
#define CBZbC          ansi[TXaBZbC]
#define CBBbC          ansi[TXaBBbC]
#define CBGbC          ansi[TXaBGbC]
#define CBCbC          ansi[TXaBCbC]
#define CBRbC          ansi[TXaBRbC]
#define CBMbC          ansi[TXaBMbC]
#define CBYbC          ansi[TXaBYbC]
#define CBWbC          ansi[TXaBWbC]
#define CNZbR          ansi[TXaNZbR]
#define CNBbR          ansi[TXaNBbR]
#define CNGbR          ansi[TXaNGbR]
#define CNCbR          ansi[TXaNCbR]
#define CNRbR          ansi[TXaNRbR]
#define CNMbR          ansi[TXaNMbR]
#define CNYbR          ansi[TXaNYbR]
#define CNWbR          ansi[TXaNWbR]
#define CBZbR          ansi[TXaBZbR]
#define CBBbR          ansi[TXaBBbR]
#define CBGbR          ansi[TXaBGbR]
#define CBCbR          ansi[TXaBCbR]
#define CBRbR          ansi[TXaBRbR]
#define CBMbR          ansi[TXaBMbR]
#define CBYbR          ansi[TXaBYbR]
#define CBWbR          ansi[TXaBWbR]
#define CNZbM          ansi[TXaNZbM]
#define CNBbM          ansi[TXaNBbM]
#define CNGbM          ansi[TXaNGbM]
#define CNCbM          ansi[TXaNCbM]
#define CNRbM          ansi[TXaNRbM]
#define CNMbM          ansi[TXaNMbM]
#define CNYbM          ansi[TXaNYbM]
#define CNWbM          ansi[TXaNWbM]
#define CBZbM          ansi[TXaBZbM]
#define CBBbM          ansi[TXaBBbM]
#define CBGbM          ansi[TXaBGbM]
#define CBCbM          ansi[TXaBCbM]
#define CBRbM          ansi[TXaBRbM]
#define CBMbM          ansi[TXaBMbM]
#define CBYbM          ansi[TXaBYbM]
#define CBWbM          ansi[TXaBWbM]
#define CNZbY          ansi[TXaNZbY]
#define CNBbY          ansi[TXaNBbY]
#define CNGbY          ansi[TXaNGbY]
#define CNCbY          ansi[TXaNCbY]
#define CNRbY          ansi[TXaNRbY]
#define CNMbY          ansi[TXaNMbY]
#define CNYbY          ansi[TXaNYbY]
#define CNWbY          ansi[TXaNWbY]
#define CBZbY          ansi[TXaBZbY]
#define CBBbY          ansi[TXaBBbY]
#define CBGbY          ansi[TXaBGbY]
#define CBCbY          ansi[TXaBCbY]
#define CBRbY          ansi[TXaBRbY]
#define CBMbY          ansi[TXaBMbY]
#define CBYbY          ansi[TXaBYbY]
#define CBWbY          ansi[TXaBWbY]
#define CNZbW          ansi[TXaNZbW]
#define CNBbW          ansi[TXaNBbW]
#define CNGbW          ansi[TXaNGbW]
#define CNCbW          ansi[TXaNCbW]
#define CNRbW          ansi[TXaNRbW]
#define CNMbW          ansi[TXaNMbW]
#define CNYbW          ansi[TXaNYbW]
#define CNWbW          ansi[TXaNWbW]
#define CBZbW          ansi[TXaBZbW]
#define CBBbW          ansi[TXaBBbW]
#define CBGbW          ansi[TXaBGbW]
#define CBCbW          ansi[TXaBCbW]
#define CBRbW          ansi[TXaBRbW]
#define CBMbW          ansi[TXaBMbW]
#define CBYbW          ansi[TXaBYbW]
#define CBWbW          ansi[TXaBWbW]

// Definitions for backward compatibility (deprecated)
#define CNZ            CNZnZ                    // NORMAL_BLACK
#define CNB            CNBnZ                    // NORMAL_BLUE
#define CNG            CNGnZ                    // NORMAL_GREEN
#define CNC            CNCnZ                    // NORMAL_CYAN
#define CNR            CNRnZ                    // NORMAL_RED
#define CNM            CNMnZ                    // NORMAL_MAGENTA
#define CNY            CNYnZ                    // NORMAL_YELLOW
#define CNW            CNWnZ                    // NORMAL_WHITE
#define CBZ            CBZnZ                    // BRIGHT_GREY
#define CBB            CBBnZ                    // BRIGHT_BLUE
#define CBG            CBGnZ                    // BRIGHT_GREEN
#define CBC            CBCnZ                    // BRIGHT_CYAN
#define CBR            CBRnZ                    // BRIGHT_RED
#define CBM            CBMnZ                    // BRIGHT_MAGENTA
#define CBY            CBYnZ                    // BRIGHT_YELLOW
#define CBW            CBWnZ                    // BRIGHT_WHITE

#define CnZ            CNWnZ                    // NOR_BG_BLACK
#define CnB            CNWnB                    // NOR_BG_BLUE
#define CnG            CNWnG                    // NOR_BG_GREEN
#define CnC            CNWnC                    // NOR_BG_CYAN
#define CnR            CNWnR                    // NOR_BG_RED
#define CnM            CNWnM                    // NOR_BG_MAGENTA
#define CnY            CNWnY                    // NOR_BG_YELLOW
#define CnW            CNWnW                    // NOR_BG_WHITE
#define CbZ            CNWbZ                    // BRI_BG_GREY
#define CbB            CNWbB                    // BRI_BG_BLUE
#define CbG            CNWbG                    // BRI_BG_GREEN
#define CbC            CNWbC                    // BRI_BG_CYAN
#define CbR            CNWbR                    // BRI_BG_RED
#define CbM            CNWbM                    // BRI_BG_MAGENTA
#define CbY            CNWbY                    // BRI_BG_YELLOW
#define CbW            CNWbW                    // BRI_BG_WHITE

// Definitions for other ANSI functions
#define CNN            ansi[NORMAL]
#define CU1            ansi[CURSOR_UP1]
#define CU2            ansi[CURSOR_UP2]
#define CU4            ansi[CURSOR_UP4]
#define CU8            ansi[CURSOR_UP8]
#define CD1            ansi[CURSOR_DOWN1]
#define CD2            ansi[CURSOR_DOWN2]
#define CD4            ansi[CURSOR_DOWN4]
#define CD8            ansi[CURSOR_DOWN8]
#define CR1            ansi[CURSOR_RIGHT1]
#define CR2            ansi[CURSOR_RIGHT2]
#define CR4            ansi[CURSOR_RIGHT4]
#define CR8            ansi[CURSOR_RIGHT8]
#define CL1            ansi[CURSOR_LEFT1]
#define CL2            ansi[CURSOR_LEFT2]
#define CL4            ansi[CURSOR_LEFT4]
#define CL8            ansi[CURSOR_LEFT8]
#define CGS            ansi[CURSOR_SAVEP]
#define CGR            ansi[CURSOR_RESTP]
#define CGE            ansi[CLEAR_TO_EOL]
#define CG170          ansi[CURS_GO_1_70]
#define CG175          ansi[CURS_GO_1_75]
#define CG1            ansi[CURS_GO_1_01]
#define CG2            ansi[CURS_GO_2_01]
#define CG23           ansi[CURS_GO23_01]
#define CG24           ansi[CURS_GO24_01]

extern  ANSIDEFS       ansi;


/*--- TRACE macro definitions -----------------------------------------------*/

#undef  TxINITmain
#undef  TxEXITmain
#undef  ENTER
#undef  BRETURN
#undef  DRETURN
#undef  VRETURN
#undef  RETURN
#undef  TREOLN
#undef  TXREOLN
#undef  TRNOLN
#undef  TXRNOLN
#undef  TRACON
#undef  TRARGS
#undef  TRARGX
#undef  TRINTF
#undef  TRACEL
#undef  TRACES
#undef  TRACEX
#undef  TRLEVX
#undef  TRHEXD
#undef  TRHEXS
#undef  TRCRCD
#undef  TRDUMP
#undef  TRHEAP
#undef  TRINIT
#undef  TREXIT

extern  ULONG       TxTrLevel;                  // trace level
extern  BOOL        TxTrLogOnly;                // trace to log only
extern  ULONG       TxTrSlowDown;               // xx ms pause per traceline


//- Special main() entry and exit macro's trace and non-trace!


#if defined (DUMP)
    //- additional tracing using TxPrint, if trace is enabled (TxTrLevel)

#include <time.h>
#include <stdio.h>
#include <string.h>

#define TxINITmain(trenv,prod,quiet,freeform,swfunc)                \
   {                                                                \
      char                     *exename = argv[0];                  \
      TXVVFUNCTION              vvfunc  = swfunc;                   \
      TxTraceInitMain( &argc, &argv, trenv, prod);                  \
      if (TxTrLevel >= 10)                                          \
      {                                                             \
         DEVICE_STATE tss = TxScreenState(DEVICE_TEST);             \
         if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);              \
         TxTraceEnter(      (char *) __FUNCTION__);                 \
         TxTraceLeader();                                           \
         TxPrint( "Executable name: '%s'\n", exename);              \
         if (TxTrLogOnly)   TxScreenState(tss);                     \
      }                                                             \
      if (vvfunc != NULL)   (vvfunc)();                             \
      TxaParseArgcArgv( argc, argv, exename, freeform, NULL);       \
      if (TxTrLevel >= 200)   TxaShowParsedCommand( TRUE);          \
      TxScreenInitialize();                                         \
      TxLibraryInit(quiet);                                         \
   }

#define TxEXITmain(retc)                                            \
   TxaDropParsedCommand( TRUE);                                     \
   if (TxTrLevel >= 10)                                             \
   {                                                                \
      DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                \
      if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                 \
      TxTraceReturn(     (char *) __FUNCTION__, (ULONG) retc);      \
      if (TxTrLogOnly)   TxScreenState(tss);                        \
   }                                                                \
   TxLibraryExit();                                                 \
   return((int) retc);


#define ENTER()                                                     \
   if (TxTrLevel >= 10)                                             \
   {                                                                \
      DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                \
      if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                 \
      TxTraceEnter(      (char *) __FUNCTION__);                    \
      if (TxTrLogOnly)   TxScreenState(tss);                        \
   }                                                                \
   else if (TxTrLevel != 0)                                         \
   {                                                                \
      txwSetDesktopTopLine( (char *) __FUNCTION__, cSchemeColor);   \
      if (TxTrSlowDown) TxSleep( TxTrSlowDown);                     \
   }

#define VRETURN()                                                   \
   if (TxTrLevel >= 10)                                             \
   {                                                                \
      DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                \
      if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                 \
      TxTraceRetVoid( (char *) __FUNCTION__);                       \
      if (TxTrLogOnly)   TxScreenState(tss);                        \
   }                                                                \
   return

#define DRETURN(rc)                                                 \
   if (TxTrLevel >= 10)                                             \
   {                                                                \
      DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                \
      if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                 \
      TxTraceRetDouble( (char *) __FUNCTION__, (double) rc);        \
      if (TxTrLogOnly)   TxScreenState(tss);                        \
   }                                                                \
   return(rc)

#define BRETURN(rc)                                                 \
   if (TxTrLevel >= 10)                                             \
   {                                                                \
      DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                \
      if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                 \
      TxTraceRetBool( (char *) __FUNCTION__, (BOOL) rc);            \
      if (TxTrLogOnly)   TxScreenState(tss);                        \
   }                                                                \
   return(rc)

#define RETURN(rc)                                                  \
   if (TxTrLevel >= 10)                                             \
   {                                                                \
      DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                \
      if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                 \
      TxTraceReturn( (char *) __FUNCTION__, (ULONG) rc);            \
      if (TxTrLogOnly)   TxScreenState(tss);                        \
   }                                                                \
   return(rc)


#define TRLEVX(lev,txf)                                             \
   {                                                                \
      if (TxTrLevel >= lev)                                         \
      {                                                             \
         DEVICE_STATE tss = TxScreenState(DEVICE_TEST);             \
         if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);              \
         TxTraceLeader();                                           \
         TxPrint  txf;                                              \
         if (TxTrLogOnly)   TxScreenState(tss);                     \
      }                                                             \
   }

// Trace to logfile only, avoiding recursive loops
//                  usage:  TRACEL(("fmt-str", arg1, arg2, ...));
#define TRACEL(txf)                                                 \
   {                                                                \
      if (TxTrLevel >= 90)                                          \
      {                                                             \
         FILE *log = TxQueryLogFile( NULL, NULL);                   \
         fprintf  txf;                                              \
         fflush(log);                                               \
      }                                                             \
   }

#define TRHEXS(lev,a,b,s)                                           \
   {                                                                \
      if (TxTrLevel >= lev)                                         \
      {                                                             \
         DEVICE_STATE tss = TxScreenState(DEVICE_TEST);             \
         if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);              \
         TxPrint( "Hexdump at  %8.8lx size:% 3lu bytes, of '%s'\n", \
                        (BYTE *) a, (ULONG) b, s);                  \
         TxDisplHexDump((BYTE *) a, (ULONG) b);                     \
         if (TxTrLogOnly)   TxScreenState(tss);                     \
      }                                                             \
   }

#define TRHEXD(lev,a,b) TRHEXS(lev,a,b,"unspecified DATA")

#define TRCRCD(l,a,b,d)                                             \
   {                                                                \
      if (TxTrLevel >= 30)                                          \
      {                                                             \
         DEVICE_STATE tss = TxScreenState(DEVICE_TEST);             \
         if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);              \
         TxPrint("%s  Data *: %8.8lX, size: % 9lu "                 \
                      "= %8.8lX, CRC: %s%8.8lX%s\n",                \
                      l, a, (ULONG) b, (ULONG) b,                   \
                      CBY, TxCrc32( a, (ULONG) b), CNN);            \
         TxDisplHexDump((BYTE *)a, (ULONG) d);                      \
         if (TxTrLogOnly)   TxScreenState(tss);                     \
      }                                                             \
   }


#define TRDUMP(lev,l,a,b,o)                                         \
   {                                                                \
      if (TxTrLevel >= lev)                                         \
      {                                                             \
         DEVICE_STATE tss = TxScreenState(DEVICE_TEST);             \
         if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);              \
         TxDisplayHex(l, (BYTE *)a, (ULONG) b, (ULONG) o);          \
         if (TxTrLogOnly)   TxScreenState(tss);                     \
      }                                                             \
   }


#if defined (DARWIN)
#define TRHEAP(lev)
#else
#define TRHEAP(lev)                                                 \
   {                                                                \
      if (TxTrLevel >= lev)                                         \
      {                                                             \
         switch (_heapchk())                                        \
         {                                                          \
            case _HEAPOK:                                           \
            case _HEAPEMPTY:                                        \
               TRLEVX(lev,( "Heap is still OK\n"));                 \
               break;                                               \
                                                                    \
            default:                                                \
               TRLEVX(lev,( "Heap corruption!\n"));                 \
               break;                                               \
         }                                                          \
      }                                                             \
   }
#endif



#define TRINIT(lev)                                                 \
   if (TxTrLevel >= lev)                                            \
   {                                                                \
      DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                \
      if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                 \
      TxTraceLeader()


#define TREXIT()                                                    \
      if (TxTrLogOnly)   TxScreenState(tss);                        \
   }



#define TREOLN (((TxTrLevel >= 10) && (!TxTrLogOnly)) ? "\n" : "")
#define TRNOLN (((TxTrLevel <  10) || ( TxTrLogOnly)) ? "\n" : "")

#define TXREOLN()                                                   \
   {                                                                \
      if ((TxTrLevel >= 10) && (!TxTrLogOnly))                      \
      {                                                             \
         DEVICE_STATE tss = TxScreenState(DEVICE_TEST);             \
         if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);              \
         TxPrint("\n");                                             \
         if (TxTrLogOnly)   TxScreenState(tss);                     \
      }                                                             \
   }

#define TXRNOLN()                                                   \
   {                                                                \
      if ((TxTrLevel <  10) || ( TxTrLogOnly))                      \
      {                                                             \
         TxPrint("\n");                                             \
      }                                                             \
   }

// TRACE printf without a leader, single character progress
#define TRINTF(txf)                                                 \
   {                                                                \
      if (TxTrLevel >= 70)                                          \
      {                                                             \
         DEVICE_STATE tss = TxScreenState(DEVICE_TEST);             \
         if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);              \
         TxPrint  txf;                                              \
         if (TxTrLogOnly)   TxScreenState(tss);                     \
      }                                                             \
   }


#define TRACON(args)            TRLEVX(1 ,args)
#define TRARGS(args)            TRLEVX(30,args)
#define TRARGX(args)            TRLEVX(50,args)
#define TRACES(args)            TRLEVX(70,args)
#define TRACEX(args)            TRLEVX(90,args)



#else                                           // non-trace version of macro's

#define TxINITmain(trenv,prod,quiet,freeform,swfunc)                \
   {                                                                \
      TXVVFUNCTION              vvfunc  = swfunc;                   \
      TxScreenInitialize();                                         \
      if (vvfunc != NULL)   (vvfunc)();                             \
      TxaParseArgcArgv( argc, argv, NULL, freeform, NULL);          \
      TxLibraryInit(quiet);                                         \
   }

#define TxEXITmain(retc)                                            \
   TxaDropParsedCommand( TRUE);                                     \
   TxLibraryExit();                                                 \
   return((int) retc);

#define ENTER()
#define VRETURN()
#define BRETURN(rc) return(rc)
#define DRETURN(rc) return(rc)
#define RETURN(rc)  return(rc)

#define TREOLN                  ""
#define TXREOLN()
#define TRNOLN                  "\n"
#define TXRNOLN()               TxPrint("\n")
#define TRACON(args)
#define TRARGS(args)
#define TRARGX(args)
#define TRINTF(args)
#define TRACEL(args)
#define TRACES(args)
#define TRACEX(args)
#define TRLEVX(lev,txf)
#define TRHEXD(lev,a,b)
#define TRHEXS(lev,a,b,s)
#define TRCRCD(l,a,b,d)
#define TRDUMP(lev,l,a,b,o)
#define TRHEAP(lev)
#define TRINIT(lev) if (TxTrLevel >= lev)                           \
                    {
#define TREXIT()    }
#endif

// Note: Trace functions are always available, even if DUMP not defined
// Init trace level and destinations at startup, to be called from main()
void TxTraceInitMain
(
   int                *pargc,                   // INOUT argument count
   char              **pargv[],                 // INOUT array of arg values
   char               *envname,                 // IN    trace env-var name
   char               *prefix                   // IN    tracefile prefix
);


// Set timestamping on tracelines on or off
void TxTraceSetStamp
(
   BOOL                stamp                    // IN    timestampming on
);


// Handle entry for a routine
void TxTraceEnter
(
   char              *mod                       // IN    module/function name
);


// Handle numeric return of a routine
void TxTraceReturn
(
   char              *mod,                      // IN    module/function name
   ULONG              rc                        // IN    return value
);


// Handle boolean return of a routine
void TxTraceRetBool
(
   char              *mod,                      // IN    module/function name
   BOOL               rc                        // IN    return value
);


// Handle double floating-point return of a routine
void TxTraceRetDouble
(
   char              *mod,                      // IN    module/function name
   double             rc                        // IN    return value
);


// Handle return of a void routine
void TxTraceRetVoid
(
   char              *mod                       // IN    module/function name
);


// Timestamp, thread-id and indent for trace-line
void TxTraceLeader
(
   void
);

// Initialize TX-libary, after parsing EXE parameters and switches
void TxLibraryInit
(
   BOOL                screenOff                // IN    start with screen off
);

// Terminate TX-libary
void TxLibraryExit
(
   void
);

// Initialise TxPrint environment
void TxPrintInit
(
   BOOL                ansiOff,                 // IN    don't use ANSI colors
   BOOL                ascii7                   // IN    Convert to 7-bit ASCII
);


// Terminate TxPrint environment
void TxPrintTerminate
(
   void
);


// Set 7-bit ASCII mode
void TxSetAscii7Mode
(
   BOOL                mode                     // IN    ASCII 7-bit mode
);

// Get ASCII mode, 7-bit or full
BOOL TxGetAscii7Mode                            // RET   ASCII 7-bit in use
(
   void
);


//- Set ansi active or inactive (initial, later controlled by TxPrint)
void TxSetAnsiMode
(
   char                mode                     // IN    ansi mode
);

// Get ansi string definitions active or inactive
char TxGetAnsiMode                              // RET   ansi mode
(
   void
);


// Perform operations on TxPrint HOOK chain
BOOL TxPrintHook                                // RET   success
(
   TXH_TYPE            type,                    // IN    type of handler
   TXH_OPERATION       operation,               // IN    requested operation
   TXH_INFO           *hinfo                    // IN    handler info
);


// Close existing and if specified, open new logfile
BOOL TxAppendToLogFile                          // RET   logfile opened
(
   char               *fname,                   // IN    name of (new) logfile
   BOOL                verbose                  // IN    Show the action
);


// Set log close/reopen on each TxPrint on or off
void TxSetLogReOpen
(
   BOOL                reopen                   // IN    log reopen on
);


// Close existing and reopen (implementing a brute-force flush)
void TxCloseReopenLogFile
(
   void
);


// Signal logfile active
FILE *TxQueryLogFile                            // RET   logfile active
(
   BOOL               *ascii7bit,               // OUT   Use 7-bit ASCII only
   BOOL               *reopenlog                // OUT   reopen logfile
);                                              //       (NULL if not wanted)

// Get name for current logfile, if any
char *TxQueryLogName                            // RET   filename or NULL
(
   void
);

// Construct name for new logfile based upon a sequence number 0..n
char *TxBuildLogName                            // RET   filename
(
   ULONG               seq,                     // IN    sequence number
   ULONG               retain,                  // IN    nr of files kept
   TXLN                buf                      // IN    filename buffer
);

// Substitute first unused auto-nr for last # in filename-spec; add extension
BOOL TxAutoNumberedFname                        // RET   TRUE when existing file
(
   char               *spec,                    // IN    (path +) file specification
   char               *ext,                     // IN    file extension to be used
   char               *anName                   // OUT   auto-numbered name
);

// Output textual information in array of char-pointers
void TxShowTxt
(
   char               *txt[]                    // IN    text to display
);


// Test and set TxScreenState
DEVICE_STATE TxScreenState                      // RET   screen active
(
   DEVICE_STATE        action                   // IN    screen action
);


// Test and set TxLogfileState, logging when state = ON and file opened
DEVICE_STATE TxLogfileState                     // RET   logfile active
(
   DEVICE_STATE        action                   // IN    logfile action
);

// Set logfile maximum size per file
void TxSetLogMaxSize
(
   ULONG               size                     // IN    maximum size, bytes
);

// Set logfile number of files to retain on cycle, #files besides .log itself
void TxSetLogRetain
(
   ULONG               retain                   // IN    retain count
);

// Make ascii-dump of data area on TxPrint output
void TxDisplAscDump
(
   char               *lead,                    // IN    leading string
   BYTE               *data,                    // IN    data area
   ULONG               size                     // IN    size to dump
);

// Make hex-dump of data area on TxPrint output
void TxDisplHexDump
(
   BYTE               *data,                    // IN    data area
   ULONG               size                     // IN    size to dump
);

// Make hex-dump of based data area on TxPrint output, with leading text
void TxDisplayHex
(
   char               *lead,                    // IN    leading text
   BYTE               *data,                    // IN    data area
   ULONG               size,                    // IN    size to dump
   ULONG               base                     // IN    base for display
);

// Make 'LONG' hex-dump of data area on TxPrint output, with 'C' comment ASCII
void TxDispLongHex
(
   BYTE               *data,                    // IN    data area
   ULONG               size                     // IN    size to dump
);

// Translate Mixed Ascii / Hex string specification to length + buffer
USHORT TxFormatMixedStr                         // RET   length of buffer
(
   char              *data,                     // IN    mixed string
   char              *buf                       // OUT   formatted buffer
);

// Return ascii printable character, conservative small range
char TxPrintable                                // RET   printable character
(
   char                c                        // IN    character
);

// Return ascii printable character, widest safe range
char TxPrintSafe                                // RET   printable character
(
   char                c                        // IN    character
);

// printf-like print on stdout, TEE filehandle and output hooks
void TxPrint
(
   char              *fmt,                      // IN    format string (printf)
   ...                                          // IN    variable arguments
);

// Claim TX keyboard access-functions for current thread (TxAbort etc)
ULONG TxClaimKeyboard                           // RET   previous owner
(
   void
);

// Cancel any pending abort status (as returned by TxAbort)
void TxCancelAbort
(
   void
);

// Set abort status to pending, for other threads (as returned by TxAbort)
void TxSetPendingAbort
(
   void
);

// Return kbhit status and latest More ... reply, read away the hit
BOOL TxAbort                                    // RET   TRUE when abort wanted
(
   void
);

// Display a message using format string and ask confirmation; Yes/No/cancel
BOOL TxConfirm                                  // RET   Confirmed
(
   ULONG               helpid,                  // IN    helpid confirmation
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
);

// Display a message or warning using format string and wait for OK (or ESC)
BOOL TxMessage                                  // RET   OK, not escaped
(
   BOOL                acknowledge,             // IN    wait for acknowledge
   ULONG               helpid,                  // IN    helpid confirmation
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
);


// Prompt for user input with a message and read the input
BOOL TxPrompt                                   // RET   value not empty
(
   ULONG               helpid,                  // IN    helpid confirmation
   short               length,                  // IN    max length of value
   char               *value,                   // INOUT entry field value
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
);


/*======== Screen related functions, TXWIOP.C ===============================*/

// Return number of columns on the screen
short  TxScreenCols                             // RET   current screen columns
(
   void
);

#if defined (USEWINDOWING) || defined (DOS32)
// Return current cursor column, 0 = left
short  TxCursorCol                              // RET   current cursor column
(
   void
);
#endif

// Return number of lines on the screen
short  TxScreenRows                             // RET   current screen rows
(
   void
);

#if defined (USEWINDOWING)
// Return current cursor row, 0 = top
short  TxCursorRow                              // RET   current cursor row
(
   void
);

// Set new cursor position row and column
void TxSetCursorPos
(
   short               row,                     // IN    destination row
   short               col                      // IN    destination column
);

// Set new cursor visibility and style
void TxSetCursorStyle
(
   BOOL                visible,                 // IN    cursor visible
   BOOL                insert                   // IN    insert style
);
#endif

// Set blinking/bright-background mode for fullscreen sessions (VGA)
void TxSetBrightBgMode
(
   BOOL                bright                   // IN    use bright background
);


#if defined (DARWIN)

// Implement CONIO style keyboard-hit test
int kbhit (void);

// Implement CONIO style keyboard read character, no echo, non-blocking
int getch (void);

#endif


#endif
