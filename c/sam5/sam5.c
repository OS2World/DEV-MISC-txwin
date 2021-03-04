#define SAM_D "Text-viewer sample for TXwin build environment."

#define SAM_C "(c) 2005-2018: Jan van Wijk"

#define SAM_V "3.01 06-10-2018" // Minor updates for TXwin 5.x
//efine SAM_V "3.00 03-08-2017" // Added trivial HELP to footer
//efine SAM_V "2.00 13-06-2014" // Minor update for TXLib 2.0
//efine SAM_V "1.00 23-09-2005" // Initial version

#include <txlib.h>                              // TX library interface
#if   defined (WIN32)
   #define SAM_N "SAM5 Win32"
#elif defined (DOS32)
   #define SAM_N "SAM5 Dos32"
#elif defined (LINUX)
   #define SAM_N "SAM5 Linux"
#elif defined (DARWIN)
   #define SAM_N "SAM5 macOS"
#else
   #define SAM_N "SAM5  OS/2"
#endif


#define SAM_MANDATORY_PARAMS     1              // # of mandatory params to EXE

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
   NULL
};


// SAMView read-file procedure
static char **samvReadText                      // RET   ptr to text-array
(
   FILE               *file                     // IN    file opened for read
);

int main (int argc, char *argv[]);

/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = NO_ERROR;           // function return
   FILE               *vf;
   char              **viewtext;
   TXWQMSG             qmsg;
   TXWHANDLE           view;
   TXWINDOW            setup;                   // setup data
   TXWINDOW           *win;

   TxINITmain( "SAMTRACE", "SAM", FALSE, TRUE, 0); // allow switches AFTER params

   if (TxaExeSwitch('l'))                       // start logfile now ?
   {
      TxAppendToLogFile( TxaExeSwitchStr( 'l', NULL, "sam"), FALSE);
   }
   if ((TxaExeSwitch('?')) ||                   // switch help requested
       (TxaExeArgc() <= SAM_MANDATORY_PARAMS))  // or not enough params
   {
      TxPrint( "\n%s %s %s\n%s\n\nUsage: %s", SAM_N, SAM_V, SAM_C, SAM_D, TxaExeArgv(0));
      TxShowTxt( usagetext);
   }
   else
   {
      if ((vf = fopen( TxaExeArgv(1), "r")) != NULL)
      {
         if ((viewtext = samvReadText( vf)) != NULL)
         {
            if (txwInitializeDesktop( NULL, NULL) != 0)
            {
               txwRegisterHelpText( SAM_HELP, "sam5view", "Sample-5 file-viewer help", viewerHelpText);

               txwSetupWindowData(
                  0, 0,                         // upper left corner
                  TxScreenRows(),               // vertical size   full-screen
                  TxScreenCols(),               // horizontal size full-screen
                  TXWS_FRAMED        |          // borders (scroll indicator)
                  TXWS_SAVEBITS      |          // save underlying screen
                  TXWS_MOVEABLE      |          // allow window movement
                  TXCS_CLOSE_BUTTON,            // include a close button [X]

                  SAM_HELP,                     // minimal help for now
                  ' ', ' ', TXWSCHEME_COLORS,   // default colors
                  TxaExeArgv(1),                // title, filename
                  SAM_N " " SAM_V "; " SAM_C    // footer, program name/version
                  ";  F1=Help F3=quit) ",       // and minimal help :)
                  &setup);                      // resulting window data structure

               view = txwCreateWindow(
                         0,                     // parent window
                         TXW_TEXTVIEW,          // class of this window
                         0,                     // owner window
                         0,                     // insert after ...
                         &setup,                // window setup data
                         txwDefWindowProc);

               win = txwWindowData( view);
               win->tv.topline = 0;             // set to start of text
               win->tv.leftcol = 0;
               win->tv.maxtop  = TXW_INVALID;
               win->tv.maxcol  = TXW_INVALID;
               win->tv.buf     = viewtext;      // attach the file data

               txwShowWindow( view, TRUE);      // make sure it is visible
               txwSetFocus(   view);            // and gets input focus

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
         else
         {
            TxPrint("\nError reading file: %s\n", TxaExeArgv(1));
         }
         fclose( vf);
      }
      else
      {
         TxPrint("\nFile '%s' not found\n", TxaExeArgv(1));
      }
   }
   TxEXITmain(rc);                              // TX Exit code, incl tracing
}                                               // end 'main'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// SAMView read-file procedure
/*****************************************************************************/
static char **samvReadText                      // RET   ptr to text-array
(
   FILE               *file                     // IN    file opened for read
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
      }
      TxFreeMem( line);
   }
   RETURN( data);
}                                               // end 'samvReadText'
/*---------------------------------------------------------------------------*/

