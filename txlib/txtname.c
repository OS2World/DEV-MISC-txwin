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
// TxLib filesystem functions, TrueName section
//

#include <txlib.h>                              // TxLib interface
#include <txwpriv.h>                            // private window interface

#include <sys/stat.h>                           // for low level stuff

// Normalize relative path+filename specification (canonicalize)
static char *TxFsRealPath                       // RET   resulting name (pfn)
(
   char               *pfn                      // INOUT path+filename (LINE)
);


/*****************************************************************************/
// Query type for specified path and resolve truename (canonical)
// Directorynames (for DIR or stripped filename) will ALWAYS end in a '\'
/*****************************************************************************/
ULONG  TxTrueNameDir                            // RET   0 = Dir, 80 = File
(                                               //       others are errors
   char               *path,                    // IN    path/fn specification
   BOOL                stripfile,               // IN    strip filename part
   char               *truename                 // OUT   true filename or NULL
)
{
   ULONG               rc = TXTN_DIR;           // return value, dir
   TXLN                pspec;                   // path/fn, modifyable
   TXLN                tname;                   // True name, internal
   int                 last;
   #if   defined (WIN32)
      ULONG            ulAttr;
      char            *p;
   #elif defined (DOS32)
      union  REGS      regs;
      struct SREGS     sreg;
      TXDX_RMINFO      txdx_rmi;
      char            *fn;
      char            *tn;
      struct stat      fstat;
   #elif defined (UNIX)
      struct stat      fstat;
      int              stat_rc;
      USHORT           st_mode;
      #if !defined (DARWIN)
         struct stat64    fstat64;
      #endif
   #else
      FILESTATUS3      fstat;
   #endif

   ENTER();
   TRACES(("path: '%s'\n", path));
   if (strlen(path) == 0)
   {
      strcpy( pspec, ".");                      // use current directory
   }
   else
   {
      strcpy( pspec, path);
      last = strlen(pspec) -1;
      if (pspec[ last] == FS_PATH_SEP)
      {
         if (((pspec[1] != ':') && (last > 1)) || (last > 2))
         {
            pspec[last] = '\0';                 // remove trailing \ from DIR
         }
      }
   }
   TRACES(("path: '%s' - pspec: '%s'\n", path, pspec));
   #if defined (WIN32)
      ulAttr = GetFileAttributes(pspec);
      if (ulAttr != TXINVALID)
      {
         GetFullPathName( pspec, TXMAXLN, tname, &p);
         if ((ulAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
         {
            if (stripfile)
            {
               TxStripBaseName( tname);
            }
            rc = TXTN_FILE;
         }
      }
      else
      {
         if (stripfile)
         {
            TxStripBaseName( pspec);
            if ((strlen(pspec) != 2) || (pspec[1] != ':'))
            {
               if (GetFullPathName(pspec, TXMAXLN, tname, &p) == 0)
               {
                  rc = TX_INVALID_DRIVE;
               }
            }
            else                                // Root directory
            {
               strcpy( tname, pspec);
            }
         }
      }
   #elif defined (DOS32)
      if ((txwa->dpmi1 != NULL) && (txwa->dpmi2 != NULL))
      {                                         // proper TXinit/term
         memset( txwa->dpmi1, 0, 512);
         memset( txwa->dpmi2, 0, 512);
         fn    = txwa->dpmi1;                   // DPMI compatible mem
         tn    = txwa->dpmi2;                   // DPMI compatible mem

         strcpy( fn, pspec);                    // filename IN

         memset( &regs,  0, sizeof(regs));
         memset( &sreg,  0, sizeof(sreg));

         memset( &txdx_rmi, 0, sizeof(txdx_rmi));
         txdx_rmi.eax = 0x6000;                 // Canonical filename AH=60
         txdx_rmi.ds  = txDpmiSegment(fn);      // in  fname (ds:dx, dx=0)
         txdx_rmi.es  = txDpmiSegment(tn);      // out tname (es:di, di=0)

         TRHEXS( 100,  &txdx_rmi,  sizeof(txdx_rmi), "txdx_rmi");

         regs.w.ax    = TXDX_DPMI_RMINT;        // simulate realmode INT
         regs.h.bl    = TXDX_DOS;               // DOS interrupt 21
         regs.x.edi   = FP_OFF( &txdx_rmi);     // real mode register struct
         sreg.es      = FP_SEG( &txdx_rmi);

         txDpmiCall( &regs, &sreg);

         TRHEXS( 100,  txwa->dpmi2,  512, "dpmi2 (tname)");
         TRACES(("regs.x.cflag:%4.4hx  ax:%4.4hx\n", regs.x.cflag, TXWORD.ax));

         if (regs.x.cflag == 0)
         {
            if ((tn[0] == '\\') &&              // Hack to correct strange
                (tn[1] == '\\') &&              // truename resolved for
                (tn[2] != '\\') &&              // CDROM paths on FreeDOS
                (tn[3] == '.')  &&              // using the generic driver
                (tn[4] == '\\'))                // like: "\\O.\B." for "O:"
            {
               tname[0] = tn[2];                // driveletter
               tname[1] = ':';
               tname[2] = '\0';
               strcat( tname, tn + 7);          // corrected truename OUT
            }
            else
            {
               strcpy( tname, tn);              // truename OUT
            }
            if (stat( tname, &fstat) == NO_ERROR)
            {
               if (S_ISREG(fstat.st_mode))      // regular file ?
               {
                  if (stripfile)
                  {
                     TxStripBaseName( tname);
                  }
                  rc = TXTN_FILE;
               }
            }
            else
            {
               //- bug in OpenWatcom 1.6/1.7 stat seems to fail for directories
               TRACES(( "stat errno: %d for '%s' = %s\n", errno, tname, strerror(errno)));
               if (__WATCOMC__ > 1170)          // If OpenWatcom later than 1.7
               {                                // it is a real error :-)
                  rc = ERROR_FILE_NOT_FOUND;
               }
            }
         }
         else
         {
            rc = TXWORD.ax;
         }
      }
   #elif defined (UNIX)
      strcpy( tname, TxFsRealPath(pspec));      // canonicalized

      #if defined (DARWIN)
         if ((stat_rc = stat( tname, &fstat)) != -1)
         {
            TRHEXS( 70,  &fstat,  sizeof(fstat), "fstat");
            st_mode = (USHORT) fstat.st_mode;
         }
      #else
         if ((stat_rc = stat( tname, &fstat)) != -1)
         {
            TRHEXS( 70,  &fstat,  sizeof(fstat), "fstat");
            st_mode = (USHORT) fstat.st_mode;
         }
         else if (errno == EOVERFLOW)
         {
            //- retry with 64-bit stat (latest Linux kernels fail on regular)
            TRACES(( "EOVERFLOW, retry with stat64 ...\n"));
            if ((stat_rc = stat64( tname, &fstat64)) != -1)
            {
               TRHEXS( 70,  &fstat64,  sizeof(fstat64), "fstat64");
               st_mode = (USHORT) fstat64.st_mode;
            }
         }
      #endif

      if (stat_rc != -1)                        // name exists
      {
         if (S_ISREG( st_mode))                 // regular file ?
         {
            TRACES(("st_mode is 0x%4.4hx is regular FILE\n", st_mode));
            rc = TXTN_FILE;
         }
      }
      else
      {
         TRACES(( "stat errno: %d for '%s' = %s\n", errno, tname, strerror(errno)));
         if ((errno == EFBIG) ||                // must be a large-file
             (errno == EOVERFLOW))
         {
            rc = TXTN_FILE;
         }
         else
         {
            rc = ERROR_FILE_NOT_FOUND;
         }
      }
      if (rc == TXTN_FILE)
      {
         if (stripfile)
         {
            TxStripBaseName( tname);
         }
      }
   #else
      rc = DosQPathInfo(pspec,                  // Path string
                        FIL_STANDARD,           // Path data required
                        &fstat,                 // Path data buffer
                        sizeof(fstat));         // Path data buffer size
      TRACES(("DosQPathInfo '%s' FIL_STANDARD rc: %lu\n", pspec, rc));
      switch (rc)
      {
         case NO_ERROR:
            rc = DosQPathInfo( pspec,  FIL_QUERYFULLNAME, tname, TXMAXLN);
            TRACES(("DosQPathInfo '%s' FIL_QUERYFULLNAME rc: %lu\n", pspec, rc));
            TRACES(("resolved name from API: '%s'\n", tname));
            if ((fstat.attrFile & FILE_DIRECTORY) == 0)
            {
               if (stripfile)
               {
                  TxStripBaseName( tname);
               }
               rc = TXTN_FILE;
            }
            break;

         case ERROR_INVALID_PATH:               // wildcard or root
            if (stripfile)
            {
               TxStripBaseName( pspec);
               if ((strlen(pspec) != 2) || (pspec[1] != ':'))
               {
                  rc = DosQPathInfo( pspec,  FIL_QUERYFULLNAME, tname, TXMAXLN);
                  TRACES(("DosQPathInfo '%s' FIL_QUERYFULLNAME rc: %lu\n", pspec, rc));
                  TRACES(("resolved name from API: '%s'\n", tname));
               }
               else                             // Root directory
               {
                  strcpy( tname, pspec);
                  rc = TXTN_DIR;
               }
            }
            break;

         default:
            strcpy( tname, "");                 // return empty name
            break;
      }
   #endif
   TRACES(("tname: '%s'\n", tname));
   if (truename != NULL)                        // return the name ?
   {
      strcpy( truename, tname);
      if (((rc == TXTN_DIR) || stripfile) &&
          (truename[strlen(truename)-1] != FS_PATH_SEP))
      {
         strcat( truename, FS_PATH_STR);
      }
      TRACES(("true : '%s'\n", truename));
   }
   RETURN (rc);
}                                               // end 'TxTrueNameDir'
/*---------------------------------------------------------------------------*/


#if defined (UNIX)
/*****************************************************************************/
// Normalize relative path+filename specification (canonicalize)
/*****************************************************************************/
static char *TxFsRealPath                       // RET   resulting name (pfn)
(
   char               *pfn                      // INOUT path+filename (LINE)
)
{
   char               *s, *p;                   // search pointers
   TXLN                prefix;
   TXTS                token;

   ENTER();

   TRACES(( "Starting  pfn: '%s'\n", pfn));
   if (*pfn == FS_PATH_SEP)                     // first is root, absolute
   {
      strcpy( prefix, "");
   }
   else
   {
      getcwd( prefix, TXMAXLN);
   }
   if (*pfn == '.')
   {
      strcat( prefix, pfn +1);
   }
   else
   {
      if (*pfn != FS_PATH_SEP)                  // no root yet
      {
         strcat( prefix, FS_PATH_STR);
      }
      strcat( prefix, pfn);
   }
   strcpy( pfn, prefix);
   TRACES(( "Cleanup   pfn: '%s'\n", pfn));

   sprintf( token, "%c..", FS_PATH_SEP);        // parent directory token
   while ((s = strstr( pfn, token)) != NULL)
   {
      if (s == pfn)                             // pfn IS token /..
      {
         strcpy( pfn, "");                      // remove it
      }
      else                                      // token embedded in path
      {
         for (p = s-1; (p > pfn) && (*p != FS_PATH_SEP); p--)
         {
            //- search backward to path separator
         }
         memcpy( p, s+3, strlen(s+3) +1);       // remove "xx/.." residues
      }
      if (strlen(pfn) == 0)                     // found the root, empty now
      {
         strcpy( pfn, FS_PATH_STR);             // leave single root char
      }
      TRACES(( "Cleaned   pfn: '%s' using token '%s'\n", pfn, token));
   }
   sprintf( token, ".%c", FS_PATH_SEP);         // current directory token
   while ((s = strstr( pfn, token)) != NULL)
   {
      memcpy( s, s+2, strlen(s+2) +1);          // remove "./" residues
      TRACES(( "Cleaned   pfn: '%s' using token '%s'\n", pfn, token));
   }
   TRACES(( "Resulting pfn: '%s'\n", pfn));
   RETURN (pfn);
}                                               // end '*TxFsRealPath'
/*---------------------------------------------------------------------------*/
#endif

