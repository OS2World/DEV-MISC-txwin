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
// TX Basic text I/O primitives
//
// Author: J. van Wijk
//
// JvW  25-05-1999 Initial version, split off from ANSILOG.C
// JvW  22-08-2001 Added ReadCellStr & fixed WriteCellStr bug (garbage char)
// JvW  08-09-2001 Combined all functions from txw32con and txw32ans
// JvW  28-05-2002 Added DOS32 support (DOS4G/W or causeway or compatibles)
// JvW  07-06-2002 Speedup using selective direct video-memory access
// JvW  10-03-2004 Added Linux code and initializaton/termination API

#include <txlib.h>
#include <txwpriv.h>                            // private window interface

#if defined (WIN32)

#define TX32CON_MAXERR  16

typedef enum tx_color_value
{
   Black,     Blue,          Green,       Cyan,
   Red,       Magenta,       Brown,       Light_Gray,
   Dark_Gray, Light_Blue,    Light_Green, Light_Cyan,
   Light_Red, Light_Magenta, Yellow,      White
} TX_COLOR_VALUE;

static HANDLE virtualScrBuf = NULL;

static void TxWntSetFgCol
(
   TX_COLOR_VALUE      color                    // IN    color value
);

static void TxWntSetBgCol
(
   TX_COLOR_VALUE      color                    // IN    color value
);

// set background and foreground color for NT console
static void txNtSet_color
(
   WORD                color                    // IN    combined color value
);


// get the size of display area, horizontal
static int  TxWntSx
(
   void
);

// get the size of display area, vertical
static int  TxWntSy
(
   void
);

// get the position of the cursor, horizontal
static int  TxWntCx
(
   void
);

// get the position of the cursor, vertical
static int  TxWntCy
(
   void
);

// set the position of the cursor
static void TxWntGo
(
   int                 x,                       // IN    horizontal position
   int                 y                        // IN    horizontal position
);

// clear the screen
static void TxWntClearScreen
(
   void
);

// clear to end of line
static void TxWntClrToEndOfLine
(
   void
);

// write a single character to the console
static void TxWntWriteChar
(
   char                ch                       // IN    character to write
);

//- write string to console, limit to length (clipping)
static void TxWntWriteString
(
   char               *str,                     // IN    string pointer
   short               length                   // IN    length to write
);

// get the current status of the output buffer
static int TxWntGetScrInfo
(
   HANDLE                      buffer,
   PCONSOLE_SCREEN_BUFFER_INFO info
);

#elif defined (UNIX)                            // Unix console/xterm
#include <sys/ioctl.h>
#include <sys/time.h>

typedef struct tx_screen
{
   short               cols;                    // screen width in characters
   short               rows;                    // screen length in lines
   TXCELL             *buffer;                  // screen buffer
} TX_SCREEN;


//- WARNING: Edit the two tables below in codepage-437 for readability!
//- character translation table, for alternate-set and controlchar replacement
//- Codepage-437 capable version (drawing characters) like Linux console
static char chTransConsole[256] =
{
    //-   1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 00
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 10
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 30
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 40
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 50
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '^', // 70
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 80
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 90
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '®', '¯', // A0
    '°', '±', '²', '³', '´', '¹', '¹', '»', '»', '¹', 'º', '»', '¼', '¼', '¼', '¿', // B0
    'À', 'Á', 'Â', 'Ã', 'Ä', 'Å', 'Ì', 'Ì', 'È', 'É', 'Ê', 'Ë', 'Ì', 'Í', 'Î', 'Á', // C0
    'Ê', 'Ë', 'Ë', 'È', 'È', 'É', 'É', 'Î', 'Î', 'Ù', 'Ú', 'Û', 'Ü', 'Ý', 'Þ', 'ß', // D0
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // E0
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 'x',   0,   0,   0, // F0
};

#if defined (USEWINDOWING)
//- character translation table, for alternate-set and controlchar replacement
//- Generic terminal version (no drawing characters) like X-terminal or VT220
static char chTransGeneric[256] =
{
    //-   1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 00
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 10
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 30
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 40
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 50
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '^', // 70
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 80
    '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^', // 90
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '<', '>', // A0
     97,  97,  97, 120, 117, 117, 117, 107, 107, 117, 166, 107, 106, 106, 106, 107, // B0
    109, 118, 119, 116, 113, 110, 116, 116, 109, 108, 118, 119, 116, '=', 110, 118, // C0
    118, 119, 119, 109, 109, 108, 108, 110, 110, 106, 108,  97, 113, 120, 120, 113, // D0
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // E0
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 'x',   0,   0,   0, // F0
};

//- terminal dependent definitions and escape-sequences
//- Note: All terminals are supposed to understand the ANSI sequenses for colors

static TXTT      txlTermCap_eA  = "";           // Terminal initialize ??

static short     virtualCurCol;                 // cursor pos on virtual screen
static short     virtualCurRow;
static BOOL      cursorVisible  = FALSE;        // cursor should be visible
static BOOL      cursorInsert   = FALSE;        // cursor shape insert mode

#endif                                          // USEWINDOWING

static TXLTERM   txlTerminal    = TXLT_CONSOLE;   // default linux console
static char     *txlTransTable  = chTransConsole; // actual translate table
static BOOL      txl16ColorsBg  = TRUE;           // allow 16 colors background
static TXTT      txlTermCap_ae  = "";             // Switch to standard  charset
static TXTT      txlTermCap_as  = "";             // Switch to alternate charset
static short     displayCurCol;                   // cursor pos on display device
static short     displayCurRow;
static BYTE      currentColor   =  0;             // current color value
static BOOL      alternateChr   = FALSE;          // alternate set in use now
static TX_SCREEN virtualScreen  = {80, 25, NULL}; // updated by application
static TX_SCREEN displayScreen  = {80, 25, NULL}; // as shown on display


//- initialize Linux display
static ULONG TxLinInitializeDisplay
(
   void
);

//- Restore Linux console to initial status
static void TxLinTerminateDisplay
(
   void
);

#if defined (USEWINDOWING)
// set cursor position, update visibility if moving off/on screen
static void TxLinSetCursorPos
(
   short               row,
   short               col
);

// make cursor invisible
static void TxLinHideCursor
(
   void
);

// make cursor visible
static void TxLinShowCursor
(
   void
);
#endif

// Draw characters to the actual display terminal for specified fragment
static void TxLinUpdateFragment
(
   short               start,
   short               end
);

// refresh specified area in display-buffer from virtual one
static void TxLinUpdateArea
(
   short               refresh_start,           // start offset in virtual
   short               refresh_end              // end offset
);

#if defined (NEVER)                             // not used yet
// refresh multiple lines on display
static void TxLinUpdateLines
(
   short               first,                   // IN    first line to refresh
   short               last                     // IN    last line to refresh
);
#endif

// Get console/window display size
static void TxLinGetScreenSize
(
   short              *cols,
   short              *rows
);

// Set global screen buffers to specified size
static ULONG TxLinAllocScrBuffers
(
   short               cols,                    // IN    number of columns
   short               rows                     // IN    number of rows
);

// Set cursor position on display device
static void TxLinSetDevCursorPos
(
   short               col,
   short               row
);

#if defined (USEWINDOWING)
// Make cursor invisible on display device
static void TxLinHideDevCursor
(
   void
);

// Make cursor visible on display device
static void TxLinShowDevCursor
(
   void
);
#endif

#define TXX_VIDEO  ((BYTE *) virtualScreen.buffer)

#elif defined (DOS32)                           // extended DOS

