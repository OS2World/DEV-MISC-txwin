//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2007 Fsys Software and Jan van Wijk
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
// TX generic assignment and expression parser and executor
//
// Author: J. van Wijk
//
// Developed for LPT/DFS utilities
//

#include <txlib.h>                              // TX library interface
#include <txwpriv.h>                            // private window interface

//- limit for built-in function, number of parameters
#define TXS_MAXPARAMS  9

#define VmDebug if (ps->verbose) TxPrint

// Handle a TX-script expression: binary, numeric logical OR (Perl like)
static ULONG txsEvalExpression                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric logical AND (Perl like)
static ULONG txsEvalPerlLogAnd                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: unary, Logical NOT (Perl like)
static ULONG txsEvalPerlLogNot                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, comma operator
static ULONG txsEvalCommaOperator               // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, variable assignment
// recursion point for comma seperated lists like function arguments
static ULONG txsEvalAssignment                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric logical OR
static ULONG txsEvalNumLogOr                    // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric logical AND
static ULONG txsEvalNumLogAnd                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric bit OR
static ULONG txsEvalNumBitOr                    // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric bit XOR
static ULONG txsEvalNumBitXor                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric bit AND
static ULONG txsEvalNumBitAnd                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, string compare
static ULONG txsEvalStrCompare                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, type and value equality (identity)
static ULONG txsEvalTypeIdentity                // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric compare
static ULONG txsEvalNumCompare                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric bit shift
static ULONG txsEvalNumBitShift                 // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, string concatenation
static ULONG txsEvalStrConcat                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, string replication
static ULONG txsEvalStrReplic                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric plus and minus
static ULONG txsEvalNumPlusMin                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: binary, numeric multiply and division
static ULONG txsEvalNumMulDiv                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script expression: unary, +/-, Logical NOT and Bit-complement
static ULONG txsEvalUnaryExpr                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script atomic expression, litteral, variable, function ...
static ULONG txsEvalAtomicExpr                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script atomic-operand 'constant'
static ULONG txsEvalConstant                    // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Handle a TX-script atomic-operand 'builtin-function' with up to 9 params
static ULONG txsEvalBuiltinFunction             // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
);

// Validate parameter count for builtin functions, set error message if not OK
static ULONG txsFuncParamsOk
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *s,                       // IN    position
   char               *name,                    // IN    function name
   int                 count,                   // IN    number of parameters
   int                 minimum,                 // IN    minimum number
   int                 maximum                  // IN    maximum number
);

// Parse variable or function identifier, including optional [] or {} index
static ULONG txsParseIdentifier                 // RET   length of identifier
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char              **pp,                      // INOUT parse position
   char               *ident,                   // OUT   identifier
   TXSVALUE           *index                    // OUT   index or NULL
);                                              //       NUM-0 is no index

// Get and convert next digit
static BOOL txsGetDigit
(
   char                ch,                      // IN    digit character
   long                max,                     // IN    maximum digit value
   long               *val                      // OUT   digit value
);

// Return little-endian LLONG representation for binary-string
static LLONG txBinStrIntelReverse
(
   char               *bin,                     // IN    binary string l-endian
   int                 len                      // IN    number of bytes
);

// Return little-endian LLONG representation for hex-string
static LLONG txHexStrIntelReverse
(
   char               *hex                      // IN    hexadecimal string in
);                                              //       little-endian order

// Convert hex-string to ASCII string representation, in same string space!
static void txHexStr2Ascii
(
   char               *hex                      // INOUT hex/ASCII string
);



