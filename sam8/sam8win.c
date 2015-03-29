// SAM8 windowed user interface
//
// Author: J. van Wijk
//
// 1.00 14-02-2008    Initial version, derived from txtwin.c

#include <txlib.h>                              // TX library interface

#include <sam8.h>                               // command executor
#include <sam8win.h>                            // windowed entry point

#define SAM_SCROLL_L   9999                     // total  5 Mb
#define SAM_SCROLL_W    250

// minimum scroll-buffer for memory constrained environments (mainly DOS)
#define SAM_SMALLB_L   5000                     // total  1 Mb
#define SAM_SMALLB_W    100

#define SAM_WID_ENTRY      201                  // window id entry-field
#define SAM_WID_SCROLL     202                  // window id scroll-buffer

#define SAM_H_APPLIC       100                  // help ID SAMPLE application
#define SAM_H_INTERF       101                  // help ID SAMPLE interface


#define SAM_HIST_SIZE       96
#define SAM_HIST_LINE  TXMAXLN

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
   "#100 SAMPLE application and desktop window",
   "",
   " Note: To get help on the TxWindows userinterface, press <F1> again",
   "",
   " This is the built in help text for the SAMPLE application.",
   "",
   " It lists (function) key assignments specific to SAMPLE, the generic",
   " assignments are listed with the TxWindows help information (F1).",
   "",
   "       F3         Quit TXTest completely",
   "       F4         Save current screen-buffer to a file (samest.log)",
   "       F5         Start generic test dialog",
   "       F6         Show a simple message-box",
   "       F7         Present an input dialog",
   "       F8         Start test dialog for a selection list",
   "       F10        Activate the MenuBar with pulldown menus",
   "       F11        Execute command 'about' SAMPLE version information",
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
   "  information and examples see the SAMPLE documentation",
   "",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#101 Command entry field and output text window",
   "",
   " Note: To get more generic help on SAMPLE, press <F1>",
   "",
   " T H E   C O M M A N D   E N T R Y   F I E L D",
   " =============================================",
   "",
   " This is the (normally green) field at the bottom of the SAMPLE screen.",
   "",
   " Commands can be typed here, and will be processed by the SAMPLE",
   " application when <Enter> is pressed.",
   "",
   " The syntax of a (multiple) SAMPLE command is:",
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
   " This is the output window for all commands executed by SAMPLE.",
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
   " This section contains help for all SAMPLE confirmation requests",
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
   "#003 About the SAMPLE program",
   "",
   " This dialog informs you of the version of the SAMPLE program, and",
   " some of the other components and tools used in building it.",
   "",
   " All rights reserved by Fsys Software for SAMPLE and TXlib, and by",
   " the owners of the mentioned tools and components for the rest.",
   "",
   " Usage of 3rd party components and tools permitted by the licences",
   " as found with the distribution or on the related web-site.",
   "", "", "", "", "", "", "", "", "", "", "", "",
   "",
   "#004 SAMPLE version and operating-system mismatch",
   "",
   " This is to inform you that the SAMPLE version that is now active",
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



static  TXTM bordertxt;                         // versioned window title

static  char stattxt[] =
   "F1=help F3=quit F4=save    F10=menu   F12=collapse  Esc=abort";


//- Definition of static main-menu selection lists

TXSitem(mmsp,0            ,0,TXSF_DISABLED | TXSF_SEPARATOR,     0,""    ,"");

TXSitem(mm11,SAMC_OPEN    ,0,0             , 1,"Open logfile"              ,"Open a logfile for screen output");
TXSitem(mm12,SAMC_SAVE    ,0,0             , 1,"Save screen   F4"          ,"Save screenbuffer to a file");
TXSitem(mm13,SAMC_RUNS    ,0,0             , 1,"Run script"                ,"Run a SAMest script");
TXSitem(mm14,SAMC_EXIT    ,0,0             , 2,"Exit      Alt-F4"          ,"Exit the TXTest program");
static TXS_ITEM *mm1[] = {&mm11, &mm12, &mm13, &mmsp, &mm14};
TXSlist(tstmm1,5,5,mm1);


TXSitem(mm91,SAMC_CMDHELP ,0,0             , 1,"Application commands"      ,"One line descriptions for each SAM commands");
TXSitem(mm92,SAMC_SW_HELP ,0,TXSF_MARK_STAR, 1,"Exe startup switches"      ,"One line descriptions for each executable switch");
TXSitem(mm93,SAMC_UIHELP  ,0,0             , 1,"User interface"            ,"Help on the application and TxWin user interface");
TXSitem(mm94,SAMC_ABOUT   ,0,0             , 1,"About ...         F11"     ,"Show version details and copyright");
static TXS_ITEM *mm9[] =
{
   &mm91, &mm92, &mm93, &mmsp,
   &mm94
};
TXSlist(tstmm9,5,5,mm9);



