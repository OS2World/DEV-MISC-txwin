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
// TX Windowed text handling
//
// Author: J. van Wijk
//
// JvW  29-08-2001   Added dialog windows
// JvW  20-08-2001   Added window movement and cleaned up
// JvW  16-05-1999   Added window USHORT/ULONG query/set functions
// JvW  25-07-1998   Split off drawing and default window proc
// JvW  07-07-1998   Initial version

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface



// Recursive set position and size (relative) for child windows
static void txwSetChildPos
(
   TXWHANDLE           hwnd,                    // IN    window handle or 0
   TXRECT             *move                     // IN    relative move/size
);

static BYTE  StaticVbuf[1024];                  // possible VIO bug avoider
       BYTE *txVbuf = StaticVbuf;               // scrollbuf upto 512 cells

/*****************************************************************************/
// Initialize TX window manager and create the desktop window
/*****************************************************************************/
TXWHANDLE txwInitializeDesktop                  // RET   Desktop handle
(
   TXWINDOW           *custom,                  // IN    custom desktop style
   TXWINPROC           winproc                  // IN    custom desktop proc
)
{
   TXWHANDLE           desktop;
   TXWINDOW            deskwin;
   TXWINDOW           *window  = custom;
   char               *csname;                  // color scheme name

   ENTER();

   #ifndef WIN32                                // ALWAYS check on Windows :-)
   if (TxaExeSwitchSet('W'))                    // or explicitly specified
   #endif                                       // default is 75 on startup
   {
      txwa->rsmaxlines = TxaExeSwitchNum( 'W', NULL, 0);
      if (txwa->rsmaxlines == 0)                // -W or -W:0
      {
         //- slightly smaller than screen (on WIN, fixed to 40 for others :-)
         txwa->rsmaxlines  = txwScrVisibleLines();
      }
      else
      {
         txwa->rsmaxlines += 2;                 // correct for later adjust
      }
   }

   if ((TxaExeSwitch('b') == FALSE) && (txwa->rsmaxlines) &&
       (TxScreenRows()    > (USHORT)   (txwa->rsmaxlines) ))
   {
      if (TxConfirm( 0,
         "The number of lines in your text window is larger than %hu lines,\n"
         "and might need vertical scrollbars when running in a GUI window.\n"
         "This does NOT work very well with this programs windowing!\n\n"
         "If you are seeing vertical scrollbars you should resize, you\n"
         "can do that now, or use the 'mode cols,lines' command later.\n\n"
         "Do you want to resize the window to %hu lines now ? [Y/N] : ",
          txwa->rsmaxlines, txwa->rsmaxlines -2))
      {
         TXLN       command;
         short      delta = TxScreenRows();     // current nr of rows

         sprintf( command, "mode %hu,%lu", TxScreenCols(), txwa->rsmaxlines -2);

         TxExternalCommand( command);           // set mode, will cls too

         if (TxScreenRows() > (USHORT) (txwa->rsmaxlines))
         {
            TxPrint( "Screen resize failed, 'mode' command missing from PATH ?\n");
         }
         else if (window != NULL)               // resized, update window
         {
            delta -= TxScreenRows();
            window->border.bottom -= delta;     // recalculate sizes
            window->client.bottom -= delta;
         }
      }
   }

   if (TxaExeSwitchSet(TXA_O_SCHEME))           // WIN color scheme
   {
      csname = TxaExeSwitchStr( TXA_O_SCHEME, "", "");
      txwColorScheme( csname[0], NULL);
   }
   if (TxaExeSwitchSet(TXA_O_STYLE))            // WIN linestyle
   {
      txwcs->linestyle = TxaExeSwitchNum( TXA_O_STYLE, "", 0);
   }

   desktop = TXHWND_NULL;

   TRACES(( "Maxlines now: %lu\n", txwa->rsmaxlines));

   txwa->screen.right  = TxScreenCols() -1;
   txwa->screen.bottom = TxScreenRows() -1;
   txwa->cursorRow     = TxCursorRow();
   txwa->cursorCol     = TxCursorCol();
   txwa->autoid        = 0;

   TRACES(( "ScreenCols: %lu   ScreenRows: %lu\n", TxScreenCols(), TxScreenRows()));

   if (window == NULL)                          // create default desktop style
   {
      window = &deskwin;
      txwSetupWindowData( 0, 0,                 // upper left corner
                          TxScreenRows(),       // vertical size
                          TxScreenCols(),       // horizontal size
                          TXWS_CANVAS, 0,       // style and help
                          ' ', ' ',
                          cSchemeColor,    cSchemeColor,
                          cDskTitleStand,  cDskTitleFocus,
                          cDskFooterStand, cDskFooterFocus,
                          "",  "",
                          window);
   }
   TRACES(( "window->border.bottom: %lu   window->border.right: %lu\n",
             window->border.bottom,       window->border.right));

   desktop = txwCreateWindow( 0,                // parent window
                              TXW_FRAME,        // class of this window
                              0,                // owner window
                              0,                // insert after ...
                              window,           // window setup data
                              winproc);         // window procedure


   txwa->focus         = (TXWINBASE *) desktop;
   txwSendMsg( desktop, TXWM_SETFOCUS, TRUE, 0);

   txwInitStandardDialogs();                    // needed for messageboxes etc

   txwSetAccelerator( TXWACCEL_SYSTEM, TXk_F1,  TXWACCEL_MHELP);

   #if   defined (DEV32)
   {
      COUNTRYCODE     ccode = {0};              // 0 = default codepage
      COUNTRYINFO     cinfo;
      ULONG           csize;                    // returned size in cinfo

      DosGetCtryInfo( sizeof(cinfo), &ccode, &cinfo, &csize);

      txwa->codepage = (ULONG) cinfo.codepage;
   }
   #elif defined (UNIX)
      txwa->codepage = 0;                       // assume no 437 for now
   #else
      //- codepage / 437 compatibility to be refined for DOS/WIN etc
   #endif

   if (txwa->desktop)
   {
      TxInputDesktopInit();                     // start keyboard reader
   }
   RETURN( desktop);
}                                               // end 'txwInitializeDesktop'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate TX window manager including the desktop and other windows
/*****************************************************************************/
void txwTerminateDesktop                        // RET   Desktop handle
(
   void
)
{
   ENTER();

   txwTermStandardDialogs();                    // messageboxes etc

   txwa->sbview  = NULL;                        // SBVIEW for debug status
   while (txwa->last != NULL)                   // destroy all windows
   {
      txwDestroyWindow((TXWHANDLE) txwa->last);
   }
   txwa->desktop = NULL;
   TxFreeMem( txwa->acceltable);

   TxSetCursorPos( txwa->cursorRow, txwa->cursorCol);
   TxSetCursorStyle( TRUE, txwa->insert);

   VRETURN();
}                                               // end 'txwTerminateDesktop'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if specified handle is an existing window
/*****************************************************************************/
BOOL txwIsWindow
(
   TXWHANDLE           hwnd                     // IN    window handle
)
{
   return (txwValidateHandle( hwnd, NULL) != NULL);
}                                               // end 'txwIsWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query window-handles related to the specified one
/*****************************************************************************/
TXWHANDLE txwQueryWindow
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               relation                 // IN    window relation
)
{
   TXWINBASE          *wnd;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, NULL)) != NULL)
   {
      TRCLAS( "Query", hwnd);
      switch (relation)
      {
         case TXQW_NEXT   : wnd = wnd->next;      break;
         case TXQW_PREV   : wnd = wnd->prev;      break;
         case TXQW_TOP    : wnd = txwa->last;     break;
         case TXQW_BOTTOM : wnd = txwa->desktop;  break;
         case TXQW_OWNER  : wnd = wnd->owner;     break;
         case TXQW_PARENT : wnd = wnd->parent;    break;
         default:           wnd = TXHWND_NULL;    break;
      }
   }
   else
   {
      wnd = TXHWND_NULL;
   }
   RETURN ((TXWHANDLE) wnd);
}                                               // end 'txwQueryWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get a pointer to the TXWINDOW data from window-handle
/*****************************************************************************/
TXWINDOW *txwWindowData                         // RET   valid window pointer
(
   TXWHANDLE           hwnd                     // IN    window handle
)
{
   TXWINDOW           *win;

   txwValidateHandle( hwnd, &win);
   return( win);
}                                               // end 'txwWindowData'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Initialize a setup window structure for Create
/*****************************************************************************/
void txwSetupWindowData
(
   short               row,                     // IN    top-left row (border)
   short               col,                     // IN    top-left column
   short               rows,                    // IN    nr of rows incl border
   short               cols,                    // IN    nr of cols incl border
   ULONG               style,                   // IN    window & border style
   ULONG               helpid,                  // IN    compound helpid
   BYTE                winchar,                 // IN    default window char
   BYTE                borchar,                 // IN    default border char
   BYTE                wincolor,                // IN    default window color
   BYTE                borcolor,                // IN    default border color
   BYTE                titlecolor,              // IN    default title color
   BYTE                titlefocus,              // IN    default t-focus color
   BYTE                footercolor,             // IN    default footer color
   BYTE                footerfocus,             // IN    default f-focus color
   char               *title,                   // IN    title text string
   char               *footertxt,               // IN    footer text string
   TXWINDOW           *win                      // OUT   window structure
)
{
   ENTER();

   TRACES(( "row:%hd  col:%hd  rows:%hd  cols:%hd\n", row, col, rows, cols));
   TRACES(( "style:%8.8lx  helpid:%lu  title:'%s'\n", style, helpid, title));

   memset( win, 0, sizeof(TXWINDOW));           // all unitialized fields zero

   win->style          = style;
   win->helpid         = helpid;
   win->clientclear.ch = winchar;
   win->clientclear.at = wincolor;
   win->borderclear.ch = borchar;
   win->borderclear.at = borcolor;
   win->titlecolor     = titlecolor;
   win->titlefocus     = titlefocus;
   win->title          = title;
   win->trhtext        = NULL;
   win->footercolor    = footercolor;
   win->footerfocus    = footerfocus;
   win->footer         = footertxt;
   win->frhtext        = NULL;

   win->border.top     = row;
   win->border.left    = col;
   win->border.bottom  = row + rows -1;
   win->border.right   = col + cols -1;

   win->client = win->border;                   // start with same size

   if (style & TXWS_TITLEBORDER) win->client.top++;
   if (style & TXWS_FOOTRBORDER) win->client.bottom--;
   if (style & TXWS_SIDEBORDERS)
   {
      win->client.left++;
      win->client.right--;
   }
   TRHEXS(200,win,sizeof(TXWINDOW),"TXWINDOW Setup");

   TRECTA("border", (&win->border));
   TRECTA("client", (&win->client));
   VRETURN();
}                                               // end 'txwSetupWindowData'
/*---------------------------------------------------------------------------*/

