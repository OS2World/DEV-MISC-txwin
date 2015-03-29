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
// TxLib filesystem functions, find file/path section
//

#include <txlib.h>                              // TxLib interface


/*****************************************************************************/
// Find file in a path specified by environment variable (like PATH)
/*****************************************************************************/
char *TxFindByPath                              // RET   ptr to filename
(
   char               *fname,                   // IN    filename string
   char               *path,                    // IN    env-var with path
   char               *found                    // OUT   found file
)
{
   char               *rc  = NULL;
   char               *p;
   char               *this;
   TXLN                try;
   FILE               *fp;
   char               *ep;
   char               *ec;                      // modifyable copy

   ENTER();
   TRACES(( "Find '%s' in paths specified by: '%s'\n", fname, path));

   if ((ep = getenv( path)) != NULL)
   {
      TRACES(( "path is: '%s'\n", ep));

      if ((ec = TxAlloc( 1, strlen( ep) +1)) != NULL)
      {
         p = strcpy( ec, ep);
         do
         {
            this = p;
            p = strchr( p, FS_ENVP_SEP);
            if (p != NULL)
            {
               *p++ = '\0';
            }
            strcpy( try, this);
            strcat( try, FS_PATH_STR);
            strcat( try, fname);
            TRACES(( "Trying fspec: '%s'\n", try));
            if ((fp = fopen( try, "r")) != NULL)
            {
               fclose( fp);
               strcpy( found, try);
               rc = found;
            }
         } while ((p != NULL) && (rc == NULL));
         TxFreeMem( ec);
      }
   }
   RETURN( rc);
}                                               // end 'TxFindByPath'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Find file in current-dir, exe-dir or PATH, and when found, open for read
/*****************************************************************************/
FILE *TxFindAndOpenFile                         // RET   opened file handle
(
   char               *fname,                   // IN    filename string
   char               *path,                    // IN    env-var with path
   char               *found                    // OUT   found file
)
{
   TXLN                try;
   FILE               *fp;
   char               *pp;

   ENTER();

   TRACES(( "Find file: '%s' using path: '%s'\n", fname, path));

   strcpy( try, fname);
   if ((fp = fopen( try, "r")) == NULL)         // try full-spec/current dir
   {
      TRACES(( "FAIL in current dir or absolute path: '%s'\n", try));

      if (TxaExeSwitchSet(TXA_O_DIR))           // explicit files dir given
      {
         strcpy( try, TxaExeSwitchStr(TXA_O_DIR, "", ""));
         if (try[strlen(try)-1] != FS_PATH_SEP) // fspec does not end in SEP
         {
            strcat( try, FS_PATH_STR);          // append directory separator
         }
         strcat( try, fname);                   // append fname
         fp = fopen( try, "r");
      }
      if (fp == NULL)                           // not found yet ...
      {
         strcpy( try, TxaExeArgv(0));
         if ((pp = strrchr( try, FS_PATH_SEP)) != NULL)
         {
            *(pp +1) = 0;                       // try executable directory
         }
         else                                   // if no executable DIR known,
         {
            strcpy( try, FS_PATH_STR);          // try root directory
         }
         strcat( try, fname);                   // append fname

         if ((fp = fopen( try, "r")) == NULL)   // not here either, try PATH
         {
            TRACES(( "FAIL in EXE-directory or root:'%s'\n", try));
            if (TxFindByPath( fname, "PATH", try) != NULL)
            {
               fp = fopen( try, "r");
            }
            else
            {
               TRACES(( "FAIL in paths for env-var: '%s'\n", "PATH"));
            }
         }
      }
   }
   if (fp!= NULL)                               // found the file
   {
      strcpy( found, try);                      // set the found filename
      TRACES(("found file: '%s'\n", found));
   }                                            // and return the (read) fp
   RETURN (fp);
}                                               // end 'TxFindAndOpenFile'
/*---------------------------------------------------------------------------*/

