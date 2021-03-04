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
   FALSE,                                       // BASIC menu/dialog active (not EXPERT)
   FALSE,                                       // no automatic menu at start
   TRUE,                                        // automatic pulldown drop
   0,                                           // default drop-down menu
   0,                                           // worklevel
   FALSE,                                       // logAuto default OFF
   NULL,                                        // selection list, Color schemes
};

SAMINF       *sama  = &SAM_anchor;              // SAM anchor block

static  char *separator;

static char *usagehelp[] =
{
   "  [global-switches]  [multi-command]",
   "",
   "  Switch character for switches is '-' or '/'. All single letter",
   "  switches are case-sensitive, long switchnames like 'query' are not.",
   "",
   NULL
};

char *samSwitchhelp[] =
{
   "",
   "Sample program specific switches:",
   "================================",
   "",
   " -?            = help on Sample commandline switches (this text)",
   " -expert       = Set UI menus and dialogs to EXPERT mode on startup",
   " -expert-      = Set UI menus and dialogs to BASIC  mode on startup (default)",
   "",
   " For help use the '?' and '?\?' commands, use the <F1> key or read DFSCMDS.TXT",
   "",
   NULL
};


char               *samGenericHelp[] =
{
   " ?            [*] = Show list of generic commands with short description",
   " set [prop] [val] = Set various SAM8 properties, use 'set' for more help",
   " vol     [floppy] = Show all volumes, optional including floppies",
   " run macro        = Run a SAM macro in a .SAM file",
   " q                = Quit",
   NULL
};

char *hostvarhelp[] =
{
   "",
   "All variable names start with the '$' character. To embed a variable or",
   "expression in a command, enclose it in DOUBLE curly braces: {{expr}}",
   "",
   "Naming of user variables is free, except for names with the '$_' prefix",
   "which are reserved for system variables (like DFSee host variables) and:",
   "",
   "  $0 .. $9 which are reserved for argument passing into scripts.",
   "",
   "  $_rc     special host-variable, set automatically after every command,",
   "           when executed from a script, but can also be set manually with",
   "           a regular assignment like '$_rc = 500'",
   "",
   "           The return value from a script, either running to the end, or",
   "           with a RETURN statement will be the value of this $_rc variable.",
   "",
   "  $_retc   Always set by app itself after executing a command, so it can",
   "           be used from the commandline too (outside a script).",
   "",
   "           Displaying these variables from a script is best done using",
   "           a 'PRINT' statement since a 'say' command resets them to 0.",
   "",
   "           For reserved rc values (constant definitions), see further below.",
   "",
   "",
   " Constant values defined with TXwin:",
   "",
   " true                    1          Logical values",
   " false                   0",
   "",
   " rc_ok                   0          $_rc and $_retc 'OK'",
   "",
   " rc_file_not_found       2          Generic OS RC values",
   " rc_path_not_found       3",
   " rc_too_many_files       4",
   " rc_access_denied        5",
   " rc_invalid_handle       6",
   " rc_no_more_files       18",
   " rc_write_protect       19",
   " rc_not_ready           21",
   " rc_crc                 23",
   " rc_seek                25",
   " rc_sector_not_found    27",
   " rc_write_fault         29",
   " rc_read_fault          30",
   " rc_gen_failure         31",
   " rc_file_sharing        32",
   " rc_lock_violation      33",
   " rc_wrong_disk          34",
   "",
   " rc_error              200          TXwin specific RC values",
   " rc_invalid_file       202",
   " rc_invalid_path       203",
   " rc_access_denied      205",
   " rc_invalid_handle     206",
   " rc_invalid_data       207",
   " rc_alloc_error        208",
   " rc_syntax_error       210",
   " rc_invalid_drive      215",
   " rc_pending            217",
   " rc_failed             218",
   " rc_write_protect      219",
   " rc_cmd_unknown        222",
   " rc_no_compress        223",
   " rc_no_initialize      224",
   " rc_aborted            225",
   " rc_bad_option_char    226",
   " rc_too_many_args      227",
   " rc_display_change     228",
   " rc_app_quit           229",
   NULL
};



// Interpret and execute SAMPLE command;
static ULONG samSingleCommand
(
   char               *samcmd,                  // IN    SAMPLE command
   BOOL                echo,                    // IN    Echo command
   BOOL                quiet                    // IN    screen-off during cmd
);

// Set extra long names usable as switch or command option
static void samSetLongSwitchNames
(
   void
);


int main (int argc, char *argv[]);

