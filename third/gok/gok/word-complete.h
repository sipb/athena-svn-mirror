/* word-complete.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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
* To use this thing:
* - Call "WordCompleteOpen". If it returns TRUE then you're ready to go.
* - Call "WordCompletePredict" to make the word predictions.
* - Call "WordCompleteClose" when you're done. 
* - To add a word, call "WordCompleteAddNewWord".
*
*/

#ifndef __WORDCOMPLETE_H__
#define __WORDCOMPLETE_H__

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gok-word-complete.h"

/* maximum length of a word in the dictionary */
#define MAXWORDLENGTH 30

/* maximum number of words predicted */
#define MAXPREDICTIONS 10

/* state of words in the dictionary */
#define STATE_TEMPORARY 1
#define STATE_PERMANENT 2

/* priority of new word added to the dictionary */
#define PRIORITY_NEWWORD 100

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif


#define GOK_TYPE_TRIEWORDCOMPLETE           (gok_trie_wordcomplete_get_type ())
#define GOK_TRIEWORDCOMPLETE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOK_TYPE_TRIEWORDCOMPLETE, GokTrieWordComplete))
#define GOK_TRIEWORDCOMPLETE_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, GOK_TYPE_TRIEWORDCOMPLETE, GokTrieWordCompleteClass)
#define GOK_IS_TRIEWORDCOMPLETE(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, GOK_TYPE_TRIEWORDCOMPLETE)
#define GOK_IS_TRIEWORDCOMPLETE_CLASS(klass)       G_TYPE_CHECK_CLASS_TYPE (klass, GOK_TYPE_TRIEWORDCOMPLETE)

typedef struct _GokTrieWordComplete       GokTrieWordComplete;
typedef struct _GokTrieWordCompleteClass  GokTrieWordCompleteClass;

struct _GokTrieWordComplete
{
	GokWordComplete parent;
};

struct _GokTrieWordCompleteClass
{
	GokWordCompleteClass parent_class;
};

/* a node in the trie */
struct NodeStruct
{
	gchar letter;
	gint priority; /* if this is not 0 then this is the end of a word */
	gint state;
	struct NodeStruct* pNextNode;
	struct NodeStruct* pFirstChildNode;
};
typedef struct NodeStruct Node;

#endif /* #ifndef __WORDCOMPLETE_H__ */
