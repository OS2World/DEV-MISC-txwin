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
// TxLib command, argv/argc and number parsing
//
// 04-07-2007 JvW Interpret -x123 as longname, not -x:123   (DFSee 9.00)
// 21-11-2004 JvW Allow explicit decimal prefix 0n and 0t   (DFSee 6.16)
// 08-08-2003 JvW Long-option support and allow '/' switch  (DFSee 5.25)
// 15-12-2001 JvW No string-value for -tValue type syntax   (DFSee 5.08)
// 04-12-2001 JvW Make numeric 0 the default for option -o  (DFSee 4.11)
// 17-11-2001 JvW Added '' as alternative string syntax     (DFSee 4.10)
// 05-11-2001 JvW Added -tValue and -t- type of syntax      (DFSee 4.09)
// 27-08-2001 JvW Added argv/argc as input alternative      (DFSee 4.03)
// 13-08-2001 JvW First released version                    (DFSee 4.02)

#include <txlib.h>                              // TxLib interface
#include <txwpriv.h>                            // TXW private interfaces
#include <txtpriv.h>                            // TXT private interfaces

#define TXA_SIGNATURE   0xFACEBEEF              // valid magic signature value

       TXA_ELEMENT  *txacur = NULL;             // current, top of stack
       TXA_ELEMENT  *txa1st = NULL;             // first, bottom of stack

typedef char TXA_LONGOPT[ TXA_O_LEN +1];

static TXA_LONGOPT   txalongopt[TXA_LSIZE] =
{
   "query",                                     // predefined long names
   "menu",
   "debug",
   "test",
   "trace",
   "auto",
   "entry",
   "color",
   "scheme",
   "lines",
   "ini",
   "config",
   "style",
   "keyb",
   "mouse",
   "simulate",
   "list",
   "screen",
   "label",
   "xml",
   "url",
   "dir",
   "file",
   "disk",
   "",                                          // TXA_O_APP0
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
   ""
   ""
   ""
   ""
   ""
   ""                                           // TXA_O_APPF
};



