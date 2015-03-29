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
// TX Windowed text, default window procedures
//
// Author: J. van Wijk
//
// JvW  05-09-2001 Added DefDlgProc and other Dlg functions
// JvW  15-07-1998 Initial version, split off from txwind

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface

// Generic window procedure, for any window-class including dialogs
static ULONG txwGenericWinProc                  // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

static char txwam_move[]  = "Use arrow keys to MOVE window; insert=SIZE, ENTER=end";
static char txwam_size[]  = "Use arrow keys to SIZE window; insert=MOVE, ENTER=end";
static char txwam_color[] = "Use arrow keys to change;  ENTER=end";
static TXTM txwam_custom;

/*****************************************************************************/
// Default window procedure, for any window-class except dialogs
/*****************************************************************************/
ULONG txwDefWindowProc                          // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      TRCLAS( "DefWindowProc", hwnd);
      switch (msg)
      {
         case TXWM_CLOSE:
            if (wnd == txwa->desktop)           // quit application
            {
               rc = txwPostMsg( hwnd, TXWM_QUIT, 0, 0);
            }
            else                                // delegate to owner
            {
               txwPostMsg((wnd->owner) ? (TXWHANDLE) wnd->owner
                                       :  TXHWND_DESKTOP, msg, mp1, mp2);
            }
            break;

         default:
            rc = txwGenericWinProc( hwnd, msg, mp1, mp2);
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwDefWindowProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Default window procedure, for dialogs including their control windows
/*****************************************************************************/
ULONG txwDefDlgProc                             // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      USHORT  focusid = txwQueryWindowUShort( (TXWHANDLE) txwa->focus, TXQWS_ID);

      TRCLAS( "DefDlgProc", hwnd);
      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)                        // key-value
            {
               case TXk_ESCAPE:
                  txwPostMsg( hwnd, TXWM_CLOSE, 0, 0);
                  break;

               case TXk_F4:                       //- unless captured higher up
                  txwDismissDlg( hwnd, TXDID_OK); //- handle as OK button
                  break;

               case TXk_ENTER:
                  if (txwIsMinimized( hwnd, TRUE) || // unless a parent or
                      txwIsMinimized( hwnd, FALSE) ) // this is minimized
                  {
                     rc = txwGenericWinProc( hwnd, msg, mp1, mp2); // restore
                  }
                  else                          // End the dialog, returning
                  {                             // the proper dialog-RC
                     switch (focusid)
                     {
                        case TXDID_CANCEL:
                           txwPostMsg( hwnd, TXWM_CLOSE, 0, 0);
                           break;

                        default:
                           txwDismissDlg( hwnd, focusid);
                           break;
                     }
                  }
                  break;

               case TXc_UP:                     // special keys bounced by
               case TXc_DOWN:                   // control to their owner
               case TXc_LEFT:
               case TXc_RIGHT:
               case TXc_PGUP:
               case TXc_PGDN:
               case TXc_HOME:
               case TXc_END:
               case TXa_UP:
               case TXa_DOWN:
               case TXa_LEFT:
               case TXa_RIGHT:
               case TXa_PGUP:
               case TXa_PGDN:                   // relay to scroll-buffer
               case TXa_HOME:
               case TXa_END:
               case TXk_HOME:
               case TXk_END:
               case TXk_F8:
               case TXk_F7:
               case TXa_F7:
               case TXa_U:
               case TXa_1:
               case TXa_2:
               case TXc_B:
               case TXc_F:
               case TXc_N:
               case TXc_P:
                  txwPostMsg((TXWHANDLE) txwa->sbview, TXWM_CHAR, mp1, mp2);
                  break;

               default:
                  rc = txwGenericWinProc( hwnd, msg, mp1, mp2);
                  break;
            }
            break;

         case TXWM_CLOSE:
            txwDismissDlg( hwnd, TXDID_CANCEL);
            break;

         case TXWM_DESTROY:                     // keep position for next time
            {                                   // if position struct was set
               TXRECT  deskpos;
               TXRECT *initial = (TXRECT *) txwQueryWindowPtr( hwnd, TXQWP_DATA);

               TRECTA( "DLG border on destroy", (&win->border));
               if (initial != NULL)
               {
                  txwQueryWindowPos( TXHWND_DESKTOP, FALSE, &deskpos);
                  initial->left = win->border.left - deskpos.left; //- calculate new pos
                  initial->top  = win->border.top  - deskpos.top;  //- relative to desktop
               }
            }
            break;

         case TXWM_COMMAND:
            switch (mp2)                        // command source
            {
               case TXCMDSRC_PUSHBUTTON:        // if not captured by higher
                  txwDismissDlg( hwnd, mp1);    // level, dismiss dialog on
                  break;                        // every pushbutton!

               default:                         // otherwise ignore the message
                  break;
            }
            break;

         case TXWM_INITDLG:
            if (mp1 != 0)
            {
               txwSetFocus( (TXWHANDLE) mp1);
               txwPostMsg(   hwnd, TXWM_ACTIVATE, TRUE, 0);
            }
            break;

         default:
            rc = txwGenericWinProc( hwnd, msg, mp1, mp2);
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwDefDlgProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Default window procedure, for any window-class including dialogs
/*****************************************************************************/
static ULONG txwGenericWinProc                  // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win = wnd->window;
   TXWHANDLE           parent = (TXWHANDLE) wnd->parent;
   TXWHANDLE           owner  = (TXWHANDLE) wnd->owner;
   USHORT              flags  = txwQueryWindowUShort( hwnd, TXQWS_FLAGS);
   USHORT              wid    = txwQueryWindowUShort( hwnd, TXQWS_ID);
   TXWHANDLE           target;
   TXSELIST           *list;
   ULONG               altcol = 0;

   ENTER();
   TRCMSG( (TXWHANDLE) wnd, msg, mp1, mp2);
   TRCLAS( "GenericWinProc", hwnd);

   switch (msg)
   {
      case TXWM_ACTIVATE:
         if ((BOOL) mp1)                        // set window active ?
         {
            flags |= TXFF_ACTIVE;
         }
         else
         {
            flags &= ~TXFF_ACTIVE;
         }
         txwSetWindowUShort( hwnd, TXQWS_FLAGS, flags);
         txwInvalidateBorder( hwnd);
         break;

      case TXWM_BORDER:
         if (txwIsWindowShowing( hwnd))
         {
            txwPaintBorder( wnd, (wnd == txwa->focus));
         }
         break;

      case TXWM_CHAR:
         if (txwa->arrowMode != TXW_ARROW_STD)  // special arrow handling
         {
            TRACES(("Handling arrow mode: %8.8lx\n", txwa->arrowMode));
            if ((mp2 == TXk_UP)   || (mp2 == TXk_RIGHT) ||
                (mp2 == TXk_DOWN) || (mp2 == TXk_LEFT)  ||
                (mp2 == TXk_PGUP) || (mp2 == TXk_PGDN)  ||
                (mp2 == TXk_HOME) || (mp2 == TXk_END )  ||
                (mp2 == TXk_INSERT))
            {
               switch (txwa->arrowMode)
               {
                  case TXW_ARROW_MOVE:
                  case TXW_ARROW_SIZE:
                     if (win->style & TXWS_MOVEABLE) // move & resize allowed
                     {
                        TXRECT  psize;          // parent pos & size
                        TXRECT  wsize;          // window pos & size
                        short   delta;

                        txwQueryWindowPos( parent, FALSE, &psize);
                        txwQueryWindowPos( hwnd,   FALSE, &wsize);

                        if (txwa->arrowMode == TXW_ARROW_MOVE)
                        {
                           switch (mp2)
                           {
                              case TXk_UP:
                                 txwSetWindowPos( hwnd, 0,  0, -1, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              case TXk_DOWN:
                                 txwSetWindowPos( hwnd, 0,  0, +1, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              case TXk_LEFT:
                                 txwSetWindowPos( hwnd, 0, -1,  0, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              case TXk_RIGHT:
                                 txwSetWindowPos( hwnd, 0, +1,  0, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              case TXk_PGUP:
                                 delta = psize.top - wsize.top;
                                 txwSetWindowPos( hwnd, 0,  0, delta, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              case TXk_PGDN:
                                 delta = psize.bottom - wsize.bottom;
                                 txwSetWindowPos( hwnd, 0,  0, delta, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              case TXk_HOME:
                                 delta = psize.left - wsize.left;
                                 txwSetWindowPos( hwnd, 0,  delta, 0, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              case TXk_END:
                                 delta = psize.right - wsize.right;
                                 txwSetWindowPos( hwnd, 0,  delta, 0, 0, 0,
                                                  TXSWP_MOVE | TXSWP_RELATIVE);
                                 break;

                              default:
                                 txwa->arrowMode = TXW_ARROW_SIZE;
                                 break;
                           }
                        }
                        else                    // size window
                        {
                           switch (mp2)
                           {
                              case TXk_UP:
                                 txwSetWindowPos( hwnd, 0,  0,  0,  0, -1,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              case TXk_DOWN:
                                 txwSetWindowPos( hwnd, 0,  0,  0,  0, +1,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              case TXk_LEFT:
                                 txwSetWindowPos( hwnd, 0,  0,  0,  -2, 0,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              case TXk_RIGHT:
                                 txwSetWindowPos( hwnd, 0,  0,  0,  +2, 0,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              case TXk_PGUP:
                                 delta = wsize.top - wsize.bottom;
                                 txwSetWindowPos( hwnd, 0,  0,  0,  0, delta,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              case TXk_PGDN:
                                 delta = psize.bottom - wsize.bottom;
                                 txwSetWindowPos( hwnd, 0,  0,  0,  0, delta,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              case TXk_HOME:
                                 delta = wsize.left - wsize.right + 7;
                                 txwSetWindowPos( hwnd, 0,  0,  0,  delta, 0,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              case TXk_END:
                                 delta = psize.right - wsize.right;
                                 txwSetWindowPos( hwnd, 0,  0,  0,  delta, 0,
                                                  TXSWP_SIZE | TXSWP_RELATIVE);
                                 break;

                              default:
                                 txwa->arrowMode = TXW_ARROW_MOVE;
                                 break;
                           }
                        }
                        if (txwa->arrowMode == TXW_ARROW_MOVE)
                        {
                           txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) txwam_move, 0);
                        }
                        else
                        {
                           txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) txwam_size, 0);
                        }
                     }
                     else
                     {
                        txwPostMsg( parent, msg, mp1, mp2);
                     }
                     break;

                  case TXW_ARROW_COLOR:
                     switch (mp2)
                     {
                        case TXk_LEFT:          // cycle through Scrollbuffer
                        case TXk_RIGHT:         // color alternatives (8)
                           if (txwa->sbview)
                           {
                              TXWINDOW  *sbwin = txwa->sbview->window;
                              if (mp2 == TXk_LEFT)
                              {
                                 sbwin->sb.altcol--;
                              }
                              else
                              {
                                 sbwin->sb.altcol++;
                              }
                              sbwin->sb.altcol &= TXSB_COLOR_MASK;
                              txwInvalidateWindow((TXWHANDLE) txwa->sbview, FALSE, FALSE);
                           }
                           break;

                        default:
                           switch (mp2)
                           {
                              case TXk_UP:
                                 txwColorScheme( TXWCS_PREV_SCHEME,  NULL);
                                 break;

                              case TXk_DOWN:
                                 txwColorScheme( TXWCS_NEXT_SCHEME,  NULL);
                                 break;

                              case TXk_HOME:
                                 txwColorScheme( TXWCS_FIRST_SCHEME, NULL);
                                 break;

                              case TXk_END:
                                 txwColorScheme( TXWCS_LAST_SCHEME,  NULL);
                                 break;

                              case TXk_PGUP:
                                 if ((txwcs->linestyle--) == 0)
                                 {
                                    txwcs->linestyle = TXW_CS_LAST;
                                 }
                                 break;

                              case TXk_PGDN:
                                 if ((txwcs->linestyle++) == TXW_CS_LAST)
                                 {
                                    txwcs->linestyle = 0;
                                 }
                                 break;
                           }
                           txwInvalidateAll();  // redraw all windows
                           break;
                     }
                     if (txwa->sbview)
                     {
                        TXWINDOW  *sbwin = txwa->sbview->window;
                        altcol = sbwin->sb.altcol;
                     }
                     sprintf( txwam_custom, "Colors: %19.19s %lu/%lu - %s",
                              txwcs->name, altcol, txwcs->linestyle, txwam_color);
                     txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) txwam_custom, 0);
                     break;

                  default:
                     break;
               }
            }
            else                                // end arrow mode
            {
               txwa->arrowMode = TXW_ARROW_STD;
               txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, 0, 0);
               if (mp2 != TXk_ENTER)
               {
                  txwPostMsg(hwnd, msg, mp1, mp2); // exec end-key if not ENTER
               }
            }
         }
         else
         {
            switch (mp2)                        // key-value
            {
               case TXk_F3:                     // Close Owner (Dlg) or self
                  switch (win->class)
                  {
                     case TXW_SBVIEW:           // should go to cmdline ...
                        txwPostMsg( hwnd, msg, mp1, TXk_TAB);
                        break;

                     default:
                        txwPostMsg((owner != 0) ? owner : hwnd, TXWM_CLOSE, 0, 0);
                        break;
                  }
                  break;

               case TXa_F3:                     // application exit
               case TXa_BACKQUOTE:              // emergency exit
                  txwPostMsg( hwnd, TXWM_QUIT, 0, 0);
                  break;

               case TXs_F1:                     // HELP alternative, Linux GNOME
                  txwPostMsg( hwnd, TXWM_HELP, 0, 0);
                  break;

               case TXa_C:
                  //- to be refined, move code to a WM_COMMAND so it can be
                  // called from the menu too (using an accelerator ?)
                  txwa->arrowMode = TXW_ARROW_COLOR;
                  if (txwa->sbview)
                  {
                     TXWINDOW  *sbwin = txwa->sbview->window;
                     altcol = sbwin->sb.altcol;
                  }
                  sprintf( txwam_custom, "Colors: %19.19s %lu/%lu - %s",
                           txwcs->name, altcol, txwcs->linestyle, txwam_color);
                  txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) txwam_custom, 0);
                  break;

               case TXa_M:
                  txwa->arrowMode = TXW_ARROW_MOVE;
                  txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) txwam_move, 0);
                  break;

               case TXk_INSERT:
                  txwa->insert = (txwa->insert) ? FALSE : TRUE;
                  txwSetCursorStyle( hwnd, txwa->insert);
                  break;

            #if defined (UNIX)
               case TXc_L:                      // Linux refresh screen
                  TXSCREEN_FLUSH_CACHED();      // flush screen caches
                  txwInvalidateAll();           // logical redraw all windows
                  TXSCREEN_ENDOF_UPDATE();      // one extra, just to be sure
                  break;
            #endif

               case TXa_SLASH:                  // toggle trace value
                  switch (TxTrLevel)            // and slowdown delay
                  {
                     case 0:
                        TxTrLevel    = 1;
                        break;

                     case 1:
                        TxTrLevel    = 100;
                        if (TxQueryLogFile( NULL, NULL) != NULL)
                        {
                           TXLN line;

                           sprintf( line, "Tracing to: %s ", TxQueryLogName());
                           txwSetDesktopTopLine( line, cSchemeColor);
                        }
                        else if (TxTrLogOnly == FALSE)
                        {
                           txwSetDesktopTopLine( "Tracing to SCREEN ...      ",
                                                  cSchemeColor);
                        }
                        else
                        {
                           txwSetDesktopTopLine( "Tracing to nowhere ...     ",
                                                  cSchemeColor);
                        }
                        break;

                     default:
                        TxTrLevel    = 0;
                        TxTrSlowDown = 0;
                        txwSetDesktopTopLine( "Tracing stopped, no delay ... ",
                                               cSchemeColor);
                        break;
                  }
                  break;

               case TXk_F12:                    // Minimize win to title only
               case TXa_F9:                     // (or undo that)
                  if (win->style & TXWS_MOVEABLE) // move & resize allowed
                  {
                     if (flags & TXFF_MINIMIZED)
                     {
                        TRACES(("Set MINIMIZED OFF for window:%8.8lx\n", hwnd))
                        flags &= ~TXFF_MINIMIZED;
                        txwSetWindowUShort( hwnd, TXQWS_FLAGS, flags);
                        txwSetWindowPos( hwnd, 0,
                                         wnd->restore.left,    wnd->restore.top,
                                         wnd->restore.right  - wnd->restore.left +1,
                                         wnd->restore.bottom - wnd->restore.top  +1,
                                         TXSWP_SIZE | TXSWP_MOVE | TXSWP_ABSOLUTE);
                     }
                     else                       // set to vertical size 0 or 1
                     {
                        TRACES(("Set MINIMIZED ON  for window:%8.8lx\n", hwnd))
                        flags |= TXFF_MINIMIZED;
                        txwSetWindowUShort( hwnd, TXQWS_FLAGS, flags);

                        wnd->restore = win->border; // new restore size/pos

                        txwSetWindowPos( hwnd, 0,  0, 0,
                                         win->border.right - win->border.left  +1,
                                        (win->style & TXWS_TITLEBORDER) ? 1 : 0,
                                         TXSWP_SIZE | TXSWP_ABSOLUTE);
                     }
                     txwInvalidateAll();
                  }
                  else
                  {
                     TRACES(("window NOT movable, delegate MINIMIZE to parent ...\n"));
                     txwPostMsg( parent, msg, mp1, mp2);
                  }
                  break;

               case TXa_F10:                    // Maximize to parent
               case TXa_F5:                     // restore size/position
                  if (win->style & TXWS_MOVEABLE) // move & resize allowed
                  {
                     TXRECT  b    = win->border; // current border
                     TXRECT  rect = wnd->restore; // previous pos/size
                     BOOL    update_restore = TRUE;

                     TRECTA( "restore", (&rect));
                     TRECTA( "border ", (&b));

                     if (flags & TXFF_MINIMIZED)
                     {
                        TRACES(("Set MINIMIZED OFF for window:%8.8lx\n", hwnd))
                        flags &= ~TXFF_MINIMIZED;
                        txwSetWindowUShort( hwnd, TXQWS_FLAGS, flags);
                        update_restore = FALSE; // never restore to MINIMIZED
                     }
                     if (mp2 == TXa_F10)        // maximize to parent size
                     {
                        txwQueryWindowRect((TXWHANDLE) wnd->parent, FALSE, &rect);

                        TRECTA( "parent ", (&rect));

                        if (wnd != txwa->desktop)
                        {
                           if ((rect.bottom != (b.bottom - b.top  +1)) ||
                               (rect.right  != (b.right  - b.left +1))  )
                           {
                              TRACES(("MAXIMIZE to parent for window:%8.8lx\n", hwnd))
                              if (update_restore)
                              {
                                 wnd->restore = b; // new restore size/pos
                              }
                              txwSetWindowPos( hwnd, 0,  0, 0, rect.right, rect.bottom,
                                               TXSWP_SIZE | TXSWP_MOVE);
                           }
                        }
                     }
                     else                       // restore to previous pos/size
                     {
                        if (update_restore)
                        {
                           wnd->restore = b;    // to toggle back and forth ...
                        }
                        txwSetWindowPos( hwnd, 0,
                                         rect.left,    rect.top,
                                         rect.right  - rect.left +1,
                                         rect.bottom - rect.top  +1,
                                         TXSWP_SIZE | TXSWP_MOVE | TXSWP_ABSOLUTE);
                     }
                  }
                  else
                  {
                     TRACES(("window NOT movable, delegate to parent ...\n"));
                     txwPostMsg( parent, msg, mp1, mp2);
                  }
                  break;

               case TXa_F12:                    // cycle through all SB styles
                  if (txwa->sbview)
                  {
                     TXWINDOW  *sbwin = txwa->sbview->window;

                     if ((sbwin->sb.altcol++) == TXSB_COLOR_MAXVAL)
                     {
                        sbwin->sb.altcol = TXSB_COLOR_NORMAL;
                     }
                     txwInvalidateWindow((TXWHANDLE) txwa->sbview, FALSE, FALSE);
                  }
                  break;

               default:
                  if (flags & TXFF_MINIMIZED)   // un-collapse on other keys
                  {                             // (does NOT work on child!)
                     TRACES(("Set MINIMIZED OFF for window:%8.8lx\n", hwnd))
                     flags &= ~TXFF_MINIMIZED;
                     txwSetWindowUShort( hwnd, TXQWS_FLAGS, flags);
                     txwSetWindowPos( hwnd, 0,
                                      wnd->restore.left,    wnd->restore.top,
                                      wnd->restore.right  - wnd->restore.left +1,
                                      wnd->restore.bottom - wnd->restore.top  +1,
                                      TXSWP_SIZE | TXSWP_MOVE | TXSWP_ABSOLUTE);
                     txwInvalidateAll();
                  }
                  else
                  {
                     if (txwIsMinimized( hwnd, TRUE)) // any parent minimized ?
                     {
                        txwPostMsg( parent, msg, mp1, mp2);
                     }
                     else                       // non move/size/exit keys
                     {
                        switch (mp2)
                        {
                           case TXk_TAB:        // move to other window
                           case TXs_TAB:
                              target = (mp2 == TXs_TAB)
                                     ? txwFindPrevFocus( hwnd, TRUE)
                                     : txwFindNextFocus( hwnd, TRUE);
                              if (target != hwnd)
                              {
                                 txwSetFocus( target);
                                 break;
                              }
                              else if (win->style & TXWS_TAB_2_OWNER)
                              {
                                 txwPostMsg( owner, msg, mp1, mp2);
                                 break;
                              }                 // when no TAB action, allow
                           default:             // class specific usage

                              //- to be refined, replace this by class-specific
                              //- processing on the HIGHEST level (see HexEd)

                              switch (win->class)
                              {
                                 case TXW_TEXTVIEW:
                                    rc = txwIkeyTextview( hwnd, mp1, mp2);
                                    break;

                                 case TXW_SBVIEW:
                                    rc = txwSbViewWinProc( hwnd, msg, mp1, mp2);
                                    break;

                                 case TXW_ENTRYFIELD:
                                    rc = txwIkeyEntryfield( hwnd, mp1, mp2);
                                    break;

                                 case TXW_BUTTON:
                                    rc = txwIkeyButton( hwnd, mp1, mp2);
                                    break;

                                 case TXW_LISTBOX:
                                    rc = txwIkeyListBox( hwnd, mp1, mp2);
                                    break;

                                 default:
                                    txwPostMsg( owner, msg, mp1, mp2);
                                    break;
                              }
                              break;
                        }
                     }
                  }
                  break;
            }
         }
         break;

#if defined (HAVEMOUSE)

      case TXWM_BUTTONDOWN:                     // Mouse button clicked
         txwMouseButtonDown( hwnd, mp1, mp2);
         break;

      case TXWM_BUTTONUP:
         TRACES(( "BtnUP:%4.4hx/%4.4hx on %8.8lx at % 3hd,%-3hd %s '%s'\n",
                   TXSH1FROMMP(mp2),
                   TXSH2FROMMP(mp2),  hwnd,
                   TXSH1FROMMP(mp1) - win->client.left,
                   TXSH2FROMMP(mp1) - win->client.top,
                   txwClassDescription( hwnd),
                   (win->title) ? win->title  : ""));

         if (TXMOUSEKEYSCA() == TXm_KS_NONE)    // no shift/alt/ctrl keys
         {
            if (txwQueryCapture() != TXHWND_NULL)
            {
               if (txwa->mDragFlags & TXSWP_MOVE)
               {
                  txwSetWindowPos( hwnd, TXHWND_NULL,
                                   TXMOUSECOL() - txwa->mDragCol,
                                   TXMOUSEROW() - txwa->mDragRow,
                                   0, 0, TXSWP_MOVE | TXSWP_RELATIVE);
               }
               else
               {
                  txwSetWindowPos( hwnd, TXHWND_NULL,
                                   0, 0,
                                   TXMOUSECOL() - txwa->mDragCol,
                                   TXMOUSEROW() - txwa->mDragRow,
                                   TXSWP_SIZE | TXSWP_RELATIVE);
               }
               txwSetCapture( TXHWND_NULL);
            }
         }
         break;

      case TXWM_MOUSEMOVE:                      // with button down (drag)
         TRACES(( "mMove:%4.4hx/%4.4hx on %8.8lx at % 3hd,%-3hd %s '%s'\n",
                   TXSH1FROMMP(mp2),
                   TXSH2FROMMP(mp2),  hwnd,
                   TXSH1FROMMP(mp1) - win->client.left,
                   TXSH2FROMMP(mp1) - win->client.top,
                   txwClassDescription( hwnd),
                   (win->title) ? win->title  : ""));

         if (TXMOUSEKEYSCA() == TXm_KS_NONE)    // no shift/alt/ctrl keys
         {
            if ((TXMOUSEBUTTON() & TXm_BUTTON2) == 0) // no drag on help
            {
               short  col = TXMOUSECOL();       // absolute mouse position!
               short  row = TXMOUSEROW();

               if (txwQueryCapture() == TXHWND_NULL)
               {
                  if ( txwIsDescendant( hwnd, txwa->modality) && // active
                      ((row <= win->client.top)    || // on window title line
                      ((row >= win->client.bottom) &&
                       (col >= win->client.right )))) // or lower-right corner
                  {
                     if (row <= win->client.top) // window move drag operation
                     {
                        txwa->mDragFlags = TXSWP_MOVE;
                     }
                     else                       // window resize drag operation
                     {
                        txwa->mDragFlags = TXSWP_SIZE;
                        col &= 0xfffe;          // size to EVEN column pos
                     }
                     if (win->style & TXWS_MOVEABLE) // frame itself movable
                     {
                        txwSetCapture( hwnd);
                        txwa->mDragCol = col;
                        txwa->mDragRow = row;
                     }
                     else if (((win->class == TXW_LISTBOX)   ||
                               (win->class == TXW_TEXTVIEW)) &&
                               (wnd->owner->window->style & TXWS_MOVEABLE))
                     {
                        txwSetCapture( owner);
                        txwa->mDragCol = col;
                        txwa->mDragRow = row;
                     }
                  }
               }
               else                             // window is being dragged
               {
                  if ((TXMOUSEBUTTON() & TXm_BUTTON1)) // full-window drag
                  {
                     if (txwa->mDragFlags & TXSWP_MOVE)
                     {
                        txwSetWindowPos( hwnd, TXHWND_NULL,
                                         col - txwa->mDragCol,
                                         row - txwa->mDragRow,
                                         0, 0, TXSWP_MOVE | TXSWP_RELATIVE);
                     }
                     else                       // to allow 1/2 move/size
                     {                          // of dialog-controls, only
                        col &= 0xfffe;          // size to EVEN column pos
                        txwSetWindowPos( hwnd, TXHWND_NULL,
                                         0, 0,
                                         col - txwa->mDragCol,
                                         row - txwa->mDragRow,
                                         TXSWP_SIZE | TXSWP_RELATIVE);
                     }
                     txwa->mDragCol = col;      // set new reference position
                     txwa->mDragRow = row;
                  }
               }
            }
         }
         break;

#endif

      case TXWM_CURSORVISIBLE:
         wnd->curvisible = (BOOL) mp1;
         txwSetCursorStyle( hwnd, txwa->insert);
         break;

      case TXWM_CREATE:
         txwSetCursorStyle( hwnd, txwa->insert);
         switch (win->class)
         {
            case TXW_SBVIEW:
               break;

            case TXW_ENTRYFIELD:
               if (win->style & TXES_MAIN_CMDLINE)
               {
                  txwa->maincmd  = hwnd;        // ENTRYFIELD commandline
                  TRACES(( "setting MAINCMD  to: %8.8lx\n", txwa->maincmd));
               }
               break;

            default:
               break;
         }
         break;

      case TXWM_DESTROY:                        // window will be destroyed
         switch (win->class)
         {
            case TXW_SBVIEW:
               txwa->sbview = NULL;             // SBVIEW for debug status
               break;

            case TXW_ENTRYFIELD:
               if (win->style & TXES_MAIN_CMDLINE)
               {
                  txwa->maincmd  = TXHWND_NULL; // ENTRYFIELD for commandline
                  TRACES(( "setting MAINCMD  to: %8.8lx\n", txwa->maincmd));
               }
               break;

            default:
               break;
         }
         break;                                 // notification

      case TXWM_HELP:                           // context-sensitive help
         {
            ULONG      helpid = TXWH_USE_WIN_HELP;

            switch (win->class)
            {
               case TXW_LISTBOX:
                  if ((list = win->lb.list) != NULL)
                  {
                     ULONG      li;
                     TXS_ITEM  *item;

                     if ((list->selarray == NULL) && // NOT multiple select
                         ((win->style & TXLS_DROP_MENU) == 0)) // no heading
                     {
                        li = (ULONG) (list->top + win->lb.cpos);

                        if (li < list->count)   // avoid empty list trap
                        {
                           item = list->items[li];

                           if ((helpid = item->helpid) == TXWH_USE_CMD_CODE)
                           {
                              helpid = item->value; // use cmd value
                              TRACES(("Using cmdcode %8.8lx as helpid\n", helpid));
                           }
                           else if (helpid == TXWH_USE_OWNER_HELP)
                           {
                              TRACES(("Delegate help to owner\n"));
                              txwPostMsg( owner, msg, mp1, mp2);
                           }
                        }
                     }
                  }
                  break;

               default:
                  break;
            }
            TRACES(("helpid: %8.8lx = %lu\n", helpid, helpid));
            if (helpid != TXWH_USE_OWNER_HELP)  // not delegated to owner ?
            {
               if (helpid == TXWH_USE_WIN_HELP) // not changed to specific ?
               {
                  helpid = win->helpid;         // use default window help
               }
               txwHelpDialog( helpid, 0, "");   // context-sensitive help
               TxCancelAbort();                 // don't set global abort
            }
         }
         break;                                 // after simple help screen

      case TXWM_MOVE:                           // window has been moved
         break;                                 // notification

      case TXWM_PAINT:                          // window completely visible
         if (txwIsWindowShowing( hwnd) ||       // or scrollbuf, painting a
             (win->class == TXW_SBVIEW))        // partly covered window
         {
            rc = txwPaintWindow( wnd, mp1);     // mp1 is border paint
         }
         break;

      case TXWM_SELECTED:
         if ((BOOL) mp1)                        // set window selected ?
         {
            flags |= TXFF_SELECTED;
         }
         else
         {
            flags &= ~TXFF_SELECTED;
         }
         txwSetWindowUShort( hwnd, TXQWS_FLAGS, flags);
         txwInvalidateBorder( hwnd);
         break;

      case TXWM_SETFOCUS:
         if (((BOOL) mp1) &&                    // got focus, will be on top,
             (win->style & TXWS_FOCUS_PAINT))   // so can be repainted
         {
            txwInvalidateWindow( hwnd, TRUE, TRUE);
         }
         else                                   // just update status part
         {                                      // and borders
            if (txwIsWindowShowing( hwnd))
            {
               if (win->style & TXWS_FOCUS_PAINT)
               {
                  txwInvalidateWindow( hwnd, TRUE, TRUE);
               }
               else
               {
                  txwPaintBorder( wnd, (BOOL) mp1);
               }
            }
            txwPaintWinStatus( wnd, NULL, 0);   // checks 'Showing' for status!
         }
         switch (win->class)                    // additional FOCUS processing
         {                                      // for specific classes
            case TXW_LISTBOX:
               if ((list = win->lb.list) != NULL)
               {
                  TRACES(("Drop-menu:  %s\n", (win->style & TXLS_DROP_MENU ) ? "YES" : "NO"));
                  TRACES(("Auto-drop:  %s\n", (win->style & TXLS_AUTO_DROP ) ? "YES" : "NO"));

                  if ((BOOL) mp1)               // got focus
                  {
                     TRACES(("LB:%8.8lx L:%8.8lx f:%4.4x t:'%s' F:'%s'\n",
                              win, list, flags, (win->title)  ? win->title  : "none",
                                                (win->footer) ? win->footer : "none"));

                     if (win->style & TXLS_DROP_MENU)
                     {
                        txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER,
                                    (ULONG) win->footer, 0);

                        if ((flags & TXFF_ACTIVE) == 0) // unless now dropped
                        {
                           if (win->style & TXLS_AUTO_DROP)
                           {
                              if (flags & TXFF_AUTODROPNEXT) // delay drop to next time
                              {
                                 TRACES(("Autodrop menu delayed until next time ...\n"));
                                 flags &= ~TXFF_AUTODROPNEXT;
                                 txwSetWindowUShort( hwnd, TXQWS_FLAGS, flags);
                              }
                              else
                              {
                                 txwPostMsg( hwnd, TXWM_CHAR, 0, TXa_ENTER); // forced drop
                              }
                           }
                        }
                     }
                     else                       // signal select on get-focus
                     {
                        txwPostMsg( owner, TXWM_CONTROL,
                                    TXMPFROM2SH(wid,TXLN_SETFOCUS),
                                    (ULONG) (win->lb.list));
                     }
                  }
                  else                          // lost focus
                  {
                     txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, 0, 0); // reset description
                  }
                  if (list->flags & TXSL_MULTI_QUICK)
                  {
                     if (strlen( txwa->listmatch) != 0)
                     {
                        txwSetSbviewStatus( "", cSchemeColor);
                        strcpy( txwa->listmatch, ""); // reset string
                     }
                  }
               }
               txwPostMsg( hwnd, TXWM_PAINT, FALSE, 0); // repaint, list-bar
               break;

            default:
               break;
         }
         break;

      case TXWM_SIZE:                           // window has been sized
         switch (win->class)                    // additional SIZE processing
         {                                      // for specific classes
            case TXW_LISTBOX:
               if ((list = win->lb.list) != NULL)
               {
                  if      (win->style & TXLS_DROP_VALUE)
                  {
                  }
                  else if (win->style & TXLS_DROP_MENU)
                  {
                  }
                  else                          // multi-line or spin
                  {
                     list->vsize = (ULONG) (win->client.bottom - win->client.top +1);
                     TRACES(("Updated list->vsize to %hu lines\n",
                              win->client.bottom - win->client.top +1));
                  }
               }
               break;

            default:
               break;
         }
         break;                                 // notification

      case TXWM_SHOW:                           // window visibility change
         break;                                 // notification

      case TXWM_STATUS:                         // checks own visibility!
         rc = txwPaintWinStatus( wnd, (char *) mp1, (BYTE) mp2);
         break;

      case TXWM_SETFOOTER:                      // set/reset footer text
         if (mp1)
         {
            win->footer = (char *) mp1;         // set new footer text
         }
         else
         {
            win->footer = wnd->oldFooter;       // reset to original
         }
         txwPaintBorder( wnd,                   // repaint will NOT check
                        (wnd == txwa->focus));  // for border visibility!
         break;

      default:
         if (win->class == TXW_SBVIEW)          // can handle user messages!
         {
            rc = txwSbViewWinProc( hwnd, msg, mp1, mp2);
         }
         else
         {
            TRACES(("Warning: unhandled message in lowest level WinProc!\n"));
         }
         break;
   }
   RETURN( rc);
}                                               // end 'txwGenericWinProc'
/*---------------------------------------------------------------------------*/


