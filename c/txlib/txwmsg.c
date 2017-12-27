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
// Author: J. van Wijk
//
// TX keyboard and message handling for text Windows
//
// JvW  06-07-1998   Initial version
// JvW  13-05-1999   Expanded msg-queue to 16-messages (circular buffer)
// JvW  04-11-2002   Removed getch() usage from KeyEvent (Watcom WIN translate)
// JvW  01-07-2017   Added mouse event support for Linux and Darwin
// JvW  04-08-2017   OS/2 kbd reader polling, to avoid need for <ENTER> to quit

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface


typedef struct txw_acskeystate
{
   USHORT              Std;
   USHORT              Shift;
   USHORT              Ctrl;
   USHORT              Alt;
} TXW_ACSKEYSTATE;                           // end of struct "txw_acskeystate"


#if defined (HAVEMOUSE)
// Create series of messages to assure a continuous Y-range for mouse movement
// Returns X,Y for first point (current MSG) and Posts messages for the rest
static void txwMouseInterpolateMovement
(
   TXWHANDLE           hwnd,                    // IN    destination window
   short               xFrom,                   // IN    starting X pos (col)
   short               yFrom,                   // IN    starting Y pos (row)
   TXW_INPUT_EVENT    *msTo                     // INOUT mouse event (TO pos)
);                                              // col,row updated to 1st step
#endif


#if defined (WIN32)

static HANDLE  winConsole = (HANDLE) TXW_INVALID;

typedef struct txw_key_event                    // modelled after INPUT_RECORD
{                                               // but dedicated to keystrokes
   USHORT              Type;
   USHORT              Fill1;                   // conform to MS packing ?
   USHORT              bKeyDown;
   USHORT              wRepeatCount;
   USHORT              Fill2;
   USHORT              wVirtualKeyCode;
   USHORT              wVirtualScanCode;
   USHORT              Ascii;
   USHORT              KeyState;
   USHORT              Fill3;
} TXW_KEY_EVENT;

typedef struct txw_mou_event                    // modelled after INPUT_RECORD
{                                               // but dedicated to mouse info
   USHORT              Type;
   USHORT              Fill1;                   // conform to MS packing ?
   short               X;
   short               Y;
   ULONG               Button;
   ULONG               KeyState;
   ULONG               Flags;
} TXW_MOU_EVENT;

typedef union txw_event
{
   USHORT           Type;                       // just the Type
   TXW_MOU_EVENT    mou;                        // complete MOU event
   TXW_KEY_EVENT    kbd;                        // complete KBD event
} TXW_EVENT;                                    // end of union "txw_event"


static TXW_ACSKEYSTATE txw_winkey[] =
{
  //- Std   Shift   Ctrl    Alt

  { 0x000,  0x000,  0x000,  0x000 },            // 0x000 unused :-)
  { 0x001,  0x001,  0x001,  0x001 },            // 0x001
  { 0x002,  0x002,  0x002,  0x002 },            // 0x002
  { 0x003,  0x003,  0x003,  0x003 },            // 0x003
  { 0x004,  0x004,  0x004,  0x004 },            // 0x004
  { 0x005,  0x005,  0x005,  0x005 },            // 0x005
  { 0x006,  0x006,  0x006,  0x006 },            // 0x006
  { 0x007,  0x007,  0x007,  0x007 },            // 0x007
  { 0x008,  0x008,  0x008,  0x10e },            // 0x008 BACKSPACE
  { 0x009,  0x10f,  0x009,  0x009 },            // 0x009 TAB
  { 0x00a,  0x00a,  0x00a,  0x00a },            // 0x00a
  { 0x00b,  0x00b,  0x00b,  0x00b },            // 0x00b
  { 0x00c,  0x00c,  0x00c,  0x00c },            // 0x00c
  { 0x00d,  0x00d,  0x00a,  0x11c },            // 0x00d ENTER
  { 0x00e,  0x00e,  0x00e,  0x00e },            // 0x00e
  { 0x00f,  0x00f,  0x194,  0x00f },            // 0x00f
  { 0x010,  0x010,  0x010,  0x010 },            // 0x010
  { 0x011,  0x011,  0x011,  0x011 },            // 0x011
  { 0x012,  0x012,  0x012,  0x012 },            // 0x012
  { 0x013,  0x013,  0x013,  0x013 },            // 0x013
  { 0x014,  0x014,  0x014,  0x014 },            // 0x014
  { 0x015,  0x015,  0x015,  0x015 },            // 0x015
  { 0x016,  0x016,  0x016,  0x016 },            // 0x016
  { 0x017,  0x017,  0x017,  0x017 },            // 0x017
  { 0x018,  0x018,  0x018,  0x018 },            // 0x018
  { 0x019,  0x019,  0x019,  0x019 },            // 0x019
  { 0x01a,  0x01a,  0x01a,  0x01a },            // 0x01a
  { 0x01b,  0x01b,  0x01b,  0x01b },            // 0x01b
  { 0x00d,  0x00d,  0x00a,  0x11c },            // 0x01c PAD ENTER
  { 0x01d,  0x01d,  0x01d,  0x01d },            // 0x01d
  { 0x01e,  0x01e,  0x01e,  0x01e },            // 0x01e
  { 0x01f,  0x01f,  0x01f,  0x01f },            // 0x01f
  { 0x020,  0x020,  0x020,  0x020 },            // 0x020
  { 0x021,  0x021,  0x021,  0x021 },            // 0x021
  { 0x022,  0x022,  0x022,  0x022 },            // 0x022
  { 0x023,  0x023,  0x023,  0x023 },            // 0x023
  { 0x024,  0x024,  0x024,  0x024 },            // 0x024
  { 0x025,  0x025,  0x025,  0x025 },            // 0x025
  { 0x026,  0x026,  0x026,  0x026 },            // 0x026
  { 0x027,  0x027,  0x027,  0x128 },            // 0x027 ' QUOTE
  { 0x028,  0x028,  0x028,  0x028 },            // 0x028
  { 0x029,  0x029,  0x029,  0x029 },            // 0x029
  { 0x02a,  0x02a,  0x02a,  0x02a },            // 0x02a
  { 0x02b,  0x02b,  0x02b,  0x02b },            // 0x02b
  { 0x02c,  0x02c,  0x02c,  0x133 },            // 0x02c , COMMA
  { 0x02d,  0x02d,  0x01f,  0x182 },            // 0x02d - MINUS
  { 0x02e,  0x02e,  0x02e,  0x134 },            // 0x02e . DOT
  { 0x02f,  0x02f,  0x02f,  0x135 },            // 0x02f / SLASH
  { 0x030,  0x030,  0x030,  0x181 },            // 0x030 0
  { 0x031,  0x031,  0x031,  0x178 },            // 0x031 1
  { 0x032,  0x032,  0x103,  0x179 },            // 0x032 2
  { 0x033,  0x033,  0x033,  0x17a },            // 0x033 3
  { 0x034,  0x034,  0x034,  0x17b },            // 0x034 4
  { 0x035,  0x035,  0x035,  0x17c },            // 0x035 5
  { 0x036,  0x036,  0x01e,  0x17d },            // 0x036 6
  { 0x037,  0x037,  0x037,  0x17e },            // 0x037 7
  { 0x038,  0x038,  0x038,  0x17f },            // 0x038 8
  { 0x039,  0x039,  0x039,  0x180 },            // 0x039 9
  { 0x03a,  0x03a,  0x03a,  0x03a },            // 0x03a
  { 0x13b,  0x154,  0x15e,  0x168 },            // 0x03b F1
  { 0x13c,  0x155,  0x15f,  0x169 },            // 0x03c F2
  { 0x13d,  0x156,  0x160,  0x16a },            // 0x03d F3
  { 0x13e,  0x157,  0x161,  0x16b },            // 0x03e F4
  { 0x13f,  0x158,  0x162,  0x16c },            // 0x03f F5
  { 0x140,  0x159,  0x163,  0x16d },            // 0x040 F6
  { 0x141,  0x15a,  0x164,  0x16e },            // 0x041 F7
  { 0x142,  0x15b,  0x165,  0x16f },            // 0x042 F8
  { 0x143,  0x15c,  0x166,  0x170 },            // 0x043 F9
  { 0x144,  0x15d,  0x167,  0x171 },            // 0x044 F10
  { 0x045,  0x045,  0x045,  0x045 },            // 0x045
  { 0x046,  0x046,  0x046,  0x046 },            // 0x046
  { 0x147,  0x047,  0x177,  0x197 },            // 0x047 HOME
  { 0x148,  0x048,  0x18d,  0x198 },            // 0x048 UP
  { 0x149,  0x049,  0x184,  0x199 },            // 0x049 PGUP
  { 0x04a,  0x04a,  0x04a,  0x04a },            // 0x04a
  { 0x14b,  0x04b,  0x173,  0x19b },            // 0x04b LEFT
  { 0x04c,  0x04c,  0x04c,  0x04c },            // 0x04c
  { 0x14d,  0x04d,  0x174,  0x19d },            // 0x04d RIGHT
  { 0x04e,  0x04e,  0x04e,  0x04e },            // 0x04e
  { 0x14f,  0x04f,  0x175,  0x19f },            // 0x04f END
  { 0x150,  0x050,  0x191,  0x1a0 },            // 0x050 DOWN
  { 0x151,  0x051,  0x176,  0x1a1 },            // 0x051 PGDN
  { 0x152,  0x052,  0x192,  0x1a2 },            // 0x052 INSERT
  { 0x153,  0x053,  0x193,  0x1a3 },            // 0x053 DELETE
  { 0x054,  0x054,  0x054,  0x054 },            // 0x054
  { 0x055,  0x055,  0x055,  0x055 },            // 0x055
  { 0x056,  0x056,  0x056,  0x056 },            // 0x056
  { 0x185,  0x187,  0x189,  0x18b },            // 0x057 F11
  { 0x186,  0x188,  0x18a,  0x18c },            // 0x058 F12
  { 0x059,  0x059,  0x059,  0x059 },            // 0x059
  { 0x05a,  0x05a,  0x05a,  0x05a },            // 0x05a
  { 0x05b,  0x05b,  0x01b,  0x11a },            // 0x05b [ LBRACKET
  { 0x05c,  0x05c,  0x01c,  0x12b },            // 0x05c \ BACKSLASH
  { 0x05d,  0x05d,  0x01d,  0x11b },            // 0x05d ] RBRACKET
  { 0x05e,  0x05e,  0x05e,  0x05e },            // 0x05e
  { 0x05f,  0x05f,  0x05f,  0x05f },            // 0x05f
  { 0x060,  0x060,  0x060,  0x129 },            // 0x060 ` BACKQUOTE
  { 0x061,  0x041,  0x061,  0x11e },            // 0x061 a
  { 0x062,  0x042,  0x062,  0x130 },            // 0x062 b
  { 0x063,  0x043,  0x063,  0x12e },            // 0x063 c
  { 0x064,  0x044,  0x064,  0x120 },            // 0x064 d
  { 0x065,  0x045,  0x065,  0x112 },            // 0x065 e
  { 0x066,  0x046,  0x066,  0x121 },            // 0x066 f
  { 0x067,  0x047,  0x067,  0x122 },            // 0x067 g
  { 0x068,  0x048,  0x068,  0x123 },            // 0x068 h
  { 0x069,  0x049,  0x069,  0x117 },            // 0x069 i
  { 0x06a,  0x04a,  0x06a,  0x124 },            // 0x06a j
  { 0x06b,  0x04b,  0x06b,  0x125 },            // 0x06b k
  { 0x06c,  0x04c,  0x06c,  0x126 },            // 0x06c l
  { 0x06d,  0x04d,  0x06d,  0x132 },            // 0x06d m
  { 0x06e,  0x04e,  0x06e,  0x131 },            // 0x06e n
  { 0x06f,  0x04f,  0x06f,  0x118 },            // 0x06f o
  { 0x070,  0x050,  0x070,  0x119 },            // 0x070 p
  { 0x071,  0x051,  0x071,  0x110 },            // 0x071 q
  { 0x072,  0x052,  0x072,  0x113 },            // 0x072 r
  { 0x073,  0x053,  0x073,  0x11f },            // 0x073 s
  { 0x074,  0x054,  0x074,  0x114 },            // 0x074 t
  { 0x075,  0x055,  0x075,  0x116 },            // 0x075 u
  { 0x076,  0x056,  0x076,  0x12f },            // 0x076 v
  { 0x077,  0x057,  0x077,  0x111 },            // 0x077 w
  { 0x078,  0x058,  0x078,  0x12d },            // 0x078 x
  { 0x079,  0x059,  0x079,  0x115 },            // 0x079 y
  { 0x07a,  0x05a,  0x07a,  0x12c }             // 0x07a z
};

#define TXW_ACS_TABLE_SIZE 0x7b                 // Alt/Ctrl/Shift table size


// Translate Mouse Wheel event to corresponding TX movement key value
static ULONG TxWinTranslateWheel                // RET   translated key value
(
   TXW_MOU_EVENT      *m                        // IN    mouse event info
);

/*****************************************************************************/
// Translate Windows Mouse Wheel event to corresponding TX movement key value
/*****************************************************************************/
static ULONG TxWinTranslateWheel                // RET   translated key value
(
   TXW_MOU_EVENT      *m                        // IN    mouse event info
)
{
   ULONG               key;
   BOOL                wdown = ((m->Button   & 0xff000000)   != 0);
   BOOL                shift = ((m->KeyState & TXm_KS_SHIFT) != 0);
   BOOL                ctrl  = ((m->KeyState & TXm_KS_CTRL)  != 0);

   if (m->KeyState & TXm_KS_ALT)                // left/right movement
   {
      if (ctrl)    key = (wdown) ? TXc_RIGHT : TXc_LEFT;
      else         key = (wdown) ? TXk_RIGHT : TXk_LEFT;
   }
   else                                         // up/down movement
   {
      if (shift)                                // move per page
      {
         if (ctrl) key = (wdown) ? TXc_PGDN : TXc_PGUP;
         else      key = (wdown) ? TXk_PGDN : TXk_PGUP;
      }
      else                                      // move per line
      {
         if (ctrl) key = (wdown) ? TXc_DOWN : TXc_UP;
         else      key = (wdown) ? TXk_DOWN : TXk_UP;
      }
   }
   return (key);
}                                               // end 'TxWinTranslateWheel'
/*---------------------------------------------------------------------------*/


#elif defined (UNIX)
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>

#define TXW_ACS_TABLE_SIZE 0x26                 // Alt/Ctrl/Shift table size
#define TXW_INVKEY        ((int) -1)            // invalid key index

#define TXW_KEYS_SIZE     8                     // length of escape-seq in table

// implied keyboard shift-state values
#define  TXW_SST_STND             0x00          // use table column 'unshifted'
#define  TXW_SST_SHFT             0x01          // use table column 'shifted'
#define  TXW_SST_ALTK             0x02          // use table column 'alt'
#define  TXW_SST_CTRL             0x04          // use table column 'control'
#define  TXW_SST_KEYB             0x08          // use shiftstate from keyboard

//- Note: shiftstate only available in a real Linux CONSOLE session, no XTERM
//- Usable in OSX (Darwin) Terminal, but escape-sequences slightly different.
//- Maximum escape-sequence length is 8 (without the Esc, no need for term ZERO)
typedef struct txw_unixkbd
{
   char                es[8];                   // Esc string, except 1st char
   unsigned char       base;                    // base key-value
   unsigned char       state;                   // implied shift-state
} TXW_UNIXKBD;                                  // end of struct "txw_unixkbd"

//- For most values, use SST_REAL, using ShiftState(), on unique escape-strings
//- generated for alt/ctrl/shift special keys, include the state in the table

//- use a second table (like txw_winkey) to resolve 9-bit unique TXW keyvalues

