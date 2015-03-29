#ifndef TXLIB_H
#define TXLIB_H
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
//
// TX library interfaces
//
// Author: J. van Wijk
//
// Developed for LPT/DFSee utilities
//

//+++++++++++++++ OEM and version specific definitions

#if defined    (__WATCOMC__)                    // Auto TX target macro defs
   #if defined      (__NT__)
      #define WIN32
   #elif defined   (__OS2__)
      #define DEV32
   #elif defined   (__DOS__)
      #define DOS32
   #elif defined (__LINUX__)
      #define UNIX
      #define LINUX
   #endif
   #define TX_NATIVE_BIG_ENDIAN 0
#else
   #define UNIX
   #define DARWIN
   #if TARGET_RT_BIG_ENDIAN
      #define TX_NATIVE_BIG_ENDIAN 1
      #warning MAC platform detected as BIG ENDIAN!
   #else
      #define TX_NATIVE_BIG_ENDIAN 0
   #endif
#endif

#if defined (TXOEM) || defined (TXMIN)
   #undef  USEWINDOWING
   #undef  HAVEMOUSE
   #undef  HAVETHREADS
#else
   #if defined   (LINUX)
      #undef  HAVEMOUSE
   #elif defined (DARWIN)
      #undef  HAVEMOUSE
   #else
      #define HAVEMOUSE
   #endif
   #define USEWINDOWING
   #if defined (DEV32) || defined (WIN32)
      #define HAVETHREADS
   #else
      #undef  HAVETHREADS
   #endif
#endif


#include <stdarg.h>                             // variable vfprintf
#include <stdio.h>                              // std/file i/o
#include <stdlib.h>                             // std C stuff
#include <stddef.h>                             // threadid and errno
#include <string.h>                             // fast strings
#include <ctype.h>                              // type tests & conversions
#include <limits.h>                             // number limits
#include <time.h>                               // time functions
#include <errno.h>                              // UNIX like error handling

#ifndef DARWIN
   #include <malloc.h>                          // memory allocation
   #include <process.h>                         // threading etc
   #include <conio.h>                           // console  i/o
#endif

#if defined (UNIX)
   #include <strings.h>                         // POSIX string functions
   #include <sys/types.h>
   #include <sys/ioctl.h>
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <unistd.h>
#else
   #include <io.h>                              // special  i/o
   #include <direct.h>                          // directory functions
#endif

                                                // min and max macros
#if !defined(max)
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif


#define TX_ERROR                     200        // Generic TX error
#define TX_IGNORED                   201        // command ignored (empty)
#define TX_INVALID_FILE              202        // File cannot be opened
#define TX_INVALID_PATH              203        // Path not found
#define TX_ACCESS_DENIED             205        // File access denied (locked)
#define TX_INVALID_HANDLE            206        // Invalid handle specified
#define TX_INVALID_DATA              207        // Invalid data (structure)
#define TX_ALLOC_ERROR               208        // Memory allocation error
#define TX_SYNTAX_ERROR              210        // Syntax error in script/expr
#define TX_STL_SKIP                  211        // Break/Continue in progres
#define TX_INVALID_DRIVE             215        // Drive (letter) invalid
#define TX_PENDING                   217        // Function pending
#define TX_FAILED                    218        // Function failed
#define TX_WRITE_PROTECT             219        // Write protected (readonly)
#define TX_CMD_UNKNOWN               222        // cmd not known, unhandled
#define TX_NO_COMPRESS               223        // No compression achieved
#define TX_NO_INITIALIZE             224        // No compress init
#define TX_ABORTED                   225        // Function aborted
#define TX_BAD_OPTION_CHAR           226        // option char out of range
#define TX_TOO_MANY_ARGS             227        // too many arguments for cmd
#define TX_DISPLAY_CHANGE            228        // display mode changed
#define TX_APP_QUIT                  229        // quit script/application

#ifndef NOTRACE                                 // by default, include tracing
  #define DUMP 1
#endif

#if   defined (WIN32)
   #define WIN32_LEAN_AND_MEAN 1
   #include <windows.h>
   #include <winioctl.h>
   #if defined (_DEBUG)
      #define DUMP 1
   #endif
#elif defined (DOS32)                           // 32-bit extended DOS (WATCOM)
   #include <dos.h>
   #include <i86.h>
#elif defined (LINUX)                           // LINUX native        (WATCOM)
#elif defined (DARWIN)                          // DARWIN MAC OS X     (GCC)
   // no need for this, off_t is 64 bits by default on darwin, define a dummy
   #ifndef O_LARGEFILE
      #define O_LARGEFILE 0
   #endif
#else
   #ifndef DEV32
     #define DEV32
   #endif
   #define INCL_BASE
   #define INCL_NOPMAPI                         // avoid the real PM defs
   #define INCL_LONGLONG                        // Use 64bit compiler support
   #define INCL_WINSHELLDATA
   #define INCL_DOSDEVIOCTL                     // OS2 device definitions
   #include <os2.h>
   #define INCL_REXXSAA
   #include <rexxsaa.h>                         // REXX API's
#endif

#if defined (TXFTRC)                            // TXlib low impact Flow Trace
   #define TXFLOW(cond,nr,hex,str)                                         \
   if (cond)                                                               \
   {                                                                       \
      fprintf(stderr, "\n%12.12s %5u: ", __FILE__, __LINE__);              \
      fprintf(stderr, "%9lu %8.8lX %s", (ULONG) nr, (ULONG) hex, str);     \
   }
#else
   #define TXFLOW(cond,nr,hex,str)
#endif


#if   defined (WIN32)
   #define  TXFMODE                ""
   #define  USHORT                 unsigned short int
   #define  TXINVALID              ((ULONG) -1)
   #define  TXCURTHREAD            ((ULONG)(*(ULONG  *) _threadid))
   #define  TXHTHREAD              HANDLE
   #define  TXHFILE                HANDLE
   #define  TxRead(hf,b,z,w)       (ULONG) !ReadFile(hf,b,(z),w,NULL)
   #define  TxWrite(hf,b,z,w)      (ULONG) !WriteFile(hf,b,(z),w,NULL)
   #define  TxSetSize(hf,s)        0, SetFilePointer(hf,s,NULL,FILE_BEGIN); \
                                   SetEndOfFile(hf)
   #define  TxClose(hf)            (ULONG) !CloseHandle(hf)
   #define  TxDeleteFile(f)        remove(f)
   #define  TxSleep(msec)          Sleep(msec)
   #define  TxThreadPrioMin()      SetThreadPriority(GetCurrentThread(), \
                                      THREAD_PRIORITY_LOWEST)
   #define  TxThreadPrioLow()      SetThreadPriority(GetCurrentThread(), \
                                      THREAD_PRIORITY_BELOW_NORMAL)
   #define  TxThreadPrioStd()      SetThreadPriority(GetCurrentThread(), \
                                      THREAD_PRIORITY_NORMAL)
   #define  TxThreadPrioHigh()     SetThreadPriority(GetCurrentThread(), \
                                      THREAD_PRIORITY_ABOVE_NORMAL)
   #define  TxThreadPrioMax()      SetThreadPriority(GetCurrentThread(), \
                                      THREAD_PRIORITY_HIGHEST)
   #define  TxBeginThread(f,s,p)   (TXHTHREAD) _beginthread(f,s,p)
   #define  TxKillThread(ht)       TerminateThread(ht, 0)
   #define  FATTR_ARCHIVED         FILE_ATTRIBUTE_ARCHIVE
   #define  FATTR_SYSTEM           FILE_ATTRIBUTE_SYSTEM
   #define  FATTR_HIDDEN           FILE_ATTRIBUTE_HIDDEN
   #define  FATTR_READONLY         FILE_ATTRIBUTE_READONLY
   #define  FATTR_DIRECTORY        FILE_ATTRIBUTE_DIRECTORY
   #define  FATTR_LABEL            0x0008
   #define  FS_WILDCARD            "*"
   #define  FS_DIR_CMD             "dir"
   #define  FS_PATH_STR            "\\"
   #define  FS_PATH_SEP            '\\'
   #define  FS_ENVP_SEP            ';'
#elif defined (DOS32)
   #define  TXFMODE                ""
   #define  TXWORD                 regs.w
   #define  TXIWRD                 ireg.w
   #define  TXOWRD                 oreg.w
   typedef  unsigned char          BYTE;        // b
   typedef  unsigned long          BOOL;        // f
   typedef  unsigned short         USHORT;      // us
   typedef  unsigned long          ULONG;       // ul
   typedef  long                   LONG;        // l
   typedef  void                  *PVOID;       // p
   #define  LOBYTE(w)              ((BYTE)(          w        & 0xff))
   #define  HIBYTE(w)              ((BYTE)(((USHORT)(w) >> 8) & 0xff))
   #define  NO_ERROR               0
   #define  TRUE                   1
   #define  FALSE                  0
   #define  TXCURTHREAD            ((ULONG) 1)
   #define  TXHTHREAD              ULONG
   #define  TXHFILE                FILE *
   #define  TxRead(hf,b,z,w)       (ULONG) !(*w = fread(b,1,(z),hf))
   #define  TxWrite(hf,b,z,w)      (ULONG) !(*w = fwrite(b,1,(z),hf))
   #define  TxSetSize(hf,s)        0, fseek( hf, (long) (s), SEEK_SET); \
                                   chsize(fileno(hf),(long) (s))
   #define  TxClose(hf)            fclose(hf)
   #define  TxDeleteFile(f)        remove(f)
   #define  TxSleep(msec)          TxBusyWait(msec)
   #define  TxThreadPrioMin()
   #define  TxThreadPrioLow()
   #define  TxThreadPrioStd()
   #define  TxThreadPrioHigh()
   #define  TxThreadPrioMax()
   #define  TxBeginThread(f,s,p)   f(p)
   #define  TxKillThread(ht)
   #define  FATTR_ARCHIVED         _A_ARCH
   #define  FATTR_SYSTEM           _A_SYSTEM
   #define  FATTR_HIDDEN           _A_HIDDEN
   #define  FATTR_READONLY         _A_RDONLY
   #define  FATTR_DIRECTORY        _A_SUBDIR
   #define  FATTR_LABEL            _A_VOLID
   #define  FS_WILDCARD            "*.*"
   #define  FS_DIR_CMD             "dir"
   #define  FS_PATH_STR            "\\"
   #define  FS_PATH_SEP            '\\'
   #define  FS_ENVP_SEP            ';'
   #define  ERROR_FILE_NOT_FOUND           2
   #define  ERROR_PATH_NOT_FOUND           3
   #define  ERROR_TOO_MANY_OPEN_FILES      4    // OUT_OF_HANDLES
   #define  ERROR_ACCESS_DENIED            5
   #define  ERROR_INVALID_HANDLE           6    // INVALID_HANDLE
   #define  ERROR_INVALID_DRIVE           15
   #define  ERROR_NO_MORE_FILES           18
   #define  ERROR_WRITE_PROTECT           19
   #define  ERROR_NOT_READY               21    // DRIVE_NOT_READY
   #define  ERROR_CRC                     23    // DATA_ERROR
   #define  ERROR_SEEK                    25    // SEEK_ERROR
   #define  ERROR_SECTOR_NOT_FOUND        27    // MSG%SECTOR_NOT_FOUND
   #define  ERROR_WRITE_FAULT             29    // WRITE_FAULT
   #define  ERROR_READ_FAULT              30    // READ_FAULT
   #define  ERROR_GEN_FAILURE             31    // GENERAL_FAILURE
   #define  ERROR_SHARING_VIOLATION       32    // SHARING_VIOLATION
   #define  ERROR_LOCK_VIOLATION          33    // LOCK_VIOLATION
   #define  ERROR_WRONG_DISK              34    // INVALID_DISK_CHANGE
   #define  ERROR_NOT_SUPPORTED           50
   #define  ERROR_FILE_EXISTS             80
