//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2007 Fsys Software and Jan van Wijk
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
// TX-library and TXWindowing test program
//
// Author: J. van Wijk
//

#include <txlib.h>                              // TX library interface

#include <fcntl.h>                              // low-level I/O

#include <txtver.h>                             // TXT version info
#include <txtwin.h>                             // windowed entry point
#include <txt.h>                                // TXT navigation and defs

#if defined (DUMP)
#define TXT_TRACE      "TXTSTTRACE"             // Trace startup values
#endif

#define TXT_STARTCMD    "say Hello to the TXT test program"


TXTINF      txt_anchor =
{
   FALSE,                                       // batch-mode disabled
   TRUE,                                        // dialogs will be used
   TXAO_NORMAL,                                 // normal verbosity
   TXAE_CONFIRM,                                // confirmed quit on errors
   0,                                           // overall return-code
   80, 22,                                      // visible screen area
   FALSE,                                       // auto quit on fdisk/setboot
   TRUE,                                        // reg confirm required
   FALSE,                                       // running in classic mode
   NULL,                                        // SB buffer
   0,                                           // SB size
   0,                                           // actual sb linelength
   0,                                           // scroll-buffer window handle
   0,                                           // command handling window
   FALSE,                                       // no automatic menu at start
   TRUE,                                        // automatic pulldown drop
   0,                                           // default drop-down menu
   0,                                           // worklevel
   NULL,                                        // selection list, Color schemes
   #if defined (DOS32)
      FALSE,                                    // Windows-9x DosBox detected
   #endif
};

TXTINF       *txta  = &txt_anchor;              // TXT anchor block

static  char *separator;

char *switchhelp[] =
{
   "  [global-txtst-switches]  [TXTst-multi-command]",
   "",
#if defined (DUMP)
   " -123[t][s][n] = set trace level to 123, TXTst internal function trace;",
   "                 [t]imestamp lines; trace to [s]creen too; [n]o tracefile",
#endif
   "",
   " -?            = help on TXTst commandline switches (this text)",
   " -7            = Use 7-bit ASCII only (no 'graphic' characters)",
   " -a            = switch off usage of ANSI escape characters for color",
   " -b            = batch option, automatic 'batch on' command at startup",
   " -e   or  -e-  = Include or surpress (-e-) command echo before each cmd",
   " -E:[c|i|q]    = default error strategy Confirm, Ignore or Quit",
   " -f            = frame, use a border-frame on scroll-buffer and desktop",
   " -f-           = do not use border-frames. (default NO on 80x25 screen)",
   " -l:logfile    = start logging immediately to 'logfile.log'",
   " -menu         = automatic menu at startup and after each menu-selection",
   " -O:[n|v|q|m]  = default output verbosity Normal, Verbose, Quiet, Maximum",
   " -P-           = don't use prompting and input dialog windows by default",
   " -q            = quiet option, automatic 'screen off' command at startup",
   " -Q            = quit automatically after executing specified command",
   " -Q-           = do NOT quit automatically on normally autoquiting commands",
   " -S            = Shell mode, do not allow quit from TXTst (use as shell)",
   " -s:separator  = specify a command-separator character, default is '#'",
   " -t   or  -t-  = Include or surpress (-t-) timestamp before each command",
   " -scheme:name  = Window scheme: grey|3d|nobl|cmdr|half|full|white|black|dfsee",
   " -color:value  = Output colors, 0..7 add any: 0=std 1=invert 2=bright 4=blue",
   " -style:value  = Line style, 0..3: 0=std-double 1=3d 2=halfblock 3=fullblock",
   " -w   or  -w+  = use windowing, even if command would surpress that",
   " -w-           = do NOT use windowing, default is -w+ (windowing)",
   " -W:[sl]       = Screen resize dialog if screen is more than [sl] lines",
#if defined (WIN32)
   " -W:0 or  -W   = Screen resize dialog if scrollbars are present on window",
#else
   " -W:0 or  -W   = Screen resize dialog if scrollbars are likely (lines > 40)",
#endif
   "",
   " For help on TXTst commands, use the '?' command and use the <F1>",
   " key when shown at bottom line",
   "",
   NULL
};


char               *cmdhelptxt[] =
{
   " ?            [*] = Show list of generic commands with short description",
   " alloc     [size] = Allocate memory, default is in Mib, -k is Kib",
   " boot      [cold] = Reboot the system, warm or cold",
   " cd        [path] = Change current directory an current drive",
   " conf      [text] = Display confirmation dialog window with 'text'",
   " cursor [col,row] = Show, and optionally set the cursor position",
   " dlg              = pop-up a test dialog from windowed mode",
   " display          = Show display size, rows and columns",
   " help         [*] = Show list of generic commands with short description",
   " log       [file] = Log (append) to 'file' (.log); (No file => stop logging)",
   " lines   [number] = Write lines of text, for display performance tests",
   " listbox          = Start listbox test dialog window",
   " mode [x,y] [-w-] = Set screen size and windowing mode, -w- is non-windowed",
   " msgbox    [text] = Show a MessageBox with the supplied text",
   " opt  option-char = Show value for option-char as set by last 'parse'",
   " pack   in [o][m] = Compress file 'in' to [o], using method [m]",
   " parse cmd-string = Parse command string to options and arguments",
   " pdrop      [all] = Drop one or all levels of parsed commands",
   " pshow      [all] = Show one or all levels of parsed commands",
   " prompt   [quest] = Prompt for a value using the 'quest' text as question",
   " pbuf [$]s/f  [m] = Compress+Decompress string/file using method [m]",
   " prep [$]s [c][s] = Measure compress string/file [cnt] times, size [s] Kib",
   " screen  [on|off] = Switch output to the screen on or off",
   " setansi [on|off] = Set use of ANSI escape-sequences (colors) on or off",
   " sbnum            = Number lines in scroll-buffer for testing",
   " scrfile [fn] [l] = Save screen-buffer to file [fn], last [l] lines",
   " unpack in [o][m] = De-compress file 'in' to [o], using method [m]",
   " vol     [floppy] = Show all volumes, optional including floppies",
   " vols    [floppy] = Show all volumes, as a single string",
#if defined (DUMP)
   " trace [lvl]      = Set trace level for TXT internal function tracing",
#endif
#if defined (DEV32)
   " run macro        = Run a TXT macro in a .TXT file",
#else
#endif
   " q                = Quit",
   NULL
};


static  char       txtMsgTitle[] = " Test Message box ";
static  char       txtMsgText[]  =
  "This is message text to test the txwMessageBox. It has some long lines that "
  "need to be split by the Box and also some that are short enough already.\n"
  "So let us see if that works, with an empty line:\n\nAnd a last one ...";

