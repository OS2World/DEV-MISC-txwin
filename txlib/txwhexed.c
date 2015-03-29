//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2006 Fsys Software and Jan van Wijk
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
// TX standard dialogs
//
// Author: J. van Wijk
//
// JvW  11-06-2006 Initial version, hex editor dialog and control-class

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // txwa anchor interface

/*
 Minimum width is 76 for 16 byte/line (52 for 8 bytes, 142 for 32 bytes),
 when wider adds absolute position column. Layout for typical usage:

ÚÄÄÄÄÄÄÄÄÄÄ´ Partition 03 type:07 D: HPFS   size: 1114.8 MiB ÃÄÄÄÄÄÄÄÄÄÄ[X]¿
³[PgUp]   0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f [ItemUp]  [LineUp]³
³000000  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000010  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000020  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000030  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000040  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000050  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000060  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000070  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000080  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³000090  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³0000a0  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³0000b0  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³0000c0  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³0000d0  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³0000e0  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³0000f0  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF [0123456789abcdef]³
³[PgDn]  Cursor:012345  Modified  Absolute:0x11223344556 [ItemDn]  [LineDn]³
³LSN:0x00000010 = PSN:0x001ea547 = Cyl:  125 H:1   S:11   item-description ³
À´F1=Help F4=OK  Up/Dn=Row PgUp/PgDn=Page Alt-PgUp/PgDn=Item TAB=Hex/AsciiÃÙ
*/

#define TXWD_WID_HEXED    0xff80                // window id hex editor
#define TXWD_HELPHEXED    (TXWH_SYSTEM_SECTION_BASE + 800)
#define TXWD_HELPHECTL    (TXWD_HELPHEXED + 10)

static char           *hexedhelp[] =
{
   "#000 Hex editor, generic dialog",
   "",
   " The Hex editor dialog is a single window with just one",
   " control-window, the Hex-editor control window."
   "",
   " The dialog can be used by applications like sector editors",
   " and binary-file editors if they do not need additional",
   " functionality like multiple windows ...",
   "",
   " For help on using the Hex-editor control itself, refer to the",
   " previous help-screen that has all the details about operating",
   " the hex editor with the keyboard and mouse.",
   "",
   TXHELPITEM(10,"Hex editor control window")
   " The Hex editor control is used to view and update unstructered",
   " binary information in hexadecimal and ASCII format, as used by",
   " applications like sector editors and binary-file editors.",
   "",
   " For generic info on the Hex-editor and possibly the way it is",
   " to be used within the current application, press <F1> again ...",
   "",
   "",
   " The control window has a relative position displayed at the left,",
   " a HEX-byte area, an ASCII area and an absolute position shown on",
   " the far right side (screen-size permitting :-)",
   "",
   " The number of columns used will be set to 8, 16, 32, 64 or 128 by",
   " default, depending on the available window-width. It can be changed",
   " to any value between 1 and 133 using the <Alt>+arrow keys.",
   "",
   " The top and bottom lines include positioning information as well",
   " as scroll-buttons to be used with the mouse. The informational",
   " line at the very bottom is application defined, and contains",
   " information about the data (Item) being edited, like sector",
   " numbering in some form, and perhaps the type of the data.",
   "",
   " The 'Cursor:'   position is the RELATIVE byte position in the",
   "                 current item (sector/cluster) starting at 0.",
   "",
   " The 'Absolute:' position is the ABSOLUTE byte position in the",
   "                 whole edited object (disk, partition, file)",
   "                 also starting at 0.",
   "",
   " The following keys are used to operate the HEX editor control:",
   "",
   "   UP/DOWN/LEFT/RIGHT arrow   Move the cursor within the area",
   "                              moving to the PREVious and NEXT",
   "                              item when needed.",
   "",
   "   PgUp and PgDn              Move up or down by the size of the",
   "                              window (or data-item when smaller)",
   "                              moving to the PREVious and NEXT",
   "                              item when needed.",
   "",
   "   Alt+PgUp and Alt+PgDn      Move up or down by the size of the",
   "                              data-item (goto previous/next item)",
   "",
   "   Alt-1 .. Alt-0             Set mark at current cursor position",
   "                              with length 1 upto 10 bytes",
   "",
   "   Alt-B                      Block/Byte mark, set begin or end of",
   "                              of a byte-size oriented marked area",
   "                              in the CURRENT item buffer.",
   "",
   "   Alt-C                      Copy mark-data to current position",
   "                              (restricted to CURRENT buffer only)",
   "",
   "   Alt-E                      Mark from cursor to end of object",
   "   Alt-J                      Mark from begin of line to cursor",
   "   Alt-K                      Mark from cursor to end of line",
   "",
   "   Alt-L                      Line mark, set begin or end of a line",
   "                              oriented marked area in the CURRENT",
   "                              item buffer.",
   "",
   "   Alt-M                      Copy mark-data to current position",
   "                              then move the mark there as well.",
   "",
   "   Alt-R                      Reverse byte-order in marked area,",
   "                              useful for endian correction etc.",
   "",
   "   Alt-S                      Mark from begin of object to cursor",
   "",
   "   Alt-U                      Unmark, remove any byte or line mark",
   "",
   "   Ctrl-A                     Find Again, repeat the last find with",
   "                              same search arguments (if available),",
   "                              without presenting the dialog again",
   "",
   "   Ctrl-B   or  Alt+Home      Go Back to the item and position it",
   "                              was started with (initial sector)",
   "",
   "   Ctrl-D                     Delete all bytes AFTER current one (*)",
   "   Alt-D                      (see DELETE/INSERT, file-editing only)",
   "",
   "   Ctrl-E                     Erase/Fill whole item or marked area",
   "   Alt-F                      with specified HEX value, default 00",
   "",
   "   Ctrl-F                     Find data and go to the found item.",
   "   Ctrl-T                     Collapse edit-window, then Ctrl-F",
   "                              (search is application specific)",
   "",
   "   Ctrl-G                     Go to other item, to be specified by",
   "                              user, using sector number or other",
   "                              selection. (application specific)",
   "",
   "   Ctrl-W                     Write back the curent item (sector).",
   "                              Normally this is done automatically",
   "                              on moving to other items or at exit.",
   "",
   "   Ctrl-Z                     Undo changes to CURRENT item (sector).",
   "                              This reverts the item to the state it",
   "                              had when loaded (from disk) last time.",
   "                              To correct mistakes or to allow moving",
   "                              to another item after a write failure.",
   "",
   "   Ctrl+UP/DOWN/LEFT/RIGHT    Shift the cursor within the visible",
   "                              window without changing the actual",
   "                              cursor position in the data item.",
   "                              Can be used to align certain bytes",
   "                              with the start of the line/window.",
   "",
   "   Alt+LEFT/RIGHT             Decrease or increase the number of",
   "                              columns used in the data area, and",
   "                              switch off auto 8/16/32/64/128 column",
   "                              selection when resizing the window.",
   "                              Number of columns can be 1 .. 133",
   "",
   "   ALT+F10                    Maximize the edit window, most columns",
   "",
   "   Alt+DOWN                   Return to auto 8/16/32/64/128 columns",
   "",
   "   0..9 and a..f or A..F      Change bytes values when in HEX area",
   "",
   "   Any valid ASCII key        Change bytes values when in ASCII area",
   "",
   "   BACKSPACE  in HEX   area   Change byte value to 0x00, move LEFT",
   "   BACKSPACE  in ASCII area   Change byte value to 0x20, move LEFT",
   "",
   "   DELETE                     Delete byte at the cursor position (*)",
   "   INSERT                     Insert byte at the cursor position (*)",
   "",
   "     *                        Insert or Delete may be possible at any",
   "                              position, at the end of the object only,",
   "                              or not all, as defined by application.",
   "                              (Usually possible for binary file edit)",
   "",
   "   F1                         Show help on the Hex-edit control",
   "",
   "   F2                         Activate alternate-format viewer, like a",
   "                              disassembly module application-specific",
   "",
   "   F3, F4, Enter or Esc       End the Hex-edit dialog and write",
   "                              back changes in the current item.",
   "",
   "   F8  (or TAB)               Switch edit-cursor between the HEX",
   "                              and ASCII data area. The TAB key",
   "                              may not be available inside a more",
   "                              complex dialog where it switches",
   "                              between multiple control windows.",
   "",
   "   F9                         Switch position indicators between",
   "                              the default hexadecimal and decimal",
   "                              representation",
   "",
   "",
   " Whenever moving the cursor causes a change from one item to the",
   " next or previous one, any changes to the current item will be",
   " written back. The application may provide a confirmation popup",
   " in that case to allow user intervention.",
   "",
   "",
   " The MOUSE can be used to set the cursor position, and to MOVE",
   " the cursor in the data area using the shown 'scroll buttons'.",
   " Clicking the TOP or BOTTOM 'button' lines outside the buttons",
   " will move the cursor UP or DOWN, clicking on the left (position)",
   " column moves the cursor LEFT and clicking beyond the ASCII area",
   " on the right will move the cursor to the RIGHT ...",
   "",
   "",
   " Once help is displayed, another <F1> will call up the help info",
   " about the application level dialog or generic hex-editor dialog",
   "",
   NULL
};

