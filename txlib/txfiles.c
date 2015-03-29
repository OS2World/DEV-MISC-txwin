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
// TxLib file functions, generic Seek and determine filesize
//

#include <txlib.h>                              // TxLib interface

#include <sys/stat.h>                           // for low level stuff


/*****************************************************************************/
// Test if exact (path+) filename is accessible; determine size > 2GiB files
/*****************************************************************************/
BOOL TxFileSize                                 // RET   file exists
(
   char               *fname,                   // IN    filename string
   LLONG              *size                     // OUT   filesize or NULL
)
{
   BOOL                rc = FALSE;              // function return
   TXHFILE             fh = 0;
   #if   defined (DEV32)
      ULONG            action;                  // action taken on open
      ULONG            ulSize;
      TXF_OS2LFAPI     api;                     // large file API
   #elif defined (WIN32)
      ULONG            hi = 0;                  // hi part of size
      ULONG            lo = 0;                  // lo part of size
   #elif defined (UNIX)
   #else
   #endif

   ENTER();

   #if defined (DEV32)                          // test large file support APIs
      if (TxLargeFileApiOS2( &api) && api.DosOpenLarge)
      {
         (api.DosOpenLarge)( fname, &fh, &action, 0, FILE_NORMAL, FILE_OPEN,
                             OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL);
      }
      else
      {
         DosOpen( fname, &fh, &action, 0, FILE_NORMAL, FILE_OPEN,
                  OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL);
      }
   #elif defined (WIN32)
      fh = CreateFile( fname, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   #elif defined (UNIX)
      fh = open( fname, O_RDONLY | O_LARGEFILE);
   #else
      fh = fopen( fname, "r");
   #endif

   if ((fh != 0) && (fh != ((TXHFILE) -1)))
   {
      rc = TRUE;

      if (size != NULL)
      {
         *size  = 0;                            // initialize to zero size

         #if   defined (DEV32)
            if (TxLargeFileApiOS2( &api) && api.DosSeekLarge)
            {
               (api.DosSeekLarge)( fh, 0, FILE_END, size);
            }
            else
            {
               DosSetFilePtr( fh, 0, FILE_END, &ulSize);
               *size = (LLONG) ulSize;
            }
         #elif defined (WIN32)
            lo    = GetFileSize( fh, &hi);
            *size = (((LLONG) hi) << 32) + lo;

            TxPrint( "TxFileSize:  hi = %8.8lx lo = %8.8lx\n", hi, lo);
            TxPrint( "Size: %16.16llX = %lld bytes\n", *size, *size);
         #elif defined (LINUX)
             _llseek( fh, 0, 0, size, SEEK_END);
         #elif defined (DARWIN)
             *size = lseek( fh, 0, SEEK_END);
         #else
            fseek( fh, 0, SEEK_END);
            *size = (LLONG) ftell( fh);
         #endif
         TRACES(( "Size: %16.16llX = %lld bytes\n", *size, *size));
      }
      (void) TxClose( fh);
   }
   BRETURN( rc);
}                                               // end 'TxFileSize'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Seek to specified position in open file (platform specific fseek)
/*****************************************************************************/
ULONG TxFileSeek
(
   TXHFILE             fh,                      // IN    file handle
   LLONG               offset,                  // IN    seek offset
   int                 whence                   // IN    seek reference
)
{
   ULONG               rc = NO_ERROR;           // function return
   #if   defined (DEV32)
      TXF_OS2LFAPI     api;                     // large file API
      LLONG            current;
   #elif defined (WIN32)
      ULONG            hi = 0;                  // High part of seek offset
      ULONG            lo = 0;                  // Low  part of seek offset
   #elif defined (LINUX)
      LLONG            ll;
   #else
   #endif

   ENTER();

   #if   defined (DEV32)
      if (TxLargeFileApiOS2( &api) && api.DosSeekLarge)
      {
         (api.DosSeekLarge)(    fh, offset, (ULONG) whence, &current);
      }
      else
      {
         DosSetFilePtr( fh, (ULONG) offset, (ULONG) whence,  (ULONG *) &current);
      }
   #elif defined (WIN32)
      lo = offset & 0xffffffff;                 // Note: no ...PointerEx() use
      hi = offset >> 32;                        // to allow running on NT4!

      if (SetFilePointer( fh, lo, (PLONG) &hi, (DWORD) whence) == 0xffffffff)
      {
         rc = GetLastError();
      }
   #elif defined (LINUX)
      if ( _llseek( fh, (offset >> 32), (offset & 0xffffffff), &ll, whence) == -1)
      {
         rc = TxRcFromErrno( errno);
      }
   #elif defined (DARWIN)
      if (lseek( fh, offset, whence) == -1)
      {
         rc = TxRcFromErrno( errno);
      }
   #else
      fseek( fh, (LONG) offset, whence);
   #endif
   RETURN (rc);
}                                               // end 'TxFileSeek'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set size of a file to the specified number of bytes
/*****************************************************************************/
ULONG TxSetFileSize
(
   TXHFILE             fh,                      // IN    file handle
   LLONG               size                     // IN    filesize to set
)
{
   ULONG               rc = NO_ERROR;           // function return
   #if   defined (DEV32)
      TXF_OS2LFAPI     api;                     // large file API
   #elif defined (WIN32)
   #elif defined (UNIX)
   #else
   #endif

   ENTER();
   TRACES(( "handle: %8.8lx, size: %llu\n", fh, size));

   #if   defined (DEV32)
      if (TxLargeFileApiOS2( &api) && api.DosSetFileSizeLarge)
      {
         rc = (api.DosSetFileSizeLarge)( fh, size);
      }
      else
      {
         if (size < 0x8000000)                  // within 2Gb
         {
            rc = DosSetFileSize( fh, (ULONG) size);
         }
         else
         {
            rc = TX_INVALID_DATA;
         }
      }
   #elif defined (WIN32)
     if (TxFileSeek( fh, size, SEEK_SET) == NO_ERROR)
     {
        if (!SetEndOfFile( fh))
        {
           rc = GetLastError();
        }
     }
   #elif defined (UNIX)
     //- to be refined, may use systemcall 194 (ftruncate64)
     //- which is NOT in OpenWatcom yet
     if (size < 0x8000000)                      // within 2Gb
     {
        rc = ftruncate( fh, (off_t) size);
     }
     else
     {
        rc = TX_INVALID_DATA;
     }
   #else
     if (size < 0x8000000)                      // within 2Gb
     {
        rc = chsize( fileno( fh), (long) size);
     }
     else
     {
        rc = TX_INVALID_DATA;
     }
   #endif
   RETURN (rc);
}                                               // end 'TxSetFileSize'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set File timestamp(s) to specified values
/*****************************************************************************/
ULONG TxSetFileTime                             // RET   result
(
   char               *fname,                   // IN    filename string
   time_t             *create,                  // IN    create time or NULL
   time_t             *access,                  // IN    access time or NULL
   time_t             *modify                   // IN    modify time or NULL
)
{
   ULONG               rc = NO_ERROR;           // function return
   #if   defined (DEV32)
      FILESTATUS3      fs3;
      FILESTATUS3L     fs3l;
   #elif defined (WIN32)
      TXHFILE          fh;
      FILETIME         cre;
      FILETIME         acc;
      FILETIME         mod;
   #elif defined (UNIX)
   #else
      FILE            *fh;
      unsigned        udate;
      unsigned        utime;
      USHORT          fdate;
      USHORT          ftime;
   #endif

   ENTER();
   TRACES(( "C:%8.8lx A:%8.8lx M:%8.8lx fname:'%s'\n",
          (create) ? *create : 0, (access) ? *access : 0, (modify) ? *modify : 0, fname));

   #if defined (DEV32)
      if (TxLargeFileApiOS2( NULL))             // large file support there ?
      {
         if ((rc = DosQueryPathInfo( fname, FIL_STANDARDL,  &fs3l, sizeof(fs3l))) == NO_ERROR)
         {
            if (create)
            {
               txCt2OS2FileTime( *create,   (USHORT *) &fs3l.fdateCreation,
                                            (USHORT *) &fs3l.ftimeCreation);
            }
            if (access)
            {
               txCt2OS2FileTime( *access,   (USHORT *) &fs3l.fdateLastAccess,
                                            (USHORT *) &fs3l.ftimeLastAccess);
            }
            if (modify)
            {
               txCt2OS2FileTime( *modify,   (USHORT *) &fs3l.fdateLastWrite,
                                            (USHORT *) &fs3l.ftimeLastWrite);
            }
            rc = DosSetPathInfo( fname, FIL_STANDARDL, &fs3l, sizeof(fs3l), 0);
         }
      }
      else
      {
         if ((rc = DosQueryPathInfo( fname, FIL_STANDARD,   &fs3, sizeof(fs3))) == NO_ERROR)
         {
            if (create)
            {
               txCt2OS2FileTime( *create,   (USHORT *) &fs3.fdateCreation,
                                            (USHORT *) &fs3.ftimeCreation);
            }
            if (access)
            {
               txCt2OS2FileTime( *access,   (USHORT *) &fs3.fdateLastAccess,
                                            (USHORT *) &fs3.ftimeLastAccess);
            }
            if (modify)
            {
               txCt2OS2FileTime( *modify,   (USHORT *) &fs3.fdateLastWrite,
                                            (USHORT *) &fs3.ftimeLastWrite);
            }
            rc = DosSetPathInfo( fname, FIL_STANDARD,  &fs3, sizeof(fs3), 0);
         }
      }
   #elif defined (WIN32)
      fh = CreateFile( fname,
                       GENERIC_READ    | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (fh != 0)
      {
         if (GetFileTime( fh, &cre, &acc, &mod))
         {
            if (create)
            {
               txCt2WinFileTime( *create, (NTIME *) &cre, 0);
            }
            if (access)
            {
               txCt2WinFileTime( *access, (NTIME *) &acc, 0);
            }
            if (modify)
            {
               txCt2WinFileTime( *modify, (NTIME *) &mod, 0);
            }
            if (!SetFileTime( fh, &cre, &acc, &mod))
            {
               rc = TX_INVALID_FILE;
            }
         }
         (void) TxClose( fh);
      }
      else
      {
         rc = TX_INVALID_PATH;
      }
   #elif defined (UNIX)
      //- to be refined
      //- use stat() to get the info
      //- but how set it ???
   #else
      if ((fh = fopen( fname, "w")) != NULL)
      {
         //-   _dos_getftime( fileno( fh), &fdate, &ftime);
         _dos_getftime( fileno( fh), &udate, &utime);
         fdate = (USHORT) udate;
         ftime = (USHORT) utime;
         if (modify)
         {
            txCt2OS2FileTime( *modify, &fdate, &ftime);
         }
         else if (access)
         {
            txCt2OS2FileTime( *access, &fdate, &ftime);
         }
         else if (create)
         {
            txCt2OS2FileTime( *create, &fdate, &ftime);
         }
         if (_dos_setftime( fileno( fh),  fdate,  ftime) != 0)
         {
            rc = TX_INVALID_FILE;
         }
         (void) TxClose( fh);
      }
      else
      {
         rc = TX_INVALID_PATH;
      }
   #endif

   RETURN( rc);
}                                               // end 'TxSetFileTime'
/*---------------------------------------------------------------------------*/