static  char       txtPromptTitle[] = " Test Prompt box ";
static  char       txtPromptText[]  =
  "This is message text to test the txwPromptBox. Fill in any value ...";



// Interactive TXT mode
static ULONG txtInteractive
(
   char               *cmdstring                // IN    initial command
);

// Interpret and execute TXTst command;
static ULONG txtSingleCommand
(
   char               *txtcmd,                  // IN    TXT command
   BOOL                echo,                    // IN    Echo command
   BOOL                quiet                    // IN    screen-off during cmd
);


// Callback function to service HEXEDIT service requests
static ULONG txtHexEditAppCallback
(
   ULONG               cmd,                     // IN    action command
   struct txhexedit   *dat                      // INOUT hexedit data
);

#define TXT_HEX_ITEMSIZE 512

static BYTE      txtHexDataBufP[TXT_HEX_ITEMSIZE] =
   "This is buffer-P for the HEX-Editor test, BEFORE buffer-0, just some text "
   "followed by a remainder of ZERO bytes";

static TXHEBUF   txtHexEditBufP =
{
   txtHexDataBufP,                              // start of data buffer
   0x03f79c00,                                  // abs start position (ref)
   TXT_HEX_ITEMSIZE,                            // size of buffer    (item)
   "LSN:0xffffffff = PSN:0x0181fbce = Cyl: 1672 H:254 S:63  Fsys boot sector"
};

static BYTE      txtHexDataBuf0[TXT_HEX_ITEMSIZE] =
   "This is buffer-0 for the HEX-Editor test, just some lines of text "
   "followed by a remainder of ZERO bytes";

static TXHEBUF   txtHexEditBuf0 =
{
   txtHexDataBuf0,                              // start of data buffer
   0x03f79e00,                                  // abs start position (ref)
   TXT_HEX_ITEMSIZE,                            // size of buffer    (item)
   "LSN:0x00000000 = PSN:0x0181fbcf = Cyl: 1673 H:1   S:1   Fsys boot sector"
};

static BYTE      txtHexDataBuf1[TXT_HEX_ITEMSIZE] =
   "Buffer-1, the current buffer when starting the test             "
   "                                                                "
   "with some more ASCII stuff and followed by a remainder of ZERO bytes";

static TXHEBUF   txtHexEditBuf1 =
{
   txtHexDataBuf1,                              // start of data buffer
   0x03f7a000,                                  // abs start position (ref)
   TXT_HEX_ITEMSIZE,                            // size of buffer    (item)
   "LSN:0x00000001 = PSN:0x0181fbd0 = Cyl: 1673 H:1   S:2   Unidentified data"
};

static BYTE      txtHexDataBuf2[TXT_HEX_ITEMSIZE] =
   "The next initial buffer (NEXT) when starting the test           "
   "                                                                "
   "                                                                "
   "Some stuff in the middle                                        "
   "                                                                "
   "                                                                "
   "followed by a remainder of ZERO bytes";

static TXHEBUF   txtHexEditBuf2 =
{
   txtHexDataBuf2,                              // start of data buffer
   0x03f7a200,                                  // abs start position (ref)
   TXT_HEX_ITEMSIZE,                            // size of buffer    (item)
   "LSN:0x00000002 = PSN:0x0181fbd1 = Cyl: 1673 H:1   S:3   Unidentified data"
};

static BYTE      txtHexDataBufN[TXT_HEX_ITEMSIZE] =
   "The last initial buffer (NEXT +1) when starting the test        "
   "                                                                "
   "                                                                "
   "Some stuff in the middle                                        "
   "                                                                "
   "                                                                "
   "followed by a remainder of ZERO bytes";

static TXHEBUF   txtHexEditBufN =
{
   txtHexDataBufN,                              // start of data buffer
   0x03f7a400,                                  // abs start position (ref)
   TXT_HEX_ITEMSIZE,                            // size of buffer    (item)
   "LSN:0x00000003 = PSN:0x0181fbd2 = Cyl: 1673 H:1   S:4   Unidentified data"
};

static TXHEXEDIT txtHexEditData =
{
   0x40,                                        // offset in buffer, 1st byte
   0x23,                                        // offset in screen, cursor
   0,                                           // hex LSB nibble   (0 or 1)
   0,                                           // nr of rows shown
   0,                                           // nr of bytes per row
   TRUE,                                        // auto bytes/row from width
   FALSE,                                       // not in ASCII area
   FALSE,                                       // no decimal positions
   &txtHexEditBufP,                             // buffer BEFORE pref    one
   &txtHexEditBuf0,                             // buffer BEFORE current one
   &txtHexEditBuf1,                             // current buffer (item)
   &txtHexEditBuf2,                             // buffer AFTER  current one
   &txtHexEditBufN,                             // buffer AFTER  next    one
   NULL,                                        // difference buf (original)
   0,                                           // size of difference buff
   0,                                           // 32-bit buf CRC (original)
   0,0,0,                                       // mark start, size and base
   NULL,                                        // application user data
   txtHexEditAppCallback,                       // application action handler
   FALSE, ""                                    // Alternate display format info
};


/*****************************************************************************/
// Callback function to service HEXEDIT service requests
/*****************************************************************************/
static ULONG txtHexEditAppCallback
(
   ULONG               cmd,                     // IN    action command
   struct txhexedit   *dat                      // INOUT hexedit data
)
{
   ULONG               rc = 0;                  // function return
   TXHEBUF            *tmp;

   ENTER();
   TRACES(( "cmd: %lu\n"));

   switch (cmd)
   {
      case TXHE_CB_NEXT_BUF:
         tmp = dat->curr;
         dat->curr = (dat->next) ? dat->next : &txtHexEditBuf0;
         dat->next = (dat->nex2) ? dat->nex2 : &txtHexEditBufN;
         dat->nex2 = (dat->pre2) ? dat->pre2 : &txtHexEditBufP;
         dat->pre2 = (dat->prev) ? dat->pre2 : &txtHexEditBuf2;
         dat->prev = tmp;
         break;

      case TXHE_CB_PREV_BUF:
         tmp = dat->curr;
         dat->curr = (dat->prev) ? dat->prev : &txtHexEditBuf0;
         dat->prev = (dat->next) ? dat->next : &txtHexEditBuf2;
         dat->pre2 = (dat->nex2) ? dat->nex2 : &txtHexEditBufN;
         dat->nex2 = (dat->next) ? dat->next : &txtHexEditBufP;
         dat->next = tmp;
         break;

      case TXHE_CB_TO_START:
         dat->pre2 = NULL;
         dat->prev = &txtHexEditBufP;
         dat->curr = &txtHexEditBuf0;
         dat->next = &txtHexEditBuf1;
         dat->nex2 = &txtHexEditBuf2;
         dat->curr->start = 0;
         break;

      case TXHE_CB_TO_FINAL:
         dat->pre2 = &txtHexEditBuf0;
         dat->prev = &txtHexEditBuf1;
         dat->curr = &txtHexEditBuf2;
         dat->next = &txtHexEditBufN;
         dat->nex2 = NULL;
         dat->curr->start = 0x03f7a200;
         break;

      default:
         break;
   }
   RETURN (rc);
}                                               // end 'txtHexEditAppCallback'
/*---------------------------------------------------------------------------*/



