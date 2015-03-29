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
// TX generic Widget collection, to be included in STD or USER dialogs
//
// Author: J. van Wijk
//
// JvW  24-01-2005 Initial version, check-boxes to be added to File-Dialog

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // txwa anchor interface


#define TXWD_GENWIDGET      (TXWH_SYSTEM_SECTION_BASE + 0)
#define TXGW_H_CHECKBOX     0x0001              // help offset check-boxes
#define TXGW_H_RADIOBUT     0x0002              // help offset radio-buttons
#define TXGW_H_ENTFIELD     0x0003              // help offset entry-fields


//- Note this is the generic help, only used when no user-defined
//- help-ID is supplied on the initialization of the widgets
//- The index is equal to the CLASS value for the control
static char           *genwidgethelp[] =
{
   "#000, TX Generic Widget help",
   "",
   " The generic widget is a set of dialog-controls that can be",
   " included in standard dialogs (like the File-Dialog) and in",
   " other user dialogs to enhance functionality without too much",
   " specific codeing.",
   "",
   " The behaviour of the widget controls (check-boxes etc) is",
   " largely defined by the generic-widget code, and the usage",
   " of the contained information by the dialog (or application)."
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   TXHELPITEM(002, "Generic Widget Static text")
   " This is a generic Widget STATIC control",
   "",
   " It is OUTPUT only, so you should not have",
   " been able to call up help on such a control :-)",
   "",
   TXHELPITEM(003, "Generic Widget Static line")
   " This is a generic Widget STLINE control",
   "",
   " It is OUTPUT only, so you should not have",
   " been able to call up help on such a control :-)",
   "",
   TXHELPITEM(005, "Generic Widget Entryfield")
   " This is a generic Widget BUTTON control, there are 3 types:",
   "",
   " An entryfield is used for numeric or text input on a single line.",
   "",
   " For the meaning of this specific entryfield, refer to the global",
   " help available for the whole dialog by pressing <F1> again ...",
   "",
   TXHELPITEM(006, "Generic Widget Texview")
   " This is a generic Widget TEXTVIEW control",
   "",
   " A textview is used to display multiple lines of text, and",
   " allows scrolling to view text that falls outside the window",
   "",
   " For the meaning of this specific textview, refer to the global",
   " help available for the whole dialog by pressing <F1> again ...",
   "",
   TXHELPITEM(007, "Generic Widget Button")
   " This is a generic Widget BUTTON control, there are 3 types:",
   "",
   " - A checkbox is used for simple ON/OFF properties in a dialog.",
   " - A radiobutton is used in a group of ON/OFF properties",
   " - A push-button is used to start an action",
   "",
   " You can toggle the value of checkbox or radiobutton by using",
   " the SPACEBAR when it has the focus, as indicated by the cursor.",
   "",
   " For the meaning of this specific button, refer to the global",
   " help available for the whole dialog by pressing <F1> again ...",
   "",
   TXHELPITEM(008, "Generic Widget Listbox")
   " This is a generic Widget LISTBOX control",
   "",
   " A listbox is used to present a list of choices to the user",
   " and allow one or more selections to be made.",
   "",
   " For the meaning of this specific listbox, refer to the global",
   " help available for the whole dialog by pressing <F1> again ...",
   "",
   TXHELPITEM(999, "End of Generic Widget help")
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   NULL
};

