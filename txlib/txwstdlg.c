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
// TX standard dialogs
//
// Author: J. van Wijk
//
// JvW  25-03-2004 Added Linux support
// JvW  12-09-2003 Added Standard File dialogs
// JvW  15-08-2003 Added (main) menus, menubar
// JvW  30-07-2003 Added select-lists, listbox
// JvW  12-03-2002 Added entryfield, promptbox
// JvW  16-09-2001 Initial version, messagebox

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // txwa anchor interface

#define TXWD_MB_WMIN        42                  // min window width messagebox
#define TXWD_MB_WDEFAULT    76                  // default window  width

#define TXWD_WID_MBOX       0xff10              // window id messagebox
#define TXWD_WID_MBVIEW     0xff11              // window id textview
#define TXWD_WID_MBICON     0xff12              // window id icon static
                                                // window id for buttons are
                                                // equal to TXMBID_xxx 1..9

#define TXWD_WID_PBOX       0xff20              // window id promptbox
#define TXWD_WID_PBVIEW     0xff21              // window id textview
#define TXWD_WID_PBENTRY    0xff22              // window id entryfield

#define TXWD_WID_LBOX       0xff30              // window id promptbox
#define TXWD_WID_LBLIST     0xff31              // window id listbox

#define TXWD_WID_MBAR       0xff3f              // MenuBar id
#define TXWD_WID_MENU       0xff40              // MenuBar menu base id

#define TXWD_WID_VIEW       0xff50              // TextView id

#define TXWD_HELPFOPEN     (TXWD_HELPFDLG +1)   // File Open
#define TXWD_HELPFSAVE     (TXWD_HELPFDLG +2)   // File Save As
#define TXWD_HELPFDIRS     (TXWD_HELPFDLG +3)   // Directory Select

#define TXFD_WID_DFRAME     0xff60              // window-id dialog frame
#define TXFD_WID_SFNAME     0xff61              // window-id select-fname
#define TXFD_WID_VOLUME     0xff62              // window-id volume list
#define TXFD_WID_CREATE     0xff63              // window-id CREATE button
#define TXFD_WID_CFPATH     0xff64              // window-id PATH output field
#define TXFD_WID_DIRECT     0xff65              // window-id directory list
#define TXFD_WID_FNAMES     0xff66              // window-id filename list
#define TXFD_WID_ACCEPT     0xff67              // window-id ACCEPT button

#define TXWD_FD_WWIDTH      78                  // default window  width

#define TXWD_FD_MAXWILD     10                  // maximum nr of wildcards

typedef struct txwfdata                         // file dialog private data
{
   TXSELIST           *direct;                  // list of directories
   TXSELIST           *fnames;                  // list of filenames
   TXLN                wildcard;                // selection wildcard
} TXWFDATA;                                     // end of struct "txwfdata"


static TXSELIST       *txfd_drivelist = NULL;   // cached list of volumes
static int             txfd_drive_cnt = 0;      // drives in TxFsVolumes


static  char           txwNotReadyMsg[]  = "Not ready, disconnected LAN "
                                           "or unrecognized filesystem";
static  char           txwSeparatorLine[] =
   "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴"
   "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴";

static  TX1K           txwSaveAsPath     = "";
static  TX1K           txwOpenFilePath   = "";
static  TX1K           txwSelDirFilePath = "";