#elif defined (UNIX)                            // UNIX like    (WATCOM/GCC)
   #define  TX_DEFAULT_OPEN_MODE (S_IRUSR  | S_IWUSR | S_IRGRP  | S_IWGRP | S_IROTH | S_IWOTH)
   #define  TXFMODE                "b"
   typedef  unsigned char          BYTE;        // b
   typedef  unsigned long          BOOL;        // f
   typedef  unsigned short         USHORT;      // us
   typedef  unsigned long          ULONG;       // ul
   typedef  long                   LONG;        // l
   typedef  void                  *PVOID;       // p
   #define  LOBYTE(w)              ((BYTE)(          w        & 0xff))
   #define  HIBYTE(w)              ((BYTE)(((USHORT)(w) >> 8) & 0xff))
   #define  NO_ERROR               0
   #define  TRUE                   1
   #define  FALSE                  0
   #define  TXCURTHREAD            ((ULONG) 1)
   #define  TXHTHREAD              ULONG
   #define  TXHFILE                int
   #define  TxRead(hf,b,z,w)       (ULONG) !(*w = read(hf,b,(z)))
   #define  TxWrite(hf,b,z,w)      (ULONG) !(*w = write(hf,b,(z)))
#if defined (LINUX)
   #define  TxSetSize(hf,s)        0, lseek( hf, (off_t) (s), SEEK_SET); \
                                   chsize((hf),(long) (s))
#else
   #define  TxSetSize(hf,s)        0, lseek( hf, (off_t) (s), SEEK_SET); \
                                   ftruncate(hf, (off_t) (s))
#endif
   #define  TxClose(hf)            close(hf)
   #define  TxDeleteFile(f)        remove(f)
   #define  TxSleep(msec)          TxSleepMsec(msec)
   #define  TxThreadPrioMin()
   #define  TxThreadPrioLow()
   #define  TxThreadPrioStd()
   #define  TxThreadPrioHigh()
   #define  TxThreadPrioMax()
   #define  TxBeginThread(f,s,p)   f(p)
   #define  TxKillThread(ht)
   #define  FATTR_READONLY         0x01
   #define  FATTR_HIDDEN           0x02
   #define  FATTR_SYSTEM           0x04
   #define  FATTR_LABEL            0x08
   #define  FATTR_DIRECTORY        0x10
   #define  FATTR_ARCHIVED         0x20
   #define  FS_WILDCARD            "*"
   #define  FS_DIR_CMD             "ls -l"
   #define  FS_PATH_STR            "/"
   #define  FS_PATH_SEP            '/'
   #define  FS_ENVP_SEP            ':'
   #define  ERROR_FILE_NOT_FOUND           2
   #define  ERROR_PATH_NOT_FOUND           3
   #define  ERROR_TOO_MANY_OPEN_FILES      4    // OUT_OF_HANDLES
   #define  ERROR_ACCESS_DENIED            5
   #define  ERROR_INVALID_HANDLE           6    // INVALID_HANDLE
   #define  ERROR_INVALID_DRIVE           15
   #define  ERROR_NO_MORE_FILES           18
   #define  ERROR_WRITE_PROTECT           19
   #define  ERROR_NOT_READY               21    // DRIVE_NOT_READY
   #define  ERROR_CRC                     23    // DATA_ERROR
   #define  ERROR_SEEK                    25    // SEEK_ERROR
   #define  ERROR_SECTOR_NOT_FOUND        27    // MSG%SECTOR_NOT_FOUND
   #define  ERROR_WRITE_FAULT             29    // WRITE_FAULT
   #define  ERROR_READ_FAULT              30    // READ_FAULT
   #define  ERROR_GEN_FAILURE             31    // GENERAL_FAILURE
   #define  ERROR_SHARING_VIOLATION       32    // SHARING_VIOLATION
   #define  ERROR_LOCK_VIOLATION          33    // LOCK_VIOLATION
   #define  ERROR_WRONG_DISK              34    // INVALID_DISK_CHANGE
   #define  ERROR_NOT_SUPPORTED           50
   #define  ERROR_FILE_EXISTS             80
   #define  ERROR_INVALID_PARAMETER       87    // NET_INCORR_PARAMETER
   #define  ERROR_DEVICE_IN_USE           99    // DEVICE_IN_USE
   #define  ERROR_DISK_CHANGE            107    // DISK_CHANGE
   #define  ERROR_DRIVE_LOCKED           108    // DRIVE_LOCKED
   #define  ERROR_BROKEN_PIPE            109    // BROKEN_PIPE
   #define  ERROR_OPEN_FAILED            110    // ERROR_OPEN_FAILED
   #define  ERROR_DISK_FULL              112    // DISK_FULL
   #define  ERROR_NEGATIVE_SEEK          131    // APPL_DOUBLEFRAMECHAR
   #define  ERROR_SEEK_ON_DEVICE         132    // APPL_ARROWCHAR
   #define  ERROR_DISCARDED              157    // DISCARDED
   #define  ERROR_FILENAME_EXCED_RANGE   206    // NAME_TOO_LONG
   #define  ERROR_INVALID_PATH           253    // INVALID_PATH
#else
   #define  TXFMODE                ""
   #define  TXCURTHREAD         ((ULONG)(*(ULONG  *) _threadid))
   #define  TXHTHREAD              TID
   #define  TXHFILE                HFILE
   #define  TxRead(hf,b,z,w)       DosRead(hf,b,(z),w)
   #define  TxWrite(hf,b,z,w)      DosWrite(hf,b,(z),w)
   #define  TxSetSize(hf,s)        DosNewSize(hf,s)
   #define  TxClose(hf)            DosClose(hf)
   #define  TxDeleteFile(f)        remove(f)
   #define  TxSleep(msec)          DosSleep(msec)
   #define  TxThreadPrioMin()      DosSetPrty( PRTYS_THREAD, \
                                               PRTYC_IDLETIME, PRTYD_MAXIMUM, 0)
   #define  TxThreadPrioLow()      DosSetPrty( PRTYS_THREAD, \
                                               PRTYC_REGULAR,  PRTYD_MINIMUM, 0)
   #define  TxThreadPrioStd()      DosSetPrty( PRTYS_THREAD, \
                                               PRTYC_REGULAR,  PRTYD_MAXIMUM, 0)
   #define  TxThreadPrioHigh()     DosSetPrty( PRTYS_THREAD, \
                                       PRTYC_FOREGROUNDSERVER, PRTYD_MINIMUM, 0)
   #define  TxThreadPrioMax()      DosSetPrty( PRTYS_THREAD, \
                                       PRTYC_FOREGROUNDSERVER, PRTYD_MAXIMUM, 0)
   #define  TxBeginThread(f,s,p)   _beginthread(f, NULL, s, p)
   #define  TxKillThread(ht)       DosKillThread(ht)
   #define  FATTR_ARCHIVED         FILE_ARCHIVED
   #define  FATTR_SYSTEM           FILE_SYSTEM
   #define  FATTR_HIDDEN           FILE_HIDDEN
   #define  FATTR_READONLY         FILE_READONLY
   #define  FATTR_DIRECTORY        FILE_DIRECTORY
   #define  FATTR_LABEL            0x0008
   #define  FS_WILDCARD            "*"
   #define  FS_DIR_CMD             "dir"
   #define  FS_PATH_STR            "\\"
   #define  FS_PATH_SEP            '\\'
   #define  FS_ENVP_SEP            ';'
#endif

#define  TxAlloc(num,size)         calloc((size_t) (num), (size_t) (size))

#define  TxFreeMem(ptr)            TRACES(("TxFreeMem:%8.8lx\n",ptr)); \
                                   if (ptr != NULL) {free(ptr); ptr = NULL;}

typedef ULONG          TXHANDLE;

#ifndef SECTORSIZE
#define SECTORSIZE  512
#define SECTORBITS  4096
#endif

// Make sure a value is between a MIN and a MAX value
#define clamp(low, x, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

//- note: LLONG type is equivalent to OS/2 LONGLONG and __i64 types
typedef long long          LLONG;
typedef unsigned long long ULLONG;

// macro's to convert between sector count and 64 bits LLONG
#define TXSECT2LLONG(x)  ((LLONG)(((LLONG) (x)) <<  9))
#define TXLLONG2SECT(x)  ((ULONG)(((LLONG) (x)) >>  9))


//- equivalent to M$ 64-bit VCN (and usually storage compatible with LLONG)
typedef union xlong                             // eXtreme LONG value (64 bits)
{
   LLONG               ll;                      // as 64 bits quantity, signed
   ULLONG              ull;                     // as 64 bits quantity, unsigned
   struct
   {
      ULONG            lo;                      // low  32 bits, unsigned
      ULONG            hi;                      // high 32 bits, unsigned
   };
} XLONG;                                        // end of struct "xlong"

// macro's to convert sector count to hi/lo part of XLONG/VCN (64 bits)
#define TXXLONG_LO(x)    ((ULONG)( (x)    <<  9) )
#define TXXLONG_HI(x)    ((ULONG)( (x)    >> 23) )
#define TXXLONG2SECT(x)  ((ULONG)(((x).lo >>  9) | ((x).hi << 23)))


//- create multiples of 2/4/8/16/32/64/128/256/512 for any given number
#define TXMULTI02(n)    (((n) + 0x001) & ~0x001)
#define TXMULTI04(n)    (((n) + 0x003) & ~0x003)
#define TXMULTI08(n)    (((n) + 0x007) & ~0x007)
#define TXMULTI16(n)    (((n) + 0x00f) & ~0x00f)
#define TXMULTI32(n)    (((n) + 0x01f) & ~0x01f)
#define TXMULTI64(n)    (((n) + 0x03f) & ~0x03f)
#define TXMULT128(n)    (((n) + 0x07f) & ~0x07f)
#define TXMULT256(n)    (((n) + 0x0ff) & ~0x0ff)
#define TXMULT512(n)    (((n) + 0x1ff) & ~0x1ff)


//- Definitions to make a fixed-size bitfield-structure type of 2 or 4 bytes
#if defined (WIN32) || defined (__WATCOMC__)    // force 16-bit on MSC++ and WATCOM
   typedef unsigned short BITFIELD16;
#else
   typedef unsigned       BITFIELD16;
#endif
typedef    unsigned       BITFIELD32;

//- determine if a number is a power of 2 (non-power-of-2 has >= two "1" bits)
#define TxIsPowerOf2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))


//- TX-BITMAP derived from C-FAQ: use array of ULONG, address individual bits

// size of bit-array items
#define TXBULBITS           32
#define TXBULSIZE(x)  (((x) + TXBULBITS -1) / TXBULBITS)

// dynamic declare x-size TX bit array
#define TXBITALLOC(x)      ((ULONG *) TxAlloc(TXBULSIZE(x),sizeof(ULONG)))

#define TXBITMASK(b)     (1 << ((b) % TXULBITS))
#define TXBITSLOT(b)     (      (b) / TXULBITS)

#define TXBITSET(a,b)    ((a)[TXBITSLOT(b)] |=  TXBITMASK(b))
#define TXBITTEST(a,b)   ((a)[TXBITSLOT(b)] &   TXBITMASK(b))
#define TXBITCLEAR(a,b)  ((a)[TXBITSLOT(b)] &= ~TXBITMASK(b))
#define TXBITFLIP(a,b)   ((a)[TXBITSLOT(b)] ^=  TXBITMASK(b))


#include <txcon.h>                              // TX ANSI console and tracing


typedef enum txsetreset
{
   TX_SET,
   TX_RESET,
   TX_TOGGLE
} TXSETRESET;                                   // end of enum "txsetreset"


#if defined (WIN32)
 #define TXVER_WINDOWS_9X       10000
 #define TXVER_WINNT_NT4        40000
 #define TXVER_WINNT_WIN2K      50000
 #define TXVER_WINNT_WINXP      50100
 #define TXVER_WINNT_WS03       50200
 #define TXVER_WINNT_WIN6       60000
 #define TXVER_WINNT_VISTA      60000
 #define TXVER_WINNT_WS08       60000
 #define TXVER_WINNT_LONGHORN   60000
 #define TXVER_WINNT_WIN7       60100
#endif

// Get operatingsystem version major*minor*micro and text (Win-9x major = 1)
ULONG TxOsVersion                               // RET   number 000000..999999
(
   char               *descr                    // OUT   OS description or NULL
);

// Generic definition for a linked list of strings (like warnings/errors)
typedef struct txstrlist
{
   char               *string;                  // payload
   struct txstrlist   *next;                    // next element
} TXSTRLIST;                                    // end of struct "txstrlist"

// Add string to string-list (allocate memory)
char *TxStrListAdd                              // RET   added string
(
   TXSTRLIST         **list,                    // IN    address of list (head)
   char               *str                      // IN    string to add
);

// Free string-list (free memory)
void  TxStrListFree
(
   TXSTRLIST         **list                     // IN    address of list (head)
);

