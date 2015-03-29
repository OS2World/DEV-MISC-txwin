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
// TxLib filesystem functions, base section
//

#include <txlib.h>                              // TxLib interface
#include <txwpriv.h>                            // private window interface

#include <sys/stat.h>                           // for low level stuff


#if defined (USEWINDOWING)
// Sorting criteria attached to each volume list
static TXSELSORT       txfd_vol_sort =
{
  {                                             // actual, initial sort order
     TXS_SORT_4,                                // desc ascending case-insensitive letter
     TXS_SORT_TEXT | TXS_SORT_IGNCASE | TXS_SORT_ASCEND  | 23,  //- (Vsize)
     0,
     0,
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_ASCEND  |  7,  //- (letter)
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_ASCEND  | 23,  //- (fsys)
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_ASCEND  | 10,  //- (label)
     TXS_SORT_DESC | TXS_SORT_IGNCASE | TXS_SORT_ASCEND  | 37,  //- (Fspace)
  },
  {                                             // reverse   (c-F8 / c-R)
     "unsorted   -",                            // unsorted  (c-F7 / c-U)
     "total size -",                            // text   1  (c-F2 / c-X)
     "",                                        // text   2  (c-F1 / c-D)
     "",                                        // text   3  (c-F9 / c-A)
     "dr. letter -",                            // desc   4  (c-F5 / c-T / c-L)
     "filesystem -",                            // desc   5  (c-F6 / c-B / c-S / c-F)
     "vol. label -",                            // desc   6  (c-F3 / c-N)
     "free space -",                            // desc   7  (c-F4 / c-E)
  },
  ""                                            // current description
};
#endif


#if   defined (WIN32)

// OpenWatcom older than 1.8 (1280) did not have these ...
#if __WATCOMC__ < 1280
//- stuff needed for (DDK) IOCTL (conflicts if including whole ntddstor.h)

#pragma pack(4)
typedef enum _STORAGE_PROPERTY_ID
{
  StorageDeviceProperty = 0,
  StorageAdapterProperty,
  StorageDeviceIdProperty
} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

typedef enum _STORAGE_QUERY_TYPE
{
  PropertyStandardQuery = 0,
  PropertyExistsQuery,
  PropertyMaskQuery,
  PropertyQueryMaxDefined
} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

typedef struct _STORAGE_PROPERTY_QUERY
{
  STORAGE_PROPERTY_ID  PropertyId;
  STORAGE_QUERY_TYPE  QueryType;
  UCHAR  AdditionalParameters[1];
} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;


typedef enum _STORAGE_BUS_TYPE
{
        BusTypeUnknown = 0x00,
        BusTypeScsi,
        BusTypeAtapi,
        BusTypeAta,
        BusType1394,
        BusTypeSsa,
        BusTypeFibre,
        BusTypeUsb,
        BusTypeRAID,
        BusTypeMaxReserved = 0x7F
} STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;


typedef struct _STORAGE_DEVICE_DESCRIPTOR
{
  ULONG  Version;
  ULONG  Size;
  UCHAR  DeviceType;
  UCHAR  DeviceTypeModifier;
  BOOLEAN  RemovableMedia;
  BOOLEAN  CommandQueueing;
  ULONG  VendorIdOffset;
  ULONG  ProductIdOffset;
  ULONG  ProductRevisionOffset;
  ULONG  SerialNumberOffset;
  STORAGE_BUS_TYPE  BusType;
  ULONG  RawPropertiesLength;
  UCHAR  RawDeviceProperties[1];
} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;


#define IOCTL_STORAGE_QUERY_PROPERTY \
  CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack()
#endif // watcom older than 1.8

#elif defined (DOS32)

#define DRIVETYPE_REMOTE  0x1000                // BIT 12 from Ioctl 4409 dx
#define DRIVETYPE_RAMDRV  0x0800                // BIT 11 from Ioctl 4409 dx

// Return data structure for DOS INT21 function 7303
typedef struct _dos_disksize_ex
{
   USHORT              size;                    // size of structure
   USHORT              version;                 // IN/OUT structure version (0)
   ULONG               Spc;                     // Sectors per cluster  (compr)
   ULONG               Bps;                     // Bytes per Sector
   ULONG               ClusAvail;               // Available clusters
   ULONG               ClusTotal;               // Total     clusters
   ULONG               SectAvail;               // Available sectors
   ULONG               SectTotal;               // Total     sectors
   ULONG               CcprAvail;               // Available clusters   (compr)
   ULONG               CcprTotal;               // Total     clusters   (compr)
   ULONG               Reserved1;
   ULONG               Reserved2;
} DOS_DISKSIZE_EX;                              // end of struct "_dos_disksize_ex"


#elif defined (DARWIN)
#elif defined (LINUX)

#define BLKGETSIZE        0x1260
#define BLKSSZGET         0x1268

#define ROOT_DEVICE       "/dev/root"

// Translate root-devicename to disk-partition device name using cached /etc/fstab info
static BOOL TxLinuxRootDevice                   // RET   rootdevice translated
(
   char               *devname,                 // IN    Line with a device-name
   char               *root                     // OUT   Real root-device name
);

#else

#define TXFSDC_BLOCKR         0x00              // block device removable
#define TXFSDC_GETBPB         0x00              // get device bpb info

#define TXFSDC_UNLOCK         0x00              // unlock logical drive
#define TXFSDC_LOCK           0x01              // lock logical drive
#define TXFSDC_EJECT          0x02              // eject removable
#define TXFSDC_LOAD           0x03              // load removable

typedef struct drivecmd
{
   BYTE                cmd;                     // 0=unlock 1=lock 2=eject
   BYTE                drv;                     // 0=A, 1=B 2=C ...
} DRIVECMD;                                     // end of struct "drivecmd"

#define TXFSBPB_REMOVABLE     0x08              // BPB attribute for removable

