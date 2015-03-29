//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2014 Fsys Software and Jan van Wijk
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
// TX windows help manager
//
// Author: J. van Wijk
//
// JvW  12-09-2001 Initial version
// JvW  08-05-2014 Added a separate SECTION level data, and next/prev section
// JvW  09-05-2014 Added section selection from a list of section names
// JvW  14-05-2014 Added popup search dialog for the HELP texts, and actual search
// JvW  14-05-2014 Added item selection, for items in current section
// JvW  23-05-2014 Added help-section search (after startup, and load help-file)
// JvW  10-06-2014 Added 'search titles only' to help dialog; Footer text in GREP list

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface

#define TXWH_IDCHAR            '#'              // helpid character in text

#define TXWH_HELPHELP          0xfffffffe       // help-id help-on-help
#define TXWH_WID_HELP          0xff01           // window id help screen

#define TXWH_STATIC_T          0x00000000       // static, no free required
#define TXWH_ALLOCTXT          0x00000001       // dynamic allocated text

#define TXWH_SECT_CHUNK        20               // allocate chunks of sections
#define TXWH_NO_SECTION        0xffffffff

#define TXWH_DOC_EXT           "txt"
#define TXWH_DOC_DIR           "doc"

// ITEM holds just the linenr for THIS item in the text, and a title
typedef struct txwhelp_item
{
   ULONG               helpid;                  // helpid for this item
   ULONG               linenr;                  // line-nr for the item
   char              **text;                    // attached text array (alias!)
   TXTM                title;                   // short item title
} TXWHELP_ITEM;                                 // end of struct "txwhelp_item"

// SECTION holds base/last ID, a title, array of items (linenr+title) and text
typedef struct txwhelp_sect
{
   ULONG               baseid;                  // baseid for this section
   ULONG               lastid;                  // lastid for this section
   ULONG               flags;                   // static/allocated text etc
   ULONG               itemcount;               // number of items in section
   TXWHELP_ITEM       *itemdata;                // attached item array
   char              **text;                    // attached text array (primary)
   TXTS                name;                    // section name   (for linking)
   TXTM                title;                   // short section title
} TXWHELP_SECT;                                 // end of struct "txwhelp_section"

static ULONG           txwh_allocsect = 0;      // allocated items
static ULONG           txwh_sect_used = 0;      // used sections
static ULONG           txwh_this_sect = 0;      // current section index

static TXWHELP_SECT   *txwh_sects = NULL;       // allocated array of sections

static TXSELIST       *grep_list  = NULL;
static ULONG           grep_index = 0;          // current index in grep list

static ULONG           txwh_request_code = TXWH_REQ_NONE;

// maximum number of search results in the grep list
#define TXWH_GREP_LIMIT        8192

// Each loaded file will start at the next base, in big increments of 1000
// allowing for almost 800 files to be loaded with up to 1000 items per file
#define TXWH_LOAD_INCREMENT      1000

static ULONG           txwh_next_load_base = TXWH_LOADED_SECTION_BASE;

static BOOL            txwh_help_registred = FALSE;


