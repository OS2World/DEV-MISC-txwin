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
// On UNIX, it will also open a DIRECTORY! (unlike DOS, WIN and OS/2)
/*****************************************************************************/
ULONG TxFileOpenReadOnly                        // RET   file open result RC
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
      fh = CreateFile( fname, GENERIC_READ,                  //- read access only
                       FILE_SHARE_READ | FILE_SHARE_WRITE,   //- share read+write
                       NULL, OPEN_EXISTING,                  //- do not create
                       FILE_ATTRIBUTE_NORMAL, NULL);         //-
      if (fh == INVALID_HANDLE_VALUE)
      {
         TRACES(( "CreateFile error on '%s': %s\n", fname, txNtLastError()));
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
}                                               // end 'TxFileOpenReadOnly'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Open file for writing, supporting large files (native API's, TX handles)
/*****************************************************************************/
ULONG TxFileOpenForWrite                        // RET   file open result RC
(
   char               *fname,                   // IN    filename string
   BOOL                appendto,                // IN    append to file
   TXHFILE            *fhandle                  // OUT   TX read file handle
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXHFILE             fh = 0;

   #if   defined (WIN32)
   #elif defined (DOS32)
   #elif defined (UNIX)
      int              openflag = O_WRONLY | O_CREAT | O_LARGEFILE; // purpose NOW
      int              openmode = TX_DEFAULT_OPEN_MODE; // permissions AFTER
      #if defined (LINUX)
         LLONG         nsp;                     // llseek result
      #endif
   #else
      TXF_OS2LFAPI     api;                     // large file API
      ULONG            action;                  // action taken on open
      ULONG            openflag = FILE_CREATE | OPEN_ACTION_CREATE_IF_NEW;
      LONGLONG         nsp = 0;                 // new seek position
   #endif

   ENTER();
   TRACES(( "Append:%s Fname:'%s'", (appendto) ? "YES" : "NO", fname));

   #if defined (DEV32)                          // test large file support APIs
      if (appendto)
      {
         openflag |= OPEN_ACTION_OPEN_IF_EXISTS;
      }
      else
      {
         openflag |= FILE_TRUNCATE | OPEN_ACTION_REPLACE_IF_EXISTS;
      }
      if (TxLargeFileApiOS2( &api))
      {
         rc = (api.DosOpenLarge)( fname, &fh, &action, 0, FILE_NORMAL, openflag,
                                  OPEN_SHARE_DENYWRITE | OPEN_ACCESS_WRITEONLY, NULL);
         if ((rc == NO_ERROR) && (appendto))
         {
            rc = (api.DosSeekLarge)(fh, 0, FILE_END, &nsp);
         }
      }
      else
      {
         rc = DosOpen( fname, &fh, &action, 0, FILE_NORMAL, openflag,
                       OPEN_SHARE_DENYWRITE | OPEN_ACCESS_WRITEONLY, NULL);
         if ((rc == NO_ERROR) && (appendto))
         {
            rc = DosSetFilePtr( fh, 0, FILE_END, (ULONG *) &nsp);
         }
      }
   #elif defined (WIN32)
      fh = CreateFile( fname,
                       GENERIC_WRITE,           // write access only
                       0,                       // deny-write
                       NULL,                    // default security info
          (appendto) ? OPEN_EXISTING : CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
      if (fh == INVALID_HANDLE_VALUE)
      {
         TRACES(( "CreateFile error on '%s': %s\n", fname, txNtLastError()));
         rc = ERROR_OPEN_FAILED;
      }
      else if (appendto)
      {
         LONG          hi = 0;

         if (SetFilePointer( fh, 0, (PLONG) &hi, FILE_END) == -1)
         {
            TRACES(( "SetFilePointer error: %s\n", txNtLastError()));
            rc = GetLastError();
         }
      }
   #elif defined (DOS32)
      fh = fopen( fname, (appendto) ? "ab" : "wb");
      if (fh == 0)
      {
         rc = ERROR_OPEN_FAILED;
      }
   #elif defined (UNIX)
      if (!appendto)
      {
         openflag |= O_TRUNC;
      }
      if ((fh = open( fname, openflag, openmode)) == -1)
      {
         fh = 0;
         rc = TxRcFromErrno( errno);
      }
      else if (appendto)
      {
         #if defined (DARWIN)
            if ( lseek( fh, 0LL, SEEK_END) == -1LL)
            {
               rc = TxRcFromErrno( errno);
            }
         #else
            if ( _llseek( fh, 0, 0, &nsp, SEEK_END) == -1)
            {
               rc = TxRcFromErrno( errno);
            }
         #endif
      }
   #else
      #error Unsupported OS environment
   #endif

   *fhandle = fh;

   TRACES(("TX handle: %lu\n", fh));
   RETURN( rc);
}                                               // end 'TxFileOpenForWrite'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Close file, compatible with TxFileOpen... and  native API's, TX handles
/*****************************************************************************/
ULONG TxClose                                   // RET   file close result RC
(
   TXHFILE             fh                       // IN    TX file handle
)
{
   ULONG               rc = NO_ERROR;           // function return

   TRACES(("TxClose handle: %lu\n", fh));

   #if defined (DEV32)                          // test large file support APIs
      rc = DosClose( fh);
   #elif defined (WIN32)
      rc = (ULONG) !CloseHandle( fh);
   #elif defined (DOS32)
      rc = fclose( fh);
   #elif defined (UNIX)
      if (close( fh) == -1)
      {
         rc = TxRcFromErrno( errno);
      }
   #else
      #error Unsupported OS environment
   #endif

   return( rc);
}                                               // end 'TxClose'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test if exact (path+) filename is accessible; supports > 2GiB files
// Will return FALSE when fname represents an existing DIRECTORY (even on UNIX)
/*****************************************************************************/
BOOL TxFileExists                               // RET   file is accessible
(
   char               *fname                    // IN    filename string
)
{
   BOOL                rc = FALSE;              // function return
   TXHFILE             fh = 0;
   #if defined (UNIX)
      int              stat_rc;
      USHORT           st_mode;
      struct stat      f;
      #if !defined (DARWIN)
         struct stat64 f64;
      #endif
   #endif

   ENTER();

   if (TxFileOpenReadOnly( fname, &fh) == NO_ERROR)
   {
      (void) TxClose( fh);
      rc = TRUE;

      //- avoid directories to pass the EXIST test (can be opened like files on UNIX)
      #if defined (UNIX)
         if ((stat_rc = stat( fname, &f)) != -1)
         {
            st_mode = (USHORT) f.st_mode;
         }
       #if !defined (DARWIN)
         else if ((errno == EFBIG) ||          // must be a large-file
                  (errno == EOVERFLOW))
         {
            //- retry with 64-bit stat (latest Linux kernels fail on regular)
            if ((stat_rc = stat64( fname, &f64)) != -1)
            {
               st_mode   = (USHORT) f64.st_mode;
            }
         }
       #endif
         TRACES(("stat_rc: %d  st_mode: %4.4hX\n", stat_rc, st_mode));
         if (stat_rc != -1)                     // name exists
         {
            if (S_ISDIR( st_mode) != 0)         // but is a directory ?
            {
               TRACES(("Represents a DIRECTORY, not a regular file!\n"));
               rc = FALSE;                      // name is an existing directory
            }
         }
      #endif
   }
   RETURN( rc);
}                                               // end 'TxFileExists'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test if exact (path+) filename is accessible; determine size > 2GiB files
/*****************************************************************************/
BOOL TxFileSize                                 // RET   file exists
(
   char               *fname,                   // IN    filename string
   ULN64              *size                     // OUT   filesize or NULL
)
{
   BOOL                rc = FALSE;              // function return
   TXHFILE             fhandle;

   if (TxFileOpenReadOnly( fname, &fhandle) == NO_ERROR)
   {
      if (size)
      {
         TxHandle2FileSize( fhandle, size);
      }
      TxClose( fhandle);
      rc = TRUE;
   }
   return( rc);
}                                               // end 'TxFileSize'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get size of an already OPEN file; supports > 2GiB files; modifies file-pos!
/*****************************************************************************/
void TxHandle2FileSize
(
   TXHFILE             fh,                      // IN    TX file handle
   ULN64              *size                     // OUT   filesize or NULL
)
{
   #if   defined (DEV32)
      ULONG            ulSize;
      TXF_OS2LFAPI     api;                     // large file API
   #elif defined (WIN32)
      ULONG            hi = 0;                  // hi part of size
      ULONG            lo = 0;                  // lo part of size
   #elif defined (UNIX)
   #else
   #endif

   if (size)                                    // avoid traps when NULL
   {
      *size  = 0;                               // initialize to zero size

      #if   defined (DEV32)
         if (TxLargeFileApiOS2( &api) && api.DosSeekLarge)
         {
            (api.DosSeekLarge)( fh, 0, FILE_END, (LLONG *) size);
         }
         else
         {
            DosSetFilePtr( fh, 0, FILE_END, &ulSize);
            *size = (ULN64) ulSize;
         }
      #elif defined (WIN32)
         lo    = GetFileSize( fh, &hi);
         *size = (((ULN64) hi) << 32) + lo;
      #elif defined (LINUX)
          _llseek( fh, 0, 0, (LLONG *) size, SEEK_END);
      #elif defined (DARWIN)
          *size = (ULN64) lseek( fh, 0, SEEK_END);
      #else
         fseek( fh, 0, SEEK_END);
         *size = (ULN64) ftell( fh);
      #endif
      TRACES(( "TxHandle2FileSize: %16.16llX = %llu bytes\n", *size, *size));
   }
}                                               // end 'TxHandle2FileSize'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Seek to specified position in open file (platform specific fseek)
/*****************************************************************************/
ULONG TxFileSeek
(
   TXHFILE             fh,                      // IN    file handle
   ULN64               offset,                  // IN    seek offset
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

   TRACES(("TxFileSeek: whence:%d  offset: 0x%16.16llx\n", whence, offset));

   #if   defined (DEV32)
      if (TxLargeFileApiOS2( &api) && api.DosSeekLarge)
      {
         (api.DosSeekLarge)(    fh, (LLONG) offset, (ULONG) whence, &current);
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
         TRACES(("SetFilePointer rc: %lu, %s\n", rc, txNtLastError()));
      }
   #elif defined (LINUX)
      if ( _llseek( fh, (offset >> 32), (offset & 0xffffffff), &ll, whence) == -1)
      {
         rc = TxRcFromErrno( errno);
      }
   #elif defined (DARWIN)
      if (lseek( fh, (LLONG) offset, whence) == -1)
      {
         rc = TxRcFromErrno( errno);
      }
   #else
      fseek( fh, (LONG) offset, whence);
   #endif
   return (rc);
}                                               // end 'TxFileSeek'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set size of a file to the specified number of bytes
/*****************************************************************************/
ULONG TxSetFileSize
(
   TXHFILE             fh,                      // IN    file handle
   ULN64               size                     // IN    filesize to set
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
         rc = (api.DosSetFileSizeLarge)( fh, (LLONG) size);
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
           TRACES(("SetEndOfFile rc: %lu, %s\n", rc, txNtLastError()));
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
      struct utimbuf   am_dt;
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
      am_dt.actime  = *access;
      am_dt.modtime = *modify;

      if ((utime( fname, &am_dt)) == -1)
      {
         rc = TxRcFromErrno( errno);
      }
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


/*****************************************************************************/
// Get File timestamp(s) to specified values from specified filename
/*****************************************************************************/
ULONG TxGetFileTime                             // RET   result
(
   char               *fname,                   // IN    filename string
   time_t             *create,                  // OUT   create time or NULL
   time_t             *access,                  // OUT   access time or NULL
   time_t             *modify                   // OUT   modify time or NULL
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
      int              stat_rc;
      struct stat      f;
      #if !defined (DARWIN)
         struct stat64 f64;
      #endif
   #else
      FILE            *fh;
      unsigned        udate;
      unsigned        utime;
      USHORT          fdate;
      USHORT          ftime;
   #endif

   ENTER();
   #if defined (DEV32)
      if (TxLargeFileApiOS2( NULL))             // large file support there ?
      {
         if ((rc = DosQueryPathInfo( fname, FIL_STANDARDL,  &fs3l, sizeof(fs3l))) == NO_ERROR)
         {
            if (create)
            {
               *create = txOS2FileTime2t( (USHORT *) &fs3l.fdateCreation,
                                          (USHORT *) &fs3l.ftimeCreation);
            }
            if (access)
            {
               *access = txOS2FileTime2t( (USHORT *) &fs3l.fdateLastAccess,
                                          (USHORT *) &fs3l.ftimeLastAccess);
            }
            if (modify)
            {
               *modify = txOS2FileTime2t( (USHORT *) &fs3l.fdateLastWrite,
                                          (USHORT *) &fs3l.ftimeLastWrite);
            }
         }
      }
      else
      {
         if ((rc = DosQueryPathInfo( fname, FIL_STANDARD,   &fs3, sizeof(fs3))) == NO_ERROR)
         {
            if (create)
            {
               *create = txOS2FileTime2t( (USHORT *) &fs3.fdateCreation,
                                          (USHORT *) &fs3.ftimeCreation);
            }
            if (access)
            {
               *access = txOS2FileTime2t( (USHORT *) &fs3.fdateLastAccess,
                                          (USHORT *) &fs3.ftimeLastAccess);
            }
            if (modify)
            {
               *modify = txOS2FileTime2t( (USHORT *) &fs3.fdateLastWrite,
                                          (USHORT *) &fs3.ftimeLastWrite);
            }
         }
      }
   #elif defined (WIN32)
      fh = CreateFile( fname,
                       GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (fh != 0)
      {
         if (GetFileTime( fh, &cre, &acc, &mod))
         {
            if (create)
            {
               *create = txWinFileTime2t( (NTIME *) &cre, 0);
            }
            if (access)
            {
               *access = txWinFileTime2t( (NTIME *) &acc, 0);
            }
            if (modify)
            {
               *modify = txWinFileTime2t( (NTIME *) &mod, 0);
            }
         }
         (void) TxClose( fh);
      }
      else
      {
         rc = TX_INVALID_PATH;
      }
   #elif defined (UNIX)
      if ((stat_rc = stat( fname, &f)) != -1)
      {
         if (create)
         {
            *create = f.st_ctime;
         }
         if (access)
         {
            *access = f.st_atime;
         }
         if (modify)
         {
            *modify = f.st_mtime;
         }
      }
    #if !defined (DARWIN)
      else if ((errno == EFBIG) ||                 // must be a large-file
               (errno == EOVERFLOW))
      {
         //- retry with 64-bit stat (latest Linux kernels fail on regular)
         if ((stat_rc = stat64( fname, &f64)) != -1)
         {
            if (create)
            {
               *create = f64.st_ctime;
            }
            if (access)
            {
               *access = f64.st_atime;
            }
            if (modify)
            {
               *modify = f64.st_mtime;
            }
         }
      }
    #endif
      else
      {
         rc = TxRcFromErrno( errno);
      }
   #else
      if (TxFileOpenReadOnly( fname, &fh) == NO_ERROR)
      {
         _dos_getftime( fileno( fh), &udate, &utime);
         fdate = (USHORT) udate;
         ftime = (USHORT) utime;
         if (modify)
         {
            *modify = txOS2FileTime2t( &fdate, &ftime);
         }
         if (access)
         {
            *access = txOS2FileTime2t( &fdate, &ftime);
         }
         if (create)
         {
            *create = txOS2FileTime2t( &fdate, &ftime);
         }
         (void) TxClose( fh);
      }
      else
      {
         rc = TX_INVALID_PATH;
      }
   #endif

   TRACES(( "C:%8.8lx A:%8.8lx M:%8.8lx fname:'%s'\n",
          (create) ? *create : 0, (access) ? *access : 0, (modify) ? *modify : 0, fname));

   RETURN( rc);
}                                               // end 'TxGetFileTime'
/*---------------------------------------------------------------------------*/