int main (int argc, char *argv[]);

/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = NO_ERROR;           // function return
   char               *exename = argv[0];       // save before INITmain

   TxINITmain( TXT_TRACE, "TXT", FALSE, FALSE, 0); // TX Init code, incl tracing
                                                // argv/argc modified if TRACE

   if (TxaExeSwitch('l'))                       // start logfile now ?
   {
      TxAppendToLogFile( TxaExeSwitchStr( 'l', "txt Logfile", "TXTest"), TRUE);
   }
   if (TxaExeSwitch('?'))                       // switch help requested
   {
      TxPrint( "\nUsage: %s ", exename);
      TxShowTxt( switchhelp);
   }
   else
   {
      TXLN          ar;                         // arguments

      strcpy( ar,  TXT_STARTCMD);               // default command

      if (TxaExeSwitch('Q'))
      {
         txta->autoquit = TRUE;                 // auto quit after cmd
         txta->nowindow = TRUE;                 // default no win on autoquit
      }

      if (TxaExeSwitch('b'))
      {
         txta->batch     = TRUE;                // set global force on
      }
      separator          = TxaExeSwitchStr(    's', "SeparatorCh", "#");
      txta->eStrategy    = TxaErrorStrategy(   'E', txta->batch);
      txta->verbosity    = TxaOutputVerbosity( 'O');

      if (rc == NO_ERROR)
      {
         if (TxaExeArgc() > 1)                  // arguments given
         {
            char    qstring[3] = "?q";

            qstring[0] = separator[0];          // use current separator

            TxaGetArgString( TXA_1ST, 1, TXA_ALL, TXMAXLN, ar);
            if (strstr( ar, qstring) != NULL)
            {
               txta->nowindow = TRUE;           // don't use windowing
            }
            else if ((TxaExeArgc() > 1)  &&
                     (strncasecmp( TxaExeArgv(1), "nonag", 5) == 0))
            {
               if (strncasecmp( TxaExeArgv(1), "query", 5) == 0)
               {
                  txta->regconfirm = FALSE;     // no reg-nagging for now ...
               }
               if (!TxaExeSwitchUnSet('Q'))     // no -Q-
               {
                  txta->nowindow = TRUE;        // don't use windowing
                  txta->autoquit = TRUE;        // auto quit after cmd
               }
            }
         }

         if (TxaExeSwitchSet('w'))              // explicit windowing setting
         {
            if (TxaOption('w'))                 // set windowing on
            {
               txta->nowindow = FALSE;          // use windowing
            }
            else
            {
               txta->nowindow = TRUE;           // don't use windowing
            }
         }
         do
         {
            if (txta->nowindow)
            {
               txta->dialogs = FALSE;           // dialogs not supported
               txtInteractive( ar);             // non windowed interface
            }
            else
            {
               txta->dialogs = !TxaExeSwitchUnSet('P');
               txtWindowed(    ar);             // windowed interface
            }
            strcpy( ar, "display");             // new initial command

         } while (txta->retc == TX_DISPLAY_CHANGE);

         rc = txta->retc;
//       TxrRegistrationInfo( TXREG_CONFIRM_ONLY, TXT_V, txta->regconfirm);
      }
   }
   TxEXITmain(rc);                              // TX Exit code, incl tracing
}                                               // end 'main'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Print TXTst logo+status, do startup checks, run startup commands + profile
/*****************************************************************************/
ULONG txtStartupLogo                            // RET   Checks and firstcmd RC
(
   char               *firstcmd                 // IN    initial command
)
{
   ULONG               rc = NO_ERROR;
   FILE               *fp;
   TXTM                osd;                     // OS description string
   TXLN                msg;

   ENTER();
   TRACES(("Startup command is '%s'\n", firstcmd));

   TxPrint(  "\n  TxWindows Test program; version %s  %s\n", TXT_V, TXT_C);
   TxPrint(  " อออออออออออออออออออออออออออ[ www.dfsee.com"
             " ]ออออออออออออออออออออออออออออออออออ\n\n");

   if (TxaExeSwitch('S'))
   {
      TxPrint(  "%16.16sRun as SHELL  %s-S%s, quit and <F3> are disabled\n\n",
                "", CBC, CNN);
   }

   (void) TxOsVersion( osd);                    // get/check OS version info
   strcpy( msg, "");
   #if   defined (WIN32)
      if (strstr( osd, "Windows-9") != NULL)    // Win-9x detected
      {
         strcpy( msg, "Windows-9x is not supported by this "
                      "version of TXTst!  Use TXTDOS.EXE after "
                      "'Restart in MS-DOS mode' or use a DOS bootdisk.");
      }
   #elif defined (DOS32)
      if (strstr( osd, "DosBox") != NULL)       // Win or OS/2 DosBox (VDM)
      {
         if (strstr( osd, "Windows-9") != NULL) // Win-9x detected
         {
            txta->win9x = TRUE;

            strcpy( msg, "Running TXTDOS.EXE in a Win9x DosBox might "
                         "give unpredictable results!  Use TXTDOS.EXE after "
                         "'Restart in MS-DOS mode' or use a DOS bootdisk.");
         }
         else
         {
            sprintf( msg, "Running TXTDOS.EXE in a virtual DosBox "
                          "might give unpredictable results! Please use "
                          "the native version TXT%s.EXE when possible.",
                           (strstr( osd, "Win")) ? "WIN" : "OS2");
         }
      }
      else if (strstr( osd, "MemMgr") != NULL)  // EMM386 or other manager
      {
         strcpy( msg, "Running TXTDOS.EXE under a Memory Manager like "
                      "EMM386 or QEMM might give unpredictable results! "
                      "For maximum reliability, run without any external "
                      "memory manager (check autoexec.bat and config.sys).");
      }
   #else
      //- OS2 and LINUX versions
   #endif

   txtBEGINWORK();
   if (strlen( msg) != 0)                       // OS versus TXTst mismatch
   {
      if (txta->batch)                          // just show to screen/log
      {
         strcat( msg, "\n\n");
         TxMessage( FALSE, 5004, msg);
      }
      else                                      // interactive, confirm
      {
         strcat( msg, "\n\nDo you want to take the risk and continue with the program ? [Y/N]: ");
         if (!TxConfirm( 5004, msg))
         {
            rc = TXT_QUIT;
         }
      }
   }

   if (rc == NO_ERROR)                          // not aborted yet ...
   {
      if ((fp = fopen( TXT_PROFILE, "r")) != NULL)
      {
         TXLN             command;

         fclose( fp);
         sprintf( command, "run %s", TXT_PROFILE);
         txtMultiCommand( command, 0, TRUE, FALSE, FALSE); // exec profile
      }

      if (strlen( firstcmd))
      {
         if (strstr( firstcmd, "about") == NULL) // no about in command
         {
            txtMultiCommand( "about -c- -P-", 0, FALSE, FALSE, FALSE);
         }
         rc = txtMultiCommand( firstcmd, 0, TRUE, FALSE, FALSE); // execute command
      }
   }
   txtENDWORK();
   RETURN (rc);
}                                               // end 'txtStartupLogo'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Use classic text mode and accept interactive display commands
/*****************************************************************************/
static ULONG txtInteractive
(
   char               *cmdstring                // IN    initial command
)
{
   ULONG               rc;
   TXLN                command;                 // txt command

   ENTER();

   rc = txtStartupLogo( cmdstring);
   while ((rc != TXT_QUIT) && !txta->autoquit)
   {
      TxPrint("%s; cmd/SN : ",  TXT_N);
      fflush( stdout);
      memset( command, 0, TXMAXLN);
      fgets(  command, TXMAXLN, stdin);         // get next command
      TxRepl( command, '\n', 0);
      TxCancelAbort();                          // reset pending abort status
      rc = txtMultiCommand( command, 0, TRUE, TRUE, FALSE);
   }
   RETURN ( txta->retc);
}                                               // end 'txtInteractive'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Execute multiple txt-commands separated by # characters
/*****************************************************************************/
ULONG txtMultiCommand
(
   char               *cmdstring,               // IN    multiple command
   ULONG               lnr,                     // IN    linenumber or 0
   BOOL                echo,                    // IN    Echo  before execute
   BOOL                prompt,                  // IN    prompt after execute
   BOOL                quiet                    // IN    screen-off during cmd
)
{
   ULONG               rc = NO_ERROR;
   TXLN                resolved;                // parse copy, expandable
   TXTM                errmsg;                  // parse message
   char               *nextcmd;                 // next of multiple commands
   char               *command;                 // txt command
   int                 ln;

   ENTER();

   txtBEGINWORK();                              // signal work starting
   TRACES(("Worklevel: %lu Cmdstring: '%s'\n", txta->worklevel, cmdstring));
   if (strlen( cmdstring) != 0)                 // replace empty by space
   {
      for ( command  = cmdstring;
           (command != NULL) && !TxAbort();     // abort from long multi
            command  = nextcmd)                 // cmds must be possible!
      {
         if ((nextcmd = strchr( command, separator[0])) != NULL)
         {
            ln = nextcmd - command;
            nextcmd++;                          // skip the separator
         }
         else                                   // use whole string
         {
            ln = strlen( command);
         }
         strcpy( resolved, "");                 // start empty
         strncat(resolved, command, ln);        // concat this command
         TRACES(("Command: '%s'  Nextcmd: '%s'\n", resolved, (nextcmd) ? nextcmd : ""));

         if ((rc = txsResolveExpressions( resolved - (command - cmdstring), lnr, FALSE,
                                          resolved, TXMAXLN, errmsg)) == TX_PENDING)
         {
            TxCancelAbort();                    // reset pending abort status
            rc = txtSingleCommand( resolved, echo, quiet);
         }
         else if (rc != NO_ERROR)
         {
            TxPrint("\nRC: %lu, %s\n", rc, errmsg);
         }
      }
   }
   else
   {
      rc = txtSingleCommand( " ", TRUE, FALSE); // default cmd, <ENTER>
   }
   txtENDWORK();                                // signal work done
   if ((rc == TXT_QUIT) && (TxaExeSwitch('S')))
   {
      TxPrint( "\nTXTst is running in SHELL mode, quit is not allowed ...\n");
      rc = NO_ERROR;
   }
   RETURN (rc);
}                                               // end 'txtMultiCommand'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Interpret and execute DHPFS command;
/*****************************************************************************/
static ULONG txtSingleCommand
(
   char               *txtcmd,                  // IN    TXT command
   BOOL                echo,                    // IN    Echo command
   BOOL                quiet                    // IN    screen-off during cmd
)
{
   ULONG               rc;
   TXLN                dc;
   int                 cc = 0;                  // command string count
   char               *c0, *c1, *c2, *c3, *c4;  // parsed command parts
   TXLN                s0, s1;                  // temporary string space
   char               *pp;                      // parameter pointers
   USHORT              ml = 0;                  // mixed string length
   USHORT              cp = 0;                  // cursor position
   ULONG               size;
   ULONG               count;
   FILE               *fi;
   int                 fh;                      // low-level file handle
   TXHANDLE            ph = 0;                  // parse handle
   BOOL                l_force   = FALSE;       // local batch-mode used
   BOOL                l_dialogs = FALSE;       // local dialogs set
   BOOL                v_dialogs = FALSE;       // saved dialogs copy
   BOOL                l_errst   = FALSE;       // local error strategy
   int                 v_errst   = FALSE;       // saved copy
   BOOL                l_verbo   = FALSE;       // local verbosity
   ULONG               v_verbo   = FALSE;       // saved copy
   DEVICE_STATE        v_screen;                // saved copy screen state

   ENTER();

   TxaParseCommandString( txtcmd, TRUE, NULL);  // parse, free-format
   pp = TxaGetArgString( TXA_CUR, 0, 0, TXMAXLN, dc); // dc = pp => cmd from arg 0
   cc = TxaArgCount( );                         // number of parameters
   c0 = TxaArgValue(0);
   c1 = TxaArgValue(1);
   c2 = TxaArgValue(2);
   c3 = TxaArgValue(3);
   c4 = TxaArgValue(4);

   for (pp = &(dc[0]) + strlen(c0); *pp == ' '; pp++) {};

   if (TxaOption('-'))                          // option --
   {
      TxPrint("Command  : '%s'\n", dc);
      TxaShowParsedCommand( TRUE);              // option diagnostic
   }
   if (TxaOption('Q'))                          // locally forced quiet
   {
      quiet = TRUE;                             // force input parameter
   }
   if (TxaOption('B'))                          // local batch mode
   {
      if (txta->batch == FALSE)                 // no global force yet
      {
         txta->batch  =  TRUE;                  // register force globally
         l_force = TRUE;                        // use local force
      }
   }
   if ((txta->batch) || (TxaOptSet('P')))       // batch or local -P option
   {
      v_dialogs    = txta->dialogs;             // save old value
      l_dialogs    = TRUE;                      // and signal restore

      txta->dialogs = (TxaOption('P') != 0);
   }
   if (TxaOptValue('E') != NULL)                // local error strategy
   {
      v_errst = txta->eStrategy;
      l_errst = TRUE;
      txta->eStrategy = TxaErrorStrategy(   'E', txta->batch);
   }
   if (TxaOptValue('O') != NULL)                // local output verbosity
   {
      v_verbo = txta->verbosity;
      l_verbo = TRUE;
      txta->verbosity = TxaOutputVerbosity( 'O');
   }

   TRACES(("batch: %lu   eStrategy: %u    Verbosity: %lu\n",
      txta->batch, txta->eStrategy, txta->verbosity));

   if ((echo == TRUE) && (strcasecmp(c0, "screen" ) != 0)
                      && (strcasecmp(c0, "say"    ) != 0))
   {
      if (!TxaExeSwitchUnSet('e'))              // no surpress echo ?
      {
         TxPrint("%s%s version : %4.4s executing: %s%s%s\n",
                  (quiet)   ? "" : "\n",  TXT_N, TXT_V, CBG,
                  (cc == 0) ? "<Enter>" : txtcmd, CNN);
      }

      if ( (TxaExeSwitch('t') || TxaExeSwitch('q') || TxaExeSwitch('l')) &&
          (!TxaExeSwitchUnSet('t')))            // no surpress timestamp ?
      {
         time_t           tt = time( &tt);      // current date/time

         strftime( s0, TXMAXLN, "%A %d-%m-%Y %H:%M:%S", localtime( &tt));
         TxPrint( "Execute timestamp : %s\n", s0);
      }
   }

   v_screen = TxScreenState( DEVICE_TEST);
   if (quiet)
   {
      TxScreenState( DEVICE_OFF);
   }

   TRACES(("cc: %u c0:'%s' c1:'%s' c2:'%s' c3:'%s' c4:'%s' pp:'%s'\n",
            cc,    c0,     c1,     c2,     c3,     c4,     pp));

   rc = TxStdCommand();
   if ((rc == TX_PENDING) || (rc == TX_CMD_UNKNOWN))
   {
      rc = NO_ERROR;
      if      (strcasecmp(c0, "sep"      ) == 0)
      {
         if (cc > 1)
         {
            separator[0] = c1[0];
         }
      }
      else if (strcasecmp(c0, "menu"      ) == 0)
      {
         if (TxaOption('?') || (c1[0] == '?'))  // explicit help request
         {
            TxPrint("\nActivate the menu with default or selected pulldown\n");
            TxPrint("\n Usage:  %s  [pulldown-select-letter]\n\n"
                      "                f = File\n"
                      "                t = Test\n"
                      "                d = Display\n"
                      "                s = Settings\n"
                      "                h = Help\n", c0);
         }
         else
         {
            if (txta->automenu)
            {
               txta->menuOwner = TXHWND_DESKTOP;
            }
            txta->menuopen = (ULONG) c1[0];
         }
      }
      else if (strcasecmp(c0, "about"     ) == 0)
      {
         BOOL          plaintext;
         TX1K          about;                   // about text
         TXLN          text;                    // one line of text
         TXTS          alead;                   // leader text

         plaintext = (TxaOptUnSet('P') || (!txwIsWindow( TXHWND_DESKTOP)));
         strcpy( alead, (plaintext) ? "     " : "");
         if (!TxaOptUnSet('c'))                 // show program copyright ?
         {
            sprintf( about, "%s               Details on this "
                            "Fsys Software program\n\n%s   %s : %s %s\n",
                             alead, alead, TXT_N, TXT_V, TXT_C);
         }
         else
         {
            strcpy( about, "");
         }
         sprintf( text,  "%sUI TxWindows : %s\n", alead, txVersionString());
         strcat( about, text);
         sprintf( text,  "%s'C' compiler : ",     alead);
         strcat( about, text);
         #if defined (__WATCOMC__)
            if (__WATCOMC__ > 1100)             // must be OpenWatcom
            {
               sprintf( text, "OpenWatcom %4.2lf (c) 1988-2004: "
                               "Sybase & openwatcom.org\n",
                              ((double) ( __WATCOMC__ - 1100)) / 100);
            }
            else
            {
               sprintf( text, "Watcom C/C++ version : %4.2lf\n",
                              ((double) ( __WATCOMC__ )) / 100);
            }
         #elif defined (DARWIN)
               sprintf( text, "GNU  gcc  4.0.1 for MAC OS X : Apple computer, inc\n");
         #else
            #if defined (DEV32)
               sprintf( text, "VisualAge  3.65 (c) 1991-1997: IBM Corporation\n");
            #else
               sprintf( text, "Visual C++ 5.0  (c) 1986-1997: Microsoft Corporation\n");
            #endif
         #endif
         strcat( about, text);
         #if !defined (DARWIN)
            sprintf( text, "%sEXE compress : ", alead);
            strcat( about, text);
            #if defined (DEV32)
               sprintf( text,    "lxLite     1.33 (c) 1996-2003: Max Alekseyev\n");
            #else
               sprintf( text,    "UPX        1.20 (c) 1996-2002: Markus Oberhumer\n");
            #endif
            strcat( about, text);
         #endif
         #if defined (DOS32)
            sprintf( text,  "%sDOS extender : %s\n", alead, txDosExtVersion());
            strcat(  about, text);
         #endif
         (void) TxOsVersion( s0);               // Get operating system version
         sprintf( text,  "%sOS   version : %s\n", alead, s0);
         strcat( about, text);
         #if defined (DOS32)
            sprintf( text,  "%sDPMI version : %s\n", alead, txDosExtDpmiInfo());
            strcat(  about, text);
         #endif
         strcat( about, "\n");

         TxMessage( !(txta->batch || plaintext), 5003, about);
      }
      else if (strcasecmp(c0, "display") == 0)
      {
         TxPrint( "\nTXTst display size: %hu colums x %hu rows\n",
                   TxScreenCols(), TxScreenRows());

         #if defined (WIN32)
            txwDisplayConsoleInfo();
         #endif
      }
      else if (strcasecmp(c0, "83"    ) == 0)
      {
         count = TxMake8dot3( c1, c2);
         TxPrint( "Org length %3.3lu : '%s'\n", (ULONG) strlen(c1), c1);
         TxPrint( "8.3 length %3.3lu : '%s'\n", count, c2);
      }
      else if (strcasecmp(c0, "truename"    ) == 0)
      {
         rc = TxTrueNameDir( c1, TRUE, s0);
         TxPrint( "True DIR  rc:%3lu for : '%s'\n",   rc, s0);
         rc = TxTrueNameDir( c1, FALSE, s0);
         TxPrint( "True name rc:%3lu for : '%s'\n\n", rc, s0);
      }
      else if (strcasecmp(c0, "opt"  ) == 0)
      {
         TXA_OPTION         *o;

         if ((o = TxaOptValue(c1[0])) != NULL)
         {
            TxPrint("\n");
            if (o->type == TXA_NUMBER)
            {
               TxPrint("Option %c number val : %lu = 0x%8.8lx\n",
                        c1[0], o->value.number, o->value.number);
            }
            else                                // string
            {
               TxPrint("Option %c string val : '%s'\n",
                        c1[0], o->value.string);
            }
         }
         TxPrint("Option %c setting is : %s\n", c1[0],
                  TxaOption(c1[0]) ? "YES" : "NO");
      }
      else if ((strcasecmp(c0, "parse"  ) == 0) ||
               (strcasecmp(c0, "pitem"  ) == 0) ||
               (strcasecmp(c0, "pspace" ) == 0) ||
               (strcasecmp(c0, "pfree"  ) == 0)  )
      {
         if (strcasecmp( c0, "pfree") == 0)
         {
            rc = TxaParseCommandString( (char *)     pp,
                                        (BOOL)       TRUE,
                                        (TXHANDLE *) &ph);
         }
         else if (strcasecmp( c0, "pitem") == 0)
         {
            rc = TxaSetItem( TXA_CUR, (char *) pp);
         }
         else if (strcasecmp( c0, "pspace") == 0)
         {
            strcpy( s0, " ");

            rc = TxaParseCommandString( s0, FALSE,  &ph);
         }
         else
         {
            rc = TxaParseCommandString( pp, FALSE, &ph);
         }
         if (rc == NO_ERROR)
         {
            TxaShowParsedCommand( FALSE);
         }
         else
         {
            TxPrint( "\nError %lu from command parser\n", rc);
         }
      }
      else if (strcasecmp(c0, "pdrop"  ) == 0)
      {
         TxaDropParsedCommand( cc > 1);
      }
      else if (strcasecmp(c0, "pshow"  ) == 0)
      {
         TxaShowParsedCommand( cc > 1);
      }
      else if (strcasecmp(c0, "mk"    ) == 0)
      {
         strcpy( s1, c1);
         count = TxMakePath( s1);
         TxPrint( "MakePath RC %3.3lu : '%s'\n", count, s1);
      }
      else if (strcasecmp(c0, "set"    ) == 0)
      {
         TxPrint( "TXTest   SET cmds : none\n");
      }
      else if (strcasecmp(c0, "vol"    ) == 0)
      {
         TxFsDrivemap( "Free on volume:  ", (cc > 1));
      }
      else if (strcasecmp(c0, "hexdump"  ) == 0)
      {
         if (cc > 1)
         {
            size = atol( c1);
         }
         else
         {
            size = 256;
         }
         TxDisplHexDump( (char *) cmdhelptxt, size);
      }
      else if (strcasecmp(c0, "alloc"  ) == 0)
      {
         char         *m = NULL;

         if (cc > 1)
         {
            size = atol( c1);
            if (!TxaOption('k'))
            {
               size *= 1024;                    // make it Mb
            }
            m = (char *) TxAlloc( size, 1024);

            TxPrint( "Allocating %lu Kb, result: %8.8lx\n", size, m);

            TxFreeMem( m);
         }
         else
         {
            TxPrint( "\nUsage: alloc  size   [-k]       (default is Mb)\n");
         }
      }
      else if (strcasecmp(c0, "boot"    ) == 0)
      {
         TxReboot( TRUE);
      }
      else if (strcasecmp(c0, "sizeof"    ) == 0)
      {
         TxPrint( "Size of:  int     = %u\n", sizeof( int    ));
         TxPrint( "Size of:  char    = %u\n", sizeof( char   ));
         TxPrint( "Size of:  ptr     = %u\n", sizeof( void * ));
         TxPrint( "Size of:  LLONG   = %u\n", sizeof( LLONG  ));
         TxPrint( "Size of:  ULONG   = %u\n", sizeof( ULONG  ));
         TxPrint( "Size of:  USHORT  = %u\n", sizeof( USHORT ));
         TxPrint( "Size of:  BYTE    = %u\n", sizeof( BYTE   ));
      }
      else if (strcasecmp(c0, "lines"    ) == 0)
      {
         if (cc > 1)
         {
            size = atol( c1);
         }
         else
         {
            size = txta->sbsize;
         }
         for (count = 0; count < size; count++)
         {
            TxPrint( "Scrollbuf testline nr: %3lu '%s'\n", count, c2);
         }
      }
      else if (strcasecmp(c0, "sbnum"    ) == 0)
      {
         if (txta->sbwindow != 0)
         {
            size = txta->sbsize;
            for (count = 0; count < size; count++)
            {
               sprintf( s0, "%4.4lu", count);

               txta->sbbuf[count * txta->sblwidth + 66].ch = s0[0];
               txta->sbbuf[count * txta->sblwidth + 67].ch = s0[1];
               txta->sbbuf[count * txta->sblwidth + 68].ch = s0[2];
               txta->sbbuf[count * txta->sblwidth + 69].ch = s0[3];

               txta->sbbuf[count * txta->sblwidth + 66].at = 0x5e;
               txta->sbbuf[count * txta->sblwidth + 67].at = 0x5e;
               txta->sbbuf[count * txta->sblwidth + 68].at = 0x5e;
               txta->sbbuf[count * txta->sblwidth + 69].at = 0x5e;
            }
            txwInvalidateWindow( txta->sbwindow, TRUE, TRUE);
         }
         else
         {
            TxPrint("Windowing/scrollbuffer not active\n");
         }
      }
      else if (strcasecmp(c0, "ansi"    ) == 0)
      {
         ml = TxCursorCol();
         printf("X%sX", CNN);
         fflush(stdout);
         cp = TxCursorCol();
         printf("Pre-cursor pos: %u, Post-cursor-pos: %u\n", ml, cp);
         if ((ml+2) == cp)
         {
            TxPrint("ANSI present!\n");
         }
         else
         {
            TxPrint("ANSI not present!\n");
         }
      }
      else if (strcasecmp(c0, "cursor"    ) == 0)
      {
         TxPrint("Cursor ");
         if (cc >= 2)
         {
            TxSetCursorPos( atoi(c1), atoi(c2));
         }
         ml = TxCursorCol();
         cp = TxCursorRow();
         TxPrint("col, row: %u, %u\n", ml, cp);
      }
      else if (strcasecmp(c0, "fopen"    ) == 0)
      {
         if ((fi = fopen( c1, "rb")) != NULL)
         {
            TxPrint("'%s' successfully opened, fi = %p\n", c1, fi);
            fclose( fi);
         }
         else
         {
            TxPrint("'%s' cannot be opened with 'fopen'...\n", c1);
         }
      }
      else if (strcasecmp(c0, "open"    ) == 0)
      {
         if ((fh = open( c1, O_RDONLY, 0)) != -1)
         {
            TxPrint("'%s' successfully opened, fh = %u\n", c1, fh);
            close( fh);
         }
         else
         {
            TxPrint("'%s' cannot be opened with 'open'...\n", c1);
         }
      }
      else if (strcasecmp(c0, "fpath"    ) == 0)
      {
         if (TxFindByPath( c2, c1, c4) != NULL)
         {
            TxPrint( "Found '%s' using '%s' in '%s'\n", c2, c1, c4);
         }
      }
      else if (strcasecmp(c0, "vols"   ) == 0)
      {
         if (cc > 1)
         {
            TxFsVolumes( TRUE, s1);
         }
         else
         {
            TxFsVolumes( FALSE, s1);
         }
         TxPrint("Present volumes: '%s'\n", s1);
      }
      else if (strcasecmp(c0, "dlg"   ) == 0)
      {
         TxPrint( "Starting test dialog from commandline ...\n");
         rc = txtTestDialog();
         TxPrint( "Dialog result code: %lu\n", rc);
      }
      else if (strcasecmp(c0, "hexed"   ) == 0)
      {
         TxPrint( "Starting hex-edit dialog from commandline ...\n");
         txtHexEditData.rows = 0;
         txtHexEditData.cols = 0;
         txtHexEditData.rows = (USHORT) atoi( c1);
         txtHexEditData.cols = (USHORT) atoi( c2);
         rc = txwHexEditor( TXHWND_DESKTOP, TXHWND_DESKTOP,
                            &txtHexEditData,
                            " Partition 03 type:07 D: HPFS   size: 1114.8 MiB ",
                            0, TXHE_MOVEABLE);
         TxPrint( "Dialog result code: %lu\n", rc);
      }
      else if (strcasecmp(c0, "listbox"   ) == 0)
      {
         TxPrint( "Starting ListBox dialog from commandline ...\n");
         rc = txtListBoxDialog();
         TxPrint( "Dialog result code: %lu\n", rc);
      }
      else if (strcasecmp(c0, "conf"   ) == 0)
      {
         TxPrint( "Starting confirmation from commandline ...\n");
         if (TxConfirm( 203, "%s", pp) == TRUE)
         {
            TxPrint( "YES, Confirmed!\n");
         }
         else
         {
            TxPrint( "NO, Rejected%s!\n", (TxAbort()) ? " and Aborted" : "" );
         }
      }
      else if (strcasecmp(c0, "msgbox"   ) == 0)
      {
         TxPrint( "Starting MessageBox dialog from commandline ...\n");
         if (cc > 2)
         {
            TxRepl( c2, '_', ' ' );
            TxRepl( c2, '|', '\n');
         }
         rc = txwMessageBox( TXHWND_DESKTOP, TXHWND_DESKTOP,
                             (cc > 2) ? c2 : txtMsgText,
                             (cc > 1) ? c1 : txtMsgTitle,
                             (cc > 3) ? (ULONG) atol(c3) : 201,
                            ((cc > 4) ? (ULONG) atol(c4) : 4) |
                                             TXMB_MOVEABLE);
         TxPrint( "MessageBox result code: %lu\n", rc);
      }
      else if (strcasecmp(c0, "prompt" ) == 0)
      {
         strcpy( s1, c1);
         TxPrompt( 0, 40, s1, "Input : %s %s %s", c2, c3, c4);
         TxPrint( "Prompted value is : '%s'\n", s1);
      }
      else if (strcasecmp(c0, "pbox"   ) == 0)
      {
         TxPrint( "Starting PromptBox dialog from commandline ...\n");
         if (cc > 2)
         {
            TxRepl( c2, '_', ' ' );
            TxRepl( c2, '|', '\n');
         }
         strcpy( s1, "default value");
         rc = txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL,
                             (cc > 2) ? c2 : txtPromptText,
                             (cc > 1) ? c1 : txtPromptTitle,
                             (cc > 3) ? (ULONG) atol(c3) : 201,
                            ((cc > 4) ? (ULONG) atol(c4) : 4) |
                                             TXMB_MOVEABLE,
                                             TXMAXLN, s1);
         TxPrint( "PromptBox result code: %lu\n, value: '%s'\n", rc, s1);
      }
      else if (strcasecmp(c0, "hwin"     ) == 0)
      {
         txwHelpDialog( (ULONG) atol(c1), TXWH_REQ_NONE, NULL);
      }
      else if (strcasecmp(c0, "help"     ) == 0)
      {
         TxShowTxt( cmdhelptxt);
         TxPrint(  " %s: %s %s\n", TXT_N, txVersionString(), TXT_C);
      }
      else if (strcasecmp(c0, "log"      ) == 0)
      {
         TxAppendToLogFile( c1, TRUE);
      }
      #if defined (DUMP)
      else if (strcasecmp(c0, "trace"    ) == 0)
      {
         TxTrLevel = atol(c1);
         TxPrint("Debug detail level: %lu\n", TxTrLevel);
      }
      #endif
      else if (strcasecmp(c0, "run"    ) == 0)  // RUN frontend, prompt for
      {                                         // name & params if needed
         if (TxaOption('?') || (c1[0] == '?'))  // explicit help request
         {
            rc = TxsNativeRun( NULL, NULL);     // get usage for RUN
         }
         else
         {
            #if defined (USEWINDOWING)
            if (((cc == 1) || TxaOption('P')) && // filedialog prompt for script
                (txwIsWindow( TXHWND_DESKTOP) )) // only when windowed ...
            {
               char    *path = NULL;

               strcpy( s0, "*");                // default wildcard, all files
               if (cc > 1)                      // path or wildcard specified
               {
                  if (TxStrWcnt(c1) == strlen(c1)) // no wildcard, must be path
                  {
                     path = c1;                 // use for filedialog path
                  }
                  else
                  {
                     strcpy( s0, c1);           // use for filedialog wildcard
                  }
               }
               sprintf( s1, "%s.dfs", s0);
               #if defined (DEV32)
                  if (strcmp(s0, "*") == 0)     // to be refined, DLG wildcard
                  {                             // does not work with TWO ...
                     strcat( s1, ";");          // unless it is a single *
                     strcat( s1, s0 );
                     strcat( s1, ".cmd");       // add REXX scripts for OS/2
                  }
               #endif

               strcpy( s0, "");
               if (txwOpenFileDialog( s1, path, NULL, 0, NULL,
                    " Select TX script file to RUN ", s1))
               {
                  TXLN               descr;

                  sprintf( s0, "'%s'", s1);     // script name, may have spaces!
                  TxFnameExtension( s1, "txs");
                  TxsValidateScript( s1, NULL, s1, NULL); // get description
                  if (strstr( s1, "no-parameters") == NULL)
                  {
                     if (strlen( s1) != 0)
                     {
                        sprintf( descr, "%s\n\nParameters enclosed in [] are "
                                 "optional, others are mandatory.\n%s", s0, s1);
                     }
                     else
                     {
                        sprintf( descr, "%s\n\nSpecify additional parameters for "
                                 "the script or just leave as is ...", s0);
                     }
                     TxaGetArgString( TXA_CUR, 2, TXA_ALL, TXMAXLN, s1);
                     if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL,
                           descr,
                           " Specify parameter(s) for the script ", 0,
                           TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER,
                           50, s1) != TXDID_CANCEL)
                     {
                        strcat( s0, " ");
                        strcat( s0, s1);
                     }
                  }
               }
            }
            else                                // parameter scriptname ?
            #endif                              // USEWINDOWING
            {
               strcpy( s0, c1);
               strcpy( s1, "");
               if ((cc == 1) || TxaOption('P')) // prompt for script + params
               {
                  TxPrompt( 0, 40, s0, "Specify script to run plus parameters ...");
               }
            }
         }
         if (strlen(s0) > 0)                    // rebuild with name & params
         {
            TRACES(("Rebuild RUN with '%s' and '%s'\n", s0, s1));
            TxaGetArgString( TXA_CUR, 2, TXA_OPT, TXMAXLN, s1);
            sprintf( dc,   "runscript %s %s", s0, s1);
            rc = txtMultiCommand( dc, 0, TRUE, FALSE, TRUE);
         }
         if (TxaOption('q'))
         {
            rc = TXT_QUIT;
         }
      }
      else if ((strcasecmp(c0, "runscript") == 0)) // RUN, stage 2, reparsed
      {
         TXLN       scriptname;
         BOOL       isRexxScript;

         strcpy( s0, c1);                       // scriptname mandatory here!
         TxFnameExtension( s0, "txs");
         if (TxsValidateScript( s0, &isRexxScript, NULL, scriptname))
         {
            DEVICE_STATE screen = TxScreenState( DEVICE_TEST);

            if (isRexxScript)
            {
               TxPrint( "\nREXX scripts not supported in the TXT test program.\n");
            }
            else                                // TXS native script
            {
               rc = TxsNativeRun( scriptname, txtMultiCommand);
            }
            TxScreenState( screen);             // restore initial state
         }
         else
         {
            TxPrint( "TX script file : '%s' not found\n", s0);
            rc = TX_INVALID_FILE;
         }
      }
      #if defined (DEV32)
      else if (strcasecmp(c0, "CopyOutput"   ) == 0)
      {
         if (cc > 1)
         {
            if (!isalpha(c1[0]) || (strlen(c1)<2) || (c1[strlen(c1)-1] != '.'))
            {
               rc = 30000;                      // REXX variable error
            }
            strcpy( s1, c1);
         }
         else
         {
            strcpy( s1, "txt_output.");
         }
      }
      #endif
      else if ((strcasecmp(c0, "exit"     ) == 0) ||
               (strcasecmp(c0, "quit"     ) == 0) ||
               (strcasecmp(c0, "q"        ) == 0) )
      {
         rc = TXT_QUIT;
      }
      else if ((strcasecmp(c0, "help"     ) == 0) ||
               (strcasecmp(c0, "?"        ) == 0) )
      {
         TxShowTxt( cmdhelptxt);
         TxPrint(  " %s; %s  %s\n", TXT_N, txVersionString(), TXT_C);
      }
      else if (strstr( c0, "htst") != NULL)     // history test
      {
         TxPrint( "History test command: '%s' params: '%s'\n", c0, pp);
      }
      else
      {
         TxPrint("'%s' is not a TXT command,"
                 " execute externally ...\n", c0);
         rc = TxExternalCommand( dc);           // execute inputstring as cmd
      }
   }
   if (rc != TXT_QUIT)
   {
      txta->retc = rc;                          // set overall return-code
      if (rc == TX_DISPLAY_CHANGE)              // display mode has changed
      {
         if (TxaOptUnSet('w'))                  // non-windowed -w- ?
         {
            txta->nowindow = TRUE;
         }
         else                                   // default windowed
         {
            txta->nowindow = FALSE;
         }
         rc = TXT_QUIT;                         // quit command loop, restart
      }
   }
   TxaDropParsedCommand( FALSE);                // drop 1 level of parsed info
   if (l_force)                                 // did we apply local force ?
   {
      txta->batch = FALSE;                      // reset global force
   }
   if (l_dialogs)                               // local strategy used ?
   {
      txta->dialogs = v_dialogs;                // reset dialogs strategy
   }
   if (l_errst)                                 // local strategy used ?
   {
      txta->eStrategy = v_errst;                // reset error strategy
   }
   if (l_verbo)                                 // local strategy used ?
   {
      txta->verbosity = v_verbo;                // reset verbosity
   }
   if (quiet)                                   // local/overruled screen state
   {
      TxScreenState( v_screen);                 // reset explicit quiet
   }
   if (txta->sbwindow)                          // remove status text
   {
      txwSendMsg( txta->sbwindow, TXWM_STATUS, 0, TX_Yellow_on_Cyan);
   }
   RETURN (rc);
}                                               // end 'txtSingleCommand'
/*---------------------------------------------------------------------------*/

