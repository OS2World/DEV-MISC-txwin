// Scrollbuffer output, entryfield and menu sample
//
// Author: J. van Wijk
//

#include <txlib.h>                              // TX library interface

#include <sam8win.h>                            // windowed entry point
#include <sam8.h>                               // module navigation and defs

#if defined (DUMP)
#define SAM_TRACE      "SAMSTTRACE"             // Trace startup values
#endif

#define SAM_STARTCMD    "say Hello to the SAMPLE program"


SAMINF      SAM_anchor =
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

SAMINF       *sama  = &SAM_anchor;              // SAM anchor block

static  char *separator;

char *switchhelp[] =
{
   "  [global-sample-switches]  [SAMPLE-multi-command]",
   "",
#if defined (DUMP)
   " -123[t][s][n] = set trace level to 123, SAMPLE internal function trace;",
   "                 [t]imestamp lines; trace to [s]creen too; [n]o tracefile",
#endif
   "",
   " -?            = help on SAMPLE commandline switches (this text)",
   " -7            = Use 7-bit ASCII only (no 'graphic' characters)",
   " -a            = switch off usage of ANSI escape characters for color",
   " -b            = batch option, automatic 'batch on' command at startup",
   " -e   or  -e-  = Include or surpress (-e-) command echo before each cmd",
   " -f            = frame, use a border-frame on scroll-buffer and desktop",
   " -f-           = do not use border-frames. (default NO on 80x25 screen)",
   " -l:logfile    = start logging immediately to 'logfile.log'",
   " -menu         = automatic menu at startup and after each menu-selection",
   " -q            = quiet option, automatic 'screen off' command at startup",
   " -Q            = quit automatically after executing specified command",
   " -Q-           = do NOT quit automatically on normally autoquiting commands",
   " -S            = Shell mode, do not allow quit from SAMPLE (use as shell)",
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
   " For help on SAMPLE commands, use the '?' command and use the <F1>",
   " key when shown at bottom line",
   "",
   NULL
};


char               *cmdhelptxt[] =
{
   " ?            [*] = Show list of generic commands with short description",
   " cd        [path] = Change current directory an current drive",
   " display          = Show display size, rows and columns",
   " help         [*] = Show list of generic commands with short description",
   " log       [file] = Log (append) to 'file' (.log); (No file => stop logging)",
   " screen  [on|off] = Switch output to the screen on or off",
   " setansi [on|off] = Set use of ANSI escape-sequences (colors) on or off",
   " scrfile [fn] [l] = Save screen-buffer to file [fn], last [l] lines",
   " vol     [floppy] = Show all volumes, optional including floppies",
#if defined (DUMP)
   " trace [lvl]      = Set trace level for SAM internal function tracing",
#endif
#if defined (DEV32)
   " run macro        = Run a SAM macro in a .SAM file",
#else
#endif
   " q                = Quit",
   NULL
};


// Interpret and execute SAMPLE command;
static ULONG samSingleCommand
(
   char               *samcmd,                  // IN    SAMPLE command
   BOOL                echo,                    // IN    Echo command
   BOOL                quiet                    // IN    screen-off during cmd
);



int main (int argc, char *argv[]);

