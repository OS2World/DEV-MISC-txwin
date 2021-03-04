#define SAM_D "Text-viewer sample with File-Open-Dialog for TXwin build environment."

#define SAM_C "(c) 2005-2018: Jan van Wijk"

#define SAM_V "3.01 06-10-2018" // Minor updates for TXwin 5.x
//efine SAM_V "2.00 13-06-2014" // Minor update for TXLib 2.0
//efine SAM_V "1.00 26-09-2005" // Initial version

#include <txlib.h>                              // TX library interface
#if   defined (WIN32)
   #define SAM_N "SAM6 Win32"
#elif defined (DOS32)
   #define SAM_N "SAM6 Dos32"
#elif defined (LINUX)
   #define SAM_N "SAM6 Linux"
#elif defined (DARWIN)
   #define SAM_N "SAM6 macOS"
#else
   #define SAM_N "SAM6  OS/2"
#endif


char *usagetext[] =
{
   "  filename",
   "",
   "  filename = name of file to be viewed, with optional drive/path",
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


#define SAM_HELP      1000
static char           *viewerHelpText[] =
{
   "#000 Sample text viewer HELP",
   "",
   " This program uses the standard TXwin 'textview' control to view",
   " an ASCII text file specified as a parameter.",
   "",
   " You can use the keyboard and mouse to navigate/scroll through the text",
   " and use mouse-dragging to mark part of the text, then copy it to the",
   " clipboard using Ctrl-C or Alt-C",
   "",
   "",
   " New files can be loaded (discarding the current one) using either the",
   " <F8> or the <Ctrl>+o key, which will bring up the standard File-Open",
   " dialog that comes with TXwin",
   "",
   NULL
};


static char        sam_footer[]    = " F1=Help F3=Quit F8=OpenFile ";
static TXLN        sam_title       = {0};
static TXLN        sam_filename    = {0};
static TXWHANDLE   view;                    // the text-view window
static char      **viewtext = NULL;         // attached text, or NULL

// SAM6 application messages
#define SAM_FILEDIALOG    TXWM_USER             // prompt for new file
#define SAM_ATTACHFILE    TXWM_USER +1          // attach named file to window


// SAMPLE standard window procedure, for any window-class
static ULONG samStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   TXWMPARAM           mp1,                     // IN    msg param 1
   TXWMPARAM           mp2                      // IN    msg param 2
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
      TxPrint( "\n%s %s %s\n%s\n\nUsage: %s",
                  SAM_N, SAM_V, SAM_C, SAM_D, TxaExeArgv(0));
      TxShowTxt( usagetext);
   }
   else
   {
      if (txwInitializeDesktop( NULL, NULL) != 0)
      {
         txwRegisterHelpText( SAM_HELP, "sam6view", "Sample-6 file-viewer help", viewerHelpText);

         strcpy( sam_title, SAM_N " " SAM_V "; " SAM_C); // initial title
         txwSetupWindowData(
            0, 0,                               // upper left corner
            TxScreenRows(),                     // vertical size   full-screen
            TxScreenCols(),                     // horizontal size full-screen
            TXWS_FRAMED        |                // borders (scroll indicator)
            TXWS_SAVEBITS      |                // save underlying screen
            TXWS_MOVEABLE      |                // allow window movement
            TXCS_CLOSE_BUTTON,                  // include a close button [X]
            SAM_HELP,                           // minimal help for now
            ' ', ' ', TXWSCHEME_COLORS,         // default colors
            sam_title,                          // variable title line
            sam_footer,                         // footer, function-key help
            &setup);                            // resulting window data structure

         view = txwCreateWindow(
                   0,                           // parent window
                   TXW_TEXTVIEW,                // class of this window
                   0,                           // owner window
                   0,                           // insert after ...
                   &setup,                      // window setup data
                   samStdWindowProc);

         win = txwWindowData( view);
         win->tv.topline = 0;                   // set to start of text
         win->tv.leftcol = 0;
         win->tv.maxtop  = TXW_INVALID;
         win->tv.maxcol  = TXW_INVALID;
         win->tv.buf     = NULL;                // no text to start with

         txwShowWindow( view, TRUE);            // make sure it is visible
         txwSetFocus(   view);                  // and gets input focus

         if (TxaExeArgc() > 1)                  // filename specified
         {
            strcpy( sam_filename, TxaExeArgv(1));
            txwPostMsg( view, SAM_ATTACHFILE, 0, 0);
         }
         else                                   // start with file-open dialog
         {
            txwPostMsg( view, SAM_FILEDIALOG, 0, 0);
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
      txFreeText( viewtext);
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
   TXWMPARAM           mp1,                     // IN    msg param 1
   TXWMPARAM           mp2                      // IN    msg param 2
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
         case TXWM_CHAR:
            switch ((ULONG) mp2)
            {
               case TXc_O:
               case TXk_F8:                     // open new file
                  txwPostMsg( hwnd, SAM_FILEDIALOG, 0, 0);
                  break;

               case TXk_ESCAPE:
                  txwPostMsg( hwnd, TXWM_CLOSE, 0, 0);
                  break;

               default:
                  rc = txwDefWindowProc( hwnd, msg, mp1, mp2);
                  break;
            }
            break;

         case SAM_FILEDIALOG:
            strcpy( text, "*.?;*.log;*.txt");     // default extensions
            if (txwOpenFileDialog( text, NULL, NULL, 0, NULL, NULL,
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
      txFreeText( viewtext);
      if ((viewtext = samvReadText( vf, &nr)) != NULL)
      {
         sprintf( sam_title, "%s - %lu lines", name, nr); // updated title
         rc = TRUE;
      }
      else
      {
         TxMessage( TRUE, 0, "\nError reading file: %s\n", name);
      }
      fclose( vf);
   }
   else
   {
      TxMessage( TRUE, 0, "\nFile '%s' not found\n", name);
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

