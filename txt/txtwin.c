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
// TXTst windowed user interface
//
// Author: J. van Wijk
//
// 1.00 08-07-2003    Initial version, derived from dfswin.c and tptest.c

#include <txlib.h>                              // TX library interface

#include <txtver.h>                             // TXT version info
#include <txt.h>                                // command executor
#include <txtwin.h>                             // windowed entry point

#define TXT_SCROLL_L   9999                     // total  5 Mb
#define TXT_SCROLL_W    250

// minimum scroll-buffer for memory constrained environments (mainly DOS)
#define TXT_SMALLB_L   5000                     // total  1 Mb
#define TXT_SMALLB_W    100

#define TXT_WID_ENTRY      201                  // window id entry-field
#define TXT_WID_SCROLL     202                  // window id scroll-buffer

#define TXT_H_APPLIC       100                  // help ID TXTst application
#define TXT_H_INTERF       101                  // help ID TXTst interface


#define TXT_HIST_SIZE       96
#define TXT_HIST_LINE  TXMAXLN

static TXLN        entryftxt = "";
static TXHIST      cmd_history;                 // history buffer info

static TXWINDOW    entryfwin;
static TXWINDOW    scrbuffwin;
static TXWINDOW    desktopwin;

static TXWHANDLE   desktop = 0;
static TXWHANDLE   sbufwin = 0;
static TXWHANDLE   entrwin = 0;

static TXSBDATA    scrollBufData =
{
   20,
   80,
   60,
   0,
   0,0,0,                                       // no marked area
   FALSE,                                       // no scrolling when in middle
   TRUE,                                        // wrap on write on long lines
   0,
   NULL
};


static  char       *mainwinhelp[] =
{
   "",
   "#100 TXTst application and desktop window",
   "",
   " Note: To get help on the TxWindows userinterface, press <F1> again",
   "",
   " This is the built in help text for the TXTst application.",
   "",
   " It lists (function) key assignments specific to TXTst, the generic",
   " assignments are listed with the TxWindows help information (F1).",
   "",
   "       F3         Quit TXTest completely",
   "       F4         Save current screen-buffer to a file (txtest.log)",
   "       F5         Start generic test dialog",
   "       F6         Show a simple message-box",
   "       F7         Present an input dialog",
   "       F8         Start test dialog for a selection list",
   "       F10        Activate the MenuBar with pulldown menus",
   "       F11        Execute command 'about' TXTst version information",
   "",
   " On large screens (> 80x25), the desktop will have a visible border",
   " around it. This can also be forced using program switches as follows:",
   "",
   "    -f         = force a desktop frame border to be used",
   "    -f:yes     = force a desktop frame",
   "    -f-        = Do NOT use a frame border around the desktop",
   "",
   "",
   "  For help on commands, use '?' in the entry-field or select the",
   "  'application command help' from the menu and for more detailed",
   "  information and examples see the TXTst documentation",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#101 Command entry field and output text window",
   "",
   " Note: To get more generic help on TXTst, press <F1>",
   "",
   " T H E   C O M M A N D   E N T R Y   F I E L D",
   " =============================================",
   "",
   " This is the (normally green) field at the bottom of the TXTst screen.",
   "",
   " Commands can be typed here, and will be processed by the TXTst",
   " application when <Enter> is pressed.",
   "",
   " The syntax of a (multiple) TXTst command is:",
   "",
   "    cmd1 [-options | parameters]#cmd2 [-options | parameters]#cmd3 ...",
   "",
   " Where the '#' is the separator character (adjustable with '-s' switch)",
   " that is used to execute multiple commands as used a lot from scripts.",
   "",
   " In interactive mode you normally execute a single command at a time.",
   "",
   " Options are recognized by the first character after any space being",
   " a '-' and are case sensitive, so '-r' and '-R' are different!",
   " Options may appear at any place in the command (free format)",
   "",
   " Options can have a numeric or string value, the string part can use",
   " single or double quotes to allow embedded spaces in them. Explicit",
   " values, when given, must be specified directly after the semicolon",
   "",
   " Some examples of valid option syntax are:",
   "",
   "    '-r'     or '-refresh'   or '-r+'        refresh option, value YES",
   "    '-r:no'  or '-refresh:0' or '-r-'        refresh option, value NO",
   "    '-s:256' or '-size:0xff' or '-s:33,c'    size option variants",
   "    '-i:WIN2000' or '-iba:\"eCS GA\"'          for a setboot option ",
   "",
   " All parts of the command that are not options, are command parameters",
   " They too can use single or double quotes to allow embedded spaces.",
   "",
   " For more details and specific command-options see: DFSCMDS/TXT and",
   " the usage descriptions for some commands like 'find' and 'create'.",
   "",
   "",
   " After completing the command it will be saved in the historybuffer.",
   " Older commands can be retrieved from this buffer by using the UP",
   " and DOWN keys and <Alt> + [ or <Alt> + ] (completion)",
   "",
   " For very long commands, that do not fit in the visible part of the",
   " entryfield, the contents will be scrolled to the left so the last part",
   " of the field where you are typing stays visible all the time.",
   "",
   " While the entryfield has the focus, the output from executed commands",
   " that appears in the text output window above it, can be scrolled using",
   " the PGUP, PGDN or the <Ctrl>+arrow keys",
   "",
   " For easier scrolling you can also change the focus to that window using",
   " the <Tab> or <Shift>+<Tab> key, notice the DOUBLE window-lines after this",
   "",
   " Another <Tab>, <Shift>+<Tab> or <Esc> sets focus back to the entryfield",
   "",
   " As a convenience, the scroll-buffer will be scrolled to the end just",
   " before executing any command, allowing you to see generated output,",
   " even if you had scrolled up a few pages ...",
   " For special cases this can be avoided by using <Ctrl>+<Enter>",
   "",
   " You can tell the entryfield has focus because the border-markers '[ ]'",
   " will change color, and the cursor is blinking inside the entryfield.",
   " You will notice that the text output window above it has a SINGLE line",
   " border drawn around it to indicate it does NOT have focus.",
   "",
   " The following keys can be used to edit the entryfield:",
   "",
   "    Insert              Toggle between Insert and replace mode",
   "    Delete              Delete the character at the cursor position",
   "    Backspace           Delete the character before the cursor position",
   "    Home                Move cursor to the start of the entryfield",
   "    End                 Move cursor to the end of the entryfield",
   "    Escape              Clear entry-field completely, making it empty",
   "    Ctrl + Backspace    Clear entry-field completely (like Escape)",
   "    Ctrl + B            Clear entry-field from cursor to begin-of-field",
   "    Ctrl + E            Clear entry-field from cursor to end-of-field",
   "    Ctrl + Right Arrow  Move one word to the right in the field",
   "    Ctrl + Left  Arrow  Move one word to the left  in the field",
   "    Up                  Recall previous (older)  command from history",
   "    Down                Recall next     (newer)  command from history",
   "    Alt  + \\            Show history contents, for recall/completion",
   "    Alt  + [            Find previous match in history (completion)",
   "    Alt  + ]            Find next     match in history (completion)",
   "",
   "    Other keys are either inserted/replaced in the entryfield content,",
   "    like letters, digits and interpunction, or ignored",
   "",
   "      completion: All characters in the entryfield upto the cursor are",
   "                  used in the partial-match. Use Alt + up/down to walk",
   "                  through commands starting with the same characters",
   "",
   "",
   "",
   " T H E   O U T P U T   T E X T   W I N D O W",
   " ===========================================",
   "",
   " This is the output window for all commands executed by TXTst.",
   " It can contain many lines of output, of which normally only the",
   " last screenfull is displayed.",
   "",
   " The screen can be scrolled up and down to view older output.",
   "",
   " For new output to cause automatic scrolling, the display must be at",
   " the LAST line. You can use the <Ctrl>+<End> key to get there.",
   " As a convenience, the scroll-buffer will automatically be scrolled to",
   " the end just before executing any command using the <Enter> key",
   "",
   "",
   " The following keys can be used:",
   "",
   "    Ctrl + PgUp/PgDn Scroll output window up/down by one page",
   "    Ctrl + Home      Scroll up to first non-empty line in the buffer",
   "    Ctrl + End       Scroll down to last, most current, output-line",
   "    Ctrl + Arrows    Scroll one line/col in the direction of arrow",
   "",
   "    These work when the window has focus (DOUBLE line border) as well",
   "    as when the focus is on the entryfield (SINGLE line border) or",
   "    even when a (help) dialog is over the text output window.",
   "",
   "    PgUp and PgDn    Scoll output window when entryfield has focus",
   "",
   "    When the output window has focus, some more keys are available:",
   "",
   "    Left / Right  Scroll output window left/right to view long lines",
   "    Up   / Down   Scroll output window up  / down line by line",
   "    Home / End    Scroll output window to left/right margin",
   "    Ctrl + -      Scroll up to first line of the buffer (usualy empty)",
   "    Alt  + Arrows Move and/or resize the text output window",
   "    F12           Collapse scroll-window to just the title-bar",
   "    Alt  + F12    Cycle SCROLLBUFFER colors: NORMAL - BRIGHT - INVERT",
   "",
   "",
   NULL
};


static  char       *confirmhelp[] =
{
   " No help available. (memory restriction)",
   " This section contains help for all TXTst confirmation requests",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#001 Program bug, from menu selection",
   "",
   " This is to inform you that the menu item you have choosen is",
   " not fully implemented yet. This is most likely a program bug,",
   " but it could be a 'work in progress' if the TXTest version you",
   " are using is a BETA or PREVIEW one ...",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#003 About the TXTst program",
   "",
   " This dialog informs you of the version of the TXTst program, and",
   " some of the other components and tools used in building it.",
   "",
   " All rights reserved by Fsys Software for TXTst and TXlib, and by",
   " the owners of the mentioned tools and components for the rest.",
   "",
   " Usage of 3rd party components and tools permitted by the licences",
   " as found with the distribution or on the related web-site.",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#004 TXTst version and operating-system mismatch",
   "",
   " This is to inform you that the TXTst version that is now active",
   " is not optimal for the operating system detected, or that the",
   " operating environment is not configured optimally.",
   "",
   " Use the recommended TXTest version or make changes to your",
   " operating system configuration as instructed for the best",
   " results and maximum reliability.",
   "",
   " If you want to take the risk and run the program anyway, select 'Yes'",
   " in the dialog, otherwise select 'No' or use the <Esc> key to quit ...",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   NULL
};


