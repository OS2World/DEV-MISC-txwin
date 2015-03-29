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
// TxLib generic wildcard handling
//

#include <txlib.h>                              // TxLib interface


/*****************************************************************************/
// Function : count number of non-wildcard chars in string
/*****************************************************************************/
int TxStrWcnt
(
    char              *s
)
{
    int                result = 0;

    if (!s)
        return 0;

    do                                          // start of DO loop
    {
        if ((*s != '*') && (*s != '?'))
        {
            result ++;
        }
    } while ( *( ++s) );                        /* until '\0' */

    return result;
}                                               /* end TxStrWcnt             */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
/* Function : wildcard compare of candidate to template string               */
/*            check if template is a valid description of the candidate      */
/*            wildcard characters are : '*' : matches zero or more of any    */
/*                                            character in the candidate     */
/*                                      '?' : matches exactly one of any     */
/*                                            character in the candidate     */
/*                                            except the '*' character       */
/*            return value : negative if template is no valid description    */
/*                                                                           */
/* Note that the routine is case sensitive on candidate and template         */
/* Note that trailing spaces are considered as significant                   */
/*****************************************************************************/
int TxStrWcmp
(
    char              *cs,                      // IN    candidate string
    char              *ts                       // IN    template string
)
{
    int                dc = 0;                  // declare result variable
    TXLN               sub;                     // substring buffer
    int                l;                       // length of sub string

    while ((*ts != '\0') && (dc == 0))
    {
        switch (*ts)
        {
            case '*':
                do                              // skip adjacent wildcard
                {                               // characters in template
                    if (*ts++ == '?')           // need one candidate char
                    {
                        if (*cs++ == '\0')      // end of candidate
                        {
                            dc--;               // difference found !
                        }
                    }                           // end if single wildcard
                } while ((*ts == '*') ||
                         (*ts == '?') );        // until non-wildcard

                if (dc == 0)
                {
                    if (*ts == '\0')
                    {                           // signal '*' at end of
                        dc++;                   // template
                    }
                    else
                    {
                        l = strcspn(ts, "*?");  // next wildcard position
                        strncpy( sub, ts, l);
                        sub[ l] = '\0';         // terminate it

                        if ((cs = strstr(cs, sub)) != NULL)
                        {
                            ts += l;            // skip common substring
                            cs += l;
                        }
                        else
                        {
                            dc--;               // difference found !
                        }                       // end if
                    }                           // end if
                }                               // end if no difference yet
                break;

            case '?':
                if (*cs++ != '*')
                {
                    ts++;
                }
                else
                {
                    dc--;                       // difference found !
                }
                break;

            default:
                if (*cs++ != *ts++)
                {
                    dc--;                       // difference found !
                }
                break;
        }                                       // end switch  *template
    }                                           // end while in template
    switch (dc)
    {
        case 0:
            if ((*ts == '\0') && (cs != NULL) && (*cs != '\0'))
            {
                dc--;                           // candidate too long
            }
            break;

        case 1:
            dc--;                               // template ends in '*'
            break;                              // result is no-difference

        default:
            break;
    }                                           // end switch
    return dc;
}                                               /* end TxStrWcmp             */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Function : See FlStrwcmp; case-insensitive
/*****************************************************************************/
int TxStrWicmp
(
    char              *cs,                      // IN    candidate string
    char              *ts                       // IN    template string
)
{
    TX1K               candi;                   // candidate string
    TXLN               templ;                   // template string

    TxStrToUpper( strcpy( candi, cs));
    TxStrToUpper( strcpy( templ, ts));
    return( TxStrWcmp( candi, templ));
}                                               /* end TxStrWicmp            */
/*---------------------------------------------------------------------------*/

