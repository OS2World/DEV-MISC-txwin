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

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface


typedef struct txw_acskeystate
{
   USHORT              Std;
   USHORT              Shift;
   USHORT              Ctrl;
   USHORT              Alt;
} TXW_ACSKEYSTATE;                           // end of struct "txw_acskeystate"


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

#define TXW_ACS_TABLE_SIZE 0x20                 // Alt/Ctrl/Shift table size
#define TXW_INVKEY        ((int) -1)            // invalid key index

// implied keyboard shift-state values
#define  TXW_SST_STND             0x00          // use table column 'unshifted'
#define  TXW_SST_SHFT             0x01          // use table column 'shifted'
#define  TXW_SST_ALTK             0x02          // use table column 'alt'
#define  TXW_SST_CTRL             0x04          // use table column 'control'
#define  TXW_SST_KEYB             0x08          // use shiftstate from keyboard

//- Note: shiftstate only available in a real Linux CONSOLE session, no XTERM
typedef struct txw_linuxkbd
{
   char                es[6];                   // Esc string, except 1st char
   unsigned char       base;                    // base key-value
   unsigned char       state;                   // implied shift-state
} TXW_LINUXKBD;                                 // end of struct "txw_linuxkbd"

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

//- Table of LINUX keyboard strings, starting with Escape, resolves index plus
//- shifstate to get the TX-code from a second table.
static TXW_LINUXKBD txw_linuxkey[] =
{
   //-- K-string from 2nd byte   Index St   Tx-code TX-description
   {{0x09,   0,   0,   0,   0,   0},0x09,TXW_SST_SHFT}, // 0x10f TXs_TAB
   {{0x0a,   0,   0,   0,   0,   0},0x0a,TXW_SST_ALTK}, // 0x11c TXa_ENTER
   {{0x1b,0x5b,0x32,0x7e,   0,   0},0x14,TXW_SST_ALTK}, // 0x1a2 TXa_INSERT
   {{0x1b,0x5b,0x33,0x7e,   0,   0},0x10,TXW_SST_ALTK}, // 0x1a3 TXa_DELETE
   {{0x1b,0x5b,0x35,0x7e,   0,   0},0x17,TXW_SST_ALTK}, // 0x199 TXa_PGUP
   {{0x1b,0x5b,0x36,0x7e,   0,   0},0x16,TXW_SST_ALTK}, // 0x1a1 TXa_PGDN
   {{0x1b,0x5b,0x41,   0,   0,   0},0x19,TXW_SST_ALTK}, // 0x198 TXa_UP
   {{0x1b,0x5b,0x42,   0,   0,   0},0x11,TXW_SST_ALTK}, // 0x1a0 TXa_DOWN
   {{0x1b,0x5b,0x43,   0,   0,   0},0x18,TXW_SST_ALTK}, // 0x19d TXa_RIGHT
   {{0x1b,0x5b,0x44,   0,   0,   0},0x15,TXW_SST_ALTK}, // 0x19b TXa_LEFT
   {{0x1b,0x5b,0x46,   0,   0,   0},0x12,TXW_SST_ALTK}, // 0x19f TXa_END
   {{0x1b,0x5b,0x48,   0,   0,   0},0x13,TXW_SST_ALTK}, // 0x197 TXa_HOME
   {{0x27,   0,   0,   0,   0,   0},0x08,TXW_SST_ALTK}, // 0x128 TXa_QUOTE
   {{0x2c,   0,   0,   0,   0,   0},0x00,TXW_SST_SHFT}, // 0x133 TXa_COMMA
   {{0x2d,   0,   0,   0,   0,   0},0x08,TXW_SST_SHFT}, // 0x182 TXa_MINUS
   {{0x2e,   0,   0,   0,   0,   0},0x09,TXW_SST_ALTK}, // 0x134 TXa_DOT
   {{0x2f,   0,   0,   0,   0,   0},0x07,TXW_SST_ALTK}, // 0x135 TXa_SLASH
   {{0x30,   0,   0,   0,   0,   0},0x0b,TXW_SST_STND}, // 0x181 TXa_0
   {{0x31,   0,   0,   0,   0,   0},0x0c,TXW_SST_STND}, // 0x178 TXa_1
   {{0x32,   0,   0,   0,   0,   0},0x0d,TXW_SST_STND}, // 0x179 TXa_2
   {{0x33,   0,   0,   0,   0,   0},0x0b,TXW_SST_SHFT}, // 0x17a TXa_3
   {{0x34,   0,   0,   0,   0,   0},0x0c,TXW_SST_SHFT}, // 0x17b TXa_4
   {{0x35,   0,   0,   0,   0,   0},0x0d,TXW_SST_SHFT}, // 0x17c TXa_5
   {{0x36,   0,   0,   0,   0,   0},0x0b,TXW_SST_ALTK}, // 0x17d TXa_6
   {{0x37,   0,   0,   0,   0,   0},0x0c,TXW_SST_ALTK}, // 0x17e TXa_7
   {{0x38,   0,   0,   0,   0,   0},0x0b,TXW_SST_CTRL}, // 0x17f TXa_8
   {{0x39,   0,   0,   0,   0,   0},0x0c,TXW_SST_CTRL}, // 0x180 TXa_9
   {{0x3b,   0,   0,   0,   0,   0},0x06,TXW_SST_ALTK}, // 0x127 TXa_SEMICOLON
   {{0x3d,   0,   0,   0,   0,   0},0x0a,TXW_SST_SHFT}, // 0x183 TXa_EQUAL
   {{0x4f,0x32,0x50,   0,   0,   0},0x1a,TXW_SST_SHFT}, // 0x154 TXs_F1
   {{0x4f,0x32,0x51,   0,   0,   0},0x1a,TXW_SST_ALTK}, // 0x155 TXs_F2
   {{0x4f,0x32,0x52,   0,   0,   0},0x1b,TXW_SST_SHFT}, // 0x156 TXs_F3
   {{0x4f,0x32,0x53,   0,   0,   0},0x1b,TXW_SST_ALTK}, // 0x157 TXs_F4
   {{0x4f,0x50,   0,   0,   0,   0},0x1a,TXW_SST_STND}, // 0x13b TXk_F1
   {{0x4f,0x51,   0,   0,   0,   0},0x1a,TXW_SST_CTRL}, // 0x13c TXk_F2
   {{0x4f,0x52,   0,   0,   0,   0},0x1b,TXW_SST_STND}, // 0x13d TXk_F3
   {{0x4f,0x53,   0,   0,   0,   0},0x1b,TXW_SST_CTRL}, // 0x13e TXk_F4
   {{0x5b,   0,   0,   0,   0,   0},0x08,TXW_SST_STND}, // 0x11a TXa_LBRACKET
   {{0x5b,0x31,0x35,0x7e,   0,   0},0x1c,TXW_SST_STND}, // 0x13f TXk_F5
   {{0x5b,0x31,0x35,0x3b,0x32,0x7e},0x1c,TXW_SST_SHFT}, // 0x158 TXs_F5
   {{0x5b,0x31,0x37,0x7e,   0,   0},0x1c,TXW_SST_CTRL}, // 0x140 TXk_F6
   {{0x5b,0x31,0x37,0x3b,0x32,0x7e},0x1c,TXW_SST_ALTK}, // 0x159 TXs_F6
   {{0x5b,0x31,0x38,0x7e,   0,   0},0x1d,TXW_SST_STND}, // 0x141 TXk_F7
   {{0x5b,0x31,0x38,0x3b,0x32,0x7e},0x1d,TXW_SST_SHFT}, // 0x15a TXs_F7
   {{0x5b,0x31,0x39,0x7e,   0,   0},0x1d,TXW_SST_CTRL}, // 0x142 TXk_F8
   {{0x5b,0x31,0x39,0x3b,0x32,0x7e},0x1d,TXW_SST_ALTK}, // 0x15b TXs_F8
   {{0x5b,0x31,0x7e,   0,   0,   0},0x13,TXW_SST_KEYB}, // 0x197 TX*_HOME
   {{0x5b,0x32,0x30,0x7e,   0,   0},0x1e,TXW_SST_STND}, // 0x143 TXk_F9
   {{0x5b,0x32,0x30,0x3b,0x32,0x7e},0x1e,TXW_SST_SHFT}, // 0x15c TXs_F9
   {{0x5b,0x32,0x31,0x7e,   0,   0},0x1e,TXW_SST_CTRL}, // 0x144 TXk_F10
   {{0x5b,0x32,0x31,0x3b,0x32,0x7e},0x1e,TXW_SST_ALTK}, // 0x15d TXs_F10
   {{0x5b,0x32,0x33,0x7e,   0,   0},0x1f,TXW_SST_STND}, // 0x185 TXk_F11
   {{0x5b,0x32,0x33,0x3b,0x32,0x7e},0x1f,TXW_SST_SHFT}, // 0x187 TXs_F11
   {{0x5b,0x32,0x34,0x7e,   0,   0},0x1f,TXW_SST_CTRL}, // 0x186 TXk_F12
   {{0x5b,0x32,0x34,0x3b,0x32,0x7e},0x1f,TXW_SST_ALTK}, // 0x188 TXs_F12
   {{0x5b,0x32,0x35,0x7e,   0,   0},0x1a,TXW_SST_SHFT}, // 0x154 TXs_F1
   {{0x5b,0x32,0x36,0x7e,   0,   0},0x1a,TXW_SST_ALTK}, // 0x155 TXs_F2
   {{0x5b,0x32,0x38,0x7e,   0,   0},0x1b,TXW_SST_SHFT}, // 0x156 TXs_F3
   {{0x5b,0x32,0x39,0x7e,   0,   0},0x1b,TXW_SST_ALTK}, // 0x157 TXs_F4
   {{0x5b,0x32,0x7e,   0,   0,   0},0x14,TXW_SST_KEYB}, // 0x1a2 TX*_INSERT
   {{0x5b,0x33,0x31,0x7e,   0,   0},0x1c,TXW_SST_SHFT}, // 0x158 TXs_F5
   {{0x5b,0x33,0x32,0x7e,   0,   0},0x1c,TXW_SST_ALTK}, // 0x159 TXs_F6
   {{0x5b,0x33,0x33,0x7e,   0,   0},0x1d,TXW_SST_SHFT}, // 0x15a TXs_F7
   {{0x5b,0x33,0x34,0x7e,   0,   0},0x1d,TXW_SST_ALTK}, // 0x15b TXs_F8
   {{0x5b,0x33,0x7e,   0,   0,   0},0x10,TXW_SST_KEYB}, // 0x1a3 TX*_DELETE
   {{0x5b,0x34,0x7e,   0,   0,   0},0x12,TXW_SST_KEYB}, // 0x19f TX*_END
   {{0x5b,0x35,0x7e,   0,   0,   0},0x17,TXW_SST_KEYB}, // 0x199 TX*_PGUP
   {{0x5b,0x36,0x7e,   0,   0,   0},0x16,TXW_SST_KEYB}, // 0x1a1 TX*_PGDN
   {{0x5b,0x41,   0,   0,   0,   0},0x19,TXW_SST_KEYB}, // 0x198 TX*_UP
   {{0x5b,0x42,   0,   0,   0,   0},0x11,TXW_SST_KEYB}, // 0x1a0 TX*_DOWN
   {{0x5b,0x43,   0,   0,   0,   0},0x18,TXW_SST_KEYB}, // 0x19d TX*_RIGHT
   {{0x5b,0x44,   0,   0,   0,   0},0x15,TXW_SST_KEYB}, // 0x19b TX*_LEFT
   {{0x5b,0x46,   0,   0,   0,   0},0x12,TXW_SST_STND}, // 0x14f TXk_END
   {{0x5b,0x48,   0,   0,   0,   0},0x13,TXW_SST_STND}, // 0x147 TXk_HOME
   {{0x5b,0x5b,0x41,   0,   0,   0},0x1a,TXW_SST_STND}, // 0x13b TXk_F1
   {{0x5b,0x5b,0x42,   0,   0,   0},0x1a,TXW_SST_CTRL}, // 0x13c TXk_F2
   {{0x5b,0x5b,0x43,   0,   0,   0},0x1b,TXW_SST_STND}, // 0x13d TXk_F3
   {{0x5b,0x5b,0x44,   0,   0,   0},0x1b,TXW_SST_CTRL}, // 0x13e TXk_F4
   {{0x5b,0x5b,0x45,   0,   0,   0},0x1c,TXW_SST_STND}, // 0x13f TXk_F5
   {{0x5c,   0,   0,   0,   0,   0},0x00,TXW_SST_CTRL}, // 0x12b TXa_BACKSLASH
   {{0x5d,   0,   0,   0,   0,   0},0x08,TXW_SST_CTRL}, // 0x11b TXa_RBRACKET
   {{0x60,   0,   0,   0,   0,   0},0x00,TXW_SST_ALTK}, // 0x129 TXa_BACKQUOTE
   {{0x61,   0,   0,   0,   0,   0},0x01,TXW_SST_STND}, // 0x11e TXa_A
   {{0x62,   0,   0,   0,   0,   0},0x02,TXW_SST_STND}, // 0x130 TXa_B
   {{0x63,   0,   0,   0,   0,   0},0x03,TXW_SST_STND}, // 0x12e TXa_C
   {{0x64,   0,   0,   0,   0,   0},0x04,TXW_SST_STND}, // 0x120 TXa_D
   {{0x65,   0,   0,   0,   0,   0},0x05,TXW_SST_STND}, // 0x112 TXa_E
   {{0x66,   0,   0,   0,   0,   0},0x06,TXW_SST_STND}, // 0x121 TXa_F
   {{0x67,   0,   0,   0,   0,   0},0x07,TXW_SST_STND}, // 0x122 TXa_G
   {{0x68,   0,   0,   0,   0,   0},0x01,TXW_SST_SHFT}, // 0x123 TXa_H
   {{0x69,   0,   0,   0,   0,   0},0x02,TXW_SST_SHFT}, // 0x117 TXa_I
   {{0x6a,   0,   0,   0,   0,   0},0x03,TXW_SST_SHFT}, // 0x124 TXa_J
   {{0x6b,   0,   0,   0,   0,   0},0x04,TXW_SST_SHFT}, // 0x125 TXa_K
   {{0x6c,   0,   0,   0,   0,   0},0x05,TXW_SST_SHFT}, // 0x126 TXa_L
   {{0x6d,   0,   0,   0,   0,   0},0x06,TXW_SST_SHFT}, // 0x132 TXa_M
   {{0x6e,   0,   0,   0,   0,   0},0x07,TXW_SST_SHFT}, // 0x131 TXa_N
   {{0x6f,   0,   0,   0,   0,   0},0x01,TXW_SST_CTRL}, // 0x118 TXa_O
   {{0x70,   0,   0,   0,   0,   0},0x02,TXW_SST_CTRL}, // 0x119 TXa_P
   {{0x71,   0,   0,   0,   0,   0},0x03,TXW_SST_CTRL}, // 0x110 TXa_Q
   {{0x72,   0,   0,   0,   0,   0},0x04,TXW_SST_CTRL}, // 0x113 TXa_R
   {{0x73,   0,   0,   0,   0,   0},0x05,TXW_SST_CTRL}, // 0x11f TXa_S
   {{0x74,   0,   0,   0,   0,   0},0x06,TXW_SST_CTRL}, // 0x114 TXa_T
   {{0x75,   0,   0,   0,   0,   0},0x07,TXW_SST_CTRL}, // 0x116 TXa_U
   {{0x76,   0,   0,   0,   0,   0},0x01,TXW_SST_ALTK}, // 0x12f TXa_V
   {{0x77,   0,   0,   0,   0,   0},0x02,TXW_SST_ALTK}, // 0x111 TXa_W
   {{0x78,   0,   0,   0,   0,   0},0x03,TXW_SST_ALTK}, // 0x12d TXa_X
   {{0x79,   0,   0,   0,   0,   0},0x04,TXW_SST_ALTK}, // 0x115 TXa_Y
   {{0x7a,   0,   0,   0,   0,   0},0x05,TXW_SST_ALTK}, // 0x12c TXa_Z
   {{0x7f,   0,   0,   0,   0,   0},0x0f,TXW_SST_ALTK}, // 0x10e TXa_BACKSP
   {{   0,   0,   0,   0,   0,   0},   0,           0}  //- Sentinel
};