// Consolidated help section for most TXWIN controls
static char           *txwinhelp[] =
{
   "#100 Controls, Messagebox",
   "",
   " A Message box is used to pass a relative small text (the message)",
   " to you, and let you acknowledge it by pushing a button or use the",
   " <Esc> key on the keyboard.",
   "",
   " Most messageboxes can be moved, resized and minimized/maximized",
   " For moving, use the <Alt> + 'm' key, and then the arrow keys.",
   " The <Insert> key toggles between MOVE and SIZE as indicated",
   " by the text in the status line near the bottom of the screen.",
   "",
   " If the box is too small for the text to fit, you can scroll using:",
   "    Ctrl + Home   Scroll up to first non-empty line in the text",
   "    Ctrl + End    Scroll down to last line in the text",
   "    Alt  + . (>)  Scroll message text right by 1 char",
   "    Alt  + , (<)  Scroll message text left  by 1 char",
   "    Up   / Down   Scroll message text up / down line by line",
   "",
   " Message boxes come in several flavors with the most significant",
   " difference being the number and kind of push buttons provided.",
   "",
   " Possible buttons with their most common meanings are:",
   "",
   "   [Ok]        Accept the message, acknowledge it",
   "   [Cancel]    Don't accept the message, deny it",
   "   [Yes]       Accept the message, answer Yes to the question",
   "   [No]        Don't accept the message, answer No to the question",
   "   [Enter]     Accept the message, acknowledge it",
   "   [Abort]     Abort the failing action indicated by the message",
   "   [Retry]     Retry the failed action indicated by the message",
   "   [Ignore]    Ignore the failed action, and continue",
   "   [Help]      Give additional help on the content of the message",
   "",
   " Pushing one of the buttons using the  <Space bar> or <Enter> key,",
   " or using the <Esc> key will end the message box display, and pass",
   " on the associated result to the application.",
   " The <Esc> key is equivalent to the [No] or [Cancel] buttons.",
   "",
   " Help about the message contents and consequences can be called with",
   " the <F1> function key, or the [Help] button when available.",
   "",
   " Once help is displayed, another <F1> will call up the help info",
   " about the message box itself (this page)",
   "",
   TXHELPITEM(200,"Controls, Prompt Box")
   "",
   " A Prompt box is used to get a single value from you to be used as",
   " input to the program. There will be a short message explaining what",
   " the value is needed for",
   "",
   " Prompt boxes may be enhanced with additional fields and buttons",
   " called custom 'widgets' to specify more options or values.",
   "",
   " Most promptboxes can be moved, resized and minimized/maximized",
   " For moving, use the <Alt> + 'm' key, and then the arrow keys.",
   " The <Insert> key toggles between MOVE and SIZE as indicated",
   " by the text in the status line near the bottom of the screen.",
   "",
   " Help about the prompted value and consequences can be called with",
   " the <F1> function key",
   "",
   " Once help is displayed, another <F1> will call up the help info",
   " about the prompt box itself (this page)",
   "",
   TXHELPITEM(300,"Controls, List dialogs")
   "",
   " The list dialog is used to display a list, usually for menu selection",
   "",
   "",
   TXHELPITEM(500,"Controls, Text Viewer")
   "",
   " The text viewer window is used to display text lines",
   "",
   TXHELPITEM(600,"Confirm, Untrusted command")
   "",
   " Untrusted external command!",
   "",
   " This is to acknowledge that you want the displayed command to",
   " be executed, knowing it is not in the list of trusted commands.",
   "",
   " It is likely that you just made a typing error ...",
   "",
   "",
   "",
   TXHELPITEM(701,"Controls, File Open Dialog")
   " Used to select a file to be used by the program for reading.",
   "",
   " Note: To get program specific HELP on this dialog, press <F1>",
   "",
   " The dialog has an entryfield that can contain a fileselection",
   " to restrict displayed files (wildcard). On selection of a file",
   " this field will contain the base-filename for it (no path).",
   "",
   " You can retrieve the original value (wildcard) with <F2>.",
   "",
   " The path for this filename, and for other filenames displayed",
   " in the list, is shown just below this entryfield and will be",
   " updated when changing drives and/or directories.",
   "",
   " Then there are three listboxes to select DRIVE, DIRECTORY and",
   " the actual FILENAME.",
   "",
   " The dialog may be enhanced with additional fields and buttons",
   " called custom 'widgets' to specify more options or values.",
   " For help on those widgets, use the <F1> help key a 2nd time",
   "                                    ------------------------",
   "",
   " The File-dialog can be moved, resized and minimized/maximized",
   " For moving, use the <Alt> + 'm' key, and then the arrow keys.",
   " The <Insert> key toggles between MOVE and SIZE as indicated",
   " by the text in the status line near the bottom of the screen.",
   "",
   "",
   " Combined 'Filename or wildcard' entryfield",
   " ==========================================",
   "",
   " This field my contain a selection wildcard, limiting the files",
   " displayed in the FILENAME list, or it may contain a complete",
   " filename (non-wildcard) that represents the selected file.",
   "",
   " Using <ENTER> with a complete filename will end the dialog and",
   " return the selected path and filename to the application.",
   "",
   " At any time, for example if a filename (from the filelist) is",
   " displayed here, you can retrieve the original wildcard string",
   " with the UP-arrow key. Using <ENTER> on this field will update",
   " the other fields and lists in the dialog accordingly.",
   "",
   " The wildcard-string entered may contain MULTIPLE wildcards",
   " separated by a ';' character, to allow more complex selections",
   " to me made. An example would be to display all .CMD and .BAT",
   " files in the displayed path using the wildcard-string:",
   "",
   "                   *.bat;*.cmd",
   "",
   " Another example is displaying any .LOG file that has one of,",
   " three possible fragments in the name:",
   "",
   "                   *test*.log;*first*.log;*\\my*.log",
   "",
   " Note the leading '*' characters in each, these are REQUIRED as",
   " the whole path will be considered in the match-process.",
   "",
   "",
   " List of selectable volumes (driveletters)",
   " =========================================",
   "",
   " This is a 'spin-value' list that shows the current volume on",
   " a single line, and allows selection of others using the <Up>",
   " and <Down> arrow keys."
   "",
   " It contains all the volumes (driveletters) that are available",
   " to the operating system for accessing regular files.",
   "",
   " The list can be expanded to a pop-up by using <Alt> + <Enter>",
   "",
   " Selection of a new volume, either by using the <Up> or <Down>",
   " arrow keys or by using <Enter> on the popup, will result in",
   " the current PATH being changed to the root-directory on that",
   " volume and the directory/filename lists being updated as well.",
   "",
   "",
   " Sorting the (expanded) volume list",
   " ==================================",
   "",
   " The default (Drive letter) sorting can be changed using:",
   "",
   "   " TXS_SORT_KEYS2 " Total volume size",
   "   " TXS_SORT_KEYS3 " Volume label",
   "   " TXS_SORT_KEYS4 " Freespace on volume",
   "   " TXS_SORT_KEYS5 " Drive letter",
   "   " TXS_SORT_KEYS6 " Filesystem name",
   "   " TXS_SORT_KEYS7 " Unsort, restore original list ordering",
   "   " TXS_SORT_KEYS8 " Reverse the current list ordering",
   "",
   "",
   " List of directories in current PATH",
   " ===================================",
   "",
   " This is a selection list where all subdirectories, and",
   " the parent directory of the current PATH are shown.",
   "",
   " Selection of one of the directories by highlighting it and",
   " using the <Enter> key results in changing the current PATH",
   " to that directory and the directory/filename being updated.",
   "",
   "",
   " List of matching files in the current PATH",
   " ==========================================",
   "",
   " This is a selection list of all files in the current PATH that",
   " match the wildcard specification in the filename field (if any).",
   "",
   " Selection of one of the filenames by highlighting it and using",
   " the <Enter> key results in THAT file being the selected one.",
   " This will end the dialog and return the selected path and",
   " filename to the application.",
   "",
   "",
   " Sorting the directory or file lists",
   " ===================================",
   "",
   " The default (alphanumeric, ascending) sorting can be changed using:",
   "",
   "   " TXS_SORT_KEYS1 " Text    at last dot (extension), case sensitive",
   "   " TXS_SORT_KEYS2 " Text in listbox     (file-name), case sensitive",
   "   " TXS_SORT_KEYS3 " Description, col 42 (file-name), ignoring case",
   "   " TXS_SORT_KEYS4 " Descr.  at last dot (extension), ignoring case",
   "   " TXS_SORT_KEYS5 " Description, col  6 (date/time), descending",
   "   " TXS_SORT_KEYS6 " Description, col 26 (file-size), descending",
   "   " TXS_SORT_KEYS7 " Unsort, restore original list ordering",
   "   " TXS_SORT_KEYS8 " Reverse the current list ordering",
   "",
   "",
   " [ OK ] button to accept the path",
   " ================================",
   "",
   " The [ OK ] button or the <F4> key will end the dialog with the",
   " currently displayed full-path (in blue) as the selected path.",
   "",
   TXHELPITEM(702,"Controls, File Save As Dialog")
   " Used to specify a file to be used by the program for writing.",
   "",
   " Note: To get program specific HELP on this dialog, press <F1>",
   "",
   " The dialog has an entryfield 'Filename or Wildcard' that can",
   " contain a complete filename to use directly, or a wildcard to",
   " restrict displayed files. On selection of a file this field",
   " will automatically get the base-filename for it (no path).",
   "",
   " You can retrieve the original value (wildcard) with <F2>.",
   "",
   " The path for this filename, and for other filenames displayed",
   " in the list, is shown just below this entryfield and will be",
   " updated when changing drives and/or directories.",
   "",
   " Then there are three listboxes to select DRIVE, DIRECTORY and",
   " the actual FILENAME.",
   "",
   " The dialog may be enhanced with additional fields and buttons",
   " called custom 'widgets' to specify more options or values.",
   " For help on those widgets, use the <F1> help key a 2nd time",
   "                                    ------------------------",
   "",
   " The File-dialog can be moved, resized and minimized/maximized",
   " For moving, use the <Alt> + 'm' key, and then the arrow keys.",
   " The <Insert> key toggles between MOVE and SIZE as indicated",
   " by the text in the status line near the bottom of the screen.",
   "",
   "",
   " Combined 'Filename or wildcard' entryfield",
   " ==========================================",
   "",
   " This field my contain a selection wildcard, limiting the files",
   " displayed in the FILENAME list, or it may contain a complete",
   " filename (non-wildcard) that represents the file to be opened.",
   "",
   " Using <ENTER> with a complete filename will end the dialog and",
   " return the selected path and filename to the application.",
   "",
   " At any time, for example if a filename (from the filelist) is",
   " displayed here, you can retrieve the original wildcard string",
   " with the UP-arrow key. Using <ENTER> on this field will update",
   " the other fields and lists in the dialog accordingly.",
   "",
   " The wildcard-string entered may contain MULTIPLE wildcards",
   " separated by a ';' character, to allow more complex selections",
   " to me made. An example would be to display all .CMD and .BAT",
   " files in the displayed path using the wildcard-string:",
   "",
   "                   *.bat;*.cmd",
   "",
   " Another example is displaying any .LOG file that has one of,",
   " three possible fragments in the name:",
   "",
   "                   *test*.log;*first*.log;*\\my*.log",
   "",
   " Note the leading '*' characters in each, these are REQUIRED as",
   " the whole path will be considered in the match-process.",
   "",
   "",
   " List of selectable volumes (driveletters)",
   " =========================================",
   "",
   " This is a 'spin-value' list that shows the current volume on",
   " a single line, and allows selection of others using the <Up>",
   " and <Down> arrow keys."
   "",
   " It contains all the volumes (driveletters) that are available",
   " to the operating system for accessing regular files.",
   "",
   " The list can be expanded to a pop-up by using <Alt> + <Enter>",
   "",
   " Selection of a new volume, either by using the <Up> or <Down>",
   " arrow keys or by using <Enter> on the popup, will result in",
   " the current PATH being changed to the root-directory on that",
   " volume and the directory/filename lists being updated as well.",
   "",
   "",
   " Sorting the (expanded) volume list",
   " ==================================",
   "",
   " The default (Drive letter) sorting can be changed using:",
   "",
   "   " TXS_SORT_KEYS2 " Total volume size",
   "   " TXS_SORT_KEYS3 " Volume label",
   "   " TXS_SORT_KEYS4 " Freespace on volume",
   "   " TXS_SORT_KEYS5 " Drive letter",
   "   " TXS_SORT_KEYS6 " Filesystem name",
   "   " TXS_SORT_KEYS7 " Unsort, restore original list ordering",
   "   " TXS_SORT_KEYS8 " Reverse the current list ordering",
   "",
   "",
   " 'Make Directory' button and entryfield",
   " ======================================",
   "",
   " The button will present you with a prompt-box entryfield to ",
   " specify the name of a new directory to be created.",
   "",
   " This will usually be a subdirectory for the current PATH used",
   " in the dialog, but it can also be an absolute location on another",
   " drive, or more than one level of subdirectories.",
   "",
   " Examples:    'newdir'        create directory 'newdir' in PATH",
   "              'sub\\new'       create directory 'sub'    in PATH",
   "                              and directory 'new' in this 'sub'",
   "              'h:\\tmp'        create directory 'tmp' on H: drive",
   "",
   " The specified directory or directories will be created on closing",
   " this prompt-box using <ENTER>, and the default PATH for the File",
   " dialog will be changed to this new location.",
   "",
   "",
   " List of directories in current PATH",
   " ===================================",
   "",
   " This is a selection list where all subdirectories, and",
   " the parent directory of the current PATH are shown.",
   "",
   " Selection of one of the directories by highlighting it and",
   " using the <Enter> key results in changing the current PATH",
   " to that directory and the directory/filename being updated.",
   "",
   "",
   " List of matching files in the current PATH",
   " ==========================================",
   "",
   " This is a selection list of all files in the current PATH that",
   " match the wildcard specification in the filename field (if any).",
   "",
   " Selection of one of the filenames by highlighting it and using",
   " the <Enter> key results in THAT file being the selected one.",
   " This will end the dialog and return the selected path and",
   " filename to the application.",
   "",
   "",
   " Sorting the directory or file lists",
   " ===================================",
   "",
   " The default (alphanumeric, ascending) sorting can be changed using:",
   "",
   "   " TXS_SORT_KEYS1 " Text    at last dot (extension), case sensitive",
   "   " TXS_SORT_KEYS2 " Text in listbox     (file-name), case sensitive",
   "   " TXS_SORT_KEYS3 " Description, col 42 (file-name), ignoring case",
   "   " TXS_SORT_KEYS4 " Descr.  at last dot (extension), ignoring case",
   "   " TXS_SORT_KEYS5 " Description, col  6 (date/time), descending",
   "   " TXS_SORT_KEYS6 " Description, col 26 (file-size), descending",
   "   " TXS_SORT_KEYS7 " Unsort, restore original list ordering",
   "   " TXS_SORT_KEYS8 " Reverse the current list ordering",
   "",
   "",
   " [ OK ] button to accept the path",
   " ================================",
   "",
   " The [ OK ] button or the <F4> key will end the dialog with the",
   " currently displayed full-path (in blue) as the selected path.",
   "",
   TXHELPITEM(703,"Controls, Select Directory Dialog")
   " Used to select a directory for reading or saving files.",
   "",
   " Note: To get program specific HELP on this dialog, press <F1>",
   "",
   " The dialog has an entryfield 'Filename or wildcard' that can",
   " be used to directly type a new drive/directory. On <Enter>",
   " the subdirectories for the specified path will  be listed in",
   " the selection list below this field.",
   "",
   " The current path for the listed subdirectories is shown just",
   " below this entryfield and will be updated when changing drives",
   " and/or directories.",
   "",
   " Then there are two listboxes to select DRIVE and DIRECTORY.",
   "",
   " The dialog may be enhanced with additional fields and buttons",
   " called custom 'widgets' to specify more options or values.",
   " For help on those widgets, use the <F1> help key a 2nd time",
   "                                    ------------------------",
   "",
   " The File-dialog can be moved, resized and minimized/maximized",
   " For moving, use the <Alt> + 'm' key, and then the arrow keys.",
   " The <Insert> key toggles between MOVE and SIZE as indicated",
   " by the text in the status line near the bottom of the screen.",
   "",
   "",
   " Combined 'Filename or wildcard' entryfield",
   " ==========================================",
   "",
   " This field my contain a selection wildcard, limiting the files",
   " displayed in the FILENAME list, or it may contain a complete",
   " filename (non-wildcard) that represents the selected file.",
   "",
   " Using <ENTER> with a complete filename will end the dialog and",
   " return the selected path and filename to the application.",
   "",
   " At any time, for example if a filename (from the filelist) is",
   " displayed here, you can retrieve the original wildcard string",
   " with the UP-arrow key. Using <ENTER> on this field will update",
   " the other fields and lists in the dialog accordingly.",
   "",
   " The wildcard-string entered may contain MULTIPLE wildcards",
   " separated by a ';' character, to allow more complex selections",
   " to me made. An example would be to display all .CMD and .BAT",
   " files in the displayed path using the wildcard-string:",
   "",
   "                   *.bat;*.cmd",
   "",
   " Another example is displaying any .LOG file that has one of,",
   " three possible fragments in the name:",
   "",
   "                   *test*.log;*first*.log;*\\my*.log",
   "",
   " Note the leading '*' characters in each, these are REQUIRED as",
   " the whole path will be considered in the match-process.",
   "",
   "",
   " List of selectable volumes (driveletters)",
   " =========================================",
   "",
   " This is a 'spin-value' list that shows the current volume on",
   " a single line, and allows selection of others using the <Up>",
   " and <Down> arrow keys."
   "",
   " It contains all the volumes (driveletters) that are available",
   " to the operating system for accessing regular files.",
   "",
   " The list can be expanded to a pop-up by using <Alt> + <Enter>",
   "",
   " Selection of a new volume, either by using the <Up> or <Down>",
   " arrow keys or by using <Enter> on the popup, will result in",
   " the current PATH being changed to the root-directory on that",
   " volume and the directory/filename lists being updated as well.",
   "",
   "",
   " Sorting the (expanded) volume list",
   " ==================================",
   "",
   " The default (Drive letter) sorting can be changed using:",
   "",
   "   " TXS_SORT_KEYS2 " Total volume size",
   "   " TXS_SORT_KEYS3 " Volume label",
   "   " TXS_SORT_KEYS4 " Freespace on volume",
   "   " TXS_SORT_KEYS5 " Drive letter",
   "   " TXS_SORT_KEYS6 " Filesystem name",
   "   " TXS_SORT_KEYS7 " Unsort, restore original list ordering",
   "   " TXS_SORT_KEYS8 " Reverse the current list ordering",
   "",
   "",
   " 'Make Directory' button and entryfield",
   " ======================================",
   "",
   " The button will present you with a prompt-box entryfield to ",
   " specify the name of a new directory to be created.",
   "",
   " This will usually be a subdirectory for the current PATH used",
   " in the dialog, but it can also be an absolute location on another",
   " drive, or more than one level of subdirectories.",
   "",
   " Examples:    'newdir'        create directory 'newdir' in PATH",
   "              'sub\\new'       create directory 'sub'    in PATH",
   "                              and directory 'new' in this 'sub'",
   "              'h:\\tmp'        create directory 'tmp' on H: drive",
   "",
   " The specified directory or directories will be created on closing",
   " this prompt-box using <ENTER>, and the default PATH for the File",
   " dialog will be changed to this new location.",
   "",
   "",
   " List of directories in current PATH",
   " ===================================",
   "",
   " This is a selection list where all subdirectories, and",
   " the parent directory of the current PATH are shown.",
   "",
   " Selection of one of the directories by highlighting it and",
   " using the <Enter> key results in changing the current PATH",
   " to that directory and the directory/filename being updated.",
   "",
   "",
   " Sorting the directory list",
   " ==========================",
   "",
   " The default (alphanumeric, ascending) sorting can be changed using:",
   "",
   "   " TXS_SORT_KEYS1 " Text    at last dot (extension), case sensitive",
   "   " TXS_SORT_KEYS2 " Text in listbox     (file-name), case sensitive",
   "   " TXS_SORT_KEYS3 " Description, col 42 (file-name), ignoring case",
   "   " TXS_SORT_KEYS4 " Descr.  at last dot (extension), ignoring case",
   "   " TXS_SORT_KEYS5 " Description, col  6 (date/time), descending",
   "   " TXS_SORT_KEYS7 " Unsort, restore original list ordering",
   "   " TXS_SORT_KEYS8 " Reverse the current list ordering",
   "",
   "",
   " [ OK ] button to accept the path",
   " ================================",
   "",
   " The [ OK ] button or the <F4> key will end the dialog with the",
   " currently displayed full-path (in blue) as the selected path.",
   "",
   TXHELPITEM(920, "Critical Error, Retry or Fail")
   "",
   " This is a DOS critical error message, meaning it is something the",
   " operating system itself can't do much about, but maybe you can!",
   "",
   " After correcting the problem (or not), you can choose how the",
   " program should continue processing:",
   "",
   "       Retry    Will cause the failing operation to be retried.",
   "",
   "       Fail     Will cause the error to be reported to the program",
   "                itself, allowing it to take appropriate measures.",
   "",
   TXHELPITEM(930, "Disabled menu item selected")
   "",
   " The menu item just selected is DISABLED, this usually means that",
   " the functionality for this menu item can not work at this point",
   " in time, but could be made available.",
   "",
   " That may require you to perform other actions first, like opening",
   " an object, or changing to another 'state' or 'mode' in the program.",
   "",
   " Calling up the item specific HELP on the menu item using the <F1> key",
   " may give you more specific info on what to do to enable the item.",
   "",
   " As an example, a 'Save as' menu item in a text editor",
   " would only be enabled if there is some text to save.",
   "",
   " The application may also have specified one or more 'reasons' for",
   " disabling a menu-item, which are displayed in the message dialog",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
   NULL
};


// Button texts for the MessageBox
static char txmb_bt_ok[]       = " OK ";
static char txmb_bt_cancel[]   = "Cancel";
static char txmb_bt_yes[]      = "Yes";
static char txmb_bt_no[]       = " No ";
static char txmb_bt_enter[]    = "Enter";
static char txmb_bt_abort[]    = "Abort";
static char txmb_bt_retry[]    = "Retry";
static char txmb_bt_ignore[]   = "Ignore";
static char txmb_bt_fail[]     = "Fail";
static char txmb_bt_help[]     = "Help";

       char txwstd_footer[]    = "F1=Help F4=OK F12=Minimize "
#ifndef UNIX
                                 "Alt+m=Move/Size Alt+F10=Maximize"
#endif
                                 ;
       char txwfld_footer[]    = "F1=Help F2=wildcard F4=OK F12=Min Sort:Ctrl+"
#ifndef UNIX
                                                                  "N/E/T/B/U/R";
#else
                                                                  "X/D/T/B/U/R";
#endif

// Dialog window procedure, for the MsgBox dialog
static ULONG txwMsgBoxDlgWinProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Window procedure, for entry-field, includes automatic value-echo
static ULONG txwPromptDlgWinProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Window procedure, for ListBox popup
static ULONG txwListDlgWinProc                  // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Window procedure, for a MenuBar Dialog
static ULONG txwMenuDlgWinProc                  // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Window procedure, for some FileDialog controls
static ULONG txwSfdControlWinProc               // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