static  char       *menusyshelp[] =
{
   "#010 Txt main menu",
   "",
   " The main menu for the application consists of a menu-bar with",
   " several menu pull-downs.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#030 Default menu pull-down",
   "",
   " The last pull-down used, will be the default selected one on",
   " subsequent activation of the menu",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#100 File menu pulldown",
   "",
   " This menu contains all File related menu selections",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#110 Open logfile",
   "",
   " This will prompt for a filename to be used for the logfile",
   " and open that file. All information that goes to the screen",
   " will be appended to this file as well.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   NULL
};


static  char       *dialoghelp[] =
{
   "",
   " Built in help text for test dialog and its controls",
   "",
   "#001 Little text viewer",
   "",
   " This is the textviewer control that displays the command-help text",
   " in a small window. The text can be scrolled in any direction.",
   "",
   " Using the <Alt> + arrow keys will move the whole dialog window.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#011 entryfield nr 1",
   "",
   " This is the first entryfield in the dialog",
   " Any text or number can be typed in here, on <Enter> the",
   " contents of the field can be processed (currently disabled)",
   " and it will be added to the entry-fields history for later",
   " retrieval with the UP and DOWN keys.",
   "",
   " Using the <Alt> + arrow keys will move the whole dialog window.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#012 entryfield nr 2",
   "",
   " This is the second entryfield in the dialog",
   " Any text or number can be typed in here, on <Enter> the",
   " contents of the field can be processed (currently disabled)",
   " and it will be added to the entry-fields history for later",
   " retrieval with the UP and DOWN keys.",
   "",
   "",
   " Using the <Alt> + arrow keys will move the whole dialog window.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#031 Dialog OK button",
   "",
   " This button ends the dialog, signalling an OK condition to the",
   " calling application, so the dialog results can be processed",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#032 Dialog HELP button",
   "",
   " This is the HELP button for the dialog.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#033 Dialog CANCEL button",
   "",
   " This button ends the dialog, signalling a CANCEL condition to the",
   " calling application, so the dialog results must be discarded",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#051 Dialog specific push-button",
   "",
   " This is the test pushbutton in the dialog",
   " Currently is also ends the dialog, using a specific result",
   " code so the application can take the needed action.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#061 Radio button nr 1",
   "",
   " This is the first Radio button in a set of three.",
   " It can be toggled using the <Space> bar on the keyboard",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#062 Radio button nr 2",
   "",
   " This is the second Radio button in a set of three.",
   " It can be toggled using the <Space> bar on the keyboard",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#063 Radio button nr 3",
   "",
   " This is the third Radio button in a set of three.",
   " It can be toggled using the <Space> bar on the keyboard",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#071 Checkbox nr 1",
   "",
   " This is the first Checkbox in a set of three.",
   " It can be toggled using the <Space> bar on the keyboard",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#072 Checkbox nr 2",
   "",
   " This is the second Checkbox in a set of three.",
   " It can be toggled using the <Space> bar on the keyboard",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#073 Checkbox nr 3",
   "",
   " This is the third Checkbox in a set of three.",
   " It can be toggled using the <Space> bar on the keyboard",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#101 Listbox nr 1",
   "",
   " This is the first ListBox, plain value selection.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#102 Listbox nr 2",
   "",
   " This is the second ListBox, value drop-down.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#103 Listbox nr 3",
   "",
   " This is the third ListBox, menu style single line.",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   NULL
};


static  TXTM bordertxt;                         // versioned window title

static  char stattxt[] =
   "F1=help F3=quit F4=save    F10=menu   F12=collapse  Esc=abort";


// definitions for the test-dialog

#define TXT_WID_DIALOG1   300                   // window id dialog1
#define TXT_WID_D1TVIEW1  301                   // dialog1 text view 1
#define TXT_WID_D1ENTRY1  311                   // dialog1 entry 1
#define TXT_WID_D1ENTRYH  317                   // dialog1 entry Head
#define TXT_WID_D1ENTRYS  318                   // dialog1 entry Sector
#define TXT_WID_D1ENTRY2  312                   // dialog1 entry 1
#define TXT_WID_D1BUTTN1  331                   // dialog1 button 1
#define TXT_WID_D1BUTTN2  332                   // dialog1 button 2
#define TXT_WID_D1BUTTN3  333                   // dialog1 button 3
#define TXT_WID_D1STATIC  341                   // dialog1 static text
#define TXT_WID_D1PUSHB1  351                   // dialog1 push button  1
#define TXT_WID_D1RADIO1  361                   // dialog1 radio button 1
#define TXT_WID_D1RADIO2  362                   // dialog1 radio button 2
#define TXT_WID_D1RADIO3  363                   // dialog1 radio button 3
#define TXT_WID_D1CHECK1  371                   // dialog1 check button 1
#define TXT_WID_D1CHECK2  372                   // dialog1 check button 2
#define TXT_WID_D1CHECK3  373                   // dialog1 check button 3

#define TXT_WID_DIALOG2   400                   // window id dialog2
#define TXT_WID_D2LIST1   401                   // dialog2 text list 1
#define TXT_WID_D2LIST2   402                   // dialog2 text list 2
#define TXT_WID_D2LIST3   403                   // dialog2 text list 3
#define TXT_WID_D2LIST4   404                   // dialog2 text list 4
#define TXT_WID_D2LIST5   405                   // dialog2 text list 5
#define TXT_WID_D2LIST6   406                   // dialog2 text list 6
#define TXT_WID_D2LIST7   407                   // dialog2 text list 7


// to be refined, these variables should be local to the dialog
// when reentrancy is an issue (otherwise stop F5=dialog from dialog)
static TXWINDOW    d1frame1win;
static TXWINDOW    d1tview1win;
static TXWINDOW    d1entry1win;
static TXWINDOW    d1entryHwin;
static TXWINDOW    d1entrySwin;
static TXWINDOW    d1entry2win;
static TXWINDOW    d1buttn1win;
static TXWINDOW    d1buttn2win;
static TXWINDOW    d1buttn3win;
static TXWINDOW    d1staticwin;
static TXWINDOW    d1pushb1win;
static TXWINDOW    d1radio1win;
static TXWINDOW    d1radio2win;
static TXWINDOW    d1radio3win;
static TXWINDOW    d1check1win;
static TXWINDOW    d1check2win;
static TXWINDOW    d1check3win;

static TXWHANDLE   d1frame1;
static TXWHANDLE   d1tview1;
static TXWHANDLE   d1entry1;
static TXWHANDLE   d1entryH;
static TXWHANDLE   d1entryS;
static TXWHANDLE   d1entry2;
static TXWHANDLE   d1buttn1;
static TXWHANDLE   d1buttn2;
static TXWHANDLE   d1buttn3;
static TXWHANDLE   d1static;
static TXWHANDLE   d1pushb1;
static TXWHANDLE   d1radio1;
static TXWHANDLE   d1radio2;
static TXWHANDLE   d1radio3;
static TXWHANDLE   d1check1;
static TXWHANDLE   d1check2;
static TXWHANDLE   d1check3;


static TXLN        d1entry1txt = "";
static TXLN        d1entryHtxt = "255";
static TXLN        d1entryStxt = "63";
static TXLN        d1entry2txt = "";

static TXHIST      d1entry1his;
static TXHIST      d1entry2his;

static  char      *d1frame1txt[] =
{
   "",
   "    ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿",
   "    ³                                                     ³ ³               ³",
   "    ³                                                     ³ ³               ³",
   "    ³                                                     ³ ³               ³",
   "    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ³               ³",
   "                                                            ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ",
   NULL
};

static  char       d1buttn1txt[] = "OK";
static  char       d1buttn2txt[] = "Help";
static  char       d1buttn3txt[] = "Cancel";

static  char      *d1statictxt[] =
{
   "Button styles",
   "® test area ¯",
   NULL
};

static  char       d1pushb1txt[] = "Push me";
static  char       d1radio1txt[] = "Radio 1";
static  char       d1radio2txt[] = "Radio 2";
static  char       d1radio3txt[] = "Radio 3";
static  char       d1check1txt[] = "Check 1";
static  char       d1check2txt[] = "Check 2";
static  char       d1check3txt[] = "Check 3";

static  BOOL       d1radio1value = FALSE;
static  BOOL       d1radio2value = TRUE;
static  BOOL       d1radio3value = FALSE;
static  BOOL       d1check1value = FALSE;
static  BOOL       d1check2value = TRUE;
static  BOOL       d1check3value = FALSE;

static  char       txtMsgTitle[] = " Test Message box ";
static  char       txtMsgText[]  =
  "This is message text to test the txwMessageBox. It has some long lines that "
  "need to be split by the Box and also some that are short enough already.\n"
  "So let us see if that works, with an empty line:\n\nAnd a last one ...";

static  char       txtPromptTitle[] = " Test Prompt box ";
static  char       txtPromptText[]  =
  "This is message text to test the txwPromptBox. Fill in any value ...";

//- Definition of a static menu selection list for ListBox test-dialog
TXSitem(tst1,2001,0,TXSF_MARKED    ,1,"First line in list"      ,"Descriptiption for line 1");
TXSitem(tst2,2002,0,TXSF_MARKED    ,1,"2nd   line in list"      ,"Must be the second line");
TXSitem(tst3,2003,0,0              ,1,"another line, and longer","says it all ...");
TXSitem(tst4,2004,0,0              ,8,"shorty 4"                ,"four");
TXSitem(tst5,2005,0,TXSF_DISABLED  ,8,"shorty 5"                ,"five");
TXSitem(tst6,2006,0,0              ,8,"shorty 6"                ,"six");
TXSitem(tst7,2007,0,TXSF_MARK_STD  ,1,"Last  line in list"      ,"You are at the last line now");
static TXS_ITEM *tst[] = {&tst1, &tst2, &tst3, &tst4, &tst5, &tst6, &tst7};

TXSlist(tstlist1,7,7,tst);
TXSlist(tstlist2,7,3,tst);
TXSlist(tstlist3,7,4,tst);


//- Definition of static main-menu selection lists

TXSitem(mmsp,0            ,0,TXSF_DISABLED | TXSF_SEPARATOR,     0,""    ,"");