static TXW_ACSKEYSTATE txw_linkey[] =
{
  //- Std   Shift   Ctrl4   Alt-2       index

  { 0x103,  0x133,  0x12b,  0x129 }, // 0x000 c_2,a_COMMA,a_BACKSLASH, a_BACKQUOTE
  { 0x11e,  0x123,  0x118,  0x12f }, // 0x001 TXa_A,TXa_H,TXa_O,TXa_V
  { 0x130,  0x117,  0x119,  0x111 }, // 0x002 TXa_B,TXa_I,TXa_P,TXa_W
  { 0x12e,  0x124,  0x110,  0x12d }, // 0x003 TXa_C,TXa_J,TXa_Q,TXa_X
  { 0x120,  0x125,  0x113,  0x115 }, // 0x004 TXa_D,TXa_K,TXa_R,TXa_Y
  { 0x112,  0x126,  0x11f,  0x12c }, // 0x005 TXa_E,TXa_L,TXa_S,TXa_Z
  { 0x121,  0x132,  0x114,  0x127 }, // 0x006 TXa_F,TXa_M,TXa_T,a_SEMICOLON
  { 0x122,  0x131,  0x116,  0x135 }, // 0x007 TXa_G,TXa_N,TXa_U,a_SLASH
  { 0x11a,  0x182,  0x11b,  0x128 }, // 0x008 TXa_LBRACKET,TXa_MINUS,TXa_RBRACKET,TXa_QUOTE
  { 0x009,  0x10f,  0x194,  0x134 }, // 0x009 ,s_TAB,c_TAB,a_DOT
  { 0x00d,  0x183,  0x00a,  0x11c }, // 0x00a k_ENTER,a_EQUAL,c_ENTER,a_ENTER
  { 0x181,  0x17a,  0x17f,  0x17d }, // 0x00b a_0,a_3,a_8,a_6
  { 0x178,  0x17b,  0x180,  0x17e }, // 0x00c a_2,a_4,a_9,a_7
  { 0x179,  0x17c,  0x00d,  0x00d }, // 0x00d a_3,a_5
  { 0x00e,  0x00e,  0x00e,  0x00e }, // 0x00e
  { 0x008,  0x008,  0x07f,  0x10e }, // 0x00f k_BACKSPACE,,c_BACKSP,a_BACKSP
  { 0x153,  0x010,  0x193,  0x1a3 }, // 0x010 k_DELETE,,c_DELETE,a_DELETE
  { 0x150,  0x011,  0x191,  0x1a0 }, // 0x011 k_DOWN,,c_DOWN,a_DOWN
  { 0x14f,  0x012,  0x175,  0x19f }, // 0x012 k_END,,c_END,a_END
  { 0x147,  0x013,  0x177,  0x197 }, // 0x013 k_HOME,,c_HOME,a_HOME
  { 0x152,  0x011,  0x192,  0x1a2 }, // 0x014 k_INSERT,,c_INSERT,a_INSERT
  { 0x14b,  0x015,  0x173,  0x19b }, // 0x015 k_LEFT,,c_LEFT,a_LEFT
  { 0x151,  0x016,  0x176,  0x1a1 }, // 0x016 k_PGDN,,c_PGDN,a_PGDN
  { 0x149,  0x017,  0x184,  0x199 }, // 0x017 k_PGUP,,c_PGUP,a_PGUP
  { 0x14d,  0x018,  0x174,  0x19d }, // 0x018 k_RIGHT,,c_RIGHT,a_RIGHT
  { 0x148,  0x019,  0x18d,  0x198 }, // 0x019 k_UP,,c_UP,a_UP
  { 0x13b,  0x154,  0x13c,  0x155 }, // 0x01a k_F1,s_F1,k_F2,s_F2
  { 0x13d,  0x156,  0x13e,  0x157 }, // 0x01b k_F3,s_F3,k_F4,s_F4
  { 0x13f,  0x158,  0x140,  0x159 }, // 0x01c k_F5,s_F5,k_F6,s_F6
  { 0x141,  0x15a,  0x142,  0x15b }, // 0x01d k_F7,s_F7,k_F8,s_F8
  { 0x143,  0x15c,  0x144,  0x15d }, // 0x01e k_F9,s_F9,k_F10,s_F10
  { 0x185,  0x187,  0x186,  0x188 }  // 0x01f k_F11,s_F11,k_F12,s_F12
};