/*****************************************************************************/
// Initialize all standard dialog data structures
/*****************************************************************************/
BOOL txwInitStandardDialogs
(
   void
)
{
   BOOL                rc = TRUE;               // function return

   ENTER();

   txwInitGenericWidgets();                     // initialize widgets

   txwRegisterHelpText( TXWH_SYSTEM_SECTION_BASE, "txcontrols", "TX Windows controls help",  txwinhelp);

   if (TxaExeSwitchUnSet('F'))                  // -F- is no-autoFail
   {
      TxFsAutoFailCriticalErrors( FALSE);       // allow TXWIN popups ...
   }
   BRETURN (rc);
}                                               // end 'txwInitStandardDialogs'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate all standard dialog data structures
/*****************************************************************************/
void txwTermStandardDialogs
(
   void
)
{
   ENTER();

   txSelDestroy( &txfd_drivelist);              // free drives   list
   txfd_drive_cnt = 0;

   TxFsAutoFailCriticalErrors( TRUE);           // no popups anymore ...

   txwTermGenericWidgets();                     // terminate widgets

   VRETURN();
}                                               // end 'txwTermStandardDialogs'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display standard message-box dialog with text, buttons and optional Widgets
/*****************************************************************************/
ULONG txwMessageBoxWidgets
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *message,                 // IN    message text
   char               *title,                   // IN    title for the dialog
   ULONG               helpid,                  // IN    help on message
   ULONG               flags                    // IN    specification flags
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRACES(("parent:%8.8lx  owner:%8.8lx  title:'%s'\n", parent, owner, title));
   TRACES(("helpid:%8.8lx = %lu  flags:%8.8lx\n", helpid, helpid, flags));

   if (txwIsWindow( TXHWND_DESKTOP))            // is there a desktop ?
   {
      TXRECT           position;                // non persistent size/position
      TXRECT          *framepos;                // frame position structure
      TXRECT           parentpos;               // parent size/position
      TXWHANDLE        mframe;                  // message-box frame
      TXWHANDLE        mview;                   // help view text
      TXWHANDLE        mbut1;                   // button 1
      TXWHANDLE        mbut2;                   // button 2
      TXWHANDLE        mbut3;                   // button 3
      TXWHANDLE        mbut4;                   // button 4
      TXWINDOW         window;                  // setup window data
      ULONG            style;
      char            *tbut1 = NULL;            // button texts
      char            *tbut2 = NULL;
      char            *tbut3 = NULL;
      char            *tbut4 = NULL;
      USHORT           rbut1 = 0;               // button result codes
      USHORT           rbut2 = 0;
      USHORT           rbut3 = 0;
      USHORT           rbut4 = 0;
      short            phsize;                  // parent window width
      short            pvsize;                  // parent window height
      USHORT           focusID;                 // Button to get focus
      char           **mText;
      int              lines;                   // nr of lines
      int              ll;                      // real max line length
      int              b1 = 0;                  // button-1 left reference
      short            wvsize;                  // widget vertical size
      int              ww = TXWD_MB_WMIN;       // minimum popup width

      if ((strlen( message) >= 160) || (gwdata && (gwdata->count != 0)))
      {
         ww = TXWD_MB_WDEFAULT;                 // use wider message popup
      }

      switch (flags & TXMB_PRIMARYSTYLES)
      {
         case TXMB_OKCANCEL:
            tbut1   = txmb_bt_ok;        rbut1 = TXMBID_OK;
            tbut3   = txmb_bt_cancel;    rbut3 = TXMBID_CANCEL;
            focusID = (flags & TXMB_DEFBUTTON2) ? rbut3 : rbut1;
            break;

         case TXMB_RETRYCANCEL:
            tbut1   = txmb_bt_retry;     rbut1 = TXMBID_RETRY;
            tbut3   = txmb_bt_cancel;    rbut3 = TXMBID_CANCEL;
            focusID = (flags & TXMB_DEFBUTTON2) ? rbut3 : rbut1;
            break;

         case TXMB_ABORTRETRYIGNORE:
            tbut1   = txmb_bt_abort;     rbut1 = TXMBID_ABORT;
            tbut2   = txmb_bt_retry;     rbut2 = TXMBID_RETRY;
            tbut3   = txmb_bt_ignore;    rbut3 = TXMBID_IGNORE;
            focusID = (flags & TXMB_DEFBUTTON3) ? rbut3 :
                      (flags & TXMB_DEFBUTTON2) ? rbut2 : rbut1;
            break;

         case TXMB_RETRYFAIL:
            tbut1   = txmb_bt_retry;     rbut1 = TXMBID_RETRY;
            tbut3   = txmb_bt_fail;      rbut3 = TXMBID_FAIL;
            focusID =                    rbut3; // fail always default
            break;

         case TXMB_YESNO:
            tbut1   = txmb_bt_yes;       rbut1 = TXMBID_YES;
            tbut3   = txmb_bt_no;        rbut3 = TXMBID_NO;
            focusID = (flags & TXMB_DEFBUTTON2) ? rbut3 : rbut1;
            break;

         case TXMB_YESNOCANCEL:
            tbut1   = txmb_bt_yes;       rbut1 = TXMBID_YES;
            tbut2   = txmb_bt_no;        rbut2 = TXMBID_NO;
            tbut3   = txmb_bt_cancel;    rbut3 = TXMBID_CANCEL;
            focusID = (flags & TXMB_DEFBUTTON3) ? rbut3 :
                      (flags & TXMB_DEFBUTTON2) ? rbut2 : rbut1;
            break;

         case TXMB_CANCEL:
            if (flags & TXMB_HELP)
            {
               tbut1 = txmb_bt_cancel;   rbut1 = TXMBID_CANCEL;
               focusID =                 rbut1;
            }
            else
            {
               tbut2 = txmb_bt_cancel;   rbut2 = TXMBID_CANCEL;
               focusID =                 rbut2;
            }
            break;

         case TXMB_ENTER:
            if (flags & TXMB_HELP)
            {
               tbut1 = txmb_bt_enter;    rbut1 = TXMBID_ENTER;
               focusID =                 rbut1;
            }
            else
            {
               tbut2 = txmb_bt_enter;    rbut2 = TXMBID_ENTER;
               focusID =                 rbut2;
            }
            break;

         case TXMB_ENTERCANCEL:
            tbut1   = txmb_bt_enter;     rbut1 = TXMBID_ENTER;
            tbut3   = txmb_bt_cancel;    rbut3 = TXMBID_CANCEL;
            focusID = (flags & TXMB_DEFBUTTON2) ? rbut3 : rbut1;
            break;

         case TXMB_OK:
         default:
            if (flags & TXMB_HELP)
            {
               tbut1   = txmb_bt_ok;     rbut1 = TXMBID_OK;
               focusID =                 rbut1;
            }
            else
            {
               tbut2   = txmb_bt_ok;     rbut2 = TXMBID_OK;
               focusID =                 rbut2;
            }
            break;
      }
      if (flags & TXMB_HELP)
      {
         tbut4      = txmb_bt_help;   rbut4 = TXMBID_HELP;
         if (ww < TXWD_MB_WDEFAULT)
         {
            ww  += 12;
         }
      }
      if (flags & TXMB_ANYICON)
      {
         if (ww < TXWD_MB_WDEFAULT)
         {
            ww   = TXWD_MB_WDEFAULT;
         }
         b1 = 11;                               // 12 characters for icon
      }
      else if (ww == TXWD_MB_WDEFAULT)
      {
         b1 = 9;                                // center icons in large MB
      }

      if (rbut2 && !(rbut1 || rbut3 || rbut4))  // center single button-2
      {
         b1 = ((ww - strlen(tbut2) -6) /2) -15; // middle, minus fixed offset
      }
      TRACES(( "left button pos b1 : %u for ww = %u\n", b1, ww));

      ll    = ww -4;
      mText = txString2Text( message, &ll, &lines);

      if ((wvsize = txwWidgetSize( gwdata, 1, NULL)) != 0)
      {
         lines += wvsize;                       // add widgets and separator
      }

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
         if (parentpos.left  + ww < phsize)
         {
            framepos->right  = ww;
            framepos->left   = phsize - ww;
            if (flags & TXMB_HCENTER)
            {
               framepos->left /= 2;             // center horizontally
            }
         }
         if (parentpos.top  + lines + 8 < pvsize)
         {
            framepos->bottom = lines + 8;
            if (flags & TXMB_VCENTER)           // center in top 60 lines
            {                                   // 1/4 above, 3/4 below ...
               framepos->top += ((min( pvsize, 60) - framepos->bottom) / 4);
            }
         }
      }
      else                                      // calculated size of Dlg
      {                                         // (but keeps position)
         framepos->right  = ww;
         framepos->bottom = lines + 8;
      }
      TRECTA( "pos/size", (framepos));

      style = TXWS_DIALOG | TXWS_DISABLED | TXCS_CLOSE_BUTTON;
      if (flags & TXMB_MOVEABLE)
      {
         style |= TXWS_MOVEABLE;                // make frame move/sizeable
      }
      txwSetupWindowData(
         framepos->top,                         // upper left corner
         framepos->left,
         framepos->bottom,                      // vertical size
         framepos->right,                       // horizontal size
         style | TXWS_CAST_SHADOW,              // window frame style
         TXWD_HELPMBOX,                         // messagebox help
         ' ', ' ', TXWSCHEME_COLORS,
         title, txwstd_footer,
         &window);
      window.st.buf     = NULL;                 // NO artwork attached
      window.dlgFocusID = focusID;              // Field to get Focus
      mframe = txwCreateWindow( parent, TXW_CANVAS, 0, 0, &window, NULL);
      txwSetWindowPtr( mframe, TXQWP_DATA, framepos); // for exit position

      if (wvsize != 0)                          // do we have any widgets ?
      {
         if ((rc = txwCreateWidgets( mframe, gwdata, 1, 1)) == NO_ERROR)
         {
            txwSetupWindowData( wvsize, 0, 1, ww,
                  TXWS_OUTPUT | TXWS_HCHILD_SIZE | TXWS_VCHILD_MOVE,
                  0, ' ', ' ',  TXWSCHEME_COLORS, "",  "", &window);
            window.sl.buf = txwSeparatorLine;
            txwCreateWindow( mframe, TXW_STLINE, mframe, 0, &window,
                             txwDefWindowProc);
         }
      }

      style = TXWS_CHILDWINDOW | TXWS_DISABLED;
      if (flags & TXMB_MOVEABLE)
      {
         style |= TXWS_HCHILD_SIZE;             // resize with parent
         style |= TXWS_VCHILD_SIZE;
      }
      txwSetupWindowData(
         1 + wvsize,                            // UL corner, line   (relative)
         (framepos->right >= ll +2) ? ((framepos->right -ll -2) / 2) :
         1,                                     // UL corner, column (relative)
         framepos->bottom - 8 - wvsize,         // vertical size
         framepos->right  - 4,                  // horizontal size
         style,                                 // window frame style
         helpid,                                // help on the message
         ' ', ' ', TXWSCHEME_COLORS, "",  "",
         &window);
      window.tv.topline  = 0;
      window.tv.leftcol  = 0;
      window.tv.maxtop   = TXW_INVALID;
      window.tv.maxcol   = TXW_INVALID;
      window.tv.markLine = 0;
      window.tv.markCol  = 0;
      window.tv.markSize = 0;
      window.tv.buf      = mText;
      mview = txwCreateWindow( mframe, TXW_TEXTVIEW, mframe, 0, &window,
                               txwDefWindowProc);
      txwSetWindowUShort( mview,  TXQWS_ID,   TXWD_WID_MBVIEW);

      if (rbut1 != 0)                           // do we have a button 1 ?
      {
         style = TXWS_PBUTTON;
         if (flags & TXMB_MOVEABLE)
         {
            style |= TXWS_VCHILD_MOVE;          // move with vertical resize
         }
         txwSetupWindowData(
            framepos->bottom - 6,               // UL corner, line   (relative)
            b1 +  1,                            // UL corner, column (relative)
            3,                                  // vertical size
            strlen( tbut1) + 4,                 // horizontal size   (auto)
            style,                              // window frame style
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.bu.text = tbut1;
         mbut1 = txwCreateWindow(   mframe, TXW_BUTTON, mframe, 0, &window,
                                    txwDefWindowProc);
         txwSetWindowUShort( mbut1, TXQWS_ID, rbut1); // returncode == ID
      }

      if (rbut2 != 0)                           // do we have a button 2 ?
      {
         style = TXWS_PBUTTON;
         if (flags & TXMB_MOVEABLE)
         {
            style |= TXWS_HCHILD2MOVE;          // 1/2 move when parent resizes
            style |= TXWS_VCHILD_MOVE;          // move with vertical resize
         }
         txwSetupWindowData(
            framepos->bottom - 6,               // UL corner, line   (relative)
            b1 + 15,                            // UL corner, column (relative)
            3,                                  // vertical size
            strlen( tbut2) + 4,                 // horizontal size   (auto)
            style,                              // window frame style
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.bu.text = tbut2;
         mbut2 = txwCreateWindow(   mframe, TXW_BUTTON, mframe, 0, &window,
                                    txwDefWindowProc);
         txwSetWindowUShort( mbut2, TXQWS_ID, rbut2); // returncode == ID
      }

      if (rbut3 != 0)                           // do we have a button 3 ?
      {
         style = TXWS_PBUTTON;
         if (flags & TXMB_MOVEABLE)
         {
            style |= TXWS_HCHILD_MOVE;          // move when parent resizes
            style |= TXWS_VCHILD_MOVE;
         }
         txwSetupWindowData(
            framepos->bottom - 6,               // UL corner, line   (relative)
            (rbut4 != 0) ? (b1 + 29) : ww - b1 - strlen(tbut3) - 7, // column
            3,                                  // vertical size
            strlen( tbut3) + 4,                 // horizontal size   (auto)
            style,                              // window frame style
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.bu.text = tbut3;
         mbut3 = txwCreateWindow(   mframe, TXW_BUTTON, mframe, 0, &window,
                                    txwDefWindowProc);
         txwSetWindowUShort( mbut3, TXQWS_ID, rbut3); // returncode == ID
      }

      if (rbut4 != 0)                           // do we have a button 4 ?
      {
         style = TXWS_PBUTTON;
         if (flags & TXMB_MOVEABLE)
         {
            style |= TXWS_HCHILD_MOVE;          // move when parent resizes
            style |= TXWS_VCHILD_MOVE;
         }
         txwSetupWindowData(
            framepos->bottom - 6,               // UL corner, line   (relative)
            ww - b1 - strlen(tbut4) - 7,        // UL corner, column (relative)
            3,                                  // vertical size
            strlen( tbut4) + 4,                 // horizontal size   (auto)
            style | TXBS_HELP,                  // window frame style
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.bu.text = tbut4;
         mbut4 = txwCreateWindow(   mframe, TXW_BUTTON, mframe, 0, &window,
                                    txwDefWindowProc);
         txwSetWindowUShort( mbut4, TXQWS_ID, rbut4); // returncode == ID
      }

      rc = txwDlgBox( parent, owner, txwMsgBoxDlgWinProc, mframe, NULL);

      txFreeText( mText);                       // free the text memory
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwMessageBoxWidgets'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Dialog window procedure, for the message-box
/*****************************************************************************/
static ULONG txwMsgBoxDlgWinProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc    = NO_ERROR;
   TXWHANDLE           target;                  // target window handle

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)
            {
               case 'y':
               case 'Y':
                  if ((target = txwWindowFromID( hwnd, TXMBID_YES)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_YES);
                  }
                  break;

               case 'n':
               case 'N':
                  if ((target = txwWindowFromID( hwnd, TXMBID_NO)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_NO);
                  }
                  break;

               case 'c':
               case 'C':
                  if ((target = txwWindowFromID( hwnd, TXMBID_CANCEL)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_CANCEL);
                  }
                  break;

               case 'o':
               case 'O':
                  if ((target = txwWindowFromID( hwnd, TXMBID_OK)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_OK);
                  }
                  break;

               case 'a':
               case 'A':
                  if ((target = txwWindowFromID( hwnd, TXMBID_ABORT)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_ABORT);
                  }
                  break;

               case 'r':
               case 'R':
                  if ((target = txwWindowFromID( hwnd, TXMBID_RETRY)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_RETRY);
                  }
                  break;

               case 'i':
               case 'I':
                  if ((target = txwWindowFromID( hwnd, TXMBID_IGNORE)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_IGNORE);
                  }
                  break;

               case 'e':
               case 'E':
                  if ((target = txwWindowFromID( hwnd, TXMBID_ENTER)) != 0)
                  {
                     txwSetFocus( target);
                     txwDismissDlg( hwnd, TXMBID_ENTER);
                  }
                  break;

               case 'h':
               case 'H':
                  if ((target = txwWindowFromID( hwnd, TXMBID_HELP)) != 0)
                  {
                     txwSetFocus( target);
                     txwPostMsg(  target, TXWM_HELP, 0, 0);
                  }
                  break;

               case ',':                        // route all '<>' variants
               case '.':                        // to the text-view to make
               case '<':                        // it scroll left-right
               case '>':
               case TXa_COMMA :
               case TXa_DOT   :
               case TXk_UP    :
               case TXk_DOWN  :
                  if (txwIsMinimized( hwnd, FALSE)) // ignore when minimized
                  {
                     //- simply ignore the keys when minimized, do NOT send
                     //- the message to a child since that would create a
                     //- logical loop because the child will send everything
                     //- to the parent if it is minimized
                  }
                  else
                  {
                     if ((target = txwWindowFromID( hwnd, TXWD_WID_MBVIEW)) != 0)
                     {
                        txwSendMsg( target, msg, mp1, mp2);
                     }
                  }
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
}                                               // end 'txwMsgBoxDlgWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display standard prompt-box dialog with text an entryfield and help-id
/*****************************************************************************/
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
   char               *efvalue                  // INOUT entry field value
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRACES(("parent:%8.8lx  owner:%8.8lx  title:'%s'\n", parent, owner, title));
   TRACES(("helpid:%8.8lx = %lu  flags:%8.8lx\n", helpid, helpid, flags));

   if (txwIsWindow( TXHWND_DESKTOP))            // is there a desktop ?
   {
      TXRECT           position;                // reference size/position
      TXWHANDLE        pframe;                  // prompt-box frame
      TXWHANDLE        pview;                   // help view text
      TXWHANDLE        pentry;                  // entryfield
      TXWHANDLE        pbtok;                   // OK button
      TXWHANDLE        pbtcan;                  // CANCEL button
      TXWINDOW         window;                  // setup window data
      ULONG            style;
      short            wvsize;                  // widget vertical size
      short            phsize;                  // parent window width
      short            pvsize;                  // parent window height
      char           **mText;
      int              lines;                   // nr of lines
      int              ll;                      // real max line length
      int              ww = ((strlen( message) < 160) &&
                                    ( gwdata == NULL)) ? TXWD_MB_WMIN
                                                       : TXWD_MB_WDEFAULT;
      ll    = ww -4;
      mText = txString2Text( message, &ll, &lines);

      if ((wvsize = txwWidgetSize( gwdata, 1, NULL)) != 0)
      {
         lines  += wvsize;                      // add widgets and separator
      }
      txwQueryWindowRect( parent, FALSE, &position);
      phsize = position.right;
      pvsize = position.bottom;

      if (position.left  + ww    + 4 < phsize)
      {
         position.right  = ww;
         position.left   = phsize - ww -4;
         if (flags & TXPB_HCENTER)
         {
            position.left /= 2;                 // center horizontally
         }
      }
      if (position.top   + lines + 10 < pvsize)
      {
         position.bottom = lines + 10;
         position.top   += 1;
         if (flags & TXPB_VCENTER)
         {
            position.top += ((pvsize - position.bottom) * 2 / 5); // center vertically
         }
      }
      TRECTA( "pos/size", (&position));

      style = TXWS_DIALOG | TXWS_DISABLED | TXCS_CLOSE_BUTTON;
      if (flags & TXPB_MOVEABLE)
      {
         style |= TXWS_MOVEABLE;                // make frame move/sizeable
      }
      txwSetupWindowData(
         position.top,                          // upper left corner
         position.left,
         position.bottom,                       // vertical size
         position.right,                        // horizontal size
         style | TXWS_CAST_SHADOW,              // window frame style
        (helpid != 0) ? helpid : TXWD_HELPPBOX, // specific or generic help
         ' ', ' ', TXWSCHEME_COLORS,
         title, txwstd_footer,
         &window);
      window.dlgFocusID = TXWD_WID_PBENTRY;     // entryfield gets focus
      window.st.buf     = NULL;                 // NO artwork attached
      pframe = txwCreateWindow( parent, TXW_CANVAS, 0, 0, &window, NULL);

      if (wvsize != 0)                          // do we have any widgets ?
      {
         if ((rc = txwCreateWidgets( pframe, gwdata, 1, 1)) == NO_ERROR)
         {
            txwSetupWindowData( wvsize, 0, 1, ww,
                  TXWS_OUTPUT | TXWS_HCHILD_SIZE | TXWS_VCHILD_MOVE,
                  0, ' ', ' ',  TXWSCHEME_COLORS, "",  "", &window);
            window.sl.buf = txwSeparatorLine;
            txwCreateWindow( pframe, TXW_STLINE, pframe, 0, &window,
                             txwDefWindowProc);
         }
      }
      if (rc == NO_ERROR)
      {
         style = TXWS_CHILDWINDOW | TXWS_DISABLED;
         if (flags & TXPB_MOVEABLE)
         {
            style |= TXWS_HCHILD_SIZE;          // resize with parent
            style |= TXWS_VCHILD_MOVE;          // move with vertical resize
         }
         txwSetupWindowData(
            wvsize + 1,                         // UL corner, line    (relative)
            (position.right >= ll +2) ? ((position.right -ll -2) / 2) :
            1,                                  // UL corner, column  (relative)
            position.bottom - 9 - wvsize,       // vertical size (to entryfield)
            position.right  - 4,                // horizontal size
            style,                              // window frame style
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.tv.topline  = 0;
         window.tv.leftcol  = 0;
         window.tv.maxtop   = TXW_INVALID;
         window.tv.maxcol   = TXW_INVALID;
         window.tv.markLine = 0;
         window.tv.markCol  = 0;
         window.tv.markSize = 0;
         window.tv.buf      = mText;
         pview = txwCreateWindow( pframe, TXW_TEXTVIEW, pframe, 0, &window,
                                  txwDefWindowProc);
         txwSetWindowUShort( pview,  TXQWS_ID,   TXWD_WID_PBVIEW);


         style = TXWS_CHILDWINDOW | TXWS_SIDEBORDERS;
         if (flags & TXPB_MOVEABLE)
         {
            style |= TXWS_HCHILD_SIZE;          // resize with parent
            style |= TXWS_VCHILD_MOVE;          // move with vertical resize
         }
         txwSetupWindowData(
            position.bottom - 8,                // UL corner, line   (relative)
            (position.right >= eflength +2) ? ((position.right -eflength -2) / 2) :
            1,                                  // UL corner, column (relative)
            1,                                  // vertical size
            (position.right <  eflength -4) ? ((position.right -4)) :
            eflength,                           // horizontal size   (auto)
            style |                             // window frame style
            TXES_DLGE_FIELD,                    // Esc/Enter to owner Dialog
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
            window.ef.leftcol = 0;
            window.ef.maxcol  = TXW_INVALID;
            window.ef.curpos  = 0;
            window.ef.rsize   = eflength;
            window.ef.buf     = efvalue;
            window.ef.history = NULL;
         pentry = txwCreateWindow( pframe, TXW_ENTRYFIELD, pframe, 0, &window,
                                   txwDefWindowProc);
         txwSetWindowUShort( pentry, TXQWS_ID, TXWD_WID_PBENTRY);


         style = TXWS_PBUTTON;
         if (flags & TXMB_MOVEABLE)
         {
            style |= TXWS_VCHILD_MOVE;          // move when parent resizes
         }
         txwSetupWindowData(
            position.bottom - 6,                // UL corner, line   (relative)
            (ww / 4) -5,                        // column
            3, strlen( txmb_bt_ok) + 4,         // vertical/horizontal size
            style,                              // window frame style
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.bu.text = txmb_bt_ok;
         pbtok = txwCreateWindow(   pframe, TXW_BUTTON, pframe, 0, &window,
                                    txwDefWindowProc);
         txwSetWindowUShort( pbtok, TXQWS_ID, TXMBID_OK); // rc == ID


         style = TXWS_PBUTTON;
         if (flags & TXMB_MOVEABLE)
         {
            style |= TXWS_HCHILD_MOVE;          // move when parent resizes
            style |= TXWS_VCHILD_MOVE;
         }
         txwSetupWindowData(
            position.bottom - 6,                // UL corner, line   (relative)
            position.right - (ww / 4) -6,       // column
            3, strlen( txmb_bt_cancel) + 4,     // vertical/horizontal size
            style,                              // window frame style
            helpid,                             // help on the message
            ' ', ' ', TXWSCHEME_COLORS, "",  "",
            &window);
         window.bu.text = txmb_bt_cancel;
         pbtcan = txwCreateWindow(  pframe, TXW_BUTTON, pframe, 0, &window,
                                    txwDefWindowProc);
         txwSetWindowUShort( pbtcan, TXQWS_ID, TXMBID_CANCEL); // rc == ID

         rc = txwDlgBox( parent, owner, txwPromptDlgWinProc, pframe, NULL);
      }
      txFreeText( mText);                       // free the text memory
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwPromptBox'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Window procedure, for simple one entry-field dialog prompt-box
/*****************************************************************************/
static ULONG txwPromptDlgWinProc                // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc    = NO_ERROR;
   TXWINDOW           *win;
   TXWHANDLE           target;                  // target window handle

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      win = txwWindowData( hwnd);
      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)
            {
               case TXa_COMMA :
               case TXa_DOT   :
               case TXk_UP    :
               case TXk_DOWN  :
                  if ((target = txwWindowFromID( hwnd, TXWD_WID_PBVIEW)) != 0)
                  {
                     txwSendMsg( target, msg, mp1, mp2);
                  }
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
}                                               // end 'txwPromptDlgWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display standard List-box dialog for menu's or selection lists
/*****************************************************************************/
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
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRACES(("parent:%8.8lx  owner:%8.8lx  title: '%s'\n", parent, owner, title));
   TRACES(("helpid:%8.8lx = %lu  flags:%8.8lx\n", helpid, helpid, flags));
   TRECTA( "IN:  pos", (pos));

   TRACES(("List at:%8.8lx\n", list));
   TRACES(("astatus:%8.8lx  userinfo:%8.8lx\n",
            list->astatus, list->userinfo));
   TRACES(("top:%6lu  selected:%6lu  count:%6lu\n",
            list->top, list->selected, list->count));
   TRACES(("asize:%6lu   vsize:%6lu  tsize:%6lu\n",
            list->asize, list->vsize, list->tsize));

   if ((txwIsWindow( TXHWND_DESKTOP)) &&        // is there a desktop ?
       (list != NULL))                          // and do we have a list ?
   {
      TXRECT           popup;                   // prefered position
      TXWHANDLE        lframe;                  // listbox frame
      TXWHANDLE        llist;                   // listbox control
      TXWINDOW         window;                  // setup window data
      ULONG            style;
      int              ll;                      // real max line length
      TXRECT           parect;                  // parent client rectangle
      short            hsize;
      short            vsize;

      ll = TxSelGetMaxLength( list) +2;
      txwQueryWindowPos( parent, FALSE, &parect);
      if ((parect.right - parect.left -1) < ll)
      {
         ll = parect.right - parect.left -1;    // limit to parents width -2
      }
      vsize = list->vsize +2;                   // include border lines
      hsize = ll          +2;
      popup.top  = pos->top  - parect.top  +1;  // one line below ...
      popup.left = pos->left - parect.left;     // and at same column
      if ((popup.left + hsize) > parect.right)
      {
         popup.left = parect.right - parect.left - hsize +1;
         if ((flags & TXLB_P_FIX_ROW) == 0)     // not fixed vertically ?
         {
            popup.top++;                        // move left, one down (submenu)
         }
      }
      else if (popup.left < 0)
      {
         popup.left = 0;
      }
      if ((popup.top + vsize) > (parect.bottom - parect.top +1))
      {
         if (popup.top > vsize)              // align ABOVE/NEXTTO prefered
         {
            popup.top = popup.top  - vsize -1;
            if (flags & TXLB_P_LISTBOX)         // popup from other listbox
            {
               popup.top += 2;                  // align bottom with current
            }
         }
         else                                   // align to bottom, resize
         {
            vsize       = min( vsize, (TxScreenRows() - 5));
            list->vsize = vsize -2;
            popup.top   = parect.bottom - parect.top - vsize;
         }
      }
      popup.right = popup.bottom = 0;           // unused

      //- make sure the list is positioned with 'selected' visible

      TRECTA( "pos/size", (&popup));

      style = TXWS_CANVAS | TXCS_LIST_POPUP;    // empty canvas for a popup-list
      if (flags & TXLB_MOVEABLE)
      {
         style |= TXWS_MOVEABLE;                // make frame move/sizeable
      }
      if (flags & TXLB_P_LISTBOX)               // popup from other listbox
      {
         style |= TXCS_P_LISTBOX;               // it is a submenu canvas
      }
      txwSetupWindowData(
         popup.top, popup.left,                 // calculated position
         vsize, hsize,                          // vertical + horizontal size
         style | TXWS_CAST_SHADOW,              // window frame style
         helpid,                                // listbox help
         ' ', ' ',
         cscheme,     bscheme,                  // use specified color schemes
         cSchemeColor,cSchemeColor,
         cSchemeColor,cSchemeColor,
         "",  "",
         &window);
      window.dlgFocusID = TXWD_WID_LBLIST;      // listbox gets focus
      window.st.buf     = NULL;                 // NO artwork attached
      lframe = txwCreateWindow( parent, TXW_CANVAS, 0, 0, &window, NULL);


      style = TXWS_FRAMED       |               // borders, scroll-indicators
              TXLS_CHAR_ENTER   |               // give LN_ENTER on select-char
              TXWS_TAB_2_OWNER;                 // menu, pass TAB to owner
      if (flags & TXLB_MOVEABLE)
      {
         style |= TXWS_VCHILD_SIZE | TXWS_HCHILD_SIZE; // size with parent
      }
      txwSetupWindowData(
         0, 0, vsize, hsize,                    // fit the canvas ...
         style,                                 // window frame style
         helpid,                                // help on the message
         ' ', ' ',
         cscheme,     bscheme,                  // use specified color schemes
         cSchemeColor,cSchemeColor,
         cSchemeColor,cSchemeColor,
         title, footer,
         &window);
      window.lb.list = list;

      txwPositionListBox( &(window.lb));        // auto position 'selected'

      llist = txwCreateWindow( lframe, TXW_LISTBOX, lframe, 0, &window,
                               txwDefWindowProc);
      txwSetWindowUShort( llist, TXQWS_ID, TXWD_WID_LBLIST);
      txwListSort2frhText( llist);                  //- update footer indicator for sort
      txwSendMsg( owner, TXWM_ACTIVATE, TRUE,  0);  //- keep owner marked (menu)

      rc = txwDlgBox( parent, owner, txwListDlgWinProc, lframe, list);

      txwPostMsg( owner, TXWM_ACTIVATE, FALSE, 0);  //- release active mark

      list->vsize = vsize -2;                   // make sure original size is
   }                                            // set again (resize/collapse)
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwListBox'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Position listbox correctly on changed 'selected' index after sort/reverse
/*****************************************************************************/
void txwPositionListBox
(
   TXLISTBOX       *box                         // IN    listbox data structure
)
{
   TXSELIST        *list = box->list;

   if (list->selected < list->vsize)
   {
      box->cpos = list->selected;
      list->top = 0;
   }
   else if (list->selected > (list->count - list->vsize))
   {
      list->top = list->count - list->vsize;
      box->cpos = list->selected - list->top;
   }
   else
   {
      list->top = list->selected - (list->vsize / 2); // center selected line
      box->cpos = list->vsize / 2;
   }
}                                               // end 'txwPositionListBox'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Window procedure, for ListBox popup
/*****************************************************************************/
static ULONG txwListDlgWinProc                  // RET   result
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
      TXWINDOW        *win   = txwWindowData(     hwnd);
      TXWHANDLE        owner = txwQueryWindow(    hwnd, TXQW_OWNER);
      TXSELIST        *list  = txwQueryWindowPtr( hwnd, TXQWP_USER);
      TXS_ITEM        *item;

      TRCMSG( hwnd, msg, mp1, mp2);
      switch (msg)
      {
         case TXWM_CONTROL:
            if ((list != NULL) && (list->selected < list->count))
            {
               item = list->items[list->selected];

               switch (TXSH2FROMMP(mp1))
               {
                  case TXLN_SELECT:             // select, not 1st time (FOCUS)
                  case TXLN_SETFOCUS:           // select after movement
                     txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) item->desc, 0);
                     break;

                  case TXLN_ENTER:              // ENTER, return selected value
                  default:                      // or present a submenu ...
                     if (item->flags & TXSF_P_LISTBOX)
                     {
                        ULONG       mcode;
                        TXRECT      where = win->client;

                        where.top  += list->selected - list->top;
                        where.left  = where.right    - 1;

                        if (item->userdata != 0)    // submenu list present ?
                        {
                           mcode = txwListBox(
                                    TXHWND_DESKTOP,
                                    hwnd,       // current is owner of popup
                                    &where,     // popup position
                                    "", "",     // no title
                                    item->helpid, // same global help as item
                                    TXLB_MOVEABLE | // moveable
                                    TXLB_P_LISTBOX, // align to current bar
                                    win->clientclear.at, // inherit color scheme
                                    win->borderclear.at,
                                   (TXSELIST *) item->userdata);

                           if (mcode != TXDID_CANCEL) // return submenu selection
                           {                    // plus current item base value
                              txwDismissDlg( hwnd, item->value + mcode);
                           }
                           else                 // back to this parent menu
                           {
                              txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) item->desc, 0);
                           }
                        }
                        else
                        {
                           TxMessage( TRUE, 0,
                                      "Dynamic submenu for '%s'\nhas not been created yet!  "
                                      "This is a program bug.\n\nPlease report this to your "
                                      "support contact for this software", item->text);
                        }
                     }
                     else
                     {
                        txwDismissDlg( hwnd, item->value);
                     }
                     break;
               }
            }
            break;

         case TXWM_CHAR:
            switch (mp2)
            {
               case TXs_TAB:                    // passed by TXWS_TAB_2_OWNER
               case TXk_TAB:
               case TXk_LEFT:                   // close the popup
               case TXk_RIGHT:                  // and delegate to owner
                  TRACES(("Close popup and delegate to owner (menubar)\n"));
                  txwPostMsg( hwnd,  TXWM_CLOSE, 0, 0);
                  if ((mp2 != TXk_LEFT) || ((win->style & TXCS_P_LISTBOX) == 0))
                  {
                     txwPostMsg( owner, msg, mp1, mp2); //- delegate key to owner
                  }                                     //- unless LEFT on submenu
                  break;

               case TXs_F10:
               case TXk_F10:
               case TXk_MENU:
                  TRACES(("Close pulldown and signal CANCEL to owner (menubar)\n"));
                  txwPostMsg( hwnd,  TXWM_CLOSE, 0, 0);
                  txwPostMsg( owner, msg, TXDID_CANCEL, mp2);
                  break;

               case TXk_F11:                    // No (history) popup when
                  break;                        // a list is active ...

               case TXc_D:
                  if (list && (list->flags & TXSL_ITEM_DELETE))
                  {
                     txwDismissDlg( hwnd, TXc_D); // item in ->selected
                  }
                  else                          // default, to owner ...
                  {
                     rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  }
                  break;

               default:
                  if (txwIsAccelCandidate(mp2)) // close menu on possible ACCEL
                  {
                     if (txwIsMinimized( hwnd, TRUE) || // unless a parent or
                         txwIsMinimized( hwnd, FALSE) ) // this is minimized
                     {
                        rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                     }
                     else
                     {
                        TRACES(("Close pulldown AND menubar, signal OK to owner\n"));
                        txwPostMsg( hwnd,  TXWM_CLOSE, 0, 0);
                        txwPostMsg( owner, msg, TXDID_OK, TXk_F10);
                        txwSendMsg( owner, msg, mp1, mp2); // pass the accel itself

                        //- SendMsg because it must be processed BEFORE the
                        //- menu dialog has closed on F10/MENUEND processing!
                     }
                  }
                  else                          // default, likely by owner
                  {
                     rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  }
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
}                                               // end 'txwListDlgWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display menu-bar dialog with attached menu's using selection lists
/*****************************************************************************/
ULONG txwMenuBar                                // RET   selected CMD_CODE
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWHANDLE           owner,                   // IN    owner  window
   TXRECT             *pos,                     // IN    pos or NULL for parent
   ULONG               open,                    // IN    0=first, x=ident
   ULONG               helpid,                  // IN    help on menubar
   ULONG               flags,                   // IN    specification flags
   TXS_MENUBAR        *mbar                     // IN    menubar information
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRACES(("parent:%8.8lx  owner:%8.8lx\n", parent, owner));
   TRACES(("open: %8.8lx = '%c'\n", open, (char) open));
   TRACES(("helpid:%8.8lx = %lu\n", helpid, helpid));
   TRECTA( "Explicit pos", (pos));

   if (txwIsWindow( TXHWND_DESKTOP))            // is there a desktop ?
   {
      TXWHANDLE        bframe;                  // menubar frame
      TXWHANDLE        bmenu;                   // menu title handle
      TXWINDOW         window;                  // setup window data
      ULONG            style;                   // window style
      int              lwidth;                  // total menu line length
      int              mwidth;                  // single menu title length
      int              m;
      int              defmenu = TXS_BARSIZE;   // default menu, none yet
      int              line;                    // current line in menu bar
      TXRECT           parect;                  // parent client rectangle
      short            hpos  = 0;
      short            vpos  = 0;
      short            hsize;
      short            vsize = 1;               // default menubar height
      TXS_MENU        *menu;

      txwQueryWindowPos( parent, FALSE, &parect);
      TRECTA( "parentCl", (&parect));

      if (pos != NULL)                          // explicit position
      {
         hpos  = pos->top   - parect.top;       // make relative to parent
         vpos  = pos->left  - parect.left;
         hsize = pos->right - pos->left +1;     // span target window width
      }
      else
      {
         hsize = parect.right - parect.left +1; // span parent window width
      }

      for (m = 0, lwidth = 1; m < mbar->count; m++)
      {
         if ((menu = mbar->menu[m]) != NULL)
         {
            if (menu->ident == open)            // menu to open
            {
               defmenu = m;
            }
            mwidth = strlen(menu->text);
            if (((lwidth + mwidth +10) > hsize) && (m < (mbar->count -1)))
            {
               lwidth = 1;                      // at end, and more to come
               vsize++;                         // add another menu-line
            }
            else
            {
               lwidth += (mwidth +2);
            }
         }
      }
      if (defmenu == TXS_BARSIZE)               // not found by ident ...
      {
         if (mbar->defopen < mbar->count)
         {
            defmenu = mbar->defopen;            // use specified initial or
         }                                      // remembered last opened menu
         else
         {
            defmenu = 0;                        // default to 1st
         }
      }                                         // last opened menu

      if ((parent == TXHWND_DESKTOP) &&         // if main-menu on desktop
          (parect.left  != 0       ) &&         // and desktop has border
          (txwa->sbview != NULL    )  )         // and there is a TXTS ...
      {
         hpos++;                                // move menubar one right
         hsize -= 2;                            // and make it two smaller
      }

      txwSetupWindowData(
         vpos, hpos,                            // calculated position
         vsize, hsize,                          // vertical + horizontal size
         TXWS_CANVAS    |                       // solid background
         TXWS_MOVEABLE,                         // must be movable!
         helpid,                                // listbox help
         ' ', ' ',
         cMbarTextStand, cMbarBorder_top,       // Menu-bar background
         cSchemeColor,   cSchemeColor,
         cSchemeColor,   cSchemeColor,
         "",  "",
         &window);
      window.dlgFocusID = TXWD_WID_MENU + defmenu;
      window.st.buf     = NULL;                 // NO artwork attached
      bframe = txwCreateWindow( parent, TXW_CANVAS, 0, 0, &window, NULL);
      txwSetWindowUShort( bframe, TXQWS_ID, TXWD_WID_MBAR);

      for (m = 0, line = 0, lwidth = 1; m < mbar->count; m++)
      {
         if ((menu = mbar->menu[m]) != NULL)
         {
            mwidth = strlen(menu->text);

            menu->list->userinfo = (ULONG) mbar; // provide backward link
            style  = TXWS_A_MENU;
            if ((flags & TXMN_NO_AUTODROP) == 0) // unless disabled ...
            {
               style |= TXLS_AUTO_DROP;
            }
            txwSetupWindowData( line,  lwidth -1, 1, mwidth,
               style, menu->helpid,
               ' ', ' ',
               cSchemeColor,   cMbarBorder_top, // Heading border
               cMbarHeadStand, cMbarHeadFocus,  // Menu-bar headings
               cSchemeColor,   cSchemeColor,
               menu->text, menu->desc,          // desc for SETFOOTER
               &window);
            window.lb.list = menu->list;
            bmenu = txwCreateWindow( bframe, TXW_LISTBOX, bframe, 0,
                                    &window, txwDefWindowProc);
            if ((flags & TXMN_DELAY_AUTODROP) && (m == defmenu))
            {
               txwSetWindowUShort( bmenu, TXQWS_FLAGS, TXFF_AUTODROPNEXT);
            }
            txwSetWindowUShort( bmenu, TXQWS_ID, TXWD_WID_MENU + m);

            if ((lwidth + mwidth +10) > hsize)
            {
               lwidth = 1;
               line++;                          // move to next menu-line
            }
            else
            {
               lwidth += (mwidth +2);
            }
         }
      }
      if (flags & TXMN_MAIN_MENU)
      {
         txwa->mainmenu = bframe;
      }
      rc = txwDlgBox( parent, owner, txwMenuDlgWinProc, bframe, mbar);
      if (flags & TXMN_MAIN_MENU)
      {
         txwa->mainmenu = TXHWND_NULL;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwMenuBar'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Window procedure, for a MenuBar Dialog
/*****************************************************************************/
static ULONG txwMenuDlgWinProc                  // RET   result
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
      TXWHANDLE        owner = txwQueryWindow(    hwnd, TXQW_OWNER);
      TXS_MENUBAR     *mbar  = txwQueryWindowPtr( hwnd, TXQWP_USER);

      TRCMSG( hwnd, msg, mp1, mp2);
      switch (msg)
      {
         case TXWM_MENUSELECT:                  // pass to owner WinProc
            txwPostMsg( txwQueryWindow( hwnd, TXQW_OWNER),
                        TXWM_COMMAND, mp1, TXCMDSRC_MENU);

            txwDismissDlg( hwnd, mp1);          // and as returncode to
            break;                              // the MenuBar caller

         case TXWM_MENUEND:                     // end menu, no selection
            if (txwa->reopenMenu != 0)          // automatic re-open request
            {
               txwSetFocus( txwWindowFromID( hwnd, txwa->reopenMenu));
               txwa->reopenMenu = 0;            // one time only
            }
            else                                // CANCEL or OK to caller
            {                                   // (OK will keep automenu)
               txwDismissDlg( hwnd, mp1);
            }
            break;

         case TXWM_CHAR:
            switch (mp2)
            {
               case TXk_F12:                    // don't collapse
               case TXa_F9:                     // or Minimize
                  break;

               case TXs_F10:
               case TXk_F10:                    // no accel
               case TXk_MENU:
                  rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  break;

               default:
                  if (txwIsAccelCandidate(mp2)) // close menu on possible ACCEL
                  {
                     TRACES(("Close menubar, signal OK to keep automenu\n"));
                     txwPostMsg( owner, msg, mp1, mp2); // pass the accel itself
                     txwDismissDlg( hwnd, TXDID_OK);
                  }
                  else if ((mp2 >  TXk_ESCAPE)      &&
                           (mp2 <  TXW_KEY_GROUP_1) && // possible ascii key
                           (mp2 != TXk_BACKQUOTE))
                  {
                     if (mbar != 0)             // do we have the mbar info ?
                     {                          // then search quick-select keys
                        int          m;
                        TXS_MENU    *menu;
                        TXWHANDLE    mh = 0;

                        for (m = 0; m < mbar->count; m++)
                        {
                           if ((menu = mbar->menu[m]) != NULL)
                           {
                              if (menu->ident == mp2) // menu with quick-key
                              {
                                 mh = txwWindowFromID( hwnd, TXWD_WID_MENU + m);
                                 break;
                              }
                           }
                        }
                        if (mh != 0)            // other menu-header found ?
                        {
                           txwSetFocus( mh);    // give focus, auto-drop
                        }
                        else                    // drop header menu
                        {
                           txwPostMsg( (TXWHANDLE) txwa->focus,
                                       TXWM_CHAR, 0, TXa_ENTER);
                        }
                     }
                  }
                  else
                  {
                     rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  }
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
}                                               // end 'txwMenuDlgWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Display text-view window with supplied text on top of master client area
/*****************************************************************************/
ULONG txwViewText
(
   TXWHANDLE           master,                  // IN    master window
   ULONG               helpid,                  // IN    help on this text
   ULONG               topline,                 // IN    first line to display
   char               *title,                   // IN    window title
   char              **text                     // IN    text to view
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRACES(("master:%8.8lx  helpid:%8.8lx = %lu\n", master, helpid, helpid));

   if ((txwIsWindow( TXHWND_DESKTOP)) &&        // is there a desktop ?
       (txwIsWindow( master))          )        // and is master valid ?
   {
      TXRECT           position;                // reference size/position
      TXWHANDLE        tframe;                  // view frame
      TXWHANDLE        tview;                   // view window
      TXWINDOW         window;                  // frame window data

      txwQueryWindowRect( TXHWND_DESKTOP, FALSE, &position);
      TRECTA( "pos ", (&position));

      txwSetupWindowData(
         position.top, position.left, position.bottom, position.right,
         TXWS_CANVAS | TXWS_MOVEABLE |
         TXCS_CLOSE_BUTTON,                     // empty canvas backgound
         TXWD_HELPVIEW,                         // text viewer help
         ' ', ' ', TXWSCHEME_COLORS, "",  "",
         &window);
      window.dlgFocusID  = TXWD_WID_VIEW;       // focus to text viewer
      tframe = txwCreateWindow( TXHWND_DESKTOP, TXW_FRAME, 0, 0, &window, NULL);

      txwSetupWindowData(
         0,  0, position.bottom, position.right,
         TXWS_FRAMED       |                    // borders (scroll indicator)
         TXCS_CLOSE_BUTTON |                    // include close button [X]
         TXWS_HCHILD_SIZE  |                    // resize with parent
         TXWS_VCHILD_SIZE,
         (helpid) ? helpid : TXWD_HELPVIEW,     // specific or textview help
         ' ', ' ', TXWSCHEME_COLORS,
         title, txwstd_footer,
         &window);
      window.tv.topline  = topline;
      window.tv.leftcol  = 0;
      window.tv.maxtop   = TXW_INVALID;
      window.tv.maxcol   = TXW_INVALID;
      window.tv.markLine = 0;
      window.tv.markCol  = 0;
      window.tv.markSize = 0;
      window.tv.buf      = text;
      tview = txwCreateWindow( tframe, TXW_TEXTVIEW, tframe, 0, &window,
                               txwDefWindowProc);
      txwSetWindowUShort( tview, TXQWS_ID, TXWD_WID_VIEW);

      rc = txwDlgBox( TXHWND_DESKTOP, 0, txwDefDlgProc, tframe, NULL);
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'txwViewText'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Present a file-open dialog with default behaviour
/*****************************************************************************/
BOOL txwOpenFileDialog
(
   char               *fName,                   // IN    filespec wildc or NULL
   char               *fPath,                   // IN    drive + path   or NULL
   char               *fDefault,                // IN    initial fname  or NULL
   ULONG               helpid,                  // IN    specific help
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *title,                   // IN    dialog title,  or NULL
   char               *fullName                 // OUT   full path & filename
)
{
   BOOL                rc = FALSE;              // function return
   TXWFILEDLG         *fd;

   ENTER();
   TRACES(( "fName:'%s'  fPath:'%s'  fDefault:'%s'  title:'%s'\n", fName, fPath, fDefault, title));

   if (fullName)
   {
      if ((fd = TxAlloc(1, sizeof(TXWFILEDLG))) != NULL)
      {
         fd->basehelp = TXWD_HELPFOPEN;
         if (fDefault)
         {
            strcpy( fd->fDefault, TxGetBaseName(fDefault));
            strcpy( fd->fPath,    fDefault);

            if ((TxStripBaseName( fd->fPath)) == NULL) // no path component ?
            {
               strcpy( fd->fPath, "");
            }
         }
         if (strlen(fd->fPath) == 0)
         {
            if (fPath)
            {
               strcpy( fd->fPath, fPath);
               if ((strlen(fPath) == 2) && (fPath[1]) == ':')
               {
                  strcat( fd->fPath, FS_PATH_STR); // explicit root for drive spec
               }
            }
            else                                // use last-used ...
            {
               strcpy( fd->fPath, txwOpenFilePath);
            }
         }
         if (fName)
         {
            strcpy( fd->fName, fName);
         }
         fd->helpid = helpid;
         fd->gwdata = gwdata;
         fd->title  = (title) ? title : "File Open";
         fd->flags  = TXFD_MOVEABLE | TXFD_OPEN_DIALOG;

         rc = txwStdFileDialog( TXHWND_DESKTOP, fd);
         if (rc)
         {
            if ((fd->fName[0] != FS_PATH_SEP) && // not absolute from root
                (fd->fName[1] != ':'        )  ) // or with a drive component
            {
               strcpy( fullName, fd->fPath);
            }
            else
            {
               strcpy( fullName, "");
            }
            strcat( fullName, fd->fName);

            strcpy( txwOpenFilePath, fd->fPath);
         }
         TxFreeMem( fd);
      }
   }
   BRETURN (rc);
}                                               // end 'txwOpenFileDialog'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Present a file-save-as dialog with default behaviour
/*****************************************************************************/
BOOL txwSaveAsFileDialog
(
   char               *fName,                   // IN    filespec wildc or NULL
   char               *fPath,                   // IN    drive + path   or NULL
   char               *fDefault,                // IN    initial fname  or NULL
   ULONG               helpid,                  // IN    specific help
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *title,                   // IN    dialog title,  or NULL
   char               *fullName                 // OUT   full path&filename
)
{
   BOOL                rc = FALSE;              // function return
   TXWFILEDLG         *fd;

   ENTER();
   TRACES(( "fName:'%s'  fPath:'%s'  fDefault:'%s'  title:'%s'\n", fName, fPath, fDefault, title));

   if (fullName)
   {
      if ((fd = TxAlloc(1, sizeof(TXWFILEDLG))) != NULL)
      {
         fd->basehelp = TXWD_HELPFSAVE;
         if (fDefault)
         {
            strcpy( fd->fDefault, TxGetBaseName(fDefault));
            strcpy( fd->fPath,    fDefault);

            if ((TxStripBaseName( fd->fPath)) == NULL) // no path component ?
            {
               strcpy( fd->fPath, "");
            }
         }
         if (strlen(fd->fPath) == 0)
         {
            if (fPath)
            {
               strcpy( fd->fPath, fPath);
               if ((strlen(fPath) == 2) && (fPath[1]) == ':')
               {
                  strcat( fd->fPath, FS_PATH_STR); // explicit root for drive spec
               }
            }
            else                                // use last-used ...
            {
               strcpy( fd->fPath, txwSaveAsPath);
            }
         }
         if (fName)
         {
            strcpy( fd->fName, fName);
         }
         fd->helpid = helpid;
         fd->gwdata = gwdata;
         fd->title  = (title) ? title : "File Save as";
         fd->flags  = TXFD_MOVEABLE | TXFD_SAVEAS_DIALOG | TXFD_CREATE_NEW_DIR;

         //- to be refined, add custom WinProc testing for RO file on FD_VALIDATE

         rc = txwStdFileDialog( TXHWND_DESKTOP, fd);
         if (rc)
         {
            if ((fd->fName[0] != FS_PATH_SEP) && // not absolute from root
                (fd->fName[1] != ':'        )  ) // or with a drive component
            {
               strcpy( fullName, fd->fPath);
            }
            else
            {
               strcpy( fullName, "");
            }
            strcat( fullName, fd->fName);

            strcpy( txwSaveAsPath, fd->fPath);
         }
         TxFreeMem( fd);
      }
   }
   BRETURN (rc);
}                                               // end 'txwSaveAsFileDialog'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Present a select-directory dialog with default behaviour
/*****************************************************************************/
BOOL txwSelDirFileDialog
(
   char               *fPath,                   // IN    drive + path   or NULL
   ULONG               helpid,                  // IN    specific help
   TXGW_DATA          *gwdata,                  // INOUT generic widget data
   char               *title,                   // IN    dialog title,  or NULL
   char               *fullPath                 // OUT   full path
)
{
   BOOL                rc = FALSE;              // function return
   TXWFILEDLG         *fd;

   ENTER();

   if (fullPath)
   {
      if ((fd = TxAlloc(1, sizeof(TXWFILEDLG))) != NULL)
      {
         fd->basehelp = TXWD_HELPFDIRS;
         if (fPath)
         {
            strcpy( fd->fPath, fPath);
            if ((strlen(fPath) == 2) && (fPath[1]) == ':')
            {
               strcat( fd->fPath, FS_PATH_STR);  // explicit root for drive spec
            }
         }
         else                                   // use last-used ...
         {
            strcpy( fd->fPath, txwSelDirFilePath);
         }
         fd->helpid = helpid;
         fd->gwdata = gwdata;
         fd->title  = (title) ? title : "Select destination Directory";
         fd->flags  = TXFD_MOVEABLE | TXFD_SELECT_DIRECT | TXFD_CREATE_NEW_DIR;

         rc = txwStdFileDialog( TXHWND_DESKTOP, fd);
         if (rc)
         {
            strcpy( fullPath, fd->fPath);

            strcpy( txwSelDirFilePath, fd->fPath);
         }
         TxFreeMem( fd);
      }
   }
   BRETURN (rc);
}                                               // end 'txwSelDirFileDialog'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Present standard file dialog as specified
/*****************************************************************************/
BOOL txwStdFileDialog
(
   TXWHANDLE           parent,                  // IN    parent window
   TXWFILEDLG         *fd                       // IN    file dialog data
)
{
   BOOL                rc = FALSE;              // function return

   ENTER();
   TRACES(( "parent:%8.8lx flags:%8.8lx title:'%s'\n", parent, fd->flags, fd->title));
   TRACES(( "reserved:%8.8lx winProc:%8.8lx fname:'%s' fpath:'%s'\n",
             fd->reserved, fd->winproc, fd->fName, fd->fPath));

   if (txwIsWindow( TXHWND_DESKTOP))            // is there a desktop ?
   {
      TXRECT           position;                // reference size/position
      TXWHANDLE        fframe;                  // file-dialog frame
      TXWHANDLE        fentry;                  // entryfield
      TXWHANDLE        fbut1;                   // button 1 Make Dir
      TXWHANDLE        fbtok;                   // button 2 OK
      TXWHANDLE        fbtcan;                  // button 3 Cancel
      TXWHANDLE        fpath;                   // path output field
      TXWHANDLE        fdirec;                  // directory list
      TXWHANDLE        ffname;                  // filename list
      TXWINDOW         window;                  // setup window data
      ULONG            style;
      short            wvsize;                  // widget vertical size
      short            phsize;                  // parent window width
      short            pvsize;                  // parent window height
      short            etline;                  // empty lines at top
      short            ww;                      // dialog window width
      short            w3;                      // 3 col width (name / drive-sel)
      short            w2l;                     // 2 col left  width  (dir  list)
      short            w2r;                     // 2 col right width  (file list)
      TXWFDATA         fdpriv;                  // private filedialog data
      TXTT             sort_dirs;               // directory sorting text
      TXTT             sort_files;              // files     sorting text
      TXTT             foot_count_dirs;         // #items footer dirs
      TXTT             foot_count_files;        // #items footer files

#ifndef UNIX
      TXWHANDLE        fspinv;                  // spin-value volume list
      TXTM             s0;                      // status message/drive string
      int              drives;
#endif

      memset( &fdpriv, 0, sizeof(TXWFDATA));    // start empty
      fd->reserved = &fdpriv;                   // make available to DlgProc

#ifndef UNIX
      sprintf( s0, "Updating volume information for file-dialog ...");
      txwSendMsg( (TXWHANDLE) txwa->sbview, TXWM_STATUS, (ULONG) s0, cSchemeColor);
      drives = TxFsVolumes( TXFSV_ALL, s0);
      if ((drives != txfd_drive_cnt) ||         // only when #drives changed
          (!TxaExeSwitchUnSet('V')))            // or no -V- switch
      {
         txSelDestroy( &txfd_drivelist);        // free existing drives list

         txfd_drive_cnt = drives;
         txfd_drivelist = TxFsDriveSelist( TXFSV_ALL, fd->flags & TXFD_RUNTIME_FLOPPY);
      }
      txwSendMsg( (TXWHANDLE) txwa->sbview, TXWM_STATUS, (ULONG) "", cSchemeColor);

      if (txfd_drivelist != NULL)
      {
#endif
         if (TxStrWcnt(fd->fName) == strlen(fd->fName))
         {
            strcpy( fdpriv.wildcard, FS_WILDCARD); // all files
         }
         else
         {
            strcpy( fdpriv.wildcard, fd->fName); // keep original wildcard
         }

         txwQueryWindowPos( parent, FALSE, &position);
         phsize = position.right  - position.left;
         pvsize = position.bottom;

         etline = 0;
         wvsize = txwWidgetSize( fd->gwdata, etline, NULL);
         if ((wvsize != 0) && ((pvsize - wvsize) > 20))
         {
            etline++;                           // empty line at top, readability
            wvsize++;
         }
         ww  = min( TXWD_FD_WWIDTH,  (phsize - 3));
         w3  = (ww - ((fd->flags & TXFD_CREATE_NEW_DIR) ? 16 : 4)) / 2;
         w2l = (ww * 2)  / 5;
         w2r =  ww - w2l - 2;

         position.left   = (phsize - ww) /2;    // always center the dialog
         position.right  = ww;
         position.bottom = pvsize - position.top +1;
         if (pvsize >= 30)
         {
            position.top    -= 1;               // overlay parent window title line
            position.bottom -= 3;               // but keep bottom clear ...
         }

         TRECTA( "pos/size", (&position));

         style = TXWS_DIALOG | TXWS_DISABLED | TXCS_CLOSE_BUTTON;
         if (fd->flags & TXFD_MOVEABLE)
         {
            style |= TXWS_MOVEABLE;             // make frame move/sizeable
         }
         txwSetupWindowData(
            position.top, position.left, position.bottom, position.right, // horizontal size
            style | TXWS_CAST_SHADOW,           // window frame style
           (fd->helpid != 0) ? fd->helpid : fd->basehelp,
            ' ', ' ', TXWSCHEME_COLORS,
            fd->title, (fd->flags & TXFD_MOVEABLE) ? txwfld_footer : "",
            &window);
         window.st.buf      = NULL;             // NO artwork attached
         if      (fd->flags & TXFD_OPEN_DIALOG)
         {
            window.dlgFocusID = TXFD_WID_FNAMES;
         }
         else if (fd->flags & TXFD_SELECT_DIRECT)
         {
            window.dlgFocusID = TXFD_WID_DIRECT;
         }
         else                                   // save-as dialog, entryfield
         {
            window.dlgFocusID = TXFD_WID_SFNAME;
         }
         fframe = txwCreateWindow( parent, TXW_CANVAS, 0, 0, &window, NULL);
         txwSetWindowUShort( fframe, TXQWS_ID, TXFD_WID_DFRAME);
         TRACES(("Attached filedata structure at %8.8lx to FD-frame\n", fd));

         if (wvsize != 0)                       // do we have any widgets ?
         {
            if ((rc = txwCreateWidgets( fframe, fd->gwdata, etline, 1)) == NO_ERROR)
            {
               txwSetupWindowData( wvsize, 0, 1, ww,
                     TXWS_OUTPUT | TXWS_HCHILD_SIZE,
                     0, ' ', ' ',  TXWSCHEME_COLORS, "",  "", &window);
               window.sl.buf = txwSeparatorLine;
               txwCreateWindow( fframe, TXW_STLINE, fframe, 0, &window, NULL);
            }
         }
         if (rc == NO_ERROR)
         {
            style = TXWS_ENTRYBT;               // entryfield with border/title
            if (fd->flags & TXFD_MOVEABLE)
            {
               style |= TXWS_HCHILD2SIZE;       // 1/2 size with hor resize
            }
            txwSetupWindowData(
               wvsize +1, 0, 2,                 // pos and vert size
#if defined (UNIX)
               w3 + w3 +1,
#else                                           // horizontal size
               w3,
#endif
               style,                           // window style
               fd->basehelp,                    // help id
               ' ', ' ', TXWSCHEME_COLORS,
               "Filename or wildcard", "",
               &window);
            window.ef.leftcol = 0;
            window.ef.maxcol  = TXW_INVALID;
            window.ef.curpos  = 0;
            window.ef.rsize   = TXMAXLN;
            window.ef.buf     = fd->fName;
            window.ef.history = NULL;
            fentry = txwCreateWindow(   fframe, TXW_ENTRYFIELD, fframe, 0, &window,
                                        txwSfdControlWinProc);
            txwSetWindowUShort( fentry, TXQWS_ID, TXFD_WID_SFNAME);

#ifndef UNIX
            style = TXWS_DSPINBT | TXLS_DROP_ENTER | TXWS_LEFTJUSTIFY;
            if (fd->flags & TXFD_MOVEABLE)
            {
               style |= TXWS_HCHILD2MOVE | TXWS_HCHILD2SIZE;
            }
            txwSetupWindowData( wvsize +1, w3 +1, 2, w3 +1,
               style,
               fd->basehelp,
               ' ', ' ', TXWSCHEME_COLORS,
               "   Drive selection   ", "",
               &window);
            window.lb.list = txfd_drivelist;
            window.lb.cpos = txfd_drivelist->selected - txfd_drivelist->top;
            fspinv = txwCreateWindow(  fframe, TXW_LISTBOX, fframe, 0, &window,
                                       txwSfdControlWinProc);
            txwSetWindowUShort( fspinv, TXQWS_ID, TXFD_WID_VOLUME);
#endif

            if (fd->flags & TXFD_CREATE_NEW_DIR) // Create Dir button ?
            {
               style = TXWS_SBUTTON | TXBS_DLGE_BUTTON;
               if (fd->flags & TXFD_MOVEABLE)
               {
                  style |= TXWS_HCHILD_MOVE;    // move with horizontal resize
               }
               txwSetupWindowData(
                  wvsize +2, ww - 13, 1, 10,    // position and size
                  style,                        // window frame style
                  fd->basehelp,
                  ' ', ' ', TXWSCHEME_COLORS, "",  "",
                  &window);
               window.bu.text = "Make Dir";
               fbut1 = txwCreateWindow(   fframe, TXW_BUTTON, fframe, 0, &window,
                                          txwSfdControlWinProc);
               txwSetWindowUShort( fbut1, TXQWS_ID, TXFD_WID_CREATE);
            }

            style = TXWS_SBUTTON;
            if (fd->flags & TXFD_MOVEABLE)
            {
               style |= TXWS_HCHILD_MOVE;       // move when parent resizes
            }
            txwSetupWindowData(
               wvsize +4,                       // UL corner, line   (relative)
               ww - strlen(txmb_bt_ok) -
                    strlen(txmb_bt_cancel) -13, // column
               1, strlen( txmb_bt_ok) + 4,      // vertical/horizontal size
               style,                           // window frame style
               fd->basehelp,                    // help on the message
               ' ', ' ', TXWSCHEME_COLORS, "",  "",
               &window);
            window.bu.text = txmb_bt_ok;
            fbtok = txwCreateWindow(   fframe, TXW_BUTTON, fframe, 0, &window,
                                       txwDefWindowProc);
            txwSetWindowUShort( fbtok, TXQWS_ID, TXFD_WID_ACCEPT); // rc == ID

            style = TXWS_SBUTTON;
            if (fd->flags & TXFD_MOVEABLE)
            {
               style |= TXWS_HCHILD_MOVE;       // move when parent resizes
            }
            txwSetupWindowData(
               wvsize +4,                       // UL corner, line   (relative)
               ww - strlen(txmb_bt_cancel) -7,  // column
               1, strlen( txmb_bt_cancel) + 4,  // vertical/horizontal size
               style,                           // window frame style
               fd->basehelp,                    // help on the message
               ' ', ' ', TXWSCHEME_COLORS, "",  "",
               &window);
            window.bu.text = txmb_bt_cancel;
            fbtcan = txwCreateWindow(  fframe, TXW_BUTTON, fframe, 0, &window,
                                       txwDefWindowProc);
            txwSetWindowUShort( fbtcan, TXQWS_ID, TXMBID_CANCEL); // rc == ID


            //- Current PATH output field
            style = TXWS_OUTPUT;
            if (fd->flags & TXFD_MOVEABLE)
            {
               style |= TXWS_HCHILD_SIZE;       // size with horizontal resize
            }
            txwSetupWindowData(
               wvsize +4, 2, 1, ww -25,         // UL corner + vert/hor size
               style,                           // style, output with title
               fd->basehelp,                    // help
               ' ', ' ', TXWSCHEME_COLORS, "",  "",
               &window);
            window.sl.buf     = fd->fPath;
            fpath = txwCreateWindow(   fframe, TXW_STLINE, fframe, 0, &window,
                                       txwSfdControlWinProc);
            txwSetWindowUShort( fpath, TXQWS_ID, TXFD_WID_CFPATH);

            style = TXWS_LEFTJUSTIFY | TXWS_FRAMED |
                    TXLS_FOOT_COUNT  | TXWS_VCHILD_SIZE;
            if (fd->flags & TXFD_SELECT_DIRECT)
            {
               style |= TXWS_HCHILD_SIZE;
            }
            else                                // file-area too, resize 1/2
            {
               style |= TXWS_HCHILD2SIZE;
            }
            txwSetupWindowData(
               wvsize +5, 0, position.bottom - 7 - wvsize,
              (fd->flags & TXFD_SELECT_DIRECT) ? ww -2 : w2l,
               style, fd->basehelp,
               ' ', ' ',
               cFileTextStand,  cFileBorder_top, // client and border
               cSchemeColor,    cSchemeColor,
               cFileCountStand, cFileCountFocus, // Footer count value
               " Directories ", foot_count_dirs,
               &window);
            window.lb.list = NULL;              // list not available yet
            window.lb.cpos = 0;
            window.frhtext = sort_dirs;
            strcpy( sort_dirs, "");
            strcpy( foot_count_dirs, "");
            fdirec = txwCreateWindow( fframe, TXW_LISTBOX, fframe, 0, &window,
                                      txwSfdControlWinProc); // window procedure
            txwSetWindowUShort( fdirec, TXQWS_ID,   TXFD_WID_DIRECT);

            if ((fd->flags & TXFD_SELECT_DIRECT) == 0) // use file list ?
            {
               style = TXWS_LEFTJUSTIFY | TXWS_FRAMED      | TXLS_FOOT_COUNT |
                       TXWS_HCHILD2SIZE | TXWS_HCHILD2MOVE | TXWS_VCHILD_SIZE;
               strcpy( foot_count_files, "");
               txwSetupWindowData(
                  wvsize +5, w2l, position.bottom - 7 - wvsize,  w2r,
                  style, fd->basehelp,
                  ' ', ' ',
                  cFileTextStand,  cFileBorder_top, // client and border
                  cSchemeColor,    cSchemeColor,
                  cFileCountStand, cFileCountFocus, // Footer count value
                  " Files ", foot_count_files,
                  &window);
               window.lb.list = NULL;           // list not available yet
               window.lb.cpos = 0;
               window.frhtext = sort_files;
               strcpy( sort_files, "");
               strcpy( foot_count_files, "");
               ffname = txwCreateWindow( fframe, TXW_LISTBOX, fframe, 0, &window,
                                         txwSfdControlWinProc); // window procedure
               txwSetWindowUShort( ffname, TXQWS_ID,   TXFD_WID_FNAMES);
            }
            fd->result = txwDlgBox( parent, parent, txwDefFileDlgProc, fframe, fd);
         }
         txSelDestroy( &fdpriv.direct);         // free directory list
         if (fd->flags & TXFD_MULTIPLESEL)
         {
            fd->list = fdpriv.fnames;           // return list of filenames
         }
         else
         {
            txSelDestroy( &fdpriv.fnames);      // free volumes list
         }
         if ((fd->result != TXDID_CANCEL) &&
             (strncmp( fd->fPath, txwNotReadyMsg, 10) != 0))
         {
            rc = TRUE;
         }
#ifndef UNIX
      }
#endif
   }
   TRACES(( "result:%8.8lx  list:%8.8lx fname:'%s' fpath:'%s\n",
                  fd->result, fd->list, fd->fName, fd->fPath));
   BRETURN (rc);
}                                               // end 'txwStdFileDialog'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Window procedure, for some FileDialog controls
/*****************************************************************************/
static ULONG txwSfdControlWinProc               // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc = NO_ERROR;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);

   if (hwnd != 0)
   {
      TXWHANDLE     owner  = txwQueryWindow(       hwnd, TXQW_OWNER);
      USHORT        wid    = txwQueryWindowUShort( hwnd, TXQWS_ID);
      USHORT        target = 0;

      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)
            {
               case TXk_LEFT:                   // additional control change
                  switch (wid)
                  {
                     case TXFD_WID_VOLUME: target = TXFD_WID_SFNAME; break;
                     case TXFD_WID_DIRECT:
                        if (txwWindowFromID( owner, TXFD_WID_CREATE))
                        {
                                           target = TXFD_WID_CREATE; break;
                        }
                        else
                        {
                                           target = TXFD_WID_VOLUME; break;
                        }
                     case TXFD_WID_FNAMES: target = TXFD_WID_DIRECT; break;
                     case TXFD_WID_ACCEPT: target = TXFD_WID_DIRECT; break;
                     default:              target = 0;               break;
                  }
                  if (target != 0)
                  {
                     txwSetFocus( txwWindowFromID( owner, target));
                  }
                  else
                  {
                     rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  }
                  break;

               case TXk_RIGHT:                  // additional control change
                  switch (wid)
                  {
                     case TXFD_WID_VOLUME:
                        if (txwWindowFromID( owner, TXFD_WID_CREATE))
                        {
                                           target = TXFD_WID_CREATE; break;
                        }
                        else
                        {
                                           target = TXFD_WID_DIRECT; break;
                        }
                     case TXFD_WID_DIRECT:
                        if (txwWindowFromID( owner, TXFD_WID_FNAMES))
                        {
                                           target = TXFD_WID_FNAMES; break;
                        }
                        else
                        {
                                           target = TXFD_WID_ACCEPT; break;
                        }
                     case TXFD_WID_FNAMES: target = TXFD_WID_SFNAME; break;
                     case TXFD_WID_ACCEPT: target = TXFD_WID_SFNAME; break;
                     default:              target = 0;               break;
                  }
                  if (target != 0)
                  {
                     txwSetFocus( txwWindowFromID( owner, target));
                  }
                  else
                  {
                     rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  }
                  break;

               case TXk_F2:
                  switch (wid)
                  {
                     case TXFD_WID_SFNAME:      // retrieve wildcard value
                        txwPostMsg( owner, TXWM_FD_WILDCARD, 0, 0);
                        break;

                     default:
                        rc = txwDefWindowProc( hwnd, msg, mp1, mp2);
                        break;
                  }
                  break;

               default:
                  rc = txwDefWindowProc( hwnd, msg, mp1, mp2);
                  break;
            }
            break;

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
}                                               // end 'txwSfdControlWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Window procedure, for the Standard File Dialog
/*****************************************************************************/
ULONG txwDefFileDlgProc                         // RET   result
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
      TXWFILEDLG      *fd    = txwQueryWindowPtr( hwnd, TXQWP_USER);
      TXWFDATA        *fdata = (TXWFDATA *) fd->reserved;
      TXSELIST        *list  = NULL;
      TXS_ITEM        *item  = NULL;
      TXLN             spec;
      TXWHANDLE        lhwnd;
      TXWINDOW        *lw;
      char            *incl[ TXWD_FD_MAXWILD];
      char           **filter = NULL;

      TRCMSG( hwnd, msg, mp1, mp2);
      switch (msg)
      {
         case TXWM_INITDLG:
            rc = txwDefDlgProc( hwnd, msg, mp1, mp2); // set-focus & activate
            txwPostMsg( hwnd, TXWM_FD_NEWPATH, 0, 0); // then our custom stuff
            break;

         case TXWM_FD_POPULATED:                // dir/file lists populated
            if (strlen(fd->fDefault) > 0)
            {
               strcpy( fd->fName, fd->fDefault);
               strcpy( fd->fDefault, "");       // just once ...
               lhwnd = txwWindowFromID( hwnd, TXFD_WID_SFNAME);
               txwInvalidateWindow( lhwnd, TRUE, TRUE);
               txwSetFocus( lhwnd);
            }
            break;

         case TXWM_FD_NEWPATH:                  // new path, update others
            TRACES(("fd->fPath now: '%s'\n", fd->fPath));
            txSelDestroy( &fdata->direct);      // free existing list
            if ((strstr(        fd->fPath, TXFS_NOTREADY)   == NULL)  &&
                (TxTrueNameDir( fd->fPath, TRUE, fd->fPath) == NO_ERROR))
            {
               TRACES(("Build directories list using '%s'\n", fd->fPath));
               fdata->direct = TxFileDirSelist( fd->fPath, "DP", "", NULL,
                                                TXFDS_ADD_DESCRIPTION);
               list = fdata->direct;
               if ((list == NULL) || (list->count == 0)) // empty list
               {                                // not a root directory
                  if (strlen(fd->fPath) > 3)    // may happen on FreeDOS :-)
                  {
                     sprintf( spec, "Error reading directory: %s", fd->fPath);
                     txwSetSbviewStatus( spec, cSchemeColor);
                  }
               }
#ifndef UNIX
               TxSelCharSelect( txfd_drivelist, fd->fPath[0]);
               lhwnd = txwWindowFromID( hwnd, TXFD_WID_VOLUME);
               txwInvalidateWindow(    lhwnd, TRUE, TRUE);
#endif
            }
            else                                // invalid drive/path
            {
               strcpy( fd->fPath, txwNotReadyMsg);
            }
            lhwnd = txwWindowFromID( hwnd, TXFD_WID_CFPATH);
            txwInvalidateWindow(    lhwnd, TRUE, TRUE);

            lhwnd   = txwWindowFromID( hwnd, TXFD_WID_DIRECT);
            if ((lw = txwWindowData(  lhwnd)) != NULL)
            {
               if ((list = fdata->direct) != NULL)
               {
                  list->vsize = lw->client.bottom - lw->client.top +1;
                  TRACES(("list vsize: %hu\n", list->vsize));
               }
               lw->lb.list = list;
               lw->lb.cpos = 0;
            }
            txwListSort2frhText( lhwnd);        // update footer indicator
            txwInvalidateWindow( lhwnd, TRUE, TRUE);
            txwPostMsg( hwnd, TXWM_FD_NEWSPEC, 0, 0);
            break;

         case TXWM_FD_WILDCARD:                 // reset to original wildcard
            strcpy(fd->fName, fdata->wildcard);
            lhwnd = txwWindowFromID( hwnd, TXFD_WID_SFNAME);
            txwInvalidateWindow( lhwnd, TRUE, TRUE);
            txwPostMsg( hwnd, TXWM_FD_NEWSPEC, 0, 0);
            break;

         case TXWM_FD_NEWSPEC:                  // new dir or wildcard
            TRACES(("fd->fPath :'%s', fd->fName :'%s', wildcard:'%s'\n",
                     fd->fPath, fd->fName, fdata->wildcard));
            if (TxStrWcnt(fd->fName) == strlen(fd->fName))
            {
               strcpy( spec, fd->fPath);        // use existing path
               strcat( spec, fdata->wildcard);  // but show desired files
            }
            else                                // fname contains wildcards
            {
               if ((fd->fName[1] != ':') && (fd->fName[0] != FS_PATH_SEP))
               {
                  strcpy( fdata->wildcard, fd->fName); // keep arround

                  strcpy( spec, fd->fPath);     // use existing path
               }
               else                             // includes a path
               {
                  strcpy( spec, "");
               }
               filter = txWildcard2Filter( spec, fd->fName,
                                           incl, TXWD_FD_MAXWILD);
            }
            txSelDestroy( &fdata->fnames);      // free existing list

            if ((spec[0] == '.'        ) ||     // valid path ?
                (spec[0] == FS_PATH_SEP) ||
                (spec[2] == FS_PATH_SEP))
            {
               TRACES(("Build filename list using '%s'\n", spec));
               fdata->fnames = TxFileDirSelist( spec, "FI", "", filter,
                                                TXFDS_ADD_DESCRIPTION);
            }
            else
            {
               TRACES(("Invalid path '%s', no new filelist!\n"));

               //- to be refined, revert to "last known good" ??
            }
            filter  = txFreeFilter( filter, TXWD_FD_MAXWILD); // free memory

            lhwnd   = txwWindowFromID( hwnd, TXFD_WID_FNAMES);
            if ((lw = txwWindowData(  lhwnd)) != NULL)
            {
               if ((list = fdata->fnames) != NULL)
               {
                  list->vsize = lw->client.bottom - lw->client.top +1;
                  TRACES(("list vsize: %hu\n", list->vsize));
               }
               lw->lb.list = list;
               lw->lb.cpos = 0;
            }
            txwListSort2frhText( lhwnd);        // update footer indicator
            txwInvalidateWindow( lhwnd, TRUE, TRUE);
            txwPostMsg( hwnd, TXWM_FD_POPULATED, 0, 0); // signal lists done
            break;

         case TXWM_COMMAND:
            switch (mp2)                        // command source
            {
               case TXCMDSRC_PUSHBUTTON:
                  if (mp1 == TXFD_WID_CREATE)   // create directory
                  {
                     TXLN dirname;
                     TXLN newpath;

                     strcpy( dirname, "");
                     strcpy( newpath, "");
                     rc = txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL,
                                        "Specify the name of the directory to "
                                        "be created (relative or absolute)",
                                        "Create new directory",
                                         fd->basehelp,
                                        TXPB_MOVEABLE | TXPB_VCENTER,
                                        TXMAXLN, dirname);
                     if (rc != TXDID_CANCEL)
                     {
                        if (strchr( dirname, ':') == NULL) // relative path
                        {
                           strcpy( newpath, fd->fPath);
                        }
                        strcat( newpath, dirname);
                        if (TxMakePath( newpath) == NO_ERROR)
                        {
                           strcpy( fd->fPath, newpath);
                           TRACES(("fd->fPath changed from CREATE: '%s'\n", fd->fPath));
                           txwPostMsg( hwnd, TXWM_FD_NEWPATH, 0, 0);
                        }
                     }
                  }
                  else if (mp1 == TXFD_WID_ACCEPT)   // select the directory
                  {
                     txwDismissDlg( hwnd, TXDID_OK); // OK to caller
                  }
                  else
                  {
                     rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  }
                  break;

               default:
                  rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                  break;
            }
            break;

         case TXWM_CONTROL:
            if ((list = (TXSELIST *) mp2) != NULL)
            {
               item = list->items[list->selected];
            }
            TRACES(("WM_CONTROL list:%8.8lx item:%8.8lx\n", list, item));
            if (item != NULL)
            {
               switch (TXSH2FROMMP(mp1))
               {
                  case TXLN_SELECT:             // select, not 1st time (FOCUS)
                     switch (TXSH1FROMMP(mp1))
                     {
                        case TXFD_WID_VOLUME:
                           if (fd->fPath[0] != item->text[0]) // really changed ?
                           {
                              if (strstr( item->text, TXFS_NOTREADY) != NULL)
                              {
                                 strcpy( fd->fPath, TXFS_NOTREADY);
                              }
                              else              // go to working-dir on drive
                              {
                                 sprintf( fd->fPath, "%c:.", item->text[0]);
                              }
                              TRACES(("fd->fPath changed from VOLUME: '%s'\n", fd->fPath));
                              txwPostMsg( hwnd, TXWM_FD_NEWPATH, 0, 0); // refresh DIR
                           }
                           break;

                        case TXFD_WID_FNAMES:
                           strcpy( fd->fName, item->text);     //- copy basename
                           if (fd->flags & TXFD_SAVEAS_DIALOG) //- for Save-as, strip
                           {                                   //- extension, avoids
                              TxStripExtension( fd->fName);    //- easy overwrites!
                           }
                           lhwnd = txwWindowFromID( hwnd, TXFD_WID_SFNAME);
                           txwInvalidateWindow(    lhwnd, TRUE, TRUE);
                           break;

                        case TXFD_WID_DIRECT:
                        default:
                           break;
                     }
                  case TXLN_SETFOCUS:
                     txwPostMsg( TXHWND_DESKTOP, TXWM_SETFOOTER, (ULONG) item->desc, 0);
                     break;

                  case TXLN_ENTER:              // ENTER on the item
                  default:
                     switch (TXSH1FROMMP(mp1))
                     {
                        case TXFD_WID_DIRECT:
                           //- to be refined, save current fPath, to be restored if
                           //- it happens to result in a "not accessible" situation
                           //- could popup a message then (not in fPath) and restore
                           //- the actual fPath to this "last known good"

                           //- Perhaps "last known good" to be set if getting
                           //- at least one directory in the list ...

                           if ((strcmp(item->text, "..") == 0) ||
                               (strlen(fd->fPath) + strlen(item->text) +3 < TXMAXLN))
                           {
                              strcat( fd->fPath, item->text);
                              TRACES(("fd->fPath changed from DIRECT: '%s'\n", fd->fPath));
                              txwPostMsg( hwnd, TXWM_FD_NEWPATH, 0, 0);
                           }
                           else                 // avoid traps on long paths
                           {
                              TxMessage( TRUE, 0,
                                         "Cannot change to selected directory!\n\n"
                                         "The total length of the path would exceed the "
                                         "system limit of %d characters.", TXMAXLN);
                           }
                           break;

                        case TXFD_WID_FNAMES:
                           if (strlen(fd->fPath) + strlen(item->text) < TXMAXLN)
                           {
                              strcpy( fd->fName, item->text);     //- copy basename
                              if (fd->flags & TXFD_SAVEAS_DIALOG) //- for Save-as, strip
                              {                                   //- extension, avoids
                                 TxStripExtension( fd->fName);    //- easy overwrites!
                              }
                              rc = txwDefDlgProc( hwnd, TXWM_CHAR, 0, TXk_ENTER);
                           }
                           else                 // avoid traps on too long
                           {                    // path+filename returned
                              TxMessage( TRUE, 0,
                                         "Cannot select the hilighted filename!\n\n"
                                         "The total length of the path plus filename would "
                                         "exceed the system limit of %d characters.", TXMAXLN);
                           }
                           break;

                        case TXFD_WID_VOLUME:
                        default:
                           break;
                     }
                     break;
               }
            }
            //- note: no DefDlgProc default processing (to be refined ?)
            break;

         case TXWM_CHAR:                        // mp1 not 0 is control-ID
            switch (mp2)
            {
               case TXk_ENTER:                  // from some control ...
                  switch (mp1)                  // select on Window-ID
                  {
                     case TXFD_WID_SFNAME:      // filename field
                        if (TxStrWcnt(fd->fName) == strlen(fd->fName))
                        {
                           if (TxFileExists( fd->fName) ||     //- fully qualified
                              ((fd->fName[1] != ':') &&        //- drive component
                               (fd->fName[0] != '.') &&        //- relative path
                               (fd->fName[0] != FS_PATH_SEP))) //- absolute from root
                           {
                              if ((fd->fName[0] != FS_PATH_SEP) && // not absolute from root
                                  (fd->fName[1] != ':'        )  ) // or with a drive component
                              {
                                 strcpy( fd->fResult, fd->fPath);
                              }
                              else
                              {
                                 strcpy( fd->fResult, "");
                              }
                              strcat( fd->fResult, fd->fName);
                              if (strlen(fd->fResult) +4 < TXMAXLN) // allow 3 char extension
                              {
                                 if ((fd->flags & TXFD_OPEN_DIALOG)      &&
                                     (TxFileExists( fd->fResult) == FALSE))
                                 {
                                    TxMessage( TRUE, 0, "%s\n\nFile does not exist!", fd->fResult);
                                 }
                                 else
                                 {
                                    rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                                 }
                              }
                              else              // path+filename[+ext] too long
                              {
                                 TxMessage( TRUE, 0,
                                            "Cannot accept the specified filename!\n\n"
                                            "The total length of the path plus filename would "
                                            "exceed the system limit of %d characters.", TXMAXLN);
                              }
                           }
                           else                 // new abs/rel path specified
                           {
                              strcpy( fd->fPath, fd->fName);
                              strcpy( fd->fName, fdata->wildcard);
                              lhwnd = txwWindowFromID(hwnd, TXFD_WID_SFNAME);
                              txwSendMsg(lhwnd, TXWM_CHAR,  TXFD_WID_SFNAME, TXk_END);
                              txwInvalidateWindow(   lhwnd, TRUE, TRUE);

                              TRACES(("fd->fPath changed from SFNAME: '%s'\n", fd->fPath));
                              txwPostMsg( hwnd, TXWM_FD_NEWPATH, 0, 0);
                           }
                        }
                        else                    // (new) wildcard filter
                        {
                           txwPostMsg( hwnd, TXWM_FD_NEWSPEC, 0, 0);
                        }
                        break;

                     default:                   // others, would end dialog
                        rc = txwDefDlgProc( hwnd, msg, mp1, mp2);
                        break;
                  }
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
}                                               // end 'txwDefFileDlgProc'
/*---------------------------------------------------------------------------*/