TXSitem(mm11,TXTC_OPEN    ,0,0             , 1,"Open logfile"              ,"Open a logfile for screen output");
TXSitem(mm12,TXTC_SAVE    ,0,0             , 1,"Save screen   F4"          ,"Save screenbuffer to a file");
TXSitem(mm13,TXTC_RUNS    ,0,0             , 1,"Run script"                ,"Run a TXTest script");
TXSitem(mm14,TXTC_EXIT    ,0,0             , 2,"Exit      Alt-F4"          ,"Exit the TXTest program");
static TXS_ITEM *mm1[] = {&mm11, &mm12, &mm13, &mmsp, &mm14};
TXSlist(tstmm1,5,5,mm1);

TXSitem(mm31,TXTC_DISPLAY ,0,0             , 1,"Display size"              ,"Show size of display screen");
TXSitem(mm32,TXTC_VOLUMES ,0,0             , 1,"Volume list"               ,"Show list of disk volumes");
TXSitem(mm33,TXTC_DIRALL  ,0,0             , 1,"Current directory"         ,"Show files and sub-directories in current directory");
TXSitem(mm34,TXTC_DIRFILE ,0,0             , 1,"Files in current DIR"      ,"Show files-only in current directory");
TXSitem(mm35,TXTC_DIRSUBS ,0,0             , 1,"Subdirs in current DIR"    ,"Show sub-directories only in current directory");
TXSitem(mm36,TXTC_DIRTREE ,0,0             ,11,"Show all, recursive"       ,"Show all, including subdirectory contents (tree)");
TXSitem(mm3a,TXTC_CHRSET  ,0,0             , 1,"Character set display"     ,"Show all 256 characters in defaultc character");
TXSitem(mm3b,TXTC_BOXES   ,0,0             , 1,"Box line drawings"         ,"Draw several boxes using all CP-437 drawing characters");
TXSitem(mm3c,TXTC_COLORR  ,0,0             , 1,"RAW colors display"        ,"Show 256 color combinations FG and BG, direct to screen");
TXSitem(mm3d,TXTC_COLORS  ,0,0             , 0,"Display colors, buffer"    ,"Show 256 color combinations FG and BG, in scroll-buffer");
static TXS_ITEM *mm3[] = {&mm31, &mm32, &mmsp,
                          &mm33, &mm34, &mm35, &mm36, &mmsp,
                          &mm3a, &mm3b, &mm3c, &mm3d};
TXSlist(tstmm3,12,12,mm3);

TXSsubm(s26s,TXTC_SCHEME  ,0,TXTB_SCHEME,0 , 1,"Select window color scheme ¯" ,"Select and activate any of the available window-color schemes              ");
TXSitem(mm41,TXTC_INVSCR  ,0,0             , 1,"Inverted output-screen"       ,"Use inverted colors (black-on-white text) on the scrollable output screen  ");
TXSitem(mm42,TXTC_BRTSCR  ,0,0             , 8,"Bright foreground-text"       ,"Use bright foreground colors only on the scrollable output screen          ");
TXSitem(mm43,TXTC_B2BSCR  ,0,0             , 1,"Blue/Brown background"        ,"Use blue (or brown, inverted) background instead of classic black or white ");
TXSitem(mm44,TXTC_ASCII7  ,0,0             , 5,"Use 7-bit ASCII only"         ,"Use 7-bit ASCII character only, avoid non-standard 'drawing chars'         ");
TXSitem(mm45,TXTC_COLTXT  ,0,0             , 1,"ANSI colored texts"           ,"Use ANSI-like colored text string on the scrollable output screen          ");
TXSitem(mm46,TXTC_AUTOMB  ,0,0             ,11,"Automatic menu activation"    ,"Automatically activate MenuBar after each menu-selection (F10 to exit menu)");
TXSitem(mm47,TXTC_AUTODR  ,0,0             ,16,"Automatic menu Dropdown"      ,"Automatically open menu pulldown on selecting menu-heading in the MenuBar  ");
static TXS_ITEM *mm4[] = {&s26s, &mmsp,
                          &mm41, &mm42, &mm43, &mmsp,
                          &mm44, &mm45, &mmsp,
                          &mm46, &mm47};
TXSlist(tstmm4,11,11,mm4);

TXSitem(mm21,TXTC_DIALOG  ,0,0             , 6,"Test dialog 1  ...  F5"    ,"Test dialog with fields and button                      ");
TXSitem(mm22,TXTC_MSGBOX  ,0,0             , 1,"Message Box    ...  F6"    ,"Test a simple message popup                             ");
TXSitem(mm23,TXTC_PROMPT  ,0,0             , 1,"Prompt box     ...  F7"    ,"Test prompting for a string value                       ");
TXSitem(mm24,TXTC_HEXED   ,0,0             , 1,"HexEdit dialog ...  F2"    ,"Test Hex editor dialog with fixed dummy data            ");
TXSitem(mm25,TXTC_LIST    ,0,0             , 1,"ListBox Dialog ...  F8"    ,"Test several ListBoxes in a dialog                      ");
TXSitem(mm26,TXTC_WIDGET  ,0,0             , 1,"Widget Dialog  ...  F9"    ,"Test dialog driven by generic widget data and functions.");
TXSitem(mm27,TXTC_PRWIDG  ,0,0             , 1,"Widget Prompt  ... "       ,"Prompt for a string and provide additional widgets ...  ");
TXSitem(mm28,TXTC_DISABL  ,0,TXSF_DISABLED , 1,"Disabled menu-entry"       ,"Entry is disabled, should NOT act on Enter ...          ");
TXSsubm(mm29,TXTC_SUBMEN  ,&tstmm4,0,0     , 1,"Settings submenu     ¯"    ,"Call up the settings-menu as a submenu popup            ");
static TXS_ITEM *mm2[] = {&mm21, &mm22, &mm23, &mm24, &mmsp,
                          &mm25, &mm26, &mm27, &mm28, &mm29};
TXSlist(tstmm2, 10, 10,mm2);


TXSitem(mm91,TXTC_CMDHELP ,0,0             , 1,"Application commands"      ,"One line descriptions for each TXT commands");
TXSitem(mm92,TXTC_SW_HELP ,0,TXSF_MARK_STAR, 1,"Exe startup switches"      ,"One line descriptions for each executable switch");
TXSitem(mm93,TXTC_UIHELP  ,0,0             , 1,"User interface"            ,"Help on the application and TxWin user interface");
TXSitem(mm94,TXTC_ABOUT   ,0,0             , 1,"About ...         F11"     ,"Show version details and copyright");
TXSsubm(mm95,TXTC_SUBMEN  ,&tstmm2,0,0     , 1,"Test submenu         ¯"    ,"Call up the test-menu as a submenu popup");
static TXS_ITEM *mm9[] = {&mm91, &mm92, &mm93, &mm94, &mmsp, &mm95};
TXSlist(tstmm9,6,6,mm9);



TXSmenu(tmenu1,&tstmm1,TXTM_FILE   ,0    , 1,'f'," File "                ,"Menu with file items");
TXSmenu(tmenu2,&tstmm2,TXTM_TEST   ,0    , 1,'t'," Test "                ,"Menu with test items");
TXSmenu(tmenu3,&tstmm3,TXTM_SHOW   ,0    , 1,'d'," Display "             ,"Menu with display items");
TXSmenu(tmenu4,&tstmm4,TXTM_SETT   ,0    , 1,'s'," Settings "            ,"Modify some TXT program settings and properties");
TXSmenu(tmenu9,&tstmm9,TXTM_HELP   ,0    , 1,'h'," Help "                ,"Menu with help items");
static TXS_MENUBAR mainmenu =
{
   5,                                           // number of menus presnt
   0,                                           // index of 1st default menu
   {&tmenu1, &tmenu2, &tmenu3, &tmenu4, &tmenu9} // menu pointers
};


// Definitions for the generic widget test dialog

BOOL      group1_cb1 = FALSE;                   // variable checkbutton 1
BOOL      group1_cb2 = TRUE;                    // variable checkbutton 2
BOOL      group1_rb1 = FALSE;                   // variable radiobutton 2
BOOL      group1_rb2 = FALSE;                   // variable radiobutton 2
BOOL      group1_rb3 = TRUE;                    // variable radiobutton 3
TXTM      group1_ef1 = "Widget field";          // variable entryfield  1

TXWIDGET  group1_widgets[9] =
{
   {
      0,  0,  1, 20, 0, 1, 0,                   // rel position, size & group
      TXWS_AUTORAD, 0, "",                      // style, helpid and title
      TXW_BUTTON, 0, NULL,                      // class, hwnd and winproc
      TXWgButton( &group1_rb1, "radiobutton 1")
   },
   {
      1,  0,  1, 20, 0, 1, 0,                   // rel position, size & group
      TXWS_AUTORAD, 0, "",                      // style, helpid and title
      TXW_BUTTON, 0, NULL,                      // class, hwnd and winproc
      TXWgButton( &group1_rb2, "radiobutton 2")
   },
   {
      2,  0,  1, 20, 0, 1, 0,                   // rel position, size & group
      TXWS_AUTORAD, 0, "",                      // style, helpid and title
      TXW_BUTTON, 0, NULL,                      // class, hwnd and winproc
      TXWgButton( &group1_rb3, "radiobutton 3")
   },
   {
      0, 25,  1, 20, 0, 0, 0,                   // rel position, size & group
      TXWS_AUTOCHK | TXWS_HCHILD2MOVE,
      0, "",                                    // style, helpid and title
      TXW_BUTTON, 0, NULL,                      // class, hwnd and winproc
      TXWgButton( &group1_cb1, "checkbutton 1")
   },
   {
      1, 25,  1, 20, 0, 0, 0,                   // rel position, size & group
      TXWS_AUTOCHK | TXWS_HCHILD2MOVE,
      0, "",                                    // style, helpid and title
      TXW_BUTTON, 0, NULL,                      // class, hwnd and winproc
      TXWgButton( &group1_cb2, "checkbutton 2")
   },
   {
      2, 23,  1, 24, 0, 0, 0,                   // rel position, size & group
      TXWS_OUTPUT | TXWS_HCHILD2MOVE,
      0, "",                                    // style, helpid and title
      TXW_STLINE, 0, NULL,                      // class, hwnd and winproc
      TXWgStline( "ÆÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍµ")
   },
   {
      4,  0,  5, 49, 0, 0, 0,                   // rel position, size & group
      TXWS_FRAMED | TXWS_HCHILD_SIZE | TXWS_VCHILD_SIZE,
      0, "Some text to be shown",               // style, helpid and title
      TXW_TEXTVIEW, 0, NULL,                    // class, hwnd and winproc
      TXWgTextview( 0,0,TXW_INVALID,TXW_INVALID, mainwinhelp)
   },
   {
      10, 0,  2, 22, 0, 0, 0,                   // rel position, size & group
      TXWS_D_SPIN | TXLS_DROP_ENTER | TXWS_LEFTJUSTIFY |
      TXWS_HCHILD2SIZE | TXWS_VCHILD_MOVE,
      0, "Select anything",                     // style, helpid and title
      TXW_LISTBOX, 0, NULL,                     // class, hwnd and winproc
      TXWgListbox( 0,0,0,0,0,0,0,0,0, &tstlist1)
   },
   {
      10,24,  2, 25, 0, 0, 0,                   // rel position, size & group
      TXWS_ENTRYT | TXWS_HCHILD2MOVE |
      TXWS_HCHILD2SIZE | TXWS_VCHILD_MOVE,
      0, "Whatever you like",                   // style, helpid and title
      TXW_ENTRYFIELD, 0, NULL,                  // class, hwnd and winproc
      TXWgEntryfield( 0,
                      0,
                      0,
                      TXMAXTM,
                      0,
                      0,
                      NULL,
                      group1_ef1)
   }
};

