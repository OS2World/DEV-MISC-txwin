//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2005 Fsys Software and Jan van Wijk
//
// ==========================================================================
//
// This file contains Original Code and/or Modifications of Original Code as
// defined in and that are subject to the GNU Lesser General Public License.
// You may not use this file except in compliance with the License.
// BY USING THIS FILE YOU AGREE TO ALL TERMS AND CONDITIONS OF THE LICENSE.
// A copy of the License is provided with the Original Code and Modifications,
// and is also available at http://www.dfsee.com/txwin/lgpl.htm
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License,
// or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; (lgpl.htm) if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// Questions on TxWin licensing can be directed to: txwin@fsys.nl
//
// ==========================================================================
//
// TX standard command execution
//
// Author: J. van Wijk
//
// Developed for LPT/DFSEE utilities
//
// 20-11-2001 // Use TXA command parser only
// 11-10-2001 // Reduced memory usage for SMALLMEM by removing commands
// 15-02-2000 // Fixed 'set ansi on/off' bug

#include <txlib.h>                              // TX library interface
#include <txtpriv.h>                            // TX library privates
#include <txwpriv.h>                            // TX windows privates

#if defined (__WATCOMC__)
   #ifndef LINUX
      #include <dos.h>
   #endif
#endif

static char txsetPrio = 'S';                    // default Std priority

