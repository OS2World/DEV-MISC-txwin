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
// TX script execution with subcommand-handler callback and TxPrint routing
//
// Author: J. van Wijk
//
// Developed for LPT/DFS utilities
//

#include <txlib.h>                              // TX library interface
#include <txwpriv.h>                            // TX library private interface

#define TXSC_HELPREG        (TXWH_SYSTEM_SECTION_BASE + 1200)
#define TXSC_HELPDIALOG     (TXSC_HELPREG + 0)
#define TXSC_HELPFUNCTIONS  (TXSC_HELPREG + 100)
#define TXSC_HELPCONTROL    (TXSC_HELPREG + 200)

#define TXSC_PRAGMA         ";;"
#define TXSC_P_PARDEF       ";;defaultparam"
#define TXSC_P_SISTEP       ";;singlestep"
#define TXSC_P_VERBOS       ";;verbose"
#define TXSC_P_PROMPT       ";;prompt"
#define TXSC_P_ERRORS       ";;error"

#define TXSC_PARAMS         10                  // substitutable parameters


#if defined (USEWINDOWING)
static BOOL txs_help_registred = FALSE;
#endif

// JvW TODO add OPERATOR desriptions!!

static  char       *runhelp[] =
{
   "#000 TXScript, generic RUN dialog",
   "",
   "Run a SCRIPT-file with a sequence of commands"
#if defined (DEV32)
   ", or containing REXX code",
#else
   " to be executed",
#endif
   "",
   "usage:       RUN  scriptfile-name  [options] [arguments]",
   "",
#if defined (DEV32)
   " Options:    (Options and Arguments for NATIVE scripts, not REXX)",
#else
   " Options:",
#endif
   "   -E:i    = ignore errors, continue execution when RC is not 0",
   "   -E:q    = quit on any errors, exit  (default when in batch mode)",
   "   -E:c    = confirm by user on errors (default when interactive)",
   "   -h      = brief help on variable usage and host variables",
   "   -p-     = no prompting and command-echo by the command handler",
   "   -p:1    = echo command by handler, but no prompting afterwards",
   "   -p:2    = no echo by handler, but prompting afterwards",
   "   -p:3    = echo before, and prompt after executing (default)",
   "   -q      = quit the program after executing the script",
   "   -s      = single-step, confirm each line before executing it",
   "   -v      = verbose, show details about running the script",
   "",
   " Arguments : upto 9 arguments can be specified and can be used anywhere",
   "             in the script as '$1' to '$9' while $0 is the scriptname.",
   "             Arguments can be enclosed in single- or double-quotes",
   "             to allow embedded spaces or special characters.",
   "",
   NULL
};


#if defined (USEWINDOWING)
static  char       *scripthelp[] =
{
   "#020 TXScript, layout and control structures",
   "",
   "Native scripts, layout and control structures:"
   "",
   "A native script consists of a series of 'statements' in a",
   "statement-list (STL), where each statement is either a single",
   "line with comment, an assignment, a pragma, a host-command or",
   "a control-structure like IF, WHILE and FOR ...",
   "",
   "Code-blocks within control-structures are statement-lists too.",
   "",
   "",
   "  ;script LAYOUT example         ;A comment line",
   "",
   "  ;;defaultparam 1 5             ;A pragma",
   "",
   "  IF  $1 < $_parts               ;Control statement",
   "                                 ;with an expression",
   "    Say $1 is OK!                ;A command to be",
   "                                 ;executed by DFSee",
   "  ENDIF                          ;End of the Control statement",
   "",
   "",
   "",
   "The following control-structures and keywords are supported:",
   "",
   "  IF  (condition)                ;Like the Perl IF, not using a",
   "    statement-list               ;{} block  but an ENDIF keyword",
   "  ELSEIF (condition)             ;() on conditions are optional",
   "    statement-list               ;Any number of the ELSEIF clauses",
   "  ELSE                           ;ELIF, ELSIF and ELSEIF accepted",
   "    statement-list",
   "  ENDIF",
   "",
   "",
   "",
   "  WHILE  (condition)             ;Like Perl or 'C' but not using a",
   "    statement-list               ;{} block  but an explicit end",
   "  ENDWHILE label                 ;() parenthesis on conditions optional",
   "",
   "",
   "  FOR init;condition;iterator    ;often used as a 'counting' loop",
   "    statement-list",
   "  ENDFOR label",
   "",
   "",
   "  DO label                       ;condition evaluated at the END",
   "    statement-list",
   "  UNTIL (condition)",
   "",
   "",
   "",
   "",
   "  LOOP                           ;no condition at all",
   "    statement-list",
   "  ENDLOOP label",
   "",
   "",
   "",
   "",
   "  LOOP                           ;condition at arbitrary location",
   "    statement-list               ;can be empty",
   "    EXIT label WHEN (condition)  ;EXIT WHEN will exit the loop, when",
   "    statement-list               ;the condition is TRUE, and commence",
   "  ENDLOOP label                  ;at the 'next statement'",
   "  next-statement",
   "",
   "",
   "",
   "  WHILE  (condition)",
   "    statement-list",
   "    IF (some condition)",
   "      BREAK label                ;BREAK will exit the loop, and",
   "    ENDIF                        ;commence at 'next-statement'",
   "    statement-list",
   "  ENDWHILE label",
   "  next-statement",
   "",
   "",
   "  WHILE  (condition)",
   "    statement-list",
   "    IF (some condition)",
   "      CONTINUE                   ;CONTINUE will 'jump' directly to",
   "    ENDIF                        ;the loop iterator (condition)",
   "    statement-list               ;bypassing the remainder of the",
   "  ENDWHILE label                 ;loop statement-list",
   "",
   "",
   "Notes:",
   "        The BREAK, CONTINUE and EXIT WHEN statements can be used ",
   "        in any of the FOR, WHILE, DO or LOOP looping constructs.",
   "",
   "        The 'label' on any of the loops is optional, and only needed",
   "        when a 'break' or 'exit when' is required that breaks out of",
   "        an outer loop instead of the current one.",
   "",
   "        The 'break' and 'exit when' statements without a label specified,",
   "        break the, current (innermost) loop they appear in.",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "#040 TXScript, operators and precedence",
   "",
   " $name[]++                  Variable, indexed and",
   " $name[]--                  auto increment/decrement",
   "",
   " Atom, Term                 String, number, function",
   "                            nested-expr or ternary",
   "",
   " - + ! ~                    Unary operators plus/minus/not",
   " * / %                      Binary multiply/division/modulo",
   " + -                        Binary plus/minus",
   " x                          String replication",
   " .                          String concatenation",
   "",
   " <<  >>                     Numeric bit-shift",
   "",
   " == != < > <= >=            Numeric compare",
   " === !==                    Same value AND type",
   " EQ NE LT GT LE GE          String compare",
   "",
   " &                          Bitwise AND",
   " ^                          Bitwise XOR",
   " |                          Bitwise OR",
   "",
   " &&                         Logical AND        (C-style)",
   " ||                         Logical OR         (C-style)",
   "",
   " =                          Assignment",
   " ,                          Comma, multi-expression",
   "",
   " NOT                        Logical NOT        (Perl style)",
   " AND                        Logical AND        (Perl style)",
   " OR                         Logical OR         (Perl style)",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "#060 TXScript, pragmas and builtin functions",
   "",
   "Pragma statements native scripts:"
   "",
   ";;defaultparam  p  value         ; Give a default 'value' to parameter 'p'",
   ";;setparam  p  ? prompt-text     ; Prompt for a new value for parameter p",
   ";;setparam  p  value             ; Set parameter p (0..9) to a fixed value",
   ";;verbose         [off]          ; set verbose mode on or off      (see -v)",
   ";;prompt      [0|1|2|3]          ; set prompting and command echo  (see -p)",
   ";;error         [i|c|q]          ; set error handling for RC not 0 (see -E)",
   ";;singlestep      [off]          ; single step, confirm each line  (see -s)",
   "",
   "",
   "Built-in functions native scripts",
   "",
   "  abs(num)                       : Absolute value, numeric",
   "  b32(num)                       : Clip to 32-bit unsigned",
   "  b2asc(str)                     : Binary string to ASCII",
   "  b2int(str[,len])               : Binary string to reversed int",
   "  chr(num)                       : ASCII value for number",
   "  canceled()                     : Test for canceled last operation",
   "  confirmed(str)                 : Confirmation Yes/No/Cancel",
   "  defined(var)                   : Is variable defined",
   "  dirselect(prompt[,path])       : Select a directory, start in path",
   "  drivefs(drv)                   : FS-name for drive letter",
   "  drivelabel(drv)                : Label string for drive letter",
   "  drives([num])                  : Drive letters in string, 7=all",
   "  drivespace(drv)                : Freespace in KiB for drive",
   "  dump()                         : Dump variable pool contents",
   "  exists(path)                   : File exists",
   "  filext(path,ext)               : Set default file extension",
   "  fileselect(prompt,[wc[,path]]) : Select existing file, wildcard/path",
   "  filesaveas(prompt,[wc[,path]]) : Specify new or existing file",
   "  fnbase(path)                   : Extract filename without ext",
   "  fnfile(path)                   : Extract filename without path",
   "  fnpath(path)                   : Extract path only, no filename",
   "  getcwd()                       : Get current working directory",
   "  h2asc(str)                     : Get string from hex-ascii str",
   "  h2int(str)                     : Get integer from hex-ascii str",
   "  i2dec(num[,len])               : Convert int to decimal str",
   "  i2hex(num[,len])               : Convert int to hexadecimal str",
   "  index(str,sub[,pos])           : Find substring in string",
   "  lc(str)                        : Return lowercased string",
   "  left(str,pos[,len])            : Left adjust string, pad/clip",
   "  length(str)                    : Get length of string",
   "  makedir(path)                  : Create full directory path",
   "  max(a,b,...)                   : Ret maximum of values",
   "  min(a,b,...)                   : Ret minimum of values",
   "  message(str)                   : Message popup, until [OK]",
   "  ord(str)                       : Numeric value 1st char in str",
   "  print([str][,str]...)          : Print zero or more stringsing",
   "  prompt(str)                    : Popup question, return string",
   "  replace(str[,old[,new]])       : Replace characters in string",
   "  sec2gib(num[,bps])             : Get GiB value for #sectors",
   "  sec2kib(num[,bps])             : Get KiB value for #sectors",
   "  sec2mib(num[,bps])             : Get MiB value for #sectors",
   "  reverse(str)                   : Reverse characters in string",
   "  right(str,num[,str])           : Right adjust string pad/clip",
   "  rindex(str,sub[,pos])          : Reverse find substring in str",
   "  sleep(milliseconds)            : Sleep specified nr of mulliseconds",
   "  strip(str[,lead[,trail]])      : Strip leading/trailing string chars",
   "  substr(str,pos[,len])          : Extract substring from string",
   "  uc(str)                        : Return uppercased string",
   "  undef(var)                     : Undefine (free) a variable",
   "",
   NULL
};
#endif

