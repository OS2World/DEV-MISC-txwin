//
// Generic Dialog functions, called from the menu and generic commands
//
// Author: J. van Wijk
//
// JvW  26-01-2005 Initial version
//

#include <txlib.h>                              // TX library interface

#include <dfsdisk.h>                            // FS disk structure defs
#include <dfspart.h>                            // FS partition info manager
#include <dfsupart.h>                           // FS partition utilities
#include <dfstore.h>                            // Store and sector I/O
#include <dfs.h>                                // DFS navigation and defs
#include <dfsver.h>                             // DFS version and naming
#include <dfsutil.h>                            // DFS utility functions
#include <dfsspace.h>                           // DFS  file-space interface
#include <dfsectio.h>                           // DFS store I/O private defs
#include <dfstable.h>                           // SLT utility functions
#include <dfswin.h>                             // window and help definitions
#include <dfstdlg.h>                            // generic dialogs definitions
#include <dfsdgen.h>                            // generic dialogs interface

//- Window styles used with the Dialog Widgets
//- DFSDM is 'moving' DFSDS' is sizing and DFSD2 is half-sizing, all horizontal

static TXTM        fdescr = "";                 // FROM object/store description
static TXTM        tdescr = "";                 // TO   object/store description

// Dialog window procedure, for the SLT-display dialog field; update status
static ULONG dfsSltWinProc                      // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


// Dialog window procedure, for the GEOMETRY dialog, set dependant fields
static ULONG dfsGeoWinProc                      // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// Calculate GEO values to be displayed from RADIO settings and global GEO
static void dfsGeoSetControls
(
   void
);

/*========================== FIND ===============================================================*/
static BOOL fdwr1  = TRUE;                      // Radio Every sector
static BOOL fdwr2  = FALSE;                     // Radio Cylinder boundaries
static BOOL fdwr3  = FALSE;                     // Radio Freespace only
static BOOL fdwr4  = FALSE;                     // Radio Allocated only

static BOOL fdwr5  = TRUE;                      // Radio Ascii
static BOOL fdwr6  = FALSE;                     // Radio Unicode
static BOOL fdwr7  = FALSE;                     // Radio Hex
static BOOL fdwr8  = FALSE;                     // Radio Mixed

static BOOL fdwrS  = FALSE;                     // Radio Single shot
static BOOL fdwrR  = TRUE;                      // Radio Repeat search
static BOOL fdwrM  = FALSE;                     // Radio Multiple hits/sector

static BOOL fdwc1  = FALSE;                     // Check Case-sensitive
static BOOL fdwc2  = TRUE;                      // Check No sector span
static BOOL fdwc3  = FALSE;                     // Check Search backward
static BOOL fdwc4  = FALSE;                     // Check NOT containing
static BOOL fdwc5  = FALSE;                     // Check Verbose output
static BOOL fdwc6  = FALSE;                     // Check Start at NEXT
static BOOL fdwc7  = TRUE;                      // Show search args

static TXTS fdwe0  = "Current";                 // startsector for find -f:
static TXTS fdwe1  = "";                        // position for 1st argument
static TXTS fdwe2  = "";                        // sector types
static TXTS fdwe3  = "";                        // sector step value
static TXTM fdwe4  = "";                        // string for 2nd argument

/*
() Ascii          () Repeat, 1 hit/sector  () Search in every sector
( ) Unicode        ( ) Repeat multiple hits  ( ) On Cylinder boundaries
( ) Hex pairs      ( ) Search once, display  ( ) In freespace (undelete)
( ) Mixed string   [ ] Case-sensitive match  ( ) In allocated (filegrep)
[ ] Verbose output [ ] Search backwards      [ ] Start at NEXT/PREV sect
[û] Show arguments [ ] NOT containing ...    [û] No sector span (faster)

 StartOffset  @Position  Types    Step    2ndary string, case-sensitive
[...........][.........][.....][........][.............................]
*/
#define   DFSDGFINDWIDGETS 23
#define   DFSDGFINDCVERBOS  4
#define   DFSDGFINDRREPEAT  6
#define   DFSDGFINDRRMULTI  7

static TXWIDGET  dfsDgFindWidgets[DFSDGFINDWIDGETS] =  // order determines TAB-order!
{
   {0,  0, 1, 18, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr5, "Ascii")},
   {1,  0, 1, 18, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr6, "Unicode")},
   {2,  0, 1, 18, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr7, "Hex pairs")},
   {3,  0, 1, 18, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr8, "Mixed string")},
   {4,  0, 1, 18, 0, 5, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdwc5, "Verbose output")},
   {5,  0, 1, 18, 0, 5, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdwc7, "Show arguments")},

   {0, 19, 1, 25, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwrR, "Repeat, 1 hit/sector")},
   {1, 19, 1, 25, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwrM, "Repeat multiple hits")},
   {2, 19, 1, 25, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwrS, "Search once, display")},
   {3, 19, 1, 25, 0, 6, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdwc1, "Case-sensitive match")},
   {4, 19, 1, 25, 0, 6, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdwc3, "Search backwards")},
   {5, 19, 1, 25, 0, 6, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdwc4, "NOT containing ...")},


   {0, 45, 1, 28, 0, 3, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr1, "Search in every sector" )},
   {1, 45, 1, 28, 0, 3, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr2, "On Cylinder boundaries" )},
   {2, 45, 1, 28, 0, 3, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr3, "In freespace (undelete)")},
   {3, 45, 1, 28, 0, 3, 0, TXWS_AUTORAD, 0, TXStdButton( &fdwr4, "In allocated (filegrep)")},
   {4, 45, 1, 28, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdwc6, "Start at NEXT/PREV sect")},
   {5, 45, 1, 28, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &fdwc2, "No sector span (faster)")},

   {7,  0, 2, 13, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( fdwe0, TXMAXTS, "StartOffset")},
   {7, 13, 2, 11, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( fdwe1, TXMAXTS, "@Position")},
   {7, 24, 2,  7, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( fdwe2, TXMAXTS, "Types")},
   {7, 31, 2, 10, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( fdwe3, TXMAXTS, "  Step")},
   {7, 41, 2, 31, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD_SIZE, 0,
                           TXStdEntryf( fdwe4, TXMAXTM, "2ndary string, case-sensitive")}
};

static TXGW_DATA dfsDgFindDlg =
{
   DFSDGFINDWIDGETS,                            // number of widgets
   DFSC_FINDTC,                                 // dialog help
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgFindWidgets                             // array of widgets
};

static TXLN   findarg1 = "";                    // first search argument