/*****************************************************************************/
// Execute standard commands
/*****************************************************************************/
ULONG TxStdCommand
(
   void                                         // parameters through TXA
)
{
   ULONG               rc  = NO_ERROR;
   LONG                nr  = 0;
   TXLN                dc;                      // local command string
   int                 cc  = 0;                 // command string count
   char               *c0, *c1, *c2, *c3;       // parsed command parts
   char               *c4, *c5, *c6, *c7;       // parsed command parts
   TXLN                s1;                      // temporary string space
   char               *pp;                      // parameter pointers

   ENTER();

   #if defined (USEWINDOWING)
      if (txwa->sbview)
      {
         TxaGetArgString( TXA_CUR, 0, TXA_OPT, TXMAXLN, dc);
         sprintf( s1, "TxWin executing: %s", dc);
         txwSendMsg( (TXWHANDLE) txwa->sbview, TXWM_STATUS, (ULONG) s1, cSchemeColor);
      }
   #endif                                       // USEWINDOWING

   pp = TxaGetArgString( TXA_CUR, 1, 0, TXMAXLN, dc); // dc => cmd from arg 1
   cc = TxaArgCount( );                         // number of parameters
   c0 = TxaArgValue(0);
   c1 = TxaArgValue(1);
   c2 = TxaArgValue(2);
   c3 = TxaArgValue(3);
   c4 = TxaArgValue(4);
   c5 = TxaArgValue(5);
   c6 = TxaArgValue(6);
   c7 = TxaArgValue(7);

   TRACES(("cc: %u c0:'%s' c1:'%s' c2:'%s' c3:'%s' c4:'%s' c5:'%s' c6:'%s' c7:'%s' pp:'%s'\n",
            cc,    c0,     c1,     c2,     c3,     c4,     c5,     c6,     c7,     pp));

   if (strcasecmp(c0, "log"      ) == 0)
   {
      char             *current = TxQueryLogName();

      if ((TxaOption('?')) || (c1[0] == '?'))
      {
         TxPrint( "\nStart or stop logging to a file\n\n"
                  "Usage:  log  [filename] [-r] [-f:ff] [-m:mm]\n\n"
                  "   -f[:ff]      retain ff numbered logfiles when cycling\n"
                  "   -m[:mm]      cycle to next logfile after size mm Kb\n"
                  "   -r           close and reopen log on each line (slow)\n"
                  "   filename     filename with optional path for logfile\n"
                  "                When not specified, logging is STOPPED\n\n");

         if (current != NULL)
         {
            TxPrint( "Logging to file   : %s\n", current);
         }
         else
         {
            TxPrint( "There is currently NO logfile active.\n");
         }
      }
      else
      {
         if (TxaOptSet('r'))
         {
            TxSetLogReOpen( TxaOption('r'));
         }
         if (TxaOptSet('m'))
         {
            TxSetLogMaxSize( TxaOptBkmg( 'm', 2047, 'k'));
         }
         if (TxaOptSet('f'))
         {
            TxSetLogRetain( TxaOptNum('f', NULL, 1));
         }
         TxAppendToLogFile( c1, TRUE);
      }
   }
   else if (strcasecmp(c0, "trace"    ) == 0)
   {
      if (TxaOption('?'))
      {
         TxPrint( "\nSet and check TRACE level, format and destinations\n\n"
                  "Usage: trace [level | filename] [-r] [-s] [-t] [-d] [-f] [-m]\n\n"
                  "   -d[:xx]      add xx ms delay for each trace line, slowdown\n"
                  "   -f[:ff]      retain ff numbered logfiles when cycling\n"
                  "   -m[:mm]      cycle to next logfile after size mm Kb\n"
                  "   -r           close and reopen log on each line (slow, default)\n"
                  "   -r-          open log just once (may be truncated on crash)\n"
                  "   -s           trace to the scoll-buffer (screen) too\n"
                  "   -s-          switch off trace to screen\n"
                  "   -t           add timestamps and force thread-id display\n"
                  "   -t-          switch off timestamp and thread-id\n"
                  "   level        0 = off, 1 = window-title, 10..999 = detail\n"
                  "   filename     filename with optional path for tracefile\n"
                  "                (this is the same file used as normal LOG)\n\n"
                  "   The (left) <Alt>+/ key cycles levels 0 -> 1 -> 100 -> 0\n\n");
      }
      else
      {
         if (TxaOptSet('d'))                    // delay xx ms per traceline
         {
            TxTrSlowDown = TxaOptNum( 'd', NULL, 100);
         }
         if (TxaOptSet('r'))
         {
            TxSetLogReOpen(  TxaOption('r'));
         }
         if (TxaOptSet('t'))                    // timestamping specified
         {
            TxTraceSetStamp( TxaOption('t'));
         }
         if (TxaOptSet('s'))
         {
            TxTrLogOnly =   !TxaOption('s');    // to screen with -s
         }
         if (TxaOptSet('m'))
         {
            TxSetLogMaxSize( TxaOptBkmg( 'm', 2047, 'k'));
         }
         if (TxaOptSet('f'))
         {
            TxSetLogRetain( TxaOptNum('f', NULL, 1));
         }
         if (isdigit( c1[0]))                   // setting a level
         {
            TxTrLevel = atol(c1);
         }
         else
         {
            if (strlen( c1))
            {
               TxAppendToLogFile( c1, TRUE);
               TxTrLevel = 88;                  // all except TRACEX
            }
         }
      }
      TxPrint("Funct trace level : %lu  trace to %s\n",  TxTrLevel,
                        (TxTrLevel == 1)              ? "Window title-bar" :
        (TxTrLogOnly) ? (TxQueryLogFile( NULL, NULL)) ? "logfile only"
                                                      : "nowhere!"
                      : (TxQueryLogFile( NULL, NULL)) ? "log and screen"
                                                      : "screen only");
   }
   else if (strcasecmp(c0, "cmd"      ) == 0)
   {
      TxaOptSetItem( "-B");                     // no trusted command check
      if (cc > 1)
      {
         rc = TxExternalCommand( pp);           // execute parameter as cmd
      }
      else
      {
         if ((pp = getenv( "COMSPEC")) != NULL) // when defined
         {
            TxExternalCommand( pp);             // run command interpreter
         }
         else
         {
            TxPrint( "No command processor defined in 'COMSPEC' environment variable\n");
         }
      }
   }
   #ifndef TXMIN
   else if (strncasecmp(c0, "uicc", 4    ) == 0) // 88/256 color cube display
   {
      int     bg;
      int     fg = (cc > 1) ? atoi( c1) : 8;    // default/set forground color
      #if defined (DARWIN)
         int  nc = TxaOptNum( 'c', NULL, 6);    // default 6x6x6
      #else
         int  nc = TxaOptNum( 'c', NULL, 4);    // default 4x4x4
      #endif
      int     r,g,b;                            // rgb cube indexes
      int     limit = (nc == 4) ? 88 : 256;     // maximum color index

      TxPrint("\nUsage: %s  [foreground-color]    [-c:6 | -c:4]\n", c0);
      TxPrint("In non-windowed mode ('mode -w-') shows all available BG colors in %d-color palette\n"
              "In the windowed mode ('mode -w' ) shows the ones mapped to std 16 background colors\n\n", limit);

      //- Show the first 16 (PC-style) color mappings, available for compatibility
      for (r = 0; r < 4; r++)
      {
         for (g = 0; g < 4; g++)
         {
            bg = (r * 4) + g;                   // 16 'PC-style' background colors

            TxPrint( "[48;5;%d;38;5;0m  %3d  [48;5;%d;38;5;%dm  %3d  [48;5;%d;38;5;15m  %3d  %s  ",
                             bg,          bg,        bg,     fg,   fg,        bg,           bg,  CNN);
         }
         TxPrint("%s\n", CNN);
      }
      TxPrint("\n");

      //- Show the cube colors, either 6x6x6 or 4x4x4
      for (r = 0; r < nc; r++)
      {
         for (g = 0; g < nc; g++)
         {
            for (b = 0; b < nc; b++)
            {
               bg = (((r * nc) + g) * nc) + b + 16;

               TxPrint( "[48;5;%d;38;5;0m  %3d  [48;5;%d;38;5;%dm  %3d  [48;5;%d;38;5;15m  %3d  %s  ",
                                bg,          bg,        bg,     fg,   fg,        bg,           bg,  CNN);
            }
            TxPrint("%s\n", CNN);
         }
         TxPrint("\n");
      }

      //- Show the greyscale at end of palette, with foreground color on top
      fg = TxaOptNum('f', NULL, 178);
      for (bg = nc * nc * nc + 16; bg < limit; bg++)
      {
         TxPrint( "[48;5;%d;38;5;%dm  %3d ", bg, fg, bg);
      }
      TxPrint("%s\n", CNN);
   }
   else if (strncasecmp(c0, "uict", 4    ) == 0)
   {
      int     bg,fg;

      TxPrint("%s", ansi[NORMAL]);
      for (bg = 0; bg < 16; bg++)
      {
         TxPrint("\n       ");
         for (fg = 0; fg < 16; fg++)
         {
            TxPrint( "%s %1X%1X %s", ansi[fg + (bg * 16)], bg, fg, ansi[NORMAL]);
         }
      }
      TxPrint("\n\n");
      TxPrint(" ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n" );
      TxPrint(" ³\\³0123456789ABCDEF³\n");
      TxPrint(" ³0³    ³\n" );
      TxPrint(" ³1³³\n" );
      TxPrint(" ³2³ ! #$%%&'()*+,-./³\n");
      TxPrint(" ³3³0123456789:;<=>?³   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿ %s ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍËÍÍÍÍÍÍÍÍ» %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint(" ³4³@ABCDEFGHIJKLMNO³   ³ Single line   ³default ³ %s º Double line   ºYellow  º %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint(" ³5³PQRSTUVWXYZ[ ]^_³   ³ box character ÃÄÄÄÄÄÄÄÄ´ %s º box character ÌÍÍÍÍÍÍÍÍ¹ %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint(" ³6³`abcdefghijklmno³   ³ with joints   ³colors  ³ %s º with joints   ºon Cyan º %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint(" ³7³pqrstuvwxyz{|}~³   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ %s ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÊÍÍÍÍÍÍÍÍ¼ %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint(" ³8³€‚ƒ„…†‡ˆ‰Š‹ŒŽ³\n" );
      TxPrint(" ³9³‘’“”•–—˜™š›œžŸ³\n" );
      TxPrint(" ³A³ ¡¢£¤¥¦§¨©ª«¬­®¯³  %s ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÑÍÍÍÍÍÍÍÍ¸ %s ÖÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÒÄÄÄÄÄÄÄÄ· %s\n", ansi[TXaBZnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint(" ³B³°±²³´µ¶·¸¹º»¼½¾¿³  %s ³ Mixed lines   ³  Grey  ³ %s º Mixed lines   ºWhite onº %s\n", ansi[TXaBZnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint(" ³C³ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ³  %s ³ box character ÆÍÍÍÍÍÍÍÍµ %s º box character ÇÄÄÄÄÄÄÄÄ¶ %s\n", ansi[TXaBZnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint(" ³D³ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß³  %s ³ with joints   ³on White³ %s º with joints   ºMagenta º %s\n", ansi[TXaBZnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint(" ³E³àáâãäåæçèéêëìíîï³  %s ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÏÍÍÍÍÍÍÍÍ¾ %s ÓÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÐÄÄÄÄÄÄÄÄ½ %s\n", ansi[TXaBZnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint(" ³F³ðñòóôõö÷øùúûüýþÿ³\n" );
      TxPrint(" ³/³0123456789ABCDEF³\n" );
      TxPrint(" ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n" );
      TxPrint("%s", ansi[NORMAL]);
   }
   else if (strncasecmp(c0, "txd", 3) == 0)
   {
      char            *filter[5] = {NULL,NULL,NULL,NULL,NULL};

      if (TxaOption('?'))                       // explicit help request
      {
         TxPrint("\nShow files and/or directories in selectable format\n");
         TxPrint("\n Usage:  %s  fspec  select  'attrs' [includes] [-b]\n\n"
                   "   fspec  = File or directory spec, including wildcards\n"
                   "   search = display selection criteria: SFDP\n"
                   "            S=subdirs, F=Files, D=Dirs, P=Parent ..\n"
                   "   attrs  = file-attributes to match  : '+A+R+S+H-A-R-S-H'\n"
                   "            +A  archive-bit  -A NO archive bit\n"
                   "            +R  read-only    -R NOT read-only\n"
                   "            +S  system-bit   -S NO system bit\n"
                   "            +H  hidden       -H NOT hidden\n"
                   "   inexcl = include/exclude wildcards\n", c0);
         TxPrint("\n   -b     = show basename only\n\n");
      }
      else
      {
         if (cc > 4)
         {
            filter[0] = c4;
         }
         if (cc > 5)
         {
            filter[1] = c5;
         }
         if (cc > 6)
         {
            filter[2] = c6;
         }
         if (cc > 7)
         {
            filter[3] = c7;
         }
         TxPrint( "\nTxDir for: %s, options: %s, attributes: %s\n\n",
                  (cc > 1) ? c1 : "current dir",
                  (cc > 2) ? c2 : "file+dirs",
                  (cc > 3) ? c3 : "all files");
         if (TxaOption('b'))                    // basename only
         {
            rc = TxFileTree( c1, c2, c3, "", filter, txFileTreeNoPath,  NULL);
         }
         else
         {
            rc = TxFileTree( c1, c2, c3, "", filter, txFileTreeOneFile, NULL);
         }
      }
   }
   else if (strcasecmp(c0, "uikeys" ) == 0)
   {
      ULONG            input;
      TXW_INPUT_EVENT  mouse;
      BOOL             doubleClick;
      ULN64            dblClkTimer = 0;

      TxPrint( "\n"
      #if defined (UNIX)
               "Note: "
         #if defined (DUMP)
               "use startup switch '-K' for extra debug info (shows table while comparing)\nAlso note that"
         #endif
               "'^' can be another Escape, check hex value!        "
      #endif
               "Type <Esc> to abort the test ...\n\n");
      do
      {
         input = txwGetInputEvent( TRUE, &mouse);
         if (input == TXW_INPUT_MOUSE)          // mouse
         {
            doubleClick = FALSE;
            if ((mouse.value != 0) && ((mouse.value & TXm_DRAGGED) == 0)) // button DOWN, not dragged
            {
               if (TxTmrTimerExpired( dblClkTimer) == FALSE) // NOT expired
               {
                  doubleClick = TRUE;
               }
            }
            dblClkTimer = TxTmrSetTimer( TMRMSEC( 250));
            TxPrint("Mouse @: %3hu %3hu button:%4.4hx Kstate:%4.4hx seconds:%12.6lf %s\n",
                     mouse.row, mouse.col, mouse.value, mouse.state,
                     TxTmrGetSecondsFromStart(), (doubleClick) ? "(DBLCLK)" : "        ");
         }
         else
         {
            TxPrint("Keyvalue: %3.3lX = %-*.*s seconds:%12.6lf        \n",
                     input, 24, 24, txwKeyDescription( input), TxTmrGetSecondsFromStart());
         }
         if (TxaOption( 'u'))                   // rewrite next line over this one
         {
            TxPrint( CU1);
         }
      } while (input != TXK_ESCAPE);
   }
   else if (strcasecmp(c0, "uitime"  ) == 0)    // compare clock() versus TXTIMER accuracy
   {
      clock_t          clockLast = clock();
      clock_t          clockNow;
      time_t           last = time( &last);     // last timestamp in seconds
      time_t           now;                     // current timestamp
      TXTT             datetime;

      TxPrint( "\nType <Esc> to abort the test ...\n");
      while (!TxAbort())
      {
         now = time( &now);
         if (now != last)                       // one second passed
         {
            clockNow = clock();
            strftime( datetime, TXMAXTT, "%Y-%m-%d %H:%M:%S", localtime( &now));
            TxPrint( "Now: %s ClockNow: %14llu Clocks-per-sec:%7llu  Seconds-UP: %15.9lf\n",
                      datetime, (ULN64) clockNow, (ULN64) clockNow - (ULN64) clockLast, TxTmrGetSecondsFromStart());
            last      = now;
            clockLast = clockNow;
         }
      }
   }
   else if (strcasecmp(c0, "quicktime"  ) == 0)
   {
      clock_t          clockNow;
      int              i;
      int              count = TxScreenRows() -5;

      if (cc > 1)
      {
         count = atoi( c1);
      }
      for (i = 0; i < count; i++)
      {
         clockNow = clock();
         TxPrint( "ClockNow: %20llu Seconds-UP: %15.9lf\n", (ULN64) clockNow, TxTmrGetSecondsFromStart());
      }
   }
   #if defined (USEWINDOWING)
   else if (strcasecmp(c0, "mode"    ) == 0)    // change display mode
   {
      #if defined (UNIX)                        // same as auto-detected resize
         TxScreenTerminate();
         TxScreenInitialize();                  // reinit at new windowsize
      #else
         if ((cc > 1) &&                        // resize screen using OS cmd
             (!TxaExeSwitch('S')))              // when not in shell-mode
         {
            USHORT        cols;
            USHORT        rows;

            if ((cc > 2) && (isdigit(c2[0])))
            {
               rows = (USHORT) atoi( c2);       // nr of rows
            }
            else if ((pp = strchr( c1, ',')) != NULL)
            {
               rows = (USHORT) atoi( pp+1);
            }
            else                                // single param, keep rows
            {
               rows = TxScreenRows();
            }
            cols = (USHORT) atoi( c1);          // nr of columns
            if (cols < 10)
            {
               cols = TxScreenCols();
            }
            sprintf( s1, "mode %hu,%hu", cols, rows);

            TxExternalCommand( s1);             // set mode, will cls too
         }
         TxSetBrightBgMode( TRUE);              // no blinking, use bright BG
      #endif
      rc = TX_DISPLAY_CHANGE;                   // signal display change to APP
   }
   #endif                                       // windowing
   #endif                                       // not TXMIN
   else if (strcasecmp(c0, "confirm"  ) == 0)
   {
      if (cc > 1)
      {
         strcpy( s1, pp);
         TxRepl( s1, '~', '\n');
      }
      else                                      // no text specified
      {
         strcpy( s1, "Continue");
      }
      if (TxaOption('y'))                       // confirm Yes/No
      {
         strcat( s1, " ? [Y/N] : ");
         if (!TxConfirm( 0, s1))
         {
            rc = TX_ABORTED;                    // Will result in 'better'
         }                                      // message to user :-)
      }
      else
      {
         if (cc == 1)                           // no ? after custom text
         {
            strcat( s1, " ?");
         }
         if (!TxMessage( !TxaOption('n'), 0, s1)) // -n needs no key pressed
         {
            rc = TX_ABORTED;                    // Will result in 'better'
         }                                      // message to user :-)
      }
   }
#if defined (DOS32)
   else if (strcasecmp(c0, "keyb"    ) == 0)       // change keyboard mapping
   {
      if ((cc > 1) && (!TxaOption('?')))
      {
         if ((rc = TxSetNlsKeyboard( c1, c2)) != NO_ERROR)
         {
            TxPrint( "\nError %lu setting keyboard mapping '%s'\n\n", rc, c1);
         }
      }
      else                                      // give help
      {
         TxPrint("\nSet country or codepage-specific keyboard mapping\n");
         TxPrint("\n Usage:   %s  def [cp]\n\n"
                   "   def  = Keyboard definition file (.kl) basename\n"
                   "   cp   = Codepage value valid for that language\n\n"
                   " Example: keyb nl 850\n\n", c0);

         TxExternalCommand( "keyb");            // show current keyb, if any
      }
   }
#endif
   else if (strcasecmp(c0, "start"   ) == 0)
   {
      sprintf( s1, "start /b /c %s", pp);
      rc = system( s1);                         // execute parameter as cmd
   }
   else if ((strcasecmp(c0, "cd") == 0) || ((strlen(c0) == 2) && (c0[1] == ':') ))
   {
      if (strcasecmp(c0, "cd") == 0)
      {
         if (*pp == '~')                        // relative to HOME directory
         {
            #if defined (WIN32)
               char   *home = getenv("HOMEPATH");
            #else
               char   *home = getenv("HOME");   // start with home directory
            #endif

            strcpy( s1, (home) ? home : ".");
            strcat( s1, pp + 1);                // add relative path, if any
         }
         else                                   // explicit path given
         {
            strcpy( s1, pp);
         }
      }
      else                                      // absolute, with driveletter
      {
         strcpy( s1, c0);
      }
      TxStrip( s1, s1, ' ', ' ');
      if (strlen( s1))                          // only when specified
      {
         if (s1[ strlen(s1) - 1] == FS_PATH_SEP)
         {
             s1[ strlen(s1) - 1] = 0;           // remove trainling path seperator
         }
         #if defined (UNIX)
            rc = (ULONG) chdir( s1);
         #else
            if (s1[1] == ':')                   // set drive too, if specified
            {
               #if defined (__WATCOMC__)
                  #ifndef LINUX
                  unsigned  drives;

                  _dos_setdrive(toupper( s1[0]) - 'A' +1, &drives);
                  #endif
               #elif defined (DARWIN)           // DARWIN MAC OS X (GCC)
               #else
                  _chdrive(toupper( s1[0]) - 'A' +1);
               #endif
            }
            if ((strlen( s1) > 2) || (s1[1] != ':'))
            {
               rc = (ULONG) chdir( s1);
            }
         #endif
      }
      if (rc != NO_ERROR)
      {
         TxPrint( "\nError changing to : '%s'\n", s1);
         rc = NO_ERROR;
      }
      getcwd(s1, TXMAXLN);
      TxPrint("\nWorking directory : %s%s%s%s\n",
                 CBC, s1, (strlen(s1) > 3) ? FS_PATH_STR : "", CNN);
   }
   else if (strcasecmp(c0, "exist"  ) == 0)
   {
      if (cc > 1)
      {
         TxPrint("File '%s' does%s exist\n", s1, (TxFileExists(c1)) ? "": " NOT");
      }
   }
   #if defined (USEWINDOWING)
   else if ((strcasecmp(c0, "scrfile") == 0))   // screen to file
   {
      ULONG            lines = -1;              // default all lines

      if (cc <= 1)
      {
         strcpy( s1, "screen");
      }
      else
      {
         strcpy( s1, c1);
      }
      TxFnameExtension( s1, "log");             // append default extension
      if (cc > 2)
      {
         lines = (ULONG) atol( c2);
      }
      lines = txwSavePrintfSB( s1, lines, (cc <= 3));
      TxPrint( "Saved %lu lines from screen-buffer to %s\n", lines, s1);
   }
   #endif                                       // USEWINDOWING
   else if ((strcasecmp(c0, "screen"   ) == 0)) // backward compatibility!
   {
      if ((cc > 1) && (!TxaOption('?')))
      {
         if ((strcasecmp(c1, "on") == 0) || (c1[0] == '1'))
         {
            TxScreenState(DEVICE_ON);
         }
         else
         {
            TxScreenState(DEVICE_OFF);
         }
      }
      else
      {
         rc = (ULONG) TxScreenState( DEVICE_TEST);
         TxPrint("Screen output is switched %s.\n",
                 (rc == (ULONG) DEVICE_ON) ? "on" : "off");
      }
   }
   else if ((strcasecmp(c0, "set"      ) == 0))
   {
      if (cc > 1)
      {
         if      ((strcasecmp(c1, "screen"   ) == 0))
         {
            if (cc > 2)
            {
               #if defined (USEWINDOWING)
               if       (strncasecmp(c2, "i", 1)  == 0)
               {
                  if (txwa->sbview)
                  {
                     txwa->sbview->window->sb.altcol ^= TXSB_COLOR_INVERT;
                  }
               }
               else if  (strncasecmp(c2, "b", 1)  == 0)
               {
                  if (txwa->sbview)
                  {
                     txwa->sbview->window->sb.altcol ^= TXSB_COLOR_BRIGHT;
                  }
               }
               else if  (strncasecmp(c2, "s", 1)  == 0)
               {
                  if (txwa->sbview)
                  {
                     txwa->sbview->window->sb.altcol ^= TXSB_COLOR_B2BLUE;
                  }
               }
               else
               #endif                           // USEWINDOWING
               if ((strcasecmp(c2, "on") == 0) || (c2[0] == '1'))
               {
                  TxScreenState(DEVICE_ON);
               }
               else
               {
                  TxScreenState(DEVICE_OFF);
               }
            }
            else
            {
               rc = TxScreenState( DEVICE_TEST);
               TxPrint("\nSet screen text-output properties\n\n Usage: %s %s  on | off"
               #if defined (USEWINDOWING)
                                                        " | invert | bright | swapblue  (toggle!)"
               #endif
                       "\n\nScreen output is switched %s\n", c0, c1, (rc == DEVICE_ON) ? "on" : "off");
            }
         }
         else if ((strncasecmp(c1, "logfile", 7 ) == 0))
         {

            if (cc > 2)
            {
               if ((strcasecmp(c2, "on") == 0) || (c2[0] == '1'))
               {
                  TxLogfileState(DEVICE_ON);
               }
               else
               {
                  TxLogfileState(DEVICE_OFF);
               }
            }
            else
            {
               rc = TxLogfileState( DEVICE_TEST);
               TxPrint("\nSet logfile output status\n\n"
                       " Usage: %s %s on | off\n\n"
                       "LOGFILE is currently %s\n", c0, c1,
                       (rc == DEVICE_ON) ? "on" : "off");
            }
         }
         else if ((strncasecmp(c1, "ansi", 4 ) == 0))
         {
            if (cc > 2)
            {
               if ((strcasecmp(c2, "on") == 0) || (c2[0] == '1'))
               {
                  TxSetAnsiMode( A_ON);
               }
               else
               {
                  TxSetAnsiMode( A_OFF);
               }
            }
            else
            {
               TxPrint("\nSet usage of ANSI color for text output\n\n"
                       " Usage: %s %s on | off\n\n"
                       "ANSI is currently %s\n", c0, c1,
                       (TxGetAnsiMode() == A_ON) ? "ON" : "OFF");
            }
         }
   #if defined (USEWINDOWING)
         else if ((strncasecmp(c1, "history", 4 ) == 0))
         {
            if (cc > 2)
            {
               if      ((strcasecmp(c2, "plain" ) == 0) || (c2[0] == '0'))
               {
                  txwa->historyMode = TXW_HIST_PLAIN;
               }
               else if ((strcasecmp(c2, "filter") == 0) || (c2[0] == '1'))
               {
                  txwa->historyMode = TXW_HIST_FILTER;
               }
               else if ((strcasecmp(c2, "unique") == 0) || (c2[0] == '2'))
               {
                  txwa->historyMode = TXW_HIST_UNIQUE;
               }
               else
               {
                  txwa->historyMode = TXW_HIST_UNIFIL;
               }
            }
            else
            {
               TxPrint("\nSet filter behaviour for the history buffer (commandline, <F11>)\n\n"
                       " Usage: %s %s plain | 0 | filter | 1 | unique | 2 | classic | 3\n", c0, c1);
            }
            TxPrint("\nHISTORY behaviour is currently: ");
            switch (txwa->historyMode)
            {
               case TXW_HIST_PLAIN:  TxPrint( "PLAIN   (no duplicate removal, no completion filter)\n"); break;
               case TXW_HIST_FILTER: TxPrint( "FILTER  (no duplicate removal, completion filtering)\n"); break;
               case TXW_HIST_UNIQUE: TxPrint( "UNIQUE  (duplicate removal, no completion filtering)\n"); break;
               default:              TxPrint( "CLASSIC (duplicate removal and completion filtering)\n"); break;
            }
         }
         else if ((strncasecmp(c1, "scheme", 6 ) == 0))
         {
            if (cc > 2)
            {
               txwColorScheme( c2[0], NULL);
            }
            else
            {
               TxPrint("\nSet color-scheme used for Windowing\n\n"
                       " Usage: %s %s  grey|m3d|nobl|cmdr|white|black|dfsee\n", c0, c1);
            }
            TxPrint("\nSCHEME is currently: '%s'\n", txwcs->name);
            if (txwa->desktop != NULL)
            {
               txwInvalidateAll();
            }
         }
         else if ((strncasecmp(c1, "style", 5 ) == 0))
         {
            if (cc > 2)
            {
               txwcs->linestyle = (atol(c2) % TXW_CS_LAST);
            }
            else
            {
               TxPrint("\nSet linestyle used for Windowing\n\n"
                       " Usage: %s %s  0..3\n\n"
                       "        0=double 1=3d 2=halfblock 3=fullblock\n", c0, c1);
            }
            TxPrint("\nSTYLE is currently: %lu\n", txwcs->linestyle);
            if (txwa->desktop != NULL)
            {
               txwInvalidateAll();
            }
         }
         else if ((strncasecmp(c1, "color", 5 ) == 0))
         {
            if (txwa->sbview)
            {
               TXWINDOW  *sbwin = txwa->sbview->window;

               if (cc > 2)
               {
                  sbwin->sb.altcol = atol(c2) & TXSB_COLOR_MASK;
                  txwInvalidateWindow((TXWHANDLE) txwa->sbview, FALSE, FALSE);
               }
               else
               {
                  TxPrint("\nSet color-scheme used for Output\n\n"
                          " Usage: %s %s  numeric-value 0 .. 7\n\n"
                          "  0 = standard output colors\n"
                          "  1 = invert all color values\n"
                          "  2 = force bright foreground\n"
                          "  4 = use Blue/Brown background\n\n"
                          "  Values can be combined by adding them.\n", c0, c1);
               }
               TxPrint("\nCOLOR value is currently: '%lu'\n", sbwin->sb.altcol);
            }
            else
            {
               TxPrint("\nSet color-scheme for output not supported.\n");
            }
         }
         else if ((strncasecmp(c1, "palet", 5 ) == 0))
         {
            if (cc > 2)
            {
               int     colors = atoi( c2);
               TxSwitchAnsiPalette( colors);    // Set the palette
               txwRefreshScreen();              // flush, redraw everything
            }
            else
            {
               TxPrint("\nSet number of colors to be used\n\n"
                       " Usage: %s %s  1 | 2 | 8 | 16"

                 #if defined (UNIX)
                                                        " | 88 | 256"
                 #endif
                   "\n\n    1 = no color, just standard set  1 FG +  1 BG\n"
                       "    2 = no color, NORMAL + INTENSE   2 FG +  1 BG\n"
                       "    8 = standard set of 'PC' colors 16 FG +  8 BG\n"
                       "   16 = extended set of 'PC' colors 16 FG + 16 BG\n"
                 #if defined (UNIX)
                       "   88 = 16 FG/BG colors taken from palette of  88\n"
                       "  256 = 16 FG/BG colors taken from palette of 256\n"
                 #endif
                       , c0, c1);
            }
            TxPrint("\nPALETTE number of colors is currently: %d\n", txwa->colors);
         }
   #endif                                 // USEWINDOWING
         else if ((strncasecmp(c1, "asc", 3) == 0))
         {
            if (cc > 2)
            {
               if ((strcasecmp(c2, "on") == 0) || (c2[0] == '7'))
               {
                  TxSetAscii7Mode( TRUE);
               }
               else
               {
                  TxSetAscii7Mode( FALSE);
               }
            }
            else
            {
               TxPrint("\nSet ASCII output to 7-bit only\n\n"
                       " Usage: %s %s 7 | 8\n\n"
                       "ASCII is currently %s-bit\n", c0, c1,
                       (TxGetAscii7Mode()) ? "7" : "8");
            }
         }
         else if ((strncasecmp(c1, "type", 4  ) == 0))
         {
            if (cc > 2)
            {
               if ((strcasecmp(c2, "on") == 0) || (c2[0] == '1'))
               {
                  txwa->typeahead = TRUE;
               }
               else
               {
                  txwa->typeahead = FALSE;
               }
            }
            else
            {
               TxPrint("\nSet keyboard type-ahead\n\n" " Usage: %s %s on | off\n", c0, c1);
            }
            TxPrint("\nTYPEahead is currently set: %s\n",
                    (txwa->typeahead) ? "on" : "off");
         }
         else if ((strncasecmp(c1, "radix", 5  ) == 0))
         {
            if (cc > 2)
            {
               if (toupper(c2[0]) == 'H')
               {
                  txwa->radixclass = 0xFFFFFFFF;
               }
               else
               {
                  txwa->radixclass = (ULONG) TxaParseNumber( c2, TX_RADIX_STD_CLASS, NULL);
               }
            }
            else
            {
               TxPrint("\nSet mcs-number Radix mask, default 0 = all decimal\n\n"
                       " Usage: %s %s H | 0 | mask\n\n"
                       "     H = all classes set to HEX\n"
                       "     0 = all classes set to DECimal\n"
                       "  mask = classes with bit SET will be HEX, others DECimal\n", c0, c1);
               rc = TX_PENDING;
            }
            TxPrint("\nNumber Radix class mask currently set to: 0x%8.8lx = %lu\n", txwa->radixclass, txwa->radixclass );
         }
         else if ((strncasecmp(c1, "reset", 5  ) == 0))
         {
            if (cc > 2)
            {
               if ((strcasecmp(c2, "on") == 0) || (c2[0] == '1'))
               {
                  txwa->resetterm = TRUE;
               }
               else
               {
                  txwa->resetterm = FALSE;
               }
            }
            else
            {
               TxPrint("\nReset terminal on program exit\n\n" " Usage: %s %s on | off\n", c0, c1);
            }
            TxPrint("\nRESET terminal is currently set: %s\n", (txwa->resetterm) ? "on" : "off");
         }
         #ifndef DOS32
         else if (strncasecmp(c1, "prio", 4  ) == 0)
         {
            TxPrint("\nRelative priority : ");
            switch (TxSetPriority( c2[0]))
            {
               case 'M': TxPrint( "Minimum\n");  break;
               case 'L': TxPrint( "Low\n");      break;
               case 'S': TxPrint( "Standard\n"); break;
               case 'H': TxPrint( "High\n");     break;
               case 'X': TxPrint( "maXimum\n");  break;
               default:  TxPrint( "unknown!\n"); break;
            }
            TxPrint( "\n");
         }
         #endif
         else                                   // non TXLIB set property
         {
            rc = TX_PENDING;
         }
      }
      else
      {
         TxPrint( "\nSet program property to specified value, or show current\n"
                  "\n Usage:  %s  property  value\n", c0);
         TxPrint( "\nTxWindows SET properties are :\n"
                  "  ANSI-colors  : on  | off\n"
                  "  ASCii-7bit   : on  | off\n"
         #if defined (USEWINDOWING)
                  "  COLOR  output: numeric-value 0 .. 8\n"
                  "  PALET colors : 1 | 2 | 8 | 16"
               #if defined (UNIX)
                                                      " | 88 | 256"
               #endif
                "\n  HISTORY mode : plain | 0 | filter | 1 | unique | 2 | classic | 3\n"
         #endif
                  "  LOGFILE      : on  | off\n"
         #ifndef DOS32
                  "  PRIOrity     : Min | Low | Std | High | maX | Query\n"
         #endif
                  "  Radix        : H   |  0  | mask\n"
                  "  RESET        : on  | off\n"
                  "  SCREEN       : on  | off"
         #if defined (USEWINDOWING)
                                            " | invert | bright | swapblue\n"
                  "  SCHEME color : 3d  |dfsee| nobl | cmdr | white | black | grey\n"
                  "  STYLE  lines : numeric-value 0 .. 3"
         #endif
                "\n  TYPEahead    : on  | off\n"
                  "\n");
         rc = TX_PENDING;
      }
   }
   else if ((strcasecmp(c0, "say"      ) == 0))
   {
      TxPrint("%s\n", pp);
   }
   else if ((strcasecmp(c0, "sleep"    ) == 0))
   {
      nr = atol( c1);
      if (nr == 0)
      {
         nr = 1;                                // default 1 sec
      }
      if (!TxaOption('q'))
      {
         TxPrint( "\nSleeping for %ld seconds ...\n", nr);
      }
      TxSleep( nr * 1000);
   }
   else
   {
      rc = TX_CMD_UNKNOWN;
   }
   #if defined (UNIX)
      if (rc == (ULONG) -1)
      {
         rc = TX_UNIX_ERROR;                    // specific 16-bit error value
      }
   #endif
   RETURN (rc & 0xffff);                        // 16-bit, preserve flag values
}                                               // end 'TxStdCommand'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set current thread priority Min/Low/Std/High/maX
/*****************************************************************************/
char TxSetPriority                              // RET   resulting priority
(
   char                prio                     // IN    priority M/L/S/H/X/Q
)
{
   ENTER();
   TRACES(( "Priority command: %2.2hu\n", prio));

   if (prio && strchr( "mlshxMLSHX", prio) != NULL)
   {
      txsetPrio = toupper(prio);
      switch (txsetPrio)
      {
         case 'M': TxThreadPrioMin();  break;
         case 'L': TxThreadPrioLow();  break;
         case 'S': TxThreadPrioStd();  break;
         case 'H': TxThreadPrioHigh(); break;
         case 'X': TxThreadPrioMax();  break;
      }
   }
   RETURN (txsetPrio);
}                                               // end 'TxSetPriority'
/*---------------------------------------------------------------------------*/