/*****************************************************************************/
// Initialize generic-widget data structures
/*****************************************************************************/
BOOL txwInitGenericWidgets
(
   void
)
{
   BOOL                rc = TRUE;               // function return

   ENTER();

   txwRegisterHelpText( TXWD_GENWIDGET, "txwidget", "TX Generic Widget help", genwidgethelp);

   BRETURN (rc);
}                                               // end 'txwInitGenericWidgets'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate all generic-widget data structures
/*****************************************************************************/
void txwTermGenericWidgets
(
   void
)
{
   ENTER();

   //- nothing to do yet

   VRETURN();
}                                               // end 'txwTermGenericWidgets'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get number of lines and columns taken by a widget group
/*****************************************************************************/
short txwWidgetSize                             // RET   nr of lines taken
(
   TXGW_DATA          *gwdata,                  // IN    generic widget data
   short               toplines,                // IN    empty lines at top
   short              *hsize                    // OUT   nr of columns taken
)                                               //       or NULL if not needed
{
   short               rc = 0;                  // function return
   short               ww = 0;                  // nr of columns

   ENTER();

   if (gwdata && (gwdata->count != 0))          // widget data ?
   {
      TXWIDGET        *wg;                      // data for one widget
      int              w;                       // widget counter

      for (w = 0; w < gwdata->count; w++)       // itterate over widgets
      {
         wg = &(gwdata->widget[w]);             // widget array element

         if ((wg->flags & TXWI_DISABLED) == 0)
         {
            rc = max( wg->vpos + wg->vsize, rc); // find max lines
            ww = max( wg->hpos + wg->hsize, ww); // find max columns
         }
      }
      if (hsize != NULL)                        // columns wanted ?
      {
         *hsize = ww;
      }
      rc += toplines;
   }
   RETURN (rc);
}                                               // end 'txwWidgetSize '
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create specified controls for this Generic Widget instance
/*****************************************************************************/
ULONG txwCreateWidgets
(
   TXWHANDLE           dframe,                  // IN    parent window (dialog)
   TXGW_DATA          *gw,                      // INOUT generic widget data
   short               line,                    // IN    UL-corner lines
   short               col                      // IN    UL-corner column
)                                               // allows multiple widget groups
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRACES(("dframe:%8.8lx  gw:%8.8lx\n", dframe, gw));

   if ((gw) && (txwIsWindow( TXHWND_DESKTOP)))  // Widgets and a desktop ?
   {
      TXWINDOW         window;                  // setup window data
      TXWIDGET        *wg;                      // data for one widget
      int              w;                       // widget counter

      for (w = 0; w < gw->count; w++)           // itterate over widgets
      {
         wg = &(gw->widget[w]);                 // widget array element

         if ((wg->flags & TXWI_DISABLED) == 0)
         {
            txwSetupWindowData( wg->vpos + line, wg->hpos + col,
                                wg->vsize,       wg->hsize, wg->style,
                               (wg->helpid) ?    wg->helpid :
                               (gw->helpid) ?    gw->helpid :
                                wg->class   +    TXWD_GENWIDGET,
                                    ' ', ' ',    TXWSCHEME_COLORS,
                               (wg->title ) ?    wg->title  : "", "",
               &window);
            switch (wg->class)                  // shallow copy class-data
            {
               case TXW_STATIC:     window.st = wg->st; break;
               case TXW_STLINE:     window.sl = wg->sl; break;
               case TXW_ENTRYFIELD: window.ef = wg->ef; break;
               case TXW_TEXTVIEW:   window.tv = wg->tv; break;
               case TXW_BUTTON:     window.bu = wg->bu; break;
               case TXW_LISTBOX:    window.lb = wg->lb; break;
               default:                                 break;
            }
            wg->hwnd = txwCreateWindow( dframe, wg->class, dframe, 0, &window,
                               (wg->winproc) ?  wg->winproc : txwDefWindowProc);
            txwSetWindowPtr(    wg->hwnd, TXQWP_USER,  gw); // make data available
            txwSetWindowUShort( wg->hwnd, TXQWS_GROUP, wg->group);
            txwSetWindowUShort( wg->hwnd, TXQWS_ID,   (wg->winid)  ? wg->winid :
                                                       gw->basewid + w);
         }
      }
   }
   RETURN (rc);
}                                               // end 'txwCreateWidgets'
/*---------------------------------------------------------------------------                    */


