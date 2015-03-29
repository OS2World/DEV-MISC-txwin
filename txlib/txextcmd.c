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
// TX external command execution
//
// Author: J. van Wijk
//
// 24-07-2005  Restyled for TXWIN open version

#include <txlib.h>                              // TX library interface
#include <txwpriv.h>                            // TX private interfaces

#ifndef DEV32

#define TXC_TEMP_FILE_NAME "_-!TX!-_.TMP"

// Execute, using FILE redirected stdout/stderr to TxPrint
static ULONG txcExecStdOutFile
(
   char               *cmd                      // IN    command to execute
);

#else                                           // OS/2

// temporary hack to het KBD handle (move IOCTl to txwmsg.c later)
extern HFILE   txw_hkeyboard;


#define TXC_PIPE_SIZE      16384                // size of output pipe
#define TXC_NEW_HANDLE     0xFFFFFFFF           // new from DosDup...
#define TXC_MAX_TIMEOUTS   9                    // max nr of receive timeouts
#define TXC_EOF            ((char) 0x1a)        // end-of-file indicator
#define TXC_CR             ((char) 0x0d)        // carriage return
#define TXC_LF             ((char) 0x0a)        // linefeed

#define STDIN              0
#define STDOUT             1
#define STDERR             2


typedef struct txc_io_handles
{
   TXHFILE             read;
   TXHFILE             write;
   TXHFILE             save;
   char                ind;                     // progress indicator
} TXC_IO_HANDLES;                               // end of struct


// Execute an external command, using PIPE redirected stdout/stderr to TxPrint
ULONG txcExecStdOutPipe
(
   char               *cmd                      // IN    command to execute
);

// TXC child-stdout thread, waiting for output by child-process; to TxPrint
static void txcPipeOutputPump
(
   void               *p                        // IN    pointer parameter
);

// Write to stream (redirected stdout) with logfile (TEE) and output hooks
static void TxWriteStream
(
   TXHFILE             stream,                  // IN    stream to write to
   char               *text                     // IN    string to write
);

#endif

#if defined (UNIX)
static char txcTrustedExternals[] = " LS LL CD RM CHMOD CAT CLEAR CP SU MV DU DF MOUNT UMOUNT ";
#else
static char txcTrustedExternals[] = " DIR CD DEL REN RENAME ATTRIB SET TYPE MODE CLS CHCP"
#if defined (DOS32)
                                    " MEM KEYB "
#else
                                    " HELP MOVE"
#endif
                                    " COPY ";
#endif




/*****************************************************************************/
// Execute an external command, redirect stdout/stderr to TxPrint
/*****************************************************************************/
ULONG TxExternalCommand
(
   char               *cmd                      // IN    command to execute
)
{
   ULONG               rc  = NO_ERROR;
   TXTS                first;                   // first word
   char               *s;

   ENTER();

   memset( first, ' ',    TXMAXTS);
   TxCopy( first +1, cmd, TXMAXTS -1);
   if ((s = strchr( first +1, ' ')) != NULL)
   {
      *(s+1) = 0;
   }
   TxStrToUpper( first);                        // uppercased first word

   TRACES(( "first: '%s' for cmd: '%s'\n", first, cmd));

   if ((TxaExeSwitch('b')) || (TxaOption('B'))       ||
       (strstr( txcTrustedExternals, first) != NULL) ||
       (TxConfirm( TXCM_HELPREG + 00, "%s%s%s is not a trusted command!\nAre you "
                   "sure you want to have :\n\n%s%s%s\n\nexecuted as an external "
                   "command ? [Y/N] : ", CBR, first +1, CNN, CBG, cmd, CNN)))
   {
      #if defined (DEV32)
         #if defined (USEWINDOWING)
            txwa->KbdKill = TRUE;               // hack to quit KBD thread

            #if defined (TRY_KBD_HACK)          // JvW: causes PM-wide KBD hang
               {
                  BYTE  kbdMode  = 2;
                  ULONG ParamLen = sizeof(kbdMode);

                  //- Use mode IOCtl, may unblock KBD subsystem from KbdCharIn()
                  DosDevIOCtl( txw_hkeyboard, IOCTL_KEYBOARD, KBD_SETINPUTMODE,
                               &kbdMode, ParamLen, &ParamLen, NULL, 0, NULL);

                  kbdMode = 0;                  // back to ASCII mode
                  DosDevIOCtl( txw_hkeyboard, IOCTL_KEYBOARD, KBD_SETINPUTMODE,
                               &kbdMode, ParamLen, &ParamLen, NULL, 0, NULL);
               }
            #else
               TxPrint( "\nPress ENTER to execute: '%s'\n", cmd);
            #endif

            rc = txcExecStdOutPipe( cmd);
            TxInputDesktopInit();               // restart KBD thread
         #else
            rc = txcExecStdOutPipe( cmd);       // simple KBD IO used
         #endif
      #else
         rc = txcExecStdOutFile( cmd);
      #endif
   }
   #if defined (USEWINDOWING)
      if (txwIsWindow( TXHWND_DESKTOP))
      {
         txwInvalidateAll();                    // repaint windows, in case
      }                                         // the screen was 'damaged'
   #endif                                       // USEWINDOWING
   RETURN (rc);
}                                               // end 'TxExternalCommand'
/*---------------------------------------------------------------------------*/

#ifndef DEV32

