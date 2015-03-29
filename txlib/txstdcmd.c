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
                  "   -r           close and reopen log on each line (slow)\n"
                  "   -r-          open log just once (this is the default)\n"
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
      if (cc > 1)
      {
         rc = TxExternalCommand( pp);           // execute parameter as cmd
      }
      else
      {
         if ((pp = getenv( "COMSPEC")) != NULL) // when defined
         {
            TxaOptSetItem( "-B");               // no trusted command check
            TxExternalCommand( pp);             // run command interpreter
         }
         else
         {
            TxPrint( "No command processor defined in 'COMSPEC' environment variable\n");
         }
      }
   }
   #ifndef TXMIN
   else if (strcasecmp(c0, "cboxes"   ) == 0)
   {
      TxPrint("\n%s", ansi[NORMAL]);
      TxPrint("                  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿  \n");
      TxPrint("                  ³ Single line   ³default ³  \n");
      TxPrint("                  ³ box character ÃÄÄÄÄÄÄÄÄ´  \n");
      TxPrint("                  ³ with joints   ³colors  ³  \n");
      TxPrint("                  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ  \n");
      TxPrint("\n");
      TxPrint("                %s  ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍËÍÍÍÍÍÍÍÍ»  %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint("                %s  º Double line   ºYellow  º  %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint("                %s  º box character ÌÍÍÍÍÍÍÍÍ¹  %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint("                %s  º with joints   ºon Cyan º  %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint("                %s  ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÊÍÍÍÍÍÍÍÍ¼  %s\n", ansi[TXaBYnC], ansi[NORMAL]);
      TxPrint("\n");
      TxPrint("    %s ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÑÍÍÍÍÍÍÍÍ¸ %s ÖÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÒÄÄÄÄÄÄÄÄ· %s\n", ansi[TXaNMnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint("    %s ³ Mixed lines   ³Magenta ³ %s º Mixed lines   ºWhite onº %s\n", ansi[TXaNMnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint("    %s ³ box character ÆÍÍÍÍÍÍÍÍµ %s º box character ÇÄÄÄÄÄÄÄÄ¶ %s\n", ansi[TXaNMnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint("    %s ³ with joints   ³on Grey ³ %s º with joints   ºMagenta º %s\n", ansi[TXaNMnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint("    %s ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÏÍÍÍÍÍÍÍÍ¾ %s ÓÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄĞÄÄÄÄÄÄÄÄ½ %s\n", ansi[TXaNMnW], ansi[TXaNWnM], ansi[NORMAL]);
      TxPrint("%s\n", ansi[NORMAL]);
   }
   else if (strcasecmp(c0, "charset"    ) == 0)
   {
      TxPrint( "       %s\n", "ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿" );
      TxPrint( "       %s\n", "³\\³0123456789ABCDEF³" );
      TxPrint( "       %s\n", "³0³    ³" );
      TxPrint( "       %s\n", "³1³³" );
      TxPrint( "       %s\n", "³2³ ! #$%&'()*+,-./³" );
      TxPrint( "       %s\n", "³3³0123456789:;<=>?³" );
      TxPrint( "       %s\n", "³4³@ABCDEFGHIJKLMNO³" );
      TxPrint( "       %s\n", "³5³PQRSTUVWXYZ[ ]^_³" );
      TxPrint( "       %s\n", "³6³`abcdefghijklmno³" );
      TxPrint( "       %s\n", "³7³pqrstuvwxyz{|}~³" );
      TxPrint( "       %s\n", "³8³€‚ƒ„…†‡ˆ‰Š‹Œ³" );
      TxPrint( "       %s\n", "³9³‘’“”•–—˜™š›œŸ³" );
      TxPrint( "       %s\n", "³A³ ¡¢£¤¥¦§¨©ª«¬­®¯³" );
      TxPrint( "       %s\n", "³B³°±²³´µ¶·¸¹º»¼½¾¿³" );
      TxPrint( "       %s\n", "³C³ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ³" );
      TxPrint( "       %s\n", "³D³ĞÑÒÓÔÕÖ×ØÙÚÛÜİŞß³" );
      TxPrint( "       %s\n", "³E³àáâãäåæçèéêëìíîï³" );
      TxPrint( "       %s\n", "³F³ğñòóôõö÷øùúûüışÿ³" );
      TxPrint( "       %s\n", "³/³0123456789ABCDEF³" );
      TxPrint( "       %s\n", "ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ" );
   }
   else if (strcasecmp(c0, "colors"    ) == 0)
   {
      int     bg,fg;

      TxPrint("%s", ansi[NORMAL]);
      for (bg = 0; bg < 16; bg++)
      {
         TxPrint("\n    ");
         for (fg = 0; fg < 16; fg++)
         {
            TxPrint( "%s %1X%1X %s", ansi[fg + (bg * 16)],
                             bg, fg, ansi[NORMAL]);
         }
      }
      TxPrint("\n\n");
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
   else if (strcasecmp(c0, "keys"   ) == 0)
   {
      ULONG            input;
      TXW_INPUT_EVENT  mouse;

      while ((input = txwGetInputEvent( &mouse)) != TXK_ESCAPE)
      {
         if (input == TXW_INPUT_MOUSE)          // mouse
         {
            TxPrint("Mouse @ : % 3hu % 3hu button:%4.4hx Kstate: %4.4hx\n",
                     mouse.row, mouse.col, mouse.value, mouse.state);
         }
         else
         {
            TxPrint("Keyvalue: %3.3lX\n", input);
         }
      }
   }
   #if defined (USEWINDOWING)
   else if (strcasecmp(c0, "mode"    ) == 0)    // change display mode
   {
      if ((cc > 1) &&                           // resize screen using OS cmd
          (!TxaExeSwitch('S')))                 // when not in shell-mode
      {
         USHORT        cols;
         USHORT        rows;

         if ((cc > 2) && (isdigit(c2[0])))
         {
            rows = (USHORT) atoi( c2);          // nr of rows
         }
         else if ((pp = strchr( c1, ',')) != NULL)
         {
            rows = (USHORT) atoi( pp+1);
         }
         else                                   // single param, keep rows
         {
            rows = TxScreenRows();
         }
         cols = (USHORT) atoi( c1);             // nr of columns
         if (cols < 10)
         {
            cols = TxScreenCols();
         }
         sprintf( s1, "mode %hu,%hu", cols, rows);

         TxExternalCommand( s1);                // set mode, will cls too
      }
      TxSetBrightBgMode( TRUE);                 // no blinking, use bright BG
      rc = TX_DISPLAY_CHANGE;                   // signal display change
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
   else if ((strcasecmp(c0, "cd") == 0) ||
            ((strlen(c0) == 2) && (c0[1] == ':') ))
   {
      if (strcasecmp(c0, "cd") == 0)
      {
         strcpy( s1, pp);
      }
      else
      {
         strcpy( s1, c0);
      }
      TxStrip( s1, s1, ' ', ' ');
      if (strlen( s1))                          // only when specified
      {
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
               TxPrint("\nSet screen text-output properties\n\n"
                       " Usage: %s %s  on | off"
               #if defined (USEWINDOWING)
                       " | invert | bright | swapblue"
               #endif                           // USEWINDOWING
                       " Usage: %s %s  on | off | invert | bright | swapblue\n\n"
                       "\n\nScreen output is switched %s.\n\n", c0, c1,
                       (rc == DEVICE_ON) ? "on" : "off");
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
                       "ANSI is currently %s\n\n", c0, c1,
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
                       "ANSI is currently %s\n\n", c0, c1,
                       (TxGetAnsiMode() == A_ON) ? "ON" : "OFF");
            }
         }
         #if defined (USEWINDOWING)
         else if ((strncasecmp(c1, "scheme", 6 ) == 0))
         {
            if (cc > 2)
            {
               txwColorScheme( c2[0], NULL);
            }
            else
            {
               TxPrint("\nSet color-scheme used for Windowing\n\n"
                       " Usage: %s %s  grey|m3d|nobl|cmdr|half|full|white|black|dfsee\n\n"
                       "SCHEME is currently: '%s'\n\n", c0, c1, txwcs->name);
            }
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
                       "        0=double 1=3d 2=halfblock 3=fullblock\n\n"
                       "STYLE is currently: %lu\n\n", c0, c1, txwcs->linestyle);
            }
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
                          " Usage: %s %s  numeric-value 0 .. 8\n\n"
                          "  0 = standard output colors\n"
                          "  1 = invert all color values\n"
                          "  2 = force bright foreground\n"
                          "  4 = use Blue/Brown background\n\n"
                          "  Values can be combined by adding them.\n\n"
                          "COLOR value is currently: '%lu'\n\n",
                           c0, c1, sbwin->sb.altcol);
               }
            }
            else
            {
               TxPrint("\nSet color-scheme for output not supported.\n");
            }
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
                       "ASCII is currently %s-bit\n\n", c0, c1,
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
               TxPrint("\nSet keyboard type-ahead"
                       " Usage: %s %s on | off\n\n"
                       "TYPEahead is currently set: %s\n", c0, c1,
                        (txwa->typeahead) ? "on" : "off");
            }
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
                  txwa->radixclass = TxaParseNumber( c2, TX_RADIX_STD_CLASS, NULL);
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
            TxPrint("\nNumber Radix class mask currently set to: 0x%8.8lx = %lu\n",
                     txwa->radixclass, txwa->radixclass );
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
         #endif
                  "  LOGFILE      : on  | off\n"
         #ifndef DOS32
                  "  PRIOrity     : Min | Low | Std | High | maX | Query\n"
         #endif
                  "  Radix        : H   |  0  | mask\n"
                  "  SCREEN       : on  | off"
         #if defined (USEWINDOWING)
                                            " | invert | bright | swapblue\n"
                  "  SCHEME color : 3d  |dfsee| nobl | cmdr | half | full | white | black | grey\n"
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
   RETURN (rc);
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

