#define SAM_D "Scrollbuffer output, entryfield and menu sample."

#define SAM_C "(c) 2008-2018: Jan van Wijk"

#define SAM_V "2.01 07-10-2018" // Minor update for TXLib 5.x
//efine SAM_V "2.00 13-06-2014" // Minor update for TXLib 2.0
//efine SAM_V "1.00 14-02-2008" // Initial version

#include <txlib.h>                              // TX library interface

#if   defined (WIN32)
   #define SAM_N "SAM8 Win32"
#elif defined (DOS32)
   #define SAM_N "SAM8 Dos32"
#elif defined (LINUX)
   #define SAM_N "SAM8 Linux"
#elif defined (DARWIN)
   #define SAM_N "SAM5 macOS"
#else
   #define SAM_N "SAM8  OS/2"
#endif


#ifndef    SAM8_H
   #define SAM8_H

// Long option name constants
#define SAM_O_LOGAUTO     TXA_O_APP0            // Automatic logfile numbering ON
#define SAM_O_EXPERT      TXA_O_APP1            // Expert UI mode

#define SAM_CMD_FAILED    ((USHORT) 901)        // Generic cmd failure code
#define SAM_ALLOC_ERROR   ((USHORT) 906)        // memory allocation error
#define SAM_PENDING       ((USHORT) 907)        // function pending
#define SAM_QUIT          ((USHORT) 909)        // quit TXT interactive


typedef struct saminf                           // information anchor block
{
   BOOL                batch;                   // batch-mode active
   BOOL                dialogs;                 // dialogs will be used
   ULONG               verbosity;               // output verbosity
   int                 eStrategy;               // error strategy
   ULONG               retc;                    // overall return-code
   USHORT              sbWidth;                 // visible scroll-buffer width
   USHORT              sbLength;                // visible scroll-buffer length
   BOOL                autoquit;                // auto quit after fdisk/setb
   BOOL                regconfirm;              // registration conf required
   BOOL                nowindow;                // running in classic mode
   TXCELL             *sbbuf;                   // actual scroll buffer
   ULONG               sbsize;                  // actual scrollbuf size
   ULONG               sblwidth;                // actual sb linelength
   TXWHANDLE           sbwindow;                // scroll-buffer window
   TXWHANDLE           menuOwner;               // menu handling window
   BOOL                expertui;                // EXPERT menu/dialog UI (not BASIC)
   BOOL                automenu;                // automatic menu activation
   BOOL                autodrop;                // automatic pulldown drop
   ULONG               menuopen;                // default drop-down menu
   ULONG               worklevel;               // when 0, activate menu
   BOOL                logAuto;                 // Automatic logfile numbering from DLG
   TXSELIST           *slSchemes;               // selection list, Color schemes
} SAMINF;                                       // end of struct "txtinf"

extern  SAMINF     *sama;                       // SAM anchor block

extern  char       *SamSwitchhelp[];
extern  char       *samGenericHelp[];

#define SAM_PROFILE     "profile.txt"

// scroll to end (output), cancel-abort and execute a new command
#define samExecEnd(c) txwSendMsg( hwnd, TXWM_CHAR, 0, (TXWMPARAM) TXc_END),  \
                      TxCancelAbort(),                                       \
                      txtMultiCommand((c), 0, TRUE, TRUE, FALSE)

// cancel-abort and execute a new command
#define samExecCmd(c) TxCancelAbort(),                                       \
                      samMultiCommand((c), 0, TRUE, TRUE, FALSE)


// cancel-abort and execute a new command, NO echo or prompting
#define samExecSilent(c) TxCancelAbort(),                                    \
                         samMultiCommand((c), 0, FALSE, FALSE, TRUE)

//- WARNING: Never execute two txtExec... from the menu, that will
//-          cause the menu to restart after the 1st, causing hangs
//-          Use a real "multiple command" in one go instead.

#define samBEGINWORK() {sama->worklevel++;}
#define samENDWORK()   {                                                     \
                          if (sama->worklevel)                               \
                          {                                                  \
                             if ((--(sama->worklevel) == 0) &&               \
                                    (sama->menuOwner  != 0)  )               \
                             {                                               \
                                txwPostMsg( sama->menuOwner,                 \
                                 TXWM_COMMAND, (TXWMPARAM) SAMM_DEFAULT, 0); \
                             }                                               \
                          }                                                  \
                       }


// Print SAMPLE logo+status, do startup checks, run startup commands + profile
ULONG samStartupLogo                            // RET   Checks and firstcmd RC
(
   char               *firstcmd                 // IN    initial command
);


// Execute multiple sample-commands separated by # characters
ULONG samMultiCommand
(
   char               *cmdstring,               // IN    multiple command
   ULONG               lnr,                     // IN    linenumber or 0
   BOOL                echo,                    // IN    Echo  before execute
   BOOL                prompt,                  // IN    prompt after execute
   BOOL                quiet                    // IN    screen-off during cmd
);

#endif
