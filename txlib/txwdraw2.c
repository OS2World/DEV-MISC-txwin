
#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface


#if defined (USEWINDOWING)

/*****************************************************************************/
// Set specified status text on default SBview status window (progress info)
/*****************************************************************************/
void txwSetSbviewStatus
(
   char               *text,                    // IN    optional status text
   BYTE                color                    // IN    optional status color
)
{
   if (txwa->sbview)
   {
      TXWINBASE       *wnd  = txwa->sbview;
      TXWHANDLE        hwnd = (TXWHANDLE) wnd;
      TXWINDOW        *win  = wnd->window;
      short            sy = win->client.bottom - win->client.top   +1;
      short            sx = win->client.right  - win->client.left  +1;
      static TXLN      sbStat;

      TXSCREEN_BEGIN_UPDATE();

      //- note: sy is the vertical size of the scrollable-area,
      //-       plus the bottom line with the status-info

      memset( sbStat, ' ', TXMAXLN);
      if (sx > 33)
      {
         sbStat[ sx - ((txwa->defaultStatus) ? 27 : 14)] = 0;
      }
      else
      {
         sbStat[ sx] = 0;
      }
      txwStringAt( sy -1, 12, sbStat, TxwAC( cSbvProgreStand, win->borderclear.at));
      txwStringAt( sy -1, 12, text,   TxwAC( cSbvProgreStand, color));

      #if defined (HAVETHREADS)
         txwSignalEventHook( TXHK_SBSTATUS, sbStat);
      #endif

      TXSCREEN_ENDOF_UPDATE();
   }
}                                               // end 'txwSetSbviewStatus'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set display of default-status in sbview ON or OFF
/*****************************************************************************/
void txwDefaultStatusShow
(
   BOOL                show                     // IN    show default status
)
{
   if (txwa->sbview)
   {
      txwa->defaultStatus = show;
   }
}                                               // end 'txwDefaultStatusShow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set specified (trace) text on Desktop title- or top-line (quick and dirty)
/*****************************************************************************/
void txwSetDesktopTopLine
(
   char               *text,                    // IN    optional status text
   BYTE                color                    // IN    optional status color
)
{
   TXRECT              clip;                    // parent Clip rectangle
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   if ((wnd = txwValidateAndClip( TXHWND_DESKTOP, &win, TRUE, &clip)) != NULL)
   {
      short            sx = win->border.right - win->border.left  +1;
      short            at = win->border.left  + ((sx - 34) / 2); // centered
      static TXLN      dskTop;

      //- Must use border and txwScr... here because top is outside client

      TXSCREEN_BEGIN_UPDATE();

      memset( dskTop, ' ', TXMAXLN);
      dskTop[ 32] = 0;
      txwScrDrawCharStrCol( win->border.top, at,  &clip, dskTop,
         TxwAC( cDskTraceStand, win->borderclear.at));
      txwScrDrawCharStrCol( win->border.top, at +1, &clip, text,
         TxwAC( cDskTraceStand, color));

      #if defined (NEVER)                       // recursion problem with TRACE
         txwSignalEventHook( TXHK_DSKTOPLN,     // since PostMsg traces too
                             dskTop);
      #endif

      TXSCREEN_ENDOF_UPDATE();
   }
}                                               // end 'txwSetDesktopTopLine'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Draw a character-string at specified position, clip to parent
/*****************************************************************************/
void txwDrawCharString
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   char               *str                      // IN    string to draw
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              clip;                    // parent Clip rectangle

   ENTER();
   TRACES(("hwnd:%8.8lx, row:%4u, col:%4u, str:'%s'\n", hwnd, row, col, str));

   if ((wnd = txwValidateAndClip( hwnd, &win, FALSE, &clip)) != NULL)
   {
      txwScrDrawCharString( win->client.top   + row,
                            win->client.left  + col,
                            &clip, str);
   }
   VRETURN();
}                                               // end 'txwDrawCharString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Draw a character-string at position in specified color, clip to parent
/*****************************************************************************/
void txwDrawCharStrCol
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   char               *str,                     // IN    string to draw
   BYTE                color                    // IN    color attribute
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              clip;

   if ((wnd = txwValidateAndClip( hwnd, &win, FALSE, &clip)) != NULL)
   {
      txwScrDrawCharStrCol( win->client.top   + row,
                            win->client.left  + col,
                            &clip, str, color);
   }
}                                               // end 'txwDrawCharStrCol'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Draw a cell-string at specified position, clip to parent
/*****************************************************************************/
void txwDrawCellString
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXCELL             *cell,                    // IN    cell-string to draw
   short               size                     // IN    nr of cells
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              clip;

   if ((wnd = txwValidateAndClip( hwnd, &win, FALSE, &clip)) != NULL)
   {
      txwScrDrawCellString( win->client.top   + row,
                            win->client.left  + col,
                            &clip, cell, size, TXSB_COLOR_NORMAL);
   }
}                                               // end 'txwDrawCellString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Fill area for rectangle with specified cell (clear area)
/*****************************************************************************/
void txwFillClientWindow
(
   TXWHANDLE           hwnd,                    // IN    window handle
   TXCELL              cell                     // IN    cell-string to draw
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              clip;

   ENTER();
   TRACES(("hwnd:%8.8lx  ch:%2.2hx, at:%2.2hx\n",
            hwnd, (USHORT) cell.ch, (USHORT) cell.at));

   if ((wnd = txwValidateAndClip( hwnd, &win, FALSE, &clip)) != NULL)
   {
      txwScrFillRectangle( &clip, cell);
   }
   VRETURN();
}                                               // end 'txwFillClientWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Scroll client area up or down, fill new with specified cell
// Note: As of 05-03-2004 this function is NOT used in the library itself!
/*****************************************************************************/
void txwScrollClientWindow
(
   TXWHANDLE           hwnd,                    // IN    window handle
   int                 scroll                   // IN    scroll lines (+ = up)
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              clip;

   ENTER();
   TRACES(("hwnd:%8.8lx  scroll:%3hd\n", hwnd, (short ) scroll));

   if ((wnd = txwValidateAndClip( hwnd, &win, FALSE, &clip)) != NULL)
   {
      if (txwNormalRect( FALSE, &clip))         // true normalized rect?
      {
         txwScrScrollRectangle( &clip, scroll, win->clientclear);
      }
   }
   VRETURN();
}                                               // end 'txwScrollClientWindow'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set cursor position inside a client window, when inside (parent) clip area
/*****************************************************************************/
void txwSetCursorPos
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col                      // IN    destination column
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;

   ENTER();
   TRACES(("hwnd:%8.8lx  row:%3hu col:%3hu\n", hwnd, row, col));

   if ((wnd = txwValidateHandle( hwnd, &win)) != NULL)
   {
      wnd->cursor.y = row;
      wnd->cursor.x = col;

      TxSetCursorPos(  win->client.top   + row,
                       win->client.left  + col);

      txwSetCursorStyle( hwnd, txwa->insert);   // update visibility
   }
   VRETURN();
}                                               // end 'txwSetCursorPos'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set cursor style for window, visible only when inside (parent) clip area
/*****************************************************************************/
void txwSetCursorStyle
(
   TXWHANDLE           hwnd,                    // IN    window handle
   BOOL                insert                   // IN    insert style
)
{
   TXWINBASE          *wnd;
   TXWINDOW           *win;
   TXRECT              clip;

   ENTER();
   TRACES(("hwnd:%8.8lx  insert:%lu\n", hwnd, insert));

   if ((wnd = txwValidateAndClip( hwnd, &win, FALSE, &clip)) != NULL)
   {
      TXSCREEN_BEGIN_UPDATE();
      if ((wnd->curvisible) &&                  // should be visible ?
          (txwInRectangle( win->client.top   + wnd->cursor.y,
                           win->client.left  + wnd->cursor.x, &clip)))
      {
         TxSetCursorStyle( TRUE,  txwa->insert);
      }
      else
      {
         TxSetCursorStyle( FALSE, txwa->insert); // hide cursor
      }
      TXSCREEN_ENDOF_UPDATE();
   }
   VRETURN();
}                                               // end 'txwSetCursorStyle'
/*---------------------------------------------------------------------------*/

#endif