#if defined (NEVER)                             // template for user WinProc
/*****************************************************************************/
// Default Window procedure, for a Generic Widgets control
/*****************************************************************************/
ULONG txwWidgetWinProc                          // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc  = NO_ERROR;

   ENTER();
   if (hwnd != 0)
   {
      TXGW_DATA       *gw    = txwQueryWindowPtr(    hwnd, TXQWP_USER);

      TRCMSG( hwnd, msg, mp1, mp2);
      if (gw != NULL)                           // valid widget data ?
      {
         TXWHANDLE     owner = txwQueryWindow(       hwnd, TXQW_OWNER);
         USHORT        wid   = txwQueryWindowUShort( hwnd, TXQWS_ID);
         int           w     = wid - gw->basewid;

         switch (msg)
         {
            default:
               rc = txwDefWindowProc( hwnd, msg, mp1, mp2);
               break;
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
}                                               // end 'txwWidgetWinProc'
/*---------------------------------------------------------------------------*/
#endif

/*****************************************************************************/
// Display generic widget dialog, adding standard empty canvas to the widgets
/*****************************************************************************/
ULONG txwWidgetDialog
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   void               *usrdata,                 // IN    window data (QWP_USER)
   char               *title,                   // IN    title for the dialog
   ULONG               flags,                   // IN    specification flags
   USHORT              focus,                   // IN    focus to index 0..n-1
   TXGW_DATA          *gwdata                   // INOUT generic widget data
)
{
   ULONG               rc = TX_INVALID_HANDLE;  // function return

   ENTER();
   TRACES(("parent:%8.8lx  owner:%8.8lx  title:'%s'\n", parent, owner, title));
   TRACES(("gwdata:%8.8lx\n", gwdata));

   if ((txwIsWindow( TXHWND_DESKTOP)) &&        // is there a desktop ?
       (gwdata != NULL))                        // and widget data ?
   {
      TXRECT           position;                // non persistent size/position
      TXRECT          *framepos;                // frame position structure
      TXRECT           parentpos;               // parent size/position
      TXWHANDLE        pframe;                  // widget-dialog frame
      TXWINDOW         window;                  // setup window data
      ULONG            style;
      USHORT           phsize;                  // parent window width
      USHORT           pvsize;                  // parent window height
      short            ll = 0;                  // vertical size
      short            ww = 0;                  // horizontal size

      ll = txwWidgetSize( gwdata, 0, &ww);      // get widget sizes

      txwQueryWindowRect( parent, FALSE, &parentpos);
      phsize = parentpos.right;
      pvsize = parentpos.bottom;

      if (gwdata && (gwdata->posPersist))       // dedicated persistent position
      {
         framepos = gwdata->posPersist;
      }
      else
      {                                         // default non-persistent
         framepos = &position;                  // forced calculation each time
         framepos->right = 0;                   // avoids non-centered popups
      }                                         // due to size differences

      if (framepos->right == 0)                 // not initialized yet ?
      {                                         // start with parent position
         *framepos = parentpos;
         if (framepos->left  + ww + 6 < phsize)
         {
            framepos->right  = ww + 4;
            framepos->left   = phsize - ww -4;
            if (flags & TXWD_HCENTER)
            {
               framepos->left /= 2;                 // center horizontally
            }
         }
         if (framepos->top   + ll + 6 < pvsize)
         {
            framepos->bottom = ll + 4;
            framepos->top   += 1;
            if (flags & TXPB_VCENTER)
            {
               framepos->top += ((pvsize - framepos->bottom) * 2 / 5); // center vertically
            }
         }
      }
      else                                      // calculated size of Dlg
      {                                         // (but keeps position)
         framepos->right  = ww + 4;
         framepos->bottom = ll + 4;
      }
      TRECTA( "pos/size", (framepos));

      style = TXWS_DIALOG | TXWS_DISABLED | TXCS_CLOSE_BUTTON;
      if (flags & TXPB_MOVEABLE)
      {
         style |= TXWS_MOVEABLE;                // make frame move/sizeable
      }
      txwSetupWindowData(
         framepos->top,                         // upper left corner
         framepos->left,
         framepos->bottom,                      // vertical size
         framepos->right,                       // horizontal size
         style | TXWS_CAST_SHADOW,              // window frame style
         gwdata->helpid,                        // messagebox help
         ' ', ' ', TXWSCHEME_COLORS,
         title, txwstd_footer,
         &window);

      if ((focus < gwdata->count) &&
          (gwdata->widget[focus].winid != 0))   // explicit window-ID
      {
         window.dlgFocusID = gwdata->widget[focus].winid;
      }
      else                                      // calculated window-ID
      {
         window.dlgFocusID = gwdata->basewid + focus; // field to get focus
      }
      window.st.buf     = NULL;                 // NO artwork attached
      pframe = txwCreateWindow( parent, TXW_CANVAS, 0, 0, &window, NULL);
      txwSetWindowPtr( pframe, TXQWP_DATA, framepos); // for exit position

      if ((rc = txwCreateWidgets( pframe, gwdata, 1, 1)) == NO_ERROR)
      {
         rc = txwDlgBox( parent, owner,
                        (gwdata->winproc) ? gwdata->winproc : txwDefDlgProc,
                         pframe, usrdata);
      }
   }
   RETURN (rc);
}                                               // end 'txwWidgetDialog'
/*---------------------------------------------------------------------------*/