#define VideoInt10()   int386( TXDX_VIDEO, &regs, &regs)

#define TXX_VIDEO  ((BYTE *)  0xb8000)          // Video memory start

#else                                           // OS/2 VIO subsystem

typedef struct txviostate
{
   USHORT        length;                        // length of struct = 6
   USHORT        request;                       // request type = 3
   USHORT        bright;                        // 0 = blink; 1 = bright
}  TXVIOSTATE;                                  // end of struct txviostate
#endif


/*****************************************************************************/
// Initialize low level screen/terminal handling
/*****************************************************************************/
ULONG TxScreenInitialize
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return

   #if defined (UNIX)
      rc = TxLinInitializeDisplay();
   #endif

   return (rc);
}                                               // end 'TxScreenInitialize'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Terminate low level screen/terminal handling
/*****************************************************************************/
ULONG TxScreenTerminate
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return

   #if defined (UNIX)
      TxLinTerminateDisplay();
   #elif defined (WIN32)
      txNtSet_color((WORD) TX_Default);
   #endif

   return (rc);
}                                               // end 'TxScreenTerminate'
/*---------------------------------------------------------------------------*/



/*****************************************************************************/
// Return number of columns on the screen
/*****************************************************************************/
short  TxScreenCols                             // RET   current screen columns
(
   void
)
{
   short               col = 80;

   #if defined (WIN32)
      col = (short ) TxWntSx();
   #elif defined (DOS32)
      col = TxxBiosWord(TXX_CRT_COLS);          // get from BIOS data area
   #elif defined (UNIX)
      col = virtualScreen.cols;
   #else
      VIOMODEINFO   vio;

      vio.cb = sizeof(VIOMODEINFO);

      if (VioGetMode(&vio, 0) == NO_ERROR)      // get current screenmode
      {
         col = vio.col;
      }
   #endif
   return( col);
}                                               // end 'TxScreenCols'
/*---------------------------------------------------------------------------*/

#if defined (USEWINDOWING) || defined (DOS32)
/*****************************************************************************/
// Return current cursor column, 0 = left
/*****************************************************************************/
short  TxCursorCol                              // RET   current cursor column
(
   void
)
{
   USHORT              col = 1;

   #if defined (WIN32)
      col = (USHORT) TxWntCx();
   #elif defined (DOS32)
      union  REGS      regs;

      TxxClearReg( regs);
      regs.h.bh = 0x00;                         // first page of text-video mem
      TxxVideoInt( regs, TXDX_VIDEO_GETCURSOR);

      col = (USHORT) regs.h.dl;
   #elif defined (UNIX)
      col = (USHORT) virtualCurCol;
   #else
      USHORT           row;

      VioGetCurPos( &row, &col, 0);
   #endif
   return((short) col);
}                                               // end 'TxCursorCol'
/*---------------------------------------------------------------------------*/
#endif

/*****************************************************************************/
// Return number of lines on the screen
/*****************************************************************************/
short  TxScreenRows                             // RET   current screen rows
(
   void
)
{
   short               row = 25;

   #if defined (WIN32)
      row = (short ) TxWntSy();
   #elif defined (DOS32)
      row = (USHORT) TxxBiosChar(TXX_CRT_ROWS) +1; // get from BIOS data area
   #elif defined (UNIX)
      row = virtualScreen.rows;
   #else
      VIOMODEINFO   vio;

      vio.cb = sizeof(VIOMODEINFO);

      if (VioGetMode(&vio, 0) == NO_ERROR)      // get current screenmode
      {
         row = vio.row;
      }
   #endif
   return( row);
}                                               // end 'TxScreenRows'
/*---------------------------------------------------------------------------*/


#if defined (USEWINDOWING)
/*****************************************************************************/
// Return current cursor row, 0 = top
/*****************************************************************************/
short  TxCursorRow                              // RET   current cursor row
(
   void
)
{
   USHORT              row = 1;

   #if defined (WIN32)
      row = (USHORT) TxWntCy();
   #elif defined (DOS32)
      union  REGS      regs;

      TxxClearReg( regs);
      regs.h.bh = 0x00;                         // first page of text-video mem
      TxxVideoInt( regs, TXDX_VIDEO_GETCURSOR);

      row = (USHORT) regs.h.dh;
   #elif defined (UNIX)
      row = (USHORT) virtualCurRow;
   #else
      USHORT           col;

      VioGetCurPos( &row, &col, 0);
   #endif
   return((short) row);
}                                               // end 'TxCursorRow'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set new cursor position row and column
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
void TxSetCursorPos
(
   short               row,                     // IN    destination row
   short               col                      // IN    destination column
)
{
   #if defined (WIN32)
      TxWntGo((int) col, (int) row);
   #elif defined (DOS32)
      union  REGS      regs;

      TxxClearReg( regs);
      regs.h.bh = 0x00;                         // first page of text-video mem
      regs.h.dh = (BYTE) row;
      regs.h.dl = (BYTE) col;
      TxxVideoInt( regs, TXDX_VIDEO_SETCURSOR);
   #elif defined (UNIX)
      TxLinSetCursorPos( row, col);
   #else
      VioSetCurPos((USHORT) row, (USHORT) col, 0);
   #endif
   return;
}                                               // end 'TxSetCursorPos'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set new cursor visibility and style
/*****************************************************************************/
void TxSetCursorStyle
(
   BOOL                visible,                 // IN    cursor visible
   BOOL                insert                   // IN    insert style
)
{
   #if defined (WIN32)
      CONSOLE_CURSOR_INFO   cinfo;

          cinfo.dwSize   = (insert) ? 50 : 10;
          cinfo.bVisible = visible;

      SetConsoleCursorInfo( virtualScrBuf, &cinfo);
   #elif defined (DOS32)
      union  REGS      regs;

      TxxClearReg( regs);
      if (visible)
      {
         regs.h.ch = (insert)  ? (BYTE) 4 : (BYTE) 6;
         regs.h.cl = 7;
      }
      else
      {
         regs.h.ch = 0x20;
      }
      TxxVideoInt( regs, TXDX_VIDEO_SETCSHAPE);
   #elif defined (UNIX)
      //- to be refined, set cursor shape for insert (ioctl ?)
      //- use TermCap ve (normal) vs (insert) and vi (hidden) escape strings
      if (visible)
      {
         TxLinShowCursor();
      }
      else
      {
         TxLinHideCursor();
      }
      cursorInsert = insert;
   #else
      VIOCURSORINFO    ci;

      VioGetCurType( &ci, 0);                   // get current cursor style

      ci.attr   = (USHORT) ((visible) ?   1 :  -1);
      ci.yStart = (USHORT) ((insert)  ? -50 : -90);
      ci.cEnd   = (USHORT) (-100);

      VioSetCurType( &ci, 0);                   // set current cursor style
   #endif
   return;
}                                               // end 'TxSetCursorStyle'
/*---------------------------------------------------------------------------*/
#endif

/*****************************************************************************/
// Set blinking/bright-background mode for fullscreen sessions (VGA)
/*****************************************************************************/
void TxSetBrightBgMode
(
   BOOL                bright                   // IN    use bright background
)
{
   #if   defined (WIN32)                        // no setting needed (no FS)
   #elif defined (DOS32)
      union  REGS      regs;

      TxxClearReg( regs);
      regs.h.al = 0x03;                         // set bright/blinking
      regs.h.bl = (BYTE) ((bright) ? 0 : 1);
      TxxVideoInt( regs, TXDX_VIDEO_SETSTATUS);
   #elif defined (UNIX)
      //- to be refined
   #else                                        // use VioSetState INTENSITY
      TXVIOSTATE       viostate;

      viostate.length  = 6;                     // length of struct
      viostate.request = 2;                     // request type
      viostate.bright  = (bright) ? 1 : 0;      // 0 = blink; 1 = bright

      VioSetState( &viostate, 0);               // set bright/blink state
   #endif
   return;
}                                               // end 'TxSetBrightBgMode'
/*---------------------------------------------------------------------------*/

