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
// TX Windowed text utility functions
//
// Author: J. van Wijk
//
// JvW  01-09-2001 Added Begin/EndEnumWindows and GetNextWindow
// JvW  02-01-2000 Fixed qsort callback bug in history sort (DOS 16bit only)
// JvW  31-12-1999 Updated current-position after find existing cmd in history
// JvW  10-07-1998 Initial version

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private interface

#include <time.h>                               // time functions


#define TXWE_SIGNATURE   0xFEEDBEEF             // valid magic signature value

typedef struct txwe_element                     // window enumeration element
{
   ULONG               signature;               // signature value
   TXWINBASE          *parent;                  // parent window for enum
   TXWINBASE          *nextstart;               // start for next search
} TXWE_ELEMENT;                                 // end of struct "txa_element"



// Compare History order for descending sort
static int txwHistoryDescend
(
   const void         *h1,
   const void         *h2
);

// Compare History order for ascending sort
static int txwHistoryAscend
(
   const void         *h1,
   const void         *h2
);

static char         ascii[32];


/*****************************************************************************/
// Validate window-handle and return corresponding TXWBASE pointer
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
TXWINBASE *txwValidateHandle                    // RET   window base pointer
(
   TXWHANDLE           hwnd,                    // IN    handle to validate
   TXWINDOW          **win                      // OUT   attached window
)
{
   TXWINBASE          *wnd = NULL;
   TXWINBASE          *chk = NULL;

   switch (hwnd)
   {
      case TXHWND_NULL:                         // invalid by definition
         break;

      case TXHWND_DESKTOP:
      case TXHWND_BOTTOM:
         wnd = txwa->desktop;
         break;

      case TXHWND_TOP:
         wnd = txwa->last;
         break;

      case TXHWND_OBJECT:                       // not supported yet
         break;

      default:
         for (chk = txwa->desktop; chk != NULL; chk = chk->next)
         {
            if ((TXWHANDLE) chk == hwnd)
            {
               wnd = chk;                       // found valid wnd for hwnd
               break;
            }
         }
         break;
   }
   if (win != NULL)                             // window pointer requested
   {
      if (wnd != NULL)                          // valid window base
      {
         if ((*win = wnd->window) == NULL)      // no attached window, invalid
         {
            wnd = NULL;
         }
      }
      else                                      // make returned window invalid
      {
         *win = NULL;
      }
   }
   return( wnd);
}                                               // end 'txwValidateHandle'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Validate handle and calculate clip rectangle from all (parent) client areas
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
TXWINBASE *txwValidateAndClip                   // RET   window base pointer
(
   TXWHANDLE           hwnd,                    // IN    handle to validate
   TXWINDOW          **window,                  // OUT   attached window
   BOOL                border,                  // IN    start clip with border
   TXRECT             *pclip                    // OUT   combined clip rect
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              clip;                    // parent Clip rectangle

   if ((wnd = txwValidateHandle( hwnd, window)) != NULL)
   {
      TXWINBASE       *ancestor = wnd->parent;  // first ancestor level

      //- to be refined, use IsWindowVisible() and return empty clip rectangle ?
      //- to be refined, or  IsWindowShowing()  ?

      if (border)                               // clip including border area ?
      {
         clip = (border) ? wnd->window->border : wnd->window->client;
         if (wnd->window->style & TXWS_CAST_SHADOW)
         {
            clip.bottom += 1;                   // adjust for a shadow area
            clip.right  += 2;                   // below and at right side
         }
      }
      else                                      // use plain client area
      {
         clip = wnd->window->client;
      }
      while (txwValidateHandle((TXWHANDLE) ancestor, &win) != NULL)
      {
         txwIntersectRect( &clip, &win->client, &clip);
         ancestor = ancestor->parent;           // next ancestor level
      }

      //- finally, clip to whole screen, to avoid line-wrapping :-)
      txwIntersectRect( &clip, &txwa->screen, &clip);
      *pclip = clip;
   }
   return( wnd);
}                                               // end 'txwValidateAndClip'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Start enumeration of child windows
/*****************************************************************************/
TXWHENUM txwBeginEnumWindows
(
   TXWHANDLE           parent                   // IN    Parent window handle
)
{
   TXWE_ELEMENT       *rc = NULL;               // function return
   TXWINBASE          *wnd;

   ENTER();

   if ((wnd = txwValidateHandle( parent, NULL)) != NULL)
   {
      if ((rc = TxAlloc( 1, sizeof(TXWE_ELEMENT))) != NULL)
      {
         rc->signature = TXWE_SIGNATURE;
         rc->parent    = wnd;
         rc->nextstart = txwa->last;            // highest Z-order window
      }
   }
   RETURN ((TXWHENUM) rc);
}                                               // end 'txwBeginEnumWindows'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// End an enumeration of child-windows (free resources)
/*****************************************************************************/
BOOL txwEndEnumWindows
(
   TXWHENUM            henum                    // IN    Enumeration handle
)
{
   BOOL                rc = FALSE;              // function return
   TXWE_ELEMENT       *el = (TXWE_ELEMENT *) henum;

   ENTER();

   if ((el) && (el->signature == TXWE_SIGNATURE))
   {
      el->signature = 0;                        // make memory content invalid
      TxFreeMem( el);
      rc = TRUE;
   }
   BRETURN (rc);
}                                               // end 'txwEndEnumWindows'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Retrieve next child's handle in enumeration
/*****************************************************************************/
TXWHANDLE txwGetNextWindow
(
   TXWHENUM            henum                    // IN    Enumeration handle
)
{
   TXWHANDLE           rc = 0;                  // function return
   TXWE_ELEMENT       *el = (TXWE_ELEMENT *) henum;
   TXWINBASE          *wnd;

   ENTER();

   if ((el) && (el->signature == TXWE_SIGNATURE))
   {
      for (wnd = el->nextstart; rc == 0; wnd = wnd->prev)
      {
         if (wnd == NULL)                       // at lowest Z-order
         {
            el->nextstart = txwa->last;         // prepare wrap-arround
            break;                              // and stop the search
         }
         else if (wnd == el->parent)
         {
            continue;                           // skip parent itself
         }
         else if (txwIsChild( (TXWHANDLE) wnd, (TXWHANDLE) el->parent))
         {
            rc = (TXWHANDLE) wnd;               // result found
            el->nextstart = wnd->prev;          // next itteration
            break;                              // and stop the search
         }
      }
   }
   RETURN (rc);
}                                               // end 'txwGetNextWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if window is a direct descendant of parent (or parent itself)
/*****************************************************************************/
BOOL txwIsChild                                 // RET   window is a child
(
   TXWHANDLE           hwnd,                    // IN    window to test
   TXWHANDLE           parent                   // IN    Parent window handle
)
{
   TXWINBASE          *wnd = txwValidateHandle( hwnd,   NULL);
   TXWINBASE          *par = txwValidateHandle( parent, NULL);

   if ((wnd != par) && (txwValidateHandle((TXWHANDLE) wnd, NULL) != NULL))
   {
      wnd = wnd->parent;                        // next ancestor level
   }
   return((wnd == par));
}                                               // end 'txwIsChild'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if window is a descendant of given parent  (or parent itself)
/*****************************************************************************/
BOOL txwIsDescendant                            // RET   window is descendant
(
   TXWHANDLE           hwnd,                    // IN    window to test
   TXWHANDLE           parent                   // IN    Parent window handle
)
{
   TXWINBASE          *wnd = txwValidateHandle( hwnd,   NULL);
   TXWINBASE          *par = txwValidateHandle( parent, NULL);

   while ((wnd != par) && (txwValidateHandle((TXWHANDLE) wnd, NULL) != NULL))
   {
      wnd = wnd->parent;                        // next ancestor level
   }
   return((wnd == par));
}                                               // end 'txwIsDescendant'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if either this window OR a parent is currently MINIMIZED
/*****************************************************************************/
BOOL txwIsMinimized                             // RET   window is descendant
(
   TXWHANDLE           hwnd,                    // IN    window to test
   BOOL                parent                   // IN    check parents only
)                                               //       or check THIS only
{
   BOOL                rc = FALSE;
   TXWINBASE          *wnd = txwValidateHandle( hwnd,   NULL);

   ENTER();

   if (wnd)
   {
      if (parent == FALSE)                      // check only THIS window
      {
         rc = ((wnd->us[TXQWS_FLAGS] & TXFF_MINIMIZED) != 0);
      }
      else
      {
         for ( wnd = txwValidateHandle((TXWHANDLE) wnd->parent, NULL);
              (wnd) && (rc == FALSE);
               wnd = txwValidateHandle((TXWHANDLE) wnd->parent, NULL))
         {
            rc = ((wnd->us[TXQWS_FLAGS] & TXFF_MINIMIZED) != 0);
         }
      }
   }
   BRETURN( rc);
}                                               // end 'txwIsMinimized'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Reset all AutoRadioButtons in a group to the 'unset' condition
/*****************************************************************************/
void txwResetAutoRadioGroup
(
   TXWHANDLE           hwnd,                    // IN    dialog handle or 0
   USHORT              group                    // IN    Group ID or 0
)
{
   TXWHENUM      henum;
   TXWHANDLE     child;
   TXWINDOW     *cwin;

   ENTER();
   TRACES(( "dialog:%8.8lx  group-nr: %hu\n", hwnd, group));

   if ((henum = txwBeginEnumWindows( hwnd)) != 0)
   {
      while ((child = txwGetNextWindow( henum)) != 0)
      {
         cwin = txwWindowData( child);

         if (cwin->class == TXW_BUTTON)
         {
            TXWINBASE *wnd = (TXWINBASE *) child;

            TRACES(( "Button %8.8lx: group:%hu '%s'\n",
                      child, wnd->us[TXQWS_GROUP], cwin->bu.text));

            if ((cwin->style & TXBS_PRIMARYSTYLES) == TXBS_AUTORADIO)
            {
               TRACES(("AutoRadio: %s\n", (*(cwin->bu.checked)) ? "ON" : "OFF"));
               if ((*(cwin->bu.checked)) &&     // only reset the set one(s)
                   (wnd->us[TXQWS_GROUP] == group))
               {
                  TRACES(( "Reset autoradio button %8.8lx: '%s'\n",
                                                   child, cwin->bu.text));
                  *(cwin->bu.checked) = FALSE;
                  txwInvalidateWindow( child, FALSE, FALSE);
               }
            }
         }
      }
      txwEndEnumWindows( henum);
   }
   VRETURN();
}                                               // end 'txwResetAutoRadioGroup'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if keyvalue is a possible ACCELERATOR key
/*****************************************************************************/
BOOL txwIsAccelCandidate                        // RET   key could be ACCEL
(
   ULONG               key                      // IN    key value
)
{
   BOOL                rc = TRUE;               // function return

   ENTER();

   if (((key <  0x0ff) && (key > TXc_Z) &&
        (key != '<')   && (key != '>'))   ||
       ((key <  0x1b0) && (key > 0x18a))  ||
        (key == TXs_TAB)                  ||
        (key == TXk_BACKSPACE)            ||
        (key == TXk_TAB      )            ||
        (key == TXc_ENTER    )            ||
        (key == TXk_ENTER    )            ||
        (key == TXa_BACKQUOTE)            ||
        (key == TXa_SLASH)                ||
        (key == TXc_PGUP)                 ||
        (key == TXk_F11)                  ||    // History popup
        (key == TXk_F12)                  ||
        (key == TXa_F5)                   ||
        (key == TXa_F8)                   ||
        (key == TXa_F9)                   ||
        (key == TXa_F10)                  ||
        (key == TXc_D)                    ||    // History delete
        (key == TXc_E)                    ||    // Clear to end of line
        (key == TXc_K)                    ||    // History add
        (key == TXc_L)                    ||    // Refresh screen
       ((key >= 0x147) && (key <= 0x153)) ||
       ((key >= 0x173) && (key <= 0x177))
      )
   {
      rc = FALSE;
   }
   BRETURN (rc);
}                                               // end 'txwIsAccelCandidate'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return short description for msg-id
/*****************************************************************************/
char *txwMsgDescription
(
   ULONG               msg                      // IN    message-id
)
{
   switch (msg)
   {
      case TXWM_CREATE:          return( "WM_CREATE");
      case TXWM_DESTROY:         return( "WM_DESTROY");
      case TXWM_CHAR:            return( "WM_CHAR");
      case TXWM_COMMAND:         return( "WM_COMMAND");
      case TXWM_CLOSE:           return( "WM_CLOSE");
      case TXWM_QUIT:            return( "WM_QUIT");
      case TXWM_HELP:            return( "WM_HELP");
      case TXWM_BORDER:          return( "WM_BORDER");
      case TXWM_STATUS:          return( "WM_STATUS");
      case TXWM_PAINT:           return( "WM_PAINT");
      case TXWM_SETFOCUS:        return( "WM_SETFOCUS");
      case TXWM_ENABLE:          return( "WM_ENABLE");
      case TXWM_SHOW:            return( "WM_SHOW");
      case TXWM_ACTIVATE:        return( "WM_ACTIVATE");
      case TXWM_SELECTED:        return( "WM_SELECTED");
      case TXWM_CURSORVISIBLE:   return( "WM_CURSORVIS");
      case TXWM_MOVE:            return( "WM_MOVE");
      case TXWM_SIZE:            return( "WM_SIZE");
      case TXWM_CONTROL:         return( "WM_CONTROL");
      case TXWM_INITDLG:         return( "WM_INITDLG");
      case TXWM_MENUSELECT:      return( "WM_MENUSELECT");
      case TXWM_MENUEND:         return( "WM_MENUEND");
      case TXWM_SETFOOTER:       return( "WM_SETFOOTER");
      case TXWM_BUTTONDOWN:      return( "WM_BUTTONDOWN");
      case TXWM_BUTTONUP:        return( "WM_BUTTONUP");
      case TXWM_BUTTONDBLCLK:    return( "WM_BUTTONDBLCLK");
      case TXWM_MOUSEMOVE:       return( "WM_MOUSEMOVE");
      case TXWM_HOOKEVENT:       return( "WM_HOOKEVENT");
      case TXWM_FD_FILTER:       return( "WM_FD_FILTER");
      case TXWM_FD_VALIDATE:     return( "WM_FD_VALIDATE");
      case TXWM_FD_ERROR:        return( "WM_FD_ERROR");
      case TXWM_FD_NEWSPEC:      return( "WM_FD_NEWSPEC");
      case TXWM_FD_NEWPATH:      return( "WM_FD_NEWPATH");
      case TXWM_FD_WILDCARD:     return( "WM_FD_WILDCARD");
      case TXWM_FD_POPULATED:    return( "WM_FD_POPULATED");
      case TXWM_PERFORM_SEARCH:  return( "WM_PERFORM_SEARCH");
      case TXWM_PERFORM_GREP:    return( "WM_PERFORM_GREP");
      default:
         if (msg < TXWM_USER)
         {
            return( "UNKNOWN txw msg");
         }
         else
         {
            return( "USER MESSAGE");
         }
   }
}                                               // end 'txwMsgDescription'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return short description for WM_CHAR key code (mp2)
/*****************************************************************************/
char *txwKeyDescription
(
   ULONG               key                      // IN    message-id
)
{
   switch (key)
   {
      case TXk_F1:                 return( "k_F1");
      case TXk_F2:                 return( "k_F2");
      case TXk_F3:                 return( "k_F3");
      case TXk_F4:                 return( "k_F4");
      case TXk_F5:                 return( "k_F5");
      case TXk_F6:                 return( "k_F6");
      case TXk_F7:                 return( "k_F7");
      case TXk_F8:                 return( "k_F8");
      case TXk_F9:                 return( "k_F9");
      case TXk_F10:                return( "k_F10");
      case TXk_F11:                return( "k_F11");
      case TXk_F12:                return( "k_F12");
      case TXk_ENTER:              return( "k_ENTER");
      case TXk_BACKSPACE:          return( "k_BACKSPACE");
      case TXk_TAB:                return( "k_TAB");
      case TXk_ESCAPE:             return( "k_ESCAPE");
      case TXk_BACKQUOTE:          return( "k_BACKQUOTE");
      case TXk_INSERT:             return( "k_INSERT");
      case TXk_DELETE:             return( "k_DELETE");
      case TXk_UP:                 return( "k_UP");
      case TXk_DOWN:               return( "k_DOWN");
      case TXk_LEFT:               return( "k_LEFT");
      case TXk_RIGHT:              return( "k_RIGHT");
      case TXk_PGUP:               return( "k_PGUP");
      case TXk_PGDN:               return( "k_PGDN");
      case TXk_HOME:               return( "k_HOME");
      case TXk_END:                return( "k_END");
      case TXs_F1:                 return( "s_F1");
      case TXs_F2:                 return( "s_F2");
      case TXs_F3:                 return( "s_F3");
      case TXs_F4:                 return( "s_F4");
      case TXs_F5:                 return( "s_F5");
      case TXs_F6:                 return( "s_F6");
      case TXs_F7:                 return( "s_F7");
      case TXs_F8:                 return( "s_F8");
      case TXs_F9:                 return( "s_F9");
      case TXs_F10:                return( "s_F10");
      case TXs_F11:                return( "s_F11");
      case TXs_F12:                return( "s_F12");
      case TXs_TAB:                return( "s_TAB");
      case TXc_F1:                 return( "c_F1");
      case TXc_F2:                 return( "c_F2");
      case TXc_F3:                 return( "c_F3");
      case TXc_F4:                 return( "c_F4");
      case TXc_F5:                 return( "c_F5");
      case TXc_F6:                 return( "c_F6");
      case TXc_F7:                 return( "c_F7");
      case TXc_F8:                 return( "c_F8");
      case TXc_F9:                 return( "c_F9");
      case TXc_F10:                return( "c_F10");
      case TXc_F11:                return( "c_F11");
      case TXc_F12:                return( "c_F12");
      case TXc_ENTER:              return( "c_ENTER");
      case TXc_BACKSP:             return( "c_BACKSP");
      case TXc_TAB:                return( "c_TAB");
      case TXc_2:                  return( "c_2");
      case TXc_6:                  return( "c_6");
      case TXc_MINUS:              return( "c_MINUS");
      case TXc_RBRACKET:           return( "c_RBRACKET");
      case TXc_BACKSLASH:          return( "c_BACKSLASH");
      case TXc_INSERT:             return( "c_INSERT");
      case TXc_DELETE:             return( "c_DELETE");
      case TXc_UP:                 return( "c_UP");
      case TXc_DOWN:               return( "c_DOWN");
      case TXc_LEFT:               return( "c_LEFT");
      case TXc_RIGHT:              return( "c_RIGHT");
      case TXc_PGUP:               return( "c_PGUP");
      case TXc_PGDN:               return( "c_PGDN");
      case TXc_HOME:               return( "c_HOME");
      case TXc_END:                return( "c_END");
      case TXc_A:                  return( "c_A");
      case TXc_B:                  return( "c_B");
      case TXc_C:                  return( "c_C");
      case TXc_D:                  return( "c_D");
      case TXc_E:                  return( "c_E");
      case TXc_F:                  return( "c_F");
      case TXc_G:                  return( "c_G");
      case TXc_K:                  return( "c_K");
      case TXc_L:                  return( "c_L");
      case TXc_N:                  return( "c_N");
      case TXc_O:                  return( "c_O");
      case TXc_P:                  return( "c_P");
      case TXc_Q:                  return( "c_Q");
      case TXc_R:                  return( "c_R");
      case TXc_S:                  return( "c_S");
      case TXc_T:                  return( "c_T");
      case TXc_U:                  return( "c_U");
      case TXc_V:                  return( "c_V");
      case TXc_W:                  return( "c_W");
      case TXc_X:                  return( "c_X");
      case TXc_Y:                  return( "c_Y");
      case TXc_Z:                  return( "c_Z");
      case TXa_F1:                 return( "a_F1");
      case TXa_F2:                 return( "a_F2");
      case TXa_F3:                 return( "a_F3");
      case TXa_F4:                 return( "a_F4");
      case TXa_F5:                 return( "a_F5");
      case TXa_F6:                 return( "a_F6");
      case TXa_F7:                 return( "a_F7");
      case TXa_F8:                 return( "a_F8");
      case TXa_F9:                 return( "a_F9");
      case TXa_F10:                return( "a_F10");
      case TXa_F11:                return( "a_F11");
      case TXa_F12:                return( "a_F12");
      case TXa_ENTER:              return( "a_ENTER");
      case TXa_BACKSP:             return( "a_BACKSP");
      case TXa_1:                  return( "a_1");
      case TXa_2:                  return( "a_2");
      case TXa_3:                  return( "a_3");
      case TXa_4:                  return( "a_4");
      case TXa_5:                  return( "a_5");
      case TXa_6:                  return( "a_6");
      case TXa_7:                  return( "a_7");
      case TXa_8:                  return( "a_8");
      case TXa_9:                  return( "a_9");
      case TXa_0:                  return( "a_0");
      case TXa_MINUS:              return( "a_MINUS");
      case TXa_EQUAL:              return( "a_EQUAL");
      case TXa_LBRACKET:           return( "a_LBRACKET");
      case TXa_RBRACKET:           return( "a_RBRACKET");
      case TXa_SEMICOLON:          return( "a_SEMICOLON");
      case TXa_QUOTE:              return( "a_QUOTE");
      case TXa_BACKQUOTE:          return( "a_BACKQUOTE");
      case TXa_BACKSLASH:          return( "a_BACKSLASH");
      case TXa_COMMA:              return( "a_COMMA");
      case TXa_DOT:                return( "a_DOT");
      case TXa_SLASH:              return( "a_SLASH");
      case TXa_INSERT:             return( "a_INSERT");
      case TXa_DELETE:             return( "a_DELETE");
      case TXa_UP:                 return( "a_UP");
      case TXa_DOWN:               return( "a_DOWN");
      case TXa_LEFT:               return( "a_LEFT");
      case TXa_RIGHT:              return( "a_RIGHT");
      case TXa_PGUP:               return( "a_PGUP");
      case TXa_PGDN:               return( "a_PGDN");
      case TXa_HOME:               return( "a_HOME");
      case TXa_END:                return( "a_END");
      case TXa_A:                  return( "a_A");
      case TXa_B:                  return( "a_B");
      case TXa_C:                  return( "a_C");
      case TXa_D:                  return( "a_D");
      case TXa_E:                  return( "a_E");
      case TXa_F:                  return( "a_F");
      case TXa_G:                  return( "a_G");
      case TXa_H:                  return( "a_H");
      case TXa_I:                  return( "a_I");
      case TXa_J:                  return( "a_J");
      case TXa_K:                  return( "a_K");
      case TXa_L:                  return( "a_L");
      case TXa_M:                  return( "a_M");
      case TXa_N:                  return( "a_N");
      case TXa_O:                  return( "a_O");
      case TXa_P:                  return( "a_P");
      case TXa_Q:                  return( "a_Q");
      case TXa_R:                  return( "a_R");
      case TXa_S:                  return( "a_S");
      case TXa_T:                  return( "a_T");
      case TXa_U:                  return( "a_U");
      case TXa_V:                  return( "a_V");
      case TXa_W:                  return( "a_W");
      case TXa_X:                  return( "a_X");
      case TXa_Y:                  return( "a_Y");
      case TXa_Z:                  return( "a_Z");
      case TXk_LWIN:               return( "L-Win");
      case TXk_RWIN:               return( "R-Win");
      case TXk_MENU:               return( "Menu");
      case TXk_FIND:               return( "Find");
      default:
         if (isprint(key))
         {
            sprintf( ascii, "ascii: '%c'", (int) key);
         }
         else
         {
            strcpy( ascii, "unknown key");
         }
         return ascii;
   }
}                                               // end 'txwKeyDescription'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return short description for class-id
/*****************************************************************************/
char *txwClassDescription
(
   TXWHANDLE           hwnd                     // IN    window handle or 0
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      sprintf( ascii, "%2d = ", win->class);
      switch (win->class)
      {
         case TXW_FRAME:
            strcat( ascii, (wnd == txwa->desktop) ? "Desktop Frame"
                                                  : "Frame window "); break;
         case TXW_CANVAS:
            strcat( ascii, (win->st.buf == NULL)  ? "Canvas       "
                                                  : "CanvasArtwork"); break;

         case TXW_SBVIEW:            strcat( ascii, "Scrollbuffer "); break;
         case TXW_ENTRYFIELD:        strcat( ascii, "Entryfield   "); break;
         case TXW_TEXTVIEW:          strcat( ascii, "Text viewer  "); break;
         case TXW_STATIC:            strcat( ascii, "Static text  "); break;
         case TXW_STLINE:            strcat( ascii, "Static line  "); break;
         case TXW_BUTTON:            strcat( ascii, "Button       "); break;
         case TXW_LISTBOX:           strcat( ascii, "ListBox      "); break;
         case TXW_HEXEDIT:           strcat( ascii, "Hex Editor   "); break;
         default:                    strcat( ascii, "unknown class"); break;
      }
   }
   else
   {
      strcpy( ascii, "ERROR: TXWHANDLE = 0");
   }
   return ascii;
}                                               // end 'txwClassDescription'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return short description for Window-USHORT ID
/*****************************************************************************/
char *txwWusDescription
(
   USHORT              wus                      // IN    window ushort ID
)
{
   switch (wus)
   {
      case TXQWS_ID:           return( "Window ID   ");
      case TXQWS_X:            return( "X value     ");
      case TXQWS_Y:            return( "Y value     ");
      case TXQWS_FLAGS:        return( "Window Flags");
      default:                 return( "User defined");
   }
}                                               // end 'txwWusDescription'
/*---------------------------------------------------------------------------*/


