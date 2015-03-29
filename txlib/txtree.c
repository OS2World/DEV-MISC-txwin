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
// Recursive findfile with call-back function to operate on found files
//
// Author: J. van Wijk
//
// Developed for LPT file transfer utility, updated for TxWin File dialog
//
// Modelled after SysFileTree implementation from REXXUTIL
// Major functional changes made:
//
//   - REXX interfaces (stem, variablepool) changed to normal 'C'
//   - Action on each matching file implemented as a call-back function
//     with the full "true-filename" and a FILEFINDBUF4 structure as parameters
//   - Source and Destination mask use different syntax (+,-,* and ARSHD)
//
//     options  - Any combo of the following:
//                 'B' - Search for files and directories. (default)
//                 'D' - Search for directories only.
//                 'F' - Search for files only.
//                 'S' - Recursively scan subdirectories.
//
// Includes the lower-level TxFindFirstFile() and TxFindNextFile() APIs too
// with an associated TX specific (OS-neutral) TXFILEFIND structure
//
// Also includes a wrapper to deliver a result-set in a TXSELIST structure

#include <txlib.h>


#if defined (USEWINDOWING)
// Define sort information for file and directory lists
static TXSELSORT       txfd_file_sort =
{
  {                                             // actual, initial sort order
     #if defined (UNIX)
        TXS_SORT_1,                         // ascending case-sensitive fname
     #else
        TXS_SORT_6,                         // ascending case-insensitive name
     #endif
     TXS_SORT_TEXT | TXS_SORT_ON_CASE | TXS_SORT_ASCEND  |  1,  // (fname)
     TXS_SORT_TEXT | TXS_SORT_ON_CASE | TXS_SORT_LASTDOT,       // (f-ext)
     0,
     TXS_SORT_DESC | TXS_SORT_ON_CASE | TXS_SORT_DESCEND |  6,  // (date)
     TXS_SORT_DESC | TXS_SORT_ON_CASE | TXS_SORT_DESCEND | 26,  // (size)
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_ASCEND  | 42,  // (name)
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_LASTDOT        // (ext)
  },
  {                                             // reverse   (c-F8 / c-R)
     "none -",                                  // unsorted  (c-F7 / c-U)
     "name -",                                  // text   1  (c-F2 / c-X)
     ".ext -",                                  // text   2  (c-F1 / c-D)
     "",                                        // text   3  (c-F9 / c-F / c-A)
     "Time -",                                  // desc   4  (c-F5 / c-T)
     "Size -",                                  // desc   5  (c-F6 / c-B / c-S)
     "Name -",                                  // desc   6  (c-F3 / c-N)
     ".Ext -"                                   // desc   7  (c-F4 / c-E)
  },
  ""                                            // current description
};

static TXSELSORT       txfd_dir_sort =
{
  {                                             // actual, initial sort order
     #if defined (UNIX)
        TXS_SORT_1,                             // text ascending case-sensitive fname
     #else
        TXS_SORT_6,                             // desc ascending case-insensitive name
     #endif
     TXS_SORT_TEXT | TXS_SORT_ON_CASE | TXS_SORT_ASCEND  |  1,  // (fname)
     TXS_SORT_TEXT | TXS_SORT_ON_CASE | TXS_SORT_LASTDOT,       // (f-ext)
     0,
     TXS_SORT_DESC | TXS_SORT_ON_CASE | TXS_SORT_DESCEND |  6,  // (date)
     0,
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_ASCEND  | 42,  // (name)
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_LASTDOT        // (ext)
  },
  {                                             // reverse   (c-F8 / c-R)
     "none ",                                  // unsorted  (c-F7 / c-U)
     "name ",                                  // text   1  (c-F2 / c-X)
     ".ext ",                                  // text   2  (c-F1 / c-D)
     "",                                        // text   3  (c-F9 / c-F / c-A)
     "Time ",                                  // desc   4  (c-F5 / c-T)
     "",                                        // desc   5  (c-F6 / c-B / c-S)
     "Name ",                                  // desc   6  (c-F3 / c-N)
     ".Ext "                                   // desc   7  (c-F4 / c-E)
  },
  ""                                            // current description
};
#endif

#if defined (UNIX)
   #include <dirent.h>
   #include <sys/stat.h>

typedef struct txlin_ffinfo
{
   DIR             *dir;                        // opendir/readdir handle
   TXLN             dirname;                    // original directory path
   TXLN             wildcard;                   // filename part of fspec
} TXLIN_FFINFO;                                 // end of struct "txlin_ffinfo"


// Find next file matching the specified (wildcard) name
static ULONG TxUnixNextFile                     // RET   result
(
   TXLIN_FFINFO       *ffi,                     // IN    TX filefind handle
   TXFILEFIND         *found                    // OUT   found file details
);


#endif


/*********************************************************************/
/* Defines uses by TxTree                                            */
/*********************************************************************/

#define  FIRST_TIME     0x0001
#define  RECURSE        0x0002
#define  DO_DIRS        0x0004
#define  DO_FILES       0x0008
#define  INCLUDESPEC    0x0010
#define  INCL_DOTDOT    0x0020                  // keep the .. directory

#define  TX_ALL_ATTRS   FATTR_READONLY  | FATTR_HIDDEN    | \
                        FATTR_SYSTEM    | FATTR_DIRECTORY | FATTR_ARCHIVED

typedef struct TxTreeData                       // Tree data structure
{
   TXLN                TargetSpec;              // Filespec FlTree searches
   TXLN                truefile;                // actual file name
   TXLN                tempfile;                // temp file name
   TXLN                Temp;                    // Used when building data
   char              **filter;                  // Include/Exclude file-specs
   ULONG               nattrib;                 // New attrib, diff for each
} TXTREEDATA;


// Split path and filename part of a full-filespec
static void txGetPath
(
   char               *string,                  // IN    full filespec
   char               *path,                    // OUT   path component, incl SEP
   char               *filename                 // OUT   filename component
);                                              //       or NULL if not wanted


// Compare two sets of file-attributes in REXXXUtil format
static BOOL txSameAttr                          // RET   attrs are  same
(
   char               *mask,                    // IN    1st set, as string
   ULONG               attr                     // IN    2nd set as bitmask
);


// Combine two sets of file-attributes in REXXXUtil format
static ULONG txNewAttr
(
   char               *mask,
   ULONG               attr
);


// Change file attribute bits
static ULONG  txSetFileMode
(
   char               *file,                    // file name
   ULONG               attr                     // new file attributes
);

