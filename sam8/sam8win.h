// SAM8 windowed user interface
//
// Author: J. van Wijk
//
// 1.00 14-02-2008    Initial version
//
#ifndef    SAMWIN
   #define SAMWIN

#define SAMH_GENERIC     0
#define SAMH_CONFIRM   5000

//- enable or mark menu-items by ID
#define txtMiEnable(i,c) txwMiEnable(&mainmenu,(i),(c))
#define txtMiMarked(i,c) txwMiMarked(&mainmenu,(i),(c))

//- menu (equal to help) and accelerator codes
#define SAMH_MENUS     3000                     // menu help base
#define SAMM_BAR       3010
#define SAMM_DEFAULT   3030
#define SAMM_AUTOMENU  3040

#define SAMM_FILE      3100
#define SAMC_OPEN      3110
#define SAMC_SAVE      3120
#define SAMC_RUNS      3130
#define SAMC_EXIT      3140


#define SAMM_HELP      3900
#define SAMC_CMDHELP   3910
#define SAMC_SW_HELP   3920
#define SAMC_UIHELP    3930
#define SAMC_ABOUT     3940




// Start and maintain SAMPLE windowed user-interface
ULONG samWindowed
(
   char               *initial                  // IN    initial SAMPLE cmd
);


#endif