TXSmenu(tmenu1,&tstmm1,SAMM_FILE   ,0    , 1,'f'," File "                ,"Menu with file items");
TXSmenu(tmenu9,&tstmm9,SAMM_HELP   ,0    , 1,'h'," Help "                ,"Menu with help items");
static TXS_MENUBAR mainmenu =
{
   2,                                           // number of menus presnt
   0,                                           // index of 1st default menu
   {&tmenu1, &tmenu9}                           // menu pointers
};




// Test window procedure, for any window-class
static ULONG samStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// SAMPLE window procedure, for entry-field, includes automatic value-echo
static ULONG samEntryWindowProc                 // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


/*****************************************************************************/
// Start and maintain SAMPLE interactive text-based windowed user-interface
/*****************************************************************************/
ULONG samWindowed
(
   char               *initial                  // IN    initial SAMPLE cmd
)
{
   ULONG               rc = NO_ERROR;
   ULONG               bm;                      // border mode
   BOOL                vborder;                 // vertical borders
   BOOL                hborder;                 // horizontal borders
   BOOL                borders   = FALSE;       // forced borders, NO
   BOOL                noborders = FALSE;       // forced no-borders, NO

   ENTER();

   //- to be refined, move to sam.c with help text later
   txwRegisterHelpText( SAMH_GENERIC, "Sample help",   "SAMPLE main help items",    mainwinhelp);
   txwRegisterHelpText( SAMH_CONFIRM, "Confirmations", "SAMPLE confirmation items", confirmhelp);
   txwRegisterHelpText( SAMH_MENUS,   "Menu system",   "SAMPLE menu system help",   menusyshelp);

   if (TxaExeSwitchValue('f') != NULL)          // frame switch specified
   {
      borders   = TxaExeSwitch('f');
      noborders = !borders;
   }
   TRACES(("borders:%lu  noborders:%lu\n", borders, noborders));

   vborder = (borders || ((TxScreenCols() > 84) && !noborders));
   hborder = (borders || ((TxScreenRows() > 28) && !noborders));

   sprintf( bordertxt, "%s: %s %s", SAM_N, SAM_V, SAM_C);
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
      bm, SAM_H_APPLIC,                         // style / helpid
      ' ', ' ',
      cSchemeColor,    cSchemeColor,
      cDskTitleStand,  cDskTitleFocus,
      cDskFooterStand, cDskFooterFocus,
      bordertxt, "",
      &desktopwin);

   if ((desktop = txwInitializeDesktop( &desktopwin, samStdWindowProc )) != 0)
   {
      TXRECT           dtsize;                  // desktop client size

      txwQueryWindowRect( desktop, FALSE, &dtsize); // get client area
      txwEnableWindow(    desktop, FALSE);      // no focus on desktop

      scrollBufData.length = SAM_SCROLL_L;
      scrollBufData.width  = max( SAM_SCROLL_W, TxScreenCols());
      scrollBufData.vsize  = dtsize.bottom -2;  // entryfield and sbstatus
      if (hborder)
      {
         scrollBufData.vsize--;                 // subtract SB title line
      }
      rc = txwInitPrintfSBHook(&scrollBufData);
      if (rc == TX_ALLOC_ERROR)                 // try smaller scroll-buffer
      {
         scrollBufData.length = SAM_SMALLB_L;
         scrollBufData.width  = SAM_SMALLB_W;
         rc = txwInitPrintfSBHook( &scrollBufData);
      }
      if (rc == NO_ERROR)
      {
         TXRECT   sbsize;                       // scrollbuf client size

         sama->sbsize   = scrollBufData.length;
         sama->sblwidth = scrollBufData.width;
         sama->sbbuf    = scrollBufData.buf;

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
            bm, SAM_H_INTERF,                   // style & help
            ' ', ' ', TXWSCHEME_COLORS,
            " text output window ", "",
            &scrbuffwin);
         scrbuffwin.sb.topline = scrollBufData.firstline;
         scrbuffwin.sb.leftcol = 0;
         scrbuffwin.sb.sbdata  = &scrollBufData;
         scrbuffwin.sb.scolor  = cSchemeColor;
         scrbuffwin.sb.altcol  = TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT;
         sbufwin = txwCreateWindow( desktop, TXW_SBVIEW, 0, 0,
                                    &scrbuffwin, samStdWindowProc);
         scrollBufData.view = sbufwin;          // register view for update
         txwQueryWindowRect( sbufwin, FALSE, &sbsize);
         sama->sbWidth  = sbsize.right +1;
         sama->sbLength = sbsize.bottom;
         TRACES(("sbW: %hu  sbL: %hu\n", sama->sbWidth, sama->sbLength));
         txwSetWindowUShort(  sbufwin, TXQWS_ID, SAM_WID_SCROLL);
         txwInvalidateWindow( sbufwin, TRUE, TRUE);

         sama->sbwindow = sbufwin;              // make handle available

         //- commandline entryfield
         bm = TXWS_CHILDWINDOW | TXWS_SIDEBORDERS | TXES_MAIN_CMDLINE;
         txwSetupWindowData(
            dtsize.bottom -1, 0,                // upper left corner
            1, dtsize.right,                    // vert + hor size
            bm, SAM_H_INTERF,                   // style & helpid
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
                                   &entryfwin, samEntryWindowProc);

         txwInitializeHistory( entryfwin.ef.history, SAM_HIST_SIZE, SAM_HIST_LINE);
         txwSetWindowUShort(   entrwin, TXQWS_ID, SAM_WID_ENTRY);
         txwSetFocus(          entrwin);
         txwInvalidateWindow(  entrwin, TRUE, TRUE);

         if (!TxaExeSwitchUnSet(TXA_O_MENU))    // automatic menu activation
         {
            char     *open   = TxaExeSwitchStr( TXA_O_MENU, NULL, "");

            sama->automenu   = TRUE;
            sama->menuOwner  = entrwin;
            if (*open != 0)
            {
               sama->menuopen   = (ULONG) tolower(open[0]);
            }
            else
            {
               sama->menuopen   = (ULONG) 'f';  // start with File pulldown
            }
         }
         sama->autodrop = ((TxaExeSwitchNum( 'M', NULL, 0) & 2) == 0);

         if (TxaExeSwitch('S'))                 // Shell mode
         {
            txwSetAccelerator( entrwin, TXa_F4,  SAMC_EXIT);
         }
         txwSetAccelerator( entrwin, TXk_F3,     SAMC_EXIT);
         txwSetAccelerator( entrwin, TXk_F4,     SAMC_SAVE);
         txwSetAccelerator( entrwin, TXk_F11,    SAMC_ABOUT);
         txwSetAccelerator( entrwin, TXk_MENU,   SAMM_DEFAULT);
         txwSetAccelerator( entrwin, TXk_F10,    SAMM_DEFAULT);
         txwSetAccelerator( entrwin, TXs_F10,    SAMM_DEFAULT);
         txwSetAccelerator( entrwin, TXa_F,      SAMM_FILE);
         txwSetAccelerator( entrwin, TXa_H,      SAMM_HELP);

         if ((samStartupLogo( initial) != SAM_QUIT) && !sama->autoquit)
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
         sama->retc = rc;
      }
      txwTerminateHelpManager();
      txwTerminateDesktop();
   }
   else
   {
      TxPrint("Failed to initialize desktop\n");
   }
   RETURN (sama->retc);
}                                               // end 'samWindowed'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// SAMPLE standard window procedure, for any window-class
/*****************************************************************************/
static ULONG samStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
)
{
   ULONG               rc = NO_ERROR;
   TXLN                s1,s2;
   ULONG               flag = 0;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      TRCLAS( "SAM std - ", hwnd);
      switch (msg)
      {
         case TXWM_COMMAND:
            switch (mp1)                        // unique command code
            {
               case SAMM_FILE:
               case SAMM_HELP:
               case SAMM_DEFAULT:
                  {
                     ULONG menuopen;

                     switch (mp1)
                     {
                        case SAMM_FILE: menuopen = 'f';  break;
                        case SAMM_HELP: menuopen = 'h';  break;
                        default:        menuopen = sama->menuopen;
                           sama->menuopen = 0;  // one time default !
                           break;
                     }
                     sama->automenu = TRUE;     // make menu sticky
                     sama->menuOwner = hwnd;

                     flag = TXMN_MAIN_MENU;     // signal main-menu will be up
                     if (mp1 == SAMM_AUTOMENU)  // automatic menu after command
                     {                          // completion, do not drop yet!
                        flag |= TXMN_DELAY_AUTODROP;
                        TRACES(("MenuBar - delayed autodrop\n"));
                     }
                     if (sama->autodrop == FALSE)
                     {
                        TRACES(("MenuBar - no autodrop\n"));
                        flag |= TXMN_NO_AUTODROP;
                     }

                     txwSendMsg( sama->sbwindow, TXWM_STATUS, 0, cSchemeColor);

                     if (txwMenuBar( TXHWND_DESKTOP, hwnd, NULL,
                                     menuopen, SAMM_BAR,
                                     flag, &mainmenu) == TXDID_CANCEL)
                     {
                        sama->menuOwner = 0;    // quit automatic menuBar
                     }
                  }
                  break;

               case SAMC_OPEN:
                  samBEGINWORK();               // signal work starting

                  strcpy( s1, "samlog");
                  if (txwSaveAsFileDialog( s1, NULL, NULL, 0, NULL,
                      " Specify file for logging (append) ", s1))
                  {
                     TxAppendToLogFile( s1, TRUE);
                  }
                  samENDWORK();                 // signal work done
                  break;

               case SAMC_SAVE:
                  samExecCmd( "scrfile samtest");
                  break;

               case SAMC_RUNS:
                  samBEGINWORK();               // signal work starting
                  strcpy( s1, "*.sam");
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
                           " Specify parameter(s) for the script ", SAMC_RUNS,
                           TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER,
                           50, s1) != TXDID_CANCEL)
                     {
                        strcat( s2, s1);
                     }
                     samExecCmd( s2);
                  }
                  samENDWORK();                 // signal work done
                  break;

               case SAMC_EXIT:
                  if (TxaExeSwitch('S'))
                  {
                     TxMessage( TRUE, 0,
                                "SAMPLE is running in SHELL mode,\n"
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

               case SAMC_CMDHELP:
                  samBEGINWORK();               // signal work starting
                  txwViewText( TXHWND_DESKTOP, 0, 0,
                              "SAMPLE application command summary",
                               cmdhelptxt);
                  samENDWORK();                 // signal work done
                  break;

               case SAMC_SW_HELP:
                  samBEGINWORK();               // signal work starting
                  txwViewText( TXHWND_DESKTOP, 0, 0,
                              "SAMPLE executable switches summary",
                               switchhelp);
                  samENDWORK();                 // signal work done
                  break;

               case SAMC_UIHELP:
                  txwPostMsg( entrwin, TXWM_HELP, 0, 0);
                  break;

               case SAMC_ABOUT:
                  samExecCmd( "about -r");
                  break;

               default:
                  samBEGINWORK();              // signal work starting
                  TxMessage( TRUE, 5001,
                             "Unknown menu command-code %lu,\nthis is a "
                             "program bug.\n\nPlease report this to your "
                             "support contact for this software", mp1);
                  samENDWORK();              // signal work done
                  break;
            }
            break;

         case TXWM_CHAR:
            if (txwIsAccelCandidate(mp2))       // menu will be closed, allow
            {                                   // automenu to restart it.
               samBEGINWORK();
               samENDWORK();                    // signal work done
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
}                                               // end 'samStdWindowProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// SAMPLE window procedure, for entry-field, includes automatic value-echo
/*****************************************************************************/
static ULONG samEntryWindowProc                 // RET   result
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
      TRCLAS( "SAM entry - ", hwnd);
      win = txwWindowData( hwnd);
      switch (msg)
      {
         case TXWM_CHAR:
            switch (mp2)
            {
               case TXk_ENTER:                  // execute as SAMPLE command
                  txwSendMsg( sama->sbwindow, TXWM_CHAR, 0, TXc_END);
               case TXc_ENTER:                  // execute, no auto-scroll
                  txwSendMsg( hwnd,  TXWM_CURSORVISIBLE, FALSE, 0);

                  TxPrint("\n");
                  TxCancelAbort();              // reset pending abort status
                  if (((dr = samMultiCommand( win->ef.buf, 0, TRUE, TRUE, FALSE))
                           == SAM_QUIT) || ( sama->autoquit))
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
                  txwSendMsg( sama->sbwindow, msg, mp1, key);
                  break;

               case TXa_LEFT:                   // avoid left/right movemement
               case TXa_RIGHT:                  // of the desktop (corruption)
                  break;

               default:
                  rc = samStdWindowProc( hwnd, msg, mp1, mp2);
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
            rc = samStdWindowProc( hwnd, msg, mp1, mp2);
            break;
      }
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN( rc);
}                                               // end 'samEntryWindowProc'
/*---------------------------------------------------------------------------*/

