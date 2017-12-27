#define SAM_D "Popup-window sample for JvW-Fsys TXW OpenWatcom build environment."

#define SAM_C "(c) 2014: Jan van Wijk"

#define SAM_V "2.00 13-06-2014" // Minor update for TXLib 2.0
//efine SAM_V "1.00 21-09-2005" // Initial version

#include <txlib.h>                              // TX library interface
#if   defined (WIN32)
   #define SAM_N "SAM3 winNT"
#elif defined (DOS32)
   #define SAM_N "SAM3 Dos32"
#elif defined (LINUX)
   #define SAM_N "SAM3 Linux"
#elif defined (DARWIN)
   #define SAM_N "SAM3 OS-X "
#else
   #define SAM_N "SAM3  OS/2"
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


int main (int argc, char *argv[]);

/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                text;

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
      if (txwInitializeDesktop( NULL, NULL) != 0)
      {

         sprintf( text, "Hello from SAMPLE-3, parameter = '%s'", TxaExeArgv(1));

         TxMessage( TRUE, 0, text);             // present message in a popup

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

