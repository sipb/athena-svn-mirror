/* gok-utf8-word-complete.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
**/

#ifndef __GOK_UTF8_WORDCOMPLETE_H__
#define __GOK_UTF8_WORDCOMPLETE_H__

#include "gok-word-complete.h"

#define GOK_TYPE_UTF8WORDCOMPLETE           (gok_utf8_wordcomplete_get_type ())
#define GOK_UTF8WORDCOMPLETE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOK_TYPE_UTF8WORDCOMPLETE, GokUTF8WordComplete))
#define GOK_UTF8WORDCOMPLETE_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, GOK_TYPE_UTF8WORDCOMPLETE, GokUTF8WordCompleteClass)
#define GOK_IS_UTF8WORDCOMPLETE(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, GOK_TYPE_UTF8WORDCOMPLETE)
#define GOK_IS_UTF8WORDCOMPLETE_CLASS(klass)       G_TYPE_CHECK_CLASS_TYPE (klass, GOK_TYPE_UTF8WORDCOMPLETE)

typedef struct _GokUTF8WordComplete       GokUTF8WordComplete;
typedef struct _GokUTF8WordCompleteClass  GokUTF8WordCompleteClass;

struct _GokUTF8WordComplete
{
	GokWordComplete parent;
        gchar *primary_dict_filename;
        GList *word_list;
	GList *word_list_end;
        GList *start_search;
	GList *end_search;
	gchar *most_recent_word;
	GHashTable *unicode_start_hash; 
	GHashTable *unicode_end_hash; 
};

struct _GokUTF8WordCompleteClass
{
	GokWordCompleteClass parent_class;
};

GType gok_utf8_wordcomplete_get_type (void);

#endif /* #ifndef _GOK_UTF8_WORDCOMPLETE_H__ */
