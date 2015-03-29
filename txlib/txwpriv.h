#ifndef TXWPRIV_H
#define TXWPRIV_H
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
//
// TX window library private interfaces
//
// Author: J. van Wijk
//
// Developed for LPTool/DFSee utilities
//
// JvW  29-08-2001   Added dialog windows
// JvW  12-07-1998   Initial version

typedef struct txwinbase                        // private window and link info
{
   struct txwinbase   *parent;                  // parent window
   struct txwinbase   *owner;                   // owner  window
   TXWINDOW           *window;                  // actual window data
   ULONG              *acceltable;              // window accelerator table
   TXWINPROC           winproc;                 // window procedure
   TXPOINT             cursor;                  // relative cursor position
   BOOL                curvisible;              // cursor visibility
   USHORT              us[TXQWS_SIZE];          // window USHORT values
   ULONG               ul[TXQWL_SIZE];          // window ULONG and PTR values
   TXRECT              restore;                 // size/pos for restore (a_F5)
   TXWHANDLE           oldFocus;                // previous focus window
   TXCELL             *oldContent;              // screen area save/restore
   char               *oldFooter;               // original footer text
   struct txwinbase   *next;                    // next window (visiting order)
   struct txwinbase   *prev;                    // previous window
} TXWINBASE;                                    // end of struct "txwinbase"

#define TXW_ARROW_STD              0x0000       // no special arrow handling
#define TXW_ARROW_MOVE             0x0001       // move a window
#define TXW_ARROW_SIZE             0x0002       // size a window
#define TXW_ARROW_COLOR            0x0004       // select color scheme

typedef struct txw_anchor
{
   #if defined   (DEV32)
      ULONG            session;                 // OS2 sessiontype VIO/FS ...
      BOOL             KbdKill;                 // KBD reader thread kill
   #elif defined (DOS32)
      char            *dpmi1;                   // DPMI parameter  ptr,  512
      char            *dpmi2;                   // DPMI parameter  ptr,  512
      short            dpmi1Selector;
      short            dpmi2Selector;
   #endif
   BOOL                insert;                  // global insert-mode
   BOOL                typeahead;               // typeahead enabled
   ULONG               arrowMode;               // in special arrow mode
   USHORT              autoid;                  // auto assigned window id
   ULONG               radixclass;              // radix class bits, 1=HEX
   short               cursorRow;               // saved cursor row
   short               cursorCol;               // saved cursor col
   TXRECT              screen;                  // screen rectangle
   USHORT              reopenMenu;              // menu-header ID to re-open
   short               mDragRow;                // absolute row and col start
   short               mDragCol;                // position on window drag
   ULONG               mDragFlags;              // mouse dragging flags
   BOOL                useMouse;                // mouse usage enabled
   BOOL                pedantic;                // pedantic warnings/errors
   TXWHANDLE           capture;                 // mouse capture window
   TXWHANDLE           modality;                // modality window
   TXWINBASE          *focus;                   // focus window
   TXWINBASE          *desktop;                 // first window in chain
   TXWINBASE          *last;                    // last window in chain
   long                windows;                 // actual window count
   TXWINBASE          *sbview;                  // SBview for quick status
   TXWHANDLE           maincmd;                 // Entryfield marked as cmdline
   TXWHANDLE           mainmenu;                // Canvas marked as main menu
   BOOL                defaultStatus;           // show default status text
   ULONG               rsmaxlines;              // max lines before resize dlg
   ULONG               codepage;                // codepage, std 437 compatible
   ULONG              *acceltable;              // system accelerator table
   TXLN                listmatch;               // list match string value
} TXW_ANCHOR;                                   // end of struct "txw_anchor"

extern TXW_ANCHOR *txwa;                        // private anchor (HAB)

extern BYTE *txVbuf;                            // one line video buffer

// Validate window-handle, return its TXWBASE pointer, NULL is invalid
TXWINBASE *txwValidateHandle                    // RET   window base pointer
(
   TXWHANDLE           hwnd,                    // IN    handle to validate
   TXWINDOW          **win                      // OUT   attached window ptr
);

// Validate handle and calculate clip rectangle from all (parent) client areas
TXWINBASE *txwValidateAndClip                   // RET   window base pointer
(
   TXWHANDLE           hwnd,                    // IN    handle to validate
   TXWINDOW          **window,                  // OUT   attached window
   BOOL                border,                  // IN    start clip with border
   TXRECT             *pclip                    // OUT   combined clip rect
);

// Calculate visible region(s) for a 1-line string area
void txwVisibleLineRegions
(
   TXWINBASE          *wnd,                     // IN    window with string
   TXRECT_ELEM        *clr                      // INOUT list of clip rectangles
);                                              //       on input the full line

// Fast ScrollBuff paint, SBVIEW only; supports paint arround pop-up windows
void txwPaintSbView
(
   TXWINBASE          *wnd                      // IN    current window
);

// Default paint border-shadow for window, no tracing (for txwPaintSbView)
void txwPaintShadow
(
   TXWINBASE          *wnd                      // IN    current window
);

