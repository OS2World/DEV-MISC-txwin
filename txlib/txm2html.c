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
// Generate html/php documentation from TxWin (pulldown) menu structure
//
// Author: J. van Wijk
//
// JvW  18-04-2014 Base 'shots' subdirectory on specified menu basename (basic/expert)
// JvW  11-09-2012 Added actual LastModDate to generated screenshot pages
// JvW  20-08-2007 Initial version

#include <txlib.h>                              // public interface
#include <txvers.h>                             // version info

static char txwPdHtmlHeader[] =
   "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
   "<HTML>\n"
   " <HEAD>\n"
   "  <TITLE>%s</TITLE>\n"
   " </HEAD>\n"
   " <BODY BACKGROUND=\"backgrnd.gif\" TEXT=\"#000000\">\n"
   "  <H%lu>%s <I>%s</I></H%lu>\n";

static char txwPdHtmlFooter[] =
   " </BODY>\n"
   "</HTML>\n";

static char txwPdPhpHeader[] =
   "<?php\n"
   " $LastModDate=\"%s\";\n"
   " $MenuBase=\"%s-%s\";\n"
   " $MenuItem=\"%s\";\n"
   " $ChapterTitle = \"%s menu documentation for pulldown : \" . $MenuItem;\n"
   " include(\"%s-header.php\")\n"
   "?>\n"
   "  <H%lu>%s %s</H%lu>\n";


static char txwPdPhpFooter[] =
   "<?php\n"
   " include(\"%s-footer.php\")\n"
   "?>\n";


// Generate HTML documentation from a Selist menu structure, recursive
static ULONG txsMenu2Html
(
   FILE               *df,                      // IN    open HTML dest file
   TXSELIST           *list,                    // IN    (sub) menu list data
   char               *mtext,                   // IN    compound menutext sofar
   char               *basename,                // IN    shots basename
   BOOL                usephp,                  // IN    create PHP, not HTML
   int                 level                    // IN    HTML heading level
);


// Generate paragraph of HTML documentation from specified help-text
static ULONG txsHelp2Html
(
   FILE               *df,                      // IN    open HTML dest file
   char               *lead,                    // IN    HTML indent lead
   ULONG               helpid                   // IN    help id for item
);

// Generate conditional display of screen-shots link to PHP display code
static ULONG txsShot2Html
(
   FILE               *df,                      // IN    open HTML dest file
   char               *lead,                    // IN    HTML indent lead
   char               *basename,                // IN    shots basename
   ULONG               helpid,                  // IN    help id for item
   char               *title,                    // IN    menu title for item
   BOOL                usephp                   // IN    create PHP, not HTML
);

// Generate static-HTML to display all matching screenshots for one menu-id
static ULONG txsStaticHtmlShots
(
   char               *dirbase,                 // IN    shots directory basename
   char               *filebase,                // IN    shots file basename
   char               *title                    // IN    menu title for item
);



// JvW: To add textual description of full menu-item name (like 'File -> Open logfile')
//      Create array of strings (minimal TXWORD, 84 chars) of at least 16 levels.
//      Fill in first entries, upto pdLevel with "" and the rest, when traversing
//      the menu tree with the name of the current traversed item, at its own level.
//      Just before writing the 'help' text for the item, write out the full menu path
//      skipping empty strings, and separating them with ' ->'