#if defined (USEWINDOWING)
/*****************************************************************************/
// Draw a character-string at specified position, clip to optional clip-rect
/*****************************************************************************/
void txwScrDrawCharString
(
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXRECT             *clip,                    // IN    opt clip-rectangle
   char               *str                      // IN    string to draw
)
{
   short               length;                  // length visible string
   short               start;                   // start-offset visible string

   TXSCREEN_BEGIN_UPDATE();
   length = txwCalculateStringClip( row, col, (short) strlen(str), clip, &start);
   if (length > 0)
   {
      short         srow = TxCursorRow();
      short         scol = TxCursorCol();

      #if defined (WIN32)
         COORD         dpos;
         ULONG         todo = (ULONG) length;
         ULONG         done;

         dpos.X = col + start;
         dpos.Y = row;

         WriteConsoleOutputCharacter( virtualScrBuf,
                            (LPCTSTR) (str + start), todo, dpos, &done);

      #elif defined (DEV32)                     // OS/2 VIO subsystem
         memcpy( txVbuf, str + start, length);
         txVbuf[ length] = 0;

         VioWrtCharStr( txVbuf, length, row, col + start, 0);

      #else                                     // DOS or UNIX memory mapped
         short         i;
         BYTE         *first = TXX_VIDEO;

         first += ((row * TxScreenCols()) +col +start) *2;

         for (i = 0; i < length; i++)           // itterate over visible string
         {
            *first = str[ start + i];           // write the character
            first++;                            // move to attribute byte
            first++;                            // move to next character
         }
      #endif
      TxSetCursorPos( srow, scol);              // restore cursor position
   }
   TXSCREEN_ENDOF_UPDATE();
   return;
}                                               // end 'txwScrDrawCharString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Draw a character-string at position in specified color, using clip-rect
/*****************************************************************************/
void txwScrDrawCharStrCol
(
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXRECT             *clip,                    // IN    opt clip-rectangle
   char               *str,                     // IN    string to draw
   BYTE                color                    // IN    color attribute
)
{
   short               length;                  // length visible string
   short               start;                   // start-offset visible string

   TXSCREEN_BEGIN_UPDATE();
   length = txwCalculateStringClip( row, col, (short) strlen(str), clip, &start);
   if (length > 0)
   {
      short         srow = TxCursorRow();
      short         scol = TxCursorCol();

      #if defined (WIN32)
         if ((row == (TxWntSy() -1)) && ((col+start+length) >= TxWntSx()))
         {
            length = TxWntSx() -col -start -1;  // no write to lower-right
         }                                      // or the screen will scroll!
         TxWntGo((int) col + start, (int) row);
         txNtSet_color((WORD) color);
         TxWntWriteString( str + start, length);
      #elif defined (DEV32)                     // OS/2 VIO subsystem
         memcpy( txVbuf, str + start, length);
         txVbuf[ length] = 0;

         VioWrtCharStrAtt( txVbuf, length, row, col + start, &color, 0);

      #else                                     // DOS or UNIX memory mapped
         short         i;
         BYTE         *first = TXX_VIDEO;

         first += ((row * TxScreenCols()) +col +start) *2;

         for (i = 0; i < length; i++)           // itterate over visible string
         {
            *first = str[start + i];            // write the character
            first++;                            // move to attribute byte
            *first = color;                     // write the attribute
            first++;                            // move to next character
         }
      #endif
      TxSetCursorPos( srow, scol);              // restore cursor position
   }
   TXSCREEN_ENDOF_UPDATE();
   return;
}                                               // end 'txwScrDrawCharStrCol'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Draw a cell-string at specified position, clip to optional clip-rect
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
void txwScrDrawCellString
(
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXRECT             *clip,                    // IN    opt clip-rectangle
   TXCELL             *cell,                    // IN    cell-string to draw
   short               size,                    // IN    nr of cells
   ULONG               altcol                   // IN    color conversions
)
{
   short               length;                  // length visible string
   short               start;                   // start-offset visible string

   TXSCREEN_BEGIN_UPDATE();
   length = txwCalculateStringClip( row, col, size, clip, &start);
   if (length > 0)
   {
      short            srow = TxCursorRow();
      short            scol = TxCursorCol();
      TXCELL          *cs   = (TXCELL *) txVbuf;
      short            i;
      BYTE             ca, fg, bg;              // color attribute, FG, BG

      memcpy( txVbuf, (BYTE *) &cell[start], length *2);

      if (altcol != TXSB_COLOR_NORMAL)          // any conversions wanted ?
      {
         for (i = 0; i < length; i++)           // itterate over visible string
         {
            ca = cs[i].at;
            fg = ca & 0x0f;                     // FG component only
            bg = ca & 0xf0;                     // BG component only

            if (altcol & TXSB_COLOR_BRIGHT)     // use bright FG colors
            {
               if (fg != TXwCNW)                // if FG not normal white
               {
                  fg  |= TXwINT;                // set intensity bit
               }
            }
            if (altcol & TXSB_COLOR_B2BLUE)     // b2blue => Black to Blue BG
            {
               if (bg == TXwCnZ)                // Black to Blue BG
               {
                  bg  =  TXwCnB;
                  if (fg == TXwCNB)             // but avoid Blue_on_Blue
                  {
                     fg  =  TXwCNZ;
                  }
               }
            }
            if (altcol & TXSB_COLOR_INVERT)     // invert => white/brown BG
            {
               if (fg == TXwCNW)                // make sure white will be
               {                                // black, not "bright black"
                  ca = ~(bg | fg) & 0x77;       // invert, no bright BG/FG
               }
               else
               {
                  ca = ~(bg | fg) & 0x7f;       // invert, but no bright BG
               }
               if (ca == TX_Blue_on_Black)      // fix hard to read colors
               {
                  ca = TX_Blue_on_Grey;
               }
            }
            else
            {
               ca = bg | fg;
            }
            cs[i].at = ca;
         }
      }

      #if defined (WIN32)
      {
         CHAR_INFO     cistr[TXMAXLN];          // char-info string
         COORD         sizes;                   // source sizes
         COORD         first;                   // first cell coord
         SMALL_RECT    destination;             // destination rectangle

         if (length > TXMAXLN)
         {
            length = TXMAXLN;                   // limit to max buffersize
         }
         sizes.X = TXMAXLN;
         sizes.Y = 1;
         first.X = 0;
         first.Y = 0;
         destination.Left   = col + start;
         destination.Top    = row;
         destination.Right  = col + start + length -1;
         destination.Bottom = row;

         for (i = 0; i < length; i++)           // itterate over visible string
         {
            cistr[i].Char.AsciiChar = cs[i].ch;
            cistr[i].Attributes     = cs[i].at;
         }
         WriteConsoleOutput( virtualScrBuf, cistr, sizes, first, &destination);
      }
      #elif defined (DEV32)                     // OS/2 VIO subsystem
         VioWrtCellStr( txVbuf, length *2, row, col + start, 0);
      #else                                     // DOS or UNIX memory mapped
      {
         BYTE         *first = TXX_VIDEO;

         first += ((row * TxScreenCols()) +col +start) *2;

         memcpy( first, txVbuf, length *2);
      }
      #endif
      TxSetCursorPos( srow, scol);              // restore cursor position
   }
   TXSCREEN_ENDOF_UPDATE();
   return;
}                                               // end 'txwScrDrawCellString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Read a cell-string from specified position on screen to a string
/*****************************************************************************/
void txwScrReadCellString
(
   short               row,                     // IN    source row
   short               col,                     // IN    source column
   TXCELL             *cell,                    // OUT   cell-string to get
   short               length                   // IN    nr of cells
)
{
   #if defined (WIN32)
      CHAR_INFO     cistr[TXMAXLN];             // char-info string
      COORD         sizes;                      // source sizes
      COORD         first;                      // first cell coord
      SMALL_RECT    destination;                // destination rectangle
      short         i;

      if (length > TXMAXLN)
      {
         length = TXMAXLN;                      // limit to max buffersize
      }
      sizes.X = TXMAXLN;
      sizes.Y = 1;
      first.X = 0;
      first.Y = 0;
      destination.Left   = col;
      destination.Top    = row;
      destination.Right  = col + length -1;
      destination.Bottom = row;

      ReadConsoleOutput( virtualScrBuf,
         cistr, sizes, first, &destination);

      for (i = 0; i < length; i++)              // itterate over visible string
      {
         cell[i].ch = (BYTE) cistr[i].Char.AsciiChar;
         cell[i].at = (BYTE) cistr[i].Attributes;
      }

   #elif defined (DEV32)                        // OS/2 VIO subsystem
      USHORT           size = (USHORT) length  *2;

      VioReadCellStr( txVbuf, &size, row, col, 0); // buffer to avoid 64Kb bug
      memcpy((BYTE *) &cell[0], txVbuf, size);
   #else                                        // DOS or UNIX memory mapped
      BYTE            *first = TXX_VIDEO;

      first += ((row * TxScreenCols()) + col)  *2;

      memcpy( (BYTE *) &cell[0], first, length *2);
   #endif
   return;
}                                               // end 'txwScrReadCellString'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Change color for rectangle to simulate shadow; BG = black, FG = low-intens
/*****************************************************************************/
void txwScrShadowRectangle
(
   TXRECT             *rect                     // IN    destination rectangle
)
{
   short               sx = rect->right  - rect->left +1;
   short               sy = rect->bottom - rect->top  +1;
   TXCELL             *cs;
   int                 line;
   int                 col;
   BYTE                color;
   BYTE                fg;

   TXSCREEN_BEGIN_UPDATE();
   if ((cs = TxAlloc( sx, sizeof( TXCELL))) != NULL)
   {
      for (line = 0; line < sy; line++)
      {
         txwScrReadCellString( rect->top + line, rect->left,       cs, sx);

         for (col = 0; col < sx; col++)
         {
            color = (BYTE) cs[col].at;
            fg    = color & 0x0f;
            if ((fg == TXwCNW) || (fg == TXwCNZ) || (fg == TXwCBZ))
            {
               color = TX_Grey_on_Black;        // make it grey on black
            }
            else
            {
               color = fg & 0x07;               // low intens FG on black
            }
            cs[col].at = color;
         }
         txwScrDrawCellString( rect->top + line, rect->left, NULL, cs, sx, 0);
      }
      TxFreeMem( cs);
   }
   TXSCREEN_ENDOF_UPDATE();
   return;
}                                               // end 'txwScrShadowRectangle'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Fill area for rectangle with specified cell (clear area)
/*****************************************************************************/
void txwScrFillRectangle
(
   TXRECT             *rect,                    // IN    destination rectangle
   TXCELL              cell                     // IN    cell-string to draw
)
{
   short               row;
   short               col;
   short               cols;
   #if defined   (DOS32)
      short            srow = TxCursorRow();
      short            scol = TxCursorCol();
   #elif defined (WIN32)
      COORD            fpos;
      ULONG            todo;
      ULONG            done;
   #elif defined (UNIX)
      TXCELL          *this;
      int              i;
   #else
   #endif

   TXSCREEN_BEGIN_UPDATE();
   if (txwNormalRect( FALSE, rect))             // true normalized rect?
   {
      row  = rect->top;
      col  = rect->left;
      cols = rect->right - rect->left +1;       // width of rectangle

      #ifndef UNIX
      if (cols == TxScreenCols())               // over full screen width ?
      {                                         // optimized using wrap-mode
         #if defined (WIN32)
            fpos.X = 0;
            fpos.Y = row;
            todo   = (rect->bottom - row  +1) * TxScreenCols();

            FillConsoleOutputAttribute( virtualScrBuf,
               (WORD)  cell.at, todo, fpos, &done);

            FillConsoleOutputCharacter( virtualScrBuf,
               (TCHAR) cell.ch, todo, fpos, &done);

         #elif defined (DOS32)
            union  REGS   regs;

            TxSetCursorPos( row, 0);
            TxxClearReg( regs);
            regs.h.bh = 0x00;                   // page 0
            regs.h.al = cell.ch;                // ASCII value in cell
            regs.h.bl = cell.at;                // color value in cell
            TXWORD.cx = (rect->bottom - row +1) * TxScreenCols();
            TxxVideoInt( regs, TXDX_VIDEO_WRITNCELL);
         #elif defined (UNIX)
            //- optimize not needed for Unix, memory mapped anyway ...
         #else
            VioWrtNCell((BYTE *) &cell,
                        (rect->bottom - row +1) * TxScreenCols(), row, 0, 0);
         #endif
      }
      else                                      // not optimized, repeat rows
      #endif

      {
         #if defined (WIN32)
            fpos.X = col;
            todo   = cols;
         #endif
         for (row = rect->top; row <= rect->bottom; row++)
         {
            #if defined (WIN32)
               fpos.Y = row;

               FillConsoleOutputAttribute( virtualScrBuf,
                  (WORD)  cell.at, todo, fpos, &done);

               FillConsoleOutputCharacter( virtualScrBuf,
                  (TCHAR) cell.ch, todo, fpos, &done);

            #elif defined (DOS32)
               union  REGS   regs;

               TxSetCursorPos( row, col);
               TxxClearReg( regs);
               regs.h.bh = 0x00;                // page 0
               regs.h.al = cell.ch;             // ASCII value in cell
               regs.h.bl = cell.at;             // color value in cell
               TXWORD.cx = cols;
               TxxVideoInt( regs, TXDX_VIDEO_WRITNCELL);
            #elif defined (UNIX)
               this = &(virtualScreen.buffer[row * TxScreenCols() + col]);
               for (i = 0; i < cols; i++, this++)
               {
                  *this = cell;
               }
            #else
               VioWrtNCell((BYTE *) &cell, cols, row, col, 0);
            #endif
         }
      }
   }
   #if defined (DOS32)
      TxSetCursorPos( srow, scol);              // restore cursor position
   #endif
   TXSCREEN_ENDOF_UPDATE();
   return;
}                                               // end 'txwScrFillRectangle'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Scroll rectangle up or down, fill new with specified cell
// Note: As of 05-03-2004 this function is NOT really used in the library!
/*****************************************************************************/
void txwScrScrollRectangle
(
   TXRECT             *rect,                    // IN    scroll rectangle
   int                 scroll,                  // IN    scroll lines (+ = up)
   TXCELL              cell                     // IN    cell-string to draw
)
{
   #if defined (WIN32)
      COORD            destination;
      CHAR_INFO        fill;

      destination.X       = rect->left;
      destination.Y       = rect->top - scroll;
      fill.Char.AsciiChar = (CHAR) cell.ch;
      fill.Attributes     = (WORD) cell.at;

      ScrollConsoleScreenBuffer( virtualScrBuf,
         (SMALL_RECT *) rect, (SMALL_RECT *) rect, destination, &fill);

   #elif defined (DOS32)
      union REGS          regs;

      TxxClearReg( regs);
      regs.h.bh = cell.at;                      // color value in cell
      regs.h.ch = (BYTE) rect->top;
      regs.h.cl = (BYTE) rect->left;
      regs.h.dh = (BYTE) rect->bottom;
      regs.h.dl = (BYTE) rect->right;
      if (scroll > 0)                           // up
      {
         regs.h.al = (BYTE) (+scroll);          // lines to scroll
         TxxVideoInt( regs, TXDX_VIDEO_SCROLL_UP);
      }
      else
      {
         regs.h.al = (BYTE) (-scroll);          // lines to scroll
         TxxVideoInt( regs, TXDX_VIDEO_SCROLL_DN);
      }
   #elif defined (UNIX)
      //- to be refined use memory mapped way to scroll
   #else
      if (scroll > 0)                           // scroll up
      {
         VioScrollUp( rect->top   , rect->left ,
                      rect->bottom, rect->right,
                      (USHORT) (+scroll),
                      (BYTE *) &cell, 0);
      }
      else
      {
         VioScrollDn( rect->top   , rect->left ,
                      rect->bottom, rect->right,
                      (USHORT) (-scroll),
                      (BYTE *) &cell, 0);
      }
   #endif
   return;
}                                               // end 'txwScrScrollRectangle'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine visible lines on screen (no scrollbars)
/*****************************************************************************/
ULONG txwScrVisibleLines
(
   void
)
{
   ULONG               rc = 50;                 // default function return

   #if defined (WIN32)
      CONSOLE_SCREEN_BUFFER_INFO info;

      if (TxWntGetScrInfo( virtualScrBuf, &info))
      {
         rc = (ULONG) info.dwMaximumWindowSize.Y;
      }
   #endif

   return (rc);
}                                               // end 'txwScrVisibleLines'
/*---------------------------------------------------------------------------*/
#endif                                          // USEWINDOWING

