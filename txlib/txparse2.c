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
// TxLib command, argv/argc and number parsing, section 2
//
// 19-08-2005 JvW Initial version, split off for modularity

#include <txlib.h>                              // TxLib interface
#include <txwpriv.h>                            // TXW private interfaces
#include <txtpriv.h>                            // TXT private interfaces


/*****************************************************************************/
// Create new level of command options for a given command string
/*****************************************************************************/
ULONG TxaParseCommandString                     // RET   result
(
   char               *cmd,                     // IN    command string
   BOOL                freeform,                // IN    free format options
   TXHANDLE           *txh                      // OUT   TXA handle (optional)
)
{
   ULONG               rc;
   TXA_ELEMENT        *txa;
   char               *item;                    // parsed item
   char               *pos;                     // next start position
   int                 len = 0;                 // length of item

   ENTER();

   TRACES(( "Command: '%s'\n", cmd));

   if ((rc = TxaNewParseElement( &txa)) == NO_ERROR)
   {
      pos = cmd;
      while (((item = txaParseNextItem( &pos, ' ', &len)) != NULL) && (rc == NO_ERROR))
      {
         TRACES(( "Rest of cmd : ¯%s®\n", item));
         if (*item != ';')
         {
            rc = txaReadAndStoreItem( item, len, freeform, FALSE, txa);
         }
         else                                   // copy rest as comment
         {
            TRACES(( "add comment: '%s'\n", item +1));
            strncpy( txa->comment, item +1, TXMAXLN);
            break;                              // and break the loop
         }
      }
   }
   if (txh != NULL)
   {
      *txh = (TXHANDLE) txa;
   }
   TRINIT(30);
   {
      TxaShowParsedCommand( FALSE);             // show parse result
   }
   TREXIT();
   RETURN( rc);
}                                               // end 'TxaParseCommandString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Add one item (argument or option) to the specified TXA handle
/*****************************************************************************/
ULONG TxaSetItem                                // RET   result
(
   TXHANDLE            txh,                     // IN    TXA handle
   char               *item                     // IN    new item
)
{
   ULONG               rc = TX_INVALID_HANDLE;
   TXA_ELEMENT        *txa;

   ENTER();

   TRACES(( "Additional item: '%s'\n", item));

   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      rc = txaReadAndStoreItem( item, strlen(item), TRUE, FALSE, txa);
   }
   TRINIT(30);
   {
      TxaShowParsedCommand( FALSE);             // show parse result
   }
   TREXIT();
   RETURN( rc);
}                                               // end 'TxaSetItem'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Replace current level of command options for a new command string
/*****************************************************************************/
ULONG TxaReParseCommand                         // RET   result
(
   char               *cmd                      // IN    new command string
)
{
   ULONG               rc;

   ENTER();
   TRARGS(("ReParse new cmd: '%s%s%s'\n", CBC, cmd, CNN));

   TxaDropParsedCommand( FALSE);                // drop current
   rc = TxaParseCommandString(cmd, TRUE, NULL); // reparse, free-format
   RETURN( rc);
}                                               // end 'TxaReParseCommand'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get string value for specified option, convert to string for any type
/*****************************************************************************/
char *TxaOptionAsString                         // RET   option String value
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt,                     // IN    option character
   int                 size,                    // IN    size of buffer
   char               *str                      // OUT   string buffer, also
)                                               // default when not specified
{
   char               *rc = str;
   TXA_OPTION         *o;

   ENTER();

   if ((o = TxaGetOption( txh, opt)) != NULL)   // specified, use or convert
   {
      if (o->type == TXA_STRING)
      {
         TxCopy( str, o->value.string, size);
      }
      else                                      // other option type
      {
         sprintf( str, "%lu", o->value.number);
      }
   }
   TRACES(("Option string: '%s'\n", rc));
   RETURN( rc);
}                                               // end 'TxaOptionAsString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get verbosity value from Option 'o'
/*****************************************************************************/
ULONG TxaOutputVerbosity                        // RET   output verbosity value
(
   char                optchar                  // IN    option character
)
{
   ULONG               rc = TXAO_NORMAL;        // default normal output
   TXA_OPTION         *opt;                     // option pointer

   ENTER();

   if ((opt = TxaOptValue( optchar)) != NULL)   // specific verbosity
   {
      if (opt->type == TXA_STRING)
      {
         if      (strchr( "vVyY", opt->value.string[0]) != NULL)
         {
            rc = TXAO_VERBOSE;                  // verbose output
         }
         else if (strchr( "qQ", opt->value.string[0]) != NULL)
         {
            rc = TXAO_QUIET;                    // quiet, no output
         }
         else if (strchr( "xX", opt->value.string[0]) != NULL)
         {
            rc = TXAO_EXTREME;                  // extreme output
         }
         else if (strchr( "mM", opt->value.string[0]) != NULL)
         {
            rc = TXAO_MAXIMUM;                  // maximum output
         }
      }
      else
      {
         rc = opt->value.number;
      }
   }
   RETURN (rc);
}                                               // end 'TxaOutputVerbosity'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get errorStrategy value from Option 'E' and batch-mode indicator 'quit'
/*****************************************************************************/
int TxaErrorStrategy                            // RET   error strategy value
(
   char                optchar,                 // IN    option character
   BOOL                quit                     // IN    quit as default
)
{
   int                 rc = (quit) ? 'q' : 'c'; // function return
   TXA_OPTION         *opt;                     // option pointer

   ENTER();

   if ((opt = TxaOptValue( optchar)) != NULL)   // error handling specified
   {
      switch (opt->type)
      {
         case TXA_STRING: rc = tolower( opt->value.string[0]); break;
         default:
            if (opt->value.number != 0)
            {
               rc = 'i';                        // ignore errors on -E
            }
            break;
      }
   }
   RETURN (rc);
}                                               // end 'TxaErrorStrategy'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set radix class bits for input like ParseNumber, numeric options; 1 = HEX
/*****************************************************************************/
void TxSetNumberRadix
(
   ULONG               radix                    // IN    number radix
)
{
   TRACES(( "Default number radix set from %lu to %lu\n",
             txwa->radixclass, radix));
   txwa->radixclass = radix;
}                                               // end 'TxSetNumberRadix'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get radix class bits for input like ParseNumber, numeric options; 1 = HEX
/*****************************************************************************/
ULONG TxGetNumberRadix                          // RET   default number radix
(
   void
)
{
   return( txwa->radixclass);
}                                               // end 'TxGetNumberRadix'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse a boolean value from a string (empty string returns TRUE)
/*****************************************************************************/
BOOL TxaParseBool                               // RET   boolean representation
(
   char               *param                    // IN    pragma parameter
)
{
   BOOL                rc = TRUE;               // function return

   ENTER();

   if ((*param == '0') || (strncasecmp( param, "N",     1) == 0) ||
       (*param == '-') || (strncasecmp( param, "OFF",   3) == 0) ||
                          (strncasecmp( param, "F",     1) == 0)  )
   {
      rc = FALSE;
   }
   BRETURN (rc);
}                                               // end 'TxaParseBool'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Build space separated concatenated string of arguments (and options)
// Clips output with no warning when buffer too small!
// Supports embedded spaces in the arguments, using single/double quoting
/*****************************************************************************/
char *TxaGetArgString                           // RET   argument str or NULL
(
   TXHANDLE            txh,                     // IN    TXA handle
   int                 first,                   // IN    first argument to copy
   int                 last,                    // IN    last arg, _ALL or _OPT
   int                 bufsize,                 // IN    size of result buffer
   char               *args                     // OUT   argument string
)
{
   char               *rc = NULL;
   TXA_ELEMENT        *txa;                     // specified TXA instance
   int                 i;
   int                 limit;
   BOOL                clipped = FALSE;         // output exceeds buffer
   TXLN                str;                     // element string

   ENTER();

   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      if ((last == TXA_ALL) || (last == TXA_OPT))
      {
         limit = txa->argc;
      }
      else
      {
         limit = last;
      }
      if (first < txa->argc)                    // only when available
      {
         strcpy( args, txa->argv[first]);
         for (i = first +1; (i < limit) && !clipped; i++)
         {
            if (strchr(txa->argv[i], ' ') == NULL)
            {
               strcpy( str, txa->argv[i]);      // straight copy
            }
            else                                // embedded spaces
            {
               if (strchr(txa->argv[i], '"') == NULL)
               {
                  sprintf( str, "\"%s\"", txa->argv[i]);
               }
               else                             // use single quoting
               {
                  sprintf( str, "'%s'", txa->argv[i]);
               }
            }
            if ((strlen(args) + strlen(str) +2) < bufsize)
            {
               strcat( args, " ");              // spacing between arguments
               strcat( args, str);              // possibly quoted string
            }
            else
            {
               clipped = TRUE;
            }
         }
      }
      else if ((first == TXA_OPT) && (txa->argc != 0)) // argv[0] + options
      {
         strcpy( args, txa->argv[0]);
      }
      else
      {
         strcpy( args, "");                     // empty string
      }
      if ((last == TXA_OPT) && !clipped)        // include the options too
      {
         TXTS     unit;

         for (i = 0; (i < TXA_SIZE) && !clipped; i++)
         {
            switch (txa->opt[i].type)
            {
               case TXA_NUMBER:
                  sprintf( str, " -%c:%lu",
                           (char) (i + TXA_BASE),
                           txa->opt[i].value.number);
                  if (txa->opt[i].unit != ' ')
                  {
                     sprintf( unit, ",%c", txa->opt[i].unit);
                     strcat( str, unit);
                  }
                  break;

               case TXA_STRING:
                  if (strchr(txa->opt[i].value.string, ' ') == NULL)
                  {
                     sprintf( str, " -%c:%s",
                              (char) (i + TXA_BASE),
                              txa->opt[i].value.string);
                  }
                  else                          // embedded spaces
                  {
                     if (strchr(txa->opt[i].value.string, '"') == NULL)
                     {
                        sprintf( str, " -%c:\"%s\"",
                                 (char) (i + TXA_BASE),
                                 txa->opt[i].value.string);
                     }
                     else                       // use single quoting
                     {
                        sprintf( str, " -%c:'%s'",
                                 (char) (i + TXA_BASE),
                                 txa->opt[i].value.string);
                     }
                  }
                  break;

               case TXA_NO_VAL:
                  sprintf( str, " -%c", (char) (i + TXA_BASE));
                  break;

               default:                         // not set at all
                  strcpy( str, "");
                  break;
            }
            if ((strlen(args) + strlen(str) +1) < bufsize)
            {
               strcat( args, str);
            }
            else
            {
               clipped = TRUE;
            }
         }
      }
      rc = args;
      TRACES(("arguments+options: '%s'\n", args));
   }
   RETURN( rc);
}                                               // end 'TxaGetArgString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return comment part of command, if any
/*****************************************************************************/
char *TxaGetComment                             // RET   comment  str or NULL
(
   TXHANDLE            txh,                     // IN    TXA handle
   char               *comment                  // OUT   comment string
)
{
   char               *rc = NULL;
   TXA_ELEMENT        *txa;                     // specified TXA instance

   ENTER();

   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      if (comment && strlen(txa->comment) != 0)
      {
         strcpy( comment, txa->comment);
         rc = comment;
      }
   }
   RETURN( rc);
}                                               // end 'TxaGetComment'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test for mutual exclusive options and specified condition; handle error msg
/*****************************************************************************/
BOOL TxaMutualX
(
   TXHANDLE            txh,                     // IN    TXA handle
   BOOL                cond,                    // IN    other  exclusive cond
   char               *opts,                    // IN    mutual exclusive opts
   char               *condmsg,                 // IN    msg if cond TRUE /NULL
   char               *msgtext                  // OUT   message text     /NULL
)                                               //       (NULL ==> TxPrint)
{
   BOOL                rc;                      // function return
   int                 xo = 0;                  // exclusive option count
   TXA_ELEMENT        *txa;                     // specified TXA instance

   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      int              i;
      BYTE            *o;

      for (o = (BYTE *) opts; *o; o++)          // check each option
      {
         if ((*o >= TXA_BASE) && (*o < (TXA_BASE + TXA_SIZE)))
         {
            i = *o - TXA_BASE;                  // make it 0 based
            if (txa->opt[ i].type != TXA_NONE)
            {
               xo++;
            }
         }
      }
   }
   if (( cond && (xo == 0)) ||                  // cond but no options, or
       (!cond && (xo <= 1))  )                  // no cond and one option,
   {                                            // or nothing at all; OK
      rc = TRUE;
   }
   else                                         // invalid combination
   {
      if (condmsg)
      {
         TXLN          error;
         TXLN          options;
         TXTS          nextopt;
         int           i;

         sprintf( options, "'-%c'", opts[0]);
         for (i = 1; i < strlen(opts); i++)
         {
            sprintf( nextopt, "%s'-%c'",
                     (i < (strlen(opts) -1)) ? ", " : " or ", opts[i]);
            strcat(  options, nextopt);
         }
         if (cond)                              // option(s) AND condition
         {
            sprintf( error, "You can't use the %s%s option %s",
                     options, (i > 4)   ? "\n " : "",
                              (condmsg) ? condmsg : "now");
         }
         else                                   // non-exclusive options
         {
            sprintf( error, "You can only use one of the %s%s options "
                            "at the same time", options, (i > 4) ? "\n " : "");
         }
         if (msgtext)                           // return the error text
         {
            strcpy( msgtext, error);
         }
         else                                   // direct TxPrint output
         {
            TxPrint( "\n* %s!", error);
         }
      }
      rc = FALSE;
   }
   TRACES(( "MUTUAL cond. : %s + opts '%s' result: %s msg: '%s'\n",
          (cond)    ? "TRUE " : "FALSE", opts,
          (rc)      ? "TRUE " : "FALSE",
          (condmsg) ? condmsg : "none"));
   return (rc);
}                                               // end 'TxaMutualX'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Show contents of one or all instances of the TXA abstract-data-type
/*****************************************************************************/
void TxaShowParsedCommand                       // RET   more instances left
(
   BOOL                whole_stack              // IN    show all levels
)
{
   TXA_ELEMENT        *txa = txacur;
   ULONG               level = 0;
   int                 i;


   ENTER();

   do
   {
      if (txa != NULL)
      {
         TxPrint( "\nTXA level: %lu  nr of options: %d  nr of arguments: %d\n",
                         level, txa->optc, txa->argc);
         if (strlen( txa->comment) != 0)
         {
            TxPrint(" Comment : '%s'\n", txa->comment);
         }
         for (i = 0; i < TXA_SIZE; i++)
         {
            switch (txa->opt[i].type)
            {
               case TXA_NUMBER:
                  TxPrint( " opt. -%c : % -10lu = %8.8lx  Unit: '%c'\n",
                           (char) (i + TXA_BASE),
                           txa->opt[i].value.number,
                           txa->opt[i].value.number,
                           txa->opt[i].unit);
                  break;

               case TXA_STRING:
                  TxPrint( " opt. -%c : '%s'\n",
                           (char) (i + TXA_BASE),
                           txa->opt[i].value.string);
                  break;

               case TXA_NO_VAL:
                  TxPrint( " opt. -%c : no value\n",
                           (char) (i + TXA_BASE));
                  break;

               default:
                  break;
            }
         }
         for (i = 0; i < txa->argc; i++)
         {
            TxPrint( " argv[% 2hu]: '%s'\n", i, txa->argv[i]);
         }
         TxPrint( " argc    : % 2hu\n", txa->argc);
         txa = txa->prev;                       // to prev level
         level++;
      }
   } while (txa && whole_stack);
   TxPrint("\n");

   VRETURN();
}                                               // end 'TxaShowParsedCommand'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse next item in string, skip leading terminators, honor embedded strings
/*****************************************************************************/
char *txaParseNextItem                          // RET   next item or NULL
(
   char              **start,                   // INOUT start position
   char                term,                    // IN    terminating character
   int                *length                   // OUT   length of item
)
{
   char               *rc = NULL;               // function return
   char               *s;
   int                 l;
   char                delim  = term;           // default delimitter
   BOOL                escape = FALSE;

   ENTER();


   if (*start != NULL)
   {
      TRACES(("term:'%c'  start:% -55.55s\n", term, *start));
      for (rc = *start; *rc == term; rc++)      // skip leading term chars
      {
      }
      if ((*rc) && (*rc != ';'))                // if not at end of command
      {
         for (s = rc, l = 0; *s; s++, l++)
         {
//          TRACES(( "*s:'%c', esc:%lu, delim:'%c'\n", *s, escape, delim));
            if (*s == delim)
            {
               if (delim == term)               // end of item reached
               {
                  break;
               }
               else if (!escape)                // end-of string ...
               {
                  delim = term;                 // continue
               }
            }
            else if (((*s == '"')  || (*s == '\'')) &&
                     ((!escape) && (delim == ' ')))
            {
               delim = *s;                      // to end of string
            }
            else if ( (*s     == '\\') &&       // possible escape
                     ((*(s+1) == ';')  ||       // for comment
                      (*(s+1) == '"')  ||       // for double quote
                      (*(s+1) == '\'') ))       // or single quote
            {
               escape = TRUE;                   // escape next char
               continue;
            }
            else if ((*s == ';') && (!escape) && (delim == term))
            {
               break;
            }
            escape = FALSE;
         }
         *start  = s;                           // next parse start
         *length = l;                           // length current item
      }
      else if (*rc != ';')
      {
         rc = NULL;                             // end of string reached
      }
   }
   RETURN (rc);
}                                               // end 'txaParseNextItem'
/*---------------------------------------------------------------------------*/

