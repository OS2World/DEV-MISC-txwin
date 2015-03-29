#ifndef TXWIN_H
#define TXWIN_H
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
// TX window library interfaces
//
// Author: J. van Wijk
//
// Developed for LPTool/DFSee utilities
//

#define TXW_INVALID       (-1)                  // invalid value

typedef ULONG             TXWHANDLE;            // TX window handle
typedef ULONG             TXWHENUM;             // TX enumeration handle

// Predefined window handles
#define TXHWND_NULL       (TXWHANDLE) 0
#define TXHWND_DESKTOP    (TXWHANDLE) 1
#define TXHWND_OBJECT     (TXWHANDLE) 2         // not supported yet
#define TXHWND_TOP        (TXWHANDLE) 3         // last window in Z-order
#define TXHWND_BOTTOM     (TXWHANDLE) 4         // currently equals desktop

//- to be used as eyecatcher after destroying a window (catch stray messages)
#define TXHWND_INVALID    (TXWHANDLE) 0xdeadbeef


// Query window codes
#define TXQW_NEXT         0
#define TXQW_PREV         1
#define TXQW_TOP          2
#define TXQW_BOTTOM       3
#define TXQW_OWNER        4
#define TXQW_PARENT       5

// TXWIN window classes
//
// Note: STATIC without buffer behaves like FRAME, with empty buffer as CANVAS
//       CANVAS with a buffer behaves like STATIC, can be used as a DIALOG frame
//              (using ASCII artwork as a dialog background, text + drawing)
//
//       Do NOT use a STATIC as Dialog-frame! Might cause paint-problems

typedef enum txwclass                           // window classes
{                                               // type of client area painting
   TXW_FRAME         = 0,                       // border only, no client fill
   TXW_CANVAS        = 1,                       // background color / pattern
   TXW_STATIC        = 2,                       // static window text
   TXW_STLINE        = 3,                       // static window line
   TXW_SBVIEW        = 4,                       // TxPrint scroll buffer view
   TXW_ENTRYFIELD    = 5,                       // simple 1-line entry field
   TXW_TEXTVIEW      = 6,                       // ascii text viewer
   TXW_BUTTON        = 7,                       // button - push/radio/checkbox
   TXW_LISTBOX       = 8,                       // simple selection-list
   TXW_HEXEDIT       = 9,                       // Hex (sector) editor
} TXWCLASS;                                     // end of enum "txwclass"


//- Message parameter handling macros, 2 USHORTS in one MP
#define TXSH1FROMMP(mp)           ((USHORT)((ULONG)(mp      )))
#define TXSH2FROMMP(mp)           ((USHORT)((ULONG)(mp >> 16)))

#define TXMPFROM2SH(s1,s2) ((ULONG)(((USHORT)(s1)) | ((ULONG)((USHORT)(s2))) << 16))

typedef enum txwmsgid                           // TX window messages
{
   TXWM_CREATE,                                 // window has just been created
   TXWM_DESTROY,                                // window will be destroyed
   TXWM_CHAR,                                   // keyboard event, key in mp2
   TXWM_COMMAND,                                // window command (menu)
   TXWM_CLOSE,                                  // close this window
   TXWM_QUIT,                                   // terminate application
   TXWM_HELP,                                   // activate help system
   TXWM_BORDER,                                 // paint border only
   TXWM_STATUS,                                 // paint status info, add text
   TXWM_PAINT,                                  // paint invalided win contents
   TXWM_SETFOCUS,                               // keyboard input focus
   TXWM_ENABLE,                                 // window enable notification
   TXWM_SHOW,                                   // window visible notification
   TXWM_ACTIVATE,                               // set (frame) window active
   TXWM_SELECTED,                               // set (dialog) item selected
   TXWM_CURSORVISIBLE,                          // set cursor visibility
   TXWM_MOVE,                                   // window has been moved
   TXWM_SIZE,                                   // window has been resized
   TXWM_CONTROL,                                // dialog control message
   TXWM_INITDLG,                                // init dialog
   TXWM_MENUSELECT,                             // menu selection from control
   TXWM_MENUEND,                                // end menu, no selection
   TXWM_SETFOOTER,                              // set footer text   (or reset)
   TXWM_BUTTONDOWN,                             // mouse button went down (any)
   TXWM_BUTTONUP,                               // mouse button up (any button)
   TXWM_BUTTONDBLCLK,                           // mouse doubleclk (not implem)
   TXWM_MOUSEMOVE,                              // mouse movement  (not implem)
   TXWM_HOOKEVENT,                              // hook event signal
   TXWM_FD_FILTER    = 100,                     // filter fname before display
   TXWM_FD_VALIDATE,                            // validate fname on select
   TXWM_FD_ERROR,                               // user formatted error msg
   TXWM_FD_NEWSPEC,                             // FD new dir or wildcard
   TXWM_FD_NEWPATH,                             // FD new path/directory set
   TXWM_FD_WILDCARD,                            // FD reset to org wildcard
   TXWM_FD_POPULATED,                           // FD dir/files lists populated
   TXWM_PERFORM_SEARCH = 200,                   // Help and SB, perform search
   TXWM_PERFORM_GREP,                           // Help and SB, perform grep
   TXWM_USER         = 1000                     // lowest user message id
} TXWMSGID;                                     // end of enum "txwmsgid"


// Event hook definitions (used with the WM_HOOKEVENT message and functions)
#define  TXHK_SBSTATUS    0
#define  TXHK_EVENT_1     1
#define  TXHK_EVENT_2     2
#define  TXHK_EVENT_3     3
#define  TXHK_EVENT_4     4
#define  TXHK_EVENT_5     5
#define  TXHK_EVENT_6     6
#define  TXHK_EVENT_7     7
#define  TXHK_EVENT_8     8
#define  TXHK_DSKTOPLN    9
#define  TXHK_EVENTS     10


#define TXW_INVALID_COORD  ((short) 9999)

// Mouse messages (18-05-2005: only BUTTONDOWN is implemented) will have :
// col (X) in 1st short of mp1                  // absolute coordinates
// row (Y) in 2nd short of mp1                  // 0,0 is top-left
// button  in 1st short of mp2, button1, 2, 3, wheel-up, wheel-down
// kstate  in 2nd short of mp2, alt, control, shift

#define TXMOUSECOL()     TXSH1FROMMP(mp1)
#define TXMOUSEROW()     TXSH2FROMMP(mp1)
#define TXMOUSEBUTTON()  TXSH1FROMMP(mp2)
#define TXMOUSEKEYMOD()  TXSH2FROMMP(mp2)

// shiftstate, restricted to Shift, Ctrl and Alt, NO num-lock or scroll-lock
#define TXMOUSEKEYSCA() (TXSH2FROMMP(mp2) & 0x1f)

#define TXm_BUTTON1    0x0001                   // usually LEFT  button
#define TXm_BUTTON2    0x0002                   // usually RIGHT button
#define TXm_BUTTON3    0x0004                   // usually WHEEL, will be
                                                // translated to ENTER key!
#define TXm_DRAGGED    0x0100                   // mouse dragged, button down

#define TXm_KS_NONE    0x0000                   // Shiftstates, modelled
#define TXm_KS_ALT     0x0003                   // after windows, but no
#define TXm_KS_CTRL    0x000C                   // seperate left/right
#define TXm_KS_SHIFT   0x0010
#define TXm_KS_NUMLK   0x0020
#define TXm_KS_SCRLK   0x0040

//- work with TXWM_COMMAND ranges returned by (submenu) lists on selection
#define txwMiValue(b)   ( (mp1-TXDID_MAX)- (b))
#define txwMiRange(b,r) (((mp1-TXDID_MAX)>=(b)) && ((mp1-TXDID_MAX)<((b)+(r))))

//- enable or mark menu-items by ID
#define txwMiEnable(m,i,c,r) TxSetMenuItemFlag((m),(i),TXSF_DISABLED,(r),!(c))
#define txwMiMarked(m,i,c,r) TxSetMenuItemFlag((m),(i),TXSF_MARK_STD,(r), (c))

typedef ULONG  (* TXWINPROC)                    // window procedure
(
   TXWHANDLE           hwnd,
   ULONG               msg,
   ULONG               mp1,
   ULONG               mp2
);

typedef struct txwqmsg
{
   TXWHANDLE           hwnd;
   ULONG               msg;                     // message id
   ULONG               mp1;
   ULONG               mp2;
} TXWQMSG;                                      // end of struct "txwqmsg"

typedef struct tx2d                             // TX two-dimensional entity
{
   short               x;                       // x-coord (horizontal)
   short               y;                       // y-coord (vertical)
} TX2D;                                         // end of struct "tx2d"

typedef TX2D           TXPOINT;                 // TX two-dimensional point


typedef struct txrect                           // TX (normalized) rectangle
{
   short               left;
   short               top;
   short               right;
   short               bottom;
} TXRECT;                                       // end of struct "txrect"


typedef struct txrect_elem                      // RECTANGLE list element
{
   TXRECT              rect;                    // rectangle
   BOOL                skip;                    // invalid element, skip
   struct txrect_elem *next;                    // next pointer
} TXRECT_ELEM;                                  // end of struct "txrect_elem"


typedef struct txcell                           // TX display cell
{
   BYTE                ch;                      // character value
   BYTE                at;                      // color attribute
} TXCELL;                                       // end of struct "txcell"


typedef enum txhselect
{
   TXH_THIS,                                    // current entry, or 0
   TXH_PREV,                                    // next older entry
   TXH_NEXT                                     // next newer entry
} TXHSELECT;                                    // end of enum "txhselect"

#define TXH_NOCURR ((USHORT) 0xffff)            // no current index

typedef struct txhelem                          // history element
{
   ULONG               order;                   // element order value
   char               *string;                  // history string
} TXHELEM;                                      // end of struct "txhelem"

typedef struct txhist                           // line history information
{
   USHORT              hsize;                   // nr of history entries
   USHORT              esize;                   // size of history string
   BOOL                ascend;                  // ascending sort-order
   USHORT              current;                 // current element
   TXHELEM            *elem;                    // history element array
   char               *buf;                     // character buffer
} TXHIST;                                       // end of struct "txhist"

#define TXWSB_VIEWS    4
typedef struct txsbdata                         // scroll buffer data
{
   ULONG               vsize;                   // virtual/view size (lines)
   ULONG               width;                   // width of buffer, linelength
   ULONG               length;                  // length of buffer, #lines
   ULONG               firstline;               // First written line in buf
   ULONG               markRel;                 // Marked line relative to firstline
   short               markCol;                 // startcolumn marked area
   short               markSize;                // size  marked area, or 0 (off)
   BOOL                midscroll;               // scroll when not at end
   BOOL                wrap;                    // Wrap long lines on write
   TXWHANDLE           view;                    // auto-update view window
   TXCELL             *buf;                     // display scroll buffer
} TXSBDATA;                                     // end of struct "txsbdata"

typedef struct txsbview                         // scroll buffer view info
{
   ULONG               topline;                 // index of top-line
   short               leftcol;                 // offset in string for col 0
   ULONG               maxtop;                  // current maximum for topline
   short               maxcol;                  // current maximum for leftcol
   TXSBDATA           *sbdata;                  // related scroll-buffer data
   char               *status;                  // free-format status text
   BYTE                scolor;                  // free-format status color
   ULONG               altcol;                  // alternate color conversions
} TXSBVIEW;                                     // end of struct "txsbview"

typedef struct txentryfield                     // entry field info
{
   short               leftcol;                 // offset in string for col 0
   short               maxcol;                  // current maximum for leftcol
   short               curpos;                  // offset of cursor position
   short               rsize;                   // real allocated size
   BYTE                ccol;                    // cursor bar color
   BYTE                icol;                    // indicator color
   TXHIST             *history;                 // optional history buffer
   char               *buf;                     // entry field buffer
} TXENTRYFIELD;                                 // end of struct "txentryfield"

typedef struct txtextview                       // text viewer
{
   ULONG               topline;                 // index of top-line
   short               leftcol;                 // offset in string for col 0
   ULONG               maxtop;                  // current maximum for topline
   short               maxcol;                  // current maximum for leftcol
   ULONG               markLine;                // startline marked area in text
   short               markCol;                 // startcolumn marked area
   short               markSize;                // size  marked area, or 0 (off)
   char              **buf;                     // display text array
} TXTEXTVIEW;                                   // end of struct "txtextview"

typedef struct txstatic                         // static text
{
   char              **buf;                     // display text array
} TXSTATIC;                                     // end of struct "txstatic"

typedef struct txstline                         // static text, single line
{
   char               *buf;                     // display text array
} TXSTLINE;                                     // end of struct "txstatic"

typedef struct txbutton                         // any button class
{
   BOOL               *checked;                 // button state
   char               *text;                    // button descriptive text
} TXBUTTON;                                     // end of struct "txbutton"

typedef struct txlistbox                        // listbox, popup/submenu
{
   short               cpos;                    // current position   (cursor)
   BYTE                ccol;                    // cursor bar color
   BYTE                icol;                    // indicator color      (spin)
   BYTE                csel;                    // cursor   select color
   BYTE                ssel;                    // standard select color
   BYTE                cmrk;                    // cursor   marked color
   BYTE                smrk;                    // standard marked color
   BYTE                cdis;                    // cursor disabled text
   BYTE                sdis;                    // standard disabled text
   TXSELIST           *list;                    // actual selection list data
} TXLISTBOX;                                    // end of struct "txlistbox"


