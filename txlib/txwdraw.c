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
// TX Windowed text, drawing functions
//
// Author: J. van Wijk
//
// JvW  16-04-2004 Added color scheme logic
// JvW  21-08-2001 Added save/restore window content
// JvW  11-07-2001 Added <Ctrl>-arrow help to SBVIEW status (non-focus state)
// JvW  25-07-1998 Initial version, split off from txwind

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface

#include <txwdraw.h>                            // colorscheme definitions

#if defined (UNIX)
static    ULONG  txwcs_id =  TXWCS_3D_CMDR;     // id for current scheme
TXW_COLORSCHEME *txwcs    = &txwcs_3c;          // current color scheme
#else
static    ULONG  txwcs_id =  TXWCS_3D_GREY;     // id for current scheme
TXW_COLORSCHEME *txwcs    = &txwcs_3g;          // current color scheme
#endif

#if defined (HAVEMOUSE)
   #define TXW_CLOSEBUTTONSTRING "[X]"
#else
   #define TXW_CLOSEBUTTONSTRING "   "
#endif

TXW_COLORSCHEME *txwcschemes[TXWCS_LAST_SCHEME +1] =
{
   NULL,
   &txwcs_st,                                   // TXWCS_STANDARD            1
   &txwcs_sh,                                   // TXWCS_STD_HALF            2
   &txwcs_sf,                                   // TXWCS_STD_FULL            3
   &txwcs_nb,                                   // TXWCS_NO_BLINK            4
   &txwcs_3g,                                   // TXWCS_3D_GREY             5
   &txwcs_3h,                                   // TXWCS_3D_HALF             6
   &txwcs_3f,                                   // TXWCS_3D_FULL             7
   &txwcs_cm,                                   // TXWCS_COMMANDER           8
   &txwcs_3c,                                   // TXWCS_3D_CMDR             9
   &txwcs_ch,                                   // TXWCS_HALF_CMDR          10
   &txwcs_cf,                                   // TXWCS_FULL_CMDR          11
   &txwcs_3w,                                   // TXWCS_3D_WHITE           12
   &txwcs_bw,                                   // TXWCS_BNW_ONLY           13
   &txwcs_bh,                                   // TXWCS_BNW_FULL           14
   &txwcs_bf,                                   // TXWCS_BNW_HALF           15
};

//- wrong double in cp850               x   x   xx  xx xxx x       xxx
//- wrong half   in cp850   x   x   xx          xx              x    x   x
//- wrong full   in cp850                       xx           x           x
static char *txws_dno437 = "³ÚÄ¿³ÀÄÙ´Ã  ³ÉÍ»³ÈÍ¼[][]³ÚÄ¿³ÀÄÙ´Ã  ³ÚÄ¿³ÀÍ¼´Ã  ";
static char *txws_double = "³ÚÄ¿³ÀÄÙ´Ã  ºÉÍ»ºÈÍ¼µÆ[]ºÖÄ·ºÓÄ½´Ã  ³ÚÄ·ºÔÍ¼´Ã  ";
static char *txws_hno437 = "| Ü | ß ||  ÛÛÛÛÛÛÛÛ[][]ÛÛßÛÛÛÜÛÛÛ  | Ü ÛÛÛÛ||  ";
static char *txws_half   = "Þ Ü Ý ß ÝÞ  ÝÛßÛÞÛÜÛÝÞ[]ÛÛßÛÛÛÜÛÛÛ  Þ Ü ÛÞÛÛÝÞ  ";
static char *txws_fno437 = "°°°°°°°°°°  ±±±±±±±±[][]²²²²²²²²²²  ÛÛÛÛÛÛÛÛÛÛ  ";
static char *txws_full   = "°°°°°°°°ÝÞ  ±±±±±±±±ÝÞ[]²²²²²²²²ÝÞ  ÛÛÛÛÛÛÛÛÝÞ  ";
static char *txws_custom = NULL;