#if defined (UNIX)

/*****************************************************************************/
// Get type of Linux terminal used by low-level IO
/*****************************************************************************/
TXLTERM TxLinTerminalType                       // RET    Terminal type
(
   void
)
{
   return (txlTerminal);
}                                               // end 'TxLinTerminalType'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Initialize Linux display
/*****************************************************************************/
static ULONG TxLinInitializeDisplay
(
   void
)
{
   ULONG               rc;
   short               cols;
   short               rows;
                                                // no ESC-seq if not windowed
#if defined (USEWINDOWING)
   char               *term;

   if ((term = getenv( "TXTERM")) == NULL)      // TX specific setting ?
   {
      term = getenv( "TERM");                   // use standard TERM definition
   }

   if (term != NULL)
   {
      if (strncasecmp( term, "vt2", 3) == 0)    // VT220 compatible
      {
         txlTerminal    = TXLT_VT220;
         txlTransTable  = chTransGeneric;       // generic terminal (like X)
         txl16ColorsBg  = FALSE;                // just 8 BG colors supported
         strcpy( txlTermCap_eA, "");
         strcpy( txlTermCap_ae, "\033(B");      // use standard  charset
         strcpy( txlTermCap_as, "\033(0");      // use alternate charset
      }
      else if ((tolower(*term) == 'x') ||       // any X terminal
               (tolower(*term) == 'v') ||       // ANSI, VT100 terminal
        (strncasecmp( term, "rxvt", 4) == 0))   // RXVT, XTERM/VT compatible
      {
         txlTerminal    = TXLT_XTERM_VT100;
         txlTransTable  = chTransGeneric;       // generic terminal (like X)
         txl16ColorsBg  = FALSE;                // just 8 BG colors supported
         strcpy( txlTermCap_eA, "\033(B\033)0");
         strcpy( txlTermCap_ae, "\017");        // use standard  charset
         strcpy( txlTermCap_as, "\016");        // use alternate charset
      }
      else                                      // assume LINUX console
      {
         txlTerminal    = TXLT_CONSOLE;         // CP-437 capable console
         txlTransTable  = chTransConsole;       // including alternate set
         txl16ColorsBg  = TRUE;                 // 16 BG colors supported
         strcpy( txlTermCap_eA, "");
         strcpy( txlTermCap_ae, "\033[10m");    // use standard  charset
         strcpy( txlTermCap_as, "\033[11m");    // use CP-437    charset
      }
      if (txlTermCap_eA[0] != 0)
      {
         fwrite( txlTermCap_eA, 1, strlen(txlTermCap_eA), stdout);
         printf( "%s", txlTermCap_eA);
      }
      if (txlTermCap_ae[0] != 0)
      {
         fwrite( txlTermCap_ae, 1, strlen(txlTermCap_ae), stdout);
      }
   }
#endif                                          // USEWINDOWING

   TxLinGetScreenSize( &cols, &rows);

   if ((rc = TxLinAllocScrBuffers( cols, rows)) == NO_ERROR)
   {
      //- to be refined, not desirable in non-windowed mode
      //- not really required when windowed, will be done on next window paint

      // TxLinHideCursor();
      // TxLinUpdateScreen( TRUE);
   }
   return (rc);
}                                               // end 'TxLinInitializeDisplay'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Restore Linux console to initial status
/*****************************************************************************/
static void TxLinTerminateDisplay
(
   void
)
{
   TxFreeMem( virtualScreen.buffer);
   TxFreeMem( displayScreen.buffer);
                                                // no ESC-seq if not windowed
#if defined (USEWINDOWING)
   fwrite( "[0m", 1, 4, stdout);               // reset color to default
   if (txlTermCap_ae[0] != 0)
   {
      fwrite( txlTermCap_ae, 1, strlen(txlTermCap_ae), stdout);
   }
   TxLinSetDevCursorPos( 0, virtualScreen.rows -1);
   TxLinShowDevCursor();
#endif                                          // USEWINDOWING
}                                               // end 'TxLinTerminateDisplay'
/*---------------------------------------------------------------------------*/


