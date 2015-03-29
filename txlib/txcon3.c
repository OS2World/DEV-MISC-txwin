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
// Generic value prompt implementation text/windowed
//
// Author: J. van Wijk
//
// JvW  19-08-2005 Initial version, split off from TXCON.C

#include <txlib.h>
#include <txwpriv.h>                            // private window interface


#if defined (USEWINDOWING)
static  char txMsgPromTitle[] = " Request for additional input ";
#endif

/*****************************************************************************/
// Prompt for user input with a message and read the input
/*****************************************************************************/
BOOL TxPrompt                                   // RET   value not empty
(
   ULONG               helpid,                  // IN    helpid confirmation
   short               length,                  // IN    max length of value
   char               *value,                   // OUT   entry field value
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   char                text[512];
   va_list             varargs;
   char              **mText;                   // formatted text for display
   int                 ll;                      // real max line length
   int                 lines;                   // nr of lines
   DEVICE_STATE        sa = TxScreenState(DEVICE_TEST);

   if ((ll = TxScreenCols() -12) < 20)          // ScreenCols will be 0 if
   {                                            // output is redirected!
      ll = 20;
   }
   if ((txwa->desktop != NULL) && (!txwa->pedantic))
   {
      TxScreenState(DEVICE_OFF);                // no regular text if windowed
   }                                            // and not 'pedantic mode'
   else
   {
      TxScreenState(DEVICE_ON);                 // force screen on for input
   }
   va_start(varargs, fmt);
   vsprintf( text, fmt, varargs);               // format the message

   mText = txString2Text( text, &ll, &lines);   // convert to multi-line
   TxPrint( "\n");                              // force empty line before
   TxShowTxt(  mText);                          // display in scrollbuffer
   txFreeText( mText);                          // free the text memory
   TxPrint("\n");

   #if defined (USEWINDOWING)
   if (txwa->desktop != NULL)                   // there is a desktop
   {
      TxStripAnsiCodes( text);                  // remove colors for popup
      if (txwPromptBox( TXHWND_DESKTOP, TXHWND_DESKTOP, NULL,
                        text, txMsgPromTitle, helpid,
                        TXPB_MOVEABLE | TXPB_HCENTER,
                        length, value)
                        == TXDID_CANCEL)
      {
         strcpy( value, "");                    // wipe value, signal cancel
         TxCancelAbort();                       // avoid aborting on Escape ...
      }
   }
   else                                         // use non-windowed interface
   #endif                                       // USEWINDOWING
   {
      TXLN          defval;

      memset(  defval, 0, TXMAXLN);
      strncpy( defval, value, length);
      TxPrint( "\nDefault on <Enter>: '%s'\n", defval);

      TxPrint(   "or different value: ");
      memset(  value, 0, length);
      fgets(   value, length, stdin);           // get the value
      TxRepl(  value, '\n', 0);
      TxPrint("\n");
      if (strlen( value) == 0)                  // empty ==> use default
      {
         strcpy( value, defval);
      }
   }
   TxScreenState(sa);                           // restore screen state

   return(strlen(value) != 0);
}                                               // end 'TxPrompt'
/*---------------------------------------------------------------------------*/