TXGW_DATA widget_group1 =
{
   9,                                           // number of widgets
   0,                                           // help, widget overrules
   800,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   group1_widgets                               // array of widgets
};

//- to be refined, need additional field in WIDGET for GROUP!

BOOL g2b1 = FALSE;                   // variable radiobutton 1
BOOL g2b2 = FALSE;                   // variable radiobutton 2
BOOL g2b3 = TRUE;                    // variable radiobutton 3
BOOL g2b4 = FALSE;                   // variable radiobutton 4
BOOL g2b5 = TRUE;                    // variable radiobutton 5
BOOL g2b6 = FALSE;                   // variable radiobutton 6

TXWIDGET  group2_widgets[6] =
{
   {0,  0, 1, 20, 0, 1, 0, TXWS_AUTORAD, 0, "", TXW_BUTTON, 0, NULL, TXWgButton( &g2b1, "radio 11")},
   {1,  0, 1, 20, 0, 1, 0, TXWS_AUTORAD, 0, "", TXW_BUTTON, 0, NULL, TXWgButton( &g2b2, "radio 12")},
   {2,  0, 1, 20, 0, 1, 0, TXWS_AUTORAD, 0, "", TXW_BUTTON, 0, NULL, TXWgButton( &g2b3, "radio 13")},
   {3,  0, 1, 20, 0, 1, 0, TXWS_AUTORAD, 0, "", TXW_BUTTON, 0, NULL, TXWgButton( &g2b4, "radio 14")},
   {1, 25, 1, 20, 0, 2, 0, TXWS_AUTORAD, 0, "", TXW_BUTTON, 0, NULL, TXWgButton( &g2b5, "radio 21")},
   {2, 25, 1, 20, 0, 2, 0, TXWS_AUTORAD, 0, "", TXW_BUTTON, 0, NULL, TXWgButton( &g2b6, "radio 21")},
};

TXGW_DATA widget_group2 =
{
   6,                                           // number of widgets
   0,                                           // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   group2_widgets                               // array of widgets
};


// Test window procedure, for any window-class
static ULONG txtStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// TXTst window procedure, for entry-field, includes automatic value-echo
static ULONG txtEntryWindowProc                 // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Window procedure, adding window-color setting using Ctrl+arrows
static ULONG txwColorViewWinProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Test window procedure, for dialog windows
static ULONG txtDialogWindowProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