#if defined (USEWINDOWING)
/*****************************************************************************/
// set cursor position, update visibility if moving off/on screen
/*****************************************************************************/
static void TxLinSetCursorPos
(
   short               row,
   short               col
)
{
   virtualCurCol = col;                         // adjust global cursor pos
   virtualCurRow = row;

   if ((col >= 0) && (col < virtualScreen.cols) &&
       (row >= 0) && (row < virtualScreen.rows)  )
   {
      if (cursorVisible)
      {
         TxLinSetDevCursorPos( col, row);       // set and show device cursor
         TxLinShowDevCursor();
      }
   }
   else                                         // cursor outside screen
   {
      TxLinHideDevCursor();                     // hide device cursor
   }
   fflush( stdout);
}                                               // end 'TxLinSetCursorPos'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// make cursor invisible
/*****************************************************************************/
static void TxLinHideCursor
(
   void
)
{
   TxLinHideDevCursor();
   fflush( stdout);
   cursorVisible = FALSE;
}                                               // end 'TxLinHideCursor'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// make cursor visible
/*****************************************************************************/
static void TxLinShowCursor
(
   void
)
{
   cursorVisible = TRUE;
   TxLinSetCursorPos( virtualCurRow, virtualCurCol);
}                                               // end 'TxLinShowCursor'
/*---------------------------------------------------------------------------*/
#endif

