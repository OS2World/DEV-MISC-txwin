#ifndef TXVERS_H
#define TXVERS_H
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
// TX window library, version history
//
// Author: J. van Wijk
//
// Originally developed for LPTool/DFSee utilities, open-sourced in 2005
//
#define TXLIB_N "TX-library"
#define TXLIB_C "(c) 1995-2014: Fsys Software"

#define TXLIB_V "2.65 10-06-2014" // HELP: search 'titles only' option; <F8> footer in GREP list
//efine TXLIB_V "2.65 09-06-2014" // '/' as alias for <F7> in Help, pops up the search dialog
//efine TXLIB_V "2.65 30-05-2014" // Ctrl-N/P next/prev search; New dialog after Alt-U + Ctrl-F
//efine TXLIB_V "2.65 27-05-2014" // Search and GREP support in the scrollbuffer, like help
//efine TXLIB_V "2.65 24-05-2014" // Support for a 'marked-area' in one line, in txwPaintSbView
//efine TXLIB_V "2.65 23-05-2014" // Make sure help-section list stays sorted on baseid
//efine TXLIB_V "2.65 20-05-2014" // F8  GREP search and toggle between search-result list
//efine TXLIB_V "2.65 18-05-2014" // F10 select help item from list; Auto position listbox
//efine TXLIB_V "2.65 13-05-2014" // Fixed buffer-overflow trap in HELP selist creation
//efine TXLIB_V "2.65 09-05-2014" // F5/F6 prev/next HELP section; F9 select section from list
//efine TXLIB_V "2.65 03-05-2014" // MENU2HTML no path prefix on 'item'passed to -shots.php
//efine TXLIB_V "2.65 18-04-2014" // MENU2HTML change fixed 'txmshots' DIR to specified prefix
//efine TXLIB_V "2.65 18-04-2014" // Added 'switch to Expert mode' hint after each help item
//efine TXLIB_V "2.64 11-04-2013" // Display name of current logfile on 'log -?' or 'log ?'
//efine TXLIB_V "2.63 24-10-2012" // Added hack for altGr+backslash to '\' for German keyboards
//efine TXLIB_V "2.62 11-09-2012" // Added actual LastModDate to generated screenshot PHP files
//efine TXLIB_V "2.61 22-08-2012" // Fixed drawing of double-vertical line on buttons for OSX
//efine TXLIB_V "2.60 20-07-2012" // SCRIPT syntax warning if target label for break not found
//efine TXLIB_V "2.60 19-07-2012" // SCRIPT optional label on loops and on break/exit when
//efine TXLIB_V "2.60 15-07-2012" // SCRIPT Added 'exit when (cond)' conditional loop break
//efine TXLIB_V "2.60 15-07-2012" // SCRIPT Added 'loop .. endloop' script control structure
//efine TXLIB_V "2.56 22-06-2012" // Reduce TRACE from TxAbort() function    (options)
//efine TXLIB_V "2.56 21-06-2012" // Linux switch -stat64 force stat64    (backed out)
//efine TXLIB_V "2.56 09-06-2012" // Autonumbered logfiles using '^' as last fname char
//efine TXLIB_V "2.55 31-03-2012" // Add 16/32/64 bit endian swap and MacUniAppend()
//efine TXLIB_V "2.54 27-03-2012" // Debug version to find list-sort bug (trap!)
//efine TXLIB_V "2.53 23-03-2012" // Increased nr of APPL long-options from 10 to 16
//efine TXLIB_V "2.52 21-03-2012" // Help on MAC OSX Home/End/PgUp/PgDn key activation
//efine TXLIB_V "2.51 09-01-2012" // Added help on using SPACE to operate any button
//efine TXLIB_V "2.50 27-05-2011" // More lines in file/dir lists with smaller buttons
//efine TXLIB_V "2.50 26-05-2011" // Added sort-indicators on file/dir lists in dialog
//efine TXLIB_V "2.50 23-05-2011" // Fix sort on file-extension (Ctrl-E or F4) in desc
//efine TXLIB_V "2.38 21-05-2011" // Support sorting in file dialog DIR/FILE windows
//efine TXLIB_V "2.37 06-05-2011" // Show minor version 6.10 in 'about' as Windows-7
//efine TXLIB_V "2.36 23-04-2011" // Fixed minor display bug in Hex editor on 1st byte
//efine TXLIB_V "2.35 04-04-2011" // Added TxFileOpenRead for generic open large files
//efine TXLIB_V "2.34 27-03-2011" // Fixed TxFileExists for large files > 2Gb on Linux
//efine TXLIB_V "2.33 16-02-2011" // Added more pointer-validation to hexedit (traps)
//efine TXLIB_V "2.33 23-01-2011" // Restrict PgUp/PgDn to max TWO items in hex editor
//efine TXLIB_V "2.33 22-01-2011" // Restrict PgUp/PgDn to max ONE item in hex editor
//efine TXLIB_V "2.33 19-01-2011" // Never Abort() when in batch (sw 'b' or opt 'B')
//efine TXLIB_V "2.32 08-01-2011" // Support 'reason' string on disabled menu-item info
//efine TXLIB_V "2.32 03-01-2011" // Allow UPPERCASE operators like AND/OR/LE/EQ only
//efine TXLIB_V "2.31 12-09-2010" // Use stat64 on E_OVERFLOW in TXTREE/TXTNAME Linux
//efine TXLIB_V "2.31 12-09-2010" // Added more trace to Linux stat (file/dir check)
//efine TXLIB_V "2.30 27-07-2010" // Allow UUID syntax for FormatMixedStr function
//efine TXLIB_V "2.30 26-07-2010" // Fix minor warning and recompile on OpenWatcom 1.9
//efine TXLIB_V "2.30 19-07-2010" // Don't execute bare assignment with comment
//efine TXLIB_V "2.29 09-07-2010" // Translate illegal chars in 8.3 filename conversion
//efine TXLIB_V "2.28 20-05-2010" // Fixed initial (parent) position TxWidget dialogs
//efine TXLIB_V "2.27 27-03-2010" // Implemented persistent position for widget dialog
//efine TXLIB_V "2.26 24-02-2010" // Implemented persistent position for std dialogs
//efine TXLIB_V "2.26 23-02-2010" // Step + Verbose check-box in singlestep popup
//efine TXLIB_V "2.26 23-02-2010" // Fixed trap on single WHILE/FOR/IF on last line
//efine TXLIB_V "2.26 23-02-2010" // Fixed trap when 'until' is missing for a 'do'
//efine TXLIB_V "2.26 20-02-2010" // Add variable substitution to single-step popup
//efine TXLIB_V "2.26 20-02-2010" // Confirm single-step execute line, allow cancel
//efine TXLIB_V "2.25 05-02-2010" // Allow . as placeholder in script parameters
//efine TXLIB_V "2.24 01-01-2010" // New '-screen:COLSxROWS' switch for screen size
//efine TXLIB_V "2.24 01-01-2010" // New '-dir:program-path' switch to find files
//efine TXLIB_V "2.23 11-10-2009" // Fixed FsSpace for drives > 2Gb on FreeDOS
//efine TXLIB_V "2.22 26-09-2009" // Fixed break in FOR statement (break 1 level)
//efine TXLIB_V "2.21 23-09-2009" // Script comment/pragma start in any column
//efine TXLIB_V "2.21 23-09-2009" // Use application callback to resolve constants
//efine TXLIB_V "2.20 02-09-2009" // Make ScreenRows() available in non-windowed
//efine TXLIB_V "2.12 16-03-2009" // Max nr of bytes/row to 133 in HEX editor
//efine TXLIB_V "2.12 16-03-2009" // Fix trap with > 50 bytes/row in hex editor
//efine TXLIB_V "2.11 11-10-2008" // New fileselect(), filesave(), dirselect()
//efine TXLIB_V "2.10 30-08-2008" // Fixed broken builtin-function parsing
//efine TXLIB_V "2.09 26-08-2008" // Fixed 'set xxx -?' type of help request
//efine TXLIB_V "2.08 22-08-2008" // Implemented true/false/rc_... constants
//efine TXLIB_V "2.08 14-08-2008" // Implemented lazy evaluation on OR / AND
//efine TXLIB_V "2.08 14-08-2008" // Add control-structure help text function
//efine TXLIB_V "2.08 13-08-2008" // Fixed bug in STREQ and related operators
//efine TXLIB_V "2.08 13-08-2008" // Added BREAK and CONTINUE loop controls
//efine TXLIB_V "2.08 12-08-2008" // Added pragma and built-in function help
//efine TXLIB_V "2.08 12-08-2008" // Fixed parameter bug in left() and right()
//efine TXLIB_V "2.07 11-02-2008" // '$$' in expression resolves to '$' value
//efine TXLIB_V "2.06 06-01-2008" // Double-buffer VIO on ReadCell, buffer 1Kb
//efine TXLIB_V "2.05 27-12-2007" // Error msg 'disabled item' on mouseclick too
//efine TXLIB_V "2.05 27-12-2007" // Added APP_QUIT rc, quit script execution
//efine TXLIB_V "2.05 23-12-2007" // Added MOUSE hints to help in OS/2 and WIN
//efine TXLIB_V "2.05 20-12-2007" // Fix single '$' chars in resolve expression
//efine TXLIB_V "2.05 15-12-2007" // Add application version number to menubar
//efine TXLIB_V "2.04 07-12-2007" // Added automatic set of $_rc on commands
//efine TXLIB_V "2.04 28-11-2007" // Implement 'error ignore' pragma in script
//efine TXLIB_V "2.03 25-11-2007" // Minor tweaks, final version for 9.03
//efine TXLIB_V "2.03 20-11-2007" // Changed expression resolving, allow $x
//efine TXLIB_V "2.03 15-11-2007" // Added message() built-in function
//efine TXLIB_V "2.03 12-11-2007" // Added parse-tree based script execute
//efine TXLIB_V "2.03 01-11-2007" // Added expression/assignment evaluators
//efine TXLIB_V "2.03 20-10-2007" // Disable <> accel if not at start entryf
//efine TXLIB_V "2.02 04-10-2007" // Allow static-HTML generation menu2html
//efine TXLIB_V "2.02 01-10-2007" // Added "NDFS32" to LAN type filesystems
//efine TXLIB_V "2.01 03-09-2007" // Added "menu2html" conversion functions
//efine TXLIB_V "2.00 09-08-2007" // Fixed default value in param-prompting
//efine TXLIB_V "2.00 06-08-2007" // Limit HEXED marked-area to curr object
//efine TXLIB_V "2.00 22-07-2007" // Recognize RXVT terminal as X compatible
//efine TXLIB_V "2.00 11-07-2007" // Update 3D-NC colorscheme MAC Terminal
//efine TXLIB_V "2.00 04-07-2007" // Interpret -x123 as longname, not -x:123
//efine TXLIB_V "2.00 11-06-2007" // Updated for MAX OS X (DARWIN) platform
//efine TXLIB_V "1.20 13-05-2007" // HEXED 32-column minimum width to 140
//efine TXLIB_V "1.17 08-04-2007" // Free up RMB for other uses (clipboard)
//efine TXLIB_V "1.17 01-04-2007" // Alt-E/J/K/S for cursor related marks
//efine TXLIB_V "1.17 01-04-2007" // Add Alt-1 .. Alt-0 for 1..10 byte mark
//efine TXLIB_V "1.17 31-03-2007" // Add Alt-R reversing the bytes in mark
//efine TXLIB_V "1.17 31-03-2007" // Add Alt-C copy, Alt-M move mark data
//efine TXLIB_V "1.17 31-03-2007" // Allow single-line paste clipboard
//efine TXLIB_V "1.16 18-02-2007" // Fixed Window resize at WIN startup
//efine TXLIB_V "1.16 10-02-2007" // Fixed PATH-corruption in FindByPath
//efine TXLIB_V "1.15 24-01-2007" // Implemented IsRemovable for Windows
//efine TXLIB_V "1.14 07-01-2007" // Message popup on disabled menu items
//efine TXLIB_V "1.14 03-01-2007" // Add 'UDF' to non-HD type filesystems
//efine TXLIB_V "1.13 16-11-2006" // Updated HELP screen for hex editor
//efine TXLIB_V "1.12 10-09-2006" // Default to 7-bit ASCII on Linux (+/-)
//efine TXLIB_V "1.12 07-09-2006" // Add TxFileSetTime for file recovery
//efine TXLIB_V "1.12 29-08-2006" // xml/url/dir/file/disk option/switch
//efine TXLIB_V "1.11 16-08-2006" // Add MARKED area in hex-edit (Alt-B/L)
//efine TXLIB_V "1.11 15-08-2006" // Add SetFileSize OS-agnostoc function
//efine TXLIB_V "1.11 14-08-2006" // Fix modify exit-prompt after deleting
//efine TXLIB_V "1.11 12-08-2006" // Added SetFileSizeL() to OS/2 API's
//efine TXLIB_V "1.11 12-08-2006" // Added INSERT_1/DELETE_1 to HexEdit
//efine TXLIB_V "1.11 12-08-2006" // Made HexEdit WinProc a TOP-LEVEL
//efine TXLIB_V "1.10 13-07-2006" // APP long switch/option expanded to 9
//efine TXLIB_V "1.10 09-07-2006" // HEXEDIT Ctrl-E erase item, HEX value
//efine TXLIB_V "1.09 05-07-2006" // HEXEDIT Home/End if PgUp/PgDn fails
//efine TXLIB_V "1.09 03-07-2006" // HEXEDIT Added Find/find-Again logic
//efine TXLIB_V "1.09 28-06-2006" // HEXEDIT Added highlighted item 0x SN
//efine TXLIB_V "1.09 25-06-2006" // HEXEDIT RC check on all write-backs
//efine TXLIB_V "1.09 22-06-2006" // Fix border repaint overlapping popups
//efine TXLIB_V "1.09 22-06-2006" // HEXEDIT DEL/BACKSPACE and writeback
//efine TXLIB_V "1.09 20-06-2006" // HEXEDIT keyboard & mouse implemented
//efine TXLIB_V "1.09 18-06-2006" // HEXEDIT paint/movement logic implemented
//efine TXLIB_V "1.09 14-06-2006" // Basic HEXEDIT paint logic implemented
//efine TXLIB_V "1.08 14-05-2006" // Accurate logfile size, ext length 3
//efine TXLIB_V "1.08 12-05-2006" // Logfile maximum size and auto-cycling
//efine TXLIB_V "1.07 04-05-2006" // Avoid (history) popup from other list
//efine TXLIB_V "1.07 02-05-2006" // Ctrl-D on cmdline, auto NEXT history
//efine TXLIB_V "1.07 30-04-2006" // Ctrl-D, delete history-item from popup
//efine TXLIB_V "1.07 26-04-2006" // Add logfile ON/OFF switch  (as screen)
//efine TXLIB_V "1.07 02-04-2006" // Fix possible crash on PATH > 1024 char
//efine TXLIB_V "1.06 25-03-2006" // Fix SbView update while Msg/Conf popup
//efine TXLIB_V "1.06 17-03-2006" // Larger user-status text; txStrSec2hms
//efine TXLIB_V "1.06 11-03-2006" // Allow '-' as cmd letter  (not option)
//efine TXLIB_V "1.06 11-03-2006" // Allow '<' and '>' as accelerator keys
//efine TXLIB_V "1.06 26-01-2006" // SubstituteParams now external function
//efine TXLIB_V "1.06 06-01-2006" // Fixed trap in FileDlg due to USER ptr
//efine TXLIB_V "1.06 05-01-2006" // Topline feedback on alt-/ trace toggle
//efine TXLIB_V "1.06 05-01-2006" // Added usrdata ptr to txwWidgetDialog()
//efine TXLIB_V "1.05 29-12-2005" // Fixed Radio-repaint bug on (x == TRUE)
//efine TXLIB_V "1.05 27-12-2005" // Made O_BINARY default for Linux files
//efine TXLIB_V "1.05 13-12-2005" // Fix set focus on [OK] in WidgetDialog
//efine TXLIB_V "1.04 12-12-2005" // Fixed ValidateScript desc for 1-liner
//efine TXLIB_V "1.03 08-12-2005" // Home/End in SBVIEW now PgLeft/PgRight
//efine TXLIB_V "1.02 02-12-2005" // Add EventHook/Async Input functions
//efine TXLIB_V "1.02 30-11-2005" // PromptBox with Widgets fixed width 76
//efine TXLIB_V "1.02 29-11-2005" // minor updates for NetLabs release
//efine TXLIB_V "1.01 07-11-2005" // Added formal LGPL headers to all files
//efine TXLIB_V "1.01 30-10-2005" // Mouse fix requires explicit -mouse now
//efine TXLIB_V "1.01 30-10-2005" // TXLN made 320 now, allow longer PATHs
//efine TXLIB_V "1.01 26-10-2005" // Fix mouse-cursor full-screen (OS2 bug)
//efine TXLIB_V "1.00 28-09-2005" // Renamed to Open TxWindows, version 1.00
                                  // Previous considered to be 0.6.13 now :-)