#define TRTXWS(flag,text)                                                    \
           if (wbs & flag)                                                   \
           {                                                                 \
              TxTraceLeader();                                               \
              TxPrint("%s\n", text);                                         \
           }

/*****************************************************************************/
// Display overview of a style value, with expanded texts for each flag
/*****************************************************************************/
void txwWbsDescription
(
   char               *text,                    // IN    leading text
   ULONG               wbs                      // IN    style value
)
{
   TxTraceLeader();
   TxPrint("%s - style value: %s%8.8lx%s\n", text, CBM, wbs, CNN);

   TRTXWS( TXWS_TITLEBORDER, "TITLEBORDER use a title-border at top");
   TRTXWS( TXWS_SIDEBORDERS, "SIDEBORDERS use side borders  left/right");
   TRTXWS( TXWS_FOOTRBORDER, "FOOTRBORDER use footer-border at bottom");
   TRTXWS( TXWS_BORDERLINES, "BORDERLINES use lines in border");
   TRTXWS( TXWS_BRACKETSIDE, "BRACKETSIDE use [] with no borderlines");
   TRTXWS( TXWS_LEFTJUSTIFY, "LEFTJUSTIFY left-justify title & footer");
   TRTXWS( TXWS_MOVEABLE,    "MOVEABLE    can be moved/sized by user");
   TRTXWS( TXWS_FOCUS_PAINT, "FOCUS_PAINT paint on set-focus (popup)");
   TRTXWS( TXWS_VISIBLE,     "VISIBLE     window could be visible");
   TRTXWS( TXWS_DISABLED,    "DISABLED    window is disabled");
   TRTXWS( TXWS_SAVEBITS,    "SAVEBITS    save underlying cells");
   TRTXWS( TXWS_SYNCPAINT,   "SYNCPAINT   paint on invalidate");
}                                               // end 'txwWbsDescription'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Dump most important window variables to screen/logfile
/*****************************************************************************/
void txwDumpWindows
(
   char               *text                     // IN    identifying string
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXTM                str;
   long                i;

   TxTraceLeader();
   TxPrint("%s 1: %8.8lx  Z:%8.8lx  F:%8.8lx  M:%8.8lx  #:%d\n", text,
      txwa->desktop, txwa->last, txwa->focus, txwa->modality, txwa->windows);

   for (i = 0, wnd = txwa->desktop; wnd; wnd = wnd->next, i++)
   {
      win = wnd->window;
      sprintf( str, "% 2ld", i);
      TRCLAS( str, (TXWHANDLE) wnd);
      TxTraceLeader();
      TxPrint( "win:%8.8lx  proc:%8.8lx ID:% 4hu  oldF:%8.8lx  oldC:%8.8lx\n",
         wnd->window,   wnd->winproc, wnd->us[TXQWS_ID],
         wnd->oldFocus, wnd->oldContent);
      TxTraceLeader();
      TxPrint( "st:%8.8lx  help:%8.8lx  dlgF:%8.8lx  title:%30.30s\n",
         win->style,  win->helpid, win->dlgFocusID,
         (win->title) ? win->title : "-- no title --");
   }
}                                               // end 'txwDumpWindows'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find window on top of Z-order at specified position
/*****************************************************************************/
TXWHANDLE txwTopWindowAtPos
(
   short               row,                     // IN    vert position (Y)
   short               col                      // IN    hor. position (X)
)
{
   TXWHANDLE           rc = 0;
   TXWINBASE          *wnd;
   long                i;

   ENTER();

   for (i = 0, wnd = txwa->desktop; wnd; wnd = wnd->next, i++)
   {
      if (txwInRectangle( row, col, &(wnd->window->border)))
      {
         rc = (TXWHANDLE) wnd;
         TRACES(( "Postion %hu,%hu in hwnd: %8.8lx %s\n",
                row, col, rc, txwClassDescription( rc)));
      }
   }
   RETURN( rc);
}                                               // end 'txwTopWindowAtPos'
/*---------------------------------------------------------------------------*/

