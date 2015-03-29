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
// TX library selection-list manager
//
// Author: J. van Wijk
//
// JvW  26-06-2003 Initial version

#include <txlib.h>                              // public interface

// Global communication from sort function to qsort callback (NOT thread-safe!)
static USHORT sortAlphaColumn;
static BOOL   sortDescending;
static BOOL   sortCaseIgnore;


// Sort selection list on the 'text' item contents, at column 1..n or last-dot
static BOOL TxSelSortText                       // RET   TRUE if list changed
(
   TXSELIST           *list,                    // INOUT selection list
   USHORT              spec                     // IN    sort index to get column
);

// Compare SELIST items on 'text' from last dot for qsort, case insensitive
static int txSelCompTextLastDot
(
   const void         *one,
   const void         *two
);

// Compare SELIST items on 'text' string for qsort ascend/descend, case sensitive
static int txSelCompTextString
(
   const void         *one,
   const void         *two
);

// Sort selection list on the 'desc' item contents, at specified column 1..n
static BOOL TxSelSortDesc                       // RET   TRUE if list changed
(
   TXSELIST           *list,                    // INOUT selection list
   USHORT              spec                     // IN    sort index to get column
);

// Compare SELIST items on 'desc' from last dot for qsort, case insensitive
static int txSelCompDescLastDot
(
   const void         *one,
   const void         *two
);

// Compare SELIST items on 'desc' string for qsort ascend/descend, case insensitive
static int txSelCompDescString
(
   const void         *one,
   const void         *two
);

// Compare SELIST items on userdata value for qsort ascending
static int txSelCompUserData
(
   const void         *one,
   const void         *two
);


// Save current list 'selected' value as flag in the item 'bits' field
static void txSelSaveSelected
(
   TXSELIST           *list                     // INOUT selection list
);

// Restore current list 'selected' value from flag in the item 'bits' field
static void txSelRestoreSelected
(
   TXSELIST           *list                     // INOUT selection list
);


// Compare and validate SELIST items on 'text' and 'desc' strings
static int txSelCompValidateItems               // RET    0 if 2 valid strings
(
   TXS_ITEM           *i1,                      // IN     first item
   TXS_ITEM           *i2                       // IN     second item
);