#if defined (NEVER)
/*****************************************************************************/
// Initialize the client and border rectangle structures in a Window
/*****************************************************************************/
void txwSetupWindowRect
(
   short               row,                     // IN    top-left row (border)
   short               col,                     // IN    top-left column
   short               rows,                    // IN    nr of rows incl border
   short               cols,                    // IN    nr of cols incl border
   ULONG               style,                   // IN    window & border style
   TXWINDOW           *win                      // INOUT window structure
)
{
   ENTER();
   TRACES(( "row:%hd  col:%hd  rows:%hd  cols:%hd\n", row, col, rows, cols));

   win->style          = style;
   win->border.top     = row;
   win->border.left    = col;
   win->border.bottom  = row + rows -1;
   win->border.right   = col + cols -1;

   win->client = win->border;                   // start with same size

   if (style & TXWS_TITLEBORDER) win->client.top++;
   if (style & TXWS_FOOTRBORDER) win->client.bottom--;
   if (style & TXWS_SIDEBORDERS)
   {
      win->client.left++;
      win->client.right--;
   }
   TRHEXS(200,win,sizeof(TXWINDOW),"TXWINDOW Setup");

   TRECTA("border", (&win->border));
   TRECTA("client", (&win->client));
   VRETURN();
}                                               // end 'txwSetupWindowRect'
/*---------------------------------------------------------------------------*/
#endif


