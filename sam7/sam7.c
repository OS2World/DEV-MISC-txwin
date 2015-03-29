#define SAM_D "Text-viewer sample for JvW-Fsys TXW OpenWatcom build environment."

#define SAM_C "(c) 2014: Jan van Wijk"

#define SAM_V "2.00 13-06-2014" // Minor update for TXLib 2.0
//efine SAM_V "1.00 26-09-2005" // Initial version

#include <txlib.h>                              // TX library interface

#if   defined (WIN32)
   #define SAM_N "SAM7 winNT"
#elif defined (DOS32)
   #define SAM_N "SAM7 Dos32"
#elif defined (LINUX)
   #define SAM_N "SAM7 Linux"
#elif defined (DARWIN)
   #define SAM_N "SAM5 OS-X "
#else
   #define SAM_N "SAM7  OS/2"
#endif



char *switchhelp[] =
{
   "",
   "  Switch character for EXE switches is '-' or '/'. All single letter",
   "  switches are case-sensitive, long switchnames like 'query' are not.",
   "",
#if defined (DUMP)
   " -123[t][s][l][r][dxx] = trace level 123, [t]stamp; [s]creen; No [l]ogging;",
   "                                          [r]e-open [d]elay xx ms per line",
#endif
   "",
   " -?            = help on executable commandline switches (this text)",
   " -7            = Use 7-bit ASCII only (no 'graphic' characters)",
   " -a            = switch off usage of ANSI escape characters for color",
   " -l:logfile    = start logging immediately to 'logfile.log'",
   "",
   NULL
};


char *sam_about[] =
{
   "", "", "", "", "", "", "",
   "",
   "             Simple text-viewer using the TxWindows text-mode UI library",
   "",
   "",
   "              °°°°°°°°°      °°°°°°°°°       °°       °°        °°°°°°°°°",
   "             °°             °°°               °°     °°        °°°       ",
   "             °°             °°                °°    °°         °°        ",
   "             °°             °°                °°   °°          °°        ",
   "            °°°°°°°°°       °°°°°°°°           °° °°           °°°°°°°°  ",
   "            °°                     °°           °°                    °° ",
   "            °°                     °°           °°                    °° ",
   "           °°                     °°°          °°                    °°° ",
   "           °°              °°°°°°°°°           °°             °°°°°°°°°  ",
   "",
   "",
   NULL
};

#define SAM7H_MAIN       300
#define SAM7H_VIEW       320

#define SAM7M_MENUS     3000                    // menu help base
#define SAM7M_MENUBAR   3010

#define SAM7M_FILE      3100                     // File menu
#define SAM7M_OPENDLG   3110
#define SAM7M_EXIT      3199

#define SAM7M_HELP      3990                     // Help menu
#define SAM7M_SW_HELP   3994
#define SAM7M_UIHELP    3995
#define SAM7M_ABOUT     3996

//- Note these items are shared for several menus
TXSitem(mmsp,0            ,0,TXSF_DISABLED | TXSF_SEPARATOR,     0,""    ,"");

// File menu
TXSitem(mm1o,SAM7M_OPENDLG,0,0     , 1,"Open a file to view   c-O","Open a file to be viewed, by selecting it in a file-open dialog");
TXSitem(mm1x,SAM7M_EXIT   ,0,0     , 2,"Exit                  c-Q","Exit the SAM7 program                                          ");
static TXS_ITEM *mm1[] = {&mm1o, &mmsp, &mm1x};
TXSlist(sam7l1, 3, 3, mm1);

// Help menu
TXSitem(mm94,SAM7M_SW_HELP,0,0     , 1,"Exe startup switches"     ,"Switches that can be specified at startup of the executable    ");
TXSitem(mm95,SAM7M_UIHELP ,0,0     , 1,"User interface"           ,"Help on the SAM7 application and the TxWin user interface      ");
TXSitem(mm96,SAM7M_ABOUT  ,0,0     , 2,"About ...          s-F11" ,"Show SAM7 program version details and copyright notices        ");
static TXS_ITEM *mm9[] = {&mm94, &mm95, &mmsp, &mm96};
TXSlist(sam7l9, 4, 4, mm9);