typedef struct drivebpb
{
   TXFS_EBPB           ebpb;                    // extended BPB
   BYTE                reserved[6];
   USHORT              cyls;
   BYTE                type;
   USHORT              attributes;              // device attributes
   BYTE                fill[6];                 // documented for IOCtl
} DRIVEBPB;                                     // end of struct "drivebpb"

#endif


/*****************************************************************************/
// Build string with present volumes, optional FLOPPY and LAN drives included
/*****************************************************************************/
int TxFsVolumes                                 // RET   nr of drives listed
(
   ULONG               flags,                   // IN    FLOPPY/LAN/CD select
   char               *vols                     // OUT   Present volumes
)
{
   #if   defined (WIN32)
   #elif defined (DOS32)
      union  REGS      regs;
      char            *ep;
   #elif defined (UNIX)
      //- to be refined
   #else
      ULONG            current;
   #endif
   USHORT              first = 2;               // default start at C:
   USHORT              ml;
   ULONG               drivemap = 0;
   TXTM                drive;

   ENTER();
   TxFsAutoFailCriticalErrors( TRUE);           // avoid Not-ready pop-ups

   if (flags & TXFSV_FLOP)                      // check floppies too
   {
      #if defined (DOS32)
         ep = getenv( "FLOPPYDR");              // DFSee bootable CD/diskette ?
         if (ep && (!strncasecmp( ep, "B:", 2)))   // Booted from FSYS boot CDROM
         {                                      // bootimage is A: - VERY SLOW!
            first = 1;                          // so start at B:
         }
         else                                   // in all other cases
         {                                      // start at diskette A:
            first = 0;
         }
      #else
         first = 0;
      #endif
   }

   #if   defined (WIN32)
      drivemap = GetLogicalDrives();
   #elif defined (DOS32)
      for (ml = first; ml < 26; ml++)           // drives A/B/C to Z
      {
         TxxClearReg( regs);
         regs.h.al = 0x09;                      // block device, remote
         regs.h.bl = (BYTE) (ml+1);             // 1=A, 2=B etc
         TxxDosInt21( regs, TXDX_DOS_IOCTL);
         if (regs.x.cflag == 0)                 // drive exists
         {
            //- to be refined, might use bit 12 (remote) as LAN indicator
            //- and perhaps the MSCDEX calls for CDROM filtering

            TRACES(( "Block device drivetype: 0x%4.4lx for drive: %c\n",
                      TXWORD.dx, (char) (ml + 'A')));
            drivemap |= (1 << ml);

            // TRHEXS( 70, &gpar, sizeof(TXFS_GPARM), "TXFS_GPARM");
         }
      }
   #elif defined (UNIX)
      //- to be refined
   #else
      DosQCurDisk( &current, &drivemap);
   #endif
   strcpy( vols, "");
   for (ml = first; ml < 26; ml++)              // drives A/B/C to Z
   {
      if (drivemap & (1 << ml))
      {
         TXTT       fstype;

         sprintf(drive, "%c:", 'A' + ml);
         if (TxFsType( drive, fstype, NULL))
         {
            BOOL       removable = TxFsIsRemovable( drive);
            BOOL       incl = TRUE;

            //- Note, connected Win-NT drives use the REAL FS-name like NTFS!
            if      ((strncasecmp( fstype, "LAN",    3) == 0) || //- OS/2 LAN drives
                     (strncasecmp( fstype, "NDFS32", 5) == 0) || //- Netdrive drives
                     (strncasecmp( fstype, "REMOTE", 5) == 0)  ) //- NT disconnected
            {                                                    //- DOS REMOTE
               incl = (flags & TXFSV_LAN);
            }
            else if ((strncasecmp( fstype, "CD",  2) == 0)  || // OS2:CDFS, DOS/WIN:CDROM
                     (strncasecmp( fstype, "UDF", 3) == 0)   ) // OS2:UDF DVD's
            {
               removable = TRUE;                // overrule for any CDROM/DVD
               incl      = (flags & TXFSV_CD);
            }
            if ((flags & TXFSV_REM) && (removable == FALSE))
            {
               incl = FALSE;                    // just want removables ...
            }
            if (incl)
            {
               drive[1] = 0;                    // clip to just the letter
               strcat( vols, drive);            // and add to volume-list
            }
         }
      }
   }
   TxFsAutoFailCriticalErrors( FALSE);          // enable criterror handler
   TRACES(("Volumes: '%s'\n", vols));
   RETURN(strlen( vols));
}                                               // end 'TxFsVolumes'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get volumelabel for specified driveletter, or -not-ready- if not accessible
/*****************************************************************************/
char *TxFsVolumeLabel                           // RET   label or "-not-ready-"
(
   char               *drive,                   // IN    Drive specification
   char               *label                    // INOUT Label string (min 12)
)
{
   #if   defined (WIN32)
      TXTS             DriveRoot;
      ULONG            ulMaxCompLen;
      ULONG            ulFsFlags;
      TXTS             fsname;
   #elif defined (DOS32)
      union  REGS      regs;
      struct SREGS     sreg;
      TXDX_RMINFO      txdx_rmi;
      char            *fn;
      char            *dta;
   #elif defined (UNIX)
      //- to be refined
   #else
      FSINFO           fsi;
   #endif

   ENTER();

   strcpy( label, TXFS_NOTREADY);
   #if   defined (WIN32)
      sprintf(DriveRoot, "%c:\\", drive[0]);
      if (!GetVolumeInformation( DriveRoot,
                                 label,  TXMAXTS,
                                 NULL, &ulMaxCompLen, &ulFsFlags,
                                 fsname, TXMAXTS))
      {
         TRACES(("Get failed, label: '%s' error: %lu\n", label, GetLastError()));
      }
   #elif defined (DOS32)
      if ((txwa->dpmi1 != NULL) && (txwa->dpmi2 != NULL))
      {
         memset( txwa->dpmi1, 0, 512);
         memset( txwa->dpmi2, 0, 512);
         fn    = txwa->dpmi1;                   // DPMI compatible spec
         dta   = txwa->dpmi2;                   // DPMI compatible DTA

         sprintf( fn, "%c:\\*.*", drive[0]);    // DPMI comp drive-spec

         //- first DOS-int, 211A = Set DTA
         memset( &regs,  0, sizeof(regs));
         memset( &sreg,  0, sizeof(sreg));

         memset( &txdx_rmi, 0, sizeof(txdx_rmi));
         txdx_rmi.eax = 0x1A00;                 // Set DTA address AH=1A
         txdx_rmi.ds  = txDpmiSegment(dta);     // in  DTA   (ds:dx, dx=0)

         TRHEXS( 100,  &txdx_rmi,  sizeof(txdx_rmi), "txdx_rmi");

         regs.w.ax    = TXDX_DPMI_RMINT;        // simulate realmode INT
         regs.h.bl    = TXDX_DOS;               // DOS interrupt 21
         regs.x.edi   = FP_OFF( &txdx_rmi);     // real mode register struct
         sreg.es      = FP_SEG( &txdx_rmi);

         txDpmiCall( &regs, &sreg);

         TRACES(("regs.x.cflag:%4.4hx  ax:%4.4hx\n", regs.x.cflag, TXWORD.ax));

         if (regs.x.cflag == 0)                 // DTA is set now
         {
            //- second DOS-int, 214E = FindFirst
            memset( &regs,  0, sizeof(regs));
            memset( &sreg,  0, sizeof(sreg));

            memset( &txdx_rmi, 0, sizeof(txdx_rmi));
            txdx_rmi.eax = 0x4E00;              // FindFirst       AH=4E
            txdx_rmi.ecx = FATTR_LABEL;         // in  attribute
            txdx_rmi.ds  = txDpmiSegment(fn);   // in  spec  (ds:dx, dx=0)

            TRHEXS( 100,  &txdx_rmi,  sizeof(txdx_rmi), "txdx_rmi");

            regs.w.ax    = TXDX_DPMI_RMINT;     // simulate realmode INT
            regs.h.bl    = TXDX_DOS;            // DOS interrupt 21
            regs.x.edi   = FP_OFF( &txdx_rmi);  // real mode register struct
            sreg.es      = FP_SEG( &txdx_rmi);

            txDpmiCall( &regs, &sreg);

            TRHEXS( 100,  txwa->dpmi2,  512, "dpmi2 (DTA)");
            TRACES(("regs.x.cflag:%4.4hx  ax:%4.4hx\n", regs.x.cflag, TXWORD.ax));

            if (regs.x.cflag == 0)
            {
               strcpy( label, dta + 0x1e);      // label OUT
            }
         }
      }
   #elif defined (UNIX)
      //- to be refined
   #else
      if (DosQueryFSInfo( *drive - 'A' +1, FSIL_VOLSER,
                          &fsi, sizeof(FSINFO)) == NO_ERROR)
      {
         strcpy( label, fsi.vol.szVolLabel);
      }
   #endif
   TRACES(("label: '%s'\n", label));
   RETURN( label);
}                                               // end 'TxFsVolumeLabel'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Show volume/filesystem info for all registred drive-letters
/*****************************************************************************/
void TxFsDrivemap
(
   char               *lead,                    // IN    Lead text, max 3 chars
   ULONG               flags                    // IN    FLOPPY/LAN/CD select
)
{
   #if defined   (LINUX)
      FILE            *mnt;
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      TXTT             volumes;
   #endif
   #ifndef DARWIN
      TXLN             line;
      char            *s = NULL;
   #endif

   ENTER();

   #if defined (LINUX)
      if ((mnt = fopen( "/proc/mounts", "rb")) != NULL)
      {
         while (!feof(mnt) && !ferror(mnt))
         {
            if (fgets( line, TXMAXLN, mnt) != NULL)
            {
               if (strncasecmp( line, "/dev/", 5) == 0)
               {
                  //- to be refined, honnor FLOPY/LAN/CD flags ?
                  //- could do additional tests here ...

                  if ((s = strchr( line, ' ')) != NULL)
                  {
                     *s = '\0';
                     TxFsShow( lead, line);
                  }
               }
            }
         }
         fclose( mnt);
      }
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
      //- to be refined, use getmntinfo() with statfs structure
      //- and perhaps some filtering (review Linux too!)
      TxFsShow( lead, "");
   #else
      TxFsVolumes( flags, volumes);             // get available volumes
      for (s = volumes; *s && !TxAbort(); s++)  // walk all volume letters
      {
         sprintf( line, "%c:", *s);
         TxFsShow( lead, line);
      }
   #endif
   VRETURN();
}                                               // end 'TxFsDrivemap'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Show volume/filesystem info on a single output line using TxPrint
/*****************************************************************************/
ULONG TxFsShow
(
   char               *lead,                    // IN    Lead text, max 3 chars
   char               *drive                    // IN    Drive specification
)
{
   ULONG               rc = TX_INVALID_DRIVE;
   #if defined (DARWIN)
   #else
      ULONG            frees;                   // Free sectors
      ULONG            total;                   // Total sectors
      USHORT           bps;                     // Sectorsize
      BOOL             fType = FALSE;
      TXTM             text;
      TXTM             detail;
      #if defined (UNIX)
         TXTM          device;
      #endif
   #endif
   TXTT                leader;

   ENTER();

   strcpy( leader, lead);                       // optional lead fragment
   #if defined (UNIX)
      strcat( leader, "Mounted device");
   #else
      if (TxFsIsRemovable( drive))
      {
         strcat( leader, "RemovableDrive");
      }
      else
      {
         strcat( leader, "Volume info on");
      }
   #endif

   TxFsAutoFailCriticalErrors( TRUE);           // avoid Not-ready pop-ups
   #ifndef UNIX
   if (isupper(drive[0]))
   #endif
   #ifndef DARWIN
   {
      fType = TxFsType( drive, text, detail);
      TxStrip( text, text, 0, ' ');             // strip spaces from FS-name
      if (strcmp(text, "UNKNOWN") != 0)
      {
         TxStrToUpper( text);                   // uppercase for readability
         rc = TxFsSpace(drive, &frees, &total, &bps);
      }
   }
   #endif
   #if defined (LINUX)
      if (rc == NO_ERROR)
      {
         strcpy( device, drive);
         TxLinuxRootDevice(drive, device);      // translate root device
         TxPrint("%-18.18s: %s%-15.15s%s Size% 9.1lf MiB, fs: %s%s%s  %s%s\n",
                  leader, CBG, device, CNN,
                  (total == 0) ? 0 : TXSMIB(total,  bps),
                  CBM, text, CBY, detail, CNN);
      }
      else
      {
         TxPrint("%-18.18s: %s%-15.15s%s is not accessible, fs: %s%s%s\n",
                  leader, CBG, drive, CNN, CBM, (fType) ? text : "-", CNN);
      }
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
      TxPrint("%-18.18s: %s%-15.15s%s info not implemented yet\n",
               leader, CBG, drive, CNN);
   #else
      if (rc == NO_ERROR)
      {
         TxPrint("%-18.18s: %s%c:%s Free% 9.1lf MiB of% 9.1lf MiB, fs: %s%s%s  %s%s\n",
                  leader, CBG, drive[0], CNN,
                  (frees == 0) ? 0 : TXSMIB(frees,  bps),
                  (total == 0) ? 0 : TXSMIB(total,  bps),
                  CBM, text, CBY, detail, CNN);
      }
      else
      {
         TxPrint("%-18.18s: %s%s%s           Not ready, no size info, fs: %s%s%s\n",
                  leader, CBG, drive, CNN, CBM, (fType) ? text : "-", CNN);
      }
   #endif
   TxFsAutoFailCriticalErrors( FALSE);          // enable criterror handler
   RETURN (rc);
}                                               // end 'TxFsShow'
/*---------------------------------------------------------------------------*/