/*****************************************************************************/
// Set custom linestyle for border painting
/*****************************************************************************/
BOOL txwSetLinesCustom                          // RET   linestyle string OK
(
   char               *custom                   // IN   custom string (40)
)
{
   BOOL                rc  = FALSE;

   ENTER();

   if (custom && (strlen(custom) == TXLP_SIZE))
   {
      TRACES(("custom lines: '%s'\n", custom));
      txws_custom = custom;
      rc = TRUE;
   }
   BRETURN( rc);
}                                               // end 'txwSetLinesCustom'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query current and set new window ColorScheme by scheme-value or letter
/*****************************************************************************/
ULONG txwColorScheme                            // RET   Current scheme
(
   ULONG               scheme,                  // IN    Scheme selector value
   TXTT                name                     // OUT   Scheme name or NULL
)
{
   ULONG               rc = txwcs_id;

   ENTER();

   switch (scheme)
   {
      case TXWCS_QUERY_ONLY:                    // query only
         break;

      case TXWCS_NEXT_SCHEME:
         if ((++txwcs_id) > TXWCS_LAST_SCHEME)
         {
            txwcs_id  = TXWCS_FIRST_SCHEME;
         }
         break;

      case TXWCS_PREV_SCHEME:
         if ((--txwcs_id) < TXWCS_FIRST_SCHEME)
         {
            txwcs_id  = TXWCS_LAST_SCHEME;
         }
         break;

      default:                                  // explicit scheme selection
         if ((scheme >= TXWCS_FIRST_SCHEME) &&
             (scheme <= TXWCS_LAST_SCHEME))
         {
            txwcs_id  = scheme;
         }
         else
         {
            switch (toupper(scheme))
            {
               case 'N': txwcs_id = TXWCS_NO_BLINK;    break;
               case 'C': txwcs_id = TXWCS_COMMANDER;   break;
               case 'M': txwcs_id = TXWCS_3D_CMDR;     break;
               case 'H': txwcs_id = TXWCS_HALF_CMDR;   break;
               case 'F': txwcs_id = TXWCS_FULL_CMDR;   break;
               case 'D': txwcs_id = TXWCS_STANDARD;    break;
               case 'E': txwcs_id = TXWCS_STD_HALF;    break;
               case 'S': txwcs_id = TXWCS_STD_FULL;    break;
               case 'W': txwcs_id = TXWCS_3D_WHITE;    break;
               case 'B': txwcs_id = TXWCS_BNW_ONLY;    break;
               case 'L': txwcs_id = TXWCS_BNW_HALF;    break;
               case 'P': txwcs_id = TXWCS_BNW_FULL;    break;
               case 'X': txwcs_id = TXWCS_3D_HALF;     break;
               case 'Y': txwcs_id = TXWCS_3D_FULL;     break;
               default:  txwcs_id = TXWCS_3D_GREY;     break;
            }
         }
         break;
   }
   if (txwcs_id != rc)                          // change scheme ?
   {
      txwcs = txwcschemes[txwcs_id];
      if (txwa->sbview)                         // set default SB colors too
      {
         txwa->sbview->window->sb.altcol = txwcs->sbcolors;
      }
   }
   if (name != NULL)
   {
      strcpy( name, txwcs->name);
   }
   RETURN (rc);
}                                               // end 'txwColorScheme'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create selist from the available Color schemes
/*****************************************************************************/
ULONG TxSelistColorSchemes                      // RET   result
(
   TXSELIST          **list                     // OUT   selection list
)
{
   ULONG               rc    = NO_ERROR;        // function return
   ULONG               items = TXWCS_LAST_SCHEME - TXWCS_FIRST_SCHEME +1;

   ENTER();

   rc = TxSelCreate( items, items, items, TXS_AS_NOSTATIC, FALSE, NULL, list);
   if (rc == NO_ERROR)
   {
      TXSELIST        *selist = *list;
      ULONG            line;
      char            *listdescr;               // list level description

      selist->astatus = TXS_AS_NOSTATIC      |  // all dynamic allocated
                        TXS_LST_DESC_PRESENT |  // with list description
                        TXS_LST_DYN_CONTENTS;

      if ((listdescr  = TxAlloc( 1, TXMAXTM)) != NULL)
      {
         sprintf( listdescr, "colorscheme to be used");

         selist->userinfo = (ULONG) listdescr;
      }
      selist->selected = txwcs_id -1;           // current scheme
      for (line = 0; (line < items) && (rc == NO_ERROR); line++)
      {
         TXS_ITEM  *item = TxAlloc( 1, sizeof(TXS_ITEM));

         if (item != NULL)
         {
            if (((item->text = TxAlloc( 1, TXMAXTT)) != NULL) &&
                ((item->desc = TxAlloc( 1, TXMAXTM)) != NULL)  )
            {
               strcpy( item->text, txwcschemes[line+1]->name);
               strcpy( item->desc, txwcschemes[line+1]->desc);
               item->value  = TXDID_MAX + line +1; // scheme id code
               item->helpid = TXWH_USE_OWNER_HELP; // from owner-menu-item
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
            selist->items[line] = item;
            selist->count       = line +1;      // number of items in list

         }
         else
         {
            rc = TX_ALLOC_ERROR;
         }
      }
      if (rc != NO_ERROR)
      {
         txSelDestroy( list);                   // free partial list memory
      }
   }
   RETURN (rc);
}                                               // end 'TxSelistColorSchemes'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Save window contents to be restored at destroy time (incl borders)
/*****************************************************************************/
ULONG txwSaveWindowContent
(
   TXWINBASE          *wnd                      // IN    window
)
{
   ULONG               rc  = NO_ERROR;
   TXWINDOW           *win = wnd->window;
   TXRECT              area;
   short               sx;
   short               sy;
   short               i;

   ENTER();

   if (win->style & TXWS_MOVEABLE)              // move & resize allowed
   {                                            // save whole screen area
      area = txwa->screen;
   }
   else                                         // just save window area
   {
      area = win->border;
   }
   sx = area.right  - area.left  +1;
   sy = area.bottom - area.top   +1;

   TxFreeMem( wnd->oldContent);                 // free previous, if any
   if ((wnd->oldContent = TxAlloc( (sx * sy), sizeof(TXCELL))) != NULL)
   {
      TRACES(("Saving %hd lines of %hd cells to %8.8lx\n", sy, sx, wnd->oldContent));
      TRECTA("area", (&area));
      for (i = 0; i < sy; i++)
      {
         txwScrReadCellString( area.top + i, area.left,
                               &(wnd->oldContent[i * sx]), sx);
      }
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   RETURN( rc);
}                                               // end 'txwSaveWindowContent'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Restore window contents, incl borders or clipped within 1 or 2 area's
/*****************************************************************************/
ULONG txwRestoreWindowContent
(
   TXWINBASE          *wnd,                     // IN    window
   TXRECT             *clip1,                   // IN    partial restore area 1
   TXRECT             *clip2                    // IN    partial restore area 2
)
{
   ULONG               rc  = NO_ERROR;
   TXWINDOW           *win = wnd->window;
   TXRECT              area;
   short               sx;
   short               sy;
   short               i;

   ENTER();

   TXSCREEN_BEGIN_UPDATE();
   if (wnd->oldContent != NULL)
   {
      if (win->style & TXWS_MOVEABLE)           // move & resize allowed
      {                                         // whole screen is saved
         area = txwa->screen;
      }
      else                                      // window area saved only
      {
         area = win->border;
      }
      sx = area.right  - area.left  +1;
      sy = area.bottom - area.top   +1;

      TRACES(("Restoring (at most) %hd lines of %hd cells\n", sy, sx));
      TRECTA("area ", (&area));
      TRECTA("clip1", clip1);
      TRECTA("clip2", clip2);

      for (i = 0; i < sy; i++)
      {
         if ((clip1 != NULL) ||                 // 1st area defined
             (clip2 == NULL))                   // or none (whole window)
         {
            txwScrDrawCellString( area.top + i, area.left, clip1,
                                  &(wnd->oldContent[i * sx]), sx, TXSB_COLOR_NORMAL);
         }
         if  (clip2 != NULL)                    // 2nd area defined
         {
            txwScrDrawCellString( area.top + i, area.left, clip2,
                                  &(wnd->oldContent[i * sx]), sx, TXSB_COLOR_NORMAL);
         }
      }
   }
   TXSCREEN_ENDOF_UPDATE();
   RETURN( rc);
}                                               // end 'txwRestoreWindowContent'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Default paint window border and focus indication, for any window-class
/*****************************************************************************/
void txwPaintBorder
(
   TXWINBASE          *wnd,                     // IN    current window
   BOOL                focus                    // IN    focus indicator
)
{
   TXWINDOW           *win;
   TXRECT              clip;                    // parent Clip rectangle
   TXWHANDLE           hwnd = (TXWHANDLE) wnd;

   ENTER();

   TXSCREEN_BEGIN_UPDATE();
   if ((txwValidateAndClip( hwnd, &win, TRUE, &clip)) != NULL)
   {
      ULONG            style;
      TXRECT           fill;
      short            pos;
      TXCELL           bc;
      short            sx;
      short            sy;
      short            cy;
      char            *lc;                      // line character
      char             borderclear[TXLP_SIZE];
      BOOL             act = (focus || (wnd->us[TXQWS_FLAGS] & TXFF_ACTIVE));
      BYTE             ba;
      int              csi_offset = 0;          // colorscheme offset to Win...

      TRACES(("wnd:%8.8lx, focus: %s  Minimized: %s style:%8.8lx\n", wnd,
             ( focus) ? "Yes" : "No",
             ( wnd->us[TXQWS_FLAGS] & TXFF_MINIMIZED) ? "ON " : "OFF", win->style));
      TRECTA("clip  ", (&clip));
      TRECTA("border", (&win->border));
      style = win->style;
      bc    = win->borderclear;
      ba    = win->borderclear.at;              // remember original attribute
      sx    = win->border.right  - win->border.left  +1;
      sy    = win->border.bottom - win->border.top   +1;
      cy    = win->client.bottom - win->client.top   +1;

      if (ba != cSchemeColor)                   // user specific color scheme ?
      {
         csi_offset = ba - cWinBorder_top;
      }
      else                                      // use class-specific defaults
      {
         switch (win->class)                    // class-specific border colors
         {
            case TXW_FRAME:                     // Regular windows borders,
               if (wnd == txwa->desktop)        // except the Desktop itself
               {
                               csi_offset = cDskBorder_top  - cWinBorder_top; break;
               }
            case TXW_CANVAS:                                                  break;
            case TXW_LISTBOX:
               if (win->style & TXLS_DROP_VALUE) // spin-value is a Dlg-control
               {
                               csi_offset = cDlgBorder_top  - cWinBorder_top; break;
               }
            case TXW_STATIC:                                                  break;

            case TXW_SBVIEW:   csi_offset = cSbvBorder_top  - cWinBorder_top; break;
            case TXW_TEXTVIEW: csi_offset = cViewBorder_top - cWinBorder_top; break;
            case TXW_BUTTON:
               if ((win->style & TXBS_PRIMARYSTYLES) == TXBS_PUSHBUTTON)
               {
                               csi_offset = cPushBorder_top - cWinBorder_top; break;
               }
            default:           csi_offset = cDlgBorder_top  - cWinBorder_top; break;
         }
      }

      memset( borderclear, win->borderclear.ch, TXLP_SIZE);
      if (style & TXWS_BRACKETSIDE)             // use side bracket lines
      {                                         // on active-dialog and focus
         borderclear[TXLP_FOCUS  + TXLP_L1L] = '[';
         borderclear[TXLP_FOCUS  + TXLP_R1L] = ']';
         borderclear[TXLP_ACTIVE + TXLP_L1L] = '[';
         borderclear[TXLP_ACTIVE + TXLP_R1L] = ']';
      }
      lc = borderclear;

      if (style & TXWS_BORDERLINES)             // change to linestyle ...
      {
         switch (txwcs->linestyle)              // check scheme linestyle
         {
            case TXW_CS_CUST: lc = (txws_custom   != NULL) ? txws_custom : borderclear; break;
            case TXW_CS_HALF: lc = (txwa->codepage == 437) ? txws_half   : txws_hno437; break;
            case TXW_CS_FULL: lc = (txwa->codepage == 437) ? txws_full   : txws_fno437; break;
            default:          lc = (txwa->codepage == 437) ? txws_double : txws_dno437;
               break;
         }
         if ((txwcs->linestyle   != TXW_CS_3D_LINES)   || //- leave 3D lines single
             ((sy == 1) || (style & TXWS_BRACKETSIDE)) || //- except on one-liners
             (win->class         == TXW_BUTTON))          //- and any buttons
         {
            if (focus)                                     lc += TXLP_FOCUS;
            else if (wnd->us[TXQWS_FLAGS] & TXFF_ACTIVE)   lc += TXLP_ACTIVE;
            else if (wnd->us[TXQWS_FLAGS] & TXFF_SELECTED) lc += TXLP_SELECTED;
         }
      }

      TRACES(("using line-chars: '%*.*s'\n", TXLP_POS, TXLP_POS, lc));

      if (style & TXWS_FOOTRBORDER)
      {
         fill        = win->border;
         fill.top    = fill.bottom;
         txwIntersectRect( &clip, &fill, &fill);

         if (txwIsRectShowing( hwnd, &fill))    // whole footer visible ?
         {
            bc.ch = lc[TXLP_BOT];
            bc.at = TxwSC( cWinBorder_bot + csi_offset);
            if ((style & TXWS_TF_TEXTONLY) == 0) // unless lines suppressed
            {
               txwScrFillRectangle( &fill, bc);
            }
            if (win->footer && strlen(win->footer))
            {
               TRACES(( "Window footer: '%s'\n", win->footer));
               if (((style & TXWS_LEFTJUSTIFY) || (strlen(win->footer) > sx -2)) ||
                    (win->frhtext && strlen(win->frhtext)))
               {
                  pos = win->border.left;
                  if (style & TXWS_SIDEBORDERS)
                  {
                     pos++;
                  }
               }
               else
               {
                  pos = win->border.left + ((sx - strlen(win->footer)) / 2);
               }
               if ( (style & TXWS_BORDERLINES) && (pos > 1) &&
                   ((style & TXWS_TF_TEXTONLY) == 0)) // unless lines suppressed
               {
                  bc.ch = lc[TXLP_TXL];
                  txwScrDrawCellString( win->border.bottom,
                                        pos -1,
                                        &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);

                  bc.ch = lc[TXLP_TXR];
                  txwScrDrawCellString( win->border.bottom,
                                        pos +strlen(win->footer),
                                        &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);
               }
               TRACES(("Write %u length footer to pos %hd\n", strlen(win->footer), pos));
               txwScrDrawCharStrCol( win->border.bottom, pos, &fill, win->footer, ((act) ?
                              TxwAC( cWinFooterFocus + csi_offset,   win->footerfocus) :
                              TxwAC( cWinFooterStand + csi_offset,   win->footercolor)));
            }
            if (win->frhtext && strlen(win->frhtext))
            {
               TRACES(( "Window frhtext: '%s'\n", win->frhtext));
               pos = win->border.right - strlen(win->frhtext) -1;
               if ( (style & TXWS_BORDERLINES) && (pos > 1) &&
                   ((style & TXWS_TF_TEXTONLY) == 0)) // unless lines suppressed
               {
                  bc.ch = lc[TXLP_TXL];
                  txwScrDrawCellString( win->border.bottom,
                                        pos -1,
                                        &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);

                  bc.ch = lc[TXLP_TXR];
                  txwScrDrawCellString( win->border.bottom,
                                        pos +strlen(win->frhtext),
                                        &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);
               }
               TRACES(("Write %u length frhtext to pos %hd\n", strlen(win->frhtext), pos));
               txwScrDrawCharStrCol( win->border.bottom, pos, &fill, win->frhtext, ((act) ?
                              TxwAC( cWinFooterFocus + csi_offset,   win->footerfocus) :
                              TxwAC( cWinFooterStand + csi_offset,   win->footercolor)));
            }
         }
      }
      if (style & TXWS_TITLEBORDER)
      {
         fill        = win->border;
         fill.bottom = fill.top;
         txwIntersectRect( &clip, &fill, &fill);

         if (txwIsRectShowing( hwnd, &fill))    // whole titlebar visible ?
         {
            bc.ch = lc[TXLP_TOP];
            bc.at = TxwSC( cWinBorder_top + csi_offset);
            if ((style & TXWS_TF_TEXTONLY) == 0) // unless lines suppressed
            {
               txwScrFillRectangle( &fill, bc);
            }
            if (win->title && strlen(win->title))
            {
               TRACES(( "Window title: '%s'\n", win->title));
               if ((style & TXWS_LEFTJUSTIFY) || (strlen(win->title) > sx -2))
               {
                  pos = win->border.left;
                  if (style & TXWS_SIDEBORDERS)
                  {
                     pos++;
                  }
               }
               else
               {
                  pos = win->border.left + ((sx - strlen(win->title)) / 2);
               }
               if ( (style & TXWS_BORDERLINES) && (pos > 1) &&
                   ((style & TXWS_TF_TEXTONLY) == 0)) // unless lines suppressed
               {
                  bc.ch = lc[TXLP_TXL];
                  txwScrDrawCellString( win->border.top,
                                        pos -1,
                                        &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);

                  bc.ch = lc[TXLP_TXR];
                  txwScrDrawCellString( win->border.top,
                                        pos +strlen(win->title),
                                        &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);
               }
               txwScrDrawCharStrCol( win->border.top, pos, &fill,  win->title, ((act) ?
                              TxwAC( cWinTitleFocus + csi_offset,  win->titlefocus) :
                              TxwAC( cWinTitleStand + csi_offset,  win->titlecolor)));
            }
            if (wnd->us[TXQWS_FLAGS] & TXFF_MINIMIZED)
            {
               TRACES(("Add 'F12' to titlebar of wnd:%8.8lx\n", wnd))
               txwScrDrawCharStrCol( win->border.top,
                                     win->border.right -3, &fill, "F12",
                              TxwAC( cWinFooterFocus + csi_offset, win->footerfocus));
            }
            #if defined (HAVEMOUSE)
            else if (((win->class == TXW_FRAME )          ||
                      (win->class == TXW_CANVAS)          ||  //- frame or canvas,
                      (win->class == TXW_TEXTVIEW))       &&  //- or a textview
                     ((win->style  & TXCS_CLOSE_BUTTON))  &&  //- with close-button
                   (txwIsDescendant( hwnd, txwa->modality)))  //- and now active
            {
               TRACES(("Add close-button to titlebar of wnd:%8.8lx\n", wnd))
               txwScrDrawCharStrCol( win->border.top,
                                     win->border.right -3, &fill, TXW_CLOSEBUTTONSTRING,
                              TxwAC( cWinFooterFocus + csi_offset, win->footerfocus));
            }
            #endif
         }
      }
      if (style & TXWS_SIDEBORDERS)
      {
         short         upscroll = TXW_INVALID_COORD; // linenr up   arrow
         short         dnscroll = TXW_INVALID_COORD; // linenr down arrow
         TXSELIST     *list;

         fill        = win->border;
         fill.right  = fill.left;
         if ((sy == 1) || (style & TXWS_BRACKETSIDE))
         {
            fill.top    = win->client.top;      // next to client area only
            fill.bottom = win->client.bottom;
            txwIntersectRect( &clip, &fill, &fill);

            if (txwIsRectShowing( hwnd, &fill)) // whole titlebar visible ?
            {
               bc.ch = lc[TXLP_L1L];            // lft & rgt use same color!
               bc.at = TxwSC( cWinBorder_rgt + csi_offset);
               txwScrFillRectangle( &fill, bc);
            }
         }
         else                                   // multi line window, left
         {
            txwIntersectRect( &clip, &fill, &fill);

            if (txwIsRectShowing( hwnd, &fill)) // whole titlebar visible ?
            {
               bc.ch = lc[TXLP_LFT];
               bc.at = TxwSC( cWinBorder_lft + csi_offset);
               txwScrFillRectangle( &fill, bc);

               bc.ch = lc[TXLP_TLC];
               bc.at = TxwSC( cWinBorder_tlc + csi_offset);
               txwScrDrawCellString( win->border.top,
                                     win->border.left,
                                     &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);
               bc.ch = lc[TXLP_BLC];
               bc.at = TxwSC( cWinBorder_blc + csi_offset);
               txwScrDrawCellString( win->border.bottom,
                                     win->border.left,
                                     &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);
            }
         }

         fill        = win->border;
         fill.left   = fill.right;
         if ((sy == 1) || (style & TXWS_BRACKETSIDE))
         {
            fill.top    = win->client.top;      // next to client area only
            fill.bottom = win->client.bottom;
            txwIntersectRect( &clip, &fill, &fill);

            if (txwIsRectShowing( hwnd, &fill)) // whole titlebar visible ?
            {
               bc.ch = lc[TXLP_R1L];
               bc.at = TxwSC( cWinBorder_rgt + csi_offset);
               txwScrFillRectangle( &fill, bc);
            }
         }
         else                                   // multi line window, right
         {
            txwIntersectRect( &clip, &fill, &fill);

            if (txwIsRectShowing( hwnd, &fill)) // whole titlebar visible ?
            {
               bc.ch = lc[TXLP_RGT];
               bc.at = TxwSC( cWinBorder_rgt + csi_offset);
               txwScrFillRectangle( &fill, bc);

               bc.ch = lc[TXLP_TRC];
               bc.at = TxwSC( cWinBorder_trc + csi_offset);
               txwScrDrawCellString( win->border.top,    win->border.right,
                                     &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);
               bc.ch = lc[TXLP_BRC];
               bc.at = TxwSC( cWinBorder_brc + csi_offset);
               txwScrDrawCellString( win->border.bottom, win->border.right,
                                     &clip, &bc, (short ) 1, TXSB_COLOR_NORMAL);
               switch (win->class)
               {
                  case TXW_TEXTVIEW:
                     //- to be refined, does not work on most text view
                     //- windows because the frame has the border, not
                     //- the TEXTVIEW control

                     if (win->tv.topline > 0)
                     {
                        upscroll = win->border.top +1;
                     }
                     if (win->tv.topline < win->tv.maxtop)
                     {
                        dnscroll = win->border.bottom -1;
                     }
                     break;

                  case TXW_LISTBOX:
                     if ((list = win->lb.list) != NULL)
                     {
                        if (list->top > 0)
                        {
                           upscroll = win->border.top +1;
                        }
                        if (list->top + list->vsize < list->count)
                        {
                           dnscroll = win->border.bottom -1;
                        }
                     }
                     break;

                  default:
                     //- no scroll indicators in others, specifically NOT in the
                     //- SB because of frequent repaint and TRACE problems
                     break;
               }
               bc.at = TxwSC( cWinBorder_rgt + csi_offset);
               if (upscroll != TXW_INVALID_COORD)
               {
                  bc.ch = (txwa->codepage == 437) ? '' : '-';
                  txwScrDrawCellString( upscroll, win->border.right, &clip, &bc,
                                                 (short ) 1, TXSB_COLOR_NORMAL);
                  #if defined (HAVEMOUSE)
                  if (sy > 4)
                  {
                     bc.ch = (txwa->codepage == 437) ? '' : 'U';
                     txwScrDrawCellString( upscroll +1, win->border.right, &clip,
                                            &bc, (short ) 1, TXSB_COLOR_NORMAL);
                  }
                  #endif
               }
               if (dnscroll != TXW_INVALID_COORD)
               {
                  bc.ch = (txwa->codepage == 437) ? '' : '+';
                  txwScrDrawCellString( dnscroll, win->border.right, &clip, &bc,
                                                 (short ) 1, TXSB_COLOR_NORMAL);
                  #if defined (HAVEMOUSE)
                  if (sy > 4)
                  {
                     bc.ch = (txwa->codepage == 437) ? '' : 'D';
                     txwScrDrawCellString( dnscroll -1, win->border.right, &clip,
                                            &bc, (short ) 1, TXSB_COLOR_NORMAL);
                  }
                  #endif
               }
            }
         }
      }
      if ( (style & TXWS_CAST_SHADOW) &&        // add shadows to lower-right
          ((style & TXWS_TITLEBORDER) ||        // unless minimized non-title
           (sy > 1)))
      {
         TRACES(( "vertical size for MINIMIZED test: %hd\n", sy));
         fill         = win->border;
         fill.left    = fill.right;             // right border area

         fill.top    += 1;                      // adjust to right-side shadow
         fill.bottom += 1;
         fill.left   += 1;
         fill.right  += 2;

         txwIntersectRect( &clip, &fill, &fill);
         TRECTA("fill right-shadow ", (&fill));
         txwScrShadowRectangle( &fill);

         fill         = win->border;
         fill.top     = fill.bottom;            // bottom border area

         fill.top    += 1;                      // adjust to bottom-side shadow
         fill.bottom += 1;
         fill.left   += 2;
         fill.right  += 2;

         txwIntersectRect( &clip, &fill, &fill);
         TRECTA("fill bottom-shadow", (&fill));
         txwScrShadowRectangle( &fill);
      }
   }
   TXSCREEN_ENDOF_UPDATE();
   VRETURN();
}                                               // end 'txwPaintBorder'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Default paint border-shadow for window, no tracing (for SbView update)
/*****************************************************************************/
void txwPaintShadow
(
   TXWINBASE          *wnd                      // IN    current window
)
{
   TXWINDOW           *win;
   TXRECT              fill;
   TXRECT              clip;                    // parent Clip rectangle

   //- note: keep code in sync with paintBorder above!

   TXSCREEN_BEGIN_UPDATE();
   if ((txwValidateAndClip((TXWHANDLE) wnd, &win, TRUE, &clip)) != NULL)
   {
      short            sy    = win->client.bottom - win->client.top   +1;
      ULONG            style = win->style;

      if ( (style & TXWS_CAST_SHADOW) &&        // add shadows to lower-right
          ((style & TXWS_TITLEBORDER) ||        // unless minimized non-title
           (sy > 1)))
      {
         fill         = win->border;
         fill.left    = fill.right;             // right border area

         fill.top    += 1;                      // adjust to right-side shadow
         fill.bottom += 1;
         fill.left   += 1;
         fill.right  += 2;

         txwIntersectRect( &clip, &fill, &fill);
         txwScrShadowRectangle( &fill);

         fill         = win->border;
         fill.top     = fill.bottom;            // bottom border area

         fill.top    += 1;                      // adjust to bottom-side shadow
         fill.bottom += 1;
         fill.left   += 2;
         fill.right  += 2;

         txwIntersectRect( &clip, &fill, &fill);
         txwScrShadowRectangle( &fill);
      }
   }
   TXSCREEN_ENDOF_UPDATE();
   return;
}                                               // end 'txwPaintShadow'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Default paint, for standard window-classes; draw client window contents
/*****************************************************************************/
ULONG txwPaintWindow
(
   TXWINBASE          *wnd,                     // IN    current window
   BOOL                border                   // IN    paint border too
)
{
   ULONG               rc   = NO_ERROR;
   TXWHANDLE           hwnd = (TXWHANDLE) wnd;
   TXWINDOW           *win  = wnd->window;
   short               sx = win->client.right  - win->client.left  +1;
   short               sy = win->client.bottom - win->client.top   +1;
   short               nr;
   short               sl;                      // string length value
   TX1K                string;
   char               *line;
   char              **s;
   ULONG               li;                      // list index
   TXSELIST           *list;
   BOOL                footer_update = FALSE;
   BYTE                at;                      // default color attribute
   TXCELL              fill;
   int                 csi_offset = 0;          // colorscheme offset Lists
   TXTM                word;                    // small word buffer

   ENTER();

   TXSCREEN_BEGIN_UPDATE();
   memset( string, ' ', TXMAX1K); string[min(sx,TXMAX1K-1)] = 0;

   TRACES(("wnd:%8.8lx, sx:%4u, sy:%4u\n", wnd, sx, sy));
   TRCLAS( "Paint", hwnd);
   TRECTA("client", (&win->client));

   if ((sx >= 0) && ((sy + 2) > 0))             // no negative sx, sy may be -1
   {
      switch (win->class)
      {
         case TXW_FRAME:                        // border only
            break;

         case TXW_CANVAS:
            if (win->st.buf == NULL)            // no static-text attached
            {                                   // clear client window
               fill    = win->clientclear;
               fill.at = TxwAC( cWinClientClr, win->clientclear.at);
               txwFillClientWindow( hwnd, fill);
               if (((TXWHANDLE) wnd) == txwa->mainmenu) // canvas for main menu
               {
                  TXS_MENUBAR *mbar = txwQueryWindowPtr( hwnd, TXQWP_USER);

                  at = TxwAC( cWinClientClr, win->clientclear.at);
                  if ((mbar) && ((line = strchr( mbar->appdesc, '.')) != NULL))
                  {
                     sprintf( word, "%-6.6s %s", line -2, TXW_CLOSEBUTTONSTRING);
                     txwStringAt( 0, sx -10, word, at);
                  }
                  else                          // just the close button
                  {
                     txwStringAt( 0, sx -3, TXW_CLOSEBUTTONSTRING, at);
                  }
               }
               break;
            }                                   // CANVAS with text, as static
         case TXW_STATIC:                       // regular static class
            at = TxwAC( cWinClientClr, win->clientclear.at);
            if (win->st.buf != NULL)
            {
               for ( nr = 0,   s = &(win->st.buf[0]);
                     nr < sy;
                     nr++)
               {
                  if ((*s != NULL) && (strlen(*s) > 0))
                  {
                     line = *s;
                     txwStringAt( nr, 0, line, at);
                     if ((sl = txSlen( line)) < sx) // clear to end-of-window
                     {
                        txwStringAt( nr, sl, string + sl, at);
                     }
                  }
                  else
                  {
                     txwStringAt( nr, 0, string, at);
                  }
                  if (*s != NULL)               // is this the last one ?
                  {
                     s++;                       // to next string, if present
                  }
               }
            }
            break;

         case TXW_STLINE:                       // static with single line
            line = win->sl.buf;
            at   = TxwAC( cLineTextStand, win->clientclear.at);
            if ((line != NULL) && (strlen(line) > 0))
            {
               TRACES(( "Painting: '%s'\n", line));
               txwStringAt( 0, 0, line, at);
               if ((sl = txSlen( line)) < sx)   // clear to end-of-window
               {
                  txwStringAt( 0, sl, string + sl, at);
               }
            }
            else                                // clear the line area
            {
               txwStringAt( 0, 0, string, at);
            }
            break;

         case TXW_TEXTVIEW:
            if (win->tv.buf != NULL)
            {
               short   markNr = TXW_INVALID;

               if (win->tv.markSize != 0)      // we have a mark
               {
                  if ((win->tv.markLine >= win->tv.topline)     &&
                      (win->tv.markLine < (win->tv.topline + sy)))
                  {
                     markNr = (short) (win->tv.markLine - win->tv.topline);
                  }
               }
               for ( nr = 0,   s = &(win->tv.buf[win->tv.topline]);
                     nr < sy;
                     nr++)
               {
                  at = TxwAC( cViewTextStand, win->clientclear.at);
                  if ((*s != NULL) && (txSlen(*s) > win->tv.leftcol))
                  {
                     //- some part of this line is visible in the view window

                     line = *s + win->tv.leftcol;
                     txwStringAt( nr, 0, line, at);
                     if ((sl = txSlen( line)) < sx) // clear to end-of-window
                     {
                        txwStringAt( nr, sl, string + sl, at);
                     }
                     if (nr == markNr)          // we are on the marked line
                     {
                        short   markSize  = win->tv.markSize;
                        short   markStart = win->tv.markCol;
                        short   markAtCol = win->tv.markCol; ;

                        if (win->tv.leftcol > markStart) // markstart is invisible
                        {
                           markSize  -= (win->tv.leftcol - markStart);
                           markStart  =  win->tv.leftcol;
                           markAtCol  = 0;
                        }
                        else
                        {
                           markAtCol  = markStart - win->tv.leftcol;
                        }
                        if (markSize > 0)
                        {
                           at = TxwAC( cViewTextMark, win->clientclear.at);
                           sl = min( markSize + 1, TXMAXTM);
                           TxCopy( word, *s + markStart, sl); // include terminating 0
                           txwStringAt( nr, markAtCol, word, at); // repaint in mark color
                        }
                     }
                  }
                  else                          // clear whole line
                  {
                     txwStringAt( nr, 0, string, at);
                  }
                  if (*s != NULL)               // is this the last one ?
                  {
                     s++;                       // to next string, if present
                  }
               }
            }
            break;

         case TXW_SBVIEW:                       // paint main win text only
            if (win->sb.sbdata != NULL)
            {
               txwPaintSbView( wnd);            // supporting partly covered
            }                                   // windows (paints arround)
            break;

         case TXW_HEXEDIT:                      // paint main win text only
            txwPaintHexEdit( wnd);
            break;

         case TXW_ENTRYFIELD:
            if (win->ef.buf != NULL)
            {
               at = TxwAC( cEntryTextStand, win->clientclear.at);
               if (wnd == txwa->focus)          // only set on focus window!
               {
                  if (win->style & TXES_DLGE_FIELD) // field in dialog
                  {
                     at = TxwAC( cDlgEfTextFocus, win->clientclear.at);
                  }
                  else
                  {
                     at = TxwAC( cEntryTextFocus, win->clientclear.at);
                  }
                  txwSetCursorPos( hwnd, 0, win->ef.curpos);
               }
               else
               {
                  if (win->style & TXES_DLGE_FIELD) // field in dialog
                  {
                     at = TxwAC( cDlgEfTextStand, win->clientclear.at);
                  }
                  else
                  {
                     at = TxwAC( cEntryTextStand, win->clientclear.at);
                  }
               }
               line = win->ef.buf + win->ef.leftcol;
               TRACES(( "Painting: '%s'\n", line));
               txwStringAt( 0, 0, line, at);
               sl = txSlen( line);
               if (sl < sx)                     // clear to end-of-window
               {
                  txwStringAt( 0, sl, string + sl, at);
               }
               {
                  TXHIST   *h;
                  if ((h = win->ef.history) != NULL)
                  {
                     if (TxTrLevel > 900)       // severe tracing only :-)
                     {
                        sprintf( string,
                                 "|%4.4hx|%4.4hx|%1.1x|%4.4hx|%8.8lx|%8.8lx",
                                 h->hsize,   h->esize, (int) h->ascend,
                                 h->current, (ULONG) h->elem,  (ULONG) h->buf);
                     }
                     else
                     {
                        if (h->current != TXH_NOCURR)
                        {
                           sprintf( string, "%hu", (USHORT) (h->current +1));
                        }
                        else
                        {
                           strcpy( string, "");
                        }
                     }
                     if ((sl + txSlen(string) +2) < sx) // if enough room ...
                     {
                        at = TxwAC( cEntryHistStand, win->borderclear.at);
                        txwStringAt( 0, sx - strlen(string), string, at);
                     }
                  }
               }
            }
            break;

         case TXW_BUTTON:
            switch (win->style & TXBS_PRIMARYSTYLES)
            {
               case TXBS_RADIOBUTTON:
               case TXBS_AUTORADIO:
                  if (wnd == txwa->focus)
                  {
                     txwSetCursorPos(  hwnd, wnd->cursor.y,   wnd->cursor.x);
                     at      = TxwAC( cRadioTextFocus,  win->clientclear.at);
                     fill.at = TxwAC( cRadioValueFocus, win->clientclear.at);
                  }
                  else
                  {
                     at      = TxwAC( cRadioTextStand,  win->clientclear.at);
                     fill.at = TxwAC( cRadioValueStand, win->clientclear.at);
                  }
                  sprintf( string, "(%c)", (*(win->bu.checked)) ?
                  #if defined (WIN32)
                             'ù'
                  #else                         // OS/2 + DOS use 0x07 (BELL)
                      (txwa->codepage == 437) ? '' : '*'
                  #endif
                           : ' ');
                  txwStringAt( sy/2, 0, string,  fill.at);
                  txwStringAt( sy/2, 4, win->bu.text, at);
                  TRACES(( "Painting: '%s'\n", win->bu.text));
                  break;

               case TXBS_CHECKBOX:
               case TXBS_AUTOCHECK:
                  if (wnd == txwa->focus)
                  {
                     txwSetCursorPos(  hwnd, wnd->cursor.y,   wnd->cursor.x);
                     at      = TxwAC( cCheckTextFocus,  win->clientclear.at);
                     fill.at = TxwAC( cCheckValueFocus, win->clientclear.at);
                  }
                  else
                  {
                     at      = TxwAC( cCheckTextStand,  win->clientclear.at);
                     fill.at = TxwAC( cCheckValueStand, win->clientclear.at);
                  }
                  sprintf( string, "[%c]", (*(win->bu.checked)) ?
                     (txwa->codepage == 437) ? 'û' : 'x' : ' ');
                  txwStringAt( sy/2, 0, string,  fill.at);
                  txwStringAt( sy/2, 4, win->bu.text, at);
                  TRACES(( "Painting: '%s'\n", win->bu.text));
                  break;

               default:
                  TRACES(("Default, draw button text for '%s'\n", win->bu.text));
                  fill    = win->clientclear;
                  if (wnd == txwa->focus)
                  {
                     fill.at = TxwAC( cPushTextFocus, win->clientclear.at);
                  }
                  else
                  {
                     fill.at = TxwAC( cPushTextStand, win->clientclear.at);
                  }
                  TRACES(("fill.at = %hu cc.at = %hu\n", fill.at, win->clientclear.at));
                  txwFillClientWindow( hwnd, fill); // could be optimized
                  txwStringAt( sy/2, (sx - strlen(win->bu.text)) / 2,
                                                  win->bu.text, fill.at);
                  TRACES(( "Painting: '%s'\n", win->bu.text));
                  break;
            }
            break;

         case TXW_LISTBOX:
            list = win->lb.list;
            if    (win->style & TXLS_DROP_VALUE)
            {
               TRACES(("cpos:%4hu, ccol:%2hx, icol:%2hx\n",
                        win->lb.cpos, win->lb.ccol, win->lb.icol));

               strcpy( string, "");
               if ((list != NULL) &&            // do we have a list ?
                   (list->selarray == NULL) &&  // single-select
                   (list->selected < list->count))
               {
                  li = list->selected;
                  sprintf( string, "%-*.*s", sx, sx, list->items[ li]->text);
               }
               if (wnd == txwa->focus)          // color depends on focus ...
               {
                  at      = TxwAC( cSpinTextFocus, win->clientclear.at);
                  fill.at = TxwAC( cSpinIndcFocus, win->lb.icol);
               }
               else
               {
                  at      = TxwAC( cSpinTextStand, win->clientclear.at);
                  fill.at = TxwAC( cSpinIndcStand, win->lb.icol);
               }
               txwStringAt( 0, 0, string, at);
               txwStringAt( 0, sx -1, (txwa->codepage == 437) ? "" : "V", fill.at);
            }
            else if (win->style & TXLS_DROP_MENU)
            {
               if ((win->style & TXWS_TITLEBORDER) == 0)
               {
                  //- to be refined, add 'user-draw' of menu-title instead
                  //- of using the default draw as the window-title
                  //- with added highlighted selection letter
                  //- (requires detection of that letter too, in txwikey!
               }
            }
            else                                // multi-line
            {
               if (win->clientclear.at != cSchemeColor) // other color scheme ?
               {
                  csi_offset = win->clientclear.at - cListTextStand;
               }
               for (nr = 0; nr < sy; nr++)
               {
                  if ((list != NULL) && ((li = list->top + nr) < list->count))
                  {
                     BYTE      ctext;           // text color
                     BYTE      cselc;           // select character color
                     BYTE      cmark;           // mark character color
                     int       index;           // select char index
                     BOOL      disabled = FALSE;
                     BOOL      hasfocus = FALSE;
                     char      mark[2]  = {' ',0}; // default mark string
                     TXS_ITEM *item;
                     BYTE      flags;

                     item = list->items[li];
                     if (list->selarray == NULL)
                     {
                        flags = item->flags;
                     }
                     else
                     {
                        flags = list->selarray[ li];
                     }
                     hasfocus = ((nr == win->lb.cpos) && (wnd == txwa->focus));
                     disabled = ((flags & TXSF_DISABLED ) != 0);

                     if (flags & TXSF_SEPARATOR)
                     {
                        strcpy(  string, " ");
                        TxPClip( string, sx -1, 'Ä');
                        strcat(  string, " ");
                        txwStringAt( nr, 0, string, (hasfocus)   ?
                                     TxwSC( cListSeparatFocus + csi_offset) :
                                     TxwSC( cListSeparatStand + csi_offset));
                     }
                     else
                     {
                        if (hasfocus)
                        {
                           if (disabled)
                           {
                              ctext = TxwAC( cListDisableFocus + csi_offset, win->lb.cdis);
                           }
                           else
                           {
                              ctext = TxwAC( cListTextFocus    + csi_offset, win->lb.ccol);
                           }
                           cselc    = TxwAC( cListSelectFocus  + csi_offset, win->lb.csel);
                           cmark    = TxwAC( cListMarkFocus    + csi_offset, win->lb.cmrk);
                        }
                        else
                        {
                           if (disabled)
                           {
                              ctext = TxwAC( cListDisableStand + csi_offset, win->lb.sdis);
                           }
                           else
                           {
                              ctext = TxwSC( cListTextStand    + csi_offset);
                           }
                           cselc    = TxwAC( cListSelectStand  + csi_offset, win->lb.ssel);
                           cmark    = TxwAC( cListMarkStand    + csi_offset, win->lb.smrk);
                        }

                        if      (flags & TXSF_MARK_STD )
                        {
                           mark[0] = (txwa->codepage == 437) ? 'û' : '+';
                        }
                        else if (flags & TXSF_MARK_STAR)   mark[0] = '*';
                        else if (hasfocus)
                        {
                           cmark = ctext;                  mark[0] = ' ';
                        }
                        else                               mark[0] = ' ';

                        txwStringAt( nr, 0, mark, cmark);

                        strcpy(  string, list->items[ li]->text);
                        TxPClip( string, sx -1, ' ');
                        #if defined (UNIX)      // could be on monochrome
                           if ((hasfocus) &&    // terminal or b/w scheme
                               (txwcs->csflags & TXW_CSF_LIST_UNDERLINE))
                           {
                              TxRepl( string, ' ', '_'); // focus indicator
                           }
                        #endif
                        txwStringAt( nr, 1, string, ctext);

                        if ((index = item->index) != 0)
                        {
                           string[index] = 0;   // terminate after selchar
                           txwStringAt( nr, index, string  + index -1, cselc);
                        }
                     }
                  }
                  else                          // paint empty background
                  {
                     sprintf( string, "%*.*s", sx, sx, "");
                     txwStringAt( nr, 0, string, TxwSC( cListTextStand + csi_offset));
                  }
               }
               if (win->style & TXLS_FOOT_COUNT) // footer must be a TXTT
               {
                  if (list != NULL)
                  {
                     if ((wnd == txwa->focus) && // include current selected
                         (list->count != 0) )   // if there are any items
                     {
                        sprintf( win->footer, "%3lu of %3lu",
                                 list->selected +1, list->count);
                     }
                     else
                     {
                        sprintf( win->footer, " %lu ", list->count);
                     }
                  }
                  else                          // remove the count
                  {
                     strcpy( win->footer, "");
                  }
                  footer_update = TRUE;
               }
            }
            break;

         default:
            break;
      }
      if (rc == NO_ERROR)
      {
         if ((border) || (footer_update))       // paint full border too
         {
            txwPaintBorder( wnd, (wnd == txwa->focus));
         }
         rc = txwPaintWinStatus( wnd, NULL, 0); // paint status info with
      }                                         // default focus indicator
   }
   TXSCREEN_ENDOF_UPDATE();
   RETURN( rc);
}                                               // end 'txwPaintWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Default paint status-indicator parts for a window-class
// can be called separate from PaintWindow if only focus changes
/*****************************************************************************/
ULONG txwPaintWinStatus
(
   TXWINBASE          *wnd,                     // IN    current window
   char               *text,                    // IN    optional status text
   BYTE                color                    // IN    optional status color
)
{
   ULONG               rc   = NO_ERROR;
   TXWHANDLE           hwnd = (TXWHANDLE) wnd;
   TXWINDOW           *win  = wnd->window;
   short               sx = win->client.right  - win->client.left  +1;
   short               sy = win->client.bottom - win->client.top   +1;
   ULONG               top;
   ULONG               first;
   ULONG               length;
   TX1K                string;
   TXTM                s2;
   TXRECT              status;
   TXRECT              area;
   BYTE                fc;                      // footer color
   BYTE                sc;                      // status color
   char               *lc  = NULL;              // ptr in border strings
   char                pad = ' ';               // border padding character

   //- to be refined ? respect border+parent clipping ? ValidateAndClip

   TXSCREEN_BEGIN_UPDATE();

   if (win->style & TXWS_BORDERLINES)           // determine padding char
   {
      switch (txwcs->linestyle)
      {
         case TXW_CS_CUST: lc =                           txws_custom;               break;
         case TXW_CS_HALF: lc = (txwa->codepage == 437) ? txws_half   : txws_hno437; break;
         case TXW_CS_FULL: lc = (txwa->codepage == 437) ? txws_full   : txws_fno437; break;
         default:          lc = (txwa->codepage == 437) ? txws_double : txws_dno437;
            break;
      }
      if (lc != NULL)
      {
         if ((txwcs->linestyle != TXW_CS_3D_LINES) && (wnd == txwa->focus))
         {
            lc += TXLP_FOCUS;
         }
         pad = lc[TXLP_BOT];
      }
   }

   memset( string, pad, TXMAX1K); string[min(sx,TXMAX1K-1)] = 0;
   if ((wnd != NULL) && (win != NULL))
   {
      switch (win->class)
      {
         case TXW_SBVIEW:
            top        = win->sb.topline;
            first      = win->sb.sbdata->firstline;
            length     = win->sb.sbdata->length;
            status     = win->client;
            status.top = status.bottom;         // restrict to status line

            if (win->sb.status != NULL)
            {
               if (text != NULL)                // jvw 'completed' mod
               {
                  win->sb.scolor = color;
                  if ((text == NULL) || (strlen(text) == 0))
                  {
                     strcpy( win->sb.status, "");
                  }
                  else
                  {
                     strcpy( win->sb.status, text);
                  }
               }
            }
            if (wnd == txwa->focus)
            {
               fc = TxwAC( cSbvFooterFocus, win->borderclear.at);
               sc = TxwAC( cSbvStatusFocus, win->sb.scolor);
            }
            else
            {
               fc = TxwAC( cSbvFooterStand, win->borderclear.at);
               sc = TxwAC( cSbvStatusStand, win->sb.scolor);
            }

            //- first clear the complete status line using footer color

            area = status;
            if (txwa->defaultStatus)            // not switched off ?
            {
               if (txwIsRectShowing( hwnd, &area)) // whole status visible ?
               {
                  txwStringAt( sy -1, 0, string, fc);
               }
            }

            //- Now write the Left-side status-text
            if (top + win->sb.sbdata->vsize > first)
            {
               if (top + win->sb.sbdata->vsize >= length)
               {
                  sprintf( string, "Lines %4lu", length - first);
                  if (win->sb.leftcol != 0)
                  {
                     sprintf( s2, "%c%cCol %3d", pad, pad, win->sb.leftcol);
                     strcat( string, s2);
                  }
               }
               else
               {
                  sprintf( string, "%cLine %4lu of %4lu", pad,
                           top + win->sb.sbdata->vsize  - first,
                           length - first);
               }
            }
            else
            {
               sprintf( string, "Empty %4lu of %4lu", top, length);
            }

            area.right = area.left + strlen( string) -1;
            if (txwIsRectShowing( hwnd, &area)) // line counting visible ?
            {
               txwStringAt( sy -1, 1, string, fc);
            }

            if (txwa->defaultStatus)            // not switched off ?
            {
               if ((win->sb.status) && strlen(win->sb.status))
               {
                  strcpy(  string, win->sb.status);
               }
               else
               {
                  sprintf( string, "%s", (wnd == txwa->focus)
                           ? " TAB to switch back to commandline or the menu "
                           : " Ctrl+arrows/PgUp/PgDn=Scroll  F10=menu on/off F11=History ");
               }
               if (strlen(string) != 0)
               {
                  area = status;                // full status line
                  area.left  = area.left + 20;
                  area.right = area.left + strlen( string) -1;
                  if (txwIsRectShowing( hwnd, &area)) // status text visible ?
                  {
                     txwStringAt( sy -1, 20, string, sc);
                  }
               }
               if (strlen( string) < (sx - 36)) // Right-side status-text
               {                                // only if still empty
                  sprintf( string, "%6.6s%c%6.6s",
                          (top > 0)              ? "<PgUp>" : "At-Top", pad,
                          (top < win->sb.maxtop) ? "<PgDn>" : "Bottom" );

                  area = status;                // full status line
                  area.left  = area.right - strlen(string);
                  if (txwIsRectShowing( hwnd, &area)) // status text visible ?
                  {
                     txwStringAt( sy -1, sx - strlen(string) -2, string, fc);
                  }
               }
            }

            //- note: sy is the vertical size of the scrollable-area,
            //-       plus the bottom line with the status-info
            break;

         default:
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   TXSCREEN_ENDOF_UPDATE();
   return( rc);
}                                               // end 'txwPaintWinStatus'
/*---------------------------------------------------------------------------*/