/*****************************************************************************/
// Generate HTML (optional PHP) documentation from a pulldown menu structure
/*****************************************************************************/
ULONG txSelPdMenu2Html
(
   char               *product,                 // IN    product identification
   TXS_MENUBAR        *mbar,                    // IN    Main menu structure
   char               *select,                  // IN    partial PD select str
   char               *htmlnav,                 // IN    HTML navigation header
   char               *filename,                // IN    html filename or NULL
   char               *prefix,                  // IN    auto filename prefix
   char               *pdLead,                  // IN    Pulldown lead or NULL
   ULONG               pdLevel                  // IN    HTML top heading level
)
{
   ULONG               rc = TX_FAILED;          // function return
   int                 m;
   TXS_MENU           *pd = NULL;
   char               *s;
   FILE               *df;                      // destination file ptr
   TXTM                pdname;                  // full pulldown name
   TXTM                mename;                  // partial pulldown name
   TXTM                pddesc;
   TXLN                fname;
   BOOL                usephp = (htmlnav == NULL);

   ENTER();

   TRACES(( "select: '%s' level: %lu\n", select, pdLevel));
   for (m = 0; m < mbar->count; m++)
   {
      if (mbar->menu[m] != NULL)
      {
         strcpy(  pdname, mbar->menu[m]->text);
         TxStrip( pdname, pdname, ' ', ' ');

         if (strncasecmp( pdname, select, strlen( select)) == 0)
         {
            pd = mbar->menu[m];
            break;
         }
      }
   }
   if (pd != NULL)
   {
      if ((s = strchr( pdname, '=')) == NULL)
      {
         s = pdname;
      }
      else
      {
         s++;
      }
      strcpy( mename, s);
      TxStrToLower( mename);
      if (filename && strlen( filename))
      {
         strcpy( fname, filename);
      }
      else                                      // construct default name
      {
         strcpy( fname, prefix);
         strcat( fname, "-");
         strcat( fname, mename);
         TxStrToLower(  fname);
      }
      TxFnameExtension( fname, (usephp) ? "php" : "htm");

      if ((df = fopen( fname, "w" TXFMODE)) != NULL) // create new text file
      {
         TxCopy(  pddesc, pd->desc, 52);
         TxStrip( pddesc, pddesc, ' ', ' ');

         if (usephp)
         {
            TXTS       today;
            time_t     tt = time( &tt);         // current date/time

            strftime( today, TXMAXTS, "%d-%b-%Y", localtime( &tt));
            fprintf( df, txwPdPhpHeader,   today,   prefix,
                          mename,  pdname, product, prefix,
                          pdLevel, pdname, pddesc,  pdLevel);
         }
         else
         {
            fprintf( df, txwPdHtmlHeader,  pddesc, pdLevel, pdname,  pddesc, pdLevel);
            fprintf( df, "%s\n",  htmlnav);
         }
         rc = txsMenu2Html( df, pd->list, pdname, prefix, usephp, (int) pdLevel +1);

         fprintf( df, "  <HR>  %sHTML created by : %s %s %s\n",
                         ( usephp) ? "PHP and " : "", TXLIB_N, TXLIB_V, TXLIB_C);
         if (strlen( mbar->appdesc))
         {
            fprintf( df, "  <BR>  For application : %s\n", mbar->appdesc);
         }

         if (rc == NO_ERROR)
         {
            if (usephp)
            {
               if (fprintf( df, txwPdPhpFooter, prefix) < 0)
               {
                  rc = ERROR_WRITE_FAULT;
               }
            }
            else
            {
               fprintf( df, "%s\n",  htmlnav);
               if (fprintf( df, "%s", txwPdHtmlFooter) < 0)
               {
                  rc = ERROR_WRITE_FAULT;
               }
            }
         }
         if (rc == ERROR_WRITE_FAULT)
         {
            TxPrint( "Error writing to '%s', disk might be full\n", fname);
         }
         else
         {
            TxPrint( "\nFinished html-doc : '%s', for pulldown: %s\n%s\n\n",
                        fname, pd->text, pddesc);
         }
         fclose( df);
      }
      else
      {
         TxPrint( "Destination file '%s' cannot be created\n", fname);
         rc = TX_INVALID_FILE;
      }
   }
   RETURN (rc);
}                                               // end 'txSelPdMenu2Html'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Generate HTML documentation from a Selist menu structure, recursive
/*****************************************************************************/
static ULONG txsMenu2Html
(
   FILE               *df,                      // IN    open HTML dest file
   TXSELIST           *list,                    // IN    (sub) menu list data
   char               *mtext,                   // IN    compound menutext sofar
   char               *basename,                // IN    shots basename
   BOOL                usephp,                  // IN    create PHP, not HTML
   int                 level                    // IN    HTML heading level
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXLN                compound;                // complete menutext
   TXTM                itext;                   // item text
   TXTT                indent;
   TXS_ITEM           *item;
   int                 li;
   ULONG               helpid;

   ENTER();

   sprintf( indent, "%*s", level, "");
   for (li = 0; li < list->count; li++)
   {
      item = list->items[li];

      if      (item->flags & TXSF_SEPARATOR)
      {
         fprintf( df, "%*s<HR>\n", level, "");
      }
      else
      {
         strcpy(  itext, item->text);
         TxStrip( itext, itext, ' ', '¯');      // remove submenu arrows
         TxStrip( itext, itext, ' ', '>');
         TxStrip( itext, itext, ' ', '<');
         TxStrip( itext, itext, ' ', ' ');      // then trailing spaces

         strcpy(  compound, mtext);
         strcat(  compound, " -&gt; ");
         strcat(  compound, itext);

         if ((helpid = item->helpid) == TXWH_USE_CMD_CODE)
         {
            helpid = item->value;               // use cmd value
         }
         fprintf( df, "%s<H%d><A name=\"id%05lu\">%s</A></H%d>\n",
                                        indent, level, helpid, itext, level);
         fprintf( df, "%sFrom main menu: <I>%s</I>\n", indent, compound);
         fprintf( df, "%s<P>%s\n",                     indent, item->desc);

         txsShot2Html( df, indent, basename, helpid, compound, usephp);
         txsHelp2Html( df, indent,           helpid);

         fprintf( df, "%s<P>\n",             indent);
         fprintf( df, "%s<BR clear=all>\n",  indent);

         TRACES(( "check submenu for '%s'\n", compound));
         if ((item->flags & TXSF_P_LISTBOX) &&
             (item->userdata != 0))             // submenu list present ?
         {
            TXSELIST *submenu = (TXSELIST *) item->userdata;

            TRACES(( "Submenu at %p is %s\n", submenu,
                     (submenu->astatus & TXS_LST_DYN_CONTENTS) ? "Dynamic" : "Static"));

            if (submenu->astatus & TXS_LST_DYN_CONTENTS) // do not recurse
            {
               if (submenu->astatus & TXS_LST_DESC_PRESENT) // summary descr
               {
                  fprintf( df, "%*s<P>Selection of this item leads to "
                               "a dynamic created submenu, to select a %s\n",
                                level, "", (char *) submenu->userinfo);
               }
               else
               {
                  fprintf( df, "%*s<P>Selection of this item leads to "
                               "a submenu with dynamicly created contents\n", level, "");
               }
            }
            else                                // static, fixed submenu, recurse
            {
               rc = txsMenu2Html( df, submenu, compound, basename, usephp, level +1);
            }
         }
      }
   }
   fprintf( df, "%*s<P>\n", level, "");
   RETURN (rc);
}                                               // end 'txsMenu2Html'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Generate paragraph of HTML documentation from specified help-text
/*****************************************************************************/
static ULONG txsHelp2Html
(
   FILE               *df,                      // IN    open HTML dest file
   char               *lead,                    // IN    HTML indent lead
   ULONG               helpid                   // IN    help id for item
)
{
   ULONG               rc = NO_ERROR;           // function return
   char              **ht;                      // help-text
   ULONG               empty = 0;               // empty lines
   TXLN                line1;
   TXLN                line2;                   // linefilter buffers

   ENTER();

   if ((ht = txwGetHelpText( helpid)) != NULL)
   {
      ht++;                                     // skip item header line
      fprintf( df, "%s<P>\n",   lead);
      fprintf( df, "%s<PRE>\n", lead);
      while ((*ht != NULL) && (*ht[0] != '#'))
      {
         if (strlen( *ht))                      // add line inside block
         {
            TxRepStr( *ht,   '<', "&lt;", line1, TXMAXLN);
            TxRepStr( line1, '>', "&gt;", line2, TXMAXLN);
            fprintf( df, "   %s\n",       line2);
            empty = 0;
         }
         else if (empty++ == 0)                 // one empty line ...
         {
            fprintf( df, "\n");                 // just include in block
         }
         else
         {
            if (empty == 1)                     // terminate & start new block
            {
               fprintf( df, "%s</PRE>\n", lead);
               fprintf( df, "%s<P>\n",    lead);
               fprintf( df, "%s<PRE>\n",  lead);
            }
         }
         ht++;
      }
      fprintf( df, "%s</PRE>\n",          lead);
   }
   RETURN (rc);
}                                               // end 'txsHelp2Html'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Generate conditional display of screen-shots link to PHP/HTML display code
/*****************************************************************************/
static ULONG txsShot2Html
(
   FILE               *df,                      // IN    open HTML dest file
   char               *lead,                    // IN    HTML indent lead
   char               *basename,                // IN    shots basename
   ULONG               helpid,                  // IN    help id for item
   char               *title,                   // IN    menu title for item
   BOOL                usephp                   // IN    create PHP, not HTML
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXTT                shotbase;
   TXTM                thumb;                   // filename thumbnail
   TXLN                img_url;

   ENTER();

   sprintf( shotbase, "id%05lu", helpid);
   sprintf( thumb,    "%s-shots/thumbs/%s.gif", basename, shotbase);

   if (usephp)
   {
      sprintf( img_url,  "<A HREF=\"%s-shots.php?item=%s"
                                        "&amp;moddate=<?php print($LastModDate) ?>"
                                          "&amp;title=%s\">",          basename, shotbase, title);

      fprintf( df, "<?php if (is_file(\"%s\")) { ?>\n",                          thumb);
      fprintf( df, "%s%s\n",                                           lead,     img_url);
      fprintf( df, "%s<IMG SRC=\"%s\" WIDTH=\"171\" HEIGHT=\"150\" "
                                     "ALIGN=\"RIGHT\"></A>\n",         lead,     thumb);
      fprintf( df, "%s<P>%sMenu item screenshot(s)</A>\n",             lead,     img_url);
      fprintf( df, "<?php } ?>\n");
   }
   else                                         // generate static HTML, based
   {                                            // on images NOW present!
      if (TxFileExists( thumb))
      {
         sprintf( img_url,  "<A HREF=\"%s-shots/%s.htm\">",            basename, shotbase);

         fprintf( df, "%s%s\n",                                           lead,     img_url);
         fprintf( df, "%s<IMG SRC=\"%s\" WIDTH=\"171\" HEIGHT=\"150\" "
                                        "ALIGN=\"RIGHT\"></A>\n",         lead,     thumb);
         fprintf( df, "%s<P>%sMenu item screenshot(s)</A>\n",             lead,     img_url);

         rc = txsStaticHtmlShots( basename, shotbase, title);  // create static HTML multiple-shot view
      }
   }
   RETURN (rc);
}                                               // end 'txsShot2Html'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Generate static-HTML to display all matching screenshots for one menu-id
/*****************************************************************************/
static ULONG txsStaticHtmlShots
(
   char               *dirbase,                 // IN    shots directory basename
   char               *filebase,                // IN    shots file basename
   char               *title                    // IN    menu title for item
)
{
   ULONG               rc = NO_ERROR;           // function return
   TXTM                fname;
   FILE               *df;                      // destination file ptr
   ULONG               i;

   ENTER();

   sprintf( fname, "%s-shots%s%s.htm", dirbase, FS_PATH_STR, filebase);
   if ((df = fopen( fname, "w" TXFMODE)) != NULL) // create new text file
   {
      fprintf( df, txwPdHtmlHeader, title, 1, "",  title, 1);

      for (i = 1; i < 19; i++)                  // max 18 screenshots per item
      {
         sprintf( fname, "%s-shots%s%s-%lu.gif", dirbase, FS_PATH_STR, filebase, i);
         if (TxFileExists( fname))
         {
            fprintf( df, "  <CENTER>\n");
            fprintf( df, "   <A href=\"javascript:history.go(-1)\">\n");
            fprintf( df, "    <IMG SRC=\"%s-%lu.gif\" WIDTH=\"720\" "
                              "HEIGHT=\"630\" ALIGN=\"middle\">\n", filebase,  i);
            fprintf( df, "   </A>\n");
            fprintf( df, "   <H3>%s :  screenshot %lu</H3>\n",      title, i);
            fprintf( df, "   <I>Click anywhere in the image to return</I> &nbsp;&nbsp; "
                                 "(<A href=\"javascript:history.go(-1)\">Back</A>)");
            fprintf( df, "  </CENTER>\n");
            fprintf( df, "  <P><HR><P>\n");
         }
      }
      if (fprintf( df, "%s", txwPdHtmlFooter) < 0)
      {
         rc = ERROR_WRITE_FAULT;
      }
      fclose( df);
   }
   else
   {
      TxPrint( "Destination file '%s' cannot be created\n", fname);
      rc = TX_INVALID_FILE;
   }
   RETURN (rc);
}                                               // end 'txsStaticHtmlShots'
/*---------------------------------------------------------------------------*/