typedef enum txhexedcmd
{
   TXHE_CB_UPD_DESC,                            // update buffer desc (hex/dec)
   TXHE_CB_INIT_POS,                            // load initial buffers
   TXHE_CB_NEX2_BUF,                            // make 'nex2' buffer current
   TXHE_CB_NEXT_BUF,                            // make 'next' buffer current
   TXHE_CB_PREV_BUF,                            // make 'prev' buffer current
   TXHE_CB_PRE2_BUF,                            // make 'pre2' buffer current
   TXHE_CB_TO_START,                            // to start of object
   TXHE_CB_TO_FINAL,                            // to end of object
   TXHE_CB_GOTOITEM,                            // go to specified item (sn)
   TXHE_CB_FINDDATA,                            // find data, goto found item
   TXHE_CB_FNDAGAIN,                            // find data again, same data
   TXHE_CB_WRITEBUF,                            // write (changed) buffer
   TXHE_CB_INSERT_1,                            // expand object by one byte
   TXHE_CB_DELETE_1,                            // shrink object by one byte
   TXHE_CB_DELTOEND,                            // delete to end-of-file
   TXHE_CB_ALT_DISP,                            // use alternate display format
} TXHEXEDCMD;                                   // end of enum "txhexedcmd"



typedef struct txhebuf                          // hex editor data buffer
{
   BYTE               *data;                    // start of data buffer
   LLONG               start;                   // abs start position (ref)
   ULONG               size;                    // size of buffer    (item)
   TXLN                desc;                    // buffer description  (76)
} TXHEBUF;                                      // end of struct "txhebuf"


typedef struct txhexedit                        // hex editor control
{
   //- note: a negative offset in the buffer means the current item (buffer)
   //-       is NOT at the top of the display, but further down, and part of
   //-       the PREV buffer is also visible. (0 = buffer start at top-left)

   LONG                posCurBuf;               // offset in buffer, 1st byte
   LONG                posCursor;               // offset in screen, cursor
   USHORT              hexLsb;                  // hex LSB nibble   (0 or 1)
   USHORT              rows;                    // nr of rows shown    (16)
   USHORT              cols;                    // nr of bytes per row (16)
   BOOL                autocols;                // get 8/16/32/cols from width
   BOOL                ascii;                   // cursor in ascii area
   BOOL                decimal;                 // use decimal positions
   TXHEBUF            *pre2;                    // buffer BEFORE prev    one
   TXHEBUF            *prev;                    // buffer BEFORE current one
   TXHEBUF            *curr;                    // current buffer (item)
   TXHEBUF            *next;                    // buffer AFTER  current one
   TXHEBUF            *nex2;                    // buffer AFTER  next    one
   BYTE               *diff;                    // difference buf (original)
   ULONG               diffSize;                // size of difference buf
   ULONG               currCrc;                 // 32-bit buf CRC (original)
   LONG                markStart;               // start marked area in curr
   LONG                markSize;                // size  marked area in bytes
   LLONG               markBase;                // valid curr->start for mark
   void               *userdata;                // application user data
   void               *actionCallback;          // application action handler
   BOOL                altStart;                // start in alternate view
   TXTS                altDispText;             // alt display fmt menu text
} TXHEXEDIT;                                    // end of struct "txhexedit"


typedef ULONG (* TXHE_CALLBACK)                 // Hex editor action callback
(
   ULONG               cmd,                     // IN    action command
   TXHEXEDIT          *data                     // INOUT hexedit data
);


// Windows control data, used at runtime
typedef struct txwindow                         // TX window definitions
{
   TXRECT              client;                  // client window dimensions
   TXRECT              border;                  // border window dimensions
   TXCELL              clientclear;             // default client cell
   TXCELL              borderclear;             // default border cell
   ULONG               style;                   // window & border styles
   ULONG               helpid;                  // compound helpid
   char               *title;                   // window title
   char               *trhtext;                 // window title right-hand text
   BYTE                titlecolor;              // title color
   BYTE                titlefocus;              // title focus color
   char               *footer;                  // footer border text
   char               *frhtext;                 // window footer right-hand text
   BYTE                footercolor;             // footer color
   BYTE                footerfocus;             // footer focus color
   USHORT              dlgFocusID;              // ID of child to get focus
   ULONG               dlgResult;               // result when used as dialog
   TXWCLASS            class;                   // class of window
   union                                        // class-dependant data
   {
      TXSBVIEW         sb;                      // scroll-buffer view
      TXENTRYFIELD     ef;                      // entryfield 1-line
      TXTEXTVIEW       tv;                      // text viewer (scrollable)
      TXSTATIC         st;                      // static text
      TXSTLINE         sl;                      // static text, single line
      TXBUTTON         bu;                      // push- radio or checkbox
      TXLISTBOX        lb;                      // listbox with selection list
      TXHEXEDIT        he;                      // hex editor control
   };
} TXWINDOW;                                     // end of struct "txwindow"



// Widget control data, used for IO (setup and dialog results)

#define TXWI_DISABLED   0x00000001              // widget is disabled

typedef struct txwidget                         // TX (generic) widget
{                                               // extended subset of WINDOW
   short               vpos;                    // UL corner, line   (relative)
   short               hpos;                    // UL corner, column (relative)
   short               vsize;                   // vertical size
   short               hsize;                   // horizontal size
   ULONG               flags;                   // widget creation flags
   USHORT              group;                   // group-id (radio ...)
   USHORT              winid;                   // specific window-id or 0
   ULONG               style;                   // control style bits
   ULONG               helpid;                  // specific helpid or 0
   char               *title;                   // window title (ef, tv, lb)
   TXWCLASS            class;                   // class of window
   TXWHANDLE           hwnd;                    // created widget window
   TXWINPROC           winproc;                 // window procedure or NULL
   //- changed from anonymous union to allow
   //- static initializers using TXWg macros
   TXENTRYFIELD        ef;                      // entryfield 1-line
   TXTEXTVIEW          tv;                      // text viewer (scrollable)
   TXSTATIC            st;                      // static text
   TXSTLINE            sl;                      // static text, single line
   TXBUTTON            bu;                      // push- radio or checkbox
   TXLISTBOX           lb;                      // listbox with selection list
} TXWIDGET;                                     // end of struct "txwidget"

// static init macros for each control type: {ef},{tv},{st},{sl},{bu},{lb}
#define TXWgEntryfield(l,m,p,r,c,i,h,b)  {l,m,p,r,c,i,h,b},{0},{0},{0},{0},{0}
#define TXWgTextview(t,l,m,c,text)       {0},{t,l,m,c,0,0,0,text},{0},{0},{0},{0}
#define TXWgStatic(str)                  {0},{0},{str},{0},{0},{0}
#define TXWgStline(str)                  {0},{0},{0},{str},{0},{0}
#define TXWgButton(boolean,str)          {0},{0},{0},{0},{boolean,str},{0}
#define TXWgListbox(p,c,i,s,t,m,n,d,e,l) {0},{0},{0},{0},{0},{p,c,i,s,t,m,n,d,e,l}

//- specific widget macro's to save line-space
#define TXStdEntryf(v,s,text) text, TXW_ENTRYFIELD, 0, NULL, TXWgEntryfield(0,0,0,(s),0,0,NULL,(v))
#define TXStdStatic(    text) "",   TXW_STATIC,     0, NULL, TXWgStatic(text)
#define TXStdStline(    text) "",   TXW_STLINE,     0, NULL, TXWgStline(text)
#define TXStdButton(var,text) "",   TXW_BUTTON,     0, NULL, TXWgButton(var,text)
#define TXStdLstBox(v,p,text) text, TXW_LISTBOX,    0, NULL, TXWgListbox((p),0,0,0,0,0,0,0,0,(v))

// special help-id's
#define TXWH_USE_CMD_CODE     0x00000000        // cmd-code as help-id  (lists)
#define TXWH_USE_WIN_HELP     0x00000001        // use helpid on window (lists)
#define TXWH_USE_OWNER_HELP   0x00000002        // use owner helpid   (submenu)
#define TXWH_NO_MORE_HELP     0xfffffffd        // no more help  (stop recurse)

// TXWS_ style bits for window and border
#define TXWS_TITLEBORDER      0x00000001        // use a title-border at top
#define TXWS_SIDEBORDERS      0x00000002        // use side borders  left/right
#define TXWS_FOOTRBORDER      0x00000004        // use footer-border at bottom
#define TXWS_BORDERLINES      0x00000008        // use lines in border
#define TXWS_BRACKETSIDE      0x00000010        // use [] with no borderlines
#define TXWS_LEFTJUSTIFY      0x00000020        // left-justify title & footer
#define TXWS_TF_TEXTONLY      0x00000040        // text-only    title & footer
#define TXWS_FOCUS_PAINT      0x00000080        // paint on set-focus required
#define TXWS_VISIBLE          0x00000100        // window could be visible
#define TXWS_DISABLED         0x00000200        // window is disabled
#define TXWS_SAVEBITS         0x00000400        // save underlying cells
#define TXWS_MOVEABLE         0x00000800        // can be moved/sized by user
#define TXWS_HCHILD2SIZE      0x00001000        // H half resize with parent
#define TXWS_HCHILD_SIZE      0x00002000        // H full resize with parent
#define TXWS_HCHILD2MOVE      0x00004000        // H half move child on resize
#define TXWS_HCHILD_MOVE      0x00008000        // H full move child on resize
#define TXWS_VCHILD2SIZE      0x00010000        // V half resize with parent
#define TXWS_VCHILD_SIZE      0x00020000        // V full resize with parent
#define TXWS_VCHILD2MOVE      0x00040000        // V half move child on resize
#define TXWS_VCHILD_MOVE      0x00080000        // V full move child on resize
#define TXWS_TAB_2_OWNER      0x00100000        // Pass Tab/sTab to owner (menu)
#define TXWS_CAST_SHADOW      0x00200000        // Let window cast a shadow
#define TXWS_SYNCPAINT        0x00400000        // paint on invalidate

#define TXWS_FOURBORDERS (TXWS_TITLEBORDER | TXWS_SIDEBORDERS | TXWS_FOOTRBORDER)
#define TXWS_CHILD_BASE  (TXWS_SYNCPAINT   | TXWS_VISIBLE)
#define TXWS_CHILDWINDOW (TXWS_CHILD_BASE  | TXWS_BORDERLINES)
#define TXWS_STDWINDOW   (TXWS_CHILDWINDOW | TXWS_SAVEBITS)

// TXxS_ style bits for control flags (B = button, E = entryfield ...)
// The 8 high-order bits in the style are reserved for controls
// These bits are NOT unique for different control classes!

// TXBS_ button control styles
#define TXBS_PUSHBUTTON       0x00000000        // push  button
#define TXBS_RADIOBUTTON      0x01000000        // radio button
#define TXBS_AUTORADIO        0x05000000        // automatic radio button
#define TXBS_CHECKBOX         0x02000000        // checkbox
#define TXBS_AUTOCHECK        0x06000000        // automatic checkbox
#define TXBS_AUTOMATIC        0x04000000        // automatic draw variant
#define TXBS_PRIMARYSTYLES    0x07000000        // primary styles mask
#define TXBS_HELP             0x08000000        // Help button (TXWM_HELP)
#define TXBS_DLGE_BUTTON      0x10000000        // handle Enter in Dialog


// TXES_ entryfield control styles
#define TXES_LEFT             0x00000000        // left justified
#define TXES_CENTER           0x01000000        // centered
#define TXES_RIGHT            0x02000000        // right justified
#define TXES_UNREADABLE       0x04000000        // unreadable field (password)
#define TXES_DLGE_FIELD       0x10000000        // handle Esc/Enter in Dialog
#define TXES_MAIN_CMDLINE     0x80000000        // entryfield for main cmdline
                                                // routes certain events to it

// TXLS_ listbox control styles
#define TXLS_DROP_MENU        0x01000000        // menu heading drop down
#define TXLS_DROP_VALUE       0x02000000        // value drop-down
#define TXLS_AUTO_DROP        0x04000000        // automatic drop on focus
#define TXLS_SPIN_WRAP        0x08000000        // wrap arround on spin-usage
#define TXLS_FOOT_COUNT       0x20000000        // display #items in footer
#define TXLS_CHAR_ENTER       0x40000000        // LN_ENTER on select-char
#define TXLS_DROP_ENTER       0x80000000        // drop-down on Enter too, no
                                                // select and LN_ENTER

// TXCS_ styles shared between canvas, frame and textview controls
#define TXCS_CLOSE_BUTTON     0x01000000        // include close button in border
#define TXCS_P_LISTBOX        0x10000000        // canvas for a submenu
#define TXCS_LIST_POPUP       0x20000000        // canvas for single popup list
#define TXCS_MAIN_MENU        0x80000000        // canvas for a main menu

// TXVS_ text view control styles
#define TXVS_CLOSE_BUTTON     0x01000000        // include close button in border

// TXHE_ hex editor control styles
#define TXHE_CLOSE_BUTTON     0x01000000        // include close button in border
#define TXHE_WRITE_PROMPT     0x10000000        // prompt before write-back
#define TXHE_TAB_HTOGGLE      0x20000000        // TAB toggles hex/ascii

//- often used Dialog frame/child window combinations
#define TXWS_CANVAS   TXWS_DISABLED    | TXWS_VISIBLE      | TXWS_SAVEBITS    \
                                       | TXWS_SYNCPAINT