// Get keyboard shift/alt/control status for Linux keyboard
int TxLinuxShiftState
(
   void
);

// Get keyboard key-ready info, with short timeout
int TxLinuxKbhit
(
   long                delay                    // IN    delay in ms
);

// Translate given index-value plus shifstate into TX-keycode
USHORT TxLinuxIndex2key                         // RET   TX keycode
(
   int                 index,                   // IN    index in table
   int                 sst                      // IN    shiftstate
);

// Translate KBD Esc-string plus shifstate to index and final shiftstate
int TxLinuxKstring2Index                        // RET   index or -1
(
   char               *keys,                    // IN    keystring, from 2nd
   int                 length,                  // IN    length to check
   int                *sst                      // INOUT shifstate
);

/*****************************************************************************/
// Get keyboard shift/alt/control status for Linux keyboard (fails in XDM/KDE)
/*****************************************************************************/
int TxLinuxShiftState
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
}                                               // end 'TxLinuxShiftState'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Get keyboard key-ready info, with short timeout allowing slower keyboards
/*****************************************************************************/
int TxLinuxKbhit
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
}                                               // end 'TxLinuxKbhit'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Translate given index-value plus shifstate into TX-keycode
/*****************************************************************************/
USHORT TxLinuxIndex2key                         // RET   TX keycode
(
   int                 index,                   // IN    index in table
   int                 sst                      // IN    shiftstate
)
{
   USHORT              rc = 0;                  // function return

   if ((index >= 0) && (index < TXW_ACS_TABLE_SIZE))
   {
      if      (sst == TXW_SST_ALTK) rc = txw_linkey[ index].Alt;
      else if (sst == TXW_SST_CTRL) rc = txw_linkey[ index].Ctrl;
      else if (sst == TXW_SST_SHFT) rc = txw_linkey[ index].Shift;
      else                          rc = txw_linkey[ index].Std;
   }
   return (rc);
}                                               // end 'TxLinuxIndex2key'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Translate KBD Esc-string plus shifstate to index and final shiftstate
/*****************************************************************************/
int TxLinuxKstring2Index                        // RET   index or -1
(
   char               *keys,                    // IN    keystring, from 2nd
   int                 length,                  // IN    length to check
   int                *sst                      // INOUT shifstate
)
{
   int                 rc  = TXW_INVKEY;        // function return
   TXW_LINUXKBD       *key = txw_linuxkey;      // lookup table

   while (key->es[0] != 0)                      // until end of table or found
   {
      if (strncmp( keys, key->es, length) == 0) // key string matches
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
}                                               // end 'TxLinuxKstring2Index'
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
   TXW_INPUT_EVENT    *event                    // IN    input even to add
);