#pragma pack(1)                                 // byte packing
typedef struct txfs_ebpb                        // ext. boot parameter block
{                                               // at offset 0x0b in bootsector
   USHORT              SectSize;                // 0B bytes per sector
   BYTE                ClustSize;               // 0D sectors per cluster
   USHORT              FatOffset;               // 0E sectors to 1st FAT
   BYTE                NrOfFats;                // 10 nr of FATS     (FAT only)
   USHORT              RootEntries;             // 11 Max entries \ (FAT only)
   USHORT              Sectors;                 // 13 nr of sectors if <  64K
   BYTE                MediaType;               // 15 mediatype (F8 for HD)
   USHORT              FatSectors;              // 16 sectors/FAT (FAT only)
   USHORT              LogGeoSect;              // 18 sectors/Track
   USHORT              LogGeoHead;              // 1a nr of heads
   ULONG               HiddenSectors;           // 1c sector-offset from MBR/EBR
   ULONG               BigSectors;              // 20 nr of sectors if >= 64K
} TXFS_EBPB;                                    // last byte is at offset 0x23


#if defined (DOS32)                             // Dos interface defs
   typedef struct txfs_gparm                    // IoCtl structure
   {
      BYTE             special;                 // special functions
      BYTE             devtype;                 // device type
      USHORT           devattr;                 // device attributes
      USHORT           cylinders;               // Nr of cylinders
      BYTE             density;                 // Media density
      TXFS_EBPB        eb;                      // Extended BPB
      BYTE             reserved[32];            // used by WIN9x !!
   } TXFS_GPARM;                                // end of struct "txfs_gparm"


#endif


// Execute standard commands
ULONG TxStdCommand
(
   void                                         // parameters through TXA
);

// Set current thread priority Min/Low/Std/High/maX
char TxSetPriority                              // RET   resulting priority
(
   char                prio                     // IN    priority M/L/S/H/X/Q
);


//--------- TXlib SCRIPT/REXX subcommand definitions and functions -----------

typedef ULONG (* TX_NATIVE_CALLBACK)            // modelled after MultiCmd
(
   char               *cmd,                     // IN    command string (input)
   ULONG               lnr,                     // IN    linenumber or 0
   BOOL                echo,                    // IN    echo   before each cmd
   BOOL                prompt,                  // IN    prompt after  each cmd
   BOOL                quiet                    // IN    screen-off during cmd
);

// Test if script exists, is REXX, return parameter description and full-name
BOOL TxsValidateScript                          // RET   script exists
(
   char               *sname,                   // IN    script name
   BOOL               *rexx,                    // OUT   script is REXX
   char               *pdesc,                   // OUT   param description or NULL
   char               *fname                    // OUT   full scriptname or NULL
);

// Run native TXS script using callback, params & options from parsed RUN cmd
ULONG TxsNativeRun
(
   char               *name,                    // IN    TXS script to execute
   TX_NATIVE_CALLBACK  subcom                   // IN    subcommand handler
);

// Substitute $n parameters in a string by supplied values; n = 0..9 (maximum)
ULONG txsSubstituteParams                       // RET   nr of substitutions
(
   char               *string,                  // IN    base string
   char              **values,                  // IN    values to substitute
   int                 count,                   // IN    number of values
   int                 size,                    // IN    size of output buffer
   char               *result                   // OUT   substituted (MAXLIN)
);


#if   defined (DEV32)
typedef ULONG (_System * TX_SUBCOM_CALLBACK)
(
   PRXSTRING           cmd,                     // IN    command string
   PUSHORT             err_flag,                // OUT   ERROR/FAILURE
   PRXSTRING           rv                       // OUT   string retcode
);

// Execute a REXX command script using a subcommand environment
ULONG TxsRexxRun
(
   char               *name,                    // IN    REXX proc to execute
   char               *args,                    // IN    Arguments
   char               *envname,                 // IN    REXX environment name
   TX_SUBCOM_CALLBACK  subcom                   // IN    subcommand handler
);

// Set string variable in the REXX variable pool using printf syntax
ULONG TxsRexxSetnum
(
   char              *name,                     // IN    Name of variable
   ULONG              ulv,                      // IN    value for variable
   char              *fmt                       // IN    format strings
);

// Set a string variable in the REXX variable pool
ULONG TxsRexxSetvar
(
   char              *name,                     // IN    Name of variable
   void              *value,                    // IN    ptr to value buffer
   ULONG              length                    // IN    length of value
);
#endif


//--------- TXlib generic assignment and expression parser and executor

// Strings are null-terminated (for debug) but may contain embedded NULLs too
typedef struct txsvalue                         // TX-script numeric/string
{
   LLONG               num;                     // numeric integer value
   ULONG               mem;                     // allocated string memory
   ULONG               len;                     // exact string length
   char               *str;                     // string value, terminated
   BOOL                defined;                 // value is defined
} TXSVALUE;                                     // end of struct "txsvalue"

// initial values for an UNDEF value, and a NUM 0
#define TXSUNDEF  {0,0,0,NULL,FALSE}            // undefined, all zero (alloc)
#define TXSNUM_0  {0,0,0,NULL,TRUE}             // numeric zero, defined

// size to allocate, always in multiples of TXTT, and with some spare room
#define TXSVSIZE(s)   ((ULONG)(((((s)+20)/TXMAXTT)+1) * TXMAXTT))

typedef enum txseresult
{
   TXSEXPR_INITIAL,
   TXSEXPR_ASSIGNMENT,                          // top-level is an assignment
   TXSEXPR_EXPRESSION,                          // normal expresion
   TXSEXPR_SYNTAX_ERR                           // syntax error
} TXSERESULT;                                   // end of enum "txseresult"

typedef struct txsparsestate                    // TX-script parse state
{
   char               *start;                   // start of whole expression
   ULONG               linenr;                  // linenr for start of expr
   BOOL                syntax;                  // syntax-check only, no execute
   BOOL                verbose;                 // output operation code
   TXSVALUE            value;                   // expression result value
   TXSERESULT          result;                  // expression status
   int                 endpos;                  // position after expression
   TXTM                msg;                     // explanation for error
} TXSPARSESTATE;                                // end of struct "txsparsestate"

// Character that signal substitution required
#define TXSL_RESOLV_CHARS "{$"

// Replace each variable reference like $var in string, with their value
ULONG txsResolveVariables                       // RET   INVALID_DATA if too large
(
   char               *string,                  // IN    string to be resolved
   char               *resolved,                // OUT   resolved string
   ULONG               length                   // IN    max length of resolved
);

// Replace each expression like {4 * $sz} or $var in string, with their value
// Expressions must either start with a variable $xx, or be enclosed as {expr}
ULONG txsResolveExpressions
(
   char               *ref,                     // IN    ref string for errors
   ULONG               lnr,                     // IN    linenr, start of expr
   BOOL                syntax,                  // IN    syntax-check only
   char               *string,                  // INOUT string with expressions
   ULONG               length,                  // IN    max length of string
   char               *errtxt                   // INOUT error text (or NULL)
);                                              //       minimal length 80!

// Handle a TX-script expression: top-level, non recursive
ULONG txsEvaluateExpr                           // RET   function result
(
   char               *ref,                     // IN    start of line
   ULONG               lnr,                     // IN    linenr, start of expr
   BOOL                syntax,                  // IN    syntax-check only
   char              **pos,                     // INOUT position in string
   TXSPARSESTATE      *pstate                   // OUT   result value and state
);


// Get long-long-integer representation for number-string
ULONG txsParseLlongNumber                       // RET   function result
(
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   LLONG              *value                    // OUT   result value
);


#define TXS_POP   '('                           // Parenthesis open
#define TXS_PCL   ')'                           // Parenthesis close
#define TXS_BOP   '{'                           // Bracket open
#define TXS_BCL   '}'                           // Bracket close
#define TXS_NONE  0xff                          // not a known token

// Skip white-space in input string, and return next Token character
char txsNextToken                               // RET   next token in input
(
   char              **pos,                     // INOUT position in string
   int                *len                      // OUT   length of token
);


// Determine boolean value from a TXS value, convert value in-situ too
BOOL txsVal2Bool                                // RET   boolean value
(
   TXSVALUE           *value                    // INOUT TXS value
);

// Determine numeric value from a TXS value, convert value in-situ too
LLONG txsVal2Num                                // RET   numeric value
(
   TXSVALUE           *value                    // INOUT TXS value
);

// Determine string value from a TXS value, convert value in-situ too
char *txsVal2Str                                // RET   string value or NULL
(
   TXSVALUE           *value,                   // INOUT TXS value
   int                 digits,                  // IN    number of digits, or 0
   char               *format                   // IN    Hex, Dec, etc or NULL
);

// Initialize a TXS value to numeric ZERO, freeing any string memory
void txsVal2Zero
(
   TXSVALUE           *value                    // INOUT TXS value
);

// Handle error message, printf-like interface
void TxsExprError
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *location,                // IN    location in ps->start
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
);

//--------- TXlib variable pool and variable-resolving interfaces

// Note: indexes are numerical for arrays and strings for hashes

#define TXSCONSTANT   0xfffffffe
#define TXSNOINDEX    0xffffffff

// resolve variable value, scalar or array with numeric index
// return TX_INVALID_HANDLE on any scalar/array mismatch
typedef ULONG (* TXS_VAR_CALLBACK)
(
   char               *name,                    // IN    variable name
   ULONG               index,                   // IN    index or TXS...
   TXSVALUE           *prompt                   // OUT   variable value
);

// Initialize variable pool and set host callback
ULONG txsInitVariablePool
(
   TXS_VAR_CALLBACK    hostCb                   // IN    callback function for
);                                              //       $_xxx host variables

// Terminate variable pool and reset host callback
void txsTermVariablePool
(
   void
);

// Add a local level to the variable pool, empty
ULONG txsAddVarPoolLevel
(
   void
);

// Terminate the most local variable pool level (could be global!)
void txsDelVarPoolLevel
(
   void
);

// Get constant numeric value, if defined for TXLIB or by application-callback
ULONG txsGetConstant                            // RET   function result
(
   char               *name,                    // IN    possible  constant name
   TXSVALUE           *value                    // OUT   resulting constant value
);

// Get value for named variable and optional (array) index
// return TX_INVALID_HANDLE on any scalar/array mismatch
ULONG txsGetVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   char               *name,                    // IN    variable name
   TXSVALUE           *index,                   // IN    array/hash index or NULL
   TXSVALUE           *value                    // OUT   new variable value
);

// Set value for named variable and optional (array) index, no host variables!
// return TX_INVALID_HANDLE on any scalar/array mismatch
ULONG txsSetVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   char               *name,                    // IN    variable name
   TXSVALUE           *index,                   // IN    array/hash index or NULL
   TXSVALUE           *value                    // IN    new variable value
);

// Undefine (delete) named scalar or array variable,  optional (array) index
ULONG txsDelVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   char               *name,                    // IN    variable name
   TXSVALUE           *index                    // IN    array/hash index or NULL
);

// Dump variable pool contents for debugging purposes to TxPrint destination
void txsDumpVarPool
(
   void
);


//--------- TXlib TxFileTree, recursive tree itterator ---------------------
// Recursive findfile with call-back function to operate on found files
// Modelled after SysFileTree implementation from REXXUTIL
// Major functional changes made:
//
//   - REXX interfaces (stem, variablepool) changed to normal 'C'
//   - Action on each matching file implemented as a call-back function with
//     full "true-filename" and OS-neutral TXFILEFIND structure as parameters
//   - Source and Destination mask use different syntax (+,-,* and ARSHD)
//
//     options  - Any combo of the following:
//                 'B' - Search for files and directories. (default)
//                 'D' - Search for directories only.
//                 'F' - Search for files only.
//                 'S' - Recursively scan subdirectories.
//
// Includes the lower-level TxFindFirstFile() and TxFindNextFile() APIs too


typedef struct ntime                            // NT time value (64 bits)
{
   ULONG               lo;                      // low  32 bits
   ULONG               hi;                      // high 32 bits
} NTIME;                                        // end of struct "ntime"


#define MAX_S_TIME    0xbf80                    // maximum valid time value

typedef struct s_time
{
   BITFIELD16         twosecs : 5;
   BITFIELD16         minutes : 6;
   BITFIELD16         hours   : 5;
} S_TIME;

#define MAX_S_DATE    0x7f9f                    // maximum valid date (Y 2043)
                                                // date 0x0000 is invalid too!