/*****************************************************************************/
// Execute an external command, using FILE redirected stdout/stderr to TxPrint
/*****************************************************************************/
static ULONG txcExecStdOutFile
(
   char               *cmd                      // IN    command to execute
)
{
   ULONG               rc  = NO_ERROR;
   TXLN                command;                 // redirected command
   FILE               *so;                      // std output file

   ENTER();

   strcpy( command, cmd);                       // command to execute
   strcat( command, " > ");
   strcat( command, TXC_TEMP_FILE_NAME);
   #if defined (WIN32)                          // redirect stderr too
      strcat( command, " 2>&1");
   #endif

   TxDeleteFile( TXC_TEMP_FILE_NAME);
   TRACES(( "External command: '%s'\n", command));
   rc = (ULONG) system( command);               // execute the command

   if ((so = fopen( TXC_TEMP_FILE_NAME, "r" TXFMODE)) != NULL)
   {
      while (!TxAbort() && !feof(so) && !ferror(so))
      {
         if (fgets( command, TXMAXLN, so) != NULL)
         {
            TxPrint( "%s", command);
         }
      }
      fclose( so);
   }
   TxDeleteFile( TXC_TEMP_FILE_NAME);
   RETURN (rc);
}                                               // end 'txcExecStdOutFile'
/*---------------------------------------------------------------------------*/


#else                                           // OS/2

/*****************************************************************************/
// Execute an external command, using PIPE redirected stdout/stderr to TxPrint
/*****************************************************************************/
ULONG txcExecStdOutPipe
(
   char               *cmd                      // IN    command to execute
)
{
   ULONG               rc  = NO_ERROR;
   TXC_IO_HANDLES      ho;                      // handles stdout
   HFILE               wt;                      // temp write-handle
   TID                 tidout;                  // Thread-id stdout reader
   TXLN                command;

   ENTER();

   if ((rc = DosCreatePipe( &(ho.read),
                            &(ho.write),
                            TXC_PIPE_SIZE)) == NO_ERROR)
   {
      ho.save = TXC_NEW_HANDLE;
      if ((rc = DosDupHandle(  STDOUT, &(ho.save))) == NO_ERROR)
      {
         if (ho.write != STDOUT)                // unless we got right value
         {
            wt = STDOUT;                        // duplicate it to handle
            rc = DosDupHandle( ho.write, &wt);  // with correct value
            if (rc == NO_ERROR)
            {
               TxClose( ho.write);              // close original pipe wh
               ho.write = wt;
            }
         }
         if (rc == NO_ERROR)
         {
            tidout = TxBeginThread( txcPipeOutputPump, 16384, &ho);

            strcpy( command, cmd);
            strcat( command, " < NUL");
            if ((strstr(command, "2>") == NULL) &&
                (strstr(command, "&2") == NULL) )
            {                                   // let CMD.EXE combine stderr
               strcat( command, " 2>&1");       // with stdout, resulting in
            }                                   // better ordering of output
            TRACES(( "External command: '%s'\n", command));
            rc = (ULONG) system( command);

            fprintf( stdout, "%c", TXC_EOF);    // end stdout
            fflush(  stdout);

            DosWaitThread( &tidout, DCWW_WAIT); // wait for pump to complete

            wt = STDOUT;                        // reconnect stdout
            if (DosDupHandle( ho.save, &wt) == NO_ERROR)
            {
               TxClose( ho.save);               // close original save-handle
            }
         }
      }
      TxClose( ho.read);                        // close the pipe-handle
   }
   RETURN (rc);
}                                               // end 'txcExecStdOutPipe'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// TXC child-stdout thread, waiting for output by child-process; to TXPRINT
/*****************************************************************************/
void txcPipeOutputPump
(
   void               *p                        // IN    pointer parameter
)
{
   TXC_IO_HANDLES     *sh = (TXC_IO_HANDLES *) p;
   char                ch = 0;                  // temporary char
   ULONG               nr;                      // nr of chars read
   ULONG               i;
   TX1K                buf;

   ENTER();
   while (ch != TXC_EOF)
   {
      for ( i = 0, ch = '\0';
           (i < TXMAX1K -1) && (ch != TXC_EOF) && (ch != '\n');
          )
      {
         if ((TxRead( sh->read, &ch, 1, &nr) == NO_ERROR) && nr)
         {
            switch (ch)
            {
               case TXC_EOF:                    // end of stream
               case TXC_CR:
                  break;

               case TXC_LF:                     // end of line
                  ch = '\n';                    // fall through
               default:
                  buf[i++] = ch;
                  break;
            }
         }
         else
         {
            ch = TXC_EOF;                       // terminate on errors
         }
      }
      if (i > 0)                                // anything to redirect ?
      {
         buf[i]  = '\0';                        // terminate string
         TxWriteStream( sh->save, buf);
      }
   }
   VRETURN();
}                                               // end 'txcPipeOutputPump'
/*---------------------------------------------------------------------------                    */


/*****************************************************************************/
// Write to stream (redirected stdout) with logfile (TEE) and output hooks
/*****************************************************************************/
static void TxWriteStream
(
   TXHFILE             stream,                  // IN    stream to write to
   char               *text                     // IN    string to write
)
{
   ULONG               size = strlen(text);

   TxPrint( "%s", text);                        // output to logfile/hooks

   if (((TxScreenState(DEVICE_TEST)) == DEVICE_ON) &&  //- screen active
       (txwa->desktop == NULL))                        //- and not windowed!
   {                                                   //- direct to stdout
      if (text[size-1] == '\n')
      {
         text[--size]   = '\0';
         TxWrite( stream, text, size, &size);
         TxWrite( stream, "\r\n",  2, &size);
      }
      else
      {
         TxWrite( stream, text, size, &size);
      }
   }
}                                               // end 'TxWriteStream'
/*---------------------------------------------------------------------------*/
#endif