/*************************************************************************************************/
// Present FIND options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsFindDialog
(
   BOOL                single,                  // IN    allow single-shot only
   BOOL                quiet,                   // IN    no result sector display
   BOOL                again,                   // IN    skip dialog if valid args
   ULONG               start,                   // IN    starting LSN or LSN_NULL
   ULONG               spos                     // IN    start offset or 0
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXTS                optstr;
   TXTM                escaped;
   BOOL                repeat = again && (strlen(findarg1) ||
                                          strlen(fdwe4)    ||
                                          strlen(fdwe2));
   ENTER();

   if (single)
   {
      fdwrS  = TRUE;                            // Radio Single shot
      fdwrR  = FALSE;                           // Radio Repeat search
      fdwrM  = FALSE;                           // Radio Multiple hits/sector

      dfsDgFindWidgets[ DFSDGFINDCVERBOS].flags |=  TXWI_DISABLED;
      dfsDgFindWidgets[ DFSDGFINDRREPEAT].flags |=  TXWI_DISABLED;
      dfsDgFindWidgets[ DFSDGFINDRRMULTI].flags |=  TXWI_DISABLED;
   }
   else                                         // enable radio buttons
   {
      dfsDgFindWidgets[ DFSDGFINDCVERBOS].flags &= ~TXWI_DISABLED;
      dfsDgFindWidgets[ DFSDGFINDRREPEAT].flags &= ~TXWI_DISABLED;
      dfsDgFindWidgets[ DFSDGFINDRRMULTI].flags &= ~TXWI_DISABLED;
   }
   if (start != LSN_NULL)
   {
      sprintf( fdwe0, "0x%8.8lx", start);
   }

   dfsBEGINWORK();                              // signal work starting
   while (repeat || txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &dfsDgFindDlg,
         "Specify the primary ASCII, UNICODE, HEX or Mixed string "
         "to search the sectors for. Adjust the search options to "
         "your liking, for usage consult online-help with <F1> or "
         "the 'FIND' command documentation\n\n"
         " Primary search string, format and case-sensitivity set by the options",
         " Specify search string(s) and options ", DFSC_FINDTC,
         TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER, 72, findarg1)
      != TXDID_CANCEL)
   {
      if (strlen(findarg1) || strlen(fdwe4) || strlen(fdwe2))
      {
         strcpy( command, "find -o:");

         if (!fdwc1 && !fdwr7) strcat( command, "$"); // no ASCII compare if HEX!
         if ( fdwc2)           strcat( command, "%");
         if ( fdwc3)           strcat( command, "-");
         if ( fdwc4)           strcat( command, "^");
         if ( quiet)
         {
                               strcat( command, "Q"); // no auto sector display
         }
         else
         {
            if ( fdwc5)        strcat( command, "+"); // optional verbose for repeat
            if ( fdwrS)        strcat( command, "+"); // always verbose
         }
         if (!single)
         {
            if ( fdwrR)        strcat( command, "*");
            if ( fdwrM)        strcat( command, "*!M");
         }
         if ( fdwr2)           strcat( command, "~");
         if (!fdwc7)           strcat( command, "!");
         if ( fdwr3)           strcat( command, "[");
         if ( fdwr4)           strcat( command, "]");

         if ((strlen(fdwe1) != 0) && (isdigit(fdwe1[0]))) // valid position
         {
            strcat( command, "@");
            strcat( command, fdwe1);
         }
         if (fdwc6)
         {
            strcat( command, " -n");
         }
         if (strlen(fdwe2) != 0)                // sector types specified
         {
            strcat( command, " -t:'");
            strcat( command, fdwe2);
            strcat( command, "'");
         }
         if (strlen(fdwe3) != 0)                // sector step value
         {
            strcat( command, " -i:");
            strcat( command, fdwe3);
         }
         if (strlen(fdwe4))                     // 2nd search string
         {
            TxRepStr( fdwe4, '"', "\\\"", escaped, TXMAXTM); // escape double quotes
            if ( fdwr5) strcat( command, " -A:\"");
            if ( fdwr6) strcat( command, " -U:\"");
            if ( fdwr7) strcat( command, " -H:\"");
            if ( fdwr8) strcat( command, " -M:\"");
            strcat(             command, escaped);
            strcat(             command, "\"");
         }
         if (strlen(findarg1))                  // 1st search string
         {
            TxRepStr( findarg1, '"', "\\\"", escaped, TXMAXTM); // escape double quotes
            if ( fdwr5) strcat( command, " -a:\"");
            if ( fdwr6) strcat( command, " -u:\"");
            if ( fdwr7) strcat( command, " -h:\"");
            if ( fdwr8) strcat( command, " -m:\"");
            strcat(             command, escaped);
            strcat(             command, "\"");
         }
         if ((strlen( fdwe0) != 0) &&           // Start value specified
             (isxdigit( fdwe0[0]))  )           // and it is a number
         {
            TxStrip( fdwe0, fdwe0, ' ', ' ');
            strcat(  command, " -f:");
            strcat(  command, fdwe0);
         }
         if (repeat)                            // start from next sector
         {
            strcat(  command, " -n");
         }
         else if (spos != 0)                      // explicit start offset
         {
            sprintf( optstr, " -n:0x%lx", (fdwc3) ? spos -1 : spos +1);
            strcat(  command, optstr);
         }
         if (quiet)
         {
            rc = dfsMultiCommand( command, 0, TRUE, FALSE, FALSE);
         }
         else
         {
            TxCancelAbort();
            txwAdd2History( dfsa->cmdwindow, command);
            rc = dfsMultiCommand( command, 0, TRUE, TRUE, TRUE);
         }
         break;                                 // out of while ...
      }
      else
      {
         TxMessage( TRUE, DFSC_FINDTC,
                    "You must specify at least one search "
                    "argument or a sector Type ...");
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsFindDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== RUN SCRIPT =========================================================*/
static BOOL rsVerbose = FALSE;                  // Verbose
static BOOL rsStep    = FALSE;                  // SingleStep

/*
[ ] Verbose, display each script line when executed
[ ] Single step, confirm before executing each line
*/
#define   DFSDGRSWIDGETS 2
static TXWIDGET  dfsDgRsWidgets[DFSDGRSWIDGETS] = // order determines TAB-order!
{
   {0,  0, 1, 58, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( &rsVerbose,
                          "Verbose, display each script line when executed")},
   {1,  0, 1, 58, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( &rsStep,
                          "Single step, confirm before executing each line")}
};

static TXGW_DATA dfsDgRsDlg =
{
   DFSDGRSWIDGETS,                              // number of widgets
   DFSC_RUNS,                                   // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgRsWidgets                               // array of widgets
};

/*************************************************************************************************/
// Present Run-script file-dialog with options and execute resulting command
/*************************************************************************************************/
ULONG dfsRunScriptDialog
(
   char               *firstParam,              // IN    path/scriptname, or empty
   char               *scriptInfo               // OUT   scriptname + parameters
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                params;
   TXLN                fspec;
   TXTM                wildcard;
   TX1K                dlgText;

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   // Handle input options when specified
   if (TxaOptSet('s'))                          // single-step option used
   {
      rsStep = TxaOption('s');
   }
   if (TxaOptSet('v'))                          // verbose option used
   {
      rsVerbose = TxaOption('v');
   }

   strcpy( wildcard, firstParam);               // Specified (partial) name
   if (strchr( wildcard, '.') == NULL)          // no extension
   {
      strcat( wildcard, "*");                   // add wildcard
   }
   TxFnameExtension( wildcard, "dfs");          // add default extension

   strcpy( fspec, wildcard);
   while (txwOpenFileDialog( fspec, NULL, NULL, DFSC_RUNS,
         (dfsa->expertui) ? &dfsDgRsDlg : NULL,
          " Select DFSee script file to RUN ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         sprintf(scriptInfo, "%s%s'%s'", (rsVerbose) ? "-v " : "",
                                         (rsStep)    ? "-s " : "", fspec);

         TxsValidateScript( fspec, NULL, params, NULL); // get description in params
         if (strstr( params, "no-parameters") == NULL)
         {
            if (strlen( params) != 0)
            {
               sprintf( dlgText, "%s\n\nParameters enclosed in [] are "
                        "optional, others are mandatory.\n%s", fspec, params);
            }
            else
            {
               sprintf( dlgText, "%s\n\nSpecify additional parameters for "
                        "the script or just leave as is ...", fspec);
            }
            // Get the parmeters specified on the commandline, for editing
            TxaGetArgString( TXA_CUR, 2, TXA_ALL, TXMAXLN, params);
            if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL,
                  dlgText, " Specify parameter(s) for the script ", DFSC_RUNS,
                  TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER,
                  50, params) != TXDID_CANCEL)
            {
               strcat( scriptInfo, " ");
               strcat( scriptInfo, params);
            }
            else                                // ESC on parameter prompt
            {                                   // cancel script execution
               strcpy( scriptInfo, "");
            }
         }
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_RUNS, "You must specify a script filename, "
                                     "not a wildcard or directory ...");
         strcpy( fspec, wildcard);
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsRunScriptDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== IMPORT =============================================================*/
static BOOL impdwc1 = FALSE;                    // Include data sectors

/*
[ ] Import each DATA sector from .BIN/.BN? file too
*/
#define   DFSDGIMPWIDGETS 1
static TXWIDGET  dfsDgImpWidgets[DFSDGIMPWIDGETS] = // order determines TAB-order!
{
   {0,  0, 1, 58, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( &impdwc1,
                          "Import each DATA sector from .BIN/.BN? file too")}
};

static TXGW_DATA dfsDgImpDlg =
{
   DFSDGIMPWIDGETS,                             // number of widgets
   DFSC_IMPLIST,                                // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgImpWidgets                              // array of widgets
};

/*************************************************************************************************/
// Present Import options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsImportDialog
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXTS                wildcard;

   ENTER();

   dfsBEGINWORK();                              // signal work starting
   strcpy( wildcard, "*.sn?;*.lsn;*.lst");
   strcpy( fspec, wildcard);
   while (txwOpenFileDialog( fspec, NULL, NULL, DFSC_IMPLIST, &dfsDgImpDlg,
          " Select the sector-list file to be imported ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         sprintf(    command, "import \"%s\"%s", fspec, (impdwc1) ? " -d" : "");
         dfsExecCmd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_IMPLIST, "You must specify a filename, "
                                        "not a wildcard or directory ...");
         strcpy( fspec, wildcard);
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsImportDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== EXPORT =============================================================*/
static BOOL expdwc1 = FALSE;                    // Include data sectors

/*
[ ] Export each DATA sector to a .BIN/.BN? file too
*/
#define   DFSDGEXPWIDGETS 1
static TXWIDGET  dfsDgExpWidgets[DFSDGEXPWIDGETS] = // order determines TAB-order!
{
   {0,  0, 1, 58, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( &expdwc1,
                          "Export each DATA sector to a .BIN/.BN? file too")}
};

static TXGW_DATA dfsDgExpDlg =
{
   DFSDGEXPWIDGETS,                             // number of widgets
   DFSC_EXPLIST,                                // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgExpWidgets                              // array of widgets
};

/*************************************************************************************************/
// Present Export options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsExportDialog
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXTS                wildcard;

   ENTER();

   dfsBEGINWORK();                              // signal work starting
   strcpy( wildcard, "*.sn?;*.lsn");
   strcpy( fspec, wildcard);
   while (txwSaveAsFileDialog( fspec, NULL, NULL, DFSC_EXPLIST, &dfsDgExpDlg,
          " Specify file for exporting the sector-list to ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         sprintf(    command, "export \"%s\"%s", fspec, (expdwc1) ? " -d" : "");
         dfsExecCmd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_EXPLIST, "You must specify a filename, "
                                        "not a wildcard or directory ...");
         strcpy( fspec, wildcard);
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsExportDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== SIM ================================================================*/
static BOOL        simr1  = FALSE;              // Single image-file
static BOOL        simr2  = TRUE;               // Multi file, max size
static BOOL        simr3  = FALSE;              // Multi file removable

static BOOL        simr4  = FALSE;              // RAW image
static BOOL        simr5  = TRUE;               // Compressed
static BOOL        simr6  = FALSE;              // Smart compr

static BOOL        simc1  = FALSE;              // Append to file
static BOOL        simc2  = FALSE;              // Ignore errors
static BOOL        simc3  = FALSE;              // Exclude LVMsig
static BOOL        simc4  = TRUE;               // HEX sector values

static TXTS        sime1  = "2047";             // Multiple max size
static BOOL        simrk  = FALSE;              // KiB
static BOOL        simrm  = TRUE;               // MiB
static BOOL        simrg  = FALSE;              // GiB

static TXTS        sime2  = "";                 // From
static TXTS        sime3  = "";                 // Size
static BOOL        simrK  = FALSE;              // KiB
static BOOL        simrM  = FALSE;              // MiB
static BOOL        simrG  = FALSE;              // GiB
static BOOL        simrS  = TRUE;               // Sec

/*
 From Partition:[04 ECS12       F: Log 07 HPFS        917.7 MiB ]

[ ] Append to file    ( ) Smart compress  () Multiple files, on one disk
[ ] Ignore errors     () Compressed      ( ) Multiple file, media change
[ ] Exclude LVM area  ( ) RAW image       ( ) Single file, unlimited size

 First sector   Image size  [û] Sector values are HEX, not decimal
[0...........] [Whole Area] () Sectors  ( ) KiB  ( ) MiB  ( ) GiB
Multi only: max size per file: [650.....]( ) KiB  () MiB  ( ) GiB
*/
#define   DFSDGSIMWIDGETS 24
#define   DFSDGSIMWBASICS 12
#define   DFSDGSIMT1      0                     // index of description
#define   DFSDGSIMT2      1                     // index of leader
#define   DFSDGSIML1      2                     // index of list-1
#define   DFSDGSIMAF      3                     // index of append to file
#define   DFSDGSIMXL      5                     // index of exclude LVM
static TXWIDGET  dfsDgSimWidgets[DFSDGSIMWIDGETS] =    // order determines TAB-order!
{
   {0, 17, 1, 50, 1, 0, 1, DFSDSOUTPUT,  0, TXStdStline( fdescr)},
   {0,  1, 1, 15, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( NULL)},
   {0, 16, 1, 50, 0, 0, 0, DFSDSLISTBOX, 0, TXStdLstBox( NULL, 0, NULL)},

   {2,  0, 1, 24, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &simc1, "Append to file")},
   {3,  0, 1, 24, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &simc2, "Ignore errors")},
   {4,  0, 1, 24, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &simc3, "Exclude LVM area")},

   {2, 22, 1, 18, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &simr6, "Smart compress")},
   {3, 22, 1, 18, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &simr5, "Compressed")},
   {4, 22, 1, 18, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &simr4, "RAW image")},

   {2, 42, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &simr2, "Multiple files, on one disk")},
   {3, 42, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &simr3, "Multiple file, media change")},
   {4, 42, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &simr1, "Single file, unlimited size")} ,

   {6, 28, 1, 38, 0, 0, 0, DFSDMAUTOCHK, 0, TXStdButton( &simc4, "Sector values are HEX, not decimal")},
   {6,  0, 2, 14, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD2SIZE,
                                         0, TXStdEntryf( sime2, TXMAXTS, "First sector")},
   {6, 15, 2, 12, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD2SIZE |  TXWS_HCHILD2MOVE,
                                         0, TXStdEntryf( sime3, TXMAXTS, "Image size")},

   {7, 28, 1, 11, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &simrS, "Sectors")},
   {7, 41, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &simrK, "KiB")},
   {7, 50, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &simrM, "MiB")},
   {7, 59, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &simrG, "GiB")},

   {8, 16, 1, 15, 0, 0, 0, DFSD2OUTPUT,  0, TXStdStline( "Filesize limit:")},
   {8, 31, 1, 10, 0, 0, 0, TXWS_ENTRYB   |  TXWS_HCHILD2SIZE |  TXWS_HCHILD2MOVE,
                                         0, TXStdEntryf( sime1, TXMAXTS, "")},
   {8, 41, 1,  7, 0, 5, 0, DFSDMAUTORAD, 0, TXStdButton( &simrk, "KiB")},
   {8, 50, 1,  7, 0, 5, 0, DFSDMAUTORAD, 0, TXStdButton( &simrm, "MiB")},
   {8, 59, 1,  7, 0, 5, 0, DFSDMAUTORAD, 0, TXStdButton( &simrg, "GiB")}
};

static TXGW_DATA dfsDgSimDlg =
{
   DFSDGSIMWIDGETS,                             // number of widgets
   DFSC_IMAGDLG,                                // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgSimWidgets                              // array of widgets
};


/*************************************************************************************************/
// Present SIM options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsSimDialog
(
   DFSOBJECT           source,                  // IN    Source for the image
   char               *image,                   // IN    filename, "" or NULL
   BOOL                media,                   // IN    set media-change
   ULONG               mfsize,                  // IN    max filesize or 0
   ULONG               tsector,                 // IN    To   sector number
   ULONG               sizetodo                 // IN    Size to clone (0 = all)
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXTT                option;
   TXSELIST           *list = NULL;
   TXTM                text;

   ENTER();

   TRACES(( "IMAGE to:'%s'  mfsize: %lu start position:%8.8lx  size:%8.8lx\n",
            (image) ? image : "NULL", mfsize, tsector, sizetodo));

   dfsBEGINWORK();                              // signal work starting

   if (media)
   {
      simr1 = FALSE;
      simr2 = FALSE;
      simr3 = TRUE;                             // force media-change set
   }
   if (mfsize != 0)                             // maximum filesize specified
   {
      sprintf( sime1, "%lu", mfsize);           // value in ASCII
      simrk  = FALSE;                           // KiB
      simrm  = TRUE;                            // MiB
      simrg  = FALSE;                           // GiB
   }
   sprintf( sime2, "%lx", tsector);             // set start sector number
   if (sizetodo != 0)
   {
      sprintf( sime3, "%lx", sizetodo);         // set Size to process (0 = all)
   }
   else
   {
      strcpy(  sime3, "Whole Area");
   }

   dfsDgSimWidgets[DFSDGSIMT1].flags  |=  TXWI_DISABLED; // text field OFF
   dfsDgSimWidgets[DFSDGSIML1].flags  &= ~TXWI_DISABLED; // list field ON
   switch (source)
   {
      case DFSO_VOLD:
         TRACES(( "Volume, list: %8.8lx\n", dfsa->slVolumes));
         dfsDgSimWidgets[DFSDGSIMT2].sl.buf = "From    volume:";
         dfsDgSimWidgets[DFSDGSIML1].title  = " Create image from volume ";
         if (dfsa->slVolumes == NULL)
         {
            dfsa->slVolumes = TxFsDriveSelist( TXFSV_HD | TXFSV_CD |
                                               TXFSV_FLOP, FALSE);
         }
         list = dfsa->slVolumes;
         TxSelStringSelect( list,  SINF->drive, 2);
         break;

      case DFSO_DISK:
         TRACES(( "Disk, list: %8.8lx\n", dfsa->slDiskOne));
         dfsDgSimWidgets[DFSDGSIMT2].sl.buf = "From      disk:";
         dfsDgSimWidgets[DFSDGSIML1].title  = " Create image from disk ";
         list = dfsa->slDiskOne;
         sprintf( text, "%2hu",    SINF->disknr);
         TxSelStringSelect( list,  text, 2);
         break;

      case DFSO_PART:
         TRACES(( "Disk, list: %8.8lx\n", dfsa->slPartOne));
         dfsDgSimWidgets[DFSDGSIMT2].sl.buf = "From partition:";
         dfsDgSimWidgets[DFSDGSIML1].title  = " Create image from partition ";
         list = dfsa->slPartOne;
         sprintf( text, "%2.2hu", SINF->partid);
         TxSelStringSelect( list,  text, 2);
         break;

      default:
         if (source == DFSO_IMGF)
         {
            strcpy( fdescr, dfstStoreDesc2( DFSTORE) + 10);
            dfsDgSimWidgets[DFSDGSIMT2].sl.buf  = "From imagefile:";
         }
         else                                   // other types
         {
            strcpy( fdescr, dfstStoreDesc1( DFSTORE) + 10);
            dfsDgSimWidgets[DFSDGSIMT2].sl.buf  = "From   current:";
         }
         dfsDgSimWidgets[DFSDGSIMT1].flags  &= ~TXWI_DISABLED; // text field ON
         dfsDgSimWidgets[DFSDGSIML1].flags  |=  TXWI_DISABLED; // list field OFF
         break;
   }
   dfsDgSimWidgets[DFSDGSIML1].lb.list = list;

   if (dfsa->expertui)                          // set available widgets Expert/Basic
   {
      dfsDgSimDlg.count = DFSDGSIMWIDGETS;
      dfsDgSimWidgets[DFSDGSIMAF].flags  &= ~TXWI_DISABLED;
      dfsDgSimWidgets[DFSDGSIMXL].flags  &= ~TXWI_DISABLED;
   }
   else
   {
      dfsDgSimDlg.count = DFSDGSIMWBASICS;
      dfsDgSimWidgets[DFSDGSIMAF].flags  |=  TXWI_DISABLED;
      dfsDgSimWidgets[DFSDGSIMXL].flags  |=  TXWI_DISABLED;
   }

   strcpy( fspec, "*.i??");
   while (txwSaveAsFileDialog( fspec, NULL, image, DFSC_IMAGDLG, &dfsDgSimDlg,
      " Specify imaging options and name for (1st) imagefile ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         TxStrip( fspec, fspec, ' ', ' ');
         TxStrip( sime2, sime2, ' ', ' ');
         sprintf( command, "image \"%s\" %s%s",
                  fspec, (simc4) ? "0x" : "", (strlen(sime2)) ? sime2 : "0");

         if ((strlen( sime3) != 0) &&           // Size value specified
             (isxdigit( sime3[0]))  )           // and it is a number
         {
            TxStrip( sime3, sime3, ' ', ' ');
            sprintf( option, " %s%s,%s", (simc4 && simrS) ? "0x" : "", sime3,
                    (simrK) ? "k" : (simrM) ? "m" : (simrG) ? "g" : "s");
            strcat(  command, option);
         }
         if (list)
         {
            strcpy(  text, list->items[ list->selected]->text);
            TxStrip( text, text, ' ', ' ');     // strip leading spaces
            TxRepl(  text, ' ', 0);             // replace next space by 0
            switch (source)                     // select proper source
            {
               case DFSO_VOLD: sprintf( option, " -V:\"%s\"", text);  break;
               case DFSO_DISK: sprintf( option, " -D:%s",     text);  break;
               case DFSO_PART: sprintf( option, " -P:%s",     text);  break;
               default:        strcpy(  option, "");                  break;
            }
            strcat( command, option);
         }
         if (!simr1)                            // not a single file
         {
            strcat( command, " -m:");
            if (strlen( sime1) != 0)            // size specified
            {
               TxStrip( sime1, sime1, ' ', ' ');
               sprintf( option, "%s,%s", sime1, (simrk) ? "k" :
                                                (simrm) ? "m" :
                                                          "g");
               strcat( command, option);
            }
         }
         if ( simc1)         strcat( command, " -A");
         if ( simc2)         strcat( command, " -E:i");
         if ( simc3)         strcat( command, " -L");
         if ( simr4)         strcat( command, " -R");
         if ( simr5 | simr6) strcat( command, " -z");
         if ( simr6)         strcat( command, " -S");
         if ( simr3)         strcat( command, " -M");

         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_IMAGING, "You must specify an image filename, "
                                        "not a wildcard or directory ...");
         strcpy(  fspec, "*.i??");
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsSimDialog'
/*-----------------------------------------------------------------------------------------------*/



/*========================== WRIM ===============================================================*/
static BOOL wrimr1  = TRUE;                     // Restore
static BOOL wrimr2  = FALSE;                    // Verify
static BOOL wrimr3  = FALSE;                    // Compare

static BOOL wrimc2  = FALSE;                    // Exclude, -Lvm
static BOOL wrimc4  = TRUE;                     // HEX sector values
static BOOL wrimc6  = TRUE;                     // Use size from image file

static TXTS wrime2  = "";                       // From
static TXTS wrime3  = "";                       // Size
static BOOL wrimrK  = FALSE;                    // KiB
static BOOL wrimrM  = FALSE;                    // MiB
static BOOL wrimrG  = FALSE;                    // GiB
static BOOL wrimrS  = TRUE;                     // Sec

/*
 To Partition : [04 ECS12       F: Log 07 HPFS        917.7 MiB ]

[ ] Use size/start from image file   () Restore, write to object
                                     ( ) Compare imagefile to object
[ ] Exclude LVM area                 ( ) Verify image file integrity

 First sector    Image size  [û] Sector values are HEX, not decimal
[Start of Area] [Whole Area] () Sectors  ( ) KiB  ( ) MiB  ( ) GiB
*/
#define   DFSDGWRIMWIDGETS 15
#define   DFSDGWRIMWBASICS  5
#define   DFSDGWRIMT1       0                   // index of description
#define   DFSDGWRIMT2       1                   // index of leader
#define   DFSDGWRIML1       2                   // index of list-1
#define   DFSDGWRIMXL       4                   // index of Exclude LVM
static TXWIDGET  dfsDgWrimWidgets[DFSDGWRIMWIDGETS] =  // order determines TAB-order!
{
   {0, 17, 1, 50, 1, 0, 0, DFSDSOUTPUT,  0, TXStdStline( tdescr)},
   {0,  1, 1, 15, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( NULL)},
   {0, 16, 1, 50, 0, 0, 0, DFSDSLISTBOX, 0, TXStdLstBox( NULL, 0, NULL)},

   {2,  0, 1, 34, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &wrimc6, "Get size/start from image file")},
   {4,  0, 1, 34, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &wrimc2, "Exclude LVM area")},

   {2, 37, 1, 32, 0, 2, 0, DFSDMAUTORAD, 0, TXStdButton( &wrimr1, "Restore, write to object")},
   {3, 37, 1, 32, 0, 2, 0, DFSDMAUTORAD, 0, TXStdButton( &wrimr3, "Compare imagefile to object")},
   {4, 37, 1, 32, 0, 2, 0, DFSDMAUTORAD, 0, TXStdButton( &wrimr2, "Verify image file integrity")},

   {6, 29, 1, 38, 0, 0, 0, DFSDMAUTOCHK, 0, TXStdButton( &wrimc4, "Sector values are HEX, not decimal")},
   {6,  0, 2, 15, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD2SIZE,
                                         0, TXStdEntryf( wrime2, TXMAXTS, "First sector")},
   {6, 17, 2, 12, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD2SIZE  |  TXWS_HCHILD2MOVE,
                                         0, TXStdEntryf( wrime3, TXMAXTS, "Image size")},
   {7, 29, 1, 11, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &wrimrS, "Sectors")},
   {7, 42, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &wrimrK, "KiB")},
   {7, 51, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &wrimrM, "MiB")},
   {7, 60, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &wrimrG, "GiB")}
};

