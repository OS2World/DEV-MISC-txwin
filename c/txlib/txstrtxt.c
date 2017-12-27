//
//                     TxWin, Textmode Windowing Library
//
//   Original code Copyright (c) 1995-2014 Fsys Software and Jan van Wijk
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
// TX string to TX-text conversion and handling functions
//
// Author: J. van Wijk
//
// JvW  24-07-2005 Initial version, split off from TXUTIL
// JvW  11-05-2014 Added load text from file, for help texts
// JvW  14-05-2014 Added search text for specified string and search-options

#include <txlib.h>


/*****************************************************************************/
// Search text for given string and specified search-options
// To search from start/end, specify a posLine that is OUTSIDE the text
/*****************************************************************************/
BOOL txTextSearch
(
   char              **text,                    // IN    TX style text array
   char               *string,                  // IN    String to search for
   BOOL                caseSensitive,           // IN    Case sensitive search
   BOOL                wholeWord,               // IN    Space delimited word only
   BOOL                backwards,               // IN    Search backwards
   BOOL                titleOnly,               // IN    Search in titles only (#nnn)
   ULONG              *posLine,                 // INOUT position, Line number
   short              *posColumn                // OUT   position, Column
)
{
   BOOL                rc = FALSE;
   ULONG               linecount;               // numer of lines in text
   ULONG               cLine;                   // current linenr being searched
   short               cCol;                    // current column
   char              **lptr;
   char               *s;
   TXTM                word;                    // prepared case aware search string
   TXLN                line;                    // prepared current line contents

   ENTER();

   if (text && *text && posLine)                // non-empty list, and a position
   {
      for (lptr = text, linecount = 0; *lptr; lptr++)
      {
         linecount++;                           // count lines in text
      }

      //- prepare actual search argument for case-sensitivity
      TxCopy( word, string, TXMAXTM -1);
      if (wholeWord)
      {
         strcat( word, " ");
      }
      if (!caseSensitive)
      {
         TxStrToLower( word);
      }

      //- set first line to be searched
      if (*posLine < linecount)                 // valid start position
      {
         if (backwards)
         {
            cLine = *posLine - 1;
         }
         else
         {
            cLine = *posLine + 1;
         }
      }
      else                                      // was outside of text
      {
         cLine = linecount -1;
         if (backwards)
         {
            cLine = linecount -1;
         }
         else
         {
            cLine = 0;
         }
      }

      //- search for the string, in specified search direction until found
      if (backwards)
      {
         TRACES(("Search text: %p Backwards from %lu, lc:%lu\n", text, cLine, linecount));
         for (;(cLine < linecount);)
         {
            TxCopy( line, text[ cLine], TXMAXLN -1);

            if (!titleOnly || (line[0] == '#')) // all text, or a title line
            {
               strcat( line, " ");
               if (!caseSensitive)
               {
                  TxStrToLower( line);
               }
               if ((s = strstr( line, word)) != NULL)
               {
                  cCol = (s - line);
                  if (!wholeWord || (cCol == 0) || (line[ cCol -1] == ' '))
                  {
                     TRACES(("Found at %lu, col:%d\n", cLine, cCol));
                     *posLine   = cLine;
                     *posColumn = cCol;         // return found position in line
                     rc = TRUE;
                     break;
                  }
               }
            }
            if (cLine > 0)
            {
               cLine--;
            }
            else
            {
               break;                           // done line 0, finished search
            }
         }
      }
      else
      {
         TRACES(("Search text: %p Forward from %lu, lc:%lu\n", text, cLine, linecount));
         while (cLine < linecount)
         {
            TxCopy( line, text[ cLine], TXMAXLN -1);

            if (!titleOnly || (line[0] == '#')) // all text, or a title line
            {
               strcat( line, " ");
               if (!caseSensitive)
               {
                  TxStrToLower( line);
               }
               if ((s = strstr( line, word)) != NULL)
               {
                  cCol = (s - line);
                  if (!wholeWord || (cCol == 0) || (line[ cCol -1] == ' '))
                  {
                     TRACES(("Found at %lu, col:%d\n", cLine, cCol));
                     *posLine   = cLine;
                     *posColumn = cCol;         // return found position in line
                     rc = TRUE;
                     break;
                  }
               }
            }
            cLine++;
         }
      }
   }
   BRETURN (rc);
}                                               // end 'txTextSearch'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Read lines from opened file into dynamically allocated TXLIB text structure
/*****************************************************************************/
char **txFile2Text                              // RET   ptr to text-array
(
   FILE               *file,                    // IN    file opened for read
   ULONG              *size                     // OUT   size in lines
)
{
   char              **data  = NULL;
   char               *line  = NULL;
   char               *new   = NULL;
   ULONG               lines = 0;
   ULONG               nr;

   ENTER();

   if ((line = TxAlloc( 1, TXMAX4K)) != NULL)
   {
      fseek( file, 0, SEEK_SET);
      while (fgets( line, TXMAX4K, file) != NULL)
      {
         lines++;
      }
      TRACES(( "Opened file has %lu lines ...\n", lines));
      if ((data = TxAlloc( lines +1, sizeof(char *))) != NULL)
      {
         fseek( file, 0, SEEK_SET);
         for (nr = 0; nr < lines; nr++)
         {
            if (fgets( line, TXMAX4K, file) != NULL)
            {
               if ((new = TxAlloc( 1, strlen(line)+1)) != NULL)
               {
                  if (line[strlen(line)-1]=='\n')
                  {
                    line[strlen(line)-1]='\0';
                  }
                  strcpy( new, line);
                  data[ nr] = new;
               }
               else                             // quit reading on memory error
               {
                  break;
               }
            }
            else                                // unexpected read failure
            {
               break;
            }
         }
         *size = lines;
      }
      TxFreeMem( line);
   }
   RETURN( data);
}                                               // end 'txFile2Text'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Convert C-string to dynamically allocated TXLIB text structure
/*****************************************************************************/
char  **txString2Text                           // RET   TXLIB text structure
(
   char               *string,                  // IN    null terminated string
   int                *maxlen,                  // INOUT line length
   int                *lines                    // OUT   nr of lines
)
{
   char              **text = NULL;             // function return
   int                 line = 0;                // nr of lines done
   int                 lpsize;                  // nr of line pointers
   int                 length;                  // length of substring
   char               *start;                   // start of line
   char               *next;                    // end of line / next

   ENTER();

   if (string != NULL)
   {
      lpsize = (strlen( string) * 3 / (*maxlen)) +9; // worst-case
      //- Note: Fails on string with many very short lines => text is clipped

      TRACES(("Strlen: %d  Maxlen: %d => Alloc %d pointers\n",
               strlen(string), *maxlen, lpsize));

      if ((text = TxAlloc( lpsize, sizeof(char *))) != NULL)
      {
         for ( start =  string, line = 0;
               start && strlen( start) && (line < lpsize -1);
               start =  next,   line++)
         {
            length = strcspn( start, "\n");
            TRACES(("found newline at pos: %d\n", length));
            if (length > *maxlen)               // no newline in sight
            {
               for ( length = *maxlen;
                    (length > 0) && (start[ length] != ' ');
                     length --)
               {
                  //- find wordwrap position (strrchr would start at end!)
               }
               TRACES(("found word-wrap space at pos: %d\n", length));
               if (length == 0)                 // one very long word, split
               {
                  length = *maxlen;
               }
            }
            TRACES(("line:%d length:%d s:'%-40.40s'\n", line, length, start));
            text[ line] = TxAlloc( 1, length +1);
            strncpy( text[ line], start, length);
            text[ line][length] = '\0';
            TRACES(("text[%d]:'%s'\n", line, text[line]));

            next = start + length;
            if (((*next == ' ') && (*(next+1) != ' ')) || (*next == '\n'))
            {
               TRACES(( "skipping one whitespace at: '%-40.40s'\n", next));
               next++;                          // skip at most 1 whitespace
            }
         }
         text[ line] = NULL;                    // explicit end-pointer

         TRACES(("line: %d\n", line));
         *lines  = line;                        // nr of lines generated
         *maxlen = 0;
         while (line--)
         {
            length = strlen( text[ line]);
            if (length > *maxlen)
            {
               *maxlen = length;                // determine maximum length
            }
         }
         TRACES(("lines: %d  with maxlen: %d\n", *lines, *maxlen));
      }
   }
   RETURN (text);
}                                               // end 'txString2Text'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Free memory allocated for dynamic TXLIB text
/*****************************************************************************/
void txFreeText
(
   char              **text                     // IN    dyn. allocated text
)
{
   char              **line;

   ENTER();

   if (text != NULL)
   {
      for (line = text; *line; line++)
      {
         TxFreeMem( *line);
      }
      TxFreeMem( text);
   }
   VRETURN ();
}                                               // end 'txFreeText'
/*---------------------------------------------------------------------------*/

