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
// Generic TX get library and OS version information
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL

#include <txlib.h>
#include <txvers.h>                             // TXLIB version definitions

#if defined (UNIX)
   #include <sys/utsname.h>
#endif

static  char           txVersion[ TXMAXTM];     // static version string


#if   defined (DOS32)
   static TXTM         exVersion;               // DOS extender version info
#elif defined (WIN32)
#elif defined (LINUX)
#elif defined (DARWIN)                          // DARWIN MAC OS X (GCC)
   //- to be refined
#else
   #define TXKVL       32000                    // length of kernel to search
   #define TXKVS       "Internal revision "
   static TXTM         kernelrev = "";          // OS/2 kernel revision
#endif


/*****************************************************************************/
// Get operatingsystem version major*minor*micro and text (Win-9x major = 1)
/*****************************************************************************/
ULONG TxOsVersion                               // RET   number 000000..999999
(
   char               *descr                    // OUT   OS description or NULL
)
{
   ULONG               rc = 0;                  // function return
   TXTM                text;
   #if defined(UNIX)
      size_t           at;
      char            *env;
      struct utsname   un;
   #else
      double           mm;                      // major/minor double value
   #endif
   ULONG            major = 0;                  // major version component
   ULONG            minor = 0;                  // minor version component
   ULONG            micro = 0;                  // micro version component

   ENTER();

   #if   defined (WIN32)
      {
         OSVERSIONINFO osv;
         BOOL          win9x = FALSE;

         osv.dwOSVersionInfoSize = sizeof( OSVERSIONINFO);
         if (GetVersionEx( &osv))
         {
            major = (ULONG) osv.dwMajorVersion;
            minor = (ULONG) osv.dwMinorVersion;

            mm = (double) (major);
            if      (minor <  10) mm += (double) ((double) minor) / 10.0;
            else if (minor < 100) mm += (double) ((double) minor) / 100.0;
            else                  mm += (double) ((double) minor) / 1000.0;

            switch (osv.dwPlatformId)
            {
               case VER_PLATFORM_WIN32s:           strcpy( text, "Win32s API"); break;
               case VER_PLATFORM_WIN32_NT:         strcpy( text, "Unknown-NT");
                  switch (major)
                  {
                     case 3:
                     case 4:
                        strcpy( text, "Windows-NT");
                        break;

                     case 5:
                        switch (minor)
                        {
                           case 00: strcpy( text, "WinNT-2000"); break;
                           case 01: strcpy( text, "Windows-XP"); break;
                           case 02: strcpy( text, "WinNT-2003"); break;
                        }
                        break;

                     case 6:
                        switch (minor)
                        {
                           case 00: strcpy( text, "Win-Vista "); break;
                           case 01: strcpy( text, "Windows-7 "); break;
                        }
                        break;
                  }
                  if (strlen(osv.szCSDVersion) == 0)
                  {
                     strcpy( osv.szCSDVersion, "No Service Pack");
                  }
                  break;

               default:
                  strcpy( text, "Windows-9x");
                  win9x = TRUE;
                  break;
            }
            if (descr)
            {
               sprintf( descr, "%s %4.2lf    build %lu: %s",
                                text, mm, osv.dwBuildNumber, osv.szCSDVersion);
            }
            if (win9x)
            {
               major -= 3;                      // make Win9x different from NT
            }                                   // in the return value
         }
      }
   #elif defined (DOS32)
      {
         union  REGS   regs;

         TxxClearReg( regs);
         regs.h.al = 0x00;                      // include OEM info
         TxxDosInt21( regs, TXDX_DOS_GETVERSION);

         major = (ULONG) regs.h.al;
         minor = (ULONG) regs.h.ah;

         mm = (double) (major);
         if      (minor <  10) mm += (double) ((double) minor) / 10.0;
         else if (minor < 100) mm += (double) ((double) minor) / 100.0;
         else                  mm += (double) ((double) minor) / 1000.0;

         if (descr)
         {
            BOOL  vm86 = (strstr( txDosExtDpmiInfo(), "VM86") != NULL);
            BOOL  mmgr = (strstr( txDosExtDpmiInfo(), ": NO") == NULL);

            if (major == 20)                    // major version, OS/2 dosbox
            {
               strcpy( text, "32bit OS/2");
            }
            else
            {
               switch (regs.h.bh)               // check OEM indicator
               {
                  case 0xff:
                     if ((mm == 5.0) && (vm86) && (!mmgr))
                     {
                        strcpy( text, "Windows-NT/2000/XP");
                     }
                     else if (mm >= 8.0)
                     {
                        strcpy( text, "Microsoft Windows-ME");
                     }
                     else if (mm >= 7.0)
                     {
                        strcpy( text, "Microsoft Windows-9x");
                     }
                     else
                     {
                        strcpy( text, "Microsoft MS-DOS");
                     }
                     break;

                  case 0x00: strcpy( text, "IBM PC-DOS");              break;
                  case 0x01: strcpy( text, "Compaq-DOS");              break;
                  case 0xee: strcpy( text, "DR-DOS");                  break;
                  case 0xef: strcpy( text, "Novell-DOS");              break;
                  case 0xfd: strcpy( text, "FreeDOS");                 break;
                  default:   strcpy( text, "DOS, unknown OEM");        break;
               }
            }
            sprintf( descr, "DOS       %5.2lf     %s %2.2hx: %s",
                     mm, (vm86) ? (mmgr) ? "MemMgr" :
                                           "DosBox" :
                                           "OemVer", regs.h.bh, text);
         }
      }
   #elif defined (UNIX)
      if (uname( &un) == -1)                    // get uname descriptions
      {                                         // only likely failure seems
         if (descr)                             // systemcall not implemented
         {
            strcpy( descr, "Unixlike, but no 'uname' info!");
         }
      }
      else
      {
         TRACES(( "sysname : '%s'\n", un.sysname));
         TRACES(( "release : '%s'\n", un.release));
         TRACES(( "version : '%s'\n", un.version));
         TRACES(( "nodename: '%s'\n", un.nodename));
         if (descr)
         {
            if ((env = getenv("TERM")) != NULL)
            {
               strcpy( text, env);
            }
            else
            {
               strcpy( text, "unknown!");
            }
            if ((at = strspn( un.release, ".0123456789")) != 0)
            {
               un.release[at] = 0;              // terminate after number
            }
            sprintf( descr, "%-6.6s %8s env TERM=%s ", un.sysname, un.release, text);

         }
         sscanf(  un.release, "%lu.%lu.%lu", &major, &minor, &micro);
      }
      if (descr)
      {
         if ((env = getenv("LOGNAME")) != NULL)
         {
            strcpy( text, env);
         }
         else
         {
            strcpy( text, "someone");
         }
         strcat( descr, text);
         strcat( descr, " on ");
         strcat( descr, un.nodename);
      }
   #else
      {
         ULONG         sysinfo[QSV_VERSION_MINOR]; // major, minor, bootdrive etc

         if (DosQuerySysInfo( 1, QSV_VERSION_MINOR,  sysinfo,
                                 QSV_VERSION_MINOR * sizeof(ULONG)) == NO_ERROR)
         {
            major = sysinfo[QSV_VERSION_MAJOR -1];
            minor = sysinfo[QSV_VERSION_MINOR -1];

            if (major == 20)                    // major version
            {
               switch (minor)                   // minor version
               {
                  case 00:  mm = 2.0;  break;
                  case 10:  mm = 2.1;  break;
                  case 11:  mm = 2.11; break;
                  case 30:  mm = 3.0;  break;
                  case 40:  mm = 4.0;  break;
                  default:  mm = 4.50; break;
               }
            }
            if (descr)
            {
               if ((strlen(kernelrev) == 0) || (TxaOption('r')))
               {
                  TXLN          kernel;
                  FILE         *fp;
                  char          boot = (char) '@' + (char) sysinfo[QSV_BOOT_DRIVE-1];

                  sprintf( kernel, "%c:\\os2krnl", boot);
                  if ((fp = fopen( kernel, "rb")) == NULL)
                  {
                     strcat( kernel, "i");      // alternative (floppy) name
                     fp = fopen( kernel, "rb");
                  }
                  if (fp != NULL)
                  {
                     BYTE *buf     = TxAlloc( 1, TXKVL);
                     char *kver;

                     if (buf != NULL)
                     {
                        if (fread( buf, 1, TXKVL, fp) != 0)
                        {
                           if ((kver = TxMemStr( buf, TXKVS, TXKVL)) != NULL)
                           {
                              memcpy( text, kver + strlen(TXKVS), 10);
                              text[10] = 0;
                              TRACES(( "kernel string: '%s'\n", text));

                              for (kver = text; *kver; kver++)
                              {
                                 if (!isprint(*kver))
                                 {
                                    *kver = 0;
                                 }
                              }
                              sprintf( kernelrev, "OS2kernel: %s on drive %c:", text, boot);
                           }
                        }
                        TxFreeMem( buf);
                     }
                     fclose( fp);
                  }
                  if (strlen(kernelrev) == 0)
                  {
                     sprintf( kernelrev,
                              "Internal : %lu.%lu on drive %c:",
                              major, minor, boot);
                  }
               }
            }
         }
         if (descr)
         {
            sprintf( descr, "OS/2       %4.2lf     %s", mm, kernelrev);
         }
      }
   #endif
   rc = (major *10000) + (minor *100) + micro;  // numeric version

   RETURN (rc);
}                                               // end 'TxOsVersion'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return 15-character TXLIB version string
/*****************************************************************************/
char  *txVersionString                          // RET   15-char version string
(
   void
)
{
   sprintf( txVersion, "%s %s", TXLIB_V, TXLIB_C);
   return(  txVersion);
}                                               // end 'txVersionString'
/*---------------------------------------------------------------------------*/

