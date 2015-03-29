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
// TX Windowed text, redirect TxPrint to scrollbuffer, handle buffer view etc
//
// Author: J. van Wijk
//
// JvW  18-05-1999 Initial version
// JvW  31-05-1999 Fixed width * width bug in memory-alloc (width * length)
// JvW  31-01-2000 Added follow_screen_toggle to allow screen on/off commands
// JvW  24-05-2014 Support for a 'marked-area' in one line, in txwPaintSbView
// JvW  25-05-2014 Moved iKeySbView from txwikey.c and renamed to txwsbuf.c

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface


#define TXWP_ARG_SIZE   4
#define TXWP_NUM_ARGS  10
#define TXWP_ANSI_ESC  27

#define TXWP_MAX    0xffff                      // maximum rectangle value


static BYTE      txwpFgColor = TXwCNW;          // default foreground color
static BYTE      txwpBgColor = TXwCnZ;          // default background color

static ULONG     txwpVirtCol = 0;               // column on virtual screen
static ULONG     txwpVirtRow = 0;               // row    on virtual screen

static TXSBDATA *txwpScrBuff = NULL;            // scroll buffer
static char     *txwpLine    = NULL;            // intermediate line buffer
static TXH_INFO  txwpSla;                       // TxPrint copy specification

static TXCELL   *txwpCline   = NULL;            // Single line cell buffer

static char      txwpOriginalAnsiMode;          // remembered Ansi status

static char        *ascLine = NULL;             // ASCII search line, original
static char        *lowLine = NULL;             // ASCII search line, lowercased

static TXSELIST       *sbg_list  = NULL;
static ULONG           sbg_index = 0;           // current index in grep list

// maximum number of search results in the grep list
#define TXWS_GREP_LIMIT        1024

// Convert and write ANSI-input string to a windowed scroll-buffer
static void txwpHookScrollBuffer
(
   char               *text,                    // IN    TxPrint text to copy
   void               *data                     // IN    user data (not used)
);

// Parse ANSI clause
static char *txwpParseAnsi
(
   char               *current                  // IN    this character
);

// Parse ANSI clause without arguments
static void txwpParseAnsiClause
(
   char               *current,                 // IN    this character
   BOOL               *syntax                   // OUT   ANSI syntax error
);

// Parse ANSI clause including arguments
static char *txwpParseAnsiArgs
(
   char               *current,                 // IN    this character
   BOOL               *syntax                   // OUT   ANSI syntax error
);

// Clear virtual screen, clearing scroll-buffer and reset to default position
static void txwpClearScreen
(
   void
);

// Goto specified x/y position in virtual screen
static void txwpGotoXY
(
   ULONG               x,                       // IN    x coordinate (column)
   ULONG               y                        // IN    y coordinate (row)
);

// Clear to end-of-line, on current line in scroll-buffer
static void txwpClearToEOL
(
   void
);

// Write one character to the Scroll-buffer, handle newline (scroll) and wrap
static void txwpWriteCharacter
(
   char                ch                       // IN    ASCII character value
);

// Scroll the scroll-buffer up on line and adjust positions
static void txwpScrollUpOneLine
(
   void
);

// Automatic update of associated views for the scroll-buffer
static void txwpAutoUpdateView
(
   void
);

// Build list of search results in the scrollbuffer
static TXSELIST *txwSbViewGrepSelist            // RET   selection list or NULL
(
   void
);

// Search SBUF text for given string and specified search-options
// To search from start/end, specify a posLine that is OUTSIDE the text
static char *txwSbSearch                        // RET   ptr to ASCII hit line
(
   char               *string,                  // IN    String to search for
   BOOL                caseSensitive,           // IN    Case sensitive search
   BOOL                wholeWord,               // IN    Space delimited word only
   BOOL                backwards,               // IN    Search backwards
   ULONG              *posLine,                 // INOUT position, Line number
   short              *posColumn                // OUT   position, Column
);

// Copy a single line from the scrollbuffer into a buffer, as ASCII C-string
static char *txwSbLineCopy                      // RET   ptr to ASCII line
(
   char               *ascSstring,              // OUT   ASCII string representation
   ULONG               LineNr                   // IN    position, Line number
);


/*========================== SBUF FIND ==========================================================*/

static BOOL sbCheckCase  = FALSE;               // Check Case-sensitive
static BOOL sbCheckWord  = FALSE;               // Check Whole-word
static BOOL sbCheckBack  = TRUE;                // Check Search backward
static BOOL sbCheckList  = FALSE;               // Check search result list

static TXTM  sbEntrySearchArgument = "";        // search argument

/*
[ ] Case-sensitive searching
[ ] Match on whole-word only   [ ] Reverse searching, backward

[ ] Show search results in list, toggle between them (aka GREP)
*/
#define   TXWSBUFFINDWIDGETS  4

#define   TXWSBUF_ARG_WIDTH  60

static TXWIDGET  txwSbViewFindWidgets[TXWSBUFFINDWIDGETS] =  // order determines TAB-order!
{
   {0, 1, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &sbCheckCase, "Case-sensitive searching")},
   {1, 1, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &sbCheckWord, "Match on whole-word only")},
   {1,35, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &sbCheckBack, "Reverse searching, backward")},
   {3, 1, 1, 65, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &sbCheckList, "Show search results in list, toggle between them (aka GREP)")},
};

static TXGW_DATA txwSbViewFindDlg =
{
   TXWSBUFFINDWIDGETS,                          // number of widgets
   TXWS_SEARCHDIALOG,                           // dialog help
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   txwSbViewFindWidgets                         // array of widgets
};