#if defined (HAVEMOUSE)
/*****************************************************************************/
// Generic MOUSE BUTTONDOWN processing, class-independant stuff
/*****************************************************************************/
ULONG txwMouseButtonDown                        // RET   TX_PENDING if not done
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win = wnd->window;
   TXWHANDLE           owner  = (TXWHANDLE) wnd->owner;
   USHORT              flags  = txwQueryWindowUShort( hwnd, TXQWS_FLAGS);
   USHORT              wid    = txwQueryWindowUShort( hwnd, TXQWS_ID);

   ENTER();

   TRACES(( "Click:%4.4hx/%4.4hx on %8.8lx at % 3hd,%-3hd %s '%s'\n",
             TXSH1FROMMP(mp2),
             TXSH2FROMMP(mp2),  hwnd,
             TXSH1FROMMP(mp1) - win->client.left,
             TXSH2FROMMP(mp1) - win->client.top,
             txwClassDescription( hwnd),
             (win->title) ? win->title  : ""));

   if (TXMOUSEKEYSCA() == TXm_KS_NONE)          // no shift/alt/ctrl keys
   {
      if (TXMOUSEBUTTON() == TXm_BUTTON2)
      {
         //- to be refined, might present a context-menu
         //- where one of the options is the HELP screen
         //- Ignore for now since it is annoying when used
         //- with other mouse packages that are more useful :-)

         //- txwPostMsg( hwnd, TXWM_HELP, 0, 0);
      }
      else                                      // perform default actions
      {
         short  col = TXMOUSECOL() - win->client.left; // relative to
         short  row = TXMOUSEROW() - win->client.top; // the window!
         short  sy  = win->client.bottom - win->client.top +1;

         TRACES(( "col:% 3hd row:% 3hd sy:% 3hd\n", col, row, sy));
         TRACES(( "FOCUS:%8.8lx owner:%8.8lx modality:%8.8lx maincmd:%8.8lx mainmenu:%8.8lx\n",
                   txwa->focus, txwa->focus->owner, txwa->modality, txwa->maincmd, txwa->mainmenu));

         if ((win->class == TXW_LISTBOX) &&     // is this a menu-heading ?
             (win->style & TXLS_DROP_MENU))     // close any open (sub)menu
         {
            TRACES(( "Click on menu-heading, re-open menu at: '%s'\n",
                     (win->title) ? win->title  : ""));

            txwa->reopenMenu = txwQueryWindowUShort( hwnd, TXQWS_ID);
            txwPostMsg((TXWHANDLE) txwa->focus, TXWM_CHAR, 0, TXk_MENU);
         }

         //- Close active popup-listbox when clicking outside of it
         else if ((txwa->focus->window->class == TXW_LISTBOX) &&
                  (txwa->focus->owner->window->style & TXCS_LIST_POPUP) &&
                  (txwa->focus->owner == (TXWINBASE *) txwa->modality)  &&
                  (txwIsDescendant( hwnd, txwa->modality) == FALSE))
         {
            TRACES(( "Close active listbox popup, and re-issue the message\n"));

            txwSendMsg((TXWHANDLE) txwa->focus->owner, TXWM_CLOSE, 0, 0);
            txwPostMsg( hwnd, TXWM_BUTTONDOWN, mp1, mp2);   // re-post message
         }

         //- toggle the main menu on a click anywhere on desktop-border
         //- top lines of the scrollbuf, menu-canvas or cmdline (when menu up)
         else if (((hwnd == (TXWHANDLE) txwa->desktop) && (col >  1)) ||
                  ((hwnd == (TXWHANDLE) txwa->sbview ) && (row <= 1)) ||
                  ((hwnd == txwa->maincmd) && (txwa->mainmenu  != 0)) ||
                   (hwnd == txwa->mainmenu))
         {
            TRACES(( "Toggle main menu, click on desktop/scrollbuf-top etc\n"));

            txwPostMsg((TXWHANDLE) txwa->focus, TXWM_CHAR, 0, TXk_MENU);
         }

         //- Close active dialog frame/canvas/textview on click closebutton [X]
         else if ( (txwIsDescendant( hwnd, txwa->modality)) &&
                   (row < 0) && (TXMOUSECOL() > (win->client.right -3)) &&
                  (((win->class == TXW_FRAME)     ||
                    (win->class == TXW_CANVAS)    ||
                    (win->class == TXW_TEXTVIEW)) &&
                   ((win->style  & TXCS_CLOSE_BUTTON))))
         {
            if (flags & TXFF_MINIMIZED)
            {
               TRACES(( "Undo minimized window [F12] button\n"));

               txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F12);
            }
            else
            {
               TRACES(( "Close dialog on click on [X] close button\n"));

               txwPostMsg( hwnd, TXWM_CLOSE, 0, 0);
            }
         }

         //- Translate click on 'scroll-bar' to UP/DOWN or PGUP/PGDN keys
         else if ( (txwIsDescendant( hwnd, txwa->modality)) &&
                   (TXMOUSECOL() > (win->client.right)) &&
                  (((win->class == TXW_LISTBOX)   ||
                    (win->class == TXW_TEXTVIEW)) ))
         {
            ULONG   key = TXk_PGDN;

            if      (TXMOUSEROW() == win->client.top)    key = TXk_UP;
            else if (TXMOUSEROW() == win->client.bottom) key = TXk_DOWN;
            else if (row < (sy / 2))                     key = TXk_PGUP;

            TRACES(( "Scrollbar click to key: %s\n", txwKeyDescription(key)));
            txwPostMsg( hwnd, TXWM_CHAR, 0, key);
         }

         //- other cases, try to switch focus, if OK, perform post-processing
         else if (txwIsWindowEnabled(hwnd))     // enabled ?
         {
            if (txwSetFocus( hwnd) == NO_ERROR) // to window under mouse
            {
               TRACES(( "Changed focus to clicked window, post-process ...\n"));

               //- to be refined, below processing should be moved to the
               //- the class specific procedures, that call this function
               //- and on _PENDING do their own thing ... (see HexEdit)

               switch (win->class)              // additional positioning
               {
                  case TXW_ENTRYFIELD:          // set cursor to mouse pos
                     {
                        TXENTRYFIELD *dat = &win->ef;

                        if (dat->buf && strlen( dat->buf))
                        {
                           dat->curpos = min( col, txSlen(dat->buf) - dat->leftcol);
                           txwInvalidateWindow( hwnd, TRUE, TRUE);
                        }
                     }
                     break;

                  case TXW_LISTBOX:
                     {
                        TXLISTBOX *dat  = &win->lb;
                        TXSELIST  *list = dat->list;

                        TRACES(("Drop-value: %s\n", (win->style & TXLS_DROP_VALUE) ? "YES" : "NO"));
                        TRACES(("Drop-menu:  %s\n", (win->style & TXLS_DROP_MENU ) ? "YES" : "NO"));

                        if ((list != NULL) && (list->count != 0))
                        {
                           if      (win->style & TXLS_DROP_VALUE)
                           {
                              if (TXMOUSECOL() > (win->client.right -2))
                              {
                                 //- when at end of control, force a drop-down
                                 txwPostMsg( hwnd, TXWM_CHAR, 0, TXa_ENTER);
                              }
                           }
                           else if (win->style & TXLS_DROP_MENU) // menu heading
                           {
                           }
                           else                 // multi-line, including menu
                           {
                              ULONG      li = (ULONG) (list->top + row);

                              TRACES(("Listbox style:%8.8lx li:%lu\n", win->style, li));

                              if (li < list->count)
                              {
                                 TRACES(("Item: '%s' = '%s'\n", list->items[li]->text,
                                                                list->items[li]->desc));

                                 //- always ENTER even on disabled (for error message)
                                 dat->cpos = row; //- Visual, and list position update
                                 TxSelSetSelected( list, li, TX_TOGGLE, TXSF_MARK_STD);

                                 txwPostMsg( owner, TXWM_CONTROL,
                                             TXMPFROM2SH(wid,TXLN_SELECT),
                                             (ULONG) (list));

                                 txwInvalidateWindow( hwnd, TRUE, TRUE);

                                 //- execute the selected line in list
                                 txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_ENTER);
                              }
                           }
                        }
                     }
                     break;

                  case TXW_BUTTON:              // push the button
                     txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_SPACE);
                     break;

                  default:
                     rc = TX_PENDING;
                     break;
               }
            }
         }
      }
   }
   RETURN( rc);
}                                               // end 'txwMouseButtonDown'
/*---------------------------------------------------------------------------*/
#endif