#define TXWS_DIALOG   TXWS_STDWINDOW   | TXWS_FOURBORDERS
#define TXWS_DFRAME   TXWS_DIALOG      | TXWS_DISABLED     | TXWS_MOVEABLE
#define TXWS_FRAMED   TXWS_CHILDWINDOW | TXWS_FOURBORDERS
#define TXWS_SFRAMED  TXWS_CHILDWINDOW | TXWS_SIDEBORDERS
#define TXWS_SBUTTON  TXWS_SFRAMED     | TXBS_PUSHBUTTON   | TXWS_FOCUS_PAINT
#define TXWS_PBUTTON  TXWS_FRAMED      | TXBS_PUSHBUTTON   | TXWS_FOCUS_PAINT
#define TXWS_AUTORAD  TXWS_CHILDWINDOW | TXBS_AUTORADIO    | TXWS_FOCUS_PAINT
#define TXWS_RADIOB   TXWS_CHILDWINDOW | TXBS_RADIOBUTTON  | TXWS_FOCUS_PAINT
#define TXWS_AUTOCHK  TXWS_CHILDWINDOW | TXBS_AUTOCHECK    | TXWS_FOCUS_PAINT
#define TXWS_CHECKB   TXWS_CHILDWINDOW | TXBS_CHECKBOX     | TXWS_FOCUS_PAINT
#define TXWS_EFOCUS   TXWS_LEFTJUSTIFY | TXWS_BRACKETSIDE  | TXWS_FOCUS_PAINT
#define TXWS_EF_BASE  TXWS_EFOCUS      | TXES_DLGE_FIELD
#define TXWS_ENTRYF   TXWS_CHILD_BASE  | TXWS_EF_BASE      | TXWS_SIDEBORDERS
#define TXWS_ENTRYT   TXWS_ENTRYF      | TXWS_TITLEBORDER
#define TXWS_ENTRYB   TXWS_CHILDWINDOW | TXWS_EF_BASE      | TXWS_SIDEBORDERS \
                                       | TXWS_BRACKETSIDE
#define TXWS_ENTRYBT  TXWS_ENTRYB      | TXWS_TITLEBORDER  | TXWS_TF_TEXTONLY
#define TXWS_OUTPUT   TXWS_CHILD_BASE  | TXWS_DISABLED     | TXWS_LEFTJUSTIFY
#define TXWS_OUTPUTT  TXWS_OUTPUT      | TXWS_TITLEBORDER
#define TXWS_DROPMENU TXWS_CHILD_BASE  | TXLS_DROP_MENU
#define TXWS_A_MENU   TXWS_DROPMENU    | TXWS_TITLEBORDER
#define TXWS_D_DOWN   TXWS_CHILD_BASE  | TXWS_TITLEBORDER  | TXLS_DROP_VALUE
#define TXWS_D_SPIN   TXWS_D_DOWN      | TXWS_BRACKETSIDE  | TXWS_SIDEBORDERS \
                                       | TXLS_SPIN_WRAP    | TXWS_FOCUS_PAINT
#define TXWS_DSPINB   TXWS_CHILDWINDOW | TXLS_DROP_VALUE   | TXWS_BRACKETSIDE \
                    | TXWS_SIDEBORDERS | TXLS_SPIN_WRAP    | TXWS_FOCUS_PAINT
#define TXWS_DSPINBT  TXWS_DSPINB      | TXWS_TITLEBORDER  | TXWS_TF_TEXTONLY

#define TXWS_HEXECTL  TXWS_CHILDWINDOW | TXHE_TAB_HTOGGLE
#define TXWS_HEXEDIT  TXWS_HEXECTL     | TXHE_CLOSE_BUTTON | TXWS_FOURBORDERS

// TXWM_COMMAND msg source codes
#define TXCMDSRC_PUSHBUTTON      1
#define TXCMDSRC_MENU            2
#define TXCMDSRC_ACCELERATOR     3
#define TXCMDSRC_FONTDLG         4
#define TXCMDSRC_FILEDLG         5
#define TXCMDSRC_PRINTDLG        6
#define TXCMDSRC_COLORDLG        7
#define TXCMDSRC_RADIOBUTTON     8
#define TXCMDSRC_CHECKBOX        9
#define TXCMDSRC_OTHER           0


// A linestyle is defined by a 48 character string in 4 groups of 12
// LINESSTD, LINESHALF and LINESFULL (see colorscheme) are predefined
// and LINESCUSTOM can be defined using the txwSetLinesCustom function
//
// TXW linestyle groups
#define TXLP_STANDARD (TXLP_POS * 0)            // no special condition
#define TXLP_FOCUS    (TXLP_POS * 1)            // window has focus
#define TXLP_ACTIVE   (TXLP_POS * 2)            // window is active (dialog)
#define TXLP_SELECTED (TXLP_POS * 3)            // window is selected
#define TXLP_SIZE     (TXLP_POS * 4)            // total length of string

// TXW linestyle position definitions
#define TXLP_LFT              0                 //        left   side
#define TXLP_TLC              1                 // top    left   corner
#define TXLP_TOP              2                 // top
#define TXLP_TRC              3                 // top    right  corner
#define TXLP_RGT              4                 //        right  side
#define TXLP_BLC              5                 // bottom left   corner
#define TXLP_BOT              6                 // bottom
#define TXLP_BRC              7                 // bottom right  corner
#define TXLP_TXL              8                 // text   left   stop
#define TXLP_TXR              9                 // text   right  stop
#define TXLP_L1L             10                 // single line, left
#define TXLP_R1L             11                 // single line, right
#define TXLP_POS             12                 // nr of items in string


// TXW position option bits                        (SetWindowPos)
#define TXSWP_SIZE            0x00000001        // resize window (abs/relative)
#define TXSWP_MOVE            0x00000002        // move window (rel to parent)
#define TXSWP_ABSOLUTE        0x00010000        // move window absolute to scr
#define TXSWP_RELATIVE        0x00020000        // move window relative to self


// window flags definitions
#define TXFF_ACTIVE           0x0002            // active window (dialog)
#define TXFF_DLGDISMISSED     0x0010            // dialog is dismissed
#define TXFF_AUTODROPNEXT     0x0020            // delay autodrop till next time
#define TXFF_SELECTED         0x0040            // window is selected
#define TXFF_MINIMIZED        0x0080            // window is minimized to title
                                                // size stored in _X and _Y us

#define TXQWS_USER            0                 // user definable ushort
#define TXQWS_ID              1                 // window id
#define TXQWS_X               2                 // x value
#define TXQWS_Y               3                 // y value
#define TXQWS_FLAGS           4                 // window flags
#define TXQWS_GROUP           5                 // dialog control group
#define TXQWS_SIZE            6                 // nr of window ushorts

#define TXQWL_USER            0                 // user definable ulong
#define TXQWP_USER            1                 // user definable pointer
#define TXQWP_DATA            2                 // alternate user pointer
#define TXQWL_SIZE            3                 // nr of window ulongs and ptrs


// Predefined dialog id values
#define TXDID_OK                     1
#define TXDID_CANCEL                 2
#define TXDID_MAX                    100        // used as base offset in lists
#define TXDID_ERROR                  0xffff

extern  char txwstd_footer[];                   // default dialog footer text

// MessageBox return values and flag definitions
#define TXMBID_OK                    TXDID_OK
#define TXMBID_CANCEL                TXDID_CANCEL
#define TXMBID_ABORT                 3
#define TXMBID_RETRY                 4
#define TXMBID_IGNORE                5
#define TXMBID_YES                   6
#define TXMBID_NO                    7
#define TXMBID_HELP                  8
#define TXMBID_ENTER                 9
#define TXMBID_FAIL                  10
#define TXMBID_ERROR                 TXDID_ERROR

#define TXMB_OK                      0x0000
#define TXMB_OKCANCEL                0x0001
#define TXMB_RETRYCANCEL             0x0002
#define TXMB_ABORTRETRYIGNORE        0x0003
#define TXMB_YESNO                   0x0004
#define TXMB_YESNOCANCEL             0x0005
#define TXMB_CANCEL                  0x0006
#define TXMB_ENTER                   0x0007
#define TXMB_ENTERCANCEL             0x0008
#define TXMB_RETRYFAIL               0x000a
#define TXMB_PRIMARYSTYLES           0x000f

#define TXMB_NOICON                  0x0000
#define TXMB_QUERY                   0x0010
#define TXMB_WARNING                 0x0020
#define TXMB_INFORMATION             0x0030
#define TXMB_ERROR                   0x0040
#define TXMB_ANYICON                 0x00f0

#define TXMB_DEFBUTTON1              0x0000
#define TXMB_DEFBUTTON2              0x0100
#define TXMB_DEFBUTTON3              0x0200
#define TXMB_DEFBUTTON4              0x0400

#define TXMB_APPLMODAL               0x0000
#define TXMB_HELP                    0x1000
#define TXMB_MOVEABLE                0x2000
#define TXMB_HCENTER                 0x4000
#define TXMB_VCENTER                 0x8000


// WidgetDialog return values and flag definitions
#define TXWD_MOVEABLE                0x2000
#define TXWD_HCENTER                 0x4000
#define TXWD_VCENTER                 0x8000

// PromptBox return values and flag definitions
#define TXPB_MOVEABLE                0x2000
#define TXPB_HCENTER                 0x4000
#define TXPB_VCENTER                 0x8000

// MenuBar flag definitions
#define TXMN_DELAY_AUTODROP          0x0001     // do not use 1st time on this
#define TXMN_NO_AUTODROP             0x0002     // no autodrop at all
#define TXMN_MAIN_MENU               0x0004     // menu is the main menu

// ListBox return values and flag definitions
#define TXLB_MOVEABLE                0x2000     // move & resize allowed
#define TXLB_P_LISTBOX               0x4000     // popup from other list
#define TXLB_P_FIX_ROW               0x8000     // fixed row position

// Listbox notification codes
#define TXLN_SELECT                  1
#define TXLN_SETFOCUS                2
#define TXLN_KILLFOCUS               3
#define TXLN_SCROLL                  4
#define TXLN_ENTER                   5

// Hex edit tandard dialog flags
#define TXHE_MOVEABLE                0x2000

// Scroll-buffer related color conversions
#define TXSB_COLOR_NORMAL       0x0000          // no conversions
#define TXSB_COLOR_INVERT       0x0001          // invert colors, filter FG
#define TXSB_COLOR_BRIGHT       0x0002          // use bright FG colors only
#define TXSB_COLOR_B2BLUE       0x0004          // Change all Black BG to Blue
#define TXSB_COLOR_MAXVAL       0x0007          // Maximum color-style value
#define TXSB_COLOR_MASK         0x0007          // Bitmask to limit style value


#define TXWCS_QUERY_ONLY          0
#define TXWCS_FIRST_SCHEME        1
#define TXWCS_STANDARD            1
#define TXWCS_STD_HALF            2
#define TXWCS_STD_FULL            3
#define TXWCS_NO_BLINK            4
#define TXWCS_3D_GREY             5
#define TXWCS_3D_HALF             6
#define TXWCS_3D_FULL             7
#define TXWCS_COMMANDER           8
#define TXWCS_3D_CMDR             9
#define TXWCS_HALF_CMDR          10
#define TXWCS_FULL_CMDR          11
#define TXWCS_3D_WHITE           12
#define TXWCS_BNW_ONLY           13
#define TXWCS_BNW_FULL           14
#define TXWCS_BNW_HALF           15
#define TXWCS_LAST_SCHEME        15
#define TXWCS_NEXT_SCHEME       200
#define TXWCS_PREV_SCHEME       201

#define TXW_MAX_CSI             256             // allow full byte indexing
#define TXW_CS_SIGNATURE        0xfacefeed      // SC validation signature

#define TXW_CS_STDLINES           0             // Use standard lines, cp437
#define TXW_CS_3D_LINES           1             // Use 3D style lines, single
#define TXW_CS_HALF               2             // Use half/full solid blocks
#define TXW_CS_FULL               3             // Use full solid/grey blocks
#define TXW_CS_CUST               4             // Use custom specified lines
#define TXW_CS_LAST               4             // last linestyle value

#define TXW_CSF_NORMAL            0x0000
#define TXW_CSF_LIST_UNDERLINE    0x0001        // underline spaces in select
                                                // lists selected-item line
typedef BYTE TXW_COLORMAP[TXW_MAX_CSI];

typedef struct txw_colorscheme
{
   ULONG               signature;
   ULONG               linestyle;               // colorscheme linestyle
   ULONG               sbcolors;                // default scrollbuffer colors
   ULONG               csflags;                 // colorscheme flags
   TXTT                name;                    // short name (std 19)
   TXTM                desc;                    // longer description
   BYTE               *color;                   // scheme color mapping
} TXW_COLORSCHEME;                              // end struct "txw_colorscheme"


extern  TXW_COLORSCHEME     *txwcs;             // current color scheme

#define TxwSC(sc)    (BYTE) (        txwcs->color[(int)(sc)])
#define TxwAC(sc,ac) (BYTE) (((ac) ? txwcs->color[(int)(ac)] : txwcs->color[(int)(sc)]))

#define TXWSCHEME_COLORS cSchemeColor,TXWSCHEME_COLOR5
#define TXWSCHEME_COLOR5 cSchemeColor,TXWSCHEME_COLOR4
#define TXWSCHEME_COLOR4 cSchemeColor,TXWSCHEME_COLOR3
#define TXWSCHEME_COLOR3 cSchemeColor,TXWSCHEME_COLOR2
#define TXWSCHEME_COLOR2 cSchemeColor,cSchemeColor

#define TXWSCHEME_ERRMSG cWinClientNr4