#if defined (USEWINDOWING)
/*****************************************************************************/
// Build selection-list with volume/FS info for drive-letters or Linux-devices
/*****************************************************************************/
TXSELIST *TxFsDriveSelist                       // RET   selection list or NULL
(
   ULONG               flags,                   // IN    include FLOP/LAN/CDROM
   BOOL                flop                     // IN    runtime floppy test
)                                               //       (A: = 0, C: = 2 etc)
{
   TXSELIST           *list  = NULL;            // total list
   #ifndef DARWIN
   TXS_ITEM           *item;                    // single item
   ULONG               entries = 0;             // entries in list
   int                 i;
   #endif
   #if defined (LINUX)
      FILE            *mnt;
      TXLN             line;
      TXTM             device;
      TXTM             mount;
      TXTM             fsys;
      TXTM             rest;
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      TXTM             volumes;
      char             curdrive;
      char            *s;
      char            *startvolume = volumes;
   #endif

   ENTER();

   #if defined (LINUX)
      if ((mnt = fopen( "/proc/mounts", "rb")) != NULL)
      {
         while (!feof(mnt) && !ferror(mnt))
         {
            if (fgets( line, TXMAXLN, mnt) != NULL)
            {
               //- to be refined, honor exclude-floppy and LAN flags ?
               if (strncasecmp( line, "/dev/", 5) == 0)
               {
                  entries++;                    // count valid device names
               }
            }
         }
         if (TxSelCreate( entries, entries, entries,
                          TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
         {
            char           *listdescr;          // list level description

            list->astatus = TXS_AS_NOSTATIC      | // all dynamic allocated
                            TXS_LST_DESC_PRESENT | // with list description
                            TXS_LST_DYN_CONTENTS;

            if ((listdescr  = TxAlloc( 1, TXMAXTM)) != NULL)
            {
               sprintf( listdescr, "mounted device to be used (from /proc/mount)");

               list->userinfo = (ULONG) listdescr;
            }
            fseek( mnt, 0, SEEK_SET);           // rewind to start of file
            for (i = 0; (i < entries) && !feof(mnt) && !ferror(mnt);)
            {
               if (fgets( line, TXMAXLN, mnt) != NULL)
               {
                  if (strncasecmp( line, "/dev/", 5) == 0)
                  {
                     if ((item  = TxAlloc( 1, sizeof(TXS_ITEM))) != NULL)
                     {
                        list->count    = i +1;  // actual item count
                        list->items[i] = item;  // attach item to list

                        if (((item->text = TxAlloc( 1, TXMAXTM)) != NULL) &&
                            ((item->desc = TxAlloc( 1, TXMAXLN)) != NULL)  )
                        {
                           item->value  = TXDID_MAX +  i;
                           item->helpid = TXWH_USE_WIN_HELP; // from list-window itself

                           item->index  = 0;    // no quick-select

                           line[ TXMAXTM -1] = 0;

                           sscanf( line, "%s %s %s %s", device, mount, fsys, rest);
                           TxLinuxRootDevice( line, device); // translate root device
                           rest[ 2] = 0;
                           TxStrToUpper( rest);

                           sprintf( item->text, "%-15s %s", device, fsys);
                           sprintf( item->desc, "%-8.8s %-15.15s mounted %s at: %-35s",
                                                 fsys, device, rest, mount);

                           TRACES(("text: %d '%s'\n", strlen(item->text), item->text));
                           TRACES(("desc: %d '%s'\n", strlen(item->desc), item->desc));
                        }
                     }
                     i++;                       // to next entry
                  }
               }
            }
         }
         fclose( mnt);
      }
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      getcwd( volumes, TXMAXTM);                // get current drive/directory
      curdrive = toupper(volumes[0]);           // keep current driveletter

      TxFsVolumes( flags, volumes);             // get available volumes

      #if defined (DOS32)
         if (TxPhysDisketteDrives() == 1)       // just one drive really there
         {
            if (strncasecmp( volumes, "AB", 2) == 0) // A and B are listed
            {
               startvolume++;                   // quick hack to remove first
               if ((s = getenv("FLOPPYDR")) != NULL)
               {
                  startvolume[0] = s[0];        // env var used by DFSee to
               }                                // indicate floppy driveletter
            }
         }
      #endif
      if ((entries = strlen(startvolume)) != 0)
      {
         TRACES(( "Driveletters to be added to selection: '%s'\n", startvolume));
         if (TxSelCreate( entries, entries, entries,
                          TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
         {
            char           *listdescr;          // list level description

            list->astatus = TXS_AS_NOSTATIC      | // all dynamic allocated
                            TXS_LST_DESC_PRESENT | // with list description
                            TXS_LST_DYN_CONTENTS;


            if ((listdescr  = TxAlloc( 1, TXMAXTM)) != NULL)
            {
               sprintf( listdescr, "%svolume (driveletter) to be used",
                        (flags & TXFSV_REM) ? "removable " : "");

               list->userinfo = (ULONG) listdescr;
            }
            for (i = 0, s = startvolume; i < entries; i++, s++) // walk all volumes
            {
               if ((item  = TxAlloc( 1, sizeof(TXS_ITEM))) != NULL)
               {
                  list->count    = i +1;        // actual item count
                  list->items[i] = item;        // attach item to list
                  item->userdata = list->count; // assign item number for unsort

                  if (((item->text = TxAlloc( 1, TXMAXTT)) != NULL) &&
                      ((item->desc = TxAlloc( 1, TXMAXLN)) != NULL)  )
                  {
                     char    drive[3];

                     sprintf(drive, "%c:", *s);

                     item->value  = TXDID_MAX +  (*s - 'A');

                     item->helpid = TXWH_USE_WIN_HELP; // from list-window itself

                     item->index  = 1;          // drive-letter is quick-select
                     if ((!flop) && (*s < 'C')) // simulate floppy info ?
                     {
                        sprintf( item->text, "%s             FAT",  drive);
                        sprintf( item->desc, "Diskette drive %s   Status "
                                             "not determined yet.", drive);
                     }
                     else
                     {
                        ULONG   rc    = NO_ERROR;
                        ULONG   frees = 0;      // Free sectors
                        ULONG   total = 0;      // Total sectors
                        USHORT  bps;            // Sectorsize
                        TXTM    fsys;
                        TXTM    detail;
                        TXTS    label;

                        TxFsAutoFailCriticalErrors( TRUE); // avoid Not-ready pop-ups
                        TxFsType( drive, fsys, detail);

                        TxFsVolumeLabel( drive, label);
                        if (strcmp( label, TXFS_NOTREADY) != 0) // no label access
                        {
                           if (strcmp( fsys, "UNKNOWN") != 0) // unknown filesystem
                           {
                              rc = TxFsSpace( drive, &frees, &total, &bps);
                           }
                        }
                        else
                        {
                           rc = TX_INVALID_DRIVE;
                        }
                        TRACES(( "Drive %s  label: '%s' fsys: '%s' detail: '%s' RC:%lu\n",
                                  drive, label, fsys, detail, rc));
                        if (rc == NO_ERROR)
                        {
                           sprintf( item->text, "%s %-11.11s %-8.8s %9.1lf MiB",
                                    drive, label, fsys,
                                   (total == 0) ? 0.0 : TXSMIB( total, bps));
                           TRACES(("text: %d '%s'\n", strlen(item->text), item->text));
                           TRACES(("t*:%8.8lx  d*:%8.8lx\n", item->text,  item->desc));
                           TRACES(("total:%lu  frees:%lu  bps:%hu\n", total, frees, bps));

                           sprintf( item->desc, "Drive %s %-11.11s %-8.8s Free: %9.1lf MiB  %s",
                                    drive, label, fsys,
                                    (frees == 0) ? 0.0 : TXSMIB( frees, bps), detail);
                        }
                        else
                        {
                           sprintf( item->text, "%s %-11.11s %-8.8s", drive, label, fsys);
                           TRACES(("text: %d '%s'\n", strlen(item->text), item->text));
                           sprintf( item->desc, "Drive %s   Removable not ready "
                                                "or unknown filesystem", drive);
                        }
                        TxFsAutoFailCriticalErrors( FALSE); // enable criterror handler
                     }
                     TRACES(("desc: %d '%s'\n", strlen(item->desc), item->desc));
                  }
               }
               if (*s == curdrive)
               {
                  list->selected = i;           // make this the selected one
               }
            }
         }
      }
   #endif
   if (list == NULL)                            // empty sofar ?
   {
      list = TxSelEmptyList( "- No volumes matching the criteria -",
            "No volumes found that match the criteria for this list", FALSE);
   }
   if (list != NULL)                            // attach sort information
   {
      list->sortinfo = &txfd_vol_sort;
      TxSelSortCurrent( list);
   }
   RETURN( list);
}                                               // end 'TxFsDriveSelist'
/*---------------------------------------------------------------------------*/
#endif                                          // USEWINDOWING


/*****************************************************************************/
// Determine attached fstype, e.g. HPFS for specified drive
/*****************************************************************************/
BOOL TxFsType                                   // RET   FS type resolved
(
   char               *drive,                   // IN    Drive specification
   char               *fstype,                  // OUT   Attached FS type
   char               *details                  // OUT   details (UNC) or NULL
)
{
   BOOL                rc = FALSE;
   #if defined (WIN32)
      TXTS             DriveRoot;
      ULONG            ulMaxCompLen;
      ULONG            ulFsFlags;
   #elif defined (DOS32)
      union  REGS      regs;
      USHORT           drtype;
   #elif defined (LINUX)
      FILE            *mnt;
      TXLN             line;
      TXTM             mount;
      TXTM             rest;
      TXTM             device;
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      FSQBUFFER2   *fsinfo;                     // Attached FS info
      ULONG         fsdlen = 2048;              // Fs info data length
   #endif

   ENTER();
   strcpy(fstype, "none");
   if (details)
   {
      strcpy(details, "");
   }

   #if defined (WIN32)
      sprintf(DriveRoot, "%c:\\", drive[0]);
      rc = GetVolumeInformation( DriveRoot,
                                 NULL, 0,       // no volume name
                                 NULL,          // no volume serialnr
                                 &ulMaxCompLen,
                                 &ulFsFlags,
                                 fstype,
                                 TXMAXTS);
      if (rc == FALSE)                          // no mounted filesystem
      {
         rc = TRUE;
         switch (GetDriveType(DriveRoot))
         {
            case DRIVE_REMOTE : strcpy(fstype, "REMOTE "); break;
            case DRIVE_CDROM  : strcpy(fstype, "CDROM  "); break;
            case DRIVE_RAMDISK: strcpy(fstype, "RAMDISK"); break;

            default:
               rc = FALSE;
               break;
         }
      }
   #elif defined (DOS32)
      TxxClearReg( regs);
      regs.h.al = 0x09;                         // block device, remote
      regs.h.bl = drive[0] - 'A' +1;            // 1=A, 2=B etc
      TxxDosInt21( regs, TXDX_DOS_IOCTL);
      if (regs.x.cflag == 0)                    // drive exists
      {
         drtype = TXWORD.dx;                    // keep drivetype bits

         TxxClearReg( regs);
         TXWORD.bx = 0;
         TxxMplexInt( regs, TXDX_MPLEX_MSCDEX_INSTALL);
         if (TXWORD.bx != 0)                    // nr of CDROM driveletters
         {
            TxxClearReg( regs);
            TXWORD.cx = drive[0] - 'A';         // 0=A
            TxxMplexInt( regs, TXDX_MPLEX_MSCDEX_DRCHECK);
            if (TXWORD.ax != 0)                 // nonzero if a supported CDROM
            {
               strcpy(fstype, "CDROM  ");
               rc = TRUE;
            }
         }
         if (rc == FALSE)                       // not identified yet
         {
            if (drtype == DRIVETYPE_RAMDRV)     // MS or FreeDOS ramdisk
            {
               strcpy(fstype, "RAMDISK");
               rc = TRUE;
            }
            else if (drtype & DRIVETYPE_REMOTE) // drive is REMOTE
            {
               strcpy(fstype, "REMOTE ");
               rc = TRUE;
            }
            else                                // assume rest is local
            {
               if (drive[0] < 'C')
               {
                  strcpy(fstype, "FAT12  ");    // must be some FAT :-)
               }
               else
               {
                  strcpy(fstype, "FAT(32)");    // must be some FAT :-)
               }
               rc = TRUE;
            }
         }
      }
   #elif defined (LINUX)
      if ((mnt = fopen( "/proc/mounts", "rb")) != NULL)
      {
         int           length = strlen( drive);

         while (!feof(mnt) && !ferror(mnt))
         {
            if (fgets( line, TXMAXLN, mnt) != NULL)
            {
               strcpy( device, "");
               TxLinuxRootDevice(line, device); // translate root device

               if ((strncasecmp( line,   drive, length) == 0) || // direct match
                   (strncasecmp( device, drive, length) == 0)  ) // translated match
               {
                  line[ TXMAXTM -1] = 0;
                  sscanf( line, "%*s %s %s %s", mount, fstype, rest);
                  if (details)
                  {
                     rest[ 2] = 0;
                     TxStrToUpper( rest);
                     sprintf( details, "%s %s", rest, mount);
                  }
                  rc = TRUE;
               }
            }
         }
         fclose( mnt);
      }
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      if ((fsinfo = TxAlloc(1, fsdlen)) != NULL)
      {
         if (DosQFSAttach(drive, 0, 1, fsinfo, &fsdlen) == NO_ERROR)
         {
            strcpy(fstype,      fsinfo->szName + fsinfo->cbName +1);
            if (details && (fsinfo->cbFSAData != 0))
            {
               strcpy( details, fsinfo->szName + fsinfo->cbName +
                                                 fsinfo->cbFSDName +2);
            }
            TRACES(("FS item type  : %hu\n", fsinfo->iType));
            TRHEXS( 70, fsinfo, fsdlen, "FSQBUFFER2");
            rc = TRUE;
         }
         else
         {
            TRACES(("DosQFSAttach for drive '%s' failed\n", drive));
         }
         TxFreeMem(fsinfo);
      }
      else
      {
         TRACES(("TxAlloc buffer for drive '%s' failed\n", drive));
      }
   #endif
   TRACES(("Filesystem name : '%s' for drive: '%s'\n", fstype, drive));
   BRETURN (rc);
}                                               // end 'TxFsType'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Eject a removable medium specified by driveletter
/*****************************************************************************/
ULONG TxFsEjectRemovable
(
   char               *drive                    // IN    Driveletter to eject
)
{
   ULONG               rc = NO_ERROR;           // DOS rc
   #if   defined (WIN32)
   #elif defined (DOS32)
   #elif defined (LINUX)
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      DRIVECMD         IOCtl;
      ULONG            DataLen;
      ULONG            ParmLen;
   #endif

   ENTER();
   TxFsAutoFailCriticalErrors( TRUE);           // avoid Not-ready pop-ups

   TRACES(("Drive: %s\n", drive));
   #if   defined (WIN32)
   #elif defined (DOS32)
   #elif defined (LINUX)
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      ParmLen   = sizeof(DRIVECMD);
      IOCtl.cmd = TXFSDC_EJECT;
      IOCtl.drv = toupper(drive[0]) - 'A';
      DataLen   = 0;

      rc = DosDevIOCtl((HFILE) -1, IOCTL_DISK,
                       DSK_UNLOCKEJECTMEDIA,
                       &IOCtl, ParmLen, &ParmLen,
                       NULL,   DataLen, &DataLen);
   #endif
   TxFsAutoFailCriticalErrors( FALSE);          // enable criterror handler
   RETURN (rc);
}                                               // end 'TxFsEjectRemovable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if a driveletter represents a removable medium/device
/*****************************************************************************/
BOOL TxFsIsRemovable                            // RET   drive is removable
(
   char               *drive                    // IN    Driveletter to test
)
{
   BOOL                rc = FALSE;
   #if   defined (WIN32)
      TXTS             dspec;
      TXHFILE          dh   = 0;                // Direct Filehandle
   #elif defined (DOS32)
   #elif defined (LINUX)
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      DRIVECMD         IOCtl;
      DRIVEBPB         RemAt;
      ULONG            DataLen;
      ULONG            ParmLen;
      BYTE             NoRem;
   #endif

   ENTER();
   TxFsAutoFailCriticalErrors( TRUE);           // avoid Not-ready pop-ups

   TRACES(("Drive: %s\n", drive));
   #if   defined (WIN32)
      sprintf(dspec, "%c:\\", drive[0]);
      switch (GetDriveType(dspec))
      {
         case DRIVE_REMOVABLE:
            rc = TRUE;
            break;

         default:
            sprintf( dspec, "\\\\.\\%c:", drive[0]); // logical volume specification
            dh = CreateFile( dspec, GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (dh == INVALID_HANDLE_VALUE)     // try read-only access
            {
               dh = CreateFile( dspec, GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            }
            if (dh != INVALID_HANDLE_VALUE)
            {
               ULONG         received;          // nr of bytes received
               STORAGE_PROPERTY_QUERY    query; // query request
               STORAGE_DEVICE_DESCRIPTOR sdd;   // device descriptor

               TRACES(( "Opened '%s' with handle: %lu\n", dspec, dh));

               query.PropertyId = StorageDeviceProperty;
               query.QueryType  = PropertyStandardQuery;
               sdd.Size = sizeof( STORAGE_DEVICE_DESCRIPTOR);

               if (DeviceIoControl(dh,
                                   IOCTL_STORAGE_QUERY_PROPERTY,
                                   &query, sizeof(query),
                                   &sdd,   sdd.Size,
                                   &received, NULL))
               {
                  TRACES(( "BusType: %u Rem:%lu\n", sdd.BusType, sdd.RemovableMedia));
                  if ((sdd.RemovableMedia)        ||
                      (sdd.BusType == BusTypeUsb) ||
                      (sdd.BusType == BusType1394) )
                  {
                     rc = TRUE;
                  }
               }
               else
               {
                  TRACES(( "IOCTL_STORAGE_QUERY_PROPERTY failed for '%s'\n", dspec));
               }
               CloseHandle( dh);
            }
            break;
      }
   #elif defined (DOS32)
   #elif defined (LINUX)
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      ParmLen   = sizeof(IOCtl);
      IOCtl.cmd = TXFSDC_BLOCKR;
      IOCtl.drv = toupper(drive[0]) - 'A';
      DataLen   = sizeof(NoRem);

      if (DosDevIOCtl((HFILE) -1, IOCTL_DISK,
                                  DSK_BLOCKREMOVABLE,
                                  &IOCtl, ParmLen, &ParmLen,
                                  &NoRem, DataLen, &DataLen) == NO_ERROR)
      {
         if (NoRem)                             // non-removable sofar, check
         {                                      // BPB as well (USB devices)
            ParmLen   = sizeof(IOCtl);
            IOCtl.cmd = TXFSDC_GETBPB;
            IOCtl.drv = toupper(drive[0]) - 'A';
            DataLen   = sizeof(RemAt);

            if (DosDevIOCtl((HFILE) -1, IOCTL_DISK,
                                        DSK_GETDEVICEPARAMS,
                                        &IOCtl, ParmLen, &ParmLen,
                                        &RemAt, DataLen, &DataLen) == NO_ERROR)

            {
               if (RemAt.attributes & TXFSBPB_REMOVABLE)
               {
                  TRACES(( "BPB removable, like USB\n"));
                  rc = TRUE;                    // removable, probably USB
               }
            }
         }
         else
         {
            TRACES(( "Block removable\n"));
            rc = TRUE;                          // removable block device
         }
      }
   #endif
   TxFsAutoFailCriticalErrors( FALSE);          // enable criterror handler
   BRETURN (rc);
}                                               // end 'TxFsIsRemovable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine free and total-available space (sectors) on specified drive
/*****************************************************************************/
ULONG TxFsSpace
(
   char               *drive,                   // IN    Drive specification
   ULONG              *sfree,                   // OUT   Free sectors
   ULONG              *stotal,                  // OUT   Total sectors
   USHORT             *bpsect                   // OUT   Sectorsize
)
{
   ULONG               rc = NO_ERROR;
   #if defined (WIN32)
      TXTS             DriveRoot;
      ULONG            cSectorUnit;
      ULONG            cUnitAvail;
      ULONG            cUnit;
      ULONG            cbSector;
   #elif defined (DOS32)
      union  REGS      regs;
      struct SREGS     sreg;
      TXDX_RMINFO      txdx_rmi;
      char            *dRoot;
      DOS_DISKSIZE_EX *sizex;
   #elif defined (LINUX)
      TXHFILE          dh = 0;                  // Direct Filehandle
      TXTM             device;
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      FSALLOCATE       FSAllocate;
   #endif

   ENTER();

   *sfree  = 0;
   *stotal = 0;
   *bpsect = 512;

   #if defined (WIN32)
      sprintf(DriveRoot, "%c:\\", drive[0]);
      if (GetDiskFreeSpace(DriveRoot,
             &cSectorUnit, &cbSector, &cUnitAvail, &cUnit))
      {
         *sfree   = cSectorUnit * cUnitAvail;
         *stotal  = cSectorUnit * cUnit;
         *bpsect  = (USHORT)      cbSector;
      }
      else
      {
         rc = TX_INVALID_DRIVE;
      }
   #elif defined (DOS32)

      if ((txwa->dpmi1 != NULL) && (txwa->dpmi2 != NULL))
      {
         memset( txwa->dpmi1, 0, 512);
         memset( txwa->dpmi2, 0, 512);
         dRoot = (char *)            txwa->dpmi1; // DPMI compatible spec
         sizex = (DOS_DISKSIZE_EX *) txwa->dpmi2; // DPMI compatible data

         sprintf(dRoot, "%c:\\", drive[0]);     // path of drive root

         sizex->size    = sizeof(DOS_DISKSIZE_EX);
         sizex->version = 0;

         memset( &txdx_rmi, 0, sizeof(txdx_rmi));
         txdx_rmi.eax = TXDX_DOS_FREESPACE_EX;
         txdx_rmi.ecx = sizeof(DOS_DISKSIZE_EX);
         txdx_rmi.ds  = txDpmiSegment(dRoot);   // in  spec    (ds:dx, dx=0)
         txdx_rmi.es  = txDpmiSegment(sizex);   // return data (es:di, di=0)

         TRHEXS( 500,  &txdx_rmi,  sizeof(txdx_rmi), "txdx_rmi");

         memset( &regs,  0, sizeof(regs));
         memset( &sreg,  0, sizeof(sreg));
         regs.w.ax    = TXDX_DPMI_RMINT;        // simulate realmode INT
         regs.h.bl    = TXDX_DOS;               // DOS interrupt 21
         regs.x.edi   = FP_OFF( &txdx_rmi);
         sreg.es      = FP_SEG( &txdx_rmi);

         txDpmiCall( &regs, &sreg);

         if ((regs.x.cflag == 0) && (TXWORD.ax == 0x0300)) // FreeDOS result OK
         {
            *sfree   =          sizex->SectAvail;
            *stotal  =          sizex->SectTotal;
            *bpsect  = (USHORT) sizex->Bps;
         }
      }
      if (*sfree == 0)                          // not set, use classic call
      {
         TxxClearReg( regs);
         regs.h.dl = (BYTE) ((toupper(drive[0]) - 'A') +1);
         TxxDosInt21( regs, TXDX_DOS_FREESPACE);
         if (TXWORD.ax != 0xffff)               // drive exists
         {
            *sfree   = (ULONG)  TXWORD.ax * TXWORD.bx;
            *stotal  = (ULONG)  TXWORD.ax * TXWORD.dx;
            *bpsect  = (USHORT) TXWORD.cx;
         }
         else
         {
            rc = TX_INVALID_DRIVE;
         }
      }
   #elif defined (LINUX)
      strcpy( device, drive);
      TxLinuxRootDevice( device, device);       // translate root device
      if ((dh = open( device, O_RDONLY | O_LARGEFILE)) != -1)
      {
         ULONG      size = 0;                   // ioctl size return value

         //- Note: BLKSSZGET ioctl is NOT reliable on partition devices
         //-       As far as I can tell it will return 4096 on those ...
         //-       It does NOT report an error though.

         /*
         if (ioctl( dh, BLKSSZGET, &size) == 0)
         {
            *bpsect = (USHORT) size;
         }
         else
         {
            TRACES(( "BLKSSZGET ioctl handle %lu: %s\n", dh, strerror(errno)));
         }
         */

         if (ioctl( dh, BLKGETSIZE, &size) == 0)
         {
            *stotal = size;                     // total size in sectors
            *sfree  = 0;                        // info not available ...
         }
         else
         {
            TRACES(( "BLKSSZGET ioctl handle %lu: %s\n", dh, strerror(errno)));
         }
         close( dh);
      }
      else
      {
         rc = TX_INVALID_DRIVE;
      }
   #elif defined (DARWIN)                       // DARWIN MAC OS X (GCC)
   #else
      rc=DosQFSInfo((toupper(drive[0]) - 'A') +1, FSIL_ALLOC,
                     &FSAllocate, sizeof(FSAllocate));
      if (rc == NO_ERROR)
      {
         *sfree   = FSAllocate.cSectorUnit * FSAllocate.cUnitAvail;
         *stotal  = FSAllocate.cSectorUnit * FSAllocate.cUnit;
         *bpsect  = FSAllocate.cbSector;
      }
   #endif
   TRACES(("Free: %8.8lx, Total: %8.8lx = %.1lf MiB, bps: %hu\n",
            *sfree, *stotal, TXSMIB( *stotal, *bpsect), *bpsect));
   RETURN (rc);
}                                               // end 'TxFsSpace'
/*---------------------------------------------------------------------------*/


#if defined (LINUX)
/*************************************************************************************************/
// Translate root-devicename to disk-partition device name using cached /etc/fstab info
/*************************************************************************************************/
static BOOL TxLinuxRootDevice                   // RET   rootdevice translated
(
   char               *devname,                 // IN    Line with a device-name
   char               *root                     // OUT   Real root-device name
)
{
   BOOL                rc = FALSE;              // function return
   FILE               *fh;
   static TXTM         rootdev = "";            // cached root device string
   TXLN                line;
   TXTM                name;                    // device-name value
   TXTM                mount;                   // mount location

   ENTER();

   if (strncmp( devname, ROOT_DEVICE, strlen(ROOT_DEVICE)) == 0)
   {
      if (strlen(rootdev) != 0)
      {
         strcpy( root, rootdev);
         rc = TRUE;
      }
      else
      {
         if ((fh = fopen( "/etc/fstab", "rb")) != NULL)
         {
            while (!feof(fh) && !ferror(fh))
            {
               if (fgets( line, TXMAXLN, fh) != NULL)
               {
                  sscanf( line, "%s %s %*s", name, mount);
                  TRACES(( "mount '%s' for line: '%s'\n", mount, line));

                  if (strcmp( mount, "/") == 0) // mounted to root
                  {
                     strcpy( rootdev, name);
                     strcpy( root, rootdev);    // cache found value
                     rc = TRUE;
                  }
               }
            }
            fclose( fh);
         }
      }
   }
   BRETURN (rc);
}                                               // end 'TxLinuxRootDevice'
/*-----------------------------------------------------------------------------------------------*/
#endif

