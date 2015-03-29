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
// file-logging facilities
//
// Author: J. van Wijk
//
// JvW  26-12-96   Split-off from ansilog

#include <txlib.h>

static  FILE      *log_handle  = 0;
static  TXLN       log_fname   = "logfile";
static  BOOL       log_7bit    = TRUE;
static  BOOL       log_reopen  = FALSE;         // reopen on each line

/*****************************************************************************/
// Close existing and if specified, open new logfile
// Can NOT have any internal tracing! Used in TxPrint to scrollbuffer stream!
/*****************************************************************************/
BOOL TxAppendToLogFile                          // RET   logfile opened
(
   char               *fname,                   // IN    name of (new) logfile
   BOOL                verbose                  // IN    Show the action
)
{
   BOOL                fExist = TRUE;           // logfile exists (will append)

   if (log_handle != 0)                         // close old one
   {
      if (verbose)
      {
         TxPrint("\nClosing logfile   : '%s'\n", log_fname);
      }
      fclose(log_handle);
      log_handle = 0;
   }
   if (fname && strlen(fname))
   {
      if (strcmp( fname, ".") != 0)             // not 'current one' ?
      {
         fExist = TxAutoNumberedFname( fname, "log", log_fname);
      }
      if (((log_handle = fopen(log_fname, "a" TXFMODE)) == 0) && (verbose))
      {
         TxPrint("Error opening log : '%s'\n", log_fname);
      }
      else
      {
         TxLogfileState( DEVICE_ON);            // always ON for new file
         if (verbose)                           // avoid any TxPrint when
         {                                      // working in quiet mode
            if (TxaOptUnSet('7'))               // NO strip to 7-bit ASCII
            {
               log_7bit = FALSE;
            }
            if (TxaExeArgc() != 0)              // Switches already parsed ?
            {                                   // (-q = quiet switch to come)
               TxPrint("%s log  : '%s' (%s-bit ASCII)\n",
                         (fExist) ? "Appending to" : "Creating new",
                         log_fname, (log_7bit) ? "7" : "8");
            }
         }
      }
   }
   return( log_handle != 0);
}                                               // end 'TxAppendToLogFile'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set log close/reopen on each TxPrint on or off
/*****************************************************************************/
void TxSetLogReOpen
(
   BOOL                reopen                   // IN    log reopen on
)
{
   log_reopen = reopen;
}                                               // end 'TxSetLogReOpen'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Close existing and reopen (implementing a brute-force flush)
/*****************************************************************************/
void TxCloseReopenLogFile
(
   void
)
{
   static BOOL         warn = TRUE;

   if (log_handle != 0)
   {
      fclose(log_handle);
      if ((log_handle = fopen(log_fname, "a" TXFMODE)) == 0)
      {
         if (warn)
         {
            TxPrint("\nError re-opening  : '%s'\n", log_fname);
            warn = FALSE;                       // I will say this only once!
         }
      }
   }
}                                               // end 'TxCloseReopenLogFile'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Signal logfile active
/*****************************************************************************/
FILE *TxQueryLogFile                            // RET   logfile active
(
   BOOL               *ascii7bit,               // OUT   Use 7-bit ASCII only
   BOOL               *reopenlog                // OUT   reopen logfile
)                                               //       NULL if not wanted
{
   if (ascii7bit != NULL)
   {
      *ascii7bit  = log_7bit;
   }
   if (reopenlog != NULL)
   {
      *reopenlog  = log_reopen;
   }
   return( log_handle);
}                                               // end 'TxQueryLogFile'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get name for current logfile, if any
/*****************************************************************************/
char *TxQueryLogName                            // RET   filename or NULL
(
   void
)                                               //       (NULL if not wanted)
{
   char               *rc = NULL;

   if (log_handle != 0)
   {
      rc = log_fname;
   }
   return (rc);
}                                               // end 'TxQueryLogName'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Construct name for new logfile based upon a sequence number 0..n
/*****************************************************************************/
char *TxBuildLogName                            // RET   filename or NULL
(
   ULONG               seq,                     // IN    sequence number
   ULONG               retain,                  // IN    nr of files kept
   TXLN                buf                      // IN    filename buffer
)
{
   char               *rc = buf;
   TXTS                ext;

   if (seq == 0)                                // first will be .log
   {
      strcpy( ext, "log");
   }
   else
   {
      if ((seq >= 100) && (retain < 100))       // OK to use modulo seq ?
      {
         seq %= 100;                            // reduce to two digits, to
      }                                         // limit extension to three
      sprintf( ext, "l%02lu", seq);
   }
   strcpy( rc, log_fname);

   TxStripExtension( rc);                       // remove current extension
   TxFnameExtension( rc, ext);                  // and add new one

   return (rc);
}                                               // end 'TxBuildLogName'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Substitute first unused auto-nr for last # in filename-spec; add extension
/*****************************************************************************/
BOOL TxAutoNumberedFname                        // RET   TRUE when not unique
(
   char               *spec,                    // IN    (path +) file specification
   char               *ext,                     // IN    file extension to be used
   char               *anName                   // OUT   auto-numbered name
)
{
   BOOL                rc = TRUE;               // function return
   ULONG               fnum;
   int                 blen;                    // basename length

   if ((blen = strlen(spec)) != 0)
   {
      blen--;
   }
   if (spec[ blen] == '^')                      // fixed 3-digit filename postfix
   {
      for (fnum = 1; fnum <= 1000; fnum++)
      {
         sprintf( anName, "%*.*s%03lu.%s", blen, blen, spec, (fnum % 1000), ext);
         if (!TxFileExists( anName))            // numbered file does not exist yet
         {
            rc = FALSE;                         // found one that does NOT exist
            break;                              // last checked will be 000
         }
      }
   }
   else
   {
      strcpy(            anName, spec);         // default, supplied name as is
      TxFnameExtension(  anName, ext);
      rc = TxFileExists( anName);               // resulting filename not unique
   }
   return (rc);
}                                               // end 'TxAutoNumberedFname'
/*---------------------------------------------------------------------------*/