typedef enum txw_wincolors                      // color scheme entity indexes
{
   cSchemeColor,                                // Use appropriate scheme color
   cWinClientClr,                               // Client clear color
   cWinClientNr0,                               // first alternate client color
   cWinClientNr1,
   cWinClientNr2,
   cWinClientNr3,
   cWinClientNr4,
   cWinClientNr5,
   cWinClientNr6,
   cWinClientNr7,
   cWinBorder_top,                              // top
   cWinBorder_trc,                              // top    right  corner
   cWinBorder_rgt,                              //        right  side
   cWinBorder_brc,                              // bottom right  corner
   cWinBorder_bot,                              // bottom
   cWinBorder_blc,                              // bottom left   corner
   cWinBorder_lft,                              // left
   cWinBorder_tlc,                              // top    left   corner
   cWinTitleStand,                              // title color
   cWinTitleFocus,                              // title with focus
   cWinFooterStand,                             // footer color
   cWinFooterFocus,                             // footer with focus
   cDlgBorder_top,                              // Dialog control borders
   cDlgBorder_trc,                              //
   cDlgBorder_rgt,                              //
   cDlgBorder_brc,                              //
   cDlgBorder_bot,                              //
   cDlgBorder_blc,                              //
   cDlgBorder_lft,                              //
   cDlgBorder_tlc,                              // Dialog control borders
   cDlgTitleStand,                              // title color
   cDlgTitleFocus,                              // title with focus
   cDlgFooterStand,                             // footer color
   cDlgFooterFocus,                             // footer with focus
   cPushBorder_top,                             // Push-Button borders
   cPushBorder_trc,                             //
   cPushBorder_rgt,                             //
   cPushBorder_brc,                             //
   cPushBorder_bot,                             //
   cPushBorder_blc,                             //
   cPushBorder_lft,                             //
   cPushBorder_tlc,                             // Push-Button borders
   cPushTitleStand,                             // title color
   cPushTitleFocus,                             // title with focus
   cPushFooterStand,                            // footer color
   cPushFooterFocus,                            // footer with focus
   cViewBorder_top,                             // TextView borders, like help
   cViewBorder_trc,                             //
   cViewBorder_rgt,                             //
   cViewBorder_brc,                             //
   cViewBorder_bot,                             //
   cViewBorder_blc,                             //
   cViewBorder_lft,                             //
   cViewBorder_tlc,                             // TextView borders, like help
   cViewTitleStand,                             // title color
   cViewTitleFocus,                             // title with focus
   cViewFooterStand,                            // footer color
   cViewFooterFocus,                            // footer with focus
   cSbvBorder_top,                              // ScrollBufferView borders
   cSbvBorder_trc,                              //
   cSbvBorder_rgt,                              //
   cSbvBorder_brc,                              //
   cSbvBorder_bot,                              //
   cSbvBorder_blc,                              //
   cSbvBorder_lft,                              //
   cSbvBorder_tlc,                              // ScrollBufferView borders
   cSbvTitleStand,                              // SB-view title color
   cSbvTitleFocus,                              // SB-view title with focus
   cSbvFooterStand,                             // SB-view footer color
   cSbvFooterFocus,                             // SB-view footer with focus
   cSbvStatusStand,                             // SB-view status color
   cSbvStatusFocus,                             // SB-view status with focus
   cSbvProgreStand,                             // SB-view progress status
   cSbvTraceStand,                              // SB-view trace info
   cSbvMarkedArea,                              // SB-view marked area
   cDskTraceStand,                              // Desktop trace info
   cDskBorder_top,                              // Desktop borders
   cDskBorder_trc,                              //
   cDskBorder_rgt,                              //
   cDskBorder_brc,                              //
   cDskBorder_bot,                              //
   cDskBorder_blc,                              //
   cDskBorder_lft,                              //
   cDskBorder_tlc,                              // Desktop borders
   cDskTitleStand,                              // Desktop title
   cDskTitleFocus,
   cDskFooterStand,                             // Desktop footer/status
   cDskFooterFocus,
   cLineTextStand,                              // static line text
   cLineTextNr0,                                // line alternate 0
   cLineTextNr1,
   cLineTextNr2,
   cLineTextNr3,
   cLineTextNr4,
   cLineTextNr5,
   cLineTextNr6,
   cLineTextNr7,
   cHelpTextStand,                              // help viewer text
   cHelpTextHelp,                               // help on help viewer text
   cViewTextStand,                              // text viewer text
   cViewTextMark,                               // text viewr (search) mark
   cViewTextNr1,                                // text viewer alternate 1
   cViewTextNr2,
   cViewTextNr3,
   cViewTextNr4,
   cViewTextNr5,
   cViewTextNr6,
   cViewTextNr7,
   cListTextStand,                              // cclr text
   cListTextFocus,                              // ccol
   cListSelectStand,                            // ssel quick selection char
   cListSelectFocus,                            // csel
   cListMarkStand,                              // smrk mark character
   cListMarkFocus,                              // cmrk
   cListDisableStand,                           // sdis disabled text
   cListDisableFocus,                           // cdis
   cListSeparatStand,                           // separator line
   cListSeparatFocus,
   cListCountStand,                             // Footer count value
   cListCountFocus,
   cFileTextStand,                              // text
   cFileTextFocus,
   cFileSelectStand,                            // quick selection char
   cFileSelectFocus,
   cFileMarkStand,                              // mark character
   cFileMarkFocus,
   cFileDisableStand,                           // disabled text
   cFileDisableFocus,
   cFileSeparatStand,                           // separator line
   cFileSeparatFocus,
   cFileCountStand,                             // File list counters
   cFileCountFocus,
   cFileBorder_top,                             // File list borders
   cFileBorder_trc,
   cFileBorder_rgt,
   cFileBorder_brc,
   cFileBorder_bot,
   cFileBorder_blc,
   cFileBorder_lft,
   cFileBorder_tlc,                             // File list borders
   cFileTitleStand,                             // File list title/footer
   cFileTitleFocus,
   cFileFooterStand,
   cFileFooterFocus,
   cApplTextStand,                              // text
   cApplTextFocus,
   cApplSelectStand,                            // quick selection char
   cApplSelectFocus,
   cApplMarkStand,                              // mark character
   cApplMarkFocus,
   cApplDisableStand,                           // disabled text
   cApplDisableFocus,
   cApplSeparatStand,                           // separator line
   cApplSeparatFocus,
   cMenuTextStand,                              // text
   cMenuTextFocus,
   cMenuSelectStand,                            // quick selection char
   cMenuSelectFocus,
   cMenuMarkStand,                              // mark character
   cMenuMarkFocus,
   cMenuDisableStand,                           // disabled text
   cMenuDisableFocus,
   cMenuSeparatStand,                           // separator lines
   cMenuSeparatFocus,
   cMenuBorder_top,                             // menu borders
   cMenuBorder_trc,
   cMenuBorder_rgt,
   cMenuBorder_brc,
   cMenuBorder_bot,
   cMenuBorder_blc,
   cMenuBorder_lft,
   cMenuBorder_tlc,                             // menu borders
   cMbarBorder_top,                             // menu-bar + heading borders
   cMbarBorder_trc,
   cMbarBorder_rgt,
   cMbarBorder_brc,
   cMbarBorder_bot,
   cMbarBorder_blc,
   cMbarBorder_lft,
   cMbarBorder_tlc,                             // menu-bar + heading borders
   cMbarTextStand,                              // menubar text & border
   cMbarTextFocus,
   cMbarHeadStand,                              // menubar header text
   cMbarHeadFocus,
   cSpinTextStand,                              // Spin value text
   cSpinTextFocus,                              //
   cSpinSelectStand,                            // quick selection char
   cSpinSelectFocus,
   cSpinMarkStand,                              // mark character
   cSpinMarkFocus,
   cSpinDisableStand,                           // disabled text
   cSpinDisableFocus,
   cSpinSeparatStand,                           // separator line
   cSpinSeparatFocus,
   cSpinIndcStand,                              // icol indicator
   cSpinIndcFocus,                              //
   cPushTextStand,                              // Push button text
   cPushTextFocus,
   cRadioValueStand,                            // Radio button value
   cRadioValueFocus,
   cRadioTextStand,                             // Radio button text
   cRadioTextFocus,
   cCheckValueStand,                            // Check button value
   cCheckValueFocus,
   cCheckTextStand,                             // Check button text
   cCheckTextFocus,
   cDlgEfTextStand,                             // Dialog Entry field text
   cDlgEfTextFocus,
   cEntryTextStand,                             // Entry field text
   cEntryTextFocus,
   cEntryHistStand,                             // Entry field history
   cEntryHistFocus,
   cEntrBorder_top,                             // entryfield borders
   cEntrBorder_trc,
   cEntrBorder_rgt,
   cEntrBorder_brc,
   cEntrBorder_bot,
   cEntrBorder_blc,
   cEntrBorder_lft,
   cEntrBorder_tlc,                             // entryfield borders
   cEntrTitleStand,
   cEntrTitleFocus,
   cEntrFooterStand,
   cEntrFooterFocus,
   cMLEntTextStand,                             // Multi-line entry field text
   cMLEntTextFocus,
   cHexEdCurByteChg,                            // Hex Editor control
   cHexEdCursorByte,
   cHexEdHexByteStd,
   cHexEdHexByteChg,
   cHexEdHexByteMrk,
   cHexEdHexByteMch,
   cHexEdAscByteStd,
   cHexEdAscBracket,
   cHexEdRelPosPrev,
   cHexEdRelPosCurr,
   cHexEdRelPosNext,
   cHexEdAbsBytePos,
   cHexEdAbsByteCur,
   cHexEdRelCursorP,
   cHexEdButtonText,
   cHexEdButBracket,
   cHexEdByteNumber,
   cHexEdItemSnText,
   cHexEdItemHlight,
   cHexEdModifyText,
   cNrOfSchemeColors
} TXW_WINCOLORS;                                // end of enum "txw_wincolors"

// PC-specific color definitions, compatible with OS/2, DOS and Win-NT
// With TX translations also usable for ANSI (Unix) environments

// Attribute intensity bit
#define  TXwINT  ((BYTE) 0x08)

// Attribute value for color, starting with the default White-on-Black
#define  TXwCNN  ((BYTE) 0x07)

// Attribute values for foreground colors
#define  TXwCNZ  ((BYTE) 0x00)
#define  TXwCNB  ((BYTE) 0x01)
#define  TXwCNG  ((BYTE) 0x02)
#define  TXwCNC  ((BYTE) 0x03)
#define  TXwCNR  ((BYTE) 0x04)
#define  TXwCNM  ((BYTE) 0x05)
#define  TXwCNY  ((BYTE) 0x06)
#define  TXwCNW  ((BYTE) 0x07)
#define  TXwCBZ  ((BYTE) 0x08)
#define  TXwCBB  ((BYTE) 0x09)
#define  TXwCBG  ((BYTE) 0x0A)
#define  TXwCBC  ((BYTE) 0x0B)
#define  TXwCBR  ((BYTE) 0x0C)
#define  TXwCBM  ((BYTE) 0x0D)
#define  TXwCBY  ((BYTE) 0x0E)
#define  TXwCBW  ((BYTE) 0x0F)

// Attribute values for background colors
#define  TXwCnZ  ((BYTE) 0x00)
#define  TXwCnB  ((BYTE) 0x10)
#define  TXwCnG  ((BYTE) 0x20)
#define  TXwCnC  ((BYTE) 0x30)
#define  TXwCnR  ((BYTE) 0x40)
#define  TXwCnM  ((BYTE) 0x50)
#define  TXwCnY  ((BYTE) 0x60)
#define  TXwCnW  ((BYTE) 0x70)
#define  TXwCbZ  ((BYTE) 0x80)
#define  TXwCbB  ((BYTE) 0x90)
#define  TXwCbG  ((BYTE) 0xA0)
#define  TXwCbC  ((BYTE) 0xB0)
#define  TXwCbR  ((BYTE) 0xC0)
#define  TXwCbM  ((BYTE) 0xD0)
#define  TXwCbY  ((BYTE) 0xE0)
#define  TXwCbW  ((BYTE) 0xF0)