static char           *helphelp[] =
{
   "#000 TXwindows and its help system",
   "",
   " This is the help for the TxWindows userinterface and its help system",
   "",
   " TxWindows is a windowing system a bit like the operating system GUI's",
   " as MS-Windows, OS/2 or the Mac.  Unlike these GRAPHICAL interfaces,",
   " TxWindows is based on textmode displays only. This allows usage in",
   " simple systems and even from (DOS or OS/2) bootdiskettes.",
   "",
   "",
   " Use the <PgDn> key to browse through the various help items ...",
   "",
#if defined (DARWIN)
   " Note: To use the PgUp/PgDn/Home/End keys in the Terminal with DFSee:",
   "",
   "       Terminal->Preferences->[Settings]->[Keyboard]",
   "",
   "       Find each key there, and change the buffer scroll action into",
   "       one to send it to DFSee: 'Action' select 'send string to shell:'",
   "       The codes to be sent are (insert the \033 by using the Esc key):",
   "",
   "       Home           \033[1~",
   "       End            \033[4~",
   "       PgUp           \033[5~",
   "       PgDn           \033[6~",
   "",
#endif
#if defined (HAVEMOUSE)
   TXHELPITEM(100, "Mouse operation")
   "",
#if defined (DEV32)
   " Note: For optimal mouse usage make sure the 'Mouse Actions' in the ",
   "       (upper left) system-menu for a Windowed session is NOT checked!",
   "",
#endif
#if defined (WIN32)
   " Note: For optimal mouse usage make sure the 'Quick Edit Mode'",
   "       in the properties for the command window is NOT checked!",
   "",
#endif
   " Allthough keyboard oriented, most windows and (menu)lists can be",
   " operated using a mouse as well. The supported mouse actions are:",
   "",
   "   Button1 click    Will move the FOCUS to the window under the mouse",
   "   (left)           pointer when possible, and if succesfull operate",
   "                    the corresponding control, like selecting from a",
   "                    (menu)list, press a button, toggle a checkbox or",
   "                    radiobutton, or any other appropriate action ...",
   "",
   "   Button1 drag     When holding the button down after clicking on a",
   "                    window title-bar, the whole window can be moved",
   "                    by dragging the mouse-cursor. It will be resized",
   "                    when the drag starts from the lower-right corner",
   "",
   "   Button2 click    Available to application, no action by the UI itself",
   "   (right)          Button-2 is sometimes used by clipboard packages.",
   "",
   "   Button3 click    Available to application, no action by the UI itself",
   "                    Button-3 is often combined with the mouse wheel.",
   "",
   "   Key modifiers    Button actions combined with Ctrl, Alt or Shift",
   "                    keys are recognized, but currently not used by",
   "                    the UI itself. The application may use them.",
   "",
   "   Mouse chord      Mouse-chords, multiple buttons down at the same",
   "                    have no special meaning, and the behaviour when",
   "                    using them is undefined.",
   "",
   "   Wheel up/down    When supported by the mouse and mouse driver, the",
#if defined (WIN32)
   "                    wheel can be used to scroll UP/DOWN or LEFT/RIGHT",
   "                    This works by direct translation to ARROW keys and",
   "                    the PgUp/PgDn keys, so the position of the mouse",
   "                    is NOT significant. The Ctrl/Alt/Shift keys will",
   "                    modify the wheel behaviour as follows:",
   "",
   "   None             Wheel will scroll one LINE at a time (UP/DOWN key)",
   "   Shift            Wheel will scroll one PAGE at a time   (PgUp/PgDn)",
   "   Alt              Wheel will scroll one CHAR at a time  (LEFT/RIGHT)",
   "   Ctrl             Wheel will scroll the SCROLLBUFFER   (text output)",
   "                    instead of the currently active window",
   "",
   "   Combinations of the above work as expected, except for Alt+Shift",
#else
   "                    wheel can be used to scroll UP/DOWN per line or",
   "                    per page, depending on the mouse driver logic.",
#endif
   "",
   TXHELPITEM(110, "Menu operation with mouse")
   "",
   " You can activate/deactive the menu by clicking on the DESKTOP BORDER",
   " (title/footer), the top lines of the text output window, inside the",
   " command-line entryfield if the menu is up, or using the <F10> key.",
   "",
   " Many dialogs can be closed by clicking on the [X] close button in the",
   " upper right corner of the window border (same as using Escape/Cancel.",
   "",
   " In large text windows (like HELP) or large lists, scroll-indicators",
   " will appear in the right hand side of the window border, apart from",
   " using the keyboard, you can also click these to scroll up and down",
   " one line at a time, or one page at a time. The indicators used are:",
   "",
   "       ÄÄÄÄÄÄÄÄÄÄ¿                                   ÄÄÄÄÄÄÄÄÄÄ¿",
   "                     scroll up, 1 line    (Up)                -",
   "       text or       scroll up, 1 page    (PgUp)    text or   U",
   "       list      ³                                   list      ³",
   "       window    ³                                   window    ³",
   "       CP-437        scroll down, 1 page  (PgDn)    ASCII     D",
   "                     scroll down, 1 line  (Down)              +",
   "       ÄÄÄÄÄÄÄÄÄÄÙ                                   ÄÄÄÄÄÄÄÄÄÄÙ",
   "",
#else
   " Note: In the current implementation TxWindows can be controlled using",
   "       the keyboard only, mouse-support may be added at a later time.",
   "",
#endif
   TXHELPITEM(200, "TXWindows Desktop operation")
   "",
   " The screen for a TxWindows application always uses a DESKTOP window",
   " as background, other windows displayed will allways be on top of it.",
   " Most applications use a large scrolling output window (SCROLLBUFFER)",
   " that receives outputlines from the application which is scrolled up",
   " when the screen fills up. This scrollbuffer is usually very large and",
   " older output can be viewed using navigation keys like PgUp/PgDn",
   "",
   " User input is either in the form of COMMANDS that are entered in an",
   " input field usually near the bottom of the screen, or in the form of",
   " DIALOG-windows that may have fields, buttons, lists and so on ...",
   "",
   " A special form of dialog is the MENU BAR, it can be activated using",
   " the F10 function key or clicking the desktop/scrollbuffer title.",
   " The menu supports selection of actions from several pulldown menus.",
   " You can get specific help on each item in the menu by pressing the",
   " F1 functionkey while the item is selected, or by clicking it using",
   " mouse button-2 (usually the most right button).",
   "",
   "",
   TXHELPITEM(210, "Keyboard usage")
   "",
   " The following keys are the most important ones to control TxWindows:",
   "",
   "   Tab / Backtab Move to the next or previous selectable window, like",
   "                 between the input-field and scrollbuffer, or between",
   "                 the control-windows in an input dialog.",
   "",
   "   Esc           End or abort the current dialog, often equivalent to",
   "                 CANCELING it. Many applications also use the Esc-key",
   "                 to abort long-running commands like searching.",
   "",
   "   Enter         End current input dialog, making the entered value(s)",
   "                 available to the application for processing.",
   "",
   "   Space         Perform (mouse like) 'click' operation on any button.",
   "                 Toggles a checkbox or radio button, 'push' pushbutton.",
   "",
   "   Arrow-keys    Navigate within an inputfield, scroll textlines in a",
   "                 text-window like help or move to the next or previous",
   "                 controlwindow in a dialog (buttons, fields ...)",
   "",
   "   PgUp / PgDn   Scroll textlines in a window one page at a time",
   "   Home / End    Move to begin/end of an inputfield, or output text",
   "   Ctrl-Home/End Move to first/last line of a text output window",
   "",
   "   Ctrl + Arrow  Scroll the text in the SCROLLBUFFER in the direction",
   "                 of the arrow when possible. In an inputfield the left",
   "                 and right arrow will move the cursor over one word.",
   "",
   "   Ctrl PgUp/Dn  Scoll the SCROLLBUFFER up or down, one page at a time",
   "",
   "           Note: SCROLLBUFFER movement even works from OTHER windows!",
   "",
   "",
   "   F1            Get help on the current window, inputfield, button ...",
   "                 A second F1 will give more generic help, like on the",
   "                 complete dialog, and a third will present the help on",
   "                 the TxWindows system (this text).",
   "",
   "   F3            Usually end the current dialog, canceling input, or",
   "                 end the application itself when on the commandline",
   "",
   "   F4            End the current dialog, ACCEPTING the input",
   "                 (like an [ OK ] button)",
   "",
   "   F10           Toggle the main-menu ON or OFF",
   "",
   "   F11           Present command history in a selection popup list",
   "",
   "   Alt + F2      Send COMPLETE help-file to the SCROLLBUFFER (screen)",
   "",
   "   Alt + F5      Restore resized, minimized or maximized window to its",
   "                 previous size. (initial, or just before min/maximize)",
   "",
   "   Alt + F10     Maximize the current window to the size of its parent",
   "                 client-window. (usually the SCROLLBUFFER or desktop).",
   "",
   "   Alt + F9      Minimize the current window to just its titlebar, or",
   "   F12           make it completely invisible when no titlebar present.",
   "                 This allows you to look under or behind the window.",
   "                 When the main inputfield is active, the whole DESKTOP",
   "                 will be minimized revealing the previous screen.",
   "",
   "   Alt + F12     Cycle through available SCROLLBUFFER color-schemes:",
   "                 NORMAL: White on black background, use all FG colors",
   "                 BRIGHT: White on black background, bright colors only",
   "                 INVERT: Black on white background, no bright colors",
   "                         Select the scheme with best readability",
   "",
   "   Ctrl + l      Repaint ALL windows on the desktop, can be usefull in",
   "   Ctrl + r      very rare cases to correct screen-painting problems.",
   ""
   "   Alt  + c      Change appearance of the user interface using:",
   "                   - Arrow-key UP/DOWN:    change overall color scheme",
   "                   - Arrow-key LEFT/RIGHT: change output-window colors",
   "                   - Page Up / Page Down:  change line-style scheme",
   "",
   "   Alt  + m      Move or Resize the currently displayed window:",
   "                   - Arrow-keys: Move or Resize thw window",
   "                   - Insert-key: Switch between Move and Resize",
   "",
#if defined (DUMP)
   "   Alt  + /      Cycle through available TRACE settings: OFF, TITLEBAR",
   "                 or SCREEN/LOGFILE (depending on other trace settings).",
   "",
#endif
   " Other available (function) keys might be displayed in the bottom line",
   " for a dialog window.",
   "",
   TXHELPITEM(250, "Entryfield keyboard usage")
   "",
   " For entryfields the following editing keys are available:",
   "",
   "    Insert              Toggle between Insert and replace mode",
   "    Delete              Delete the character at the cursor position",
   "    Backspace           Delete the character before the cursor position",
   "    Home                Move cursor to the start of the entryfield",
   "    End                 Move cursor to the end of the entryfield",
   "    Ctrl + Backspace    Clear entry-field completely, making it empty",
   "    Ctrl + E            Clear entry-field from cursor to end-of-field",
   "    Ctrl + Right Arrow  Move one word to the right in the field",
   "    Ctrl + Left  Arrow  Move one word to the left  in the field",
   "",
   " When the field supports a history buffer (like a command field would):",
   "",
   "    Ctrl + D            Delete current line from history, if full match",
   "    Ctrl + K            Add current line to the history, no execute",
   "    Up         [prefix] Recall previous (older)  command from history",
   "    Down       [prefix] Recall next     (newer)  command from history",
   "    F11        [prefix] Show history contents in a selection popup list",
   "                        On each of these, the prefix is the part of the",
   "                        commandline BEFORE the cursor that will be used",
   "                        to FILTER the result of the requested action",
   "",
   "    Other keys are either inserted/replaced in the entryfield content,",
   "    like letters, digits and interpunction, or simply ignored",
   "",
   "",
   "",
   TXHELPITEM(300, "TxWindows Help system")
   "",
   " The HELP system",
   "",
   " Help can be called from many places, using a help-id to specify",
   " the subject for wich help is required. This can be a specific part",
   " of the user-interface like a button or a window, or it can be a",
   " subject known to the application like a command.",
   "",
   " The help subjects can be called by pressing a [Help] button, using",
   " the <F1> function key, or any other application specified method.",
   "",
   " Once help is displayed, it can be navigated using the following keys:",
   "",
   "    PgUp / PgDn   Scroll help window up/down by one page",
   "    Left / Right  Scroll help window left/right",
   "    Up   / Down   Scroll help window up  / down",
   "    Home / End    Scroll help window to left/right margin",
   "",
   "    Esc           Quit help, return to previous active window",
   "",
   "    F1            Get more generic help, usually related to the item",
   "                  currently displayed (the 'owner' window) and when",
   "                  pressed a 3rd time, help on TxWindows (these pages)",
   "",
   "    F5            Go to begin of the previous HELP section loaded",
   "",
   "    F6            Go to begin of the next HELP section loaded",
   "",
   "    F7            Search dialog, search current or all loaded sections",
   "",
   "    F8            Toggle between search-result list (aka GREP result) and",
   "                  actual text showing the search-hit in full context, OR",
   "                  start search with 'search-results in list' option set.",
   "",
   "    F9            Select a help-section from a list of loaded sections",
   "",
   "    F10           Select help-item from list of items in current section",
   "",
   "    Alt + F2      Send COMPLETE help-file to the SCROLLBUFFER (screen)",
   "                  Note that this may be thousands of lines!",
   "",
   "    Alt + F7      Remove search-result highlight (unmark)",
   "    Alt + U       Unmark",
   "",
   "    Ctrl + B      Search again, BACKWARD, when search argument is known,",
   "    'b', 'p'      or start search-dialog with 'backward' option set.",
   "    Alt + 2       (Alt-1/2 is a very comfortable combo for most keyboards)",
   "",
   "    Ctrl + N",
   "    Ctrl + F      Search again, FORWARD, when search argument is known,",
   "    'f', 'n'      or start search-dialog with 'backward' option cleared.",
   "    Alt + 1       (Alt-1/2 is a very comfortable combo for most keyboards)",
   "",
   "    Ctrl + G      Search FORWARD, ALL SECTIONS if search argument known,",
   "    'g'           or search-dialog with 'current section' option cleared.",
   "",
   "    Ctrl + L      Search BACKWARD, CURRENT SECTION if search argument known,",
   "    'l'           or search-dialog with 'current section' option set.",
   "",
   " Since the help window is MODAL, you cannot switch to other",
   " application windows using the <Tab> key",
   "",
   "",
   TXHELPITEM(500, "Help system search dialog")
   "",
   " The search dialog lets you specify a phrase to look for in the help contents",
   " according to several other search options that can be selected:",
   "",
   "  [ ] Case-sensitive searching         Ignore case when comparing texts",
   "  [ ] Match on whole-word only         Ignore substring matches",
   "  [ ] Current help section only        Don't search other help sections",
   "  [ ] Reverse searching, backward      Search in backward direction",
   "  [ ] Show search results in list,     GREP functionality, search and show",
   "      and toggle between them          results in list, toggle between list",
   "                                       and search-hits in actual help-items",
   "",
   "      Search in current or all HELP sections, for:",
   "  [                                                         ]",
   "",
   " The phrase or word(s) to search for, as specified by the other options.",
   "",
   " Searching starts from the current highlighted search-hit (mark) if present",
   " or from the top (FORWARD) or bottom (BACKWARD) of the help window when not.",
   "",
   "",
   "",
   "",
   TXHELPITEM(500, "Text output search dialog")
   "",
   " The search dialog lets you specify a phrase to look for in the 'scrollbuffer'",
   " where all application text output is displayed, so you can search through all",
   " previously displayed out.",
   "",
   "  [ ] Case-sensitive searching         Ignore case when comparing texts",
   "  [ ] Match on whole-word only         Ignore substring matches",
   "  [ ] Reverse searching, backward      Search in backward direction",
   "  [ ] Search in Item titles only       Search Item titles only (#nnn lines)",
   "  [ ] Show search results in list,     GREP functionality, search and show",
   "      and toggle between them          results in list, toggle between list",
   "                                       and search-hits in actual text output",
   "",
   "      Search in the text output window (scrollbuffer) for:",
   "  [                                                         ]",
   "",
   " The phrase or word(s) to search for, as specified by the other options.",
   "",
   " Searching starts from the current highlighted search-hit (mark) if present",
   " or from the top (FORWARD) or bottom (BACKWARD) of the output window when not.",
   "",
   "",
   TXHELPITEM(810, "HELP section selection")
   "",
   " This list lets you select a section of HELP items, which can be:",
   "",
   "      - System ones, supplied by the TX windowing system itself",
   "",
   "      - Application ones, supplied directly by the application",
   "",
   "      - Loaded ones, automatically of manually loaded from an",
   "        external help or documentation file",
   "",
   TXHELPITEM(820, "HELP item selection")
   "",
   " This list lets you select one HELP item to view from",
   " all items present in the CURRENT help section.",
   "",
   TXHELPITEM(830, "Text grep/find result selection")
   "",
   " This list lets you select one search-result from the list of",
   " search results, and view the corresponding full text with the",
   " search hit highlighted in the full text",
   "",
   NULL
};