/*************************************************************************************************/
// Set mouse capture to specified window (drag)
/*************************************************************************************************/
BOOL txwSetCapture
(
   TXWHANDLE           capture                  // IN    capture window
)
{
   BOOL                rc = FALSE;              // function return

   ENTER();

   if ((capture == TXHWND_NULL) || (txwa->capture == TXHWND_NULL))
   {
      txwa->capture = capture;
      rc = TRUE;
   }
   BRETURN (rc);
}                                               // end 'txwSetCapture'
/*-----------------------------------------------------------------------------------------------*/

/*************************************************************************************************/
// Query handle for current mouse capture window (drag)
/*************************************************************************************************/
TXWHANDLE txwQueryCapture
(
   void
)
{
   ENTER();
   RETURN (txwa->capture);
}                                               // end 'txwQueryCapture'
/*-----------------------------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if a position is within specified rectangle
/*****************************************************************************/
BOOL txwInRectangle                             // RET   position in rectangle
(
   short               row,                     // IN    position row
   short               col,                     // IN    position column
   TXRECT             *rect                     // IN    rectangle
)
{
   BOOL                rc = FALSE;

   if (((rect->top  <= row) && (rect->bottom >= row)) &&
       ((rect->left <= col) && (rect->right  >= col))  )
   {
      rc = TRUE;
   }
   return( rc);
}                                               // end 'txwInRectangle'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Calculate intersection of two rectangles return not-empty
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
BOOL txwIntersectRect                           // RET   non-empty intersection
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2
   TXRECT             *rect                     // OUT   intersection
)
{
   BOOL                rc = FALSE;

   rect->left   = max( rec1->left,   rec2->left );
   rect->top    = max( rec1->top ,   rec2->top  );

   rect->right  = min( rec1->right,  rec2->right);
   rect->bottom = min( rec1->bottom, rec2->bottom);

   if ((rect->top   <= rect->bottom ) &&
       (rect->left  <= rect->right  )  )
   {
      rc = TRUE;
   }
   return( rc);
}                                               // end 'txwIntersectRect'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Determine if first rectangle overlaps second (invalidates posible border)
/*****************************************************************************/
BOOL txwOverlappingRect                         // RET   overlapping rectangle
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2                     // IN    rectangle 2
)
{
   BOOL                rc = FALSE;
   TXRECT              isr;

   if (txwIntersectRect( rec1, rec2, &isr))     // non-empty intersection
   {
      if ((rec1->top    < rec2->top    ) ||
          (rec1->left   < rec2->left   ) ||
          (rec1->right  > rec2->right  ) ||
          (rec1->bottom > rec2->bottom )  )
      {
         rc = TRUE;                             // and rec1 overlaps rec2
      }
   }
   return( rc);
}                                               // end 'txwOverlappingRect'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Calculate union of two rectangles
/*****************************************************************************/
void txwUniteRect
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2
   TXRECT             *rect                     // OUT   union
)
{
   rect->left   = min( rec1->left ,  rec2->left );
   rect->top    = min( rec1->top  ,  rec2->top  );

   rect->right  = max( rec1->right,  rec2->right);
   rect->bottom = max( rec1->bottom, rec2->bottom);
}                                               // end 'txwUniteRect'
/*---------------------------------------------------------------------------*/