typedef struct s_date
{
   BITFIELD16         day     : 5;
   BITFIELD16         month   : 4;
   BITFIELD16         year    : 7;
} S_DATE;


typedef union s_datim                           // combined date/time int
{
   S_TIME              t;
   S_DATE              d;
   USHORT              u;
} S_DATIM;                                      // end of union "s_datim"


typedef struct txfilefind
{
   time_t              cTime;                   // creation   date-time
   time_t              aTime;                   // last acces date-time
   time_t              wTime;                   // last-write date-time
   ULONG               fAttr;                   // file attributes
   LLONG               fSize;                   // 64-bits filesize
   TXLN                fName;                   // ASCII filename
} TXFILEFIND;                                   // end of struct "txfilefind"


typedef ULONG (* TXTREE_CALLBACK)               // RET   callback result
(
   char               *fullname,                // IN    full filename
   TXFILEFIND         *fileinfo,                // IN    attrib, EA info etc
   void               *calldata                 // INOUT callback data
);



// Find first file matching the specified (wildcard) name
ULONG TxFindFirstFile                           // RET   result
(
   char               *fspec,                   // IN    file specification
   TXHANDLE           *handle,                  // OUT   filefind handle
   TXFILEFIND         *found                    // OUT   found file details
);

// Find next file matching the specified (wildcard) name
ULONG TxFindNextFile                            // RET   result
(
   TXHANDLE            handle,                  // IN    filefind handle
   TXFILEFIND         *found                    // OUT   found file details
);

// Close FindFirst/FindNext session for specified handle
ULONG TxFindClose                               // RET   result
(
   TXHANDLE            handle                   // IN    filefind handle
);

// Simple (example) callback for TxFileTree, listing files date/attr/full-name
ULONG txFileTreeOneFile
(
   char               *fname,                   // IN    full filename
   TXFILEFIND         *finfo,                   // IN    file information
   void               *cdata                    // IN    Callback data
);

// Simple (example) callback for FileTree, listing files/dirs without path
ULONG txFileTreeNoPath
(
   char               *fname,                   // IN    full filename
   TXFILEFIND         *finfo,                   // IN    file information
   void               *cdata                    // IN    Callback data
);

// Split multiple wildcard in BASE and INCLUDE array
char **txWildcard2Filter
(
   char               *base,                    // INOUT base specification
   char               *wildcard,                // IN    wildcard (multi)
   char              **fa,                      // IN    filter array
   int                 size                     // IN    size of filter array
);

// Free filter array memory
char **txFreeFilter                             // RET   NULL
(
   char              **filter,                  // IN    filter array
   int                 size                     // IN    max number of entries
);

// Itterate through files and directories matching filespec and attributes
ULONG TxFileTree
(
   char               *fspec,                   // IN    Filespec to match
   char               *fopts,                   // IN    Find options
   char               *fattr,                   // IN    Select attributes
   char               *mattr,                   // IN    Modify attributes
   char              **exclude,                 // IN    Exclude file-spec
   TXTREE_CALLBACK     callback,                // IN    function to call back
   void               *calldata                 // IN    callback data
);



//--------- TXlib command execution with full output redirection ------------

// Execute an external command, output redirected to TxPrint
ULONG TxExternalCommand                         // RET   command exit-code (RC)
(
   char              *cmd                       // IN    command to execute
);



//--------- 32-bit CRC implementation --------------------------------------
// CRC32 interface; derived from textbook:
//       'C' programmers guide to NETBIOS
//
// Calculate CRC-32 value for supplied data
ULONG TxCrc32
(
   void               *pcDat,                   // IN    pointer to data
   ULONG               ulSize                   // IN    length of data
);

// Calculate sector checksum, using HPFS algorithm and fixed 512 byte size
ULONG TxHpfsCheckSum                            // RET   HPFS style crc
(
   char               *data                     // IN    data sector
);

// Calculate 32-bit CRC value using LVM compatible algorithm and polynom
ULONG TxCalculateLvmCrc
(
   BYTE               *area,                    // IN    data area needing CRC
   ULONG               size                     // IN    size of the data area
);

//--------- FileSystem functions -------------------------------------------

//- Get double MiB value, no special rounding preparation, eXact value
#define TXSMIB(secs,bps) ((double) (secs == 0) ? ((double) 0.0) :           \
                                 ( ((double) (secs)                         \
                                 * ((double) (((bps) != 0) ? (bps) : 512))  \
                                 / ((double) 1048576))))

//- Get double KB value, no special rounding preparation
#define TXS2KB(secs,bps) ((double) ((double) (secs)                         \
                                 * ((double) (((bps) != 0) ? (bps) : 512))  \
                                 / ((double) 1024)))


// Perform a shutdown (if possible) followed by a (COLD) boot of the system
BOOL TxReboot                                   // RET   reboot OK
(
   BOOL                flush                    // IN    Flush buffers/FS-data
);


// Set critical error handling to auto-FAIL ==> application gets a "not-OK" rc
// Calls to this may be nested, autofail is ON until matching fail==false call
void TxFsAutoFailCriticalErrors
(
   BOOL                fail                     // IN    autofail on CritErr
);

// Set (DOS) critical error handling to use TX-handler (MessageBox or AutoFail)
void TxFsSetCritErrHandler
(
   BOOL                install                  // IN    install (or deinstall)
);

#define  TXFSV_HD      0x0000
#define  TXFSV_FLOP    0x0001
#define  TXFSV_LAN     0x0002
#define  TXFSV_CD      0x0004
#define  TXFSV_ALL     0x0007
#define  TXFSV_REM     0x0100                   // removables only
// Build string with present volumes
int TxFsVolumes                                 // RET   nr of drives listed
(
   ULONG               flags,                   // IN    FLOPPY/LAN/CD select
   char               *vols                     // OUT   Present volumes
);

// Get volumelabel for specified driveletter, or -not-ready- if not accessible
char *TxFsVolumeLabel                           // RET   label or "-not-ready-"
(
   char               *drive,                   // IN    Drive specification
   char               *label                    // INOUT Label string (min 12)
);

// Show volume/filesystem info for all registred drive-letters
void TxFsDrivemap
(
   char               *lead,                    // IN    Leading text string
   ULONG               flags                    // IN    FLOPPY/LAN/CD select
);

// Show volume/filesystem info on a single output line using TxPrint
ULONG TxFsShow
(
   char               *lead,                    // IN    Leading text string
   char               *drive                    // IN    Drive specification
);


// Determine attached fstype, e.g. HPFS for specified drive
BOOL TxFsType                                   // RET   FS type resolved
(
   char               *drive,                   // IN    Drive specification
   char               *fstype,                  // OUT   Attached FS type
   char               *details                  // OUT   details (UNC) or NULL
);

// Eject a removable medium specified by driveletter
ULONG TxFsEjectRemovable
(
   char               *drive                    // IN    Driveletter to eject
);

// Determine if a driveletter represents a removable medium/device
BOOL TxFsIsRemovable                            // RET   drive is removable
(
   char               *drive                    // IN    Driveletter to test
);

// Determine free and total-available space (sectors) on specified drive
ULONG TxFsSpace
(
   char               *drive,                   // IN    Drive specification
   ULONG              *sfree,                   // OUT   Free sectors
   ULONG              *stotal,                  // OUT   Total sectors
   USHORT             *bpsect                   // OUT   Sectorsize
);

#define TXTN_DIR     0
#define TXTN_FILE   80
// Query type for specified path and resolve truename (canonical)
// Directorynames (for DIR or stripped filename) will ALWAYS end in a '\'
ULONG  TxTrueNameDir                            // RET   0 = Dir, 80 = File;
(                                               //       others are OS errors
   char               *path,                    // IN    path specification
   BOOL                stripfile,               // IN    strip filename part
   char               *true                     // OUT   true filename or NULL
);


// Check path, optionaly create it in a recursive fashion
ULONG TxMakePath                                // RET   result
(
   char               *filename                 // IN    Directory-path
);

// Make an 8-dot-3 compatible copy of supplied path/filename string (no spaces)
ULONG TxMake8dot3                               // RET   length converted name
(
   char               *filename,                // IN    relative/absolute path
   char               *fname8d3                 // OUT   8dot3 compatible buf
);                                              //       with size >= filename

// Function : count number of non-wildcard chars in string
int TxStrWcnt                                   // RET   nr of non-wild chars
(
    char              *s                        // IN    candidate string
);


// wildcard compare of candidate to template string
int TxStrWcmp                                   // RET   negative if no match
(
    char              *cs,                      // IN    candidate string
    char              *ts                       // IN    template string
);


// See TxStrWcmp; case-insensitive
int TxStrWicmp                                  // RET   negative if no match
(
    char              *cs,                      // IN    candidate string
    char              *ts                       // IN    template string
);

// Apple MAC and OS X / HFS type string, BIG-ENDIAN pascal style unicode string
typedef struct s_macuni_string
{
   USHORT              length;                  // big-endian!
   USHORT              unicode[255];            // 2-byte chars, big-endian
} S_MACUNI_STRING;                              // end of struct "s_macuni_string"


#if TX_NATIVE_BIG_ENDIAN == 1
   //- platform is big-endian, need to convert little-endian values
   #define TxBE16(v) v
   #define TxBE32(v) v
   #define TxBE64(v) v
   #define TxLE16(v) TxEswap16(v)
   #define TxLE32(v) TxEswap32(v)
   #define TxLE64(v) TxEswap64(v)
#else
   //- platform is little-endian, need to convert big-endian values
   #define TxBE16(v) TxEswap16(v)
   #define TxBE32(v) TxEswap32(v)
   #define TxBE64(v) TxEswap64(v)
   #define TxLE16(v) v
   #define TxLE32(v) v
   #define TxLE64(v) v
#endif


// Macro's to perform endian swapping on unsigned 16, 32 and 64 bit values
// either in-place by assigning to same value, or returning the swapped value
#define TxEndianSwap16(x) x = TxEswap16(x)
#define TxEswap16(us) \
     ((((USHORT)((us) & 0x00FF)) << 8) | \
      (((USHORT)((us) & 0xFF00)) >> 8))

#define TxEndianSwap32(x) x = TxEswap32(x)
#define TxEswap32(ul) \
      ((((ULONG)((ul) & 0x000000FF)) << 24) | \
       (((ULONG)((ul) & 0x0000FF00)) <<  8) | \
       (((ULONG)((ul) & 0x00FF0000)) >>  8) | \
       (((ULONG)((ul) & 0xFF000000)) >> 24) )

#define TxEndianSwap64(x) x = TxEswap64(x)
#define TxEswap64(ull) \
     (((((ULLONG) ull) << 56) & 0xFF00000000000000ULL) | \
      ((((ULLONG) ull) << 40) & 0x00FF000000000000ULL) | \
      ((((ULLONG) ull) << 24) & 0x0000FF0000000000ULL) | \
      ((((ULLONG) ull) <<  8) & 0x000000FF00000000ULL) | \
      ((((ULLONG) ull) >>  8) & 0x00000000FF000000ULL) | \
      ((((ULLONG) ull) >> 24) & 0x0000000000FF0000ULL) | \
      ((((ULLONG) ull) >> 40) & 0x000000000000FF00ULL) | \
      ((((ULLONG) ull) >> 56) & 0x00000000000000FFULL) )

// Append translated Mac-Unicode string-fragment to an ASCII string
char *TxMacUniAppend                            // RET   ptr to ASCII string
(
   S_MACUNI_STRING    *macuni,                  // IN    Mac-Unicode string part
   char               *asc                      // INOUT ASCII string
);

// Append translated Unicode string-fragment to an ASCII string
char *TxUnicAppend                              // RET   ptr to ASCII string
(
   USHORT             *uni,                     // IN    Unicode string part
   char               *asc,                     // INOUT ASCII string
   USHORT              len                      // IN    maximum Unicode length
);

// Perform a Unicode to ASCII case-sensitive stringcompare (aka strncmp)
int TxUnicStrncmp                               // RET   compare result
(
   char               *uni,                     // IN    Unicode string
   char               *asc,                     // IN    ASCII   string
   size_t              len                      // IN    max ASCII comp-length
);

// Perform a Unicode to ASCII case-insensitive stringcompare (aka strnicmp)
int TxUnicStrnicmp                              // RET   compare result
(
   char               *uni,                     // IN    Unicode string
   char               *asc,                     // IN    ASCII   string
   size_t              len                      // IN    max ASCII comp-length
);