/*****************************************************************************/
// Draw characters to the actual display terminal for specified fragment
/*****************************************************************************/
static void TxLinUpdateFragment
(
   short               start,
   short               end
)
{
   char                display_chr;             // translated for display
   BYTE                color;
   BYTE                bg, fg;
   short               col     = start % virtualScreen.cols;
   short               row     = start / virtualScreen.cols;
   TXCELL             *display = displayScreen.buffer + start;
   TXCELL             *virtual = virtualScreen.buffer + start;
   TXCELL             *lastOne = virtualScreen.buffer + end;

   TxLinSetDevCursorPos( col, row);             // go to start of fragment

   while (virtual != lastOne)
   {
      color = virtual->at;
      if ((color      != currentColor) &&       // color change needed
          (ansi[0][0] != 0))                    // and ansi available
      {
         currentColor = color;                  // remember RAW color value

         if (txl16ColorsBg == FALSE)            // high-bit in BG may cause
         {                                      // blink instead of bright!
            bg = Ccbg(color);
            fg = Ccfg(color);
            if (bg & CcI)                       // bright background ?
            {
               if      (bg == CcI)              // bright black  BG ?
               {
                  bg = CcY;                     // Make BG Yellow (brown)
               }
               else if (bg  > CcI)              // other brights
               {
                  bg &= ~CcI;                   // make normal BG
               }
               if (bg == fg)                    // if fg same as bg now
               {
                  fg |=  CcI;                   // make fg bright
               }
            }
            color = Ccol(fg,bg);
         }
         printf( "%s%s", ansi[NORMAL],          // Complete ANSI color string
                         ansi[color]);
      }                                         // filtered for brightness

      if (virtual->ch == 0)                     // cleared or empty screen
      {                                         // with zeroes should display
         display_chr = ' ';                     // as regular spaces!
      }
      else if ((display_chr = txlTransTable[(unsigned char)virtual->ch]) != 0)
      {
         if  (alternateChr == FALSE)            // and not active yet
         {
            if (txlTermCap_as[0] != 0)
            {
               fwrite( txlTermCap_as, 1, strlen(txlTermCap_as), stdout);
            }
            alternateChr = TRUE;
         }
      }
      else
      {
         display_chr = virtual->ch;             // un-translated ch value
         if (alternateChr)                      // alternate still active
         {
            if (txlTermCap_ae[0] != 0)          // use standard charset
            {
               fwrite( txlTermCap_ae, 1, strlen(txlTermCap_ae), stdout);
            }
            alternateChr = FALSE;
         }
      }
      fwrite( &display_chr, 1, 1, stdout);      // write the actual character
      *display++ = *virtual++;                  // copy to display-reference
   }
}                                               // end 'TxLinUpdateFragment'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// refresh specified area in display-buffer from virtual one
/*****************************************************************************/
static void TxLinUpdateArea
(
   short               refresh_start,           // start offset in virtual
   short               refresh_end              // end offset
)
{
   TXCELL             *display = displayScreen.buffer + refresh_start;
   TXCELL             *virtual = virtualScreen.buffer + refresh_start;
   TXCELL             *img_end = virtualScreen.buffer + refresh_end;
   short               cur_col = displayCurCol;
   short               cur_row = displayCurRow; // save cursor position
   int                 start;

   if (txlTermCap_ae[0] != 0)                   // start with standard charset
   {
      fwrite( txlTermCap_ae, 1, strlen(txlTermCap_ae), stdout);
   }
   alternateChr = FALSE;
   while (virtual < img_end)                    // repeat until end of area
   {
      //- find start of next fragment that is different
      while ((virtual < img_end) &&
             (*((short*) virtual) == *((short*) display)))
      {
         virtual++;
         display++;
      }
      if (virtual < img_end)                    // change found
      {
         start = virtual - virtualScreen.buffer;

         //- find end of differing fragment
         while ((virtual < img_end) &&
                (*((short*) virtual) != *((short*) display)))
         {
            virtual++;
            display++;
         }
         TxLinUpdateFragment( start, (virtual - virtualScreen.buffer));
      }
   }
   if (txlTermCap_ae[0] != 0)                   // end with standard charset
   {
      fwrite( txlTermCap_ae, 1, strlen(txlTermCap_ae), stdout);
   }
   TxLinSetDevCursorPos( cur_col, cur_row);     // restore cursor position
   fflush( stdout);
}                                               // end 'TxLinUpdateArea'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// refresh whole screen
/*****************************************************************************/
void TxLinUpdateScreen
(
   BOOL                finished                 // IN    Virtual updates done
)
{
   static ULONG        VirtualUpdateLevel = 0;  // all virtual updates done

   if (finished == FALSE)                       // starting virtual update
   {
      VirtualUpdateLevel++;                     // postpone real screen update
   }                                            // until matching call
   else
   {
      if (VirtualUpdateLevel > 0)
      {
         VirtualUpdateLevel--;
      }
      if (VirtualUpdateLevel == 0)              // finished outer-level
      {
         TxLinUpdateArea( 0, virtualScreen.rows * virtualScreen.cols);
      }
   }
}                                               // end 'TxLinUpdateScreen'
/*---------------------------------------------------------------------------*/

#if defined (NEVER)                             // not used yet
/*****************************************************************************/
// refresh multiple lines on display
/*****************************************************************************/
static void TxLinUpdateLines
(
   short               first,                   // IN    first line to refresh
   short               last                     // IN    last line to refresh
)
{
   if ((first >= 0) && (first <= last) && (last < virtualScreen.rows))
   {
      TxLinUpdateArea( first    * virtualScreen.cols,
                      (last +1) * virtualScreen.cols);
   }
}                                               // end 'TxLinUpdateLines '
/*---------------------------------------------------------------------------*/
#endif

