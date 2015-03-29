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
// Operating system specific functions
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL

#include <txlib.h>

#if   defined (DOS32)

/*****************************************************************************/
// Wait for specified nr of msec (approximation, +/- 32 msec)
/*****************************************************************************/
void TxBusyWait
(
   ULONG               msec                     // IN    nr of msec to wait
)
{
   ULONG               t1 = (ULONG) clock();    // start time
   ULONG               t2;

   ENTER();

   do
   {
      t2 = (ULONG) clock();
   } while (t2 <= (t1 + msec));

   VRETURN();
}                                               // end 'TxBusyWait'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set keyboard mapping using FreeDOS keyb and .kl files, optional codepage
/*****************************************************************************/
ULONG TxSetNlsKeyboard                          // RET   result
(
   char               *spec,                    // IN    keyb file basename
   char               *cp                       // IN    codepage string or ""
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                line;                    // full-path to kl file
   TXTM                keyb;                    // kl filename / keyb cmd
   FILE               *kl;                      // .kl file

   ENTER();

   sprintf( keyb, "key\\%s.kl", spec);
   if ((kl = TxFindAndOpenFile( keyb, "PATH", line)) == NULL)
   {
      sprintf( keyb, "%s.kl", spec);
      kl = TxFindAndOpenFile( keyb, "PATH", line);
   }
   if (kl != NULL)
   {
      fclose( kl);
      TxExternalCommand(   "keyb /u");          // unload
      sprintf( keyb, "keyb %2.2s,%s,%s", spec, cp, line);
      TxExternalCommand(    keyb);              // set new
      TxExternalCommand(   "keyb");             // show
   }
   else
   {
      rc = ERROR_FILE_NOT_FOUND;
   }
   RETURN (rc);
}                                               // end 'TxSetNlsKeyboard'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Allocate DPMI compatible memory
/*****************************************************************************/
void *txDpmiAlloc                               // RET   PM linear address
(
   size_t              nr,                      // IN    number of items
   size_t              size,                    // IN    size per item
   short              *selector                 // OUT   PM selector
)
{
   void               *rc = NULL;
   union  REGS         regs;
   struct SREGS        sreg;

   ENTER();
   TRARGS(("nr of items: %d  size: %d\n", nr, size));

   if ((nr != 0) && (size != 0))
   {
      memset( &regs, 0, sizeof(regs));
      memset( &sreg, 0, sizeof(sreg));

      regs.w.ax = TXDX_DPMI_ALLOC;
      regs.w.bx = (USHORT) (((ULONG) (nr * size) -1) / 16) +1; // 16 byte paragraphs

      TRACES(("Alloc DPMI memory, paragraphs: %4.4hx\n", regs.w.bx));

      txDpmiCall( &regs, &sreg)

      if (regs.x.cflag == 0)
      {
         *selector   =  regs.w.dx;
         rc = (char *) (regs.w.ax << 4);
      }
      TRACES(("seg: %4.4hx  sel: %4.4hx  cflag: %hu\n",
               regs.w.ax, regs.w.dx, regs.x.cflag));
   }
   RETURN (rc);
}                                               // end 'txDpmiAlloc'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Free DPMI compatible memory
/*****************************************************************************/
void txDpmiFree
(
   short               selector                 // IN    PM selector
)
{
   union  REGS         regs;
   struct SREGS        sreg;

   ENTER();

   if (selector != 0)
   {
      memset( &regs, 0, sizeof(regs));
      memset( &sreg, 0, sizeof(sreg));

      TRACES(("free  DPMI memory, selector: %4.4hx\n", selector));
      regs.w.ax = TXDX_DPMI_FREEM;
      regs.w.dx = selector;
      txDpmiCall( &regs, &sreg)
   }
   VRETURN();
}                                               // end 'txDpmiFree'
/*---------------------------------------------------------------------------*/

#elif defined (WIN32)
// no WIN specific functions yet

#elif defined (UNIX)

/*****************************************************************************/
// Sleep for specified nr of msec
/*****************************************************************************/
void TxSleepMsec
(
   ULONG               msec                     // IN    nr of msec to wait
)
{
   struct timespec     requested;
   struct timespec     remaining;               // after EINTR from a signal

   ENTER();

   requested.tv_sec  =  msec / 1000;
   requested.tv_nsec = (msec % 1000) * 1000000;

   nanosleep( &requested, &remaining);
}                                               // end 'TxSleepMsec'
/*---------------------------------------------------------------------------                    */

