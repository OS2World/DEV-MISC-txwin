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
// TX keyboard (TXWM_CHAR) processing for window-classes
//
// Author: J. van Wijk
//
// JvW  06-07-1995 Initial version
// JvW  25-05-2014 Moved iKeySbView to txwsbuf.c (renamed from txwprint.c)

#include <memory.h>

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface


/*****************************************************************************/
// Input key handling for the TEXTVIEW window class
/*****************************************************************************/
ULONG txwIkeyTextview                           // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win = wnd->window;
   short               sx  = win->client.right  - win->client.left +1;
   short               sy  = win->client.bottom - win->client.top  +1;
   BOOL                upd = FALSE;             // window draw required
   BOOL                skp = FALSE;
   short               length;
   short               nr;                      // nr of lines in buf
   TXTEXTVIEW         *dat;
   char              **s;

   ENTER();
   if ((wnd != NULL) && (win != NULL))
   {
      dat = &win->tv;
      dat->maxtop = 0;
      dat->maxcol = 0;
      for (nr = 0, s = dat->buf; s && *s; s++)  // count entries
      {
         length = txSlen( *s);
         dat->maxcol = (USHORT) max( (short) dat->maxcol, length);
         nr++;
      }
      if (nr > sy)                              // text longer than window
      {
         dat->maxtop = nr - sy +1;              // allow one more than size
      }
      if ((short) dat->maxcol > sx)             // text wider as window
      {
         dat->maxcol = dat->maxcol - sx +1;     // allow one more than size
      }
      else
      {
         dat->maxcol = 0;
      }
      TRACES(("top:%4lu, left:%4hd, max:%4lu, lines:%4lu\n",
               dat->topline, dat->leftcol, dat->maxtop, nr));
      switch (mp2)                              // key value
      {
         case TXk_UP:
            if (dat->topline != 0)
            {
               dat->topline--;
               upd = TRUE;                      // full update (status)
            }
            break;

         case TXk_DOWN:
            if (dat->topline < dat->maxtop)
            {
               dat->topline++;
               upd = TRUE;                      // full update (status)
            }
            break;

         case TXk_PGUP:
            if (dat->topline > (ULONG) sy)
            {
               dat->topline -= sy;

               //- keep scrolling up until last line non-empty, or until the
               //- TOP line contains a # at start of line (#help files only)
               while ((strlen(win->tv.buf[win->tv.topline +sy -1]) ==  0 ) &&
                      (      (win->tv.buf[win->tv.topline][0])     != '#') &&
                      (      (win->tv.buf[              0][0])     == '#') &&
                      (dat->topline > (ULONG) sy))
               {
                  dat->topline--;
               }
               upd = TRUE;
            }
            else if (dat->topline != 0)
            {
               dat->topline = 0;
               upd = TRUE;
            }
            break;

         case TXk_PGDN:
            if (dat->topline + sy <= dat->maxtop)
            {
               dat->topline += sy;

               //- keep scrolling down until first line non-empty
               while ((strlen(win->tv.buf[win->tv.topline]) == 0) &&
                      (dat->topline + 1 <= dat->maxtop))
               {
                  dat->topline++;
               }
               upd = TRUE;
            }
            else if (dat->topline != dat->maxtop)
            {
               dat->topline = dat->maxtop;
               upd = TRUE;
            }
            break;

         case TXc_HOME:
            if (dat->topline != 0)
            {
               dat->topline = 0;
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

         case '<':
         case ',':
         case TXa_COMMA:
         case TXk_LEFT:
            if (dat->leftcol != 0)
            {
               dat->leftcol--;
               upd = TRUE;
            }
            break;

         case '>':
         case '.':
         case TXa_DOT:
         case TXk_RIGHT:
            if (dat->leftcol < dat->maxcol)
            {
               dat->leftcol++;
               upd = TRUE;
            }
            break;

         case TXk_HOME:
            if (dat->leftcol != 0)
            {
               dat->leftcol = 0;
               upd = TRUE;
            }
            break;

         case TXk_END:
            if (dat->leftcol != dat->maxcol)
            {
               dat->leftcol = dat->maxcol;
               upd = TRUE;
            }
            break;

         case TXa_F2:                           // print full text to SBUF
            for (s = dat->buf; s && *s; s++)    // all lines in text
            {
               length = txSlen( *s);
               if ((length != 0) || (skp == FALSE))
               {
                  TxPrint( "%s\n", *s);
               }
               skp = (length == 0);             // skip next line if empty too
            }
            break;

         default:
            txwPostMsg((TXWHANDLE) wnd->owner, TXWM_CHAR, mp1, mp2);
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
}                                               // end 'txwIkeyTextview'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Input key handling for the ENTRYFIELD window class
/*****************************************************************************/
ULONG txwIkeyEntryfield                         // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win;
   BOOL                upd = FALSE;             // window draw required
   short               length;
   short               index;
   TXENTRYFIELD       *dat;
   TXTM                completion;              // completion text
   char               *cur;

   ENTER();
   if ((wnd != NULL) && ((win = wnd->window) != NULL))
   {
      short            sx    = win->client.right  - win->client.left +1;
      USHORT           wid   = txwQueryWindowUShort( hwnd, TXQWS_ID);

      dat    = &win->ef;
      length = txSlen( dat->buf);
      index  = dat->leftcol + dat->curpos;      // index in string value buffer
      cur    = &dat->buf[ index];               // address of current char
      if (length > sx)                          // text wider as window
      {
         dat->maxcol = length - sx +1;
      }
      else
      {
         dat->maxcol = 0;
      }
      TRACES(("left:%4lu, max:%4lu, curpos: %4lu\n",
               dat->leftcol, dat->maxcol, dat->curpos));

      switch (mp2)                              // key value
      {
         case TXk_ENTER:
            txwAdd2History( hwnd, dat->buf);
            if (win->style & TXES_DLGE_FIELD)   // field in dialog
            {
               txwPostMsg((TXWHANDLE) wnd->owner, TXWM_CHAR, (ULONG) wid, mp2);
            }
            else                                // stand-alone, clear and upd
            {
               dat->curpos  = 0;
               dat->leftcol = 0;
               strcpy( dat->buf, "");
               upd = TRUE;                      // repaint
            }
            break;

         case TXc_K:                            // Store in history, clear
            txwAdd2History( hwnd, dat->buf);
            strcpy( dat->buf, "");
            dat->curpos  = 0;
            dat->leftcol = 0;
            upd = TRUE;
            break;

         case TXa_F8:                           // list history (FC2 compat)
         case TXk_F11:                          // list history buffer
            if (dat->history != NULL)
            {
               TXRECT     where = win->border;  // Close to the field's ULC
               TXSELIST  *list;
               ULONG      mcode;                // listbox result

               memset( completion, 0, TXMAXTM);
               if ((dat->leftcol + dat->curpos) > 0) // prefix there ...
               {
                  memcpy( completion, dat->buf, min( dat->leftcol + dat->curpos, TXMAXTM));
               }
               if (TxSelistHistory( dat->history, completion, &list) == NO_ERROR)
               {
                  list->flags |= TXSL_ITEM_DELETE; //- allow item delete using
                  mcode = txwListBox(              // Ctrl-D, as ListBox RC
                           TXHWND_DESKTOP,
                           hwnd,                // current is owner of popup
                           &where,              // popup position
                           "History", "",       // title in submenu
                           win->helpid,         // and same global help
                           TXLB_MOVEABLE,
                           cMenuTextStand,      // Use Menu color scheme
                           cMenuBorder_top,     // for client & border
                           list);

                  switch (mcode)
                  {
                     case TXDID_CANCEL:
                        break;

                     case TXc_D:                // delete current item
                        if (dat->history != NULL)
                        {
                           dat->history->current = list->selected;
                           txwDelCurrentHistory( dat->history);
                        }
                        break;

                     default:
                        dat->history->current = mcode - TXDID_MAX -1;
                        strcpy( dat->buf, txwGetHistory( dat->history, TXH_THIS));
                        break;
                  }
                  txSelDestroy( &list);

                  if (mcode == TXc_D)           // deleted item, display popup
                  {
                     txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F11);
                  }
                  upd = TRUE;
               }
               else
               {
                  TxMessage( TRUE, 0, "There are no history items that match the selection criteria");
               }
            }
            break;

         case TXc_D:                            // Delete from history, clear
            if (dat->history != NULL)
            {
               if ((dat->leftcol + dat->curpos) > 0) // prefix there ...
               {
                  memset( completion, 0, TXMAXTM);
                  memcpy( completion, dat->buf, min( dat->leftcol + dat->curpos, TXMAXTM));

                  txwDelCurrentHistory( dat->history);

                  if ((cur = txwRfndHistory( dat->history,
                                             completion, TRUE)) != NULL)
                  {
                     strcpy( dat->buf, cur);
                  }
               }
               else                             // no prefix, take previous
               {
                  txwDelCurrentHistory( dat->history);
                  strcpy( dat->buf, txwGetHistory( dat->history, TXH_PREV));
               }
            }
            upd = TRUE;
            break;

         case TXk_UP:
            if (dat->history != NULL)
            {
               if ((dat->leftcol + dat->curpos) > 0) // prefix there ...
               {
                  memset( completion, 0, TXMAXTM);
                  memcpy( completion, dat->buf, min( dat->leftcol + dat->curpos, TXMAXTM));
                  if ((cur = txwRfndHistory( dat->history,
                                             completion, TRUE)) != NULL)
                  {
                     strcpy( dat->buf, cur);
                  }
               }
               else                             // no prefix, take previous
               {
                  strcpy( dat->buf, txwGetHistory( dat->history, TXH_PREV));
               }
               upd = TRUE;                      // repaint
            }
            else
            {
               txwSetFocus( txwFindPrevFocus( hwnd, FALSE));
            }
            break;

         case TXk_DOWN:
            if (dat->history != NULL)
            {
               if ((dat->leftcol + dat->curpos) > 0) // prefix there ...
               {
                  memset( completion, 0, TXMAXTM);
                  memcpy( completion, dat->buf, min( dat->leftcol + dat->curpos, TXMAXTM));
                  if ((cur = txwFindHistory( dat->history,
                                             completion, TRUE)) != NULL)
                  {
                     strcpy( dat->buf, cur);
                     upd = TRUE;                // repaint
                  }
               }
               else                             // no prefix, take next
               {
                  strcpy( dat->buf, txwGetHistory( dat->history, TXH_NEXT));
               }
               upd = TRUE;                      // repaint
            }
            else
            {
               txwSetFocus( txwFindNextFocus( hwnd, FALSE));
            }
            break;

         case TXk_LEFT:
            if      (dat->curpos  != 0)         // not at start of field
            {
               dat->curpos--;
               txwSetCursorPos( hwnd, 0, dat->curpos);
            }
            else if (dat->leftcol != 0)         // scrolled field
            {
               dat->leftcol--;
               upd = TRUE;                      // repaint
            }
            break;

         case TXc_LEFT:                         // one word to left
            if (index > 0)
            {
               index--;
               cur--;
               while ((index > 0) && (*cur == ' '))
               {
                  index--;
                  cur--;
               }
               while ((index > 0) && (*cur != ' '))
               {
                  index--;
                  cur--;
               }
               if (index > 0)                   // stopped at space BEFORE word
               {
                  index++;
                  cur++;
               }
               if (index < dat->leftcol)
               {
                  dat->leftcol = index;         // curpos at start field
                  upd = TRUE;                   // repaint
               }
               dat->curpos = index - dat->leftcol;
               txwSetCursorPos( hwnd, 0, dat->curpos);
            }
            else                               // related stuff (scroll sbview)
            {
               txwPostMsg((TXWHANDLE) wnd->owner, TXWM_CHAR, mp1, mp2);
            }
            break;

         case TXk_RIGHT:
            if (index < dat->rsize -1)          // room to move/grow
            {
               if (index >= length)             // beyond current string-value
               {
                  strcat( dat->buf, " ");       // extend string by one space
               }
               if (dat->curpos  < (sx -1))      // not at end of field
               {
                  dat->curpos++;
                  txwSetCursorPos( hwnd, 0, dat->curpos);
               }
               else if (dat->rsize > sx)        // scroll field
               {
                  dat->leftcol++;
                  upd = TRUE;
               }
            }
            else if (dat->rsize == sx)          // no-scrolling field ?
            {
               if (dat->buf[0] == ' ')          // string starts with spaces
               {
                  memmove( dat->buf, dat->buf +1, txSlen( dat->buf));
               }
               upd = TRUE;
            }
            break;

         case TXc_RIGHT:                        // one word to right
            if (index < length -1)              // not at end of string
            {
               while ((index < length -1) && (*cur != ' '))
               {
                  index++;                      // skip the word itself
                  cur++;
               }
               while ((index < length -1) && (*cur == ' '))
               {
                  index++;                      // skip next whitespace
                  cur++;
               }
               if (index > dat->leftcol + sx)
               {
                  dat->leftcol = index - sx;
                  upd = TRUE;
               }
               dat->curpos  = index - dat->leftcol;
               txwSetCursorPos( hwnd, 0, dat->curpos);
            }
            else                                // related stuff (scroll sbview)
            {
               txwPostMsg((TXWHANDLE) wnd->owner, TXWM_CHAR, mp1, mp2);
            }
            break;

         case TXc_E:                            // delete to end of field
            *cur = '\0';
            upd = TRUE;                         // repaint
            break;

         case TXk_ESCAPE:
            if (win->style & TXES_DLGE_FIELD)   // field in dialog
            {
               txwPostMsg((TXWHANDLE) wnd->owner, TXWM_CHAR, mp1, mp2);
               break;
            }
         case TXc_BACKSP:                       // delete whole field
            *cur = '\0';
         case TXc_B:                            // delete to begin field
            strcpy( dat->buf, cur);
            upd = TRUE;                         // repaint
         case TXk_HOME:                         // cursor to start field
            if (dat->leftcol != 0)
            {
               dat->leftcol = 0;
               upd = TRUE;
            }
            dat->curpos = 0;
            txwSetCursorPos( hwnd, 0, dat->curpos);
            break;

         case TXk_END:
            if (dat->leftcol != dat->maxcol)
            {
               dat->leftcol = dat->maxcol;
               upd = TRUE;
            }
            dat->curpos = min((sx -1), length);
            txwSetCursorPos( hwnd, 0, dat->curpos);
            break;

         case TXk_BACKSPACE:
            if (index != 0)
            {
               if      (dat->curpos  != 0)      // not at start of field
               {
                  dat->curpos--;
                  txwSetCursorPos( hwnd, 0, dat->curpos);
               }
               else if (dat->leftcol != 0)      // scrolled filed
               {
                  dat->leftcol--;
                  upd = TRUE;                   // repaint
               }
               memmove( cur -1, cur, strlen(cur) +1);
               upd = TRUE;                      // repaint
            }
            break;

         case TXk_DELETE:
            if (index < length)
            {
               memmove( cur, cur +1, strlen(cur) +1);
               upd = TRUE;                      // repaint
            }
            break;

         default:
            if ((mp2 > TXk_ESCAPE) &&
                (mp2 < TXW_KEY_GROUP_1))        // possible ascii key
            {
               if ((length < dat->rsize) ||     // room to grow
                   ((index < dat->rsize) &&     // or no room needed
                    (txwa->insert == FALSE)))
               {
                  if (txwa->insert)
                  {
                     memmove( cur +1, cur, strlen(cur) +1);
                  }
                  else if (index >= length)     // beyond current string
                  {
                     strcat( dat->buf, " ");    // extend string by one space
                  }
                  *cur = (char) (mp2 & 0xff);
                  if (dat->curpos < (sx -1))    // not at end of field
                  {
                     dat->curpos++;
                     txwSetCursorPos( hwnd, 0, dat->curpos);
                  }
                  else if (dat->rsize > sx)     // scroll field
                  {
                     dat->leftcol++;
                  }
                  else                          // at end-of-field, no-scroll
                  {
                     if (dat->buf[0] == ' ')    // string starts with spaces
                     {
                        memmove( dat->buf, dat->buf +1, txSlen( dat->buf));
                     }
                  }
                  upd = TRUE;                   // repaint
               }
            }
            else
            {
               txwPostMsg((TXWHANDLE) wnd->owner, TXWM_CHAR, mp1, mp2);
            }
            break;
      }
      if (upd)
      {
         rc = txwInvalidateWindow( hwnd, FALSE, TRUE);
      }
      wnd->cursor.y = 0;
      wnd->cursor.x = dat->curpos;             // record actual cursor position
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwIkeyEntryfield'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Input key handling for the BUTTON window class
/*****************************************************************************/
ULONG txwIkeyButton                             // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win;

   ENTER();
   if ((wnd != NULL) && ((win = wnd->window) != NULL))
   {
      TXBUTTON        *dat   = &win->bu;
      TXWHANDLE        owner = (TXWHANDLE) wnd->owner;
      USHORT           wid   = txwQueryWindowUShort( hwnd, TXQWS_ID);

      switch (mp2)                              // key value
      {
         case 'x':                              // same as check-character 'x'
         case TXk_SPACE:
            switch (win->style & TXBS_PRIMARYSTYLES)
            {
               case TXBS_PUSHBUTTON:
                  if (win->style & TXBS_HELP)
                  {
                     txwPostMsg( hwnd,  TXWM_HELP,    (ULONG) wid, TXCMDSRC_PUSHBUTTON);
                  }
                  else
                  {
                     txwPostMsg( owner, TXWM_COMMAND, (ULONG) wid, TXCMDSRC_PUSHBUTTON);
                  }
                  break;

               case TXBS_AUTORADIO:
                  if (*(dat->checked) == FALSE) // going to SET this one ?
                  {
                     txwResetAutoRadioGroup( owner, // reset others in same group
                       txwQueryWindowUShort( hwnd, TXQWS_GROUP));
                     *(dat->checked) = TRUE;
                     txwInvalidateWindow( hwnd, FALSE, FALSE);
                  }
               case TXBS_RADIOBUTTON:           // just signal owner (dialog)
                  txwPostMsg( owner, TXWM_COMMAND, (ULONG) wid, TXCMDSRC_RADIOBUTTON);
                  break;

               case TXBS_AUTOCHECK:
                  *(dat->checked) = !(*(dat->checked));
                  txwInvalidateWindow( hwnd, FALSE, FALSE);
               case TXBS_CHECKBOX:              // signal owner (dialog)
                  txwPostMsg( owner, TXWM_COMMAND, (ULONG) wid, TXCMDSRC_CHECKBOX);
                  break;

               default:
                  txwPostMsg( owner, TXWM_CONTROL, TXMPFROM2SH(wid,0), 0);
                  break;
            }
            break;

         case TXk_ENTER:
            if (((win->style & TXBS_PRIMARYSTYLES) == TXBS_PUSHBUTTON) &&
                ((win->style & TXBS_HELP)          != 0              )  )
            {
               txwPostMsg( hwnd, TXWM_HELP, 0, 0);
            }
            else if (win->style & TXBS_DLGE_BUTTON)
            {
               txwPostMsg( owner, TXWM_COMMAND, (ULONG) wid,
                                  TXCMDSRC_PUSHBUTTON);
            }
            else                                // pass to owner (dismiss!)
            {
               txwPostMsg( owner, TXWM_CHAR, (ULONG) wid, mp2);
            }
            break;

         case TXk_UP:
         case TXk_LEFT:
            txwSetFocus( txwFindPrevFocus( hwnd, FALSE));
            break;

         case TXk_DOWN:
         case TXk_RIGHT:
            txwSetFocus( txwFindNextFocus( hwnd, FALSE));
            break;

         default:
            txwPostMsg( owner, TXWM_CHAR, mp1, mp2);
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwIkeyButton'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Input key handling for the LISTBOX window class
/*****************************************************************************/
ULONG txwIkeyListBox                            // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win;

   ENTER();
   if ((wnd != NULL) && ((win = wnd->window) != NULL))
   {
      USHORT           wid = txwQueryWindowUShort( hwnd, TXQWS_ID);
      BOOL             upd = FALSE;             // window draw required
      TXLISTBOX       *dat;

      if ((dat = &win->lb) != NULL)
      {
         TXWHANDLE        owner = (TXWHANDLE) wnd->owner;
         TXRECT           where = win->border;
         TXSELIST        *list  = dat->list;
         TXS_ITEM        *item  = NULL;

         TRECTA( "Where:  ", (&where));
         TRACES(("cpos:%4hu, ccol:%2hx, icol:%2hx, style:%8.8lx\n",
                  dat->cpos, dat->ccol, dat->icol, win->style));
         TRACES(("Drop-value: %s\n", (win->style & TXLS_DROP_VALUE) ? "YES" : "NO"));
         TRACES(("Drop-menu:  %s\n", (win->style & TXLS_DROP_MENU ) ? "YES" : "NO"));
         TRACES(("Spin-wrap:  %s\n", (win->style & TXLS_SPIN_WRAP ) ? "YES" : "NO"));

         TRACES(("List at:%8.8lx\n", list));

         if ((list != NULL) && (list->count != 0)) // we do have a real list
         {
            TRACES(("astatus:%8.8lx   userinfo:%8.8lx\n", list->astatus, list->userinfo));
            TRACES(("top:%6lu  select:%6lu count:%6lu\n", list->top, list->selected, list->count));
            TRACES(("asize:%6lu vsize:%6lu tsize:%6lu\n", list->asize, list->vsize,  list->tsize));

            if      (win->style & TXLS_DROP_VALUE) // value drop-down
            {
               switch (mp2)
               {
                  case TXk_ENTER:
                     if ((win->style & TXLS_DROP_ENTER) == 0)
                     {
                        txwPostMsg( owner, TXWM_CONTROL,
                                    TXMPFROM2SH(wid,TXLN_ENTER),
                                    (ULONG) list);
                        break;
                     }                          // else treat ENTER as a_ENTER
                  case TXa_ENTER:               // drop down
                  case TXk_MENU:
                     if (where.top > 0)
                     {                          // let popup fall on-top of
                        where.top--;            // current title/value-field
                     }
                     if (txwListBox( TXHWND_DESKTOP,
                            hwnd,               // current is owner of popup
                            &where,             // popup position
                            win->title, "",     // repeat title in popup
                            (owner != 0) ? wnd->owner->window->helpid
                                         : win->helpid, // try to get owner dlg help
                            TXLB_MOVEABLE,
                            win->clientclear.at, // inherit color scheme
                            win->borderclear.at,
                            list) != TXDID_CANCEL) // ENTER on popup item
                     {
                        //- notify owner of the original (drop_value) list so
                        //- some action could be taken by its WinProc (prompt)

                        txwPostMsg( owner, TXWM_CONTROL,
                                    TXMPFROM2SH(wid,TXLN_ENTER),
                                    (ULONG) list);
                     }
                     upd = TRUE;
                     break;

                  case TXk_DOWN:
                     if (list->selarray == NULL) // single select
                     {
                        ULONG this = list->selected;

                        do
                        {
                           if ((list->selected +1) < list->count)
                           {
                              list->selected++; // make next 'current'
                           }
                           else if (win->style & TXLS_SPIN_WRAP)
                           {
                              list->selected = 0; // wrap arround
                           }
                           else                 // wrap not allowed
                           {
                              list->selected = this; // reset to startpoint
                           }
                           item = list->items[list->selected];
                        } while ((item->flags & TXSF_DISABLED) &&
                                 (list->selected != this));

                        if (list->selected != this)
                        {
                           upd = TRUE;
                        }
                     }
                     break;

                  case TXk_UP:
                     if (list->selarray == NULL) // single select
                     {
                        ULONG this = list->selected;

                        do
                        {
                           if (list->selected > 0)
                           {
                              list->selected--; // make prev 'current'
                           }
                           else if (win->style & TXLS_SPIN_WRAP)
                           {
                              list->selected = list->count -1; // wrap arround
                           }
                           else                 // wrap not allowed
                           {
                              list->selected = this; // reset to startpoint
                           }
                           item = list->items[list->selected];
                        } while ((item->flags & TXSF_DISABLED) &&
                                 (list->selected != this));

                        if (list->selected != this)
                        {
                           upd = TRUE;
                        }
                     }
                     break;

                  case TXk_HOME:
                  case TXk_PGUP:
                     if (list->selarray == NULL) // single select
                     {
                        if (list->selected > 0)
                        {
                           list->selected = 0;  // start-of-list 'current'
                           upd = TRUE;
                        }
                     }
                     break;

                  case TXk_END:
                  case TXk_PGDN:
                     if (list->selarray == NULL) // single select
                     {
                        if ((list->selected +1) < list->count)
                        {
                           list->selected = list->count -1; // list-end 'current'
                           upd = TRUE;
                        }
                     }
                     break;

                  default:
                     if ((mp2 > TXk_ESCAPE) &&
                         (mp2 < TXW_KEY_GROUP_1)) // possible ascii key
                     {
                        ULONG   selnow = list->selected;

                        if (TxSelCharSelect( list, toupper(mp2)) != selnow)
                        {
                           upd = TRUE;
                        }
                     }
                     else
                     {
                        txwPostMsg( owner, TXWM_CHAR, mp1, mp2);
                     }
                     break;
               }
            }
            else if (win->style & TXLS_DROP_MENU)
            {
               ULONG        mcode;              // listbox result
               TXS_MENUBAR *mbar = (TXS_MENUBAR *) list->userinfo;

               switch (mp2)
               {
                  case TXa_ENTER:               // drop down
                  case TXk_ENTER:
                  case TXk_DOWN:
                  case TXk_UP:
                     mcode = txwListBox(
                              TXHWND_DESKTOP,
                              hwnd,             // current is owner of popup
                              &where,           // popup position
                              "", "",           // no title in submenu
                              win->helpid,      // and same global help
                              TXLB_MOVEABLE |   // move allowed
                              TXLB_P_FIX_ROW,   // keep row on create
                              cMenuTextStand,   // Use Menu color scheme
                              cMenuBorder_top,  // for client & border
                              list);

                     if (mcode != TXDID_CANCEL) // notify owner of selection
                     {
                        txwPostMsg( owner, TXWM_MENUSELECT,
                                    mcode, TXMPFROM2SH(wid,0));
                     }
                     if (mbar != NULL)          // make default for next time
                     {
                        int          m;
                        TXS_MENU    *menu;

                        for (m = 0; m < mbar->count; m++)
                        {
                           if ((menu = mbar->menu[m]) != NULL)
                           {
                              if (menu->list == list) // this is us ...
                              {
                                 mbar->defopen = m;
                                 break;
                              }
                           }
                        }
                     }
                     break;

                  case TXk_LEFT:                // to prev menu heading
                     txwSetFocus( txwFindPrevFocus( hwnd, FALSE));
                     break;

                  case TXk_RIGHT:               // to next menu heading
                     txwSetFocus( txwFindNextFocus( hwnd, FALSE));
                     break;

                  case TXs_F10:                 // Menu alternative (Gnome)
                  case TXk_F10:                 // Close Owner (MenuBar)
                  case TXk_MENU:                // behave like Esc/Cancel
                     txwPostMsg( owner, TXWM_MENUEND, TXDID_CANCEL, 0);
                     break;

                  default:
                     txwPostMsg( owner, TXWM_CHAR, mp1, mp2);
                     break;
               }
            }
            else                                // multi-line
            {
               ULONG      dr = NO_ERROR;
               ULONG      li;
               char       select;               // selection char
               BOOL       disabled = FALSE;
               BOOL       rightmov = FALSE;     // right will move to next
               TX1K       textbuf;

               switch (mp2)
               {
                  case TXk_F8:                  //- allow list-toggle (grep)
                  case TXk_RIGHT:               //- open submenu if present
                  case TXk_ENTER:               //- open/act unless disabled
                     if (list->selarray == NULL) // no multiple select list
                     {
                        li = (ULONG) (list->top + dat->cpos);

                        item     = list->items[li];
                        disabled = ((item->flags & TXSF_DISABLED) != 0);
                        rightmov = ((TxaExeSwitchNum( 'M', NULL, 0) & 1) == 1);

                        if ((mp2 == TXk_RIGHT) &&                   // right-arrow-key
                            (rightmov || disabled ||                // -M:1 switch or disabled
                            ((item->flags & TXSF_P_LISTBOX) == 0))) // not a submenu
                        {
                           disabled = TRUE;                         // don't execute and go
                           txwPostMsg( owner, TXWM_CHAR, mp1, mp2); // to next main-menu header
                        }
                     }
                     if (!disabled)
                     {
                        txwPostMsg( owner, TXWM_CONTROL,
                                    TXMPFROM2SH(wid,TXLN_ENTER), (ULONG) list);
                     }
                     else if (mp2 == TXk_ENTER)
                     {
                        sprintf( textbuf,
                                "Menu item '%s' is currently DISABLED!\n\n"
                                "You may need to perform other actions or use different "
                                "menu items first to enable the functionality behind "
                                "the one now selected.%s%s\n",
                                 item->text,
                                (item->info) ? "\n\nLikely reason(s) to disable this menu item:\n\n" : "",
                                (item->info) ?  item->info : "");

                        TxMessage( TRUE, TXWD_DISABLEDITEM, textbuf);
                     }
                     strcpy( txwa->listmatch, "");  //- reset string
                     break;

                  case TXk_SPACE:                   //- space, select
                     if (list->selarray != NULL)    //- multiple select list
                     {
                        li = (ULONG) (list->top + dat->cpos);

                        TxSelSetSelected( list, li, TX_TOGGLE, TXSF_MARK_STD);
                        upd = TRUE;
                     }
                     break;

                  case TXk_DOWN:
                     do
                     {
                        if (((list->count ) >  0         ) &&
                            ((dat->cpos +1) < list->vsize) &&
                            ((dat->cpos +1) < list->count)  )
                        {
                           dat->cpos++;         // make next 'current'
                           upd = TRUE;
                           dr  = NO_ERROR;
                        }
                        else if ((dr = TxSelScrollDown( list)) == NO_ERROR)
                        {
                           upd = TRUE;
                        }
                        else                    // at last item, wrap ?
                        {
                           TxSelSetPosition( list, TXSEL_TOP);
                           dat->cpos = 0;
                           upd = TRUE;
                        }
                        if ((list->selarray == NULL) && (list->count != 0))
                        {
                           if ((li = (ULONG) (list->top + dat->cpos)) < list->count)
                           {
                              item     = list->items[li];

                              TRACES(("list-item for disable-check: %lu\n", li));
                              disabled = (((item->flags & TXSF_AUTOSKIP) ||
                                           (item->flags & TXSF_SEPARATOR) ) &&
                                          ( item->flags & TXSF_DISABLED   )  );
                           }
                           else
                           {
                              disabled = FALSE;
                           }
                        }
                     } while ((disabled) && (dr == NO_ERROR));
                     strcpy( txwa->listmatch, ""); // reset string
                     TRACES(("k_DOWN processed\n"));
                     break;

                  case TXk_UP:
                     do
                     {
                        if ((dat->cpos) > 0)
                        {
                           dat->cpos--;         // make prev 'current'
                           upd = TRUE;
                           dr  = NO_ERROR;
                        }
                        else if ((dr = TxSelScrollUp( list)) == NO_ERROR)
                        {
                           upd = TRUE;
                        }
                        else                    // at top item, wrap ?
                        {
                           TxSelSetPosition( list, TXSEL_END);
                           dat->cpos = min( list->vsize, list->count) -1;
                           upd = TRUE;
                        }
                        if ((list->selarray == NULL) && (list->count != 0))
                        {
                           if ((li = (ULONG) (list->top + dat->cpos)) < list->count)
                           {
                              item = list->items[li];

                              TRACES(("list-item for disable-check: %lu\n", li));
                              disabled = (((item->flags & TXSF_AUTOSKIP) ||
                                           (item->flags & TXSF_SEPARATOR) ) &&
                                          ( item->flags & TXSF_DISABLED   )  );
                           }
                           else
                           {
                              disabled = FALSE;
                           }
                        }
                     } while ((disabled) && (dr == NO_ERROR));
                     strcpy( txwa->listmatch, ""); // reset string
                     TRACES(("k_UP processed\n"));
                     break;

                  case TXk_HOME:                // to start of list
                     TxSelSetPosition( list, TXSEL_TOP);
                     dat->cpos = 0;
                     upd = TRUE;
                     strcpy( txwa->listmatch, ""); // reset string
                     break;

                  case TXk_END:
                     TxSelSetPosition( list, TXSEL_END);
                     dat->cpos = min( list->vsize, list->count) -1;
                     upd = TRUE;
                     strcpy( txwa->listmatch, ""); // reset string
                     break;

                  case TXk_PGUP:
                     if (dat->cpos != 0)
                     {
                        dat->cpos = 0;
                        upd = TRUE;
                     }
                     else if (list->top != 0)
                     {
                        TxSelSetPosition( list, (list->top > list->vsize)
                                          ?     (list->top - list->vsize)
                                          :      TXSEL_TOP);
                        upd = TRUE;
                     }
                     strcpy( txwa->listmatch, ""); // reset string
                     break;

                  case TXk_PGDN:
                     if (dat->cpos < list->vsize -1)
                     {
                        dat->cpos = min( list->vsize, list->count) -1;
                        upd = TRUE;
                     }
                     else if ((list->top + list->vsize) < list->count)
                     {
                        TxSelSetPosition( list, list->top + list->vsize);
                        upd = TRUE;
                     }
                     strcpy( txwa->listmatch, ""); // reset string
                     break;

                  case TXc_U:
                  case TXc_F7:
                     if (TxSelSortUserData( list) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_R:
                  case TXc_F8:
                     if (TxSelReverseOrder( list) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_X:
                  case TXc_F2:
                     if (TxSelSortString( list, TXS_SORT_1) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_D:
                  case TXc_F1:
                     if (TxSelSortString( list, TXS_SORT_2) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_A:
                  case TXc_F9:
                     if (TxSelSortString( list, TXS_SORT_3) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_L:
                  case TXc_T:
                  case TXc_F5:
                     if (TxSelSortString( list, TXS_SORT_4) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_B:
                  case TXc_F:
                  case TXc_S:
                  case TXc_F6:
                     if (TxSelSortString( list, TXS_SORT_5) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_N:
                  case TXc_F3:
                     if (TxSelSortString( list, TXS_SORT_6) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXc_E:
                  case TXc_F4:
                     if (TxSelSortString( list, TXS_SORT_7) == TRUE) // list changed
                     {
                        txwListSort2frhText( hwnd); // update footer indicator
                        txwPositionListBox(  dat ); // fixup positions
                        upd = TRUE;
                     }
                     break;

                  case TXk_BACKSPACE:
                     if ((li = strlen(txwa->listmatch)) != 0)
                     {
                        txwa->listmatch[li -1] = '\0';
                     }
                     break;

                  case TXc_BACKSP:
                     strcpy( txwa->listmatch, ""); // reset string
                     break;

                  case TXa_A: case TXa_B: case TXa_C: case TXa_D: case TXa_E:
                  case TXa_F: case TXa_G: case TXa_H: case TXa_I: case TXa_J:
                  case TXa_K: case TXa_L: case TXa_M: case TXa_N: case TXa_O:
                  case TXa_P: case TXa_Q: case TXa_R: case TXa_S: case TXa_T:
                  case TXa_U: case TXa_V: case TXa_W: case TXa_X: case TXa_Y:
                  case TXa_Z:                   // any <Alt> + ASCII
                     {
                        strcpy( txwa->listmatch, ""); // reset string
                        switch (mp2)
                        {
                           case TXa_A: mp2 = (ULONG) 'a'; break;
                           case TXa_B: mp2 = (ULONG) 'b'; break;
                           case TXa_C: mp2 = (ULONG) 'c'; break;
                           case TXa_D: mp2 = (ULONG) 'd'; break;
                           case TXa_E: mp2 = (ULONG) 'e'; break;
                           case TXa_F: mp2 = (ULONG) 'f'; break;
                           case TXa_G: mp2 = (ULONG) 'g'; break;
                           case TXa_H: mp2 = (ULONG) 'h'; break;
                           case TXa_I: mp2 = (ULONG) 'i'; break;
                           case TXa_J: mp2 = (ULONG) 'j'; break;
                           case TXa_K: mp2 = (ULONG) 'k'; break;
                           case TXa_L: mp2 = (ULONG) 'l'; break;
                           case TXa_M: mp2 = (ULONG) 'm'; break;
                           case TXa_N: mp2 = (ULONG) 'n'; break;
                           case TXa_O: mp2 = (ULONG) 'o'; break;
                           case TXa_P: mp2 = (ULONG) 'p'; break;
                           case TXa_Q: mp2 = (ULONG) 'q'; break;
                           case TXa_R: mp2 = (ULONG) 'r'; break;
                           case TXa_S: mp2 = (ULONG) 's'; break;
                           case TXa_T: mp2 = (ULONG) 't'; break;
                           case TXa_U: mp2 = (ULONG) 'u'; break;
                           case TXa_V: mp2 = (ULONG) 'v'; break;
                           case TXa_W: mp2 = (ULONG) 'w'; break;
                           case TXa_X: mp2 = (ULONG) 'x'; break;
                           case TXa_Y: mp2 = (ULONG) 'y'; break;
                           case TXa_Z: mp2 = (ULONG) 'z'; break;
                        }                       // <Alt> + ASCII fall through!
                     }                          // with modified mp2 == ASCII
                  default:                      // search ASCII select-characters
                     if ((mp2 > TXk_ESCAPE) &&  // possible ascii key
                         (mp2 < TXW_KEY_GROUP_1) &&
                         (mp2 != '<') && (mp2 != '>') && // and no accelerator
                         (list->renderNewItem == NULL))
                     {
                        if (list->flags & TXSL_MULTI_QUICK)
                        {
                           strcat( txwa->listmatch, " ");
                           txwa->listmatch[ strlen( txwa->listmatch) -1] = (char) mp2;

                           for ( li = 0;
                                (li < list->count) && (upd == FALSE);
                                 li++)
                           {
                              item = list->items[li];

                              TRACES(("listmatch: '%s'  item: '%s'\n",
                                       txwa->listmatch, item->text));

                              if (strncasecmp( txwa->listmatch, item->text,
                                       strlen( txwa->listmatch)) == 0)
                              {
                                 TRACES(("Match for '%s' at item %lu = '%s'\n",
                                          txwa->listmatch, li, item->text));
                                 if (li < list->vsize)
                                 {
                                    dat->cpos = li;
                                    TxSelSetPosition( list, 0);
                                 }
                                 else
                                 {
                                    dat->cpos = list->vsize -1;
                                    TxSelSetPosition( list, li - dat->cpos);
                                 }
                                 upd = TRUE;    // force list-update
                              }
                           }
                           if (upd == FALSE)    // not found, remove char
                           {
                              txwa->listmatch[ strlen( txwa->listmatch) -1] = 0;
                           }
                        }
                        else                    // select on single char only
                        {
                           for ( li = 0;
                                (li < list->count) && (upd == FALSE);
                                 li++)
                           {
                              item = list->items[li];
                              if (item->index != 0) // explicit single quick-char
                              {
                                 select = item->text[ item->index -1];
                                 if ((tolower( select)) == (tolower((char) mp2)))
                                 {
                                    if (li < list->vsize)
                                    {
                                       dat->cpos = li;
                                       TxSelSetPosition( list, 0);
                                    }
                                    else
                                    {
                                       dat->cpos = list->vsize -1;
                                       TxSelSetPosition( list, li - dat->cpos);
                                    }
                                    upd = TRUE; // force list-update

                                    if (( win->style  & TXLS_CHAR_ENTER)  &&
                                        ((item->flags & TXSF_DISABLED) == 0))
                                    {
                                       txwPostMsg( owner, TXWM_CONTROL,
                                                   TXMPFROM2SH(wid,TXLN_ENTER),
                                                   (ULONG) list);
                                    }
                                 }
                              }
                           }
                        }
                     }
                     else
                     {
                        txwPostMsg( owner, TXWM_CHAR, mp1, mp2);
                     }
                     break;
               }
               if ((upd) && (list->selarray == NULL)) // single select list
               {
                  list->selected = (ULONG) (list->top + dat->cpos);
                  TRACES(( "dat->cpos:%hu top:%lu  selected now: %lu\n",
                            dat->cpos, list->top, list->selected));
               }
               if (list->flags & TXSL_MULTI_QUICK)
               {
                  if (strlen(txwa->listmatch) != 0)
                  {
                     sprintf( textbuf, "             Match: %s ", txwa->listmatch);
                     txwSetSbviewStatus( textbuf, cSchemeColor);
                  }
                  else
                  {
                     txwSetSbviewStatus( "", cSchemeColor);
                  }
               }
            }
            if (upd)
            {
               txwPostMsg( owner, TXWM_CONTROL,
                           TXMPFROM2SH(wid,TXLN_SELECT),
                           (ULONG) (list));
               rc = txwInvalidateWindow( hwnd, TRUE, TRUE);
            }
         }
         else                                   // no list available
         {
            TRACES(("NO list or EMPTY list attached to the window!\n"));

            txwPostMsg( owner, TXWM_CHAR, mp1, mp2); // allow things like Esc
         }
      }
      else
      {
         rc = TX_INVALID_DATA;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwIkeyListBox'
/*---------------------------------------------------------------------------*/