// Save window contents to be restored at destroy time (incl borders)
ULONG txwSaveWindowContent
(
   TXWINBASE          *wnd                      // IN    window
);

// Restore window contents saved at creation time (incl borders)
ULONG txwRestoreWindowContent
(
   TXWINBASE          *wnd,                     // IN    window
   TXRECT             *clip1,                   // IN    partial restore area 1
   TXRECT             *clip2                    // IN    partial restore area 2
);

// Default paint window border and focus indication, for any window-class
void txwPaintBorder
(
   TXWINBASE          *wnd,                     // IN    current window
   BOOL                focus                    // IN    focus indicator
);

// Default paint, for standard window-classes; draw client window contents
ULONG txwPaintWindow
(
   TXWINBASE          *wnd,                     // IN    current window
   BOOL                border                   // IN    paint border too
);

// Default paint status-indicator parts for a window-class
ULONG txwPaintWinStatus
(
   TXWINBASE          *wnd,                     // IN    current window
   char               *text,                    // IN    optional status text
   BYTE                color                    // IN    optional status color
);

// Find enabled and visible window that is 'next' to origin window
TXWHANDLE txwFindNextFocus
(
   TXWHANDLE           hwnd,                    // IN    origin window
   BOOL                tabgroup                 // IN    next GROUP, not window
);

// Find enabled and visible window that is 'previous' to origin window
TXWHANDLE txwFindPrevFocus
(
   TXWHANDLE           hwnd,                    // IN    origin window
   BOOL                tabgroup                 // IN    prev GROUP, not window
);

/*======== Generic Widgets,                       TXWIDGET.C ================*/

// Initialize generic-widget data structures
BOOL txwInitGenericWidgets
(
   void
);

// Terminate all generic-widget data structures
void txwTermGenericWidgets
(
   void
);

/*======== Standard dialog handling,              TXWSTDLG.C ================*/

// Initialize all standard dialog data structures
BOOL txwInitStandardDialogs
(
   void
);

// Terminate all standard dialog data structures
void txwTermStandardDialogs
(
   void
);

/*======== Utility window related functions,       TXWUTIL.C ================*/

// Update footer right-hand text based on current list sorting specification
void txwListSort2frhText
(
   TXWHANDLE           hwnd                     // IN    window with attached list
);

/*======== Keyboard processing for window classes, TXWIKEY.C ================*/

// Input key handling for the TEXTVIEW window class
ULONG txwIkeyTextview                           // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
);

// Input key and user-msg handling for the SBVIEW window class
ULONG txwSbViewWinProc                          // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
);

// Input key handling for the ENTRYFIELD window class
ULONG txwIkeyEntryfield                         // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
);

// Input key handling for the BUTTON window class
ULONG txwIkeyButton                             // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
);

// Input key handling for the LISTBOX window class
ULONG txwIkeyListBox                            // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
);


/*======== Hex Editor dialog and class handling TXWHEXED.C ==================*/

// Hex Editor class, paint window
void txwPaintHexEdit
(
   TXWINBASE          *wnd                      // IN    current window
);

// Window procedure for the HEXED window class, handling mouse and keyboard
// Called AFTER generic default processing, as class specific default handling
ULONG txwHexEditWinProc                         // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    param 1
   ULONG               mp2                      // IN    param 2
);


/*======== Screen related functions, TXWIOP.C ===============================*/

#if defined (UNIX)
   #define TXSCREEN_BEGIN_UPDATE()  TxLinUpdateScreen(FALSE)
   #define TXSCREEN_ENDOF_UPDATE()  TxLinUpdateScreen(TRUE)
   #define TXSCREEN_FLUSH_CACHED()  TxLinClearTerminal()

typedef enum txlterm                            // supported terminal types
{
   TXLT_CONSOLE,                                // standard Linux console
   TXLT_XTERM_VT100,                            // Xterm, ANSI or VT100
   TXLT_VT220,                                  // VT220 compatible
} TXLTERM;                                      // end of enum "txlterm"

// Get type of Linux terminal used by low-level IO
TXLTERM TxLinTerminalType                       // RET    Terminal type
(
   void
);

// refresh whole screen
void TxLinUpdateScreen
(
   BOOL                finished                 // IN    Virtual updates done
);                                              //       will cause real screen
                                                //       update if outer level

// Clear terminal screen buffer, forcing complete redraw on next Update
void TxLinClearTerminal
(
   void
);

#else
   #define TXSCREEN_FLUSH_CACHED()
   #define TXSCREEN_BEGIN_UPDATE()
   #define TXSCREEN_ENDOF_UPDATE()
#endif

#if defined (WIN32)
// Specific Win-NT console definitions and functions

// display a string on NT console, with full ANSI support
void txNtConsoleDisplay
(
   char *text
);
#endif


/*======== Message related functions, TXWMSG.C ==============================*/
// Initialize low level input handling
ULONG TxInputInitialize
(
   void
);

// Initialize input for windowed Desktop environment
ULONG TxInputDesktopInit
(
   void
);

// Terminate low level input handling
ULONG TxInputTerminate
(
   void
);

#endif