/*****************************************************************************/
// Get length of longest string in a list
/*****************************************************************************/
ULONG TxSelGetMaxLength                         // RET   longest string length
(
   TXSELIST           *list                     // IN    selection list
)
{
   ULONG               rc = 0;                  // function return
   TXS_ITEM           *item;
   int                 l;

   ENTER();

   if (list != NULL)
   {
      for (l = 0; l < list->count; l++)
      {
         if ((item = list->items[l]) != NULL)
         {
            if (item->text != NULL)             // might be NULL (separator)
            {
               if (strlen( item->text) > rc)
               {
                  rc = strlen( item->text);
               }
               TRACES(("Item %2u len:%2u '%s'\n", l, strlen(item->text), item->text));
            }
            else
            {
               TRACES(("Item %2u len: 0  Probably a separator\n", l));
            }
         }
      }
   }
   RETURN (rc);
}                                               // end 'TxSelGetMaxLength'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Scroll visible list up by one line (towards lower index)
/*****************************************************************************/
ULONG TxSelScrollUp                             // RET   scrolling succeeded
(
   TXSELIST           *list                     // INOUT selection list
)
{
   ULONG               rc = NO_ERROR;              // function return

   ENTER();

   if ((list->vsize > 1) && (list->top != 0))   // scroll-up possible ?
   {
      if (list->renderNewItem)                  // render-new available ?
      {
         ULONG     index = list->vsize -1;      // index of discarded item
         TXS_ITEM *temp  = list->items[index];  // keep item allocation

         txSelItemDestroy( list->astatus & TXS_AS_DSTRINGS, &temp);

         memmove( &(list->items[1]),            // destination of move
                  &(list->items[0]),            // source of move
                   (list->vsize -1) * sizeof(TXS_ITEM *));

         list->items[0] = temp;                 // re-use same item struct
         rc = (list->renderNewItem)( list->items[0],
                                     NULL, list->top -1);
      }
      list->top--;                              // update visible top position
   }
   else
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'TxSelScrollUp'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Scroll visible list down by one line (towards higher index)
/*****************************************************************************/
ULONG TxSelScrollDown                           // RET   scrolling succeeded
(
   TXSELIST           *list                     // INOUT selection list
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if (             (list->vsize  > 1) &&
       ((list->top + list->vsize) < list->count)) // scroll-down possible ?
   {
      if (list->renderNewItem)                  // render-new available ?
      {
         ULONG         index = list->vsize -1;  // index of new item
         TXS_ITEM     *temp  = list->items[0];  // keep item allocation

         txSelItemDestroy( list->astatus & TXS_AS_DSTRINGS, &temp);

         memmove( &(list->items[0]),            // destination of move
                  &(list->items[1]),            // source of move
                   (list->vsize -1) * sizeof(TXS_ITEM *));

         list->items[index] = temp;             // re-use same item struct
         rc = (list->renderNewItem)( list->items[index],
                                     NULL, list->top + index +1);
      }
      list->top++;                              // update visible top position
   }
   else
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'TxSelScrollDown'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set visible list to start at specified external index; Discard old items
/*****************************************************************************/
ULONG TxSelSetPosition                          // RET   resulting top index
(
   TXSELIST           *list,                    // INOUT selection list
   ULONG               index                    // IN    top external index
)
{
   ULONG               newtop = index;          // function return

   ENTER();

   if ((newtop + list->vsize) > list->count)    // set top beyond end list ?
   {
      if (list->count > list->vsize)
      {
         newtop = list->count - list->vsize;    // set to end
      }
      else
      {
         newtop = 0;                            // end equals beginning ...
      }
   }
   if (list->renderNewItem)                     // render-new available ?
   {
      ULONG         i;

      for (i = 0; i < list->vsize; i++)
      {
         TXS_ITEM     *item  = list->items[i];  // keep item allocation

         if (item == NULL)                      // not allocated yet
         {
            list->astatus |= TXS_AS_DYNITEMS;
            item = TxAlloc( 1, sizeof( TXS_ITEM));
         }
         else                                   // discard string values
         {
            txSelItemDestroy( list->astatus & TXS_AS_DSTRINGS, &item);
         }
         if (item != NULL)
         {
            (list->renderNewItem)( item, NULL, list->top + i);
         }
         list->items[i] = item;
      }
   }
   list->top = newtop;                          // update visible top position
   RETURN (newtop);
}                                               // end 'TxSelSetPosition'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set selection for specified index, and/or bits if multiple select possible
/*****************************************************************************/
ULONG TxSelSetSelected                          // RET   resultcode
(
   TXSELIST           *list,                    // INOUT selection list
   ULONG               index,                   // IN    external index
   TXSETRESET          action,                  // IN    set, reset or toggle
   BYTE                mask                     // IN    bits to set/reset
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if (index < list->tsize)
   {
      if (list->selarray != NULL)               // multiple select OK ?
      {
         BYTE  old = list->selarray[index];
         BYTE  new;

         switch (action)
         {
            case TX_SET:   new = old |  mask; break;
            case TX_RESET: new = old & ~mask; break;
            default:       new = old ^  mask; break;
         }
         if ((old == 0) && (new != 0) && (list->selected < list->tsize))
         {
            list->selected++;
         }
         if ((old != 0) && (new == 0) && (list->selected != 0))
         {
            list->selected--;
         }
         list->selarray[index] = new;
      }
      else
      {
         list->selected = (action == TX_RESET) ? 0 : index;
      }
   }
   else
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'TxSelSetSelected'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Itterate over selected list items, calback all with specified mask set
/*****************************************************************************/
ULONG TxSelItterateSelected
(
   TXSELIST           *list,                    // INOUT selection list
   TXS_ITEMHANDLER     selected,                // IN    callback selected
   BYTE                mask                     // IN    selection mask
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXS_ITEM            item;

   ENTER();

   if (list->selarray != NULL)                  // multiple select OK ?
   {
      if (list->selected != 0)
      {
         ULONG            i;

         for (i = 0; i < list->tsize; i++)      // visit all list entries
         {
            if ((list->selarray[i] & mask) == mask) // matches selection ?
            {
               if (list->vsize == list->tsize)  // all info available
               {
                  (selected)(list->items[i], &(list->selarray[i]), i);
               }
               else if (list->renderNewItem)
               {
                  (list->renderNewItem)(&item, &(list->selarray[i]), i);
                  (selected)(&item, NULL, i);
               }
               else
               {
                  (selected)( NULL, NULL, list->selected);
               }
            }
         }
      }
      else                                      // nothing selected
      {
         rc = TX_INVALID_DATA;
      }
   }
   else                                         // callback for selected
   {
      if (list->vsize == list->tsize)           // all info available
      {
         (selected)(list->items[list->selected], NULL, list->selected);
      }
      else if (list->renderNewItem)
      {
         (list->renderNewItem)(&item, NULL, list->selected);
         (selected)(&item, NULL, list->selected);
      }
      else
      {
         (selected)( NULL, NULL, list->selected);
      }
   }
   RETURN (rc);
}                                               // end 'TxSelItterateSelected'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set 'selected' to item with specified selection char (at text[index-1])
/*****************************************************************************/
ULONG TxSelCharSelect                           // RET   index now selected
(
   TXSELIST           *list,                    // INOUT selection list
   char                select                   // IN    selection character
)
{
   ULONG               rc = list->selected;
   TXS_ITEM           *item;

   ENTER();
   TRACES(("list:%8.8lx select: '%c'\n", list, select));

   if (list->selarray == NULL)                  // single-select only
   {
      ULONG            i;

      for (i = 0; i < list->count; i++)         // visit list entries
      {
         if ((item = list->items[i]) != NULL)
         {
            if (item->index != 0)               // selection char there ?
            {
               if (list->items[i]->text != NULL) // has text string ?
               {
                  if (item->text[ item->index -1] == select)
                  {
                     list->selected = i;        // set new selection
                     rc = i;
                     break;                     // end the search
                  }
               }
            }
         }
      }
   }
   RETURN (rc);
}                                               // end 'TxSelCharSelect'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set 'selected' to item where text starts with 'string' (case-insensitive)
/*****************************************************************************/
ULONG TxSelStringSelect                         // RET   index now selected
(
   TXSELIST           *list,                    // INOUT selection list
   char               *select,                  // IN    selection string
   int                 length                   // IN    significant length
)
{
   ULONG               rc = 0;
   TXS_ITEM           *item;

   ENTER();
   TRACES(( "list:%8.8lx  select:'%s'  length:%u\n", list, select, length));

   if (list != NULL)
   {
      rc = list->selected;
      TRACES(("list:%8.8lx select: '%s'\n", list, select));

      if (list->selarray == NULL)               // single-select only
      {
         ULONG            i;

         for (i = 0; i < list->count; i++)      // visit list entries
         {
            if ((item = list->items[i]) != NULL) // existing item ?
            {
               if (list->items[i]->text != NULL) // has text string ?
               {
                  if (strncasecmp( list->items[i]->text, select, length) == 0)
                  {
                     list->selected = i;        // set new selection
                     rc = i;
                     break;                     // end the search
                  }
               }
            }
         }
      }
   }
   RETURN (rc);
}                                               // end 'TxSelStringSelect'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Sort the list on text or desc string according to given sort specification
/*****************************************************************************/
BOOL TxSelSortString                            // RET   TRUE if list changed
(
   TXSELIST           *list,                    // INOUT selection list
   USHORT              spec                     // IN    sort index to get column
)
{
   BOOL                rc = FALSE;
   USHORT              sorting;

   ENTER();

   if (list && (list->sortinfo))
   {
      sorting   = list->sortinfo->sort[ spec & TXS_SORT_IDXMASK];
      TRACES(("List %8.8lx, spec:%4.4hx sorting:%4.4hx\n", list, spec, sorting));

      if ((sorting & TXS_SORT_DESC) == TXS_SORT_DESC)
      {
         rc = TxSelSortDesc( list, spec);
      }
      else                                      // SORT_TEXT
      {
         rc = TxSelSortText( list, spec);
      }
   }
   BRETURN(rc);
}                                               // end 'TxSelSortString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Sort selection list on the 'text' item contents, at column 1..n or last-dot
/*****************************************************************************/
static BOOL TxSelSortText                       // RET   TRUE if list changed
(
   TXSELIST           *list,                    // INOUT selection list
   USHORT              spec                     // IN    sort index to get column
)
{
   BOOL                rc = FALSE;
   USHORT              sortinfo;

   ENTER();

   if (list && (list->sortinfo))
   {
      sortinfo = list->sortinfo->sort[spec];
      TRACES(( "list:%8.8lx spec:%4.4hx  count:%5lu  sortinfo:0x%4.4hx\n", list, spec, list->count, sortinfo));
      if ((sortinfo != 0) && (list->count > 1)) // text sort column specified, and items
      {
         txSelSaveSelected( list);              // save selection

         sortDescending  = ((sortinfo & TXS_SORT_DESCEND) == TXS_SORT_DESCEND);
         sortCaseIgnore  = ((sortinfo & TXS_SORT_IGNCASE) == TXS_SORT_IGNCASE);

         if ((sortinfo & TXS_SORT_LASTDOT) == TXS_SORT_LASTDOT)
         {
            //- special sort, from last dot (file-extension or decimal fraction)
            qsort( list->items, (size_t) list->count,
                   sizeof(TXS_ITEM *), txSelCompTextLastDot);
         }
         else
         {
            sortAlphaColumn = sortinfo & TXS_SORT_COLUMN;

            //- alphanumeric sort on specified text column, ascending or descending
            qsort( list->items, (size_t) list->count,
                   sizeof(TXS_ITEM *), txSelCompTextString);
         }
         txSelRestoreSelected( list);           // restore selection
         rc = TRUE;
      }
      for (sortinfo = 0; sortinfo < (list->count & 0xffff); sortinfo++)
      {
         TRACES(("% 3hu = '%s'\n", sortinfo, list->items[ sortinfo]->text));
      }
      list->sortinfo->sort[ TXS_SORT_CURRENT] = spec;
   }
   BRETURN(rc);
}                                               // end 'TxSelSortText'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare SELIST items on 'text' from last dot for qsort, case insensitive
/*****************************************************************************/
static int txSelCompTextLastDot
(
   const void         *one,
   const void         *two
)
{
   int                 rc;
   TXS_ITEM           *i1 = *((TXS_ITEM **) one); // one points to an item ptr!
   TXS_ITEM           *i2 = *((TXS_ITEM **) two);

   if ((rc = txSelCompValidateItems( i1, i2)) == 0)
   {
      char            *e1 = strrchr( i1->text, '.');
      char            *e2 = strrchr( i2->text, '.');

      if (e1 == NULL)
      {
         if (e2 == NULL)
         {
            if (sortCaseIgnore)                 // none has dot, compare whole text
            {
               rc = strcasecmp( i1->text, i2->text);
            }
            else
            {
               rc = strcmp(  i1->text, i2->text);
            }
         }
         else
         {
            rc = -1;                            // with dot is considered 'larger'
         }
      }
      else
      {
         if (e2 == NULL)
         {
            rc = +1;                            // no dot is considered 'smaller'
         }
         else
         {                                      // both have dots, compare from that
            if (sortCaseIgnore)
            {
               rc = strcasecmp( e1, e2);
            }
            else
            {
               rc = strcmp(  e1, e2);
            }
         }
      }
      if (sortDescending)
      {
         rc = (-rc);
      }

      TRACES(("%+3d  i1: 0x%P i2: 0x%P  e1:%-16.16s  e2:%16.16s\n",
               rc,   i1,      i2,       e1,          e2));
   }
   return(rc);
}                                               // end 'txSelCompTextLastDot'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare SELIST items on 'text' string for qsort ascend/descend, case sensitive
/*****************************************************************************/
static int txSelCompTextString
(
   const void         *one,
   const void         *two
)
{
   int                 rc = 0;
   TXS_ITEM           *i1 = *((TXS_ITEM **) one); // one points to an item ptr!
   TXS_ITEM           *i2 = *((TXS_ITEM **) two);

   if ((rc = txSelCompValidateItems( i1, i2)) == 0)
   {
      if (sortCaseIgnore)
      {
         rc = strcasecmp( i1->text + sortAlphaColumn -1, i2->text + sortAlphaColumn -1);
      }
      else
      {
         rc = strcmp(  i1->text + sortAlphaColumn -1, i2->text + sortAlphaColumn -1);
      }
      if (sortDescending)
      {
         rc = (-rc);
      }

      TRACES(("%+3d i1:%P i2:%P  s1:%-20.20s  s2:%-20.20s\n",
               rc,  i1,   i2,   i1->text + sortAlphaColumn -1, i2->text + sortAlphaColumn -1));
   }
   return(rc);
}                                               // end 'txSelCompTextString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Sort selection list on the 'desc' item contents, at specified column 1..n
/*****************************************************************************/
static BOOL TxSelSortDesc                       // RET   TRUE if list changed
(
   TXSELIST           *list,                    // INOUT selection list
   USHORT              spec                     // IN    sort index to get column
)
{
   BOOL                rc = FALSE;
   USHORT              sortinfo;

   ENTER();

   if (list && (list->sortinfo))
   {
      sortinfo = list->sortinfo->sort[spec];
      TRACES(( "list:%8.8lx spec:%4.4hx count:%5lu  sortinfo:0x%4.4hx\n", list, spec, list->count, sortinfo));
      if ((sortinfo != 0) && (list->count > 1)) // Desc sort column specified, and items
      {
         txSelSaveSelected( list);              // save selection

         sortDescending  = ((sortinfo & TXS_SORT_DESCEND) == TXS_SORT_DESCEND);
         sortCaseIgnore  = ((sortinfo & TXS_SORT_IGNCASE) == TXS_SORT_IGNCASE);

         if ((sortinfo & TXS_SORT_LASTDOT) == TXS_SORT_LASTDOT)
         {
            //- special sort, from last dot (file-extension or decimal fraction)
            qsort( list->items, (size_t) list->count,
                   sizeof(TXS_ITEM *), txSelCompDescLastDot);
         }
         else
         {
            sortAlphaColumn = sortinfo & TXS_SORT_COLUMN;

            TRACES(("sorting on column: %hu\n", sortAlphaColumn));

            //- alphanumeric sort on specified text column, ascending or descending
            qsort( list->items, (size_t) list->count,
                   sizeof(TXS_ITEM *), txSelCompDescString);
         }
         txSelRestoreSelected( list);           // restore selection
         rc = TRUE;
      }
      for (sortinfo = 0; sortinfo < (list->count & 0xffff); sortinfo++)
      {
         TRACES(("% 3hu = '%s'\n", sortinfo, list->items[ sortinfo]->text));
      }
      list->sortinfo->sort[ TXS_SORT_CURRENT] = spec;
   }
   BRETURN(rc);
}                                               // end 'TxSelSortDesc'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare SELIST items on 'desc' from last dot for qsort, case insensitive
/*****************************************************************************/
static int txSelCompDescLastDot
(
   const void         *one,
   const void         *two
)
{
   int                 rc;
   TXS_ITEM           *i1 = *((TXS_ITEM **) one); // one points to an item ptr!
   TXS_ITEM           *i2 = *((TXS_ITEM **) two);

   if ((rc = txSelCompValidateItems( i1, i2)) == 0)
   {
      char            *e1 = strrchr( i1->desc, '.');
      char            *e2 = strrchr( i2->desc, '.');

      if (e1 == NULL)
      {
         if (e2 == NULL)
         {
            if (sortCaseIgnore)                 // none has dot, compare whole text
            {
               rc = strcasecmp( i1->desc, i2->desc);
            }
            else
            {
               rc = strcmp(  i1->desc, i2->desc);
            }
         }
         else
         {
            rc = -1;                            // with dot is considered 'larger'
         }
      }
      else
      {
         if (e2 == NULL)
         {
            rc = +1;                            // no dot is considered 'smaller'
         }
         else
         {                                      // both have dots, compare from that
            if (sortCaseIgnore)
            {
               rc = strcasecmp( e1, e2);
            }
            else
            {
               rc = strcmp(  e1, e2);
            }
         }
      }
      if (sortDescending)
      {
         rc = (-rc);
      }

      TRACES(("%+3d  i1: 0x%P i2: 0x%P  e1:%-16.16s  e2:%16.16s\n",
               rc,   i1,      i2,       e1,          e2));
   }
   return(rc);
}                                               // end 'txSelCompDescLastDot'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare SELIST items on 'desc' string for qsort ascend/descend, case insensitive
/*****************************************************************************/
static int txSelCompDescString
(
   const void         *one,
   const void         *two
)
{
   int                 rc;
   TXS_ITEM           *i1 = *((TXS_ITEM **) one); // one points to an item ptr!
   TXS_ITEM           *i2 = *((TXS_ITEM **) two);

   if ((rc = txSelCompValidateItems( i1, i2)) == 0)
   {
      TRACES(("s1:'%s'  s2:'%s'\n", i1->desc, i2->desc));
      if (sortCaseIgnore)
      {
         rc = strcasecmp( i1->desc + sortAlphaColumn -1, i2->desc + sortAlphaColumn -1);
      }
      else
      {
         rc = strcmp(  i1->desc + sortAlphaColumn -1, i2->desc + sortAlphaColumn -1);
      }
      if (sortDescending)
      {
         rc = (-rc);
      }
      TRACES(("%+3d i1:%P i2:%P  s1:%-20.20s  s2:%-20.20s\n",
               rc,  i1,   i2,   i1->desc + sortAlphaColumn -1, i2->desc + sortAlphaColumn -1));
   }
   return(rc);
}                                               // end 'txSelCompDescString'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare and validate SELIST items on 'text' and 'desc' strings
/*****************************************************************************/
static int txSelCompValidateItems               // RET    0 if 2 valid strings
(
   TXS_ITEM           *i1,                      // IN     first item
   TXS_ITEM           *i2                       // IN     second item
)
{
   int                 rc = 0;

   if      ((i1->text == NULL) || (i1->desc == NULL))
   {
      rc = -1;
   }
   else if ((i2->text == NULL) || (i2->desc == NULL))
   {
      rc = +1;
   }
   if (rc)
   {
      TRACES(("%+3d i1:%P i2:%P --------- separator --------\n", rc, i1, i2));
   }
   return(rc);
}                                               // end 'txSelCompValidateItems'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Sort a list ascending on 'userdata'  (unsorts if that contains item-nrs)
/*****************************************************************************/
BOOL TxSelSortUserData                          // RET   TRUE if list changed
(
   TXSELIST           *list                     // INOUT selection list
)
{
   BOOL                rc = FALSE;

   ENTER();
   TRACES(( "list:%8.8lx\n", list));

   if (list && (list->sortinfo))
   {
      if (list->count > 1)
      {
         txSelSaveSelected( list);              // save selection

         TRACES(("Items ptr: 0x%P, items in list: %lu\n", list->items, list->count));

         qsort( list->items, (size_t) list->count,
                sizeof(TXS_ITEM *), txSelCompUserData);

         txSelRestoreSelected( list);           // restore selection
         rc = TRUE;
      }
      list->sortinfo->sort[ TXS_SORT_CURRENT] = TXS_SORT_USERDATA;
   }
   BRETURN(rc);
}                                               // end 'TxSelSortUserData'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Compare SELIST items on userdata value for qsort ascending
/*****************************************************************************/
static int txSelCompUserData
(
   const void         *one,
   const void         *two
)
{
   int                 rc = 0;
   TXS_ITEM           *i1 = *((TXS_ITEM **) one); // one points to an item ptr!
   TXS_ITEM           *i2 = *((TXS_ITEM **) two);

   if      (i1->userdata < i2->userdata)
   {
      rc = -1;
   }
   else if (i1->userdata > i2->userdata)
   {
      rc = +1;
   }

   TRACES(("%+1d  i1:%P i2:%P  seq1:% 5lu  seq2:% 5lu\n",
            rc,   i1,   i2,   i1->userdata, i2->userdata));

   return(rc);
}                                               // end 'txSelCompUserData'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Save current list 'selected' value as flag in the item 'bits' field
/*****************************************************************************/
static void txSelSaveSelected
(
   TXSELIST           *list                     // INOUT selection list
)
{
   ULONG               i;

   ENTER();

   for (i = 0; i < list->count; i++)
   {
      list->items[ i]->bits &= ~TXS_RESERVED_SELECTED;
   }
   list->items[ list->selected]->bits |= TXS_RESERVED_SELECTED;
   VRETURN();
}                                               // end 'txSelSaveSelected'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Restore current list 'selected' value from flag in the item 'bits' field
/*****************************************************************************/
static void txSelRestoreSelected
(
   TXSELIST           *list                     // INOUT selection list
)
{
   ULONG               i;

   ENTER();

   for (i = 0; i < list->count; i++)
   {
      if ((list->items[ i]->bits & TXS_RESERVED_SELECTED) == TXS_RESERVED_SELECTED)
      {
         list->items[ i]->bits &= ~TXS_RESERVED_SELECTED;
         list->selected = i;
         break;
      }
   }
   VRETURN();
}                                               // end 'txSelRestoreSelected'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Reverse a selection list, reversing the sort-order (all items swapped)
/*****************************************************************************/
BOOL TxSelReverseOrder                          // RET   TRUE if list changed
(
   TXSELIST           *list                     // INOUT selection list
)
{
   BOOL                rc = FALSE;
   TXS_ITEM           *item;
   int                 iLo;                     // low  index
   int                 iHi;                     // high index

   ENTER();
   TRACES(( "list:%8.8lx\n", list));

   if (list && (list->sortinfo))
   {
      for (iLo = 0; iLo < (list->count / 2); iLo++)
      {
         iHi = list->count - iLo -1;            // element to swap with
         item = list->items[ iLo];
         list->items[ iLo] = list->items[ iHi];
         list->items[ iHi] = item;
      }
      iLo = list->selected;
      list->selected = list->count - iLo -1;    // point to same item again ...

      list->sortinfo->sort[ TXS_SORT_CURRENT] ^= TXS_SORT_REVERSE; // toggle indicator
      rc = TRUE;
   }
   BRETURN(rc);
}                                               // end 'TxSelReverseOrder'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Sort the list according to the 'current' sort specification
/*****************************************************************************/
void TxSelSortCurrent
(
   TXSELIST           *list                     // INOUT selection list
)
{
   USHORT              sorting;
   USHORT              sortindex;

   ENTER();

   if (list && (list->sortinfo))
   {
      sorting   = list->sortinfo->sort[ TXS_SORT_CURRENT];
      TRACES(("List %8.8lx, current sort:%4.4hx\n", list, sorting));

      sortindex = sorting & TXS_SORT_IDXMASK;
      if (sortindex != 0)                       // indexed sorting specified
      {
         TxSelSortString( list, sortindex);
      }
      if (sorting & TXS_SORT_REVERSE)           // reverse the order
      {
         TxSelReverseOrder( list);
      }
   }
   VRETURN();
}                                               // end 'TxSelSortCurrent'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Get unique quick-select character for specified list index
/*****************************************************************************/
USHORT TxSelGetCharSelect                       // RET   index for select-char
(                                               //       or 0 if not possible
   TXSELIST           *list,                    // IN    selection list
   ULONG               nr,                      // IN    item to get char for
   int                 offset                   // IN    minimum position 0..n
)
{
   USHORT              rc = 0;                  // function return
   TXS_ITEM           *item;
   TXLN                used;                    // used characters
   int                 l;
   char                c;

   ENTER();

   memset( used, 0, TXMAXLN);

   if (list != NULL)
   {
      //- build list of used characters

      for (l = 0; l < list->count; l++)
      {
         if (l != nr)                           // can replace by same value!
         {
            if ((item = list->items[l]) != NULL)
            {
               if ((item->text != NULL) && (item->index != 0))
               {
                  used[ strlen(used)] = item->text[item->index -1];
               }
            }
         }
      }
      TxStrToUpper( used);                      // convert to upper-case!
      TRACES(("item-nr %lu, Selchars used: '%s'\n", nr, used));

      if ((item = list->items[nr]) != NULL)     // item to test
      {
         for (l = offset; l < strlen( item->text); l++)
         {
            c = toupper( item->text[l]);
            if (isalnum(c))                     // accept alpha-numeric only
            {
               if (strchr( used, c) == NULL)    // not used yet
               {
                  rc = l +1;
                  TRACES(("SelChar: '%c' at index %hu\n", item->text[l], rc));
                  break;
               }
            }
         }
      }
   }
   RETURN (rc);
}                                               // end 'TxSelGetCharSelect'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set/reset specified flag-bit(s) in menu-item by searching menu-bar structure
/*****************************************************************************/
ULONG TxSetMenuItemFlag                         // RET   #items found and set
(
   TXS_MENUBAR        *mbar,                    // IN    menubar information
   ULONG               mid,                     // IN    value, menu item id
   BYTE                mask,                    // IN    bits to set/reset
   char               *info,                    // IN    info, reason etc
   BOOL                set                      // IN    set specified bit(s)
)
{
   ULONG               rc = 0;                  // function return
   TXS_MENU           *menu;
   int                 m;

   ENTER();
   TRACES(("%sSET item: %lu, using mask %2.2hx\n", (set) ? "" : "RE", mid, mask));

   for (m = 0; m < mbar->count; m++)
   {
      if ((menu = mbar->menu[m]) != NULL)
      {
         TRACES(("Menu %d='%s' list:%8.8lx\n", m +1, menu->text, menu->list));

         rc += TxSetListItemFlag( 1, menu->list, mid, mask, info, set);
      }
   }
   RETURN (rc);
}                                               // end 'TxSetMenuItemFlag'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Set/reset specified flag-bit(s) in menu-item by searching list-structure
/*****************************************************************************/
ULONG TxSetListItemFlag                         // RET   #items found and set
(
   ULONG               recurse,                 // IN    recursion level (1)
   TXSELIST           *list,                    // IN    list information
   ULONG               mid,                     // IN    value, menu item id
   BYTE                mask,                    // IN    bits to set/reset
   char               *info,                    // IN    info, reason etc or NULL
   BOOL                set                      // IN    set specified bit(s)
)
{
   ULONG               rc = 0;                  // function return
   TXS_ITEM           *item;
   int                 i;

   TRACES(("%sSET list item: %lu, using mask %2.2hx on list at %8.8lx\n",
          (set) ? "" : "RE", mid, mask, list));

   if (list != NULL)
   {
      for (i = 0; i < list->count; i++)
      {
         // TRACES(("Check item: %u\n", i));
         if ((item = list->items[i]) != NULL)   // item to test
         {
            if (item->value == mid)             // matching item id ?
            {
               if (set)
               {
                  TxSelSetItemInfo( item, info);
                  item->flags |=  mask;
               }
               else
               {
                  item->flags &= ~mask;
               }
               rc++;                            // count item as set

               /*
               TRACES(("List %8.8lx item %d='%s' %sSET %s %s\n",
                        list, i +1, item->text, (set) ? "" : "RE",
                        (mask & TXSF_DISABLED) ? "DISABLED" : "",
                        (mask & TXSF_MARKED  ) ? "MARKED"   : ""));
               */
            }
            if ((item->flags & TXSF_P_LISTBOX) && // this is a popup/submenu
                (item->userdata != 0))          // and a list is attached.
            {
               // TRACES(( "Recurse level %lu list: %8.8lx\n", recurse +1, item->userdata));

               if (recurse < 99)                // sanity check
               {
                  rc += TxSetListItemFlag( recurse +1,
                                          (TXSELIST *) item->userdata,
                                           mid, mask, info, set);
               }
               else
               {
                  TRACES(("Aborting on insane recursion level ...\n"));
                  break;
               }
            }
         }
      }
   }
   return (rc);
}                                               // end 'TxSetListItemFlag'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Attach information string to given (menu) item as dynamically allocated str
/*****************************************************************************/
ULONG TxSelSetItemInfo                          // RET   result
(
   TXS_ITEM           *item,                    // IN    list information
   char               *info                     // IN    info, reason etc
)
{
   ULONG               rc = NO_ERROR;           // function return

   TxFreeMem( item->info);                      // can only be dynamic, or NULL!

   if (info != NULL)                            // alloc for new string, and copy
   {
      if ((item->info = TxAlloc( 1, strlen( info) +1)) != NULL)
      {
         strcpy( item->info, info);
      }
      else
      {
         rc = TX_ALLOC_ERROR;
      }
   }
   RETURN (rc);
}                                               // end 'TxSelSetItemInfo'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create a dynamic selection list with specified size
/*****************************************************************************/
ULONG TxSelCreate
(
   ULONG               vsize,                   // IN    visible size
   ULONG               tsize,                   // IN    total size of list
   ULONG               asize,                   // IN    size for item array
   ULONG               astat,                   // IN    alloc status for items
   BOOL                multi,                   // IN    create multi-select info
   TXS_ITEMHANDLER     render,                  // IN    callback render new
   TXSELIST          **list                     // OUT   selection list
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXSELIST           *selist;

   ENTER();
   TRACES(("vsize:%hu  tsize:%hu  asize:%hu  astat:%8.8lx\n",
            vsize, tsize, asize, astat));

   if ((selist = TxAlloc( 1, sizeof(TXSELIST))) != NULL)
   {
      selist->astatus       = astat | TXS_AS_LISTRUCT;
      selist->renderNewItem = render;
      selist->asize         = asize;
      selist->vsize         = vsize;
      selist->tsize         = tsize;
      selist->count         = 0;                // no items yet

      if (asize)
      {
         TRACES(( "Allocate item-ptr array for %lu items\n", tsize));
         if ((selist->items  = TxAlloc( asize, sizeof(TXS_ITEM *))) != NULL)
         {
            selist->astatus |= TXS_AS_PTRARRAY;
         }
         else
         {
            rc = TX_ALLOC_ERROR;
         }
      }
      if (multi && (tsize != 0))
      {
         TRACES(( "Allocate selection array for %lu items\n", tsize));
         if ((selist->selarray = TxAlloc( tsize, sizeof(BYTE))) != NULL)
         {
            selist->astatus   |= TXS_AS_SELARRAY;
         }
         else
         {
            rc = TX_ALLOC_ERROR;
         }
      }
      *list = selist;                           // return created list
      if (rc != NO_ERROR)
      {
         txSelDestroy( list);                   // free partial list memory
      }
   }
   else
   {
      rc = TX_ALLOC_ERROR;
   }
   TRACES(("list:%8.8lx\n", *list));
   RETURN (rc);
}                                               // end 'TxSelCreate'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Destroy dynamic parts of the selection list, freeing memory
/*****************************************************************************/
ULONG txSelDestroy
(
   TXSELIST          **list                     // IN    selection list
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if ((list != NULL) && ((*list) != NULL))
   {
      ULONG            as = (*list)->astatus & TXS_AS_DYN_MASK; // allocation status
      ULONG            ic = (*list)->count;     // actual number of items
      ULONG            i;

      if ((as & TXS_AS_DYNITEMS) != 0)          // dynamic alloc in items ?
      {
         for (i = 0; i < ic; i++)
         {
            txSelItemDestroy( as, &((*list)->items[i]));
         }
      }
      if (as & TXS_AS_PTRARRAY)
      {
         TRACES(("Destroying item ptr-array at:%8.8lx\n", (*list)->items));
         TxFreeMem( (*list)->items);            // free item array
      }
      if (as & TXS_AS_LST_DESC)
      {
         char         *descr = (char *) (*list)->userinfo;

         TRACES(("Destroying list descr (userinfo) at:%8.8lx\n", descr));
         TxFreeMem( descr);                     // avoid nasty cast warnings
         (*list)->userinfo = 0;
      }
      if (as & TXS_AS_SELARRAY)
      {
         TRACES(("Destroying multi selarray at:%8.8lx\n", (*list)->selarray));
         TxFreeMem( (*list)->selarray);         // free selection array
      }
      if (as & TXS_AS_LISTRUCT)
      {
         TRACES(("Destroying list at:%8.8lx\n", *list));
         TxFreeMem( (*list));                   // free list structure
      }
   }
   else                                         // invalid parameters
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'txSelDestroy'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Destroy dynamic parts of a selection list-item, freeing memory
/*****************************************************************************/
ULONG txSelItemDestroy
(
   ULONG               as,                      // IN    allocaton status
   TXS_ITEM          **itemref                  // IN    selection list item
)
{
   ULONG               rc = NO_ERROR;           // function return

   ENTER();

   if ((itemref != NULL) && ((*itemref) != NULL))
   {
      TXS_ITEM  *item = *itemref;

      TxFreeMem( item->info);                   // can only be dynamic, or NULL!
      if (as & TXS_AS_SEL_DESC)
      {
         TxFreeMem( item->desc);
      }
      if (as & TXS_AS_SEL_TEXT)
      {
         TxFreeMem( item->text);
      }
      if (as & TXS_AS_SELITEMS)
      {
         TxFreeMem( (*itemref));
      }
   }
   else                                         // invalid parameters
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'txSelItemDestroy'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Delete current item from the list (like Ctrl-D on a history :-)
/*****************************************************************************/
ULONG TxSelDeleteCurrent                        // RET   resulting selected
(
   TXSELIST           *list                     // INOUT selection list
)
{
   ENTER();

   if ((list != NULL) && (list->count != 0) && (list->selarray == NULL))
   {
      ULONG            i;

      txSelItemDestroy( list->astatus, &(list->items[ list->selected]));

      list->count--;                            // update real list-size
      for (i = list->selected; i < list->count; i++)
      {
         list->items[i] = list->items[ i+1];    // move rest of items down
      }
      if (list->selected >= list->count)        // was the last one
      {
         list->selected--;
      }
   }
   RETURN (list->selected);
}                                               // end 'TxSelDeleteCurrent'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Create selist from existing Txt list
/*****************************************************************************/
ULONG TxSelistFromTxt                           // RET   result
(
   char               *txt[],                   // IN    NULL terminated txt
   BOOL                multi,                   // IN    keep multi-select info
   BOOL                share,                   // IN    share data, no alloc
   TXSELIST          **list                     // OUT   selection list
)
{
   ULONG               rc = NO_ERROR;           // function return
   ULONG               items = 0;

   ENTER();

   if (txt != NULL)
   {
      while (txt[items] != NULL)
      {
         items++;                               // count lines in txt
      }
      rc = TxSelCreate( items, items, items,
                        TXS_AS_DYNBASED | TXS_AS_SELITEMS,
                        multi, NULL, list);

      if (rc == NO_ERROR)
      {
         TXSELIST     *selist = *list;
         ULONG         line;

         if (!share)                            // allocated text strings ?
         {
            selist->astatus |= TXS_AS_SEL_TEXT;
         }
         for (line = 0; (line < items) && (rc == NO_ERROR); line++)
         {
            TXS_ITEM  *newitem = TxAlloc( 1, sizeof(TXS_ITEM));

            if (newitem != NULL)
            {
               if (share)                       // just copy string pointer
               {
                  newitem->text = txt[line];
               }
               else
               {
                  if ((newitem->text = TxAlloc(1,strlen(txt[line]) +1)) != NULL)
                  {
                     strcpy( newitem->text, txt[line]);
                  }
                  else
                  {
                     rc = TX_ALLOC_ERROR;
                  }
               }
               selist->items[line] = newitem;
               selist->count       = line +1;   // number of items in list
            }
            else
            {
               rc = TX_ALLOC_ERROR;
            }
         }
         if (rc != NO_ERROR)
         {
            txSelDestroy( list);                // free partial list memory
         }
      }
   }
   else
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'TxSelistFromTxt'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Create selist from existing selist (duplicate)
/*****************************************************************************/
ULONG TxSelistDuplicate                         // RET   result
(
   TXSELIST           *org,                     // IN    original Selist
   BOOL                multi,                   // IN    keep multi-select info
   BOOL                share,                   // IN    share data, no alloc
   TXSELIST          **list                     // OUT   selection list
)
{
   ULONG               rc = NO_ERROR;           // function return
   ULONG               items = 0;

   ENTER();

   if (org != NULL)
   {
      if (!share)
      {
         items = org->asize;
      }
      rc = TxSelCreate( org->vsize, org->tsize, items, 0,
                       (org->selarray != NULL), NULL, list);

      if (rc == NO_ERROR)
      {
         TXSELIST     *selist = *list;
         ULONG         line;

         if (share)
         {
            selist->items = org->items;         // just point to same info
            selist->count = org->count;
         }
         else
         {
            selist->astatus |= TXS_AS_DYNITEMS;
            for (line = 0; (line < items) && (rc == NO_ERROR); line++)
            {
               TXS_ITEM  *orgitem = org->items[line];
               TXS_ITEM  *newitem = TxAlloc( 1, sizeof(TXS_ITEM));

               if (newitem != NULL)
               {
                  if (((newitem->text = TxAlloc(1,strlen(orgitem->text) +1)) != NULL) &&
                      ((newitem->desc = TxAlloc(1,strlen(orgitem->desc) +1)) != NULL)  )
                  {
                     strcpy( newitem->text, orgitem->text);
                     strcpy( newitem->desc, orgitem->desc);
                  }
                  else
                  {
                     rc = TX_ALLOC_ERROR;
                  }
                  selist->items[line] = newitem;
                  selist->count       = line +1; // number of items in list
               }
               else
               {
                  rc = TX_ALLOC_ERROR;
               }
            }
         }
         if (rc == NO_ERROR)
         {
            if (multi && (org->selarray))       // keep same multi-selected
            {                                   // items selected
               memcpy( selist->selarray, org->selarray, org->tsize);
            }
         }
         else
         {
            txSelDestroy( list);                // free partial list memory
         }
      }
   }
   else
   {
      rc = TX_INVALID_DATA;
   }
   RETURN (rc);
}                                               // end 'TxSelistDuplicate'
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
// Create single-item list with ONE disabled text-item (for 'empty' selections)
/*****************************************************************************/
TXSELIST *TxSelEmptyList                        // RET   selection list or NULL
(
   char               *text,                    // IN    text for single item
   char               *desc,                    // IN    description (footer)
   BOOL                selectable               // IN    item should be selectable
)
{
   TXSELIST           *list  = NULL;            // total list
   TXS_ITEM           *item;                    // single item

   ENTER();

   if (TxSelCreate( 1, 1, 1, TXS_AS_NOSTATIC, FALSE, NULL, &list) == NO_ERROR)
   {
      list->astatus  = TXS_AS_NOSTATIC;         // all dynamic allocated

      if ((item  = TxAlloc( 1, sizeof(TXS_ITEM))) != NULL)
      {
         list->count    = 1;                    // actual items in list
         list->items[0] = item;                 // attach item to list

         item->helpid = TXWH_USE_OWNER_HELP;    // from owner-menu-item

         if (((item->text = TxAlloc( 1, strlen(text) +1)) != NULL) &&
             ((item->desc = TxAlloc( 1, strlen(desc) +1)) != NULL)  )
         {
            if (selectable == FALSE)
            {
               item->flags  = TXSF_DISABLED;
            }
            strcpy( item->text, text);
            strcpy( item->desc, desc);
         }
      }
   }
   RETURN( list);
}                                               // end 'TxSelEmptyList'
/*---------------------------------------------------------------------------*/