// get input event from the queue
static void TxOS2GetQueueEvent
(
   TXW_INPUT_EVENT    *event                    // OUT   input even to add
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
   TXW_INPUT_EVENT    *event                    // OUT   input even to add
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
/*****************************************************************************/
void TxOS2KeyboardReader
(
   void               *arg                      // IN    thread argument
)
{
   TXW_INPUT_EVENT     event;

   while ((txwa->desktop != NULL) &&            // while windowed ...
          (txwa->KbdKill == FALSE) )            // and no stop request
// while (txwa->KbdKill == FALSE)               // until stop request
   {
      event.key = getch();                      // first (or only) keyvalue
      switch (event.key)
      {
         case 0x00:
         case 0xe0: event.key = TXW_KEY_GROUP_1 + getch();  break;
         default:                                           break;
      }
      TxOS2AddQueueEvent( &event);
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
      //- to be refined
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

      while ((key == 0) &&
             (ReadConsoleInput( winConsole, (INPUT_RECORD *) &we, 1, &dummy)))
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
                     case DOUBLE_CLICK:         // ignored for now
                        break;

                     case MOUSE_WHEELED:        // translate to movement keys
                        key = TxWinTranslateWheel( me);
                        break;

                     case MOUSE_MOVED:          // Dragging or simple move
                        event->value = TXm_DRAGGED;
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
      char                ks[64];               // assembled key string
      int                 index;

      ch = getch();
      st = TxLinuxShiftState();

      if (ch == TXk_ESCAPE)                     // Escape
      {
         while (TxLinuxKbhit( 1000))            // 1 msec delay between keys
         {                                      // (adjust for slow terminal?)
            ks[nr++] = (char) ch;
            ch = getch();
         }
      }
      ks[nr++] = (char) ch;

      if ((ch = ks[0]) == TXk_ESCAPE)           // Escape string
      {
         if (nr > 1)                            // real-string
         {
            if ((index = TxLinuxKstring2Index(ks +1, nr -1, &st)) != TXW_INVKEY)
            {
               key = (ULONG) TxLinuxIndex2key( index, st);
            }
         }
         else                                   // single Escape
         {
            key = (ULONG) ch;                   // use direct keyvalue (ASCII)
         }
      }
      else                                      // Single key
      {
         switch (ch)
         {
            case 0x00: case 0x09: case 0x0a: case 0x7f:
               key = (ULONG) TxLinuxIndex2key( (int) (ch & 0x0f), st);
               break;

            default:
               key = (ULONG) ch;                // use direct keyvalue (ASCII)
               break;
         }
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
BOOL txwGetMsg                                  // RET   FALSE if QUIT
(
   TXWQMSG            *qmsg                     // OUT   message packet
)
{
   BOOL                rc = TRUE;
   BOOL                async_input_received;

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
               txwGetInputEvent( NULL);         // read and discard type-ahead
            }
         }
         input = txwGetInputEvent( &mouse);     // wait for an event
         if (txwa->sbview)
         {
            txwPaintWinStatus( txwa->sbview, "", cSchemeColor); // reset user message
         }
         if (input == TXW_INPUT_MOUSE)
         {
            if ((qmsg->hwnd = txwQueryCapture()) == TXHWND_NULL)
            {
               qmsg->hwnd = txwTopWindowAtPos( mouse.row, mouse.col);
            }
            if      (mouse.value & TXm_DRAGGED)
            {
               qmsg->msg  = TXWM_MOUSEMOVE;
            }
            else if (mouse.value == 0)
            {
               qmsg->msg  = TXWM_BUTTONUP;
            }
            else
            {
               qmsg->msg  = TXWM_BUTTONDOWN;
            }
            qmsg->mp1  = TXMPFROM2SH( mouse.col,   mouse.row);
            qmsg->mp2  = TXMPFROM2SH( mouse.value, mouse.state);
         }
         else if (input == TXW_INPUT_ASYNC)     // msg queued by other thread
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

   if (qmsg->msg == TXWM_QUIT)
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

   rc = txwSendMsg( qmsg->hwnd,
                    qmsg->msg,
                    qmsg->mp1,
                    qmsg->mp2);
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
