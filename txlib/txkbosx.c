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
// Kbhit and Getch implementation for MAC OS X (DARWIN)
// Non-blocking keyboard handling like PC conio using terminal manipulation
//
// Author: J. van Wijk
//
// JvW  31-05-2007 Initial version, derived from txkbhit.c and OW Linux clib

#include <txlib.h>

#if defined (DARWIN)

#include <sys/time.h>
#include <termios.h>


/*****************************************************************************/
// Implement CONIO style keyboard-hit test
/*****************************************************************************/
int kbhit (void)
{
   int                 hit = 0;
   fd_set              fileset;
   struct timeval      tv = { 0, 0 };
   struct termios      old, new;

   tcgetattr( STDIN_FILENO, &old );
   new = old;
   new.c_iflag    &= ~(IXOFF | IXON);
   new.c_lflag    &= ~(ECHO | ICANON | NOFLSH);
   new.c_lflag    |= ISIG;
   new.c_cc[VMIN]  = 1;
   new.c_cc[VTIME] = 0;
   tcsetattr( STDIN_FILENO, TCSADRAIN, &new );

   FD_ZERO(&fileset);
   FD_SET(STDIN_FILENO, &fileset);
   hit = (select(STDIN_FILENO +1, &fileset, NULL, NULL, &tv) > 0);

   tcsetattr( STDIN_FILENO, TCSADRAIN, &old );
   return( hit );
}                                               // end 'kbhit'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Implement CONIO style keyboard read character, no echo, non-blocking
/*****************************************************************************/
int getch (void)
{
   unsigned int        c = 0;
   char                readbuffer[1];
   struct termios      old, new;

   tcgetattr( STDIN_FILENO, &old );
   new = old;
   new.c_iflag    &= ~(IXOFF | IXON);
   new.c_lflag    &= ~(ECHO | ICANON | NOFLSH);
   new.c_lflag    |= ISIG;
   new.c_cc[VMIN]  = 1;
   new.c_cc[VTIME] = 0;
   tcsetattr( STDIN_FILENO, TCSADRAIN, &new );

   read( STDIN_FILENO, &readbuffer, 1 );
   c = readbuffer[0];

   tcsetattr( STDIN_FILENO, TCSADRAIN, &old );
   return( c);
}                                               // end 'kbhit'
/*---------------------------------------------------------------------------*/

#endif
