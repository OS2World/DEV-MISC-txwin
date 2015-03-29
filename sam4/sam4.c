#define SAM_D "Popup and scrollbuffer sample for JvW-Fsys TXW OpenWatcom build environment."

#define SAM_C "(c) 2014: Jan van Wijk"

#define SAM_V "2.00 13-06-2014" // Minor update for TXLib 2.0
//efine SAM_V "1.01 23-09-2005" // Added scroll-buffer for better trace demo
//efine SAM_V "1.00 21-09-2005" // Initial version

#include <txlib.h>                              // TX library interface
#if   defined (WIN32)
   #define SAM_N "SAM4 winNT"
#elif defined (DOS32)
   #define SAM_N "SAM4 Dos32"
#elif defined (LINUX)
   #define SAM_N "SAM4 Linux"
#elif defined (DARWIN)
   #define SAM_N "SAM4 OS-X "
#else
   #define SAM_N "SAM4  OS/2"
#endif


#define SAM_MANDATORY_PARAMS     1              // # of mandatory params to EXE

char *usagetext[] =
{
   " mandatory-params   [optional-params]",
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


#define SAM_SCROLL_L   1000                     // lines in scroll-buffer
#define SAM_SCROLL_W    132                     // columns in scroll-buffer

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

static TXWHANDLE   desktop = 0;
static TXWHANDLE   sbufwin = 0;
static TXWINDOW    scrbuffwin;


// Standard window procedure, for any window-class
static ULONG samStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   ULONG               mp1,                     // IN    msg param 1
   ULONG               mp2                      // IN    msg param 2
);


int main (int argc, char *argv[]);

/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = NO_ERROR;           // function return

   TxINITmain( "SAMTRACE", "SAM", FALSE, TRUE, 0); // allow switches AFTER params

   if (TxaExeSwitch('l'))                       // start logfile now ?
   {
      TxAppendToLogFile( TxaExeSwitchStr( 'l', NULL, "sam"), FALSE);
   }
   if ((TxaExeSwitch('?')) ||                   // switch help requested
       (TxaExeArgc() <= SAM_MANDATORY_PARAMS))  // or not enough params
   {
      TxPrint( "\n%s %s %s\n%s\n\nUsage: %s",
                  SAM_N, SAM_V, SAM_C, SAM_D, TxaExeArgv(0));
      TxShowTxt( usagetext);
   }
   else
   {
      if ((desktop = txwInitializeDesktop( NULL, NULL)) != 0)
      {
         TXWQMSG          qmsg;                 // one message
         TXRECT           dtsize;               // desktop client size

         txwQueryWindowRect( desktop, FALSE, &dtsize); // get client area
         scrollBufData.length = SAM_SCROLL_L;
         scrollBufData.width  = max( SAM_SCROLL_W, TxScreenCols());
         scrollBufData.vsize  = dtsize.bottom -1; // sbstatus line

         if ((rc = txwInitPrintfSBHook(&scrollBufData)) == NO_ERROR)
         {
            txwSetupWindowData(
               0, 0 , dtsize.bottom, dtsize.right,
               TXWS_STDWINDOW   | TXWS_MOVEABLE    |
               TXWS_HCHILD_SIZE | TXWS_VCHILD_SIZE |
               TXWS_SIDEBORDERS | TXWS_TITLEBORDER,
               0, ' ', ' ', TXWSCHEME_COLORS, " text output window " , "",
               &scrbuffwin);
            scrbuffwin.sb.topline = scrollBufData.firstline;
            scrbuffwin.sb.leftcol = 0;
            scrbuffwin.sb.sbdata  = &scrollBufData;
            scrbuffwin.sb.scolor  = cSchemeColor;
            scrbuffwin.sb.altcol  = TXSB_COLOR_B2BLUE | TXSB_COLOR_BRIGHT;
            sbufwin = txwCreateWindow( desktop, TXW_SBVIEW, desktop, 0,
                                       &scrbuffwin, samStdWindowProc);
            scrollBufData.view = sbufwin;       // register view for update
            txwInvalidateWindow( sbufwin, TRUE, TRUE); // and have it painted

            while (txwGetMsg(  &qmsg))
            {
               txwDispatchMsg( &qmsg);
            }
            txwTermPrintfSBHook();
         }
         txwTerminateDesktop();
      }
      else
      {
         TxPrint("Failed to intialize desktop\n");
      }
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
      TRCLAS( "SAM std - ", hwnd);
      win = txwWindowData( hwnd);
      switch (msg)
      {
         case TXWM_CREATE:
            txwPostMsg( hwnd, TXWM_USER, 0, 0); // start the action ...
            break;

         case TXWM_USER:
            sprintf( text, "Hello from SAMPLE-4\n\n"
                           "  param-1 = '%s'\n  param-2 = '%s'\n  param-3 = '%s'\n\n"
                           "Want to quit this game ? [Y/N] ",
                            TxaExeArgv(1),    TxaExeArgv(2),    TxaExeArgv(3));

            if (TxConfirm( 0, text))            // confirmation popup
            {
               txwPostMsg( hwnd, TXWM_CLOSE, 0, 0); // quit the application
            }
            else
            {
               TxPrint( "\n\nOK, there we go again ...\n");
               TxSleep( 500);
               txwPostMsg( hwnd, TXWM_USER, 0, 0); // restart the action ...
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