static TXGW_DATA dfsDgWrimDlg =
{
   DFSDGWRIMWIDGETS,                            // number of widgets
   DFSC_WRIMAGE,                                // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgWrimWidgets                             // array of widgets
};


/*************************************************************************************************/
// Present WRIM options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsWrimDialog
(
   DFSOBJECT           destination,             // IN    Destination for image
   char               *image,                   // IN    filename, "" or NULL
   BOOL                verify,                  // IN    verify only
   ULONG               tsector,                 // IN    To   sector number
   ULONG               sizetodo                 // IN    Size to clone (0 = all)
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXTT                option;
   TXTM                title;
   TXGW_DATA          *widgets = NULL;
   TXTS                wildcard;
   TXSELIST           *list    = NULL;
   TXTM                text;

   ENTER();
   TRACES(( "%s  from:'%s'  start position:%8.8lx  size:%8.8lx\n",
          (verify) ? "RESTORE" : "VERIFY ", tsector, sizetodo));

   dfsBEGINWORK();                              // signal work starting

   if (tsector != 0)
   {
      sprintf( wrime2, "%lx", tsector);        // Offset to process (0 = 0)
   }
   else
   {
      strcpy(  wrime2, "Start of Area");
   }
   if (sizetodo != 0)
   {
      sprintf( wrime3, "%lx", sizetodo);        // set Size to process (0 = all)
   }
   else
   {
      strcpy(  wrime3, "Whole Area");
   }

   if (verify)                                  // VERIFY only
   {
      strcpy( title, " Select an imagefile to be checked for file damage ");
      strcpy( wildcard, "*.imz");
   }
   else
   {
      dfsDgWrimWidgets[DFSDGWRIMT1].flags  |=  TXWI_DISABLED; // text field OFF
      dfsDgWrimWidgets[DFSDGWRIML1].flags  &= ~TXWI_DISABLED; // list field ON
      strcpy( title, " Select an imagefile and options for the Restore ");
      if (dfsa->expertui)
      {
         strcat( title, "/ Compare ");
      }
      switch (destination)
      {
         case DFSO_VOLD:
            dfsDgWrimWidgets[DFSDGWRIMT2].sl.buf = "To     volume :";
            dfsDgWrimWidgets[DFSDGWRIML1].title  = " Restore image to volume ";
            if (dfsa->slVolumes == NULL)
            {
               dfsa->slVolumes = TxFsDriveSelist( TXFSV_HD | TXFSV_CD |
                                                  TXFSV_FLOP, FALSE);
            }
            list = dfsa->slVolumes;
            TxSelStringSelect( list,  SINF->drive, 2);
            break;

         case DFSO_DISK:
            dfsDgWrimWidgets[DFSDGWRIMT2].sl.buf = "To       disk :";
            dfsDgWrimWidgets[DFSDGWRIML1].title  = " Restore image to disk ";
            list = dfsa->slDiskOne;
            sprintf( text, "%2hu",    SINF->disknr);
            TxSelStringSelect( list,  text, 2);
            break;

         case DFSO_PART:
            dfsDgWrimWidgets[DFSDGWRIMT2].sl.buf = "To  partition :";
            dfsDgWrimWidgets[DFSDGWRIML1].title  = " Restore image to partition ";
            list = dfsa->slPartOne;
            sprintf( text, "%2.2hu", SINF->partid);
            TxSelStringSelect( list,  text, 2);
            break;

         default:
            if (destination == DFSO_IMGF)
            {
               strcpy( tdescr, dfstStoreDesc2( DFSTORE) + 10);
               dfsDgWrimWidgets[DFSDGWRIMT2].sl.buf  = "To  imagefile :";
            }
            else                                // other types
            {
               strcpy( tdescr, dfstStoreDesc1( DFSTORE) + 10);
               dfsDgWrimWidgets[DFSDGWRIMT2].sl.buf  = "To    current :";
            }
            dfsDgWrimWidgets[DFSDGWRIMT1].flags  &= ~TXWI_DISABLED; // text field ON
            dfsDgWrimWidgets[DFSDGWRIML1].flags  |=  TXWI_DISABLED; // list field OFF
            break;
      }
      dfsDgWrimWidgets[DFSDGWRIML1].lb.list = list;
      strcpy( wildcard, "*.i??");

      if (dfsa->expertui)                       // set available widgets Expert/Basic
      {
         dfsDgWrimDlg.count = DFSDGWRIMWIDGETS;
         dfsDgWrimWidgets[DFSDGWRIMXL].flags  &= ~TXWI_DISABLED;
      }
      else
      {
         dfsDgWrimDlg.count = DFSDGWRIMWBASICS;
         dfsDgWrimWidgets[DFSDGWRIMXL].flags  |=  TXWI_DISABLED;
      }
      widgets = &dfsDgWrimDlg;
   }

   strcpy( fspec, wildcard);
   while (txwOpenFileDialog( fspec, NULL, image, DFSC_WRIMAGE, widgets, title, fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         TxStrip( fspec, fspec, ' ', ' ');
         sprintf( command, "restore \"%s\" ", fspec);
         TxStrip( wrime2, wrime2, ' ', ' ');
         if ((strlen( wrime2) != 0) &&          // Start value specified
             (isxdigit( wrime2[0]))  )          // and it is a number
         {
            sprintf( option, "%s%s", (wrimc4) ? "0x" : "", wrime2);
            strcat( command, option);
         }
         else
         {                                      // mandatory if size given,
            strcat( command, "0");              // positional parameter!
         }
         TxStrip( wrime3, wrime3, ' ', ' ');
         if ((strlen( wrime3) != 0) &&          // Size value specified
             (isxdigit( wrime3[0]))  )          // and it is a number
         {
            sprintf( option, " %s%s,%s", (wrimc4 && wrimrS) ? "0x" : "", wrime3,
                    (wrimrK) ? "k" : (wrimrM) ? "m" : (wrimrG) ? "g" : "s");
            strcat(  command, option);
            wrimc6 = FALSE;                     // ignore 'size from image'
         }
         if (list)
         {
            strcpy(  text, list->items[ list->selected]->text);
            TxStrip( text, text, ' ', ' ');     // strip leading spaces
            TxRepl(  text, ' ', 0);             // replace next space by 0
            switch (destination)                // select proper destination
            {
               case DFSO_VOLD: sprintf( option, " -V:\"%s\"", text);  break;
               case DFSO_DISK: sprintf( option, " -D:%s",     text);  break;
               case DFSO_PART: sprintf( option, " -P:%s",     text);  break;
               default:        strcpy(  option, "");                  break;
            }
            strcat( command, option);
         }
         if ( wrimr2 | verify) strcat( command, " -test");
         if ( wrimr3)          strcat( command, " -c");
         if ( wrimc2)          strcat( command, " -L");
         if ( wrimc6)          strcat( command, " -S");

         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_WRIMAGE, "You must specify an image filename, "
                                        "not a wildcard or directory ...");
         strcpy( fspec, wildcard);
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsWrimDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== CLONE ==============================================================*/
static BOOL        cloc1  = FALSE;              // Merge on error
static BOOL        cloc2  = FALSE;              // Ignore errors
static BOOL        cloc3  = FALSE;              // Exclude LVMsig
static BOOL        cloc4  = TRUE;               // HEX sector values
static BOOL        cloc5  = FALSE;              // COMPARE only
static BOOL        cloc6  = FALSE;              // SMART sector skip
static BOOL        cloc7  = FALSE;              // REVERSE High-to-Low

static TXTS        cloe1  = "";                 // From sector
static TXTS        cloe2  = "";                 // To   sector
static TXTS        cloe3  = "";                 // Size
static TXTS        cloe4  = "0";                // SkipBads
static BOOL        clorK  = FALSE;              // KiB
static BOOL        clorM  = FALSE;              // MiB
static BOOL        clorG  = FALSE;              // GiB
static BOOL        clorS  = TRUE;               // Sec
static BOOL        cloOK  = TRUE;               // OK button, dummy variable
static BOOL        cloCA  = FALSE;              // Cancel button dummy

/*
[ ] VERIFY only, compare FROM against TO    [ ] High-to-Low sectors
[ ] Skip unused areas in cloning (smart)    [ ] Ignore errors
[ ] Merge original data on read errors      [ ] Exclude LVM area

 From Partition:[04 ECS12       F: Log 07 HPFS        917.7 MiB ]

 To   Partition:[17                Log 07 -           917.7 MiB ]

                     Nr. of sectors skipped   ÚÄÄÄÄÄÄ¿  ÚÄÄÄÄÄÄÄÄ¿
     First sector    on any bad-sector area   ³  OK  ³  ³ Cancel ³
To  [............]        [..........]        ÀÄÄÄÄÄÄÙ  ÀÄÄÄÄÄÄÄÄÙ

     First sector  Size to do [û] Sector values are HEX, not decimal
From[............][Whole Area]() Sectors  ( ) KiB  ( ) MiB  ( ) GiB
*/
#define   DFSDGCLOWIDGETS 28                    // For Expert UI
#define   DFSDGCLOWBASICS 14                    // For Basic  UI
#define   DFSDGCLOCV       0                    // index of compare/verify
#define   DFSDGCLOHL       1                    // index of High to Low
#define   DFSDGCLOME       4                    // index of Merge Errors
#define   DFSDGCLOXL       5                    // index of exclude LVM
#define   DFSDGCLOD1       6                    // index of from description
#define   DFSDGCLOT1       7                    // index of from leader
#define   DFSDGCLOL1       8                    // index of from list-1
#define   DFSDGCLOD2       9                    // index of to description
#define   DFSDGCLOT2      10                    // index of to leader
#define   DFSDGCLOL2      11                    // index of to list-2
#define   DFSDGCLOOK      12                    // index of OK button
static TXWIDGET  dfsDgCloWidgets[DFSDGCLOWIDGETS] =    // order determines TAB-order!
{
   {0,  0, 1, 42, 0, 6, 0, TXWS_AUTOCHK, 0, TXStdButton( &cloc5, "COMPARE only, verify FROM against TO")},
   {0, 44, 1, 24, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &cloc7, "High-to-Low sectors")},
   {1,  0, 1, 42, 0, 6, 0, TXWS_AUTOCHK, 0, TXStdButton( &cloc6, "Skip unused areas in cloning (smart)")},
   {1, 44, 1, 24, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &cloc2, "Ignore errors")},
   {2,  0, 1, 42, 0, 6, 0, TXWS_AUTOCHK, 0, TXStdButton( &cloc1, "Merge original data on read errors")},
   {2, 44, 1, 24, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &cloc3, "Exclude LVM area")},

   {4, 17, 1, 50, 1, 0, 0, DFSDSOUTPUT,  0, TXStdStline( fdescr)},
   {4,  0, 1, 15, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( NULL)},
   {4, 16, 1, 50, 0, 0, 0, DFSDSLISTBOX, 0, TXStdLstBox( NULL, 0, NULL)},

   {7, 17, 1, 50, 1, 0, 0, DFSDSOUTPUT,  0, TXStdStline( tdescr)},
   {7,  0, 1, 15, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( NULL)},
   {7, 16, 1, 50, 0, 0, 0, DFSDSLISTBOX, 0, TXStdLstBox( NULL, 0, NULL)},

   {9, 45, 3,  8, 0, 0, TXDID_OK,
                           DFSDMPBUTTON, 0, TXStdButton( &cloOK, " OK ")},
   {9, 57, 3,  8, 0, 0, TXDID_CANCEL,
                           DFSDMPBUTTON, 0, TXStdButton( &cloCA, "Cancel")},

   {9, 20, 1, 24, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( "Nr. of sectors skipped")},
   {10,20, 1, 24, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( "on any bad-sector area")},

   {10, 4, 2, 14, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD2SIZE,
                                         0, TXStdEntryf( cloe1, TXMAXTS, "First sector")},
   {11, 0, 1,  4, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( "From")},

   {11,24, 1, 14, 0, 0, 0, TXWS_ENTRYB   |  TXWS_HCHILD2SIZE,
                                         0, TXStdEntryf( cloe4, TXMAXTS, "")},

   {13, 4, 2, 14, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD2SIZE,
                                         0, TXStdEntryf( cloe2, TXMAXTS, "First sector")},
   {13,18, 2, 12, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD2SIZE |  TXWS_HCHILD2MOVE,
                                         0, TXStdEntryf( cloe3, TXMAXTS, "Size to do")},
   {13,30, 1, 38, 0, 0, 0, DFSDMAUTOCHK, 0, TXStdButton( &cloc4, "Sector values are HEX, not decimal")},
   {14, 0, 1,  4, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( "To")},

   {14,30, 1, 11, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &clorS, "Sectors")},
   {14,43, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &clorK, "KiB")},
   {14,52, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &clorM, "MiB")},
   {14,61, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &clorG, "GiB")}
};