/*****************************************************************************/
// Set or query value for a specific LONGNAME value (the LONG option name)
/*****************************************************************************/
char  *TxaOptionLongName                        // RET   resulting option name
(
   char                opt,                     // IN    TXA_O_ option value
   char               *name                     // IN    long name for option
)                                               //       or NULL to query
{
   char               *rc = NULL;               // function return
   int                 lni;

   ENTER();

   if ((opt >= TXA_LBASE) && ((lni = (opt - TXA_LBASE)) < TXA_LSIZE))
   {
      if (name != NULL)
      {
         TxCopy( txalongopt[lni], name, TXA_O_LEN +1);
      }
      rc = &(txalongopt[lni][0]);
      TRACES(("Name for APP%lu option set to '%s'\n", opt - TXA_O_APP0, rc));
   }
   RETURN (rc);
}                                               // end 'TxaOptionLongName'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create new level of command options for a set of argc/argv variables
/*****************************************************************************/
ULONG TxaParseArgcArgv                          // RET   result
(
   int                 argc,                    // IN    argument count
   char               *argv[],                  // IN    array of arguments
   char               *exename,                 // IN    alternative argv[0]
   BOOL                freeform,                // IN    free format options
   TXHANDLE           *txh                      // OUT   TXA handle (optional)
)
{
   ULONG               rc;
   TXA_ELEMENT        *txa;
   char               *item;                    // parsed item
   int                 len;                     // length of item
   int                 i;

   ENTER();

   if ((rc = TxaNewParseElement( &txa)) == NO_ERROR)
   {
      for (i = 0; (i < argc) && (rc == NO_ERROR); i++)
      {
         if ((i == 0) && (exename != NULL))
         {
            item = exename;                     // preserve alternative argv[0]
         }
         else
         {
            item = argv[i];
         }
         len  = strlen( item);

         rc = txaReadAndStoreItem( item, len, freeform, TRUE, txa);
      }
   }
   if (txh != NULL)
   {
      *txh = (TXHANDLE) txa;
   }
   RETURN( rc);
}                                               // end 'TxaParseArgcArgv'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse a decimal or hex number value from a string
/*****************************************************************************/
ULONG TxaParseNumber                            // RET   number value
(
   char               *value,                   // IN    value string with nr
   ULONG               rclass,                  // IN    HEX/DEC radix class
   BYTE               *unit                     // OUT   optional unit char, if
)                                               //       not needed use NULL
{
   ULONG               rc = 0;                  // function return
   BOOL                prefix     = TRUE;       // number parsed using prefix
   int                 distance   = 0;          // distance to specific char
   int                 radixclass = rclass;     // class to be used for default

   TRLEVX(700,( "value: '%s'  rclass: 0x%lX\n", value, rclass));

   if (unit != NULL)                            // read unit char suffix
   {
      if ((( distance = (int) strcspn( value, ", ")) > 0) &&
          ( value[ distance] == ','))           // unit specifier present
      {
         *unit = value[ distance +1];           // unit character
         if (strchr( "sS", *unit) != NULL)      // 's' unit
         {
            radixclass = TX_RADIX_UN_S_CLASS;   // use sector-unit class
         }
         else
         {
            radixclass = TX_RADIX_UNIT_CLASS;   // use 'other' unit class
         }
      }
      else
      {
         *unit = TXA_DFUNIT;                    // assume default unit
      }
      TRLEVX(700,( "Unit for value: '%s' is: '%c'\n", value, *unit));
   }
   if (*value == '0')                           // possible hex/oct/dec prefix
   {
      switch (tolower(*(value +1)))
      {
         case 'x': sscanf( value +2, "%lx", &rc); break; // hexadecimal number
         case 'o': sscanf( value +2, "%lo", &rc); break; // octal number base
         case 'n':
         case 't': sscanf( value +2, "%lu", &rc); break; // decimal number base
         default:  prefix = FALSE;                break; // to be determined
      }
   }
   else
   {
      prefix = FALSE;
   }
   if (prefix == FALSE)                         // no prefix, test HEX or DEC
   {
      TRLEVX(700,( "value has no prefix ...\n"));
      if ( txIsValidHex( value) &&              // valid HEX upto separator
          !txIsValidDec( value) )               // and has non-decimal digits
      {
         TRLEVX(700,( "auto-interpret as HEX!\n"));
         sscanf( value, "%lx", &rc);            // interpret as HEX
      }
      else                                      // could be both ...
      {
         if (txwa->radixclass & radixclass)     // class set to HEX default
         {
            TRLEVX(700,( "Radix-selection HEX!\n"));
            sscanf( value, "%lx", &rc);         // interpret as HEX
         }
         else
         {
            TRLEVX(700,( "Radix-selection DEC!\n"));
            sscanf( value, "%lu", &rc);         // interpret as DECimal
         }
      }
   }
   else
   {
      TRLEVX(700,( "Selection by prefix '%2.2s'\n", value));
   }
   return (rc);
}                                               // end 'TxaParseNumber'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Read one item (option or argument) and store it in the txa element
/*****************************************************************************/
ULONG txaReadAndStoreItem                       // RET   result
(
   char               *item,                    // IN    item string
   int                 len,                     // IN    length of item
   BOOL                freeform,                // IN    free format options
   BOOL                passthrough,             // IN    keep parameter quotes
   TXA_ELEMENT        *txa                      // INOUT TXA element
)
{
   ULONG               rc = NO_ERROR;
   int                 arguments = txa->argc;   // current number of arguments

   ENTER();

   TRACES(( "Freeform: %s Passthrough: %s Args: %d  Len:%3.3u, Item: '%*.*s'\n",
            (freeform) ? "YES" : "NO ", (passthrough) ? "YES" : "NO ",
             arguments, len, len, len, item));
   if   ((((txa      == txa1st)  &&             // switch on 1st level '/' only
         (*(item)    == '/'))    ||
         (*(item)    == '-'  ))  &&             // switch or option using  '-'
         (*(item +1) != ' '  )   &&             // allowed at this position ?
         (*(item +1) != '\0' )   &&
         (arguments  > 0     )   &&             // never BEFORE 1st word (cmd)
        ((arguments == 1) || freeform))         // possibly after params too
   {
      int              index;                   // index for the option char
      char            *value;                   // ptr to value part of item
      int              distance;                // distance to specific char
      TXA_LONGOPT      lname = {0};             // empty longname to start with

      index = *(item +1);
      if ((index >= TXA_BASE) && (index < (TXA_BASE + TXA_SIZE)))
      {
         index -= TXA_BASE;                     // make it 0 based
         TRACES(( "Direct option index:%3hu\n", index));

         switch (*(item +2))
         {
            case '"': case '\'':                // quoted string
            case '+': case '-':                 // boolean yes/no
               value = item +2;
               break;

            default:
               if ((distance = (int) strcspn( item, ": ")) < len)
               {
                  value = item + distance +1;   // skip the colon ...
               }
               else
               {
                  if ((len > 2) &&              // non-empty option name
                      (item[len -1] == '-'))    // ending in a '-'
                  {
                     distance = len -1;         // length of the name
                     value = item + distance;   // '-' is the value
                  }
                  else
                  {
                     value = item + len;        // no explicit value given
                  }
               }

               if (distance > 2)                // non-trivial longname
               {
                  int  lni;

                  strncpy( lname, item +1, distance -1);
                  lname[distance -1] = 0;       // terminate the name
                  TRACES(( "longname '%s' for: '%s'\n", lname, item));

                  if (strcasecmp( "help", lname) == 0) // predefined, map to '?'
                  {
                     index = '?' - TXA_BASE;
                  }
                  else
                  {
                     for (lni = 0; lni < TXA_LSIZE; lni++)
                     {
                        if (strlen(txalongopt[lni]) != 0)
                        {
                           if (strcasecmp( txalongopt[lni], lname) == 0)
                           {
                              index = lni + TXA_LBASE - TXA_BASE;

                              TRACES(("Matched defined longname '%s', index: %hu\n",
                                       txalongopt[lni], index));
                           }
                        }
                     }
                  }
               }
               break;
         }
         TxFreeMem(  txa->opt[index].name);     // free existing name, if any
         if (strlen( lname) != 0)               // new longname available ?
         {
            if ((txa->opt[ index].name = TxAlloc(1, strlen(lname) +1)) != NULL)
            {
               strcpy( txa->opt[ index].name, lname);
            }
         }

         if (txa->opt[ index].type == TXA_STRING) // already has a string!
         {
            TRACES(("Free existing string value: '%s'\n",
                       txa->opt[ index].value.string));
            TxFreeMem( txa->opt[ index].value.string);
         }
         txa->opt[ index].type = TXA_NUMBER;
         txa->opt[ index].unit = TXA_DFUNIT;    // default unit
         switch (*value)
         {
            case TXk_SPACE: case '\0':
               txa->opt[ index].type = TXA_NO_VAL;
               txa->opt[ index].value.number = 0;
               TRACES(("NoValueOpt (%3hu='%c')\n", index + TXA_BASE, index + TXA_BASE));
               break;

            case '+': txa->opt[ index].value.number = 1;      break;
            case '-': txa->opt[ index].value.number = 0;      break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
               txa->opt[ index].value.number =
                  TxaParseNumber( value, TX_RADIX_STD_CLASS,
                                  &(txa->opt[ index].unit));

               TRACES(("NumericOpt (%3hu='%c'), value: %lu\n",
                        index + TXA_BASE, index + TXA_BASE,
                        txa->opt[ index].value.number));
               break;

            default:
               txa->opt[ index].type = TXA_STRING;
               if ((txa->opt[ index].value.string = TxAlloc(1, len +1)) != NULL)
               {
                  if (passthrough)              // straight copy
                  {
                     strcpy(      txa->opt[ index].value.string, value);
                  }
                  else                          // space delimited stuff!
                  {
                     txaCopyItem( txa->opt[ index].value.string, value, len);
                  }

                  TRACES(("String Opt (%3hu='%c') at %8.8lx, value: '%s'\n",
                               index + TXA_BASE, index + TXA_BASE,
                               txa->opt[ index].value.string,
                               txa->opt[ index].value.string));
               }
               else
               {
                  rc = TX_ALLOC_ERROR;
               }
               break;
         }
         txa->optc++;                           // count the option
      }
      else                                      // option char out of range
      {
         rc = TX_BAD_OPTION_CHAR;
      }
   }
   else                                         // argv 0 .. n
   {
      if (arguments < TXA_ARGC)                 // room for another ?
      {
         if ((txa->argv[ arguments] = TxAlloc(1, len +1)) != NULL)
         {
            if (passthrough)                    // straight copy
            {
               strcpy(      txa->argv[ arguments], item);
            }
            else                                // space delimited stuff!
            {
               txaCopyItem( txa->argv[ arguments], item, len);
            }
            TRACES(("Stored argv[%d] at %8.8lx, value: '%s'\n", arguments,
                         txa->argv[ arguments], txa->argv[ arguments]));
            arguments++;
         }
         else
         {
            rc = TX_ALLOC_ERROR;
         }
      }
      else
      {
         rc = TX_TOO_MANY_ARGS;
      }
   }
   txa->argc = arguments;
   RETURN( rc);
}                                               // end 'txaReadAndStoreItem'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create new level of command options for a set of argc/argv variables
/*****************************************************************************/
ULONG TxaNewParseElement                        // RET   result
(
   TXA_ELEMENT       **element                  // OUT   TXA element
)
{
   ULONG               rc  = NO_ERROR;
   TXA_ELEMENT        *txa = NULL;

   if ((txa = TxAlloc(1, sizeof(TXA_ELEMENT))) != NULL)
   {
      txa->signature = TXA_SIGNATURE;
      txa->prev      = txacur;

      if (txacur == NULL)                       // first one ?
      {
         txa1st   = txa;
      }
      txacur      = txa;
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   *element = txa;
   return( rc);
}                                               // end 'TxaNewParseElement'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate use of current or all instances of the TXA abstract-data-type
/*****************************************************************************/
BOOL TxaDropParsedCommand                       // RET   more instances left
(
   BOOL                whole_stack              // IN    drop all, terminate
)
{
   TXA_ELEMENT        *txa;
   int                 i;

   ENTER();

   do
   {
      if ((txa = txacur) != NULL)
      {
         for (i = 0; i < TXA_SIZE; i++)         // free allocated options
         {
            if (txa->opt[i].type == TXA_STRING)
            {
               TxFreeMem( txa->opt[i].value.string);
            }
            TxFreeMem( txa->opt[i].name);
         }
         for (i = 0; i < txa->argc; i++)        // free argv storage
         {
            if (txa->argv[i] != NULL)
            {
               TxFreeMem( txa->argv[i]);
            }
         }
         txacur = txa->prev;
         if (txacur == NULL)                    // empty again ?
         {
            txa1st   = NULL;
         }
         TxFreeMem( txa);
      }
   } while (txacur && whole_stack);

   BRETURN((txacur != NULL));
}                                               // end 'TxaDropParsedCommand'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get reference to specified option data, or NULL if option not set
/*****************************************************************************/
TXA_OPTION *TxaGetOption                        // RET   option ptr or NULL
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt                      // IN    option character
)
{
   TXA_OPTION         *rc = NULL;
   TXA_ELEMENT        *txa;                     // specified TXA instance
   int                 i;
   TXA_OPTION         *o;

   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      if (((BYTE) opt >= TXA_BASE) && ((BYTE) opt < (TXA_BASE + TXA_SIZE)))
      {
         i = opt - TXA_BASE;                    // make it 0 based
         o = &txa->opt[ i];
         if (o->type != TXA_NONE)
         {
            rc  = o;
         }
         TRARGS(("TXH (%s): %lx option: %3hu = '%c'  value: %lu '%s'\n",
            (txh == TXA_1ST) ? "EXE switch" : "CMD option", txh, opt, opt,
             (o->type == TXA_STRING) ? strlen(o->value.string) : o->value.number,
             (o->type == TXA_NONE  ) ?       "---NONE---"      :
             (o->type != TXA_STRING) ?       "--NUMBER--"      : o->value.string ));
      }
   }
   return( rc);
}                                               // end 'TxaGetOption'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get simple YES/NO status for specified option. Not set is "NO"
/*****************************************************************************/
BOOL TxaOptionYes                               // RET   option set to YES
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt                      // IN    option character
)
{
   BOOL                rc = FALSE;
   TXA_OPTION         *o;

   if ((o = TxaGetOption( txh, opt)) != NULL)
   {
      switch (o->type)
      {
         case TXA_NUMBER:
            if (o->value.number != 0)
            {
               rc = TRUE;
            }
            break;

         case TXA_STRING:
            if ((strcasecmp( o->value.string, "NO")    != 0) &&
                (strcasecmp( o->value.string, "N")     != 0) &&
                (strcasecmp( o->value.string, "0")     != 0) &&
                (strcasecmp( o->value.string, "OFF")   != 0) &&
                (strcasecmp( o->value.string, "FALSE") != 0)  )
            {
               rc = TRUE;
            }
            break;

         default:                               // no value, but set
            rc = TRUE;
            break;
      }
   }
   return( rc);
}                                               // end 'TxaOptionYes'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get string value for specified option, use default if no string (and warn!)
/*****************************************************************************/
char *TxaOptionStr                              // RET   option String value
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt,                     // IN    option character
   char               *error,                   // IN    error text or NULL
   char               *deflt                    // IN    default value
)
{
   char               *rc = deflt;
   TXA_OPTION         *o;

   if ((o = TxaGetOption( txh, opt)) != NULL)
   {
      if (o->type == TXA_STRING)
      {
         rc = o->value.string;
      }
      else                                      // other option type
      {
         if (error != NULL)                     // give error/warning
         {
            TxPrint( "\n%s %s: %s'-%c'%s has no STRING value, using "
                     "default: '%s'\n",
                     (txh != TXA_1ST) ? "Option" : "Switch",
                      error, CBC, opt, CNN, deflt);
         }
      }
   }
   return( rc);
}                                               // end 'TxaOptionStr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get number value for specified option, use default if no number (and warn!)
/*****************************************************************************/
ULONG TxaOptionNum                              // RET   option Number value
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                opt,                     // IN    option character
   char               *error,                   // IN    error text or NULL
   ULONG               deflt                    // IN    default value
)
{
   ULONG               rc = deflt;
   TXA_OPTION         *o;

   if ((o = TxaGetOption( txh, opt)) != NULL)
   {
      if (o->type == TXA_NUMBER)
      {
         rc = o->value.number;
      }
      else                                      // other option type
      {
         if (error != NULL)                     // give error/warning
         {
            TxPrint( "\n%s %s: %s'-%c'%s has no NUMBER value, using "
                     "default: 0x%8.8lx = %lu\n",
                     (txh != TXA_1ST) ? "Option" : "Switch",
                      error, CBC, opt, CNN, deflt, deflt);
         }
      }
   }
   return( rc);
}                                               // end 'TxaOptionNum'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get option value num/string, with bytes/kilo/mega/giga modifier and default
/*****************************************************************************/
ULONG TxaOptionBkmg                             // RET   number value in bytes
(
   TXHANDLE            txh,                     // IN    TXA handle
   char                option,                  // IN    option character
   ULONG               def,                     // IN    default value
   BYTE                mod                      // IN    b,kb,mb,gb modifier
)
{
   ULONG               rc = 0;                  // function return
   BYTE                unit = TXA_DFUNIT;
   TXA_OPTION         *opt;                     // option pointer

   ENTER();
   TRACES(("Option: '%c' default: %8.8lx mod:%2.2hx = '%c'\n",
            option, def, mod, mod));

   if ((opt = TxaOptValue( option)) != NULL)    // get the option details
   {
      switch (opt->type)
      {
         case TXA_STRING:
            rc   = TxaParseNumber( opt->value.string, TX_RADIX_STD_CLASS, &unit);
         case TXA_NO_VAL:
            rc   = def;
            break;

         default:                               // convert, default is MiB!
            rc   = opt->value.number;
            unit = opt->unit;
            break;
      }
   }
   if (unit == TXA_DFUNIT)
   {
      unit = mod;
   }
   switch (tolower(unit))
   {
      case 'g':
      if (rc >= 4)
      {
         rc = 0xffffffff;                       // limit at 4GiB -1
         break;
      }
      else
      {
         rc *= 1024;                            // Giga
      }
      case 'm': rc *= 1024;                     // Mega
      case 'k': rc *= 1024;                     // Kilo
      default:  break;                          // Bytes, OK
   }
   RETURN (rc);
}                                               // end 'TxaOptionBkmg'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get reference to specified argument-string
/*****************************************************************************/
char *TxaGetArgv                                // RET   argv string or ""
(
   TXHANDLE            txh,                     // IN    TXA handle
   int                 argn                     // IN    argument number 0..n
)
{
   char               *rc = "";                 // return empty if not there
   TXA_ELEMENT        *txa;                     // specified TXA instance

   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      if ((argn >= 0) && (argn < txa->argc))    // valid argument nr ?
      {
         rc = txa->argv[argn];
         TRACES(("GETARGV argument: %d = '%s'\n", argn, rc));
      }
   }
   return( rc);
}                                               // end 'TxaGetArgv'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Get the number of arguments for specified TXA instance
/*****************************************************************************/
int TxaGetArgc                                  // RET   argument count
(
   TXHANDLE            txh                      // IN    TXA handle
)
{
   int                 rc = 0;
   TXA_ELEMENT        *txa;                     // specified TXA instance

   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      rc = txa->argc;
   }
   return( rc);
}                                               // end 'TxaGetArgc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get the number of options for specified TXA instance
/*****************************************************************************/
int TxaGetOptc                                  // RET   option count
(
   TXHANDLE            txh                      // IN    TXA handle
)
{
   int                 rc = 0;
   TXA_ELEMENT        *txa;                     // specified TXA instance

   ENTER();
   if ((txa = txaHandle2Element( txh)) != NULL)
   {
      rc = txa->optc;
   }
   RETURN( rc);
}                                               // end 'TxaGetOptc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Take handle value and convert to element, validate signature
/*****************************************************************************/
TXA_ELEMENT *txaHandle2Element                  // RET   element ptr or NULL
(
   TXHANDLE            txh                      // IN    TXA handle
)
{
   TXA_ELEMENT        *txa;

   switch (txh)
   {
      case TXA_CUR:    txa =                 txacur;                break;
      case TXA_PREV:   txa = (txacur) ?      txacur->prev : NULL;   break;
      case TXA_1ST:    txa =                 txa1st;                break;
      default:         txa = (TXA_ELEMENT *) txh;                   break;
   }
   if ((txa) && (txa->signature != TXA_SIGNATURE))
   {
      txa = NULL;
   }
   return( txa);
}                                               // end 'txaHandle2Element'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Copy item from string, honor embedded strings, strip quotes and escapes
/*****************************************************************************/
char *txaCopyItem                               // RET   copied string
(
   char               *destin,                  // OUT   destination buffer
   char               *start,                   // IN    start position
   int                 length                   // IN    max length to copy
)
{
   char               *s;
   char               *d;
   int                 l;
   char                delim  = ' ';            // default delimitter
   BOOL                escape = FALSE;

   if ((start != NULL) && (destin != NULL))
   {
      TRACES(( "COPY: Max length:%3.3u, start:}}}% -*.*s{{{\n",
                length, length, length, start));
      for (s = start, d = destin, l = 0; l < length; l++, s++)
      {
         if (*s == delim)
         {
            if (delim == ' ')                   // end of item reached
            {
               break;
            }
            else if (((*s == '"') || (*s == '\'')) && (escape))
            {
               *d++ = *s;                       // copy the character
            }
            else                                // end-of-string
            {
               delim = ' ';                     // to end of item ...
            }
         }
         else if ((!escape)  &&                 // prev was no escape
                  (delim == ' ')  &&            // and not in a string yet
                 ((*s == '"') || (*s == '\''))) // double or single quote
         {
            delim = *s;                         // to end of string
         }
         else if ( (*s     == '\\') &&          // possible escape
                  ((*(s+1) == ';')  ||          // for comment
                   (*(s+1) == '"')  ||          // for double quote
                   (*(s+1) == '\'') ))          // or single quote
         {
            escape = TRUE;                      // indicate escape next char
            continue;                           // and don't reset too early
         }
         else                                   // normal character
         {
            *d++ = *s;                          // copy the character
         }
         escape = FALSE;
//       TRACES(( "*s:'%c', esc:%lu, destin:'%s'\n", *s, escape, destin));
      }
      *d = '\0';                                // terminate the string
      TRACES(("COPY : '%s'\n", destin));
   }
   return (destin);
}                                               // end 'txaCopyItem'
/*---------------------------------------------------------------------------*/