//- ch Index Shiftstate- Single key translates, use (key & 0x0f) and shiftstate
// 0x00,0x00,TXW_SST_CTRL, // 0x103 TXc_2
// 0x09,0x09,TXW_SST_CTRL, // 0x194 TXc_TAB
// 0x0a,0x0a,TXW_SST_STND, // 0x00d TXk_ENTER
// 0x0a,0x0a,TXW_SST_CTRL, // 0x00a TXc_ENTER
// 0x7f,0x0f,TXW_SST_STND, // 0x008 TXk_BACKSPACE
// 0x7f,0x0f,TXW_SST_CTRL, // 0x07f TXc_BACKSP

#if defined (LINUX)

//- Table of LINUX keyboard strings, starting with Escape, resolves index plus
//- shifstate to get the TX-code from a second table.
static TXW_UNIXKBD txw_unixesckey[] =
{
   //-- K-string from 2nd byte                Index shift-State      TX-description
   {{0x09,   0,   0,   0,   0,   0,   0,   0},0x09,TXW_SST_SHFT}, // TXs_TAB
   {{ '[', 'Z',   0,   0,   0,   0,   0,   0},0x09,TXW_SST_SHFT}, // TXs_TAB
   {{0x0a,   0,   0,   0,   0,   0,   0,   0},0x0a,TXW_SST_ALTK}, // TXa_ENTER
   {{0x27,   0,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_ALTK}, // TXa_QUOTE
   {{ ',',   0,   0,   0,   0,   0,   0,   0},0x00,TXW_SST_SHFT}, // TXa_COMMA
   {{ '-',   0,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_SHFT}, // TXa_MINUS
   {{ '.',   0,   0,   0,   0,   0,   0,   0},0x09,TXW_SST_ALTK}, // TXa_DOT
   {{ '/',   0,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_ALTK}, // TXa_SLASH
   {{ ';',   0,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_ALTK}, // TXa_SEMICOLON
   {{ '=',   0,   0,   0,   0,   0,   0,   0},0x0a,TXW_SST_SHFT}, // TXa_EQUAL
   {{ '[',   0,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_STND}, // TXa_LBRACKET
   {{ '0',   0,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_STND}, // TXa_0
   {{ '1',   0,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_STND}, // TXa_1
   {{ '2',   0,   0,   0,   0,   0,   0,   0},0x0d,TXW_SST_STND}, // TXa_2
   {{ '3',   0,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_SHFT}, // TXa_3
   {{ '4',   0,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_SHFT}, // TXa_4
   {{ '5',   0,   0,   0,   0,   0,   0,   0},0x0d,TXW_SST_SHFT}, // TXa_5
   {{ '6',   0,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_ALTK}, // TXa_6
   {{ '7',   0,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_ALTK}, // TXa_7
   {{ '8',   0,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_CTRL}, // TXa_8
   {{ '9',   0,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_CTRL}, // TXa_9
   {{ 'O', 'P',   0,   0,   0,   0,   0,   0},0x1a,TXW_SST_STND}, // TXk_F1
   {{ '[', '[', 'A',   0,   0,   0,   0,   0},0x1a,TXW_SST_STND}, // TXk_F1
   {{ '[', '1', '1', '~',   0,   0,   0,   0},0x1a,TXW_SST_STND}, // TXk_F1
   {{ 'O', '2', 'P',   0,   0,   0,   0,   0},0x1a,TXW_SST_SHFT}, // TXs_F1
   {{ 'O', '1', ';', '2', 'P',   0,   0,   0},0x1a,TXW_SST_SHFT}, // TXs_F1
   {{ '[', '1', ';', '2', 'P',   0,   0,   0},0x1a,TXW_SST_SHFT}, // TXs_F1
   {{ '[', '1', '1', '^',   0,   0,   0,   0},0x1a,TXW_SST_CTRL}, // TXc_F1
   {{ 'O', 'Q',   0,   0,   0,   0,   0,   0},0x1b,TXW_SST_STND}, // TXk_F2
   {{ '[', '[', 'B',   0,   0,   0,   0,   0},0x1b,TXW_SST_STND}, // TXk_F2
   {{ '[', '1', '2', '~',   0,   0,   0,   0},0x1b,TXW_SST_STND}, // TXk_F2
   {{ 'O', '2', 'Q',   0,   0,   0,   0,   0},0x1b,TXW_SST_SHFT}, // TXs_F2
   {{ 'O', '1', ';', '2', 'Q',   0,   0,   0},0x1b,TXW_SST_SHFT}, // TXs_F2
   {{ '[', '1', ';', '2', 'Q',   0,   0,   0},0x1b,TXW_SST_SHFT}, // TXs_F2
   {{ '[', '1', '2', '^',   0,   0,   0,   0},0x1b,TXW_SST_CTRL}, // TXc_F2
   {{ 'O', 'R',   0,   0,   0,   0,   0,   0},0x1c,TXW_SST_STND}, // TXk_F3
   {{ '[', '[', 'C',   0,   0,   0,   0,   0},0x1c,TXW_SST_STND}, // TXk_F3
   {{ '[', '1', '3', '~',   0,   0,   0,   0},0x1c,TXW_SST_STND}, // TXk_F3
   {{ '[', '2', '5', '~',   0,   0,   0,   0},0x1c,TXW_SST_SHFT}, // TXs_F3
   {{ 'O', '2', 'R',   0,   0,   0,   0,   0},0x1c,TXW_SST_SHFT}, // TXs_F3
   {{ 'O', '1', ';', '2', 'R',   0,   0,   0},0x1c,TXW_SST_SHFT}, // TXs_F3
   {{ '[', '1', ';', '2', 'R',   0,   0,   0},0x1c,TXW_SST_SHFT}, // TXs_F3
   {{ '[', '1', '3', '^',   0,   0,   0,   0},0x1c,TXW_SST_CTRL}, // TXc_F3
   {{0x1b, '[', '1', '3', '~',   0,   0,   0},0x1c,TXW_SST_ALTK}, // TXa_F3
   {{ '[', '[', 'D',   0,   0,   0,   0,   0},0x1d,TXW_SST_STND}, // TXk_F4
   {{ 'O', 'S',   0,   0,   0,   0,   0,   0},0x1d,TXW_SST_STND}, // TXk_F4
   {{ '[', '1', '4', '~',   0,   0,   0,   0},0x1d,TXW_SST_STND}, // TXk_F4
   {{ '[', '2', '6', '~',   0,   0,   0,   0},0x1d,TXW_SST_SHFT}, // TXs_F4
   {{ 'O', '2', 'S',   0,   0,   0,   0,   0},0x1d,TXW_SST_SHFT}, // TXs_F4
   {{ 'O', '1', ';', '2', 'S',   0,   0,   0},0x1d,TXW_SST_SHFT}, // TXs_F4
   {{ '[', '1', ';', '2', 'S',   0,   0,   0},0x1d,TXW_SST_SHFT}, // TXs_F4
   {{ '[', '1', '4', '^',   0,   0,   0,   0},0x1d,TXW_SST_CTRL}, // TXc_F4
   {{ '[', '[', 'E',   0,   0,   0,   0,   0},0x1e,TXW_SST_STND}, // TXk_F5
   {{ '[', '1', '5', '~',   0,   0,   0,   0},0x1e,TXW_SST_STND}, // TXk_F5
   {{ '[', '1', '5', ';', '2', '~',   0,   0},0x1e,TXW_SST_SHFT}, // TXs_F5
   {{ '[', '2', '8', '~',   0,   0,   0,   0},0x1e,TXW_SST_SHFT}, // TXs_F5
   {{ '[', '1', '5', ';', '3', '~',   0,   0},0x1e,TXW_SST_ALTK}, // TXa_F5
   {{ '[', '1', '5', '^',   0,   0,   0,   0},0x1e,TXW_SST_CTRL}, // TXc_F5
   {{0x1b, '[', '1', '5', '~',   0,   0,   0},0x1e,TXW_SST_ALTK}, // TXa_F5
   {{ '[', '1', '7', '~',   0,   0,   0,   0},0x1f,TXW_SST_STND}, // TXk_F6
   {{ '[', '1', '7', ';', '2', '~',   0,   0},0x1f,TXW_SST_SHFT}, // TXs_F6
   {{ '[', '2', '9', '~',   0,   0,   0,   0},0x1f,TXW_SST_SHFT}, // TXs_F6
   {{ '[', '1', '7', '^',   0,   0,   0,   0},0x1f,TXW_SST_CTRL}, // TXc_F6
   {{ '[', '1', '7', ';', '3', '~',   0,   0},0x1f,TXW_SST_ALTK}, // TXa_F6
   {{ '[', '1', '8', '~',   0,   0,   0,   0},0x20,TXW_SST_STND}, // TXk_F7
   {{ '[', '1', '8', ';', '2', '~',   0,   0},0x20,TXW_SST_SHFT}, // TXs_F7
   {{ '[', '3', '1', '~',   0,   0,   0,   0},0x20,TXW_SST_SHFT}, // TXs_F7
   {{ '[', '1', '8', '^',   0,   0,   0,   0},0x20,TXW_SST_CTRL}, // TXc_F7
   {{0x1b, '[', '1', '8', '~',   0,   0,   0},0x20,TXW_SST_ALTK}, // TXa_F7
   {{ '[', '1', '8', ';', '3', '~',   0,   0},0x20,TXW_SST_ALTK}, // TXa_F7
   {{ '[', '1', '9', '~',   0,   0,   0,   0},0x21,TXW_SST_STND}, // TXk_F8
   {{ '[', '1', '9', ';', '2', '~',   0,   0},0x21,TXW_SST_SHFT}, // TXs_F8
   {{ '[', '3', '2', '~',   0,   0,   0,   0},0x21,TXW_SST_SHFT}, // TXs_F8
   {{ '[', '1', '9', '^',   0,   0,   0,   0},0x21,TXW_SST_CTRL}, // TXc_F8
   {{0x1b, '[', '1', '9', '~',   0,   0,   0},0x21,TXW_SST_ALTK}, // TXa_F8
   {{ '[', '1', '9', ';', '3', '~',   0,   0},0x21,TXW_SST_ALTK}, // TXa_F8
   {{ '[', '2', '0', '~',   0,   0,   0,   0},0x22,TXW_SST_STND}, // TXk_F9
   {{ '[', '2', '0', ';', '2', '~',   0,   0},0x22,TXW_SST_SHFT}, // TXs_F9
   {{ '[', '3', '3', '~',   0,   0,   0,   0},0x22,TXW_SST_SHFT}, // TXs_F9
   {{ '[', '2', '0', '^',   0,   0,   0,   0},0x22,TXW_SST_CTRL}, // TXc_F9
   {{ '[', '2', '0', ';', '3', '~',   0,   0},0x22,TXW_SST_ALTK}, // TXa_F9
   {{ '[', '2', '1', '~',   0,   0,   0,   0},0x23,TXW_SST_STND}, // TXk_F10
   {{ '[', '2', '1', ';', '2', '~',   0,   0},0x23,TXW_SST_SHFT}, // TXs_F10
   {{ '[', '3', '4', '~',   0,   0,   0,   0},0x23,TXW_SST_SHFT}, // TXs_F10
   {{ '[', '2', '1', '^',   0,   0,   0,   0},0x23,TXW_SST_CTRL}, // TXc_F10
   {{ '[', '2', '1', ';', '3', '~',   0,   0},0x23,TXW_SST_ALTK}, // TXa_F10
   {{ '[', '2', '3', '~',   0,   0,   0,   0},0x24,TXW_SST_STND}, // TXk_F11
   {{ '[', '2', '3', '$',   0,   0,   0,   0},0x24,TXW_SST_SHFT}, // TXs_F11
   {{ '[', '2', '3', ';', '2', '~',   0,   0},0x24,TXW_SST_SHFT}, // TXs_F11
   {{ '[', '2', '3', '^',   0,   0,   0,   0},0x24,TXW_SST_CTRL}, // TXc_F11
   {{0x1b, '[', '2', '3', '~',   0,   0,   0},0x24,TXW_SST_ALTK}, // TXa_F11
   {{ '[', '2', '3', ';', '3', '~',   0,   0},0x24,TXW_SST_ALTK}, // TXa_F11
   {{ '[', '2', '4', '~',   0,   0,   0,   0},0x25,TXW_SST_STND}, // TXk_F12
   {{ '[', '2', '4', '$',   0,   0,   0,   0},0x25,TXW_SST_SHFT}, // TXs_F12
   {{ '[', '2', '4', ';', '2', '~',   0,   0},0x25,TXW_SST_SHFT}, // TXs_F12
   {{ '[', '2', '4', '^',   0,   0,   0,   0},0x25,TXW_SST_CTRL}, // TXc_F12
   {{0x1b, '[', '2', '4', '~',   0,   0,   0},0x25,TXW_SST_ALTK}, // TXa_F12
   {{ '[', '2', '4', ';', '3', '~',   0,   0},0x25,TXW_SST_ALTK}, // TXa_F12
   {{ '[', '2', '~',   0,   0,   0,   0,   0},0x14,TXW_SST_KEYB}, // TX*_INSERT
   {{ '<',   0,   0,   0,   0,   0,   0,   0},0x14,TXW_SST_STND}, // TXk_INSERT (Alt +/-) LXTerminal
   {{ '[', '2', '^',   0,   0,   0,   0,   0},0x14,TXW_SST_CTRL}, // TXc_INSERT
   {{ '[', '2', ';', '5', '~',   0,   0,   0},0x14,TXW_SST_CTRL}, // TXc_INSERT
   {{0x1b, '[', '2', '~',   0,   0,   0,   0},0x14,TXW_SST_ALTK}, // TXa_INSERT
   {{ '[', '2', ';', '3', '~',   0,   0,   0},0x14,TXW_SST_ALTK}, // TXa_INSERT
   {{ '[', '3', '~',   0,   0,   0,   0,   0},0x10,TXW_SST_KEYB}, // TX*_DELETE
   {{ '[', '3', ';', '2', '~',   0,   0,   0},0x10,TXW_SST_SHFT}, // TXs_DELETE
   {{ '[', '3', '^',   0,   0,   0,   0,   0},0x10,TXW_SST_CTRL}, // TXc_DELETE
   {{ '[', '3', ';', '5', '~',   0,   0,   0},0x10,TXW_SST_CTRL}, // TXc_DELETE
   {{0x1b, '[', '3', '~',   0,   0,   0,   0},0x10,TXW_SST_ALTK}, // TXa_DELETE
   {{ '[', '3', ';', '3', '~',   0,   0,   0},0x10,TXW_SST_ALTK}, // TXa_DELETE
   {{ '[', '4', '~',   0,   0,   0,   0,   0},0x12,TXW_SST_KEYB}, // TX*_END
   {{ '[', 'F',   0,   0,   0,   0,   0,   0},0x12,TXW_SST_STND}, // TXk_END
   {{ 'O', 'F',   0,   0,   0,   0,   0,   0},0x12,TXW_SST_STND}, // TXk_END
   {{ '[', '1', ';', '2', 'F',   0,   0,   0},0x12,TXW_SST_SHFT}, // TXs_END
   {{ '[', '1', ';', '5', 'F',   0,   0,   0},0x12,TXW_SST_CTRL}, // TXc_END
   {{ '[', '1', ';', '3', 'F',   0,   0,   0},0x12,TXW_SST_ALTK}, // TXa_END
   {{0x1b, '[', 'F',   0,   0,   0,   0,   0},0x12,TXW_SST_ALTK}, // TXa_END
   {{ '[', '1', '~',   0,   0,   0,   0,   0},0x13,TXW_SST_KEYB}, // TX*_HOME
   {{ '[', 'H',   0,   0,   0,   0,   0,   0},0x13,TXW_SST_STND}, // TXk_HOME
   {{ 'O', 'H',   0,   0,   0,   0,   0,   0},0x13,TXW_SST_STND}, // TXk_HOME
   {{ '[', '1', ';', '2', 'H',   0,   0,   0},0x13,TXW_SST_SHFT}, // TXs_HOME
   {{ '[', '1', ';', '5', 'H',   0,   0,   0},0x13,TXW_SST_CTRL}, // TXc_HOME
   {{ '[', '1', ';', '3', 'H',   0,   0,   0},0x13,TXW_SST_ALTK}, // TXa_HOME
   {{0x1b, '[', 'H',   0,   0,   0,   0,   0},0x13,TXW_SST_ALTK}, // TXa_HOME
   {{ '[', '5', '~',   0,   0,   0,   0,   0},0x17,TXW_SST_KEYB}, // TX*_PGUP
   {{ '[', '5', '^',   0,   0,   0,   0,   0},0x17,TXW_SST_CTRL}, // TXc_PGUP
   {{ '[', '5', ';', '5', '~',   0,   0,   0},0x17,TXW_SST_CTRL}, // TXc_PGUP
   {{0x1b, '[', '5', '~',   0,   0,   0,   0},0x17,TXW_SST_ALTK}, // TXa_PGUP
   {{ '[', '5', ';', '3', '~',   0,   0,   0},0x17,TXW_SST_ALTK}, // TXa_PGUP
   {{ '[', '6', '~',   0,   0,   0,   0,   0},0x16,TXW_SST_KEYB}, // TX*_PGDN
   {{ '[', '6', '^',   0,   0,   0,   0,   0},0x16,TXW_SST_CTRL}, // TXc_PGDN
   {{ '[', '6', ';', '5', '~',   0,   0,   0},0x16,TXW_SST_CTRL}, // TXc_PGDN
   {{0x1b, '[', '6', '~',   0,   0,   0,   0},0x16,TXW_SST_ALTK}, // TXa_PGDN
   {{ '[', '6', ';', '3', '~',   0,   0,   0},0x16,TXW_SST_ALTK}, // TXa_PGDN
   {{ '[', 'A',   0,   0,   0,   0,   0,   0},0x19,TXW_SST_KEYB}, // TX*_UP
   {{ 'O', 'a',   0,   0,   0,   0,   0,   0},0x19,TXW_SST_CTRL}, // TXc_UP
   {{ '[', '1', ';', '5', 'A',   0,   0,   0},0x19,TXW_SST_CTRL}, // TXc_UP
   {{ '[', 'a',   0,   0,   0,   0,   0,   0},0x19,TXW_SST_SHFT}, // TXs_UP
   {{ '[', '1', ';', '2', 'A',   0,   0,   0},0x19,TXW_SST_SHFT}, // TXs_UP
   {{ '[', '1', ';', '3', 'A',   0,   0,   0},0x19,TXW_SST_ALTK}, // TXa_UP
   {{0x1b, '[', 'A',   0,   0,   0,   0,   0},0x19,TXW_SST_ALTK}, // TXa_UP
   {{ '[', 'B',   0,   0,   0,   0,   0,   0},0x11,TXW_SST_KEYB}, // TX*_DOWN
   {{ 'O', 'b',   0,   0,   0,   0,   0,   0},0x11,TXW_SST_CTRL}, // TXc_DOWN
   {{ '[', '1', ';', '5', 'B',   0,   0,   0},0x11,TXW_SST_CTRL}, // TXc_DOWN
   {{ '[', 'b',   0,   0,   0,   0,   0,   0},0x11,TXW_SST_SHFT}, // TXs_DOWN
   {{ '[', '1', ';', '2', 'B',   0,   0,   0},0x11,TXW_SST_SHFT}, // TXs_DOWN
   {{ '[', '1', ';', '3', 'B',   0,   0,   0},0x11,TXW_SST_ALTK}, // TXa_DOWN
   {{0x1b, '[', 'B',   0,   0,   0,   0,   0},0x11,TXW_SST_ALTK}, // TXa_DOWN
   {{ '[', 'C',   0,   0,   0,   0,   0,   0},0x18,TXW_SST_KEYB}, // TX*_RIGHT
   {{ 'O', 'c',   0,   0,   0,   0,   0,   0},0x18,TXW_SST_CTRL}, // TXc_RIGHT
   {{ '[', '1', ';', '5', 'C',   0,   0,   0},0x18,TXW_SST_CTRL}, // TXc_RIGHT
   {{ '[', 'c',   0,   0,   0,   0,   0,   0},0x18,TXW_SST_SHFT}, // TXs_RIGHT
   {{ '[', '1', ';', '2', 'C',   0,   0,   0},0x18,TXW_SST_SHFT}, // TXs_RIGHT
   {{ '[', '1', ';', '6', 'C',   0,   0,   0},0x0e,TXW_SST_SHFT}, // TXscRIGHT
   {{ '[', '1', ';', '3', 'C',   0,   0,   0},0x18,TXW_SST_ALTK}, // TXa_RIGHT
   {{0x1b, '[', 'C',   0,   0,   0,   0,   0},0x18,TXW_SST_ALTK}, // TXa_RIGHT
   {{ '[', 'D',   0,   0,   0,   0,   0,   0},0x15,TXW_SST_KEYB}, // TX*_LEFT
   {{ 'O', 'd',   0,   0,   0,   0,   0,   0},0x15,TXW_SST_CTRL}, // TXc_LEFT
   {{ '[', '1', ';', '5', 'D',   0,   0,   0},0x15,TXW_SST_CTRL}, // TXc_LEFT
   {{ '[', 'd',   0,   0,   0,   0,   0,   0},0x15,TXW_SST_SHFT}, // TXs_LEFT
   {{ '[', '1', ';', '2', 'D',   0,   0,   0},0x15,TXW_SST_SHFT}, // TXs_LEFT
   {{ '[', '1', ';', '6', 'D',   0,   0,   0},0x0e,TXW_SST_STND}, // TXscLEFT
   {{ '[', '1', ';', '3', 'D',   0,   0,   0},0x15,TXW_SST_ALTK}, // TXa_LEFT
   {{0x1b, '[', 'D',   0,   0,   0,   0,   0},0x15,TXW_SST_ALTK}, // TXa_LEFT
   {{'\\',   0,   0,   0,   0,   0,   0,   0},0x00,TXW_SST_CTRL}, // TXa_BACKSLASH
   {{ ']',   0,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_CTRL}, // TXa_RBRACKET
   {{ '`',   0,   0,   0,   0,   0,   0,   0},0x00,TXW_SST_ALTK}, // TXa_BACKQUOTE
   {{ 'a',   0,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_STND}, // TXa_A
   {{ 'b',   0,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_STND}, // TXa_B
   {{ 'c',   0,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_STND}, // TXa_C
   {{ 'd',   0,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_STND}, // TXa_D
   {{ 'e',   0,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_STND}, // TXa_E
   {{ 'f',   0,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_STND}, // TXa_F
   {{ 'g',   0,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_STND}, // TXa_G
   {{ 'h',   0,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_SHFT}, // TXa_H
   {{ 'i',   0,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_SHFT}, // TXa_I
   {{ 'j',   0,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_SHFT}, // TXa_J
   {{ 'k',   0,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_SHFT}, // TXa_K
   {{ 'l',   0,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_SHFT}, // TXa_L
   {{ 'm',   0,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_SHFT}, // TXa_M
   {{ 'n',   0,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_SHFT}, // TXa_N
   {{ 'o',   0,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_CTRL}, // TXa_O
   {{ 'p',   0,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_CTRL}, // TXa_P
   {{ 'q',   0,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_CTRL}, // TXa_Q
   {{ 'r',   0,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_CTRL}, // TXa_R
   {{ 's',   0,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_CTRL}, // TXa_S
   {{ 't',   0,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_CTRL}, // TXa_T
   {{ 'u',   0,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_CTRL}, // TXa_U
   {{ 'v',   0,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_ALTK}, // TXa_V
   {{ 'w',   0,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_ALTK}, // TXa_W
   {{ 'x',   0,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_ALTK}, // TXa_X
   {{ 'y',   0,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_ALTK}, // TXa_Y
   {{ 'z',   0,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_ALTK}, // TXa_Z
   {{0x7f,   0,   0,   0,   0,   0,   0,   0},0x0f,TXW_SST_ALTK}, // TXa_BACKSP
   {{   0,   0,   0,   0,   0,   0,   0,   0},   0,           0}  //- Sentinel
};


//- Table of LINUX (xterm) specific keyboard strings, NOT starting with Escape,
//- resolves index plus shifstate to get the TX-code from a second table.
static TXW_UNIXKBD txw_bytecombokey[] =
{
   //-- K-string from 2nd byte                Index shift-State      TX-description
   {{0xc3,0x9c,   0,   0,   0,   0,   0,   0},0x00,TXW_SST_CTRL}, // TXa_BACKSLASH
   {{0xc2,0xac,   0,   0,   0,   0,   0,   0},0x00,TXW_SST_SHFT}, // TXa_COMMA
   {{0xc2,0xae,   0,   0,   0,   0,   0,   0},0x09,TXW_SST_ALTK}, // TXa_DOT
   {{0xc2,0xbd,   0,   0,   0,   0,   0,   0},0x0a,TXW_SST_SHFT}, // TXa_EQUAL
   {{0xc3,0x9b,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_STND}, // TXa_LBRACKET
   {{0xc2,0xad,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_SHFT}, // TXa_MINUS
   {{0xc2,0xa7,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_ALTK}, // TXa_QUOTE
   {{0xc3,0x9d,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_CTRL}, // TXa_RBRACKET
   {{0xc2,0xbb,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_ALTK}, // TXa_SEMICOLON
   {{0xc2,0xaf,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_ALTK}, // TXa_SLASH
   {{0xc3,0xbf,   0,   0,   0,   0,   0,   0},0x0f,TXW_SST_ALTK}, // TXa_BACKSP
   {{0xc2,0xbc,   0,   0,   0,   0,   0,   0},0x14,TXW_SST_STND}, // TXk_INSERT (+/- key)
   {{0xc2,0xb0,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_STND}, // TXa_0
   {{0xc2,0xb1,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_STND}, // TXa_1
   {{0xc2,0xb2,   0,   0,   0,   0,   0,   0},0x0d,TXW_SST_STND}, // TXa_2
   {{0xc2,0xb3,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_SHFT}, // TXa_3
   {{0xc2,0xb4,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_SHFT}, // TXa_4
   {{0xc2,0xb5,   0,   0,   0,   0,   0,   0},0x0d,TXW_SST_SHFT}, // TXa_5
   {{0xc2,0xb6,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_ALTK}, // TXa_6
   {{0xc2,0xb7,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_ALTK}, // TXa_7
   {{0xc2,0xb8,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_CTRL}, // TXa_8
   {{0xc2,0xb9,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_CTRL}, // TXa_9
   {{0xc3,0xa1,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_STND}, // TXa_A
   {{0xc3,0xa2,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_STND}, // TXa_B
   {{0xc3,0xa3,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_STND}, // TXa_C
   {{0xc3,0xa4,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_STND}, // TXa_D
   {{0xc3,0xa5,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_STND}, // TXa_E
   {{0xc3,0xa6,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_STND}, // TXa_F
   {{0xc3,0xa7,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_STND}, // TXa_G
   {{0xc3,0xa8,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_SHFT}, // TXa_H
   {{0xc3,0xa9,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_SHFT}, // TXa_I
   {{0xc3,0xaa,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_SHFT}, // TXa_J
   {{0xc3,0xab,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_SHFT}, // TXa_K
   {{0xc3,0xac,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_SHFT}, // TXa_L
   {{0xc3,0xad,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_SHFT}, // TXa_M
   {{0xc3,0xae,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_SHFT}, // TXa_N
   {{0xc3,0xaf,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_CTRL}, // TXa_O
   {{0xc3,0xb0,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_CTRL}, // TXa_P
   {{0xc3,0xb1,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_CTRL}, // TXa_Q
   {{0xc3,0xb2,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_CTRL}, // TXa_R
   {{0xc3,0xb3,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_CTRL}, // TXa_S
   {{0xc3,0xb4,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_CTRL}, // TXa_T
   {{0xc3,0xb5,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_CTRL}, // TXa_U
   {{0xc3,0xb6,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_ALTK}, // TXa_V
   {{0xc3,0xb7,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_ALTK}, // TXa_W
   {{0xc3,0xb8,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_ALTK}, // TXa_X
   {{0xc3,0xb9,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_ALTK}, // TXa_Y
   {{0xc3,0xba,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_ALTK}, // TXa_Z
   {{   0,   0,   0,   0,   0,   0,   0,   0},   0,           0}  //- Sentinel
};


#else

//- Table of DARWIN (OSX) keyboard strings, starting with Escape, resolves index plus
//- shifstate to get the TX-code from a second table.
static TXW_UNIXKBD txw_unixesckey[] =
{
   //-- K-string from 2nd byte                Index shift-State      TX-description
   {{ '[', 'Z',   0,   0,   0,   0,   0,   0},0x09,TXW_SST_SHFT}, // TXs_TAB
   {{ 'f',   0,   0,   0,   0,   0,   0,   0},0x18,TXW_SST_ALTK}, // TXa_RIGHT    (a_F)
   {{ 'b',   0,   0,   0,   0,   0,   0,   0},0x15,TXW_SST_ALTK}, // TXa_LEFT     (a_B)
   {{ 'O', 'P',   0,   0,   0,   0,   0,   0},0x1a,TXW_SST_STND}, // TXk_F1
   {{ '[', '1', ';', '2', 'P',   0,   0,   0},0x1a,TXW_SST_SHFT}, // TXs_F1
   {{ 'O', 'Q',   0,   0,   0,   0,   0,   0},0x1b,TXW_SST_STND}, // TXk_F2
   {{ '[', '1', ';', '2', 'Q',   0,   0,   0},0x1b,TXW_SST_SHFT}, // TXs_F2
   {{ 'O', 'R',   0,   0,   0,   0,   0,   0},0x1c,TXW_SST_STND}, // TXk_F3
   {{ '[', '1', ';', '2', 'R',   0,   0,   0},0x1c,TXW_SST_SHFT}, // TXs_F3
   {{ 'O', 'S',   0,   0,   0,   0,   0,   0},0x1d,TXW_SST_STND}, // TXk_F4
   {{ '[', '1', ';', '2', 'S',   0,   0,   0},0x1d,TXW_SST_SHFT}, // TXs_F4
   {{ '[', '1', '5', '~',   0,   0,   0,   0},0x1e,TXW_SST_STND}, // TXk_F5
   {{ '[', '2', '5', '~',   0,   0,   0,   0},0x1e,TXW_SST_SHFT}, // TXs_F5
   {{ '[', '1', '5', ';', '2', '~',   0,   0},0x1e,TXW_SST_SHFT}, // TXs_F5
   {{ '[', '1', '7', '~',   0,   0,   0,   0},0x1f,TXW_SST_STND}, // TXk_F6
   {{ '[', '2', '6', '~',   0,   0,   0,   0},0x1f,TXW_SST_SHFT}, // TXs_F6
   {{ '[', '1', '7', ';', '2', '~',   0,   0},0x1f,TXW_SST_SHFT}, // TXs_F6
   {{ '[', '1', '8', '~',   0,   0,   0,   0},0x20,TXW_SST_STND}, // TXk_F7
   {{ '[', '2', '8', '~',   0,   0,   0,   0},0x20,TXW_SST_SHFT}, // TXs_F7
   {{ '[', '1', '8', ';', '2', '~',   0,   0},0x20,TXW_SST_SHFT}, // TXs_F7
   {{ '[', '1', '9', '~',   0,   0,   0,   0},0x21,TXW_SST_STND}, // TXk_F8
   {{ '[', '2', '9', '~',   0,   0,   0,   0},0x21,TXW_SST_SHFT}, // TXs_F8
   {{ '[', '1', '9', ';', '2', '~',   0,   0},0x21,TXW_SST_SHFT}, // TXs_F8
   {{ '[', '2', '0', '~',   0,   0,   0,   0},0x22,TXW_SST_STND}, // TXk_F9
   {{ '[', '3', '1', '~',   0,   0,   0,   0},0x22,TXW_SST_SHFT}, // TXs_F9
   {{ '[', '2', '0', ';', '2', '~',   0,   0},0x22,TXW_SST_SHFT}, // TXs_F9
   {{ '[', '2', '1', '~',   0,   0,   0,   0},0x23,TXW_SST_STND}, // TXk_F10
   {{ '[', '3', '2', '~',   0,   0,   0,   0},0x23,TXW_SST_SHFT}, // TXs_F10
   {{ '[', '2', '1', ';', '2', '~',   0,   0},0x23,TXW_SST_SHFT}, // TXs_F10
   {{ '[', '2', '3', '~',   0,   0,   0,   0},0x24,TXW_SST_STND}, // TXk_F11
   {{ '[', '3', '3', '~',   0,   0,   0,   0},0x24,TXW_SST_SHFT}, // TXs_F11
   {{ '[', '2', '3', ';', '2', '~',   0,   0},0x24,TXW_SST_SHFT}, // TXs_F11
   {{ '[', '2', '4', '~',   0,   0,   0,   0},0x25,TXW_SST_STND}, // TXk_F12
   {{ '[', '2', '4', ';', '2', '~',   0,   0},0x25,TXW_SST_SHFT}, // TXs_F12
   {{ '[', '3', '4', '~',   0,   0,   0,   0},0x25,TXW_SST_SHFT}, // TXs_F12
   {{ '[', '3', '~',   0,   0,   0,   0,   0},0x10,TXW_SST_STND}, // TXk_DELETE
   {{ '[', '3', ';', '5', '~',   0,   0,   0},0x14,TXW_SST_STND}, // TXc_DELETE -> INSERT remapped!
   {{ '[', '3', ';', '2', '~',   0,   0,   0},0x10,TXW_SST_SHFT}, // TXs_DELETE   (Terminal)
   {{0x1b, '[', '3', '~',   0,   0,   0,   0},0x10,TXW_SST_ALTK}, // TXa_DELETE
   {{0x1b, '[', '2', '~',   0,   0,   0,   0},0x14,TXW_SST_ALTK}, // TXa_INSERT
   {{ '[', '1', '~',   0,   0,   0,   0,   0},0x13,TXW_SST_STND}, // TXk_HOME
   {{ '[', '1', ';', '5', 'H',   0,   0,   0},0x13,TXW_SST_CTRL}, // TXc_HOME     (iTerm)
   {{ '[', '1', ';', '9', 'H',   0,   0,   0},0x13,TXW_SST_ALTK}, // TXa_HOME     (iTerm)
   {{ '[', 'H',   0,   0,   0,   0,   0,   0},0x13,TXW_SST_SHFT}, // TXs_HOME     (Terminal)
   {{ '[', '1', ';', '2', 'H',   0,   0,   0},0x13,TXW_SST_SHFT}, // TXs_HOME     (iTerm)
   {{ '[', '4', '~',   0,   0,   0,   0,   0},0x12,TXW_SST_STND}, // TXk_END
   {{ '[', '1', ';', '5', 'F',   0,   0,   0},0x12,TXW_SST_CTRL}, // TXc_END      (iTerm)
   {{ '[', '1', ';', '9', 'F',   0,   0,   0},0x12,TXW_SST_ALTK}, // TXa_END      (iTerm)
   {{ '[', 'F',   0,   0,   0,   0,   0,   0},0x12,TXW_SST_SHFT}, // TXs_END      (Terminal)
   {{ '[', '1', ';', '2', 'F',   0,   0,   0},0x12,TXW_SST_SHFT}, // TXs_END      (iTerm)
   {{ '[', '5', '~',   0,   0,   0,   0,   0},0x17,TXW_SST_STND}, // TXk_PGUP
   {{0x1b, '[', '5', '~',   0,   0,   0,   0},0x17,TXW_SST_ALTK}, // TXa_PGUP     (iTerm)
   {{ '[', '6', '~',   0,   0,   0,   0,   0},0x16,TXW_SST_STND}, // TXk_PGDN
   {{0x1b, '[', '6', '~',   0,   0,   0,   0},0x16,TXW_SST_ALTK}, // TXa_PGDN     (iTerm)
   {{ '[', 'A',   0,   0,   0,   0,   0,   0},0x19,TXW_SST_STND}, // TXk_UP
   {{ '[', '1', ';', '5', 'A',   0,   0,   0},0x19,TXW_SST_CTRL}, // TXc_UP
   {{ '[', '1', ';', '2', 'A',   0,   0,   0},0x19,TXW_SST_SHFT}, // TXs_UP
   {{0x1b, '[', 'A',   0,   0,   0,   0,   0},0x19,TXW_SST_ALTK}, // TXa_UP
   {{ '[', 'B',   0,   0,   0,   0,   0,   0},0x11,TXW_SST_STND}, // TXk_DOWN
   {{ '[', '1', ';', '5', 'B',   0,   0,   0},0x11,TXW_SST_CTRL}, // TXc_DOWN
   {{ '[', '1', ';', '2', 'B',   0,   0,   0},0x11,TXW_SST_SHFT}, // TXs_DOWN
   {{0x1b, '[', 'B',   0,   0,   0,   0,   0},0x11,TXW_SST_ALTK}, // TXa_DOWN
   {{ '[', 'C',   0,   0,   0,   0,   0,   0},0x18,TXW_SST_STND}, // TXk_RIGHT
   {{ '[', '1', ';', '5', 'C',   0,   0,   0},0x18,TXW_SST_CTRL}, // TXc_RIGHT
   {{ '[', '1', ';', '2', 'C',   0,   0,   0},0x18,TXW_SST_SHFT}, // TXs_RIGHT
   {{0x1b, '[', 'C',   0,   0,   0,   0,   0},0x18,TXW_SST_ALTK}, // TXa_RIGHT
   {{ '[', 'D',   0,   0,   0,   0,   0,   0},0x15,TXW_SST_STND}, // TXk_LEFT
   {{ '[', '1', ';', '5', 'D',   0,   0,   0},0x15,TXW_SST_CTRL}, // TXc_LEFT
   {{ '[', '1', ';', '2', 'D',   0,   0,   0},0x15,TXW_SST_SHFT}, // TXs_LEFT
   {{0x1b, '[', 'D',   0,   0,   0,   0,   0},0x15,TXW_SST_ALTK}, // TXa_LEFT
   {{   0,   0,   0,   0,   0,   0,   0,   0},   0,           0}  //- Sentinel
};




//- Table of DARWIN specific keyboard strings, NOT starting with Escape,
//- resolves index plus shifstate to get the TX-code from a second table.
static TXW_UNIXKBD txw_bytecombokey[] =
{
   //-- K-string from 2nd byte                Index shift-State      TX-description
   {{0xc2,0xab,   0,   0,   0,   0,   0,   0},0x00,TXW_SST_CTRL}, // TXa_BACKSLASH
   {{0xe2,0x89,0xa4,   0,   0,   0,   0,   0},0x00,TXW_SST_SHFT}, // TXa_COMMA
   {{0xe2,0x89,0xa5,   0,   0,   0,   0,   0},0x09,TXW_SST_ALTK}, // TXa_DOT
   {{0xe2,0x89,0xa0,   0,   0,   0,   0,   0},0x0a,TXW_SST_SHFT}, // TXa_EQUAL
   {{0xe2,0x80,0x9c,   0,   0,   0,   0,   0},0x08,TXW_SST_STND}, // TXa_LBRACKET
   {{0xe2,0x80,0x93,   0,   0,   0,   0,   0},0x08,TXW_SST_SHFT}, // TXa_MINUS
   {{0xc3,0xa6,   0,   0,   0,   0,   0,   0},0x08,TXW_SST_ALTK}, // TXa_QUOTE
   {{0xe2,0x80,0x98,   0,   0,   0,   0,   0},0x08,TXW_SST_CTRL}, // TXa_RBRACKET
   {{0xe2,0x80,0xa6,   0,   0,   0,   0,   0},0x06,TXW_SST_ALTK}, // TXa_SEMICOLON
   {{0xc3,0xb7,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_ALTK}, // TXa_SLASH
   {{0xef,0x9d,0x86,   0,   0,   0,   0,   0},0x14,TXW_SST_ALTK}, // TXa_INSERT
   {{0xc2,0xb1,   0,   0,   0,   0,   0,   0},0x14,TXW_SST_STND}, // TXk_INSERT (+/- key)
   {{0xc2,0xba,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_STND}, // TXa_0
   {{0xc2,0xa1,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_STND}, // TXa_1
   {{0xe2,0x84,0xa2,   0,   0,   0,   0,   0},0x0d,TXW_SST_STND}, // TXa_2
   {{0xc2,0xa3,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_SHFT}, // TXa_3
   {{0xc2,0xa2,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_SHFT}, // TXa_4
   {{0xe2,0x88,0x9e,   0,   0,   0,   0,   0},0x0d,TXW_SST_SHFT}, // TXa_5
   {{0xc2,0xa7,   0,   0,   0,   0,   0,   0},0x0b,TXW_SST_ALTK}, // TXa_6
   {{0xc2,0xb6,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_ALTK}, // TXa_7
   {{0xe2,0x80,0xa2,   0,   0,   0,   0,   0},0x0b,TXW_SST_CTRL}, // TXa_8
   {{0xc2,0xaa,   0,   0,   0,   0,   0,   0},0x0c,TXW_SST_CTRL}, // TXa_9
   {{0xc3,0xa5,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_STND}, // TXa_A
   {{0xe2,0x88,0xab,   0,   0,   0,   0,   0},0x02,TXW_SST_STND}, // TXa_B
   {{0xc3,0xa7,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_STND}, // TXa_C
   {{0xe2,0x88,0x82,   0,   0,   0,   0,   0},0x04,TXW_SST_STND}, // TXa_D
   {{0xc2,0xb4,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_STND}, // TXa_E
   {{0xc6,0x92,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_STND}, // TXa_F
   {{0xc2,0xa9,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_STND}, // TXa_G
   {{0xcb,0x99,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_SHFT}, // TXa_H
   {{0xcb,0x86,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_SHFT}, // TXa_I
   {{0xe2,0x88,0x86,   0,   0,   0,   0,   0},0x03,TXW_SST_SHFT}, // TXa_J
   {{0xcb,0x9a,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_SHFT}, // TXa_K
   {{0xc2,0xac,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_SHFT}, // TXa_L
   {{0xc2,0xb5,   0,   0,   0,   0,   0,   0},0x06,TXW_SST_SHFT}, // TXa_M
   {{0xcb,0x9c,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_SHFT}, // TXa_N
   {{0xc3,0xb8,   0,   0,   0,   0,   0,   0},0x01,TXW_SST_CTRL}, // TXa_O
   {{0xcf,0x80,   0,   0,   0,   0,   0,   0},0x02,TXW_SST_CTRL}, // TXa_P
   {{0xc5,0x93,   0,   0,   0,   0,   0,   0},0x03,TXW_SST_CTRL}, // TXa_Q
   {{0xc2,0xae,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_CTRL}, // TXa_R
   {{0xc3,0x9f,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_CTRL}, // TXa_S
   {{0xe2,0x80,0xa0,   0,   0,   0,   0,   0},0x06,TXW_SST_CTRL}, // TXa_T
   {{0xc2,0xa8,   0,   0,   0,   0,   0,   0},0x07,TXW_SST_CTRL}, // TXa_U
   {{0xe2,0x88,0x9a,   0,   0,   0,   0,   0},0x01,TXW_SST_ALTK}, // TXa_V
   {{0xe2,0x88,0x91,   0,   0,   0,   0,   0},0x02,TXW_SST_ALTK}, // TXa_W
   {{0xe2,0x89,0x88,   0,   0,   0,   0,   0},0x03,TXW_SST_ALTK}, // TXa_X
   {{0xc2,0xa5,   0,   0,   0,   0,   0,   0},0x04,TXW_SST_ALTK}, // TXa_Y
   {{0xce,0xa9,   0,   0,   0,   0,   0,   0},0x05,TXW_SST_ALTK}, // TXa_Z
   {{   0,   0,   0,   0,   0,   0,   0,   0},   0,           0}  //- Sentinel
};

#endif


static TXW_ACSKEYSTATE txw_unixacskeystate[] =
{
  //- Std   Shift   Ctrl4   Alt-2       index

  { 0x103,  0x133,  0x12b,  0x129 }, // 0x00 c_2,a_COMMA,a_BACKSLASH, a_BACKQUOTE
  { 0x11e,  0x123,  0x118,  0x12f }, // 0x01 TXa_A,TXa_H,TXa_O,TXa_V
  { 0x130,  0x117,  0x119,  0x111 }, // 0x02 TXa_B,TXa_I,TXa_P,TXa_W
  { 0x12e,  0x124,  0x110,  0x12d }, // 0x03 TXa_C,TXa_J,TXa_Q,TXa_X
  { 0x120,  0x125,  0x113,  0x115 }, // 0x04 TXa_D,TXa_K,TXa_R,TXa_Y
  { 0x112,  0x126,  0x11f,  0x12c }, // 0x05 TXa_E,TXa_L,TXa_S,TXa_Z
  { 0x121,  0x132,  0x114,  0x127 }, // 0x06 TXa_F,TXa_M,TXa_T,a_SEMICOLON
  { 0x122,  0x131,  0x116,  0x135 }, // 0x07 TXa_G,TXa_N,TXa_U,a_SLASH
  { 0x11a,  0x182,  0x11b,  0x128 }, // 0x08 TXa_LBRACKET,TXa_MINUS,TXa_RBRACKET,TXa_QUOTE
  { 0x009,  0x10f,  0x194,  0x134 }, // 0x09 k_TAB,s_TAB,c_TAB,a_DOT
  { 0x00d,  0x183,  0x00a,  0x11c }, // 0x0a k_ENTER,a_EQUAL,c_ENTER,a_ENTER
  { 0x181,  0x17a,  0x17f,  0x17d }, // 0x0b a_0,a_3,a_8,a_6
  { 0x178,  0x17b,  0x180,  0x17e }, // 0x0c a_2,a_4,a_9,a_7
  { 0x179,  0x17c,  0x00d,  0x00d }, // 0x0d a_3,a_5
  { 0x1cb,  0x1cd,  0x00e,  0x00e }, // 0x0e scLEFT,scRIGHT
  { 0x008,  0x1a2,  0x07f,  0x10e }, // 0x0f k_BACKSPACE,a_INSERT,c_BACKSP,a_BACKSP
  { 0x153,  0x1c3,  0x193,  0x1a3 }, // 0x10 k_DELETE,s_DELETE,c_DELETE,a_DELETE
  { 0x150,  0x1c0,  0x191,  0x1a0 }, // 0x11 k_DOWN,s_DOWN,c_DOWN,a_DOWN
  { 0x14f,  0x1bf,  0x175,  0x19f }, // 0x12 k_END,s_END,c_END,a_END
  { 0x147,  0x1b7,  0x177,  0x197 }, // 0x13 k_HOME,s_HOME,c_HOME,a_HOME
  { 0x152,  0x1c2,  0x192,  0x1a2 }, // 0x14 k_INSERT,s_INSERT,c_INSERT,a_INSERT
  { 0x14b,  0x1bb,  0x173,  0x19b }, // 0x15 k_LEFT,s_LEFT,c_LEFT,a_LEFT
  { 0x151,  0x1c1,  0x176,  0x1a1 }, // 0x16 k_PGDN,s_PGDN,c_PGDN,a_PGDN
  { 0x149,  0x1b9,  0x184,  0x199 }, // 0x17 k_PGUP,s_PGUP,c_PGUP,a_PGUP
  { 0x14d,  0x1bd,  0x174,  0x19d }, // 0x18 k_RIGHT,s_RIGHT,c_RIGHT,a_RIGHT
  { 0x148,  0x1b8,  0x18d,  0x198 }, // 0x19 k_UP,s_UP,c_UP,a_UP
  { 0x13b,  0x154,  0x15e,  0x168 }, // 0x1a k_F1,s_F1,c_F1,a_F1
  { 0x13c,  0x155,  0x15f,  0x169 }, // 0x1b k_F2,s_F2,c_F2,a_F2
  { 0x13d,  0x156,  0x160,  0x16a }, // 0x1c k_F3,s_F3,c_F3,a_F3
  { 0x13e,  0x157,  0x161,  0x16b }, // 0x1d k_F4,s_F4,c_F4,a_F4
  { 0x13f,  0x158,  0x162,  0x16c }, // 0x1e k_F5,s_F5,c_F5,a_F5
  { 0x140,  0x159,  0x163,  0x16d }, // 0x1f k_F6,s_F6,c_F6,a_F6
  { 0x141,  0x15a,  0x164,  0x16e }, // 0x20 k_F7,s_F7,c_F7,a_F7
  { 0x142,  0x15b,  0x165,  0x16f }, // 0x21 k_F8,s_F8,c_F8,a_F8
  { 0x143,  0x15c,  0x166,  0x170 }, // 0x22 k_F9,s_F9,c_F9,a_F9
  { 0x144,  0x15d,  0x167,  0x171 }, // 0x23 k_F10,s_F10,c_F10,a_F10
  { 0x185,  0x187,  0x189,  0x18b }, // 0x24 k_F11,s_F11,c_F11,a_F11
  { 0x186,  0x188,  0x18a,  0x18c }  // 0x25 k_F12,s_F12,c_F12,a_F12
};


// Get keyboard shift/alt/control status for Linux keyboard
int TxUnixShiftState
(
   void
);

// Get keyboard key-ready info, with short timeout
int TxUnixKbhit
(
   long                delay                    // IN    delay in ms
);

// Translate given index-value plus shifstate into TX-keycode
static USHORT TxUnixIndex2key                   // RET   TX keycode
(
   int                 index,                   // IN    index in table
   int                 sst                      // IN    shiftstate
);

// Translate KBD Esc-string to a complete mouse-event including shiftstate
static BOOL TxUnixKstring2MouseEvent            // RET   string was mouseevent
(
   char               *keys,                    // INOUT keystring, from 2nd
   TXW_INPUT_EVENT    *mev                      // OUT   mouse event info
);

// Translate KBD BYTE COMBO sequence plus shifstate to index/final shiftstate
static int TxByteComboKstring2Index             // RET   index or -1
(
   char               *keys,                    // IN    keystring, from 1st
   int                 length,                  // IN    length to check
   int                *sst                      // INOUT shifstate
);


/*****************************************************************************/
// Get keyboard shift/alt/control status for Linux keyboard (fails in XDM/KDE)
/*****************************************************************************/
int TxUnixShiftState
(
   void
)
{
   int                 rc = 0;                  // function return

   #if defined (LINUX)
      int                 ms = 6;

      if (ioctl( fileno(stdin), TIOCLINUX, &ms) == 0)
      {
         rc = (ms == 0x08) ? 0x02 : ms;         // translate left-alt key
      }
   #endif
   return (rc);
}                                               // end 'TxUnixShiftState'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Get keyboard key-ready info, with short timeout allowing slower keyboards
/*****************************************************************************/
int TxUnixKbhit
(
   long                delay                    // IN    delay in usec
)
{
   int                 rc = 0;                  // function return
   fd_set              s;
   struct timeval      tv = {0, 0};
   struct termios      old, new;

   tv.tv_usec = delay;                          // set timeout value in usec

   tcgetattr( fileno(stdin), &old );
   new = old;
   new.c_iflag &= ~(IXOFF | IXON);
   new.c_lflag &= ~(ECHO | ICANON | NOFLSH);
   new.c_lflag |= ISIG;
   new.c_cc[VMIN] = 1;
   new.c_cc[VTIME] = 0;
   tcsetattr( fileno(stdin), TCSADRAIN, &new );

   FD_ZERO(&s);
   FD_SET( fileno(stdin), &s);
   rc = (select( fileno(stdin) +1, &s, NULL, NULL, &tv) > 0);

   tcsetattr( fileno(stdin), TCSADRAIN, &old );
   return (rc);
}                                               // end 'TxUnixKbhit'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Translate given UNIX-KEY index-value plus shifstate into TX-keycode
/*****************************************************************************/
static USHORT TxUnixIndex2key                   // RET   TX keycode
(
   int                 index,                   // IN    index in table
   int                 sst                      // IN    shiftstate
)
{
   USHORT              rc = 0;                  // function return

   if ((index >= 0) && (index < TXW_ACS_TABLE_SIZE))
   {
      if      (sst == TXW_SST_ALTK) rc = txw_unixacskeystate[ index].Alt;
      else if (sst == TXW_SST_CTRL) rc = txw_unixacskeystate[ index].Ctrl;
      else if (sst == TXW_SST_SHFT) rc = txw_unixacskeystate[ index].Shift;
      else                          rc = txw_unixacskeystate[ index].Std;
   }
   return (rc);
}                                               // end 'TxUnixIndex2key'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Translate KBD Esc-string plus shifstate to index and final shiftstate
/*****************************************************************************/
static int TxUnixKstring2Index                  // RET   index or -1
(
   char               *keystr,                  // IN    keystring, from 2nd
   int                *sst                      // INOUT shifstate
)
{
   int                 rc  = TXW_INVKEY;        // function return
   TXW_UNIXKBD        *tbl = txw_unixesckey;    // lookup table
   TXTS                key;                     // padded key string

   memset( key, 0, TXW_KEYS_SIZE);              // create full padded key string
   strcpy( key, keystr);

   while (tbl->es[0] != 0)                      // until end of table or found
   {
      #if defined (DUMP)
      if (TxaExeSwitch('K'))                    // trace table-lookup
      {
         int           i;

         TxPrint( "\nCompare to : ");
         for (i = 0; i < TXW_KEYS_SIZE; i++)
         {
            TxPrint("%c", (tbl->es[i]) ? (tbl->es[i] != 0x1b) ? tbl->es[i] : '^' : ' ');
         }
         TxPrint( " =");
         for (i = 0; i < TXW_KEYS_SIZE; i++)
         {
            #if defined (NEVER)                 // for REALLY hard debugging :)
               TxPrint( " %2.2hx=%2.2hx", tbl->es[i], key[i]);
            #else
               if (tbl->es[i])
               {
                  TxPrint( " %2.2hx", tbl->es[i]);
               }
               else
               {
                  TxPrint( "   ");
               }
            #endif
         }
         TxPrint("         ");
      }
      #endif

      if (memcmp( key, tbl->es, TXW_KEYS_SIZE) == 0) // key string matches
      {
         if (tbl->state != TXW_SST_KEYB)        // overruled shiftstate
         {
            *sst = tbl->state;                  // assign implied shiftstate
         }
         rc = tbl->base;                        // base index in next table
         break;
      }
      tbl++;                                    // advance to next string
   }
   return (rc);
}                                               // end 'TxUnixKstring2Index'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Translate KBD Esc-string to a complete mouse-event including shiftstate
// Supports XTERM mouse encoding 1000, 1015 (rxvt) and 1006 (SGR)
// 20170822 JvW: Discard Escape chars after the first position (Esc-Esc is OK)
/*****************************************************************************/
static BOOL TxUnixKstring2MouseEvent            // RET   string was mouse event
(
   char               *keys,                    // INOUT keystring, from 2nd
   TXW_INPUT_EVENT    *mev                      // OUT   mouse event info
)
{
   BOOL                rc  = FALSE;
   BOOL                sgr = FALSE;
   int                 Cb, Cx, Cy;              // parameter values
   char               *s;

   //- start search AFTER first char, which can be a regular Esc in the Esc-sequence
   if ((s = strchr( keys + 1, TXK_ESCAPE)) != NULL) // discard all but 1st Esc seq
   {
      *s = 0;
   }
   if (keys[0] == '[')                          // 2nd part of CSI pre-amble
   {
      if ((keys[1] == 'M') && strlen( keys) == 5) // Mouse sequence 1000
      {
         Cb = keys[2] - 32;                     // copy the single byte, adjust for offset 32
         Cx = keys[3] - 32;
         Cy = keys[4] - 32;
         rc = TRUE;
      }
      else if ((keys[strlen(keys) -1] == 'M') || // Mouse sequence 1006/1015
               (keys[strlen(keys) -1] == 'm')  ) // Mouse sequence 1006 B-DN
      {
         if (keys[1] == '<')                    // SGR 1006 sequence
         {
            s = keys + 2;                       // start of parameter list
            sgr = TRUE;
         }
         else                                   // rxvt 1015 sequence
         {
            s = keys + 1;                       // start of parameter list
         }
         if (sscanf( s, "%d;%d;%d%*s", &Cb, &Cx, &Cy) == 3) // need 3 decimal parameters
         {
            if (sgr == FALSE)                   // 1000/1002/1015 use offset 32
            {
               Cb -= 32;
            }
            rc = TRUE;                          // looks like a mouse event
         }
      }
      if (rc == TRUE)                           // Decode button number and shiftstate
      {
         mev->col = Cx - 1;                     // mouse coordinates
         mev->row = Cy - 1;                     // converted to 0,0 based

         switch (Cb & 0x43)                     // lower 2 bits, and 'wheel' bit
         {
            case 0x00: mev->value = TXm_BUTTON1; break; // button 1, left
            case 0x01: mev->value = TXm_BUTTON3; break; // button 2, middle, may be wheel
            case 0x02: mev->value = TXm_BUTTON2; break; // button 3, right
            case 0x40: mev->value = TXm_WHEELUP; break; // wheel move UP
            case 0x41: mev->value = TXm_WHEELDN; break; // wheel move DOWN
            default:   mev->value = 0;           break; // possible 1015 button-up
         }
         if (Cb & 0x20)                         // mouse movement bit set too
         {
            mev->value |= TXm_DRAGGED;          // for window dragging support
         }
         mev->state = TXm_KS_NONE;
         if (Cb & 0x04)  mev->state |= TXm_KS_SHIFT;
         if (Cb & 0x08)  mev->state |= TXm_KS_ALT;
         if (Cb & 0x10)  mev->state |= TXm_KS_CTRL;

         if ((sgr == TRUE) && (keys[strlen(keys) -1] == 'm')) // button-down in 1006
         {
            mev->value = 0;                     // signal button-down
         }
      }
   }
   return (rc);
}                                               // end 'TxUnixKstring2MouseEvent'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Translate KBD BYTE COMBO sequence plus shifstate to index/final shiftstate
/*****************************************************************************/
static int TxByteComboKstring2Index             // RET   index or -1
(
   char               *keys,                    // IN    keystring, from 1st
   int                 length,                  // IN    length to check
   int                *sst                      // INOUT shifstate
)
{
   int                 rc  = TXW_INVKEY;        // function return
   TXW_UNIXKBD        *key = txw_bytecombokey;  // lookup table

   while (key->es[0] != 0)                      // until end of table or found
   {
      if (memcmp( keys, key->es, length) == 0)  // key string matches
      {
         if (key->state != TXW_SST_KEYB)        // overruled shiftstate
         {
            *sst = key->state;                  // assign implied shiftstate
         }
         rc = key->base;                        // base index in next table
         break;
      }
      key++;                                    // advance to next string
   }
   return (rc);
}                                               // end 'TxByteComboKstring2Index'
/*---------------------------------------------------------------------------*/


#elif defined (DOS32)                           // extended DOS

static  USHORT          txw_mOus = 0;           // mouse presence
static  USHORT          txw_mBut = 0;           // button state
static  USHORT          txw_mCol = 0;           // horizontal position
static  USHORT          txw_mRow = 0;           // vertical   position
static  BOOL            txw_mDragged = FALSE;   // drag in progress
static  BOOL            txw_mCached  = FALSE;   // cached event present
static  TXW_INPUT_EVENT txw_mEvt;               // single event cache

#elif defined (DEV32) & defined (USEWINDOWING)

#define TXW_EVTQUEUESIZE  128
#define READERSTACKSIZE 16384

#define TXOS2_MOUSEEVENTMASK   0x7e             // event mask (all except move)
#define TXOS2_MOUSEDRAWMASK    0x00             // draw  mask (draw by driver)

#define TXOS2_MOUSEBUTTONMASK  0xfffe           // button mask (FS filter)

#define TXOS2_BN1_DRAG         (MOUSE_BN1_DOWN | MOUSE_MOTION_WITH_BN1_DOWN)
#define TXOS2_BN2_DRAG         (MOUSE_BN2_DOWN | MOUSE_MOTION_WITH_BN2_DOWN)
#define TXOS2_BN3_DRAG         (MOUSE_BN3_DOWN | MOUSE_MOTION_WITH_BN3_DOWN)

#if !defined (MOU_GRADD_REGISTER)
#define MOU_GRADD_REGISTER           0x005E     // undocumented, may fix
                                                // draw-bug in FS sessions
typedef struct _TSKTIME
{
    ULONG fTaskPtr;                             // 1 = register, 0=deregister
} TSKTIME, *PTSKTIME;

/* From Lars Erdman, on analysing/fixing the mouse-cursor in full-screen BUG:
   Looking at the pointer draw code in file "util1.asm","DrawPointer"
   function, I would try and set fTaskPtr=0 once on program start as that
   should force a call to the draw function of the POINTER$ device driver
   (POINTDD.SYS, which does the actual mouse pointer drawing).
*/
#endif

static HMOU    txw_hmouse    = 0;               // handle for Mou* API
static HFILE   txw_hmou32    = 0;               // handle for DosIOCtl
       HFILE   txw_hkeyboard = 0;

static HMTX    txw_semEvtQueueAccess = 0;
static HEV     txw_semInputAvailable = 0;

static TXW_INPUT_EVENT  txw_EvtQueueData[TXW_EVTQUEUESIZE];
static int              txw_EvtQueueTail = 0;
static int              txw_EvtQueueHead = 0;


// Add input event to the queue
static void TxOS2AddQueueEvent
(
   TXW_INPUT_EVENT    *event                    // IN    input event to add
);

// get input event from the queue
static void TxOS2GetQueueEvent
(
   TXW_INPUT_EVENT    *event                    // OUT   input event to add
);

// Mouse Reader thread, adds all MOUSE input to queue
static void TxOS2MouseReader
(
   void               *arg                      // IN    thread argument
);

// Keyboard Reader thread, adds all KBD input to queue
static void TxOS2KeyboardReader
(
   void               *arg                      // IN    thread argument
);

/*****************************************************************************/
// Replacement for the kbhit() in the C runtime library (check our own queue)
/*****************************************************************************/
int kbhit (void);                               // replacement for kbhit avoids
int kbhit (void)                                // hang with IBMCPP 3.6 runtime
{                                               // or threaded KBD/MOU system
   int                 hit = 0;

   DosRequestMutexSem( txw_semEvtQueueAccess, SEM_INDEFINITE_WAIT);

   if (txw_EvtQueueTail != txw_EvtQueueHead)    // any events waiting ?
   {
      hit = 1;
   }
   DosReleaseMutexSem( txw_semEvtQueueAccess);

   return(hit);
}                                               // end 'kbhit'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Add input event to the queue
/*****************************************************************************/
static void TxOS2AddQueueEvent
(
   TXW_INPUT_EVENT    *event                    // IN    input event to add
)
{
   DosRequestMutexSem( txw_semEvtQueueAccess, SEM_INDEFINITE_WAIT);

   txw_EvtQueueData[ txw_EvtQueueHead] = *event;
   txw_EvtQueueHead = ((txw_EvtQueueHead +1) % TXW_EVTQUEUESIZE);

   if (txw_EvtQueueTail == txw_EvtQueueHead)    // queue full now ?
   {                                            // discard oldest event
      txw_EvtQueueTail = ((txw_EvtQueueTail +1) % TXW_EVTQUEUESIZE);
   }
   DosPostEventSem(    txw_semInputAvailable);

   DosReleaseMutexSem( txw_semEvtQueueAccess);
}                                               // end 'TxOS2AddQueueEvent'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// get input event from the queue
/*****************************************************************************/
static void TxOS2GetQueueEvent
(
   TXW_INPUT_EVENT    *event                    // OUT   input event to add
)
{
   ULONG            count;
   BOOL             queue_empty;
   NOPTRRECT        rect;
   ULONG            ParamLen = 0;

   DosRequestMutexSem( txw_semEvtQueueAccess, SEM_INDEFINITE_WAIT);
   queue_empty = (txw_EvtQueueTail == txw_EvtQueueHead);
   DosReleaseMutexSem( txw_semEvtQueueAccess);

   if (queue_empty)                             // wait when now empty
   {
      if ((txwa->useMouse) && (txwa->session == PT_FULLSCREEN))
      {
         DosDevIOCtl( txw_hmou32, IOCTL_POINTINGDEVICE, MOU_DRAWPTR,
                      NULL, 0, 0, NULL, 0, 0);
      }

      DosResetEventSem( txw_semInputAvailable, &count);
      DosWaitEventSem(  txw_semInputAvailable, SEM_INDEFINITE_WAIT);

      if ((txwa->useMouse) && (txwa->session == PT_FULLSCREEN))
      {
         rect.row  = rect.col  = 0;
         rect.cRow = TxScreenRows() -1;
         rect.cCol = TxScreenCols() -1;
         ParamLen  = sizeof(rect);
         DosDevIOCtl( txw_hmou32, IOCTL_POINTINGDEVICE, MOU_REMOVEPTR,
                      &rect, ParamLen, &ParamLen, NULL, 0, 0);
      }
   }

   DosRequestMutexSem( txw_semEvtQueueAccess, SEM_INDEFINITE_WAIT);
   *event = txw_EvtQueueData[ txw_EvtQueueTail];
   txw_EvtQueueTail = ((txw_EvtQueueTail +1) % TXW_EVTQUEUESIZE);
   DosReleaseMutexSem( txw_semEvtQueueAccess);
}                                               // end 'TxOS2GetQueueEvent'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Mouse Reader thread, adds all MOUSE input to queue
/*****************************************************************************/
static void TxOS2MouseReader
(
   void               *arg                      // IN    thread argument
)
{
   MOUEVENTINFO        mouInfo;
   SHIFTSTATE          kbdInfo;
   ULONG               DataLen = sizeof(kbdInfo);
   USHORT              wait;
   USHORT              s;
   TXW_INPUT_EVENT     event;

   while (1)                                    // keep running ...
   {
      wait = MOU_WAIT;
      MouReadEventQue( &mouInfo, &wait, txw_hmouse);

      if ( mouInfo.time != 0L )
      {
         kbdInfo.fsState = 0;

         //- must use IOCtl, KBD subsystem is blocked in KbdCharIn()
         DosDevIOCtl( txw_hkeyboard, IOCTL_KEYBOARD, KBD_GETSHIFTSTATE,
                      NULL, 0, 0, &kbdInfo, DataLen, &DataLen);

         s           = kbdInfo.fsState;
         event.state = TXm_KS_NONE;
         if (s & (RIGHTSHIFT | LEFTSHIFT)) event.state |= TXm_KS_SHIFT;
         if (s &  CONTROL)                 event.state |= TXm_KS_CTRL;
         if (s &  ALT)                     event.state |= TXm_KS_ALT;
         if (s &  NUMLOCK_ON)              event.state |= TXm_KS_NUMLK;
         if (s &  SCROLLLOCK_ON)           event.state |= TXm_KS_SCRLK;

         #if defined (DUMP)
         if (TxaExeSwitch('K'))                 // trace mouse
         {
            TxPrint( "  X:% 3hu Y: %3hu  buttons:%4.4hx  Ctrl:%8.8lx\n",
                     mouInfo.col, mouInfo.row, mouInfo.fs, event.state);
         }
         #endif
         event.value = 0;
         switch (mouInfo.fs & TXOS2_MOUSEBUTTONMASK)
         {
            case TXOS2_BN1_DRAG:
            case MOUSE_MOTION_WITH_BN1_DOWN:
               event.value  = TXm_DRAGGED;
            case MOUSE_BN1_DOWN:
               event.value |= TXm_BUTTON1;
               break;

            case TXOS2_BN2_DRAG:
            case MOUSE_MOTION_WITH_BN2_DOWN:
               event.value  = TXm_DRAGGED;
            case MOUSE_BN2_DOWN:
               event.value |= TXm_BUTTON2;
               break;

            case TXOS2_BN3_DRAG:
            case MOUSE_MOTION_WITH_BN3_DOWN:
               event.value  = TXm_DRAGGED;
            case MOUSE_BN3_DOWN:
               event.value |= TXm_BUTTON3;
               break;

            default:                            // button up events
               break;                           // value will be zero
         }
         event.row = mouInfo.row;
         event.col = mouInfo.col;
         event.key = TXW_INPUT_MOUSE;

         TxOS2AddQueueEvent( &event);
      }
   }
}                                               // end 'TxOS2MouseReader'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Keyboard Reader thread, adds all KBD input to queue
// 20170804 JvW: Polling KbdPeek + Sleep / getch, no need for <Enter> to quit!
/*****************************************************************************/
void TxOS2KeyboardReader
(
   void               *arg                      // IN    thread argument
)
{
   ULONG               rc = NO_ERROR;
   TXW_INPUT_EVENT     event;
   KBDKEYINFO          kbinfo;                  // keyboard info structure

   while ((txwa->desktop != NULL) &&            // while windowed ...
          (txwa->KbdKill == FALSE) )            // and no stop request
   {
      //- kbhit name can't be used, it is overriden for application use
      //- by a function that tests for available events in the queue ...
      if ((rc = KbdPeek( &kbinfo, 0)) == NO_ERROR) // OS/2 native kbhit equivalent
      {
         if (kbinfo.fbStatus & KBDTRF_FINAL_CHAR_IN) // key available
         {
            //- use getch() from watcom, since it translates shift/ctrl states for us

            event.key = getch();                // first (or only) keyvalue
            switch (event.key)
            {
               case 0x00:
               case 0xe0: event.key = TXW_KEY_GROUP_1 + getch();  break;
               default:                                           break;
            }
            TxOS2AddQueueEvent( &event);

            TRACES(( "char:0x%2.2hhx scan:0x%2.2hhx fbStatus:0x%2.2hhx NlsShift:0x%2.2hhx fsState:0x%4.4hx TX key:0x%3.3lx\n",
                      kbinfo.chChar, kbinfo.chScan, kbinfo.fbStatus,   kbinfo.bNlsShift,  kbinfo.fsState,     event.key));
         }
         else
         {
            TxSleep( 10);                       // short sleep to avoid CPU pegging
         }
      }
      else
      {
         TRACES(( "KbdPeek returncode: %lu\n", rc));
         break;
      }
   }
   TRACES(( "Keyboard reader thread stopped ...\n"));
}                                               // end 'TxOS2KeyboardReader'
/*---------------------------------------------------------------------------*/

#endif


/*****************************************************************************/
// Initialize low level input handling
/*****************************************************************************/
ULONG TxInputInitialize
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return

   #if defined   (DOS32)
      union  REGS      regs;
   #elif defined (WIN32)
   #elif defined (UNIX)
   #else
      ULONG            act;                     // action taken
      USHORT           mask;
      PTRLOC           mousepos = {0, 0};
   #endif

   ENTER();

   #if defined   (DOS32)
      TxxClearReg( regs);
      TxxMouseInt( regs, TXDX_MOUSE_RESET);     // Reset and get status
      if (TXWORD.ax == TXDX_MOUSE_PRESENT)
      {
         txw_mOus = TXDX_MOUSE_PRESENT;         // remember status

         TRACES(( "Mouse driver initialized, #buttons: %hu\n", TXWORD.bx));
      }
      else
      {
         TRACES(( "Failed to initialize mousedriver!\n"));
      }
   #elif defined (WIN32)
      //- automatically initialized by screen-init (console)
   #elif defined (UNIX)
      (void) system( "stty -echo");             // make sure echo is OFF
   #else

      DosCreateMutexSem( NULL, &txw_semEvtQueueAccess, 0, FALSE);
      DosCreateEventSem( NULL, &txw_semInputAvailable, 0, FALSE);

      if (DosOpen((PSZ) "KBD$",                 // need RAW device, not stdin
                   &txw_hkeyboard, &act, 0,     // file handle, action, size
                   FILE_NORMAL,                 // no attributes
                   OPEN_ACTION_OPEN_IF_EXISTS,  // do not create
                   OPEN_ACCESS_READONLY |       // and allow sharing
                   OPEN_SHARE_DENYNONE,         // When open fails, just the
                   0) != NO_ERROR)              // MOU shift-status is bogus
      {
         TRACES(( "Failed to open KBD$ for mouse shift-state!\n"));
      }

      if ((txwa->useMouse) && (MouOpen(NULL, &txw_hmouse) == NO_ERROR))
      {
         if (txwa->session == PT_FULLSCREEN)
         {
            if (DosOpen((PSZ) "MOUSE$",         // need RAW device, not MOU
                         &txw_hmou32, &act, 0,  // file handle, action, size
                         FILE_NORMAL,           // no attributes
                         OPEN_ACTION_OPEN_IF_EXISTS, // do not create
                         OPEN_ACCESS_READONLY | // and allow sharing
                         OPEN_SHARE_DENYNONE, 0) == NO_ERROR)
            {
               if (TxaExeSwitchSet(TXA_O_MOUSE) && // explicit -mouse fixes
                   TxaExeSwitch(   TXA_O_MOUSE)  ) // mouse cursor on OS2
               {
                  TSKTIME  tsktime  = {0};
                  ULONG    ParamLen = sizeof(tsktime);

                  DosDevIOCtl( txw_hmou32,
                               IOCTL_POINTINGDEVICE, MOU_GRADD_REGISTER,
                               &tsktime, ParamLen, &ParamLen, NULL, 0, 0);
               }
            }
            else
            {
               TRACES(( "Failed to open MOUSE$ for pointer draw/hide!\n"));
            }
         }
         MouSetPtrPos( &mousepos, txw_hmouse);  // mouse to 0,0

         mask = TXOS2_MOUSEEVENTMASK;           // report all except move
         MouSetEventMask( &mask, txw_hmouse);
         mask = TXOS2_MOUSEDRAWMASK;            // Draw by driver, not appl
         MouSetDevStatus( &mask, txw_hmouse);

         MouFlushQue( txw_hmouse);

         TxBeginThread( TxOS2MouseReader, READERSTACKSIZE, NULL);
         TRACES(( "Mouse opened, reader thread started ...\n"));
      }
   #endif

   RETURN(rc);
}                                               // end 'TxInputInitialize'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Initialize input for windowed Desktop environment
/*****************************************************************************/
ULONG TxInputDesktopInit
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   #if defined   (DOS32)
   #elif defined (WIN32)
   #elif defined (UNIX)
   #else
      DosRequestMutexSem( txw_semEvtQueueAccess, SEM_INDEFINITE_WAIT);
      txw_EvtQueueHead = 0;
      txw_EvtQueueTail = 0;                     // flush input queue
      DosReleaseMutexSem( txw_semEvtQueueAccess);

      txwa->KbdKill = FALSE;
      TxBeginThread( TxOS2KeyboardReader, READERSTACKSIZE, NULL);
      TRACES(( "keyboard reader thread started ...\n"));
   #endif

   RETURN(rc);
}                                               // end 'TxInputDesktopInit'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Terminate low level input handling
/*****************************************************************************/
ULONG TxInputTerminate
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return

   #if defined   (DOS32)
   #elif defined (WIN32)
   #elif defined (UNIX)
   #else
      if (txwa->session == PT_FULLSCREEN)
      {
         if (txw_hmou32 != 0)
         {
            DosClose( txw_hmou32);
         }
      }
      else
      {
         if (txw_hmouse != 0)
         {
            MouClose( txw_hmouse);              // hangs in full-screen due
         }                                      // to bad design of MOU system
      }
      if (txw_hkeyboard != 0)
      {
         DosClose( txw_hkeyboard);
      }
      DosCloseMutexSem( txw_semEvtQueueAccess);
      DosCloseEventSem( txw_semInputAvailable);
   #endif

   return (rc);
}                                               // end 'TxInputTerminate'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get keystroke(s) and return a unique single value for each (getch) key
// Uses a special OTHER value and additional info structure for mouse events
/*****************************************************************************/
ULONG txwGetInputEvent                          // RET   unique event value
(
   BOOL                debug,                   // IN    show internals (ESC strings)
   TXW_INPUT_EVENT    *event                    // OUT   optional event info
)                                               //       NULL when keystroke
{                                               //       events only desired
   ULONG               key = 0;                 // RET   event value

   #if defined (WIN32)
      ULONG            mode;
      ULONG            dummy;
      TXW_KEY_EVENT    we;
      TXW_MOU_EVENT   *me = (TXW_MOU_EVENT *) &we;

      if (winConsole == (HANDLE) TXW_INVALID)   // no handle yet
      {
         winConsole = GetStdHandle(STD_INPUT_HANDLE);
      }
      GetConsoleMode( winConsole, &mode );
      SetConsoleMode( winConsole, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

      while ((key == 0) && (ReadConsoleInput( winConsole, (INPUT_RECORD *) &we, 1, &dummy)))
      {
         switch (we.Type)
         {
            case KEY_EVENT:
               if (we.bKeyDown != 0)
               {
                  int        index =   0;
                  int        asc   =   we.Ascii;
                  BOOL       altGr = ((we.KeyState & 0x01) != 0);
                  BOOL       alt   = ((we.KeyState & 0x03) != 0);
                  BOOL       ctrl  = ((we.KeyState & 0x0c) != 0);
                  BOOL       shft  = ((we.KeyState & 0x10) != 0);

                  switch (we.wVirtualKeyCode)
                  {
                     case VK_SHIFT:
                     case VK_MENU:              // Alt
                     case VK_CONTROL:           // ignore these as single keys
                        break;

                     default:
                        #if defined (DUMP)
                        if (TxaExeSwitch('K'))  // keytrace switch
                        {
                           TxPrint( "Key:%4.4hx down:%4.4hx rep:%4.4hx vKey:%4.4hx "
                                "vScan:%4.4hx Sate:%4.4hx Asc:%4.4hx = %c\n",
                                 we.Type,
                                 we.bKeyDown,
                                 we.wRepeatCount,
                                 we.wVirtualKeyCode,
                                 we.wVirtualScanCode,
                                 we.KeyState,
                                 we.Ascii, TxPrintSafe( (char) we.Ascii));
                        }
                        #endif

                        if ((asc == 0) || ((we.KeyState & ENHANCED_KEY) != 0))
                        {
                           switch (asc)
                           {
                              case 0x00:
                                 switch (we.wVirtualScanCode)
                                 {
                                    case 0x03:  // Ctrl-2
                                       asc = 0x103;
                                       break;

                                    case 0x07:  // Ctrl-6
                                       asc = 0x1e;
                                       break;

                                    case 0x0c:  // Ctrl-MINUS
                                       asc = 0x1f;
                                       break;

                                    case 0x02:  // Ctrl-1
                                    case 0x04:
                                    case 0x05:
                                    case 0x06:  // made 'dead keys'
                                    case 0x08:
                                    case 0x09:
                                    case 0x0a:
                                    case 0x0b:  // Ctrl-0
                                    case 0x0d:  // Ctrl-EQUAL
                                       break;

                                    default:
                                       index = we.wVirtualScanCode & 0xff;
                                       break;
                                 }
                                 break;

                              case 0x0d:        // NUMPAD ENTER
                              case 0x2f:        // NUMPAD /
                                 break;

                              default:          // translate
                                 index = we.wVirtualScanCode & 0xff;
                                 break;
                           }
                        }
                        else if ((ctrl && ((asc >= '0') && (asc <= '9'))) ||
                                 (alt  && ((asc >= '0') && (asc <= '9'))) ||
                                 (alt  && ((asc >= 'a') && (asc <= 'z'))) ||
                                 (asc == 0x09) || (asc == 0x08)           ||
                                 ((ctrl || alt) && (strchr( "-=[]`';\\./,", asc))))
                        {
                           if      (asc == '=') // avoid translate clash = F3
                           {
                              if (alt)
                              {
                                 asc = 0x183;
                              }
                           }
                           else if (asc == ';') // avoid translate clash = F1
                           {
                              if (alt)
                              {
                                 asc = 0x127;
                              }
                           }
                           else                 // use the table
                           {
                              index = asc & 0xff; // translate ascii value
                           }
                        }
                        if ((index > 0) && (index < TXW_ACS_TABLE_SIZE))
                        {
                           if      (alt ) key = txw_winkey[ index].Alt;
                           else if (ctrl) key = txw_winkey[ index].Ctrl;
                           else if (shft) key = txw_winkey[ index].Shift;
                           else           key = txw_winkey[ index].Std;

                           if (altGr && (key == TXa_BACKSLASH))
                           {
                              key = '\\';       // make it regular backslash (German kbd hack)
                           }
                        }
                        else                    // untranslated ascii key
                        {
                           key = asc;
                        }
                        break;
                  }
               }
               break;

            case MOUSE_EVENT:
               #if defined (DUMP)
               if (TxaExeSwitch('K'))        // trace mouse
               {
                  switch (me->Flags)
                  {
                     case DOUBLE_CLICK:  TxPrint( "DClick:"); break;
                     case MOUSE_WHEELED: TxPrint( "mWheel:"); break;
                     case MOUSE_MOVED:   TxPrint( "mMoved:"); break;
                     default:            TxPrint( "Button:"); break;
                  }
                  TxPrint( "  X:% 3hu Y: %3hu  buttons:%8.8lx  Ctrl:%8.8lx flags:%8.8lx\n",
                           me->X, me->Y, me->Button, me->KeyState, me->Flags);
               }
               #endif
               if ((txwa->useMouse) && (event != NULL)) // mouse events wanted ?
               {
                  event->value = 0;
                  switch (me->Flags)
                  {
                     case MOUSE_WHEELED:        // translate to movement keys
                        key = TxWinTranslateWheel( me);
                        break;

                     case MOUSE_MOVED:          // Dragging or simple move
                        event->value = TXm_DRAGGED;
                     case DOUBLE_CLICK:         // Handle DBLCLK by caller!
                     default:                   // BUTTON
                        if ((me->Flags  != MOUSE_MOVED) ||
                            (me->Button != 0))  // no simple moves
                        {
                           event->col    = (short) me->X;
                           event->row    = (short) me->Y;
                           event->state  = me->KeyState;
                           event->value |= me->Button;
                           key = TXW_INPUT_MOUSE;
                        }
                        break;
                  }
               }
               break;

            case FOCUS_EVENT:                   // TxWin injected ASYNC event
               key = TXW_INPUT_ASYNC;
               break;

            default:
               #if defined (DUMP)
               if (TxaExeSwitch('K'))           // keytrace switch
               {
                  TxDisplayHex( "Evt", (char *) &we, 16, 0);
               }
               #endif
               break;
         }
      }
      SetConsoleMode( winConsole, mode );
   #elif defined (UNIX)
      int                 ch;                   // single character read
      int                 st = 0;               // shifstate for Linux console
      int                 nr = 0;               // number of key-characters
      TXTS                ks;                   // assembled key string
      int                 index;

      ch = getch();
      st = TxUnixShiftState();

      switch (ch)
      {
         case TXk_ESCAPE:                       // Escape sequences
         case 0xc2: case 0xc3:
      #if defined (DARWIN)
         case 0xc5: case 0xc6: case 0xcb: case 0xce: case 0xcf:
         case 0xe2: case 0xef:                  // DARWIN specific sequences
      #endif
            memset( ks, 0, TXMAXTS);
            while ((TxUnixKbhit( 5000)) &&      // 5 msec delay between keys
                   (nr < TXMAXTS))
            {                                   // (allowing key repeats upto 20/sec)
               ks[nr++] = (char) ch;            // (assuming average ESC string < 10)
               ch = getch();
            }
            break;

         default:                               // single keys, just pass on
            break;
      }
      ks[nr++] = (char) ch;                     // add final one
      ks[nr]   = 0;                             // make sure it is terminated

      if ((ch = ks[0]) == TXk_ESCAPE)           // Escape string
      {
         if (nr > 1)                            // real-string
         {
            if (debug)
            {
               TxPrint("Escape-seq : ");
               for (index = 1; index <= TXW_KEYS_SIZE; index++)
               {
                  TxPrint("%c", (ks[ index]) ? (ks[ index] != 0x1b) ? ks[ index] : '^' : ' ');
               }
               TxPrint( " =");
               for (index = 1; index <= TXW_KEYS_SIZE; index++)
               {
                  if (ks[ index])
                  {
                     TxPrint(" %2.2hx", ks[ index]);
                  }
                  else
                  {
                     TxPrint( "   ");
                  }
               }
               TxPrint("         ");
            }

            //- to be refined, when multitasking and slow keyboard handling,
            //- there COULD be multiple escape sequences in the string!
            //- would need to handle each one seperately to make type-ahead
            //- more reliable than it is now.
            //- PROBLEM: Only ONE key can be returned each time, so would
            //- need to keep a static FIFO key-buffer to save the rest
            //- for subsequent calls (until empty)
            //- perhaps the ks[] buffer could be used to buffer raw characters
            //- and each time just read a single key or ESC sequence from it.
            //- (and move the rest forward, for the next call)

            //- Since the problem has never been seen (yet), we'll leave it for now
            //- Update, happens with rapid mouse-wheel movement
            //- for now, ignore all but the first one ...

            //- Note: all but FIRST Escape characters are set to 0 (by ..2MouseEvent)

            //- Mouse handling: need to reognize mouse esc sequence, and extract
            //- the button, X and Y coordinates (and perhaps shift-state too)
            //- can not be handled by a table-lookup, needs to be some sort of
            //- pattern matching. See Xterm Control Sequences document for details

            if ((event != NULL) && (TxUnixKstring2MouseEvent( ks + 1, event)))
            {
               switch (event->value)
               {
                  case TXm_WHEELUP:             // translate to movement-key
                     if       ((event->state & TXm_KS_ALT)   != 0)
                     {
                        key = ((event->state & TXm_KS_CTRL)  != 0) ? TXc_LEFT  : TXk_LEFT;
                     }
                     else if  ((event->state & TXm_KS_SHIFT) != 0)
                     {
                        key = ((event->state & TXm_KS_CTRL)  != 0) ? TXc_PGUP  : TXk_PGUP;
                     }
                     else
                     {
                        key = ((event->state & TXm_KS_CTRL)  != 0) ? TXc_UP    : TXk_UP;
                     }
                     break;

                  case TXm_WHEELDN:             // translate to movement-key
                     if       ((event->state & TXm_KS_ALT)   != 0)
                     {
                        key = ((event->state & TXm_KS_CTRL)  != 0) ? TXc_RIGHT : TXk_RIGHT;
                     }
                     else if  ((event->state & TXm_KS_SHIFT) != 0)
                     {
                        key = ((event->state & TXm_KS_CTRL)  != 0) ? TXc_PGDN  : TXk_PGDN;
                     }
                     else
                     {
                        key = ((event->state & TXm_KS_CTRL)  != 0) ? TXc_DOWN  : TXk_DOWN;
                     }
                     break;

                  default:
                     key = TXW_INPUT_MOUSE;     // it was a regular mouse event
                     break;
               }
            }
            else if ((index = TxUnixKstring2Index( ks + 1, &st)) != TXW_INVKEY)
            {
               key = (ULONG) TxUnixIndex2key( index, st);
            }
         }
         else                                   // single Escape
         {
            key = (ULONG) ch;                   // use direct keyvalue (ASCII)
         }
      }
      else                                      // Single key, or BYTE COMBOsequence
      {
         switch (ch)
         {
            case 0x00: case 0x09: case 0x0a: case 0x7f:
               key = (ULONG) TxUnixIndex2key( (int) (ch & 0x0f), st);
               break;

            case 0xc2: case 0xc3:
         #if defined (DARWIN)
            case 0xc5: case 0xc6: case 0xcb: case 0xce: case 0xcf:
            case 0xe2: case 0xef:
         #endif
               if (nr > 1)                      // real-string
               {
                  if (debug)
                  {
                     TxPrint("BYTE-Combo : ");
                     for (index = 0; index < nr; index++)
                     {
                        TxPrint("%2.2hhx ", ks[ index]);
                     }
                     TxPrint("%*.*s", (43 - index * 3), (43 - index * 3), " ");
                  }
                  if ((index = TxByteComboKstring2Index( ks, nr, &st)) != TXW_INVKEY)
                  {
                     key = (ULONG) TxUnixIndex2key( index, st);
                  }
               }
               else
               {
                  key = (ULONG) ch;             // use direct keyvalue (ASCII)
               }
               break;

            default:
               key = (ULONG) ch;                // use direct keyvalue (ASCII)
               break;
         }
      }
      if ((debug) && (nr == 1))
      {
         TxPrint("Atomic-key :    %*.*s", 40, 40, " ");
      }
   #elif defined (DEV32)
      TXW_INPUT_EVENT  evtData;

      TxOS2GetQueueEvent( &evtData);

      key = evtData.key;                        // separate the keycode
      if (event != NULL)                        // other data wanted ?
      {
         *event = evtData;
      }
   #else                                        // std DOS handling
      union  REGS      regs;

      if ((txw_mOus == TXDX_MOUSE_PRESENT)  &&  // mouse present and
          (txwa->useMouse) && (event != NULL))  // mouse input wanted ?
      {
         TxxClearReg( regs);
         TxxMouseInt( regs, TXDX_MOUSE_SHOW);   // Show mouse cursor
         while (key == 0)
         {
            if (kbhit())
            {
               key = getch();                   // first (or only) keyvalue
               switch (key)
               {
                  case 0x00:
                  case 0xe0: key = TXW_KEY_GROUP_1 + getch();  break;
                  default:                                     break;
               }
            }
            else                                // check for mouse-event
            {
               if (txw_mCached)                 // cached (drag) event ?
               {
                  txw_mCached = FALSE;
                  *event  = txw_mEvt;
                  key = TXW_INPUT_MOUSE;
               }
               else                             // check for new changes ...
               {
                  TxxClearReg( regs);
                  TxxMouseInt( regs, TXDX_MOUSE_STATUS); // position & buttons

                  if ((TXWORD.bx != txw_mBut) || // if any change ...
                      (TXWORD.cx != txw_mCol) ||
                      (TXWORD.dx != txw_mRow)  )
                  {
                     if ((TXWORD.bx != 0) ||    // any button down (drag)
                         (TXWORD.bx != txw_mBut)) // or button state change
                     {
                        union  REGS  kbd;

                        TxxClearReg( kbd);
                        TxxKeyBdInt( kbd, TXDX_KBD_SHIFTSTATUS);
                        event->state                       = TXm_KS_NONE;
                        if (kbd.h.al & 0x03) event->state |= TXm_KS_SHIFT;
                        if (kbd.h.al & 0x04) event->state |= TXm_KS_CTRL;
                        if (kbd.h.al & 0x08) event->state |= TXm_KS_ALT;
                        if (kbd.h.al & 0x10) event->state |= TXm_KS_SCRLK;
                        if (kbd.h.al & 0x20) event->state |= TXm_KS_NUMLK;

                        event->col    = (short) TXWORD.cx / 8;
                        event->row    = (short) TXWORD.dx / 8;
                        event->value  = (ULONG) TXWORD.bx;

                        if (TXWORD.bx == txw_mBut) // must be a drag ...
                        {
                           event->value |= TXm_DRAGGED;
                           if (txw_mDragged == FALSE) // this is START drag
                           {
                              txw_mDragged = TRUE;
                              txw_mCached  = TRUE;
                              txw_mEvt     = *event; // cache this event

                              //- first send it using previous col/row
                              //- which is the real start-drag position
                              event->col    = (short) txw_mCol / 8;
                              event->row    = (short) txw_mRow / 8;
                           }
                        }
                        else if (TXWORD.bx == 0) // button up, end drag
                        {
                           txw_mDragged = FALSE;
                        }
                        key = TXW_INPUT_MOUSE;
                     }
                     txw_mBut = TXWORD.bx;      // update status
                     txw_mCol = TXWORD.cx;
                     txw_mRow = TXWORD.dx;

                     #if defined (DUMP)
                     if (TxaExeSwitch('K'))     // trace mouse
                     {
                        TxPrint( "  X:% 3hu Y: %3hu  buttons:%4.4hx\n",
                                 txw_mCol / 8, txw_mRow / 8, txw_mBut);
                     }
                     #endif
                  }
               }
            }
         }
         TxxClearReg( regs);
         TxxMouseInt( regs, TXDX_MOUSE_HIDE);   // Hide, avoid screen damage
      }
      else                                      // just get next keystroke
      {
         key = getch();                         // first (or only) keyvalue
         switch (key)
         {
            case 0x00:
            case 0xe0: key = TXW_KEY_GROUP_1 + getch();  break;
            default:                                     break;
         }
      }
   #endif
   if (key == TXK_ESCAPE)
   {
      TxSetPendingAbort();                      // signal abort from current
   }                                            // function is requested ...
   if (event != NULL)                           // extended info wanted ?
   {
      event->key = key;                         // key in event structure too
      event->tmr = TxTmrGetNanoSecFromStart();  // add timestamp (DBLCLK etc)
   }
   return( key);
}                                               // end 'txwGetInputEvent'
/*---------------------------------------------------------------------------*/


#if defined (USEWINDOWING)

#if defined (HAVETHREADS)

/*****************************************************************************/
// Abort synchronious input (wait) to pickup async event in queue
/*****************************************************************************/
void txwNotifyAsyncInput
(
   void
)
{
   ENTER();

   #if defined (WIN32)
      if (winConsole != (HANDLE) TXW_INVALID)   // kbd handle available
      {
         TXW_KEY_EVENT    we;
         ULONG            dummy;

         we.Type = FOCUS_EVENT;                 // TxWin injected ASYNC event
                                                // to unblock
         if (!WriteConsoleInput( winConsole, (INPUT_RECORD *) &we, 1, &dummy))
         {
            TRACES(( "WriteConsole failure, rc: %lu\n", GetLastError()));
         }
      }
   #elif defined (DOS32)
   #elif defined (UNIX)
   #else
      {
         TXW_INPUT_EVENT     event;

         event.key = TXW_INPUT_ASYNC;           // signal async
         TxOS2AddQueueEvent( &event);
      }
   #endif

   VRETURN ();
}                                               // end 'txwNotifyAsyncInput'
/*---------------------------------------------------------------------------*/

#endif

// Read next message from queue, and optionaly remove it from the queue
static BOOL txwReadQueueMsg                     // RET   FALSE if queue empty
(
   TXWQMSG            *qmsg,                    // OUT   message packet
   BOOL                peek                     // IN    peek only, no remove
);

// message-que must be rather big to allow large dialogs where a lot of
// fields are created (each posting a msg) without servicing the queue
#define TXWQUEUESIZE   256                      // size of msg-queue

#define TXWQNext(x)    ((x+1) % TXWQUEUESIZE)   // next index in circle

static TXWQMSG        txwQueueBuf[TXWQUEUESIZE];
static ULONG          txwQueueHead  = 0;        // head of queue (next msg)
static ULONG          txwQueueTail  = 0;        // tail of queue (next free)


/*****************************************************************************/
// Get next message from queue if available, get keyboard msg otherwise
/*****************************************************************************/
BOOL txwGetMsg                                  // RET   FALSE if QUIT/RESIZE
(
   TXWQMSG            *qmsg                     // OUT   message packet
)
{
   BOOL                rc = TRUE;
   BOOL                async_input_received;
#if defined (HAVEMOUSE)
   static ULN64        btnUpTimer = 0;          // for double-click detection
   static short        lastXpos   = 0;          // for double-click and DRAG
   static short        lastYpos   = 0;          // including interpolation
#endif

   ENTER();
   do
   {
      async_input_received = FALSE;
      if (txwReadQueueMsg( qmsg, FALSE) == FALSE) // nothing queued
      {
         ULONG            input;
         TXW_INPUT_EVENT  mouse;

         if (txwa->typeahead == FALSE)          // no typeahead wanted
         {
            while (kbhit())
            {
               txwGetInputEvent( FALSE, NULL);  // read and discard type-ahead
            }
         }
         input = txwGetInputEvent( FALSE, &mouse); // wait for an event
         if (txwa->sbview)
         {
            txwPaintWinStatus( txwa->sbview, "", cSchemeColor); // reset user message
         }
      #if defined (UNIX)                        // to be refined, make generic (WIN32) ?
         if (TxUnixScreenSizeInValid())         // has screen-size changed ?
         {
            //- signal screenresize to DESKTOP message-loop so it can perform a 'mode' command to re-init

            qmsg->hwnd = TXHWND_DESKTOP;
            qmsg->msg  = TXWM_SCREENRESIZE;
            qmsg->mp1  = 0;
            qmsg->mp2  = 0;
         }
         else
      #endif
      #if defined (HAVEMOUSE)
         if (input == TXW_INPUT_MOUSE)
         {
            short      thisRow = mouse.row;     // needed to update the 'last' values AFTER
            short      thisCol = mouse.col;     // the Interpolate may have changed them!

            if ((qmsg->hwnd = txwQueryCapture()) == TXHWND_NULL)
            {
               qmsg->hwnd = txwTopWindowAtPos( mouse.row, mouse.col);
            }
            if      (mouse.value & TXm_DRAGGED)
            {
               //- On some environments, the first DRAGGED event is on the same as where the button was clicked
               if ((lastXpos == mouse.col) && (lastYpos == mouse.row)) // still on same cell as button-down
               {
                  qmsg->msg  = TXWM_STARTDRAG;  // different message, can either be processed or ignored
               }
               else
               {
                  qmsg->msg  = TXWM_MOUSEMOVE;

                  //- DRAG-filter: Check against lastXpos being adjacent, when NOT (jumping overs rows)
                  //- queue one MSG for every row in between (end at mouse.row) and patch-up mouse.row/col
                  //- to be the FIRST step from the previous position (and not the final destination)

                  if (abs( mouse.row - lastYpos) > 1)
                  {
                     txwMouseInterpolateMovement( qmsg->hwnd, lastXpos, lastYpos, &mouse);
                  }
               }
            }
            else if (mouse.value == 0)          // button up again
            {
               qmsg->msg  = TXWM_BUTTONUP;
               btnUpTimer = TxTmrSetTimer( TMRMSEC( 250));
            }
            else if ((TxTmrTimerExpired( btnUpTimer) == FALSE) &&
                      (lastXpos == mouse.col) && (lastYpos == mouse.row))
            {
               qmsg->msg  = TXWM_BUTTONDBLCLK;  // quick 2nd button-down
            }
            else
            {
               qmsg->msg  = TXWM_BUTTONDOWN;
            }
            lastXpos   = thisCol;               // needed for dblclk and startdrag detect
            lastYpos   = thisRow;
            qmsg->mp1  = TXMPFROM2SH( mouse.col,   mouse.row);
            qmsg->mp2  = TXMPFROM2SH( mouse.value, mouse.state);
         }
         else
      #endif
         if (input == TXW_INPUT_ASYNC)          // msg queued by other thread
         {
            async_input_received = TRUE;        // re-read the msg-queue
         }
         else                                   // keyboard, to FOCUS window
         {
            qmsg->hwnd = (TXWHANDLE) txwa->focus;
            qmsg->msg  = TXWM_CHAR;
            qmsg->mp1  = 0;
            qmsg->mp2  = input;
         }
      }
   } while (async_input_received);

   if ((qmsg->msg == TXWM_QUIT) || (qmsg->msg == TXWM_SCREENRESIZE))
   {
      rc = FALSE;
   }
   TRCMSG( qmsg->hwnd, qmsg->msg, qmsg->mp1, qmsg->mp2);
   if (qmsg->msg == TXWM_CHAR)
   {
      txwTranslateAccel( qmsg->hwnd, qmsg);
   }
   BRETURN( rc);
}                                               // end 'txwGetMsg'
/*---------------------------------------------------------------------------*/


#if defined (HAVEMOUSE)
/*****************************************************************************/
// Create series of messages to assure a continuous Y-range for mouse movement
// Returns X,Y for first point (current MSG) and Posts messages for the rest
/*****************************************************************************/
static void txwMouseInterpolateMovement
(
   TXWHANDLE           hwnd,                    // IN    destination window
   short               xFrom,                   // IN    starting X pos (col)
   short               yFrom,                   // IN    starting Y pos (row)
   TXW_INPUT_EVENT    *msTo                     // INOUT mouse event (TO pos)
)                                               // col,row updated to 1st step
{
   LONG                steps  =  abs( msTo->row - yFrom);
   LONG                yStep  =      (msTo->row > yFrom) ? 1 : -1;
   LONG                yDelta =       msTo->row - yFrom;
   LONG                xDelta =       msTo->col - xFrom;
   LONG                x,y;                     // coordinates for this step
   LONG                s;                       // step number 1 .. steps

   ENTER();
   TRACES(("Steps:%d   From:%hd,%hd  To:%hd,%hd  Y-step:%d\n", steps, xFrom, yFrom, msTo->col, msTo->row, yStep));

   for ( s = 1; s <= steps; s++)
   {
      y = yFrom + (s * yStep);
      x = xFrom + (s * yStep) * xDelta / yDelta;

      TRACES(("step:%3d x:%4d y:%4d (%s)\n", s, x, y, (s == 1) ? "THIS msg" : "POST msg"));

      if (s == 1)                               // first step, return in msTo event
      {
         msTo->col = x;                         // update THIS event msg to just do step 1
         msTo->row = y;                         // not the complete distance to endpoint
      }
      else                                      // Post as message to be handled next
      {
         if (!txwPostMsg( hwnd, TXWM_MOUSEMOVE,
                                TXMPFROM2SH( x,           y),
                                TXMPFROM2SH( msTo->value, msTo->state)))
         {
            break;
         }
      }
   }
   VRETURN();
}                                               // end 'txwMouseInterpolateMovement'
/*---------------------------------------------------------------------------*/
#endif


/*****************************************************************************/
// Dispatch a message
/*****************************************************************************/
ULONG txwDispatchMsg                            // RET   result
(
   TXWQMSG            *qmsg                     // IN    message packet
)
{
   ULONG               rc  = NO_ERROR;

   ENTER();

   rc = txwSendMsg( qmsg->hwnd, qmsg->msg, qmsg->mp1, qmsg->mp2);
   RETURN( rc);
}                                               // end 'txwDispatchMsg'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Send a message to specified window, 0 hwnd ill be discarded!
/*****************************************************************************/
ULONG txwSendMsg                                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    destination window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINPROC           winproc;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, NULL)) != NULL)
   {
      if ((wnd->winproc    != NULL) &&          // APP specific procedure
          (txwa->arrowMode == TXW_ARROW_STD))   // and no arrow mode active
      {
         winproc = wnd->winproc;
      }
      else                                      // default, no APP override
      {
         winproc = txwDefWindowProc;
      }
      TRCMSG( hwnd, msg, mp1, mp2);
      TRCLAS( "SendMsg", hwnd);
      rc = (winproc)( (TXWHANDLE) wnd, msg, mp1, mp2);
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwSendMsg'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Post a message to specified window, 0 hwnd can be intercepted at GetMsg
/*****************************************************************************/
BOOL txwPostMsg                                 // RET   message posted
(
   TXWHANDLE           hwnd,                    // IN    destination window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   BOOL                rc  = FALSE;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   TRCLAS( "PostMsg", hwnd);
   if (TXWQNext(txwQueueTail) != txwQueueHead)  // not full yet
   {
      txwQueueBuf[txwQueueTail].hwnd = hwnd;
      txwQueueBuf[txwQueueTail].msg  = msg;
      txwQueueBuf[txwQueueTail].mp1  = mp1;
      txwQueueBuf[txwQueueTail].mp2  = mp2;
      txwQueueTail = TXWQNext(txwQueueTail);
      rc = TRUE;
   }
   else
   {
      TRACES(("WARNING: %smessage queue is full%s!\n", CBR, CNN));
   }
   BRETURN( rc);
}                                               // end 'txwPostMsg'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Make messages for specified window invalid (on destroy)
/*****************************************************************************/
void txwDestroyMessages
(
   TXWHANDLE           hwnd                     // IN    destination window
)
{
   ULONG               index;

   ENTER();
   TRACES(("ZAP msg-queue hwnd for %8.8lx to %8.8lx\n", hwnd, TXHWND_INVALID));

   for (index = 0; index < TXWQUEUESIZE; index++)
   {
      if (txwQueueBuf[index].hwnd == hwnd)
      {
         TRCMSG( txwQueueBuf[index].hwnd, txwQueueBuf[index].msg,
                 txwQueueBuf[index].mp1,  txwQueueBuf[index].mp2);
         TRCLAS( "DestroyMessages", hwnd);

         txwQueueBuf[index].hwnd = TXHWND_INVALID;
      }
   }
   VRETURN ();
}                                               // end 'txwDestroyMessages'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Check if specified message is queued for a window handle
/*****************************************************************************/
BOOL txwQueueMsgPresent                         // RET   FALSE if no msg there
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               message                  // IN    message id
)
{
   BOOL                rc = FALSE;
   ULONG               this = txwQueueHead;

   if (this != txwQueueTail)                    // not empty
   {
      do
      {
         if ((txwQueueBuf[this].hwnd == hwnd)  &&
             (txwQueueBuf[this].msg  == message))
         {
            rc = TRUE;                          // message found
            break;                              // out of loop
         }
         else                                   // to next message
         {
            this = TXWQNext(this);
         }
      } while (this != txwQueueTail);
   }
   return( rc);
}                                               // end 'txwQueueMsgPresent'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Read next message from queue, and optionaly remove it from the queue
/*****************************************************************************/
static BOOL txwReadQueueMsg                     // RET   FALSE if queue empty
(
   TXWQMSG            *qmsg,                    // OUT   message packet
   BOOL                peek                     // IN    peek only, no remove
)
{
   BOOL                rc = FALSE;

   if (txwQueueTail != txwQueueHead)            // not empty
   {
      *qmsg = txwQueueBuf[txwQueueHead];
      txwQueueBuf[txwQueueHead].hwnd = TXHWND_INVALID; // nullify message
      txwQueueHead = TXWQNext(txwQueueHead);    // avoiding confusing trace
      rc = TRUE;                                // with DestroyMessages()
   }
   return( rc);
}                                               // end 'txwReadQueueMsg'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set command-code for specified key; 0 = clear
/*****************************************************************************/
ULONG txwSetAccelerator                         // RET   function result
(
   TXWHANDLE           hwnd,                    // IN    handle or 0 for system
   ULONG               key,                     // IN    keycode 0 .. TXk_MAX
   ULONG               cmd                      // IN    command code
)
{
   ULONG               rc  = NO_ERROR;          // function return
   ULONG             **pat;                     // ptr to accel table
   TXWINBASE          *wnd;

   ENTER();

   TRACES(( "hwnd:%8.8lx key: 0x%3.3hx ==> command: %8.8lx = %lu\n",
             hwnd, key, cmd, cmd));

   if ((wnd = txwValidateHandle( hwnd, NULL)) != NULL)
   {
      pat = &(wnd->acceltable);
   }
   else                                         // use system table
   {
      pat = &(txwa->acceltable);
   }
   if ((*pat) == NULL)                          // need to allocate memory
   {
      *pat = TxAlloc( TXk_MAX +1, sizeof(ULONG));
      TRACES(("New table for %lu entries at:%8.8lx\n", TXk_MAX +1, (*pat)));
   }
   if ((*pat) != NULL)
   {
      (*pat)[key & TXk_MAX] = cmd;              // assign cmd code
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   RETURN (rc);
}                                               // end 'txwSetAccelerator'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Translate WM_CHAR message to WM_COMMAND when defined as accelerator
/*****************************************************************************/
BOOL txwTranslateAccel                          // RET   msg is translated
(
   TXWHANDLE           hwnd,                    // IN    handle or 0 for system
   TXWQMSG            *qmsg                     // INOUT message packet
)
{
   BOOL                rc = FALSE;              // function return
   TXWINBASE          *wnd;
   ULONG              *table = NULL;
   ULONG               cmd   = 0;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, NULL)) != NULL)
   {
      if ((wnd->window->class != TXW_ENTRYFIELD) || // disable accelerators in
          (wnd->window->ef.curpos == 0)          || // range 0x20 to 0x7f when
          (qmsg->mp2 > 0x80) || (qmsg->mp2 < 0x20)) // not at start of entryf
      {
         if ((table = wnd->acceltable) != NULL) // window specific table ?
         {
            cmd = table[qmsg->mp2 & TXk_MAX];
         }
      }
   }
   if ((cmd == 0) && ((table = txwa->acceltable) != NULL))
   {
      cmd = table[qmsg->mp2 & TXk_MAX];
   }
   if ((cmd != 0) && (cmd != TXWACCEL_BLOCK))   // valid translation ?
   {
      if (cmd == TXWACCEL_MHELP)                // translate to WM_HELP
      {
         qmsg->msg = TXWM_HELP;
         qmsg->mp1 = 0;
      }
      else
      {
         qmsg->msg = TXWM_COMMAND;
         qmsg->mp1 = cmd;
      }
      qmsg->mp2 = TXCMDSRC_ACCELERATOR;
      TRACES(("Translated using %s acceltable to WM_%s: %8.8lx = %lu\n",
               (table == txwa->acceltable) ? "SYSTEM" : "WINDOW",
               (cmd   == TXWACCEL_MHELP)   ? "HELP"   : "COMMAND",
                qmsg->mp1, qmsg->mp1));

      txwa->arrowMode = TXW_ARROW_STD;          // end pending arrow mode
      txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, 0, 0);

      rc = TRUE;
   }
   else
   {
      TRACES(("Untranslated: Key:0x%3.3lx, cmd:%ld\n", qmsg->mp2, cmd));
   }
   BRETURN (rc);
}                                               // end 'txwTranslateAccel'
/*---------------------------------------------------------------------------*/

#endif                                          // USEWINDOWING