#define TXHE_CUR_POS        15
#define TXHE_ABS_POS        42
#define TXHE_AC_COLUMN(c)   (((c) * 3) + 8)
#define TXHE_AC_08_COLS     32
#define TXHE_AC_16_COLS     56
#define TXHE_AC_32_COLS     128
#define TXHE_AC_64_COLS     198
#define TXHE_AC_128_COLS    398
#define TXHE_ABS_WIDTH      14
#define TXHE_HIDE_CURSOR    0

static char txwhe_footer[]  = "F1=Help F2=%s F4=OK F8=Hex/Ascii F9=decimal "
                              "PgUp/PgDn=Page Alt-PgUp/PgDn=Item";

// Prepare CRC and difference buffer handling for current data buffer
static void txtHexEditPrepareBuf
(
   TXHEXEDIT         *he                        // IN    HEXEDIT data structure
);

// Show or hide real and 'alternate' cursor in current location, use modify
static void txwHexEditShowCursor
(
   TXWHANDLE           hwnd,                    // IN    current window
   BOOL                show,                    // IN    show in cursor color
   BOOL                mod,                     // IN    buffer is modified
   TXHEXEDIT          *he                       // IN    HEXEDIT data structure
);

/*****************************************************************************/
// Initialize the Hex-editor dialog/control data structures
/*****************************************************************************/
BOOL txwInitHexeditDialogs
(
   void
)
{
   BOOL                rc = TRUE;               // function return

   ENTER();

   txwRegisterHelpText( TXWD_HELPHEXED, "txhexed", "Hex Editor help",  hexedhelp);

   BRETURN (rc);
}                                               // end 'txwInitHexeditDialogs'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display standard Hex editor dialog, supporting 8 or 16 byte wide display
/*****************************************************************************/
ULONG txwHexEditor
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXHEXEDIT          *hedat,                   // IN    hex edit data
   char               *title,                   // IN    title for the dialog
   ULONG               helpid,                  // IN    help on hexedit
   ULONG               flags                    // IN    specification flags
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRACES(("parent:%8.8lx  owner:%8.8lx  title:'%s'\n", parent, owner, title));
   TRACES(("helpid:%8.8lx = %lu  flags:%8.8lx\n", helpid, helpid, flags));
   TRHEAP(1);

   if (txwIsWindow( TXHWND_DESKTOP) && (hedat->curr != NULL))
   {
      TXRECT           position;                // reference size/position
      TXWHANDLE        eframe;                  // editor frame
      TXWHANDLE        ehexed;                  // editor hexedit control
      TXWINDOW         window;                  // setup window data
      ULONG            style;
      short            phsize;                  // parent window width
      short            pvsize;                  // parent window height
      int              lines;                   // nr of lines in window
      int              width;                   // nr of columns
      TXTM             footer;

      txwInitHexeditDialogs();                  // attach help-texts

      txwQueryWindowRect( parent, FALSE, &position);
      phsize = position.right;
      pvsize = position.bottom;

      if (hedat->rows == 0)                     // match to full-screen
      {
         hedat->rows = pvsize - 5;
      }
      lines = hedat->rows + 5;

      if ((hedat->autocols) || (hedat->cols == 0))
      {
         hedat->autocols = TRUE;
         hedat->cols = (phsize > TXHE_AC_128_COLS) ? 128 :
                       (phsize > TXHE_AC_64_COLS)  ?  64 :
                       (phsize > TXHE_AC_32_COLS)  ?  32 :
                       (phsize > TXHE_AC_16_COLS)  ?  16 : 8;
      }

      while (hedat->posCursor >= (hedat->rows * hedat->cols))
      {
         hedat->posCursor -= hedat->cols;
         hedat->posCurBuf += hedat->cols;
      }

      width = hedat->cols * 4 + 12;             // required width
      if (width + TXHE_ABS_WIDTH <= phsize)
      {
         width += TXHE_ABS_WIDTH;               // add room for Abs column
      }

      if (position.left  + width < phsize)      // centered
      {
         position.left   = ((phsize - width -1) / 2);
      }
      if (position.top   + lines < pvsize)
      {
         position.top   += ((min( pvsize, 60) - lines) / 4);
      }
      TRECTA( "pos/size", (&position));

      style = TXWS_DIALOG | TXWS_DISABLED | TXCS_CLOSE_BUTTON;
      if (flags & TXHE_MOVEABLE)
      {
         style |= TXWS_MOVEABLE;                // make frame move/sizeable
      }
      sprintf( footer, txwhe_footer, hedat->altDispText);
      txwSetupWindowData(
         position.top,                          // upper left corner
         position.left,
         lines,                                 // vertical size
         width,                                 // horizontal size
         style | TXWS_CAST_SHADOW,              // window frame style
         (helpid) ? helpid : TXWD_HELPHEXED,
         ' ', ' ', TXWSCHEME_COLORS,
         title, footer,
         &window);
      window.st.buf     = NULL;                 // NO artwork attached
      window.dlgFocusID = TXWD_WID_HEXED;       // Field to get Focus
      eframe = txwCreateWindow( parent, TXW_FRAME, 0, 0, &window, NULL);

      style = TXWS_HEXECTL;
      if (flags & TXHE_MOVEABLE)
      {
         style |= TXWS_HCHILD_SIZE;             // resize with parent
         style |= TXWS_VCHILD_SIZE;
      }
      txwSetupWindowData(
         0, 0,                                  // upper left corner
         lines -2,                              // vertical size
         width -2,                              // horizontal size
         style,                                 // window frame style
         TXWD_HELPHECTL,                        // help on hexedit control
         ' ', ' ', TXWSCHEME_COLORS, "",  "",
         &window);
      window.he = *hedat;
      ehexed = txwCreateWindow( eframe, TXW_HEXEDIT, eframe, 0, &window,
                                txwHexEditWinProc);
      txwSetWindowUShort( ehexed, TXQWS_ID, TXWD_WID_HEXED);

      rc = txwDlgBox( parent, owner, txwDefDlgProc, eframe, NULL);
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwHexEditor'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Prepare CRC and difference buffer handling for current data buffer
/*****************************************************************************/
static void txtHexEditPrepareBuf
(
   TXHEXEDIT          *he                       // IN    HEXEDIT data structure
)
{
   ENTER();

   if (he->curr && he->curr->data && he->curr->size)
   {
      he->currCrc = TxCrc32( he->curr->data, he->curr->size);
      if (he->diffSize < he->curr->size)
      {
         TxFreeMem( he->diff);                  // free if too small
      }
      if (he->diff == NULL)                     // allocate when needed
      {
         he->diffSize = max( 512, he->curr->size); // minimum alloc to avoid
         he->diff     = TxAlloc( 1, he->diffSize); // frequent re-alloc
      }
      else
      {
         memset( he->diff, 0, he->diffSize);
      }
      if (he->diff != NULL)                     // make fresh diff copy
      {
         TRACES(( "Copy CURR to DIFF\n"));
         memcpy( he->diff, he->curr->data, he->curr->size);
      }
   }
   VRETURN();
}                                               // end 'txtHexEditPrepareBuf'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Hex Editor, paint window for the HEXED class
/*****************************************************************************/
void txwPaintHexEdit
(
   TXWINBASE          *wnd                      // IN    current window
)
{
   TXWHANDLE           hwnd = (TXWHANDLE) wnd;
   TXWINDOW           *win  = wnd->window;
   TXLN                ascii;                   // ascii buffer
   TX1K                line;                    // line  buffer
   TXTS                value;                   // to be displayed
   TXHEXEDIT          *dat = &win->he;
   USHORT              brow;
   USHORT              bcol;
   LONG                bIndex;
   short               sy = win->client.bottom - win->client.top   +1;
   short               sx = win->client.right  - win->client.left  +1;
   short               ln = 0;                          // line-nr while painting
   short               cl;                              // column  while painting
   short               ac;                              // ascii  column start col
   short               tc;                              // tail column, after ascii
   BYTE                posColor;
   BYTE               *buffer;
   LONG                offset;
   BOOL                modified = FALSE;
   int                 pad;
   char               *hl;

   ENTER()

   dat->rows =  sy - 3;                         // adapt to resizing
   if (dat->autocols)
   {
      dat->cols = (sx > TXHE_AC_128_COLS) ? 128 :
                  (sx > TXHE_AC_64_COLS)  ?  64 :
                  (sx > TXHE_AC_32_COLS)  ?  32 :
                  (sx > TXHE_AC_16_COLS)  ?  16 : 8;
   }
   ac = TXHE_AC_COLUMN(dat->cols);
   tc = ac + dat->cols + 2;


   //- Draw top line with the 'Up' buttons and bytenumbers
   txwStringAt( ln,         0, "[",             TxwSC( cHexEdButBracket));
   txwStringAt( ln,         1, "PgUp",          TxwSC( cHexEdButtonText));
   txwStringAt( ln,         5, "]",             TxwSC( cHexEdButBracket));

   strcpy( line, "  ");
   for (bcol = 0, cl = dat->posCurBuf & 0x0f; bcol < dat->cols; bcol++, cl++)
   {
      sprintf( value, (dat->decimal) ? "%2d " : "%2x ", (cl % dat->cols));
      strcat( line, value);
   }
   txwStringAt( ln,         6, line,            TxwSC( cHexEdByteNumber));

   cl = ac;
   txwStringAt( ln,        cl++,  "[",          TxwSC( cHexEdButBracket));
   if (dat->cols >= 5)
   {
      txwStringAt( ln,     cl,    "ItemUp",     TxwSC( cHexEdButtonText));
      cl += 6;
      txwStringAt( ln,     cl++,  "]",          TxwSC( cHexEdButBracket));
   }
   else
   {
      txwStringAt( ln,     cl,    "Up",         TxwSC( cHexEdButtonText));
      cl += 2;
      txwStringAt( ln,     cl++,  "]",          TxwSC( cHexEdButBracket));
   }
   if (dat->cols >= 16)
   {
      txwStringAt( ln,     cl,    "  ",         TxwSC( cHexEdByteNumber));
      cl += 2;
      txwStringAt( ln,     cl++,  "[",          TxwSC( cHexEdButBracket));
      txwStringAt( ln,     cl,    "LineUp",     TxwSC( cHexEdButtonText));
      cl += 6;
      txwStringAt( ln,     cl++,  "]",          TxwSC( cHexEdButBracket));
   }
   if (tc > cl)
   {
      sprintf( line, "%*.*s", (tc - cl), (tc - cl), "");
      txwStringAt( ln,     cl,    line,         TxwSC( cHexEdByteNumber));
   }

   if (sx > tc)
   {
      if ((sx-tc) >= TXHE_ABS_WIDTH)            // add absolute byte positions
      {
         sprintf( line, " Abs byte Pos%*.*s", (sx-tc-13), (sx-tc-13), "");
         txwStringAt( ln,  tc,    line,         TxwSC( cHexEdByteNumber));
      }
      else
      {
         sprintf( line, "%*.*s", (sx-tc), (sx-tc), "");
         txwStringAt( ln,  tc,    line,         TxwSC( cHexEdByteNumber));
      }
   }
   if ((dat->curr) && (dat->curr->data))        // do we have any data ?
   {
      modified = (dat->currCrc != TxCrc32( dat->curr->data, dat->curr->size));
   }
   TRACES(("dat->pre2: %8.8lx dat->pre2->data: %8.8lx dat->pre2->size: %lu\n",
            dat->pre2,        dat->pre2->data,        dat->pre2->size));
   TRACES(("dat->prev: %8.8lx dat->prev->data: %8.8lx dat->prev->size: %lu\n",
            dat->prev,        dat->prev->data,        dat->prev->size));
   TRACES(("dat->curr: %8.8lx dat->curr->data: %8.8lx dat->curr->size: %lu dat->rows: %hu\n",
            dat->curr,        dat->curr->data,        dat->curr->size,     dat->rows));
   TRACES(("dat->next: %8.8lx dat->next->data: %8.8lx dat->next->size: %lu\n",
            dat->next,        dat->next->data,        dat->next->size));
   TRACES(("dat->nex2: %8.8lx dat->nex2->data: %8.8lx dat->nex2->size: %lu\n",
            dat->nex2,        dat->nex2->data,        dat->nex2->size));

   for (ln = 1, brow = 0; dat->curr && dat->curr->data && (brow < dat->rows); brow++, ln++)
   {
      BOOL marked = ((dat->markSize) && (dat->markBase == dat->curr->start));

      bIndex = brow * dat->cols;
      TRACES(("ln: %d, bIndex: %ld\n", ln, bIndex));

      if (dat->decimal)
      {
         sprintf( line, (dat->posCurBuf + bIndex < 0) ? "%5.5ld  " : "%6.6ld  ",
                         dat->posCurBuf + bIndex);
      }
      else
      {
         sprintf( line, "%6.6lX ", (dat->posCurBuf + bIndex) & 0xffffff);
      }
      offset = dat->posCurBuf + bIndex;
      if (offset < 0)                           // line is in PREV buffer
      {
         posColor = TxwSC( cHexEdRelPosPrev);
      }
      else if (offset < dat->curr->size)        // line is in CURR buffer
      {
         posColor = TxwSC( cHexEdRelPosCurr);
      }
      else                                      // line is in CURR buffer
      {
         posColor = TxwSC( cHexEdRelPosNext);
      }
      txwStringAt( ln, 0, line, posColor);
      txwStringAt( ln, 7, " ",  TxwSC( cHexEdHexByteStd));

      strcpy( line, "");                        // HEX bytes line
      memset( ascii, 0, TXMAXLN);
      for (bcol = 0; bcol < dat->cols; bcol++)
      {
         offset = dat->posCurBuf + bIndex + bcol;

         TRACES(("ln: %d, initial offset: %ld curr->size: %lu\n", ln, offset, dat->curr->size));

         if (offset < 0)
         {
            if (!dat->prev  ||  !dat->prev->data  ||
                (offset + (LONG) dat->prev->size) < 0)
            {
               if (!dat->pre2  ||  !dat->pre2->data  ||
                   (offset + (LONG) dat->pre2->size + (LONG) dat->prev->size) < 0)
               {
                  buffer = NULL;                // line is before PRE2 (draw --)
               }
               else
               {
                  buffer  = dat->pre2->data;    // line is in PRE2 buffer
                  offset += dat->pre2->size;
                  offset += dat->prev->size;
               }
            }
            else
            {
               buffer  = dat->prev->data;       // line is in PREV buffer
               offset += dat->prev->size;
            }
         }
         else if (offset < dat->curr->size)
         {
            buffer  = dat->curr->data;          // line is in CURR buffer
         }
         else if (dat->next && dat->next->data &&
                  (offset < dat->curr->size + dat->next->size))
         {
            buffer  = dat->next->data;          // line is in NEXT buffer
            offset -= dat->curr->size;
         }
         else
         {
            if (dat->nex2 && dat->nex2->data &&
                (offset < dat->curr->size + dat->nex2->size + dat->next->size))
            {
               buffer  = dat->nex2->data;       // line is in NEX2 buffer
               offset -= dat->curr->size;
               offset -= dat->next->size;
            }
            else
            {
               buffer  = NULL;                  // line is beyond NEX2 (draw --)
            }
         }

         TRACES(("ln: %d, buffer: %8.8lx  Buffer offset: %ld\n", ln, buffer, offset));

         if ((buffer) && (offset >= 0))
         {
            ascii[ bcol] = TxPrintable( buffer[ offset]);
            sprintf( value, "%2.2hhx ", buffer[ offset]);
            strcat( line, value);
         }
         else
         {
            ascii[ bcol] = ' ';
            strcat( line, "-- ");
         }
      }
      txwStringAt( ln,      8,    line,         TxwSC( cHexEdHexByteStd));

      if ((modified || marked) && (dat->curr->data)) // colored areas ?
      {
         BOOL          bmodif;                  // cursor pos is modified byte
         BOOL          inmark;                  // cursor pos is inside mark

         for (cl = 8, bcol = 0; bcol < dat->cols; bcol++, cl += 3)
         {
            bmodif = inmark = FALSE;
            offset = dat->posCurBuf + bIndex + bcol;

            //- use coloring inside the CURRENT buffer only!
            if ((offset >= 0) && (offset < dat->curr->size))
            {
               sprintf( value, "%2.2hhx ",  dat->curr->data[ offset]);
               if (modified && dat->diff && dat->curr->data)
               {
                  bmodif = (dat->curr->data[ offset] != dat->diff[ offset]);
               }
               if (marked)
               {
                  inmark = ((offset >= dat->markStart) &&
                            (offset < (dat->markStart + dat->markSize)));
                  if ((bcol   == (dat->cols -1)) || // end of line/end of mark
                      (offset == (dat->markStart + dat->markSize -1)))
                  {
                     value[2] = 0;              // strip space after HEX value
                  }
               }
               if (inmark)
               {
                  if (bmodif)
                  {
                     txwStringAt( ln, cl, value, TxwSC( cHexEdHexByteMch));
                  }
                  else
                  {
                     txwStringAt( ln, cl, value, TxwSC( cHexEdHexByteMrk));
                  }
               }
               else if (bmodif)
               {
                  txwStringAt( ln, cl, value,    TxwSC( cHexEdHexByteChg));
               }
            }
         }
      }

      txwStringAt( ln,     ac,    "[",          TxwSC( cHexEdAscBracket));
      txwStringAt( ln,     ac +1, ascii,        TxwSC( cHexEdAscByteStd));
      txwStringAt( ln,     tc -1, "]",          TxwSC( cHexEdAscBracket));

      if (sx > tc)
      {
         if ((sx-tc) >= TXHE_ABS_WIDTH)         // add absolute byte positions
         {
            sprintf( line, (dat->decimal) ? " %13.13llu" : " 0x%11.11llX",
                            dat->posCurBuf + bIndex + dat->curr->start);
            txwStringAt( ln, tc, line, TxwSC( cHexEdAbsBytePos));
            sprintf( line, "%*.*s", (sx-tc-TXHE_ABS_WIDTH),
                                    (sx-tc-TXHE_ABS_WIDTH), "");
            txwStringAt( ln, tc + TXHE_ABS_WIDTH, line, TxwSC( cHexEdAbsBytePos));
         }
         else                                   // just fill to end
         {
            sprintf( line, "%*.*s", (sx-tc), (sx-tc), "");
            txwStringAt( ln, tc, line, TxwSC( cHexEdAbsBytePos));
         }
      }

   }

   ln = dat->rows +1;                           // first line after hex data
   txwStringAt( ln,        0,    "[",           TxwSC( cHexEdButBracket));
   txwStringAt( ln,        1,    "PgDn",        TxwSC( cHexEdButtonText));
   txwStringAt( ln,        5,    "]",           TxwSC( cHexEdButBracket));

   if      (dat->cols < 4)
   {
      sprintf( line, " %s", (modified) ? "Mod" : "   ");
      txwStringAt( ln,     6,    line,          TxwSC( cHexEdModifyText));
      pad = (dat->cols - 1) * 3 +1;
      sprintf( line, "%*.*s", pad, pad , "");
      txwStringAt( ln,    10,    line,          TxwSC( cHexEdByteNumber));
   }
   else if (dat->cols < 8)
   {
      txwStringAt( ln,     6,    "  ",          TxwSC( cHexEdByteNumber));
      sprintf( line, "  %s", (modified) ? "Mod" : "   ");
      txwStringAt( ln,    14,    line,          TxwSC( cHexEdModifyText));
      pad = (dat->cols - 4) * 3 +1;
      sprintf( line, "%*.*s", pad, pad , "");
      txwStringAt( ln,    19,    line,          TxwSC( cHexEdByteNumber));
   }
   else
   {
      sprintf( line, "  %s  ", (modified) ? "Modified" : "        ");
      txwStringAt( ln,     6,    "  Cursor:",   TxwSC( cHexEdByteNumber));
      txwStringAt( ln,    21,    line,          TxwSC( cHexEdModifyText));
      if (dat->cols < 16)
      {
         pad = (dat->cols - 8) * 3;
         sprintf( line, "%*.*s", pad, pad , "");
         txwStringAt( ln, 33,    line,          TxwSC( cHexEdByteNumber));
      }
      else
      {
         txwStringAt( ln, 33, "Absolute:",      TxwSC( cHexEdByteNumber));
         pad = (dat->cols - 16) * 3;
         sprintf( line, "%*.*s", pad, pad , "");
         txwStringAt( ln, 56,    line,          TxwSC( cHexEdByteNumber));
      }
   }

   cl = ac;
   txwStringAt( ln,        cl++,  "[",          TxwSC( cHexEdButBracket));
   if (dat->cols >= 5)
   {
      txwStringAt( ln,     cl,    "ItemDn",     TxwSC( cHexEdButtonText));
      cl += 6;
      txwStringAt( ln,     cl++,  "]",          TxwSC( cHexEdButBracket));
   }
   else
   {
      txwStringAt( ln,     cl,    "Dn",         TxwSC( cHexEdButtonText));
      cl += 2;
      txwStringAt( ln,     cl++,  "]",          TxwSC( cHexEdButBracket));
   }
   if (dat->cols >= 16)
   {
      txwStringAt( ln,     cl,    "  ",         TxwSC( cHexEdByteNumber));
      cl += 2;
      txwStringAt( ln,     cl++,  "[",          TxwSC( cHexEdButBracket));
      txwStringAt( ln,     cl,    "LineDn",     TxwSC( cHexEdButtonText));
      cl += 6;
      txwStringAt( ln,     cl++,  "]",          TxwSC( cHexEdButBracket));
   }
   if (tc > cl)
   {
      sprintf( line, "%*.*s", (tc - cl), (tc - cl), "");
      txwStringAt( ln,     cl,    line,         TxwSC( cHexEdByteNumber));
   }
   if (sx > tc)
   {
      sprintf( line, "%*.*s", (sx-tc), (sx-tc), "");
      txwStringAt( ln,     tc,    line,         TxwSC( cHexEdByteNumber));
   }

   if (dat->curr)
   {
      sprintf( line, "%-*.*s", sx, sx, dat->curr->desc);
      txwStringAt( ln +1,      0, line,            TxwSC( cHexEdItemSnText));
   }

   if ((hl = strstr( line, ":0")) != NULL)      // support at most two
   {                                            // highlighted numbers
      char            *sp;                      // starting with ':0'

      TxCopy( ascii, hl +1, TXMAXLN);
      if ((sp = strchr( ascii, ' ')) != NULL)
      {
         *sp = 0;                               // terminate ':0' at 1st space
      }
      txwStringAt( ln +1, hl - line +1, ascii,  TxwSC( cHexEdItemHlight));

      if ((hl = strstr( hl +1, ":0")) != NULL)
      {
         TxCopy( ascii, hl +1, TXMAXLN);
         if ((sp = strchr( ascii, ' ')) != NULL)
         {
            *sp = 0;                            // terminate ':0' at 1st space
         }
         txwStringAt( ln +1, hl - line +1, ascii, TxwSC( cHexEdItemHlight));
      }
   }
   txwHexEditShowCursor( hwnd, TRUE, modified, dat);
   VRETURN();
}                                               // end 'txwPaintHexEdit'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Show or hide real and 'alternate' cursor in current location, use modify
/*****************************************************************************/
static void txwHexEditShowCursor
(
   TXWHANDLE           hwnd,                    // IN    current window
   BOOL                show,                    // IN    show in cursor color
   BOOL                mod,                     // IN    buffer is modified
   TXHEXEDIT          *he                       // IN    HEXEDIT data structure
)
{
   USHORT              rcol;
   USHORT              bcol  = he->posCursor % he->cols;
   USHORT              row   = he->posCursor / he->cols +1;
   short               ac    = TXHE_AC_COLUMN( he->cols);
   BOOL                bmod  = FALSE;           // current byte is modified
   BOOL                mark  = FALSE;           // current byte not in MARK
   LONG                index = he->posCurBuf + he->posCursor;
   TXTS                value;

   if ((he->rows > 0) && (index < he->curr->size))
   {
      BYTE             cbyte = 0;

      if (he->curr->data)
      {
         cbyte = he->curr->data[ index];
      }

      if (he->ascii)
      {
         if ((mod) && (he->diff))              // buffer has modified bytes
         {
            bmod = (cbyte != he->diff[ index]);
         }
         if ((he->markSize) && (he->markBase == he->curr->start))
         {                                      // we have a marked area
            mark = ((index >= he->markStart) &&
                    (index < (he->markStart + he->markSize)));
         }
         rcol = bcol + ac +1;
         bcol = bcol * 3 + 8;
         sprintf( value, "%2.2hhx", cbyte);
         txwStringAt( row, bcol, value,
            (show) ? (mark) ? (bmod) ? TxwSC( cHexEdHexByteChg) : TxwSC( cHexEdHexByteStd)
                            : (bmod) ? TxwSC( cHexEdCurByteChg) : TxwSC( cHexEdCursorByte)
                   : (mark) ? (bmod) ? TxwSC( cHexEdHexByteMch) : TxwSC( cHexEdHexByteMrk)
                            : (bmod) ? TxwSC( cHexEdHexByteChg) : TxwSC( cHexEdHexByteStd));
      }
      else
      {
         rcol  = bcol * 3 + 8 + he->hexLsb;     // real cursor position
         bcol += ac +1;
         sprintf( value, "%c", TxPrintable( cbyte));
         txwStringAt( row, bcol, value, (show) ? TxwSC( cHexEdCursorByte)
                                               : TxwSC( cHexEdAscByteStd));
      }
      if (show)                                 // setting new position
      {
         txwSetCursorPos( hwnd, row, rcol);     // set normal (real) cursor

         if (he->cols > 3)                      // update status line
         {
            sprintf( value, (he->decimal) ? "%6.6lu" : "%6.6lX",
                           ((he->posCurBuf + he->posCursor) & 0xffffff));
            txwStringAt( he->rows +1, (he->cols < 8) ? 8 : 15,
                         value, TxwSC( cHexEdRelCursorP));

            if (he->cols >= 16)
            {
               sprintf( value, (he->decimal) ? "%13.13llu " : "0x%11.11llX ",
                               (he->posCurBuf + he->posCursor) + he->curr->start);
               txwStringAt( he->rows +1, TXHE_ABS_POS, value, TxwSC( cHexEdAbsByteCur));
            }
         }
      }
   }
}                                               // end 'txwHexEditShowCursor'
/*---------------------------------------------------------------------------*/


// Execute callback, but gracefull fail when not available
#define txwHexEditCallBack(act,he) ((callback) ? (callback)(act,he) : TX_CMD_UNKNOWN)


/*****************************************************************************/
// Window procedure for the HEXED window class, handling mouse and keyboard
// Called as TOP-LEVEL winproc! Handles (most) class specific stuff only
/*****************************************************************************/
ULONG txwHexEditWinProc                         // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
)
{
   ULONG               rc  = NO_ERROR;          // WinProc return code
   ULONG               wr  = NO_ERROR;          // write-back return code
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win;
   BOOL                upd = FALSE;             // window draw required
   BYTE                tmp;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);

   if ((wnd != NULL) && ((win = wnd->window) != NULL))
   {
      TXWHANDLE        owner    = (TXWHANDLE) wnd->owner;
      short            sy       = win->client.bottom - win->client.top  +1;
      TXHEXEDIT       *dat      = &win->he;
      TXHE_CALLBACK    callback = (TXHE_CALLBACK)  dat->actionCallback;
      LONG             offset   = dat->posCurBuf + dat->posCursor;
      LONG             wbytes   = dat->rows * dat->cols;
      BOOL             modified = FALSE;
      LONG             value;
      TXTS             str;

      TRACES(( "sy:%hu  offset:%ld  wbytes:%ld  currsize:%lu\n",
                sy,     offset,     wbytes, dat->curr->size));

      if ((dat->curr) && (dat->curr->data))
      {
         modified = (dat->currCrc != TxCrc32( dat->curr->data, dat->curr->size));
      }
      switch (msg)
      {
         case TXWM_CREATE:                      // hexed control just created
            wr = txwHexEditCallBack( TXHE_CB_INIT_POS, dat);
            txtHexEditPrepareBuf(dat);          // reset modify status & diff
            upd = TRUE;                         // even if no data at all :-)
            if (dat->altStart)
            {
               txwPostMsg( hwnd, TXWM_CHAR, mp1, TXa_F2); // use alternate view
            }
            break;

         case TXWM_CHAR:
            TRACES(( "key: %8.8lx\n", mp2));
            switch (mp2)                        // key value
            {
               case TXk_INSERT:                 // when at last byte of a file
                  if (txwHexEditCallBack( TXHE_CB_INSERT_1, dat) == TX_PENDING)
                  {
                     txtHexEditPrepareBuf(dat); // reset modify status & diff
                     if (dat->diff)
                     {                          // reset first DIFF byte copied
                        dat->diff[0] = 0;       // from newly created buffer so
                        dat->currCrc++;         // status will be 'modified'
                        modified = TRUE;
                     }
                  }
                  upd = TRUE;
                  break;

               case TXk_DELETE:
                  if (txwHexEditCallBack( TXHE_CB_DELETE_1, dat) == TX_PENDING)
                  {
                     txtHexEditPrepareBuf(dat); // reset modify status & diff
                  }
                  upd = TRUE;
                  break;

               case TXc_D:
               case TXa_D:
                  if (txwHexEditCallBack( TXHE_CB_DELTOEND, dat) == TX_PENDING)
                  {
                     txtHexEditPrepareBuf(dat); // reset modify status & diff
                  }
                  upd = TRUE;
                  break;

               case TXa_B:                      // byte/block mark
                  if (dat->markSize == 0)       // new mark
                  {
                     dat->markBase  = dat->curr->start;
                     dat->markStart = offset;
                     dat->markSize  = 1;
                  }
                  else                          // (re)size the mark
                  {
                     if      (offset > dat->markStart)
                     {
                        dat->markSize  = offset - dat->markStart + 1;
                     }
                     else if (offset < dat->markStart)
                     {
                        dat->markSize  = dat->markStart - offset + 1;
                        dat->markStart = offset;
                     }
                  }
                  upd = TRUE;
                  break;

               case TXa_E:                      // mark to end of object
                  dat->markStart = offset;
                  dat->markSize  = dat->curr->size - offset;
                  upd = TRUE;
                  break;

               case TXa_J:                      // mark from begin of line
                  dat->markBase  = dat->curr->start;
                  dat->markStart = offset - (dat->posCursor % dat->cols);
                  dat->markSize  = dat->posCursor % dat->cols +1;
                  upd = TRUE;
                  break;

               case TXa_K:                      // mark to end of line
                  dat->markBase  = dat->curr->start;
                  dat->markStart = offset;
                  dat->markSize  = dat->cols - (dat->posCursor % dat->cols);
                  upd = TRUE;
                  break;

               case TXa_L:                      // line mark
                  value = offset - (dat->posCursor % dat->cols);
                  if (dat->markSize == 0)       // new mark
                  {
                     dat->markBase  = dat->curr->start;
                     dat->markStart = value;
                     dat->markSize  = dat->cols;
                  }
                  else                          // (re)size the mark
                  {
                     if      (value > dat->markStart)
                     {
                        dat->markSize  = value - dat->markStart + dat->cols;
                     }
                     else if (value < dat->markStart)
                     {
                        dat->markSize  = dat->markStart - value + dat->cols;
                        dat->markStart = value;
                     }
                  }
                  upd = TRUE;
                  break;

               case TXa_1: case TXa_2: case TXa_3: case TXa_4: case TXa_5:
               case TXa_6: case TXa_7: case TXa_8: case TXa_9: case TXa_0:
                  dat->markBase  = dat->curr->start;
                  dat->markStart = offset;
                  switch (mp2)
                  {
                     case TXa_1: dat->markSize  =  1; break;
                     case TXa_2: dat->markSize  =  2; break;
                     case TXa_3: dat->markSize  =  3; break;
                     case TXa_4: dat->markSize  =  4; break;
                     case TXa_5: dat->markSize  =  5; break;
                     case TXa_6: dat->markSize  =  6; break;
                     case TXa_7: dat->markSize  =  7; break;
                     case TXa_8: dat->markSize  =  8; break;
                     case TXa_9: dat->markSize  =  9; break;
                     case TXa_0: dat->markSize  = 10; break;
                  }
                  if ((offset + dat->markSize) > dat->curr->size)
                  {
                     dat->markSize = dat->curr->size - offset;
                  }
                  upd = TRUE;
                  break;

               case TXa_C:                      // copy mark data to cursor
               case TXa_M:                      // copy mark data, move mark
               case TXa_O:                      // EOS2 compatible overlay :-)
                  if (dat->markSize)            // we have a mark, could be
                  {                             // in another object (sector)
                     value = min( dat->markSize, (dat->curr->size - offset));
                     memmove( dat->curr->data + offset,
                              dat->curr->data + dat->markStart, value);
                     if (mp2 == TXa_M)
                     {
                        dat->markBase  = dat->curr->start;
                        dat->markStart = offset;
                        dat->markSize  = value;
                     }
                     upd = TRUE;
                  }
                  break;

               case TXa_R:                      // reverse bytes in mark area
                  if ((dat->markSize) && (dat->markBase == dat->curr->start))
                  {
                     BYTE  *lo = dat->curr->data + dat->markStart;
                     BYTE  *hi = lo + dat->markSize -1;

                     while (lo < hi)
                     {
                        tmp   = *lo;
                        *lo++ = *hi;
                        *hi-- = tmp;
                     }
                     upd = TRUE;
                  }
                  break;

               case TXa_S:                      // mark from start of object
                  dat->markBase  = dat->curr->start;
                  dat->markStart = 0;
                  dat->markSize  = offset +1;
                  upd = TRUE;
                  break;

               case TXa_U:                      // unmark
                  dat->markStart = 0;
                  dat->markSize  = 0;
                  upd = TRUE;
                  break;

               case TXk_F8:                     // toggle hex/ascii area
               case TXk_TAB:                    // TAB may be unavailable when
                  dat->ascii   = !dat->ascii;   // multiple windows in dialog
                  upd = TRUE;
                  break;

               case TXk_F9:                     // toggle decimal/hex position
                  dat->decimal = !dat->decimal;
                  txwHexEditCallBack( TXHE_CB_UPD_DESC, dat);
                  upd = TRUE;
                  break;

               case TXc_HOME:
                  if (modified && dat->prev && dat->prev->data)
                  {
                     wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                  }
                  if (wr == NO_ERROR)
                  {
                     if (dat->prev && dat->prev->data)
                     {
                        txwHexEditCallBack( TXHE_CB_TO_START, dat);
                        txtHexEditPrepareBuf( dat);
                     }
                     dat->posCursor = 0;
                     dat->posCurBuf = 0;
                     upd = TRUE;
                  }
                  break;

               case TXc_END:
                  if (modified && dat->next && dat->next->data)
                  {
                     wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                  }
                  if (wr == NO_ERROR)
                  {
                     if (dat->next && dat->next->data)
                     {
                        txwHexEditCallBack( TXHE_CB_TO_FINAL, dat);
                        txtHexEditPrepareBuf( dat);
                     }
                     if (dat->curr->size <= wbytes)
                     {
                        dat->posCurBuf = 0;
                        dat->posCursor = dat->curr->size -1;
                     }
                     else
                     {
                        dat->posCurBuf = dat->curr->size - wbytes;
                        dat->posCursor = wbytes -1;
                     }
                     upd = TRUE;
                  }
                  break;

               case TXa_F2:                     // switch to alt display fmt
               case TXk_F2:                     // switch to alt display fmt
               case TXc_A:                      // find again
               case TXc_F:                      // find data, cursor/buffer
               case TXc_T:
               case TXc_G:                      // goto specified location
                  if (modified)                 // pos to be set by callback!
                  {
                     wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                  }
                  if (wr == NO_ERROR)
                  {
                     switch (mp2)
                     {
                        case TXc_T:             // find, collapse edit window
                           txwPostMsg( owner,  TXWM_CHAR, mp1, TXk_F12);
                           txwHexEditCallBack( TXHE_CB_FINDDATA, dat);
                           txwPostMsg( owner,  TXWM_CHAR, mp1, TXk_F12);
                           break;

                        case TXc_A:
                        case TXc_F:             // find, keep window up
                           txwHexEditCallBack( (mp2 == TXc_F) ?
                                               TXHE_CB_FINDDATA :
                                               TXHE_CB_FNDAGAIN, dat);
                           break;

                        case TXc_G:
                           txwHexEditCallBack( TXHE_CB_GOTOITEM, dat);
                           break;

                        default:
                           dat->altStart  =  ( mp2 == TXa_F2 );
                           txwHexEditCallBack( TXHE_CB_ALT_DISP, dat);
                           break;
                     }
                     txtHexEditPrepareBuf( dat);
                     upd = TRUE;
                  }
                  break;

               case TXc_B:                      // back to begin ...
               case TXa_HOME:                   // INITIAL sector/pos
                  if (modified)
                  {
                     wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                  }
                  wr = txwHexEditCallBack( TXHE_CB_INIT_POS, dat);
                  if (wr == NO_ERROR)
                  {
                     txtHexEditPrepareBuf(dat); // reset modify status & diff
                     upd = TRUE;
                  }
                  break;

               case TXc_E:                      // Erase/Fill current item
               case TXa_F:
                  if (dat->curr->data)
                  {
                     BOOL marked = ((dat->markSize) && (dat->markBase == dat->curr->start));

                     strcpy( str, "00");
                     if (TxPrompt( 0, 5, str, "Specify hexadecimal value to erase "
                                  "the %s with", (marked) ? "marked area" : "entire item"))
                     {
                        BYTE erase = 0;

                        sscanf( str, "%hx", (USHORT *) &erase);
                        memset( dat->curr->data + dat->markStart, erase,
                               (marked) ?  dat->markSize : dat->curr->size);
                        upd = TRUE;
                     }
                  }
                  break;

               case TXc_W:                      // unconditional write back
                  wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                  if (wr == NO_ERROR)
                  {
                     txtHexEditPrepareBuf(dat); // reset modify status & diff
                     upd = TRUE;
                  }
                  break;

               case TXc_Z:                      // undo changes to CURR
                  if (dat->diff && dat->curr->data)
                  {
                     memcpy( dat->curr->data, dat->diff, dat->curr->size);
                     upd = TRUE;
                  }
                  break;

               case TXk_HOME:
                  dat->posCursor = 0;
                  dat->posCurBuf = 0;
                  upd = TRUE;
                  break;

               case TXk_END:
                  if (dat->curr->size <= wbytes)
                  {
                     dat->posCurBuf = 0;
                     dat->posCursor = dat->curr->size -1;
                  }
                  else
                  {
                     dat->posCurBuf = dat->curr->size - wbytes;
                     dat->posCursor = wbytes -1;
                  }
                  upd = TRUE;
                  break;

               case TXa_LEFT:
                  if (dat->cols > 1)
                  {
                     dat->cols--;
                     dat->autocols = FALSE;     // no change on window resize
                     if (dat->posCursor >= dat->rows * dat->cols)
                     {
                        dat->posCurBuf += dat->rows; // move view UP
                        dat->posCursor -= dat->rows; // to keep cursor visible
                     }
                     upd = TRUE;
                  }
                  break;

               case TXa_RIGHT:
                  if (dat->cols < 133)
                  {
                     dat->cols++;
                     dat->autocols = FALSE;     // no change on window resize
                     upd = TRUE;
                  }
                  break;

               case TXa_DOWN:
                  dat->autocols = TRUE;         // back to automatic width
                  upd = TRUE;
                  break;

               case TXc_UP:
                  if (dat->posCursor >= dat->cols) // keep cursor inside data area
                  {
                     dat->posCurBuf += dat->cols;
                     dat->posCursor -= dat->cols;
                     upd = TRUE;
                  }
                  break;

               case TXc_DOWN:
                  if (dat->posCursor < wbytes - dat->cols)
                  {
                     dat->posCurBuf -= dat->cols;
                     dat->posCursor += dat->cols;
                     upd = TRUE;
                  }
                  break;

               case TXc_LEFT:
                  if (dat->posCursor > 0)       // keep cursor inside data area
                  {
                     dat->posCurBuf += 1;
                     dat->posCursor -= 1;
                     upd = TRUE;
                  }
                  break;

               case TXc_RIGHT:
                  if (dat->posCursor < wbytes -1)
                  {
                     dat->posCurBuf -= 1;
                     dat->posCursor += 1;
                     upd = TRUE;
                  }
                  break;

               case TXk_UP:
               case TXk_PGUP:
               case TXa_PGUP:
                  switch (mp2)
                  {
                     case TXk_UP:   value = dat->cols;       break;
                     case TXk_PGUP: if (wbytes < (dat->curr->size + dat->prev->size))
                                    {
                                       value = wbytes;       break;
                                    }
                     default:       value = dat->curr->size; break;
                  }
                  if (offset >= value)           // in same buffer ?
                  {
                     if (dat->posCursor >= value)
                     {
                        txwHexEditShowCursor( hwnd, FALSE, modified, dat);
                        dat->posCursor -= value;
                        txwHexEditShowCursor( hwnd, TRUE,  modified, dat);
                     }
                     else                       // scroll buffer position
                     {
                        dat->posCurBuf -= value;
                        upd = TRUE;
                     }
                  }
                  else if (dat->prev && dat->prev->data) // move to PREV or PRE2
                  {
                     if (modified)
                     {
                        wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                     }
                     if (wr == NO_ERROR)
                     {
                        if ((offset + dat->prev->size) >= value)
                        {
                           dat->posCurBuf += dat->prev->size;
                           txwHexEditCallBack( TXHE_CB_PREV_BUF, dat);
                        }
                        else
                        {
                           if (dat->pre2 && dat->pre2->data) // move to PRE2
                           {
                              dat->posCurBuf += dat->prev->size;
                              dat->posCurBuf += dat->pre2->size;
                              txwHexEditCallBack( TXHE_CB_PRE2_BUF, dat);
                           }
                           else
                           {
                              txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_HOME);
                              break;
                           }
                        }
                        txtHexEditPrepareBuf( dat);

                        if (dat->posCursor >= value)
                        {
                           dat->posCursor -= value;
                        }
                        else                    // scroll buffer position
                        {
                           dat->posCurBuf -= value;
                        }
                        upd = TRUE;
                     }
                  }
                  else                          // no PREV, goto start buffer
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_HOME);
                  }
                  break;

               case TXk_DOWN:
               case TXk_PGDN:
               case TXa_PGDN:
                  switch (mp2)
                  {
                     case TXk_DOWN: value = dat->cols;       break;
                     case TXk_PGDN: if (wbytes < (dat->curr->size + dat->prev->size))
                                    {
                                       value = wbytes;       break;
                                    }
                     default:       value = dat->curr->size; break;
                  }
                  if ((offset + value) < dat->curr->size) // in same buffer ?
                  {
                     if (dat->posCursor < wbytes - value)
                     {
                        txwHexEditShowCursor( hwnd, FALSE, modified, dat);
                        dat->posCursor += value;
                        txwHexEditShowCursor( hwnd, TRUE,  modified, dat);
                     }
                     else                       // scroll buffer position
                     {
                        dat->posCurBuf += value;
                        upd = TRUE;
                     }
                  }
                  else if (dat->next && dat->next->data) // move to NEXT or NEX2
                  {
                     if (modified)
                     {
                        wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                     }
                     if (wr == NO_ERROR)
                     {
                        if ((offset + value) < (dat->curr->size + dat->curr->size))
                        {
                           dat->posCurBuf -= dat->curr->size;
                           txwHexEditCallBack( TXHE_CB_NEXT_BUF, dat);
                        }
                        else
                        {
                           if (dat->nex2 && dat->nex2->data) // move to NEX2
                           {
                              dat->posCurBuf -= dat->curr->size;
                              dat->posCurBuf -= dat->next->size;
                              txwHexEditCallBack( TXHE_CB_NEX2_BUF, dat);
                           }
                           else
                           {
                              txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_END);
                              break;
                           }
                        }
                        txtHexEditPrepareBuf( dat);

                        if (dat->posCursor < wbytes - value)
                        {
                           dat->posCursor += value;
                        }
                        else                    // scroll buffer position
                        {
                           dat->posCurBuf += value;
                        }
                        upd = TRUE;
                     }
                  }
                  else                          // no NEXT, goto end buffer
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_END);
                  }
                  break;

               case TXk_BACKSPACE:
                  if (dat->curr->data)
                  {
                     if (dat->ascii)
                     {
                        dat->curr->data[ offset] = ' ';
                     }
                     else
                     {
                        dat->curr->data[ offset] = 0;
                        dat->hexLsb = 0;        // move to leftmost nibble
                     }
                     upd = TRUE;
                  }
               case TXk_LEFT:
                  if (!dat->ascii && dat->hexLsb)
                  {
                     dat->hexLsb = 0;           // move to leftmost nibble
                     txwHexEditShowCursor( hwnd, TRUE,  modified, dat);
                  }
                  else                          // move to previous byte
                  {
                     dat->hexLsb = 1;           // move to rightmost nibble
                     if (offset > 0)            // not on first buffer byte
                     {
                        if (dat->posCursor >= 1)
                        {
                           txwHexEditShowCursor( hwnd, FALSE, modified, dat);
                           dat->posCursor -= 1;
                           txwHexEditShowCursor( hwnd, TRUE,  modified, dat);
                        }
                        else                    // scroll buffer position
                        {
                           dat->posCurBuf -= dat->cols;
                           dat->posCursor += dat->cols -1;
                           upd = TRUE;
                        }
                     }
                     else if (dat->prev && dat->prev->data) // move to PREV buffer
                     {
                        if (modified)
                        {
                           wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                        }
                        if (wr == NO_ERROR)
                        {
                           dat->posCurBuf += dat->prev->size;
                           txwHexEditCallBack( TXHE_CB_PREV_BUF, dat);
                           txtHexEditPrepareBuf( dat);

                           if (dat->posCursor > 0)
                           {
                              dat->posCursor -= 1;
                           }
                           else                 // scroll buffer position
                           {
                              dat->posCurBuf -= dat->cols;
                              dat->posCursor += dat->cols -1;
                           }
                           upd = TRUE;
                        }
                     }
                  }
                  break;

               default:
                  if ((mp2 > TXk_ESCAPE) &&     // possible ascii key
                      (mp2 < TXW_KEY_GROUP_1))
                  {
                     if (dat->curr->data)
                     {
                        if (dat->ascii)
                        {
                           dat->curr->data[ offset] = mp2 & 0xff;
                           upd = TRUE;
                        }
                        else if (isxdigit( mp2))
                        {
                           BYTE  hex = toupper( mp2) - '0';

                           if (hex > 9)
                           {
                              hex -= 7;
                           }
                           if (dat->hexLsb)
                           {
                              dat->curr->data[ offset] &= 0xf0;
                              dat->curr->data[ offset] |= hex;
                           }
                           else
                           {
                              dat->curr->data[ offset] &= 0x0f;
                              dat->curr->data[ offset] |= (hex << 4);
                           }
                           upd = TRUE;
                        }
                        else if (mp2 != TXk_RIGHT)
                        {
                           dat->hexLsb = 0;     // stay on same byte for
                        }                       // non-hex keys on HEX
                     }
                  }
                  else if (mp2 != TXk_RIGHT)
                  {
                     rc = txwDefWindowProc( hwnd, msg, mp1, mp2);
                     break;
                  }
                  if (!dat->ascii && !dat->hexLsb)
                  {
                     if (mp2 != TXk_SPACE)
                     {
                        dat->hexLsb = 1;        // move to rightmost nibble
                        txwHexEditShowCursor( hwnd, TRUE,  modified, dat);
                     }
                  }
                  else                          // move to next byte
                  {
                     dat->hexLsb = 0;           // move to leftmost nibble
                     if (offset < (dat->curr->size - 1)) // not on last buffer byte
                     {
                        if (dat->posCursor < wbytes -1)
                        {
                           txwHexEditShowCursor( hwnd, FALSE, modified, dat);
                           dat->posCursor += 1;
                           txwHexEditShowCursor( hwnd, TRUE,  modified, dat);
                        }
                        else                    // scroll buffer position
                        {
                           dat->posCurBuf += dat->cols;
                           dat->posCursor -= dat->cols -1;
                           upd = TRUE;
                        }
                     }
                     else if (dat->next && dat->next->data) // move to NEXT buffer
                     {
                        if (modified)
                        {
                           wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
                        }
                        if (wr == NO_ERROR)
                        {
                           dat->posCurBuf -= dat->curr->size;
                           txwHexEditCallBack( TXHE_CB_NEXT_BUF, dat);
                           txtHexEditPrepareBuf( dat);

                           if (dat->posCursor < wbytes -1)
                           {
                              dat->posCursor += 1;
                           }
                           else                 // scroll buffer position
                           {
                              dat->posCurBuf += dat->cols;
                              dat->posCursor -= dat->cols -1;
                           }
                           upd = TRUE;
                        }
                     }
                  }
                  break;
            }
            break;