/*****************************************************************************/
// Create a new base window element and initialize its window-data
/*****************************************************************************/
TXWHANDLE txwCreateWindow
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWCLASS            class,                   // IN    class of this window
   TXWHANDLE           owner,                   // IN    owner window
   TXWHANDLE           after,                   // IN    insert after ...
   TXWINDOW           *setup,                   // IN    window setup data
   TXWINPROC           winproc                  // IN    window procedure
)
{
   TXWINBASE          *aft = txwValidateHandle( after, NULL);
   TXWINBASE          *wnd = NULL;
   TXWINDOW           *win = NULL;

   ENTER();
   TRHEXS(200,setup,sizeof(TXWINDOW),"TXWINDOW Create");

   if ((win = TxAlloc(1, sizeof(TXWINDOW)))  != NULL)
   {
      if ((wnd = TxAlloc(1, sizeof(TXWINBASE))) != NULL)
      {
         TXWINBASE    *vwnd;                    // validated winbase
         TXWINDOW     *vwin;                    // validated window

         if ((vwnd = txwValidateHandle( owner, NULL)) != NULL)
         {
            wnd->owner     = vwnd;
         }
         else
         {
            wnd->owner     = txwa->desktop;
         }
         wnd->window       = win;
         wnd->winproc      = winproc;
         wnd->cursor.x     = 0;
         wnd->cursor.y     = 0;
         wnd->us[TXQWS_ID] = (txwa->autoid)--;  // assign next automatic id
         wnd->oldFocus     = (TXWHANDLE) txwa->focus;
         wnd->oldFooter    = setup->footer;

         *win = *setup;                         // (shallow) copy setup info
         win->class        = class;

         switch (class)
         {
            case TXW_SBVIEW:
               win->sb.status = TxAlloc( 1, TXMAXLN);

               if (TxaExeSwitchSet(TXA_O_COLOR)) // SB color scheme
               {
                  win->sb.altcol = TxaExeSwitchNum( TXA_O_COLOR, "", 1);
               }
               else                             // use SB colors for scheme
               {
                  win->sb.altcol = txwcs->sbcolors;
               }
               txwa->sbview = wnd;              // SBVIEW for debug status
               break;

            case TXW_ENTRYFIELD:
            case TXW_HEXEDIT:
               wnd->curvisible = TRUE;
               break;

            case TXW_BUTTON:
               switch (win->style & TXBS_PRIMARYSTYLES)
               {
                  case TXBS_RADIOBUTTON:
                  case TXBS_AUTORADIO:
                  case TXBS_CHECKBOX:
                  case TXBS_AUTOCHECK:
                     wnd->curvisible = TRUE;
                     wnd->cursor.x   = 1;
                     wnd->cursor.y   = (win->client.bottom - win->client.top) /2;
                     break;

                  default:
                     wnd->curvisible = FALSE;
                     break;
               }
               break;

            default:
               wnd->curvisible = FALSE;
               break;
         }

         if ((vwnd = txwValidateHandle( parent, &vwin)) != NULL)
         {
            wnd->parent = vwnd;
                                                // make position absolute
            win->client.top    += vwin->client.top;
            win->client.bottom += vwin->client.top;
            win->border.top    += vwin->client.top;
            win->border.bottom += vwin->client.top;
            win->client.left   += vwin->client.left;
            win->client.right  += vwin->client.left;
            win->border.left   += vwin->client.left;
            win->border.right  += vwin->client.left;
         }
         else
         {
            wnd->parent = txwa->desktop;
         }
         wnd->restore = win->border;            // initial restore size/pos

         if (win->style & TXWS_SAVEBITS)
         {
            txwSaveWindowContent( wnd);
         }

         if (aft == NULL)                       // no position specified
         {
            aft = txwa->last;
         }
         TRACES(( "new window after: %8.8lx, txwa: %8.8lx\n", aft, txwa));
         if (aft == NULL)                       // first window (desktop)
         {
            txwa->last    = wnd;
            txwa->desktop = wnd;
         }
         else                                   // attach after ...
         {
            wnd->next     = aft->next;          // new forward link
            if (aft->next != NULL)
            {
               aft->next->prev = wnd;           // rerouted backward link
            }
            aft->next     = wnd;                // rerouted forward link
            wnd->prev     = aft;                // new backward link
            if (txwa->last == aft)              // new last element ?
            {
               txwa->last = wnd;
            }
         }
         txwa->windows++;                       // update window count
         TRCLAS("Creating", (TXWHANDLE) wnd);

         if (winproc != NULL)                   // dialogs created without ...
         {
            //- to be refined, pass cData pointer as param
            //- cData to be added as param for CreateWindow
            txwPostMsg( (TXWHANDLE) wnd, TXWM_CREATE, 0, 0);
         }

         TRACES(("size of TXWINBASE : %d bytes\n", sizeof(TXWINBASE)));
         TRACES(("size of TXWINDOW  : %d bytes\n", sizeof(TXWINDOW)));
         TRACES(("size of TXRECT    : %d bytes\n", sizeof(TXRECT)));
         TRACES(("border: top:%hd left:%hd bot:%hd right:%hd\n", win->border.top,
                           win->border.left, win->border.bottom, win->border.right));
         TRACES(("client: top:%hd left:%hd bot:%hd right:%hd\n", win->client.top,
                           win->client.left, win->client.bottom, win->client.right));

         TRSTYL("window", (TXWHANDLE) wnd);
         TRECTA("border", (&win->border));
         TRECTA("client", (&win->client));
         TRACES(("parent:%8.8lx after:%8.8lx helpid:%8.8lx=%lu  #windows: %s%ld%s\n",
                  wnd->parent, wnd->prev,
                  win->helpid, win->helpid, CBY, txwa->windows, CNN));
      }
      else
      {
         TxFreeMem( win);
      }
   }
   RETURN( (TXWHANDLE) wnd);
}                                               // end 'txwCreateWindow'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Destroy a base window element and remove it from the list
/*****************************************************************************/
ULONG txwDestroyWindow                          // RET   result
(
   TXWHANDLE           hwnd                     // IN    window handle
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if (((wnd  = txwValidateHandle( hwnd, &win)) != NULL) && // valid, desktop
       ((wnd != txwa->desktop) ||  (wnd->next   == NULL)) ) // only when last
   {
      TXWHENUM         henum;
      TXWHANDLE        child;

      if ((win->class == TXW_SBVIEW) && (txwa->sbview == wnd))
      {
         txwa->sbview  = NULL;                  // SBVIEW for debug status
         TxFreeMem( win->sb.status);            // status string buffer
      }

      if (hwnd == txwa->modality)               // just in case ...
      {
         txwa->modality = TXHWND_DESKTOP;       // reset to desktop
      }
      if (txwIsDescendant((TXWHANDLE) txwa->focus, hwnd)) // we had focus
      {
         if (txwSetFocus(wnd->oldFocus) != NO_ERROR)
         {
            txwSetFocus( txwFindPrevFocus( hwnd, FALSE));
         }
      }

      TRACES(("Destroy children for window:%8.8lx\n", hwnd));
      if ((henum = txwBeginEnumWindows( hwnd)) != 0)
      {
         while ((child = txwGetNextWindow( henum)) != 0)
         {
            txwDestroyWindow( child);           // recurse over children
         }
         txwEndEnumWindows( henum);
      }

      TRACES(("wnd:%8.8lx win:%8.8lx id:%4.4hu\n", wnd, win, wnd->us[TXQWS_ID]));
      TRCLAS( "Destroy", hwnd);

      txwSendMsg( hwnd, TXWM_DESTROY, 0,  0);   // signal window-procedure
      txwDestroyMessages( hwnd);                // destroy pending messages
                                                // in the MsgQueue
      if (wnd->prev == NULL)
      {
         txwa->desktop   = wnd->next;           // destroying current desktop!
      }
      else
      {
         wnd->prev->next = wnd->next;
      }
      if (wnd->next == NULL)
      {
         txwa->last = wnd->prev;
      }
      else
      {
         wnd->next->prev = wnd->prev;
      }

      if (wnd->oldContent != NULL)              // something to restore ?
      {
         txwRestoreWindowContent( wnd, NULL, NULL);
         TxFreeMem( wnd->oldContent);           // saved image
      }
      TxFreeMem( win);                          // window data
      TxFreeMem( wnd->acceltable);              // accelerator table
      TxFreeMem( wnd);

      txwa->windows--;                          // update window count

      TRACES(("Total number of windows remaining in the list is now: %s%ld%s\n",
               (txwa->windows > 0) ? CBG : CBR, txwa->windows, CNN));
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwDestroyWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set focus to specified window, notify old and new focus-windows
/*****************************************************************************/
ULONG txwSetFocus
(
   TXWHANDLE           hwnd                     // IN    destination window
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;                     // window gaining focus
   TXWINDOW           *win;
   TXWINBASE          *losingFocus;             // window losing focus

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      TRCLAS( "Current win", (TXWHANDLE) txwa->focus);
      TRCLAS( "SetFocus to", hwnd);
      if (txwa->focus != wnd)                   // no focus already ?
      {
         if (txwIsDescendant( hwnd, txwa->modality)) // and inside modality
         {
            losingFocus   = txwa->focus;
            txwa->focus   = wnd;

            TRACES(("Focus from %8.8lx to %8.8lx\n", losingFocus, wnd));

            txwSendMsg((TXWHANDLE) losingFocus, TXWM_SETFOCUS, FALSE, 0);

            txwSendMsg((TXWHANDLE) txwa->focus, TXWM_SETFOCUS, TRUE,  0);

            txwSetCursorPos( hwnd, wnd->cursor.y, wnd->cursor.x);
         }
         else
         {
            rc = TX_FAILED;
            TRACES(("Focus NOT changed, target NOT in current MODAL area\n"));
         }
      }
      else
      {
         TRACES(("Focus NOT changed, was already at target window\n"));
         //- Send FOCUS message, to trigger 'on-focus' behaviour (mouse click)
         txwSendMsg((TXWHANDLE) txwa->focus, TXWM_SETFOCUS, TRUE,  0);
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwSetFocus'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Enable or disable specified window
/*****************************************************************************/
ULONG txwEnableWindow
(
   TXWHANDLE           hwnd,                    // IN    destination window
   BOOL                enable                   // IN    enable flag
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      TRACES(("enable: %lu <= %lu\n", enable, !(win->style & TXWS_DISABLED)));
      if (enable == (BOOL) (win->style & TXWS_DISABLED)) // status change ?
      {
         if (enable)
         {
            win->style &= ~TXWS_DISABLED;       // not disabled
         }
         else
         {
            win->style |=  TXWS_DISABLED;
         }
         txwSendMsg( hwnd, TXWM_ENABLE, enable, 0);
      }
      //- to be refined, enable/disable children recursively (no PostMsg)
      //- could cause problem with desktop being disabled ?
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwEnableWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query enable state of window
/*****************************************************************************/
BOOL txwIsWindowEnabled                         // RET   window is enabled
(
   TXWHANDLE           hwnd                     // IN    window to query
)
{
   BOOL                rc  = FALSE;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      rc = !(win->style & TXWS_DISABLED);
   }
   BRETURN( rc);
}                                               // end 'txwIsWindowEnabled'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Sets or clears windows VISIBLE flag, and shows/hides the window
/*****************************************************************************/
ULONG txwShowWindow
(
   TXWHANDLE           hwnd,                    // IN    destination window
   BOOL                visible                  // IN    visible flag
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      TRACES(("now visible: %s\n", ( (win->style & TXWS_VISIBLE )) ? "yes" : "no"));
      TRACES(("new visible: %s\n",                    ( visible  ) ? "yes" : "no"));
      if (visible != (BOOL) (win->style & TXWS_VISIBLE)) // status change ?
      {
         if (visible)
         {
            win->style |=  TXWS_VISIBLE;        // visible
            if (win->style & TXWS_SAVEBITS)
            {
               txwSaveWindowContent( wnd);      // save underlying cells
            }
            txwInvalidateWindow( hwnd, TRUE, TRUE);
         }
         else
         {
            win->style &= ~TXWS_VISIBLE;

            if (wnd->oldContent != NULL)        // something to restore ?
            {
               txwRestoreWindowContent( wnd, NULL, NULL);
               TxFreeMem( wnd->oldContent);     // saved image
            }
            else                                // repaint all
            {
               txwInvalidateAll();
            }
            if (hwnd == txwa->modality)         // just in case ...
            {
               txwa->modality = TXHWND_DESKTOP; // reset to desktop
            }
            if (txwIsDescendant((TXWHANDLE) txwa->focus, hwnd)) // we had focus
            {
               if (txwSetFocus(wnd->oldFocus) != NO_ERROR)
               {
                  txwSetFocus( txwFindPrevFocus( hwnd, FALSE));
               }
            }
         }
         txwSendMsg( hwnd, TXWM_SHOW, visible, 0);
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwShowWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query visible state (STYLE bits) of window (and ALL of its parents)
/*****************************************************************************/
BOOL txwIsWindowVisible                         // RET   window is visible
(
   TXWHANDLE           hwnd                     // IN    window to query
)
{
   BOOL                rc  = FALSE;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      TXWINBASE       *ancestor = wnd->parent;  // first ancestor level

      if (win->style & TXWS_VISIBLE)
      {
         rc = TRUE;
      }
      while (txwValidateHandle((TXWHANDLE) ancestor, &win) != NULL)
      {
         if ((win->style & TXWS_VISIBLE) == 0)
         {
            rc = FALSE;
            break;                              // one invisible ancestor!
         }
         else
         {
            ancestor = ancestor->parent;        // next ancestor level
         }
      }
   }
   BRETURN( rc);
}                                               // end 'txwIsWindowVisible'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query complete visibility of (client) window
/*****************************************************************************/
BOOL txwIsWindowShowing                         // RET   window fully visible
(
   TXWHANDLE           hwnd                     // IN    window to query
)
{
   BOOL                rc;

   ENTER();

   if ((rc = txwIsWindowVisible( hwnd)) != FALSE)
   {
      TXWINDOW           *win;
      TXWINBASE          *wnd;

      if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
      {
         rc = txwIsRectShowing( hwnd, &(win->client));
      }
   }
   BRETURN( rc);
}                                               // end 'txwIsWindowShowing'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query complete visibility of specified rectangle
/*****************************************************************************/
BOOL txwIsRectShowing                           // RET   rect fully visible
(
   TXWHANDLE           hwnd,                    // IN    start window or NULL
   TXRECT             *rect                     // IN    rectangle to query
)
{
   BOOL                rc  = TRUE;
   TXWINBASE          *wnd = txwValidateHandle( hwnd, NULL);
   TXWINBASE          *area;
   TXWCLASS            class;
   TXRECT              overlap;                 // overlapping area

   if (wnd == NULL)                             // invalid handle
   {
      wnd = txwa->sbview;
   }
   if (wnd != NULL)
   {
      for (area  = wnd->next;                   // start from next window in
           area != NULL;                        // Z-order possibly overlapping
           area  = area->next)                  // until last one (on top)
      {
         class = area->window->class;           // for performance optimization
         if ((class == TXW_CANVAS) ||           // most dialogs use a canvas
             (class == TXW_FRAME )  )           // help uses a simple frame
         {                                      // rest are likely controls
            if (txwIntersectRect( &(area->window->border), rect, &overlap))
            {
               TRECTA( "IsRectShowing :", ( rect                ));
               TRECTA( "covered by AREA", (&area->window->border));
               TRCLAS( "AREA  win CLASS", ((TXWHANDLE) area));
               rc = FALSE;                      // not completely visible
               break;                           // out of loop
            }
         }
      }
   }
   return( rc);
}                                               // end 'txwIsRectShowing'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find enabled and visible window that is 'next' to origin window
/*****************************************************************************/
TXWHANDLE txwFindNextFocus
(
   TXWHANDLE           hwnd,                    // IN    origin window
   BOOL                tabgroup                 // IN    next GROUP, not window
)
{
   TXWHANDLE           rc  = TXHWND_NULL;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   USHORT              grp = (tabgroup) ? wnd->us[TXQWS_GROUP] : 0;

   ENTER();
   TRCLAS( "From ", hwnd);

   do
   {
      if (wnd->next != NULL)
      {
         wnd = wnd->next;
      }
      else
      {
         wnd = txwa->desktop;
      }
      TRCLAS( "Probe", (TXWHANDLE) wnd);
      TRSTYL( "Probe", (TXWHANDLE) wnd);
      TRACES(("modality: %s\n", (txwIsDescendant((TXWHANDLE) wnd, txwa->modality)) ? "OK" : "outside"));
      TRACES(("enabled : %s\n", (!(wnd->window->style & TXWS_DISABLED)) ? "yes" : "no"));
      TRACES(("visible : %s\n", ( (wnd->window->style & TXWS_VISIBLE )) ? "yes" : "no"));
      TRACES(("groupid : %hu => %hu\n", grp, wnd->us[TXQWS_GROUP]));

      if (  (wnd == (TXWINBASE *) hwnd) ||     // current is only one, or
          ( txwIsDescendant((TXWHANDLE) wnd, txwa->modality) && //in modality
          (!(wnd->window->style & TXWS_DISABLED)) && // target enabled and
          ( (wnd->window->style & TXWS_VISIBLE )) && // could be visible
            ((grp == 0) || (grp != wnd->us[TXQWS_GROUP]))))
      {
         rc = (TXWHANDLE) wnd;
      }
   } while (rc == TXHWND_NULL);

   RETURN (rc);
}                                               // end 'txwFindNextFocus'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find enabled and visible window that is 'previous' to origin window
/*****************************************************************************/
TXWHANDLE txwFindPrevFocus
(
   TXWHANDLE           hwnd,                    // IN    origin window
   BOOL                tabgroup                 // IN    prev GROUP, not window
)
{
   TXWHANDLE           rc  = TXHWND_NULL;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   USHORT              grp = (tabgroup) ? wnd->us[TXQWS_GROUP] : 0;

   ENTER();
   TRCLAS( "From ", hwnd);

   do
   {
      if (wnd->prev != NULL)
      {
         wnd = wnd->prev;
      }
      else
      {
         wnd = txwa->last;
      }
      TRCLAS( "Probe", (TXWHANDLE) wnd);
      TRSTYL( "Probe", (TXWHANDLE) wnd);
      TRACES(("modality: %s\n", (txwIsDescendant((TXWHANDLE) wnd, txwa->modality)) ? "OK" : "outside"));
      TRACES(("enabled : %s\n", (!(wnd->window->style & TXWS_DISABLED)) ? "yes" : "no"));
      TRACES(("visible : %s\n", ( (wnd->window->style & TXWS_VISIBLE )) ? "yes" : "no"));
      TRACES(("groupid : %hu => %hu\n", grp, wnd->us[TXQWS_GROUP]));

      if (  (wnd == (TXWINBASE *) hwnd) ||     // current is only one, or
          ( txwIsDescendant((TXWHANDLE) wnd, txwa->modality) && //in modality
          (!(wnd->window->style & TXWS_DISABLED)) && // target enabled and
          ( (wnd->window->style & TXWS_VISIBLE )) && // could be visible
            ((grp == 0) || (grp != wnd->us[TXQWS_GROUP]))))
      {
         rc = (TXWHANDLE) wnd;
      }
   } while (rc == TXHWND_NULL);

   RETURN (rc);
}                                               // end 'txwFindPrevFocus'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Invalidate all windows in Z-order, causing a repaint for all
/*****************************************************************************/
void txwInvalidateAll
(
   void
)
{
   TXWINBASE          *wnd;

   ENTER();

   for (wnd = txwa->desktop; wnd != NULL; wnd = wnd->next)
   {
      txwInvalidateWindow( (TXWHANDLE) wnd, TRUE, FALSE);
   }
   VRETURN ();
}                                               // end 'txwInvalidateAll'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Invalidate specified window, forcing repaint
/*****************************************************************************/
ULONG txwInvalidateWindow
(
   TXWHANDLE           hwnd,                    // IN    destination window
   BOOL                border,                  // IN    invalidate border
   BOOL                children                 // IN    invalidate children
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      TRACES(("Invalidate border: %s,  children: %s\n",
             (border) ? "Yes" : "No", (children) ? "Yes" : "No"));
      TRECTA("client ", (&win->client));

      if (win->style & TXWS_SYNCPAINT)
      {
         txwSendMsg( hwnd, TXWM_PAINT, border, 0);
      }
      else
      {
         txwPostMsg( hwnd, TXWM_PAINT, border, 0);
      }

      if (children)                             // invalidate children too
      {
         TXWHENUM      henum;
         TXWHANDLE     child;

         if ((henum = txwBeginEnumWindows( hwnd)) != 0)
         {
            while ((child = txwGetNextWindow( henum)) != 0)
            {
               txwInvalidateWindow( child, TRUE, children); // recursive!
            }
            txwEndEnumWindows( henum);
         }
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwInvalidateWindow'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Invalidate border area of specified window only, forcing border repaint
/*****************************************************************************/
ULONG txwInvalidateBorder
(
   TXWHANDLE           hwnd                     // IN    destination window
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      if (win->style & TXWS_SYNCPAINT)
      {
         txwSendMsg( hwnd, TXWM_BORDER, 0, 0);
      }
      else
      {
         txwPostMsg( hwnd, TXWM_BORDER, 0, 0);
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwInvalidateBorder'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query window rectangle, as a rectangle indicating position and size
/*****************************************************************************/
BOOL txwQueryWindowPos
(
   TXWHANDLE           hwnd,                    // IN    window handle
   BOOL                border,                  // IN    include border
   TXRECT             *rect                     // OUT   client rectangle
)                                               //       absolute positions
{
   BOOL                rc = FALSE;              // function return
   TXWINDOW           *win;

   ENTER();
   TRARGS(("HWND:%8.8lx, %s pos\n", hwnd, (border) ? "border" : "client"));

   if (txwValidateHandle( hwnd, &win) != NULL)
   {
      *rect = (border) ? win->border : win->client;
      TRECTA("queryPos", rect);
      rc = TRUE;
   }
   BRETURN(rc);
}                                               // end 'txwQueryWindowPos'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query window rectangle, as a rectangle indicating the client area size
/*****************************************************************************/
BOOL txwQueryWindowRect
(
   TXWHANDLE           hwnd,                    // IN    window handle
   BOOL                border,                  // IN    include border
   TXRECT             *rect                     // OUT   client area size
)                                               //       (left/top are zero)
{
   BOOL                rc = FALSE;              // function return
   TXWINDOW           *win;

   ENTER();
   TRARGS(("HWND:%8.8lx, %s size\n", hwnd, (border) ? "border" : "client"));

   if (txwValidateHandle( hwnd, &win) != NULL)
   {
      TXRECT          *target = (border) ? &(win->border) : &(win->client);

      rect->left   = 0;                         // relative to 0,0
      rect->top    = 0;
      rect->right  = target->right  - target->left +1;
      rect->bottom = target->bottom - target->top  +1;
      TRECTA("queryRect", rect);
      rc = TRUE;
   }
   BRETURN(rc);
}                                               // end 'txwQueryWindowRect'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query USHORT window value
/*****************************************************************************/
USHORT txwQueryWindowUShort
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index                    // IN    index of window-ushort
)
{
   USHORT              rc = 0;                  // function return
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      if (index < TXQWS_SIZE)
      {
         rc = wnd->us[index];
         TRACES(("QueryWindowUShort %lu=%s : %4.4hx = %hu\n",
                  index, txwWusDescription(index), rc, rc));
      }
   }
   return (rc);
}                                               // end 'txwQueryWindowUShort'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query ULONG  window value
/*****************************************************************************/
ULONG  txwQueryWindowULong
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index                    // IN    index of window-ulong
)
{
   ULONG               rc = FALSE;              // function return
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      if (index < TXQWL_SIZE)
      {
         rc = wnd->ul[index];
      }
      TRACES(("index:%lu  value: %8.8lx = %lu\n", index, rc, rc));
   }
   RETURN (rc);
}                                               // end 'txwQueryWindowULong'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Query PTR    window value
/*****************************************************************************/
void  *txwQueryWindowPtr
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index                    // IN    index of window-ptr
)
{
   return((void *) txwQueryWindowULong( hwnd, index));
}                                               // end 'txwQueryWindowPtr'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set USHORT window value
/*****************************************************************************/
BOOL txwSetWindowUShort
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index,                   // IN    index of window-ushort
   USHORT              us                       // IN    value to set
)
{
   BOOL                rc = FALSE;              // function return
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      if (index < TXQWS_SIZE)
      {
         TRACES(("SetWindowUShort %lu=%s : %4.4hx = %hu\n",
                  index, txwWusDescription(index), us, us));
         wnd->us[index] = us;
         rc = TRUE;
      }
   }
   return (rc);
}                                               // end 'txwSetWindowUShort'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set ULONG  window value
/*****************************************************************************/
BOOL txwSetWindowULong
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index,                   // IN    index of window-ulong
   ULONG               ul                       // IN    value to set
)
{
   BOOL                rc = FALSE;              // function return
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();
   TRACES(("index:%lu  value:%8.8lx = %lu\n", index, ul, ul));

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      if (index < TXQWL_SIZE)
      {
         wnd->ul[index] = ul;
         rc = TRUE;
      }
   }
   BRETURN (rc);
}                                               // end 'txwSetWindowULong'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Set   PTR    window value
/*****************************************************************************/
BOOL txwSetWindowPtr
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index,                   // IN    index of window-ptr
   void               *p                        // IN    value to set
)
{
   return(txwSetWindowULong( hwnd, index, (ULONG) p));
}                                               // end 'txwSetWindowPtr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set window position related attributes
/*****************************************************************************/
BOOL txwSetWindowPos
(
   TXWHANDLE           hwnd,                    // IN    window handle or 0
   TXWHANDLE           after,                   // IN    after this in Z-order
   short               x,                       // IN    new abs x-position
   short               y,                       // IN    new abs y-position
   short               cx,                      // IN    new x-size
   short               cy,                      // IN    new y-size
   ULONG               fl                       // IN    positioning options
)
{
   BOOL                rc = TRUE;               // function return
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              mv = {0,0,0,0};          // move/size delta rectangle

   ENTER();

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      short            sx = win->border.right  - win->border.left  +1;
      short            sy = win->border.bottom - win->border.top   +1;
      TXRECT          *pc;                      // parent client window

      TRACES(("pos %hd, %hd  size %hd, %hd\n",
             win->border.left, win->border.top, sx, sy));

      if (txwValidateHandle((TXWHANDLE) wnd->parent, NULL) != NULL)
      {
         pc = &wnd->parent->window->client;
      }
      else                                      // simulate parent
      {
         pc = &txwa->screen;
      }
      TRECTA("Parent client", pc);

      if (fl & TXSWP_MOVE)                      // move requested
      {
         if      (fl & TXSWP_RELATIVE)          // relative to self
         {
            mv.left = x;
            mv.top  = y;
         }
         else if (fl & TXSWP_ABSOLUTE)          // absolute to screen
         {
            mv.left = x - win->border.left;
            mv.top  = y - win->border.top;
         }
         else                                   // relative to parent
         {
            mv.left = x - win->border.left + pc->left;
            mv.top  = y - win->border.top  + pc->top;
         }
      }
      if (fl & TXSWP_SIZE)                      // size requested
      {
         if (fl & TXSWP_RELATIVE)               // relative size
         {
            mv.right  = cx;
            mv.bottom = cy;
         }
         else                                   // absolute, calc relative
         {
            mv.right  = cx - sx;
            mv.bottom = cy - sy;
         }
      }
      TRECTA("delta-mv", (&mv));

      if ((mv.left  != 0) || (mv.top    != 0) || // real move or
          (mv.right != 0) || (mv.bottom != 0)  ) // resize requested
      {
         TXRECT        newB;
         TXRECT        newC;

         //- calculate new window rectangles
         txwMoveSizeRect( &win->client, &mv, &newC);
         txwMoveSizeRect( &win->border, &mv, &newB);

         TRECTA("new client", (&newC));
         TRECTA("new border", (&newB));

         if (((fl & TXSWP_RELATIVE) == 0) ||    // when relative,
            ((newB.right  >= pc->left   ) &&    // limit moves and
             (newB.left   <= pc->right  ) &&    // resize to borders
             (newB.top    <= pc->bottom ) &&    // just inside parent
             (newB.bottom >= pc->top    ) &&
             (newB.bottom >= newB.top   ) &&    // vertical   1 line
             (newB.right  >  newB.left+6) ))    // horizontal 6 columns
         {
            if (win->style & TXWS_SAVEBITS)     // window contents saved
            {
               //- cannot optimize with just two rectangles if the new border
               //- is completely within the old border, upto old client size
               //- Holds true for resizing, making window smaller (like a_F5)

               if (txwInRectangle( newB.top,    newB.left,  &win->client) &&
                   txwInRectangle( newB.bottom, newB.right, &win->client)  )
               {
                  txwRestoreWindowContent( wnd, NULL, NULL); // whole screen
               }
               else
               {
                  if ((fl & TXSWP_MOVE) ||      // move, or a resize
                      ((mv.bottom < 0)  ||      // to smaller size
                       (mv.right  < 0) ) )      // try to optimize
                  {
                     TXRECT oldB = win->border; // border without shadow
                     TXRECT newX = newB;        // border, for exclusion
                     TXRECT exclude1;           // optimized restore area-1
                     TXRECT exclude2;           // optimized restore area-2
                     BOOL   x2valid;            // 2nd area is present

                     if (fl & TXSWP_SIZE)       // fake a move to UL for
                     {                          // the exclusion calculation
                        newX.top--;             // make sure it overlaps
                        newX.left--;
                     }
                     if (win->style & TXWS_CAST_SHADOW)
                     {
                        oldB.right  += 2;       // add shadow areas to the
                        oldB.bottom += 1;       // old-position total area
                     }
                     x2valid = txwExclusionRectangles( &oldB,     &newX,
                                                       &exclude1, &exclude2);
                     txwRestoreWindowContent( wnd,
                        (exclude1.bottom >= exclude1.top) ? &exclude1 : NULL,
                                                (x2valid) ? &exclude2 : NULL);
                  }
               }
            }

            win->border = newB;                 // update border position/size
            win->client = newC;                 // update client position/size

            if ((mv.left  != 0) || (mv.top    != 0))
            {
               txwSendMsg( hwnd, TXWM_MOVE, 0,  0); // signal window-procedure
            }
            if ((mv.right != 0) || (mv.bottom != 0))
            {
               txwSendMsg( hwnd, TXWM_SIZE, 0,  0); // signal window-procedure
            }

            txwSetChildPos( hwnd, &mv);         // recurse over children

            if (txwIsDescendant((TXWHANDLE) txwa->focus, hwnd) && // focus
                (win->style & TXWS_SAVEBITS))   // and contents saved
            {
               txwInvalidateWindow( (TXWHANDLE) wnd, TRUE, TRUE);
            }
            else
            {
               txwInvalidateAll();
            }
         }
      }
   }
   else
   {
      rc = FALSE;
   }
   BRETURN (rc);
}                                               // end 'txwSetWindowPos'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Recursive set position and size (relative) for child windows
/*****************************************************************************/
static void txwSetChildPos
(
   TXWHANDLE           hwnd,                    // IN    window handle or 0
   TXRECT             *move                     // IN    relative move/size
)
{
   TXWHENUM      henum;
   TXWHANDLE     child;
   TXWINDOW     *cwin;

   ENTER();

   if ((henum = txwBeginEnumWindows( hwnd)) != 0)
   {
      //- calculate largest row, to allow 1/2 vertical size/move
      TXWINBASE    *wnd = (TXWINBASE *) hwnd;   // sized window (parent)
      short         largest = wnd->window->border.bottom;

      if (move->bottom > 0)                     // enlarging window ?
      {
         largest += move->bottom;               // update maximum size
      }

      while ((child = txwGetNextWindow( henum)) != 0)
      {
         TXRECT        mv = *move;

         cwin = txwWindowData( child);
         if ((cwin->style & TXWS_HCHILD_SIZE) == 0) // no full hor resize
         {
            //- horizontal sizing always in increments of 2, so 1/2 works OK
            if (cwin->style & TXWS_HCHILD2MOVE) // 1/2 move when resize parent
            {
               mv.left += (mv.right / 2);       // child moves in direction
            }                                   // that parent grows/shrinks
            if (cwin->style & TXWS_HCHILD_MOVE) // move when resize parent
            {
               mv.left += mv.right;             // child moves in direction
            }                                   // that parent grows/shrinks
            if (cwin->style & TXWS_HCHILD2SIZE) // 1/2 size when resize parent
            {
               mv.right /= 2;
            }
            else
            {
               mv.right = 0;
            }
         }
         if ((cwin->style & TXWS_VCHILD_SIZE) == 0) // no vertical   resize
         {
            if (cwin->style & TXWS_VCHILD2MOVE) // 1/2 move when resize parent
            {
               if ((mv.bottom == 1) || (mv.bottom == -1))
               {
                  mv.top += mv.bottom * (largest % 2); // on ODD values only
               }
               else
               {
                  mv.top += mv.bottom / 2;      // child moves in direction
               }
            }                                   // that parent grows/shrinks
            if (cwin->style & TXWS_VCHILD_MOVE) // move when resize parent
            {
               mv.top += mv.bottom;             // child moves in direction
            }                                   // that parent grows/shrinks
            if (cwin->style & TXWS_VCHILD2SIZE) // 1/2 size when resize parent
            {
               if ((mv.bottom == 1) || (mv.bottom == -1))
               {
                  mv.bottom = mv.bottom * (largest % 2); // on ODD values only
               }
               else
               {
                  mv.bottom = mv.bottom / 2;
               }
            }
            else
            {
               mv.bottom = 0;
            }
         }
         txwMoveSizeRect( &cwin->border, &mv, &cwin->border);
         txwMoveSizeRect( &cwin->client, &mv, &cwin->client);

         if ((mv.left  != 0) || (mv.top    != 0))
         {
            txwSendMsg( child, TXWM_MOVE, 0,  0); // signal window-procedure
         }
         if ((mv.right != 0) || (mv.bottom != 0))
         {
            txwSendMsg( child, TXWM_SIZE, 0,  0); // signal window-procedure
         }

         txwSetChildPos( child, move);          // recurse over grand-children
      }
      txwEndEnumWindows( henum);
   }
   VRETURN();
}                                               // end 'txwSetChildPos'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Resolve window-handle from specified ID, for specified parent
/*****************************************************************************/
TXWHANDLE txwWindowFromID
(
   TXWHANDLE           parent,                  // IN    window handle or 0
   USHORT              id                       // IN    window id
)
{
   TXWHANDLE           rc = TXHWND_NULL;        // function return
   TXWINBASE          *base;

   ENTER();
   TRACES(("search ID: %4.4hx = %hu\n", id, id));

   for ( base  = txwa->desktop;
        (base != NULL)       && (rc == TXHWND_NULL);
         base  = base->next)
   {
      if ((parent == TXHWND_NULL) || ((TXWHANDLE) base->parent == parent))
      {
         if (base->us[TXQWS_ID] == id)
         {
            rc = (TXWHANDLE) base;
         }
      }
   }
   RETURN (rc);
}                                               // end 'txwWindowFromID'
/*---------------------------------------------------------------------------                    */