static TXGW_DATA dfsDgCloDlg =
{
   DFSDGCLOWIDGETS,                             // number of widgets
   DFSC_CLONDLG,                                // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgCloWidgets                              // array of widgets
};


/*************************************************************************************************/
// Present CLONE options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsCloneDialog
(
   BOOL                clone,                   // IN    Clone, not VERIFY
   DFSOBJECT           fobject,                 // IN    From object reference
   ULONG               fsector,                 // IN    From sector number
   DFSOBJECT           tobject,                 // IN    To   object reference
   ULONG               tsector,                 // IN    To   sector number
   ULONG               sizetodo                 // IN    Size to clone (0 = all)
)
{
   ULONG               rc = NO_ERROR;           // function return
   ULONG               fstart = 0;              // from start
   ULONG               tstart = 0;              // to   start
   ULONG               fselected;               // from selected
   ULONG               tselected;               // to   selected
   USHORT              focus;                   // field to get focus
   TXSELIST           *flist  = NULL;           // from list
   TXSELIST           *tlist  = NULL;           // to   list
   DFST_HANDLE         tstore = DFSTORE;        // DFSee STORE ids
   DFST_HANDLE         fstore = (tstore == 1) ? 2 : 1;
   DFSISTORE          *fsi;                     // FROM store info struct
   TXLN                command;
   TXTT                option;
   TXTM                text;

   ENTER();
   TRACES(( "%s  from:%8.8lx  at:%8.8lx  size:%8.8lx\n",
          (clone) ? "CLONE " : "VERIFY", fsector, tsector, sizetodo));

   dfsBEGINWORK();                              // signal work starting

   if (dfstQueryStoreType( fstore, &fsi, NULL) == DFST_UNUSED)
   {
      fsi = SINF;                               // when not set yet, default to
   }                                            // same as the CURRENT to-store
   dfsDgCloWidgets[DFSDGCLOD1].flags  |=  TXWI_DISABLED; // disable static
   dfsDgCloWidgets[DFSDGCLOL1].flags  &= ~TXWI_DISABLED; // enable list
   switch (fobject)
   {
      case DFSO_VOLD:
         dfsDgCloWidgets[DFSDGCLOT1].sl.buf = "From    volume:";
         dfsDgCloWidgets[DFSDGCLOL1].title  = " Clone from volume ";
         if (dfsa->slVolumes == NULL)
         {
            dfsa->slVolumes = TxFsDriveSelist( TXFSV_HD | TXFSV_CD |
                                               TXFSV_FLOP, FALSE);
         }
         rc = TxSelistDuplicate( dfsa->slVolumes, FALSE, TRUE, &flist);
         TxSelStringSelect( flist,  fsi->drive, 2);
         break;

      case DFSO_DISK:
         dfsDgCloWidgets[DFSDGCLOT1].sl.buf = "From      disk:";
         dfsDgCloWidgets[DFSDGCLOL1].title  = " Clone from disk ";
         rc = TxSelistDuplicate( dfsa->slDiskOne, FALSE, TRUE, &flist);
         sprintf( text, "%2hu",     fsi->disknr);
         TxSelStringSelect( flist,  text, 2);
         break;

      case DFSO_PART:
         dfsDgCloWidgets[DFSDGCLOT1].sl.buf = "From partition:";
         dfsDgCloWidgets[DFSDGCLOL1].title  = " Clone from partition ";
         rc = TxSelistDuplicate( dfsa->slPartOne, FALSE, TRUE, &flist);
         sprintf( text, "%2.2hu",  fsi->partid);
         TxSelStringSelect( flist,  text, 2);
         break;

      default:
         if (fobject == DFSO_IMGF)
         {
            strcpy( fdescr, dfstStoreDesc2( fstore) + 10);
            dfsDgCloWidgets[DFSDGCLOT1].sl.buf  = "From imagefile:";
         }
         else                                   // other types
         {
            strcpy( fdescr, dfstStoreDesc1( fstore) + 10);
            dfsDgCloWidgets[DFSDGCLOT1].sl.buf  = "From   current:";
         }
         dfsDgCloWidgets[DFSDGCLOD1].flags  &= ~TXWI_DISABLED;
         dfsDgCloWidgets[DFSDGCLOL1].flags  |=  TXWI_DISABLED;
         break;
   }
   dfsDgCloWidgets[DFSDGCLOL1].lb.list = flist;

   dfsDgCloWidgets[DFSDGCLOD2].flags  |=  TXWI_DISABLED; // disable static
   dfsDgCloWidgets[DFSDGCLOL2].flags  &= ~TXWI_DISABLED; // enable list
   switch (tobject)
   {
      case DFSO_VOLD:
         dfsDgCloWidgets[DFSDGCLOT2].sl.buf = "To      volume:";
         dfsDgCloWidgets[DFSDGCLOL2].title  = " Clone to volume ";
         if (dfsa->slVolumes == NULL)
         {
            dfsa->slVolumes = TxFsDriveSelist( TXFSV_HD | TXFSV_CD |
                                               TXFSV_FLOP, FALSE);
         }
         tlist = dfsa->slVolumes;
         TxSelStringSelect( tlist,  SINF->drive, 2);
         break;

      case DFSO_DISK:
         dfsDgCloWidgets[DFSDGCLOT2].sl.buf = "To        disk:";
         dfsDgCloWidgets[DFSDGCLOL2].title  = " Clone to disk ";
         tlist = dfsa->slDiskOne;
         sprintf( text, "%2hu",    SINF->disknr);
         TxSelStringSelect( tlist,  text, 2);
         break;

      case DFSO_PART:
         dfsDgCloWidgets[DFSDGCLOT2].sl.buf = "To   partition:";
         dfsDgCloWidgets[DFSDGCLOL2].title  = " Clone to partition ";
         tlist = dfsa->slPartOne;
         sprintf( text, "%2.2hu", SINF->partid);
         TxSelStringSelect( tlist,  text, 2);
         break;

      default:
         if (tobject == DFSO_IMGF)
         {
            strcpy( tdescr, dfstStoreDesc2( tstore) + 10);
            dfsDgCloWidgets[DFSDGCLOT2].sl.buf  = "To   imagefile:";
         }
         else                                   // other types
         {
            strcpy( tdescr, dfstStoreDesc1( tstore) + 10);
            dfsDgCloWidgets[DFSDGCLOT2].sl.buf  = "To     current:";
         }
         dfsDgCloWidgets[DFSDGCLOD2].flags  &= ~TXWI_DISABLED;
         dfsDgCloWidgets[DFSDGCLOL2].flags  |=  TXWI_DISABLED;
         break;
   }
   dfsDgCloWidgets[DFSDGCLOL2].lb.list = tlist;

   if      (flist != NULL) focus = DFSDGCLOL1;
   else if (tlist != NULL) focus = DFSDGCLOL2;
   else                    focus = DFSDGCLOOK;

   cloc5 = (clone == FALSE);                    // set COMPARE checkbox
   cloc4 = TRUE;
   sprintf( cloe1, "%lx", fsector);
   sprintf( cloe2, "%lx", tsector);             // set From/To sector numbers
   if (sizetodo != 0)
   {
      sprintf( cloe3, "%lx", sizetodo);         // set Size to clone  (0 = all)
   }
   else
   {
      strcpy( cloe3, "Whole Area");
   }

   if (dfsa->expertui)                          // set available widgets Expert/Basic
   {
      dfsDgCloDlg.count = DFSDGCLOWIDGETS;
      dfsDgCloWidgets[DFSDGCLOCV].flags  &= ~TXWI_DISABLED;
      dfsDgCloWidgets[DFSDGCLOHL].flags  &= ~TXWI_DISABLED;
      dfsDgCloWidgets[DFSDGCLOME].flags  &= ~TXWI_DISABLED;
      dfsDgCloWidgets[DFSDGCLOXL].flags  &= ~TXWI_DISABLED;
   }
   else
   {
      dfsDgCloDlg.count = DFSDGCLOWBASICS;
      dfsDgCloWidgets[DFSDGCLOCV].flags  |=  TXWI_DISABLED;
      dfsDgCloWidgets[DFSDGCLOHL].flags  |=  TXWI_DISABLED;
      dfsDgCloWidgets[DFSDGCLOME].flags  |=  TXWI_DISABLED;
      dfsDgCloWidgets[DFSDGCLOXL].flags  |=  TXWI_DISABLED;
   }

   while ((rc == NO_ERROR) && (txwWidgetDialog( TXHWND_DESKTOP, TXHWND_DESKTOP,
           NULL, " Specify cloning options and 'From' plus 'To' objects ",
           TXWD_MOVEABLE | TXWD_HCENTER | TXWD_VCENTER, focus, &dfsDgCloDlg)
        != TXDID_CANCEL))
   {
      sscanf( cloe1, "%lx", &fstart);
      sscanf( cloe2, "%lx", &tstart);            // get to/from start value

      fselected = (flist) ? flist->selected : 11111; // different values, always
      tselected = (tlist) ? tlist->selected : 22222; // larger than list sizes

      if ((fobject != tobject) || (fselected != tselected) || (fstart != tstart))
      {
         sprintf( command, "%s %s%s", (cloc5)         ? "comp" : "clone",
                                      (cloc4)         ? "0x"   : "",
                                      (strlen(cloe2)) ?  cloe2 : "0");
         if ((strlen( cloe3) != 0) &&           // Size value specified
             (isxdigit( cloe3[0]))  )           // and it is a number
         {
            TxStrip( cloe3, cloe3, ' ', ' ');
            sprintf( option, " %s%s,%s", (cloc4) ? "0x" : "", cloe3,
                    (clorK) ? "k" : (clorM) ? "m" : (clorG) ? "g" : "s");
            strcat(  command, option);
         }

         if ((strlen( cloe1) != 0)     &&       // From start sector
             (strcmp( cloe1, "0") != 0) )       // and not zero
         {
            TxStrip( cloe1, cloe1, ' ', ' ');
            sprintf( option, " -f:%s%s,%s", (cloc4) ? "0x" : "", cloe1,
                    (clorK) ? "k" : (clorM) ? "m" : (clorG) ? "g" : "s");
            strcat(  command, option);
         }

         if ((strlen( cloe4) != 0)     &&       // SkipBads sector value
             (strcmp( cloe4, "0") != 0) )       // and not zero
         {
            TxStrip( cloe4, cloe4, ' ', ' ');
            sprintf( option, " -skipbads:%s%s", (cloc4) ? "0x" : "", cloe4);
            if (strchr( cloe4, ',') == NULL)    // no explicit unit given
            {
               strcat( option, ",");
               strcat( option, (clorM) ? "m" : (clorG) ? "g" : "s");
            }
            strcat(  command, option);
         }

         if (flist)
         {
            strcpy(  text, flist->items[ flist->selected]->text);
            TxStrip( text, text, ' ', ' ');     // strip leading spaces
            TxRepl(  text, ' ', 0);             // replace next space by 0
            switch (fobject)                    // select proper source
            {
               case DFSO_VOLD: sprintf( option, " -v:\"%s\"", text);  break;
               case DFSO_DISK: sprintf( option, " -d:%s",     text);  break;
               case DFSO_PART: sprintf( option, " -p:%s",     text);  break;
               default:        strcpy(  option, "");                  break;
            }
            strcat( command, option);
         }

         if (tlist)
         {
            strcpy(  text, tlist->items[ tlist->selected]->text);
            TxStrip( text, text, ' ', ' ');     // strip leading spaces
            TxRepl(  text, ' ', 0);             // replace next space by 0
            switch (tobject)                    // select proper destination
            {
               case DFSO_VOLD: sprintf( option, " -V:\"%s\"", text);  break;
               case DFSO_DISK: sprintf( option, " -D:%s",     text);  break;
               case DFSO_PART: sprintf( option, " -P:%s",     text);  break;
               default:        strcpy(  option, "");                  break;
            }
            strcat( command, option);
         }

         if ( cloc1)         strcat( command, " -M");
         if ( cloc2)         strcat( command, " -E:i");
         if ( cloc3)         strcat( command, " -L");
         if ( cloc6)         strcat( command, " -S");
         if ( cloc7)         strcat( command, " -R");

         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_CLONING, "The selected FROM and TO objects/First sector seem "
                                        "to be the same!\n\nYou must specify a 'FROM' "
                                        "location that is DIFFERENT from the 'TO' location "
                                        "to be able to clone between them ...");
      }
   }
   if (flist)                                   // free Duplicated list
   {
      txSelDestroy( &flist);
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsCloneDialog'
/*-----------------------------------------------------------------------------------------------*/



/*========================== GENPART ============================================================*/
static BOOL        genc1  = FALSE;              // Sector based size/location
static BOOL        genc2  = FALSE;              // No location/freespace

static TXTM        gene1  = "";                 // Description

/*
    From   disk:  1 Maxtor 114.5 GiB      117239.9 MiB          ]

    Description:[                                                ]

[ ] Use sector based sizes and location, for exact copy on any GEO
[ ] Do NOT include freespace areas, use default location on create

    Specify base filename only, path is added, extension stripped
*/
#define   DFSDGGENWIDGETS 7
#define   DFSDGGENL1      1                     // index of list-1
static TXWIDGET  dfsDgGenWidgets[DFSDGGENWIDGETS] =    // order determines TAB-order!
{
   {0,  4, 1, 12, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( "From   disk:")},
   {0, 16, 1, 50, 0, 0, 0, DFSDSLISTBOX, 0, TXStdLstBox( NULL, 0, NULL)},

   {2,  4, 1, 12, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "Description:")},
   {2, 16, 1, 50, 0, 0, 0, TXWS_ENTRYB   |  TXWS_HCHILD2SIZE |  TXWS_HCHILD2MOVE,
                                         0, TXStdEntryf( gene1, TXMAXTM, "")},

   {4,  0, 1, 66, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &genc1,
                          "Use sector based sizes and location, for exact copy on any GEO")},
   {5,  0, 1, 66, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &genc2,
                          "Do NOT include freespace areas, use default location on create")},

   {7,  4, 1, 62, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline(
                          "Specify base filename only, path is added, extension stripped")},
};