// Append default filename extension to filename, if none present
char *TxFnameExtension                          // RET   ptr to filename
(
   char               *fname,                   // INOUT filename string
   char               *ext                      // IN    default extention
);

// Remove filename extension from filename, if present
char *TxStripExtension                          // RET   ptr to filename
(
   char               *fname                    // INOUT filename string
);

// Get base-name part from a path+filename string
char *TxGetBaseName                             // RET   ptr to basename
(
   char               *fname                    // IN    path+filename string
);

// Strip basename from path+filename string, leaving the PATH component only
char *TxStripBaseName                           // RET   BaseName location,
(                                               //       or NULL if not there
   char               *fname                    // IN    path+filename string
);                                              // OUT   path only string

// Test if exact (path+) filename is accessible; supports > 2GiB files
BOOL TxFileExists                               // RET   file is accessible
(
   char               *fname                    // IN    filename string
);

// Open file for reading, supporting large files (native API's, TX handles)
ULONG TxFileOpenRead                            // RET   file open result RC
(
   char               *fname,                   // IN    filename string
   TXHFILE            *fhandle                  // OUT   TX read file handle
);

// Create empty file with specified path/name, prompt to replace existing
ULONG TxCreateEmptyFile
(
   char               *fname,                   // IN    path and filename
   BOOL                prompt                   // IN    prompt on replace existing
);

// Test if exact (path+) filename is accessible; determine size > 2GiB files
BOOL TxFileSize                                 // RET   file exists
(
   char               *fname,                   // IN    filename string
   LLONG              *size                     // OUT   filesize or NULL
);

// Seek to specified position in open file, platform specific, supports large files
ULONG TxFileSeek
(
   TXHFILE             fh,                      // IN    file handle
   LLONG               offset,                  // IN    seek offset
   int                 whence                   // IN    seek reference
);

// Set size of a file to the specified number of bytes, supports large files
ULONG TxSetFileSize
(
   TXHFILE             fh,                      // IN    file handle
   LLONG               size                     // IN    filesize to set
);

// Set File timestamp(s) to specified values
ULONG TxSetFileTime                             // RET   result
(
   char               *fname,                   // IN    filename string
   time_t             *create,                  // IN    create time or NULL
   time_t             *access,                  // IN    access time or NULL
   time_t             *modify                   // IN    modify time or NULL
);

// Find file in a path specified by environment variable (like PATH)
char *TxFindByPath                              // RET   ptr to filename
(
   char               *fname,                   // IN    filename string
   char               *path,                    // IN    env-var with path
   char               *found                    // OUT   found file
);


// Find file in current-dir, exe-dir or PATH, and when found, open for read
FILE *TxFindAndOpenFile                         // RET   opened file handle
(
   char               *fname,                   // IN    filename string
   char               *path,                    // IN    env-var with path
   char               *found                    // OUT   found file
);




//--------- CommandParser definitions and functions ---------------------------

#define TXA_BASE      32                        // first valid option ASCII
#define TXA_LAST     'z'                        // last  valid option ASCII
#define TXA_LBASE    123                        // index first 'long' options

#define TXA_O_HELP   '?'                        // remapped from 'help' to '?'
#define TXA_O_QUERY  123                        // predefined 'query'
#define TXA_O_MENU   124                        // predefined 'menu'
#define TXA_O_DEBUG  125                        // predefined 'debug'
#define TXA_O_TEST   126                        // predefined 'test'
#define TXA_O_TRACE  127                        // predefined 'trace'
#define TXA_O_AUTO   128                        // predefined 'auto'
#define TXA_O_ENTRY  129                        // predefined 'entry'
#define TXA_O_COLOR  130                        // predefined 'color'
#define TXA_O_SCHEME 131                        // predefined 'scheme'
#define TXA_O_LINES  132                        // predefined 'lines'
#define TXA_O_INI    133                        // predefined 'ini'
#define TXA_O_CONFIG 134                        // predefined 'config'
#define TXA_O_STYLE  135                        // predefined 'style'
#define TXA_O_KEYB   136                        // predefined 'keyb'
#define TXA_O_MOUSE  137                        // predefined 'mouse'
#define TXA_O_SIMUL  138                        // predefined 'simulate'
#define TXA_O_LIST   139                        // predefined 'list'
#define TXA_O_SCREEN 140                        // predefined 'screen'
#define TXA_O_LABEL  141                        // predefined 'label'
#define TXA_O_XML    142                        // predefined 'xml'
#define TXA_O_URL    143                        // predefined 'url'
#define TXA_O_DIR    144                        // predefined 'dir'
#define TXA_O_FILE   145                        // predefined 'file'
#define TXA_O_DISK   146                        // predefined 'disk'
#define TXA_O_APP0   147
#define TXA_O_APP1   148                        // application defined
#define TXA_O_APP2   149                        // options/switches
#define TXA_O_APP3   150
#define TXA_O_APP4   151
#define TXA_O_APP5   152
#define TXA_O_APP6   153
#define TXA_O_APP7   154
#define TXA_O_APP8   155
#define TXA_O_APP9   156
#define TXA_O_APPA   157
#define TXA_O_APPB   158
#define TXA_O_APPC   159
#define TXA_O_APPD   160
#define TXA_O_APPE   161
#define TXA_O_APPF   162

#define TXA_LSIZE     40                        // number of long options
#define TXA_SIZE     131                        // total number of options

#define TXA_O_LEN     31                        // maximum length long name

#define TXA_ARGC      32                        // maximum nr of params

#define TXA_NONE      ((TXA_TYPE)  0)
#define TXA_NUMBER    ((TXA_TYPE)  1)
#define TXA_STRING    ((TXA_TYPE)  2)
#define TXA_NO_VAL    ((TXA_TYPE)  3)

#define TXA_DFUNIT    ((BYTE)    ' ')           // default unit

#define TXA_CUR       ((TXHANDLE)  3)           // special CURRENT  handle
#define TXA_PREV      ((TXHANDLE)  2)           // special PREVIOUS handle
#define TXA_1ST       ((TXHANDLE)  1)           // special FIRST    handle
#define TXA_NUL       ((TXHANDLE)  0)           // special bad TXA  handle

#define TXA_ALL       ((int)       0)           // get ALL arguments
#define TXA_OPT       ((int)  0xffff)           // get ALL args + options

// shorthand macro's for most used option/argument retrieval

#define TxaArgCount()           TxaGetArgc(       TXA_CUR  )
#define TxaArgValue(n)          TxaGetArgv(       TXA_CUR,n)
#define TxaOption(o)            TxaOptionYes(     TXA_CUR,o)
#define TxaOptSet(o)           (TxaGetOption(     TXA_CUR,o) != NULL)
#define TxaOptAsString(o,s,b)   TxaOptionAsString(TXA_CUR,o,s,b)
#define TxaOptStr(o,e,d)        TxaOptionStr(     TXA_CUR,o,e,d)
#define TxaOptNum(o,e,d)        TxaOptionNum(     TXA_CUR,o,e,d)
#define TxaOptBkmg(o,d,m)       TxaOptionBkmg(    TXA_CUR,o,d,m)
#define TxaOptValue(o)          TxaGetOption(     TXA_CUR,o)
#define TxaOptCount()           TxaGetOptc(       TXA_CUR  )
#define TxaOptMutEx(c,o,m,e)    TxaMutualX(       TXA_CUR,c,o,m,e)
#define TxaOptSetItem(i)        TxaSetItem(       TXA_CUR,i)
#define TxaOptUnSet(o)         (TxaOptSet(o) && (!TxaOption(o)))

#define TxaExeArgc()            TxaGetArgc(       TXA_1ST  )
#define TxaExeArgv(n)           TxaGetArgv(       TXA_1ST,n)
#define TxaExeSwitch(o)         TxaOptionYes(     TXA_1ST,o)
#define TxaExeSwitchSet(o)     (TxaGetOption(     TXA_1ST,o) != NULL)
#define TxaExeSwAsString(o,s,b) TxaOptionAsString(TXA_1ST,o,s,b)
#define TxaExeSwitchStr(o,e,d)  TxaOptionStr(     TXA_1ST,o,e,d)
#define TxaExeSwitchNum(o,e,d)  TxaOptionNum(     TXA_1ST,o,e,d)
#define TxaExeSwitchBkmg(o,d,m) TxaOptionBkmg(    TXA_1ST,o,d,m)
#define TxaExeSwitchValue(o)    TxaGetOption(     TXA_1ST,o)
#define TxaExeOptc()            TxaGetOptc(       TXA_1ST  )
#define TxaExeMutEx(c,o,m,e)    TxaMutualX(       TXA_1ST,c,o,m,e)
#define TxaExeSetItem(i)        TxaSetItem(       TXA_1ST,i)
#define TxaExeSwitchUnSet(o)   (TxaExeSwitchSet(o) && (!TxaExeSwitch(o)))



typedef BYTE TXA_TYPE;                          // option none/number/string

typedef union txa_value
{
   ULONG               number;                  // number value
   char               *string;                  // string value
} TXA_VALUE;                                    // end of union "txa_value"

typedef struct txa_option
{
   TXA_TYPE            type;                    // none/number/string
   BYTE                unit;                    // one char c,m,s etc (on nrs)
   TXA_VALUE           value;
   char               *name;                    // Full option name or NULL
} TXA_OPTION;                                   // end of struct "txa_option"


// Set or query value for a specific LONGNAME value (the LONG option name)
char  *TxaOptionLongName                        // RET   resulting option name
(
   char                opt,                     // IN    TXA_O_ option value
   char               *name                     // IN    long name for option
);                                              //       or NULL to query

// Create new level of command options for a given command string
ULONG TxaParseCommandString                     // RET   result
(
   char               *cmd,                     // IN    command string
   BOOL                freeform,                // IN    free format options
   TXHANDLE           *txh                      // OUT   TXA handle (optional)
);


// Add one item (argument or option) to the specified TXA handle
ULONG TxaSetItem                                // RET   result
(
   TXHANDLE            txh,                     // IN    TXA handle
   char               *item                     // IN    new item
);


// Replace current level of command options for a new command string (Reparse)
ULONG TxaReParseCommand                         // RET   result
(
   char               *cmd                      // IN    new command string
);


// Create new level of command options for a set of argc/argv variables
ULONG TxaParseArgcArgv                          // RET   result
(
   int                 argc,                    // IN    argument count
   char               *argv[],                  // IN    array of arguments
   char               *exename,                 // IN    alternative argv[0]
   BOOL                freeform,                // IN    free format options
   TXHANDLE           *txh                      // OUT   TXA handle (optional)
);

#define  TXAO_QUIET    ((ULONG)0)
#define  TXAO_NORMAL   ((ULONG)1)
#define  TXAO_VERBOSE  ((ULONG)2)
#define  TXAO_EXTREME  ((ULONG)3)
#define  TXAO_MAXIMUM  ((ULONG)4)

// Get verbosity value from Option 'o'
ULONG TxaOutputVerbosity                        // RET   output verbosity value
(
   char                optchar                  // IN    option character
);

#define  TXAE_QUIT     ((int)'q')
#define  TXAE_IGNORE   ((int)'i')
#define  TXAE_CONFIRM  ((int)'c')

// Get errorStrategy value from Option 'e' and batch-mode indicator 'quit'
int TxaErrorStrategy                            // RET   error strategy value
(
   char                optchar,                 // IN    option character
   BOOL                quit                     // IN    quit as default
);

//- define classes each having their own HEX/DEC default in ParseNumber
//- Application can define higher class-numbers from 2^3 upto 2^31
//- A bit that is 'SET' for a class, will result in a HEXadecimal default
#define TX_RADIX_DEC_CLASS       0               // radix-class fixed decimal
#define TX_RADIX_STD_CLASS       1               // radix-class standard
#define TX_RADIX_UN_S_CLASS      2               // radix-class n,s unit
#define TX_RADIX_UNIT_CLASS      4               // radix-class any unit
#define TX_RADIX_APP_CLASS       8               // radix-class, application 1

// Set radix class bits for input like ParseNumber, numeric options; 1 = HEX
void TxSetNumberRadix
(
   ULONG               radixclasses             // IN    radix class bits
);

// Get radix class bits for input like ParseNumber, numeric options; 1 = HEX
ULONG TxGetNumberRadix                          // RET   radix class bits
(
   void
);

