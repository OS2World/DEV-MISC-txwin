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
// TxLib filesystem functions, file-exist section
//

#include <txlib.h>                              // TxLib interface

#include <sys/stat.h>                           // for low level stuff


/*****************************************************************************/
// Create empty file with specified path/name, prompt to replace existing
/*****************************************************************************/
ULONG TxCreateEmptyFile
(
   char               *fname,                   // IN    path and filename
   BOOL                prompt                   // IN    prompt on replace
)
{
   ULONG               rc = NO_ERROR;           // function return
   FILE               *fp;

   ENTER();
   TRACES(( "Fname:'%s'  prompt:%s\n", fname, (prompt) ? "YES" : "NO"));

   if (prompt && (TxFileExists( fname)))
   {
      if (!TxConfirm( 0, "File '%s' exists, replace ? [Y/N] : ", fname))
      {
         rc = TX_ABORTED;
      }
   }
   if (rc == NO_ERROR)
   {
      if ((fp = fopen( fname, "w")) != NULL)
      {
         fclose( fp);
      }
      else
      {
         rc = TX_ACCESS_DENIED;
      }
   }
   RETURN (rc);
}                                               // end 'TxCreateEmptyFile'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Open file for reading, supporting large files (native API's, TX handles)
/*****************************************************************************/
ULONG TxFileOpenRead                            // RET   file open result RC
(
   char               *fname,                   // IN    filename string
   TXHFILE            *fhandle                  // OUT   TX read file handle
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXHFILE             fh = 0;

   #if defined (DEV32)                          // test large file support APIs
      ULONG         action;                     // action taken on open
      TXF_OS2LFAPI  api;                        // large file API
   #endif

   ENTER();
   TRACES(( "Fname:'%s'", fname));

   #if defined (DEV32)                          // test large file support APIs
      if (TxLargeFileApiOS2( &api))
      {
         rc = (api.DosOpenLarge)( fname, &fh, &action, 0, FILE_NORMAL, FILE_OPEN,
                                  OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL);
      }
      else
      {
         rc = DosOpen( fname, &fh, &action, 0, FILE_NORMAL, FILE_OPEN,
                       OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL);
      }
   #elif defined (WIN32)
      fh = CreateFile( fname,
                       GENERIC_READ,            // read access only
                       FILE_SHARE_READ,         // share
                       NULL,                    // default security info
                       OPEN_EXISTING,           // do not create
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
      if (fh == INVALID_HANDLE_VALUE)
      {
         rc = ERROR_FILE_NOT_FOUND;
      }
   #elif defined (DOS32)
      fh = fopen( fname, "rb");                 // open binary read
      if (fh == 0)
      {
         rc = ERROR_FILE_NOT_FOUND;
      }
   #elif defined (UNIX)
      if ((fh = open( fname, O_RDONLY | O_LARGEFILE)) == -1)
      {
         fh = 0;
         rc = TxRcFromErrno( errno);
      }
   #else
      #error Unsupported OS environment
   #endif

   *fhandle = fh;

   TRACES(("TX handle: %lu\n", fh));
   RETURN( rc);
}                                               // end 'TxFileOpenRead'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test if exact (path+) filename is accessible; supports > 2GiB files
/*****************************************************************************/
BOOL TxFileExists                               // RET   file is accessible
(
   char               *fname                    // IN    filename string
)
{
   BOOL                rc = FALSE;              // function return
   TXHFILE             fh = 0;

   if (TxFileOpenRead( fname, &fh) == NO_ERROR)
   {
      (void) TxClose( fh);
      rc = TRUE;
   }
   return( rc);
}                                               // end 'TxFileExists'
/*---------------------------------------------------------------------------*/