/*************************************************************************************************/
// Present SBUF-section FIND options dialog making the info available
/*************************************************************************************************/
static ULONG txwSbViewFindDialog
(
   void
)
{
   ULONG               rc = TX_ABORTED;         // function return

   ENTER();

   while (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &txwSbViewFindDlg,
         "Search in the text output window (scrollbuffer) for:",
         " Specify OUTPUT search string and options ", TXWS_SEARCHDIALOG,
         TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER, TXWSBUF_ARG_WIDTH, sbEntrySearchArgument)
      != TXDID_CANCEL)
   {
      if (strlen( sbEntrySearchArgument))
      {
         rc = NO_ERROR;                         // search arguments available
         break;                                 // out of while ...
      }
      else
      {
         TxMessage( TRUE, TXWS_SEARCHDIALOG, "You must specify a search argument ...");
      }
   }
   RETURN (rc);
}                                               // end 'txwSbViewFindDialog'
/*-----------------------------------------------------------------------------------------------*/


/*****************************************************************************/
// Initiate a FIND request for the scrollbuffer, including GREP and dialogs
/*****************************************************************************/
ULONG txwSbViewFindRequest
(
   ULONG               request,                 // IN    specific request code
   char               *param                    // IN    string parameter
)
{
   ULONG               rc = NO_ERROR;           // function return

   if (txwpScrBuff != NULL)
   {
      txwpScrBuff->markSize = 0;                // unmark
      if (param != NULL)
      {
         strcpy( sbEntrySearchArgument, param);
      }
      switch (request)
      {
         case TXWS_REQ_FINDSTRING:
            sbCheckBack = TRUE;
            txwPostMsg( (TXWHANDLE) txwa->sbview, TXWM_PERFORM_SEARCH, 0, 0);
            break;

         case TXWS_REQ_FINDDIALOG:
            sbCheckList = FALSE;
            sbCheckBack = TRUE;
            txwPostMsg( (TXWHANDLE) txwa->sbview, TXWM_CHAR, 0, TXk_F7);
            break;

         case TXWS_REQ_GREPDIALOG:
            sbCheckList = TRUE;
            sbCheckBack = TRUE;
            txwPostMsg( (TXWHANDLE) txwa->sbview, TXWM_CHAR, 0, TXk_F7);
            break;

         case TXWS_REQ_GREPSTRING:
            txwPostMsg( (TXWHANDLE) txwa->sbview, TXWM_PERFORM_GREP, 0, 0);
            break;

         case TXWH_REQ_NONE:
         default:
            break;
      }
   }
   RETURN (rc);
}                                               // end 'txwSbViewFindRequest'
/*-----------------------------------------------------------------------------------------------*/