// Parse a decimal or hex number value from a string
ULONG TxaParseNumber                            // RET   number value
(
   char               *value,                   // IN    value string with nr
   ULONG               rclass,                  // IN    HEX/DEC radix class
   BYTE               *unit                     // OUT   optional unit char, if
);                                              //       not needed use NULL

// Parse a boolean value from a string (empty string returns TRUE)
BOOL TxaParseBool                               // RET   FALSE: 0 - n f off
(
   char               *param                    // IN    pragma parameter
);

// Terminate use of current or all instances of the TXA abstract-data-type
BOOL TxaDropParsedCommand                       // RET   more instances left
(
   BOOL                whole_stack              // IN    drop all, terminate
);

// Show contents of one or all instances of the TXA abstract-data-type
void TxaShowParsedCommand                       // RET   more instances left
(
   BOOL                whole_stack              // IN    show all levels
);

// Parse next item in string, skip leading terminators, honor embedded strings
char *txaParseNextItem                          // RET   next item or NULL
(
   char              **start,                   // INOUT start position
   char                term,                    // IN    terminating character
   int                *length                   // OUT   length of item
);

// Get reference to specified option data, or NULL if option not set
TXA_OPTION *TxaGetOption                        // RET   option ptr or NULL
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt                      // IN    option character
);

// Get simple YES/NO status for specified option. Not set is "NO"
BOOL TxaOptionYes                               // RET   option set to YES
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt                      // IN    option character
);

// Get string value for specified option, convert to string for any type
char *TxaOptionAsString                         // RET   option String value
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt,                     // IN    option character
   int                 size,                    // IN    size of buffer
   char               *str                      // OUT   string buffer, also
);                                              // default when not specified

// Get string value for specified option, use default if no string (and warn!)
char *TxaOptionStr                              // RET   option String value
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt,                     // IN    option character
   char               *error,                   // IN    error text or NULL
   char               *deflt                    // IN    default value
);

// Get number value for specified option, use default if no number (and warn!)
ULONG TxaOptionNum                              // RET   option Number value
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt,                     // IN    option character
   char               *error,                   // IN    error text or NULL
   ULONG               deflt                    // IN    default value
);

// Get option value num/string, with bytes/kilo/mega/giga modifier and default
ULONG TxaOptionBkmg                             // RET   number value in bytes
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                option,                  // IN    option character
   ULONG               def,                     // IN    default value
   BYTE                mod                      // IN    b,kb,mb,gb modifier
);

// Build space separated concatenated string of arguments
// Clips output with no warning when buffer too small!
char *TxaGetArgString                           // RET   argument str or NULL
(
   TXHANDLE            txh,                     // IN    TXA handle
   int                 first,                   // IN    first argument to copy
   int                 last,                    // IN    last arg, _ALL or _OPT
   int                 bufsize,                 // IN    size of result buffer
   char               *args                     // OUT   argument string
);

// Return comment part of command, if any
char *TxaGetComment                             // RET   comment  str or NULL
(
   TXHANDLE            txh,                     // IN    TXA handle
   char               *comment                  // OUT   comment string
);

// Get reference to specified argument-string
char *TxaGetArgv                                // RET   ptr to argv string
(
   TXHANDLE            txh,                     // IN    TXA handle
   int                 argn                     // IN    argument number 0..n
);

// Get the number of arguments for specified TXA instance
int TxaGetArgc                                  // RET   argument count
(
   TXHANDLE            txh                      // IN    TXA handle
);

// Get the number of options for specified TXA instance
int TxaGetOptc                                  // RET   option count
(
   TXHANDLE            txh                      // IN    TXA handle
);

// Test for mutual exclusive options and specified condition; handle error msg
BOOL TxaMutualX
(
   TXHANDLE            txh,                     // IN    TXA handle
   BOOL                cond,                    // IN    other  exclusive cond
   char               *opts,                    // IN    mutual exclusive opts
   char               *condmsg,                 // IN    msg when cond TRUE or NULL
   char               *msgtext                  // OUT   message text or NULL
);                                              //       (NULL ==> TxPrint)


//--------- Selection list support (not tied to TXW yet)    (TX-SELIST) ------
//- supports either static (fixed) lists and dynamic ones where the total
//- list can be many times larger than the expanded (visible) part

//- definitions for flags that apply to one item (item->flags)
#define TXSF_MARK_STD                0x01
#define TXSF_MARK_STAR               0x02
#define TXSF_MARKED                  0x03
#define TXSF_P_LISTBOX               0x10       // popup another listbox level
#define TXSF_SEPARATOR               0x20
#define TXSF_AUTOSKIP                0x40       // skip disabled on up/down
#define TXSF_DISABLED                0x80

//- definitions for flags that apply to the whole list (list->flags)

#define TXSL_MULTI_QUICK       0x00000001       // use itemtext for quickselect
                                                // not just the item-index char
                                                // (used in directory listing)

#define TXSL_ITEM_DELETE       0x00000010       // allow item delete  (Ctrl-D)
                                                // del from list, send Ctrl-D
                                                // to the owner as well ...

//- definitions for Alloc-Status of selist objects (Static/Shared or Dynamic)
//- when dynamic bits for an item are set, TxFreeMem() will be used on them
#define TXS_AS_DYN_MASK            0xffff       // dynamic allocation flags
#define TXS_AS_LISTRUCT            0x0001       // main struct
#define TXS_AS_SELARRAY            0x0002       // selection array
#define TXS_AS_PTRARRAY            0x0004       // ptr-array to items
#define TXS_AS_SELITEMS            0x0008       // item structures
#define TXS_AS_SEL_DESC            0x0010       // item description
#define TXS_AS_SEL_TEXT            0x0020       // item display text
#define TXS_AS_LST_DESC            0x0040       // list descr in 'userinfo'
#define TXS_AS_DSTRINGS  (TXS_AS_SEL_DESC | TXS_AS_SEL_TEXT)
#define TXS_AS_DYNITEMS  (TXS_AS_SELITEMS | TXS_AS_DSTRINGS)
#define TXS_AS_DYNBASED  (TXS_AS_LISTRUCT | TXS_AS_SELARRAY | TXS_AS_PTRARRAY)
#define TXS_AS_STATIC                0x00       // all static, no TxAlloc
#define TXS_AS_NOSTATIC   TXS_AS_DYN_MASK       // all Tx dynamic allocation

//- upper bits used for other list-level flag bits
//-
#define TXS_LST_DESC_PRESENT   0x00010000       // list level descr is present
#define TXS_LST_DYN_CONTENTS   0x00020000       // list contents not fixed
                                                // documentation recursion is
                                                // not very meaningfull

#define TXS_RESERVED_SELECTED  0x01             // Flag for 'selected' in reserved

typedef struct txs_item
{
   ULONG               value;                   // value, cmd-code, alt helpid
   ULONG               helpid;                  // helpid or 0 to use cmd value
   ULONG               userdata;                // user value, list for submenu
   USHORT              index;                   // pos of sel-char 1..x or 0
   BYTE                flags;                   // item flags,  (disabled etc)
   BYTE                bits;                    // indicator bits (temporary)
   char               *desc;                    // descriptive help on item
   char               *text;                    // rendered item text
   char               *info;                    // info (disable reason) always dynamic!
} TXS_ITEM;                                     // end of struct "txs_item"


//- TXSELIST callback function to render new visible list item based on index
typedef ULONG (* TXS_ITEMHANDLER)
(
   TXS_ITEM           *item,                    // INOUT item to handle or NULL
   BYTE               *select,                  // INOUT selection data or NULL
   ULONG               index                    // IN    external index
);


// Key bindings for sorting, to be used in help-texts (platform dependant!)

#if defined (UNIX)
 #define TXS_SORT_KEYS1   "Ctrl-D"
 #define TXS_SORT_KEYS2   "Ctrl-X"
 #define TXS_SORT_KEYS3   "Ctrl-N"
 #define TXS_SORT_KEYS4   "Ctrl-E"
 #define TXS_SORT_KEYS5   "Ctrl-T"
 #define TXS_SORT_KEYS6   "Ctrl-B"
 #define TXS_SORT_KEYS7   "Ctrl-U"
 #define TXS_SORT_KEYS8   "Ctrl-R"
 #define TXS_SORT_KEYS9   "Ctrl-F"
#else
 #define TXS_SORT_KEYS1   "Ctrl+F1 or Ctrl-D  "
 #define TXS_SORT_KEYS2   "Ctrl+F2 or Ctrl-X  "
 #define TXS_SORT_KEYS3   "Ctrl+F3 or Ctrl-N  "
 #define TXS_SORT_KEYS4   "Ctrl+F4 or Ctrl-E  "
 #define TXS_SORT_KEYS5   "Ctrl+F5 or Ctrl-T/L"
 #define TXS_SORT_KEYS6   "Ctrl+F6 or Ctrl-B/F"
 #define TXS_SORT_KEYS7   "Ctrl+F7 or Ctrl-U  "
 #define TXS_SORT_KEYS8   "Ctrl+F8 or Ctrl-R  "
 #define TXS_SORT_KEYS9   "Ctrl+F9 or Ctrl-F  "
#endif

// Sort specification mask and flag values
// Some flags are shared between colums based sort specs and base, values must be unique!
#define TXS_SORT_MASK     0x03ff                // Shared mask for index and column values
#define TXS_SORT_IDXMASK  TXS_SORT_MASK         // Mask to isolate index value   (max 1023)
#define TXS_SORT_COLUMN   TXS_SORT_MASK         // Sort column number mask (1023 chars max)
#define TXS_SORT_REVERSE  0x8000                // Last sort was reversed, flag      (c-F8)
#define TXS_SORT_DESC     0x4000                // Sort on the 'description' string
#define TXS_SORT_TEXT     0x0000                // Sort on the 'text' string
#define TXS_SORT_DESCEND  0x2000                // Sort descending on alpha sorts
#define TXS_SORT_ASCEND   0x0000                // Sort ascending  on alpha sorts
#define TXS_SORT_IGNCASE  0x1000                // Sort ignoring case    (like DOS/WIN/OS2)
#define TXS_SORT_ON_CASE  0x0000                // Sort case sensitive     (like Linux/MAC)
#define TXS_SORT_USERDATA 0x0800                // Last sort was userdata, flag     (c-F7)
#define TXS_SORT_LASTDOT  0x0400                // Sort from last dot      (file extension)


// Indexes in sort-spec array for alpha-numeric sorting of text and desc
// Value is 1-based column to sort on, and 0 means sort is not supported
#define TXS_SORT_CURRENT  0                     // Combined index + flags last active sort
#define TXS_SORT_1        1                     // Sort at column/last-dot  1 (c-F2)
#define TXS_SORT_2        2                     // Sort at column/last-dot  2 (c-F1)
#define TXS_SORT_3        3                     // Sort at column/last-dot  3 (c-F9)
#define TXS_SORT_4        4                     // Sort at column/last-dot  4 (c-F5)
#define TXS_SORT_5        5                     // Sort at column/last-dot  5 (c-F6)
#define TXS_SORT_6        6                     // Sort at column/last-dot  6 (c-F3)
#define TXS_SORT_7        7                     // Sort at column/last-dot  7 (c-F3)
#define TXS_SORT_COUNT    8                     // Number of sort specifications

typedef struct txselsort                        // sort information for a selist
{
   USHORT              sort[TXS_SORT_COUNT];    // sort types and indexes
   TXTS                show[TXS_SORT_COUNT];    // sort descriptions per index
   TXTS                currentShow;             // actual description shown
} TXSELSORT;                                    // end of struct "txselsort"

typedef struct txselist
{
   ULONG               astatus;                 // alloc-status bits; 0=static
   ULONG               userinfo;                // user definable info
   ULONG               flags;                   // list specific flags
   ULONG               top;                     // index of 1st visible item
   ULONG               count;                   // actual number of items
   ULONG               asize;                   // size allocated  (items array)
   ULONG               vsize;                   // size of visible part (scroll)
   ULONG               tsize;                   // size for total list (maximum)
   ULONG               selected;                // index or number of selected
   TXSELSORT          *sortinfo;                // sort information or NULL
   BYTE               *selarray;                // selarray if multi-select
   TXS_ITEMHANDLER     renderNewItem;           // callback when made visible
   TXS_ITEM          **items;                   // array of visible items
} TXSELIST;                                     // end of struct "txselist"

