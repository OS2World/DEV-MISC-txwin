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
// TxLib reboot functions
//
// 22-02-2001 JvW Added native NT API reboot
// 30-08-2004 JvW Added native Linux  reboot

#include <txlib.h>                              // TxLib interface

#if   defined (UNIX)

   #include <signal.h>

#elif defined (DOS32)
   #define DOS_REVECT_OFF    0x0000             // offset  DOS reboot vector
   #define DOS_REVECT_SEG    0xffff             // segment DOS reboot vector

#elif defined (WIN32)

#define NTDLL_MODULENAME         "NTDLL.DLL"
#define NTDLL_SHUTDOWNSYS        "NtShutdownSystem"

typedef enum _SHUTDOWN_ACTION
{
   ShutdownNoReboot,
   ShutdownReboot,
   ShutdownPowerOff
} SHUTDOWN_ACTION;

//- Native NT shutdown (bypassing the WIN32 subsystem completely)
#if defined (__WATCOMC__)
typedef LONG (_System * NTSDS)(SHUTDOWN_ACTION ParAction);
#else
typedef LONG ( * NTSDS)(SHUTDOWN_ACTION ParAction);
#endif
#endif

/*****************************************************************************/
// Perform a shutdown (if possible) followed by a (COLD) boot of the system
/*****************************************************************************/
BOOL TxReboot                                   // RET   reboot OK
(
   BOOL                flush                    // IN    Flush buffers/FS-data
)
{
   #if   defined (WIN32)
      HANDLE           hToken;                  // handle to process token
      TOKEN_PRIVILEGES tkp;                     // ptr to token structure
      DWORD            Rc;
      TXLN             ErrMsg;
   #elif defined (DOS32)
      union  REGS      regs;
      struct SREGS     sreg;
      TXDX_RMINFO      txdx_rmi;
   #elif defined (UNIX)
   #else                                        // OS/2 32 bit
      HFILE            hf;                      // File Handle to DOS.SYS
      ULONG            dummy;                   // Dummy used in DosOpen
   #endif
   FILE               *log;

   ENTER();
   TxPrint("\nAutomatic reboot ");

   if ((log = TxQueryLogFile( NULL, NULL)) != NULL)
   {
      fprintf( log, "\nLogfile being closed automatically, just before forced reboot ...\n");
      fclose(  log);
   }
   #if defined (WIN32)
      //- Get current process token handle so we can get shutdown privilege.
      if (OpenProcessToken(GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &hToken))
      {
         //- Get the LUID for shutdown privilege, and get the privilege

         LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

         tkp.PrivilegeCount = 1;                // one privilege to set
         tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

         AdjustTokenPrivileges( hToken, FALSE, &tkp, 0,
                               (PTOKEN_PRIVILEGES) NULL, 0);

         if ((Rc = GetLastError()) == ERROR_SUCCESS)
         {
            HMODULE    hNTdll;
            NTSDS      fnShutDown;
            UINT       OldErrMode;

            OldErrMode = SetErrorMode (SEM_FAILCRITICALERRORS);

            if ((hNTdll = LoadLibrary( NTDLL_MODULENAME)) != 0)
            {
               fnShutDown = (NTSDS) GetProcAddress( hNTdll, NTDLL_SHUTDOWNSYS);

               if (fnShutDown != NULL)
               {
                  TxPrint("in progress ...\n");

                  //- Note: No-flush is not an option on Windows
                  TxPrint("Flushing the filesystem, please wait ...\n");

                  (void) (fnShutDown)(ShutdownReboot);
               }
               else
               {
                  TxPrint("not possible, no NtShutDownSystem entrypoint\n");
               }
               FreeLibrary( hNTdll);
            }
            else
            {
               TxPrint("not possible, NTDLL not available\n");
            }
            SetErrorMode (OldErrMode);

            //- Disable shutdown privilege, just to be nice
            tkp.Privileges[0].Attributes = 0;
            AdjustTokenPrivileges( hToken, FALSE, &tkp, 0,
                                  (PTOKEN_PRIVILEGES) NULL, 0);
         }
         else
         {
            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, Rc,
                          (DWORD) MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           ErrMsg, 128, NULL);
            TxPrint("not possible, probably insufficient rights:\n");
            TxPrint("AdjustTokenPrivileges error: %s", ErrMsg);
         }
      }
      else
      {
         Rc = GetLastError();
         FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, Rc,
                       (DWORD) MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        ErrMsg, 128, NULL);
         TxPrint("not possible, probably insufficient rights:\n");
         TxPrint("OpenProcessToken error: %s", ErrMsg);
      }
   #elif defined (DOS32)
      TxPrint("in progress ...\n");

      if (flush)
      {
         TxPrint("Flushing the filesystem, please wait ...\n");

         memset( &regs,  0, sizeof(regs));
         memset( &sreg,  0, sizeof(sreg));
         memset( &txdx_rmi, 0, sizeof(txdx_rmi));

         txdx_rmi.eax = 0x0d00;                 // Reset disk, usually hooked
                                                // by cache-programs ...
         regs.w.ax    = TXDX_DPMI_RMINT;        // simulate realmode INT
         regs.h.bl    = 0x21;                   // DOS interrupt 21
         regs.x.edi   = FP_OFF( &txdx_rmi);     // real mode register struct
         sreg.es      = FP_SEG( &txdx_rmi);

         txDpmiCall( &regs, &sreg);

         TxSleep( 3000);                        // extra wait 3 seconds, flush
      }

      memset( &regs,  0, sizeof(regs));
      memset( &sreg,  0, sizeof(sreg));
      memset( &txdx_rmi, 0, sizeof(txdx_rmi));

      txdx_rmi.cs  = (short) DOS_REVECT_SEG;    // CS:IP for reboot vector
      txdx_rmi.ip  = (short) DOS_REVECT_OFF;

      regs.w.ax    = TXDX_DPMI_RMFAR;           // simulate realmode FAR call
      regs.x.edi   = FP_OFF( &txdx_rmi);        // real mode register struct
      sreg.es      = FP_SEG( &txdx_rmi);

      txDpmiCall( &regs, &sreg);
   #elif defined (UNIX)
      TxPrint("in progress ...\n");

      if (flush)
      {
         TxPrint("Flushing the filesystem, please wait ...\n");
         sync();                                // flush inodes & buffers
         TxSleep( 1000);                        // allow another second ...
      }
      TxPrint( "Attempt automatic system shutdown and reboot ...\n");
      if (kill( 1, SIGINT) == -1)               // interrupt INIT process
      {
         TxPrint( "\nError in 'kill': %s\n", strerror(errno));
      }
      if (errno == EPERM)
      {
         TxPrint( "\nYou need to have ROOT privileges for shutdown/reboot!\n");
      }
      else
      {
         TxSleep( 60000);                       // should be down after 60 secs
      }
   #else                                        // OS/2 32 bit
      if (DosOpen("DOS$", &hf, &dummy, 0L,      // DOS.SYS with reboot IOctl
                   FILE_NORMAL, FILE_OPEN,
                   OPEN_ACCESS_WRITEONLY |      // must open BEFORE DosShutdown
                   OPEN_SHARE_DENYNONE   |      // or the Open call will block!
                   OPEN_FLAGS_FAIL_ON_ERROR,
                   0L) == NO_ERROR)
      {
         TxPrint("in progress ...\n");

         if (flush)
         {
            TxPrint("Flushing the filesystem, please wait ...\n");
            DosShutdown(0L);                    // flush file-systems
            TxSleep( 1000);                     // allow another second ...

            //- printf NOT guaranteed to reach the screen, but usually does :-)
            TxPrint( "Attempt automatic system shutdown and reboot ...\n");
         }
         DosDevIOCtl( hf, 0xd5, 0xab,           // reboot IOctl
                      NULL, 0, NULL,
                      NULL, 0, NULL);
         DosClose(hf);                          // unreachable code :-)

         if (!flush)                            // without the DosShutDown,
         {                                      // processing will continue
            TxSleep( 60000);                    // should be down after 60 secs
         }
      }
      else
      {
         TxPrint("not possible!\n");
         TxPrint("SYS1736: The DOS.SYS device driver cannot be found.\n");
      }
   #endif
   TxPrint( "\nReboot the system manually to complete the function.\n");
   BRETURN( FALSE);                             // should never get here ...
}                                               // end 'TxReboot'
/*---------------------------------------------------------------------------*/