static char           *notfound[] =
{
   "",
   " The requested help item could not be found.",
   "",
   " Please report this to your support contact for this software",
   " and include the help-id number as specified below, as well as.",
   " information on how to reproduce this possible error",
   "",
   NULL,
   NULL
};

#define TXWH_NOTFOUND_LINE 7                    // insert text at line ...


static char txwhlp_footer[]    = "F1=Help F7=Search F8=Grep/Results F9=Sections F10=Items F12=Minimize";


// Present HELP-section FIND options dialog making the info available
static ULONG txwhFindDialog
(
   void
);

// Dialog window procedure, for the help-window to add selection and searching
static ULONG txwHelpDlgWinProc                  // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Add a helptext of one section with multiple items, with allocation flag
static ULONG txwAddSectionHelpText
(
   ULONG               baseid,                  // IN    base id type/section
   char               *name,                    // IN    section name (16)
   char               *title,                   // IN    section title
   char               *text[],                  // IN    help text array
   BOOL                allocated                // IN    text array is allocated
);

// Get section space, check for existing one and add extra space when needed
static ULONG txwGetSectionSpace                 // RET   section index
(
   char               *name                     // IN    Section name
);

// Find existing section by name
static ULONG txwFindSection                     // RET   section index
(
   char               *name                     // IN    Section name
);

// Find a help item by its helpid (simple linear search)
static TXWHELP_ITEM *txwFindItemId              // RET   ref to item or NULL
(
   ULONG               helpid                   // IN    helpid
);

// Get reference to next help section from specified one, wrap around at end
static TXWHELP_SECT *txwNextSection             // RET   ref to sect or NULL
(
   ULONG              *section                  // INOUT section index
);

// Get reference to prev help section from specified one, wrap around at begin
static TXWHELP_SECT *txwPrevSection             // RET   ref to sect or NULL
(
   ULONG              *section                  // INOUT section index
);

// Sort HELP section data-structure on BASEID, ascending
static void txwSortSections
(
   void
);

// Compare TXWHELP_SECT items on baseid numeric value for qsort ascending
static int txwSectionBaseid
(
   const void         *one,
   const void         *two
);

// Build selection-list with HELP sections currently present
static TXSELIST *txwHelpSectionSelist           // RET   selection list or NULL
(
   char              **text                     // IN    Text for help section
);

// Build selection-list with HELP items present in current section
static TXSELIST *txwHelpItemSelist           // RET   selection list or NULL
(
   ULONG               linenr,                  // IN    current linenumber
   char              **text                     // IN    Text for help section
);

// Build list of search results
static TXSELIST *txwHelpGrepSelist              // RET   selection list or NULL
(
   void
);

/*========================== HELP FIND ==========================================================*/

static BOOL fdCheckCase  = FALSE;               // Check Case-sensitive
static BOOL fdCheckWord  = FALSE;               // Check Whole-word
static BOOL fdCheckSect  = FALSE;               // Check current section
static BOOL fdCheckBack  = FALSE;               // Check Search backward
static BOOL fdCheckList  = FALSE;               // Check search result list
static BOOL fdCheckTitl  = FALSE;               // Check search titles only

static TXTM  fdEntrySearchArgument = "";        // search argument

/*
[ ] Case-sensitive searching   [ ] Current help section only
[ ] Match on whole-word only   [ ] Reverse searching, backward
[ ] Search Item titles only

[ ] Show search results in list, toggle between them (aka GREP)
*/
#define   TXWHELPFINDWIDGETS  6

#define   TXWHELP_ARG_WIDTH  60

static TXWIDGET  txwHelpFindWidgets[TXWHELPFINDWIDGETS] =  // order determines TAB-order!
{
   {0, 1, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdCheckCase, "Case-sensitive searching")},
   {0,35, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdCheckSect, "Current help section only")},
   {1, 1, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdCheckWord, "Match on whole-word only")},
   {1,35, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdCheckBack, "Reverse searching, backward")},
   {2, 1, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdCheckTitl, "Search in Item titles only")},
   {4, 1, 1, 65, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdCheckList, "Show search results in list, toggle between them (aka GREP)")},
};

static TXGW_DATA txwHelpFindDlg =
{
   TXWHELPFINDWIDGETS,                          // number of widgets
   TXWH_SEARCHDIALOG,                           // dialog help
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   txwHelpFindWidgets                           // array of widgets
};


/*************************************************************************************************/
// Present HELP-section FIND options dialog making the info available
/*************************************************************************************************/
static ULONG txwhFindDialog
(
   void
)
{
   ULONG               rc = TX_ABORTED;         // function return

   ENTER();

   while (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &txwHelpFindDlg,
         "Search in current or all HELP sections, for:",
         " Specify HELP search string and options ", TXWH_SEARCHDIALOG,
         TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER, TXWHELP_ARG_WIDTH, fdEntrySearchArgument)
      != TXDID_CANCEL)
   {
      if (strlen(fdEntrySearchArgument))
      {
         rc = NO_ERROR;                         // search arguments available
         break;                                 // out of while ...
      }
      else
      {
         TxMessage( TRUE, TXWH_SEARCHDIALOG, "You must specify a search argument ...");
      }
   }
   RETURN (rc);
}                                               // end 'txwhFindDialog'
/*-----------------------------------------------------------------------------------------------*/