typedef struct txs_menu
{
   ULONG               helpid;                  // help-id for pulldown
   USHORT              index;                   // pos of sel-char 1..x or 0
   BYTE                flags;                   // item flags,  (disabled etc)
   BYTE                ident;                   // identification/select char
   char               *desc;                    // descriptive help on menu
   char               *text;                    // rendered item text
   TXSELIST           *list;                    // list of menu items
} TXS_MENU;                                     // end of struct "txs_menu"

#define TXS_BARSIZE     32
typedef struct txs_menubar
{
   ULONG               count;                   // number of menus
   ULONG               defopen;                 // index of default menu
   TXS_MENU           *menu[TXS_BARSIZE];       // array of menus
   TXTM                appdesc;                 // mbar application description
} TXS_MENUBAR;                                  // end of struct "txs_menubar"


//- macros to define (menubar/popupmenu/submenu) list items statically

#define TXSitem static TXGitem
#define TXGitem(name,cmd,help,flg,selpos,text,desc)                           \
        TXS_ITEM name = {cmd,help,0,selpos,flg,0,desc,text}

#define TXSsubm static TXGsubm
#define TXGsubm(name,help,sub,base,flg,selpos,text,desc)                      \
        TXS_ITEM name = {base,help,(ULONG)sub,selpos,(flg)|TXSF_P_LISTBOX,0,desc,text}

#define TXSlist static TXGlist
#define TXGlist(name,s,v,items)                                               \
        TXSELIST name = {0,0,0,0,s,s,v,s,0,NULL,NULL,NULL,(items)}

#define TXSmenu static TXGmenu
#define TXGmenu(name,list,help,flg,selpos,ident,text,desc)                    \
        TXS_MENU name = {help,selpos,flg,ident,desc,text,(list)}

// Get length of longest string in a list
ULONG TxSelGetMaxLength                         // RET   longest string length
(
   TXSELIST           *list                     // IN    selection list
);

// Scroll visible list up by one line (towards lower index)
ULONG TxSelScrollUp                             // RET   scrolling succeeded
(
   TXSELIST           *list                     // INOUT selection list
);

// Scroll visible list down by one line (towards higher index)
ULONG TxSelScrollDown                           // RET   scrolling succeeded
(
   TXSELIST           *list                     // INOUT selection list
);

// Adjust visible list to start at the specified external index
#define TXSEL_TOP   ((ULONG) 0x00000000)        // top of list position
#define TXSEL_END   ((ULONG) 0x7fffffff)        // end of list position
ULONG TxSelSetPosition                          // RET   resulting top index
(
   TXSELIST           *list,                    // INOUT selection list
   ULONG               index                    // IN    top external index
);

// Set selection for specified index, and/or bits if multiple select possible
ULONG TxSelSetSelected                          // RET   resultcode
(
   TXSELIST           *list,                    // INOUT selection list
   ULONG               index,                   // IN    external index
   TXSETRESET          action,                  // IN    set, reset or toggle
   BYTE                mask                     // IN    bits to set/reset
);

// Itterate over selected list items, calback all with specified mask set
ULONG TxSelItterateSelected
(
   TXSELIST           *list,                    // INOUT selection list
   TXS_ITEMHANDLER     selected,                // IN    callback selected
   BYTE                mask                     // IN    selection mask
);


// Set 'selected' to item with specified selection char (at text[index-1])
ULONG TxSelCharSelect                           // RET   index now selected
(
   TXSELIST           *list,                    // INOUT selection list
   char                select                   // IN    selection character
);

// Set 'selected' to item where text starts with 'string' (case-insensitive)
ULONG TxSelStringSelect                         // RET   index now selected
(
   TXSELIST           *list,                    // INOUT selection list
   char               *select,                  // IN    selection string
   int                 length                   // IN    significant length
);

// Sort the list on text or desc string according to given sort specification
BOOL TxSelSortString                            // RET   TRUE if list changed
(
   TXSELIST           *list,                    // INOUT selection list
   USHORT              spec                     // IN    sort index to get column
);

// Sort a list ascending on 'userdata'  (unsorts if that contains item-nrs)
BOOL TxSelSortUserData                          // RET   TRUE if list changed
(
   TXSELIST           *list                     // INOUT selection list
);

// Reverse a selection list, reversing the sort-order (all items swapped)
BOOL TxSelReverseOrder                          // RET   TRUE if list changed
(
   TXSELIST           *list                     // INOUT selection list
);

// Sort the list according to the 'curent' sort specification
void TxSelSortCurrent
(
   TXSELIST           *list                     // INOUT selection list
);

// Get unique quick-select character for specified list index
USHORT TxSelGetCharSelect                       // RET   index for select-char
(                                               //       or 0 if not possible
   TXSELIST           *list,                    // IN    selection list
   ULONG               nr,                      // IN    item to get char for
   int                 offset                   // IN    minimum position 0..n
);

// Set/reset specified flag-bit(s) in menu-item by searching menu-bar structure
ULONG TxSetMenuItemFlag                         // RET   #items found and set
(
   TXS_MENUBAR        *mbar,                    // IN    menubar information
   ULONG               mid,                     // IN    menu item id
   BYTE                mask,                    // IN    bits to set/reset
   char               *info,                    // IN    info, reason etc or NULL
   BOOL                set                      // IN    set specified bit(s)
);

// Set/reset specified flag-bit(s) in list-item by searching list-structure
ULONG TxSetListItemFlag                         // RET   #items found and set
(
   ULONG               recurse,                 // IN    recursion level (1)
   TXSELIST           *list,                    // IN    list information
   ULONG               mid,                     // IN    value, menu item id
   BYTE                mask,                    // IN    bits to set/reset
   char               *info,                    // IN    info, reason etc or NULL
   BOOL                set                      // IN    set specified bit(s)
);

// Attach information string to given (menu) item as dynamically allocated str
ULONG TxSelSetItemInfo                          // RET   result
(
   TXS_ITEM           *item,                    // IN    list information
   char               *info                     // IN    info, reason etc
);

// Create a dynamic selection list with specified size
ULONG TxSelCreate
(
   ULONG               vsize,                   // IN    visible size
   ULONG               tsize,                   // IN    total size of list
   ULONG               asize,                   // IN    size for item array
   ULONG               astat,                   // IN    alloc status for items
   BOOL                multi,                   // IN    create multi-select info
   TXS_ITEMHANDLER     render,                  // IN    callback render new
   TXSELIST          **list                     // OUT   selection list
);

// Destroy a dynamic selection list, freeing memory
ULONG txSelDestroy
(
   TXSELIST          **list                     // IN    selection list
);

// Destroy dynamic parts of a selection list-item, freeing memory
ULONG txSelItemDestroy
(
   ULONG               as,                      // IN    allocaton status
   TXS_ITEM          **itemref                  // IN    selection list item
);

// Delete current item from the list (like Ctrl-D on a history :-)
ULONG TxSelDeleteCurrent                        // RET   resulting selected
(
   TXSELIST           *list                     // INOUT selection list
);

// Create static selist from existing Txt list
ULONG TxSelistFromTxt                           // RET   result
(
   char               *txt[],                   // IN    NULL terminated txt
   BOOL                multi,                   // IN    keep multi-select info
   BOOL                share,                   // IN    share data, no alloc
   TXSELIST          **list                     // OUT   selection list
);

// Create selist from existing selist (duplicate)
ULONG TxSelistDuplicate                         // RET   result
(
   TXSELIST           *org,                     // IN    original Selist
   BOOL                multi,                   // IN    keep multi-select info
   BOOL                share,                   // IN    share data, no alloc
   TXSELIST          **list                     // OUT   selection list
);


// Create single-item list with ONE disabled text-item (for 'empty' selections)
TXSELIST *TxSelEmptyList                        // RET   selection list or NULL
(
   char               *text,                    // IN    text for single item
   char               *desc,                    // IN    description (footer)
   BOOL                selectable               // IN    item should be selectable
);

//- substring in not-ready drives in list
#define  TXFS_NOTREADY   "-not-ready-"

// Build selection-list with volume/FS info for drive-letters or Linux-devices
TXSELIST *TxFsDriveSelist                       // RET   selection list or NULL
(
   ULONG               flags,                   // IN    include FLOP/LAN/CDROM
   BOOL                flop                     // IN    runtime floppy test
);

// Flags for file-dialog (note: lower USHORT used for window-width!)
#define TXFDS_ADD_DESCRIPTION     0x8000        // add descriptions too

// Build selection-list with File/Directory/Tree information
TXSELIST *TxFileDirSelist                       // RET   selection list or NULL
(                                               //       Tree RC is in userinfo
   char               *fspec,                   // IN    Filespec to match
   char               *fopts,                   // IN    Find options
   char               *fattr,                   // IN    Select attributes
   char              **filter,                  // IN    Excl/Incl file-specs
   ULONG               ffw                      // IN    flags + filewindow width
);


// Generate HTML (optional PHP) documentation from a pulldown menu structure
ULONG txSelPdMenu2Html
(
   char               *product,                 // IN    product identification
   TXS_MENUBAR        *mbar,                    // IN    Main menu structure
   char               *select,                  // IN    partial PD select str
   char               *htmlnav,                 // IN    HTML navigation header
   char               *filename,                // IN    html filename or NULL
   char               *phptemplate,             // IN    PHP template  or NULL
   char               *pdLead,                  // IN    Pulldown lead or NULL
   ULONG               pdLevel                  // IN    HTML top heading level
);

//--------- Generic utility functions and OS-specific support (TXUTIL) -------

// Remove ANSI control code from a text-string (in place conversion)
ULONG TxStripAnsiCodes                          // RET   length stripped string
(
   char               *text                     // INOUT ANSI text to strip
);

// Replace specfied character by another in whole string, count replacements
int TxRepl                                      // RET   nr of replacements
(
   char              *str,                      // INOUT string to convert
   char               old,                      // IN    old char, to replace
   char               new                       // IN    new char
);

// Copy string, and replace specfied character by a string
int TxRepStr                                    // RET   nr of replacements
(
   char               *str,                     // IN    string to convert
   char                old,                     // IN    old char, to replace
   char               *new,                     // IN    new string
   char               *dest,                    // OUT   destination string
   int                 len                      // IN    max length destination
);

// Strip leading/trailing characters from a string, dest and source can be same
int TxStrip                                     // RET   nr of stripped chars
(
   char               *dest,                    // OUT   destination string
   char               *source,                  // IN    source string
   char                lead,                    // IN    leading  chars to strip
   char                trail                    // IN    trailing chars to strip
);

// Pad/Clip a string to specified length, concatenating or removing characters
int TxPClip                                     // RET   +:padded -:removed
(
   char               *str,                     // INOUT destination string
   int                 size,                    // IN    requested length
   char                pad                      // IN    trailing chars to pad
);


// Convert string to all uppercase (like non-possix strupr function)
char *TxStrToUpper                              // RET   Updated string
(
   char               *str                      // INOUT destination string
);


// Convert string to all lowercase (like non-possix strlwr function)
char *TxStrToLower                              // RET   Updated string
(
   char               *str                      // INOUT destination string
);

// Test if area consists completely of the empty byte/character ==> is empty
BOOL TxAreaEmpty                                // RET   area is empty
(
   char               *area,                    // IN    area to check
   int                 size,                    // IN    size of area, bytes
   char                empty                    // IN    the empty char
);

#define TXASCII827_STRIP   ((unsigned char) 0x00)
#define TXASCII827_TRANS   ((unsigned char) 0x80)
// Filter 8-bit ASCII, strip to 7-bit, translate, or replace by specified char
void TxAscii827                                 // translate 8-bit to 7-bit
(
   char               *str,                     // INOUT string to convert
   unsigned char       new                      // IN    new char, or 0x00/0x80
);

// Copy string to destination, replacing non printables, clip and terminate it
void TxCopy
(
   char               *dest,                    // OUT   destination string
   char               *source,                  // IN    source string
   int                 len                      // IN    max length incl \0
);

// Find occurence of a string in a memory region (with embedded NUL)
char *TxMemStr                                  // RET   ptr to string or NULL
(
   void               *mem,                     // IN    memory region
   char               *str,                     // IN    string to find
   int                 len                      // IN    length of mem region
);

// Search text for given string and specified search-options
BOOL txTextSearch
(
   char              **text,                    // IN    TX style text array
   char               *string,                  // IN    String to search for
   BOOL                caseSensitive,           // IN    Case sensitive search
   BOOL                wholeWord,               // IN    Space delimited word only
   BOOL                backwards,               // IN    Search backwards
   BOOL                titleOnly,               // IN    Search in titles only (#nnn)
   ULONG              *posLine,                 // INOUT position, Line number
   short              *posColumn                // OUT   position, Column
);