//efine TXLIB_V "6.13 28-09-2005" // Added dlgproc parameter to txwWidgetDialog
//efine TXLIB_V "6.13 22-09-2005" // Added -d:xx option, slowdown while tracing
//efine TXLIB_V "6.13 15-09-2005" // Removed the DlgBox hack for exit position
//efine TXLIB_V "6.12 01-09-2005" // Pragma 'setparam ? prompt' added to script
//efine TXLIB_V "6.12 30-08-2005" // Strip spaces from CR/LF on setdefaultparam
//efine TXLIB_V "6.12 25-08-2005" // Fix quoted switches like -l:"spaced name"
//efine TXLIB_V "6.11 22-08-2005" // Auto-close (trace) logfile on TxReboot()
//efine TXLIB_V "6.11 20-08-2005" // Moved Exe-name tracing to TxINITmain macro
//efine TXLIB_V "6.11 18-08-2005" // Buffer-flush optional on TxReboot (PM msg)
//efine TXLIB_V "6.11 16-08-2005" // Allow any single digit APP-cmdline switches
//efine TXLIB_V "6.10 24-07-2005" // Added optional codepage to DOS keyb command
//efine TXLIB_V "6.09 13-07-2005" // Fixed mouse bug on OS2-PM after DFSee usage
//efine TXLIB_V "6.08 05-07-2005" // Enhanced mouse-PTR drawing in OS2 version
//efine TXLIB_V "6.07 29-06-2005" // Fixed mouse-dragging on OS2 full-screen
//efine TXLIB_V "6.07 29-06-2005" // Fixed hang on exit in OS2 full-screen
//efine TXLIB_V "6.07 29-06-2005" // Fixed numLock/scrollLock mouse button bug
//efine TXLIB_V "6.07 21-06-2005" // Fixed child 1/2 resize 'floating' bug
//efine TXLIB_V "6.07 20-06-2005" // Fixed a few minor paint-bugs on move SB
//efine TXLIB_V "6.07 17-06-2005" // Completed DOS mouse with KBD shift-state
//efine TXLIB_V "6.07 16-06-2005" // Implemented most DOS mouse handling
//efine TXLIB_V "6.07 15-06-2005" // Fixed 1/2 vertical SIZE/MOVE by 1 char
//efine TXLIB_V "6.07 14-06-2005" // Added full-window drag and optimized move
//efine TXLIB_V "6.07 11-06-2005" // Added OK and Cancel buttons to std dialogs
//efine TXLIB_V "6.07 10-06-2005" // Mouse support for OS2 version, working OK
//efine TXLIB_V "6.07 03-06-2005" // Fix screen resize trap and 'mode' missing
//efine TXLIB_V "6.06 28-05-2005" // Click 'scrollbar' TEXTVIEW/LISTBOX scrolls
//efine TXLIB_V "6.06 27-05-2005" // Added close-button to HELP and TextView()
//efine TXLIB_V "6.06 26-05-2005" // Added listpopup autoclose and menu-toggle
//efine TXLIB_V "6.06 25-05-2005" // Added close-button to dialog frame/canvas
//efine TXLIB_V "6.06 25-05-2005" // Fixed 'mouse-trap' / F10 in scroll-window
//efine TXLIB_V "6.05 24-05-2005" // Toggle menu with click on top-line SBVIEW
//efine TXLIB_V "6.05 20-05-2005" // Mouse support completed for WIN version
//efine TXLIB_V "6.05 17-05-2005" // Rudimentary mouse support for WIN version
//efine TXLIB_V "6.05 13-05-2005" // Refresh filedialog volume-list unless -V-
//efine TXLIB_V "6.05 09-05-2005" // TxTrueName fix for (Free?)DOS CDROM paths
//efine TXLIB_V "6.05 09-05-2005" // Keep embedded spaces in arguments (GetArgs)
//efine TXLIB_V "6.04 05-05-2005" // Fix retrieval of prefixed history strings
//efine TXLIB_V "6.04 03-05-2005" // Avoid any escape-sequence for linux TXOEM
//efine TXLIB_V "6.03 19-04-2005" // Reverted WIN screen to vertical -2 change
//efine TXLIB_V "6.02 01-04-2005" // Reduced resized WIN screen to vertical -2
//efine TXLIB_V "6.01 21-03-2005" // Cleanup FileDialog path+basename handling
//efine TXLIB_V "6.01 19-03-2005" // Added CreateEmptyFile for DFSee RAW2IMZ
//efine TXLIB_V "6.01 17-03-2005" // Implemented TAB-groups (radio/checkboxes)
//efine TXLIB_V "6.01 04-03-2005" // Add display of REXX returned STRING value
//efine TXLIB_V "6.01 04-03-2005" // Improved filedialogs (default filename etc)
//efine TXLIB_V "6.00 20-02-2005" // Improved HELP handling File&Prompt dialogs
//efine TXLIB_V "6.00 17-02-2005" // Use Widget helpid in filedlg & promptbox
//efine TXLIB_V "6.00 16-02-2005" // Optimized FileDialog sizing and placement
//efine TXLIB_V "6.00 09-02-2005" // PgDn/PgUp in help will now sync to #items
//efine TXLIB_V "6.00 30-01-2005" // Fixed MOVE on ListBox popu from Spin-value
//efine TXLIB_V "6.00 27-01-2005" // Add Widget handling to PromptBox/FileDlg
//efine TXLIB_V "6.00 25-01-2005" // New GROUP Window-US for better autoradios
//efine TXLIB_V "6.00 24-01-2005" // Implemented TXWIDGET handling and dialog
//efine TXLIB_V "6.00 23-01-2005" // Prepared TXZIP for new compr-types and CRC
//efine TXLIB_V "5.17 19-01-2005" // Changed 3D Gray/Cmdr to 'closed' corners
//efine TXLIB_V "5.17 19-01-2005" // Added generic TxSeekFile, large file API
//efine TXLIB_V "5.17 05-01-2005" // Fix ScreenCols=0 TRAP (redirected output)
//efine TXLIB_V "5.17 04-01-2005" // No trace on DFSEETRACE envvar if -l switch
//efine TXLIB_V "5.17 03-01-2005" // Add REN and RENAME to trusted commands
//efine TXLIB_V "5.17 03-01-2005" // Translate spaces to underscore in ..8dot3()
//efine TXLIB_V "5.17 31-12-2004" // More TXZIP decode-stack and error messages
//efine TXLIB_V "5.17 30-12-2004" // Reversed default output on subcom handler
//efine TXLIB_V "5.17 28-12-2004" // Fixed TRAP on trace timestamping
//efine TXLIB_V "5.17 22-12-2004" // Fixed quoted-string passthrough on argv[]
//efine TXLIB_V "5.17 02-12-2004" // Updated to allow _TXOEM_ library versions
//efine TXLIB_V "5.16 22-11-2004" // default number radix, "set radix" for input
//efine TXLIB_V "5.16 21-11-2004" // Allow 0n or 0t prefix DECIMAL in ParseNumber
//efine TXLIB_V "5.16 17-11-2004" // Limit max path+fname FileDlg to avoid traps
//efine TXLIB_V "5.16 17-11-2004" // Fix Linux TxTree/TxTrueName for files > 2Gb
//efine TXLIB_V "5.16 16-11-2004" // New TxFileSize() retrieves 64-bit filesize
//efine TXLIB_V "5.16 14-11-2004" // Allow Ctrl-L as screen-repaint, as Ctrl-R
//efine TXLIB_V "5.16 09-11-2004" // RC 225 (aborted) on escaped 'confirm' cmd
//efine TXLIB_V "5.15 05-11-2004" // Fixed Linux crash on '*' expanded parameter
//efine TXLIB_V "5.14 05-10-2004" // Fixed Linux clear-screen non-windowed init
//efine TXLIB_V "5.14 30-09-2004" // Fixed cursor-home bug in non-windowed mode
//efine TXLIB_V "5.14 13-09-2004" // Assign Shift-F1 as alternative HELP (GNOME)
//efine TXLIB_V "5.13 09-09-2004" // Fixed find-file by path for Linux (key)
//efine TXLIB_V "5.13 09-09-2004" // Added $LOGNAME to Linux version string
//efine TXLIB_V "5.13 08-09-2004" // Fixed garbage-chars on win-restore  Linux
//efine TXLIB_V "5.13 08-09-2004" // Changed unreadable footer colors on Linux
//efine TXLIB_V "5.12 30-08-2004" // Add TxReboot() for Linux with sync+reboot
//efine TXLIB_V "5.12 28-08-2004" // Added TxSleep for linux using nanosleep()
//efine TXLIB_V "5.12 26-08-2004" // Fixed SCRIPT handling for CR/LF on Linux
//efine TXLIB_V "5.11 19-08-2004" // Fixed txIsValidHex() (??? seen as hex bug)
//efine TXLIB_V "5.11 19-08-2004" // Force lower-case keyname for Linux (DFSee)
//efine TXLIB_V "5.11 12-08-2004" // Changed Linux default color-scheme to 3d-NC
//efine TXLIB_V "5.11 04-08-2004" // New Selist create with 1 disabled text item
//efine TXLIB_V "5.11 03-08-2004" // Minor fixes in list-handling (Linux, empty)
//efine TXLIB_V "5.10 08-07-2004" // Handle F3 in SBVIEW as TAB key (go to cmd)
//efine TXLIB_V "5.09 23-06-2004" // Added 'IsValidHex' function
//efine TXLIB_V "5.08 05-06-2004" // Updated SB-status text to "Ctrl", not "Alt"
//efine TXLIB_V "5.08 04-06-2004" // Enabled TRACE command for all versions (R)
//efine TXLIB_V "5.08 03-06-2004" // Changed blinking title/footer in 3D-Commander
//efine TXLIB_V "5.08 02-06-2004" // Fix minor typo in "set color" value display
//efine TXLIB_V "5.07 24-05-2004" // Allow DOT in PATH for Add/Strip extension
//efine TXLIB_V "5.07 19-05-2004" // Auto-select SB-colors on scheme-change
//efine TXLIB_V "5.07 17-05-2004" // Fix 'extension' select bug in FileDialog
//efine TXLIB_V "5.06 15-05-2004" // Default rsmaxlines to 75 (Windows :-)
//efine TXLIB_V "5.06 06-05-2004" // Add '\' to drive-spec in file-dialogs
//efine TXLIB_V "5.05 27-04-2004" // Added KEYB switch & cmd for FreeDOS
//efine TXLIB_V "5.04 25-04-2004" // Finalized colorschemes and made it work
//efine TXLIB_V "5.04 20-04-2004" // Added colorschemes for windowed stuff
//efine TXLIB_V "5.03 15-04-2004" // Fixed trap on resized list-window (menu)
//efine TXLIB_V "5.02 15-04-2004" // Added arrowMode for size/move window
//efine TXLIB_V "5.02 13-04-2004" // Fixed trap on FindFirst path with wildcard
//efine TXLIB_V "5.02 12-04-2004" // Added non-color focus indicator to lists
//efine TXLIB_V "5.02 08-04-2004" // Removed ANSI test of DOS when windowed
//efine TXLIB_V "5.01 18-03-2004" // Made COLORS, CHARSET and CBOXES stdcmd
//efine TXLIB_V "5.01 14-03-2004" // ANSI table now full 256 color-strings
//efine TXLIB_V "5.00 13-03-2004" // First version to include a Linux target
//efine TXLIB_V "4.05 03-03-2004" // Trace funcs always available; -r on LOG cmd
//efine TXLIB_V "4.04 25-02-2004" // Updated WIN keyboard-translations (table)
//efine TXLIB_V "4.03 16-02-2004" // Updated building to reduce cflags contents
//efine TXLIB_V "4.02 06-02-2004" // Switch off blink attribute after mode change
//efine TXLIB_V "4.01 03-02-2004" // 'Not Ready ..' same as CANCEL in FileDialog
//efine TXLIB_V "4.01 30-01-2004" // Keep exename in Argv(0) even when tracing
//efine TXLIB_V "4.00 12-01-2004" // Fix wrong ;;defaultparam values with comment
//efine TXLIB_V "4.á6 03-01-2004" // Add description-display on menu-headers
//efine TXLIB_V "4.á6 26-12-2003" // Fix SET PRIO command, add query option
//efine TXLIB_V "4.á5 17-12-2003" // Fix CritErr handler init for OS/2 and WIN
//efine TXLIB_V "4.á5 17-12-2003" // Add focus-indicator to FileDlg Entry/Drive
//efine TXLIB_V "4.00 14-12-2003" // Fixed CritErr handler and -F- to enable it
//efine TXLIB_V "4.00 13-12-2003" // Added -M menustyle switch (right-arrow)
//efine TXLIB_V "4.00 12-12-2003" // Added critical error handler MsgBox
//efine TXLIB_V "4.00 10-12-2003" // No fname copy on LN_SETFOCUS in filedialog
//efine TXLIB_V "4.00 06-12-2003" // Delay autodrop menu on automenu activate
//efine TXLIB_V "4.00 04-12-2003" // Respect current directory in drive-select
//efine TXLIB_V "4.00 04-12-2003" // Accept F4=OK as "Yes" response on confirm
//efine TXLIB_V "4.00 03-12-2003" // Fix CD and X: commands for Watcom compiler
//efine TXLIB_V "4.00 03-12-2003" // F4 handled as OK button in std dialog-proc
//efine TXLIB_V "4.00 25-11-2003" // Made normal lists wrap at top/end of list
//efine TXLIB_V "4.00 20-11-2003" // Fixed TxFsType/TxFsVolumes for DOS version
//efine TXLIB_V "4.00 19-11-2003" // Focus to Dir-list in select-dir dialog
//efine TXLIB_V "4.00 17-11-2003" // Return FALSE in TxPrompt on CANCELed dialog
//efine TXLIB_V "4.00 15-11-2003" // Add 'REMOTE' for disconnected WIN-NT drives
//efine TXLIB_V "4.00 14-11-2003" // Cache std-file-dialog volume-list; status
//efine TXLIB_V "4.00 12-11-2003" // Added multiple-wildcards to file-dialog
//efine TXLIB_V "4.00 11-11-2003" // Remeber last-path-used to file-dialogs
//efine TXLIB_V "4.00 09-11-2003" // Added Win/Menu/Find special keys
//efine TXLIB_V "3.10 06-11-2003" // Fixed trap when list-selected is invalid
//efine TXLIB_V "3.10 03-11-2003" // Fixed 'shadow-only' minimized windows
//efine TXLIB_V "3.10 02-11-2003" // Fixed trap on empty FD-filelist (any :-)
//efine TXLIB_V "3.09 28-10-2003" // WM_COMMAND msg on RADIO/CHECK button 'space'
//efine TXLIB_V "3.08 20-10-2003" // Added SelDirFileDialog DIR selection
//efine TXLIB_V "3.07 15-10-2003" // Added AUTOSKIP flag on listitems
//efine TXLIB_V "3.07 06-10-2003" // Fix trap on <F1> with empty filelist
//efine TXLIB_V "3.06 03-10-2003" // Use std footer on dialogs; 2 * a_F10 fix
//efine TXLIB_V "3.06 02-10-2003" // F12 on menu and a_F10/a_F5 maximize/restore
//efine TXLIB_V "3.06 28-09-2003" // Add GetSelectChar for dynamic list create
//efine TXLIB_V "3.06 27-09-2003" // Position helptext on item-title itself
//efine TXLIB_V "3.06 25-09-2003" // Allow c_A..c_Z as accelerator in ListBox
//efine TXLIB_V "3.06 23-09-2003" // Fix paint bug on stray-msg destroyed window
//efine TXLIB_V "3.05 21-09-2003" // Added submenu FLAG to SELISTitem/listbox
//efine TXLIB_V "3.05 19-09-2003" // Fixed parsing of -menu- type switches
//efine TXLIB_V "3.05 15-09-2003" // FileDialog direct/fnames list working OK
//efine TXLIB_V "3.05 12-09-2003" // FileDialog entry, volume & create working
//efine TXLIB_V "3.05 07-09-2003" // Added Black-to-Blue background for SBview
//efine TXLIB_V "3.05 05-09-2003" // Added border-shadow style, use in dialogs
//efine TXLIB_V "3.05 04-09-2003" // Added Inverted/Bright setting functions
//efine TXLIB_V "3.04 27-08-2003" // Made TxFileTree OS-neutral; W32 working
//efine TXLIB_V "3.04 24-08-2003" // Added Disabled and separator to menus
//efine TXLIB_V "3.04 23-08-2003" // Added F1/WM_HELP as system accelerator
//efine TXLIB_V "3.04 22-08-2003" // Completed MenuBar and AccelTable stuff
//efine TXLIB_V "3.04 17-08-2003" // Added MenuBar and more ListBox processing
//efine TXLIB_V "3.03 08-08-2003" // Support selected long switchnames like query
//efine TXLIB_V "3.03 07-08-2003" // Allow '/' as SWITCH-char (but not options)
//efine TXLIB_V "3.03 02-08-2003" // Fixed 'arrow' hang when minimized (F12)
//efine TXLIB_V "3.03 27-07-2003" // Added more LISTBOX processing and defs
//efine TXLIB_V "3.02 24-07-2003" // Reduced linelength for registration text
//efine TXLIB_V "3.02 20-07-2003" // Removed CTRL_FIELD style for entryfields
//efine TXLIB_V "3.02 10-07-2003" // Fixed wrapping footer-text on Windows-NT
//efine TXLIB_V "3.02 07-07-2003" // Fixed 'transparent' CANVAS in dialog
//efine TXLIB_V "3.01 24-06-2003" // Added "set ASC" for 7-bit ascii output
//efine TXLIB_V "3.01 21-06-2003" // Updated codepage-437/850 compatibility
//efine TXLIB_V "3.01 18-06-2003" // Refined Windows NT/W2K/XP version display
//efine TXLIB_V "3.00 16-06-2003" // Changed major version after rename to TX
//efine TXLIB_V "2.23 16-06-2003" // Fixed centering of too long title/footer
//efine TXLIB_V "2.23 05-06-2003" // Global rename to TxWindow library
//efine TXLIB_V "2.22 03-06-2003" // Made some Mb to MiB type updates
//efine TXLIB_V "2.22 02-06-2003" // Added trace to find expiry problem
//efine TXLIB_V "2.22 26-05-2003" // Added Windows-ME specific version string
//efine TXLIB_V "2.21 25-05-2003" // Remove APIRET/SPARAM, fix DOS TxRead/TxWrite
//efine TXLIB_V "2.20 05-05-2003" // Removed semaphore screen guard from OS/2
//efine TXLIB_V "2.19 03-05-2003" // Minor update to registration text (regs@)
//efine TXLIB_V "2.19 17-04-2003" // Add kernel-revision to OS/2 version string
//efine TXLIB_V "2.19 13-04-2003" // Updated makefile to use AUTODEPEND on .h
//efine TXLIB_V "2.19 08-04-2003" // Use Alt-F12 to cycle through all SB colors
//efine TXLIB_V "2.19 06-04-2003" // '-C:2' or "screen bright" bright SB colors
//efine TXLIB_V "2.18 27-03-2003" // Added '-7-' option on log for 8bit ASCII
//efine TXLIB_V "2.17 24-03-2003" // Added xx days left for evaluation message
//efine TXLIB_V "2.17 22-03-2003" // Added big/little endian convert txSwapUS/L
//efine TXLIB_V "2.16 10-03-2003" // Update dialog footers & sbview status
//efine TXLIB_V "2.16 09-03-2003" // Small change to (un) registred text
//efine TXLIB_V "2.16 08-03-2003" // Implemented "screen invert" and "set ..."
//efine TXLIB_V "2.16 05-03-2003" // Implemented -C option for inverted sbview
//efine TXLIB_V "2.16 02-03-2003" // Removed updateRect & updateBorder, cleanup
//efine TXLIB_V "2.16 25-02-2003" // Support SBview scroll under pop-up window
//efine TXLIB_V "2.15 13-02-2003" // Moved class indicator to TXWINDOW struct
//efine TXLIB_V "2.15 12-02-2003" // Improved msg trace; Field copy functions
//efine TXLIB_V "2.15 05-02-2003" // Increased msg-queue size for large dialogs
//efine TXLIB_V "2.15 05-02-2003" // Added TXW_STLINE class, single line static
//efine TXLIB_V "2.15 04-02-2003" // No (Tp)Message if screen-off & no confirm
//efine TXLIB_V "2.15 02-02-2003" // Increased 'large window' limit to 40 lines
//efine TXLIB_V "2.14 29-01-2003" // Add 'r' and -r as LogReopen trace option
//efine TXLIB_V "2.13 28-01-2003" // Support "keys" command for trouble-shoot
//efine TXLIB_V "2.13 27-01-2003" // Split error-message on Mutex > 4 options
//efine TXLIB_V "2.13 24-01-2003" // New "-t" timestamp on trace (default off)
//efine TXLIB_V "2.13 23-01-2003" // Added DOS32A build (D32) and version str
//efine TXLIB_V "2.12 17-01-2003" // Support SBview painting arround popups
//efine TXLIB_V "2.12 15-01-2003" // Repaint all windows after collapse (F12)
//efine TXLIB_V "2.11 13-01-2003" // Allow desktop resize/move/collapse (F12)
//efine TXLIB_V "2.10 01-01-2003" // TX returncodes range 200..229 (DOS batch)
//efine TXLIB_V "2.10 31-12-2002" // Search exedir and PATH for native script
//efine TXLIB_V "2.10 29-12-2002" // Fixed status/progress flicker on TxZip
//efine TXLIB_V "2.10 28-12-2002" // Allow 1 day date/time difference for key
//efine TXLIB_V "2.10 22-12-2002" // Add "use <Enter> no mouse" to reg message
//efine TXLIB_V "2.10 15-12-2002" // New TxaSetItem() and no -tValue syntax
//efine TXLIB_V "2.09 11-12-2002" // Registration now using TxMessage with [OK]
//efine TXLIB_V "2.09 09-12-2002" // No Pending-abort after registration check!
//efine TXLIB_V "2.08 28-11-2002" // fixed output for native scripts (quiet)
//efine TXLIB_V "2.07 22-11-2002" // changed radio/check button interface (ptr)
//efine TXLIB_V "2.07 19-11-2002" // fixed bug in prompt-box (curpos not set)
//efine TXLIB_V "2.06 14-11-2002" // screen-off on windowed confirm/prompt etc
//efine TXLIB_V "2.06 13-11-2002" // Added mode command for display switching
//efine TXLIB_V "2.05 05-11-2002" // Fixed TRAP on window size 0 (WATCOM only)
//efine TXLIB_V "2.05 05-11-2002" // Fixed all Ctrl/Alt/Shift codes   (WC-WIN)
//efine TXLIB_V "2.05 04-11-2002" // Removed getch() from TxwKeyEvent (WC-WIN)
//efine TXLIB_V "2.04 30-10-2002" // Removed all DOS16 16-bit code
//efine TXLIB_V "2.03 27-10-2002" // Removed assert() usage (Watcom PMWIN dep)
//efine TXLIB_V "2.02 21-09-2002" // Added TxAreaEmpty() function
//efine TXLIB_V "2.02 20-09-2002" // Added 'type' to trusted commands
//efine TXLIB_V "2.01 21-08-2002" // Changed Reparse mode to 'free-format'
//efine TXLIB_V "2.01 18-08-2002" // Better VOL text for unknown-FS / Removable
//efine TXLIB_V "2.00 02-08-2002" // Fine-tune Sbview, fit 53 chars on 80 wide
//efine TXLIB_V "2.0á 31-07-2002" // Updated registration module for 5.xx
//efine TXLIB_V "2.0á 30-07-2002" // Moved Sbview status to position 15 from 20
//efine TXLIB_V "2.0á 26-07-2002" // Added Sbview status/topline string updates
//efine TXLIB_V "2.0á 20-07-2002" // Fixed delay for unregistred 'query RV'
//efine TXLIB_V "2.0á 19-07-2002" // Fixed date/days calculations (two-way now!)
//efine TXLIB_V "2.0á 18-07-2002" // Don't use '/' as option char (for ext cmds)
//efine TXLIB_V "2.00 18-07-2002" // Added 'MEM' and 'SET' to trusted commands
//efine TXLIB_V "2.0á 16-07-2002" // Fixed 'days left' calculation for keys
//efine TXLIB_V "2.0á 15-07-2002" // Update TxSetSize macro to avoid WATCOM bug
//efine TXLIB_V "2.0á 12-07-2002" // TRACE file auto reopen when ext is '.lro'
//efine TXLIB_V "2.0á 11-07-2002" // Fixed FlMakePath trap/function
//efine TXLIB_V "2.0á 11-07-2002" // DOS32 close/reopen log on each trace-line
//efine TXLIB_V "2.0á 09-07-2002" // Watcom/Causeway version; DOS32 Reboot code