/*****************************************************************************/
// Start and maintain TXTst interactive text-based windowed user-interface
/*****************************************************************************/
ULONG txtWindowed
(
   char               *initial                  // IN    initial TXTst cmd
)
{
   ULONG               rc = NO_ERROR;
   ULONG               bm;                      // border mode
   BOOL                vborder;                 // vertical borders
   BOOL                hborder;                 // horizontal borders
   BOOL                borders   = FALSE;       // forced borders, NO
   BOOL                noborders = FALSE;       // forced no-borders, NO

   ENTER();

   //- to be refined, move to txt.c with help text later
   txwRegisterHelpText( TXTH_GENERIC,    "TXT main",      "TXTst main help items",    mainwinhelp);
   txwRegisterHelpText( TXTH_CONFIRM,    "Confirmations", "TXTst confirmation items", confirmhelp);
   txwRegisterHelpText( TXTH_MENUS,      "Menu system",   "TXTst menu system help",   menusyshelp);
   txwRegisterHelpText( TXT_WID_DIALOG1, "Dialog info",    "Txtest dialog help",      dialoghelp);

   if (TxaExeSwitchValue('f') != NULL)          // frame switch specified
   {
      borders   = TxaExeSwitch('f');
      noborders = !borders;
   }
   TRACES(("borders:%lu  noborders:%lu\n", borders, noborders));

   vborder = (borders || ((TxScreenCols() > 84) && !noborders));
   hborder = (borders || ((TxScreenRows() > 28) && !noborders));

   sprintf( bordertxt, "%s: %s %s", TXT_N, TXT_V, TXT_C);
   bm = TXWS_STDWINDOW | TXWS_FOOTRBORDER | TXWS_MOVEABLE;
   if (vborder)
   {
      bm |= TXWS_SIDEBORDERS;                   // add side borders
   }
   if (hborder)
   {
      bm |= TXWS_TITLEBORDER;                   // add title border
   }
   txwSetupWindowData(
      0, 0 ,                                    // upper left corner
      TxScreenRows(), TxScreenCols(),           // vert/hor size
      bm, TXT_H_APPLIC,                         // style / helpid
      ' ', ' ',
      cSchemeColor,    cSchemeColor,
      cDskTitleStand,  cDskTitleFocus,
      cDskFooterStand, cDskFooterFocus,
      bordertxt, "",
      &desktopwin);

   if ((desktop = txwInitializeDesktop( &desktopwin, txtStdWindowProc )) != 0)
   {
      TXRECT           dtsize;                  // desktop client size

      txwQueryWindowRect( desktop, FALSE, &dtsize); // get client area
      txwEnableWindow(    desktop, FALSE);      // no focus on desktop

      scrollBufData.length = TXT_SCROLL_L;
      scrollBufData.width  = max( TXT_SCROLL_W, TxScreenCols());
      scrollBufData.vsize  = dtsize.bottom -2;  // entryfield and sbstatus
      if (hborder)
      {
         scrollBufData.vsize--;                 // subtract SB title line
      }
      rc = txwInitPrintfSBHook(&scrollBufData);
      if (rc == TX_ALLOC_ERROR)                 // try smaller scroll-buffer
      {
         scrollBufData.length = TXT_SMALLB_L;
         scrollBufData.width  = TXT_SMALLB_W;
         rc = txwInitPrintfSBHook( &scrollBufData);
      }
      if (rc == NO_ERROR)
      {
         TXRECT   sbsize;                       // scrollbuf client size

         txta->sbsize   = scrollBufData.length;
         txta->sblwidth = scrollBufData.width;
         txta->sbbuf    = scrollBufData.buf;

         bm = TXWS_CHILDWINDOW;
         if (vborder)
         {
            bm |= TXWS_SIDEBORDERS;             // add side borders
         }
         if (hborder)
         {
            bm |= TXWS_TITLEBORDER;             // add title border only
         }                                      // (status replaces footer)
         bm |= TXWS_MOVEABLE;                   // scrollbuf movable/sizeable
         bm |= TXWS_SAVEBITS;                   // including <F12> support

         txwSetupWindowData(
            0, 0 ,                              // upper left corner
            dtsize.bottom -1,                   // vertical size
            dtsize.right,                       // horizontal size
            bm, TXT_H_INTERF,                   // style & help
            ' ', ' ', TXWSCHEME_COLORS,
            " text output window ", "",
            &scrbuffwin);
         scrbuffwin.sb.topline = scrollBufData.firstline;
         scrbuffwin.sb.leftcol = 0;
         scrbuffwin.sb.sbdata  = &scrollBufData;
         scrbuffwin.sb.scolor  = cSchemeColor;
         scrbuffwin.sb.altcol  = TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT;
         sbufwin = txwCreateWindow( desktop, TXW_SBVIEW, 0, 0,
                                    &scrbuffwin, txtStdWindowProc);
         scrollBufData.view = sbufwin;          // register view for update
         txwQueryWindowRect( sbufwin, FALSE, &sbsize);
         txta->sbWidth  = sbsize.right +1;
         txta->sbLength = sbsize.bottom;
         TRACES(("sbW: %hu  sbL: %hu\n", txta->sbWidth, txta->sbLength));
         txwSetWindowUShort(  sbufwin, TXQWS_ID, TXT_WID_SCROLL);
         txwInvalidateWindow( sbufwin, TRUE, TRUE);

         txta->sbwindow = sbufwin;              // make handle available

         //- commandline entryfield
         bm = TXWS_CHILDWINDOW | TXWS_SIDEBORDERS | TXES_MAIN_CMDLINE;
         txwSetupWindowData(
            dtsize.bottom -1, 0,                // upper left corner
            1, dtsize.right,                    // vert + hor size
            bm, TXT_H_INTERF,                   // style & helpid
            ' ', ' ',
            cSchemeColor, cEntrBorder_top,
            cSchemeColor, cSchemeColor,
            cSchemeColor, cSchemeColor,
            "", "",
            &entryfwin);
         entryfwin.ef.leftcol = 0;
         entryfwin.ef.maxcol  = TXW_INVALID;
         entryfwin.ef.rsize   = TXMAXLN;
         entryfwin.ef.buf     = entryftxt;
         entryfwin.ef.history = &cmd_history;

         entrwin = txwCreateWindow( desktop, TXW_ENTRYFIELD, 0, 0,
                                   &entryfwin, txtEntryWindowProc);

         txwInitializeHistory( entryfwin.ef.history, TXT_HIST_SIZE, TXT_HIST_LINE);
         txwSetWindowUShort(   entrwin, TXQWS_ID, TXT_WID_ENTRY);
         txwSetFocus(          entrwin);
         txwInvalidateWindow(  entrwin, TRUE, TRUE);

         if (!TxaExeSwitchUnSet(TXA_O_MENU))    // automatic menu activation
         {
            char     *open   = TxaExeSwitchStr( TXA_O_MENU, NULL, "");

            txta->automenu   = TRUE;
            txta->menuOwner  = entrwin;
            if (*open != 0)
            {
               txta->menuopen   = (ULONG) tolower(open[0]);
            }
            else
            {
               txta->menuopen   = (ULONG) 't';  // start with Test pulldown
            }
         }
         txta->autodrop = ((TxaExeSwitchNum( 'M', NULL, 0) & 2) == 0);

         if (TxaExeSwitch('S'))                 // Shell mode
         {
            txwSetAccelerator( entrwin, TXa_F4,  TXTC_EXIT);
         }
         txwSetAccelerator( entrwin, TXk_F3,     TXTC_EXIT);
         txwSetAccelerator( entrwin, TXk_F4,     TXTC_SAVE);
         txwSetAccelerator( entrwin, TXk_F5,     TXTC_DIALOG);
         txwSetAccelerator( entrwin, TXk_F6,     TXTC_MSGBOX);
         txwSetAccelerator( entrwin, TXk_F7,     TXTC_PROMPT);
         txwSetAccelerator( entrwin, TXk_F2,     TXTC_HEXED);
         txwSetAccelerator( entrwin, TXk_F8,     TXTC_LIST);
         txwSetAccelerator( entrwin, TXk_F9,     TXTC_WIDGET);
         txwSetAccelerator( entrwin, TXk_F11,    TXTC_ABOUT);
         txwSetAccelerator( entrwin, TXk_MENU,   TXTM_DEFAULT);
         txwSetAccelerator( entrwin, TXk_F10,    TXTM_DEFAULT);
         txwSetAccelerator( entrwin, TXs_F10,    TXTM_DEFAULT);
         txwSetAccelerator( entrwin, TXa_F,      TXTM_FILE);
         txwSetAccelerator( entrwin, TXa_T,      TXTM_TEST);
         txwSetAccelerator( entrwin, TXa_S,      TXTM_SHOW);
         txwSetAccelerator( entrwin, TXa_H,      TXTM_HELP);

         if ((txtStartupLogo( initial) != TXT_QUIT) && !txta->autoquit)
         {
            TXWQMSG             qmsg;

            while (txwGetMsg(  &qmsg))
            {
               txwDispatchMsg( &qmsg);
            }
         }
         txwTerminateHistory( &cmd_history);
         txwTermPrintfSBHook();
      }
      else
      {
         txta->retc = rc;
      }
      txwTerminateHelpManager();
      txwTerminateDesktop();
   }
   else
   {
      TxPrint("Failed to initialize desktop\n");
   }
   RETURN (txta->retc);
}                                               // end 'txtWindowed'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// TXTst standard window procedure, for any window-class
/*****************************************************************************/
static ULONG txtStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc = NO_ERROR;
   ULONG               dr = 0;
   TXLN                s1,s2;
   ULONG               flag = 0;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      TRCLAS( "TXT std - ", hwnd);
      switch (msg)
      {
         case TXWM_COMMAND:
            switch (mp1)                        // unique command code
            {
               case TXTM_FILE:
               case TXTM_TEST:
               case TXTM_SHOW:
               case TXTM_HELP:
               case TXTM_DEFAULT:
                  {
                     ULONG menuopen;

                     //- Recreate and attach dynamic submenus

                     txSelDestroy(          &(txta->slSchemes)); // remove scheme list
                     TxSelistColorSchemes(  &(txta->slSchemes)); // and refresh it

                     s26s.userdata = (ULONG) (txta->slSchemes ); // TXTA_SCHEME

                     //- set dynamic marks and disable flags ...

                     txtMiMarked( TXTC_INVSCR, txwSbColorStyle(TXSB_COLOR_INVERT), NULL);
                     txtMiMarked( TXTC_BRTSCR, txwSbColorStyle(TXSB_COLOR_BRIGHT) ^
                                               txwSbColorStyle(TXSB_COLOR_INVERT), NULL);
                     txtMiMarked( TXTC_B2BSCR, txwSbColorStyle(TXSB_COLOR_B2BLUE), NULL);
                     txtMiMarked( TXTC_ASCII7, TxGetAscii7Mode()                 , NULL);
                     txtMiMarked( TXTC_COLTXT, TxGetAnsiMode() == A_ON           , NULL);
                     txtMiMarked( TXTC_AUTOMB, txta->automenu  == TRUE           , NULL);
                     txtMiMarked( TXTC_AUTODR, txta->autodrop  == TRUE           , NULL);

                     switch (mp1)
                     {
                        case TXTM_FILE: menuopen = 'f';  break;
                        case TXTM_TEST: menuopen = 't';  break;
                        case TXTM_SHOW: menuopen = 'd';  break;
                        case TXTM_SETT: menuopen = 's';  break;
                        case TXTM_HELP: menuopen = 'h';  break;
                        default:        menuopen = txta->menuopen;
                           txta->menuopen = 0;  // one time default !
                           break;
                     }
                     txta->automenu = TRUE;     // make menu sticky
                     txta->menuOwner = hwnd;

                     flag = TXMN_MAIN_MENU;     // signal main-menu will be up
                     if (mp1 == TXTM_AUTOMENU)  // automatic menu after command
                     {                          // completion, do not drop yet!
                        flag |= TXMN_DELAY_AUTODROP;
                        TRACES(("MenuBar - delayed autodrop\n"));
                     }
                     if (txta->autodrop == FALSE)
                     {
                        TRACES(("MenuBar - no autodrop\n"));
                        flag |= TXMN_NO_AUTODROP;
                     }

                     txwSendMsg( txta->sbwindow, TXWM_STATUS, 0, cSchemeColor);

                     if (txwMenuBar( TXHWND_DESKTOP, hwnd, NULL,
                                     menuopen, TXTM_BAR,
                                     flag, &mainmenu) == TXDID_CANCEL)
                     {
                        txta->menuOwner = 0;    // quit automatic menuBar
                     }
                  }
                  break;

               case TXTC_OPEN:
                  txtBEGINWORK();               // signal work starting

                  strcpy( s1, "txtlog");
                  if (txwSaveAsFileDialog( s1, NULL, NULL, 0, NULL,
                      " Specify file for logging (append) ", s1))
                  {
                     TxAppendToLogFile( s1, TRUE);
                  }
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_SAVE:
                  txtExecCmd( "scrfile txtest");
                  break;

               case TXTC_RUNS:
                  txtBEGINWORK();               // signal work starting
                  strcpy( s1, "*.dfs");
                  #if defined (DEV32)
                     strcat( s1, ";*.cmd");     // add REXX scripts for OS/2
                  #endif

                  if (txwOpenFileDialog( s1, NULL, NULL, 0, NULL,
                      " Select a TxTest script file ", s1))
                  {
                     TXLN               descr;

                     sprintf( s2, "run %s ", s1);

                     TxsValidateScript( s1, NULL, s1, NULL); // get description
                     if (strlen( s1) != 0)
                     {
                        sprintf( descr, "Parameters enclosed in [] are "
                                 "optional, others are mandatory.\n%s", s1);
                     }
                     else
                     {
                        strcpy( descr, "Specify any parameters needed by "
                                "the script or just leave blank ...");
                     }
                     strcpy( s1, "");           // default same as current
                     if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL, descr,
                           " Specify parameter(s) for the script ", TXTC_RUNS,
                           TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER,
                           50, s1) != TXDID_CANCEL)
                     {
                        strcat( s2, s1);
                     }
                     txtExecCmd( s2);
                  }
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_EXIT:
                  if (TxaExeSwitch('S'))
                  {
                     TxMessage( TRUE, 0,
                                "TXTst is running in SHELL mode,\n"
                                "quit is not allowed ...");
                  }
                  else
                  {
                     txwPostMsg( hwnd, TXWM_CLOSE, 0, 0);
                  }
                  #if defined (DEV32)
                     txwInvalidateAll();        // avoid VIO64K bug
                  #endif
                  break;

               case TXTC_DIALOG:
                  txtBEGINWORK();               // signal work starting
                  dr = txtTestDialog();
                  TxPrint( "TestDialog result code: %lu\n", dr);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_WIDGET:
                  txtBEGINWORK();               // signal work starting
                  dr = txwWidgetDialog( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL,
                                        "Widget test Dialog",
                                        TXWD_MOVEABLE | TXWD_HCENTER | TXWD_VCENTER,
                                        0, &widget_group1);
                  TxPrint( "WidgetDialog result code: %lu\n", dr);
                  TxPrint( "Check-1: %s\n",  (group1_cb1) ? "TRUE" : "FALSE");
                  TxPrint( "Check-2: %s\n",  (group1_cb2) ? "TRUE" : "FALSE");
                  TxPrint( "Radio-1: %s\n",  (group1_rb1) ? "TRUE" : "FALSE");
                  TxPrint( "Radio-2: %s\n",  (group1_rb2) ? "TRUE" : "FALSE");
                  TxPrint( "Radio-3: %s\n",  (group1_rb3) ? "TRUE" : "FALSE");
                  TxPrint( "Entry-F: '%s'\n", group1_ef1);
                  TxPrint( "List nr: %lu, '%s' = '%s'\n", tstlist1.selected,
                                           tstlist1.items[tstlist1.selected]->text,
                                           tstlist1.items[tstlist1.selected]->desc);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_MSGBOX:
                  txtBEGINWORK();               // signal work starting
                  dr = txwMessageBox( sbufwin, TXHWND_DESKTOP,
                                      txtMsgText, txtMsgTitle, 202,
                                      TXMB_MOVEABLE | TXMB_VCENTER |
                                      TXMB_HCENTER  | TXMB_HELP);
                  TxPrint( "MessageBox result code: %lu\n", dr);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_PROMPT:
                  txtBEGINWORK();               // signal work starting
                  strcpy( s1, "another prompted value");

                  dr = txwPromptBox( sbufwin, TXHWND_DESKTOP, NULL,
                                     txtPromptText, txtPromptTitle, 202,
                                     TXMB_MOVEABLE | TXMB_HELP,
                                     50, s1);
                  TxPrint( "PromptBox result code: %lu\n\nvalue: '%s'\n", dr, s1);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_PRWIDG:                // widget prompt
                  txtBEGINWORK();               // signal work starting
                  strcpy( s1, "another prompted value");

                  dr = txwPromptBox( sbufwin, TXHWND_DESKTOP, &widget_group2,
                                     txtPromptText, txtPromptTitle, 202,
                                     TXMB_MOVEABLE | TXMB_HELP,
                                     50, s1);
                  TxPrint( "PromptBox result code: %lu\n\nvalue: '%s'\n", dr, s1);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_LIST:
                  txtBEGINWORK();               // signal work starting
                  dr = txtListBoxDialog();
                  TxPrint( "ListBoxDialog result: %lu\n", dr);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_CHRSET:
                  txtExecEnd( "charset");
                  break;

               case TXTC_BOXES:
                  txtExecEnd( "cboxes");
                  break;

               case TXTC_HEXED:
                  txtExecEnd( "hexed");
                  break;

               case TXTC_COLORR:                // paint directly to screen
                  {                             // very fast, but will be
                     TXTS    color;             // scrolled or refreshed away
                     int     bg,fg;

                     txtBEGINWORK();            // signal work starting

                     TxPrint( "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                              " Press any key to continue ... \n");
                     for (bg = 0; bg < 16; bg++)
                     {
                        for (fg = 0; fg < 16; fg++)
                        {
                           sprintf( color, " %1X%1X ", bg, fg);
                           txwScrDrawCharStrCol( bg +3, fg * 4 +13, NULL, color,
                                                 (BYTE) ((bg << 4) + fg));
                        }
                     }
                     txwGetInputEvent( NULL);   // wait for a key / mouse
                     txtENDWORK();              // signal work done
                  }
                  break;

               case TXTC_COLORS:
                  txtExecEnd( "colors");
                  break;

               case TXTC_INVSCR:
                  txtExecSilent( "set screen i#set screen b"); // invert & bright
                  txwInvalidateAll();
                  break;

               case TXTC_BRTSCR:
                  txtExecSilent( "set screen b");
                  txwInvalidateAll();
                  break;

               case TXTC_B2BSCR:
                  txtExecSilent( "set screen s");
                  txwInvalidateAll();
                  break;

               case TXTC_ASCII7:
                  txtBEGINWORK();               // signal work starting
                  TxSetAscii7Mode(!TxGetAscii7Mode());
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_COLTXT:
                  txtBEGINWORK();               // signal work starting
                  if (TxGetAnsiMode() == A_ON)
                  {
                     TxSetAnsiMode( A_OFF);
                  }
                  else
                  {
                     TxSetAnsiMode( A_ON);
                  }
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_AUTOMB:
                  txtBEGINWORK();               // signal work starting
                  txta->automenu  = !(txta->automenu);
                  txta->menuOwner =  (txta->automenu) ? entrwin: 0;
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_AUTODR:
                  txtBEGINWORK();               // signal work starting
                  txta->autodrop  = !(txta->autodrop);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_DISABL:
                  TxPrint("Disabled menu entry got processed!\n");
                  break;

               case TXTC_VOLUMES:
                  txtExecEnd( "vol");
                  break;

               case TXTC_DISPLAY:
                  txtExecEnd( "display");
                  break;

               case TXTC_DIRALL:
                  txtExecEnd( "txdir");
                  break;

               case TXTC_DIRFILE:
                  txtExecEnd( "txdir * f");
                  break;

               case TXTC_DIRSUBS:
                  txtExecEnd( "txdir * d");
                  break;

               case TXTC_DIRTREE:
                  txtExecEnd( "txdir * s");
                  break;

               case TXTC_CMDHELP:
                  txtBEGINWORK();               // signal work starting
                  txwViewText( TXHWND_DESKTOP, 0, 0,
                              "TXTst application command summary",
                               cmdhelptxt);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_SW_HELP:
                  txtBEGINWORK();               // signal work starting
                  txwViewText( TXHWND_DESKTOP, 0, 0,
                              "TXTst executable switches summary",
                               switchhelp);
                  txtENDWORK();                 // signal work done
                  break;

               case TXTC_UIHELP:
                  txwPostMsg( entrwin, TXWM_HELP, 0, 0);
                  break;

               case TXTC_ABOUT:
                  txtExecCmd( "about -r");
                  break;

               default:
                  if (txwMiRange( TXTB_SCHEME, TXWCS_LAST_SCHEME +1))
                  {
                     txtBEGINWORK();            // signal work starting
                     txwColorScheme( txwMiValue( TXTB_SCHEME), NULL);
                     txwInvalidateAll();        // redraw everything
                     txtENDWORK();              // signal work done
                  }
                  else
                  {
                     txtBEGINWORK();            // signal work starting
                     TxMessage( TRUE, 5001,
                                "Unknown menu command-code %lu,\nthis is a "
                                "program bug.\n\nPlease report this to your "
                                "support contact for this software", mp1);
                     txtENDWORK();              // signal work done
                  }
                  break;
            }
            break;

         case TXWM_CHAR:
            if (txwIsAccelCandidate(mp2))       // menu will be closed, allow
            {                                   // automenu to restart it.
               txtBEGINWORK();
               txtENDWORK();                    // signal work done
            }                                   // fall through to default!
         default:
            rc = txwDefWindowProc( hwnd, msg, mp1, mp2);
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txtStdWindowProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// TXTst window procedure, for entry-field, includes automatic value-echo
/*****************************************************************************/
static ULONG txtEntryWindowProc                 // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc   = NO_ERROR;
   ULONG               dr;
   ULONG               key;
   TXWINDOW           *win;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      TRCLAS( "TXT entry - ", hwnd);
      win = txwWindowData( hwnd);
      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)
            {
               case TXk_ENTER:                  // execute as TXTst command
                  txwSendMsg( txta->sbwindow, TXWM_CHAR, 0, TXc_END);
               case TXc_ENTER:                  // execute, no auto-scroll
                  txwSendMsg( hwnd,  TXWM_CURSORVISIBLE, FALSE, 0);

                  TxPrint("\n");
                  TxCancelAbort();              // reset pending abort status
                  if (((dr = txtMultiCommand( win->ef.buf, 0, TRUE, TRUE, FALSE))
                           == TXT_QUIT) || ( txta->autoquit))
                  {
                     txwPostMsg( hwnd, TXWM_CLOSE, 0, 0);
                  }
                  else                          // update history etc
                  {
                     rc = txwDefWindowProc( hwnd, msg, mp1, mp2);

                     txwSendMsg( hwnd,  TXWM_CURSORVISIBLE, TRUE, 0);

                     #if defined (DEV32)
                        txwInvalidateAll();     // avoid VIO64K bug
                     #endif
                  }
                  break;

               case TXk_PGUP  :                 // redirect to scroll-buf
               case TXk_PGDN  :
               case TXc_UP    :
               case TXc_DOWN  :
               case TXc_PGUP  :
               case TXc_PGDN  :
               case TXa_PGUP  :
               case TXa_PGDN  :
               case TXc_HOME  :
               case TXc_END   :
               case TXa_COMMA :
               case TXa_DOT   :
                  switch (mp2)                  // translate some Ctrl-xxx
                  {                             // to normal movement keys
                     case TXc_UP    : key = TXk_UP;     break;
                     case TXc_DOWN  : key = TXk_DOWN;   break;
                     default:         key = mp2;        break;
                  }
                  txwSendMsg( txta->sbwindow, msg, mp1, key);
                  break;

               case TXa_LEFT:                   // avoid left/right movemement
               case TXa_RIGHT:                  // of the desktop (corruption)
                  break;

               default:
                  rc = txtStdWindowProc( hwnd, msg, mp1, mp2);
                  break;
            }
            break;

         case TXWM_SETFOCUS:
            if ((BOOL) mp1 == TRUE)             // Entryfield got focus ?
            {
               txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER,
                   (ULONG) stattxt, 0);         // Fkey help
            }
            else
            {
               txwPostMsg( TXHWND_DESKTOP,
                           TXWM_SETFOOTER, 0, 0); // reset Fkey help
            }
            break;

         default:
            rc = txtStdWindowProc( hwnd, msg, mp1, mp2);
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'txtEntryWindowProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Window procedure, adding window-color setting using Ctrl+arrows
/*****************************************************************************/
static ULONG txwColorViewWinProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc  = NO_ERROR;
   static TXTM         color;

   ENTER();
   if (hwnd != 0)
   {
      TXWINDOW        *win = txwWindowData( hwnd);

      TRCMSG( hwnd, msg, mp1, mp2);
      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)
            {
               case TXc_UP:
               case TXc_DOWN:
               case TXc_LEFT:
               case TXc_RIGHT:
                  switch (mp2)
                  {
                     case TXc_UP:    win->clientclear.at +=  1; break;
                     case TXc_DOWN:  win->clientclear.at -=  1; break;
                     case TXc_LEFT:  win->clientclear.at -= 16; break;
                     case TXc_RIGHT: win->clientclear.at += 16; break;
                  }
                  txwInvalidateWindow(  hwnd, TRUE, TRUE);
                  sprintf( color, "Foreground: %hx  on  background: %hx",
                           (USHORT) (win->clientclear.at & 0x0f),
                           (USHORT) (win->clientclear.at & 0xf0));
                  txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) color, 0);
                  break;

               default:
                  rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  break;
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
}                                               // end 'txwColorViewWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Check if dialog is wanted and possible, message otherwise
/*****************************************************************************/
BOOL txtDialogAppropriate
(
   void
)
{
   BOOL                rc = FALSE;              // function return

   ENTER();

   if (txta->dialogs)                           // -P switch or option set
   {
      if (desktop)                              // desktop present ?
      {
         rc = TRUE;                             // allow dialog
      }
   }
   BRETURN (rc);
}                                               // end 'txtDialogAppropriate'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Setup test dialog
/*****************************************************************************/
ULONG txtTestDialog                             // RET   result
(
   void
)
{
   ULONG               rc    = NO_ERROR;

   ENTER();

   if (desktop != 0)
   {
      txwSetupWindowData(
         3,  3,                                 // upper left corner
         21,                                    // vertical size
         80,                                    // horizontal size
         TXWS_DFRAME | TXWS_CAST_SHADOW,        // window frame style
         TXT_WID_DIALOG1,                       // help id
         ' ', ' ', TXWSCHEME_COLORS,
         " Txtest Dialog window nr 1 ",
         " <Enter> = end dialog; <Esc> = abort ",
         &d1frame1win);

      d1frame1win.st.buf     = d1frame1txt;     // dialog artwork template
      d1frame1win.dlgFocusID = TXT_WID_D1ENTRY1;

      d1frame1 = txwCreateWindow(
                   desktop,                     // desktop is parent
                   TXW_CANVAS,                  // class of this window
                   0,
                   0,                           // insert after ...
                   &d1frame1win,                // window setup data
                   NULL);                       // DlgProc set in CreateDlg

      txwSetWindowUShort(d1frame1, TXQWS_ID, TXT_WID_DIALOG1);

      txwSetupWindowData(
         10, 2,                                 // upper left corner
         8,  50,                                // vert + hor size
         TXWS_FRAMED | TXWS_HCHILD_SIZE | TXWS_VCHILD_SIZE,
         TXT_WID_D1TVIEW1,                      // help id
         ' ', ' ', TXWSCHEME_COLORS,
         " Viewer text 1 ",
         " Ctrl+arrow-keys sets color ",
         &d1tview1win);

      d1tview1win.tv.topline = 0;
      d1tview1win.tv.leftcol = 0;
      d1tview1win.tv.maxtop  = TXW_INVALID;
      d1tview1win.tv.maxcol  = TXW_INVALID;
      d1tview1win.tv.buf     = mainwinhelp;

      d1tview1 = txwCreateWindow(
                   d1frame1,
                   TXW_TEXTVIEW,                // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1tview1win,                // window setup data
                   txwColorViewWinProc);        // window procedure

      txwSetWindowUShort(d1tview1, TXQWS_ID, TXT_WID_D1TVIEW1);

      txwSetupWindowData(
         7,  2,                                 // upper left corner
         2, 16,                                 // vert + hor size
         TXWS_ENTRYT | TXWS_HCHILD2SIZE,        // entryfield with title
         TXT_WID_D1ENTRY1,                      // help id
         ' ', ' ', TXWSCHEME_COLORS,
         "Cylinders", "",
         &d1entry1win);

      d1entry1win.ef.leftcol = 0;
      d1entry1win.ef.maxcol  = TXW_INVALID;
      d1entry1win.ef.rsize   = TXMAXLN;
      d1entry1win.ef.buf     = d1entry1txt;
      d1entry1win.ef.history = &d1entry1his;

      d1entry1 = txwCreateWindow(
                   d1frame1,
                   TXW_ENTRYFIELD,              // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1entry1win,                // window setup data
                   txwDefWindowProc);           // window procedure

      txwInitializeHistory( d1entry1win.ef.history, TXT_HIST_SIZE, TXT_HIST_LINE);
      txwSetWindowUShort(   d1entry1, TXQWS_ID, TXT_WID_D1ENTRY1);

      txwSetupWindowData(
         7, 18,                                 // upper left corner
         2,  6,                                 // vert + hor size
         TXWS_ENTRYT | TXWS_HCHILD2MOVE,        // entryfield with title
         TXT_WID_D1ENTRY1,                      // help id
         ' ', ' ', TXWSCHEME_COLORS,
         "Heads", "",
         &d1entryHwin);

      d1entryHwin.ef.leftcol = 0;
      d1entryHwin.ef.maxcol  = TXW_INVALID;
      d1entryHwin.ef.rsize   = TXMAXLN;
      d1entryHwin.ef.buf     = d1entryHtxt;

      d1entryH = txwCreateWindow(
                   d1frame1,
                   TXW_ENTRYFIELD,              // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1entryHwin,                // window setup data
                   txwDefWindowProc);           // window procedure

      txwSetWindowUShort(d1entryH, TXQWS_ID, TXT_WID_D1ENTRYH);

      txwSetupWindowData(
         7, 24,                                 // upper left corner
         2,  6,                                 // vert + hor size
         TXWS_ENTRYT | TXWS_HCHILD2MOVE,        // entryfield with title
         TXT_WID_D1ENTRY1,                      // help id
         ' ', ' ', TXWSCHEME_COLORS,
         "Sector", "",
         &d1entrySwin);

      d1entrySwin.ef.leftcol = 0;
      d1entrySwin.ef.maxcol  = TXW_INVALID;
      d1entrySwin.ef.rsize   = TXMAXLN;
      d1entrySwin.ef.buf     = d1entryStxt;

      d1entryS = txwCreateWindow(
                   d1frame1,
                   TXW_ENTRYFIELD,              // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1entrySwin,                // window setup data
                   txwDefWindowProc);           // window procedure


      txwSetWindowUShort(d1entryS, TXQWS_ID, TXT_WID_D1ENTRYS);


      txwSetupWindowData(
         8,   32,                               // upper left corner
         1,   14,                               // vert + hor size
         TXWS_ENTRYF | TXWS_BORDERLINES |
         TXWS_HCHILD2MOVE | TXWS_HCHILD2SIZE,   // 1/2 move 1/2 size on resize
         TXT_WID_D1ENTRY2,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1entry2win);
      d1entry2win.ef.leftcol = 0;
      d1entry2win.ef.maxcol  = TXW_INVALID;
      d1entry2win.ef.rsize   = TXMAXLN;
      d1entry2win.ef.buf     = d1entry2txt;
      d1entry2win.ef.history = &d1entry2his;
      d1entry2 = txwCreateWindow( d1frame1, TXW_ENTRYFIELD, d1frame1, 0,
                                  &d1entry2win, txwDefWindowProc);
      txwInitializeHistory( d1entry2win.ef.history, TXT_HIST_SIZE, TXT_HIST_LINE);
      txwSetWindowUShort(   d1entry2, TXQWS_ID, TXT_WID_D1ENTRY2);


      txwSetupWindowData(
         2,  6,                                 // upper left corner
         3,  6,                                 // vert + hor size
         TXWS_PBUTTON,                          // button style
         TXT_WID_D1BUTTN1,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1buttn1win);

      d1buttn1win.bu.text       = d1buttn1txt;

      d1buttn1 = txwCreateWindow(
                   d1frame1,
                   TXW_BUTTON,                  // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1buttn1win,                // window setup data
                   txwDefWindowProc);           // window procedure

      txwSetWindowUShort(  d1buttn1, TXQWS_ID, TXDID_OK);
//    txwPostMsg(          d1buttn1, TXWM_SELECTED, TRUE, 0);

      txwSetupWindowData(
         2,  21,                                // upper left corner
         3,  8,                                 // vert + hor size
         TXWS_PBUTTON | TXBS_HELP,              // button style
         TXT_WID_D1BUTTN2,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1buttn2win);

      d1buttn2win.bu.text       = d1buttn2txt;

      d1buttn2 = txwCreateWindow(
                   d1frame1,
                   TXW_BUTTON,                  // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1buttn2win,                // window setup data
                   txwDefWindowProc);           // window procedure

      txwSetWindowUShort(  d1buttn2, TXQWS_ID, TXT_WID_D1BUTTN2);

      txwSetupWindowData(
         2,  38,                                // upper left corner
         3,  10,                                // vert + hor size
         TXWS_PBUTTON  |                        // button style
         TXWS_HCHILD_MOVE,                      // move when parent sizes
         TXT_WID_D1BUTTN3,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1buttn3win);

      d1buttn3win.bu.text       = d1buttn3txt;

      d1buttn3 = txwCreateWindow(
                   d1frame1,
                   TXW_BUTTON,                  // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1buttn3win,                // window setup data
                   txwDefWindowProc);           // window procedure

      txwSetWindowUShort(  d1buttn3, TXQWS_ID, TXDID_CANCEL);

      txwSetupWindowData(
         2,  62,                                // upper left corner
         2,  14,                                // vert + hor size
         TXWS_CHILDWINDOW,                      // no savebits
         TXT_WID_D1STATIC,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1staticwin);

      d1staticwin.st.buf     = d1statictxt;

      d1static = txwCreateWindow(
                   d1frame1,
                   TXW_STATIC,                  // class of this window
                   d1frame1,                    // owner window
                   0,                           // insert after ...
                   &d1staticwin,                // window setup data
                   txwDefWindowProc);           // window procedure


      txwSetWindowUShort(d1static, TXQWS_ID, TXT_WID_D1STATIC);
      txwEnableWindow(   d1static, FALSE);

      txwSetupWindowData(
         5,  62,                                // upper left corner
         3,  13,                                // vert + hor size
         TXWS_PBUTTON,                          // button style
         TXT_WID_D1PUSHB1,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1pushb1win);
      d1pushb1win.bu.text    = d1pushb1txt;
      d1pushb1 = txwCreateWindow( d1frame1, TXW_BUTTON, d1frame1, 0, &d1pushb1win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d1pushb1, TXQWS_ID, TXT_WID_D1PUSHB1);


      txwSetupWindowData(
         9,  62,
         1,  14,
         TXWS_AUTORAD,
         TXT_WID_D1RADIO1,
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1radio1win);
      d1radio1win.bu.text    = d1radio1txt;
      d1radio1win.bu.checked = &d1radio1value;
      d1radio1 = txwCreateWindow(  d1frame1, TXW_BUTTON, d1frame1, 0, &d1radio1win,
                                   txwDefWindowProc);
      txwSetWindowUShort(d1radio1, TXQWS_ID, TXT_WID_D1RADIO1);
      txwSetWindowUShort(d1radio1, TXQWS_GROUP, 2); // make seperate group


      txwSetupWindowData(
         10, 62,                                // upper left corner
         1,  14,                                // vert + hor size
         TXWS_AUTORAD,
         TXT_WID_D1RADIO2,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1radio2win);
      d1radio2win.bu.text    = d1radio2txt;
      d1radio2win.bu.checked = &d1radio2value;
      d1radio2 = txwCreateWindow( d1frame1, TXW_BUTTON, d1frame1, 0, &d1radio2win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d1radio2, TXQWS_ID, TXT_WID_D1RADIO2);
      txwSetWindowUShort(d1radio2, TXQWS_GROUP, 2); // make seperate group

      txwSetupWindowData(
         11, 62,                                // upper left corner
         1,  14,                                // vert + hor size
         TXWS_AUTORAD,
         TXT_WID_D1RADIO3,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1radio3win);
      d1radio3win.bu.text    = d1radio3txt;
      d1radio3win.bu.checked = &d1radio3value;
      d1radio3 = txwCreateWindow( d1frame1, TXW_BUTTON, d1frame1, 0, &d1radio3win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d1radio3, TXQWS_ID, TXT_WID_D1RADIO3);
      txwSetWindowUShort(d1radio3, TXQWS_GROUP, 2); // make seperate group


      txwSetupWindowData(
         13, 62,                                // upper left corner
         1,  14,                                // vert + hor size
         TXWS_AUTOCHK,
         TXT_WID_D1CHECK1,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1check1win);
      d1check1win.bu.text    = d1check1txt;
      d1check1win.bu.checked = &d1check1value;
      d1check1 = txwCreateWindow( d1frame1, TXW_BUTTON, d1frame1, 0, &d1check1win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d1check1, TXQWS_ID, TXT_WID_D1CHECK1);


      txwSetupWindowData(
         14, 62,                                // upper left corner
         1,  14,                                // vert + hor size
         TXWS_AUTOCHK,
         TXT_WID_D1CHECK2,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1check2win);
      d1check2win.bu.text    = d1check2txt;
      d1check2win.bu.checked = &d1check2value;
      d1check2 = txwCreateWindow( d1frame1, TXW_BUTTON, d1frame1, 0, &d1check2win,
                                  txwDefWindowProc);

      txwSetWindowUShort(d1check2, TXQWS_ID, TXT_WID_D1CHECK2);

      txwSetupWindowData(
         15, 62,                                // upper left corner
         1,  14,                                // vert + hor size
         TXWS_AUTOCHK,
         TXT_WID_D1CHECK3,                      // help id
         ' ', ' ', TXWSCHEME_COLORS, "", "",
         &d1check3win);
      d1check3win.bu.text    = d1check3txt;
      d1check3win.bu.checked = &d1check3value;
      d1check3 = txwCreateWindow( d1frame1, TXW_BUTTON, d1frame1, 0, &d1check3win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d1check3, TXQWS_ID, TXT_WID_D1CHECK3);

      txwPostMsg(  d1frame1, TXWM_ACTIVATE, TRUE, 0);
      rc = txwDlgBox( TXHWND_DESKTOP, TXHWND_DESKTOP,
                      txtDialogWindowProc, d1frame1, NULL);
   }
   else
   {
      TxPrint( "No desktop available yet\n");
   }
   RETURN( rc);
}                                               // end 'txtTestDialog'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test window procedure, for dialog windows
/*****************************************************************************/
static ULONG txtDialogWindowProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc    = NO_ERROR;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)
            {
               case TXc_A:                      // set 'active' flag
                  txwPostMsg( hwnd, TXWM_ACTIVATE, TRUE, 0);
                  break;

               case TXa_A:                      // reset 'active' flag
                  txwPostMsg( hwnd, TXWM_ACTIVATE, FALSE, 0);
                  break;

               case TXc_S:                      // set 'selected' flag
                  txwPostMsg( hwnd, TXWM_SELECTED, TRUE, 0);
                  break;

               case TXa_S:                      // reset 'selected' flag
                  txwPostMsg( hwnd, TXWM_SELECTED, FALSE, 0);
                  break;

               case TXc_V:                      // set visible
                  txwShowWindow( hwnd, TRUE);
                  break;

               case TXa_V:                      // set invisible
                  txwShowWindow( hwnd, FALSE);
                  break;

               case TXk_F10:
                  txwMenuBar( hwnd, hwnd, NULL, 0,
                              TXTM_BAR, 0, &mainmenu);
                  break;

               default:
                  rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  break;
            }
            break;

         case TXWM_CONTROL:
            {
               TXSELIST           *list;
               TXS_ITEM           *item;
               USHORT              notify = TXSH2FROMMP(mp1);

               list   = (TXSELIST *) mp2;
               item   = list->items[list->selected];

               switch (notify = TXSH2FROMMP(mp1))
               {
                  case TXLN_SELECT:
                     txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) item->desc, 0);
                     break;

                  case TXLN_ENTER:
                     txwDismissDlg( hwnd, item->value);
                     break;

                  default:                      // ignore other controls
                     break;                     // like LN_FOCUS
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
}                                               // end 'txtDialogWindowProc'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Simple test dialog with a selection-list (LISTBOX) client control
/*****************************************************************************/
ULONG txtListBoxDialog                          // RET   result
(
   void
)
{
   ULONG               rc    = NO_ERROR;
   TXWHANDLE           d2frame1;
   TXWINDOW            d2frame1win;
   TXWHANDLE           d2tlist1;
   TXWINDOW            d2tlist1win;
   TXWHANDLE           d2tlist2;
   TXWINDOW            d2tlist2win;
   TXWHANDLE           d2tlist3;
   TXWINDOW            d2tlist3win;
   TXWHANDLE           d2tlist4;
   TXWINDOW            d2tlist4win;
   TXWHANDLE           d2tlist5;
   TXWINDOW            d2tlist5win;
   TXWHANDLE           d2tlist6;
   TXWINDOW            d2tlist6win;
   TXWHANDLE           d2tlist7;
   TXWINDOW            d2tlist7win;

   ENTER();

   if (desktop != 0)
   {
      txwSetupWindowData(
         03,  4,                                // upper left corner
         21, 72,                                // vert + hor size
         TXWS_DFRAME | TXWS_CAST_SHADOW,        // frame style
         TXT_WID_DIALOG1,                       // help id
         ' ', ' ', TXWSCHEME_COLORS,
         " Txtest Dialog window nr 2, Listboxes ", "",
         &d2frame1win);
      d2frame1win.st.buf     = NULL;            // no dialog artwork
      d2frame1win.dlgFocusID = TXT_WID_D2LIST1;
      d2frame1 = txwCreateWindow( desktop, TXW_CANVAS, 0, 0, &d2frame1win, NULL);
      txwSetWindowUShort(d2frame1, TXQWS_ID, TXT_WID_DIALOG2);

      txwSetupWindowData( 1, 2, 9,  35,
         TXWS_FRAMED | TXWS_HCHILD_SIZE,        // resize with parent
         TXT_WID_D2LIST1,                       // help id
         ' ', ' ', TXWSCHEME_COLORS,
         " Listbox number 1 ",
         " Use arrow-keys to scroll ",
         &d2tlist1win);
      d2tlist1win.lb.list = &tstlist1;
      d2tlist1win.lb.cpos = tstlist1.selected - tstlist1.top;
      d2tlist1 = txwCreateWindow( d2frame1, TXW_LISTBOX, d2frame1, 0, &d2tlist1win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d2tlist1, TXQWS_ID, TXT_WID_D2LIST1);


      txwSetupWindowData( 1,  40, 1,  25,
         TXWS_A_MENU | TXWS_HCHILD_MOVE,
         TXT_WID_D2LIST2,                       // help id
         ' ', ' ', TXWSCHEME_COLORS,
         " Listbox number 2 ", "",
         &d2tlist2win);
      d2tlist2win.lb.list = &tstlist1;
      d2tlist2win.lb.cpos = tstlist1.selected - tstlist1.top;
      d2tlist2 = txwCreateWindow( d2frame1, TXW_LISTBOX, d2frame1, 0, &d2tlist2win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d2tlist2, TXQWS_ID, TXT_WID_D2LIST2);


      txwSetupWindowData( 12, 40, 2, 20,
         TXWS_D_SPIN | TXWS_HCHILD_MOVE,
         TXT_WID_D2LIST3,                       // help id
         ' ', ' ', TXWSCHEME_COLORS,
         " Listbox 3 ", "",
         &d2tlist3win);
      d2tlist3win.lb.list = &tstlist1;
      d2tlist3win.lb.cpos = tstlist1.selected - tstlist1.top;
      d2tlist3 = txwCreateWindow( d2frame1, TXW_LISTBOX, d2frame1, 0, &d2tlist3win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d2tlist3, TXQWS_ID, TXT_WID_D2LIST3);


      txwSetupWindowData( 12,   2, 5,  30,
         TXWS_FRAMED | TXWS_HCHILD_SIZE | TXWS_VCHILD_SIZE,
         TXT_WID_D2LIST4,                       // help id
         ' ', ' ', TXWSCHEME_COLORS,
         " Listbox number 4 ", "",
         &d2tlist4win);
      d2tlist4win.lb.list = &tstlist2;
      d2tlist4win.lb.cpos = tstlist2.selected - tstlist2.top;
      d2tlist4 = txwCreateWindow( d2frame1, TXW_LISTBOX, d2frame1, 0, &d2tlist4win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d2tlist4, TXQWS_ID, TXT_WID_D2LIST4);


      txwSetupWindowData( 6,  40, 1,   7,
         TXWS_A_MENU | TXLS_AUTO_DROP | TXWS_HCHILD_MOVE,
         TXT_WID_D2LIST5,
         ' ', ' ', TXWSCHEME_COLORS,
         " File ", "",
         &d2tlist5win);
      d2tlist5win.lb.list = &tstlist2;
      d2tlist5win.lb.cpos = tstlist2.selected - tstlist2.top;
      d2tlist5 = txwCreateWindow( d2frame1, TXW_LISTBOX, d2frame1, 0, &d2tlist5win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d2tlist5, TXQWS_ID, TXT_WID_D2LIST5);

      txwSetupWindowData( 6,  47, 1,   8,
         TXWS_A_MENU | TXLS_AUTO_DROP | TXWS_HCHILD_MOVE,
         TXT_WID_D2LIST6,
         ' ', ' ', TXWSCHEME_COLORS,
         " Edit ", "",
         &d2tlist6win);
      d2tlist6win.lb.list = &tstlist1;
      d2tlist6win.lb.cpos = tstlist1.selected - tstlist1.top;
      d2tlist6 = txwCreateWindow( d2frame1, TXW_LISTBOX, d2frame1, 0, &d2tlist6win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d2tlist6, TXQWS_ID, TXT_WID_D2LIST6);

      txwSetupWindowData( 6,  55, 1,  12,
         TXWS_A_MENU | TXLS_AUTO_DROP | TXWS_HCHILD_MOVE,
         TXT_WID_D2LIST7,
         ' ', ' ', TXWSCHEME_COLORS,
         " Commands ", "",
         &d2tlist7win);
      d2tlist7win.lb.list = &tstlist3;
      d2tlist7win.lb.cpos = tstlist3.selected - tstlist3.top;
      d2tlist7 = txwCreateWindow( d2frame1, TXW_LISTBOX, d2frame1, 0, &d2tlist7win,
                                  txwDefWindowProc);
      txwSetWindowUShort(d2tlist7, TXQWS_ID, TXT_WID_D2LIST7);

      txwPostMsg(  d2frame1, TXWM_ACTIVATE, TRUE, 0);
      rc = txwDlgBox( TXHWND_DESKTOP, TXHWND_DESKTOP,
                      txtDialogWindowProc, d2frame1, NULL);
   }
   else
   {
      TxPrint( "No desktop available yet\n");
   }
   RETURN( rc);
}                                               // end 'txtListBoxDialog'
/*---------------------------------------------------------------------------*/