/*****************************************************************************/
// Display standard help dialog with text for specified help-id
/*****************************************************************************/
ULONG txwHelpDialog
(
   ULONG               helpid,                  // IN    id for specific helpitem
   ULONG               request,                 // IN    specific request code
   char               *param                    // IN    string parameter
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXWHELP_ITEM       *hi;
   char              **display = notfound;      // text to display
   ULONG               topline = 0;             // starting line number
   TXTM                title;
   TXTM                info;

   ENTER();
   TRACES(("helpid: %8.8lx = %lu  requestcode:%8.8lx\n", helpid, helpid, request));
   TRWINS("1");

   if (!txwh_help_registred)
   {
      txwRegisterHelpText( TXWH_HELPREG, "txwindows", "TX Userinterface, Help and search", helphelp);
      txwh_help_registred = TRUE;
   }

   txwh_request_code = TXWH_REQ_NONE;           // no special request by default

   if ((helpid != 0) && (helpid != TXWH_NO_MORE_HELP))
   {
      strcpy( title, " Help item: ");           // start of help title
      if ((helpid == TXWH_HELPHELP)  ||         // help on help
          (helpid == TXWH_REQUEST_ID) )         // special requests
      {
         if ((hi = txwFindItemId( TXWH_HELPREG)) != NULL) // help on help
         {
            display = hi->text;
            topline = hi->linenr;
            strcat( title, hi->title);
         }
         else
         {
            display = helphelp;
            strcat( title, "TxWindows and the helpsystem itself");
         }
         if (helpid == TXWH_REQUEST_ID)         // prepare special requests
         {
            txwh_request_code = request;
            if (param != NULL)
            {
               strcpy( fdEntrySearchArgument, param);
            }
         }
      }
      else if ((hi = txwFindItemId(helpid)) != NULL) // is item registred ?
      {
         display = hi->text;
         topline = hi->linenr;
         strcat( title, hi->title);
      }
      else                                      // prepare notfound help-page
      {
         display = notfound;
         strcat( title, "not found");

         sprintf( info, " The requested help-id is: 0x%8.8lx = %lu",
                                helpid, helpid);

         notfound[ TXWH_NOTFOUND_LINE] = info;
      }
      strcat( title, " ");

      if (txwIsWindow( TXHWND_DESKTOP))         // is there a desktop ?
      {
         TXRECT           position;             // reference size/position
         TXWHANDLE        hframe;               // help frame
         TXWHANDLE        hview;                // help view text
         TXWINDOW         window;               // frame window data
         ULONG            ownerhelp = TXWH_HELPHELP;
         ULONG            focushelp = TXWH_HELPHELP;
         TXWINDOW        *win;
         short            helpWidth;            // desired help window width

         txwQueryWindowRect( TXHWND_DESKTOP, FALSE, &position);
         TRECTA( "desktop size", (&position));

         helpWidth = (position.right < 85) ? 76 : 82;

         if ((txwValidateHandle((TXWHANDLE) txwa->focus, &win)) != NULL)
         {
            focushelp = win->helpid;
            if ((txwValidateHandle((TXWHANDLE) txwa->focus->owner, &win)) != NULL)
            {
               ownerhelp = win->helpid;
               if ((focushelp == TXWH_HELPHELP) && (ownerhelp == TXWH_HELPHELP))
               {
                  ownerhelp = TXWH_NO_MORE_HELP; // stop recursive help on help
               }
               if (((win->client.left < 10) && (win->client.top  < 5)) ||
                   ((win->client.left + helpWidth + 2 < txwa->screen.right )   &&
                    (win->client.top  + 22            < txwa->screen.bottom) ) )
               {
                  position = win->client;       // owners position
               }
            }
         }
         TRECTA( "pos ", (&position));

         position.left += 2;                    // cascade the new window
         position.top  += 1;
         if (position.left + helpWidth < txwa->screen.right)
         {
            position.right = helpWidth;
         }
         else
         {
            position.right = txwa->screen.right - position.left;
         }
         position.bottom = txwa->screen.bottom - position.top;
         if (position.bottom > 25)
         {
            position.bottom -= 4;               // allow some room
         }
         TRECTA( "size", (&position));

         TRACES(("focus help-id = %8.8lx = %lu\n", focushelp, focushelp));
         TRACES(("owner help-id = %8.8lx = %lu\n", ownerhelp, ownerhelp));

         txwSetupWindowData(
            position.top, position.left, position.bottom, position.right,
            TXWS_CANVAS | TXWS_CAST_SHADOW | TXWS_MOVEABLE,
            TXWH_HELPHELP,                      // help-on-help id
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.dlgFocusID  = TXWH_WID_HELP;    // focus to help-text viewer
         hframe = txwCreateWindow( TXHWND_DESKTOP, TXW_FRAME, 0, 0, &window, NULL);

         txwSetupWindowData(
            0,  0, position.bottom, position.right,
            TXWS_FRAMED        |                // borders (scroll indicator)
            TXCS_CLOSE_BUTTON  |                // include a close button [X]
            TXWS_HCHILD_SIZE   |                // resize with parent
            TXWS_VCHILD_SIZE,
            ownerhelp,                          // help-on-help = owner
            ' ', ' ', TXWSCHEME_COLORS,
            title, txwhlp_footer,
            &window);
         window.tv.topline  = topline;
         window.tv.leftcol  = 0;
         window.tv.maxtop   = TXW_INVALID;
         window.tv.maxcol   = TXW_INVALID;
         window.tv.markLine = 0;
         window.tv.markCol  = 0;                 // test search-marking
         window.tv.markSize = 0;
         window.tv.buf      = display;
         hview = txwCreateWindow( hframe, TXW_TEXTVIEW, hframe, 0, &window,
                                  txwDefWindowProc);
         txwSetWindowUShort( hview, TXQWS_ID, TXWH_WID_HELP);

         rc = txwDlgBox( TXHWND_DESKTOP, 0, txwHelpDlgWinProc, hframe, NULL);
      }
   }
   RETURN (rc);
}                                               // end 'txwHelpDialog'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Dialog window procedure, for the help-window to add selection and searching
/*****************************************************************************/
static ULONG txwHelpDlgWinProc                  // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc    = NO_ERROR;
   TXWHANDLE           target;                  // target window handle
   TXLN                status;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      switch (msg)
      {
         case TXWM_INITDLG:                     // Help view dialog just created
            switch (txwh_request_code)
            {
               case TXWH_REQ_ITEMLIST:
                  txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_F10);
                  break;

               case TXWH_REQ_SECTIONLIST:
                  txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_F9);
                  break;

               case TXWH_REQ_FINDDIALOG:
                  fdCheckList = FALSE;
                  txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_F7);
                  break;

               case TXWH_REQ_GREPDIALOG:
                  fdCheckList = TRUE;
                  txwPostMsg( hwnd, TXWM_CHAR, mp1, TXk_F7);
                  break;

               case TXWH_REQ_GREPSTRING:
                  txwPostMsg( hwnd, TXWM_PERFORM_GREP, 0, 0);
                  break;

               case TXWH_REQ_NONE:
               default:
                  break;
            }
            rc = txwDefDlgProc( hwnd, msg, mp1, mp2); // make sure dialog gets initialized
            break;

         case TXWM_CHAR:
            if ((target = txwWindowFromID( hwnd, TXWH_WID_HELP)) != 0)
            {
               TXWHELP_SECT       *dest = NULL; // optional destination section
               TXSELIST           *list = NULL;
               TXWINDOW          *tview = txwWindowData( target);
               short              sy    = tview->client.bottom - tview->client.top +1;

               switch (mp2)
               {
                  case 'g':
                  case TXc_G:                   // search forward, all sections
                     fdCheckSect  = FALSE;
                  case 'f':
                  case 'n':
                  case TXa_2:
                  case TXc_N:
                  case TXc_F:                   // search forward
                     fdCheckBack  = FALSE;
                     if (strlen( fdEntrySearchArgument) && (tview->tv.markSize != 0))
                     {
                        txwPostMsg( hwnd, TXWM_PERFORM_SEARCH, 0, 0);
                     }
                     else                       // dialog when no argument yet
                     {
                        txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F7);
                     }
                     break;

                  case 'l':
                  case TXc_L:                   // search backward, this section
                     fdCheckSect  = TRUE;
                  case 'b':
                  case 'p':
                  case TXa_1:
                  case TXc_B:                   // search backward
                     fdCheckBack  = TRUE;
                     if (strlen( fdEntrySearchArgument) && (tview->tv.markSize != 0))
                     {
                        txwPostMsg( hwnd, TXWM_PERFORM_SEARCH, 0, 0);
                     }
                     else                       // dialog when no argument yet
                     {
                        txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F7);
                     }
                     break;

                  case TXk_F5:
                     dest = txwPrevSection( &txwh_this_sect);
                     tview->tv.buf = dest->text;
                     tview->tv.topline = 0;
                     sprintf( tview->title, "Help Section: %6.6lu %-30s", dest->baseid, dest->title);
                     txwInvalidateWindow( hwnd, TRUE, TRUE);
                     break;

                  case TXk_F6:
                     dest = txwNextSection( &txwh_this_sect);
                     tview->tv.buf = dest->text;
                     tview->tv.topline = 0;
                     sprintf( tview->title, "Help Section: %6.6lu %-30s", dest->baseid, dest->title);
                     txwInvalidateWindow( hwnd, TRUE, TRUE);
                     break;

                  case '/':
                  case TXk_F7:
                     if (txwhFindDialog() == NO_ERROR)
                     {
                        if (fdCheckList)        // want list output
                        {
                           txwPostMsg( hwnd, TXWM_PERFORM_GREP, 0, 0);
                        }
                        else                    // execute single search
                        {
                           txwPostMsg( hwnd, TXWM_PERFORM_SEARCH, 0, 0);
                        }
                     }
                     break;

                  case TXk_F8:
                     if (grep_list != NULL)
                     {
                        ULONG  listcode = TXDID_OK;
                        TXRECT where    = {18,0,0,0}; // fixed position
                        TXTM   title;
                        TXTM   footr;

                        grep_list->selected = grep_index;

                        TRACES(("grep_list:%p at index: %lu\n", grep_list, grep_index));

                        sprintf( title, " %lu search hits, select with <F8> to view in context ", grep_list->count);
                        sprintf( footr, " <Enter> or <F8> = Select/Toggle ");
                        listcode = txwListBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &where, title, footr,
                                   TXWH_HELPREG + 830, TXLB_MOVEABLE,
                                   cMenuTextStand, cMenuBorder_top, // same color as menu
                                   grep_list);

                        if ((listcode >= TXDID_MAX) && (listcode != TXDID_CANCEL)) // valid and not escaped
                        {
                           ULONG   searchSect;
                           ULONG   searchLine;
                           short   searchCol;
                           char   *description = grep_list->items[ grep_list->selected]->desc;

                           TRACES(("Selected %lu = '%s'\n", grep_list->selected, description));

                           if (sscanf( description, "%lu %lu %hd", &searchSect, &searchLine, &searchCol) == 3)
                           {
                              if (searchSect != txwh_this_sect) // we ended up in another section
                              {
                                 TXWHELP_SECT  *new;
                                 TRACES(("Section: %lu to %lu\n", txwh_this_sect, searchSect));
                                 new = &(txwh_sects[ searchSect]);
                                 tview->tv.buf      = new->text;
                                 tview->tv.topline  = 0;
                                 txwh_this_sect     = searchSect;
                                 sprintf( tview->title, "Help Section: %6.6lu %-30s", new->baseid, new->title);
                              }
                              tview->tv.markLine = searchLine;
                              tview->tv.markCol  = searchCol;
                              tview->tv.markSize = strlen( fdEntrySearchArgument);

                              if ((searchLine <   tview->tv.topline)     || // mark outside
                                  (searchLine >= (tview->tv.topline + sy))) // visible area
                              {
                                 ULONG     margin = (fdCheckBack) ? sy - 4 : 3;
                                 if (searchLine >= margin)
                                 {
                                    tview->tv.topline = searchLine - margin;
                                 }
                                 else
                                 {
                                    tview->tv.topline = 0;
                                 }
                              }
                              txwInvalidateWindow( hwnd, TRUE, TRUE);
                           }
                           grep_index = grep_list->selected + 1; // auto advance for toggle
                           if (grep_index >= grep_list->count)
                           {
                              grep_index = 0;
                           }
                        }
                        txwInvalidateAll();
                     }
                     else
                     {
                        fdCheckList = TRUE;
                        txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F7);
                     }
                     break;

                  case TXa_F7:
                  case TXa_U:
                     tview->tv.markSize = 0;    // unmark
                     txwInvalidateWindow( hwnd, TRUE, TRUE);
                     break;

                  case TXk_F9:
                     if ((list = txwHelpSectionSelist( tview->tv.buf)) != NULL)
                     {
                        ULONG  listcode  = TXDID_OK;
                        TXRECT where  = {18,0,0,0}; // fixed position

                        listcode = txwListBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &where,
                                 " Select a HELP section to view ", "",
                                   TXWH_HELPREG + 810, TXLB_MOVEABLE,
                                   cMenuTextStand, cMenuBorder_top, // same color as menu
                                   list);

                        if ((listcode >= TXDID_MAX) && (listcode != TXDID_CANCEL)) // valid and not escaped
                        {
                           txwh_this_sect = listcode - TXDID_MAX; // remember section index
                           dest = &(txwh_sects[ txwh_this_sect]); // selected section info
                           tview->tv.buf = dest->text;
                           tview->tv.topline = 0;
                           sprintf( tview->title, "Help Section: %6.6lu %-30s", dest->baseid, dest->title);
                           txwInvalidateWindow( hwnd, TRUE, TRUE);
                        }
                        txSelDestroy( &list);
                     }
                     break;

                  case TXk_F10:
                     if ((list = txwHelpItemSelist( tview->tv.topline, tview->tv.buf)) != NULL)
                     {
                        ULONG  listcode  = TXDID_OK;
                        TXRECT where     = {18,0,0,0}; // fixed position

                        listcode = txwListBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &where,
                                 " Select a HELP item to view ", "",
                                   TXWH_HELPREG + 820, TXLB_MOVEABLE,
                                   cMenuTextStand, cMenuBorder_top, // same color as menu
                                   list);

                        if ((listcode >= TXDID_MAX) && (listcode != TXDID_CANCEL)) // valid and not escaped
                        {
                           tview->tv.topline = listcode - TXDID_MAX;
                           txwInvalidateWindow( hwnd, TRUE, TRUE);
                        }
                        txSelDestroy( &list);
                     }
                     break;

                  default:
                     rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                     break;
               }
            }
            break;

         case TXWM_PERFORM_GREP:                // perform GREP, no dialog
            if (grep_list != NULL)
            {
               txSelDestroy( &grep_list);
            }
            if ((grep_list = txwHelpGrepSelist()) != NULL)
            {
               grep_index = 0;
               txwPostMsg( hwnd, TXWM_CHAR, 0, TXk_F8); // activate the toggle select list
            }
            else
            {
               sprintf( status, " Search argument '%s' not found. ", fdEntrySearchArgument);
               txwSetSbviewStatus( status, cSchemeColor);
            }
            break;

         case TXWM_PERFORM_SEARCH:              // perform a search operation now
            if ((target = txwWindowFromID( hwnd, TXWH_WID_HELP)) != 0)
            {
               TXLN                status;
               TXWHELP_SECT       *dest = NULL; // optional destination section
               TXWINDOW *tview = txwWindowData( target);
               ULONG     section_index;         // index of searched section
               ULONG     searchLine;            // line number to start the search from
               short     searchCol;
               short     sy;                    // vertical size of window in lines
               BOOL      otherSect = FALSE;     // did go to another section
               char    **sect1 = tview->tv.buf; // section active when starting search
               char    **sectionText = sect1;   // and current section being searched
               int       sanity;                // sanity check counter

               sy    = tview->client.bottom - tview->client.top +1;

               if (tview->tv.markSize != 0)     // have a mark? (start from there)
               {
                  searchLine = tview->tv.markLine;
               }
               else if (fdCheckBack)            // backwards, start from end-window
               {
                  searchLine = tview->tv.topline + sy;
               }
               else
               {
                  searchLine = tview->tv.topline - 1;
               }

               TRACES(("DO_SEARCH on text:%p at line %lu\n", sect1, searchLine));

               //- Multi sections search by looping, until first section searched AGAIN
               for (sanity = 0,  section_index = txwh_this_sect;
                    sanity <= txwh_sect_used;
                    sanity++) // loop safeguard
               {
                  if (txTextSearch( sectionText, fdEntrySearchArgument,
                                    fdCheckCase, fdCheckWord, fdCheckBack, fdCheckTitl,
                                    &searchLine, &searchCol))
                  {
                     tview->tv.markLine = searchLine;
                     tview->tv.markCol  = searchCol;
                     tview->tv.markSize = strlen( fdEntrySearchArgument);

                     if ((searchLine <   tview->tv.topline)     || // mark outside
                         (searchLine >= (tview->tv.topline + sy))) // visible area
                     {
                        ULONG     margin = (fdCheckBack) ? sy - 4 : 3;
                        if (searchLine >= margin)
                        {
                           tview->tv.topline = searchLine - margin;
                        }
                        else
                        {
                           tview->tv.topline = 0;
                        }
                     }
                     if (sectionText != sect1)  // we ended up in another section
                     {
                        TRACES(("Section: %p to %p\n", sect1, sectionText));
                        tview->tv.buf  = sectionText; // equal to dest->text;
                        txwh_this_sect = section_index;
                        sprintf( tview->title, "Help Section: %6.6lu %-30s", dest->baseid, dest->title);
                     }
                     txwInvalidateWindow( hwnd, TRUE, TRUE);
                     break;                     // out of multi-section while
                  }
                  else
                  {
                     if ((fdCheckSect) || (otherSect && (sectionText == sect1)))
                     {
                        TRACES(("Not found! (and changed to initial section)\n"));
                        sprintf( status, " Search argument '%s' not found. ", fdEntrySearchArgument);
                        txwSetSbviewStatus( status, cSchemeColor);
                        txwInvalidateWindow( hwnd, TRUE, TRUE);
                        break;                  // out of multi-section while
                     }
                     else                       // wrap to next/prev section
                     {
                        if (fdCheckBack)
                        {
                           dest = txwPrevSection( &section_index);
                        }
                        else
                        {
                           dest = txwNextSection( &section_index);
                        }
                        TRACES(("Section: %p wrap to %p\n", sectionText, dest->text));
                        sectionText = dest->text;
                        searchLine  = ((ULONG) -1); // start search from OUTSIDE text
                        otherSect   = TRUE;
                     }
                  }
               }
            }
            break;

         default:
            rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txwHelpDlgWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set starting base-ID for help-items loaded from file; Will auto-increment!