// last version to be used in DFSee version 4.xx
//efine TXLIB_V "1.91 03-07-2002" // Allow specified expire time on #T/#U keys
//efine TXLIB_V "1.90 02-07-2002" // Updated registration for tmp/update keys
//efine TXLIB_V "1.89 25-06-2002" // TRACE update TxFsSpace; 'trace logfile'
//efine TXLIB_V "1.88 21-06-2002" // Fixed '-E' option character for errors
//efine TXLIB_V "1.87 13-06-2002" // Added DPMI alloc and version info (TXUTIL)
//efine TXLIB_V "1.86 07-06-2002" // Faster Draw and save/restore
//efine TXLIB_V "1.85 06-06-2002" // First WATCOM ported sources (DOS extender)
//efine TXLIB_V "1.84 30-05-2002" // Added "123h" syntax for numeric options
//efine TXLIB_V "1.83 18-05-2002" // Added generic "confirm" command for scripts
//efine TXLIB_V "1.82 12-05-2002" // Removed confusing " = xx%" from scrollbuf
//efine TXLIB_V "1.82 08-05-2002" // Nicer display of MutEx option errors
//efine TXLIB_V "1.81 19-04-2002" // Removed assert from TxZip code (use RC)
//efine TXLIB_V "1.80 16-04-2002" // Fixed parser alloc bug (possible trap)
//efine TXLIB_V "1.80 15-04-2002" // Added OutputVerbosity and ErrorStrategy
//efine TXLIB_V "1.79 03-04-2002" // Search reg-key in executable-DIR/root too
//efine TXLIB_V "1.78 02-04-2002" // Allow 01-01-xxxx date on key validation
//efine TXLIB_V "1.77 17-03-2002" // New PromptBox dialog and TxPrompt function
//efine TXLIB_V "1.76 04-03-2002" // Use basename for $0, not full scriptname
//efine TXLIB_V "1.75 25-02-2002" // Implemented window-collapse with <F12>
//efine TXLIB_V "1.74 22-02-2002" // Updated native script engine substitution
//efine TXLIB_V "1.73 18-02-2002" // MsgBox in UR corner; Alt + <> scrolls text
//efine TXLIB_V "1.72 12-02-2002" // Added TxsNativeRun to run a script
//efine TXLIB_V "1.71 10-02-2002" // Added comment parsing to command parser
//efine TXLIB_V "1.70 04-02-2002" // Moved '-7' to EXE; and AnsiInit 2 params
//efine TXLIB_V "1.69 01-02-2002" // Added '-7' Exe-switch for 7-bit ASCII
//efine TXLIB_V "1.68 31-01-2002" // Added "sleep" standard command
//efine TXLIB_V "1.67 15-01-2002" // Removed built-in "More ..." funcionality
//efine TXLIB_V "1.66 30-12-2001" // Added .xx check on registration key
//efine TXLIB_V "1.65 13-12-2001" // Fixed display of NO_VAL options
//efine TXLIB_V "1.64 10-12-2001" // Fixed nested-string bug in parser
//efine TXLIB_V "1.63 04-12-2001" // TXA_NO_VAL option type for '-o' => value 0
//efine TXLIB_V "1.62 20-11-2001" // Use TXA parser in TxStdCmd
//efine TXLIB_V "1.61 17-11-2001" // Added '' as alternative string syntax
//efine TXLIB_V "1.60 09-11-2001" // Added -t- and -tValue switches and ,u Unit
//efine TXLIB_V "1.59 05-11-2001" // Accept '-a' switch for no-color
//efine TXLIB_V "1.58 04-11-2001" // Added Ctrl-BS, ctrl-D and <Esc> in entry-F
//efine TXLIB_V "1.57 19-10-2001" // Added TxSleep / TxBusyWait for DOS
//efine TXLIB_V "1.56 09-10-2001" // Removed some trace from cmd-parser
//efine TXLIB_V "1.55 03-10-2001" // Added bright/blink support on fullscreen
//efine TXLIB_V "1.54 30-09-2001" // Support MDOS library (no windows)
//efine TXLIB_V "1.53 29-09-2001" // Registration functions added
//efine TXLIB_V "1.52 18-09-2001" // First MessageBox incarnation
//efine TXLIB_V "1.51 16-09-2001" // resizable child-windows and fixed help
//efine TXLIB_V "1.50 14-09-2001" // Bug fixing release (traps)
//efine TXLIB_V "1.49 12-09-2001" // Help manager txwhelp introduced
//efine TXLIB_V "1.48 10-09-2001" // Dismiss on EVERY pushbutton; Help dialog
//efine TXLIB_V "1.47 08-09-2001" // First displayable button classes
//efine TXLIB_V "1.46 06-09-2001" // Fixed paint-border side clipping bug
//efine TXLIB_V "1.45 05-09-2001" // Added Dlg functions Process, Dismiss, Box
//efine TXLIB_V "1.44 04-09-2001" // Added DISABLED & VISIBLE
//efine TXLIB_V "1.43 03-09-2001" // Destroy children
//efine TXLIB_V "1.42 02-09-2001" // Fixed recursive parent clipping P#1
//efine TXLIB_V "1.41 01-09-2001" // Fixed NT clipping bug on strings
//efine TXLIB_V "1.40 31-08-2001" // Created test dialog structures
//efine TXLIB_V "1.39 26-08-2001" // moveable focus windows working!
//efine TXLIB_V "1.38 22-08-2001" // desktop-border when > 80x25
//efine TXLIB_V "1.37 19-08-2001" // Relative sizes in CreateWindow
//efine TXLIB_V "1.36 16-08-2001" // Added command parser tests (tparse.c)
//efine TXLIB_V "1.35 18-07-2001" // Added FindByPath test
//efine TXLIB_V "1.34 11-07-2001" // Added Ctrl>-arrow help to SBVIEW status
//efine TXLIB_V "1.33 26-02-2001" // Added 8.3 conversion test "83"
//efine TXLIB_V "1.32 22-02-2001" // Added Win-NT/2000 native reboot API
//efine TXLIB_V "1.31 07-11-2000" // Removed 'exit' on error from WIN32CON
//efine TXLIB_V "1.30 15-02-2000" // Fixed 'set ansi on/off' bug in stdcmd
//efine TXLIB_V "1.29 31-01-2000" // Fixed 'screen on' bug in windowed mode
//efine TXLIB_V "1.28 17-01-2000" // Added UNICODE modus to FormatMixedStr
//efine TXLIB_V "1.27 02-01-2000" // Updates to history handling
//efine TXLIB_V "1.26 11-10-1999" // No timestamp when tracelevel >= 900
//efine TXLIB_V "1.25 03-09-1999" // setahead command for typeahead
//efine TXLIB_V "1.24 20-08-1999" // Keyboard handling in one thread only!
//efine TXLIB_V "1.23 28-07-1999" // Cleaned up ANSI handling
//efine TXLIB_V "1.22 22-07-1999" // external-cmd output redirected for DOS/NT
//efine TXLIB_V "1.21 20-07-1999" // midscroll fix;
//efine TXLIB_V "1.20 04-07-1999" // Minor fix, repaint status-only on SBuf
//efine TXLIB_V "1.19 27-06-1999" // New StripAnsi function for Txrintf
//efine TXLIB_V "1.18 22-06-1999" // Ported txwiop window primitives to NT
//efine TXLIB_V "1.17 31-05-1999" // Large scrollbuf; fixed SB alloc bug
//efine TXLIB_V "1.16 22-05-1999" // Dos TXWIN tests, win-ID, scrollbuf
//efine TXLIB_V "1.15 06-04-1999" // Added reboot test
//efine TXLIB_V "1.14 16-03-1999" // Added quick argv[0] test
//efine TXLIB_V "1.13 13-03-1999" // Added critical-error handling for DOS
//efine TXLIB_V "1.12 28-07-1998" // Fixed entry-field, changed TxShowTxt !
//efine TXLIB_V "1.11 26-07-1998" // Added entryfield
//efine TXLIB_V "1.10 25-07-1998" // Added automatic parent-clipping
//efine TXLIB_V "1.09 22-07-1998" // Added txwInvalidate... functions
//efine TXLIB_V "1.08 21-07-1998" // Added fake "cancel" button
//efine TXLIB_V "1.07 19-07-1998" // Limit keys to 0x1ff; new optimized VIEWTXT
//efine TXLIB_V "1.06 06-07-1998" // key-value test added
//efine TXLIB_V "1.05 21-06-1998" // Minor TxZip fixes and txtree root handling
//efine TXLIB_V "1.04 11-06-1998" // Don't expand on Compress :-)
//efine TXLIB_V "1.03 05-06-1998" // Fixed TxZip memory leak and Uncompress hang
//efine TXLIB_V "1.02 03-06-1998" // Extra compression tests (DOS/Win32)
//efine TXLIB_V "1.01 26-05-1998" // Added compression testing and StdCommand
//efine TXLIB_V "1.00 08-03-1998" // Added WIN32 ANSI expansion function
//efine TXLIB_V "0.97 20-04-1997" // Ported to WIN32
//efine TXLIB_V "0.90 27-01-1996" // Made more generic, for LPTIO tests
//efine TXLIB_V "0.80 15-02-1996" // Added ddl global debugging variable
//efine TXLIB_V "0.70 09-03-1996" // Added trace macro's and implementation
//efine TXLIB_V "0.60 26-12-1996" // Added RetBool support for 64 threads
//efine TXLIB_V "0.50 17-06-1995" // Initial version, split off from DHPFS.C

#endif