typedef enum txst_type                          // used as token, and node type
{
   TXST_CMD,                                    // command, anything 'other'
   TXST_PRAGMA,                                 // non-specific pragma
   TXST_IF,
   TXST_ELIF,
   TXST_ELSE,
   TXST_ENDIF,
   TXST_WHILE,
   TXST_ENDWHILE,
   TXST_DO,
   TXST_UNTIL,
   TXST_LOOP,
   TXST_EXITWHEN,
   TXST_ENDLOOP,
   TXST_FOR,
   TXST_ENDFOR,
   TXST_BREAK,                                  // skip execution, break out loop
   TXST_CONTINUE,                               // skip execution until iterator
   TXST_RET,
   TXST_EOS                                     // end of script
} TXST_TYPE;                                    // end of enum "txst_type"


typedef struct txsline                          // pre-processed script line
{
   int                 lnr;                     // line number 1st line
   char               *string;                  // line contents
   struct txsline     *next;                    // next script line
} TXSLINE;                                      // end of struct "txsline"


typedef struct txsn_br                          // branch, if/else
{
   char               *cond;                    // condition expression
   struct txsn_list   *stl_if;                  // statement list 'if'
   struct txsn_list   *stl_else;                // statement list 'else'
} TXSN_BR;                                      // end of struct "txsn_br"

typedef struct txsn_lp
{
   char               *init;                    // initial    expression
   char               *cond;                    // condition  expression
   char               *iter;                    // iterator   expression
   char               *label;                   // loop name, break target
   struct txsn_list   *body;                    // statement list body
} TXSN_LP;                                      // end of struct "txsn_lp"


typedef struct txsn_list                        // statement node list element
{
   TXST_TYPE           type;
   union
   {
      TXSN_BR          br;
      TXSN_LP          lp;
   };
   TXSLINE            *line;                    // linear script line
   struct txsn_list   *next;
} TXSN_LIST;                                    // end of struct "txsn_list"

typedef struct txs_script
{
   TXSLINE            *src;                     // first statement line
   TX_NATIVE_CALLBACK  subcom;                  // subcommand handler
   ULONG               scep;                    // echo/prompt status
   BOOL                loud;                    // verbosity   status
   BOOL                skipping;                // skipping execution (continue)
   ULONG               break_line;              // break in progress on line x
   char               *breaklabel;              // target label, or NULL
   TXLN                errtxt;
   char                errs;                    // error handling modus
} TXS_SCRIPT;                                   // end of struct "txs_script"


// Read native TXS script into linear list, params & options from parsed RUN
static ULONG txsReadScript
(
   char               *name,                    // IN    TXS script to execute
   TXS_SCRIPT         *pgm                      // INOUT program structure
);

// Read one logical scriptline into allocated memory
static char  *txsReadLine
(
   FILE               *scr,                     // IN    script filehandle
   ULONG              *lnr                      // INOUT source linenr
);

// Free native TXS script linear source list from the program structure
static void txsFreeSource
(
   TXS_SCRIPT         *pgm                      // INOUT program structure
);

// Parse native TXS script into tree structure from the linear list (recursive)
static ULONG txsParseStl
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSLINE           **src,                     // INOUT line position
   TXST_TYPE          *eol,                     // OUT   end-of-list token
   char              **lbl,                     // OUT   list label, or NULL
   TXSN_LIST         **stl                      // OUT   statement list
);

// Parse IF/ELSEIF/ELSE/ENDIF control into tree structure, recursive
// src on input: at 1st line of an if/elseif statement list
// src on exit : at the line with the endif  statement
static ULONG txsParseIfStmt
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSLINE           **src,                     // INOUT line position
   TXSN_LIST          *ifs                      // INOUT if statement node
);

// Add new STL element to end of statement list
static void txsAdd2Stl
(
   TXSN_LIST          *elem,                    // IN    new statement
   TXSN_LIST         **stl                      // INOUT statement list
);

// Get token for specified line, with optional parameters/condition
static TXST_TYPE txsGetLineToken                // RET   token for this line
(
   TXSLINE            *this,                    // IN    sourceline to examine
   char              **info,                    // OUT   info (label), or NULL
   char              **rest                     // OUT   params/condition
);

// Parse and extract an expression for script control structures, no execute!
static ULONG txsGetExpr
(
   TXS_SCRIPT         *pgm,                     // IN    program info
   TXSLINE            *this,                    // IN    sourceline to examine
   char                term,                    // IN    terminating token
   char              **pos,                     // INOUT parse pos in string
   char              **exp                      // OUT   expression, in 'this'
);

// Free native TXS script tree structure, recursive
static void txsFreeListTree
(
   TXSN_LIST         **stl                      // INOUT statement list
);

// Run native TXS script from parsed execution-tree using callback (recursive)
static ULONG txsRunStl
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSN_LIST          *tree                     // IN    program tree
);

// Run native TXS script command or pragma statement
static ULONG txsRunCommand
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSLINE            *line                     // IN    program line
);

// Parse and evaluate an expression for script control structures, executing!
static ULONG txsRunExpr
(
   TXS_SCRIPT         *pgm,                     // IN    program info
   TXSLINE            *this,                    // IN    originating sourceline
   char               *expr,                    // IN    expresssion to evaluate
   char               *type,                    // IN    descriptive type of expr
   BOOL               *cond                     // OUT   result value as boolean
);                                              //       or NULL if not needed

// Return descriptive string for script token
static char  *txsToken2String
(
   TXST_TYPE           token,                   // IN    token
   char               *text                     // INOUT text buffer
);

// Handle script-level error message, printf-like interface
static void TxsScriptError
(
   TXS_SCRIPT         *pgm,                     // IN    program info
   ULONG               lnr,                     // IN    line number
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
);



/*========================== SINGLE-STEP Widgets ================================================*/
static BOOL bVerbose       = FALSE;             // Verbose mode
static BOOL bSingleStep    = FALSE;             // SingleStep
static TXTS sLineNumber    = "     5";
static TXTT sStatementType = "Condition-WHILE";

#define TXWSFOUTPUT   TXWS_OUTPUT  | TXWS_HCHILD2SIZE
#define TXW22OUTPUT   TXWS_OUTPUT  | TXWS_HCHILD2SIZE | TXWS_HCHILD2MOVE
#define TXW2FOUTPUT   TXWS_OUTPUT  | TXWS_HCHILD2MOVE
#define TXWFMOUTPUT   TXWS_OUTPUT  | TXWS_HCHILD_MOVE

/*
[ ] Verbose, display each script line when executed
[ ] Continue single stepping, confirm before executing each line

    Single step at line:   5     Statement type: Condition-FOR
*/
#define   TXDGSTEPWIDGETS 6
static TXWIDGET  txDgStepWidgets[TXDGSTEPWIDGETS] = // order determines TAB-order!
{
   {0,  0, 1, 58, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( &bVerbose,
                          "Verbose, display each script line when executed")},
   {1,  0, 1, 66, 0, 0, 0, TXWS_AUTOCHK, 0, TXStdButton( &bSingleStep,
                          "Continue single stepping, confirm before executing each line")},
   {3,  5, 1, 20, 0, 0, 0, TXWSFOUTPUT,  0, TXStdStline( "Single step at line:")},
   {3, 25, 1,  6, 0, 0, 0, TXW2FOUTPUT,  0, TXStdStline(  sLineNumber)},
   {3, 33, 1, 15, 0, 0, 0, TXW22OUTPUT,  0, TXStdStline( "Statement type:")},
   {3, 50, 1, 16, 0, 0, 0, TXWFMOUTPUT,  0, TXStdStline(  sStatementType)}
};

static TXRECT    txDgStepPos = {0, 0, 0, 0};    // start with default position

static TXGW_DATA txDgStepDlg =
{
   TXDGSTEPWIDGETS,                             // number of widgets
   0,                                           // help, widget overrules
   810,                                         // base window ID
   NULL,                                        // widget window proc or NULL
   &txDgStepPos,                                // persistent position TXRECT
   txDgStepWidgets                              // array of widgets
};

static TXRECT    txDgCondPos = {0, 0, 0, 0};    // start with default position

static TXGW_DATA txDgCondDlg =
{
   0,                                           // no widgets, just a position
   0,                                           // help, widget overrules
   0,                                           // base window ID
   NULL,                                        // widget window proc or NULL
   &txDgCondPos,                                // persistent position TXRECT
   NULL                                         // array of widgets
};