/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = NO_ERROR;           // function return
   char               *exename = argv[0];       // save before INITmain

   TxINITmain( SAM_TRACE, "SAM8", FALSE, FALSE, 0); // TX Init code, incl tracing
                                                // argv/argc modified if TRACE

   if (TxaExeSwitch('l'))                       // start logfile now ?
   {
      TxAppendToLogFile( TxaExeSwitchStr( 'l', NULL, "SAMple8"), TRUE);
   }
   if (TxaExeSwitch('?'))                       // switch help requested
   {
      TxPrint( "\nUsage: %s ", exename);
      TxShowTxt( switchhelp);
   }
   else
   {
      TXLN          ar;                         // arguments

      strcpy( ar,  SAM_STARTCMD);               // default command
      separator          = TxaExeSwitchStr(    's', "SeparatorCh", "#");
      if (rc == NO_ERROR)
      {
         samWindowed(    ar);             // windowed interface
      }
      rc = sama->retc;
   }
   TxEXITmain(rc);                              // SAM Exit code, incl tracing
}                                               // end 'main'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Print SAMPLE logo+status, do startup checks, run startup commands + profile
/*****************************************************************************/
ULONG samStartupLogo                            // RET   Checks and firstcmd RC
(
   char               *firstcmd                 // IN    initial command
)
{
   ULONG               rc = NO_ERROR;

   ENTER();
   TRACES(("Startup command is '%s'\n", firstcmd));

   TxPrint(  "\n  Scroll/Entry/Menu sample version %s  %s\n", SAM_V, SAM_C);
   TxPrint(  " อออออออออออออออออออออออออออ[ www.dfsee.com"
             " ]ออออออออออออออออออออออออออออออออออ\n\n");

   samBEGINWORK();
   if (strlen( firstcmd))
   {
      if (strstr( firstcmd, "about") == NULL) // no about in command
      {
         samMultiCommand( "about -c- -P-", 0, FALSE, FALSE, FALSE);
      }
      rc = samMultiCommand( firstcmd, 0, TRUE, FALSE, FALSE); // execute command
   }
   samENDWORK();
   RETURN (rc);
}                                               // end 'samStartupLogo'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Execute multiple sam-commands separated by # characters
/*****************************************************************************/
ULONG samMultiCommand
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
   char               *command;                 // sam command
   int                 ln;

   ENTER();

   samBEGINWORK();                              // signal work starting
   TRACES(("Worklevel: %lu Cmdstring: '%s'\n", sama->worklevel, cmdstring));
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
            rc = samSingleCommand( resolved, echo, quiet);
         }
         else if (rc != NO_ERROR)
         {
            TxPrint("\nRC: %lu, %s\n", rc, errmsg);
         }
      }
   }
   else
   {
      rc = samSingleCommand( " ", TRUE, FALSE); // default cmd, <ENTER>
   }
   samENDWORK();                                // signal work done
   if ((rc == SAM_QUIT) && (TxaExeSwitch('S')))
   {
      TxPrint( "\nSAMPLE is running in SHELL mode, quit is not allowed ...\n");
      rc = NO_ERROR;
   }
   RETURN (rc);
}                                               // end 'samMultiCommand'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Interpret and execute DHPFS command;
/*****************************************************************************/
static ULONG samSingleCommand
(
   char               *samcmd,                  // IN    SAM command
   BOOL                echo,                    // IN    Echo command
   BOOL                quiet                    // IN    screen-off during cmd
)
{
   ULONG               rc;
   TXLN                dc;
   int                 cc = 0;                  // command string count
   char               *c0, *c1, *c2, *c3, *c4;  // parsed command parts
   TXLN                s0;                      // temporary string space
   char               *pp;                      // parameter pointers
   BOOL                l_force   = FALSE;       // local batch-mode used
   BOOL                l_dialogs = FALSE;       // local dialogs set
   BOOL                v_dialogs = FALSE;       // saved dialogs copy
   BOOL                l_errst   = FALSE;       // local error strategy
   int                 v_errst   = FALSE;       // saved copy
   BOOL                l_verbo   = FALSE;       // local verbosity
   ULONG               v_verbo   = FALSE;       // saved copy
   DEVICE_STATE        v_screen;                // saved copy screen state

   ENTER();

   TxaParseCommandString( samcmd, TRUE, NULL);  // parse, free-format
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
      if (sama->batch == FALSE)                 // no global force yet
      {
         sama->batch  =  TRUE;                  // register force globally
         l_force = TRUE;                        // use local force
      }
   }
   if ((sama->batch) || (TxaOptSet('P')))       // batch or local -P option
   {
      v_dialogs    = sama->dialogs;             // save old value
      l_dialogs    = TRUE;                      // and signal restore

      sama->dialogs = (TxaOption('P') != 0);
   }
   if (TxaOptValue('E') != NULL)                // local error strategy
   {
      v_errst = sama->eStrategy;
      l_errst = TRUE;
      sama->eStrategy = TxaErrorStrategy(   'E', sama->batch);
   }
   if (TxaOptValue('O') != NULL)                // local output verbosity
   {
      v_verbo = sama->verbosity;
      l_verbo = TRUE;
      sama->verbosity = TxaOutputVerbosity( 'O');
   }

   TRACES(("batch: %lu   eStrategy: %u    Verbosity: %lu\n",
      sama->batch, sama->eStrategy, sama->verbosity));

   if ((echo == TRUE) && (strcasecmp(c0, "screen" ) != 0)
                      && (strcasecmp(c0, "say"    ) != 0))
   {
      if (!TxaExeSwitchUnSet('e'))              // no surpress echo ?
      {
         TxPrint("%s%s version : %4.4s executing: %s%s%s\n",
                  (quiet)   ? "" : "\n",  SAM_N, SAM_V, CBG,
                  (cc == 0) ? "<Enter>" : samcmd, CNN);
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
      if (strcasecmp(c0, "menu"      ) == 0)
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
            if (sama->automenu)
            {
               sama->menuOwner = TXHWND_DESKTOP;
            }
            sama->menuopen = (ULONG) c1[0];
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
                             alead, alead, SAM_N, SAM_V, SAM_C);
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

         TxMessage( !(sama->batch || plaintext), 5003, about);
      }
      else
      {
         TxPrint("'%s' is not a SAM command, execute externally ...\n", c0);
         rc = TxExternalCommand( dc);           // execute inputstring as cmd
      }
   }
   if (rc != SAM_QUIT)
   {
      sama->retc = rc;                          // set overall return-code
      if (rc == TX_DISPLAY_CHANGE)              // display mode has changed
      {
         if (TxaOptUnSet('w'))                  // non-windowed -w- ?
         {
            sama->nowindow = TRUE;
         }
         else                                   // default windowed
         {
            sama->nowindow = FALSE;
         }
         rc = SAM_QUIT;                         // quit command loop, restart
      }
   }
   TxaDropParsedCommand( FALSE);                // drop 1 level of parsed info
   if (l_force)                                 // did we apply local force ?
   {
      sama->batch = FALSE;                      // reset global force
   }
   if (l_dialogs)                               // local strategy used ?
   {
      sama->dialogs = v_dialogs;                // reset dialogs strategy
   }
   if (l_errst)                                 // local strategy used ?
   {
      sama->eStrategy = v_errst;                // reset error strategy
   }
   if (l_verbo)                                 // local strategy used ?
   {
      sama->verbosity = v_verbo;                // reset verbosity
   }
   if (quiet)                                   // local/overruled screen state
   {
      TxScreenState( v_screen);                 // reset explicit quiet
   }
   if (sama->sbwindow)                          // remove status text
   {
      txwSendMsg( sama->sbwindow, TXWM_STATUS, 0, TX_Yellow_on_Cyan);
   }
   RETURN (rc);
}                                               // end 'samSingleCommand'
/*---------------------------------------------------------------------------*/