TXSmenu(sam7m1,&sam7l1,SAM7M_MENUBAR,0, 1,'f'," File ",
        "Menu: [File operations and application exit  ]");


TXSmenu(sam7m9,&sam7l9,SAM7M_MENUBAR,0, 1,'h'," Help ",
        "Menu: [Help on menu and commands plus 'about']");


static TXS_MENUBAR mainmenu =
{
   2,                                           // max number of menus present
   0,                                           // index of 1st default menu
   {&sam7m1, &sam7m9}                           // menu pointers
};


static  char       *mainwinhelp[] =             // Actual help text
{
   #include <sam7main.hlp>
   NULL
};

static  char       *menusyshelp[] =
{
   #include <sam7menu.hlp>
   NULL
};



static char        sam_footer[]    = " F1=Help F3=Quit F8=OpenFile F10=Menu ";
static TXLN        sam_title       = {0};
static TXLN        sam_filename    = {0};
static TXWHANDLE   view;                    // the text-view window
static char      **viewtext = NULL;         // attached text, or NULL

// SAM7 application messages
#define SAM_FILEDIALOG    TXWM_USER             // prompt for new file
#define SAM_ATTACHFILE    TXWM_USER +1          // attach named file to window


// SAMPLE standard window procedure, for any window-class
static ULONG samStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);

// Present the about 'SAM7' info dialog
static void samAboutDialog
(
   void
);

// read specified file, and attach to global anchor; update title
static BOOL samLoadNewFile
(
   char               *name                     // IN    name of the file to attach
);

// SAMView read-file procedure
static char **samvReadText                      // RET   ptr to text-array
(
   FILE               *file,                    // IN    file opened for read
   ULONG              *size                     // OUT   size in lines
);

// SAMView free memory for view-text
static void  samvDiscardText
(
   char              **text                     // IN    ptr to text-array
);


int main (int argc, char *argv[]);