/*****************************************************************************/
// Replace each variable reference like $var in string, with their value
/*****************************************************************************/
ULONG txsResolveVariables                       // RET   INVALID_DATA if too large
(
   char               *string,                  // IN    string to be resolved
   char               *resolved,                // OUT   resolved string
   ULONG               length                   // IN    max length of resolved
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSPARSESTATE       pstate;                  // new result value & state
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            iv = TXSUNDEF;           // index value
   TXLN                vname;                   // variable name
   TXTT                vnum;                    // string for numeric value
   TXLN                chunk;                   // chunk to append to resolved
   int                 ln;
   char               *pp   = string;           // parse position
   char               *litt = pp;               // start of litteral fragment

   ENTER();
   TRACES(( "str:'%s'\n", string));

   pstate.syntax = FALSE;                       // execute (array index) expressions
   pstate.verbose = FALSE;                      // no debug output
   pstate.value.str = NULL;                     // init to empty
   strcpy( resolved, "");
   while (*pp && ((ln = strcspn( litt, "$")) != strlen(litt))) // variables only
   {
      pp = litt + ln;
      if (ln > 0)
      {
         if ((strlen(resolved) + ln) < length)
         {
            strncat( resolved, litt, ln);
         }
         else
         {
            rc = TX_INVALID_DATA;
            break;
         }
      }
      TRACES(("r:'%s' pp:'%s'\n", resolved, pp));
      if ((*pp == '$') && (!isalpha(*(pp+1)) && *(pp+1) != '_'))
      {
         //- Found dollar-sign but no valid variable name continuation
         //- handle as a one-character litteral and continue

         pp++;                                  // skip the dollar-sign
         strcat( resolved, "$");                // and add to result sofar
         if (*pp == '$')                        // double dollar, escaped
         {
            pp++;                               // skip second too
         }
      }
      else
      {
         //- MUST be a valid variable name, so look up its value

         litt = pp;                             // remember start of var name
         pp++;                                  // Skip the variable token itself
         if (((rc = txsParseIdentifier( &pstate, &pp, vname, &iv)) == NO_ERROR) &&
                                       (strlen( vname) > 0))
         {
            while (isspace(*(pp -1)))           // back to end of expression
            {                                   // to include exact whitespace
               pp--;                            // in the next litteral part
            }
            ln = pp - litt;                     // length of vname + possible index
            TxCopy( chunk, litt, ln +1);        // copy variable name+index itself
            TRACES(("vname:'%s' ln:%d", chunk, ln));

            rc = txsGetVariable( '$', vname, &iv, &nv); // get its value
            if (rc == NO_ERROR)
            {
               //- add {=value} to resolved, string or numeric value
               if (nv.str)
               {
                  strcat( chunk, CBY);
                  strcat( chunk, "{='");
                  strcat( chunk, CBM);
                  strcat( chunk, nv.str);
                  strcat( chunk, CBY);
                  strcat( chunk, "'}");
                  strcat( chunk, CNN);
               }
               else
               {
                  sprintf( vnum , "%s{=%s%lld%s}%s", CBY, CNC, nv.num, CBY, CNN);
                  strcat( chunk, vnum);
               }
            }
            else if (rc == TX_INVALID_HANDLE)
            {
               strcat( chunk, CBY);
               strcat( chunk, "{=[]}");         // variable index error
               strcat( chunk, CNN);
            }
            else
            {
               strcat( chunk, CBY);
               strcat( chunk, "{=?}");          // variable does not exist
               strcat( chunk, CNN);
            }
            txsVal2Zero( &nv);                  // free any allocated memory
            txsVal2Zero( &iv);
         }
         else
         {
            sprintf( chunk, "%c{=!}", *pp);     // Invalid variable name
            pp++;                               // Advance to avoid infinite loop
         }
         if ((strlen(resolved) + strlen(chunk)) < length)
         {
            strcat( resolved, chunk);
         }
         else
         {
            rc = TX_INVALID_DATA;
            break;
         }
      }
      litt = pp;
   }
   if (strlen( litt))                           // litteral at end
   {
      if ((strlen(resolved) + strlen( litt)) < length)
      {
         strcat( resolved, litt);               // final part of string
      }
      else
      {
         rc = TX_INVALID_DATA;
      }
   }

   TRACES(( "res:'%s'\n", resolved));
   RETURN (rc);
}                                               // end 'txsResolveExpressions'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Replace each expression like {4 * $sz} or $var in string, with their value
// Expressions must either start with a variable $xx, or be enclosed as {expr}
/*****************************************************************************/
ULONG txsResolveExpressions                     // RET   PENDING to execute
(
   char               *ref,                     // IN    ref string for errors
   ULONG               lnr,                     // IN    linenr, start of expr
   BOOL                syntax,                  // IN    syntax-check only
   char               *string,                  // INOUT string with expressions
   ULONG               length,                  // IN    max length of string
   char               *errtxt                   // INOUT error text (or NULL)
)                                               //       minimal length TXMAXTM
{
   ULONG               rc = NO_ERROR;           // function return
   TXSPARSESTATE       pstate;                  // new result value & state
   int                 ln;
   char                et;                      // 1st char of expr, { or $
   char               *pp   = string;           // parse position
   char               *litt = pp;               // start of litteral fragment
   char               *resolved;                // resolved result string
   BOOL                execute = FALSE;         // execute the resulting string
   BOOL                verbose = TxaExeSwitch( TXA_O_DEBUG);

   ENTER();
   TRACES(( "ref: %8.8lx str:%8.8lx lnr:%lu syntax:%s\n",
             ref, string, lnr, (syntax) ? "syntax-check-only" : "execute!"));
   TRACES(( "str:'%s'\n", string));

   if (errtxt != NULL)
   {
      strcpy( errtxt, "");
   }
   pstate.value.str = NULL;                     // init to empty
   if ((resolved = TxAlloc( 1, length)) != NULL)
   {
      while (*pp && ((ln = strcspn( litt, TXSL_RESOLV_CHARS)) != strlen(litt)))
      {
         pp = litt + ln;
         if (ln > 0)
         {
            if ((strlen(resolved) + ln) < length)
            {
               strncat( resolved, litt, ln);
               if ((litt != string) || (strspn( resolved, " ") < strlen( resolved)))
               {
                  TRACES(("Execute because of LITTERAL '%*.*s'\n", ln, ln, litt));
                  execute = TRUE;               // result should be executed
               }
            }
            else
            {
               TxsExprError( &pstate, pp, "Resolved string too large!\n");
               rc = TX_INVALID_DATA;
            }
            if (verbose)
            {
               TxPrint( "LITTERAL  '%*.*s'\n", ln, ln, litt);
            }
         }
         if ((*pp == '$') && (!isalpha(*(pp+1)) && *(pp+1) != '_'))
         {
            //- Found dollar-sign but no valid variable name continuation
            //- handle as a one-character litteral and continue

            pp++;                               // skip the dollar-sign
            strcat( resolved, "$");             // and add to result sofar
            if (*pp == '$')                     // double dollar, escaped
            {
               pp++;                            // skip second too
               if (verbose)
               {
                  TxPrint( "ESCAPED$  '$'\n");
               }
            }
            else
            {
               if (verbose)
               {
                  TxPrint( "LITTERAL  '$'\n");
               }
            }
         }
         else
         {
            if ((et = *pp) == TXS_BOP)          // bracketed expression ?
            {
               pp++;                            // skip start-bracket
               execute = TRUE;                  // result should be executed
               TRACES(("Execute because of bracketed expression\n"));
            }
            txsVal2Zero( &pstate.value);        // free any allocated memory
            if ((rc = txsEvaluateExpr( ref, lnr, syntax, &pp, &pstate)) == NO_ERROR)
            {
               if (txsNextToken(&pp, &ln) == TXS_BCL)
               {
                  pp++;                         // skip end-bracket
               }
               else if (et == TXS_BOP)          // bracketed expr not terminated
               {
                  TxsExprError( &pstate, pp, "Missing '%c' or missing operator\n", TXS_BCL);
                  rc = TX_SYNTAX_ERROR;
                  break;
               }
               while (isspace(*(pp -1)))        // back to end of expression
               {                                // to include exact whitespace
                  pp--;                         // in the next litteral part
               }
               if (pstate.value.str == NULL)    // it is a number
               {
                  if (txsVal2Str( &pstate.value, 0, "d") == NULL)
                  {
                     rc = TX_ALLOC_ERROR;
                  }
               }
               if (rc == NO_ERROR)
               {
                  //- Resolved string MUST be null terminated, so can use that too
                  TRACES(( "Resolved expression: '%s'\n", pstate.value.str));

                  if ((strlen(resolved) + pstate.value.len) < length)
                  {
                     strncat( resolved, pstate.value.str, pstate.value.len);
                     if (pstate.result == TXSEXPR_EXPRESSION)
                     {
                        TRACES(("Execute because of EXPRESSION result\n"));
                        execute = TRUE;         // result should be executed
                     }
                  }
                  else
                  {
                     TxsExprError( &pstate, pp, "Resolved string too large!\n");
                     rc = TX_INVALID_DATA;
                  }
               }
            }
            else
            {
               break;
            }
         }
         litt = pp;
      }
      if (rc == NO_ERROR)
      {
         if (strlen( litt))                     // litteral at end
         {
            if ((strlen(resolved) + strlen( litt)) < length)
            {
               strcat( resolved, litt);         // final part of string
               if ((ln = strspn( litt, " ")) < strlen( litt))
               {                                // not empty and also
                  if (litt[ ln] != ';')         // not a comment ...
                  {
                     execute = TRUE;            // result should be executed
                     TRACES(("Execute because of LITTERAL at END '%s'\n", litt));
                  }
               }
               if (verbose)
               {
                  TxPrint( "LITTERAL  '%s'\n", litt);
               }
            }
            else
            {
               TxsExprError( &pstate, pp, "Resolved string too large!\n");
               rc = TX_INVALID_DATA;
            }
         }
      }
      if (rc == NO_ERROR)
      {
         if (!syntax)                           // unless syntax check only
         {
            strcpy( string, resolved);          // replace string by resolved
            if (execute)
            {
               rc = TX_PENDING;                 // indicate execution desired
            }
         }
      }
      else
      {
         if ((errtxt) && strlen( pstate.msg))
         {
            strcpy( errtxt, pstate.msg);
         }
      }
      TxFreeMem( resolved);
      txsVal2Zero( &pstate.value);              // free any allocated memory
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   TRACES(( "str:'%s'\n", string));
   RETURN (rc);
}                                               // end 'txsResolveExpressions'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: top-level, non recursive
/*****************************************************************************/
ULONG txsEvaluateExpr                           // RET   function result
(
   char               *ref,                     // IN    start of line
   ULONG               lnr,                     // IN    linenr, start of expr
   BOOL                syntax,                  // IN    syntax-check only
   char              **pos,                     // INOUT position in string
   TXSPARSESTATE      *pstate                   // OUT   result value and state
)
{
   ULONG               rc = NO_ERROR;           // function return
   char               *string = *pos;

   ENTER();

   TRACES(( "ref: %8.8lx pos:%8.8lx lnr:%lu syntax:%s\n",
             ref, *pos, lnr, (syntax) ? "syntax-check-only" : "execute!"));

   pstate->start     = ref;
   pstate->linenr    = lnr;
   pstate->syntax    = syntax;
   pstate->verbose   = TxaExeSwitch( TXA_O_DEBUG);
   pstate->result    = TXSEXPR_INITIAL;
   pstate->endpos    = 0;
   pstate->msg[0]    = 0;
   pstate->value.len = 0;
   pstate->value.num = 0;
   pstate->value.str = NULL;

   if (((rc = txsEvalExpression( pstate, TRUE, string, pos, &pstate->value))
               != NO_ERROR) && ( pstate->verbose))
   {
      if (pstate->result != TXSEXPR_INITIAL)    // some expression found
      {
         if (strlen( pstate->msg))
         {
            TxPrint( "%s\n", pstate->msg);
         }
         else
         {
            TxPrint( "\nUnexpected error %lu evaluating: '%s'\n", rc, string);
         }
      }
      else
      {
         TxPrint( "No assignment/expression found, continue with: '%.20s'\n", *pos);
      }
   }
   else if (rc == TX_ALLOC_ERROR)
   {
      TxsExprError( pstate, *pos, "Memory allocation error\n");
   }
   pstate->endpos = *pos - ref;
   if ((pstate->result != TXSEXPR_INITIAL) && (pstate->verbose)) // some expression found
   {
      TxPrint( "Parsed from position %d to %d, found an : %s\n", string - ref, pstate->endpos,
               (pstate->result == TXSEXPR_ASSIGNMENT) ? "Assignment"   :
               (pstate->result == TXSEXPR_EXPRESSION) ? "Expression"   : "Syntax error");
   }
   TRACES(("number result: %lu\n", pstate->value.num));
   RETURN (rc);
}                                               // end 'txsEvaluateExpr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric logical OR (Perl like)
// This is the recursion point for nested expressions (with/without commas)
/*****************************************************************************/
static ULONG txsEvalExpression                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length
   BOOL                syntax = ps->syntax;     // remember lazy syntax status

   if ((rc = txsEvalPerlLogAnd(ps, comma, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == 'o')
      {
         s += ln;
         txsVal2Bool( &nv);
         if (nv.num != 0LL)                     // already TRUE, lazy ...
         {
            ps->syntax = TRUE;                  // just syntax check from here
         }
         if ((rc = txsEvalPerlLogAnd(ps, comma, s, &s, &rv)) == NO_ERROR)
         {
            if (ps->syntax)
            {
               VmDebug("PERLOR, lazy\n");
            }
            else
            {
               txsVal2Bool( &rv);
               nv.num = (nv.num || rv.num) ? 1LL : 0LL;
               VmDebug("PERLOR\n");
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
      ps->syntax = syntax;                      // restore lazy syntax status
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalExpression'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric logical AND (Perl like)
/*****************************************************************************/
static ULONG txsEvalPerlLogAnd                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length
   BOOL                syntax = ps->syntax;     // remember lazy syntax status

   if ((rc = txsEvalPerlLogNot(ps, comma, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == 'a')
      {
         s += ln;
         txsVal2Bool( &nv);
         if (nv.num == 0LL)                     // already FALSE, lazy ...
         {
            ps->syntax = TRUE;                  // just syntax check from here
         }
         if ((rc = txsEvalPerlLogNot(ps, comma, s, &s, &rv)) == NO_ERROR)
         {
            if (ps->syntax)
            {
               VmDebug("PERLAND, lazy\n");
            }
            else
            {
               txsVal2Bool( &rv);
               nv.num = (nv.num && rv.num) ? 1LL : 0LL;
               VmDebug("PERLAND\n");
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
      ps->syntax = syntax;                      // restore lazy syntax status
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalPerlLogAnd'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: unary, Logical NOT (Perl like)
/*****************************************************************************/
static ULONG txsEvalPerlLogNot                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((op = txsNextToken(&s, &ln)), op == 't')
   {
      while ((op = txsNextToken(&s, &ln)), op == 't')
      {
         s += ln;
         if ((rc = txsEvalPerlLogNot(ps, comma, s, &s, &nv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            nv.num = (nv.num) ? 0LL : 1LL;
            VmDebug("PERLNOT\n");
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   else                                         // no unary operator
   {
      rc = txsEvalCommaOperator(ps, comma, s, &s, &nv);
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalPerlLogNot'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, comma operator
/*****************************************************************************/
static ULONG txsEvalCommaOperator               // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   BOOL                comma,                   // IN    comma OP allowed
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalAssignment(ps, s, &s, &nv)) == NO_ERROR)
   {
      while (comma && ((op = txsNextToken(&s, &ln)) == ','))
      {
         s += ln;
         txsVal2Zero( &nv);                     // discard previous expr
         if ((rc = txsEvalAssignment(ps, s, &s, &nv)) == NO_ERROR)
         {
            VmDebug("COMMA\n");
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalCommaOperator'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, variable assignment
// recursion point for comma seperated lists like function arguments
/*****************************************************************************/
static ULONG txsEvalAssignment                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // array index value
   char               *s  = string;             // ptr in number string
   char                vt;                      // variable type
   char                op = 0;                  // operator
   int                 ln;                      // token length
   TXTM                vname;
   TXLN                vfull;                   // full indexed name
   TXSERESULT          initial = ps->result;

   if ((vt = txsNextToken(&s, &ln)), vt == '$' || vt == '@' || vt =='%')
   {
      s += ln;
      if (txsParseIdentifier( ps, &s, vname, &rv) == NO_ERROR)
      {
         if ((op = txsNextToken(&s, &ln)) == '=')
         {
            if (initial == TXSEXPR_INITIAL)     // ps->result may have been set
            {                                   // by an array index expression
               ps->result = TXSEXPR_ASSIGNMENT;
            }
            s += ln;                            // recurse (multi assignment)

            //- to be refined, could add recognition for @var and %var assign
            //- can only assign from another such variable, not expression!
            //- (or need to implement list-syntax/semantics for assignments)
            //- Need 'deep-copy' functionality in var-pool

            if (vt == '$')                      // scalar context
            {
               //- allow recursive asignment like $a=$b=$c
               //- but NO comma operator $a=2,3 ==> use $a=(2,3) instead,
               //- otherwise a multiple  $a=2, $b=5  will NOT work as expected!
               if (((rc = txsEvalAssignment(ps, s, &s, &nv)) == NO_ERROR) &&
                   (!ps->syntax))
               {
                  if (!rv.defined)              // no index specified ?
                  {
                     strcpy( vfull, vname);
                  }
                  else
                  {
                     if (rv.str)
                     {
                        sprintf( vfull, "%s{'%s'}", vname, rv.str);
                     }
                     else
                     {
                        sprintf( vfull, "%s[%lld]", vname, rv.num);
                     }
                  }
                  if ((rc = txsSetVariable( vt, vname, &rv, &nv)) == NO_ERROR)
                  {
                     VmDebug("ASSIGN    $%s", vfull);
                     if (nv.str)
                     {
                        VmDebug(" = '%-.*s'\n", 60, nv.str);
                     }
                     else
                     {
                        VmDebug(" = %lld\n", nv.num);
                     }
                  }
                  else if (rc == TX_INVALID_HANDLE)
                  {
                     if (rv.str)                // had an index
                     {
                        TxsExprError( ps, s, "Variable '%s' is a scalar, not an array", vname);
                     }
                     else
                     {
                        TxsExprError( ps, s, "Variable '%s' is an array, not a scalar", vname);
                     }
                  }
                  else if (rc == TX_WRITE_PROTECT)
                  {
                     TxsExprError( ps, s, "Variable '%s' is read-only!", vname);
                  }
               }
            }
            else if (vt == '@')                 // array, list context
            {
               TxsExprError( ps, s, "Direct array assignment not supported (%c%s)", vt, vname);
               rc = TX_SYNTAX_ERROR;
            }
            else                                // hash, list context
            {
               TxsExprError( ps, s, "Direct hash assignment not supported (%c%s)", vt, vname);
               rc = TX_SYNTAX_ERROR;
            }
         }
         txsVal2Zero( &rv);                     // free any allocated memory
      }                                         // on any error, reparse as expr
   }
   if ((rc == NO_ERROR) && (op != '='))
   {
      s = string;                               // no assignment reset to start
      rc = txsEvalNumLogOr(ps, s, &s, &nv);
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalAssignment'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric logical OR
/*****************************************************************************/
static ULONG txsEvalNumLogOr                    // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length
   BOOL                syntax = ps->syntax;     // remember lazy syntax status

   if ((rc = txsEvalNumLogAnd(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == 'O')
      {
         s += ln;
         txsVal2Bool( &nv);
         if (nv.num != 0LL)                     // already TRUE, lazy ...
         {
            ps->syntax = TRUE;                  // just syntax check from here
         }
         if ((rc = txsEvalNumLogAnd(ps, s, &s, &rv)) == NO_ERROR)
         {
            if (ps->syntax)
            {
               VmDebug("LOGOR, lazy\n");
            }
            else
            {
               txsVal2Bool( &rv);
               nv.num = (nv.num || rv.num) ? 1LL : 0LL;
               VmDebug("LOGOR\n");
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
      ps->syntax = syntax;                      // restore lazy syntax status
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumLogOr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric logical AND
/*****************************************************************************/
static ULONG txsEvalNumLogAnd                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length
   BOOL                syntax = ps->syntax;     // remember lazy syntax status

   if ((rc = txsEvalNumBitOr(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == 'A')
      {
         s += ln;
         txsVal2Bool( &nv);
         if (nv.num == 0LL)                     // already FALSE, lazy ...
         {
            ps->syntax = TRUE;                  // just syntax check from here
         }
         if ((rc = txsEvalNumBitOr(ps, s, &s, &rv)) == NO_ERROR)
         {
            if (ps->syntax)
            {
               VmDebug("LOGAND, lazy\n");
            }
            else
            {
               txsVal2Bool( &rv);
               nv.num = (nv.num && rv.num) ? 1LL : 0LL;
               VmDebug("LOGAND\n");
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
      ps->syntax = syntax;                      // restore lazy syntax status
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumLogAnd'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric bit OR
/*****************************************************************************/
static ULONG txsEvalNumBitOr                    // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalNumBitXor(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == '|')
      {
         s += ln;
         if ((rc = txsEvalNumBitXor(ps, s, &s, &rv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            txsVal2Num( &rv);
            nv.num = nv.num | rv.num;
            VmDebug("BITOR\n");
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumBitOr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric bit XOR
/*****************************************************************************/
static ULONG txsEvalNumBitXor                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalNumBitAnd(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == '^')
      {
         s += ln;
         if ((rc = txsEvalNumBitAnd(ps, s, &s, &rv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            txsVal2Num( &rv);
            nv.num = nv.num ^ rv.num;
            VmDebug("BITXOR\n");
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumBitXor'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric bit AND
/*****************************************************************************/
static ULONG txsEvalNumBitAnd                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalStrCompare(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == '&')
      {
         s += ln;
         if ((rc = txsEvalStrCompare(ps, s, &s, &rv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            txsVal2Num( &rv);
            nv.num = nv.num & rv.num;
            VmDebug("BITAND\n");
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumBitAnd'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, string compare
/*****************************************************************************/
static ULONG txsEvalStrCompare                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalTypeIdentity(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)), op == 'e' || op == 'n' ||
                                           op == 'K' || op == 'B' ||
                                           op == 'l' || op == 'g')
      {
         s += ln;
         if ((rc = txsEvalTypeIdentity(ps, s, &s, &rv)) == NO_ERROR)
         {
            if ((txsVal2Str( &nv, 0, "d") != NULL) &&
                (txsVal2Str( &rv, 0, "d") != NULL)  )
            {
               int d = memcmp( nv.str, rv.str, min( nv.len, rv.len));

               if (d == 0)                      // string contents same over length
               {                                // of shorter string, so difference
                  d = nv.len - rv.len;          // in length determines result
               }
               txsVal2Zero( &nv);               // free any allocated memory
               switch (op)
               {
                  case 'e': nv.num = (d == 0) ? 1LL : 0LL; VmDebug("STREQ"); break;
                  case 'n': nv.num = (d != 0) ? 1LL : 0LL; VmDebug("STRNE"); break;
                  case 'K': nv.num = (d <  0) ? 1LL : 0LL; VmDebug("STRLT"); break;
                  case 'B': nv.num = (d >  0) ? 1LL : 0LL; VmDebug("STRGT"); break;
                  case 'l': nv.num = (d <= 0) ? 1LL : 0LL; VmDebug("STRLE"); break;
                  default : nv.num = (d >= 0) ? 1LL : 0LL; VmDebug("STRGE"); break;
               }
               VmDebug( " : %llu\n", nv.num);
            }
            else
            {
               TxsExprError( ps, s, "Memory allocation error in '%s'", string);
               rc = TX_ALLOC_ERROR;
            }
         }
         else
         {
            break;                              // stop on first error
         }
         txsVal2Zero( &rv);                     // free all right-hand memory
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalStrCompare'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, type and value equality (identity)
/*****************************************************************************/
static ULONG txsEvalTypeIdentity                // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalNumCompare(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)), op == 'I' || op == 'X')
      {
         s += ln;
         if ((rc = txsEvalNumCompare(ps, s, &s, &rv)) == NO_ERROR)
         {
            BOOL       identical = FALSE;

            if      ((nv.str != NULL) && (rv.str != NULL)) // both are string
            {
               identical = ((nv.len == rv.len) && (strcmp( nv.str, rv.str) == 0));
            }
            else if ((nv.str == NULL) && (rv.str == NULL)) // both are numeric
            {
               identical = (nv.num == rv.num);
            }
            txsVal2Zero( &nv);                  // free any allocated memory
            switch (op)
            {
               case 'I': nv.num = (identical) ? 1LL : 0LL; VmDebug("IDEQ"); break;
               default : nv.num = (identical) ? 0LL : 1LL; VmDebug("IDNE"); break;
            }
            VmDebug( " : %llu\n", nv.num);
         }
         else
         {
            break;                              // stop on first error
         }
         txsVal2Zero( &rv);                     // free any allocated memory
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalTypeIdentity'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric compare
/*****************************************************************************/
static ULONG txsEvalNumCompare                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalNumBitShift(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)), op == '<' || op == '>' ||
                                           op == 'L' || op == 'G' ||
                                           op == 'E' || op == 'N')
      {
         s += ln;
         if ((rc = txsEvalNumBitShift(ps, s, &s, &rv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            txsVal2Num( &rv);
            switch (op)
            {
               case 'E': nv.num = (nv.num == rv.num) ? 1LL : 0LL; VmDebug("NUMEQ"); break;
               case 'N': nv.num = (nv.num != rv.num) ? 1LL : 0LL; VmDebug("NUMNE"); break;
               case '<': nv.num = (nv.num <  rv.num) ? 1LL : 0LL; VmDebug("NUMLT"); break;
               case '>': nv.num = (nv.num >  rv.num) ? 1LL : 0LL; VmDebug("NUMGT"); break;
               case 'L': nv.num = (nv.num <= rv.num) ? 1LL : 0LL; VmDebug("NUMLE"); break;
               default : nv.num = (nv.num >= rv.num) ? 1LL : 0LL; VmDebug("NUMGE"); break;
            }
            VmDebug( " : %llu\n", nv.num);
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumCompare'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric bit shift
/*****************************************************************************/
static ULONG txsEvalNumBitShift                 // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalStrConcat(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)), op == 'S' || op =='R')
      {
         s += ln;
         if ((rc = txsEvalStrConcat(ps, s, &s, &rv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            txsVal2Num( &rv);
            if (rv.num <= 64)
            {
               switch (op)
               {
                  case 'S':
                     nv.num <<= rv.num;
                     VmDebug("SHIFTLEFT\n");
                     break;

                  default:
                     nv.num >>= rv.num;
                     VmDebug("SHIFTRIGHT\n");
                     break;
               }
            }
            else
            {
               TxsExprError( ps, s, "Shift exceeds 64 bits '%s'", string);
               rc = TX_INVALID_DATA;
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumBitShift'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, string concatenation
/*****************************************************************************/
static ULONG txsEvalStrConcat                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // length

   if ((rc = txsEvalStrReplic(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == '.')
      {
         s += ln;
         if ((rc = txsEvalStrReplic(ps, s, &s, &rv)) == NO_ERROR)
         {
            if ((txsVal2Str( &nv, 0, "d") != NULL) &&
                (txsVal2Str( &rv, 0, "d") != NULL)  )
            {
               ln = nv.len + rv.len + 1;
               if (ln > nv.mem)
               {
                  char   *lv = nv.str;          // left hand string

                  nv.mem = TXSVSIZE(ln);
                  if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                  {
                     memcpy( nv.str, lv, nv.len);
                     TxFreeMem( lv);            // free 'old' string memory
                  }
                  else
                  {
                     rc = TX_ALLOC_ERROR;
                  }
               }
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
            if (rc == NO_ERROR)
            {
               memcpy( nv.str + nv.len, rv.str, rv.len);
               nv.len += rv.len;
               VmDebug("CONCAT\n");
            }
            else
            {
               TxsExprError( ps, s, "Memory allocation error in '%s'", string);
            }
         }
         else
         {
            break;                              // stop on first error
         }
         txsVal2Zero( &rv);                     // free any allocated memory
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalStrConcat'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, string replication
/*****************************************************************************/
static ULONG txsEvalStrReplic                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in string
   char                op;                      // operator
   int                 ln;                      // length
   char               *lv = NULL;               // original left-hand value
   int                 i;

   if ((rc = txsEvalNumPlusMin(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)) == 'x')
      {
         s += ln;
         if ((rc = txsEvalNumPlusMin(ps, s, &s, &rv)) == NO_ERROR)
         {
            if (txsVal2Str( &nv, 0, "d") != NULL)
            {
               txsVal2Num( &rv);
               lv = nv.str;                     // steal original string
               ln = nv.len * rv.num;            // resulting length
               nv.mem = TXSVSIZE(ln);
               if ((nv.str = TxAlloc( 1, nv.mem)) == NULL)
               {
                  rc = TX_ALLOC_ERROR;
               }
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
            if (rc == NO_ERROR)
            {
               for (i = 0; i < rv.num; i++)
               {
                  memcpy( nv.str + nv.len * i, lv, nv.len);
               }
               nv.len = ln;
               VmDebug("REPLIC\n");
               TxFreeMem( lv);                  // free original string
            }
            else
            {
               TxsExprError( ps, s, "Memory allocation error in '%s'", string);
            }
         }
         else
         {
            break;                              // stop on first error
         }
         txsVal2Zero( &rv);                     // free any allocated memory
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalStrReplic'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric plus and minus
/*****************************************************************************/
static ULONG txsEvalNumPlusMin                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalNumMulDiv(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)), op == '+' || op == '-')
      {
         s += ln;
         if ((rc = txsEvalNumMulDiv(ps, s, &s, &rv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            txsVal2Num( &rv);
            switch (op)
            {
               case '-':
                  nv.num -= rv.num;
                  VmDebug("SUBTRACT\n");
                  break;

               default:
                  nv.num += rv.num;
                  VmDebug("ADD\n");
                  break;
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumPlusMin'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: binary, numeric multiply and division
/*****************************************************************************/
static ULONG txsEvalNumMulDiv                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((rc = txsEvalUnaryExpr(ps, s, &s, &nv)) == NO_ERROR)
   {
      while ((op = txsNextToken(&s, &ln)), op == '*' || op == '/' || op == '%')
      {
         s += ln;
         if ((rc = txsEvalUnaryExpr(ps, s, &s, &rv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            txsVal2Num( &rv);
            switch (op)
            {
               case '%':
               case '/':
                  if (rv.num != 0)
                  {
                     if (op == '/')
                     {
                        nv.num /= rv.num;
                        VmDebug("DIVIDE\n");
                     }
                     else
                     {
                        nv.num %= rv.num;
                        VmDebug("MODULO\n");
                     }
                  }
                  else if (!ps->syntax)
                  {
                     TxsExprError( ps, s, "Divide by zero in '%s'", string);
                     rc = TX_INVALID_DATA;
                  }
                  break;

               default:
                  nv.num *= rv.num;
                  VmDebug("MULTIPLY\n");
                  break;
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalNumMulDiv'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script expression: unary, +/-, Logical NOT and Bit-complement
/*****************************************************************************/
static ULONG txsEvalUnaryExpr                   // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length

   if ((op = txsNextToken(&s, &ln)), op == '+' || op == '-' ||
                                     op == '!' || op == '~')
   {
      while ((op = txsNextToken(&s, &ln)), op == '+' || op == '-' ||
                                           op == '!' || op == '~')
      {
         s += ln;
         if ((rc = txsEvalUnaryExpr(ps, s, &s, &nv)) == NO_ERROR)
         {
            txsVal2Num( &nv);
            switch (op)
            {
               case '!':
                  nv.num = (nv.num) ? 0LL : 1LL;
                  VmDebug("LOGNOT\n");
                  break;

               case '~':
                  nv.num = ~nv.num;
                  VmDebug("COMPLEMENT\n");
                  break;

               case '-':
                  nv.num = -(nv.num);
                  VmDebug("NEGATIVE\n");
                  break;

               default:
                  VmDebug("POSITIVE\n");
                  break;
            }
         }
         else
         {
            break;                              // stop on first error
         }
      }
   }
   else                                         // no unary operator
   {
      rc = txsEvalAtomicExpr(ps, s, &s, &nv);
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalUnaryExpr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script atomic expression, litteral, variable, function ...
/*****************************************************************************/
static ULONG txsEvalAtomicExpr                  // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // right-hand value
   TXSVALUE            iv = TXSUNDEF;           // index value
   char               *s  = string;             // ptr in number string
   char                op;                      // operator
   int                 ln;                      // token length
   TXTM                vname;
   TXLN                vfull;                   // full indexed name
   TXSVALUE           *aidiv = NULL;            // auto inc/dec variable
   char                aidop = 0;               // auto inc/dec operator
   char                vt;                      // variable type

   if ((op = txsNextToken( &s, &ln)) == TXS_POP)
   {                                            // nested expr or ternary
      s += ln;
      if ((rc = txsEvalExpression(ps, TRUE, s, &s, &nv)) == NO_ERROR)
      {
         if (txsNextToken( &s, &ln) == TXS_PCL)
         {
            if (ps->result == TXSEXPR_INITIAL)
            {
               ps->result = TXSEXPR_EXPRESSION;
            }
            s += ln;
            if ((op = txsNextToken(&s, &ln)) == '?')
            {                                   // ternary expression
               BOOL    cond   = txsVal2Bool( &nv);
               BOOL    syntax = ps->syntax;

               ps->syntax = !cond;              // syntax-only if false
               s += ln;
               if ((rc = txsEvalExpression(ps, TRUE, s, &s, &nv)) == NO_ERROR)
               {
                  if ((op = txsNextToken(&s, &ln)) == ':') // ternary separator
                  {
                     ps->syntax = cond;         // syntax-only if true
                     s += ln;
                     if ((rc = txsEvalExpression(ps, TRUE, s, &s, &rv)) == NO_ERROR)
                     {
                        if (cond)               // keep nv value
                        {
                           txsVal2Zero( &rv);   // free any allocated memory
                           VmDebug("DISCARD   top\n");
                        }
                        else                   // move rv to nv value
                        {
                           txsVal2Zero( &nv);   // free any allocated memory
                           VmDebug("DISCARD   top-1\n");
                           nv.num = rv.num;
                           nv.mem = rv.mem;
                           nv.len = rv.len;
                           nv.str = rv.str;

                           rv.str = NULL;       // must NOT be freed!
                        }
                     }
                     else
                     {
                        txsVal2Zero( &nv);      // free any allocated memory
                     }
                  }
                  else
                  {
                     txsVal2Zero( &nv);         // free any allocated memory
                     TxsExprError( ps, s, "Missing ':' in ternary expr: '%s'", string);
                     rc = TX_SYNTAX_ERROR;
                  }
               }
               ps->syntax = syntax;             // restore syntax-check flag
            }
         }
         else
         {
            TxsExprError( ps, s, "Missing ')' in expression: '%s'", string);
            rc = TX_SYNTAX_ERROR;
         }
      }
   }
   else if ((op == '$') || (op == '@') || (op == '%') || //- possible variable
            (op == 'i') || (op == 'd'))                  //- or auto incr/decr
   {
      if ((op == 'i') || (op == 'd'))           // with auto pre incr/decr
      {
         s += ln;
         aidop = op;
         aidiv = &nv;                           // pre inc/dec on result-var
         op = txsNextToken( &s, &ln);
      }
      if ((op == '$') || (op == '@') || (op == '%')) //- possible variable
      {
         TXSERESULT    initial = ps->result;    // remember BEFORE identifier

         s += ln;
         vt = op;
         if (((rc = txsParseIdentifier( ps, &s, vname, &iv)) == NO_ERROR) &&
                                       (strlen( vname) > 0))
         {
            ps->result = initial;               // ps->result may have been set
                                                // by an array index expression
            if (!iv.defined)                    // no index specified ?
            {
               strcpy( vfull, vname);
            }
            else
            {
               if (iv.str)
               {
                  sprintf( vfull, "%s{'%s'}", vname, iv.str);
               }
               else
               {
                  sprintf( vfull, "%s[%lld]", vname, iv.num);
               }
            }
            rc = txsGetVariable( vt, vname, &iv, &nv);
            if (rc == TX_FAILED)                  //- value will be 'undef', 0 or ""
            {                                     //- and may not be set during the
               if (!ps->syntax && txwa->pedantic) //- syntax check phase yet ...
               {
                  TxsExprError( ps, s, "Variable '%s' does not exist", vfull);
               }
               else
               {
                  rc = NO_ERROR;
               }
            }
            if (rc == NO_ERROR)
            {
               VmDebug("LOADVAR   %c%s", op, vfull);
               if (nv.str)
               {
                  VmDebug(" = '%-.*s'\n", 60, nv.str);
               }
               else
               {
                  VmDebug(" = %lld\n", nv.num);
               }
               if ((op = txsNextToken( &s, &ln)), op == 'i' || op == 'd')
               {                                // post inc/dec on copied var
                  s += ln;                      // skip auto inc/dec operator
                  if (aidiv == NULL)            // no pre ind/dec set yet ?
                  {
                     rv = nv;                   // make shallow copy
                     rv.mem = 0;                // fake static memory (no free)
                     aidop = op;
                     aidiv = &rv;
                  }
                  else
                  {
                     TxsExprError( ps, s, "Double incr/decr on same variable: '%s'", string);
                     rc = TX_SYNTAX_ERROR;
                  }
               }
               if ((aidiv != NULL) && ((vt == '@') || (vt == '%')))
               {
                  TxsExprError( ps, s, "Auto incr/decr on array or hash variable: '%s'", string);
                  rc = TX_SYNTAX_ERROR;
               }
               if ((aidiv != NULL) && (rc == NO_ERROR)) // var to inc/dec and save ?
               {
                  ps->result = TXSEXPR_ASSIGNMENT;
                  txsVal2Num( aidiv);
                  if (aidop == 'i')
                  {
                     aidiv->num++;
                     VmDebug("INCR\n");
                  }
                  else
                  {
                     aidiv->num--;
                     VmDebug("DECR\n");
                  }
                  if (!ps->syntax)
                  {
                     if ((rc = txsSetVariable( '$', vname, &iv, aidiv)) == NO_ERROR)
                     {
                        VmDebug("ASSIGN    $%s", vfull);
                        if (aidiv->str)
                        {
                           VmDebug(" = '%-.*s'\n", 60, aidiv->str);
                        }
                        else
                        {
                           VmDebug(" = %lld\n", aidiv->num);
                        }
                     }
                  }
               }
            }
            else if (rc == TX_INVALID_HANDLE)
            {
               if (rv.str)                      // had an index
               {
                  TxsExprError( ps, s, "Variable '%s' is a scalar, not an array", vname);
               }
               else
               {
                  TxsExprError( ps, s, "Variable '%s' is an array, not a scalar", vname);
               }
            }
            txsVal2Zero( &rv);                  // free any allocated memory
         }
         else
         {
            TxsExprError( ps, s, "Invalid variable name: '%s'", string);
            s -= ln;
            rc = TX_SYNTAX_ERROR;
         }
         if (ps->result == TXSEXPR_INITIAL)
         {
            ps->result = TXSEXPR_EXPRESSION;
         }
      }
      else
      {
         TxsExprError( ps, s, "Variable expected for auto incr/decr: '%s'", string);
         rc = TX_SYNTAX_ERROR;
      }
   }
   else if ((op == '"') || (op == '\''))        // quoted string
   {
      char   *start = ++s;

      while ((*s) && (*s != op))
      {
         s++;
      }
      if (*s == op)
      {
         ln = (s - start);
         s++;                                   // skip final string terminator
         nv.mem = TXSVSIZE(ln);
         if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
         {
            nv.defined = TRUE;
            nv.str[0]  = 0;
            nv.len     = ln;
            strncat( nv.str, start, ln);
            VmDebug("LOADSTR   '%s'\n", nv.str);
         }
         else
         {
            TxsExprError( ps, s, "Memory allocation error for '%s'", start);
            rc = TX_ALLOC_ERROR;
         }
      }
      else
      {
         TxsExprError( ps, s, "%s quoted string not terminated\n",
                               (op) == '"' ? "Double" : "Single");
         rc = TX_SYNTAX_ERROR;
      }
      if (ps->result == TXSEXPR_INITIAL)
      {
         ps->result = TXSEXPR_EXPRESSION;
      }
   }
   else if (isdigit( *s))
   {
      switch (rc = txsParseLlongNumber( s, &s, &nv.num))
      {
         case NO_ERROR:
            nv.defined = TRUE;
            VmDebug("LOADNUM   %lld\n", nv.num);
            break;

         case TX_FAILED:
            TxsExprError( ps, s, "Overflow in number: '%s'", string);
            rc = TX_INVALID_DATA;
            break;

         case TX_ERROR:
            TxsExprError( ps, s, "Invalid number format: '%s'", string);
            rc = TX_SYNTAX_ERROR;
            break;

         default:
            TxsExprError( ps, s, "Number conversion failure on: '%s'", string);
            rc = TX_INVALID_DATA;
            break;
      }
      if (ps->result == TXSEXPR_INITIAL)
      {
         ps->result = TXSEXPR_EXPRESSION;
      }
   }
   else if (txsEvalConstant( ps, s, &s, &nv) != NO_ERROR)
   {
      rc = txsEvalBuiltinFunction( ps, s , &s, &nv);
   }
   if (rc != NO_ERROR)
   {
      txsVal2Zero( &nv);                        // free any allocated memory
   }
   else
   {
      *pos   = s;
      *value = nv;
   }
   return (rc);
}                                               // end 'txsEvalAtomicExpr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script atomic-operand 'constant'
/*****************************************************************************/
static ULONG txsEvalConstant                    // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   char               *s  = string;             // ptr in number string
   TXTM                cname;                   // constant name

   while (isalnum(*s) || *s == '_')
   {
      s++;                                      // traverse constant name
   }
   TxCopy( cname, string, min((s - string), TXMAXTM -1) +1);

   if ((rc = txsGetConstant( cname, value)) == NO_ERROR)
   {
      if (ps->result == TXSEXPR_INITIAL)
      {
         ps->result = TXSEXPR_EXPRESSION;
      }
      VmDebug("CONSTANT  %lld\n", value->num);
      *pos   = s;
   }
   else                                         // not a valid constant
   {
      rc = TX_ERROR;
   }
   return (rc);
}                                               // end 'txsEvalConstant'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle a TX-script atomic-operand 'builtin-function' with up to 9 params
/*****************************************************************************/
static ULONG txsEvalBuiltinFunction             // RET   function result
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   TXSVALUE           *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSNUM_0;           // new result value
   char               *s  = string;             // ptr in number string
   char                vt = 0;                  // variable type
   char                op;                      // operator
   int                 ln;                      // token length
   int                 p;
   int                 q;
   TXTM                fname;
   TXLN                word;
   TXSVALUE           *param  = NULL;           // function parameters
   int                 pc     = 0;              // actual nr of parameters
   char               *at;
   BOOL                optpar = FALSE;          // parenthesis required

   while (isalnum(*s) || *s == '_')
   {
      s++;                                      // traverse function name
   }
   TxCopy( fname, string, min((s - string), TXMAXTM -1) +1);

   if ((strcasecmp( fname, "print") == 0) ||
       (strcasecmp( fname, "dump" ) == 0)  )     // 'func' has optional parenthesis
   {
      optpar = TRUE;
   }
   if (optpar || (*s == TXS_POP))               // valid function start
   {
      if (ps->result == TXSEXPR_INITIAL)
      {
         ps->result = TXSEXPR_EXPRESSION;
      }
      at = s;                                   // remember end of name
      if (*s == TXS_POP)
      {
         s++;
      }
      if ((param = (TxAlloc( TXS_MAXPARAMS, sizeof( TXSVALUE)))) != NULL)
      {
         //- Read special form of first parameter for specific functions
         //- that may work on a variable-name including a complete array/hash
         //- Now for undef, but push() and unshift() would use it too
         //- Note replaces 1st param only, rest are normal expressions

         if (strcasecmp( fname, "undef") == 0)  // undef variable, 1st param is name
         {
            if ((vt = txsNextToken( &s, &ln)), vt == '$' || vt == '@' || vt == '%')
            {
               s += ln;
               if (((rc = txsParseIdentifier( ps, &s, word, &param[0])) == NO_ERROR) &&
                                             (strlen( word) > 0))
               {
                  if ((op = txsNextToken(&s, &ln)) == ',')
                  {
                     s++;                       // more regular params follow
                  }
                  pc = 1;                       // first param is var-name in 'word'
               }                                // with possible index in param[0]
               else
               {
                  TxsExprError( ps, s, "Invalid %s variable name: '%s'",
                    (vt == '$') ? "scalar" : (vt == '@') ? "array" : "hash", string);
                  s -= ln;
                  rc = TX_SYNTAX_ERROR;
               }
            }
            else
            {
               strcpy( word, "");               // no variable name specified
            }                                   // (undef just returns 'undef' :-)
         }
         for ( p = pc,                op  = txsNextToken(&s, &ln);
              (p < TXS_MAXPARAMS) && (op != TXS_PCL) && (op != TXS_BCL) &&
                                     (*s != 0) && (rc == NO_ERROR);
               p++)
         {                                      // accept all except comma-operators
            if ((rc = txsEvalExpression(ps, FALSE, s, &s, &(param[p]))) == NO_ERROR)
            {
               pc++;
               if ((op = txsNextToken(&s, &ln)) == ',')
               {
                  s++;
               }
            }
         }
         if ((rc == NO_ERROR) && (optpar || (op == TXS_PCL))) // function should end correctly
         {
            if (*s == TXS_PCL)
            {
               s++;
            }
            if      (strcasecmp( fname, "undef"  ) == 0)
            {
               if ((rc = txsFuncParamsOk(ps, s, fname, pc, 0, 1)) == NO_ERROR)
               {
                  if ((pc == 1) && (!ps->syntax)) //- varname in 'word',
                  {                               //- index in param[0]
                     txsDelVariable( vt, word, &param[0]);
                  }
                  nv.defined = FALSE;           // always return undef :-)
                  VmDebug("UNDEF()\n");
               }
            }
            else if (strcasecmp( fname, "defined"  ) == 0) // ONE param, any type
            {
               if ((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 1)) == NO_ERROR)
               {
                  nv.num = (param[0].defined) ? 1 : 0;
                  VmDebug("DEFINED()\n");
               }
            }
            else if ((strcasecmp( fname, "getcwd"   ) == 0) || // NO params
                     (strcasecmp( fname, "canceled" ) == 0) ||
                     (strcasecmp( fname, "dump"     ) == 0)  )
            {
               if ((rc = txsFuncParamsOk(ps, s, fname, pc, 0, 0)) == NO_ERROR)
               {
                  if (strcasecmp( fname, "dump") == 0)
                  {
                     if (!ps->syntax)
                     {
                        txsDumpVarPool();
                     }
                     nv.mem = TXSVSIZE(0);
                     if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                     {
                        nv.len = 0;
                        strcpy( nv.str, "");
                        VmDebug("DUMP()\n");
                     }
                     else
                     {
                        rc = TX_ALLOC_ERROR;
                     }
                  }
                  else if (strcasecmp( fname, "canceled") == 0)
                  {
                     nv.num = (TxAbort()) ? 1LL : 0LL;
                  }
                  else
                  {
                     getcwd( word, TXMAXLN);
                     nv.mem = TXSVSIZE(strlen( word));
                     if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                     {
                        nv.len = strlen( word);
                        strcpy( nv.str,  word);
                        VmDebug("GETCWD()\n");
                     }
                     else
                     {
                        rc = TX_ALLOC_ERROR;
                     }
                  }
               }
            }
            else if (strcasecmp( fname, "print") == 0) // ZERO or more string params
            {
               if (!ps->syntax)
               {
                  for (p = 0; (p < pc) && (rc == NO_ERROR); p++)
                  {
                     if (txsVal2Str( &param[p], 0, "d") != NULL)
                     {
                        TxPrint( "%s", param[p].str);
                     }
                     else
                     {
                        rc = TX_ALLOC_ERROR;
                     }
                  }
                  TxPrint( "\n");
                  txsVal2Zero( &nv);            // do not return a value
                  VmDebug("PRINT\n");
               }
            }
            else if ((strcasecmp( fname, "max"  ) == 0) || // TWO or more numeric params
                     (strcasecmp( fname, "min"  ) == 0)  )
            {
               if ((rc = txsFuncParamsOk(ps, s, fname, pc, 2, 0)) == NO_ERROR)
               {
                  nv = param[0];
                  txsVal2Num( &nv);

                  for (p = 1; p < pc; p++)
                  {
                     txsVal2Num( &param[p]);
                     if (strcasecmp( fname, "max") == 0)
                     {
                        if (param[p].num > nv.num)
                        {
                           nv.num = param[p].num;
                        }
                        VmDebug("MAX()\n");
                     }
                     else
                     {
                        if (param[p].num < nv.num)
                        {
                           nv.num = param[p].num;
                        }
                        VmDebug("MIN()\n");
                     }
                  }
               }
            }
            else if ((strcasecmp( fname, "abs"    ) == 0) || // ONE numeric param
                     (strcasecmp( fname, "b32"    ) == 0) ||
                     (strcasecmp( fname, "chr"    ) == 0) ||
                     (strcasecmp( fname, "sleep"  ) == 0)  )
            {
               if ((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 1)) == NO_ERROR)
               {
                  txsVal2Num( &param[0]);
                  if      (strcasecmp( fname, "chr") == 0)
                  {
                     nv.mem = TXSVSIZE(1);
                     if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                     {
                        nv.len    = 1;
                        nv.str[0] = (char) param[0].num & 0xff;
                     }
                     else
                     {
                        TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                        rc = TX_ALLOC_ERROR;
                     }
                  }
                  else if (strcasecmp( fname, "b32") == 0)
                  {
                     nv.num = param[0].num & 0xffffffffLL;
                  }
                  else if (strcasecmp( fname, "abs") == 0)
                  {
                     nv.num = (param[0].num < 0) ? -param[0].num : param[0].num;
                  }
                  else
                  {
                     TxSleep((ULONG) param[0].num);
                     nv.num = 0LL;
                  }
                  VmDebug("%s()\n", TxStrToUpper( fname));
               }
            }
            else if ((strcasecmp( fname, "i2dec"   ) == 0) || // ONE numeric, 1 optional
                     (strcasecmp( fname, "i2hex"   ) == 0) ||
                     (strcasecmp( fname, "sec2kib" ) == 0) ||
                     (strcasecmp( fname, "sec2mib" ) == 0) ||
                     (strcasecmp( fname, "sec2gib" ) == 0)  )
            {
               if ((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 2)) == NO_ERROR)
               {
                  int len =   0;
                  int bps = 512;

                  nv = param[0];
                  txsVal2Num( &nv);
                  if (strncasecmp( fname, "sec2", 4) == 0) // numeric output
                  {
                     if (pc > 1)
                     {
                        bps = (int) txsVal2Num( &param[1]); // sectorsize
                     }
                     nv.num *= bps;
                     nv.num /= 1024;
                     if (strcasecmp( fname, "sec2mib" ) == 0)
                     {
                        nv.num /= 1024;
                     }
                     else if (strcasecmp( fname, "sec2gib" ) == 0)
                     {
                        nv.num /= (1024 * 1024);
                     }
                  }
                  else                          // simple hex or dec, string output
                  {
                     len = (int) txsVal2Num( &param[1]); // requested length or 0
                     if (txsVal2Str( &nv, len, fname +2) == NULL)
                     {
                        TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                        rc = TX_ALLOC_ERROR;
                     }
                  }
                  VmDebug("%s()\n", TxStrToUpper( fname));
               }
            }
            else if ((strcasecmp( fname, "ord"        ) == 0) || // ONE string param
                     (strcasecmp( fname, "length"     ) == 0) ||
                     (strcasecmp( fname, "exists"     ) == 0) ||
                     (strcasecmp( fname, "makedir"    ) == 0) ||
                     (strcasecmp( fname, "confirmed"  ) == 0) ||
                     (strcasecmp( fname, "message"    ) == 0) ||
                     (strcasecmp( fname, "prompt"     ) == 0) ||
                     (strcasecmp( fname, "drivespace" ) == 0) ||
                     (strcasecmp( fname, "h2int"      ) == 0) ||
                     (strcasecmp( fname, "h2asc"      ) == 0) ||
                     (strcasecmp( fname, "b2asc"      ) == 0) ||
                     (strcasecmp( fname, "reverse"    ) == 0) ||
                     (strcasecmp( fname, "uc"         ) == 0) ||
                     (strcasecmp( fname, "lc"         ) == 0) ||
                     (strcasecmp( fname, "fnbase"     ) == 0) ||
                     (strcasecmp( fname, "fnpath"     ) == 0) ||
                     (strcasecmp( fname, "fnfile"     ) == 0) ||
                     (strcasecmp( fname, "drivefs"    ) == 0) ||
                     (strcasecmp( fname, "drivelabel" ) == 0)  )
            {
               if (((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 1)) == NO_ERROR) &&
                   (!ps->syntax))
               {
                  if (txsVal2Str( &param[0], 0, "d") != NULL)
                  {
                     if      (strcasecmp( fname, "ord") == 0)
                     {
                        nv.num = param[0].str[0];
                     }
                     else if (strcasecmp( fname, "length") == 0)
                     {
                        nv.num = strlen(param[0].str);
                     }
                     else if (strcasecmp( fname, "exists") == 0)
                     {
                        nv.num = (TxFileExists(param[0].str)) ? 1LL : 0LL;
                     }
                     else if (strcasecmp( fname, "drivespace") == 0)
                     {
                        ULONG           sects;
                        ULONG           total;
                        USHORT          bps;

                        if (TxFsSpace(param[0].str, &sects, &total, &bps) == NO_ERROR)
                        {
                           nv.num = ((LLONG) sects * bps) / 1024;
                        }
                     }
                     else if (strcasecmp( fname, "makedir") == 0)
                     {
                        nv.num = (TxMakePath(param[0].str) == NO_ERROR) ? 1LL : 0LL;
                     }
                     else if (strcasecmp( fname, "message") == 0)
                     {
                        nv.num = (TxMessage(TRUE, 0, "%s", param[0].str)) ? 1LL : 0LL;
                     }
                     else if (strcasecmp( fname, "confirmed") == 0)
                     {
                        nv.num = (TxConfirm(0, "%s", param[0].str)) ? 1LL : 0LL;
                     }
                     else if (strcasecmp( fname, "h2int") == 0)
                     {
                        nv.num = txHexStrIntelReverse(param[0].str);
                     }
                     else                       // returning a string
                     {
                        nv = param[0];          // steal the string from param
                        param[0].str = NULL;
                        param[0].len = 0;
                        if      (strcasecmp( fname, "h2asc") == 0)
                        {
                           txHexStr2Ascii(  nv.str);
                           nv.len = strlen( nv.str);
                        }
                        else if (strcasecmp( fname, "b2asc") == 0)
                        {
                           for (p = 0; p < nv.len; p++)
                           {
                              nv.str[p] = TxPrintable( nv.str[p]);
                           }
                        }
                        else if (strcasecmp( fname, "prompt") == 0)
                        {
                           char *text = strdup( nv.str);
                           strcpy( nv.str, "");

                           TxPrompt(0, nv.mem -1, nv.str, "%s", text);
                           nv.len = strlen( nv.str);
                           free( text);
                        }
                        else if (strcasecmp( fname, "reverse") == 0)
                        {
                           #if defined (DARWIN)
                              //- to be refined
                           #else
                              _strrev(nv.str);
                           #endif
                        }
                        else if (strcasecmp( fname, "uc"     ) == 0)
                        {
                           TxStrToUpper(nv.str);
                        }
                        else if (strcasecmp( fname, "lc"     ) == 0)
                        {
                           TxStrToLower(nv.str);
                        }
                        else if (strcasecmp( fname, "fnbase" ) == 0)
                        {
                           TxStripExtension(nv.str);
                           nv.len = strlen( nv.str);
                        }
                        else if (strcasecmp( fname, "fnpath" ) == 0)
                        {
                           TxStripBaseName( nv.str);
                           nv.len = strlen( nv.str);
                        }
                        else if (strcasecmp( fname, "fnfile" ) == 0)
                        {
                           if ((at = TxGetBaseName(nv.str)) != nv.str)
                           {
                              memmove( nv.str, at, strlen(at) +1);
                              nv.len = strlen( nv.str);
                           }
                        }
                        else if (strcasecmp( fname, "drivefs" ) == 0)
                        {
                           TxCopy(   word, nv.str, 2);
                           TxFsType( word, nv.str, NULL);
                           nv.len = strlen( nv.str);
                        }
                        else                    // drivelabel
                        {
                           TxCopy(          word, nv.str, 2);
                           TxFsVolumeLabel( word, nv.str);
                           nv.len = strlen( nv.str);
                        }
                     }
                     VmDebug("%s()\n", TxStrToUpper( fname));
                  }
                  else
                  {
                     TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                     rc = TX_ALLOC_ERROR;
                  }
               }
            }
            else if (strcasecmp( fname, "b2int")   == 0) // ONE string, 1 optional
            {
               if (txsVal2Str( &param[0], 0, "d") != NULL)
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 2)) == NO_ERROR)
                  {
                     int bytes = (pc == 1) ? 4 : txsVal2Num( &param[1]);

                     nv.num = txBinStrIntelReverse(param[0].str, bytes);
                     VmDebug("%s()\n", TxStrToUpper( fname));
                  }
               }
               else
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                  rc = TX_ALLOC_ERROR;
               }
            }
            else if (strcasecmp( fname, "drives") == 0) // 1 optional
            {
               if ((rc = txsFuncParamsOk(ps, s, fname, pc, 0, 1)) == NO_ERROR)
               {
                  ULONG opt = (pc) ? (ULONG) txsVal2Num( &param[0]) : TXFSV_ALL;

                  if (txsVal2Str( &nv, 0, "d") != NULL)
                  {
                     TxFsVolumes( opt, nv.str);
                     nv.len = strlen(  nv.str);
                     VmDebug("DRIVES()\n");
                  }
                  else
                  {
                     TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                     rc = TX_ALLOC_ERROR;
                  }
               }
            }
            else if ((strcasecmp( fname, "strip")   == 0) || // ONE string, 2 optionals
                     (strcasecmp( fname, "replace") == 0)  )
            {
               if (txsVal2Str( &param[0], 0, "d") != NULL)
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 3)) == NO_ERROR)
                  {
                     char  p2 = ' ';
                     char  p3 = (strcasecmp( fname, "strip")) ? '-' : ' ';

                     nv = param[0];             // steal the string from param
                     param[0].str = NULL;

                     if (pc > 1)                // specified strip-char lead
                     {                          // note: alloc error ignored
                        if (txsVal2Str( &param[1], 0, "d") != NULL)
                        {
                           p2 = param[1].str[0];
                        }
                        if (pc > 2)             // specified strip-char trail
                        {
                           if (txsVal2Str( &param[2], 0, "d") != NULL)
                           {
                              p3 = param[2].str[0];
                           }
                        }
                        else
                        {                       // NO trail stripping (strip)
                           p3 = 0;              // or replace by str terminator
                        }
                     }
                     if (strcasecmp( fname, "strip") == 0)
                     {
                        TxStrip( nv.str, nv.str, p2, p3);
                     }
                     else
                     {
                        TxRepl( nv.str, p2, p3);
                     }
                     nv.len = strlen(  nv.str);
                     VmDebug("%s()\n", TxStrToUpper( fname));
                  }
               }
               else
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                  rc = TX_ALLOC_ERROR;
               }
            }
            else if (strcasecmp( fname, "filext")    == 0) // TWO strings
            {
               if ((txsVal2Str( &param[0], 0, "d") != NULL) &&
                   (txsVal2Str( &param[1], 0, "d") != NULL)  )
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 2, 2)) == NO_ERROR)
                  {
                     nv = param[0];             // steal the string from param
                     param[0].str = NULL;
                     ln = nv.len + param[1].len +2;
                     if (ln > nv.mem)
                     {
                        char *lv = nv.str;      // original left-hand value
                        nv.mem = TXSVSIZE(ln);
                        if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                        {
                           strcpy( nv.str, lv);
                           TxFreeMem( lv);      // free 'old' string memory
                        }
                        else
                        {
                           rc = TX_ALLOC_ERROR;
                        }
                     }
                     if (rc == NO_ERROR)
                     {
                        TxFnameExtension( nv.str, param[1].str);
                        nv.len = strlen(  nv.str);
                     }
                  }
                  VmDebug("%s()\n", TxStrToUpper( fname));
               }
               else
               {
                  rc = TX_ALLOC_ERROR;
               }
               if (rc != NO_ERROR)
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
               }
            }
            else if ((strcasecmp( fname, "fileselect") == 0) || // ONE string, 2 opt string
                     (strcasecmp( fname, "filesaveas") == 0)  )
            {
               if (txsVal2Str( &param[0], 0, "d") != NULL)
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 3)) == NO_ERROR)
                  {
                     nv.mem = TXSVSIZE(TXMAXLN); // make room for long filename
                     if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                     {
                       #if defined (USEWINDOWING)
                        if (txwa->desktop != NULL) // there is a desktop
                        {
                           char *wild = txsVal2Str( &param[1], 0, "d"); // NULL or param value
                           char *path = txsVal2Str( &param[2], 0, "d"); // NULL or param value

                           if (strcasecmp( fname, "fileselect") == 0)
                           {
                              txwOpenFileDialog(   wild,         //- selection wildcard
                                                   path, NULL,   //- path, no default fname
                                                   0, NULL,      //- no help, ni widgets
                                                   param[0].str, //- Dialog title (prompt)
                                                   nv.str);      //- Output path+filename
                           }
                           else
                           {
                              txwSaveAsFileDialog( wild,         //- selection wildcard
                                                   path, NULL,   //- path, no default fname
                                                   0, NULL,      //- no help, ni widgets
                                                   param[0].str, //- Dialog title (prompt)
                                                   nv.str);      //- Output path+filename
                           }
                           nv.len = strlen( nv.str);
                        }
                        else                   // revert to simple prompt
                       #endif
                        {
                           TxPrompt(0, nv.mem -1, nv.str, "%s", param[0].str);
                           nv.len = strlen( nv.str);
                        }
                     }
                     else
                     {
                        TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                        rc = TX_ALLOC_ERROR;
                     }
                  }
                  VmDebug("%s()\n", TxStrToUpper( fname));
               }
               else
               {
                  rc = TX_ALLOC_ERROR;
               }
               if (rc != NO_ERROR)
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
               }
            }
            else if (strcasecmp( fname, "dirselect") == 0) // ONE string, opt string
            {
               if (txsVal2Str( &param[0], 0, "d") != NULL)
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 1, 2)) == NO_ERROR)
                  {
                     nv.mem = TXSVSIZE(TXMAXLN); // make room for long filename
                     if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                     {
                       #if defined (USEWINDOWING)
                        if (txwa->desktop != NULL) // there is a desktop
                        {
                           char *path = txsVal2Str( &param[1], 0, "d"); // NULL or param value

                           txwSelDirFileDialog( path,         //- optional path
                                                0, NULL,      //- no help, no widgets
                                                param[0].str, //- Dialog title (prompt)
                                                nv.str);      //- Output path+filename
                           nv.len = strlen( nv.str);
                        }
                        else                   // revert to simple prompt
                       #endif
                        {
                           TxPrompt(0, nv.mem -1, nv.str, "%s", param[0].str);
                           nv.len = strlen( nv.str);
                        }
                     }
                     else
                     {
                        TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                        rc = TX_ALLOC_ERROR;
                     }
                  }
                  VmDebug("%s()\n", TxStrToUpper( fname));
               }
               else
               {
                  rc = TX_ALLOC_ERROR;
               }
               if (rc != NO_ERROR)
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
               }
            }
            else if ((strcasecmp( fname, "right" )   == 0) || // ONE string + num + opt
                     (strcasecmp( fname, "left"  )   == 0)  )
            {
               if (txsVal2Str( &param[0], 0, "d") != NULL)
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 2, 3)) == NO_ERROR)
                  {
                     int  len = txsVal2Num( &param[1]);
                     char p3  = ' ';            // default padding char space

                     if (pc > 2)                // specified padding-char
                     {
                        if (txsVal2Str( &param[2], 0, "d") != NULL)
                        {
                           p3 = param[2].str[0];
                        }
                     }
                     nv.mem = TXSVSIZE(len);
                     if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
                     {
                        memset( nv.str, p3, len);
                        if (strcasecmp( fname, "left") == 0)
                        {
                           memcpy( nv.str, param[0].str, min( param[0].len, len));
                        }
                        else
                        {
                           if (len > param[0].len)
                           {
                              p = len - param[0].len;
                              memcpy( nv.str + p, param[0].str, param[0].len);
                           }
                           else
                           {
                              p = param[0].len - len;
                              memcpy( nv.str, param[0].str + p, len);
                           }
                        }
                        nv.len = len;
                     }
                     else
                     {
                        TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                        rc = TX_ALLOC_ERROR;
                     }
                     nv.len = strlen(  nv.str);
                     VmDebug("%s()\n", TxStrToUpper( fname));
                  }
               }
               else
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                  rc = TX_ALLOC_ERROR;
               }
            }
            else if ((strcasecmp( fname, "index")    == 0) || // TWO strings + opt
                     (strcasecmp( fname, "rindex")   == 0)  )
            {
               if ((txsVal2Str( &param[0], 0, "d") != NULL) &&
                   (txsVal2Str( &param[1], 0, "d") != NULL)  )
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 2, 3)) == NO_ERROR)
                  {
                     int pos = txsVal2Num( &param[2]); // 0-based

                     p = param[0].len;
                     if (pos < p)               // start within string
                     {
                        if (strcasecmp( fname, "rindex" ) == 0)
                        {
                           q = (int) ((pos) ? (p - pos) : 0);

                           #if defined (DARWIN)
                              //- to be refined
                           #else
                              _strrev( param[0].str); // reverse string
                              _strrev( param[1].str); // and substring
                           #endif

                           if ((at = TxMemStr( param[0].str  + q,
                                               param[1].str, p - q)) != NULL)
                           {
                              nv.num = p - ((at - param[0].str) + param[1].len);
                           }
                           else
                           {
                              nv.num = -1;
                           }
                        }
                        else
                        {
                           q = pos;
                           if ((at = TxMemStr( param[0].str  + q,
                                               param[1].str, p - q)) != NULL)
                           {
                              nv.num = at - param[0].str;
                           }
                           else
                           {
                              nv.num = -1;
                           }
                        }
                     }
                  }
                  VmDebug("%s()\n", TxStrToUpper( fname));
               }
               else
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                  rc = TX_ALLOC_ERROR;
               }
            }
            else if (strcasecmp( fname, "substr")    == 0) // ONE str, ONE num, 1 optional
            {
               if (txsVal2Str( &param[0], 0, "d") != NULL)
               {
                  if ((rc = txsFuncParamsOk(ps, s, fname, pc, 2, 3)) == NO_ERROR)
                  {
                     int pos = txsVal2Num( &param[1]); // start pos

                     nv  = param[0];            // steal the string from param
                     param[0].str = NULL;
                     p   = nv.len;
                     if (pos < 0)               // pos from END of string
                     {
                        pos = max( 0, p + pos);
                     }
                     ln  = (pc < 3) ? p : (int) txsVal2Num( &param[2]);
                     if (ln  < 0)               // pos from END of string
                     {
                        ln  = max( 0, p + ln);
                     }
                     pos = min( pos,  p);
                     ln  = min( ln,   p - pos);

                     memmove( nv.str, nv.str + pos, ln);
                     nv.str[  ln] = 0;
                     nv.len = ln;
                  }
                  VmDebug("%s() : '%-.*s'\n", TxStrToUpper( fname), 60, nv.str);
               }
               else
               {
                  TxsExprError( ps, s, "Memory allocation error in '%s()'", fname);
                  rc = TX_ALLOC_ERROR;
               }
            }
            else
            {
               s = at;                          // restore function name position
               TxsExprError( ps, s, "Unknown function '%s()'", fname);
               rc = TX_SYNTAX_ERROR;
            }
         }
         else if (rc == NO_ERROR)
         {
            TxsExprError( ps, s, "Missing ',' or ')' in function '%s()'", fname);
            rc = TX_SYNTAX_ERROR;
         }
         for (p = 0; p < TXS_MAXPARAMS; p++)
         {
            txsVal2Zero( &(param[p]));                  // free any allocated memory
         }
         TxFreeMem( param);
      }
      else
      {
         TxsExprError( ps, s, "Memory allocation error for '%s()' parameters", fname);
         rc = TX_ALLOC_ERROR;
      }
   }
   else
   {
      if (strlen(fname))                        // something left ?
      {
         TxsExprError( ps, s, "Invalid operand '%s'", fname);
      }
      else
      {
         s++;                                   // skip one unrecognized char
         TxsExprError( ps, s, "Missing operand, unexpected end of expression ...");
      }
      rc = TX_ERROR;
   }
   *pos   = s;
   *value = nv;
   return (rc);
}                                               // end 'txsEvalBuiltinFunction'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Validate parameter count for builtin functions, set error message if not OK
/*****************************************************************************/
static ULONG txsFuncParamsOk
(
   TXSPARSESTATE      *ps,                      // INOUT parse state
   char               *s,                       // IN    position
   char               *name,                    // IN    function name
   int                 count,                   // IN    number of parameters
   int                 minimum,                 // IN    minimum number
   int                 maximum                  // IN    maximum number or 0
)
{
   ULONG               rc = NO_ERROR;           // function return

   if (count < minimum)
   {
      TxsExprError( ps, s - strlen(name), "Not enough parameters for '%s()'", name);
      rc = TX_SYNTAX_ERROR;
   }
   else if (maximum && (count > maximum))
   {
      TxsExprError( ps, s - strlen(name), "Too many parameters for '%s()'", name);
      rc = TX_SYNTAX_ERROR;
   }
   return (rc);
}                                               // end 'txsFuncParamsOk'
/*---------------------------------------------------------------------------*/



/*****************************************************************************/
// Skip white-space in input string, and return next Token character
// JvW 03-01-2011: Only accept UPPERCASE letters for operators to avoid
//                 easy to make mistakes with implicit string concatenation
/*****************************************************************************/
char txsNextToken                               // RET   next token in input
(
   char              **pos,                     // INOUT position in string
   int                *len                      // OUT   length of token
)
{
   char               *s  = *pos;               // start position for parse
   int                 tl = 1;                  // token length
   char                token;                   // token character

   while (isspace(*s))
   {
      s++;                                      // skip white-space
   }
   *pos  = s;                                   // assign new position
   token = *s++;

   switch (token)                               // first token char
   {
     case '<':
        switch (*s)                            // second token char
        {
           case '<': token = 'S'; tl++; break; // bit shift left
           case '=': token = 'L'; tl++; break; // less or equal
           default :                    break;
        }
        break;

     case '>':
        switch (*s)                            // second token char
        {
           case '>': token = 'R'; tl++; break; // bit shift right
           case '=': token = 'G'; tl++; break; // greater or equal
           default :                    break;
        }
        break;

     case '=':
        switch (*s)                            // second token char
        {
           case '=':
              switch (*(s+1))
              {
                 case '=': token = 'I'; tl += 2; break; // identity, exact equal
                 default : token = 'E'; tl += 1; break; // numeric equal
              }
              break;

           default :                    break;
        }
        break;

     case '!':
        switch (*s)                            // second token char
        {
           case '=':
              switch (*(s+1))
              {
                 case '=': token = 'X'; tl += 2; break; // identity, not equal
                 default : token = 'N'; tl += 1; break; // numeric not equal
              }
              break;

           default :                    break;
        }
        break;

     case '|':
        switch (*s)                            // second token char
        {
           case '|': token = 'O'; tl++; break; // logical or
           default :                    break;
        }
        break;

     case '+':
        switch (*s)                            // second token char
        {
           case '+': token = 'i'; tl++; break; // auto increment
           default :                    break;
        }
        break;

     case '-':
        switch (*s)                            // second token char
        {
           case '-': token = 'd'; tl++; break; // auto decrement
           default :                    break;
        }
        break;

     case '&':
        switch (*s)                            // second token char
        {
           case '&': token = 'A'; tl++; break; // logical and
           default :                    break;
        }
        break;

     case 'A':
        switch (*s)                            // second token char
        {
           case 'N':
              switch (*(s+1))
              {
                 case 'D': token = 'a'; tl += 2; break; // logical AND (Perl)
                 default : token = TXS_NONE;     break;
              }
              break;

           default : token = TXS_NONE;  break;
        }
        break;

     case 'E':
        switch (*s)                            // second token char
        {
           case 'Q': token = 'e'; tl++; break; // loose string compare
           default : token = TXS_NONE;  break;
        }
        break;

     case 'G':
        switch (*s)                            // second token char
        {
           case 'E': token = 'g'; tl++; break; // greater or equal string
           case 'T': token = 'B'; tl++; break; // greater than     string
           default : token = TXS_NONE;  break;
        }
        break;

     case 'L':
        switch (*s)                            // second token char
        {
           case 'E': token = 'l'; tl++; break; // less or equal string
           case 'T': token = 'K'; tl++; break; // less than     string
           default : token = TXS_NONE;  break;
        }
        break;

     case 'N':
        switch (*s)                            // second token char
        {
           case 'O':
              switch (*(s+1))
              {
                 case 'T': token = 't'; tl += 2; break; // logical NOT (Perl)
                 default : token = TXS_NONE;     break;
              }
              break;

           case 'E': token = 'n'; tl++; break; // not equal string
           default : token = TXS_NONE;  break;
        }
        break;

     case 'O':
        switch (*s)                            // second token char
        {
           case 'R': token = 'o'; tl++; break; // logical OR (Perl)
           default : token = TXS_NONE;  break;
        }
        break;

     case 'X':       token = 'x';       break;  // string-replicate

     default:
        if (isalnum( token))                    // avoid random letters
        {                                       // to be interpreted as
                     token = TXS_NONE;          // as a valid operator
        }
        break;
   }
   *len = tl;
   return (token);                              // next token character
}                                               // end 'txsNextToken'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine boolean value from a TXS value, convert value in-situ too
/*****************************************************************************/
BOOL txsVal2Bool                                // RET   boolean value
(
   TXSVALUE           *value                    // INOUT TXS value
)
{
   BOOL                rc = FALSE;              // function return

   ENTER();

   if (value->str != NULL)                      // string value
   {
      rc = (strlen(value->str) && !strcmp(value->str, "0"));
      if (value->mem)                           // dynamic allocated ?
      {
         TxFreeMem( value->str);                // free the string memory
         value->len = 0;
         value->mem = 0;
      }
      else                                      // static or 'stolen'
      {
         value->str = NULL;
      }
   }
   else if (value->defined)                     // numerical
   {
      rc = (value->num != 0);
   }
   value->num     = (rc) ? 1 : 0;
   value->defined = TRUE;
   BRETURN (rc);
}                                               // end 'txsVal2Bool'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine numeric value from a TXS value, convert value in-situ too
/*****************************************************************************/
LLONG txsVal2Num                                // RET   numeric value
(
   TXSVALUE           *value                    // INOUT TXS value
)
{
   char               *s;                       // ptr in number string

   ENTER();

   if (value->str != NULL)                      // string value
   {
      txsParseLlongNumber( value->str, &s, &value->num);
      if (value->mem)                           // dynamic allocated ?
      {
         TxFreeMem( value->str);                // free the string memory
         value->len = 0;
         value->mem = 0;
      }
      else                                      // static or 'stolen'
      {
         value->str = NULL;
      }
   }
   else if (!value->defined)
   {
      value->num = 0;
   }
   value->defined = TRUE;
   RETURN (value->num);
}                                               // end 'txsVal2Num'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine string value from a TXS value, convert value in-situ too
/*****************************************************************************/
char *txsVal2Str                                // RET   string value or NULL
(
   TXSVALUE           *value,                   // INOUT TXS value
   int                 digits,                  // IN    number of digits, or 0
   char               *format                   // IN    Hex, Dec
)
{
   BOOL                hex = ((*format == 'h') || (*format == 'H'));

   ENTER();

   if (value->str == NULL)                      // numeric value
   {
      value->mem = TXSVSIZE(20);
      if ((value->str = TxAlloc( 1, value->mem)) != NULL)
      {
         if (value->defined)                    // undef results in empty string
         {
            if (digits)
            {
               sprintf( value->str, (hex) ? "0x%0*llX" : "%0*lld", digits, value->num);
            }
            else
            {
               sprintf( value->str, (hex) ? "0x%llX"   : "%lld", value->num);
            }
         }
         value->len = strlen( value->str);
         value->num = 0;
      }
   }
   RETURN (value->str);
}                                               // end 'txsVal2Str'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Initialize a TXS value to numeric ZERO, freeing any string memory
/*****************************************************************************/
void txsVal2Zero
(
   TXSVALUE           *value                    // INOUT TXS value
)
{
   ENTER();

   if (value)                                   // allow NULL pointer too
   {
      if (value->str != NULL)                   // string value
      {
         TxFreeMem( value->str);                // free the string memory
      }
      value->len = 0;
      value->mem = 0;
      value->num = 0;
      value->defined = TRUE;                    // Note: was FALSE (STREQ bug)
   }
   VRETURN ();
}                                               // end 'txsVal2Zero'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse variable or function identifier, including optional [] or {} index
/*****************************************************************************/
static ULONG txsParseIdentifier                 // RET   length of identifier
(
   TXSPARSESTATE      *ps,                      // INOUT parse state, or NULL
   char              **pp,                      // INOUT parse position
   char               *ident,                   // OUT   identifier
   TXSVALUE           *index                    // OUT   index or NULL
)                                               //       undef means no index
{
   ULONG               rc = NO_ERROR;
   TXSVALUE            nv = TXSUNDEF;           // index value
   int                 ln;
   char                br;                      // bracket
   char               *start = *pp;
   char               *s     = start;

   ENTER();
   TRACES(("in:'%s'\n", *pp));

   if (isalpha(*s) || *s == '_')
   {
      s++;
      while (isalnum(*s) || *s == '_')
      {
         s++;                                   // traverse variable name
      }
      ln = min( (s - start), TXMAXTM -1) +1;
      TxCopy( ident, start, ln);

      if (index)
      {
         if ((br = txsNextToken( &s, &ln)), br == '[' || br == TXS_BOP)
         {
            s += ln;                            // skip open bracket
            br = (br == '[') ? ']' : TXS_BCL;   // set closing bracket
            if ((rc = txsEvalExpression(ps, TRUE, s, &s, &nv)) == NO_ERROR)
            {
               if (txsNextToken( &s, &ln) == br)
               {
                  s += ln;                      // skip closing bracket
                  if (br == ']')                // array, numeric index
                  {
                     txsVal2Num( &nv);
                  }
                  else                          // hash, string index
                  {
                     if (txsVal2Str( &nv, 0, "d") == NULL)
                     {
                        TxsExprError( ps, s, "Memory allocation error in var '%s{}'", ident);
                        rc = TX_ALLOC_ERROR;
                     }
                  }
               }
               else
               {
                  TxsExprError( ps, s, "Missing '%c' in array variable: '%s'", br, start);
                  rc = TX_SYNTAX_ERROR;
               }
            }
         }
         *index = nv;
      }
      *pp = s;
      TRACES(("name:'%s', rest:'%s'\n", ident, s));
   }
   else                                         // no identifier found
   {
      rc = TX_FAILED;
   }
   RETURN (rc);
}                                               // end 'txsParseIdentifier'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get long-integer representation for number-string
/*****************************************************************************/
ULONG txsParseLlongNumber                       // RET   function result
(
   char               *string,                  // IN    input string
   char              **pos,                     // OUT   position in string
   LLONG              *value                    // OUT   result value
)
{
   ULONG               rc = NO_ERROR;           // function return
   LLONG               nv = 0;                  // new result value
   double              cv = (double) 0;         // range check value
   char               *s  = string;             // ptr in number string
   char               *at_begin;                // start of number
   long                base;                    // number base
   long                digit;                   // single digit value
   double              lim = (double) LLONG_MAX; // limit-check value
   BOOL                negative = FALSE;        // negative value

   while ((*s == ' ') || (*s == '\t') || (*s == '\n'))
   {
      s++;                                      // skip white-space
   }
   if (*s == '-')                               // negative value
   {
      s++;                                      // skip minus sign
      negative = TRUE;
   }
   at_begin = s;                                // start of real digit-string

   while (*s == '0')
   {
      s++;                                      // skip leading zeroes
   }
   switch (*s)
   {
      case 'x':
         s++;
         base = 16;
         at_begin = s;                          // start of real digit-string
         break;

      case 'o':
         s++;
         base = 8;
         at_begin = s;                          // start of real digit-string
         break;

      case 'b':
         s++;
         base = 2;
         at_begin = s;                          // start of real digit-string
         break;

      default:
         base = 10;
         break;
   }

   while ((rc == NO_ERROR) && (txsGetDigit(toupper(*s), base -1, &digit)))
   {
      s++;
      cv = cv * base + digit;
      if (cv <= lim)
      {
         nv = nv * base + digit;
      }
      else                                      // overflow
      {
         rc = TX_FAILED;
      }
   }

   if (rc == NO_ERROR)
   {
      if (s == at_begin)                        // no digits read
      {
         rc = TX_ERROR;
      }
      else
      {
         if (negative)
         {
            *value = -nv;
         }
         else
         {
            *value = nv;
         }
         *pos   = s;
      }
   }
   return (rc);
}                                               // end 'txsParseLlongNumber'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get and convert next digit
/*****************************************************************************/
static BOOL txsGetDigit
(
   char                ch,                      // IN    digit character
   long                max,                     // IN    maximum digit value
   long               *val                      // OUT   digit value
)
{
   BOOL                rc = TRUE;               // function return
   long                dv;                      // digit value

   if      ((ch >= '0') && (ch <= '9'))
   {
      dv = (long) (ch - '0');
   }
   else if ((ch >= 'A') && (ch <= 'F'))
   {
      dv = (long) (ch - 'A' +10);
   }
   else
   {
      rc = FALSE;                               // not a digit
   }
   if (rc == TRUE)
   {
      if (dv <= max)                            // not exceeding maximum
      {
         *val = dv;
      }
      else
      {
         rc = FALSE;
      }
   }
   return (rc);
}                                               // end 'txsGetDigit'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return little-endian LLONG representation for binary-string
/*****************************************************************************/
static LLONG txBinStrIntelReverse
(
   char               *bin,                     // IN    binary string l-endian
   int                 len                      // IN    number of bytes
)
{
   LLONG               rc = 0;                  // function return
   int                 bytes = min( 8, len);
   int                 b;
   char               *s;


   ENTER();

   for (b = 0, s = bin; b < bytes; b++, s++)
   {
      rc += (*s << (b * 8));
   }
   RETURN (rc);
}                                               // end 'txBinStrIntelReverse'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return little-endian LLONG representation for hex-string
/*****************************************************************************/
static LLONG txHexStrIntelReverse
(
   char               *hex                      // IN    hexadecimal string in
)                                               //       little-endian order
{
   LLONG               rc = 0;                  // function return
   int                 bytes = min( 8, strlen(hex) /2);
   int                 b;
   char               *s;
   char                byte[4];
   int                 value;


   ENTER();

   for (b = 0, s = hex; b < bytes; b++)
   {
      byte[0] = *s++;
      byte[1] = *s++;
      byte[2] = 0;
      sscanf( byte, "%x", &value);
      rc += (value << (b * 8));
   }
   RETURN (rc);
}                                               // end 'txHexStrIntelReverse'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert hex-string to printable ASCII string, in same string space!
/*****************************************************************************/
static void txHexStr2Ascii
(
   char               *hex                      // INOUT hex/ASCII string
)
{
   int                 bytes = strlen(hex) /2;
   int                 b;
   char               *s;
   char               *a;
   char                byte[4];
   int                 value;


   ENTER();

   for (b = 0, s = hex, a = hex; b < bytes; b++)
   {
      byte[0] = *s++;
      byte[1] = *s++;
      byte[2] = 0;
      sscanf( byte, "%x", &value);
      *a++ = TxPrintable(value);
   }
   *a = 0;                                      // terminate ASCII string
   VRETURN ();
}                                               // end 'txHexStr2Ascii'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Handle expression error message, printf-like interface
/*****************************************************************************/
void TxsExprError
(
   TXSPARSESTATE      *ps,                      // INOUT parse state, or NULL
   char               *at,                      // IN    location in ps->start
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   va_list             varargs;
   TXLN                text;

   if (ps != NULL)
   {
      va_start( varargs,  fmt);
      if (strlen(ps->msg) == 0)                 // don't overwrite any messages,
      {                                         // first is most to-the-point!
         ULONG            lnr = ps->linenr;     // linenr for error (start)
         int              col;                  // column position of error
         char            *s;

         for (s = ps->start, col = 1; *s && (s < at); s++, col++)
         {
            if (*s == '\n')                     // another line ...
            {
               lnr++;
               col = 1;
            }
         }
         vsprintf( text, fmt, varargs);
         sprintf(  ps->msg, "Line %lu, ERROR near column %d: %.*s",
                   lnr, col, (TXMAXTM - 35), text);
      }
      if (ps->result != TXSEXPR_INITIAL)
      {
         ps->result = TXSEXPR_SYNTAX_ERR;
      }
      va_end(   varargs);
   }
}                                               // end 'TxsExprError'
/*---------------------------------------------------------------------------*/