static TXGW_DATA dfsDgGenDlg =
{
   DFSDGGENWIDGETS,                             // number of widgets
   DFSC_GENPDLG,                                // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgGenWidgets                              // array of widgets
};


/*************************************************************************************************/
// Present GENPART options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsGenPartDialog
(
   USHORT              disk,                    // IN    disk number string
   char               *base,                    // IN    base filename
   BOOL                sectors,                 // IN    use sector values
   BOOL                nofree                   // IN    no freespace
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXSELIST           *list = NULL;
   ULONG               item = 0;                // default to ALL disks

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   genc1 = sectors;
   genc2 = nofree;

   list = dfsa->slDiskAll;
   if ((disk != 0) && (disk != FDSK_ANY))
   {
      sprintf( command, "%2hu", disk);
      TxSelStringSelect( list, command, 2);
   }
   else
   {
      list->selected = 0;                       // default to 'all disks'
   }
   dfsDgGenWidgets[DFSDGGENL1].lb.list = list;
   dfsDgGenWidgets[DFSDGGENL1].title   = " Generate partition-script for disk ";

   strcpy( fspec, "*.dfs");
   while (txwSaveAsFileDialog( fspec, NULL, base, DFSC_GENPDLG, &dfsDgGenDlg,
      " Specify GENPART options and basename for the scriptfile ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         if (list)                              // get selected disk nr
         {
            item = list->items[ list->selected]->value -TXDID_MAX;
         }
         sprintf( command, "genpart %lu \"%s\" ", item, fspec);
         if ( genc1)         strcat( command, "-s ");
         if ( genc2)         strcat( command, "-f- ");
         if (strlen( gene1) != 0)              // Description specified
         {
            strcat(  command, gene1);
         }
         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_GENPDLG, "You must specify a script base filename, "
                                        "not a wildcard or directory ...");
         strcpy(  fspec, "*.dfs");
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsGenPartDialog'
/*-----------------------------------------------------------------------------------------------*/



/*========================== DFSDISK ============================================================*/
static BOOL        dskrA  = FALSE;              // Search ALL sectors (slow!)
static BOOL        dskrB  = TRUE;               // on MiB and Cyl boundaries
static BOOL        dskrC  = FALSE;              // Search Cyl only    (fast!)
static BOOL        dskrM  = FALSE;              // Search MiB only
static BOOL        dskr2  = FALSE;              // Partition, BR and LVM only
static BOOL        dskr3  = TRUE;               // Part, BR, LVM + ANY superblock
static BOOL        dskr4  = FALSE;              // Include HPFS   superblocks
static BOOL        dskr5  = FALSE;              // Include JFS    superblocks
static BOOL        dskr6  = FALSE;              // Include NTFS   NTLDR sectors
static BOOL        dskr7  = FALSE;              // Include HFS+   superblocks
static BOOL        dskj5  = FALSE;              // Include EXT2/3 superblocks
static BOOL        dskj6  = FALSE;              // Include REISER superblocks
static BOOL        dskj7  = FALSE;              // Include XFS    superblocks

static TXTS        dske1  = "calculated";       // #Cylinders
static TXTS        dske2  = "current";          // #Heads
static TXTS        dske3  = "current";          // #Sectors

/*
   Analyse disk: [1 Maxtor 114.5 GiB      117239.9 MiB          ]

() Partitioning and LVM only    #Cylinders     #Heads      #Sect/track
( ) Search HPFS   structures    [calculated]   [current]   [current    ]
( ) Search NTFS   structures
( ) Search JFS    structures   ( ) On MiB only  () On Cyl only (fastest)
( ) Search EXT2   structures   ( ) Search on Cylinder and MiB boundaries
( ) Search REISER structures   ( ) Search ALL sectors           (slowest)

*/
#define   DFSDGDSKWIDGETS 18
#define   DFSDGDSKL1      1                     // index of list-1
static TXWIDGET  dfsDgDskWidgets[DFSDGDSKWIDGETS] =    // order determines TAB-order!
{
   {0,  2, 1, 13, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( "Analyse disk:")},
   {0, 15, 1, 50, 0, 0, 0, DFSDSLISTBOX, 0, TXStdLstBox( NULL, 0, NULL)},

   {2, 31, 2, 12, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( dske1, TXMAXTS, "#Cylinders")},
   {2, 46, 2,  9, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( dske2, TXMAXTS, "#Heads")},
   {2, 58, 2, 13, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( dske3, TXMAXTS, "#Sect/track")},

   {2,  0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskr2, "Partitioning and LVM only")},
   {3,  0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskr3, "Part/LVM + ANY superblock")},
   {4,  0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskr4, "+ HPFS superblocks only")},
   {5,  0, 1, 13, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskr5, "+ JFS")},
   {6,  0, 1, 13, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskr6, "+ NTFS")},
   {7,  0, 1, 13, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskr7, "+ HFS+")},
   {5, 14, 1, 13, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskj5, "+ EXT2")},
   {6, 14, 1, 13, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskj6, "+ REISER")},
   {7, 14, 1, 13, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &dskj7, "+ XFS")},

   {5, 30, 1, 16, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &dskrM, "On MiB only")},
   {5, 47, 1, 26, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &dskrC,                  "On Cyl only (fastest)")},
   {6, 30, 1, 44, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &dskrB, "Search on Cylinder and MiB boundaries")},
   {7, 30, 1, 44, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &dskrA, "Search ALL sectors           (slowest)")},
};

static TXGW_DATA dfsDgDskDlg =
{
   DFSDGDSKWIDGETS,                             // number of widgets
   DFSC_DISKDLG,                                // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgDskWidgets                              // array of widgets
};


/*************************************************************************************************/
// Present DFSDISK options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsDfsDiskDialog
(
   USHORT              disk,                    // IN    disk number string
   char               *base                     // IN    base filename
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXSELIST           *list = NULL;
   ULONG               item = 0;                // default to ALL disks
   TXTM                title;
   TXGW_DATA          *widgets = NULL;

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   list = dfsa->slDiskAll;
   if ((disk != 0) && (disk != FDSK_ANY))
   {
      sprintf( command, "%2hu", disk);
      TxSelStringSelect( list, command, 2);
   }
   else
   {
      list->selected = 0;                       // default to 'all disks'
   }
   dfsDgDskWidgets[DFSDGDSKL1].lb.list = list;
   dfsDgDskWidgets[DFSDGDSKL1].title   = " Generate DFSDISK analysis for disk ";

   if (dfsa->expertui)
   {
      strcpy( title, " Specify DFSDISK options and basename for the report files ");
      widgets = &dfsDgDskDlg;
   }
   else
   {
      strcpy( title, " Specify a base filename for the analysis report files ");
   }

   strcpy( fspec, "*.sk?");
   while (txwSaveAsFileDialog( fspec, NULL, base, DFSC_DISKDLG, widgets, title, fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         if (list)                              // get selected disk nr
         {
            item = list->items[ list->selected]->value -TXDID_MAX;
         }
         sprintf( command, "dfsdisk %lu -b:\"%s\" ", item, fspec);
         if      ( dskr3)           strcat( command, "-s:ANY ");
         if      ( dskr4)           strcat( command, "-s:HPFS ");
         if      ( dskr5)           strcat( command, "-s:JFS ");
         if      ( dskr6)           strcat( command, "-s:NTFS ");
         if      ( dskr7)           strcat( command, "-s:HFS ");
         if      ( dskj5)           strcat( command, "-s:EXT2 ");
         if      ( dskj6)           strcat( command, "-s:REISER ");
         if      ( dskj7)           strcat( command, "-s:XFS ");
         if      ( dskrA)
         {
            strcat( command, "ALL");            // ALL sectors, very slow
         }
         else if ( dskrM)
         {
            strcat( command, "MiB");            // MiB boundaries only
         }
         else                                   // Cyl or Both, need GEO stuff
         {
            if   ( dskrC)
            {
               strcat( command, "CYL ");        // Cylinders only
            }
            else                                // default, do Both
            {
               strcat( command, "BOTH ");       // Cyl and MiB boundaries
            }
            if ((dske1[0] != 'c') || (dske2[0] != 'c') || (dske3[0] != 'c'))
            {
               strcat( command, (dske1[0] != 'c') ? dske1 : "?");
               strcat( command, " ");
               strcat( command, (dske2[0] != 'c') ? dske2 : "");
               strcat( command, " ");
               strcat( command, (dske3[0] != 'c') ? dske3 : "");
            }
         }
         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_DISKDLG, "You must specify a report base filename, "
                                        "not a wildcard or directory ...");
         strcpy(  fspec, "*.sk?");
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsDfsDiskDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== LOG ================================================================*/
static TXLN        logo1;                       // Output field user message
static BOOL        logc1;                       // Log-reopen at each line

/*
  Specify a filename on a WRITABLE volume/driveletter for logging
  All new screen output will be APPENDED to the specified file.")

  Create logfile for the session, or use Cancel/Esc for no logging

[ ] Close and re-open the logfile after writing each line (slow!)
*/
#define   DFSDGLOGWIDGETS 4
static TXWIDGET  dfsDgLogWidgets[DFSDGLOGWIDGETS] =    // order determines TAB-order!
{
   {0,  2, 1, 65, 0, 0, 1, DFSDSOUTPUT,  0, TXStdStline( fdescr)},
   {1,  2, 1, 65, 0, 0, 1, DFSDSOUTPUT,  0, TXStdStline( tdescr)},
   {3,  2, 1, 65, 0, 0, 1, DFSDSOUTPUT,  0, TXStdStline( logo1)},

   {5,  0, 1, 65, 0, 7, 0, TXWS_AUTOCHK, 0, TXStdButton( &logc1,
   "Close and reopen file after writing each line (slow!)")},
};

static TXGW_DATA dfsDgLogDlg =
{
   DFSDGLOGWIDGETS,                             // number of widgets
   DFSC_LOGF,                                   // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgLogWidgets                              // array of widgets
};


/*************************************************************************************************/
// Present LOG/TRACE options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsLogDialog
(
   char               *logname,                 // IN    default name or NULL
   ULONG               helpid,                  // IN    specific help-id
   BOOL                reopen,                  // IN    reopen logfile
   char               *message                  // IN    extra message or NULL
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   dfsDgLogDlg.helpid = helpid;
   logc1 = reopen;

   strcpy( fdescr, "Specify a filename on a WRITABLE volume/driveletter for logging");
   strcpy( tdescr, "All new screen output will be APPENDED to the specified file.");

   if (message && strlen( message))
   {
      strcpy( logo1, message);
   }
   else
   {
      strcpy( logo1, "Start logfile for the session, or use Cancel/Esc for no logging");
   }

   strcpy( fspec, "*.log");
   while (txwSaveAsFileDialog( fspec, NULL, logname, helpid, &dfsDgLogDlg,
      " Specify filename for logging this session to ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         if (helpid == DFSC_LOGF)               // regular LOG
         {
            sprintf( command, "log \"%s\"", fspec); // allow space/single-quote
         }
         else
         {
            sprintf( command, "trace \"%s\" -m:999,k -f:9 -t", fspec); // trace
         }
         if (logc1)
         {
            strcat( command, " -r");            // reopen option
         }
         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, helpid, "You must specify a filename, "
                                  "not a wildcard or directory ...");
         strcpy(  fspec, "*.log");
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsLogDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== HELP ===============================================================*/
static TXTM        hlogo;                       // Output field user message
static TXTM        descr;                       // Input field for optional description

/*
  Specify a filename on a WRITABLE volume/driveletter for logging
  All new screen output will be APPENDED to the specified file.")

  Create logfile for the session, or use Cancel/Esc for no logging

[ ] Close and re-open the logfile after writing each line (slow!)
*/
#define   DFSDGHELPWIDGETS 2
static TXWIDGET  dfsDgHelpWidgets[DFSDGHELPWIDGETS] =    // order determines TAB-order!
{
   {0,  2, 1, 65, 0, 0, 1, DFSDSOUTPUT,  0, TXStdStline( hlogo)},
   {2,  0, 1, 74, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf( descr, TXMAXTM, "")},
};

static TXGW_DATA dfsDgHelpDlg =
{
   DFSDGHELPWIDGETS,                            // number of widgets
   DFSC_H_LOAD,                                 // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgHelpWidgets                             // array of widgets
};


/*************************************************************************************************/
// Present HELP load-section from file dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsLoadHelpDialog
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   strcpy( hlogo, "Specify a description with the selected additional help file");

   strcpy( fspec, "*.txt");
   while (txwOpenFileDialog( fspec, NULL, NULL, DFSC_H_LOAD, &dfsDgHelpDlg,
      " Specify filename to load additional help from ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         sprintf( command, "loadhelp \"%s\" \"%s\"", fspec, descr);

         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_H_LOAD, "You must specify a filename, "
                                       "not a wildcard or directory ...");
         strcpy(  fspec, "*.txt");
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsLoadHelpDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== PRESTORE ===========================================================*/
static TXTS  pree1 = "";                        // Disknr to restore to
static TXTS  pree2 = "rebls";                   // Sectortypes to restore (ALL)

static BOOL  prec1 = FALSE;                     // Prompt for each sector to do
static BOOL  prec2 = FALSE;                     // List only, no sector restore
static BOOL  prec3 = FALSE;                     // Display full sector contents

/*
[SAME] Disknr to restore .PDx to  [rebls]  Restrict to specified types
'SAME' gets disknr from filename           Empty to restore *ANY* type
                                   r     = Master    boot  records
                                    e    = Extended  boot  records
[ ] Prompt for each sector to do     b   = Filesystem boot records
[ ] List only, no sector restore      l  = LVM information sectors
[ ] Display full sector contents       s = LVM  signature  sectors
*/
#define   DFSDGPREWIDGETS 14
static TXWIDGET  dfsDgPreWidgets[DFSDGPREWIDGETS] = // order determines TAB-order!
{
   {0,  0, 1,  6, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf( pree1, TXMAXTS, "")},
   {0,  7, 1, 25, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline(         "Disknr to restore .PDx to")},
   {1,  0, 1, 32, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline(  "'SAME' gets disknr from filename")},
   {4,  0, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &prec1, "Prompt for each sector to do")},
   {5,  0, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &prec2, "List only, no sector restore")},
   {6,  0, 1, 32, 0, 1, 0, TXWS_AUTOCHK, 0, TXStdButton( &prec3, "Display full sector contents")},

   {0, 34, 1,  7, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf( pree2, TXMAXTS, "")},
   {0, 43, 1, 27, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "Restrict to specified types")},
   {1, 43, 1, 27, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "Empty to restore *ANY* type")},
   {2, 35, 1, 35, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "r     = Master    boot  records")},
   {3, 35, 1, 35, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( " e    = Extended  boot  records")},
   {4, 35, 1, 35, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "  b   = Filesystem boot records")},
   {5, 35, 1, 35, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "   l  = LVM information sectors")},
   {6, 35, 1, 35, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "    s = LVM  signature  sectors")}
};

