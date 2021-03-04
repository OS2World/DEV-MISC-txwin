#define SAM_D "Expression evaluator sample for TXWin build environment."

#define SAM_C "(c) 2007-2018: Jan van Wijk"

#define SAM_V "1.01 06-10-2018"                 // Minor updates for TXwin 5.x
//efine SAM_V "1.00 11-10-2007"                 // Initial version

#include <txlib.h>                              // TX library interface
#if   defined (WIN32)
   #define SAM_N "SAM9 Win32"
#elif defined (DOS32)
   #define SAM_N "SAM9 Dos32"
#elif defined (LINUX)
   #define SAM_N "SAM9 Linux"
#elif defined (DARWIN)
   #define SAM_N "SAM9 macOS"
#else
   #define SAM_N "SAM9  OS/2"
#endif


#define SAM_MANDATORY_PARAMS     1              // # of mandatory params to EXE

char *usagetext[] =
{
   " expression",
   "",
   "  Evaluates strings like '$a = 77' and 'Var a is $a' or '{{3+5*7}}'",
   "",
   "  The actual expressions must either start with a variable ($var),",
   "  OR be enclosed in double brackets as in '{{expr}}'",
   "",
   "  All other text in the string is considered to be LITTERAL text.",
   "",
   "  Switch character for EXE switches is '-' or '/'. All single letter",
   "  switches are case-sensitive, long switchnames like 'query' are not.",
   "",
#if defined (DUMP)
   " -123[t][s][l][r][dxx] = trace level 123, [t]stamp; [s]creen; No [l]ogging;"
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

#define TXWM_PGM_INIT TXWM_USER
#define TXWM_PGM_LOOP TXWM_USER +1
#define TXWM_PGM_EXEC TXWM_USER +2

// Standard window procedure, for any window-class
static ULONG samStdWindowProc                   // RET   result
(
   TXWHANDLE           hwnd,                    // IN    current window
   ULONG               msg,                     // IN    message id
   TXWMPARAM           mp1,                     // IN    msg param 1
   TXWMPARAM           mp2                      // IN    msg param 2
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
      TxAppendToLogFile( TxaExeSwitchStr( 'l', NULL, "sam"), TRUE);
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

            txwPostMsg( sbufwin, TXWM_PGM_INIT, 0, 0); // start the action ...

            txsInitVariablePool( NULL);         // var and expr resolving
            while (txwGetMsg(  &qmsg))
            {
               txwDispatchMsg( &qmsg);
            }
            txsTermVariablePool();              // Free any txsvar resources
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
   TXWMPARAM           mp1,                     // IN    msg param 1
   TXWMPARAM           mp2                      // IN    msg param 2
)
{
   ULONG               rc   = NO_ERROR;
   TXWINDOW           *win;
   static TXLN         expression;              // input expression
   TXLN                evaluatedX;              // evaluated expression
   TXLN                text;
   TXLN                errmsg;

   ENTER();
   TRCMSG( hwnd, msg, mp1, mp2);
   if (hwnd != 0)
   {
      TRCLAS( "SAM std - ", hwnd);
      win = txwWindowData( hwnd);
      switch (msg)
      {
         case TXWM_PGM_INIT:
            strcpy( expression, TxaExeArgv(1));
            txwPostMsg( hwnd, TXWM_PGM_EXEC, 0, 0);
            txwInvalidateAll();                 // repaint before starting
            break;

         case TXWM_PGM_LOOP:
            if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL,
                "Specify a one line expression to be evaluated",
                " expression line to evaluate ", 0,
                  TXPB_MOVEABLE | TXPB_HCENTER | TXPB_VCENTER,
                  99, expression) != TXDID_CANCEL)
            {
               txwPostMsg( hwnd, TXWM_PGM_EXEC, 0, 0);
            }
            break;

         case TXWM_PGM_EXEC:
            TRACES(( "about to eval: '%s'\n", expression));

            //- sprintf( evaluatedX, "{{%s}}", expression);
            strcpy( evaluatedX, expression);    // modifyable version of input string
            rc = txsResolveExpressions( evaluatedX, 0, FALSE, evaluatedX, TXMAXLN, errmsg);

            sprintf( text, "Evaluated string : '%s'", evaluatedX);
            if (strlen( errmsg))
            {
               strcat( text, "\n");
               strcat( text, errmsg);
            }
            TxPrint( "\nRaw input string : '%s'\n%s\n", expression, text);

            strcat( text, "\n\nEvaluate another string ? [Y/N] ");
            if (TxConfirm( 0, text))           // confirmation popup
            {
               txwPostMsg( hwnd, TXWM_PGM_LOOP, 0, 0); // restart the action ...
            }
            else
            {
               txwPostMsg( hwnd, TXWM_CLOSE, 0, 0); // quit the application
            }
            break;

         case TXWM_CLOSE:
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