/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = NO_ERROR;           // function return
   TXWQMSG             qmsg;
   TXWINDOW            setup;                   // setup data
   TXWINDOW           *win;

   TxINITmain( "SAMTRACE", "SAM", FALSE, TRUE, 0); // allow switches AFTER params

   if (TxaExeSwitch('l'))                       // start logfile now ?
   {
      TxAppendToLogFile( TxaExeSwitchStr( 'l', NULL, "sam"), FALSE);
   }
   if (TxaExeSwitch('?'))                       // switch help requested
   {
      TxPrint( "\n%s %s %s\n%s\n\nUsage: %s   [filename]",
                  SAM_N, SAM_V, SAM_C, SAM_D, TxaExeArgv(0));
      TxShowTxt( switchhelp);
   }
   else
   {
      txwRegisterHelpText( 0,           "main", "SAM7 main help items", mainwinhelp);
      txwRegisterHelpText( SAM7M_MENUS, "menu", "SAM7 main menu help",  menusyshelp);

      txwSetupWindowData(
         0, 0 , TxScreenRows(), TxScreenCols(),
         TXWS_STDWINDOW | TXWS_FOURBORDERS | TXWS_MOVEABLE,
         SAM7H_MAIN, ' ', ' ', TXWSCHEME_COLORS,
         SAM_N " " SAM_V "; " SAM_C, "",
         &setup);

      if (txwInitializeDesktop( &setup, NULL) != 0)
      {
         txwSetupWindowData(
            1, 1, TxScreenRows() -2, TxScreenCols() -2,
            TXWS_FRAMED | TXWS_SAVEBITS | TXWS_MOVEABLE | TXCS_CLOSE_BUTTON,
            SAM7H_VIEW, ' ', ' ',
            cDskFooterStand, cSbvBorder_top,    // matches SB colors
            cSbvTitleStand,  cSbvTitleFocus,
            cSbvFooterStand, cSbvFooterFocus,
            sam_title, sam_footer,
            &setup);

         view = txwCreateWindow( 0, TXW_TEXTVIEW, 0, 0, &setup, samStdWindowProc);

         win = txwWindowData( view);
         win->tv.topline = 0;                   // set to start of text
         win->tv.leftcol = 0;
         win->tv.maxtop  = TXW_INVALID;
         win->tv.maxcol  = TXW_INVALID;
         win->tv.buf     = sam_about;           // start with artwork background

         txwSetAccelerator( view, TXc_O,    SAM7M_OPENDLG);
         txwSetAccelerator( view, TXk_F8,   SAM7M_OPENDLG);
         txwSetAccelerator( view, TXk_MENU, SAM7M_MENUBAR);
         txwSetAccelerator( view, TXk_F10,  SAM7M_MENUBAR);
         txwSetAccelerator( view, TXs_F11,  SAM7M_ABOUT);
         txwSetAccelerator( view, TXc_Q,    SAM7M_EXIT);

         txwShowWindow( view, TRUE);            // make sure it is visible
         txwSetFocus(   view);                  // and gets input focus

         if (TxaExeArgc() > 1)                  // filename specified
         {
            strcpy( sam_filename, TxaExeArgv(1));
            txwPostMsg( view, SAM_ATTACHFILE, 0, 0);
         }
         else                                   // start with the main-menu
         {
            txwPostMsg( view, TXWM_COMMAND, SAM7M_MENUBAR, 0);
         }

         while (txwGetMsg(  &qmsg))
         {
            txwDispatchMsg( &qmsg);
         }
         txwTerminateDesktop();
      }
      else
      {
         TxPrint("Failed to intialize desktop\n");
      }
      samvDiscardText( viewtext);
   }
   TxEXITmain(rc);                              // TX Exit code, incl tracing
}                                               // end 'main'
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
   ULONG               rc   = NO_ERROR;
   TXWINDOW           *win;
   TXLN                text;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      TRCLAS( "SAM std ",  hwnd);
      win = txwWindowData( hwnd);
      switch (msg)
      {
         case TXWM_COMMAND:                     // from menu or accelerator
            switch (mp1)
            {
               case SAM7M_MENUBAR:              // activate the menu
                  txwMenuBar( TXHWND_DESKTOP, hwnd, NULL,
                                  0, SAM7M_MENUBAR,
                                  TXMN_MAIN_MENU, &mainmenu);
                  break;

               case SAM7M_OPENDLG:              // open a (new) file
                  txwPostMsg( view, SAM_FILEDIALOG, 0, 0);
                  break;

               case SAM7M_SW_HELP:
                  txwViewText( TXHWND_DESKTOP, 0, 0,
                              "SAM7 executable switches summary",
                               switchhelp);
                  break;

               case SAM7M_UIHELP:
                  txwSendMsg( hwnd, TXWM_HELP, 0, 0);
                  break;

               case SAM7M_EXIT:
                  txwPostMsg( hwnd, TXWM_CLOSE, 0, 0);
                  break;

               case SAM7M_ABOUT:
                  samAboutDialog();
                  break;

               default:
                  TxMessage( TRUE, 0, "Command item not implemented: %lu", mp1);
                  break;
            }
            break;

         case SAM_FILEDIALOG:
            strcpy( text, "*.?;*.log;*.txt");     // default extensions
            if (txwOpenFileDialog( text, NULL, NULL, 0, NULL,
                " Select a (text) file for viewing ", text))
            {
               strcpy( sam_filename, text);
               txwPostMsg( view, SAM_ATTACHFILE, 0, 0);
            }
            break;

         case SAM_ATTACHFILE:
            if (samLoadNewFile( sam_filename))
            {
               win->tv.buf = viewtext;          // refresh text in view window
               txwInvalidateWindow( hwnd, TRUE, FALSE); // and have it painted
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
}                                               // end 'samStdWindowProc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Present the about 'SAM7' info dialog
/*****************************************************************************/
static void samAboutDialog
(
   void
)
{
   TX1K          about;                         // about text
   TXLN          text;                          // one line of text

   ENTER();

   sprintf( about, "              Fsys Software - http: //www.dfsee.com\n\n"
                   "   %s : %s %s\n", SAM_N, SAM_V, SAM_C);
   sprintf( text,  " UI TxWindows : %s\n", txVersionString());
   strcat( about, text);

   sprintf( text, " 'C' compiler : OpenWatcom %4.2lf"
                  " (c) 1988-2005: Sybase and OpenWatcom\n",
                  ((double) ( __WATCOMC__ - 1100)) / 100);
   strcat( about, text);

   sprintf( text, " EXE compress : ");
   strcat( about, text);
   #if defined (DEV32)
      sprintf( text,    "lxLite     1.33 (c) 1996-2003: Max Alekseyev\n");
   #else
      sprintf( text,    "UPX        1.25 (c) 1996-2005: Markus Oberhumer\n");
   #endif
   strcat( about, text);
   #if defined (DOS32)
      sprintf( text,  " DOS extender : %s\n", txDosExtVersion());
      strcat(  about, text);
   #endif

   sprintf( text,  " OS   version : ");
   strcat( about, text);
   (void) TxOsVersion( text);                     // Get operating system version
   strcat( about, text);
   strcat( about, "\n");

   #if defined (DOS32)
      sprintf( text,  " DPMI version : %s\n", txDosExtDpmiInfo());
      strcat(  about, text);
   #endif
   strcat( about, "\n");

   TxMessage( TRUE, SAM7M_ABOUT, about);

   VRETURN ();
}                                               // end 'samAboutDialog'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// read specified file, and attach to global anchor; update title
/*****************************************************************************/
static BOOL samLoadNewFile
(
   char               *name                     // IN    name of the file to attach
)
{
   BOOL                rc = FALSE;              // function return
   FILE               *vf;                      // file pointer
   ULONG               nr;                      // nr of lines in file

   ENTER();

   if ((vf = fopen( name, "r")) != NULL)
   {
      samvDiscardText( viewtext);               // Discard old text, if any
      if ((viewtext = samvReadText( vf, &nr)) != NULL)
      {
         sprintf( sam_title, "%s - %lu lines", name, nr); // updated title
         rc = TRUE;
      }
      else
      {
         TxMessage( TRUE, 0, "Error reading file: %s", name);
         strcpy( sam_title, " --- no file loaded --- ");
         viewtext = sam_about;
      }
      fclose( vf);
   }
   else
   {
      TxMessage( TRUE, 0, "File '%s' not found", name);
   }
   BRETURN (rc);
}                                               // end 'samLoadNewFile'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// SAMView read-file procedure
/*****************************************************************************/
static char **samvReadText                      // RET   ptr to text-array
(
   FILE               *file,                    // IN    file opened for read
   ULONG              *size                     // OUT   size in lines
)
{
   char              **data  = NULL;
   char               *line  = NULL;
   char               *new   = NULL;
   ULONG               lines = 0;
   ULONG               nr;

   ENTER();

   if ((line = TxAlloc( 1, TXMAX4K)) != NULL)
   {
      fseek( file, 0, SEEK_SET);
      while (fgets( line, TXMAX4K, file) != NULL)
      {
         lines++;
      }
      TRACES(( "Opened file has %lu lines ...\n", lines));
      if ((data = TxAlloc( lines +1, sizeof(char *))) != NULL)
      {
         fseek( file, 0, SEEK_SET);
         for (nr = 0; nr < lines; nr++)
         {
            if (fgets( line, TXMAX4K, file) != NULL)
            {
               if ((new = TxAlloc( 1, strlen(line)+1)) != NULL)
               {
                  if (line[strlen(line)-1]=='\n')
                  {
                    line[strlen(line)-1]='\0';
                  }
                  strcpy( new, line);
                  data[ nr] = new;
               }
               else                             // quit reading on memory error
               {
                  break;
               }
            }
            else                                // unexpected read failure
            {
               break;
            }
         }
         *size = lines;
      }
      TxFreeMem( line);
   }
   RETURN( data);
}                                               // end 'samvReadText'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// SAMView free memory for view-text
/*****************************************************************************/
static void  samvDiscardText
(
   char              **text                     // IN    ptr to text-array
)
{
   char              **line  = NULL;

   ENTER();

   if ((text != NULL) && (text != sam_about))   // only free dynamic text
   {
      for (line = text; *line != NULL; line++)
      {
         TxFreeMem( *line);
      }
      TxFreeMem( text);
   }
   VRETURN();
}                                               // end 'samvDiscardText'
/*---------------------------------------------------------------------------*/