static TXGW_DATA dfsDgPreDlg =
{
   DFSDGPREWIDGETS,                             // number of widgets
   DFSC_RESTPD,                                 // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgPreWidgets                              // array of widgets
};

/*************************************************************************************************/
// Present PRESTORE options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsPrestoreDialog
(
   USHORT              disknr                   // IN    disknr or 0
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXTS                wildcard;

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   if (disknr == 0)
   {
      strcpy( pree1, "SAME");
   }
   else
   {
      sprintf( pree1, "%hu", disknr);
   }
   strcpy( wildcard, "*.pd?");
   strcpy( fspec, wildcard);
   while (txwOpenFileDialog( fspec, NULL, NULL, DFSC_RESTPD, &dfsDgPreDlg,
          " Select .PDx file to be restored, and set options ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         sprintf( command, "prestore %s \"%s\" %s",
                  (isdigit( pree1[0])) ?  pree1 : "*",
                   fspec,   pree2);

         if (!prec1) strcat( command, " -c-");  // no prompting
         if ( prec2) strcat( command, " -l");   // list only
         if ( prec3) strcat( command, " -v");   // verbose

         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_RESTPD, "You must specify a .PDx filename, "
                                       "not a wildcard or directory ...");
         strcpy( fspec, wildcard);
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsPrestoreDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== PSAVE ==============================================================*/
static TXTM  psae1 = "Partition info backup made from DFSee menu selection";

/*
 Textual description to be added to the partition BACKUP files
[                                                                ]
*/
#define   DFSDGPSAWIDGETS 1
static TXWIDGET  dfsDgPsaWidgets[DFSDGPSAWIDGETS] = // order determines TAB-order!
{
   {0,  0, 2, 57, 0, 0, 0, TXWS_ENTRYBT  |  TXWS_HCHILD_SIZE, 0, TXStdEntryf( psae1, TXMAXTM,
                          "Description to be added to the partition BACKUP files")}
};

static TXGW_DATA dfsDgPsaDlg =
{
   DFSDGPSAWIDGETS,                             // number of widgets
   DFSC_SAVEPD,                                 // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgPsaWidgets                              // array of widgets
};

/*************************************************************************************************/
// Present PSAVE options dialog and execute resulting command
/*************************************************************************************************/
ULONG dfsPsaveDialog
(
   USHORT              disknr                   // IN    disknr or 0
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   TXLN                fspec;
   TXTS                wildcard;

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   strcpy( wildcard, "*.pd?");
   strcpy( fspec, wildcard);
   while (txwSaveAsFileDialog( fspec, NULL, DFS_X, DFSC_SAVEPD, &dfsDgPsaDlg,
       " Specify filename and description for partition info BACKUP ", fspec))
   {
      if ((fspec[strlen(fspec)-1] != FS_PATH_SEP) && // not a directory ?
                (strlen(fspec) == TxStrWcnt(fspec))) // and no wildcard ?
      {
         sprintf( command, "psave %hu \"%s\" %s", disknr, fspec, psae1);

         dfsExecEnd( command);
         break;
      }
      else
      {
         TxMessage( TRUE, DFSC_SAVEPD, "You must specify a .PDx filename, "
                                       "not a wildcard or directory ...");
         strcpy( fspec, wildcard);
      }
   }
   dfsENDWORK();                                // signal work done
   RETURN (rc);
}                                               // end 'dfsPsaveDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== LIST/RECOVER files =================================================*/
static BOOL flsr1  = TRUE;                      // Radio Minimum
static BOOL flsr2  = FALSE;                     // Radio Maximum
static BOOL flsrK  = TRUE;                      // KiB
static BOOL flsrM  = FALSE;                     // MiB
static BOOL flsrG  = FALSE;                     // GiB
static BOOL flsrS  = FALSE;                     // Sec

static TXTS flse1  = "100";                     // percentage value
static TXTS flse2  = "0";                       // minimum filesize
static TXTS flse3  = "*";                       // maximum filesize

/*
    Recovery reliability                        File size  () KiB
() minimum  percentage                minimum [.........] ( ) MiB
( ) maximum [..........]               maximum [.........] ( ) GiB
                                                           ( ) sectors
*/
#define   DFSDGFLISTWIDGETS 13
static TXWIDGET  dfsDgFlistWidgets[DFSDGFLISTWIDGETS] =  // order determines TAB-order!
{
   {0,  4, 1, 22, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "Recovery reliability")},
   {1,  0, 1, 12, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &flsr1, "minimum")},
   {2,  0, 1, 12, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &flsr2, "maximum")},
   {1, 12, 2, 12, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( flse1, TXMAXTS, "percentage")},

   {0, 48, 1, 11, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "File size")},
   {1, 39, 1,  8, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "minimum")},
   {1, 47, 1, 11, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf( flse2, TXMAXTS, "")},
   {2, 39, 1,  8, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "maximum")},
   {2, 47, 1, 11, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf( flse3, TXMAXTS, "")},

   {0 ,59, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &flsrK, "KiB")},
   {1 ,59, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &flsrM, "MiB")},
   {2 ,59, 1,  7, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &flsrG, "GiB")},
   {3, 59, 1, 11, 0, 4, 0, DFSDMAUTORAD, 0, TXStdButton( &flsrS, "Sectors")}
};

#define DFSDGFLIST_HELP    5208

static TXGW_DATA dfsDgFlistDlg =
{
   DFSDGFLISTWIDGETS,                           // number of widgets
   DFSDGFLIST_HELP,                             // dialog help
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgFlistWidgets                            // array of widgets
};

static TXLN   wildcard = "";                    // wildcard string


/*************************************************************************************************/
// Present LIST/RECOVER file selection dialog and return compound selection string
/*************************************************************************************************/
ULONG dfsFileListDialog                         // RET   result
(
   char               *title,                   // IN    dialog title
   ULONG               count,                   // IN    listed files, info only
   TXTM                select                   // INOUT file selection string
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                prompt;                  // prompt text
   ULONG               threshold = 0;           // threshold percentage
   ULONG               minS;                    // minimal size
   ULONG               maxS;                    // maximal size

   ENTER();

   if (strlen( select))                         // string on input ?
   {
      dfsParseFileSelection( select, wildcard, &flsr1, &threshold, &minS, &maxS);
      sprintf( flse1, "%lu",   threshold);
      sprintf( flse2, "%lu,s", minS);
      sprintf( flse3, "%lu,s", maxS);
   }
   sprintf( prompt, "The sector list currently contains "
                    "%lu potential filenames.\n\n"
                    "Specify a wildcard for path+filename "
                    "or leave blank to show entire list.", count);

   if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &dfsDgFlistDlg,
         prompt, title, DFSDGFLIST_HELP,
         TXPB_MOVEABLE | TXPB_HCENTER |  TXPB_VCENTER,
         TXMAXTM - TXMAXTS, wildcard) != TXDID_CANCEL)
   {
      if (strlen( wildcard))
      {
         strcpy( select, wildcard);
      }
      else
      {
         strcpy( select, "*");                  // explicit 'everything'
      }
      strcpy( select, wildcard);
      strcat( select, "%");
      if (strlen( flse1))
      {
         if ( flsr2) strcat( select, "-");
         strcat(             select, flse1);
      }
      strcat( select, "%");
      TxStrip( flse2, flse2, ' ', ' ');
      strcat( select, flse2);
      if ( flsrK) strcat( select, ",k");
      if ( flsrM) strcat( select, ",m");
      if ( flsrG) strcat( select, ",g");
      if ( flsrS) strcat( select, ",s");
      strcat( select, "%");
      TxStrip( flse3, flse3, ' ', ' ');
      strcat( select, flse3);
      if ( flsrK) strcat( select, ",k");
      if ( flsrM) strcat( select, ",m");
      if ( flsrG) strcat( select, ",g");
      if ( flsrS) strcat( select, ",s");
   }
   else
   {
      rc = DFS_NO_CHANGE;
   }
   RETURN (rc);
}                                               // end 'dfsFileListDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== SLT display ========================================================*/
static BOOL sltr1  = TRUE;                      // Radio every
static BOOL sltr2  = FALSE;                     // Radio error-only
static BOOL sltr3  = FALSE;                     // Radio error-value

static BOOL sltr4  = FALSE;                     // Radio default format
static BOOL sltr5  = FALSE;                     // Radio verbose
static BOOL sltr6  = TRUE;                      // Radio verbose 1-liner

static BOOL sltr7  = TRUE;                      // Radio start
static BOOL sltr8  = FALSE;                     // Radio to-end
static BOOL sltr9  = FALSE;                     // Radio at-sector

static BOOL sltc1  = FALSE;                     // Force rebuild

static TXTS slte0  = "*";                       // sector type
static TXTS slte1  = "";                        // error mask
static TXTS slte2  = "";                        // slt start index
static TXTS slte3  = "this";                    // at hex sector

static TXTM slto1  = "";                        // SLT status string