static ULONG txRecursiveFindFile                // RET   result error code
(
   char               *FileSpec,                // IN    Filespecs to search
   TXTREEDATA         *ldp,                     // INOUT local data
   char               *smask,                   // IN    select attributes
   char               *dmask,                   // IN    modify attributes
   ULONG               options,                 // IN    search options
   TXTREE_CALLBACK     callback,                // IN    function to call back
   void               *calldata                 // IN    callback data
);


#if defined (USEWINDOWING)
// Add file/directory instance to the Selist being built
static ULONG txFileDirAdd2List
(
   char               *fname,                   // IN    full filename
   TXFILEFIND         *finfo,                   // IN    file information
   void               *cdata                    // IN    Callback data
);

/*****************************************************************************/
// Build selection-list with File/Directory/Tree information
/*****************************************************************************/
TXSELIST *TxFileDirSelist                       // RET   selection list or NULL
(                                               //       Tree RC is in userinfo
   char               *fspec,                   // IN    Filespec to match
   char               *fopts,                   // IN    Find options
   char               *fattr,                   // IN    Select attributes
   char              **filter,                  // IN    Excl/Incl file-specs
   ULONG               ffw                      // IN    flags + filewindow width
)
{
   TXSELIST           *list  = NULL;            // total list
   ULONG               size  = 500;             // initial allocated size

   ENTER();

   TRACES(( "spec:'%s' opts:'%s' attr:'%s' flt:%8.8lx flg:%8.8lx\n",
             fspec,    fopts,    fattr,    filter,    ffw));

   TxCancelAbort();                             // possible previous ESC ...
                                                // FileTree would abort on it
   if (TxSelCreate( size, size, size,
                    TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
   {
      list->astatus  = TXS_AS_NOSTATIC;        // all dynamic allocated
      list->userinfo = ffw;                     // input for FileDirAdd2List

      TxFileTree( fspec, fopts, fattr, "", filter, txFileDirAdd2List, list);

      if ((list->count == 0) && (strchr(fopts, 'D') != NULL)) // inaccessible directory
      {
         txSelDestroy( &list);
         list = TxSelEmptyList( "..", "Directory is not accessible, contents unknown!", TRUE);
      }
      if (list != NULL)
      {
         list->flags    = TXSL_MULTI_QUICK;     // multi-char quick select
         list->userinfo = 0;
         list->tsize    = list->asize;

         list->selected = 0;                    // always start with first ...

         if (strchr(fopts, 'D') != NULL)        // directories
         {
            list->sortinfo = &txfd_dir_sort;
         }
         else                                   // files
         {
            list->sortinfo = &txfd_file_sort;
         }
         TxSelSortCurrent( list);
      }
   }
   RETURN( list);
}                                               // end 'TxFileDirSelist'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Add file/directory instance to the Selist being built
/*****************************************************************************/
static ULONG txFileDirAdd2List
(
   char               *fname,                   // IN    full filename
   TXFILEFIND         *finfo,                   // IN    file information
   void               *cdata                    // IN    Callback data
)
{
   ULONG               rc  = NO_ERROR;
   TXSELIST         *list = (TXSELIST *) cdata; // list being built
   TXS_ITEM         *item;                      // single item

   ENTER();
   TRARGS(("fname: '%s'\n",  fname));           // full filename

   if (list != NULL)                            // do we have a list ?
   {
      if (list->count >= list->asize)           // reached maximum array size
      {
         ULONG         newsize = 2 * list->asize + 100;

         TRACES(("realloc items to %lu items\n",  newsize));
         if ((list->items = realloc( list->items, newsize *
                                     sizeof(TXS_ITEM *))) != NULL)
         {
            list->asize = newsize;              // size has doubled now
         }
         else
         {
            rc = TX_ALLOC_ERROR;
         }
      }
      if (rc == NO_ERROR)
      {
         if ((item = TxAlloc( 1, sizeof(TXS_ITEM))) != NULL)
         {
            TRACES(("Alloc: %lu, adding item %lu: '%s' for '%s'\n",
                     list->asize, list->count, finfo->fName, fname));

            item->userdata = list->count;       // assign item number for unsort
            list->items[list->count++] = item;  // attach item to list

            //- note: filelist window width available in userinfo lower 2 bytes
            if ((item->text = TxAlloc( 1, strlen(finfo->fName)+1)) != NULL)
            {
               strcpy( item->text, finfo->fName); // basename as item-text
            }
            item->helpid = TXWH_USE_WIN_HELP;   // from list-window itself
            item->index  = 1;                   // quick select on 1st char
                                                // (unless multi-quick flag)

            TRACES(("item: 0x%P sequence:% 5lu\n", item, item->userdata));

            if (list->userinfo & TXFDS_ADD_DESCRIPTION)
            {
               int    fnlength = max( strlen(finfo->fName) +1, TXMAXTT -8);
               TRACES(( "fnlength: %d\n", fnlength));

               if ((item->desc = TxAlloc(1, fnlength + 44)) != NULL)
               {
                  TXTT   size;                  // size string for files, 15 digits
                  TXTT   date;
                  TXLN   name;                  // fnlength may be TXMAXLN!

                  strftime( date, TXMAXTT, " %Y-%m-%d %H:%M:%S ", gmtime( &(finfo->wTime)));
                  sprintf(  name, " %-*.*s", fnlength, fnlength,  finfo->fName);
                  if (finfo->fAttr & FATTR_DIRECTORY)
                  {
                     strcpy( size, "    <directory>");
                  }
                  else
                  {
                     sprintf( size, "%15llu", finfo->fSize);
                  }
                  txFileAttr2String( finfo->fAttr, item->desc);
                  strcat( item->desc, date);
                  strcat( item->desc, size);
                  strcat( item->desc, name);
               }
            }
         }
      }
   }
   else
   {
      TRACES(("supplied listpointer is NULL!\n"));
   }
   RETURN (rc);
}                                               // end 'txFileDirAdd2List'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Split multiple wildcard in BASE and INCLUDE array
/*****************************************************************************/
char **txWildcard2Filter
(
   char               *base,                    // INOUT base specification
   char               *spec,                    // IN    wildcard (multi)
   char              **fa,                      // IN    filter array
   int                 size                     // IN    size of filter array
)
{
   char              **rc = NULL;               // function return
   char               *we;                      // wildcard end
   char               *wb;                      // wildcard begin
   int                 i;
   TXLN                wildcard;                // modifiable copy

   ENTER();

   TRACES(( "base: '%s'  wildcard: '%s'\n", base, spec));

   strcpy( wildcard, spec);                     // use local copy to modify
   if ((we = strchr( wildcard, ';')) != NULL)   // multiple wildcards, use
   {                                            // include mechanism
      for ( i = 0,           wb  = wildcard;
           (i < size -1) && (wb != NULL);
            i++)
      {
         if (we != NULL)
         {
            *we = '\0';                         // terminate this wildcard
         }
         if ((fa[i] = TxAlloc( 1, strlen( wb) + 1)) != NULL)
         {
            strcpy( fa[i], wb);
            TRACES(( "Added filter %u: '%s'\n", i, fa[i]));
         }
         if ((wb = we) != NULL)                 // more to come
         {
            wb++;                               // advance beyond the ';'
            we = strchr( wb, ';');              // find new end
         }
      }
      fa[i] = NULL;                             // terminate the array
      strcat( base, FS_WILDCARD);               // standard wildcard as base

      rc = fa;                                  // return array
   }
   else
   {
      strcat( base, wildcard);                  // use supplied wildcard
   }
   RETURN (rc);
}                                               // end 'txWildcard2Filter'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Free filter array memory
/*****************************************************************************/
char **txFreeFilter                             // RET   NULL
(
   char              **filter,                  // IN    filter array
   int                 size                     // IN    max number of entries
)
{
   int                 i;

   ENTER();

   if (filter != NULL)
   {
      for (i = 0; (i < size) && (filter[i] != NULL); i++)
      {
         TRACES(( "Freeing wildcard %u at %8.8lx\n", i, filter[i]));
         TxFreeMem( filter[i]);
      }
   }
   RETURN (NULL);
}                                               // end 'txFreeFilter'
/*---------------------------------------------------------------------------*/
#endif                                          // USEWINDOWING


/*****************************************************************************/
// Find first file matching the specified (wildcard) name
/*****************************************************************************/
ULONG TxFindFirstFile                           // RET   result
(
   char               *fspec,                   // IN    file specification
   TXHANDLE           *handle,                  // OUT   filefind handle
   TXFILEFIND         *found                    // OUT   found file details
)
{
   ULONG               rc = NO_ERROR;           // function return
   #if   defined (WIN32)
      WIN32_FIND_DATA     ffb;                  // filefind buffer
   #elif defined (DOS32)
      struct _finddata_t  ffb;                  // filefind buffer
   #elif defined (UNIX)
      TXLIN_FFINFO       *ffi = NULL;
   #else
      ULONG               fnum = 1;             // number of files
      FILEFINDBUF4        ffs;                  // OS/2 kernel 4.0 or older
      FILEFINDBUF4L       ffl;                  // OS/2 kernel 4.5 or newer
   #endif

   ENTER();
   TRARGS(("fspec: '%s'\n", fspec));

   memset( found, 0, sizeof(TXFILEFIND));       // initialize all fields

   #if defined (WIN32)
      if ((*handle = (TXHANDLE) FindFirstFile( fspec, &ffb)) != -1)
      {
         found->fAttr = ffb.dwFileAttributes;
         found->fSize = (((LLONG) ffb.nFileSizeHigh) << 32) + ffb.nFileSizeLow;

         found->cTime = txWinFileTime2t( (NTIME *) &ffb.ftCreationTime,   0);
         found->aTime = txWinFileTime2t( (NTIME *) &ffb.ftLastAccessTime, 0);
         found->wTime = txWinFileTime2t( (NTIME *) &ffb.ftLastWriteTime,  0);

         strcpy( found->fName, ffb.cFileName);
      }
      else
      {
         rc = ERROR_NO_MORE_FILES;
      }
   #elif defined (DOS32)
      if ((*handle = (TXHANDLE) _findfirst( fspec, &ffb)) != -1)
      {
         found->fAttr = (ULONG) ffb.attrib;
         found->fSize = ffb.size;

         found->cTime = ffb.time_create  - 18000; // WATCOM seems 5 hours off
         found->aTime = ffb.time_access  - 18000;
         found->wTime = ffb.time_write   - 18000;

         strcpy( found->fName, ffb.name);
      }
      else
      {
         rc = ERROR_NO_MORE_FILES;
      }
   #elif defined (UNIX)
      if ((ffi = TxAlloc(1, sizeof( TXLIN_FFINFO))) != NULL)
      {
         txGetPath( fspec, ffi->dirname, ffi->wildcard);

         TRACES(( "Unix opendir for: '%s'\n", ffi->dirname));
         if ((ffi->dir = opendir( ffi->dirname)) != NULL)
         {
            if ((rc = TxUnixNextFile( ffi, found)) != NO_ERROR)
            {
               closedir(  ffi->dir);
               TxFreeMem( ffi);
            }
         }
         else                                   // invalid, perhaps wildcard ...
         {
            TxFreeMem( ffi);
         }
      }
      *handle = (TXHANDLE) ffi;                 // assign handle
   #else
      *handle = (TXHANDLE) HDIR_CREATE;
      if (TxLargeFileApiOS2( NULL))
      {
         rc = DosFindFirst( fspec, (HDIR *) handle, TX_ALL_ATTRS,
                            &ffl, sizeof( FILEFINDBUF4L),
                            &fnum, FIL_QUERYEASIZEL);
         if (rc == NO_ERROR)
         {
            found->fAttr = ffl.attrFile;
            found->fSize = ffl.cbFile;

            found->cTime = txOS2FileTime2t( (USHORT *) &ffl.fdateCreation,
                                            (USHORT *) &ffl.ftimeCreation);
            found->aTime = txOS2FileTime2t( (USHORT *) &ffl.fdateLastAccess,
                                            (USHORT *) &ffl.ftimeLastAccess);
            found->wTime = txOS2FileTime2t( (USHORT *) &ffl.fdateLastWrite,
                                            (USHORT *) &ffl.ftimeLastWrite);

            strcpy( found->fName, ffl.achName);
         }
      }
      else
      {
         rc = DosFindFirst( fspec, (HDIR *) handle, TX_ALL_ATTRS,
                            &ffs, sizeof( FILEFINDBUF4),
                            &fnum, FIL_QUERYEASIZE);
         if (rc == NO_ERROR)
         {
            found->fAttr = ffs.attrFile;
            found->fSize = ffs.cbFile;

            found->cTime = txOS2FileTime2t( (USHORT *) &ffs.fdateCreation,
                                            (USHORT *) &ffs.ftimeCreation);
            found->aTime = txOS2FileTime2t( (USHORT *) &ffs.fdateLastAccess,
                                            (USHORT *) &ffs.ftimeLastAccess);
            found->wTime = txOS2FileTime2t( (USHORT *) &ffs.fdateLastWrite,
                                            (USHORT *) &ffs.ftimeLastWrite);

            strcpy( found->fName, ffs.achName);
         }
      }
   #endif
   TRARGS(("handle OUT: %8.8lx, fName: '%s'\n", *handle, found->fName));
   RETURN (rc);
}                                               // end 'TxFindFirstFile'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Find next file matching the specified (wildcard) name
/*****************************************************************************/
ULONG TxFindNextFile                            // RET   result
(
   TXHANDLE            handle,                  // IN    filefind handle
   TXFILEFIND         *found                    // OUT   found file details
)
{
   ULONG               rc = NO_ERROR;           // function return
   #if   defined (WIN32)
      WIN32_FIND_DATA     ffb;                  // filefind buffer
   #elif defined (DOS32)
      struct _finddata_t  ffb;
   #elif defined (UNIX)
      TXLIN_FFINFO       *ffi = NULL;
   #else
      ULONG               fnum = 1;             // number of files
      FILEFINDBUF4        ffs;                  // OS/2 4.xx or older
      FILEFINDBUF4L       ffl;                  // OS/2 4.5x or newer
   #endif

   ENTER();

   memset( found, 0, sizeof(TXFILEFIND));       // initialize all fields

   #if defined (WIN32)
      if (FindNextFile( (HANDLE) handle, &ffb))
      {
         found->fAttr = ffb.dwFileAttributes;
         found->fSize = (((LLONG) ffb.nFileSizeHigh) << 32) + ffb.nFileSizeLow;

         found->cTime = txWinFileTime2t( (NTIME *) &ffb.ftCreationTime,   0);
         found->aTime = txWinFileTime2t( (NTIME *) &ffb.ftLastAccessTime, 0);
         found->wTime = txWinFileTime2t( (NTIME *) &ffb.ftLastWriteTime,  0);

         strcpy( found->fName, ffb.cFileName);
      }
      else
      {
         rc = ERROR_NO_MORE_FILES;
      }
   #elif defined (DOS32)
      if (_findnext( (long) handle, &ffb) != -1)
      {
         found->fAttr = (ULONG) ffb.attrib;
         found->fSize = ffb.size;

         found->cTime = ffb.time_create  - 18000; // WATCOM seems 5 hours off
         found->aTime = ffb.time_access  - 18000;
         found->wTime = ffb.time_write   - 18000;

         strcpy( found->fName, ffb.name);
      }
      else
      {
         rc = ERROR_NO_MORE_FILES;
      }
   #elif defined (UNIX)
      if ((ffi = (TXLIN_FFINFO *) handle) != NULL) // handle open ?
      {
         rc = TxUnixNextFile( ffi, found);
      }
      else
      {
         rc = TX_INVALID_HANDLE;
      }
   #else
      if (TxLargeFileApiOS2( NULL))
      {
         rc = DosFindNext( (HDIR) handle, &ffl, sizeof( FILEFINDBUF4L), &fnum);
         if (rc == NO_ERROR)
         {
            found->fAttr = ffl.attrFile;
            found->fSize = ffl.cbFile;

            found->cTime = txOS2FileTime2t( (USHORT *) &ffl.fdateCreation,
                                            (USHORT *) &ffl.ftimeCreation);
            found->aTime = txOS2FileTime2t( (USHORT *) &ffl.fdateLastAccess,
                                            (USHORT *) &ffl.ftimeLastAccess);
            found->wTime = txOS2FileTime2t( (USHORT *) &ffl.fdateLastWrite,
                                            (USHORT *) &ffl.ftimeLastWrite);

            strcpy( found->fName, ffl.achName);
         }
      }
      else
      {
         rc = DosFindNext( (HDIR) handle, &ffs, sizeof( FILEFINDBUF4), &fnum);
         if (rc == NO_ERROR)
         {
            found->fAttr = ffs.attrFile;
            found->fSize = ffs.cbFile;

            found->cTime = txOS2FileTime2t( (USHORT *) &ffs.fdateCreation,
                                            (USHORT *) &ffs.ftimeCreation);
            found->aTime = txOS2FileTime2t( (USHORT *) &ffs.fdateLastAccess,
                                            (USHORT *) &ffs.ftimeLastAccess);
            found->wTime = txOS2FileTime2t( (USHORT *) &ffs.fdateLastWrite,
                                            (USHORT *) &ffs.ftimeLastWrite);

            strcpy( found->fName, ffs.achName);
         }
      }
   #endif
   TRARGS(("handle  IN: %8.8lx, fName: '%s'\n", handle, found->fName));
   RETURN (rc);
}                                               // end 'TxFindNextFile'
/*---------------------------------------------------------------------------*/

#if defined (UNIX)
/*****************************************************************************/
// Find next file matching the specified (wildcard) name
/*****************************************************************************/
static ULONG TxUnixNextFile                     // RET   result
(
   TXLIN_FFINFO       *ffi,                     // IN    TX filefind handle
   TXFILEFIND         *found                    // OUT   found file details
)
{
   ULONG               rc = NO_ERROR;           // function return
   struct stat         f;
   struct dirent      *dent;                    // one directory entry
   TXLN                filename;                // full filename
   int                 stat_rc;
   USHORT              st_mode;
   #if !defined (DARWIN)
      struct stat64    f64;
   #endif

   ENTER();
   TRACES(( "Unix NextFile handle: 0x%lx  path: '%s' wildcard: '%s'\n",
             ffi->dir, ffi->dirname, ffi->wildcard));
   do
   {
      memset( found, 0, sizeof(TXFILEFIND));    // reset before EVERY file!
      memset( &f,    0, sizeof(struct stat));   // just in case ...

      TRACES(( "read next dir entry using DIR handle: %8.8lx\n", ffi->dir));
      if ((ffi->dir != 0) && ((dent = readdir( ffi->dir)) != NULL))
      {
         strcpy( found->fName, dent->d_name);
         strcpy( filename, ffi->dirname);
         strcat( filename, found->fName);

         #if defined (DARWIN)
            if ((stat_rc = stat( filename, &f)) != -1)
            {
               TRHEXS( 70,  &f,  sizeof(f), "f");
               st_mode      = (USHORT) f.st_mode;
               found->fSize = (ULONG)  f.st_size;
               found->aTime = f.st_atime;
               found->wTime = f.st_mtime;
               found->cTime = f.st_ctime;
            }
         #else
            if ((stat_rc = stat( filename, &f)) != -1)
            {
               TRHEXS( 70,  &f,  sizeof(f), "f");
               st_mode      = (USHORT) f.st_mode;
               found->fSize = (ULONG)  f.st_size;
               found->aTime = f.st_atime;
               found->wTime = f.st_mtime;
               found->cTime = f.st_ctime;
            }
            else if (errno == EOVERFLOW)
            {
               //- retry with 64-bit stat (latest Linux kernels fail on regular)
               TRACES(( "EOVERFLOW, retry with stat64 ...\n"));
               if ((stat_rc = stat64( filename, &f64)) != -1)
               {
                  TRHEXS( 70,  &f64,  sizeof(f64), "f64");
                  st_mode      = (USHORT) f64.st_mode;
                  found->fSize = (ULONG)  f64.st_size;
                  found->aTime = f64.st_atime;
                  found->wTime = f64.st_mtime;
                  found->cTime = f64.st_ctime;
               }
            }
         #endif

         if (stat_rc != -1)                     // name exists
         {
            if (S_ISDIR( st_mode))              // is it a directory ?
            {
               found->fAttr |= FATTR_DIRECTORY;
            }
            if (( st_mode & S_IWRITE) == 0)
            {
               found->fAttr |= FATTR_READONLY;
            }
         }
         else if ((errno == EFBIG) ||            // must be a large-file
                  (errno == EOVERFLOW))
         {
            TRACES(( "stat errno %d on large-file '%s'\n", errno, filename));
            TxFileSize( filename, &(found->fSize)); // get real size
         }
         else                                   // DIR-entry invalid ??
         {
            TRACES(( "stat errno %d on '%s'\n", errno, filename));
            rc = ERROR_NO_MORE_FILES;
         }
      }
      else
      {
         TRACES(( "readdir error %d on '%s'\n", errno, ffi->dirname));
         rc = ERROR_NO_MORE_FILES;
      }
   } while ((rc == NO_ERROR) && (TxStrWcmp(found->fName, ffi->wildcard) < 0));

   TRARGS(("ffi IN: %8.8lx, fName OUT: '%s'\n", ffi, found->fName));
   RETURN (rc);
}                                               // end 'TxUnixNextFile'
/*---------------------------------------------------------------------------*/

#endif

/*****************************************************************************/
// Close FindFirst/FindNext session for specified handle
/*****************************************************************************/
ULONG TxFindClose                               // RET   result
(
   TXHANDLE            handle                   // IN    filefind handle
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();
   TRARGS(("handle: %8.8lx\n", handle));

   if (handle != 0)
   {
      #if defined (WIN32)
         rc = (ULONG) FindClose( (HANDLE) handle);
      #elif defined (DOS32)
         _findclose( (long) handle);
      #elif defined (UNIX)
         TXLIN_FFINFO  *ffi = (TXLIN_FFINFO *) handle;
         rc = (ULONG) closedir( ffi->dir);
         TxFreeMem( ffi);
      #else
         rc = (ULONG) DosFindClose( handle);
      #endif
   }
   else
   {
      rc = TX_INVALID_HANDLE;
   }
   RETURN (rc);
}                                               // end 'TxFindClose'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Simple (example) callback for FileTree, listing files date/attr/full-name
/*****************************************************************************/
ULONG txFileTreeOneFile
(
   char               *fname,                   // IN    full filename
   TXFILEFIND         *finfo,                   // IN    file information
   void               *cdata                    // IN    Callback data
)
{
   ULONG               rc  = NO_ERROR;
   TXLN                text;
   TXTM                tbuf;

   ENTER();
   TRARGS(("fname: '%s'\n", fname));

   strftime(text, TXMAXLN,  "%d-%m-%Y %H:%M:%S", gmtime( &(finfo->wTime)));
   sprintf( tbuf, " %10llu  ", finfo->fSize);
   strcat(  text, tbuf);
   txFileAttr2String( finfo->fAttr, tbuf);      // FILE attributes
   strcat(  text, tbuf);
   TxPrint( "%s  %s\n", text, fname);           // Full filename
   RETURN (rc);
}                                               // end 'txFileTreeOneFile'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Simple (example) callback for FileTree, listing files/dirs without path
/*****************************************************************************/
ULONG txFileTreeNoPath
(
   char               *fname,                   // IN    full filename
   TXFILEFIND         *finfo,                   // IN    file information
   void               *cdata                    // IN    Callback data
)
{
   ULONG               rc  = NO_ERROR;

   ENTER();
   TRARGS(("fname: '%s'\n",  fname));           // full filename

   TxPrint( "  %s\n", finfo->fName);            // Base filename
   RETURN (rc);
}                                               // end 'txFileTreeNoPath'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Recursive find files and directories matching filespec and attributes
/*****************************************************************************/
ULONG TxFileTree
(
   char               *fspec,                   // IN    Filespec to match
   char               *fopts,                   // IN    Find options
   char               *fattr,                   // IN    Select attributes
   char               *mattr,                   // IN    Modify attributes
   char              **filter,                  // IN    Incl/Excl file-spec
   TXTREE_CALLBACK     callback,                // IN    function to call back
   void               *calldata                 // IN    callback data
)
{
   ULONG               rc = NO_ERROR;
   TXLN                FileSpec;                // File spec to look for
   char                Opts[64];                // Search options
   ULONG               options;                 // Mask of options
   char               *s;                       // string pointer
   char                smask[6];                // select attribute mask
   char                dmask[6];                // modify attribute mask
   TXTREEDATA          ldp;                     // local data

   ENTER();
   TRACES(("fspec: '%s' fopts:%s fattr:%s mattr:%s filter:%8.8lx\n",
            fspec,      fopts,   fattr,   mattr,   filter));

   options = FIRST_TIME | DO_FILES | DO_DIRS;   // initial options

   ldp.filter = filter;                         // copy filter references

   #ifndef UNIX                                 // no driveletters in Unix :-)
   if (fspec[0] == FS_PATH_SEP)                 // auto-root handling
   {
      getcwd(FileSpec, TXMAXLN);                // get current drive/directory
      FileSpec[2] = '\0';                       // just keep drive-letter part
      strcat(FileSpec, fspec);                  // add path-component
   }
   else
   #endif
   {
      strcpy(FileSpec, fspec);                  // get supplied file spec
      if ((strcmp( fspec+1, ":") == 0) ||       // drive only, or
          (strlen( fspec)        == 0)  )       // no filespec given
      {
         strcat( FileSpec, ".");                // force 'current dir'
      }
   }
   if (strcspn(FileSpec, "*?") == strlen(FileSpec)) // if no wildcards
   {
      TxTrueNameDir( FileSpec, FALSE, FileSpec); // append SEP if a directory
   }
   if (FileSpec[strlen(FileSpec)-1] == FS_PATH_SEP) // fspec ends in SEP
   {
      strcat(FileSpec, FS_WILDCARD);            // then append wildcard
   }

   TRACES(("FileSpec to start with: '%s'\n", FileSpec));

   strcpy(Opts, fopts);
   TxStrToUpper(Opts);                          // Uppercase options string
   if (strchr(Opts, 'S') != NULL)
   {
      options |= RECURSE;                       // Should we recurse
   }
   if (strchr(Opts, 'P') != NULL)
   {
      options |= INCL_DOTDOT;                   // Should we include ..
   }
   if (strchr(Opts, 'F') != NULL)
   {
      options &= ~DO_DIRS;                      // Should not include dirs !
      options |= DO_FILES;                      // Should include files !
   }
   if (strchr(Opts, 'D') != NULL)
   {
      options |= DO_DIRS;                       // Should include dirs !
      options &= ~DO_FILES;                     // Should not include files !
   }
   if (strchr(Opts, 'B') != NULL)
   {
      options |= DO_DIRS;                       // Should include dirs !
      options |= DO_FILES;                      // Should include files !
   }
   if (strchr(Opts, 'I') != NULL)               // FSPEC's are includes
   {
      options |= INCLUDESPEC;
   }

   strcpy(smask, "");                           // No mask unless specified
   if (*fattr)                                  // if specified
   {
      strcpy(smask, "*****");                   // default mask
      strcpy(Opts, fattr);
      TxStrToUpper(Opts);                       // Uppercase options string
      for (s = Opts; *s && *(s+1); s++)
      {
         switch (*s)
         {
            case '+':
            case '-':
               switch (*(s+1))
               {
                  case 'A': smask[0] = *s++; break;
                  case 'D': smask[1] = *s++; break;
                  case 'H': smask[2] = *s++; break;
                  case 'R': smask[3] = *s++; break;
                  case 'S': smask[4] = *s++; break;
               }
               break;
         }
      }
   }

   strcpy(dmask, "");                           // No mask unless specified
   if (*mattr)                                  // if specified
   {
      strcpy(dmask, "*****");                   // default mask
      strcpy(Opts, mattr);
      TxStrToUpper(Opts);                       // Uppercase options string
      for (s = Opts; *s && *(s+1); s++)
      {
         switch (*s)
         {
            case '+':
            case '-':
               switch (*(s+1))
               {
                  case 'A': dmask[0] = *s++; break;
                  case 'D': dmask[1] = *s++; break;
                  case 'H': dmask[2] = *s++; break;
                  case 'R': dmask[3] = *s++; break;
                  case 'S': dmask[4] = *s++; break;
               }
               break;
         }
      }
   }
   TRACES(( "smask: '%s'  dmask: '%s'\n", smask, dmask));

   rc = txRecursiveFindFile( FileSpec, &ldp, smask, dmask, options,
                             callback, calldata);
   RETURN( rc);
}                                               // end 'TxFileTree'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Split path and filename part of a full-filespec
/*****************************************************************************/
static void txGetPath
(
   char               *string,                  // IN    full filespec
   char               *path,                    // OUT   path component, incl '\'
   char               *filename                 // OUT   filename component
)                                               //       or NULL if not wanted
{
   int                 len;
   int                 LastSlashPos;

   ENTER();
   len = strlen(string);                        // Get length of full fspec
   LastSlashPos = len;                          // Get max pos of last SEP

   //- Step back through string until at begin or at SEP char
   while ((string[LastSlashPos] != FS_PATH_SEP) && (LastSlashPos >= 0))
   {
      --LastSlashPos;
   }

   //- Copy filespec up to and including last SEP to path
   memcpy(path, string, LastSlashPos+1);
   path[LastSlashPos+1] = '\0';

   if (filename != NULL)                        // filename wanted too ?
   {
      strcpy(filename, &string[LastSlashPos+1]);
   }
   TRACES(("RESULTING path: '%s'  filename: '%s'\n", path, filename));
   VRETURN();
}                                               // end 'txGetPath'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare two sets of file-attributes in REXXXUtil format
/*****************************************************************************/
static BOOL txSameAttr                          // RET   attrs are  same
(
   char               *mask,                    // IN    1st set, as string
   ULONG               attr                     // IN    2nd set as bitmask
)
{
   if (!(*mask))
   {
      return  TRUE;
   }
   if (mask[0] == '-' &&  (attr & FATTR_ARCHIVED))
   {
      return  FALSE;
   }
   if (mask[0] == '+' && !(attr & FATTR_ARCHIVED))
   {
      return  FALSE;
   }
   if (mask[1] == '-' &&  (attr & FATTR_DIRECTORY))
   {
      return  FALSE;
   }
   if (mask[1] == '+' && !(attr & FATTR_DIRECTORY))
   {
      return  FALSE;
   }
   if (mask[2] == '-' &&  (attr & FATTR_HIDDEN))
   {
      return  FALSE;
   }
   if (mask[2] == '+' && !(attr & FATTR_HIDDEN))
   {
      return  FALSE;
   }
   if (mask[3] == '-' &&  (attr & FATTR_READONLY))
   {
      return  FALSE;
   }
   if (mask[3] == '+' && !(attr & FATTR_READONLY))
   {
      return  FALSE;
   }
   if (mask[4] == '-' &&  (attr & FATTR_SYSTEM))
   {
      return  FALSE;
   }
   if (mask[4] == '+' && !(attr & FATTR_SYSTEM))
   {
      return  FALSE;
   }
   return  TRUE;
}                                               // end 'txSameAttr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Combine two sets of file-attributes in REXXXUtil format
/*****************************************************************************/
static ULONG txNewAttr
(
   char               *mask,
   ULONG               attr
)
{
   if (!(*mask))
   {
      return  attr;
   }

   if (mask[0] == '-')
   {
      attr &= ~FATTR_ARCHIVED;                   // Clear
   }
   if (mask[0] == '+')
   {
      attr |= FATTR_ARCHIVED;                    // Set
   }
   if (mask[1] == '-')
   {
      attr &= ~FATTR_DIRECTORY;                  // Clear
   }
   if (mask[1] == '+')
   {
      attr |= FATTR_DIRECTORY;                   // Set
   }
   if (mask[2] == '-')
   {
      attr &= ~FATTR_HIDDEN;                     // Clear
   }
   if (mask[2] == '+')
   {
      attr |= FATTR_HIDDEN;                      // Set
   }
   if (mask[3] == '-')
   {
      attr &= ~FATTR_READONLY;                   // Clear
   }
   if (mask[3] == '+')
   {
      attr |= FATTR_READONLY;                    // Set
   }
   if (mask[4] == '-')
   {
      attr &= ~FATTR_SYSTEM;                     // Clear
   }
   if (mask[4] == '+')
   {
      attr |= FATTR_SYSTEM;                      // Set
   }
   return  attr;
}                                               // end 'txNewAttr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Change file attribute bits
/*****************************************************************************/
static ULONG  txSetFileMode
(
   char               *file,                    // file name
   ULONG               attr                     // new file attributes
)
{
   ULONG               rc = NO_ERROR;           // function return code
   #if   defined (WIN32)
   #elif defined (DOS32)
   #elif defined (UNIX)
      //- to be refined
   #else
      FILESTATUS3         status;               // status information
   #endif

   ENTER();

   //- to be refined, make OS independant


   #if   defined (WIN32)
   #elif defined (DOS32)
   #elif defined (UNIX)
      //- to be refined
   #else
      rc = DosQueryPathInfo( file, FIL_STANDARD, &status, sizeof(status));
      if (rc == NO_ERROR)
      {
         status.attrFile = attr;
         rc = DosSetPathInfo( file,             // set the file info
                              FIL_STANDARD,
                              &status,
                              sizeof(FILESTATUS3),
                              0);
      }
   #endif
   RETURN(rc);                                  // give back success flag
}                                               // end 'txSetFileMode'
/*---------------------------------------------------------------------------*/


/*****************************************************************************
* Function: txRecursiveFindFile                                              *
*                                                                            *
* Purpose:  Finds all files starting with FileSpec, and will look down the   *
*           directory tree if required.                                      *
*                                                                            *
* Params:   FileSpec - ASCIIZ string which designates filespec to search     *
*                       for.                                                 *
*                                                                            *
*           ldp      - Pointer to local data structure.                      *
*                                                                            *
*           smask    - Array of charactes describing the source attribute    *
*                      mask.  Only files with attributes matching this mask  *
*                      will be found.                                        *
*                                                                            *
*           dmask    - Array of characters describing the target attribute   *
*                      mask.  Attributes of all found files will be set      *
*                      using this mask.                                      *
*                                                                            *
*             Note:  Source and targets mask are arrays of char (string)     *
*                    Each index of the mask corresponds to a different       *
*                    file attribute:                                         *
*                                      mask[0] = FATTR_ARCHIVED              *
*                                      mask[1] = FATTR_DIRECTORY             *
*                                      mask[2] = FATTR_HIDDEN                *
*                                      mask[3] = FATTR_READONLY              *
*                                      mask[4] = FATTR_SYSTEM                *
*                                                                            *
*                    '-'  indicates an attribute bit SET                     *
*                    '+'  indicates an attribute bit CLEARED                 *
*                    '*'  indicates an attribute bit DONT_CARE or KEEP       *
*                                                                            *
*           options  - The search/output options.  The following options     *
*                       may be ORed together when calling this function:     *
*                                                                            *
*                    FIRST_TIME  - Indicates this is initial call.  This     *
*                                   should always be used.                   *
*                    RECURSE     - Indicates that function should search     *
*                                   all child subdirectories recursively.    *
*                    DO_DIRS     - Indicates that directories should be      *
*                                   included in the search.                  *
*                    DO_FILES    - Indicates that files should be included   *
*                                   in the search.                           *
*                                                                            *
*           callback - The function called for each file matching the spec   *
*                                                                            *
*           calldata - Free-format data pointer passes thru to callback      *
*                                                                            *
* Used By:  TxFileTree()                                                     *
*****************************************************************************/

/*****************************************************************************/
// Recursively find files and callback for each one
/*****************************************************************************/
static ULONG txRecursiveFindFile                // RET   result error code
(
   char               *FileSpec,                // IN    Filespecs to search
   TXTREEDATA         *ldp,                     // INOUT local data
   char               *smask,                   // IN    select attributes
   char               *dmask,                   // IN    modify attributes
   ULONG               options,                 // IN    search options
   TXTREE_CALLBACK     callback,                // IN    function to call back
   void               *calldata                 // IN    callback data
)
{
   ULONG               rc = NO_ERROR;
   ULONG               ntype;                   // name-type file or directory
   TXFILEFIND          finfo;                   // TX FileFind results
   TXHANDLE            fffh;                    // Directory handle
   TXLN                path;                    // Path to target file(s)
   TXLN                filename;                // Current file name
   BOOL                FltDotdir = FALSE;       // . or .. must be filtered
   BOOL                IsaSubdir = FALSE;       // file is a dir
   BOOL                IsIncludeExcludeMatch;   // file matches set criteria
   char              **x;                       // Exclude fspec reference

   ENTER();
   TRACES(( "FileSpec now: '%s'\n", FileSpec));

   if (TxAbort())                               // user abort request
   {
      RETURN( TX_ABORTED);                      // stop, breaking recursion
   }

   /********************************************************************
   * Search Strategy Explained:                                        *
   *                                                                   *
   * Get path and filespec to be searched.                             *
   *                                                                   *
   * If this is the initial call (see FIRST_TIME) and                  *
   * searching recursively, make sure to look for the                  *
   * original filespec in the initial directory.                       *
   *                                                                   *
   * From then on, look for a filespec of *.* to expand                *
   * the subdirectories.  Once a subdirectory is expanded,             *
   * search for the initial filespec again.                            *
   ********************************************************************/

   txGetPath(FileSpec, path, filename);

   /********************************************************************
   * Initial Recursive case:                                           *
   *                                                                   *
   * This section of code recurses to call itself in the               *
   * non-recursive form.  It will allow the function to                *
   * gather all the files in the initial subdirectory.                 *
   *                                                                   *
   * It also saves the initial filespec to be searched                 *
   * for in all directories which will be expanded (see                *
   * ldp->TargetSpec).  From then on, the recursive case               *
   * will search for directories by examining all files                *
   * matching the spec *.*.                                            *
   ********************************************************************/

   if ((options & FIRST_TIME) && (options & RECURSE))
   {
      TRACES(( "Initial Recursive case ...\n"));

      if ((rc = txRecursiveFindFile( FileSpec, ldp, smask, dmask,
                                     options &~ RECURSE &~ FIRST_TIME,
                                     callback, calldata)) != NO_ERROR)
      {
         RETURN (rc);                           // return the error code
      }
      strcpy( ldp->TargetSpec, filename);
      strcpy( filename, FS_WILDCARD);
      sprintf(FileSpec, "%s%s", path, FS_WILDCARD);
      options &= ~FIRST_TIME;
   }

   /********************************************************************
   * Non-Recursive case:                                               *
   *                                                                   *
   * This section of code finds all files matching the files           *
   * txRecursiveFindFilepec (conditionally filtering out . and ..)     *
   * and call the specified callback function to process this file.    *
   * Each found file's attributes are set as required if needed.       *
   ********************************************************************/

   if (!(options & RECURSE))
   {
      TRACES(( "Non-Recursive case ...\n"));

      rc = TxFindFirstFile( FileSpec, &fffh, &finfo);
      while (rc == NO_ERROR)
      {
         IsaSubdir = (BOOL)(finfo.fAttr & FATTR_DIRECTORY);
         FltDotdir = (BOOL)((strcmp(finfo.fName,     "." ) == 0) ||
                           ((strcmp(finfo.fName,     "..") == 0) &&
                           ((strcmp(path,     FS_PATH_STR) == 0) ||
                                  ((options & INCL_DOTDOT) == 0))));

         TRACES(("FltDotDir: %s  path: '%s' inc.. 0x%lx\n",
                 (FltDotdir) ? "YES" : "NO ", path, (options & INCL_DOTDOT)));

         if ( !FltDotdir && txSameAttr(smask, finfo.fAttr) &&
             ((IsaSubdir && (options & DO_DIRS)) ||
            ( !IsaSubdir && (options & DO_FILES))))
         {
            sprintf(ldp->tempfile, "%s%s", path, finfo.fName);
            ntype = TxTrueNameDir( ldp->tempfile, FALSE, ldp->truefile);
            if ((ntype == TXTN_DIR) ||
                (ntype == TXTN_FILE))           // filter out illegal ones
            {                                   // like .. in the root!
               if (ldp->filter != NULL)         // only when specified ...
               {
                  TRACES(( "filter: %8.8lx, first ptr : %8.8lx\n",
                            ldp->filter, ldp->filter[0]));
                  for (   x  = ldp->filter;
                       ((*x != NULL) && (**x != 0));
                          x++)                  // until none/empty
                  {
                     TRACES(( "*x: %8.8lx, first spec: '%s'\n", *x, *x));
                     if (TxStrWicmp(ldp->truefile, *x) >= 0)
                     {
                        break;                  // abort incl/excl matching
                     }
                  }
                  IsIncludeExcludeMatch =
                     ((!(*x && **x) && !(options & INCLUDESPEC)) || // EXCL no match
                      ( (*x && **x) &&  (options & INCLUDESPEC)) ); // INCL matched
               }
               else
               {
                  IsIncludeExcludeMatch = TRUE; // no spec, always matched
               }
               if (IsIncludeExcludeMatch)
               {
                  rc = (callback) ( ldp->truefile, &finfo, calldata);
                  TRACEX(("TxTree callback rc: %lu on '%s'\n", rc, ldp->truefile));
                  if (rc == NO_ERROR)
                  {
                     ldp->nattrib = txNewAttr(dmask, finfo.fAttr);
                     if (ldp->nattrib != finfo.fAttr)
                     {
                        if (txSetFileMode( ldp->tempfile,
                                          (ldp->nattrib &~ FATTR_DIRECTORY)))
                        {
                           ldp->nattrib = finfo.fAttr;
                        }
                     }
                  }
               }
            }
         }
         if (rc == NO_ERROR)
         {
            if (TxAbort())
            {
               rc = TX_ABORTED;                 // stop, breaking recursion
            }
            else
            {
               rc = TxFindNextFile( fffh, &finfo);
            }
         }
      }
   }

   /********************************************************
   * Recursive case:                                       *
   *                                                       *
   * This section of code searches for directories given   *
   * the current FileSpec.  When one is found, the         *
   * function is called in the non-recursive mode to find  *
   * all files matching the initial filespec.  It is also  *
   * called in the recursive mode to expand all subdirect- *
   * ories under the new found directory.                  *
   ********************************************************/

   else
   {
      TRACES(( "Recursive case ...\n"));

      rc = TxFindFirstFile( FileSpec, &fffh, &finfo);
      while (rc == NO_ERROR)
      {
         IsaSubdir = (BOOL)(finfo.fAttr & FATTR_DIRECTORY);
         FltDotdir = (BOOL)((strcmp(finfo.fName,     "." ) == 0) ||
                           ((strcmp(finfo.fName,     "..") == 0) &&
                           ((strcmp(path,     FS_PATH_STR) == 0) ||
                                  ((options & INCL_DOTDOT) == 0))));

         TRACES(("FltDotDir: %s  path: '%s' inc.. 0x%lx\n",
                 (FltDotdir) ? "YES" : "NO ", path, (options & INCL_DOTDOT)));

         if (!FltDotdir && IsaSubdir)
         {
            sprintf(ldp->tempfile, "%s%s%c%s",
                    path, finfo.fName, FS_PATH_SEP, ldp->TargetSpec);
            if ((rc = txRecursiveFindFile( ldp->tempfile, ldp,
                                           smask, dmask, options &~ RECURSE,
                                           callback, calldata)) != NO_ERROR)
            {
               RETURN (rc);
            }
            sprintf( ldp->tempfile, "%s%s%c%s",
                     path, finfo.fName, FS_PATH_SEP, filename);
            if ((rc = txRecursiveFindFile( ldp->tempfile, ldp,
                                           smask, dmask, options,
                                           callback, calldata)) != NO_ERROR)
            {
               RETURN (rc);
            }
         }
         if (TxAbort())
         {
            rc = TX_ABORTED;                    // stop, breaking recursion
         }
         else
         {
            rc = TxFindNextFile( fffh, &finfo);
         }
      }
   }
   TxFindClose( fffh);
   if (rc == ERROR_NO_MORE_FILES)               // no more todo, OK
   {
      rc = NO_ERROR;
   }
   RETURN (rc);
}                                               // end 'RecursiveFindFile'
/*---------------------------------------------------------------------------*/