/*****************************************************************************/
ULONG txwSetLoadedHelpBaseId                    // RET   Current base value or 0
(
   ULONG               newBase                  // IN    New base value to set
)
{
   ULONG               rc = txwh_next_load_base; // function return

   ENTER();

   if ((newBase >= TXWH_LOADED_SECTION_BASE) && (newBase < TXWH_SYSTEM_SECTION_BASE))
   {
      txwh_next_load_base = newBase;
   }
   else
   {
      rc = 0;                                   // signal failure
   }
   RETURN (rc);
}                                               // end 'txwSetLoadedHelpBaseId'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Load a helptext from given name in ABSOLUTE PATH, CURRENT DIR or ./doc DIR
/*****************************************************************************/
ULONG txwLoadRegisterHelpFile                   // RET   OK or FILE_NOT_FOUND
(
   char               *name,                    // IN    filename (or basename)
   char               *title                    // IN    optional section title
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                filename;
   TXTM                basename;
   FILE               *fp;
   ULONG               lines;
   char              **text = NULL;             // text from file

   ENTER();

   strcpy( filename, name);
   TxFnameExtension( filename, TXWH_DOC_EXT);   // append default extension

   if ((fp = fopen( filename, "r")) == NULL)
   {
      sprintf( filename, "..%c%s%c%s", FS_PATH_SEP, TXWH_DOC_DIR, FS_PATH_SEP, name);
      TxFnameExtension( filename, TXWH_DOC_EXT); // append default extension

      if ((fp = fopen( filename, "r")) == NULL)
      {
         sprintf( filename, ".%c%s%c%s", FS_PATH_SEP, TXWH_DOC_DIR, FS_PATH_SEP, name);
         TxFnameExtension( filename, TXWH_DOC_EXT); // append default extension

         if ((fp = fopen( filename, "r")) == NULL)
         {
            rc = ERROR_FILE_NOT_FOUND;
         }
      }
   }

   if (fp != NULL)                              // we have an open file
   {
      if ((text = txFile2Text( fp, &lines)) != NULL)
      {
         strcpy( basename, TxGetBaseName( filename));
         TxStripExtension( basename);

         rc = txwAddSectionHelpText( txwh_next_load_base, basename, title, text, TRUE);
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   RETURN (rc);
}                                               // end 'txwLoadRegisterHelpFile'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Register a STATIC helptext of one section with multiple items
/*****************************************************************************/
ULONG txwRegisterHelpText
(
   ULONG               baseid,                  // IN    base id type/section
   char               *name,                    // IN    section name (16)
   char               *title,                   // IN    section title
   char               *text[]                   // IN    help text array
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   rc = txwAddSectionHelpText( baseid, name, title, text, FALSE);

   RETURN (rc);
}                                               // end 'txwRegisterHelpText'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get text for one specific help-id, ends at '#' on linestart or NULL ptr
/*****************************************************************************/
char **txwGetHelpText                           // RET   HELP text or NULL
(
   ULONG               helpid                   // IN    id for helpitem
)
{
   char              **rc = NULL;               // function return
   TXWHELP_ITEM       *hi;

   ENTER();
   TRACES(("helpid:%8.8lx = %lu\n", helpid, helpid));

   if (helpid != TXWH_NO_MORE_HELP)
   {
      if ((hi = txwFindItemId(helpid)) != NULL) // is item registred ?
      {
         rc = &( hi->text[ hi->linenr]);
      }
   }
   RETURN (rc);
}                                               // end 'txwGetHelpText'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Free helpmanager resources
/*****************************************************************************/
void txwTerminateHelpManager
(
   void
)
{
   TXWHELP_SECT       *hs;
   ULONG               s;

   ENTER();

   for (s = 0, hs = txwh_sects; hs && (s < txwh_sect_used); s++, hs++)
   {
      TRACES(("Section#:% 4d  itemcount: % 4lu baseid: %8.8lx = %lu\n",
               s, hs->itemcount, hs->baseid, hs->baseid));

      if (hs->flags & TXWH_ALLOCTXT)
      {
         TRACES(("Freeing allocated text at: %p\n", hs->text));
         txFreeText( hs->text);                 // free text, when not static
      }
      TxFreeMem( hs->itemdata);                 // free item array for section
   }
   TxFreeMem( txwh_sects);                      // free section array

   txwh_allocsect = 0;                          // reset section counts
   txwh_sect_used = 0;

   txSelDestroy( &grep_list);                   // remove grep result list

   VRETURN ();
}                                              // end 'txwTerminateHelpManager'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Add a helptext of one section with multiple items, with allocation flag
/*****************************************************************************/
static ULONG txwAddSectionHelpText
(
   ULONG               baseid,                  // IN    base id type/section
   char               *name,                    // IN    section name (16)
   char               *title,                   // IN    section title
   char               *text[],                  // IN    help text array
   BOOL                allocated                // IN    text array is allocated
)
{
   ULONG               rc = NO_ERROR;           // function return
   ULONG               newSection;              // new section index
   ULONG               items;                   // nr of items
   ULONG               linenr;                  // line number
   ULONG               index;                   // item index
   ULONG               lastid = 0;              // last helpid
   TXWHELP_SECT       *hs;                      // sect pointer
   TXWHELP_ITEM       *hi;                      // item pointer
   char              **tx;                      // text array pointer
   char               *s;
   BOOL                implicitItem = FALSE;

   ENTER();
   TRACES(("adding base id : %lu for %s\n", baseid, name));

   for (tx = text, items = 0; tx && *tx; tx++)  // include default 1st item
   {
      if (*tx[0] == TXWH_IDCHAR)                // one extra for each explicit #
      {
         items++;                               // count items
      }
   }
   if (items == 0)
   {
      items++;
      implicitItem = TRUE;
   }

   newSection = txwGetSectionSpace( name);
   if (newSection != TXWH_NO_SECTION)
   {
      hs = &(txwh_sects[ newSection]);          // new section structure

      if ((hi = TxAlloc( items, sizeof( TXWHELP_ITEM))) != NULL)
      {
         //- cleanup existing section, or update dynamic next baseid
         if (hs->baseid != TXW_INVALID)         // replace existing, free existing text
         {
            if (hs->flags & TXWH_ALLOCTXT)      // existing was allocated
            {
               txFreeText( hs->text);           // free text, when not static
            }
            TxFreeMem( hs->itemdata);           // free item array for section
         }
         else                                   // new section, increment next baseid
         {
            if (allocated)                      // new is allocated text (loaded file)
            {
               txwh_next_load_base += TXWH_LOAD_INCREMENT;
            }
         }

         //- now set new section information
         hs->itemdata  = hi;
         hs->baseid    = baseid;
         hs->itemcount = items;
         hs->text      = text;
         hs->flags     = (allocated) ? TXWH_ALLOCTXT : TXWH_STATIC_T;
         TxCopy( hs->name,  name,  TXMAXTS);
         TxCopy( hs->title, title, TXMAXTM);

         //- Copy helpitem information from the text, at start and lines starting with #
         for ( linenr = 0,  tx = text, index = 0;
                  (tx) && (*tx)   &&  (index < items);
               linenr++,    tx++)
         {
            s = *tx;                            // point to current line
            if (*s == TXWH_IDCHAR)
            {
               if (implicitItem)                // add default item when no explicit ones
               {
                  lastid = baseid;
                  TxCopy( hi->title, title, TXMAXTM);
               }
               else
               {
                  lastid = baseid + TxaParseNumber( s+1, TX_RADIX_STD_CLASS, NULL);
                  if ((s = strchr( *tx, ' ')) != NULL)
                  {
                     TxCopy( hi->title, s+1, TXMAXTM);
                  }
                  else
                  {
                     strcpy( hi->title, "-");
                  }
               }
               //- TRACES(("adding help id : %lu = '%s'\n", lastid, hi->title));
               hi->helpid = lastid;
               hi->linenr = linenr;
               hi->text   = text;
               hi++;                            // to next item structure
            }
         }
         hs->lastid = lastid;

         txwSortSections();                     // keep section array sorted
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   RETURN (rc);
}                                               // end 'txwAddSectionHelpText'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get section space, check for existing one and add extra space when needed
// Free existing text for allocated sections, when it is to replaced
/*****************************************************************************/
static ULONG txwGetSectionSpace                 // RET   section index
(
   char               *name                     // IN    Section name
)
{
   ULONG               rc;                       // function return

   ENTER();

   //- check if section by this name already exists
   if ((rc = txwFindSection( name)) == TXWH_NO_SECTION)
   {
      txwh_sect_used++;                         // add one section to count
      if (txwh_allocsect < txwh_sect_used)      // allocated now too small?
      {
         char             *m = (char *) txwh_sects;

         txwh_allocsect += TXWH_SECT_CHUNK;
         m = realloc( m, (size_t) (txwh_allocsect * sizeof(TXWHELP_SECT)));

         TRACES(("Reallocated Section at: %p to hold %lu sections\n", m, txwh_allocsect));

         if (m != NULL)
         {
            txwh_sects  = (TXWHELP_SECT *) m;
            rc = txwh_sect_used - 1;
         }
      }
      else
      {
         rc = txwh_sect_used - 1;               // still room in current allocated chunk
      }
      if (rc != TXWH_NO_SECTION)                // initialize new section
      {
         txwh_sects[ rc].baseid   = TXW_INVALID; // make sure it is seen as empty
         txwh_sects[ rc].itemdata = NULL;
         txwh_sects[ rc].text     = NULL;
      }
   }
   else                                         // existing, free text if needed
   {
      TRACES(("Found existing section '%s'", name));
   }
   RETURN (rc);
}                                               // end 'txwGetSectionSpace'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find existing section by name
/*****************************************************************************/
static ULONG txwFindSection                     // RET   section index
(
   char               *name                     // IN    Section name
)
{
   ULONG               rc = TXWH_NO_SECTION;    // function return
   TXWHELP_SECT       *hs;
   ULONG               s;

   ENTER();

   for (s = 0, hs = txwh_sects; hs && (s < txwh_sect_used); s++, hs++)
   {
      //- TRACES(("Section#:% 4d  name: '%s' for wanted: '%s\n", s, hs->name, name));

      if (strcasecmp( name, hs->name) == 0)                    //
      {
         rc = s;                                // found the item
         break;
      }
   }
   RETURN (rc);
}                                               // end 'txwFindSection'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find a help item by its helpid (two-level linear search, section, then item)
/*****************************************************************************/
static TXWHELP_ITEM *txwFindItemId              // RET   ref to item or NULL
(
   ULONG               helpid                   // IN    helpid
)
{
   TXWHELP_ITEM       *rc = NULL;               // function return
   TXWHELP_ITEM       *hi;
   TXWHELP_SECT       *hs;
   ULONG               s;
   ULONG               i;

   ENTER();
   TRACES(("#sections:% 4lu  helpid: %8.8lx = %lu\n", txwh_sect_used, helpid, helpid));

   for (s = 0, hs = txwh_sects; hs && (s < txwh_sect_used); s++, hs++)
   {
      //- TRACES(("Section#:% 4d  itemcount: % 4lu baseid: %8.8lx = %lu\n", s, hs->itemcount, hs->baseid, hs->baseid));

      for (i = 0, hi = hs->itemdata; hi && (i < hs->itemcount); i++, hi++)
      {
         //- TRACES(("items#:% 4d  helpid: %8.8lx = %lu\n", i, hi->helpid, hi->helpid));
         if (hi->helpid == helpid)
         {
            txwh_this_sect = s;                 // remember section index
            rc = hi;                            // found the item
            break;
         }
      }
   }
   RETURN (rc);
}                                               // end 'txwFindItemId'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get reference to next help section from specified one, wrap around at end
/*****************************************************************************/
static TXWHELP_SECT *txwNextSection             // RET   ref to sect or NULL
(
   ULONG              *section                  // INOUT section index
)
{
   ULONG               i  = *section;

   ENTER();
   TRACES(("Section IN : %lu\n", *section));

   if (i < (txwh_sect_used -1))
   {
      *section = i + 1;
   }
   else
   {
      *section = 0;
   }
   TRACES(("Section OUT: %lu\n", *section));

   RETURN (&(txwh_sects[*section]));
}                                               // end 'txwNextSection'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get reference to prev help section from specified one, wrap around at begin
/*****************************************************************************/
static TXWHELP_SECT *txwPrevSection             // RET   ref to sect or NULL
(
   ULONG              *section                  // INOUT section index
)
{
   ULONG               i  = *section;

   ENTER();
   TRACES(("Section IN : %lu\n", *section));

   if (i > 0)
   {
      *section = i - 1;
   }
   else
   {
      *section = txwh_sect_used - 1;
   }
   TRACES(("Section OUT: %lu\n", *section));

   RETURN (&(txwh_sects[*section]));
}                                               // end 'txwPrevSection'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Sort HELP section data-structure on BASEID, ascending
/*****************************************************************************/
static void txwSortSections
(
   void
)
{
   ENTER();

   if (txwh_sect_used > 1)                      // useful only when more :)
   {
      qsort( txwh_sects, (size_t) txwh_sect_used,
                          sizeof(TXWHELP_SECT),
                          txwSectionBaseid);
   }
   VRETURN();
}                                              // end 'txwSortSections'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare TXWHELP_SECT items on baseid numeric value for qsort ascending
/*****************************************************************************/
static int txwSectionBaseid
(
   const void         *one,
   const void         *two
)
{
   int                 rc = 0;
   TXWHELP_SECT       *i1 = (TXWHELP_SECT *) one;
   TXWHELP_SECT       *i2 = (TXWHELP_SECT *) two;

   if      (i1->baseid < i2->baseid)
   {
      rc = -1;
   }
   else if (i1->baseid > i2->baseid)
   {
      rc = +1;
   }

   TRACES(("%+1d  i1:%P i2:%P  seq1:% 5lu  seq2:% 5lu\n",
            rc,   i1,   i2,   i1->baseid, i2->baseid));

   return(rc);
}                                               // end 'txwSectionBaseid'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Build selection-list with HELP sections currently present
/*****************************************************************************/
static TXSELIST *txwHelpSectionSelist           // RET   selection list or NULL
(
   char              **text                     // IN    Text for help section
)
{
   TXSELIST           *list  = NULL;            // total list

   ENTER();

   if (txwh_sect_used != 0)
   {
      TXS_ITEM           *item;                 // single item
      ULONG               lsize;                // list-size
      ULONG               i;
      ULONG               sect;                 // section index
      TXWHELP_SECT       *hs;                   // section info
      BOOL                sepLoaded   = FALSE;  // Need separator for 'loaded'
      BOOL                sepSystem   = FALSE;  // Need separator for 'system'

      for (sect = 0, hs = txwh_sects; hs && (sect < txwh_sect_used); sect++, hs++)
      {
         if (hs->baseid >= TXWH_LOADED_SECTION_BASE)
         {
            if (hs->baseid >= TXWH_SYSTEM_SECTION_BASE)
            {
               sepSystem = TRUE;
            }
            else
            {
               sepLoaded = TRUE;
            }
         }
      }
      lsize = txwh_sect_used;
      if (sepLoaded)
      {
         lsize++;
      }
      if (sepSystem)
      {
         lsize++;
      }

      if (TxSelCreate( lsize, lsize, lsize,
                       TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
      {
         list->astatus = TXS_AS_NOSTATIC      | // all dynamic allocated
                         TXS_LST_DYN_CONTENTS;

         hs     = txwh_sects;                   // start of section array
         sect   = 0;
         for (i = 0; i < lsize; i++)            // all list entries
         {
            if ((item  = TxAlloc( 1, sizeof(TXS_ITEM))) != NULL)
            {
               list->count    = i +1;           // actual items in list
               list->items[i] = item;           // attach item to list

               item->helpid = TXWH_USE_OWNER_HELP; // from owner-menu-item

               if (hs->baseid >= TXWH_LOADED_SECTION_BASE)
               {
                  if (hs->baseid >= TXWH_SYSTEM_SECTION_BASE)
                  {
                     if (sepSystem)             // need this separator?
                     {
                        item->flags = TXSF_DISABLED | TXSF_SEPARATOR;
                        sepSystem = FALSE;      // but only once ...
                        continue;               // with next list item
                     }                          // (same section!)
                  }
                  else
                  {
                     if (sepLoaded)             // need this separator?
                     {
                        item->flags = TXSF_DISABLED | TXSF_SEPARATOR;
                        sepLoaded = FALSE;      // but only once ...
                        continue;               // with next list item
                     }                          // (same section!)
                  }
               }

               if (sect < txwh_sect_used)       // safeguard check ...
               {
                  if (((item->text = TxAlloc( 1, TXMAXTM)) != NULL) &&
                      ((item->desc = TxAlloc( 1, TXMAXTM)) != NULL)  )
                  {
                     if (hs->text == text)
                     {
                        list->selected = i;     // make current section default
                     }
                     item->value   = TXDID_MAX + sect;

                     sprintf( item->text,       // length must be LESS than TXMAXTM!
                              "%-16.16s #%-4lu = %-35.35s",
                              hs->name, hs->itemcount, hs->title);

                     sprintf( item->desc,       // length must be LESS than TXMAXTM!
                             "%-16.16s Id:%6.6lu-%6.6lu #%-4lu, %-35.35s",
                              hs->name, hs->baseid, hs->lastid, hs->itemcount, hs->title);

                     //- get unique SelectChar starting at text[25] ...
                     item->index = TxSelGetCharSelect( list, i, 25);

                     //- TRACES(("value %d for section %lu at %p\n",    item->value, sect, hs));
                     //- TRACES(("text: %d '%s'\n", strlen(item->text), item->text));
                     //- TRACES(("desc: %d '%s'\n", strlen(item->desc), item->desc));
                  }
                  sect++;                       // advance to next section info
                  hs++;
               }
            }
         }
      }
   }
   else
   {
      list = TxSelEmptyList( "- No help sections available -",
                             "Application did not register any help sections!", FALSE);
   }
   RETURN( list);
}                                               // end 'txwHelpSectionSelist'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Build selection-list with HELP items present in current section
/*****************************************************************************/
static TXSELIST *txwHelpItemSelist              // RET   selection list or NULL
(
   ULONG               linenr,                  // IN    current linenumber
   char              **text                     // IN    Text for help section
)
{
   TXSELIST           *list = NULL;            // total list
   TXS_ITEM           *item;                    // single item
   ULONG               lsize;                   // list-size
   ULONG               i;                       // index in list AND help-items
   TXWHELP_ITEM       *hi;                      // helpitem info
   TXWHELP_SECT       *hs   = &(txwh_sects[ txwh_this_sect]);

   ENTER();

   lsize = hs->itemcount;
   if (TxSelCreate( lsize, lsize, lsize,
                    TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
   {
      list->astatus = TXS_AS_NOSTATIC      |    // all dynamic allocated
                      TXS_LST_DYN_CONTENTS;

      for (i = 0; i < lsize; i++)               // all list entries
      {
         if ((item  = TxAlloc( 1, sizeof(TXS_ITEM))) != NULL)
         {
            list->count    = i +1;              // actual items in list
            list->items[i] = item;              // attach item to list

            item->helpid = TXWH_USE_OWNER_HELP; // from owner-menu-item

            if (((item->text = TxAlloc( 1, TXMAXTM)) != NULL) &&
                ((item->desc = TxAlloc( 1, TXMAXTM)) != NULL)  )
            {
               hi = &(hs->itemdata[ i]);        // current item info

               if (hi->linenr <= linenr)
               {
                  list->selected = i;           // advance until at current line
               }
               item->value = TXDID_MAX + hi->linenr;

               sprintf( item->text,             // length must be LESS than TXMAXTM!
                        "#%-6lu = line %-6lu %-46.46s",
                         hi->helpid, hi->linenr, hi->title);

               sprintf( item->desc,             // length must be LESS than TXMAXTM!
                       "%-16.16s Item #%-6lu = %-46.46s",
                        hs->name, hi->helpid, hi->title);
            }
         }
      }
   }
   RETURN( list);
}                                               // end 'txwHelpItemSelist'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Build list of search results in one or all help sections
/*****************************************************************************/
static TXSELIST *txwHelpGrepSelist              // RET   selection list or NULL
(
   void
)
{
   TXSELIST           *list  = NULL;            // total list
   TXWHELP_SECT       *hs;                      // section info
   TXS_ITEM           *item;                    // single item
   ULONG               lsize;                   // list-size
   ULONG               i;                       // index in list items
   ULONG               section_index;           // index of searched section
   ULONG               searchLine;              // line number to start the search from
   short               searchCol;
   char              **sectionText;             // current text being searched

   ENTER();

   lsize = TXWH_GREP_LIMIT;
   if (TxSelCreate( lsize, lsize, lsize,
                    TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
   {
      list->astatus = TXS_AS_NOSTATIC      |    // all dynamic allocated
                      TXS_LST_DYN_CONTENTS;

      section_index = (fdCheckSect) ? txwh_this_sect : 0;
      searchLine    = ((ULONG) -1);             // start search from OUTSIDE text

      hs = &(txwh_sects[ section_index]);
      sectionText = hs->text;

      for (i = 0; i < lsize;)                   // at most 'lsize' entries
      {
         if (txTextSearch( sectionText, fdEntrySearchArgument,
                           fdCheckCase, fdCheckWord, FALSE, fdCheckTitl,
                           &searchLine, &searchCol))
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
                     sprintf( item->text,       // length must be LESS than TXMAXTM!
                              "%-16.16s %-6lu: %-50.50s",
                               hs->name, searchLine, hs->text[ searchLine]);
                  }
                  else
                  {
                     sprintf( item->text,       // length must be LESS than TXMAXTM!
                              "%-16.16s %-6lu: ...%-47.47s",
                               hs->name, searchLine, hs->text[ searchLine] + searchCol - 25);
                  }
                  sprintf( item->desc,          // length must be LESS than TXMAXTM!
                          "%-4.4lu %-8.8lu %-3.3d %-50.50s",
                           section_index, searchLine, searchCol, hs->text[ searchLine]);
               }
               i++;
            }
         }
         else                                   // no (more) search hits in current section
         {
            if ((fdCheckSect) || (section_index == (txwh_sect_used - 1)))
            {
               TRACES(("Not found, and in final section. Got %lu items\n", list->count));
               break;                           // out of enclosing item FOR loop
            }
            else                                // wrap to next/prev section
            {
               hs = txwNextSection( &section_index);
               TRACES(("Section: %p wrap to %p\n", sectionText, hs->text));
               sectionText = hs->text;
               searchLine  = ((ULONG) -1);      // start search from OUTSIDE text
            }
         }
      }
      if (list->count == 0)                     // nothing found
      {
         txSelDestroy( &list);

         TxMessage( TRUE, 0, "There are no HELP lines or titles "
                             "that match the specified search argument:\n\n%s",
                              fdEntrySearchArgument);
      }
      else
      {
         list->tsize = list->count;
         list->vsize = list->count;             // make sure small lists use small window
      }
   }
   RETURN( list);
}                                               // end 'txwHelpGrepSelist'
/*---------------------------------------------------------------------------*/

