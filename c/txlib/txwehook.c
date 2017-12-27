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
// Author: J. van Wijk
//
// TX simple event hook implementation, for multi-thread communication
// like passing status or progress information to UI dialogs
//
// JvW  02-12-2005   Initial version

#include <txlib.h>                              // public interface
#include <txwpriv.h>                            // private window interface


#if defined (USEWINDOWING)

// Note: can be refined later to have a list of hooks per event-type

// Simple event hook data structure (single hook per event-type)
static TXWHANDLE      txwEventHookHandle[TXHK_EVENTS] = {TXHWND_NULL};


/*****************************************************************************/
// Attach specified hook-event to a window-handle
/*****************************************************************************/
ULONG txwAttachEventHook                        // RET   result
(
   ULONG               hookid,                  // IN    TXHK identifier
   TXWHANDLE           hwnd                     // IN    window handle
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if (hookid < TXHK_EVENTS)
   {
      if (txwEventHookHandle[ hookid] == TXHWND_NULL)
      {
         TRACES(( "Attaching hwnd: %8.8lx for event: %lu\n", hwnd, hookid));
         txwEventHookHandle[ hookid] = hwnd;
      }
      else                                      // slot is in use
      {
         rc = TX_FAILED;
      }
   }
   else
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'txwAttachEventHook'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Detach specified hook-event from a window-handle
/*****************************************************************************/
ULONG txwDetachEventHook                        // RET   result
(
   ULONG               hookid,                  // IN    TXHK identifier
   TXWHANDLE           hwnd                     // IN    window handle
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if (hookid < TXHK_EVENTS)
   {
      if (txwEventHookHandle[ hookid] == hwnd)
      {
         TRACES(( "Detaching hwnd: %8.8lx for event: %lu\n", hwnd, hookid));
         txwEventHookHandle[ hookid] = TXHWND_NULL;
      }
      else                                      // hwnd not attached
      {
         rc = TX_FAILED;
      }
   }
   else
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'txwDetachEventHook'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Query specified hook-event for a window-handle
/*****************************************************************************/
TXWHANDLE txwQueryEventHook                     // RET   window handle
(
   ULONG               hookid                   // IN    TXHK identifier
)
{
   ULONG               rc = TXHWND_NULL;           // function return

   ENTER();

   if (hookid < TXHK_EVENTS)
   {
      rc = txwEventHookHandle[ hookid];
   }
   RETURN (rc);
}                                               // end 'txwQueryEventHook'
/*---------------------------------------------------------------------------*/

#if defined (HAVETHREADS)

/*****************************************************************************/
// Signal specified hook-event, pass optional data (data must stay valid!)
/*****************************************************************************/
TXWHANDLE txwSignalEventHook                    // RET   window signalled
(
   ULONG               hookid,                  // IN    TXHK identifier (MP1)
   void               *data                     // IN    optional data   (MP2)
)
{
   ULONG               rc = TXHWND_NULL;        // function return

   ENTER();

   if (hookid < TXHK_EVENTS)
   {
      if ((rc = txwEventHookHandle[ hookid]) != TXHWND_NULL)
      {
         //- to be refined, may add boolean parameter for multiple messages
         //- otherwise, don't add multiple to avoid msg pile-up in queue

         if (txwQueueMsgPresent( rc, TXWM_HOOKEVENT) == FALSE)
         {
            if (txwPostMsg( rc, TXWM_HOOKEVENT, hookid, (ULONG) data))
            {
               txwNotifyAsyncInput();           // signal to synchronious
            }                                   // input procedures ...
            else
            {
               rc = TXHWND_NULL;                // Post failed, MSGQ full ?
            }
         }
      }
   }
   RETURN (rc);
}                                               // end 'txwSignalEventHook'
/*---------------------------------------------------------------------------*/

#endif                                          // HAVETHREADS
#endif                                          // USEWINDOWING