/*****************************************************************************/
// Get console/window display size
/*****************************************************************************/
static void TxLinGetScreenSize
(
   short              *cols,
   short              *rows
)
{
   struct winsize      screen;                  // describes whole screen

   *cols = 80;                                  // safe defaults for both
   *rows = 25;

   if (TxaExeSwitchSet(TXA_O_SCREEN))           // explicit screen size given
   {
      sscanf( TxaExeSwitchStr(TXA_O_SCREEN, "", ""), "W%hdx%hd", cols, rows);
      TRACES(("SCREEN switch: '%s' cols: %hd rows: %hd", TxaExeSwitchStr(TXA_O_SCREEN, "", ""), *cols, *rows));
   }
   else
   {
      if (ioctl( fileno( stdin), TIOCGWINSZ, &screen) == 0)
      {
         *cols = screen.ws_col;
         *rows = screen.ws_row;
      }
   }
}                                               // end 'TxLinGetScreenSize'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set global screen buffers to specified size
/*****************************************************************************/
static ULONG TxLinAllocScrBuffers
(
   short               cols,                    // IN    number of columns
   short               rows                     // IN    number of rows
)
{
   ULONG               rc   = NO_ERROR;
   int                 size = (int) (rows * cols);

   TxFreeMem( virtualScreen.buffer);            // free existing buffers
   TxFreeMem( displayScreen.buffer);

   if ((displayScreen.buffer = TxAlloc(size,  sizeof(TXCELL))) != NULL)
   {
      displayScreen.rows = rows;
      displayScreen.cols = cols;
      TxLinClearTerminal();

      if ((virtualScreen.buffer = TxAlloc(size,  sizeof(TXCELL))) != NULL)
      {
         virtualScreen.rows = rows;
         virtualScreen.cols = cols;
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   return (rc);
}                                               // end 'TxLinAllocScrBuffers'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Clear terminal screen buffer, forcing complete redraw on next Update
/*****************************************************************************/
void TxLinClearTerminal
(
   void
)
{
   int                 size = displayScreen.rows * displayScreen.cols;

   memset( displayScreen.buffer, 0xff, (size * sizeof(TXCELL)));
}                                               // end 'TxLinClearTerminal'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set cursor position on display device
/*****************************************************************************/
static void TxLinSetDevCursorPos
(
   short               col,
   short               row
)
{
   printf( "\033[%d;%dH", row+1, col+1);
   displayCurCol = col;
   displayCurRow = row;
}                                               // end 'TxLinSetDevCursorPos'
/*---------------------------------------------------------------------------*/

#if defined (USEWINDOWING)
/*****************************************************************************/
// Make cursor invisible on display device
/*****************************************************************************/
static void TxLinHideDevCursor
(
   void
)
{
// fwrite( "\033[?25l\033[?1c", sizeof(char), 11, stdout);
   fwrite( "\033[?25l", 1, 6, stdout);
}                                               // end 'TxLinHideDevCursor'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Make cursor visible on display device
/*****************************************************************************/
static void TxLinShowDevCursor
(
   void
)
{
   if (cursorInsert)                            // to be refined
   {
   }
   else
   {
   }
// fwrite( "\033[?25h\033[?0c", sizeof(char), 11, stdout);
   fwrite( "\033[?25h", 1, 6, stdout);
}                                               // end 'TxLinShowDevCursor'
/*---------------------------------------------------------------------------*/
#endif

#elif defined (WIN32)

/*****************************************************************************/
// Check if output buffer has been initialized
/*****************************************************************************/
static int TxWntConsoleReady
(
   void
)
{
   int                 rc = TRUE;

   if (virtualScrBuf == NULL)
   {
      virtualScrBuf = GetStdHandle(STD_OUTPUT_HANDLE);
      if (virtualScrBuf == INVALID_HANDLE_VALUE)
      {
         fprintf(stderr, "WIN32 CONSOLE ERROR: invalid handle for STDOUT\n");
      }
   }
   return rc;
}
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// get the current status of the output buffer
/*****************************************************************************/
static int TxWntGetScrInfo
(
   HANDLE                      buffer,
   PCONSOLE_SCREEN_BUFFER_INFO info
)
{
   int                 rc;

   if ((rc = TxWntConsoleReady()) != FALSE)
   {
      if (GetConsoleScreenBufferInfo(buffer, info) == FALSE)
      {
         rc = FALSE;
      }
   }
   return rc;
}
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// display CONSOLE info structure details
/*****************************************************************************/
void txwDisplayConsoleInfo
(
   void
)
{
   CONSOLE_SCREEN_BUFFER_INFO info;

   if (TxWntGetScrInfo( virtualScrBuf, &info))
   {
      TxPrint("\n");
      TxPrint( "dwSize.X: %3hd  Cursor.X: %3hd  maxWin.X: %3hd\n",
           info.dwSize.X, info.dwCursorPosition.X, info.dwMaximumWindowSize.X);
      TxPrint( "dwSize.Y: %3hd  Cursor.Y: %3hd  maxWin.Y: %3hd\n",
           info.dwSize.Y, info.dwCursorPosition.Y, info.dwMaximumWindowSize.Y);
      TxPrint( "srWindow.Left %3hd  Right: %3hd  Top: %3hd  Bottom: %3hd\n\n",
           info.srWindow.Left, info.srWindow.Right,
           info.srWindow.Top,  info.srWindow.Bottom);
   }
}                                               // end 'txwDisplayConsoleInfo'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// set the foreground color attribute
/*****************************************************************************/
static void TxWntSetFgCol
(
   TX_COLOR_VALUE      color                    // IN    color value
)
{
   WORD attr;
   CONSOLE_SCREEN_BUFFER_INFO info;

   if (TxWntGetScrInfo(virtualScrBuf, &info))
   {
      attr = color | (info.wAttributes & 0x00f0);
      SetConsoleTextAttribute(virtualScrBuf, attr);
   }
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// set the background color attribute
/*****************************************************************************/
static void TxWntSetBgCol
(
   TX_COLOR_VALUE      color                    // IN    color value
)
{
   WORD attr;
   CONSOLE_SCREEN_BUFFER_INFO info;

   TxWntGetScrInfo(virtualScrBuf, &info);
   if (TxWntGetScrInfo(virtualScrBuf, &info))
   {
      attr = (color << 4) | (info.wAttributes & 0x000f);
      SetConsoleTextAttribute(virtualScrBuf, attr);
   }
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// set the text color attribute
/*****************************************************************************/
static void txNtSet_color
(
   WORD                color                    // IN    combined color value
)
{
   CONSOLE_SCREEN_BUFFER_INFO info;

   if (TxWntGetScrInfo(virtualScrBuf, &info))
   {
      TxWntGetScrInfo(virtualScrBuf, &info);
      SetConsoleTextAttribute(virtualScrBuf, color);
   }
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// get the size of display area, horizontal
/*****************************************************************************/
static int TxWntSx
(
   void
)
{
   CONSOLE_SCREEN_BUFFER_INFO info;

   TxWntConsoleReady();
   TxWntGetScrInfo(virtualScrBuf, &info);
   return info.dwSize.X;
}
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// get the size of display area, vertical
/*****************************************************************************/
static int TxWntSy
(
   void
)
{
   CONSOLE_SCREEN_BUFFER_INFO info;

   TxWntGetScrInfo(virtualScrBuf, &info);
   return info.dwSize.Y;
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// get the position of the cursor, horizontal
/*****************************************************************************/
static int TxWntCx
(
   void
)
{
   CONSOLE_SCREEN_BUFFER_INFO info;

   TxWntGetScrInfo(virtualScrBuf, &info);
   return info.dwCursorPosition.X;
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// get the position of the cursor, vertical
/*****************************************************************************/
static int TxWntCy
(
   void
)
{
   CONSOLE_SCREEN_BUFFER_INFO info;

   TxWntGetScrInfo(virtualScrBuf, &info);
   return info.dwCursorPosition.Y;
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// set the position of the cursor
/*****************************************************************************/
static void TxWntGo
(
   int                 x,                       // IN    horizontal position
   int                 y                        // IN    horizontal position
)
{
   COORD                      new_pos;
   CONSOLE_SCREEN_BUFFER_INFO info;

   new_pos.X = x;
   new_pos.Y = y;

   if (TxWntGetScrInfo(virtualScrBuf, &info))
   {
      if (new_pos.X < 0)
      {
         new_pos.X = 0;
      }
      if (new_pos.Y < 0)
      {
         new_pos.Y = 0;
      }

      if (new_pos.X >= info.dwSize.X)           // limit to Virtual window,
      {                                         // not the visible one
         new_pos.X   = info.dwSize.X -1;        // (dwMaximumWindow ...)
      }

      if (new_pos.Y >= info.dwSize.Y)
      {
         new_pos.Y   = info.dwSize.Y - 1;
      }
      SetConsoleCursorPosition(virtualScrBuf, new_pos);
   }
}
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// clear the screen
/*****************************************************************************/
static void TxWntClearScreen
(
   void
)
{
   COORD               home = {0, 0};
   DWORD               size;
   DWORD               bytes;
   CONSOLE_SCREEN_BUFFER_INFO info;

   if (TxWntGetScrInfo(virtualScrBuf, &info))
   {
      size = info.dwSize.X * info.dwSize.Y;
      FillConsoleOutputAttribute( virtualScrBuf, info.wAttributes,
                                                      size, home, &bytes);
      FillConsoleOutputCharacter( virtualScrBuf, ' ', size, home, &bytes);
      SetConsoleCursorPosition(virtualScrBuf, home);
   }
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// clear to end of line
/*****************************************************************************/
static void TxWntClrToEndOfLine
(
   void
)
{
   DWORD               bytes;
   CONSOLE_SCREEN_BUFFER_INFO info;

   if (TxWntGetScrInfo(virtualScrBuf, &info))
   {
      FillConsoleOutputCharacter( virtualScrBuf, ' ',
                                  info.dwSize.X - info.dwCursorPosition.X,
                                  info.dwCursorPosition, &bytes);
   }
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// write one character to the console
/*****************************************************************************/
static void TxWntWriteChar
(
   char                ch                       // IN    character to write
)
{
   DWORD               bytes;

   if (TxWntConsoleReady())
   {
      WriteConsole(virtualScrBuf, &ch, 1, &bytes, NULL);
   }
}                                               // end 'TxWntWriteChar'
/*---------------------------------------------------------------------------*/


#if defined (USEWINDOWING)
/*****************************************************************************/
//- write string to console, limit to length (clipping)
/*****************************************************************************/
static void TxWntWriteString
(
   char               *str,                     // IN    string pointer
   short               length                   // IN    length to write
)
{
   DWORD               bytes;

   if (TxWntConsoleReady())
   {
      WriteConsole( virtualScrBuf, str, (int) length, &bytes, NULL);
   }
}                                               // end 'TxWntWriteString'
/*---------------------------------------------------------------------------*/
#endif                                          // USEWINDOWING

//- ANSI parser definitions and functions
#define MAX_ARG_LEN  4
#define NUM_ARGS    10

#define NUL 0
#define ESC 27

/*****************************************************************************/
// Parse ANSI command
/*****************************************************************************/
static void TxWntParseAnsi
(
   char                *current,
   int                 *error
)
{
   static int           x = 0;
   static int           y = 0;

   switch (*current)
   {
      case 'A': TxWntGo(TxWntCx(), TxWntCy() - 1); break;
      case 'B': TxWntGo(TxWntCx(), TxWntCy() + 1); break;
      case 'C': TxWntGo(TxWntCx() + 1, TxWntCy()); break;
      case 'D': TxWntGo(TxWntCx() - 1, TxWntCy()); break;
      case 'H': TxWntGo(0, 0);                     break;
      case 'K': TxWntClrToEndOfLine();             break;
      case 's': x = TxWntCx(); y = TxWntCy();      break;
      case 'u': TxWntGo(x, y);                     break;
      default : *error = 1;                        break;
   }
}
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Parse ANSI argument
/*****************************************************************************/
static char *TxWntParseArgument
(
   char               *current,
   int                *error
)
{
   int                 i;
   int                 count     = 0;
   int                 intensity = 0;
   int                 argument[NUM_ARGS];

   if ((*current == '2') && (*(current + 1) == 'J'))
   {
      ++current;
      TxWntClearScreen();
   }
   else
   {
      while (isdigit(*current) && (count <= NUM_ARGS))
      {
         int index = 0;
         char arg[MAX_ARG_LEN];

         while (isdigit(*current) && (index < MAX_ARG_LEN))
         {
            arg[index++] = *current++;
         }

         if (index == MAX_ARG_LEN)
         {
            *error = 1;
         }
         else
         {
            arg[index] = NUL;
         }

         argument[count++] = atoi(arg);

         if (*current == ';')
         {
            ++current;
         }
      }

      if (count == NUM_ARGS)
      {
         *error = 1;
      }
      else
      {
         switch (*current)
         {
            case 'A':
               if (count == 1)
               {
                  TxWntGo(TxWntCx(), TxWntCy() - argument[0]);
               }
               else
               {
                  *error = 1;
               }
               break;

            case 'B':
               if (count == 1)
               {
                  TxWntGo(TxWntCx(), TxWntCy() + argument[0]);
               }
               else
               {
                  *error = 1;
               }
               break;

            case 'C':
               if (count == 1)
               {
                  TxWntGo(TxWntCx() + argument[0], TxWntCy());
               }
               else
               {
                  *error = 1;
               }
               break;

            case 'D':
               if (count == 1)
               {
                  TxWntGo(TxWntCx() - argument[0], TxWntCy());
               }
               else
               {
                  *error = 1;
               }
               break;

            case 'H':
               if (count == 2)
               {
                  TxWntGo(argument[1], argument[0]);
               }
               else
               {
                  *error = 1;
               }
               break;

            case 'm':
               for (i = 0; i < count; i++)
               {
                  switch (argument[i])
                  {
                     case  0 :
                        if (count == 1)         // color reset
                        {
                           TxWntSetBgCol(Black);
                           TxWntSetFgCol(Light_Gray);
                        }
                        else
                        {
                           intensity = 0;
                        }
                        break;

                     case  2 :
                        break;

                     case  1 :
                        intensity = TXwINT;     // Bright color
                        break;

                     case 30 : TxWntSetFgCol(Black      | intensity); break;
                     case 31 : TxWntSetFgCol(Red        | intensity); break;
                     case 32 : TxWntSetFgCol(Green      | intensity); break;
                     case 33 : TxWntSetFgCol(Brown      | intensity); break;
                     case 34 : TxWntSetFgCol(Blue       | intensity); break;
                     case 35 : TxWntSetFgCol(Magenta    | intensity); break;
                     case 36 : TxWntSetFgCol(Cyan       | intensity); break;
                     case 37 : TxWntSetFgCol(Light_Gray | intensity); break;
                     case 40 : TxWntSetBgCol(Black                 ); break;
                     case 41 : TxWntSetBgCol(Red                   ); break;
                     case 42 : TxWntSetBgCol(Green                 ); break;
                     case 43 : TxWntSetBgCol(Brown                 ); break;
                     case 44 : TxWntSetBgCol(Blue                  ); break;
                     case 45 : TxWntSetBgCol(Magenta               ); break;
                     case 46 : TxWntSetBgCol(Cyan                  ); break;
                     case 47 : TxWntSetBgCol(Light_Gray            ); break;
                     default : *error = 1;
                  }
               }
               break;

            default : *error = 1;
         }
      }
   }
   if (*error)
   {
      return NULL;
   }
   else
   {
      return current;
   }
}
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Parse one ANSI clause
/*****************************************************************************/
static char *TxWntParseClause
(
   char               *current
)
{
   int                 error = 0;
   char               *keep  = current;

   current++;
   if (*current != '[')
   {
      error = 1;
   }
   else
   {
      current++;
      if (!isdigit(*current))
      {
         TxWntParseAnsi(current, &error);
      }
      else
      {
         current = TxWntParseArgument(current, &error);
      }
   }

   if (error)
   {
      TxWntWriteChar(*keep);
      return keep;
   }
   else
   {
      return current;
   }
}
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// display a string on NT console, with full ANSI support
/*****************************************************************************/
void txNtConsoleDisplay
(
   char *text
)
{
   char               *current = text;

   while (*current != NUL)
   {
      switch (*current)
      {
         case ESC : current = TxWntParseClause(current); break;
         default  :           TxWntWriteChar( *current); break;
      }
      current++;
   }
}
/*---------------------------------------------------------------------------*/
#endif