// Some combined foreground and background colors with readable names :-)
#define  TX_Default             TXwCNN
#define  TX_Inverse            (TXwCnW + TXwCNZ)
#define  TX_Invisible          (TXwCnW + TXwCNW)
#define  TX_White_on_White     (TXwCnW + TXwCNW)
#define  TX_Lwhite_on_White    (TXwCnW + TXwCBW)
#define  TX_Black_on_White     (TXwCnW + TXwCNZ)
#define  TX_Blue_on_White      (TXwCnW + TXwCNB)
#define  TX_Green_on_White     (TXwCnW + TXwCNG)
#define  TX_Cyan_on_White      (TXwCnW + TXwCNC)
#define  TX_Lcyan_on_White     (TXwCnW + TXwCBC)
#define  TX_Red_on_White       (TXwCnW + TXwCNR)
#define  TX_Magenta_on_White   (TXwCnW + TXwCNM)
#define  TX_Brown_on_White     (TXwCnW + TXwCNY)
#define  TX_Yellow_on_White    (TXwCnW + TXwCBY)
#define  TX_Grey_on_White      (TXwCnW + TXwCBZ)
#define  TX_Yellow_on_Cyan     (TXwCnC + TXwCBY)
#define  TX_Grey_on_Cyan       (TXwCnC + TXwCBZ)
#define  TX_Red_on_Cyan        (TXwCnC + TXwCBR)
#define  TX_Blue_on_Cyan       (TXwCnC + TXwCBB)
#define  TX_Black_on_Cyan      (TXwCnC + TXwCNZ)
#define  TX_White_on_Cyan      (TXwCnC + TXwCNW)
#define  TX_Magenta_on_Cyan    (TXwCnC + TXwCNM)
#define  TX_Lwhite_on_Cyan     (TXwCnC + TXwCBW)
#define  TX_Cyan_on_Cyan       (TXwCnC + TXwCNC)
#define  TX_Lcyan_on_Cyan      (TXwCnC + TXwCBC)
#define  TX_Green_on_Cyan      (TXwCnC + TXwCNG)
#define  TX_Lgreen_on_Cyan     (TXwCnC + TXwCBG)
#define  TX_Yellow_on_Green    (TXwCnG + TXwCBY)
#define  TX_Black_on_Green     (TXwCnG + TXwCNZ)
#define  TX_White_on_Green     (TXwCnG + TXwCNW)
#define  TX_Lwhite_on_Green    (TXwCnG + TXwCBW)
#define  TX_Green_on_Green     (TXwCnG + TXwCNG)
#define  TX_Lgreen_on_Green    (TXwCnG + TXwCBG)
#define  TX_Yellow_on_Magenta  (TXwCnM + TXwCBY)
#define  TX_Black_on_Magenta   (TXwCnM + TXwCNZ)
#define  TX_White_on_Magenta   (TXwCnM + TXwCNW)
#define  TX_Lcyan_on_Magenta   (TXwCnM + TXwCBC)
#define  TX_Lwhite_on_Magenta  (TXwCnM + TXwCBW)
#define  TX_Lgreen_on_Magenta  (TXwCnM + TXwCBG)
#define  TX_Magenta_on_Magenta (TXwCnM + TXwCNM)
#define  TX_Magenta_on_Blue    (TXwCnB + TXwCNM)
#define  TX_Yellow_on_Blue     (TXwCnB + TXwCBY)
#define  TX_Brown_on_Blue      (TXwCnB + TXwCNY)
#define  TX_Grey_on_Blue       (TXwCnB + TXwCBZ)
#define  TX_Red_on_Blue        (TXwCnB + TXwCNR)
#define  TX_Black_on_Blue      (TXwCnB + TXwCNZ)
#define  TX_White_on_Blue      (TXwCnB + TXwCNW)
#define  TX_Lwhite_on_Blue     (TXwCnB + TXwCBW)
#define  TX_Cyan_on_Blue       (TXwCnB + TXwCNC)
#define  TX_Lcyan_on_Blue      (TXwCnB + TXwCBC)
#define  TX_Green_on_Blue      (TXwCnB + TXwCNG)
#define  TX_Blue_on_Blue       (TXwCnB + TXwCNB)
#define  TX_Lgreen_on_Blue     (TXwCnB + TXwCBG)
#define  TX_Black_on_Brown     (TXwCnY + TXwCNZ)
#define  TX_White_on_Brown     (TXwCnY + TXwCNW)
#define  TX_Lwhite_on_Brown    (TXwCnY + TXwCBW)
#define  TX_Brown_on_Brown     (TXwCnY + TXwCNY)
#define  TX_Yellow_on_Brown    (TXwCnY + TXwCBY)
#define  TX_Cyan_on_Brown      (TXwCnY + TXwCNC)
#define  TX_Lcyan_on_Brown     (TXwCnY + TXwCBC)
#define  TX_Green_on_Brown     (TXwCnY + TXwCBG)
#define  TX_Yellow_on_Grey     (TXwCbZ + TXwCBY)
#define  TX_Brown_on_Grey      (TXwCbZ + TXwCNY)
#define  TX_Black_on_Grey      (TXwCbZ + TXwCNZ)
#define  TX_Blue_on_Grey       (TXwCbZ + TXwCNB)
#define  TX_White_on_Grey      (TXwCbZ + TXwCNW)
#define  TX_Lwhite_on_Grey     (TXwCbZ + TXwCBW)
#define  TX_Lwhite_on_Grey     (TXwCbZ + TXwCBW)
#define  TX_Cyan_on_Grey       (TXwCbZ + TXwCNC)
#define  TX_Lcyan_on_Grey      (TXwCbZ + TXwCBC)
#define  TX_Grey_on_Grey       (TXwCbZ + TXwCBZ)
#define  TX_Blue_on_Black      (TXwCnZ + TXwCNB)
#define  TX_Green_on_Black     (TXwCnZ + TXwCNG)
#define  TX_Lgreen_on_Black    (TXwCnZ + TXwCBG)
#define  TX_Cyan_on_Black      (TXwCnZ + TXwCNC)
#define  TX_Lcyan_on_Black     (TXwCnZ + TXwCBC)
#define  TX_Yellow_on_Black    (TXwCnZ + TXwCBY)
#define  TX_Magenta_on_Black   (TXwCnZ + TXwCNM)
#define  TX_Grey_on_Black      (TXwCnZ + TXwCBZ)
#define  TX_Red_on_Black       (TXwCnZ + TXwCBR)
#define  TX_Black_on_Black     (TXwCnZ + TXwCNZ)
#define  TX_White_on_Black     (TXwCnZ + TXwCNW)
#define  TX_Lwhite_on_Black    (TXwCnZ + TXwCBW)

#define  TXWCOL_BUTTON          TX_Lwhite_on_Grey
#define  TXWCOL_BFRAME          TX_Yellow_on_Grey


// Keyboard (conio, getch) translation values to get unique key-definitions
#define  TXW_KEY_GROUP_0        0x000
#define  TXW_KEY_GROUP_1        0x100


// key-values as delivered by GetKeyEvent or in mp2 with a TXWM_CHAR msg

#define TXk_MAX                 0x1ff           // max accelerator key value

// Normal key values
#define TXk_F1                  0x13b
#define TXk_F2                  0x13c
#define TXk_F3                  0x13d
#define TXk_F4                  0x13e
#define TXk_F5                  0x13f
#define TXk_F6                  0x140
#define TXk_F7                  0x141
#define TXk_F8                  0x142
#define TXk_F9                  0x143
#define TXk_F10                 0x144
#define TXk_F11                 0x185
#define TXk_F12                 0x186

#define TXk_ENTER               0x00d
#define TXk_SPACE               0x020
#define TXk_BACKSPACE           0x008
#define TXk_TAB                 0x009
#define TXk_ESCAPE              0x01b
#define TXk_BACKQUOTE           0x060

#define TXk_INSERT              0x152
#define TXk_DELETE              0x153
#define TXk_UP                  0x148
#define TXk_DOWN                0x150
#define TXk_LEFT                0x14b
#define TXk_RIGHT               0x14d
#define TXk_PGUP                0x149
#define TXk_PGDN                0x151
#define TXk_HOME                0x147
#define TXk_END                 0x14f


// Shift+key values
#define TXs_F1                  0x154
#define TXs_F2                  0x155
#define TXs_F3                  0x156
#define TXs_F4                  0x157
#define TXs_F5                  0x158
#define TXs_F6                  0x159
#define TXs_F7                  0x15a
#define TXs_F8                  0x15b
#define TXs_F9                  0x15c
#define TXs_F10                 0x15d
#define TXs_F11                 0x187
#define TXs_F12                 0x188

#define TXs_TAB                 0x10f


// Ctrl+key values
#define TXc_F1                  0x15e
#define TXc_F2                  0x15f
#define TXc_F3                  0x160
#define TXc_F4                  0x161
#define TXc_F5                  0x162
#define TXc_F6                  0x163
#define TXc_F7                  0x164
#define TXc_F8                  0x165
#define TXc_F9                  0x166
#define TXc_F10                 0x167
#define TXc_F11                 0x189
#define TXc_F12                 0x18a

#define TXc_ENTER               0x00a
#define TXc_BACKSP              0x07f
#define TXc_TAB                 0x194
#define TXc_2                   0x103
#define TXc_6                   0x01e
#define TXc_MINUS               0x01f
#define TXc_LBRACKET            0x01b
#define TXc_RBRACKET            0x01d
#define TXc_BACKSLASH           0x01c

#define TXc_INSERT              0x192
#define TXc_DELETE              0x193
#define TXc_UP                  0x18d
#define TXc_DOWN                0x191
#define TXc_LEFT                0x173
#define TXc_RIGHT               0x174
#define TXc_PGUP                0x184
#define TXc_PGDN                0x176
#define TXc_HOME                0x177
#define TXc_END                 0x175

#define TXc_A                   0x001
#define TXc_B                   0x002
#define TXc_C                   0x003           // usualy intercepted!
#define TXc_D                   0x004
#define TXc_E                   0x005
#define TXc_F                   0x006
#define TXc_G                   0x007
#define TXc_H                   0x008           // reserved! (backspace)
#define TXc_I                   0x009           // reserved! (tab)
#define TXc_J                   0x00a           // reserved! (c_enter)
#define TXc_K                   0x00b
#define TXc_L                   0x00c
#define TXc_M                   0x00d           // reserved! (enter)
#define TXc_N                   0x00e
#define TXc_O                   0x00f
#define TXc_P                   0x010           // usualy intercepted!
#define TXc_Q                   0x011
#define TXc_R                   0x012
#define TXc_S                   0x013           // usualy intercepted!
#define TXc_T                   0x014
#define TXc_U                   0x015
#define TXc_V                   0x016
#define TXc_W                   0x017
#define TXc_X                   0x018
#define TXc_Y                   0x019
#define TXc_Z                   0x01a


// Alt+key values
#define TXa_F1                  0x168
#define TXa_F2                  0x169
#define TXa_F3                  0x16a
#define TXa_F4                  0x16b
#define TXa_F5                  0x16c
#define TXa_F6                  0x16d
#define TXa_F7                  0x16e
#define TXa_F8                  0x16f
#define TXa_F9                  0x170
#define TXa_F10                 0x171
#define TXa_F11                 0x18b
#define TXa_F12                 0x18c

#define TXa_ENTER               0x11c
#define TXa_BACKSP              0x10e
#define TXa_1                   0x178
#define TXa_2                   0x179
#define TXa_3                   0x17a
#define TXa_4                   0x17b
#define TXa_5                   0x17c
#define TXa_6                   0x17d
#define TXa_7                   0x17e
#define TXa_8                   0x17f
#define TXa_9                   0x180
#define TXa_0                   0x181
#define TXa_MINUS               0x182
#define TXa_EQUAL               0x183
#define TXa_LBRACKET            0x11a
#define TXa_RBRACKET            0x11b
#define TXa_SEMICOLON           0x127
#define TXa_QUOTE               0x128
#define TXa_BACKQUOTE           0x129
#define TXa_BACKSLASH           0x12b
#define TXa_COMMA               0x133
#define TXa_DOT                 0x134
#define TXa_SLASH               0x135

#define TXa_INSERT              0x1a2
#define TXa_DELETE              0x1a3
#define TXa_UP                  0x198
#define TXa_DOWN                0x1a0
#define TXa_LEFT                0x19b
#define TXa_RIGHT               0x19d
#define TXa_PGUP                0x199
#define TXa_PGDN                0x1a1
#define TXa_HOME                0x197
#define TXa_END                 0x19f

#define TXa_A                   0x11e
#define TXa_B                   0x130
#define TXa_C                   0x12e
#define TXa_D                   0x120
#define TXa_E                   0x112
#define TXa_F                   0x121
#define TXa_G                   0x122
#define TXa_H                   0x123
#define TXa_I                   0x117
#define TXa_J                   0x124
#define TXa_K                   0x125
#define TXa_L                   0x126
#define TXa_M                   0x132
#define TXa_N                   0x131
#define TXa_O                   0x118
#define TXa_P                   0x119
#define TXa_Q                   0x110
#define TXa_R                   0x113
#define TXa_S                   0x11f
#define TXa_T                   0x114
#define TXa_U                   0x116
#define TXa_V                   0x12f
#define TXa_W                   0x111
#define TXa_X                   0x12d
#define TXa_Y                   0x115
#define TXa_Z                   0x12c

// Modern keyboard special keys (plus de 'windows' keys)

#define TXk_LWIN                0x1ec
#define TXk_RWIN                0x1ed
#define TXk_MENU                0x1ee
#define TXk_FIND                0x1f5

// GetInputEvent return value for events other than keystrokes
#define  TXW_INPUT_MOUSE        0xffff          // mouse input
#define  TXW_INPUT_ASYNC        0xfffe          // other thread (post msg)

typedef struct txw_input_event
{
   short               row;                     // related Y position
   short               col;                     // related X position
   ULONG               value;                   // mouse-button or key value
   ULONG               state;                   // ShiftState (Ctrl/Alt/Shift)
   ULONG               key;                     // key-value  (or mouse-value)
} TXW_INPUT_EVENT;                              // end of "txw_input_event"


/*======== String related  macro ============================================*/

#define txSlen(s)  ((short) strlen(s))
#define txLlen(s)  ((ULONG) strlen(s))


/*======== Specific windowing trace macro's =================================*/