/*************************************************************************************************/
// Translate (Linux) errno value to corresponding DFSee + DOS/OS2/WIN like RC
/*************************************************************************************************/
ULONG TxRcFromErrno                             // RET    TXW return-code
(
   int                 err                      // IN    error number (errno)
)
{
   ULONG               rc;                      // function return

   ENTER();

   switch (errno)
   {
      case 0:                          rc = NO_ERROR;                      break;
      case ENOENT:                     rc = ERROR_FILE_NOT_FOUND;          break;
      case EACCES:                     rc = ERROR_ACCESS_DENIED;           break;
      case EEXIST:                     rc = ERROR_FILE_EXISTS;             break;
      case EMFILE:    case ENFILE:     rc = ERROR_TOO_MANY_OPEN_FILES;     break;
      case ENXIO:     case ENODEV:
      case ENOTDIR:   case EISDIR:     rc = ERROR_INVALID_DRIVE;           break;
      case EFBIG:                      rc = ERROR_WRITE_FAULT;             break;
      case ENOSPC:                     rc = ERROR_DISK_FULL;               break;
      case EINVAL:                     rc = ERROR_INVALID_PARAMETER;       break;
      case ENAMETOOLONG:               rc = ERROR_FILENAME_EXCED_RANGE;    break;
      case EBADF:                      rc = ERROR_INVALID_HANDLE;          break;
      case ENOSYS:                     rc = ERROR_INVALID_PARAMETER;       break;
      case ENOMEM:                     rc = TX_ALLOC_ERROR;                break;
      default:                         rc = ERROR_GEN_FAILURE;             break;
   }
   TRACES(( "Translated errno value: %d to RC: %lu\n", errno, rc));
   RETURN (rc);
}                                               // end 'TxRcFromErrno'
/*-----------------------------------------------------------------------------------------------*/

#else
   static  TXF_OS2LFAPI  txfLargeAPIentryp = {NULL, NULL};
   static  BOOL          txfLargeAPItested = FALSE;

   #define TXF_DOSCALLS   ((PSZ)  "doscalls")   // name of dll with DOS calls
   #define TXF_DOSOPENL_ORDINAL   ((ULONG) 981)
   #define TXF_DOSSEEKL_ORDINAL   ((ULONG) 988)
   #define TXF_DOSSFSIZEL_ORDINAL ((ULONG) 989)

/*****************************************************************************/
// Test if OS2 large-file support (> 2GiB) is available; Fill entrypoints
/*****************************************************************************/
BOOL TxLargeFileApiOS2                          // RET   large file API's OK
(
   TXF_OS2LFAPI       *entrypoints              // OUT   LF-API entrypoints
)                                               //       or NULL
{
   BOOL                rc = FALSE;              // function return
   TXLN                dlmerror;                // one line of data
   HMODULE             doscalls = 0;            // handle DOSCALLS dll
   ULONG               dr;                      // API return code

   if (txfLargeAPItested == FALSE)
   {
      if ((DosLoadModule( dlmerror, TXMAXLN, TXF_DOSCALLS, &doscalls)) == NO_ERROR)
      {
         TRACES(("Txf DOSCALLS module handle: %lu\n", doscalls));

         dr = DosQueryProcAddr( doscalls, TXF_DOSOPENL_ORDINAL, NULL,
                      (PFN *) &(txfLargeAPIentryp.DosOpenLarge));

         TRACES(( "Txf DosOpenL,       ordinal %lu, rc: %lu  fn:%8.8lx\n",
                   TXF_DOSOPENL_ORDINAL, dr, txfLargeAPIentryp.DosOpenLarge));

         dr = DosQueryProcAddr( doscalls, TXF_DOSSEEKL_ORDINAL, NULL,
                      (PFN *) &(txfLargeAPIentryp.DosSeekLarge));

         TRACES(( "DosSetFilePtrL, ordinal %lu, rc: %lu  fn:%8.8lx\n",
                   TXF_DOSSEEKL_ORDINAL, dr, txfLargeAPIentryp.DosSeekLarge));

         dr = DosQueryProcAddr( doscalls, TXF_DOSSFSIZEL_ORDINAL, NULL,
                      (PFN *) &(txfLargeAPIentryp.DosSetFileSizeLarge));

         TRACES(( "DosSetFilePtrL, ordinal %lu, rc: %lu  fn:%8.8lx\n",
                   TXF_DOSSFSIZEL_ORDINAL, dr, txfLargeAPIentryp.DosSetFileSizeLarge));
      }
      else
      {
         TRACES(("Txf Error %lu loading DOSCALLS DLL: '%s'\n", rc, TXF_DOSCALLS));
      }
      txfLargeAPItested = TRUE;
   }

   if (entrypoints != NULL)
   {
      *entrypoints = txfLargeAPIentryp;
   }

   if ((txfLargeAPIentryp.DosOpenLarge != NULL) &&
       (txfLargeAPIentryp.DosSeekLarge != NULL)  )
   {
      rc = TRUE;
   }
   return( rc);
}                                               // end 'TxLargeFileApiOS2'
/*---------------------------------------------------------------------------*/
#endif