#if defined (HAVEMOUSE)

         case TXWM_BUTTONDOWN:
            if (txwMouseButtonDown( hwnd, mp1, mp2) == TX_PENDING)
            {
               short  col = TXMOUSECOL()  - win->client.left;  //- relative to
               short  row = TXMOUSEROW()  - win->client.top;   //- the window!
               short  ac  = TXHE_AC_COLUMN( dat->cols);

               TRACES(( "BDOWN - row:%hd col:%hd  ac:%hd\n", row, col, ac));
               if ((row == 0) || (row > dat->rows)) // not data area, buttons
               {
                  if (col < 6)
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, mp1, (row) ? TXk_PGDN : TXk_PGUP);
                  }
                  else if ((col >= ac) && (col < (ac + 8)))
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, mp1, (row) ? TXa_PGDN : TXa_PGUP);
                  }
                  else                          // rest, line up/down
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, mp1, (row) ? TXk_DOWN : TXk_UP);
                  }
               }
               else if ((col < 8) || (col > (ac + dat->cols)))
               {
                  txwPostMsg( hwnd, TXWM_CHAR, mp1, (col < 6) ? TXk_LEFT : TXk_RIGHT);
               }
               else                             // in data area, place cursor
               {
                  txwHexEditShowCursor( hwnd, FALSE, modified, dat);
                  dat->posCursor = (row -1) * dat->cols;
                  if (col > ac)
                  {
                     dat->posCursor += col - ac -1;
                     dat->ascii = TRUE;
                  }
                  else                          // in HEX byte area
                  {
                     dat->posCursor += (col - 8) / 3;
                     dat->hexLsb = ((col -8) % 3) ? 1 : 0;
                     dat->ascii = FALSE;
                  }
                  txwHexEditShowCursor( hwnd, TRUE,  modified, dat);
               }
            }
            break;