/*
SLT Areas:  4132      Ctrl+R refreshes  status: Being built now

[ ] Rebuild SLT                Display areas for sector type: [..]

() One-line incl filenames   () Display every SLT entry
( ) Multiple-lines, verbose   ( ) Errors only  (CHECK)
( ) Default area info only    ( ) For hex ERROR flags: [.........]

() Show SLT from default or a specified start index:  [.........]
( ) Show SLT area exactly to end of table/filesystem
( ) Show SLT area containing a specified hex sector:   [.........]
*/
#define   DFSDGSLTWIDGETS 16
static TXWIDGET  dfsDgSltWidgets[DFSDGSLTWIDGETS] =  // order determines TAB-order!
{
   {0,  0, 1, 66, 0, 0, 0, DFSDSOUTPUT,  0, "", TXW_STLINE, 0, dfsSltWinProc, TXWgStline(slto1)},

   {2,  0, 1, 16, 0, 0, 0, DFSDMAUTOCHK, 0, TXStdButton( &sltc1, "Rebuild SLT")},
   {2, 31, 1, 30, 0, 0, 0, DFSDSOUTPUT,  0, TXStdStline( "Display areas for sector type:")},
   {2, 62, 1,  4, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf(  slte0,  TXMAXTS, "")},

   {4,  0, 1, 28, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr6, "One-line incl filenames")},
   {5,  0, 1, 28, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr5, "Multiple-lines, verbose")},
   {6,  0, 1, 28, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr4, "Default area info only ")},
   {4, 30, 1, 36, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr1, "Display every SLT entry")},
   {5, 30, 1, 26, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr2, "Errors only  (CHECK)")},
   {6, 30, 1, 26, 0, 2, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr3, "For hex ERROR flags:")},
   {6, 55, 1, 11, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf(  slte1,  TXMAXTS, "")},


   {8,  0, 1, 54, 0, 3, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr7, "Show SLT from default or a specified start index:")},
   {9,  0, 1, 54, 0, 3, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr8, "Show SLT area exactly to end of table/filesystem")},
   {10, 0, 1, 54, 0, 3, 0, TXWS_AUTORAD, 0, TXStdButton( &sltr9, "Show SLT area containing a specified hex sector:")},
   {8, 55, 1, 11, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf(  slte2,  TXMAXTS, "")},
   {10,55, 1, 11, 0, 0, 0, TXWS_ENTRYB,  0, TXStdEntryf(  slte3,  TXMAXTS, "")},
};

#define DFSDGSLT_HELP      5555

static TXGW_DATA dfsDgSltDlg =
{
   DFSDGSLTWIDGETS,                             // number of widgets
   DFSDGSLT_HELP,                               // SLT dialog help
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgSltWidgets                              // array of widgets
};

static TXTT   pagesize = "";                    // SLT pagesize

/*************************************************************************************************/
// Present SLT display option dialog and execute resulting SLT command
/*************************************************************************************************/
ULONG dfsSltDisplayDialog                       // RET   result
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;

   ENTER();

   if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &dfsDgSltDlg,
         "Specify number of lines to display, '*' to show ALL\n"
         "or leave empty for a default page size of one screen ...",
         " Specify options for displaying the Sector Lookup Table ",
         DFSDGSLT_HELP, TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER,
         TXMAXTT, pagesize) != TXDID_CANCEL)
   {
      TxStrip( slte2, slte2, ' ', ' ');
      sprintf( command, "slt %s %s", strlen(slte2) ? slte2 : ".", pagesize);

      if (sltr2) strcat( command, " -m");       // errors only
      if (sltr3)
      {
         TxStrip( slte1, slte1, ' ', ' ');
         strcat( command, " -m:0x");            // specifies errors only
         strcat( command, slte1);
      }

      if (sltr4) strcat( command, " -v- -1-");  // simple
      if (sltr5) strcat( command, " -v  -1-");  // verbose
      if (sltr6) strcat( command, " -v  -1");   // verbose 1-liners

      if (sltr8) strcat( command, " -e");       // upto end
      if (sltr9)
      {
         TxStrip( slte3, slte3, ' ', ' ');
         strcat( command, " -a:");              // at specified LSN
         strcat( command, slte3);
      }
      if (slte0[0] != '*')
      {
         strcat( command, " -t:");              // specific type
         strcat( command, slte0);
      }
      if      (sltc1) strcat( command, " -r");  // force rebuild

      #if defined (HAVETHREADS)
         if (dfsSlTableStatus(NULL) == SLT_UPDATE) // not updating
         {
            TxMessage( TRUE, 5556,
               "      The Sector Lookup Table is not ready yet.\n\n"
               "Results will be displayed automatically once complete.\n"
               "While waiting, the <Esc> key will return you to the menu\n"
               "or cmdline to do other work on the same partition/volume");
         }
      #endif
      dfsProgressSuspend();
      dfsExecEnd( command);
   }
   else
   {
      rc = DFS_NO_CHANGE;
   }
   RETURN (rc);
}                                               // end 'dfsSltDisplayDialog'
/*-----------------------------------------------------------------------------------------------*/


/*****************************************************************************/
// Dialog window procedure, for the SLT-display dialog field; update status
/*****************************************************************************/
static ULONG dfsSltWinProc                      // RET   result
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
      switch (msg)
      {
         case TXWM_CREATE:                      // setup event hook
            txwAttachEventHook( DFS_EHK_SLTBUILD, hwnd);
            break;

         case TXWM_HOOKEVENT:                   // force a repaint ...
            txwInvalidateWindow( hwnd, FALSE, FALSE);
            break;

         case TXWM_DESTROY:                     // cancel event hook
            txwDetachEventHook( DFS_EHK_SLTBUILD, hwnd);
            break;

         case TXWM_PAINT:                       // update before each paint
            dfsGetSltStatusString( slto1, "Ctrl+R refreshes");
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
}                                               // end 'dfsSltWinProc'
/*---------------------------------------------------------------------------*/


/*========================== CHS-STYLE ======================================*/
static BOOL        chsrI  = FALSE;              // IBM
static BOOL        chsrP  = FALSE;              // PQ
static BOOL        chsrM  = FALSE;              // MS
static BOOL        chsOK  = FALSE;              // OK button, dummy variable
static BOOL        chsCA  = FALSE;              // Cancel button dummy
static BOOL        chsc1  = FALSE;              // make default, skip dialog

/*
( ) IBM (FDISK, LVM) and default DFSee style
( ) PowerQuest (Partition Magic, Ghost, DriveImage
( ) Microsoft Windows old standard style

[û] Make this default and skip the dialog from now on

        ÚÄÄÄÄÄÄ¿              ÚÄÄÄÄÄÄÄÄ¿
        ³  OK  ³              ³ Cancel ³
        ÀÄÄÄÄÄÄÙ              ÀÄÄÄÄÄÄÄÄÙ
*/

#define   DFSDGCHSOK       4                    // index of OK button
#define   DFSDGCHSWIDGETS  6
static TXWIDGET  DfsDgChsWidgets[DFSDGCHSWIDGETS] =    // order determines TAB-order!
{
   {0, 0,  1, 51, 0, 1, 0, DFSDMAUTORAD, 0, TXStdButton( &chsrI, "IBM (FDISK, LVM) and default DFSee style")},
   {1, 0,  1, 51, 0, 1, 0, DFSDMAUTORAD, 0, TXStdButton( &chsrP, "PowerQuest (Partition Magic, Ghost, DriveImage)")},
   {2, 0,  1, 51, 0, 1, 0, DFSDMAUTORAD, 0, TXStdButton( &chsrM, "Microsoft Windows old standard style")},

   {4, 0,  1, 54, 0, 2, 0, TXWS_AUTOCHK, 0, TXStdButton( &chsc1, "Make this default and skip the dialog from now on")},

   {6, 10, 3,  8, 0, 0, TXDID_OK,     DFSDMPBUTTON, 0, TXStdButton( &chsOK, " OK ")},
   {6, 30, 3,  8, 0, 0, TXDID_CANCEL, DFSDMPBUTTON, 0, TXStdButton( &chsCA, "Cancel")}
};

#define DFSDGCHS_HELP      5560

static TXGW_DATA DfsDgChsDlg =
{
   DFSDGCHSWIDGETS,                             // number of widgets
   DFSDGCHS_HELP,                               // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   DfsDgChsWidgets                              // array of widgets
};

static ULONG  defaultChsStyle = DFS_CHSTYLE_NONE;

/*************************************************************************************************/
// Present CHS style selection dialog and return the selected value, default dfsa->chsStyle
/*************************************************************************************************/
ULONG dfsChsStyleDialog                         // RET   result
(
   BOOL                reset,                   // IN    Reset default style
   ULONG              *style                    // OUT   Clone, not VERIFY
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if (reset)
   {
      defaultChsStyle = DFS_CHSTYLE_NONE;
   }
   if (defaultChsStyle == DFS_CHSTYLE_NONE)     // not made default yet ...
   {
      if ((!chsrI) && (!chsrP) && (!chsrM))     // no dialog history yet
      {
         switch (dfsa->chsStyle)
         {
            case DFS_CHSTYLE_PQ: chsrP = TRUE; break;
            case DFS_CHSTYLE_MS: chsrM = TRUE; break;
            default:             chsrI = TRUE; break;
         }
      }

      if (txwWidgetDialog( TXHWND_DESKTOP, TXHWND_DESKTOP,
              NULL, " Specify the CHS (dummy) style to be used ",
              TXWD_MOVEABLE | TXWD_HCENTER | TXWD_VCENTER,
              DFSDGCHSOK,   &DfsDgChsDlg) != TXDID_CANCEL)
      {
         if      (chsrP) *style = DFS_CHSTYLE_PQ;
         else if (chsrM) *style = DFS_CHSTYLE_MS;
         else            *style = DFS_CHSTYLE_IBM;

         if (chsc1)                             // set this as default too
         {
            defaultChsStyle = *style;           // for this dialog
            dfsa->chsStyle  = *style;           // and other ChsStyle usage
         }
      }
      else
      {
         rc = DFS_NO_CHANGE;
      }
   }
   else                                         // just return the value that
   {                                            // was made the default
      *style = defaultChsStyle;
   }
   RETURN (rc);
}                                               // end 'dfsChsStyleDialog'
/*---------------------------------------------------------------------------*/


/*================== PROMPT entryfield plus 1 to 3 options ==================*/
/*
[û] Description for checkbox 0
[û] Description for checkbox 1
[û] Description for checkbox 2
*/
#define   DFSDGPROPTWIDGETS  3
static TXWIDGET  dfsDgPrOptWidgets[DFSDGPROPTWIDGETS] =  // order determines TAB-order!
{
   {0,  0, 1, TXMAXTM, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( NULL, NULL)},
   {1,  0, 1, TXMAXTM, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( NULL, NULL)},
   {2,  0, 1, TXMAXTM, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( NULL, NULL)}
};

static TXGW_DATA dfsDgPrOptDlg =
{
   0,                                           // number of widgets, dynamic
   0,                                           // dialog help, dynamic
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgPrOptWidgets                            // array of widgets
};


/*************************************************************************************************/
// Present PromptBox for a string value, preceeded by 1 to 3 option checkboxes
/*************************************************************************************************/
ULONG dfsPromptOptDialog                        // RET   result
(
   char               *title,                   // IN    dialog title
   char               *message,                 // IN    prompt message
   ULONG               helpid,                  // IN    dialog help
   short               length,                  // IN    entryfield length
   char               *field,                   // INOUT entryfield string
   BOOL               *opt0val,                 // INOUT Option-0 value
   char               *opt0txt,                 // IN    Option-0 text
   BOOL               *opt1val,                 // INOUT Option-1 value or NULL
   char               *opt1txt,                 // IN    Option-1 text  or NULL
   BOOL               *opt2val,                 // INOUT Option-2 value or NULL
   char               *opt2txt                  // IN    Option-2 text  or NULL
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   dfsDgPrOptDlg.helpid = helpid;

   dfsDgPrOptDlg.count  = 1;
   dfsDgPrOptDlg.widget[0].bu.text          = opt0txt;
   dfsDgPrOptDlg.widget[0].bu.checked       = opt0val;
   if (opt1val)
   {
      dfsDgPrOptDlg.count = 2;
      dfsDgPrOptDlg.widget[1].bu.text       = opt1txt;
      dfsDgPrOptDlg.widget[1].bu.checked    = opt1val;
      if (opt2val)
      {
         dfsDgPrOptDlg.count = 3;
         dfsDgPrOptDlg.widget[2].bu.text    = opt2txt;
         dfsDgPrOptDlg.widget[2].bu.checked = opt2val;
      }
   }

   if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, &dfsDgPrOptDlg,
                     message, title, helpid,
                     TXPB_MOVEABLE | TXPB_HCENTER |  TXPB_VCENTER,
                     length, field) == TXDID_CANCEL)
   {
      rc = DFS_NO_CHANGE;
   }
   RETURN (rc);
}                                               // end 'dfsPromptOptDialog'
/*-----------------------------------------------------------------------------------------------*/


/*========================== DIRSELECT + CHECKBOX ===========================*/
/*
[û] Description for checkbox 0
[û] Description for checkbox 1
[û] Description for checkbox 2
*/
#define   DFSDGDIRSWIDGETS 3
static TXWIDGET  dfsDgDirsWidgets[DFSDGDIRSWIDGETS] =  // order determines TAB-order!
{
   {0,  0, 1, TXMAXTM, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( NULL, NULL)},
   {1,  0, 1, TXMAXTM, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( NULL, NULL)},
   {2,  0, 1, TXMAXTM, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( NULL, NULL)}
};

static TXGW_DATA dfsDgDirsDlg =
{
   0,                                           // number of widgets, dynamic
   0,                                           // dialog help, dynamic
   810,                                         // base window ID
   NULL,                                        // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgDirsWidgets                             // array of widgets
};


/*************************************************************************************************/
// Present Directory select dialog, with one to three optional Checkboxes
/*************************************************************************************************/
BOOL dfsDirSelectDialog
(
   char               *dlgTitle,                // IN    dialog title
   ULONG               helpid,                  // IN    dialog help
   char               *dirPath,                 // INOUT dir path to select
   BOOL               *opt0val,                 // INOUT Option-0 value
   char               *opt0txt,                 // IN    Option-0 text
   BOOL               *opt1val,                 // INOUT Option-1 value or NULL
   char               *opt1txt,                 // IN    Option-1 text  or NULL
   BOOL               *opt2val,                 // INOUT Option-2 value or NULL
   char               *opt2txt                  // IN    Option-2 text  or NULL
)
{
   BOOL                rc;                      // function return

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   dfsDgDirsDlg.helpid = helpid;
   dfsDgDirsDlg.count  = 1;
   dfsDgDirsDlg.widget[0].bu.text          = opt0txt;
   dfsDgDirsDlg.widget[0].bu.checked       = opt0val;
   if (opt1val)
   {
      dfsDgDirsDlg.count = 2;
      dfsDgDirsDlg.widget[1].bu.text       = opt1txt;
      dfsDgDirsDlg.widget[1].bu.checked    = opt1val;
      if (opt2val)
      {
         dfsDgDirsDlg.count = 3;
         dfsDgDirsDlg.widget[2].bu.text    = opt2txt;
         dfsDgDirsDlg.widget[2].bu.checked = opt2val;
      }
   }

   rc = txwSelDirFileDialog( dirPath, helpid, &dfsDgDirsDlg, dlgTitle, dirPath);

   dfsENDWORK();                                // signal work done
   BRETURN (rc);
}                                               // end 'dfsDirSelectDialog'
/*-----------------------------------------------------------------------------------------------*/