/*****************************************************************************/
// Dismiss dialog and pass-on result code
/*****************************************************************************/
ULONG txwDismissDlg
(
   TXWHANDLE           hwnd,                    // IN    dialog handle
   ULONG               result                   // IN    result code
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();
   TRACES(("Dismiss dlg %8.8lx with RC:%8.8lx = %lu\n", hwnd, result, result));

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      win->dlgResult        = result;
      wnd->us[TXQWS_FLAGS] |= TXFF_DLGDISMISSED;

      txwShowWindow( hwnd, FALSE);              // hide the window
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   #if defined (DEV32)
      txwInvalidateAll();                       // avoid VIO 64Kb buffer bug
   #endif
   RETURN (rc);
}                                               // end 'txwDismissDlg'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create a Dialog from memory-structures (created frame + linked controls)
/*****************************************************************************/
TXWHANDLE txwCreateDlg
(
   TXWHANDLE           parent,                  // IN    parent (ignored!)
   TXWHANDLE           owner,                   // IN    owner  window
   TXWINPROC           dlgproc,                 // IN    dialog procedure
   TXWHANDLE           dlg,                     // IN    dialog frame window
   PVOID               cData                    // IN    user control data
)
{
   TXWHANDLE           rc = 0;                  // function return
   TXWINBASE          *wnd;                     // frame window (dialog)
   TXWINDOW           *win;

   ENTER();

   TRACES(( "Frame hwnd:%8.8lx  dlgproc:%8.8lx  cData:%8.8lx\n", dlg, dlgproc, cData));

   //- to be refined (long term) create dialog from template instead of
   //- complete dialog frame + controls setup with CreateWindow
   //- until then, the parent specified here will be ignored (position)

   if ((wnd = txwValidateHandle( dlg, &win)) != NULL)
   {
      TXWHANDLE        focus;
      TXWINBASE       *vwnd;                    // validated winbase

      if ((focus = txwWindowFromID( dlg, win->dlgFocusID)) == TXHWND_NULL)
      {
         focus = dlg;                           // focus to dlg window itself
      }                                         // single control, no frame

      //- owner and dlgproc specified here overrule the template
      //- must be specified as 0 to CreateWindow (owner and winproc)

      if ((vwnd = txwValidateHandle( owner, NULL)) != NULL)
      {
         wnd->owner     = vwnd;
      }
      else
      {
         wnd->owner     = txwa->desktop;
      }
      wnd->winproc      = dlgproc;              // attach dialog procedure

      win->style            &= ~TXWS_VISIBLE;       //- set invisible
      wnd->us[TXQWS_FLAGS]  &= ~TXFF_DLGDISMISSED;  //- clear dismissed flag

      if (cData != NULL)                        // optional, USER PTR could be
      {                                         // set by caller already!
         wnd->ul[TXQWP_USER] = (ULONG) cData;
      }
      txwPostMsg( (TXWHANDLE) wnd, TXWM_INITDLG, focus, (ULONG) cData);

      rc = dlg;
   }
   RETURN (rc);
}                                               // end 'txwCreateDlg'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Process a (modal) dialog using a local msg-loop
/*****************************************************************************/
ULONG txwProcessDlg                             // RET   dialog rc (dismiss)
(
   TXWHANDLE           hwnd                     // IN    dialog handle
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXWINBASE          *wnd;                     // frame window (dialog)
   TXWINDOW           *win;

   ENTER();
   TRWINS("ProcessDlg");

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      TXWQMSG          qmsg;
      TXWHANDLE        oldModality;             // previous modality window

      //- Following is a workarround for focus/loose-focus paint problems
      oldModality    = txwa->modality;          // remember previous ...
      txwa->modality = TXHWND_DESKTOP;          // temporary to desktop
      txwSetFocus(     TXHWND_DESKTOP);         // remove focus from current
      txwSendMsg(      TXHWND_DESKTOP,          // and from desktop too!
                       TXWM_SETFOCUS, FALSE, 0);

      txwa->modality = hwnd;                    // and make dialog modal

      txwShowWindow( hwnd, TRUE);               // make the dialog visible

      win->dlgResult = TXDID_CANCEL;            // if not dismissed properly

      while (((wnd->us[TXQWS_FLAGS] & TXFF_DLGDISMISSED) == 0) &&
             (txwGetMsg(  &qmsg)) )
      {
         txwDispatchMsg( &qmsg);
      }

      txwa->modality = oldModality;             // restore previous

      if (qmsg.msg == TXWM_QUIT)
      {                                         // signal program termination
         txwPostMsg( TXHWND_DESKTOP,            // to the desktop window
                     TXWM_QUIT, 0, 0);          // in regular message loop
      }
      rc = win->dlgResult;                      // return dialog result
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwProcessDlg'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create, Process and Destroy a (modal) dialog
/*****************************************************************************/
ULONG txwDlgBox                                 // RET   dialog rc (dismiss)
(
   TXWHANDLE           parent,                  // IN    parent (ignored!)
   TXWHANDLE           owner,                   // IN    owner  window
   TXWINPROC           dlgproc,                 // IN    dialog procedure
   TXWHANDLE           dlg,                     // IN    dialog frame window
   PVOID               cData                    // IN    user control data
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXWINBASE          *wnd;                     // frame window (dialog)
   TXWINDOW           *win;

   ENTER();

   if ((wnd = txwValidateHandle( dlg, &win)) != NULL)
   {
      txwCreateDlg( parent, owner, dlgproc, dlg, cData);

      rc = txwProcessDlg( dlg);

      txwDestroyWindow( dlg);
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwDlgBox'
/*---------------------------------------------------------------------------*/