#if defined (DOS32)

/*****************************************************************************/
// Return DOSEXTENDER version string
/*****************************************************************************/
char  *txDosExtVersion                          // RET   version string
(
   void
)
{
   #if defined (CAUSEWAY)
      return ("Causeway   3.52 (c) 1992-2000: M.E. Devore");
   #else
      return ("DOS32A     7.20 (c) 1996-2002: DOS/32 Advanced Team");
   #endif
}                                               // end 'txDosExtVersion'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return DOSEXTENDER DPMI info string
/*****************************************************************************/
char  *txDosExtDpmiInfo                         // RET   DPMI info string
(
   void
)
{
   union  REGS         regs;
   struct SREGS        sreg;

   memset( &regs, 0, sizeof(regs));
   memset( &sreg, 0, sizeof(sreg));
   regs.w.ax = TXDX_DPMI_VERS;

   txDpmiCall( &regs, &sreg)

   sprintf( exVersion, "%1.1hu.%02.2hu switchmode %s     swap: %s",
            regs.h.ah, regs.h.al,
           (regs.w.bx & 0x02) ? "REAL" : "VM86",
           (regs.w.bx & 0x04) ? "NO"   : "YES");
   return( exVersion);
}                                               // end 'txDosExtDpmiInfo'
/*---------------------------------------------------------------------------*/

#endif