/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = NO_ERROR;           // function return
   TXA_OPTION         *opt;
   TXLN                fileName;

   TxINITmain( SAM_TRACE, "SAM8", FALSE, FALSE, samSetLongSwitchNames); // TX Init code, incl tracing
                                                // argv/argc modified if TRACE

   if (((opt = TxaOptValue('l')) != NULL) && (opt->type == TXA_STRING)) // start named logfile now ?
   {
      TxaExeSwAsString( 'l', TXMAXLN, fileName);
      TxRepl( fileName, FS_PALT_SEP, FS_PATH_SEP); // fixup ALT separators
      TxAppendToLogFile( fileName, TRUE);
   }
   if (TxaExeSwitchSet(SAM_O_LOGAUTO))          // when default overruled
   {
      sama->logAuto = TxaExeSwitch( SAM_O_LOGAUTO);
   }
   if (TxaExeSwitchSet(SAM_O_EXPERT))           // when default overruled
   {
      sama->expertui = TxaExeSwitch( SAM_O_EXPERT);
   }
   if (TxaExeSwitch('?'))                       // switch help requested
   {
      TxPrint( "\nUsage: %s ", TxaExeArgv(0));
      TxShowTxt( usagehelp);                    // program usage, generic
      TxShowTxt( TxGetSwitchhelp());            // Library specific switches
      TxShowTxt( samSwitchhelp);                // Sample specific switches
   }
   else
   {
      TXLN          ar;                         // arguments

      strcpy( ar,  SAM_STARTCMD);               // default command
      separator = TxaExeSwitchStr( 's', "SeparatorCh", "#");
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
// Set extra long names usable as switch or command option
/*****************************************************************************/
static void samSetLongSwitchNames
(
   void
)
{
   TxaOptionLongName( SAM_O_LOGAUTO, "logauto"); // Automatic logfile numbering
   TxaOptionLongName( SAM_O_EXPERT,  "expert"); // Expert menu mode
}                                               // end 'samSetLongSwitchNames'
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
   TXTM                winTitle;

   ENTER();
   TRACES(("Startup command is '%s'\n", firstcmd));

   TxPrint(  "\n  Scroll/Entry/Menu sample version %s  %s\n", SAM_V, SAM_C);
   TxPrint(  " อออออออออออออออออออออออออออ[ www.dfsee.com"
             " ]ออออออออออออออออออออออออออออออออออ\n\n");

   //- Set title in operating-system window/console/window-list
   sprintf( winTitle, "%s %s %s", SAM_N, SAM_V, SAM_C);
   TxSetAppWindowTitle( winTitle);              // could be made program-state specific

   samBEGINWORK();
   if (strlen( firstcmd))
   {
      static BOOL initialLogDialogDone = FALSE;

      TRACES(( "Automatic start of logging, default log dialog; ONCE using -l\n"));
      if ((initialLogDialogDone == FALSE) &&       //- present dialog only once (not on resize)
          (TxQueryLogFile(NULL, NULL) == NULL) &&  //- if not logging/tracing yet
          (!TxaExeSwitchSet(  'b')))               //- and no -b  switch
      {
         TXA_OPTION *opt;

         if (((opt = TxaOptValue('l')) != NULL) && (opt->type == TXA_NO_VAL)) // -l without a value
         {
            samMultiCommand( "logfile", 0, FALSE, FALSE, TRUE); // open logfile, popup dialog
            TxCancelAbort();                    // reset pending abort status
         }
         initialLogDialogDone = TRUE;
      }
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
   TXLN                s1;                      // temporary string space
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
      else if ((strcasecmp(c0, "help"     ) == 0) ||
               (strcasecmp(c0, "?"        ) == 0) )
      {
         TxShowTxt( TxGetStdCmdHelp());
         TxShowTxt( samGenericHelp);
         TxPrint(  " %s %s %s\n", SAM_N, SAM_V, SAM_C);
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
                            "Fsys Software program\n\n%s   %s : %s %s   (%2d-bit)\n",
                             alead, alead, SAM_N, SAM_V, SAM_C,
            #if defined (__LP64__)
               64);
            #else
               32);
            #endif
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
            #if      (__WATCOMC__ > 1900)
               //- must be OpenWatcom 2.0 or later (changed version convention)
               sprintf( text, "OpenWatcom %4.2lf (c) 1988-2018: Sybase and OpenWatcom\n",
                              ((double) (( __WATCOMC__ )) / 1000));
            #elif    (__WATCOMC__ > 1100)
               //- must be OpenWatcom before 2.0 (official builds)
               sprintf( text, "OpenWatcom %4.2lf (c) 1988-2010: Sybase and OpenWatcom\n",
                              ((double) (( __WATCOMC__ - 1100)) / 100));
            #else
               sprintf( text, "Watcom C++ %4.2lf (c) 1988-1999: Sybase, Inc. and Watcom\n",
                              ((double) (( __WATCOMC__ )) / 100));
            #endif
         #elif defined (DARWIN)
            #if defined (__LP64__)
               sprintf( text, "LLVM-gcc 10.0.0 and Clang-1000.10.44.2: (c) 2018 Apple\n");
            #else
               sprintf( text, "GNU  gcc  4.0.1  on OSX 10.6.8 (c) 2007 Apple computer\n");
            #endif
         #else
            #if defined (DEV32)
               sprintf( text, "VisualAge  3.65 (c) 1991-1997: IBM Corporation\n");
            #else
               sprintf( text, "Visual C++  5.0 (c) 1986-1997: Microsoft Corporation\n");
            #endif
         #endif
         strcat( about, text);
         #if defined (DEV32)
            sprintf( text, "%sEXE compress : lxLite     1.33 (c) 1996-2003: Max Alekseyev\n", alead);
            strcat( about, text);
         #elif !defined (DARWIN) || defined (__LP64__)
            sprintf( text, "%sEXE compress : UPX        3.94 (c) 1996-2017: Oberhumer/Molnar/Reiser\n", alead);
            strcat( about, text);
         #endif
         #if defined (DOS32)
            sprintf( text,  "%sDOS extender : %s\n", alead, txDosExtVersion());
            strcat(  about, text);
         #endif

         strcpy( s0, "Initial version description string");
         (void) TxOsVersion( s0);               // Get operating system version
         sprintf( text,  "%sOS   version : %s\n", alead, s0);
         strcat( about, text);
         TRACES(("about length:%d s0:'%s' text:'%s' about:'%s'\n", strlen(about), s0, text, about));

         if (TxOsAdditionalInfo( alead, text) == NO_ERROR) // additional info for OS
         {
            strcat( about, text);
         }
         #if defined (DOS32)
            sprintf( text,  "%sDPMI version : %s\n", alead, txDosExtDpmiInfo());
            strcat(  about, text);
         #endif
         strcat( about, "\n");

         TxMessage( !(sama->batch || plaintext), 5003, about);
      }
      else if (strcasecmp(c0, "logfile" ) == 0)
      {
         #if defined (USEWINDOWING)
         if (txwIsWindow( TXHWND_DESKTOP))
         {
            samLogDialog( (cc > 1) ? c1 : (sama->logAuto) ? "sam8-^" : "sample8",
                          SAMC_OPEN, TxaOption('r'), TxaOptStr( 'm', "Message", ""));
         }
         else
         #endif
         {
            sprintf( dc, "log %s %s", (cc > 1) ? c1 : (sama->logAuto) ? "sam8-^" : "sample8",
                         (TxaOption('r')) ? " -r" : "");
            rc = samMultiCommand( dc, 0, TRUE, FALSE, TRUE);
         }
      }
      else if (strcasecmp(c0, "run"   ) == 0)   // RUN frontend, prompt for
      {                                         // name & params if needed
         if (TxaOption('?') ||
             (c1[0] == '?') ||                  // explicit RUN help request
             TxaOption('h'))                    // script help
         {
            if (TxaOption('?') || (c1[0] == '?'))
            {
               rc = TxsNativeRun( NULL, NULL);  // get usage for RUN
            }
            else
            {
               TxShowTxt( hostvarhelp);
            }
         }
         else
         {
            #if defined (USEWINDOWING)
            if (((cc == 1) || TxaOption('P')) && // filedialog based
                (txwIsWindow( TXHWND_DESKTOP) )) // only when windowed ...
            {
                                               // Get combined scriptname + parameters from dialogs
               samRunScriptDialog( c1, s0);
            }
            else                                // parameter scriptname ?
            #endif                              // USEWINDOWING
            {
                                                // No file dialog, scriptname possibly specified
               strcpy( s0, c1);
               strcpy( s1, "");
               if ((cc == 1) || TxaOption('P')) // prompt for script + params
               {
                  TxPrompt( SAMC_RUNS, 40, s0, "Specify script to run plus parameters ...");
               }
               else
               {
                  if (strlen(s0) > 0)           // rebuild with name & params
                  {
                     TxaGetArgString( TXA_CUR, 2, TXA_OPT, TXMAXLN, s1);
                     strcat( s0, " ");
                     strcat( s0, s1);
                  }
               }
            }
            if (strlen(s0) > 0)                 // rebuild with name & params
            {
               sprintf( dc,   "runscript %s", s0);
               rc = samMultiCommand( dc, 0, TRUE, FALSE, TRUE);
               if (TxaOption('q'))
               {
                  rc |= SAM_QUIT;               // add QUIT flag to the RC
               }
            }
         }
      }
      else if ((strcasecmp(c0, "runscript") == 0)) // RUN, stage 2, reparsed
      {
         TXLN       scriptname;
         BOOL       isRexxScript;

         strcpy( s0, c1);                       // scriptname mandatory here!
         TxFnameExtension( s0, "dfs");
         if (TxsValidateScript( s0, &isRexxScript, NULL, scriptname))
         {
            DEVICE_STATE screen = TxScreenState( DEVICE_TEST);

            //- Takes options and parameters from the Txa structures!
            rc = TxsNativeRun( scriptname, samMultiCommand);
            TxScreenState( screen);             // restore initial state
         }
         else
         {
            TxPrint( "SAM8  script file : '%s' not found\n", s0);
            rc = TX_INVALID_DATA;
         }
      }
      else if ((strcasecmp(c0, "set"      ) == 0))
      {
         if (cc > 1)
         {
            if (strncasecmp(c1, "error", 5) == 0)
            {
               if (cc > 2)
               {
                  switch (c2[0])
                  {
                     case 'c':
                     case 'C':
                        sama->eStrategy = TXAE_CONFIRM;
                        break;

                     case 'i':
                     case 'I':
                        sama->eStrategy = TXAE_IGNORE;
                        break;

                     default:
                        sama->eStrategy = TXAE_QUIT;
                        break;
                  }
               }
               else
               {
                  TxPrint("\nSet error handling to specified strategy\n\n"
                          " Usage: %s %s confirm | ignore | quit\n\n"
                          "    CONFIRM = Ask user confirmation to quit, or ignore the error\n"
                          "    IGNORE  = Ignore the error, set returncode ($_rc) to zero (0)\n"
                          "    QUIT    = Quit current operation, keep non-zero returncode\n\n", c0, c1);
               }
               TxPrint( "Error Strategy now: '%s'\n", (sama->eStrategy == TXAE_CONFIRM) ? "CONFIRM" :
                                                      (sama->eStrategy == TXAE_IGNORE ) ? "IGNORE"  :
                                                                                          "QUIT");
            }
            else if (strncasecmp(c1, "exp", 3) == 0)
            {
               if (cc > 2)
               {
                  if ((strcasecmp(c2, "toggle") == 0)  || (c2[0] == 'T'))
                  {
                     sama->expertui = !(sama->expertui);
                  }
                  else if ((strcasecmp(c2, "on") == 0) || (c2[0] == '1'))
                  {
                     sama->expertui = TRUE;
                  }
                  else
                  {
                     sama->expertui = FALSE;
                  }
               }
               else
               {
                  TxPrint("\nSet EXPERT UI-mode (versus BASIC mode)\n\n"
                          " Usage: %s %s on | off | toggle\n\n", c0, c1);
               }
               TxPrint( "Expert UImode now : '%s'\n", (sama->expertui) ? "ON   (expert)" : "OFF (basic)");
            }
            else if (strncasecmp(c1, "log", 3) == 0)
            {
               if (cc > 2)
               {
                  if ((strcasecmp(c2, "on") == 0) || (c2[0] == '1'))
                  {
                     sama->logAuto = TRUE;
                  }
                  else
                  {
                     sama->logAuto = FALSE;
                  }
               }
               else
               {
                  TxPrint("\nSet LOG auto numbering\n\n"
                          " Usage: %s %s on | off\n\n"
                          "        ON  = Use automatic log numbering 001..999 from dialog\n"
                          "        OFF = Use logfilename 'as is', append if existing\n\n", c0, c1);
               }
               TxPrint( "LOG auto numbering now : '%s'\n", (sama->logAuto) ? "ON" : "OFF");
            }
            else
            {
               TxPrint("SET property name : '%s' unknown\n", c1);
            }
         }
         else
         {
            TxPrint(          "%s SET properties :  (capital part required as keyword only)\n", SAM_N);
            TxPrint( "  EXPert UI mode  : on     |   off\n");
            TxPrint( "  ERROR strategy  : quit   |   ignore  |  confirm\n");
            TxPrint( "  LOG numbering   : on     |   off\n");
         }
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
      sprintf( s0, "Done : %s", samcmd);
      txwSendMsg( sama->sbwindow, TXWM_STATUS, (TXWMPARAM) s0, (TXWMPARAM) cSchemeColor);
   }
   RETURN (rc);
}                                               // end 'samSingleCommand'
/*---------------------------------------------------------------------------*/

