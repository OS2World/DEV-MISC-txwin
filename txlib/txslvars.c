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
// TX script variable pool implementation, scaler, array and hash
//
// Author: J. van Wijk
//
// Developed for DFSee utility
//

#include <txlib.h>                              // TX library interface


typedef struct txselem                          // list element, var or value
{                                               // or EXPOSED when both NULL
   char               *name;
   TXSVALUE           *value;
   struct txslist     *list;
   struct txselem     *next;
} TXSELEM;                                      // end of struct "txselem"

typedef struct txslist                          // list of variables or values
{
   TXSELEM            *head;
} TXSLIST;                                      // end of struct "txslist"

typedef struct txslevel                         // variable pool level
{
   TXSLIST            *scalar;
   TXSLIST            *array;
   TXSLIST            *hash;
   struct txslevel    *next;
} TXSLEVEL;                                     // end of struct "txslevel"


// Variable pool is a list of local levels, ending at the global level
static TXSLEVEL       *local   = NULL;

static TXS_VAR_CALLBACK  host_callback = NULL;


// Terminate a variable pool list, could be any type, can recurse!
static void txsDelVarPoolList
(
   TXSLIST           **list                     // IN    ref to list to delete
);

// Free variable pool list element including all contents, could be any type
static void txsDelVarPoolElem
(
   TXSELEM           **elem                     // IN    ref to elem to delete
);

// Set given value in specified index in array, expand to index when needed
static ULONG txsSetArrayValue
(
   TXSELEM            *var,                     // IN    variable element
   int                 ord,                     // IN    ordinal for new value
   TXSVALUE           *value                    // IN    value to be assigned
);

// Set given value in for specified index in hash
static ULONG txsSetHashValue
(
   TXSELEM            *var,                     // IN    variable element
   char               *key,                     // IN    key for tuple
   TXSVALUE           *value                    // IN    value to be assigned
);

// Get value reference from specified index in array, when it exists, or NULL
static TXSVALUE *txsGetArrayValue
(
   TXSELEM            *var,                     // IN    variable element
   int                 ord                      // IN    ordinal for new value
);

// Get value reference from specified index in hash, when it exists, or NULL
static TXSVALUE *txsGetHashValue
(
   TXSELEM            *var,                     // IN    variable element
   char               *key                      // IN    key for tuple
);

// Find named variable and optional (array) index through all pool levels
static BOOL txsFindVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   BOOL                read,                    // IN    search for Get
   char               *name,                    // IN    variable name
   TXSVALUE           *index,                   // IN    index, or undef
   TXSELEM           **elem,                    // OUT   ref to element or NULL
   TXSLIST           **list                     // OUT   list head ref  or NULL
);


// Get variable pool nesting level
static ULONG txsGetVarPoolLevel
(
   void
);

// Dump one variable pool list
static void txsDumpVarPoolList
(
   char                vtype,                   // IN    type of variables
   TXSLIST            *list                     // IN    list to dump
);

// Dump one variable pool value, including name
static void txsDumpVarValue
(
   char               *name,                    // IN    variable compund name
   TXSVALUE           *value                    // IN    variable value
);