#if defined (DUMP)
   #define TRCMSG(hwnd,msg,mp1,mp2)                                          \
           TRACES(("hwnd:%8.8lx, msg:%2.2lx = %s%s%s, "                      \
                   "mp1:%8.8lx = %lu mp2:%8.8lx = %lu  %s\n", hwnd, msg,     \
                    CBC, txwMsgDescription(msg), CNN, mp1, mp1, mp2, mp2,    \
                    (msg == TXWM_CHAR) ? txwKeyDescription(mp2) : "" ));

   #define TRCLAS(txt,hwnd)                                                  \
           TRACES(("%s - hwnd:%8.8lx, class: %s%s%s\n",                      \
                    txt, hwnd, CBM, txwClassDescription(hwnd), CNN));

   #define TRECTA(txt,rect)                                                  \
           if (rect != NULL)                                                 \
           {                                                                 \
              TRACES(("%s - top:% 3hd  left:% 3hd  bot:% 3hd  right:% 3hd\n",\
                  txt, rect->top, rect->left, rect->bottom, rect->right));   \
           }

   #define TRWBSV(txt,wbs)                                                   \
           if (TxTrLevel >= 70)                                              \
           {                                                                 \
              DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                 \
              if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                  \
              txwWbsDescription(txt,wbs);                                    \
              if (TxTrLogOnly)   TxScreenState(tss);                         \
           }
   #define TRSTYL(txt,hwnd) TRWBSV(txt,((TXWINBASE *)hwnd)->window->style)


   #define TRWINS(txt)                                                       \
           if (TxTrLevel >= 70)                                              \
           {                                                                 \
              DEVICE_STATE tss = TxScreenState(DEVICE_TEST);                 \
              if (TxTrLogOnly)   TxScreenState(DEVICE_OFF);                  \
              txwDumpWindows(txt);                                           \
              if (TxTrLogOnly)   TxScreenState(tss);                         \
           }

// Return short message description for msg-id
char *txwMsgDescription
(
   ULONG               msg                      // IN    message-id
);

// Return short description for WM_CHAR key code (mp2)
char *txwKeyDescription
(
   ULONG               key                      // IN    message-id
);

// Return short description for class-id
char *txwClassDescription
(
   TXWHANDLE           hwnd                     // IN    window handle or 0
);

// Return short description for Window-USHORT ID
char *txwWusDescription
(
   USHORT              wus                      // IN    window ushort ID
);

// Display overview of a style value, with expanded texts for each flag
void txwWbsDescription
(
   char               *text,                    // IN    leading text
   ULONG               wbs                      // IN    style value
);

// Dump most important window variables to screen/logfile
void txwDumpWindows
(
   char               *text                     // IN    identifying string
);

#else
   #define TRCMSG(hwnd,msg,mp1,mp2)
   #define TRCLAS(txt,hwnd)
   #define TRECTA(txt,rect)
   #define TRWBSV(txt,wbs)
   #define TRSTYL(txt,hwnd)
   #define TRWINS(txt)
#endif

/*======== Scroll-buffer setup functions, TXWPRINT.C ========================*/

// Request codes for HelpDialog special requests
#define TXWS_REQ_NONE                   0       // no special request
#define TXWS_REQ_FINDSTRING             2       // aka F7 with argument
#define TXWS_REQ_FINDDIALOG             3       // aka F7 dialog
#define TXWS_REQ_GREPDIALOG             4       // aka F8 dialog, grep
#define TXWS_REQ_GREPSTRING             5       // aka F8 argument, grep

// Initiate a FIND request for the scrollbuffer, including GREP and dialogs
ULONG txwSbViewFindRequest
(
   ULONG               request,                 // IN    specific request code
   char               *param                    // IN    string parameter
);

// Initialize TxPrint hook to a scroll-buffer and related views
ULONG txwInitPrintfSBHook
(
   TXSBDATA           *sbuf                     // IN    Scroll-buffer data
);

// Terminate TxPrint hook to a scroll-buffer and related views
void txwTermPrintfSBHook
(
   void
);

// Save (part of) the scroll buffer to specified file as plain ASCII
ULONG txwSavePrintfSB                           // RET   nr of lines written
(
   char               *fname,                   // IN    name of output file
   ULONG               lines,                   // IN    lines from start/end
   BOOL                fromEnd                  // IN    last lines (from end)
);


/*======== Message related functions, TXWMSG.C ==============================*/

// Get next message from queue (posted msg or keyboard events)
BOOL txwGetMsg                                  // RET   FALSE if QUIT
(
   TXWQMSG            *qmsg                     // OUT   message packet
);

// Dispatch a message to destination window
ULONG txwDispatchMsg                            // RET   result
(
   TXWQMSG            *qmsg                     // IN    message packet
);

// Send a message to specified window
ULONG txwSendMsg                                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    destination window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// Post a message to specified window, 0 indicates the focus-window
BOOL txwPostMsg                                 // RET   message posted
(
   TXWHANDLE           hwnd,                    // IN    destination window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// Make messages for specified window invalid (on destroy)
void txwDestroyMessages
(
   TXWHANDLE           hwnd                     // IN    destination window
);


// Check if specified message is queued for a window handle
BOOL txwQueueMsgPresent                         // RET   FALSE if no msg there
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               message                  // IN    message id
);


//- special command-codes to be used with SetAccelerator
#define TXWACCEL_CLEAR ((ULONG)  0)             // reset key for system/window
#define TXWACCEL_MHELP ((ULONG) -2)             // translate to WM_HELP message
#define TXWACCEL_BLOCK ((ULONG) -1)             // block system translation for
                                                // specified window and key
#define TXWACCEL_SYSTEM  0

// Set command-code for system or window acceltable and specified key
ULONG txwSetAccelerator                         // RET   function result
(
   TXWHANDLE           hwnd,                    // IN    handle or 0 for system
   ULONG               key,                     // IN    keycode 0 .. TXk_MAX
   ULONG               cmd                      // IN    command code
);


// Translate WM_CHAR message to WM_COMMAND when defined as accelerator
BOOL txwTranslateAccel                          // RET   msg is translated
(
   TXWHANDLE           hwnd,                    // IN    handle or 0 for system
   TXWQMSG            *qmsg                     // INOUT message packet
);


// Note: use of GetMsg is recommended since it handles other messages too
// Get keystroke(s) and return a unique single value for each (getch) key
// Uses a special OTHER value and additional info structure for mouse events
ULONG txwGetInputEvent                          // RET   unique event value
(
   TXW_INPUT_EVENT    *event                    // OUT   optional event info
);                                              //       NULL when keystroke
                                                //       events only desired

// Abort synchronious input (wait) to pickup async event in queue
void txwNotifyAsyncInput
(
   void
);

/*======== Event hook functions, TXWEHOOK.C ===============================*/

// Attach specified hook-event to a window-handle
ULONG txwAttachEventHook                        // RET   result
(
   ULONG               hookid,                  // IN    TXHK identifier
   TXWHANDLE           hwnd                     // IN    window handle
);

// Detach specified hook-event from a window-handle
ULONG txwDetachEventHook                        // RET   result
(
   ULONG               hookid,                  // IN    TXHK identifier
   TXWHANDLE           hwnd                     // IN    window handle
);

// Query specified hook-event for a window-handle
TXWHANDLE txwQueryEventHook                     // RET   window handle
(
   ULONG               hookid                   // IN    TXHK identifier
);

// Signal specified hook-event, pass optional data (data must stay valid!)
TXWHANDLE txwSignalEventHook                    // RET   window signalled
(                                               //       (posted WM_HOOKEVENT)
   ULONG               hookid,                  // IN    TXHK identifier (MP1)
   void               *data                     // IN    optional data   (MP2)
);

/*======== Window related functions, TXWIND.C ===============================*/

// Initialize TX window manager and create the desktop window
TXWHANDLE txwInitializeDesktop                  // RET   Desktop handle
(
   TXWINDOW           *custom,                  // IN    custom desktop style
   TXWINPROC           winproc                  // IN    custom desktop proc
);

// Terminate TX window manager including the desktop window
void txwTerminateDesktop
(
   void
);

// Determine if specified handle is an existing window
BOOL txwIsWindow
(
   TXWHANDLE           hwnd                     // IN    window handle
);

// Query window-handles related to the specified one
TXWHANDLE txwQueryWindow
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               relation                 // IN    window relation
);

// Get a pointer to the TXWINDOW data from window-handle
TXWINDOW *txwWindowData
(
   TXWHANDLE           hwnd                     // IN    window handle
);

// Initialize a setup window structure for Create
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
   BYTE                titlecolor,              // IN    default title  color
   BYTE                titlefocus,              // IN    focus   title  color
   BYTE                footercolor,             // IN    default footer color
   BYTE                footerfocus,             // IN    focus   footer color
   char               *title,                   // IN    title text string
   char               *footertxt,               // IN    footer text string
   TXWINDOW           *win                      // OUT   window structure
);

#if defined (NEVER)
#define txwSetupWindowData(y,x,sy,sx,st,hi,cc,ca,bc,ba,tc,tf,ts,fc,ff,fs,w)  \
   (w)->helpid         = (hi);                                               \
   (w)->clientclear.ch = (cc);                                               \
   (w)->clientclear.at = (ca);                                               \
   (w)->borderclear.ch = (bc);                                               \
   (w)->borderclear.at = (ba);                                               \
   (w)->titlecolor     = (tc);                                               \
   (w)->titlefocus     = (tf);                                               \
   (w)->title          = (ts);                                               \
   (w)->footercolor    = (fc);                                               \
   (w)->footerfocus    = (ff);                                               \
   (w)->footer         = (fs);                                               \
   txwSetupWindowRect((y),(x),(sy),(sx),(st),(w))


// Initialize the client and border rectangle structures in a Window
void txwSetupWindowRect
(
   short               row,                     // IN    top-left row (border)
   short               col,                     // IN    top-left column
   short               rows,                    // IN    nr of rows incl border
   short               cols,                    // IN    nr of cols incl border
   ULONG               style,                   // IN    window & border style
   TXWINDOW           *win                      // INOUT window structure
);
#endif

// Create a new base window element and initialize its window-data
// Note: owner and winproc must be 0 for a dialog frame (see CreateDlg)
TXWHANDLE txwCreateWindow
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWCLASS            class,                   // IN    class of this window
   TXWHANDLE           owner,                   // IN    owner window
   TXWHANDLE           after,                   // IN    insert after ...
   TXWINDOW           *setup,                   // IN    window setup data
   TXWINPROC           winproc                  // IN    window procedure
);

// Destroy a base window element and remove it from the list
ULONG txwDestroyWindow                          // RET   result
(
   TXWHANDLE           hwnd                     // IN    window handle
);

// Set focus to specified window, notify old and new focus-windows
ULONG txwSetFocus
(
   TXWHANDLE           hwnd                     // IN    destination window
);

// Enable or disable specified window
ULONG txwEnableWindow
(
   TXWHANDLE           hwnd,                    // IN    destination window
   BOOL                enable                   // IN    enable flag
);

// Query enable state of window
BOOL txwIsWindowEnabled                         // RET   window is enabled
(
   TXWHANDLE           hwnd                     // IN    window to query
);

// Sets or clears windows VISIBLE flag, and shows/hides the window
ULONG txwShowWindow
(
   TXWHANDLE           hwnd,                    // IN    destination window
   BOOL                visible                  // IN    visible flag
);

// Query visible state of window (and ALL of its parents)
BOOL txwIsWindowVisible                         // RET   window is visible
(
   TXWHANDLE           hwnd                     // IN    window to query
);

// Query complete visibility of (client) window
BOOL txwIsWindowShowing                         // RET   window fully visible
(
   TXWHANDLE           hwnd                     // IN    window to query
);

// Query complete visibility of specified rectangle
BOOL txwIsRectShowing                           // RET   rect fully visible
(
   TXWHANDLE           hwnd,                    // IN    start window or NULL
   TXRECT             *rect                     // IN    rectangle to query
);

// Invalidate all windows in Z-order, causing a repaint for all
void txwInvalidateAll
(
   void
);

// Invalidate specified window, forcing repaint
ULONG txwInvalidateWindow
(
   TXWHANDLE           hwnd,                    // IN    destination window
   BOOL                border,                  // IN    invalidate border
   BOOL                children                 // IN    invalidate children
);

// Invalidate border area of specified window, forcing border repaint
ULONG txwInvalidateBorder
(
   TXWHANDLE           hwnd                     // IN    destination window
);

// Query window rectangle, as a rectangle indicating position and size
BOOL txwQueryWindowPos
(
   TXWHANDLE           hwnd,                    // IN    window handle
   BOOL                border,                  // IN    include border
   TXRECT             *rect                     // OUT   client rectangle
);                                              //       absolute positions

// Query window rectangle, as a rectangle indicating the client area size
BOOL txwQueryWindowRect
(
   TXWHANDLE           hwnd,                    // IN    window handle
   BOOL                border,                  // IN    include border
   TXRECT             *rect                     // OUT   client area size
);                                              //       (left/top are zero)

// Query USHORT window value
USHORT txwQueryWindowUShort
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index                    // IN    index of window-ushort
);


// Query ULONG  window value
ULONG  txwQueryWindowULong
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index                    // IN    index of window-ulong
);

// Query PTR    window value
void  *txwQueryWindowPtr
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index                    // IN    index of window-ptr
);

// Set USHORT window value
BOOL txwSetWindowUShort
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index,                   // IN    index of window-ushort
   USHORT              us                       // IN    value to set
);

// Set ULONG  window value
BOOL txwSetWindowULong
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index,                   // IN    index of window-ulong
   ULONG               ul                       // IN    value to set
);

// Set   PTR    window value
BOOL txwSetWindowPtr
(
   TXWHANDLE           hwnd,                    // IN    window handle
   ULONG               index,                   // IN    index of window-ptr
   void               *p                        // IN    value to set
);

