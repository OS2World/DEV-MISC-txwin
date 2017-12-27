#ifndef TXTPRIV_H
#define TXTPRIV_H
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
// TX text library private interfaces
//
// Author: J. van Wijk
//
// Developed for LPTool/DFSee utilities
//
// JvW  19-08-2005   Initial version, cleanup public interfaces


//============ TXCON
                                                // Hook control structures
extern  TXH_INFO   *txh_clean;                  // clean TxPrint handler
extern  TXH_INFO   *txh_raw;                    // raw ansi TxPrint handler

//============ TXPARSE

typedef struct txa_element                      // command parser element
{
   ULONG               signature;               // signature value
   int                 optc;                    // number of options
   TXA_OPTION          opt[TXA_SIZE];           // array of options
   int                 argc;                    // number of arguments
   char               *argv[TXA_ARGC +1];       // array of argv pointers
   TXLN                comment;                 // stripped comment
   struct txa_element *prev;                    // prev element on stack
} TXA_ELEMENT;                                  // end of struct "txa_element"

extern TXA_ELEMENT  *txacur;                    // current, top of stack
extern TXA_ELEMENT  *txa1st;                    // first, bottom of stack


// Take handle value and convert to element, validate signature
TXA_ELEMENT *txaHandle2Element                  // RET   element ptr or NULL
(
   TXHANDLE            txh                      // IN    TXA handle
);

// Create new TXA element, and attach it to the TXA stack
ULONG TxaNewParseElement                        // RET   result
(
   TXA_ELEMENT       **element                  // OUT   TXA element
);


// Read one item (option or argument) and store it in the txa element
ULONG txaReadAndStoreItem                       // RET   result
(
   char               *item,                    // IN    item string
   int                 len,                     // IN    length of item
   BOOL                freeform,                // IN    free format options
   BOOL                passthrough,             // IN    keep parameter quotes
   TXA_ELEMENT        *txa                      // INOUT TXA element
);


// Copy item in string, honor embedded strings
char *txaCopyItem                               // RET   copied string
(
   char               *destin,                  // OUT   destination buffer
   char               *start,                   // IN    start position
   int                 length                   // IN    max length to copy
);


//============ TXTRACE

#define TXTHREADS                 0x10          // Shown with unique thread
#define TXTHREADID (TXCURTHREAD & 0x0f)         // indentation in trace
extern  ULONG       TxTrIndent[];               // trace indent per thread
extern  BOOL        TxTrTstamp;                 // timestamp in trace ?


//============ TXCRITE
extern ULONG        TxAutoFail;                 // AutoFail active flag
                                                // call to succeeed

#endif