/*****************************************************************************/
// Initialize variable pool and set host callback
/*****************************************************************************/
ULONG txsInitVariablePool
(
   TXS_VAR_CALLBACK    hostCb                   // IN    callback function for
)                                               //       $_xxx host variables
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   host_callback = hostCb;
   rc = txsAddVarPoolLevel();                   // add the global level
   RETURN (rc);
}                                               // end 'txsInitVariablePool'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Add a local level to the variable pool, empty
/*****************************************************************************/
ULONG txsAddVarPoolLevel
(
   void
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSLEVEL           *new;

   ENTER();

   if ((new = TxAlloc( 1, sizeof( TXSLEVEL))) != NULL)
   {
      if (((new->scalar = TxAlloc( 1, sizeof( TXSLIST))) != NULL) &&
          ((new->array  = TxAlloc( 1, sizeof( TXSLIST))) != NULL) &&
          ((new->hash   = TxAlloc( 1, sizeof( TXSLIST))) != NULL)  )
      {
         new->next = local;
         local = new;                           // all fields are NULL, empty
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   RETURN (rc);
}                                               // end 'txsAddVarPoolLevel'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate variable pool and reset host callback
/*****************************************************************************/
void txsTermVariablePool
(
   void
)
{
   ENTER();

   host_callback = NULL;
   while (local)
   {
      txsDelVarPoolLevel();
   }
   VRETURN();
}                                               // end 'txsTermVariablePool'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate the most local variable pool level (could be global!)
/*****************************************************************************/
void txsDelVarPoolLevel
(
   void
)
{
   TXSLEVEL           *this = local;

   ENTER();

   if (this)
   {
      local = local->next;
      txsDelVarPoolList( &this->scalar);
      txsDelVarPoolList( &this->array);
      txsDelVarPoolList( &this->hash);
      TxFreeMem( this);
   }
   VRETURN();
}                                               // end 'txsDelVarPoolLevel'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Terminate a variable pool list, could be any type, can recurse!
/*****************************************************************************/
static void txsDelVarPoolList
(
   TXSLIST           **list                     // IN    ref to list to delete
)
{
   TXSLIST            *ls;                      // list itself

   ENTER();

   if (*list)
   {
      ls = *list;
      while (ls->head)
      {
         txsDelVarPoolElem( &(ls->head));
      }
      TxFreeMem( *list);
   }
   VRETURN();
}                                               // end 'txsDelVarPoolList'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Free variable pool list element including all contents, could be any type
/*****************************************************************************/
static void txsDelVarPoolElem
(
   TXSELEM           **elem                     // IN    ref to elem to delete
)
{
   TXSELEM            *this;

   ENTER();

   if (*elem)
   {
      this  = *elem;
      *elem = this->next;                       // unlink this element

      TxFreeMem( this->name);                   // free name, if any
      if (this->value)                          // there is a scalar value
      {
         txsVal2Zero( this->value);             // free any string memory
         TxFreeMem(   this->value);             // free value itself
      }
      if (this->list)                           // there are array/hash values
      {
         txsDelVarPoolList( &this->list);       // recurse, delete list
      }
      TxFreeMem( this);
   }
   VRETURN();
}                                               // end 'txsDelVarPoolElem'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get constant numeric value, if defined for TXLIB or by application-callback
/*****************************************************************************/
ULONG txsGetConstant                            // RET   function result
(
   char               *name,                    // IN    possible  constant name
   TXSVALUE           *value                    // OUT   resulting constant value
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSVALUE            nv = TXSNUM_0;           // new result value

   if      (strcasecmp(  name, "true"               ) == 0) nv.num = 1;
   else if (strcasecmp(  name, "false"              ) == 0) nv.num = 0;
   else if (strcasecmp(  name, "rc_ok"              ) == 0) nv.num = NO_ERROR;

   //- Generic OS RC values
   else if (strcasecmp(  name, "rc_file_not_found"  ) == 0) nv.num = ERROR_FILE_NOT_FOUND;
   else if (strcasecmp(  name, "rc_path_not_found"  ) == 0) nv.num = ERROR_PATH_NOT_FOUND;
   else if (strcasecmp(  name, "rc_too_many_files"  ) == 0) nv.num = ERROR_TOO_MANY_OPEN_FILES;
   else if (strcasecmp(  name, "rc_access_denied"   ) == 0) nv.num = ERROR_ACCESS_DENIED;
   else if (strcasecmp(  name, "rc_invalid_handle"  ) == 0) nv.num = ERROR_INVALID_HANDLE;
   else if (strcasecmp(  name, "rc_no_more_files"   ) == 0) nv.num = ERROR_NO_MORE_FILES;
   else if (strcasecmp(  name, "rc_write_protect"   ) == 0) nv.num = ERROR_WRITE_PROTECT;
   else if (strcasecmp(  name, "rc_not_ready"       ) == 0) nv.num = ERROR_NOT_READY;
   else if (strcasecmp(  name, "rc_crc"             ) == 0) nv.num = ERROR_CRC;
   else if (strcasecmp(  name, "rc_seek"            ) == 0) nv.num = ERROR_SEEK;
   else if (strcasecmp(  name, "rc_sector_not_found") == 0) nv.num = ERROR_SECTOR_NOT_FOUND;
   else if (strcasecmp(  name, "rc_write_fault"     ) == 0) nv.num = ERROR_WRITE_FAULT;
   else if (strcasecmp(  name, "rc_read_fault"      ) == 0) nv.num = ERROR_READ_FAULT;
   else if (strcasecmp(  name, "rc_gen_failure"     ) == 0) nv.num = ERROR_GEN_FAILURE;
   else if (strcasecmp(  name, "rc_file_sharing"    ) == 0) nv.num = ERROR_SHARING_VIOLATION;
   else if (strcasecmp(  name, "rc_lock_violation"  ) == 0) nv.num = ERROR_LOCK_VIOLATION;
   else if (strcasecmp(  name, "rc_wrong_disk"      ) == 0) nv.num = ERROR_WRONG_DISK;

   //- TxLib specific values
   else if (strcasecmp(  name, "rc_error"           ) == 0) nv.num = TX_ERROR;
   else if (strcasecmp(  name, "rc_invalid_file"    ) == 0) nv.num = TX_INVALID_FILE;
   else if (strcasecmp(  name, "rc_invalid_path"    ) == 0) nv.num = TX_INVALID_PATH;
   else if (strcasecmp(  name, "rc_access_denied"   ) == 0) nv.num = TX_ACCESS_DENIED;
   else if (strcasecmp(  name, "rc_invalid_handle"  ) == 0) nv.num = TX_INVALID_HANDLE;
   else if (strcasecmp(  name, "rc_invalid_data"    ) == 0) nv.num = TX_INVALID_DATA;
   else if (strcasecmp(  name, "rc_alloc_error"     ) == 0) nv.num = TX_ALLOC_ERROR;
   else if (strcasecmp(  name, "rc_syntax_error"    ) == 0) nv.num = TX_SYNTAX_ERROR;
   else if (strcasecmp(  name, "rc_invalid_drive"   ) == 0) nv.num = TX_INVALID_DRIVE;
   else if (strcasecmp(  name, "rc_pending"         ) == 0) nv.num = TX_PENDING;
   else if (strcasecmp(  name, "rc_failed"          ) == 0) nv.num = TX_FAILED;
   else if (strcasecmp(  name, "rc_write_protect"   ) == 0) nv.num = TX_WRITE_PROTECT;
   else if (strcasecmp(  name, "rc_cmd_unknown"     ) == 0) nv.num = TX_CMD_UNKNOWN;
   else if (strcasecmp(  name, "rc_no_compress"     ) == 0) nv.num = TX_NO_COMPRESS;
   else if (strcasecmp(  name, "rc_no_initialize"   ) == 0) nv.num = TX_NO_INITIALIZE;
   else if (strcasecmp(  name, "rc_aborted"         ) == 0) nv.num = TX_ABORTED;
   else if (strcasecmp(  name, "rc_bad_option_char" ) == 0) nv.num = TX_BAD_OPTION_CHAR;
   else if (strcasecmp(  name, "rc_too_many_args"   ) == 0) nv.num = TX_TOO_MANY_ARGS;
   else if (strcasecmp(  name, "rc_display_change"  ) == 0) nv.num = TX_DISPLAY_CHANGE;
   else if (strcasecmp(  name, "rc_app_quit"        ) == 0) nv.num = TX_APP_QUIT;
   else                                         // not a valid constant
   {
      if (host_callback)                        // application constant
      {
         rc = (host_callback)(name, TXSCONSTANT, &nv);
      }
      else
      {
         rc = TX_ERROR;
      }
   }
   *value = nv;
   return (rc);
}                                               // end 'txsGetConstant'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get value for named variable and optional (array) index
/*****************************************************************************/
ULONG txsGetVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   char               *name,                    // IN    variable name
   TXSVALUE           *index,                   // IN    array/hash index var
   TXSVALUE           *value                    // OUT   new variable value
)
{
   ULONG               rc = TX_FAILED;          // function return, not found
   ULONG               ai = TXSNOINDEX;         // array index
   TXSELEM            *le = NULL;
   TXSVALUE            nv = TXSUNDEF;           // new result value
   TXSVALUE            av = TXSUNDEF;           // alternate  value
   TXSVALUE           *vr = NULL;               // value reference

   ENTER();

   //- to be refined, implement return of #of elements for array and hash variable

   if ((name[0] == '_') && strcasecmp( name, "_rc")) // R/O host variables
   {
      if (strncasecmp( name, "_txs", 4) == 0)   // TXScript system variables
      {
         //- to be refined, may need specific function here for more variables

         if (strcasecmp( name, "_txslevel") == 0) // nesting level
         {
            av.num     = txsGetVarPoolLevel();
            av.defined = TRUE;
            rc = NO_ERROR;
         }
      }
      else if (host_callback)                   // application system variables
      {
         if (index->defined)
         {
            ai = (ULONG) txsVal2Num( index);
         }
         rc = (host_callback)(name, ai, &av);
      }
      vr = &av;
   }
   else                                         // global/local variable
   {
      if (txsFindVariable( prefix, FALSE, name, index, &le, NULL))
      {
         rc = NO_ERROR;
      }
   }
   if (rc == NO_ERROR)
   {
      if (vr == NULL)
      {
         if (index->defined)
         {
            if (index->str)                     // index for a hash
            {
               vr = txsGetHashValue(  le, index->str);
            }
            else                                // index for an array
            {
               vr = txsGetArrayValue( le, (int) index->num);
            }
         }
         else                                   // no index given, scalar context
         {
            if (prefix == '$')                  // scalar value
            {
               vr = le->value;
            }
            else                                // array/hash element
            {
               TXSELEM *e;

               for (e = le->list->head; e; e = e->next)
               {
                  av.num++;                     // count ALL values, including
               }                                // any undef's in an array!
               av.defined = TRUE;
               vr = &av;                        // make av value our result
            }
         }
      }
      if (vr != NULL)
      {
         if (vr->str != NULL)                   // string value ?
         {
            nv.mem = TXSVSIZE( vr->len);
            if ((nv.str = TxAlloc( 1, nv.mem)) != NULL)
            {
               nv.len = vr->len;
               memcpy( nv.str, vr->str, nv.len);
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
         }
         else
         {
            nv.num = vr->num;
         }
         nv.defined  = vr->defined;
      }
   }
   if (av.mem)                                  // if it was allocated
   {
      TxFreeMem( av.str);                       // free any allocated memory
   }
   *value = nv;
   RETURN (rc);
}                                               // end 'txsGetVariable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get value reference from specified index in array, when it exists, or NULL
/*****************************************************************************/
static TXSVALUE *txsGetArrayValue
(
   TXSELEM            *var,                     // IN    variable element
   int                 ord                      // IN    ordinal for value
)
{
   TXSVALUE           *rc = NULL;               // function return
   TXSELEM            *ve;
   int                 i;

   ENTER();
   for (ve = var->list->head, i = 0; ve; ve = ve->next, i++)
   {
      if (i == ord)                             // found existing value
      {
         rc = ve->value;
         break;
      }
   }
   RETURN (rc);
}                                               // end 'txsGetArrayValue'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get value reference from specified index in hash, when it exists, or NULL
/*****************************************************************************/
static TXSVALUE *txsGetHashValue
(
   TXSELEM            *var,                     // IN    variable element
   char               *key                      // IN    key for tuple
)
{
   TXSVALUE           *rc = NULL;               // function return
   TXSELEM            *ve;

   ENTER();
   for (ve = var->list->head; ve; ve = ve->next)
   {
      if (strcmp( ve->name, key) == 0)          // found existing value
      {
         rc = ve->value;
         break;
      }
   }
   RETURN (rc);
}                                               // end 'txsGetHashValue'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set value for named variable and optional (array) index, no host variables!
/*****************************************************************************/
ULONG txsSetVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   char               *name,                    // IN    variable name
   TXSVALUE           *index,                   // IN    array/hash index or NULL
   TXSVALUE           *value                    // IN    new variable value
)                                               //       scalar values only!
{
   ULONG               rc = NO_ERROR;           // function return, not found
   TXSELEM            *le = NULL;
   TXSVALUE           *vr = NULL;               // value reference

   ENTER();

   if ((name[0] == '_') && strcasecmp( name, "_rc")) // R/O host variables
   {
      rc = TX_WRITE_PROTECT;
   }
   else                                         // global/local variable
   {
      if (txsFindVariable( prefix, TRUE, name, index, &le, NULL))
      {
         if ((vr = le->value) == NULL)          // not a scalar value
         {                                      // prepare new array/hash value
            if ((vr = TxAlloc( 1, sizeof( TXSVALUE))) != NULL)
            {
               if (index->str)                  // index for a hash
               {
                  rc = txsSetHashValue( le, index->str, vr);
               }
               else                             // index for an array
               {
                  rc = txsSetArrayValue( le, (int) index->num, vr);
               }
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
         }
      }
      else                                      // create new variable
      {
         switch (prefix)
         {
            case '@':
            case '%':
               rc = TX_FAILED;                  // not implemented yet
               break;

            default:                            // scalar, maybe for array/hash
               if (((le = TxAlloc( 1, sizeof( TXSELEM)))       != NULL) &&
                   ((vr = TxAlloc( 1, sizeof( TXSVALUE)))      != NULL) &&
                   ((le->name = TxAlloc( 1, strlen( name) +1)) != NULL)  )
               {
                  strcpy( le->name, name);
                  if (!index->defined)          // no index defined
                  {
                     le->value = vr;
                     le->next  = local->scalar->head;
                     local->scalar->head = le;
                  }
                  else
                  {
                     if ((le->list = TxAlloc( 1, sizeof( TXSLIST))) != NULL)
                     {
                        if (index->str)         // index for a hash
                        {
                           le->next  = local->hash->head;
                           local->hash->head = le;
                           rc = txsSetHashValue( le, index->str, vr);
                        }
                        else                    // index for an array
                        {
                           le->next  = local->array->head;
                           local->array->head = le;
                           rc = txsSetArrayValue( le, (int) index->num, vr);
                        }
                     }
                     else
                     {
                        rc = TX_ALLOC_ERROR;
                     }
                  }
               }
               else
               {
                  TxFreeMem( vr);               // cleanup partial alloc
                  TxFreeMem( le);
                  rc = TX_ALLOC_ERROR;
               }
               break;
         }
      }
      if ((rc == NO_ERROR) && (vr != NULL))
      {
         txsVal2Zero( vr);                      // free any allocated memory
         if (value->str != NULL)                // string value ?
         {
            vr->mem = TXSVSIZE(value->len);
            if ((vr->str = TxAlloc( 1, vr->mem)) != NULL)
            {
               memcpy( vr->str, value->str, value->len);
               vr->len = value->len;
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
         }
         else
         {
            vr->num = value->num;
         }
         vr->defined = TRUE;                    // value is defined now
      }
   }
   RETURN (rc);
}                                               // end 'txsSetVariable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set given value in specified index in array, expand to index when needed
/*****************************************************************************/
static ULONG txsSetArrayValue
(
   TXSELEM            *var,                     // IN    variable element
   int                 ord,                     // IN    ordinal for new value
   TXSVALUE           *value                    // IN    value to be assigned
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSELEM            *ve = NULL;
   int                 size;

   ENTER();

   for (ve = var->list->head, size = 0; ve; size++)
   {
      if (size == ord)                          // found existing value, update
      {
         txsVal2Zero( ve->value);               // free any string memory
         TxFreeMem  ( ve->value);               // and value itself
         ve->value = value;                     // assign new value
         size++;                                // minimum size now
         break;
      }
      if (ve->next)
      {
         ve = ve->next;
      }
      else
      {
         size++;                                // count current last element
         break;
      }
   }
   if ((ve == NULL) && (size == 0))             // array was empty, create
   {
      if ((ve = TxAlloc( 1, sizeof(TXSELEM))) != NULL)
      {
         var->list->head = ve;                  // new empty list-head (undef)
         if (ord == 0)
         {
            ve->value = value;                  // assign new value
         }
         size = 1;
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   while (ve && (size <= ord))                  // expand array, and assign
   {
      if ((ve->next = TxAlloc( 1, sizeof(TXSELEM))) != NULL)
      {
         ve = ve->next;                         // new empty element (undef)
      }
      else
      {
         rc = TX_ALLOC_ERROR;
         break;
      }
      if (size++ == ord)                        // was this the final one?
      {
         ve->value = value;                     // assign new value
      }
   }
   RETURN (rc);
}                                               // end 'txsSetArrayValue'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set given value in for specified index in hash
/*****************************************************************************/
static ULONG txsSetHashValue
(
   TXSELEM            *var,                     // IN    variable element
   char               *key,                     // IN    key for tuple
   TXSVALUE           *value                    // IN    value to be assigned
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSELEM            *ve = NULL;

   ENTER();

   for (ve = var->list->head; ve; ve = ve->next)
   {
      if (strcmp( ve->name, key) == 0)          // found existing value, update
      {
         txsVal2Zero( ve->value);               // free any string memory
         TxFreeMem  ( ve->value);               // and value itself
         ve->value = value;                     // assign new value
         break;
      }
   }
   if (ve == NULL)                              // tuple not found, create new
   {
      if (((ve       = TxAlloc( 1, sizeof(TXSELEM))) != NULL) &&
          ((ve->name = TxAlloc( 1, strlen(key) +1) ) != NULL)  )
      {
         strcpy( ve->name, key);                // set tuple key
         ve->value = value;                     // assign value
         ve->next  = var->list->head;           // hang list off new element
         var->list->head = ve;                  // new empty list-head (undef)
      }
      else
      {
         TxFreeMem( ve);
         rc = TX_ALLOC_ERROR;
      }
   }
   RETURN (rc);
}                                               // end 'txsSetHashValue'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set value for named variable and optional (array) index, no host variables!
/*****************************************************************************/
ULONG txsDelVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   char               *name,                    // IN    variable name
   TXSVALUE           *index                    // IN    array/hash index or undef
)
{
   ULONG               rc = NO_ERROR;           // function return, not found
   TXSLIST            *ls = NULL;
   TXSELEM            *le = NULL;
   TXSELEM            *e;

   ENTER();

   if (name[0] == '_')                          // host variable name
   {
      rc = TX_WRITE_PROTECT;
   }
   else                                         // global/local variable
   {
      if (txsFindVariable( prefix, TRUE, name, index, &le, &ls))
      {
         if (index->defined)                    // scalar in array/hash
         {
            if (index->str)                     // index for a hash
            {
               rc = TX_FAILED;                  // not supported (yet)
            }
            else                                // index for an array
            {
               rc = txsSetArrayValue( le, (int) index->num, NULL);
            }
         }
         else                                   // scalar or whole array/hash
         {
            if (le == ls->head)                 // it is the first element
            {
               txsDelVarPoolElem( &(ls->head));
            }
            else                                // further in the list
            {
               for (e = ls->head; e; e = e->next)
               {
                  if (le == e->next)            // find ptr to the element
                  {
                     txsDelVarPoolElem( &(e->next));
                     break;
                  }
               }
            }
         }
      }
   }
   RETURN (rc);
}                                               // end 'txsDelVariable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find named variable and optional (array) index through all pool levels
/*****************************************************************************/
static BOOL txsFindVariable
(
   char                prefix,                  // IN    var prefix $, @ or %
   BOOL                update,                  // IN    search for Set/Del
   char               *name,                    // IN    variable name
   TXSVALUE           *index,                   // IN    index, or undef
   TXSELEM           **elem,                    // OUT   ref to element or NULL
   TXSLIST           **list                     // OUT   list head ref  or NULL
)
{
   ULONG               rc = FALSE;              // function return, not found
   TXSLIST            *vl;                      // variable list to search
   TXSLEVEL           *lev;                     // pool level searched
   TXSELEM            *var;                     // variable in a list
   BOOL                exposed = FALSE;         // extern declarator found

   ENTER();

   *elem = NULL;
   for (lev = local; lev; lev = lev->next)
   {
      switch (prefix)
      {
         case '@': vl = lev->array; break;
         case '%': vl = lev->hash;  break;
         default:
            vl = (index->defined) ? (index->str) ? lev->hash : lev->array : lev->scalar;
            break;
      }
      for (var = vl->head; var; var = var->next)
      {
         if (strcasecmp( name, var->name) == 0) // name matched
         {
            if (var->value || var->list)        // real variable
            {
               if (list)
               {
                  *list = vl;                   // return list too (DEL)
               }
               *elem = var;                     // return variable itself
               rc    = TRUE;
               break;
            }
            else                                // EXPOSED declaration
            {
               exposed = TRUE;
               break;
            }
         }
      }
      if (rc || (update && !exposed))           // next level not allowed
      {                                         // unless exposed was found
         break;
      }
   }
   BRETURN (rc);
}                                               // end 'txsFindVariable'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Dump variable pool contents for debugging purposes
/*****************************************************************************/
void txsDumpVarPool
(
   void
)
{
   TXSLEVEL           *level;
   int                 nesting = 0;

   for (level = local; level; level = level->next, nesting++);
   for (level = local; level; level = level->next)
   {
      TxPrint( "\nVariable dump for pool level: %ld\n", --nesting);

      txsDumpVarPoolList( '$', level->scalar);
      txsDumpVarPoolList( '@', level->array);
      txsDumpVarPoolList( '%', level->hash);
   }
}                                               // end 'txsDumpVarPool'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get variable pool nesting level
/*****************************************************************************/
static ULONG txsGetVarPoolLevel
(
   void
)
{
   TXSLEVEL           *level   = local;
   ULONG               nesting = 0;

   while (level)
   {
      if ((level = level->next) != NULL)
      {
         nesting++;
      }
      else
      {
         break;
      }
   }
   return(nesting);
}                                               // end 'txsDumpVarPool'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Dump one variable pool list
/*****************************************************************************/
static void txsDumpVarPoolList
(
   char                vtype,                   // IN    type of variables
   TXSLIST            *list                     // IN    list to dump
)
{
   TXSELEM            *var;                     // variable element
   TXSELEM            *val;                     // value    element
   TXLN                vfull;                   // full variable name
   int                 ord;

   for (var = list->head; var; var = var->next)
   {
      if      ((var->value) && (var->value->defined)) // scalar
      {
         sprintf( vfull, "%-30s <SCALAR>", var->name);
         txsDumpVarValue( vfull, var->value);
      }
      else if (var->list)                       // array or hash
      {
         TxPrint( " %c%-30s <%s>\n", vtype, var->name, (vtype == '@') ? "ARRAY" : "HASH");
         for (val = var->list->head, ord = 0; val; val = val->next, ord++)
         {
            if ((val->value) && (val->value->defined)) // skip undef list elements
            {
               if (val->name)                   // named value, hash
               {
                  sprintf( vfull, "%s{'%s'}", var->name, val->name);
               }
               else
               {
                  sprintf( vfull, "%s[%d]",   var->name, ord);
               }
               txsDumpVarValue( vfull, val->value);
            }
         }
      }
      else                                      // EXPOSED
      {
         TxPrint( " %c%-30s : EXPOSE\n", vtype, var->name);
      }
   }
}                                               // end 'txsDumpVarPoolList'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Dump one variable pool value, including name
/*****************************************************************************/
static void txsDumpVarValue
(
   char               *name,                    // IN    variable compund name
   TXSVALUE           *value                    // IN    variable value
)
{
   TXTM                num;                     // string repr of numeric

   if (value->str)
   {
      TxPrint( " $%-40s : '%s'\n", name,  value->str);
   }
   else
   {
      sprintf( num, "0x%16.16llx = %lld", value->num, value->num);
      TxPrint( " $%-40s : %s\n",   name,  num);
   }
}                                               // end 'txsDumpVarValue'
/*---------------------------------------------------------------------------*/