// Set window position related attributes
BOOL txwSetWindowPos
(
   TXWHANDLE           hwnd,                    // IN    window handle or 0
   TXWHANDLE           after,                   // IN    after this in Z-order
   short               x,                       // IN    new abs x-position
   short               y,                       // IN    new abs y-position
   short               cx,                      // IN    new x-size
   short               cy,                      // IN    new y-size
   ULONG               fl                       // IN    positioning options
);

// Resolve window-handle from specified ID, for specified parent
TXWHANDLE txwWindowFromID
(
   TXWHANDLE           parent,                  // IN    window handle or 0
   USHORT              id                       // IN    window id
);


/*======== Window Procedures and Dialogs, TXWPROC.C =========================*/

// Default window procedure, for any window-class except dialogs
ULONG txwDefWindowProc                          // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// Default window procedure, for dialogs including their control windows
ULONG txwDefDlgProc                             // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// Dismiss dialog and pass-on result code
ULONG txwDismissDlg
(
   TXWHANDLE           hwnd,                    // IN    dialog handle
   ULONG               result                   // IN    result code
);

// Create a Dialog from memory-structures (created frame + linked controls)
TXWHANDLE txwCreateDlg
(
   TXWHANDLE           parent,                  // IN    parent (ignored!)
   TXWHANDLE           owner,                   // IN    owner  window
   TXWINPROC           dlgproc,                 // IN    dialog procedure
   TXWHANDLE           dlg,                     // IN    dialog frame window
   PVOID               cData                    // IN    user control data
);

// Process a (modal) dialog using a local msg-loop
ULONG txwProcessDlg                             // RET   dialog rc (dismiss)
(
   TXWHANDLE           hwnd                     // IN    dialog handle
);

// Create, Process and Destroy a (modal) dialog
ULONG txwDlgBox                                 // RET   dialog rc (dismiss)
(
   TXWHANDLE           parent,                  // IN    parent (ignored!)
   TXWHANDLE           owner,                   // IN    owner  window
   TXWINPROC           dlgproc,                 // IN    dialog procedure
   TXWHANDLE           dlg,                     // IN    dialog frame window
   PVOID               cData                    // IN    user control data
);

#if defined (HAVEMOUSE)
// Generic MOUSE BUTTONDOWN processing, class-independant stuff
ULONG txwMouseButtonDown                        // RET   TX_PENDING if not done
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);
#endif

/*======== Generic Widgets,       TXWIDGET.C ================================*/

typedef struct txgw_data
{
   ULONG               count;                   // number of widgets
   ULONG               helpid;                  // help, widget overrules
   USHORT              basewid;                 // base window ID
   TXWINPROC           winproc;                 // widget window proc or NULL
   TXRECT             *posPersist;              // persistent position or NULL
   TXWIDGET           *widget;                  // array of widgets
} TXGW_DATA;                                    // end of struct "txgw_data"


// Get number of lines and columns taken by a widget group
short txwWidgetSize                             // RET   nr of lines taken
(
   TXGW_DATA          *gwdata,                  // IN    generic widget data
   short               toplines,                // IN    empty lines at top
   short              *hsize                    // OUT   nr of columns taken
);                                              //       or NULL if not needed

// Create specified controls for this Generic Widget instance
ULONG txwCreateWidgets
(
   TXWHANDLE           dframe,                  // IN    parent window (dialog)
   TXGW_DATA          *gw,                      // INOUT generic widget data
   short               line,                    // IN    UL-corner lines
   short               col                      // IN    UL-corner column
);                                              // allows multiple widget groups


// Display generic widget dialog, adding standard empty canvas to the widgets
ULONG txwWidgetDialog
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   void               *usrdata,                 // IN    window data (QWP_USER)
   char               *title,                   // IN    title for the dialog
   ULONG               flags,                   // IN    specification flags
   USHORT              focus,                   // IN    focus to index 0..n-1
   TXGW_DATA          *gwdata                   // INOUT generic widget data
);

// Display message using format string and ask confirmation, optional widgets
BOOL TxwConfirm                                 // RET   Confirmed
(
   ULONG               helpid,                  // IN    helpid confirmation
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
);

// Display a message using format string and wait for OK, optional Widgets
BOOL TxwMessage                                 // RET   OK, not escaped
(
   BOOL                acknowledge,             // IN    wait for acknowledge
   ULONG               helpid,                  // IN    helpid confirmation
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
);

/*======== HELP system, base ID's ===========================================*/

#define TXWH_APPLIC_SECTION_BASE        ((ULONG)      0)
#define TXWH_LOADED_SECTION_BASE        ((ULONG) 100000)
#define TXWH_L_USER_SECTION_BASE        ((ULONG) 500000)
#define TXWH_SYSTEM_SECTION_BASE        ((ULONG) 900000)


#define TXWD_HELPMBOX       (TXWH_SYSTEM_SECTION_BASE + 100)
#define TXWD_HELPPBOX       (TXWH_SYSTEM_SECTION_BASE + 200)
#define TXWD_HELPLBOX       (TXWH_SYSTEM_SECTION_BASE + 300)
#define TXWD_HELPVIEW       (TXWH_SYSTEM_SECTION_BASE + 500)
#define TXCM_HELPREG        (TXWH_SYSTEM_SECTION_BASE + 600)
#define TXWD_HELPFDLG       (TXWH_SYSTEM_SECTION_BASE + 700)
#define TXWD_HELPSMSG       (TXWH_SYSTEM_SECTION_BASE + 900)

#define TXWD_RETRYFAIL      (TXWD_HELPSMSG + 20) // abort, retry, fail popup
#define TXWD_DISABLEDITEM   (TXWD_HELPSMSG + 30) // menu item is disabled

#define TXWH_HELPREG        (TXWH_SYSTEM_SECTION_BASE + 2000)
#define TXWH_SEARCHDIALOG   (TXWH_HELPREG + 400)
#define TXWS_SEARCHDIALOG   (TXWH_HELPREG + 500)


/*======== Standard dialogs,      TXWSTDLG.C ================================*/


// Display standard message-box dialog with text, buttons and optional Widgets
#define txwMessageBox(p,o,m,t,h,f) txwMessageBoxWidgets(p,o,NULL,m,t,h,f)
ULONG txwMessageBoxWidgets
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *message,                 // IN    message text
   char               *title,                   // IN    title for the dialog
   ULONG               helpid,                  // IN    help on message
   ULONG               flags                    // IN    specification flags
);

// Display standard prompt-box dialog with text an entryfield and help-id
ULONG txwPromptBox
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *message,                 // IN    message text
   char               *title,                   // IN    title for the dialog
   ULONG               helpid,                  // IN    help on message
   ULONG               flags,                   // IN    specification flags
   short               eflength,                // IN    max length of value
   char               *value                    // INOUT entry field value
);


// Display standard List-box dialog for menu's or selection lists
ULONG txwListBox
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXRECT             *pos,                     // IN    absolute position
   char               *title,                   // IN    title or ""
   char               *footer,                  // IN    footer or ""
   ULONG               helpid,                  // IN    help on total list
   ULONG               flags,                   // IN    specification flags
   BYTE                cscheme,                 // IN    client color scheme
   BYTE                bscheme,                 // IN    border color scheme
   TXSELIST           *list                     // IN    list data
);


// Position listbox correctly for 'selected' index at start or after sort/reverse
void txwPositionListBox
(
   TXLISTBOX       *box                         // IN    listbox data structure
);

// Display menu-bar dialog with attached menu's using selection lists
ULONG txwMenuBar                                // RET   selected CMD_CODE
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXRECT             *pos,                     // IN    pos or NULL for parent
   ULONG               open,                    // IN    0=first, x=ident
   ULONG               helpid,                  // IN    help on menubar
   ULONG               flags,                   // IN    TXMN_ menu flags
   TXS_MENUBAR        *mbar                     // IN    menubar information
);

// Display text-view window with supplied text on top of master client area
ULONG txwViewText
(
   TXWHANDLE           master,                  // IN    master window
   ULONG               helpid,                  // IN    help on this text
   ULONG               topline,                 // IN    first line to display
   char               *title,                   // IN    window title
   char              **text                     // IN    text to view
);


//- FileDlg related definitions and functions

#define TXFD_OPEN_DIALOG        0x0001          // assume file-open  (read)
#define TXFD_SAVEAS_DIALOG      0x0002          // assume save-as    (write)
#define TXFD_SELECT_DIRECT      0x0004          // select directory  (write)
#define TXFD_MULTIPLESEL        0x0008          // multiple-select   (read)
#define TXFD_MOVEABLE           0x0010          // dialog is movable
#define TXFD_CREATE_NEW_DIR     0x0020          // include "new dir" button
#define TXFD_RUNTIME_FLOPPY     0x0040          // runtime floppy check (slow)

typedef struct txwfiledlg
{
   ULONG               flags;                   // modify FileDlg behaviour
   ULONG               result;                  // dialog result (DID_...)
   TXLN                fName;                   // wildcard IN, selected OUT
   TX1K                fPath;                   // initial  IN, selected OUT
   TX1K                fResult;                 // Resulting fully qualified
   TXLN                fDefault;                // filename IN
   char               *title;                   // dialog title, or NULL
   TXGW_DATA          *gwdata;                  // generic widget data
   ULONG               helpid;                  // specific help, whole dialog
   ULONG               basehelp;                // Std Open/SaveAs/Dir help
   TXSELIST           *list;                    // multiple-select result list
   TXWINPROC           winproc;                 // custom window procedure
   void               *userdata;                // user definable field
   void               *reserved;                // for TXW internal use
} TXWFILEDLG;                                   // end of struct "txwfiledlg"


// Present a file-open dialog with default behaviour
BOOL txwOpenFileDialog
(
   char               *fName,                   // IN    filespec wildc or NULL
   char               *fPath,                   // IN    drive + path   or NULL
   char               *fDefault,                // IN    initial fname  or NULL
   ULONG               helpid,                  // IN    specific help
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *title,                   // IN    dialog title,  or NULL
   char               *fullName                 // OUT   full path&filename
);


// Present a file-save-as dialog with default behaviour
BOOL txwSaveAsFileDialog
(
   char               *fName,                   // IN    filespec wildc or NULL
   char               *fPath,                   // IN    drive + path   or NULL
   char               *fDefault,                // IN    initial fname  or NULL
   ULONG               helpid,                  // IN    specific help
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *title,                   // IN    dialog title,  or NULL
   char               *fullName                 // OUT   full path&filename
);

// Present a select-directory dialog with default behaviour
BOOL txwSelDirFileDialog
(
   char               *fPath,                   // IN    drive + path   or NULL
   ULONG               helpid,                  // IN    specific help
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *title,                   // IN    dialog title,  or NULL
   char               *fullPath                 // OUT   full path
);

// Present standard file dialog as specified
BOOL txwStdFileDialog                           // RET   filename selected
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWFILEDLG         *fdata                    // INOUT file dialog data
);

// Window procedure, for the Standard File Dialog
ULONG txwDefFileDlgProc                         // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

/*======== Help related functions, TXWHELP.C ================================                    */

#define TXHELPITEM(nr,title) \
 "", \
 " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -", \
 " Some options may require switching to 'Expert mode' from the Edit menu.", \
 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", \
 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", \
 "#" # nr " " title, "",

// Special help-id to pass non help-item request to the dialog
#define TXWH_REQUEST_ID                 0xfffffff0

// Request codes for HelpDialog special requests
#define TXWH_REQ_NONE                   0       // no special request
#define TXWH_REQ_ITEMLIST               1       // aka F10 from help
#define TXWH_REQ_SECTIONLIST            2       // aka F9  from help
#define TXWH_REQ_FINDDIALOG             3       // aka F7  from help, dialog
#define TXWH_REQ_GREPDIALOG             4       // aka F8  from help, dialog
#define TXWH_REQ_GREPSTRING             5       // aka F8  from help, list

// Set starting base-ID for help-items loaded from file; Will auto-increment!
// Use to set at another user-defined range, allowing fixed hyperlinking
ULONG txwSetLoadedHelpBaseId                    // RET   Current base value or 0
(
   ULONG               newBase                  // IN    New base value to set
);

// Load a helptext from given name in ABSOLUTE PATH, CURRENT DIR or ./doc DIR
ULONG txwLoadRegisterHelpFile
(
   char               *name,                    // IN    filename (or basename)
   char               *title                    // IN    optional section title
);

// Register a helptext of one section with multiple items
ULONG txwRegisterHelpText
(
   ULONG               baseid,                  // IN    base id type/section
   char               *name,                    // IN    section name (16)
   char               *title,                   // IN    section title
   char               *text[]                   // IN    help text array
);

// Display standard help dialog with text for specified help-id
ULONG txwHelpDialog
(
   ULONG               helpid,                  // IN    id for specific helpitem
   ULONG               request,                 // IN    specific request code
   char               *param                    // IN    string parameter
);

// Get text for one specific help-id, ends at '#' on linestart or NULL ptr
char **txwGetHelpText                           // RET   HELP text or NULL
(
   ULONG               helpid                   // IN    id for helpitem
);

// Free helpmanager resources
void txwTerminateHelpManager
(
   void
);

/*======== Hex Editor standard dialog, TXWHEXED.C ===========================*/

// Initialize the Hex-editor dialog/control data structures
// To be called by CUSTOM hex-editor control users (not standard dialog)
BOOL txwInitHexeditDialogs
(
   void
);

// Display standard Hex editor dialog
ULONG txwHexEditor
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXHEXEDIT          *hedat,                   // IN    hex edit data
   char               *title,                   // IN    title for the dialog
   ULONG               helpid,                  // IN    help on message
   ULONG               flags                    // IN    specification flags
);