#endif

         case TXWM_SIZE:                        // window has been resized
            dat->rows = sy - 3;                 // adapt to resizing
            if (dat->rows && (dat->posCursor >= dat->rows * dat->cols))
            {
               dat->posCurBuf += dat->cols;     // move view UP one line
               dat->posCursor -= dat->cols;     // to keep cursor visible
            }                                   // (paint to be done  by SIZE)
            break;

         case TXWM_DESTROY:                     // window will be destroyed
            if (dat->diff      &&               // determine REAL modify status
                dat->diffSize  &&               // determine REAL modify status
                dat->curr->data )               // for exact item size now.
            {                                   // currCrc wrong after delete!
               int     i;                       // (will display 'modified' :-)

               for (modified = FALSE, i = 0; i < dat->curr->size; i++)
               {
                  if (dat->diff[ i] != dat->curr->data[ i])
                  {
                     modified = TRUE;
                     break;
                  }
               }
            }
            if (modified)
            {
               wr = txwHexEditCallBack( TXHE_CB_WRITEBUF, dat);
            }
            TxFreeMem( win->he.diff);           // free difference buffer
            dat->diffSize = 0;
            break;

         default:
            rc = txwDefWindowProc( hwnd, msg, mp1, mp2);
            break;
      }
      if (upd)
      {
         rc = txwInvalidateWindow( hwnd, TRUE, TRUE);
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwHexEditWinProc'
/*---------------------------------------------------------------------------*/