// Read lines from opened file into dynamically allocated TXLIB text structure
char **txFile2Text                              // RET   ptr to text-array
(
   FILE               *file,                    // IN    file opened for read
   ULONG              *size                     // OUT   size in lines
);

// Convert C-string to dynamically allocated TXLIB text structure
char  **txString2Text                           // RET   TXLIB text structure
(
   char               *string,                  // IN    null terminated string
   int                *maxlen,                  // INOUT line length
   int                *lines                    // OUT   nr of lines
);

// Free memory allocated for dynamic TXLIB text
void txFreeText
(
   char              **text                     // IN    dyn. allocated text
);

// Convert USHORT value from big to little endian or vice-versa
USHORT txSwapUS                                 // RET   converted value
(
   USHORT              val                      // IN    value
);

// Convert ULONG value from big to little endian or vice-versa
ULONG txSwapUL                                  // RET   converted value
(
   ULONG               val                      // IN    value
);

// Test if string represents a valid hex number, allow 0x prefix and ,unit
BOOL txIsValidHex                               // RET   string is valid hex
(
   char               *num                      // IN    candidate string
);

// Test if string represents a valid decimal nr, allow 0t/0n prefix and ,unit
BOOL txIsValidDec                               // RET   string is not decimal
(
   char               *num                      // IN    candidate string
);

// Test if string starts with a valid MCS prefix 0x 0n 0t 0x (case insensitive)
BOOL txHasMcsPrefix                             // RET   string has MCS prefix
(
   char               *num                      // IN    candidate string
);

// Return 15-character TXLIB version string
char  *txVersionString                          // RET   15-char version string
(
   void
);

// Translate attributes (FAT) to 6-position string format
void txFileAttr2String
(
   ULONG               data,                    // IN    data
   char               *attrib                   // OUT   attribute string[6]
);


// Convert Windows-NT/W2K/XP filetime (64 bit) to compiler time_t value
time_t txWinFileTime2t                          // RET   time_t value
(
   NTIME              *nt,                      // IN    ptr NT time value
   LONG                TimeZone                 // IN    TZ offset to GMT (min)
);

// Convert compiler time_t to Windows-NT/W2K/XP filetime (64 bit) value
void txCt2WinFileTime
(
   time_t              ct,                      // IN    C standard time value
   NTIME              *nt,                      // OUT   NT time value
   LONG                TimeZone                 // IN    TZ offset to GMT (min)
);

// Convert DOS, OS/2 and eCS filetime (2 * USHORT) to compiler time_t value
time_t txOS2FileTime2t                          // RET   time_t value
(
   USHORT             *pd,                      // IN    USHORT OS2 coded date
   USHORT             *pt                       // IN    USHORT OS2 coded time
);

// Convert compiler time_t to DOS, OS/2 and eCS filetime (2 * USHORT) value
void txCt2OS2FileTime                           // RET   time_t value
(
   time_t              ct,                      // IN    C standard time value
   USHORT             *pd,                      // OUT   USHORT OS2 coded date
   USHORT             *pt                       // OUT   USHORT OS2 coded time
);


// Format (file) size in 7.1 value + Byte/KiB/MiB/GiB (11); to TxPrint output
void txPrtSize64
(
   char               *text,                    // IN    leading string
   LLONG               data,                    // IN    size data
   char               *trail                    // IN    trailing text
);

// Format (file) size in 7.1 value + Byte/KiB/MiB/GiB (11); append to string
char *txStrSize64                               // RET   resulting string
(
   char               *str,                     // OUT   resulting string
   char               *text,                    // IN    leading string
   LLONG               data,                    // IN    size data
   char               *trail                    // IN    trailing text
);

// Format time in seconds in an HHH:MM:SS string (Elapsed/ETA)
char *txStrSec2hms                              // RET   resulting string
(
   char               *str,                     // OUT   resulting string
   char               *text,                    // IN    leading string
   ULONG               sec,                     // IN    seconds
   char               *trail                    // IN    trailing text
);

#if   defined (DOS32)
   //- DOS (32-bit) specific utility functions

// Wait for specified nr of msec (approximation, +/- 32 msec)
void TxBusyWait                                 // RET   waited nr of msec
(
   ULONG               msec                     // IN    nr of msec to wait
);


// Set keyboard mapping using FreeDOS keyb and .kl files
ULONG TxSetNlsKeyboard                          // RET   result
(
   char               *spec,                    // IN    keyb file basename
   char               *cp                       // IN    codepage string or ""
);


// Get number of physical diskette drives from BIOS
USHORT TxPhysDisketteDrives                     // RET   nr of diskette drives
(
   void
);

#define TXX_BIOSD  0x400                        // BIOS data area

#define TXX_EQUIPMENT         0x0010            // equipment list

#define TXX_CRT_MODE          0x0049            // current CRT mode
#define TXX_CRT_COLS          0x004a            // number of columns on screen
#define TXX_CRT_LEN           0x004c            // size of video buffer
#define TXX_CURSOR_POSN       0x0050            // cursor for each page
#define TXX_CURSOR_MODE       0x0060            // cursor shape
#define TXX_PHYS_DISKS        0x0075            // number of physical disks
#define TXX_CRT_ROWS          0x0084            // number of text rows - 1
#define TXX_POINTS            0x0085            // height of character
#define TXX_INFO              0x0087            // miscellaneous info
#define TXX_INFO_3            0x0088            // more miscellaneous info

#define TxxBiosWord(p)     (*((USHORT *)(TXX_BIOSD + p)))
#define TxxBiosChar(p)     (*((char   *)(TXX_BIOSD + p)))

#define TxxClearReg(r)     memset( &(r), 0, sizeof(r))

#define TXDX_VIDEO                 0x10
#define TXDX_VIDEO_SETCSHAPE       0x01
#define TXDX_VIDEO_SETCURSOR       0x02
#define TXDX_VIDEO_GETCURSOR       0x03
#define TXDX_VIDEO_SCROLL_UP       0x06
#define TXDX_VIDEO_SCROLL_DN       0x07
#define TXDX_VIDEO_WRITNCELL       0x09
#define TXDX_VIDEO_SETSTATUS       0x10
#define TxxVideoInt(r,function)    r.h.ah = function; \
                                   int386( TXDX_VIDEO, &r, &r)

#define TXDX_PDISK                 0x13

#define TXDX_KBD                   0x16
#define TXDX_KBD_READ              0x00
#define TXDX_KBD_READY             0x01
#define TXDX_KBD_SHIFTSTATUS       0x02
#define TxxKeyBdInt(r,function)    r.h.ah = function; \
                                   int386( TXDX_KBD, &r, &r)

#define TXDX_DOS                   0x21
#define TXDX_DOS_GETVERSION        0x30
#define TXDX_DOS_FREESPACE         0x36
#define TXDX_DOS_FREESPACE_EX      0x7303
#define TXDX_DOS_IOCTL             0x44
#define TxxDosInt21(r,function)    r.h.ah = function; \
                                   int386( TXDX_DOS, &r, &r)

#define TXDX_MPLEX                 0x2F
#define TXDX_MPLEX_MSCDEX_INSTALL  0x1500
#define TXDX_MPLEX_MSCDEX_DRCHECK  0x150b
#define TxxMplexInt(r,function)    TXWORD.ax = function; \
                                   int386( TXDX_MPLEX, &r, &r)

#define TXDX_MOUSE                 0x33
#define TXDX_MOUSE_RESET           0x0000
#define TXDX_MOUSE_SHOW            0x0001
#define TXDX_MOUSE_HIDE            0x0002
#define TXDX_MOUSE_STATUS          0x0003
#define TXDX_MOUSE_PRESENT         0xffff
#define TxxMouseInt(r,function)    TXWORD.ax = function; \
                                   int386( TXDX_MOUSE, &r, &r)

#define TXDX_DPMI                  0x31
#define TXDX_DPMI_ALLOC            0x0100
#define TXDX_DPMI_FREEM            0x0101
#define TXDX_DPMI_RMINT            0x0300
#define TXDX_DPMI_RMFAR            0x0301
#define TXDX_DPMI_VERS             0x0400

#define TXDX_REAL_LIMIT  ((void *) 0xfffff)

typedef struct txdx_rminfo                      // DOS extender real-mode data
{
   long             edi;
   long             esi;
   long             ebp;
   long             reserved;
   long             ebx;
   long             edx;
   long             ecx;
   long             eax;
   short            flags;
   short            es;
   short            ds;
   short            fs;
   short            gs;
   short            ip;
   short            cs;
   short            sp;
   short            ss;
} TXDX_RMINFO;                                  // end of struct "tpdx_rminfo"


// Execute a DPMI call, including trace
#define  txDpmiCall(r,s)                                                 \
                    TRACES(("Input  txDpmiCall\n"));                     \
                    TRHEXS( 70, r, sizeof(union  REGS ), "in  REGS");    \
                    TRHEXS( 70, s, sizeof(struct SREGS), "in  SREGS");   \
                    TRACES(("Enter  txDpmiCall\n"));                     \
                    int386x( TXDX_DPMI,r,r,s);                           \
                    TRACES(("Return txDpmiCall\n"));                     \
                    TRHEXS( 70, r, sizeof(union  REGS ), "out REGS");    \
                    TRHEXS( 70, s, sizeof(struct SREGS), "out SREGS");

// get DPMI segment value from DPMI allocated linear address pointer
#define  txDpmiSegment(p)  (short)  (((unsigned long) p) >> 4)

// get DPMI 16:16 pointer from DPMI allocated linear address pointer
#define  txDpmi16ptr16(p)  (void *) (((unsigned long) p) << 12)

// Allocate DPMI compatible memory
void *txDpmiAlloc                               // RET   PM linear address
(
   size_t              nr,                      // IN    number of items
   size_t              size,                    // IN    size per item
   short              *selector                 // OUT   PM selector (for free)
);

// Free DPMI compatible memory
void txDpmiFree
(
   short               selector                 // IN    PM selector
);

// Return DOSEXTENDER version string
char  *txDosExtVersion                          // RET   version string (max 50)
(
   void
);

// Return DOSEXTENDER DPMI info string
char  *txDosExtDpmiInfo                         // RET   DPMI info string
(
   void
);


#elif defined (WIN32)
   //- no Win32 specific stuff yet

#elif defined (UNIX)                            // UNIX like    (WATCOM/GCC)

// Sleep for specified nr of msec
void TxSleepMsec
(
   ULONG               msec                     // IN    nr of msec to wait
);

// Translate (Linux) errno value to corresponding DFSee + DOS/OS2/WIN like RC
ULONG TxRcFromErrno                             // RET    TX_ return-code
(
   int                 err                      // IN    error number (errno)
);

#else
   //- OS/2 specific utility functions

typedef ULONG (APIENTRY * TXF_DOSOPENL)         // Large-file open ( > 2Gb)
(                                               // note: APIENTRY essential to
   PSZ                 pszFileName,             // force calling convention
   PHFILE              phf,                     // with all params on stack!
   PULONG              pulAction,
   LONGLONG            cbFile,
   ULONG               ulAttribute,
   ULONG               fsOpenFlags,
   ULONG               fsOpenMode,
   PEAOP2              peaop2
);

typedef ULONG (APIENTRY * TXF_DOSSEEKL)         // Large-file seek ( > 2Gb)
(
   HFILE               hFile,
   LONGLONG            ib,
   ULONG               method,
   PLONGLONG           ibActual
);

typedef ULONG (APIENTRY * TXF_DOSSFSIZEL)       // Large-file set-size ( > 2Gb)
(
   HFILE               hFile,
   LONGLONG            size
);

typedef struct txf_os2lfapi                     // entrypoint structure
{
   TXF_DOSOPENL        DosOpenLarge;            // open
   TXF_DOSSEEKL        DosSeekLarge;            // seek
   TXF_DOSSFSIZEL      DosSetFileSizeLarge;     // set size
} TXF_OS2LFAPI;                                 // end of struct "txf_os2lfapi"


// Test if OS2 large-file support (> 2GiB) is available; Fill entrypoints
BOOL TxLargeFileApiOS2                          // RET   large file API's OK
(
   TXF_OS2LFAPI       *entrypoints              // OUT   LF-API entrypoints
);                                              // needed LargeFile API's (3)
#endif

#include <txwin.h>                              // text-mode windowing

#endif