/*======== Window drawing functions, TXWDRAW.C ==============================*/

// Set custom linestyle for border painting
BOOL txwSetLinesCustom                          // RET   linestyle string OK
(
   char               *custom                   // IN   custom string (40)
);

// Query current and set new window ColorScheme by scheme-value or letter
ULONG txwColorScheme                            // RET   Current scheme
(
   ULONG               scheme,                  // IN    Scheme selector
   TXTT                name                     // OUT   Scheme name or NULL
);

// Create selist from the available Color schemes
ULONG TxSelistColorSchemes                      // RET   result
(
   TXSELIST          **list                     // OUT   selection list
);

// Draw a character-string at specified position, clip to optional clip-rect
void txwDrawCharString
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   char               *str                      // IN    string to draw
);

#define txwStringAt(r,c,s,a) txwDrawCharStrCol(hwnd,r,c,s,a)
// Draw a character-string at position in specified color, using clip-rect
void txwDrawCharStrCol
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   char               *str,                     // IN    string to draw
   BYTE                color                    // IN    color attribute
);

// Draw a cell-string at specified position, clip to optional clip-rect
void txwDrawCellString
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXCELL             *cell,                    // IN    cell-string to draw
   short               size                     // IN    nr of cells
);

// Read a cell-string from specified position on screen to a string
void txwScrReadCellString
(
   short               row,                     // IN    source row
   short               col,                     // IN    source column
   TXCELL             *cell,                    // OUT   cell-string to draw
   short               length                   // IN    nr of cells
);

// Fill area for rectangle with specified cell (clear area)
void txwFillClientWindow
(
   TXWHANDLE           hwnd,                    // IN    window handle
   TXCELL              cell                     // IN    cell-string to draw
);

// Scroll client area up or down, fill new with specified cell
void txwScrollClientWindow
(
   TXWHANDLE           hwnd,                    // IN    window handle
   int                 scroll                   // IN    scroll lines (+ = up)
);

// Set cursor position inside a client window
void txwSetCursorPos
(
   TXWHANDLE           hwnd,                    // IN    window handle
   short               row,                     // IN    destination row
   short               col                      // IN    destination column
);

// Set cursor style for window, visible only when inside (parent) clip area
void txwSetCursorStyle
(
   TXWHANDLE           hwnd,                    // IN    window handle
   BOOL                insert                   // IN    insert style
);


#if defined (USEWINDOWING)

// Set specified status text on default SBview status window (progress info)
void txwSetSbviewStatus
(
   char               *text,                    // IN    optional status text
   BYTE                color                    // IN    optional status color
);

#define txwAllowUserStatusMessages(on) txwDefaultStatusShow( !(on));

// Set display of default-status in sbview ON or OFF (Ctrl+arrows ...")
void txwDefaultStatusShow
(
   BOOL                show                     // IN    show default status
);

// Set specified (trace) text on Desktop title- or top-line (quick and dirty)
void txwSetDesktopTopLine
(
   char               *text,                    // IN    optional status text
   BYTE                color                    // IN    optional status color
);

#else
#define txwSetSbviewStatus(a,b)
#define txwSetDesktopTopLine(a,b)
#define txwAllowUserStatusMessages(on)
#endif                                          // USEWINDOWING

/*======== Window utility functions, TXWUTIL.C ==============================*/

// Start enumeration of child windows
TXWHENUM txwBeginEnumWindows                    // RET   enumeration handle
(
   TXWHANDLE           parent                   // IN    Parent window handle
);

// End an enumeration of child-windows (free resources)
BOOL txwEndEnumWindows                          // RET   henum was valid
(
   TXWHENUM            henum                    // IN    Enumeration handle
);

// Retrieve next child's handle in enumeration
TXWHANDLE txwGetNextWindow                      // RET   next child or 0
(
   TXWHENUM            henum                    // IN    Enumeration handle
);

// Determine if window is a direct descendant of parent (or parent itself)
BOOL txwIsChild                                 // RET   window is a child
(
   TXWHANDLE           window,                  // IN    window to test
   TXWHANDLE           parent                   // IN    Parent window handle
);

// Determine if window is a descendant of given parent  (or parent itself)
BOOL txwIsDescendant                            // RET   window is descendant
(
   TXWHANDLE           hwnd,                    // IN    window to test
   TXWHANDLE           parent                   // IN    Parent window handle
);


// Determine if either this window OR a parent is currently MINIMIZED
BOOL txwIsMinimized                             // RET   window is descendant
(
   TXWHANDLE           hwnd,                    // IN    window to test
   BOOL                parent                   // IN    check parents only
);                                              //       or check THIS only

// Reset all AutoRadioButtons in a group to the 'unset' condition
void txwResetAutoRadioGroup
(
   TXWHANDLE           hwnd,                    // IN    dialog handle or 0
   USHORT              group                    // IN    Group ID or 0
);

// Determine if keyvalue is a possible ACCELERATOR key
BOOL txwIsAccelCandidate                        // RET   key could be ACCEL
(
   ULONG               key                      // IN    key value
);

// Set mouse capture to specified window (drag)
BOOL txwSetCapture
(
   TXWHANDLE           capture                  // IN    capture window
);

// Query handle for current mouse capture window (drag)
TXWHANDLE txwQueryCapture
(
   void
);

// Find window on top of Z-order at specified position
TXWHANDLE txwTopWindowAtPos
(
   short               row,                     // IN    vert position (Y)
   short               col                      // IN    hor. position (X)
);

// Determine if a position is within specified rectangle
BOOL txwInRectangle                             // RET   position in rectangle
(
   short               row,                     // IN    position row
   short               col,                     // IN    position column
   TXRECT             *rect                     // IN    rectangle
);

// Calculate intersection of two rectangles return not-empty
BOOL txwIntersectRect                           // RET   non-empty intersection
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2
   TXRECT             *rect                     // OUT   intersection
);

// Determine if first rectangle overlaps second (invalidates its border)
BOOL txwOverlappingRect                         // RET   overlapping rectangle
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2                     // IN    rectangle 2
);

// Calculate union of two rectangles
void txwUniteRect
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2
   TXRECT             *rect                     // OUT   union
);

// Test if rectangle is normalized, and make normalized when requested
BOOL txwNormalRect                              // RET   input was normalized
(
   BOOL                normalize,               // IN    update rectangle
   TXRECT             *rect                     // INOUT rectangle
);

//               ÚÄÄÄÄÄÄÄÄÄ¿
//         rec1  ³111111111³                 111 is 1st exclusion
//               ³2222 ÚÄÄÄÅÄÄÄÄÄÄ¿          222 is 2nd exclusion
//               ÀÄÄÄÄÄÅÄÄÄÙ      ³ rec2
//                     ÀÄÄÄÄÄÄÄÄÄÄÙ
//
// Calculate 2 exclusion areas for two overlapping rectangles
BOOL txwExclusionRectangles                     // RET   2nd exclusion valid
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2
   TXRECT             *xcl1,                    // OUT   exclusion 1
   TXRECT             *xcl2                     // OUT   exclusion 2
);


// Calculate new rectangle, 2nd top-left as move & 2nd bottom-right as size
void txwMoveSizeRect
(
   TXRECT             *rec1,                    // IN    rectangle 1
   TXRECT             *rec2,                    // IN    rectangle 2 (reference)
   TXRECT             *rect                     // OUT   moved/sized rect 1
);

// Calculate transposition of a rectangle, using top-left of 2nd as reference
void txwTransposeRect
(
   TXRECT             *rec1,                    // IN    rectangle 1 (relative)
   TXRECT             *rec2,                    // IN    rectangle 2 (reference)
   TXRECT             *rect                     // OUT   transposed rect 1
);

// Calculate clipped sub-string start-position and length
short  txwCalculateStringClip                   // RET   clipped length
(
   short               row,                     // IN    draw dest row
   short               col,                     // IN    draw dest column
   short               len,                     // IN    draw length
   TXRECT             *clip,                    // IN    clip-rectangle
   short              *offset                   // OUT   clipped offset
);

// Add a line to History-buffer of specified EntryField window
BOOL txwAdd2History                             // RET   string added
(
   TXWHANDLE           hwnd,                    // IN    entryfield window
   char               *line                     // IN    string to be added
);

// Create history-buffer data-structure
BOOL txwInitializeHistory
(
   TXHIST             *hb,                      // IN    history structure
   USHORT              entries,                 // IN    entries in history
   USHORT              esize                    // IN    max size of entry
);

// Terminate history-buffer data-structure
void txwTerminateHistory
(
   TXHIST             *hb                       // IN    history structure
);

// Create selist from a History object, optionally filtered by a prefix string
ULONG TxSelistHistory                           // RET   result
(
   TXHIST             *hb,                      // IN    history structure
   char               *prefix,                  // IN    prefix select string
   TXSELIST          **list                     // OUT   selection list
);

// Get history-element string from the buffer, optional move current position
char  *txwGetHistory
(
   TXHIST             *hb,                      // IN    history structure
   TXHSELECT           select                   // IN    THIS/NEXT/PREV
);

// Put new character-string in the history buffer, at lowest order
void txwPutHistory
(
   TXHIST             *hb,                      // IN    history structure
   ULONG               order,                   // IN    order (or 0)
   char               *value                    // IN    string value to store
);

// Delete the current entry in the history buffer, set by Get or Find
BOOL txwDelCurrentHistory                       // RET   entry deleted
(
   TXHIST             *hb                       // IN    history structure
);

// Find a (sub) character-string in the history buffer
char *txwFindHistory                            // RET   found history string
(
   TXHIST             *hb,                      // IN    history structure
   char               *value,                   // IN    string value to store
   BOOL                use_current              // IN    start from current
);

// Reverse find a (sub) character-string in the history buffer (old to new)
char *txwRfndHistory                            // RET   found history string
(
   TXHIST             *hb,                      // IN    history structure
   char               *value,                   // IN    string value to match
   BOOL                use_current              // IN    start from current
);

// Sort history-buffer data-structure
void txwSortHistory
(
   TXHIST             *hb,                      // IN    history structure
   BOOL                ascend                   // IN    sort order wanted
);

// Update entryfield from SHORT integer value to HEX/DEC string form
void txwDlgUs2Field
(
   TXWHANDLE           hwnd,                    // IN    field handle
   USHORT              value,                   // IN    integer value
   char               *field,                   // INOUT field value
   int                 length,                  // IN    formatting length
   BOOL                hexadec                  // IN    hexadecimal formatted
);


// Update entryfield from SHORT integer value to HEX/DEC string form
void txwDlgUl2Field
(
   TXWHANDLE           hwnd,                    // IN    field handle
   ULONG               value,                   // IN    integer value
   char               *field,                   // INOUT field value
   int                 length,                  // IN    formatting length
   BOOL                hexadec                  // IN    hexadecimal formatted
);

// Update SHORT integer value from entryfield HEX/DEC string form
void txwDlgField2Us
(
   USHORT             *value,                   // OUT   integer value
   char               *field,                   // IN    field value
   BOOL                hexadec                  // IN    hexadecimal formatted
);

// Update ULONG integer value from entryfield HEX/DEC string form
void txwDlgField2Ul
(
   ULONG              *value,                   // OUT   integer value
   char               *field,                   // IN    field value
   BOOL                hexadec                  // IN    hexadecimal formatted
);

// Test if the scrollbuffer window uses specified color conversion
BOOL txwSbColorStyle                            // RET   specified style used
(
   int                 ccstyle                  // IN    color conversion style
);


/*======== Screen related functions, TXWIOP.C ===============================*/

// Initialize low level screen/terminal handling
ULONG TxScreenInitialize
(
   void
);

// Terminate low level screen/terminal handling
ULONG TxScreenTerminate
(
   void
);

// Draw a character-string at specified position, clip to optional clip-rect
void txwScrDrawCharString
(
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXRECT             *clip,                    // IN    opt clip-rectangle
   char               *str                      // IN    string to draw
);

// Draw a character-string at position in specified color, using clip-rect
void txwScrDrawCharStrCol
(
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXRECT             *clip,                    // IN    opt clip-rectangle
   char               *str,                     // IN    string to draw
   BYTE                color                    // IN    color attribute
);

// Draw a cell-string at specified position, clip to optional clip-rect
void txwScrDrawCellString
(
   short               row,                     // IN    destination row
   short               col,                     // IN    destination column
   TXRECT             *clip,                    // IN    opt clip-rectangle
   TXCELL             *cell,                    // IN    cell-string to draw
   short               size,                    // IN    nr of cells
   ULONG               altcol                   // IN    color conversions
);

// Change color for rectangle to simulate shadow; BG = black, FG = low-intens
void txwScrShadowRectangle
(
   TXRECT             *rect                     // IN    destination rectangle
);

// Fill area for rectangle with specified cell (clear area)
void txwScrFillRectangle
(
   TXRECT             *rect,                    // IN    destination rectangle
   TXCELL              cell                     // IN    cell-string to draw
);

// Scroll rectangle up or down, fill new with specified cell
void txwScrScrollRectangle
(
   TXRECT             *rect,                    // IN    scroll rectangle
   int                 scroll,                  // IN    scroll lines (+ = up)
   TXCELL              cell                     // IN    cell-string to draw
);

// Determine visible lines on screen, no scrollbars; fixed 40 for DOS and OS/2
ULONG txwScrVisibleLines
(
   void
);

#if defined (WIN32)

// display CONSOLE info structure details
void txwDisplayConsoleInfo
(
   void
);
#endif


#endif