/*===================================== GEO =====================================================*/
static BOOL        georReset;                   // Reset to system geometry
static BOOL        georCl255;                   // Classic 255/63  (desktop)
static BOOL        georCl240;                   // Classic 240/63  (laptops)
static BOOL        georMbCyl;                   // 1 MiB    64/32  (SSD/4K sects)
static BOOL        geor1Tos2;                   // OS2/eCS 255/127 (1 Tb limit)
static BOOL        geor2Tos2;                   // OS2/eCS 255/255 (2 Tb limit)
static BOOL        georCustm;                   // Custom geometry, values:

static TXTM        geooX  = "";                 // Size as reported in 8 positions

static TXTM        geosX  = "";                 // Size for geo in 8 positions
static TXTS        geosC  = "";                 // #Cylinders, system geo
static TXTS        geosH  = "";                 // #Heads
static TXTS        geosS  = "";                 // #Sectors

static TXTM        geolX  = "";                 // Size for geo in 8 positions
static TXTS        geolC  = "";                 // #Cylinders, logical geo
static TXTS        geolH  = "";                 // #Heads
static TXTS        geolS  = "";                 // #Sectors

static TXTS        geoeC  = "";                 // #Cylinders
static TXTS        geoeH  = "";                 // #Heads
static TXTS        geoeS  = "";                 // #Sectors

static BOOL        geoOK  = TRUE;               // OK button, dummy variable
static BOOL        geoCA  = FALSE;              // Cancel button dummy

/*
 Geometry info for disk: [04 ECS12       F: Log 07 HPFS        917.7 MiB ]

    Current  system geometry          12345678       255          63
    Current logical geometry          12345678       255          63

( ) Reset to system geometry         #Cylinders     #Heads      #Sect/track
() Custom geometry, values:        [calculated]   [current]   [current    ]
( ) Classic 255/63, desktop/3.5"
( ) Classic 240/63, laptops/2.5"
( ) MiB Cyl. 64/32, SSD/4K sect    ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
( ) OS2/eCS 255/127, 1 Tb limit    ³ Set Logical Geometry ³  ³ Cancel/Done ³
( ) OS2/eCS 255/255, 2 Tb limit    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

*/
#define   DFSDGGEOWIDGETS 23
#define   DFSDGGEOT1       0                    // index of from leader
#define   DFSDGGEOD1       1                    // index of from description
#define   DFSDGGEOSIZEOS   2
#define   DFSDGGEOSYSTXT   3
#define   DFSDGGEOSYSCYL   4
#define   DFSDGGEOSYSHEAD  5
#define   DFSDGGEOSYSSECT  6
#define   DFSDGGEOLOGTXT   7
#define   DFSDGGEORESET   11
#define   DFSDGGEOCL255   12
#define   DFSDGGEOCL240   13
#define   DFSDGGEOMIBCYL  14
#define   DFSDGGEO1TBOS2  15
#define   DFSDGGEO2TBOS2  16
#define   DFSDGGEOCUSTOM  17
#define   DFSDGGEOCE      21                    // cylinder entry field
#define   DFSDGGEOOK      22                    // index of OK button

static TXWIDGET  dfsDgGeoWidgets[DFSDGGEOWIDGETS] =    // order determines TAB-order!
{
   {0,  0, 1, 20, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( "Geometry info for : ")},
   {0, 24, 1, 48, 1, 0, 0, DFSDSOUTPUT,  0, TXStdStline( fdescr)},

   {2,  0, 1, 70, 1, 0, 0, DFSDSOUTPUT,  0, TXStdStline( geooX)},

   {3,  0, 1, 38, 1, 0, 0, DFSDSOUTPUT,  0, TXStdStline( geosX)},
   {3, 36, 1,  8, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( geosC)},
   {3, 49, 1,  3, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( geosH)},
   {3, 59, 1,  3, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( geosS)},

   {4,  0, 1, 38, 1, 0, 0, DFSDSOUTPUT,  0, TXStdStline( geolX)},
   {4, 36, 1,  8, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( geolC)},
   {4, 49, 1,  3, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( geolH)},
   {4, 59, 1,  3, 0, 0, 0, TXWS_OUTPUT,  0, TXStdStline( geolS)},

   { 6, 0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &georReset, "Reset to system geometry")},
   { 8, 0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &georCl255, "Classic 255/63, desktop/3.5\"")},
   { 9, 0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &georCl240, "Classic 240/63, laptops/2.5\"")},
   {10, 0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &georMbCyl, "MiB Cyl. 64/32, SSD/4K sect")},
   {11, 0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &geor1Tos2, "OS2/eCS 255/127, 1 Tb limit")},
   {12, 0, 1, 32, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &geor2Tos2, "OS2/eCS 255/255, 2 Tb limit")},

   //- moved in between other radios in same group and (next) input field for better backtab order
   { 7, 0, 1, 34, 0, 1, 0, TXWS_AUTORAD, 0, TXStdButton( &georCustm, "Custom geometry, using values:")},

   {6, 35, 2, 12, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( geoeC, TXMAXTS, "#Cylinders")},
   {6, 48, 2,  9, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( geoeH, TXMAXTS, "#Heads")},
   {6, 58, 2, 13, 0, 0, 0, TXWS_ENTRYBT, 0, TXStdEntryf( geoeS, TXMAXTS, "#Sect/track")},

   {10,35, 3, 22, 0, 0, TXDID_OK,
                           DFSDMPBUTTON, 0, TXStdButton( &geoOK, "Set Logical Geometry")},
   {10,58, 3, 13, 0, 0, TXDID_CANCEL,
                           DFSDMPBUTTON, 0, TXStdButton( &geoCA, "Cancel/Done")},
};


#define WID_GEODLG           810
#define DFSGEODLG_RECREATE  (TXDID_MAX -1)      // recreate the dialog

static TXGW_DATA dfsDgGeoDlg =
{
   DFSDGGEOWIDGETS,                             // number of widgets
   DFSC_GEODLG,                                 // help, widget overrules
   WID_GEODLG,                                  // base window ID
   dfsGeoWinProc,                               // widget window procedure
   NULL,                                        // persistent position TXRECT
   dfsDgGeoWidgets                              // array of widgets
};


/*************************************************************************************************/
// Present set GEOmetry options dialog and execute resulting command(s) until done
/*************************************************************************************************/
void dfsGeoDialog
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                command;
   ULONG               geoC = 0;                // 0 = calculate
   ULONG               geoH = 0;                // default for sys RESET
   ULONG               geoS = 0;
   TXTS                geosC;
   TXTS                geosH;
   TXTS                geosS;

   ENTER();

   dfsBEGINWORK();                              // signal work starting

   strcpy( fdescr, dfstStoreDesc1( DFSTORE) + 10);
   dfsDgGeoWidgets[DFSDGGEOD1].flags &= ~TXWI_DISABLED; // text field ON
   dfsDgGeoWidgets[DFSDGGEOSIZEOS].flags &= ~TXWI_DISABLED; // text field ON
   dfsDgGeoWidgets[DFSDGGEOLOGTXT].flags &= ~TXWI_DISABLED; // text field ON

   georReset  = FALSE;
   georCl255  = FALSE;
   georCl240  = FALSE;
   georMbCyl  = FALSE;
   geor1Tos2  = FALSE;
   geor2Tos2  = FALSE;
   georCustm  = TRUE;                           // always start with CUSTOM

   dfsGeoSetControls();                         // initial value other fields

   while ((rc = txwWidgetDialog( TXHWND_DESKTOP, TXHWND_DESKTOP,
           NULL, " Select pre-defined geometries, or set a custom one ",
           TXWD_MOVEABLE | TXWD_HCENTER | TXWD_VCENTER, DFSDGGEOCE, &dfsDgGeoDlg))
        != TXDID_CANCEL)
   {
      if (rc != DFSGEODLG_RECREATE)
      {
         geoC = dfsGetMcsNumber( geoeC, 0);     // allow hex values too
         geoH = dfsGetMcsNumber( geoeH, 0);
         geoS = dfsGetMcsNumber( geoeS, 0);

         if (geoC || geoH || geoS)
         {
            sprintf( geosC, "%lu", geoC);
            sprintf( geosH, "%lu", geoH);
            sprintf( geosS, "%lu", geoS);
            sprintf( command, "geo %s %s %s", (geoC) ? geosC : "?",
                                              (geoH) ? geosH : ".",
                                              (geoS) ? geosS : ".");
            dfsExecEnd( command);

            georReset  = FALSE;
            georCl255  = FALSE;
            georCl240  = FALSE;
            georMbCyl  = FALSE;
            geor1Tos2  = FALSE;
            geor2Tos2  = FALSE;
            georCustm  = TRUE;                  // restart with CUSTOM after set
         }
      }
      dfsGeoSetControls();                      // update other fields and visibility
   }
   dfsENDWORK();                                // signal work done
   VRETURN();
}                                               // end 'dfsGeoDialog'
/*-----------------------------------------------------------------------------------------------*/


/*****************************************************************************/
// Dialog window procedure, for the GEOMETRY dialog, set dependant fields
/*****************************************************************************/
static ULONG dfsGeoWinProc                      // RET   result
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
      switch (msg)
      {
         case TXWM_COMMAND:
            switch (mp2)                        // command source
            {
               case TXCMDSRC_RADIOBUTTON:
                  switch (mp1)                  // Window-ID
                  {
                     case WID_GEODLG + DFSDGGEORESET:
                     case WID_GEODLG + DFSDGGEOCL255:
                     case WID_GEODLG + DFSDGGEOCL240:
                     case WID_GEODLG + DFSDGGEOMIBCYL:
                     case WID_GEODLG + DFSDGGEO1TBOS2:
                     case WID_GEODLG + DFSDGGEO2TBOS2:
                     case WID_GEODLG + DFSDGGEOCUSTOM:
                        //- dfsGeoSetControls();    // update other fields and visibility
                        //- txwInvalidateWindow( hwnd, FALSE, TRUE); // whole dlg
                        txwDismissDlg( hwnd, DFSGEODLG_RECREATE);
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
            break;

         //- to be refined, catch update to disk-selection list ?

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
}                                               // end 'dfsGeoWinProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Calculate GEO values to be displayed from RADIO settings and global GEO
/*****************************************************************************/
static void dfsGeoSetControls
(
   void
)
{
   DFSTOREINFO        *si = &(sti[DFSTORE]);
   BOOL                sgeoDifferent = FALSE;
   ULONG               geoC  = 0;                // 0 = calculate
   ULONG               geoH  = si->Sys.H;        // default for sys RESET
   ULONG               geoS  = si->Sys.S;
   ULONG               sSize = si->Sys.C * si->Sys.H * si->Sys.S;
   ULONG               lSize = si->Geo.C * si->Geo.H * si->Geo.S;

   ENTER();

   if ((si->Geo.C != si->Sys.C) ||
       (si->Geo.H != si->Sys.H) ||
       (si->Geo.S != si->Sys.S) ||              // forced geometry in effect
       (si->Geo.B != si->Sys.B)  )
   {
      sgeoDifferent = TRUE;
      strcpy(    geosX, "");
      dfstrSXiB( geosX, "", sSize, " for  system geometry :");
      sprintf(   geosC, "%8lu", si->Sys.C);
      sprintf(   geosH, "%3lu", si->Sys.H);
      sprintf(   geosS, "%3lu", si->Sys.S);
   }

   dfsdShowWg( dfsDgGeoWidgets[ DFSDGGEOSYSTXT  ], (sgeoDifferent == TRUE));
   dfsdShowWg( dfsDgGeoWidgets[ DFSDGGEOSYSCYL  ], (sgeoDifferent == TRUE));
   dfsdShowWg( dfsDgGeoWidgets[ DFSDGGEOSYSHEAD ], (sgeoDifferent == TRUE));
   dfsdShowWg( dfsDgGeoWidgets[ DFSDGGEOSYSSECT ], (sgeoDifferent == TRUE));
   dfsdShowWg( dfsDgGeoWidgets[ DFSDGGEORESET   ], (sgeoDifferent == TRUE));

   strcpy(    geolX, "");
   dfstrSXiB( geolX, "", lSize, " for logical geometry :");
   sprintf(   geolC, "%8lu", si->Geo.C);
   sprintf(   geolH, "%3lu", si->Geo.H);
   sprintf(   geolS, "%3lu", si->Geo.S);

   strcpy(    geooX, "");
   dfstrSXiB( geooX, "", si->Sectors,
                        (si->Sectors <= sSize)         ? " as reported by the OS" :
             (dfstStoreType( DFSTORE) == DFST_VIRTUAL) ? " raw virtual disk size" :
                          " as reported by the OS    (includes partial cylinder)");

   //- Set custom GEO fields based on active Radio button

   if      (georReset)   { geoC = si->Sys.C;                   }
   else if (georCl255)   { geoH = 255;       geoS =  63;       }
   else if (georCl240)   { geoH = 240;       geoS =  63;       }
   else if (georMbCyl)   { geoH =  64;       geoS =  32;       }
   else if (geor1Tos2)   { geoH = 255;       geoS = 127;       }
   else if (geor2Tos2)   { geoH = 255;       geoS = 255;       }
   else                  { geoH = si->Geo.H; geoS = si->Geo.S; }

   if (geoC == 0)                               // calculate
   {
      strcpy(  geoeC, "calculated");
   }
   else
   {
      sprintf( geoeC, "%8lu", geoC);
   }
   sprintf(    geoeH, "%3lu", geoH);
   sprintf(    geoeS, "%3lu", geoS);

   VRETURN ();
}                                               // end 'dfsGeoSetControls'
/*---------------------------------------------------------------------------*/