//               ÚÄÄÄÄÄÄÄÄÄ¿
//         rec1  ³111111111³                 111 is 1st exclusion
//               ³22222ÚÄÄÄÅÄÄÄÄÄÄ¿          222 is 2nd exclusion
//               ÀÄÄÄÄÄÅÄÄÄÙ      ³ rec2
//                     ÀÄÄÄÄÄÄÄÄÄÄÙ          used for optimized restore
//
/*****************************************************************************/
// Calculate 2 exclusion areas for two overlapping rectangles
/*****************************************************************************/
BOOL txwExclusionRectangles                     // RET   2nd exclusion valid
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2
   TXRECT             *xcl1,                    // OUT   exclusion 1
   TXRECT             *xcl2                     // OUT   exclusion 2
)
{
   BOOL                rc = FALSE;

   ENTER();

   TRECTA("rec1", rec1);
   TRECTA("rec2", rec2);

   *xcl1 = *rec1;                               // start with complete rec1
   *xcl2 = *rec1;                               // as exclusion area 1 & 2

   if (txwOverlappingRect( rec1, rec2))         // optimization possible by
   {                                            // using 2 exclusion rects
      if (rec2->top  >= rec1->top)              // 2nd is 'below' 1st
      {
         xcl1->bottom = rec2->top      -1;
         xcl2->top    = rec2->top;

         if (rec2->left >= rec1->left)          // 2nd is 'right of' 1st
         {
            xcl2->right  = rec2->left  -1;
         }
         else
         {
            xcl2->left   = rec2->right +1;
         }
      }
      else                                      // 2nd is 'above' 1st
      {
         xcl1->top    = rec2->bottom   +1;
         xcl2->bottom = rec2->bottom;

         if (rec2->left >= rec1->left)           // 2nd is 'right of' 1st
         {
            xcl2->right  = rec2->left  -1;
         }
         else
         {
            xcl2->left   = rec2->right +1;
         }
      }
      rc = (xcl2->right >= xcl2->left);         // valid non-empty rectangle

      TRECTA("xcl1", xcl1);
      TRECTA("xcl2", xcl2);
   }
   BRETURN(rc);
}                                               // end 'txwExclusionRectangles'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Test if rectangle is normalized, and make normalized when requested
/*****************************************************************************/
BOOL txwNormalRect                              // RET   input was normalized
(
   BOOL                normalize,               // IN    update rectangle
   TXRECT             *rect                     // INOUT rectangle
)
{
   BOOL                rc = FALSE;
   TXRECT              norm;

   if ((rect->top  <= rect->bottom ) &&
       (rect->left <= rect->right  )  )
   {
      rc = TRUE;
   }
   else if (normalize)
   {
      norm.left   = min( rect->left , rect->right);
      norm.top    = min( rect->top  , rect->bottom);

      norm.right  = max( rect->left , rect->right);
      norm.bottom = max( rect->top  , rect->bottom);

      *rect = norm;
   }
   return( rc);
}                                               // end 'txwNormalRect'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Calculate new rectangle, 2nd top-left as move & 2nd bottom-right as size
/*****************************************************************************/
void txwMoveSizeRect
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2 (reference)
   TXRECT             *rect                     // OUT   moved/sized rect 1
)
{
   rect->left   = rec1->left   + rec2->left;    // top-left is moved only
   rect->top    = rec1->top    + rec2->top;
   rect->right  = rec1->right  + rec2->left + rec2->right;
   rect->bottom = rec1->bottom + rec2->top  + rec2->bottom;
}                                               // end 'txwMoveSizeRect'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Calculate clipped sub-string start-position and length
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
short  txwCalculateStringClip                   // RET   clipped length
(
   short               row,                     // IN    draw dest row
   short               col,                     // IN    draw dest column
   short               len,                     // IN    draw length
   TXRECT             *clip,                    // IN    clip-rectangle
   short              *offset                   // OUT   clipped offset
)
{
   short               length = len;            // length visible string
   short               start  = 0;              // start-offset visible string

   if (clip != NULL)
   {
      if ((clip->top <= row) && (clip->bottom >= row))
      {
         if (clip->left <= col)
         {
            if (clip->right >= col)
            {
               length = min( length, (clip->right - col +1));
            }
            else                                // string starts after clip
            {
               length = 0;
            }
         }
         else                                   // string starts before clip
         {
            start = clip->left - col;
            if (start < length)
            {
               length = min((length - start), (clip->right - clip->left +1));
            }
            else                                // string ends before clip
            {
               length = 0;
            }
         }
      }
      else                                      // string outside clip-lines
      {
         length = 0;
      }
   }
   *offset = start;
   return( length);
}                                               // end 'txwCalculateStringClip'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Calculate visible region(s) for a 1-line string area
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
void txwVisibleLineRegions
(
   TXWINBASE          *wnd,                     // IN    window with string
   TXRECT_ELEM        *clr                      // INOUT list of clip rectangles
)                                               //       on input the full line
{
   TXRECT              overlap;                 // overlapping area on region
   TXRECT_ELEM        *new;                     // new line region
   TXRECT_ELEM        *reg;                     // existing line region
   TXWINBASE          *area;                    // possibly overlapping window
   TXWCLASS            class;

   for (area  = wnd->next;                      // start from next window in
        area != NULL;                           // Z-order possibly overlapping
        area  = area->next)                     // until last one (on top)
   {
      class = area->window->class;              // for performance optimization
      if ((class == TXW_CANVAS) ||              // most dialogs use a canvas
          (class == TXW_FRAME )  )              // help uses a simple frame
      {                                         // rest are likely controls
         for (reg = clr; reg; reg = reg->next)
         {                                      // not completely covered
            if (reg->skip == FALSE)             // already by another area
            {
               if (txwIntersectRect( &(area->window->border),
                                     &(reg->rect), &overlap))
               {
                  if ((overlap.left  > reg->rect.left) &&
                      (overlap.right < reg->rect.right) ) // split region needed
                  {
                     if ((new = TxAlloc( 1, sizeof(TXRECT_ELEM))) != NULL)
                     {
                        new->rect = reg->rect;  // exact copy

                        new->rect.left  = overlap.right +1;
                        reg->rect.right = overlap.left  -1;
                        reg->next       = new;
                     }
                  }
                  else                          // adjust the region
                  {
                     if      (overlap.left  > reg->rect.left)
                     {
                        reg->rect.right = overlap.left  -1; // keep left part
                     }
                     else if (overlap.right < reg->rect.right)
                     {
                        reg->rect.left  = overlap.right +1; // keep right part
                     }
                     else                       // whole region covered by area
                     {
                        reg->skip = TRUE;       // skip this one from now ...
                     }
                  }
               }
            }
         }
      }
   }
   return;
}                                               // end 'txwVisibleLineRegions'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Add a line to History-buffer of specified EntryField window
/*****************************************************************************/
BOOL txwAdd2History                             // RET   string added
(
   TXWHANDLE           hwnd,                    // IN    entryfield window
   char               *line                     // IN    string to be added
)
{
   BOOL                rc  = FALSE;
   TXWINBASE          *wnd = (TXWINBASE *) hwnd;
   TXWINDOW           *win;

   ENTER();
   if (wnd && ((win = wnd->window) != NULL) && (win->class == TXW_ENTRYFIELD))
   {
      TXENTRYFIELD    *dat = &win->ef;
      char            *cur;

      if (dat->history != NULL)
      {
         cur = txwFindHistory( dat->history, line, FALSE);
         if ((cur == NULL) || (strlen( cur) > strlen( line)))
         {
            txwPutHistory( dat->history, 0, line);
         }
         else                                   // exact match already there
         {
            dat->history->current--;            // make sure next history
         }                                      // recall with UP gets it
      }
   }
   BRETURN( rc);
}                                               // end 'txwAdd2History'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Create history-buffer data-structure
/*****************************************************************************/
BOOL txwInitializeHistory
(
   TXHIST             *hb,                      // IN    history structure
   USHORT              entries,                 // IN    entries in history
   USHORT              esize                    // IN    max size of entry
)
{
   BOOL                rc = FALSE;
   USHORT              i;

   ENTER();

   if ((hb->elem   = TxAlloc( entries, sizeof(TXHELEM))) != NULL)
   {
      if ((hb->buf = TxAlloc( entries, esize)) != NULL)
      {
         hb->current   = TXH_NOCURR;
         hb->ascend    = FALSE;
         hb->esize     = esize;
         hb->hsize     = entries;
         for (i = 0; i < entries; i++)
         {
            hb->elem[i].order     = 0;
            hb->elem[i].string    = hb->buf + (i * esize);
            hb->elem[i].string[0] = 0;
            rc = TRUE;
         }
      }
      else
      {
         TxFreeMem( hb->elem);
      }
   }
   BRETURN (rc);
}                                              // end 'txwInitializeHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate history-buffer data-structure
/*****************************************************************************/
void txwTerminateHistory
(
   TXHIST             *hb                       // IN    history structure
)
{
   ENTER();

   TxFreeMem( hb->elem);
   TxFreeMem( hb->buf);
   VRETURN();
}                                              // end 'txwTerminateHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create selist from a History object, optionally filtered by a prefix string
/*****************************************************************************/
ULONG TxSelistHistory                           // RET   result
(
   TXHIST             *hb,                      // IN    history structure
   char               *prefix,                  // IN    prefix select string
   TXSELIST          **list                     // OUT   selection list
)
{
   ULONG               rc = NO_ERROR;           // function return
   ULONG               items = 0;
   ULONG               i;

   ENTER();

   if (hb != NULL)
   {
      for (i = 0; (i < hb->hsize) && (hb->elem[i].order != 0); i++) // count matches
      {
         TRACES(( "Matching:'%s', count:%2lu order:%8.8lx string:'%s'\n",
                   prefix, i, hb->elem[i].order, hb->elem[i].string));

         if ((prefix == NULL) || (*prefix == 0) || // no prefix at all
             (strncasecmp(hb->elem[i].string,
                    prefix, strlen(prefix)) == 0)) // or match on prefix
         {
            items++;
         }
      }
      if (items != 0)
      {
         rc = TxSelCreate( items, items, items, TXS_AS_NOSTATIC, FALSE, NULL, list);
         if (rc == NO_ERROR)
         {
            TXSELIST        *selist = *list;
            ULONG            line;
            char            *listdescr;         // list level description

            selist->astatus = TXS_AS_NOSTATIC      | // all dynamic allocated
                              TXS_LST_DESC_PRESENT | // with list description
                              TXS_LST_DYN_CONTENTS;

            if ((listdescr  = TxAlloc( 1, TXMAXTM)) != NULL)
            {
               sprintf( listdescr, "command (from history) to be used");

               selist->userinfo = (ULONG) listdescr;
            }
            for ( i = 0,             line = 0;
                 (i < hb->hsize) && (line < items) && (rc == NO_ERROR);
                  i++)
            {
               if ((prefix == NULL) || (*prefix == 0) || // no prefix at all
                   (strncasecmp(hb->elem[i].string,
                          prefix, strlen(prefix)) == 0)) // or match on prefix
               {
                  TXS_ITEM  *item = TxAlloc( 1, sizeof(TXS_ITEM));

                  TRACES(( "Start of item# %lu\n", line));
                  if (item != NULL)
                  {
                     if (((item->text = TxAlloc( 1, hb->esize)) != NULL) &&
                         ((item->desc = TxAlloc( 1, TXMAXTM))   != NULL)  )
                     {
                        strcpy(   item->text, hb->elem[i].string);
                        strftime( item->desc, TXMAXTM,
                                 "History item added on %A %d %B %Y at %T",
                                  localtime((time_t *) &(hb->elem[i].order)));
                        item->value  = TXDID_MAX + i +1;    // using index in history
                        item->helpid = TXWH_USE_OWNER_HELP; // from owner-menu-item

                        if (i == hb->current)
                        {
                           selist->selected = line; // make this the default one
                           TRACES(( "Selected item: %lu of %lu\n", line, items));
                        }
                        TRACES(("text: %d '%s'\n", strlen(item->text), item->text));
                        TRACES(("desc: %d '%s'\n", strlen(item->desc), item->desc));
                     }
                     else
                     {
                        rc = TX_ALLOC_ERROR;
                     }
                     selist->items[line] = item;
                     selist->count       = line +1; // number of items in list
                  }
                  else
                  {
                     rc = TX_ALLOC_ERROR;
                  }
                  line++;
               }
            }
            if (rc != NO_ERROR)
            {
               txSelDestroy( list);             // free partial list memory
            }
         }
      }
      else
      {
         rc = TX_FAILED;
      }
   }
   RETURN (rc);
}                                               // end 'TxSelistHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get history-element string from the buffer, optional move current position
/*****************************************************************************/
char  *txwGetHistory
(
   TXHIST             *hb,                      // IN    history structure
   TXHSELECT           select                   // IN    THIS/NEXT/PREV
)
{
   char               *rc = NULL;               // function return
   USHORT              index = 0;

   ENTER();
   TRACES(("get index: %hu select: %u\n", hb->current, select));

   switch (select)
   {
      case TXH_THIS:                            // get the current one
         if (hb->current != TXH_NOCURR)
         {
            index = hb->current;
         }
         break;

      case TXH_NEXT:
         if ((hb->current > 0) && (hb->current != TXH_NOCURR))
         {
            index = hb->current -1;
         }
         break;

      case TXH_PREV:
      default:
         if (hb->current < hb->hsize -1)
         {
            if (hb->elem[hb->current +1].order != 0)
            {
               hb->current++;
            }
            index = hb->current;
         }
         break;
   }
   hb->current = index;
   rc = hb->elem[index].string;
   TRACES(("get str: '%s'\n", rc));
   RETURN (rc);
}                                               // end 'txwGetHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Put new character-string in the history buffer, at lowest order
/*****************************************************************************/
void txwPutHistory
(
   TXHIST             *hb,                      // IN    history structure
   ULONG               order,                   // IN    order (or 0)
   char               *value                    // IN    string value to store
)
{
   USHORT              i;                       // index in history
   USHORT              lru_i = 0;               // least recently used index
   ULONG               lru_o = (ULONG) -1;      // least recently used order

   ENTER();

   if ((hb->elem != NULL) && (strlen(value) != 0))
   {
      for (i = 0; i < hb->hsize; i++)           // find entry with lowest order
      {
         if (hb->elem[i].order < lru_o)
         {
            lru_o = hb->elem[i].order;
            lru_i = i;
         }
      }
      hb->elem[lru_i].order = (order) ? order : (ULONG) time(NULL);
      strcpy( hb->elem[lru_i].string, value);
      TRACES(("put index: %hu, order: %09lu str: '%s'\n", lru_i,
               hb->elem[lru_i].order, hb->elem[lru_i].string));

      txwSortHistory( hb, FALSE);

      hb->current = TXH_NOCURR;
   }
   VRETURN();
}                                              // end 'txwPutHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Delete the current entry in the history buffer, set by Get or Find
/*****************************************************************************/
BOOL txwDelCurrentHistory                       // RET   entry deleted
(
   TXHIST             *hb                       // IN    history structure
)
{
   BOOL                rc = FALSE;
   USHORT              i;

   ENTER();

   if ((hb) && (hb->elem != NULL) && (hb->current != TXH_NOCURR))
   {
      rc = TRUE;
      i  = hb->current;
      TRACES(("del index: %hu, order: %09lu str: '%s'\n",
               i, hb->elem[i].order, hb->elem[i].string));
      hb->elem[i].order     = 0;
      hb->elem[i].string[0] = 0;

      if (hb->current > 0)
      {
         hb->current--;
      }
      //- hb->current = TXH_NOCURR;
      txwSortHistory( hb, FALSE);

   }
   BRETURN(rc);
}                                              // end 'txwDelCurrentHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find a (sub) character-string in the history buffer (new to old)
/*****************************************************************************/
char *txwFindHistory                            // RET   found history string
(
   TXHIST             *hb,                      // IN    history structure
   char               *value,                   // IN    string value to match
   BOOL                use_current              // IN    start from current
)
{
   char               *rc = NULL;               // function return
   USHORT              i;                       // index in history

   ENTER();

   if ((hb->elem != NULL) && (strlen(value) != 0))
   {
      if ((hb->current != TXH_NOCURR) && use_current)
      {
         i = hb->current +1;                    // start at next one
      }
      else
      {
         i = 0;                                 // start at latest entry
      }
      do                                        // find match
      {
         if (strncasecmp( hb->elem[i].string, value, strlen(value)) == 0)
         {
            hb->current = i;
            rc = hb->elem[i].string;
            TRACES(("found index: %hu, order: %09lu str: '%s'\n", i,
                     hb->elem[i].order, hb->elem[i].string));
            break;
         }
      } while (++i < hb->hsize);
   }
   RETURN( rc);
}                                              // end 'txwFindHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Reverse find a (sub) character-string in the history buffer (old to new)
/*****************************************************************************/
char *txwRfndHistory                            // RET   found history string
(
   TXHIST             *hb,                      // IN    history structure
   char               *value,                   // IN    string value to match
   BOOL                use_current              // IN    start from current
)
{
   char               *rc = NULL;               // function return
   USHORT              i;                       // index in history

   ENTER();

   if ((hb->elem != NULL) && (strlen(value) != 0))
   {
      if ((hb->current != TXH_NOCURR) && use_current)
      {
         i = hb->current;
      }
      else
      {
         i = hb->hsize;                         // start at oldest entry
      }
      if (i > 0)
      {
         i--;
      }
      do                                        // find match
      {
         if (strncasecmp( hb->elem[i].string, value, strlen(value)) == 0)
         {
            hb->current = i;
            rc = hb->elem[i].string;
            TRACES(("found index: %hu, order: %09lu str: '%s'\n", i,
                     hb->elem[i].order, hb->elem[i].string));
            break;
         }
      } while (i--);
   }
   RETURN( rc);
}                                              // end 'txwRfndHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Sort history-buffer data-structure
/*****************************************************************************/
void txwSortHistory
(
   TXHIST             *hb,                      // IN    history structure
   BOOL                ascend                   // IN    sort order wanted
)
{
   ENTER();

   if (hb->elem != NULL)
   {
      qsort( hb->elem, (size_t) hb->hsize, sizeof(TXHELEM),
             (ascend) ? txwHistoryAscend : txwHistoryDescend);
   }
   VRETURN();
}                                              // end 'txwSortHistory'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare History order for descending sort
/*****************************************************************************/
static int txwHistoryDescend
(
   const void         *h1,
   const void         *h2
)
{
   int                 rc = 0;
   TXHELEM            *hist1 = (TXHELEM *) h1;
   TXHELEM            *hist2 = (TXHELEM *) h2;
   long                diff = hist2->order - hist1->order;

   if      (diff > 0)
   {
      rc = 1;
   }
   else if (diff < 0)
   {
      rc = -1;
   }
   return (rc);
}                                               // end 'txwHistoryDescend'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare History order for ascending sort
/*****************************************************************************/
static int txwHistoryAscend
(
   const void         *h1,
   const void         *h2
)
{
   int                 rc = 0;
   TXHELEM            *hist1 = (TXHELEM *) h1;
   TXHELEM            *hist2 = (TXHELEM *) h2;
   long                diff = hist1->order - hist2->order;

   if      (diff > 0)
   {
      rc = 1;
   }
   else if (diff < 0)
   {
      rc = -1;
   }
   return (rc);
}                                               // end 'txwHistoryAscend'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Update entryfield from SHORT integer value to HEX/DEC string form
/*****************************************************************************/
void txwDlgUs2Field
(
   TXWHANDLE           hwnd,                    // IN    field handle
   USHORT              value,                   // IN    integer value
   char               *field,                   // INOUT field value
   int                 length,                  // IN    formatting length
   BOOL                hexadec                  // IN    hexadecimal formatted
)
{
   sprintf( field, (hexadec) ? "%*hX" : "%*hu", length, value);
   if (hwnd)
   {
      txwInvalidateWindow( hwnd, FALSE, FALSE);
   }
}                                               // end 'txwDlgUs2Field'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Update entryfield from SHORT integer value to HEX/DEC string form
/*****************************************************************************/
void txwDlgUl2Field
(
   TXWHANDLE           hwnd,                    // IN    field handle
   ULONG               value,                   // IN    integer value
   char               *field,                   // INOUT field value
   int                 length,                  // IN    formatting length
   BOOL                hexadec                  // IN    hexadecimal formatted
)
{
   sprintf( field, (hexadec) ? "%*lX" : "%*lu" , length, value);
   if (hwnd)
   {
      txwInvalidateWindow( hwnd, FALSE, FALSE);
   }
}                                               // end 'txwDlgUl2Field'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Update SHORT integer value from entryfield HEX/DEC string form
/*****************************************************************************/
void txwDlgField2Us
(
   USHORT             *value,                   // OUT   integer value
   char               *field,                   // IN    field value
   BOOL                hexadec                  // IN    hexadecimal formatted
)
{
   TXTS             temp;

   TxStrip( temp, field, ' ', ' ');
   sscanf(  temp, (hexadec) ? "%hX" : "%hu", value);
}                                               // end 'txwDlgField2Us'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Update ULONG integer value from entryfield HEX/DEC string form
/*****************************************************************************/
void txwDlgField2Ul
(
   ULONG              *value,                   // OUT   integer value
   char               *field,                   // IN    field value
   BOOL                hexadec                  // IN    hexadecimal formatted
)
{
   TXTS             temp;

   TxStrip( temp, field, ' ', ' ');
   sscanf(  temp, (hexadec) ? "%lX" : "%lu", value);
}                                               // end 'txwDlgField2Ul'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Test if the scrollbuffer window uses specified color conversion
/*****************************************************************************/
BOOL txwSbColorStyle                            // RET   specified style used
(
   int                 ccstyle                  // IN    color conversion style
)
{
   BOOL                rc = FALSE;              // function return

   ENTER();

   if ((txwa != NULL) && (txwa->sbview != NULL))
   {
      if ((txwa->sbview->window->sb.altcol & ccstyle))
      {
         rc = TRUE;
      }
   }
   BRETURN (rc);
}                                               // end 'txwSbColorBright'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Update footer right-hand text based on current list sorting specification
/*****************************************************************************/
void txwListSort2frhText
(
   TXWHANDLE           hwnd                     // IN    window with attached list
)
{
   TXWINDOW           *win;                     // base window structure
   TXSELIST           *list;                    // selection list structure
   USHORT              sorting;                 // current sort index + indicators
   USHORT              sortspec;                // column, case and descend values
   BOOL                descend;                 // combined asc/desc and reversed

   ENTER();

   if (((win = txwWindowData( hwnd)) != NULL) && (win->class == TXW_LISTBOX))
   {
      list = win->lb.list;

      TRACES(("list:%8.8lx  sortinfo:%8.8lx\n", list, list->sortinfo));
      if (list && list->sortinfo)
      {
         sorting  = list->sortinfo->sort[ TXS_SORT_CURRENT];
         sortspec = list->sortinfo->sort[ sorting & TXS_SORT_IDXMASK];
         descend  = ((sortspec & TXS_SORT_DESCEND) == TXS_SORT_DESCEND);
         TRACES(("descend:%u = %s\n", (int) descend, (descend) ? "TRUE" : "FALSE"));
         if ((sorting & TXS_SORT_REVERSE) == TXS_SORT_REVERSE)
         {
            TRACES(("descend:%u = %s\n", (int) descend, (descend) ? "TRUE" : "FALSE"));
            descend = !descend;                 // reverse the ascend/descend base
         }
         win->frhtext = list->sortinfo->currentShow; // point to available buffer
         strcpy( win->frhtext, list->sortinfo->show[ sorting & TXS_SORT_IDXMASK]);
         if (strlen( win->frhtext) > 1)
         {
            win->frhtext[ strlen(win->frhtext) - 1] = (descend) ?
                                (txwa->codepage == 437) ? '' : '^' :
                                (txwa->codepage == 437) ? '' : 'v';
         }
         TRACES(("sorting:0x%4.4hx sortspec:%4.4hx descend:%s  text:'%s'\n",
                  sorting, sortspec, (descend) ? "YES" : "NO ", win->frhtext));
      }
   }
   VRETURN();
}                                               // end 'txwListSort2frhText'
/*---------------------------------------------------------------------------*/