/*****************************************************************************/
// Initialize TxPrint hook to a scroll-buffer and related views
/*****************************************************************************/
ULONG txwInitPrintfSBHook
(
   TXSBDATA           *sbuf                     // IN    Scroll-buffer data
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if ((txwpScrBuff == NULL) && (sbuf->buf == NULL))
   {
      if ((sbuf->buf = TxAlloc(sbuf->width * sizeof(TXCELL), sbuf->length)) != NULL)
      {
         if ((txwpCline = TxAlloc(sbuf->width * sizeof(TXCELL), 1)) != NULL)
         {
            if ((txwpLine = TxAlloc(1,  TXMAX4K)) != NULL)
            {
               sbuf->firstline = sbuf->length - sbuf->vsize;
               txwpScrBuff     = sbuf;

               sbuf->markRel   = 0;             // Line rel to firstline
               sbuf->markCol   = 0;             // mark column
               sbuf->markSize  = 0;             // mark length

               txwpClearScreen();               // clear the scroll-buffer

               txwpSla.user    = NULL;
               txwpSla.cbuf    = txwpLine;
               txwpSla.size    = TXMAX4K;
               txwpSla.copy    = txwpHookScrollBuffer;
               txwpSla.active  = TRUE;
               txwpSla.follow_screen_toggle = TRUE;
               TxPrintHook( TXH_T_RAW, TXH_REGISTER, &txwpSla);

               txwpOriginalAnsiMode = TxGetAnsiMode();
               if (TxaExeSwitch('a') == FALSE)
               {
                  TxSetAnsiMode( A_ON);         // allow color from now on
               }
            }
            else
            {
               TxFreeMem( txwpCline);
               TxFreeMem( sbuf->buf);
               rc = TX_ALLOC_ERROR;
            }
         }
         else
         {
            TxFreeMem( sbuf->buf);
            rc = TX_ALLOC_ERROR;
         }
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   else
   {
      rc = TX_PENDING;                          // buffer already in use
   }
   RETURN (rc);
}                                               // end 'txwInitPrintfSBHook'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate TxPrint hook to a scroll-buffer and related views
/*****************************************************************************/
void txwTermPrintfSBHook
(
   void
)
{
   ENTER();

   TxSetAnsiMode( txwpOriginalAnsiMode);        // restore ansi mode

   TxPrintHook( TXH_T_RAW, TXH_DEREGISTER, &txwpSla);

   TxFreeMem( ascLine);                         // free related buffers
   TxFreeMem( lowLine);
   TxFreeMem( txwpLine);
   TxFreeMem( txwpCline);
   TxFreeMem( txwpScrBuff->buf);
   txwpScrBuff = NULL;                          // clear aliased pointer too!

   VRETURN();
}                                               // end 'txwTermPrintfSBHook'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert and write ANSI-input string to a windowed scroll-buffer
/*****************************************************************************/
static void txwpHookScrollBuffer
(
   char               *text,                    // IN    TxPrint text to copy
   void               *data                     // IN    user data (not used)
)
{
   char               *current = text;

   while (*current)
   {
      if (*current == TXWP_ANSI_ESC)
      {
         current = txwpParseAnsi(current);
      }
      else
      {
         txwpWriteCharacter(*current);
      }
      current++;
   }
   txwpAutoUpdateView();
}                                               // end 'txwpHookScrollBuffer'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse ANSI clause
/*****************************************************************************/
static char *txwpParseAnsi
(
   char               *current                  // IN    this character
)
{
   BOOL                error = FALSE;           // syntax error indicator
   char               *keep  = current;

   current++;
   if (*current != '[')
   {
      error = TRUE;
   }
   else
   {
      current++;
      if (!isdigit(*current))
      {
         txwpParseAnsiClause( current, &error);
      }
      else
      {
         current = txwpParseAnsiArgs(current, &error);
      }
   }
   if (error)
   {
      txwpWriteCharacter(*keep);
      return keep;
   }
   else
   {
      return current;
   }
}                                               // end 'txwpParseAnsi'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse ANSI clause without arguments
/*****************************************************************************/
static void txwpParseAnsiClause
(
   char               *current,                 // IN    this character
   BOOL               *syntax                   // OUT   ANSI syntax error
)
{
   static ULONG x = 0;
   static ULONG y = 0;

   switch (*current)
   {
      case 'A': txwpGotoXY( txwpVirtCol    , txwpVirtRow + 1);     break;
      case 'B': txwpGotoXY( txwpVirtCol    , txwpVirtRow - 1);     break;
      case 'C': txwpGotoXY( txwpVirtCol + 1, txwpVirtRow    );     break;
      case 'D': txwpGotoXY( txwpVirtCol - 1, txwpVirtRow    );     break;
      case 'H': txwpGotoXY( 0              , 0              );     break;
      case 'u': txwpGotoXY( x              , y              );     break;
      case 's':             x = txwpVirtCol; y  = txwpVirtRow;     break;
      case 'K': txwpClearToEOL();                                  break;
      default : *syntax = TRUE;
   }
}                                               // end 'txwpParseAnsiClause'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse ANSI clause including arguments
/*****************************************************************************/
static char *txwpParseAnsiArgs
(
   char               *current,                 // IN    this character
   BOOL               *syntax                   // OUT   ANSI syntax error
)
{
   int                 i;
   int                 argument[TXWP_NUM_ARGS];
   int                 count    = 0;
   BYTE                intensFg = 0;
   BYTE                intensBg = 0;

   if ((*current == '2') && (*(current + 1) == 'J'))
   {
      ++current;
      txwpClearScreen();
   }
   else
   {
      while (isdigit(*current) && (count <= TXWP_NUM_ARGS))
      {

         int           index = 0;
         char          arg[TXWP_ARG_SIZE];

         while (isdigit(*current) && (index < TXWP_ARG_SIZE))
         {
            arg[index++] = *current++;
         }

         if (index == TXWP_ARG_SIZE)
         {
            *syntax = TRUE;
         }
         else
         {
            arg[index] = 0;
         }

         argument[count++] = atoi(arg);

         if (*current == ';')
         {
            ++current;
         }
      }

      if (count == TXWP_NUM_ARGS)
      {
         *syntax = TRUE;
      }
      else
      {
         switch (*current)
         {
            case 'A':
               if (count == 1)
               {
                  txwpGotoXY( txwpVirtCol, txwpVirtRow - (ULONG) argument[0]);
               }
               else
               {
                  *syntax = TRUE;
               }
               break;

            case 'B':
               if (count == 1)
               {
                  txwpGotoXY( txwpVirtCol, txwpVirtRow + (ULONG) argument[0]);
               }
               else
               {
                  *syntax = TRUE;
               }
               break;

            case 'C':
               if (count == 1)
               {
                  txwpGotoXY( txwpVirtCol + (ULONG) argument[0], txwpVirtRow);
               }
               else
               {
                  *syntax = TRUE;
               }
               break;

            case 'D':
               if (count == 1)
               {
                  txwpGotoXY( txwpVirtCol - (ULONG) argument[0], txwpVirtRow);
               }
               else
               {
                  *syntax = TRUE;
               }
               break;

            case 'H':
               if (count == 2)
               {
                  txwpGotoXY( argument[1], argument[0]);
               }
               else
               {
                  *syntax = TRUE;
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
                           txwpFgColor = TXwCNW; // default foreground color
                           txwpBgColor = TXwCnZ; // default background color
                        }
                        else
                        {
                           intensFg = 0;
                           intensBg = 0;
                        }
                        break;

                     case 1:                    // High intensity (FG)
                        intensFg = TXwINT;
                        break;

                     case 2:                    // DIM intensity
                     case 4:                    // Underlined
                     case 6:                    // Fast-blink
                     case 7:                    // Reverse video
                     case 8:                    // Hidden character
                        break;

                     case 5:                    // Blink (or high-intensity BG)
                        intensBg = (TXwINT << 4);
                        break;

                     case 30 : txwpFgColor = ( TXwCNZ | intensFg ); break;
                     case 31 : txwpFgColor = ( TXwCNR | intensFg ); break;
                     case 32 : txwpFgColor = ( TXwCNG | intensFg ); break;
                     case 33 : txwpFgColor = ( TXwCNY | intensFg ); break;
                     case 34 : txwpFgColor = ( TXwCNB | intensFg ); break;
                     case 35 : txwpFgColor = ( TXwCNM | intensFg ); break;
                     case 36 : txwpFgColor = ( TXwCNC | intensFg ); break;
                     case 37 : txwpFgColor = ( TXwCNW | intensFg ); break;
                     case 40 : txwpBgColor = ( TXwCnZ | intensBg ); break;
                     case 41 : txwpBgColor = ( TXwCnR | intensBg ); break;
                     case 42 : txwpBgColor = ( TXwCnG | intensBg ); break;
                     case 43 : txwpBgColor = ( TXwCnY | intensBg ); break;
                     case 44 : txwpBgColor = ( TXwCnB | intensBg ); break;
                     case 45 : txwpBgColor = ( TXwCnM | intensBg ); break;
                     case 46 : txwpBgColor = ( TXwCnC | intensBg ); break;
                     case 47 : txwpBgColor = ( TXwCnW | intensBg ); break;
                     default : *syntax = TRUE;
                  }
               }
               break;

            default : *syntax = TRUE;
         }
      }
   }
   if (*syntax)
   {
      return NULL;
   }
   else
   {
      return current;
   }
}                                               // end 'txwpParseAnsiArgs'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Clear virtual screen, clearing scroll-buffer and reset to default position
/*****************************************************************************/
static void txwpClearScreen
(
   void
)
{
   size_t              lsize = (size_t) txwpScrBuff->width * sizeof(TXCELL);

   memset(  txwpScrBuff->buf, 0,  ((size_t) txwpScrBuff->length * lsize));
   txwpGotoXY( 0, 0);
}                                               // end 'txwpClearScreen'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Goto specified x/y position in virtual screen
/*****************************************************************************/
static void txwpGotoXY
(
   ULONG               x,                       // IN    x coordinate (column)
   ULONG               y                        // IN    y coordinate (row)
)
{
   txwpVirtCol = x;
   txwpVirtRow = min( y, txwpScrBuff->vsize -1);
}                                               // end 'txwpGotoXY'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Clear to end-of-line, on current line in scroll-buffer
/*****************************************************************************/
static void txwpClearToEOL
(
   void
)
{
   TXCELL              cell;                    // CELL contents to write
   ULONG               line;                    // targetline in scroll-buffer
   ULONG               index;                   // index in scroll-buffer
   ULONG               col;                     // current column in line

   cell.ch = ' ';                               // fill with spaces
   cell.at = txwpFgColor | txwpBgColor;         // in the current color

   line  = txwpScrBuff->length - txwpScrBuff->vsize + txwpVirtRow;
   index = line * txwpScrBuff->width + txwpVirtCol;
   for (col = txwpVirtCol; col < txwpScrBuff->width; col++, index++)
   {
      txwpScrBuff->buf[ index] = cell;
   }
}                                               // end 'txwpClearToEOL'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Write one character to the Scroll-buffer, handle newline (scroll) and wrap
/*****************************************************************************/
static void txwpWriteCharacter
(
   char                ch                       // IN    ASCII character value
)
{
   TXCELL              cell;                    // CELL contents to write
   ULONG               line;                    // targetline in scroll-buffer

   switch (ch)
   {
      case '\n':                                // newline in string
         if (txwpVirtRow == txwpScrBuff->vsize -1) // writing at bottom line
         {
            txwpScrollUpOneLine();
         }
         else                                   // write on previous lines
         {
            txwpGotoXY( 0, txwpVirtRow + 1);
         }
         break;

      default:
         cell.ch = ch;
         cell.at = txwpFgColor | txwpBgColor;
         if (txwpVirtRow == txwpScrBuff->vsize -1) // writing at bottom line
         {
            if ((txwpScrBuff->wrap) && (txwpVirtCol >= txwpScrBuff->width))
            {
               txwpScrollUpOneLine();
            }
         }
         if (txwpVirtCol < txwpScrBuff->width)  // within the line-width ?
         {
            line = txwpScrBuff->length - txwpScrBuff->vsize + txwpVirtRow;
            txwpScrBuff->buf[ line * txwpScrBuff->width + txwpVirtCol] = cell;
         }
         txwpVirtCol++;
         break;
   }
}                                               // end 'txwpWriteCharacter'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Scroll the scroll-buffer up on line and adjust positions
/*****************************************************************************/
static void txwpScrollUpOneLine
(
   void
)
{
   size_t              lsize = (size_t) txwpScrBuff->width * sizeof(TXCELL);
   ULONG               lines = txwpScrBuff->length -1;

   txwpVirtCol         = 0;                     // column in virtual screen

   memmove( &(txwpScrBuff->buf[0]),                  //- dest, begin buf
            &(txwpScrBuff->buf[txwpScrBuff->width]), //- source, 1 line ahead
            lsize * ((size_t) lines));               //- bufsize minus 1 line

   memset(  &(txwpScrBuff->buf[lines * txwpScrBuff->width]),
            0, lsize);                          // clear the bottom line

   if (txwpScrBuff->firstline != 0)
   {                                            // count back, until at top
      txwpScrBuff->firstline--;                 // of the scroll-buffer
   }
   if (txwpScrBuff->midscroll == FALSE)         // adjust scroll position
   {
      TXWINBASE   *wnd = (TXWINBASE *) txwpScrBuff->view;

      if ((wnd = (TXWINBASE *) txwpScrBuff->view) != NULL)
      {
         //- if the view-window is at the last line, leave it there,
         //- if NOT, then scroll the view upwards with the text to
         //- make sure the shown contents stays the same!
         if (wnd->window->sb.topline < (txwpScrBuff->length -
                                        txwpScrBuff->vsize))
         {
            if (wnd->window->sb.topline != 0)
            {                                   // count back, until at top
               wnd->window->sb.topline--;       // of the scroll-buffer
            }
         }
      }
   }
}                                               // end 'txwpScrollUpOneLine'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Automatic update of invalid areas of associated views for the scroll-buffer
/*****************************************************************************/
static void txwpAutoUpdateView
(
   void
)
{
   TXWINBASE          *wnd;

   if ((wnd = (TXWINBASE *) txwpScrBuff->view) != NULL)
   {
      txwPaintSbView( wnd);
   }
}                                               // end 'txwpAutoUpdateView'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// ScrollBuff paint window, SBVIEW only; without any internal tracing!!!
/*****************************************************************************/
void txwPaintSbView
(
   TXWINBASE          *wnd                      // IN    current window
)
{
   TXWINDOW           *win;
   short               sy;                      // vertical size
   short               nr;                      // line-nr while painting
   short               size;                    // size of one line
   ULONG               first;                   // first cell (top-left)
   TXCELL             *cline;                   // cell line pointer
   TXRECT              parent;                  // parent clip rectangle
   TXRECT              clip;                    // region clip rectangle
   TXRECT_ELEM         regs;                    // visible line regions
   TXRECT_ELEM        *r;                       // current region
   TXRECT_ELEM        *n;                       // next region
   TXWINBASE          *focusShadow;             // focus related shadow window
   BOOL                lineHasMarkArea;

   if ((txwValidateAndClip((TXWHANDLE) wnd, &win, FALSE, &parent)) != NULL)
   {
      sy    = win->client.bottom - win->client.top   +1;
      size  = (short) txwpScrBuff->width;
      first = win->sb.topline * size + (ULONG) win->sb.leftcol;

      cline = &(txwpScrBuff->buf[first]);       // top-left cell in SBview

      TXSCREEN_BEGIN_UPDATE();
      for (nr = 0; nr < (sy -1); nr++, cline += size)
      {
         if ((txwpScrBuff->markSize  != 0) &&   // is there a mark on THIS line ?
             ((win->sb.topline + nr) == (txwpScrBuff->firstline + txwpScrBuff->markRel)))
         {
            short   markIndex;                  // mark cell index
            ULONG   markPos;                    // Index of first cell in mark area

            markPos = ((win->sb.topline + nr) * size) + txwpScrBuff->markCol;
            for (markIndex = 0; markIndex < txwpScrBuff->markSize; markIndex++)
            {
               txwpCline[ markIndex].ch = txwpScrBuff->buf[ markPos++].ch;
               txwpCline[ markIndex].at = TxwSC( cSbvMarkedArea);
            }
            lineHasMarkArea = TRUE;
         }
         else
         {
            lineHasMarkArea = FALSE;
         }

         regs.rect = win->client;               // start with full line
         regs.next = NULL;                      // as a single element
         regs.skip = FALSE;                     // it is a valid element

         regs.rect.top   += nr;                 // actual line position
         regs.rect.bottom = regs.rect.top;      // just 1-line vertical

         txwVisibleLineRegions( wnd, &regs);    // calculate visible regions
         for (r = &regs; r; r = r->next)        // Draw each line region
         {
            if (r->skip == FALSE)               // but only when valid
            {
               if (txwIntersectRect( &parent, &(r->rect), &clip))
               {
                  txwScrDrawCellString( win->client.top + nr,
                                        win->client.left,
                                        &clip, cline, size, win->sb.altcol);

                  if (lineHasMarkArea)          // is there a mark ?
                  {
                     //- write the prepared mark line into this region
                     txwScrDrawCellString( win->client.top  + nr,
                                           win->client.left + txwpScrBuff->markCol - win->sb.leftcol,
                                           &clip, txwpCline,  txwpScrBuff->markSize, win->sb.altcol);
                  }
               }
            }
         }

         for (r = regs.next; r; r = n)          // Free extra line regions
         {
            n = r->next;                        // next region in the list
            free( r);
         }
      }
      txwPaintWinStatus( wnd, NULL, 0);         // update SB line counters etc
                                                // without resetting status text
      focusShadow = txwa->focus;
      if ((focusShadow != NULL) && (focusShadow->window != NULL))
      {
         if (focusShadow->window->style & TXWS_CAST_SHADOW)
         {
            txwPaintShadow( focusShadow);       // focus window border refresh
         }
         else                                   // retry on focus owner (dlg)
         {
            focusShadow = focusShadow->owner;
            if ((focusShadow != NULL) && (focusShadow->window != NULL))
            {
               if (focusShadow->window->style & TXWS_CAST_SHADOW)
               {
                  txwPaintShadow( focusShadow);
               }
            }
         }
      }
      TXSCREEN_ENDOF_UPDATE();
   }
}                                               // end 'txwPaintSbView'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Save (part of) the scroll buffer to specified file as plain ASCII
/*****************************************************************************/
ULONG txwSavePrintfSB                           // RET   nr of lines written
(
   char               *fname,                   // IN    name of output file
   ULONG               lines,                   // IN    lines from start/end
   BOOL                fromEnd                  // IN    last lines (from end)
)
{
   FILE               *df;                      // destination file ptr
   ULONG               done = 0;                // nr of lines done

   ENTER();

   if (txwpScrBuff != NULL)
   {
      if (ascLine == NULL)
      {
         ascLine = TxAlloc( txwpScrBuff->width + 2, 1);
      }
      if ((df = fopen( fname, "w" TXFMODE)) != NULL) // create new text file
      {
         ULONG         used;                    // nr of lines in buffer
         ULONG         first;                   // first line to save
         ULONG         todo;                    // nr of lines to save
         ULONG         l;                       // line nr in buffer

         used  = txwpScrBuff->length - txwpScrBuff->firstline;
         if (lines >= used)
         {
            first = txwpScrBuff->firstline;
            todo  = txwpScrBuff->length - first;
         }
         else
         {
            todo = lines;
            if (fromEnd)
            {
               first = txwpScrBuff->length - todo;
            }
            else
            {
               first = txwpScrBuff->firstline;
            }
         }
         for (l = first, done = 0;
                         done < todo;
              l++,       done++)
         {
            txwSbLineCopy( ascLine, l);
            if (fprintf( df, "%s\n", ascLine) < 0)
            {
               TxPrint( "Error writing to '%s', disk might be full\n", fname);
               break;
            }
         }
         fclose( df);
      }
      else
      {
         TxPrint( "Destination file '%s' cannot be created\n", fname);
      }
   }
   else
   {
      TxPrint( "Screen-buffer is not active, nothing to save!\n");
   }
   RETURN (done);
}                                               // end 'txwSavePrintfSB'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Input key handling for the SBVIEW window class
/*****************************************************************************/
ULONG txwSbViewWinProc                             // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win;
   BOOL                upd = FALSE;             // window draw required
   short               size;
   TXSBVIEW           *dat;
   TXLN                status;

   //- note: sy is the vertical size of the scrollable-area, this does NOT
   //-       include the bottom line of the display with the status-info

   ENTER();
   if ((wnd != NULL) && ((win = wnd->window) != NULL))
   {
      short            sx    = win->client.right  - win->client.left +1;
      short            sy    = win->client.bottom - win->client.top;

      dat  = &win->sb;
      size = (short) dat->sbdata->width;
      if (dat->sbdata->length > (ULONG) sy)
      {
         dat->maxtop = dat->sbdata->length - sy;
      }
      else
      {
         dat->maxtop = 0;
      }
      if (size  > sx)
      {
         dat->maxcol = size - sx;
      }
      else
      {
         dat->maxcol = 0;
      }
      TRACES(("top:%4lu, left:%4hu, max:%4lu, lines: %4lu width:%4lu\n",
               dat->topline, dat->leftcol, dat->maxtop,
               dat->sbdata->length, dat->sbdata->width));

      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)                        // key value
            {
               case TXk_UP:
               case TXc_UP:
               case TXa_UP:
                  if (dat->topline != 0)
                  {
                     dat->topline--;
                     upd = TRUE;
                  }
                  break;

               case TXk_DOWN:
               case TXc_DOWN:
               case TXa_DOWN:
                  if (dat->topline < dat->maxtop)
                  {
                     dat->topline++;
                     upd = TRUE;
                  }
                  break;

               case TXk_PGUP:
               case TXc_PGUP:
               case TXa_PGUP:
                  if (dat->topline > (ULONG) sy)
                  {
                     dat->topline -= sy;
                     upd = TRUE;
                  }
                  else if (dat->topline != 0)
                  {
                     dat->topline = 0;
                     upd = TRUE;
                  }
                  break;

               case TXk_PGDN:
               case TXc_PGDN:
               case TXa_PGDN:
                  if (dat->topline + sy <= dat->maxtop)
                  {
                     dat->topline += sy;
                     upd = TRUE;
                  }
                  else if (dat->topline != dat->maxtop)
                  {
                     dat->topline = dat->maxtop;
                     upd = TRUE;
                  }
                  break;

               case TXc_HOME:
               case TXc_MINUS:
                  if (dat->topline != 0)
                  {
                     if (mp2 == TXc_MINUS)
                     {
                        dat->topline = 0;
                     }
                     else
                     {
                        dat->topline = dat->sbdata->firstline;
                     }
                     upd = TRUE;
                  }
                  break;

               case TXc_END:
                  if (dat->topline != dat->maxtop)
                  {
                     dat->topline = dat->maxtop;
                     upd = TRUE;
                  }
                  break;

               case ',':
               case TXa_COMMA:
               case TXk_LEFT:
               case TXc_LEFT:
               case TXa_LEFT:
                  if (dat->leftcol != 0)
                  {
                     dat->leftcol--;
                     upd = TRUE;
                  }
                  break;

               case '.':
               case TXa_DOT:
               case TXk_RIGHT:
               case TXc_RIGHT:
               case TXa_RIGHT:
                  if (dat->leftcol < dat->maxcol)
                  {
                     dat->leftcol++;
                     upd = TRUE;
                  }
                  break;

               case TXk_HOME:
                  if (dat->leftcol != 0)
                  {
                     if (dat->leftcol > sx)
                     {
                        dat->leftcol -= sx;
                     }
                     else
                     {
                        dat->leftcol = 0;
                     }
                     upd = TRUE;
                  }
                  break;

               case TXk_END:
                  if (dat->leftcol != dat->maxcol)
                  {
                     if (dat->leftcol + sx < dat->maxcol)
                     {
                        dat->leftcol += sx;
                     }
                     else
                     {
                        dat->leftcol = dat->maxcol;
                     }
                     upd = TRUE;
                  }
                  break;

               case TXk_ESCAPE:
               case TXk_F10:                    // Focus to cmdline
               case TXk_MENU:
                  if (txwa->maincmd != TXHWND_NULL)
                  {
                     if (txwSetFocus(txwa->maincmd) == NO_ERROR)
                     {
                        TRACES(( "Focus changed to main cmdline window ...\n"));
                        if (mp2 != TXk_ESCAPE)  // then pass on keystroke
                        {
                           txwPostMsg( txwa->maincmd, TXWM_CHAR, mp1, mp2);
                           TRACES(( "Re-issue (menu) key to cmdline\n"));
                        }
                     }
                  }
                  break;

               case 'f':
               case 'n':
               case TXa_2:
               case TXc_N:
               case TXc_F:                      // search forward
                  sbCheckBack  = FALSE;
                  if (strlen( sbEntrySearchArgument) && (dat->sbdata->markSize != 0))
                  {
                     txwPostMsg( hwnd, TXWM_PERFORM_SEARCH, 0, 0);
                  }
                  else                          // dialog when no argument yet
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F7);
                  }
                  break;

               case 'b':
               case 'p':
               case TXa_1:
               case TXc_B:                      // search forward
                  sbCheckBack  = TRUE;
                  if (strlen( sbEntrySearchArgument) && (dat->sbdata->markSize != 0))
                  {
                     txwPostMsg( hwnd, TXWM_PERFORM_SEARCH, 0, 0);
                  }
                  else                          // dialog when no argument yet
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F7);
                  }
                  break;

               case TXk_F7:
                  if (txwSbViewFindDialog() == NO_ERROR)
                  {
                     if (sbCheckList)           // want list output
                     {
                        txwPostMsg( hwnd, TXWM_PERFORM_GREP, 0, 0);
                     }
                     else                       // execute single search
                     {
                        txwPostMsg( hwnd, TXWM_PERFORM_SEARCH, 0, 0);
                     }
                  }
                  break;

               case TXk_F8:
                  if (sbg_list != NULL)
                  {
                     ULONG  listcode = TXDID_OK;
                     TXRECT where    = {18,0,0,0}; // fixed position
                     TXTM   title;

                     sbg_list->selected = sbg_index;

                     TRACES(("sbg_list:%p at index: %lu\n", sbg_list, sbg_index));

                     sprintf( title, " %lu search hits, select with <F8> to view in context ", sbg_list->count);
                     listcode = txwListBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &where, title, "",
                                TXWD_HELPSMSG + 120, TXLB_MOVEABLE,
                                cMenuTextStand, cMenuBorder_top, // same color as menu
                                sbg_list);

                     if ((listcode >= TXDID_MAX) && (listcode != TXDID_CANCEL)) // valid and not escaped
                     {
                        ULONG   searchLine;
                        short   searchCol;
                        char   *description = sbg_list->items[ sbg_list->selected]->desc;

                        TRACES(("Selected %lu = '%s'\n", sbg_list->selected, description));

                        if (sscanf( description, "%lu %hd", &searchLine, &searchCol) == 2)
                        {
                           dat->sbdata->markRel  = searchLine;
                           dat->sbdata->markCol  = searchCol;
                           dat->sbdata->markSize = strlen( sbEntrySearchArgument);

                           if (((dat->sbdata->firstline + searchLine) <   dat->topline)     || // mark outside
                               ((dat->sbdata->firstline + searchLine) >= (dat->topline + sy))) // visible area
                           {
                              ULONG     margin = (sbCheckBack) ? sy - 4 : 3;
                              if (searchLine >= margin)
                              {
                                 dat->topline = dat->sbdata->firstline + searchLine - margin;
                              }
                              else
                              {
                                 dat->topline = dat->sbdata->firstline;
                              }
                              if (dat->topline > dat->maxtop) // no empty lines AFTER written lines
                              {
                                 dat->topline = dat->maxtop;
                              }
                           }
                           upd = TRUE;
                        }
                        sbg_index = sbg_list->selected + 1; // auto advance for toggle
                        if (sbg_index >= sbg_list->count)
                        {
                           sbg_index = 0;
                        }
                     }
                  }
                  else
                  {
                     sbCheckList = TRUE;
                     txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F7);
                  }
                  break;

               case TXa_F7:
               case TXa_U:
                  dat->sbdata->markSize = 0;    // unmark
                  upd = TRUE;
                  break;

               default:
                  txwPostMsg((TXWHANDLE) wnd->owner, TXWM_CHAR, mp1, mp2);
                  break;
            }
            break;

         case TXWM_PERFORM_GREP:                // perform GREP, no dialog
            if (sbg_list != NULL)
            {
               txSelDestroy( &sbg_list);
            }
            if ((sbg_list = txwSbViewGrepSelist()) != NULL)
            {
               sbg_index = 0;
               txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F8); // activate the toggle select list
            }
            else
            {
               sprintf( status, " Search argument '%s' not found. ", sbEntrySearchArgument);
               txwSetSbviewStatus( status, cSchemeColor);
            }
            break;

         case TXWM_PERFORM_SEARCH:              // perform a search operation now
            {
               ULONG     searchLine;            // line number to start the search from
               short     searchCol;
               char     *hitLine;

               if (dat->sbdata->markSize != 0)   // have a mark? (start from there)
               {
                  searchLine = dat->sbdata->markRel;
               }
               else if (sbCheckBack)            // backwards, start from end-window
               {
                  searchLine = dat->topline - dat->sbdata->firstline + sy;
               }
               else                             // start from begin window
               {
                  searchLine = dat->topline - dat->sbdata->firstline;
               }

               TRACES(("DO_SEARCH at line %lu\n", searchLine));

               if ((hitLine = txwSbSearch( sbEntrySearchArgument,
                                           sbCheckCase, sbCheckWord, sbCheckBack,
                                           &searchLine, &searchCol)) != NULL)
               {
                  dat->sbdata->markRel  = searchLine;
                  dat->sbdata->markCol  = searchCol;
                  dat->sbdata->markSize = strlen( sbEntrySearchArgument);

                  if (((dat->sbdata->firstline + searchLine) <   dat->topline)     || // mark outside
                      ((dat->sbdata->firstline + searchLine) >= (dat->topline + sy))) // visible area
                  {
                     ULONG     margin = (sbCheckBack) ? sy - 4 : 3;
                     if (searchLine >= margin)
                     {
                        dat->topline = dat->sbdata->firstline + searchLine - margin;
                     }
                     else
                     {
                        dat->topline = dat->sbdata->firstline;
                     }
                     if (dat->topline > dat->maxtop) // no empty lines AFTER written lines
                     {
                        dat->topline = dat->maxtop;
                     }
                  }
               }
               else
               {
                  TRACES(("Not found!\n"));
                  sprintf( status, " Search argument '%s' not found. ", sbEntrySearchArgument);
                  txwSetSbviewStatus( status, cSchemeColor);
               }
               upd = TRUE;
            }
            break;

         default:
            TRACES(("Warning: unhandled message in SbViewWinProc!\n"));
            break;
      }
      if (upd)
      {
          rc = txwInvalidateWindow( hwnd, FALSE, TRUE);
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwSbViewWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Build list of search results in the scrollbuffer
/*****************************************************************************/
static TXSELIST *txwSbViewGrepSelist            // RET   selection list or NULL
(
   void
)
{
   TXSELIST           *list  = NULL;            // total list
   TXS_ITEM           *item;                    // single item
   ULONG               lsize;                   // list-size
   ULONG               i;                       // index in list items
   ULONG               searchLine;              // line number to start the search from
   short               searchCol;
   char               *hitLine;                 // ASCII copy of search-hit line, or NULL

   ENTER();

   lsize = TXWS_GREP_LIMIT;
   if (TxSelCreate( lsize, lsize, lsize,
                    TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
   {
      list->astatus = TXS_AS_NOSTATIC      |    // all dynamic allocated
                      TXS_LST_DYN_CONTENTS;

      searchLine    = ((ULONG) -1);             // start search from OUTSIDE text

      for (i = 0; i < lsize;)                   // at most 'lsize' entries
      {
         if ((hitLine = txwSbSearch( sbEntrySearchArgument,
                                     sbCheckCase, sbCheckWord, FALSE,
                                     &searchLine, &searchCol)) != NULL)
         {
            //- add one item to the Selist
            if ((item  = TxAlloc( 1, sizeof(TXS_ITEM))) != NULL)
            {
               list->count    = i +1;           // actual items in list
               list->items[i] = item;           // attach item to list

               item->helpid = TXWH_USE_OWNER_HELP; // from owner-menu-item

               if (((item->text = TxAlloc( 1, TXMAXTM)) != NULL) &&
                   ((item->desc = TxAlloc( 1, TXMAXTM)) != NULL)  )
               {
                  item->value = TXDID_MAX + i;

                  if (searchCol < 40)
                  {
                     sprintf( item->text, "%-6lu: %-50.50s",
                                           searchLine, hitLine);
                  }
                  else
                  {
                     sprintf( item->text, "%-6lu: ...%-47.47s",
                                           searchLine, hitLine + searchCol - 25);
                  }
                  sprintf( item->desc,    "%-8.8lu %-3.3d %-50.50s",
                                           searchLine, searchCol, hitLine);
               }
               i++;
            }
         }
         else
         {
            break;                              // not found (anymore), stop search
         }
      }
      if (list->count == 0)                     // nothing found
      {
         txSelDestroy( &list);
      }
      else
      {
         list->tsize = list->count;
         list->vsize = list->count;             // make sure small lists use small window
      }
   }
   RETURN( list);
}                                               // end 'txwSbViewGrepSelist'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Search SBUF text for given string and specified search-options
// To search from start/end, specify a posLine that is OUTSIDE the text
/*****************************************************************************/
static char *txwSbSearch                        // RET   ptr to ASCII hit line
(
   char               *string,                  // IN    String to search for
   BOOL                caseSensitive,           // IN    Case sensitive search
   BOOL                wholeWord,               // IN    Space delimited word only
   BOOL                backwards,               // IN    Search backwards
   ULONG              *posLine,                 // INOUT position, Line number, Rel
   short              *posColumn                // OUT   position, Column
)
{
   char               *rc = NULL;
   ULONG               linecount;               // numer of lines in text
   ULONG               cLine;                   // current linenr, rel to SB firstline
   short               cCol;                    // current column
   char               *s;
   TXTM                word;                    // prepared case aware search string

   ENTER();

   if (ascLine == NULL)                         // exact case copy of search line
   {
      ascLine = TxAlloc( txwpScrBuff->width + 2, 1); // allow extra \0 and ' '
   }
   if (lowLine == NULL)
   {
      lowLine = TxAlloc( txwpScrBuff->width + 2, 1); // lowercased copy of the line
   }

   if (ascLine && lowLine && posLine)           // ASCII line buffers, and a position
   {
      linecount = txwpScrBuff->length - txwpScrBuff->firstline;

      //- prepare actual search argument for case-sensitivity
      TxCopy( word, string, TXMAXTM -1);
      if (wholeWord)
      {
         strcat( word, " ");
      }
      if (!caseSensitive)
      {
         TxStrToLower( word);
      }
      TRACES(("Search %s for '%s' starting at line: %lu\n",
             (backwards) ? "BACK" : "FORWARD", word, *posLine));

      //- set first line to be searched
      if (*posLine < linecount)                 // valid start position
      {
         if (backwards)
         {
            cLine = *posLine - 1;
         }
         else
         {
            cLine = *posLine + 1;
         }
      }
      else                                      // was outside of text
      {
         cLine = linecount -1;
         if (backwards)
         {
            cLine = linecount -1;
         }
         else
         {
            cLine = 0;
         }
      }

      //- search for the string, in specified search direction until found
      if (backwards)
      {
         TRACES(("Search Backwards from REL %lu, lc:%lu\n", cLine, linecount));
         for (;(cLine < linecount);)
         {
            txwSbLineCopy( ascLine, txwpScrBuff->firstline + cLine);
            strcpy( lowLine, ascLine);
            strcat( lowLine, " ");
            if (!caseSensitive)
            {
               TxStrToLower( lowLine);
            }
            if ((s = strstr( lowLine, word)) != NULL)
            {
               cCol = (s - lowLine);
               if (!wholeWord || (cCol == 0) || (lowLine[ cCol -1] == ' '))
               {
                  TRACES(("Found at %lu, col:%d\n", cLine, cCol));
                  *posLine   = cLine;
                  *posColumn = cCol;            // return found position in line
                  rc = ascLine;                 // and case-correct ASCII line
                  break;
               }
            }
            if (cLine > 0)
            {
               cLine--;
            }
            else
            {
               break;                           // done line 0, finished search
            }
         }
      }
      else
      {
         TRACES(("Search Forward from REL %lu, lc:%lu\n", cLine, linecount));
         while (cLine < linecount)
         {
            txwSbLineCopy( ascLine, txwpScrBuff->firstline + cLine);
            strcpy( lowLine, ascLine);
            strcat( lowLine, " ");
            if (!caseSensitive)
            {
               TxStrToLower( lowLine);
            }
            if ((s = strstr( lowLine, word)) != NULL)
            {
               cCol = (s - lowLine);
               if (!wholeWord || (cCol == 0) || (lowLine[ cCol -1] == ' '))
               {
                  TRACES(("Found at %lu, col:%d\n", cLine, cCol));
                  *posLine   = cLine;
                  *posColumn = cCol;            // return found position in line
                  rc = ascLine;                 // and case-correct ASCII line
                  break;
               }
            }
            cLine++;
         }
      }
   }
   RETURN (rc);
}                                               // end 'txwSbSearch'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Copy a single line from the scrollbuffer into a buffer, as ASCII C-string
/*****************************************************************************/
static char *txwSbLineCopy                      // RET   ptr to ASCII line
(
   char               *ascSstring,              // OUT   ASCII string representation
   ULONG               LineNr                   // IN    position, Line number
)
{
   ULONG               firstCell = LineNr * txwpScrBuff->width; // first cell to copy
   ULONG               i;

   ENTER();

   for (i = 0; i < txwpScrBuff->width; i++)
   {
      ascSstring[i] = txwpScrBuff->buf[ firstCell + i].ch;
   }
   ascSstring[i] = '\0';
   while (--i && (ascSstring[i] == ' '))
   {
      ascSstring[i] = '\0';
   }
   RETURN (ascSstring);
}                                               // end 'txwSbLineCopy'
/*---------------------------------------------------------------------------*/