/*****************************************************************************/
// Run native TXS script using callback, params & options from parsed RUN cmd
/*****************************************************************************/
ULONG TxsNativeRun
(
   char               *name,                    // IN    TXS script to execute
   TX_NATIVE_CALLBACK  subcom                   // IN    subcommand handler
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXS_SCRIPT          program;                 // program contents and status
   TXSN_LIST          *tree = NULL;
   TXSLINE            *source;
   TXST_TYPE           endtoken;
   TXTT                word1;
   TXTT                word2;
   BOOL                local_step = bSingleStep;
   BOOL                local_loud = bVerbose;

   ENTER();

   #if defined (USEWINDOWING)
      if (!txs_help_registred)
      {
         txwRegisterHelpText( TXSC_HELPDIALOG,    "txscript" ,   "TX Script, functions and operators", scripthelp);
         txs_help_registred = TRUE;
      }
   #endif                                       // USEWINDOWING

   if (subcom != NULL)                          // command handler present ?
   {
      bSingleStep    = TxaOption('s');          // single step (global dlg var)
      bVerbose       = TxaOption('v');          // verbose script handling
      program.scep   = TxaOptNum('p', NULL, 3); // prompt and echo state
      program.errs   = TxaErrorStrategy( 'E', TxaExeSwitch('b'));
      program.subcom = subcom;
      program.src    = NULL;
      strcpy( program.errtxt, "");

      if ((rc = txsAddVarPoolLevel()) == NO_ERROR) // add new local level
      {
         if ((rc = txsReadScript( name, &program)) == NO_ERROR)
         {
            source = program.src;
            program.skipping = FALSE;
            program.break_line = 0;
            if ((rc = txsParseStl( &program, &source, &endtoken, NULL, &tree)) == NO_ERROR)
            {
               if ((endtoken == TXST_EOS) ||     // end of script reached
                   (endtoken == TXST_RET)  )     // or a return statement
               {
                  //- to be refined, use return value (caller level $_rc ?)
                  /*
                  if ((endtoken = txsGetLineToken( source, NULL, &s)) == TXST_RET)
                  {
                     rc = txsGetExpr( &program, source, 0, &s, &s);
                  }
                  */
                  rc = txsRunStl( &program, tree);
                  if (rc == NO_ERROR)
                  {
                     if (       ( program.break_line != 0)     && // break/exit pending
                         (strcmp( program.breaklabel, "*") != 0)) // and not break ALL
                     {
                        TxsScriptError( &program, program.break_line,
                                        "BREAK or EXIT-WHEN target label: '%s' "
                                        "was not found", program.breaklabel);
                        rc = TX_SYNTAX_ERROR;
                     }
                  }
               }
               else
               {
                  TxsScriptError( &program, source->lnr,
                                  "Unexpected program end: '%s', "
                                  "expected '%s' or end of file",
                                  txsToken2String( endtoken, word1),
                                  txsToken2String( TXST_RET, word2));
                  rc = TX_SYNTAX_ERROR;
               }
            }
            txsFreeListTree( &tree);
         }
         if ((rc != NO_ERROR) && (strlen( program.errtxt)))
         {
            TxPrint( "\n%s\n", program.errtxt);
         }
         txsFreeSource( &program);
         txsDelVarPoolLevel();                  // remove local variable level
      }
   }
   else                                         // give usage for RUN
   {
      TxShowTxt( runhelp);
   }
   bSingleStep = local_step;                    // restore previous step state
   bVerbose    = local_loud;                    // restore previous loud state
   RETURN (rc);
}                                               // end 'TxsNativeRun'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Read native TXS script into linear list, params & options from parsed RUN
/*****************************************************************************/
static ULONG txsReadScript
(
   char               *name,                    // IN    TXS script to execute
   TXS_SCRIPT         *pgm                      // INOUT program structure
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSLINE            *txsline;
   TXSLINE           **nextline = &(pgm->src);  // first source line
   char               *line;                    // allocated script line
   FILE               *script = NULL;
   ULONG               linenr = 1;              // 1st linenr of NEXT line
   BOOL                abort  = FALSE;
   TXLN                fullname;                // script fullname
   TXTM                basename;                // script basename ($0)
   char               *params[10];              // script parameters
   char               *pardef[10];              // default parameter values
   char               *pp;                      // parameter
   char               *s;                       // generic char pointer
   int                 pi;                      // parameter index

   ENTER();

   if ((script = TxFindAndOpenFile( name, "PATH", fullname)) != NULL)
   {
      if (bVerbose)                             // verbose ?
      {
         TxPrint( "\nRUN native script : '%s'\n", fullname);
      }
      strcpy( basename, TxGetBaseName( name));
      TxStripExtension( basename);

      for (pi = 0; pi < 10; pi++)               // params 0..9
      {
         pardef[ pi] = NULL;                    // default parameter value
         params[ pi] = TxaArgValue( pi +1);     // parsed arguments
      }
      params[0] = basename;                     // use base script name

      while (!abort && !TxAbort() && !feof(script) && !ferror(script))
      {
         if ((txsline = TxAlloc( 1, sizeof( TXSLINE))) != NULL)
         {
            *nextline = txsline;                // link new to existing
            nextline  = &(txsline->next);       // set next link location

            txsline->lnr = linenr;
            if ((line = txsReadLine( script, &linenr)) != NULL)
            {
               char   *repl;
               int     rlen = 2 * strlen( line) + TXMAXLN;

               if ((repl = TxAlloc( 1, rlen)) != NULL)
               {
                  txsSubstituteParams( line, params,  TXSC_PARAMS, rlen, repl);
                  TxFreeMem( line);
                  if ((line = TxAlloc( 1, strlen( repl) +1)) != NULL)
                  {
                     strcpy( line, repl);
                     TxFreeMem( repl);
                  }
                  else
                  {
                     line = repl;
                  }
               }
               txsline->string = line;

               //- to be refined, may run this for selected pragmas (not pardef)!
               //- rc = txsRunCommand( pgm, txsline);

               if (rlen && ((line[0] != ';') || (line[1] == ';')))
               {
                  if (strncmp( line, TXSC_PRAGMA, strlen(TXSC_PRAGMA)) == 0)
                  {
                     TRACES(("Pragma: '%s'\n", line));
                     for (pp = line; *pp && (*pp != ' '); pp++)
                     {
                     }
                     while (*pp && (*pp == ' '))
                     {
                        pp++;                   // skip whitespace
                     }
                     TRACES(("Params: '%s'\n", pp));
                     if (strncasecmp( line, TXSC_P_PARDEF, strlen( TXSC_P_PARDEF)) == 0)
                     {
                        if (isdigit(*pp))
                        {
                           pi = (int) ((*pp) - '0');        //- index 0..9
                           if ((params[pi] == NULL) || (strlen(params[pi]) == 0) ||
                               ((strlen(params[pi]) == 1) && (*params[pi] == '.')))
                           {                    // not set, need a default
                              pp += 2;
                              TxFreeMem( pardef[pi]);
                              if ((pardef[pi] = TxAlloc(  1,  strlen(pp) + 1)) != NULL)
                              {
                                 if (bVerbose)  // verbose ?
                                 {
                                    TxPrint( "RUN def  $%d value : '%s'\n", pi, pp);
                                 }
                                 strcpy( pardef[pi], pp);
                                 if ((s = strchr( pardef[pi], ';')) != NULL)
                                 {
                                    *s = 0;     // strip comment from value
                                 }
                                 //- also removes translated CR/LF on Unix
                                 TxStrip( pardef[pi], pardef[pi], 0, ' ');
                                 params[pi] = pardef[pi];
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
         else
         {
            abort = TRUE;
         }
      }
      for (pi = 0; pi < 10; pi++)               // params 0..9
      {
         TxFreeMem( pardef[ pi]);               // free allocated defaults
      }
      fclose( script);
   }
   else
   {
      TxPrint( "TXs native script : '%s' not found\n", name);
      rc = TX_INVALID_FILE;
   }
   RETURN (rc);
}                                               // end 'txsReadScript'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Read one logical scriptline into allocated memory
/*****************************************************************************/
static char  *txsReadLine
(
   FILE               *scr,                     // IN    script filehandle
   ULONG              *lnr                      // INOUT source linenr
)
{
   char               *rc = NULL;               // function return
   char               *ln;                      // new chunk of line
   TXLN                line;                    // script line

   ENTER();

   do
   {
      if (fgets( line, TXMAXLN, scr) != NULL)
      {
         (*lnr)++;
         TxRepl( line, '\n', '\0');
         TxRepl( line, '\r', '\0');             // mainly for Linux/Unix ...

         if (rc)
         {
            rc[ strlen(rc) -1] = 0;             // remove continuation char
            if ((ln = TxAlloc( 1, strlen( line) + strlen(rc) + 3)) != NULL)
            {
               strcpy( ln, rc);
               strcat( ln, "\n");
               strcat( ln, line);
            }
         }
         else                                   // first line
         {
            if ((ln = TxAlloc( 1, strlen( line) + 1)) != NULL)
            {
               strcpy( ln, line);
            }
         }
         TxFreeMem( rc);
         rc = ln;
      }
   } while (rc && (rc[strlen(rc) -1] == '\\')); // line continued
   RETURN (rc);
}                                               // end 'txsReadLine'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Free native TXS script linear source list from the program structure
/*****************************************************************************/
static void txsFreeSource
(
   TXS_SCRIPT         *pgm                      // INOUT program structure
)
{
   TXSLINE            *txsline;
   TXSLINE            *txsnext;

   ENTER();

   for (txsline = pgm->src; txsline; txsline = txsnext)
   {
      txsnext  = txsline->next;
      TxFreeMem( txsline->string);
      TxFreeMem( txsline);
   }
   VRETURN();
}                                               // end 'txsFreeSource'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse native TXS script into tree structure from the linear list (recursive)
/*****************************************************************************/
static ULONG txsParseStl
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSLINE           **src,                     // INOUT line position
   TXST_TYPE          *eol,                     // OUT   end-of-list token
   char              **lbl,                     // OUT   list label, or NULL
   TXSN_LIST         **stl                      // OUT   statement list
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXST_TYPE           token;
   TXST_TYPE           endtoken;
   TXSLINE            *pos  = *src;
   TXSN_LIST          *new;
   char               *s;
   char                close;
   TXTT                word1;
   TXTT                word2;
   char               *label = NULL;            // optional label info

   ENTER();

   *eol = TXST_EOS;
   while (pos && pos->string && (rc == NO_ERROR))
   {
      token = txsGetLineToken( pos, &label, &s);
      if (   (token == TXST_UNTIL)  || (token == TXST_ENDWHILE) || (token == TXST_ENDFOR) ||
             (token == TXST_ELIF )  || (token == TXST_ELSE    ) || (token == TXST_ENDIF ) ||
             (token == TXST_EOS  )  || (token == TXST_ENDLOOP ) )
      {
         if ((token == TXST_ENDFOR) || (token == TXST_ENDWHILE) || (token == TXST_ENDLOOP ))
         {
            if (lbl != NULL)                    // want the optional label too
            {
               *lbl  = label;                   // hand over (allocated if not NULL!)
               label = NULL;
            }
         }
         *eol = token;
         break;
      }
      else
      {
         if ((new = TxAlloc( 1, sizeof( TXSN_LIST))) != NULL)
         {
            txsAdd2Stl( new, stl);              // add to end of current list
            new->line = pos;
            new->type = token;
            close     = 0;                      // no mandatory closing token
            switch (token)
            {
               case TXST_WHILE:
                  if ((rc = txsGetExpr( pgm, pos, close, &s, &new->lp.cond)) == NO_ERROR)
                  {
                     pos = pos->next;           // skip the 'while' line
                     if ((rc = txsParseStl( pgm, &pos, &endtoken,
                                            &new->lp.label, &new->lp.body)) == NO_ERROR)
                     {
                        if (endtoken != TXST_ENDWHILE)
                        {
                           TxsScriptError(  pgm, pos->lnr, "Missing '%s' for '%s' on line %lu",
                                txsToken2String( TXST_ENDWHILE, word1),
                                txsToken2String( TXST_WHILE,    word2), new->line->lnr);
                           rc = TX_SYNTAX_ERROR;
                        }
                     }
                  }
                  break;

               case TXST_DO:
                  new->lp.label = label;        // hand over (allocated if not NULL!)
                  label = NULL;
                  pos = pos->next;              // skip the 'do' line
                  if ((rc = txsParseStl( pgm, &pos, &endtoken, NULL, &new->lp.body)) == NO_ERROR)
                  {
                     //- need to reparse the UNTIL line to get the proper start position
                     //- for the condition-expression (not returned by ParseStl)
                     if ((endtoken = txsGetLineToken( pos, NULL, &s)) == TXST_UNTIL)
                     {
                        rc = txsGetExpr( pgm, pos, close, &s, &new->lp.cond);
                     }
                     else
                     {
                        TRACES(("invalid end-token %d for DO statement\n", endtoken));
                        TxsScriptError(  pgm, pos->lnr, "Missing '%s' for '%s' on line %lu",
                             txsToken2String( TXST_UNTIL, word1),
                             txsToken2String( TXST_DO,    word2), new->line->lnr);
                        rc = TX_SYNTAX_ERROR;
                     }
                  }
                  break;

               case TXST_LOOP:
                  pos = pos->next;              // skip the 'loop' line
                  if ((rc = txsParseStl( pgm, &pos, &endtoken,
                                         &new->lp.label, &new->lp.body)) == NO_ERROR)
                  {
                     if (endtoken != TXST_ENDLOOP)
                     {
                        TRACES(("invalid end-token %d for LOOP statement\n", endtoken));
                        TxsScriptError(  pgm, pos->lnr, "Missing '%s' for '%s' on line %lu",
                             txsToken2String( TXST_ENDLOOP, word1),
                             txsToken2String( TXST_LOOP,    word2), new->line->lnr);
                        rc = TX_SYNTAX_ERROR;
                     }
                  }
                  break;

               case TXST_FOR:
                  if (*s == TXS_POP)
                  {
                     close = TXS_PCL;           // mandatory closing
                     s++;                       // skip the open parenthesis
                  }
                  if ((rc = txsGetExpr( pgm, pos, ';', &s, &new->lp.init)) == NO_ERROR)
                  {
                     if ((rc = txsGetExpr( pgm, pos, ';', &s, &new->lp.cond)) == NO_ERROR)
                     {
                        if ((rc = txsGetExpr( pgm, pos, close, &s, &new->lp.iter)) == NO_ERROR)
                        {
                           pos = pos->next;     // skip the 'for' line
                           if ((rc = txsParseStl( pgm, &pos, &endtoken,
                                                  &new->lp.label, &new->lp.body)) == NO_ERROR)
                           {
                              if (endtoken != TXST_ENDFOR)
                              {
                                 TxsScriptError(  pgm, pos->lnr, "Missing '%s' for '%s' on line %lu",
                                      txsToken2String( TXST_ENDFOR, word1),
                                      txsToken2String( TXST_FOR,    word2), new->line->lnr);
                                 rc = TX_SYNTAX_ERROR;
                              }
                           }
                        }
                     }
                  }
                  break;

               case TXST_IF:
                  if ((rc = txsGetExpr( pgm, pos, close, &s, &new->br.cond)) == NO_ERROR)
                  {
                     pos = pos->next;           // skip the 'if' line
                     rc = txsParseIfStmt( pgm, &pos, new);
                  }
                  break;

               case TXST_EXITWHEN:
                  rc = txsGetExpr( pgm, pos, close, &s, &new->lp.cond);
               case TXST_BREAK:
                  TRACES(("Label targeted by EXIT/BREAK: '%s'\n", label));
                  new->lp.label = label;        // hand over (allocated if not NULL!)
                  label = NULL;
                  break;

               case TXST_CMD:
               case TXST_PRAGMA:
               default:
                  //- nothing to do for that
                  break;
            }
            if (pos != NULL)                    // may already be NULL after
            {                                   // skipped WHILE/FOR/IF cond!
               pos = pos->next;                 // iterate to next statement
            }
         }
         else
         {
            rc = TX_ALLOC_ERROR;
         }
      }
      TxFreeMem( label);                        // free when not handed over
   }
   TRACES(("Token: 0x%02hx on line: %3d = '%s'\n", *eol, pos->lnr, pos->string));

   *src = pos;
   RETURN (rc);
}                                               // end 'txsParseStl'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse IF/ELSEIF/ELSE/ENDIF control into tree structure, recursive
// src on input: at 1st line of an if/elseif statement list
// src on exit : at the line with the endif  statement
/*****************************************************************************/
static ULONG txsParseIfStmt
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSLINE           **src,                     // INOUT line position
   TXSN_LIST          *ifs                      // INOUT if statement node
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXST_TYPE           endtoken;
   TXSLINE            *pos  = *src;
   TXSN_LIST          *new;
   char               *s;
   TXTT                word1;
   TXTT                word2;

   ENTER();

   if ((rc = txsParseStl( pgm, &pos, &endtoken, NULL, &ifs->br.stl_if)) == NO_ERROR)
   {
      switch (endtoken)
      {
         case TXST_ELIF:
            if ((new = TxAlloc( 1, sizeof( TXSN_LIST))) != NULL)
            {
               ifs->br.stl_else = new;
               new->line = pos;
               new->type = TXST_IF;
               txsGetLineToken( pos, NULL, &s); // get start of expression again
               if ((rc = txsGetExpr( pgm, pos, 0, &s, &new->br.cond)) == NO_ERROR)
               {
                  pos = pos->next;              // skip the 'elif' line
                  rc  = txsParseIfStmt( pgm, &pos, new);
               }
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
            break;

         case TXST_ELSE:
            pos = pos->next;                    // skip the 'else' line
            if ((rc = txsParseStl( pgm, &pos, &endtoken, NULL, &ifs->br.stl_else)) == NO_ERROR)
            {
               if (endtoken != TXST_ENDIF)
               {
                  TxsScriptError(  pgm, pos->lnr, "Missing '%s' for '%s' on line %lu",
                       txsToken2String( TXST_ENDIF, word1),
                       txsToken2String( TXST_IF,    word2), ifs->line->lnr);
                  rc = TX_SYNTAX_ERROR;
               }
            }
            break;

         case TXST_ENDIF:                       // done, return
            break;

         default:
            TxsScriptError(  pgm, pos->lnr, "Missing '%s' for '%s' on line %lu",
                 txsToken2String( TXST_ENDIF, word1),
                 txsToken2String( TXST_IF,    word2), ifs->line->lnr);
            rc = TX_SYNTAX_ERROR;
            break;
      }
   }
   *src = pos;
   RETURN (rc);
}                                               // end 'txsParseIfStmt'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse and extract an expression for script control structures, no execute!
// Resulting expression is ALLOCATED, so must be freed with the tree later
/*****************************************************************************/
static ULONG txsGetExpr
(
   TXS_SCRIPT         *pgm,                     // IN    program info
   TXSLINE            *this,                    // IN    sourceline to examine
   char                term,                    // IN    terminating token
   char              **pp,                      // INOUT parse pos in string
   char              **exp                      // OUT   expression, allocated!
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSPARSESTATE       pstate;                  // result value & state
   int                 ln;                      // token length
   char               *start;
   char                nt;                      // next token
   char               *newExpr = NULL;          // new expression string

   ENTER();
   TRACES(( "evaluate: '%s'\n", *pp));

   for (start = *pp; *start == ' '; start++)
   {
      // start at current position, but skip whitespace to find real expression start
   }
   if ((rc = txsEvaluateExpr( this->string, this->lnr, TRUE, pp, &pstate)) == NO_ERROR)
   {
      nt = txsNextToken( pp, &ln);              // skip whitespace, next token
      if (term)
      {
         if (nt == term)
         {
            *pp += ln;
         }
         else
         {
            TxsExprError( &pstate, *pp, "Missing '%c' in: '%s'", term, start);
            rc = TX_SYNTAX_ERROR;
         }
      }
      else if (nt && (nt != ';'))               // something there, but no comment
      {
         TxsExprError( &pstate, *pp, "Missing operator, or garbage in: '%s'", start);
         rc = TX_SYNTAX_ERROR;
      }
   }
   if (strlen( pstate.msg))
   {
      strcpy( pgm->errtxt, pstate.msg);
   }
   if (rc == NO_ERROR)
   {
      ln = (*pp - start) +1;
      if ((newExpr = TxAlloc(1, ln)) != NULL)
      {
         TxCopy( newExpr, start, ln);
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   txsVal2Zero( &pstate.value);                 // free any allocated memory
   *exp = newExpr;
   RETURN (rc);
}                                               // end 'txsGetExpr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Add new STL element to end of statement list
/*****************************************************************************/
static void txsAdd2Stl
(
   TXSN_LIST          *elem,                    // IN    new statement
   TXSN_LIST         **stl                      // INOUT statement list
)
{
   TXSN_LIST          *new;

   ENTER();

   if (*stl)
   {
      for (new = *stl; new->next; new = new->next) // find the end
      {
      }
      new->next = elem;
   }
   else
   {
      *stl = elem;
   }
   VRETURN();
}                                               // end 'txsAdd2Stl'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get token for specified line, with optional parameters/condition
/*****************************************************************************/
static TXST_TYPE txsGetLineToken                // RET   token for this line
(
   TXSLINE            *this,                    // IN    sourceline to examine
   char              **info,                    // OUT   info (label), or NULL
   char              **rest                     // OUT   params/condition
)
{
   TXST_TYPE           rc = TXST_CMD;           // function return
   char               *s  = this->string;
   char               *pp;
   char               *w2;                      // end of NEXT word (label etc)
   int                 w2length;                // length of second word
   char               *word2 = NULL;            // allocated word2  (label)

   ENTER();
   TRACES(("Line: '%s'\n", s));

   //- to be refined, parse and allocate label into info on:
   //- DO, ENDWHILE, ENDFOR, ENDLOOP, BREAK and EXIT-label-WHEN

   if (s != NULL)                               // avoid trap on illegal end
   {
      while (isspace(*s))
      {
         s++;                                   // skip white-space
      }
      for (pp = s; *pp && !isspace(*pp); pp++)  // skip first word
      {
      }
      while (isspace(*pp))
      {
         pp++;                                  // skip white-space
      }
      for (w2 = pp; *w2 && !isspace(*w2); w2++) // skip second word
      {
      }
      w2length = w2 - pp;
      if ((info != NULL) && (w2length != 0))    // info wanted and available
      {
         if ((word2 = TxAlloc( 1, w2length + 1)) != NULL)
         {
            memcpy( word2, pp, w2length);
            *info = word2;                      // return the label word
            TRACES(("Assigned word2 value: '%s'\n", word2));
         }
      }
      if      (strncmp( s, TXSC_PRAGMA, strlen(TXSC_PRAGMA)) == 0)
      {
         rc = TXST_PRAGMA;
      }
      else if (strncasecmp( s, "if",       2) == 0)
      {
         rc = TXST_IF;
      }
      else if ((strncasecmp( s, "elseif",  6) == 0) ||
               (strncasecmp( s, "elsif",   5) == 0) ||
               (strncasecmp( s, "elif",    4) == 0)  )
      {
         rc = TXST_ELIF;
      }
      else if (strncasecmp( s, "else",     4) == 0)
      {
         rc = TXST_ELSE;
      }
      else if (strncasecmp( s, "endif",    5) == 0)
      {
         rc = TXST_ENDIF;
      }
      else if (strncasecmp( s, "while",    5) == 0)
      {
         rc = TXST_WHILE;
      }
      else if (strncasecmp( s, "endwhile", 8) == 0)
      {
         pp = w2;                               // skip the label
         rc = TXST_ENDWHILE;
      }
      else if (strncasecmp( s, "do",       2) == 0)
      {
         rc = TXST_DO;
      }
      else if (strncasecmp( s, "until",    5) == 0)
      {
         rc = TXST_UNTIL;
      }
      else if (strncasecmp( s, "loop",     4) == 0)
      {
         rc = TXST_LOOP;
      }
      else if (strncasecmp( s, "exit",     4) == 0)
      {
         if (word2 != NULL)                     // have a 2nd word
         {
            if (strcasecmp( word2, "when") == 0) // it is the 'when' keyword
            {
               TxFreeMem( word2);
               if (info != NULL)
               {
                  *info = NULL;                 // undo the label assignment
               }
            }
            else                                // it was a label
            {
               pp = w2;                         // skip the label
               while (isspace(*pp))
               {
                  pp++;                         // skip white-space after label
               }
            }
         }
         if (strncasecmp( pp, "when",      4) == 0)
         {
            pp += 4;                            // skip the 'when' keyword
            while (isspace(*pp))
            {
               pp++;                            // skip white-space
            }
            rc = TXST_EXITWHEN;
         }
      }
      else if (strncasecmp( s, "endloop",  7) == 0)
      {
         pp = w2;                               // skip the label
         rc = TXST_ENDLOOP;
      }
      else if (strncasecmp( s, "for",      3) == 0)
      {
         rc = TXST_FOR;
      }
      else if (strncasecmp( s, "endfor",   6) == 0)
      {
         pp = w2;                               // skip the label
         rc = TXST_ENDFOR;
      }
      else if (strncasecmp( s, "break",    5) == 0)
      {
         pp = w2;                               // skip the label
         rc = TXST_BREAK;
      }
      else if (strncasecmp( s, "continue", 8) == 0)
      {
         rc = TXST_CONTINUE;
      }
      else if (strncasecmp( s, "end",      3) == 0)
      {
         rc = TXST_EOS;
      }

      while (isspace(*pp))                      // may have moved over label ...
      {
         pp++;                                  // skip white-space
      }
      TRACES(("rest: '%s'\n", pp));
      *rest = pp;
   }
   else
   {
      rc = TXST_EOS;
   }
   RETURN (rc);
}                                               // end 'txsGetLineToken'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Free native TXS script tree structure, recursive
/*****************************************************************************/
static void txsFreeListTree
(
   TXSN_LIST         **stl                      // INOUT statement list
)
{
   TXSN_LIST          *stmt;
   TXSN_LIST          *next;

   ENTER();

   for (stmt = *stl; stmt; stmt = next)
   {
      next =  stmt->next;
      switch (stmt->type)
      {
         case TXST_IF:
            TxFreeMem( stmt->lp.cond);
            txsFreeListTree( &(stmt->br.stl_if));
            txsFreeListTree( &(stmt->br.stl_else));
            break;

         case TXST_WHILE:
         case TXST_LOOP:
         case TXST_DO:
         case TXST_FOR:
            if (stmt->type == TXST_FOR)
            {
               TxFreeMem( stmt->lp.init);
               TxFreeMem( stmt->lp.iter);
            }
            TxFreeMem( stmt->lp.cond);
            TxFreeMem( stmt->lp.label);
            txsFreeListTree( &(stmt->lp.body));
            break;

         case TXST_EXITWHEN:
            TxFreeMem( stmt->lp.cond);
         case TXST_BREAK:
            TxFreeMem( stmt->lp.label);
            break;

         case TXST_CMD:
         case TXST_PRAGMA:                      // non-specific pragma
         default:
            break;
      }
      TxFreeMem( stmt);
   }
   *stl = NULL;
   VRETURN();
}                                               // end 'txsFreeListTree'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Run native TXS script from parsed execution-tree using callback (recursive)
/*****************************************************************************/
static ULONG txsRunStl
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSN_LIST          *tree                     // IN    program tree
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSN_LIST          *stmt;                    // current statement in list
   BOOL                condition;
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            rv = TXSUNDEF;           // array index value
   char               *sL;                      // start of line contents
   static TX1K         substLine;               // variable substitution

   ENTER();

   for (stmt = tree;
        stmt && (rc == NO_ERROR) && (pgm->skipping == FALSE);
        stmt = stmt->next)
   {
      //- no step/loud on comment and pragma or empty lines
      for (sL = stmt->line->string; *sL == ' '; sL++)
      {
         //- find first non-whitespace character
      }
      if (*sL && (*sL != ';') && (bVerbose || bSingleStep))
      {
         txsResolveVariables( sL, substLine, TXMAX1K);
         if (bVerbose)                          // verbose ?
         {
            TxPrint( "%sLine%4lu : %s%s\n", CNZ, stmt->line->lnr, CNN, substLine);
         }
         TxStripAnsiCodes( substLine);          // remove colors for popup
         if (bSingleStep)                       // single-stepping
         {
            txsToken2String( stmt->type, sStatementType);
            sprintf( sLineNumber, "%6d", stmt->line->lnr);
            if (!TxwConfirm( 0, &txDgStepDlg, "%-120s\n\n%18.18s"
                            "Execute this script line ?\n", substLine, ""))
            {
               if (TxAbort())
               {
                  rc = TX_ABORTED;  //- Abort execution of the script completely
                  break;
               }
               else
               {
                  continue;         //- Do not execute this line, but continue with next one
               }
            }
         }
      }
      switch (stmt->type)
      {
         case TXST_WHILE:
            while (((rc = txsRunExpr( pgm, stmt->line, stmt->lp.cond,
                         "Condition-WHILE", &condition)) == NO_ERROR) && condition)
            {
               if ((rc = txsRunStl( pgm, stmt->lp.body)) == NO_ERROR)
               {
                  pgm->skipping = FALSE;        // continue stl execution (or break)
                  if (pgm->break_line != 0)
                  {
                     TRACES(("Label: br: '%s' lp: '%s'\n", pgm->breaklabel, stmt->lp.label));
                     if ((pgm->breaklabel == NULL) || // no target label
                         ((stmt->lp.label != NULL) && // or matching this loop
                          (strcasecmp( pgm->breaklabel, stmt->lp.label) == 0)))
                     {
                        pgm->break_line = 0;    // don't break higher levels
                        pgm->breaklabel = NULL;
                     }
                     break;
                  }
               }
               else
               {
                  break;                        // break loop on errors
               }
            }
            break;

         case TXST_DO:
            do
            {
               if ((rc = txsRunStl( pgm, stmt->lp.body)) == NO_ERROR)
               {
                  pgm->skipping = FALSE;        // continue stl execution (or break)
                  if (pgm->break_line != 0)
                  {
                     TRACES(("Label: br: '%s' lp: '%s'\n", pgm->breaklabel, stmt->lp.label));
                     if ((pgm->breaklabel == NULL) || // no target label
                         ((stmt->lp.label != NULL) && // or matching this loop
                          (strcasecmp( pgm->breaklabel, stmt->lp.label) == 0)))
                     {
                        pgm->break_line = 0;    // don't break higher levels
                        pgm->breaklabel = NULL;
                     }
                     break;
                  }
               }
               else
               {
                  break;                        // break loop on errors
               }
            } while (((rc = txsRunExpr( pgm, stmt->line, stmt->lp.cond,
                           "Condition-DO", &condition)) == NO_ERROR) && !condition);
            break;

         case TXST_LOOP:
            do
            {
               if ((rc = txsRunStl( pgm, stmt->lp.body)) == NO_ERROR)
               {
                  pgm->skipping = FALSE;        // continue stl execution (or break)
                  if (pgm->break_line != 0)
                  {
                     TRACES(("Label: br: '%s' lp: '%s'\n", pgm->breaklabel, stmt->lp.label));
                     if ((pgm->breaklabel == NULL) || // no target label
                         ((stmt->lp.label != NULL) && // or matching this loop
                          (strcasecmp( pgm->breaklabel, stmt->lp.label) == 0)))
                     {
                        pgm->break_line = 0;    // don't break higher levels
                        pgm->breaklabel = NULL;
                     }
                     break;
                  }
               }
               else
               {
                  break;                        // break loop on errors
               }
            } while (1);                        // exits on break only
            break;

         case TXST_FOR:
            if ((rc = txsRunExpr( pgm, stmt->line, stmt->lp.init,
                                 "Initialize-FOR", NULL)) == NO_ERROR)
            {
               while (((rc = txsRunExpr( pgm, stmt->line, stmt->lp.cond,
                            "Condition-FOR", &condition)) == NO_ERROR) && condition)
               {
                  if ((rc = txsRunStl( pgm, stmt->lp.body)) == NO_ERROR)
                  {
                     ULONG iter_rc;

                     iter_rc = txsRunExpr( pgm, stmt->line, stmt->lp.iter,
                                                    "Iterator-FOR", NULL);
                     if (iter_rc == NO_ERROR)
                     {
                        pgm->skipping = FALSE;  // continue stl execution (or break)
                        if (pgm->break_line != 0)
                        {
                           TRACES(("Label: br: '%s' lp: '%s'\n", pgm->breaklabel, stmt->lp.label));
                           if ((pgm->breaklabel == NULL) || // no target label
                               ((stmt->lp.label != NULL) && // or matching this loop
                                (strcasecmp( pgm->breaklabel, stmt->lp.label) == 0)))
                           {
                              pgm->break_line = 0; // don't break higher levels
                              pgm->breaklabel = NULL;
                           }
                           break;
                        }
                     }
                     else
                     {
                        rc = iter_rc;           // assign original rc
                        break;                  // and exit the loop
                     }
                  }
                  else
                  {
                     break;                     // break loop on errors
                  }
               }
            }
            break;

         case TXST_IF:
            if ((rc = txsRunExpr( pgm, stmt->line, stmt->br.cond,
                     "Condition-IF", &condition)) == NO_ERROR)
            {
               if (condition)
               {
                  rc = txsRunStl( pgm, stmt->br.stl_if);
               }
               else
               {
                  rc = txsRunStl( pgm, stmt->br.stl_else);
               }
            }
            break;

         case TXST_EXITWHEN:
            if ((rc = txsRunExpr( pgm, stmt->line, stmt->lp.cond,
                     "Condition-EXIT-WHEN", &condition)) == NO_ERROR)
            {
               if (condition)
               {
                  pgm->skipping = TRUE;               //- stop stl execution
                  pgm->break_line = stmt->line->lnr;  //- notify enclosing loops
                  pgm->breaklabel = stmt->lp.label;
                  if (bVerbose)                       //- verbose ?
                  {
                     TxPrint( "EXIT WHEN at line % 3lu\n", stmt->line->lnr);
                  }
                  TRACES(("EXITWHEN Label: '%s'\n", pgm->breaklabel));
               }
            }
            break;

         case TXST_BREAK:
            pgm->skipping = TRUE;               // stop stl execution
            pgm->break_line = stmt->line->lnr;  // notify enclosing loops
            pgm->breaklabel = stmt->lp.label;
            if (bVerbose)                      // verbose ?
            {
               TxPrint( "BREAK at line % 3lu\n", stmt->line->lnr);
            }
            TRACES(("BREAK Label: '%s'\n", pgm->breaklabel));
            break;

         case TXST_CONTINUE:
            pgm->skipping = TRUE;               // stop stl execution
            if (bVerbose)                       // verbose ?
            {
               TxPrint( "CONTINUE at line % 3lu\n", stmt->line->lnr);
            }
            break;

         case TXST_CMD:
         case TXST_PRAGMA:
         default:
            rc = txsRunCommand( pgm, stmt->line);
            if (rc == TX_IGNORED)
            {
               // Empty or comment, do NOT overwrite the $_rc value!
               rc = NO_ERROR;
            }
            else
            {
               nv.defined = TRUE;
               nv.num     = rc;
               txsSetVariable( '$', "_rc", &rv, &nv);
            }
            if ((rc != NO_ERROR) && (rc != TX_APP_QUIT))
            {
               switch (pgm->errs)
               {
                  case 'q':                     // quit on errors
                     break;

                  case 'c':                     // confirmation by user
                     txsToken2String( stmt->type, sStatementType);
                     if (TxConfirm( 0,
                        "ERROR %lu at line number : % 6lu\n%s\nStatement type is %s\n\n%s\n\n"
                        "Ignore the error and continue with next line of the script ? [Y/N]\n",
                         rc, stmt->line->lnr, (strlen( pgm->errtxt) ? pgm->errtxt : ""),
                         sStatementType, stmt->line->string))
                     {
                        rc = NO_ERROR;
                     }
                     break;

                  case 'i':
                  default:
                     rc = NO_ERROR;             // ignore errors here
                     break;
               }
            }
            break;
      }
   }
   RETURN (rc);
}                                               // end 'txsRunStl'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Run native TXS script command or pragma statement
/*****************************************************************************/
static ULONG txsRunCommand
(
   TXS_SCRIPT         *pgm,                     // IN    program source
   TXSLINE            *line                     // IN    program line
)
{
   ULONG               rc = NO_ERROR;           // function return
   char               *cc;                      // command start
   char               *pp;                      // parameter
   char               *command = line->string;

   ENTER();

   for (cc = command; *cc && isspace(*cc); cc++) // search first non-space
   {
   }
   if (strlen( cc) && ((*cc != ';') || (*(cc+1) == ';')))
   {
      if (strncmp( cc, TXSC_PRAGMA, strlen(TXSC_PRAGMA)) == 0)
      {
         TRACES(("Pragma: '%s'\n", cc));
         for (pp = cc; *pp && !isspace(*pp); pp++)
         {
         }
         while (*pp && isspace(*pp))
         {
            pp++;                               // skip whitespace
         }
         TRACES(("Params: '%s'\n", pp));
         if (strncasecmp( cc, TXSC_P_VERBOS, strlen(TXSC_P_VERBOS)) == 0)
         {
            BOOL      verbose = bVerbose;

            bVerbose = TxaParseBool( pp);
            if (bVerbose || verbose)
            {
               TxPrint( "RUN set verbose   : %s\n", (bVerbose) ? "ON" : "OFF");
            }
         }
         else if (strncasecmp( cc, TXSC_P_SISTEP, strlen(TXSC_P_SISTEP)) == 0)
         {
            bSingleStep = TxaParseBool( pp);
            if (bVerbose)                       // verbose ?
            {
               TxPrint( "RUN set singlestep: %s\n", (bSingleStep) ? "ON" : "OFF");
            }
         }
         else if (strncasecmp( cc, TXSC_P_PROMPT, strlen(TXSC_P_PROMPT)) == 0)
         {
            if (isdigit(*pp))
            {
               pgm->scep = (ULONG) (*pp - '0');
               if (bVerbose)                    // verbose ?
               {
                  TxPrint( "RUN set prompting : %lu\n", pgm->scep);
               }
            }
         }
         else if (strncasecmp( cc, TXSC_P_ERRORS, strlen(TXSC_P_ERRORS)) == 0)
         {
            if (strchr( "icqICQ", *pp) != NULL)
            {
               pgm->errs = tolower(*pp);
               if (bVerbose)                    // verbose ?
               {
                  TxPrint( "RUN set error mode: '%c'\n", pgm->errs);
               }
            }
         }
         else if (strncasecmp( cc, TXSC_P_PARDEF, strlen(TXSC_P_PARDEF)) != 0)
         {
            if (bVerbose)
            {
               TxPrint( "RUN warning Pragma: '%s' unknown!\n", cc);
            }
         }
      }
      else                                      // execute by subcom handler
      {
         TXLN          cmdstring;               // cmd IN, errortext OUT

         TRACES(("Subcom: '%s'\n", cc))
         strcpy( cmdstring, cc);
         rc = (pgm->subcom)( cmdstring, line->lnr, (pgm->scep & 0x01), (pgm->scep & 0x02), TRUE);
         if (rc != NO_ERROR)
         {
            strcpy( pgm->errtxt, cmdstring);    // get the error text
         }
      }
   }
   else
   {
      // Nothing was executed
      rc = TX_IGNORED;
   }
   RETURN (rc);
}                                               // end 'txsRunCommand'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Parse and evaluate an expression for script control structures, executing!
/*****************************************************************************/
static ULONG txsRunExpr
(
   TXS_SCRIPT         *pgm,                     // IN    program info
   TXSLINE            *this,                    // IN    originating sourceline
   char               *expr,                    // IN    expresssion to evaluate
   char               *type,                    // IN    descriptive type of expr
   BOOL               *cond                     // OUT   result value as boolean
)                                               //       or NULL if not needed
{
   ULONG               rc = NO_ERROR;           // function return
   char               *s  = expr;
   TXSPARSESTATE       pstate;                  // result value & state
   TX1K                substLine;

   ENTER();
   TRACES(( "execute: '%s'\n", expr));

   if ((bSingleStep) || (bVerbose))
   {
      txsResolveVariables( expr, substLine, TXMAX1K);
      if (bVerbose)                             // verbose ?
      {
         TxPrint( "%s%4.4s%4lu : %s%s\n", CNZ, type, this->lnr, CNN, substLine);
      }
      TxStripAnsiCodes( substLine);             // remove colors for popup
      if (bSingleStep)                          // single-stepping
      {
         // to be refined, should use TxConfirm when not windowed!

         strcpy( sStatementType, type);
         sprintf( sLineNumber, "%6d", this->lnr);
         if (!TxwConfirm( 0, &txDgStepDlg, "%-120s\n\n%18.18s"
                         "Execute this script line ?\n", substLine, ""))
         {
            if (TxAbort())
            {
               rc = TX_ABORTED;  //- Abort execution of the script completely
            }
            else
            {
               rc = TX_PENDING;  //- Do not execute this line, but continue with next one
            }
         }
      }
   }

   // Should expression be evaluated ?
   if (rc == NO_ERROR)
   {
      if ((rc = txsEvaluateExpr( this->string, this->lnr, FALSE, &s, &pstate)) == NO_ERROR)
      {
         if (cond)
         {
            *cond = txsVal2Bool( &pstate.value);
            if (bSingleStep)                    // single-stepping
            {
               TxwMessage( TRUE, 0, &txDgCondDlg,
                           "Evaluated condition at line : % 6lu\n"
                           "Condition evaluates to      :  %s\n\n%s\n",
                            this->lnr, (*cond) ? "TRUE" : "FALSE", expr);
            }
         }
      }
      txsVal2Zero( &pstate.value);              // free any allocated memory
   }
   else if (rc == TX_PENDING)                   // evaluation skipped, but OK
   {
      rc = NO_ERROR;
   }
   RETURN (rc);
}                                               // end 'txsRunExpr'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Return descriptive string for script token
/*****************************************************************************/
static char  *txsToken2String
(
   TXST_TYPE           token,                   // IN    token
   char               *text                     // INOUT text buffer
)
{
   switch (token)
   {
      case TXST_CMD:       strcpy( text, "COMMAND"      ); break;
      case TXST_PRAGMA:    strcpy( text, "PRAGMA"       ); break;
      case TXST_IF:        strcpy( text, "IF"           ); break;
      case TXST_ELIF:      strcpy( text, "ELSEIF"       ); break;
      case TXST_ELSE:      strcpy( text, "ELSE"         ); break;
      case TXST_ENDIF:     strcpy( text, "ENDIF"        ); break;
      case TXST_WHILE:     strcpy( text, "WHILE"        ); break;
      case TXST_ENDWHILE:  strcpy( text, "ENDWHILE"     ); break;
      case TXST_DO:        strcpy( text, "DO"           ); break;
      case TXST_UNTIL:     strcpy( text, "UNTIL"        ); break;
      case TXST_LOOP:      strcpy( text, "LOOP"         ); break;
      case TXST_EXITWHEN:  strcpy( text, "EXIT-WHEN"    ); break;
      case TXST_ENDLOOP:   strcpy( text, "ENDLOOP"      ); break;
      case TXST_FOR:       strcpy( text, "FOR"          ); break;
      case TXST_ENDFOR:    strcpy( text, "ENDFOR"       ); break;
      case TXST_BREAK:     strcpy( text, "BREAK"        ); break;
      case TXST_CONTINUE:  strcpy( text, "CONTINUE"     ); break;
      case TXST_RET:       strcpy( text, "RETURN"       ); break;
      case TXST_EOS:       strcpy( text, "END-OF-SCRIPT"); break;
   }
   return (text);
}                                               // end 'txsToken2String'
/*---------------------------------------------------------------------------*/



/*****************************************************************************/
// Handle script-level error message, printf-like interface
/*****************************************************************************/
static void TxsScriptError
(
   TXS_SCRIPT         *pgm,                     // IN    program info
   ULONG               lnr,                     // IN    line number
   char               *fmt,                     // IN    format string (printf)
   ...                                          // IN    variable arguments
)
{
   va_list             varargs;
   TXLN                text;

   va_start( varargs,  fmt);
   if (strlen( pgm->errtxt) == 0)               // don't overwrite any messages,
   {                                            // first is most to-the-point!
      vsprintf( text, fmt, varargs);
      sprintf(  pgm->errtxt, "Line %lu, script error: %.*s", lnr, (TXMAXTM - 25), text);
   }
   va_end(   varargs);
}                                               // end 'TxsScriptError'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Substitute $n parameters in a string by supplied values; n = 0..9 (maximum)
/*****************************************************************************/
ULONG txsSubstituteParams                       // RET   nr of substitutions
(
   char               *string,                  // IN    base string
   char              **values,                  // IN    values to substitute
   int                 count,                   // IN    number of values
   int                 size,                    // IN    size of output buffer
   char               *result                   // OUT   substituted (MAXLIN)
)
{
   ULONG               rc = 0;                  // function return
   char               *bs = string;
   char               *rs = result;
   int                 rl = 0;                  // resulting length
   int                 si;                      // substitution index

   ENTER();
   TRARGS(("string: '%s'\n", string));

   while ((*bs) && (rl < TXMAXLN))
   {
      if ((*bs == '$') && isdigit(*(bs+1)))     // substitution candidate
      {
         bs++;                                  // skip  $
         si  = (int) ((*bs) - '0');             // index 0..9
         if ((si < count) &&
             (values[si] != NULL))              // value available ?
         {
            rl += strlen( values[si]);
            if (rl < size)                      // room to grow ?
            {
               strcpy(  rs,  values[si]);       // copy value
               rs += strlen( values[si]);       // and update destination
            }
         }
         bs++;                                  // skip digit
      }
      else
      {
         *rs++ = *bs++;                         // copy single char
         rl++;
      }
   }
   *rs = '\0';                                  // terminate result string

   TRARGS(("result: '%s'\n", result));
   RETURN (rc);
}                                               // end 'txsSubstituteParams'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Test if script exists, is REXX, return parameter description and full-name
/*****************************************************************************/
BOOL TxsValidateScript                          // RET   script exists
(
   char               *sname,                   // IN    script name
   BOOL               *rexx,                    // OUT   script is REXX
   char               *pdesc,                   // OUT   param description or NULL
   char               *fname                    // OUT   full scriptname or NULL
)
{
   BOOL                rc = FALSE;              // function return
   TXLN                line;
   FILE               *script;
   char               *pp = NULL;               // parameter position

   ENTER();

   if ((script = TxFindAndOpenFile( sname, "PATH", line)) != NULL)
   {
      rc = TRUE;                                // script exists
      if (fname != NULL)                        // full name wanted
      {
         strcpy( fname, line);                  // copy full name
         TRACES(("fname: '%s'\n", fname));
      }
      if (fgets( line, TXMAXLN, script) != NULL)
      {
         if (rexx != NULL)                      // want REXX test ?
         {
            *rexx = (memcmp( line, "/" "*", 2) == 0);
         }
         if (pdesc != NULL)                     // want descriptions ?
         {
            strcpy( pdesc, "");
            if (fgets( line, TXMAXLN, script) != NULL)
            {
               if ((pp = strrchr( line, '~')) != NULL)
               {
                  *(pp+1) = 0;                  // clean up the line
               }
               if ((pp = strchr( line, '~') ) != NULL)
               {
                  TxRepl( pp, '~', '\n');       // make descriptions readable
                  strcpy( pdesc, pp);           // and copy to caller ...
               }
            }
            TRACES(("pdesc: '%s'\n", pdesc));
         }
      }
      fclose( script);
   }
   BRETURN (rc);
}                                               // end 'TxsValidateScript'
/*---------------------------------------------------------------------------*/



#if defined (DEV32)                             // REXX for OS/2 only
#define TX_SIOEXIT  ((PSZ) "tx_sioexit")        // name of exit function

#define TX_REXXDLL  ((PSZ) "REXX")              // name of REXX dll
#define TX_REXXAPI  ((PSZ) "REXXAPI")           // name of REXXAPI dll

PFN txRexxStart;
PFN txRexxRegExe;
PFN txRexxRegExit;
PFN txRexxDeregEx;
PFN txRexxDereg;
PFN txRexxVarPool;

// Route 'say' output through the TxPrint channel
static LONG _System tx_sioexit
(
   LONG               exitnr,                   // IN    exit-id (RXSIO)
   LONG               funcnr,                   // IN    func-id
   PEXIT              parmblock                 // IN    parameters to func
);


/*****************************************************************************/
// Execute a REXX command script using a subcommand environment
/*****************************************************************************/
ULONG TxsRexxRun
(
   char               *name,                    // IN    REXX proc to execute
   char               *args,                    // IN    Arguments
   char               *envname,                 // IN    REXX environment name
   TX_SUBCOM_CALLBACK  subcom                   // IN    subcommand handler
)
{
   ULONG               dr = NO_ERROR;
   LONG                rc;                      /* rc from REXX interpreter  */
   RXSTRING            arg;                     /* argument string for REXX  */
   RXSTRING            rxret;                   /* return value from REXX    */
   SHORT               rexxrc = 0;              /* return code from function */
   TXLN                rxcode;
   HMODULE             rexxdll;                 // handle for REXX dll
   HMODULE             rexxapi;                 // handle for REXXAPI dll
   RXSYSEXIT           exit_list[2];            // list of exit-handlers

   ENTER();

   if (TxaOption('v'))                          // verbose ?
   {
      TxPrint( "\nRUN  REXX  script : '%s %s'\n", name, args);
   }
   dr = DosLoadModule( rxcode, TXMAXLN,         // error buffer and size
                       TX_REXXDLL,              // name of dll
                       &rexxdll);               // module handle
   if (dr == NO_ERROR)
   {
      dr = DosLoadModule( rxcode, TXMAXLN,      // error buffer and size
                          TX_REXXAPI,           // name of dll
                          &rexxapi);            // module handle
      if (dr == NO_ERROR)
      {
         TRACES(("REXX/API mod handles: %lu / %lu\n", rexxdll, rexxapi));
         if ((DosQueryProcAddr(rexxapi, 0,
             "RexxRegisterSubcomExe", (PFN *) &txRexxRegExe ) == NO_ERROR) &&
             (DosQueryProcAddr(rexxapi, 0,
             "RexxRegisterExitExe",   (PFN *) &txRexxRegExit) == NO_ERROR) &&
             (DosQueryProcAddr(rexxapi, 0,
             "RexxDeregisterExit",    (PFN *) &txRexxDeregEx) == NO_ERROR) &&
             (DosQueryProcAddr(rexxapi, 0,
             "RexxDeregisterSubcom",  (PFN *) &txRexxDereg  ) == NO_ERROR) &&
             (DosQueryProcAddr(rexxdll, 0,
             "RexxStart",             (PFN *) &txRexxStart  ) == NO_ERROR) &&
             (DosQueryProcAddr(rexxdll, 0,
             "RexxVariablePool",      (PFN *) &txRexxVarPool) == NO_ERROR)
            )
         {
            dr = txRexxRegExe( envname, subcom, NULL );
            TRACES(("RegSubcomExe RC: %lu\n", (ULONG) dr));
            if (dr == RXSUBCOM_OK)
            {
               dr = txRexxRegExit( TX_SIOEXIT, (PFN) &tx_sioexit, NULL );
               if (dr == RXSUBCOM_OK)
               {
                  exit_list[0].sysexit_name = TX_SIOEXIT;
                  exit_list[0].sysexit_code = RXSIO;
                  exit_list[1].sysexit_code = RXENDLST;

                  rxret.strlength = 0L;         /* force REXX to alloc rxret */
                  MAKERXSTRING(arg, args, strlen(args));

                  TRACES(("RexxStart to be executed\n"));
                  rc = txRexxStart(
                          1,                    /* number of arguments       */
                          &arg,                 /* array of arguments        */
                          (PSZ) name,           /* name of REXX file         */
                          0,                    /* No INSTORE used           */
                          envname,              /* Command env. name         */
                          RXSUBROUTINE,         /* invoked as subroutine     */
                          exit_list,            /* used EXITs on this call   */
                          &rexxrc,              /* Rexx program output       */
                          &rxret );             /* Rexx program output       */
                  if (rc == 0)
                  {
                     if (rexxrc != 0)
                     {
                        dr = (ULONG) rexxrc;
                     }
                  }
                  else
                  {
                     dr = (ULONG) 20000-rc;
                     TxPrint("Rexx interpreter error: %d on macro '%s'\n", rc, name);
                     sprintf( rxcode, "help REX%04.4u", (USHORT) (0-rc));
                     TxExternalCommand( rxcode);
                  }
                  if (rxret.strlength != 0)
                  {
                     //- to be refined, may want to pass that back
                     //- as a string to caller ?

                     TxPrint( "REXX return: '%*.*s'\n",
                               rxret.strlength, rxret.strlength, rxret.strptr);
                  }
                  DosFreeMem(rxret.strptr);     /* Release storage           */
                  txRexxDeregEx( TX_SIOEXIT, NULL);
               }
               else
               {
                  TxPrint("Error registering SAY exit handler\n");
               }
               txRexxDereg( envname, NULL);
            }
            else
            {
               TxPrint("Error registering REXX subcommand handler\n");
            }
         }
         else
         {
            TxPrint("Error resolving REXX API's from: '%s' and/or '%s'\n",
                     TX_REXXDLL, TX_REXXAPI);
         }
         DosFreeModule( rexxapi);
      }
      else
      {
         TxPrint("Error %lu loading Rexx API DLL: '%s'\n", dr, TX_REXXAPI);
      }
      DosFreeModule( rexxdll);
   }
   else
   {
      TxPrint("Error %lu loading Rexx DLL: '%s'\n", dr, TX_REXXDLL);
      TxPrint("Rexx functionality not available\n");
   }
   RETURN (dr);
}                                               // end 'TxsRexxRun'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Route 'say' output through the TxPrint channel
/*****************************************************************************/
static LONG _System tx_sioexit
(
   LONG                exitnr,                  // IN    exit-id (RXSIO)
   LONG                funcnr,                  // IN    func-id
   PEXIT               parmblock                // IN    parameters to func
)
{
   ULONG               dr;
   RXSIOSAY_PARM      *sio = (RXSIOSAY_PARM *) parmblock;

   ENTER();
   TRARGS(("Rexx exit: %ld, subfunction: %ld\n", exitnr, funcnr));

   switch (funcnr)
   {
      case RXSIOSAY:                            // normal Say output
      case RXSIOTRC:                            // trace and error output
         TxPrint( "%s\n", sio->rxsio_string.strptr);
         dr = RXEXIT_HANDLED;
         break;

      default:
         dr = RXEXIT_NOT_HANDLED;
         break;
   }
   RETURN (dr);
}                                               // end 'tx_sioexit'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set number variable in the REXX variable pool using printf syntax
/*****************************************************************************/
ULONG TxsRexxSetnum
(
   char              *name,                     // IN    Name of variable
   ULONG              ulv,                      // IN    value for variable
   char              *fmt                       // IN    format strings
)
{
   ULONG              dr;
   TXLN               value;

   sprintf(value, fmt, ulv);
   TRACES(("Value to rx-set: '%s'\n", value));
   dr = TxsRexxSetvar( name, value, strlen(value));
   return (dr);
}                                               // end 'TxsRexxSetnum'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set a string variable in the REXX variable pool
/*****************************************************************************/
ULONG TxsRexxSetvar
(
   char              *name,                     // IN    Name of variable
   void              *value,                    // IN    ptr to value buffer
   ULONG              length                    // IN    length of value
)
{
   ULONG               dr = 1;
   SHVBLOCK            var;                     // REXX variable pool block
   int                 i  = 9;                  // maximum 9 retries to avoid
                                                // RxVar bug when dynamic
                                                // loading is used
   ENTER();

   var.shvnext      = NULL;
   var.shvcode      = RXSHV_SYSET;
   var.shvnamelen   = strlen(name);
   var.shvvaluelen  = length;
   var.shvret       = 0;
   MAKERXSTRING(var.shvname,  name,  strlen(name));
   MAKERXSTRING(var.shvvalue, value, length);

   TRACES(("Set REXX variable: '%s', length: %lu, value:\n", name, length));
   TRHEXS(70, value, length, "REXX variable contents");
   while (i-- && ((dr = txRexxVarPool(&var)) != 0))
   {
      TRACES(("RexxVariablePool failed, retry!\n"));
   }
   TRACES(("RexxVariablePool rc: %lu, shvret: %lu\n", dr, var.shvret));
   RETURN (dr);
}                                               // end 'TxsRexxSetvar'
/*---------------------------------------------------------------------------*/

#endif
