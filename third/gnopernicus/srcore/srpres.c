/* srpres.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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
 */

#include "config.h"
#include "srpres.h"
#include "SRObject.h"
#include "glib.h"
#include <string.h>
#include "srspc.h"
#include "libsrconf.h"
#include "srintl.h"
#include <libxml/parser.h>

extern gint src_caret_position;

typedef enum
{
    SRC_PRESENTATION_NAME,
    SRC_PRESENTATION_DESCRIPTION,
    SRC_PRESENTATION_ROLE,
    SRC_PRESENTATION_ACCELERATOR,
    SRC_PRESENTATION_SHORTCUT,
    SRC_PRESENTATION_LEVEL,
    SRC_PRESENTATION_POSITION_TOP,
    SRC_PRESENTATION_POSITION_LEFT,
    SRC_PRESENTATION_POSITION_RIGHT,
    SRC_PRESENTATION_POSITION_BOTTOM,
    SRC_PRESENTATION_POSITION_WIDTH,
    SRC_PRESENTATION_POSITION_HEIGHT,
    SRC_PRESENTATION_TEXT_CHAR,
    SRC_PRESENTATION_TEXT_BEST,
    SRC_PRESENTATION_TEXT_WORD,
    SRC_PRESENTATION_TEXT_LINE,
    SRC_PRESENTATION_TEXT_SENTENCE,
    SRC_PRESENTATION_TEXT_PARAGRAPH,
    SRC_PRESENTATION_TEXT_DOCUMENT,
    SRC_PRESENTATION_TEXT_SELECTION,
    SRC_PRESENTATION_TEXT_SELECTIONS,
    SRC_PRESENTATION_TEXT_DIFFERENCE,
    SRC_PRESENTATION_STATE_CHECKED,
    SRC_PRESENTATION_STATE_UNCHECKED,
    SRC_PRESENTATION_STATE_ENABLED,
    SRC_PRESENTATION_STATE_EXPANDED,
    SRC_PRESENTATION_STATE_COLLAPSED,
    SRC_PRESENTATION_STATE_MINIMIZED,
    SRC_PRESENTATION_CHILDREN_COUNT,
    SRC_PRESENTATION_VALUE_CRT,
    SRC_PRESENTATION_COLUMN_HEADER,
    SRC_PRESENTATION_ROW_HEADER,
    SRC_PRESENTATION_CELL,
    SRC_PRESENTATION_NONE
}SRCPresentationItemType;


typedef enum
{
    SRC_PRESENTATION_INFO_TYPE_INFO,
    SRC_PRESENTATION_INFO_TYPE_TEXT,
    SRC_PRESENTATION_INFO_TYPE_COND,
    SRC_PRESENTATION_INFO_TYPE_PARANTHESIS,
    SRC_PRESENTATION_INFO_TYPE_COND1,
    SRC_PRESENTATION_INFO_TYPE_SPECIAL,
    SRC_PRESENTATION_INFO_TYPE_CHILDREN,
    SRC_PRESENTATION_INFO_TYPE_NONE
}SRCPresentationInfoType;

typedef enum
{
    SRC_PRESENTATION_BOUNDS_TYPE_OBJECT,
    SRC_PRESENTATION_BOUNDS_TYPE_TEXT_CHAR
}SRCPresentationBoundsType;


typedef struct _SRCPresentationItem
{
    SRCPresentationInfoType	type;
    SRCPresentationItemType	info;
    gpointer			data1;
    gpointer			data2;
    gpointer			voice;
}SRCPresentationItem;

static const gchar *src_pres_lang = NULL;

static const gchar*
srcp_get_lang (void)
{
    if (!src_pres_lang)
    {
	src_pres_lang = g_getenv ("LANG");
	if (!src_pres_lang)
	    src_pres_lang = "C";
    }
    return src_pres_lang;
}

static SRCPresentationItem*
src_presentation_item_new ()
{
    SRCPresentationItem *item;
    
    item = g_new (SRCPresentationItem, 1);
    if (item)
    {
	item->type  = SRC_PRESENTATION_INFO_TYPE_NONE;
	item->info  = SRC_PRESENTATION_NONE;
	item->data1 = item->data2 = NULL;
	item->voice = NULL;    
    }
    return item;
}

static void
src_presentation_item_terminate (SRCPresentationItem *item)
{
    sru_assert (item);
    
    switch (item->type)
    {
	case SRC_PRESENTATION_INFO_TYPE_TEXT:
	    g_free (item->data1);
	    g_free (item->voice);
	    break;
	case SRC_PRESENTATION_INFO_TYPE_COND:
	case SRC_PRESENTATION_INFO_TYPE_COND1:
	    g_free (item->data1);
	    g_free (item->data2);
	    g_free (item->voice);
	    break;
	case SRC_PRESENTATION_INFO_TYPE_CHILDREN:
	    if (item->data1)
	    {
		SRCPresentationItem **item1;
		gint i;
		item1 = (SRCPresentationItem **)item->data1;
		for (i = 0; item1[i]; i++)	
		    src_presentation_item_terminate (item1[i]);
		g_free (item->data1);
	    }
	    g_free (item->data2);
	    break;
	case SRC_PRESENTATION_INFO_TYPE_INFO:
	case SRC_PRESENTATION_INFO_TYPE_PARANTHESIS:    
	    g_free (item->voice);
	    break;
	case SRC_PRESENTATION_INFO_TYPE_SPECIAL:
	    g_free (item->data1);
	    break;
	default:
	    sru_assert_not_reached ();
	    break;
    }
    g_free (item);    
}


static void
src_presentation_items_terminate (SRCPresentationItem **items)
{
    gint i;
    /*sru_assert (items);*/
    if (!items)
	return;
    
    for (i = 0; items[i]; i++)
    {
        src_presentation_item_terminate (items[i]);
    }

    g_free (items);
}

static SRCPresentationItemType
src_presentation_get_item_type (const gchar *item)
{
    static struct
    {
	SRCPresentationItemType type;
	gchar *name;
    }src_presentation_type_name [] = 
	{
	    {SRC_PRESENTATION_NAME,		"name"},
	    {SRC_PRESENTATION_DESCRIPTION,	"description"},
	    {SRC_PRESENTATION_ROLE,		"role"},
	    {SRC_PRESENTATION_ACCELERATOR,	"accelerator"},
	    {SRC_PRESENTATION_SHORTCUT,		"shortcut"},
	    {SRC_PRESENTATION_LEVEL,		"level"},
	    {SRC_PRESENTATION_POSITION_TOP, 	"position:top"},
	    {SRC_PRESENTATION_POSITION_LEFT,	"position:left"},
	    {SRC_PRESENTATION_POSITION_RIGHT,	"position:right"},
	    {SRC_PRESENTATION_POSITION_BOTTOM,	"position:bottom"},
	    {SRC_PRESENTATION_POSITION_WIDTH,	"position:width"},
	    {SRC_PRESENTATION_POSITION_HEIGHT,	"position:height"},
	    {SRC_PRESENTATION_TEXT_CHAR,	"text:char"},
	    {SRC_PRESENTATION_TEXT_BEST,	"text:best"},
	    {SRC_PRESENTATION_TEXT_WORD,	"text:word"},
	    {SRC_PRESENTATION_TEXT_LINE,	"text:line"},
	    {SRC_PRESENTATION_TEXT_SENTENCE,	"text:sentence"},
	    {SRC_PRESENTATION_TEXT_PARAGRAPH,	"text:paragraph"},
	    {SRC_PRESENTATION_TEXT_DOCUMENT,	"text:document"},
	    {SRC_PRESENTATION_TEXT_SELECTION,	"text:selection"},
	    {SRC_PRESENTATION_TEXT_SELECTIONS,	"text:selections"},
	    {SRC_PRESENTATION_TEXT_DIFFERENCE,	"text:difference"},
	    {SRC_PRESENTATION_STATE_CHECKED,	"state:checked"},
	    {SRC_PRESENTATION_STATE_UNCHECKED,	"state:unchecked"},
	    {SRC_PRESENTATION_STATE_ENABLED,	"state:enabled"},
	    {SRC_PRESENTATION_STATE_EXPANDED,	"state:expanded"},
	    {SRC_PRESENTATION_STATE_COLLAPSED,	"state:collapsed"},
	    {SRC_PRESENTATION_STATE_MINIMIZED,	"state:minimized"},
	    {SRC_PRESENTATION_CHILDREN_COUNT,	"childcount"},
	    {SRC_PRESENTATION_VALUE_CRT,	"value:crt"},
	    {SRC_PRESENTATION_COLUMN_HEADER,	"column_header"},
	    {SRC_PRESENTATION_ROW_HEADER,       "row_header"},
	    {SRC_PRESENTATION_CELL,             "cell"}
	};
    gint i;

    sru_assert (item);
    
    for (i = 0; i < G_N_ELEMENTS (src_presentation_type_name); i++)
    {
	if (strcmp (src_presentation_type_name[i].name, item) == 0)
	{
	    return src_presentation_type_name[i].type;
	}
    }

    sru_assert_not_reached ();
    return SRC_PRESENTATION_NONE;
}


static SRCPresentationItem*
src_presentation_get_presentation_item_from_literal (xmlNode *node,
						   const gchar *device)
{
    SRCPresentationItem *item;

    sru_assert (xmlStrcmp (node->name, "literal") == 0);
    
    item = src_presentation_item_new ();
    if (!item)
        return NULL;

    item->type  = SRC_PRESENTATION_INFO_TYPE_TEXT;
    item->voice = g_strdup (xmlGetProp (node, "voice"));
    item->data1 = g_strdup (xmlNodeGetContent (node));
    return item;
}


static SRCPresentationItem*
src_presentation_get_presentation_item_from_dots (xmlNode *node,
						      const gchar *device)
{
    SRCPresentationItem *item;

    sru_assert (xmlStrcmp (node->name, "brdots") == 0);
    
    item = src_presentation_item_new ();
    if (!item)
        return NULL;
     
    item->type  = SRC_PRESENTATION_INFO_TYPE_SPECIAL;
    item->data1 = g_strdup (xmlGetProp (node, "dots"));

    return item;
}

static const xmlNode*
src_presentation_get_literal_node_lang (xmlNode *start,
					xmlChar *lang)
{
    xmlNode *crt;
    xmlNode *c = NULL, *partial = NULL;

    sru_assert (start && lang);

    for (crt = start; crt && xmlStrcmp (crt->name, "literal") == 0; crt= crt->next)
    {
	xmlChar *nlang = xmlNodeGetLang (crt);
	if (nlang && xmlStrcmp (lang, nlang) == 0) /* perfect match */
	{
	    xmlFree (nlang);
	    return crt;
	}
	if (nlang && xmlStrncmp (lang, nlang, 2) == 0) /* partial match */
	    partial = crt;
	if (!nlang && !c)
	    c = crt;
	xmlFree (nlang);
    }

    return partial ? partial : c;
}


static gchar*
src_presentation_get_from_true_false (xmlNode *node)
{
    const xmlNode *crt = NULL;

    if (node && node->xmlChildrenNode)
	crt = src_presentation_get_literal_node_lang (node->xmlChildrenNode, (xmlChar*)srcp_get_lang ());
    if (crt)
	return xmlNodeGetContent ((xmlNode*)crt);
    return NULL;
}

static SRCPresentationItem*
src_presentation_get_presentation_item_from_conditional_ (xmlNode *node,
						   const gchar *device)
{
    xmlNode *crt;
    SRCPresentationItem *item;
    
    sru_assert (xmlStrcmp (node->name, "conditional") == 0);
    
    item = src_presentation_item_new ();
    if (!item)
        return NULL;

    item->type  = SRC_PRESENTATION_INFO_TYPE_COND;
    for (crt = node->xmlChildrenNode; crt; crt= crt->next)
    {
	if (xmlStrcmp (crt->name, "true") == 0)
	    item->data1 = src_presentation_get_from_true_false (crt);
	else if (xmlStrcmp (crt->name, "false") == 0)
	    item->data2 = src_presentation_get_from_true_false (crt);
    }
    item->voice = g_strdup (xmlGetProp (node, "voice"));

    return item;
}

static SRCPresentationItem*
src_presentation_get_presentation_item_from_conditional (xmlNode *node,
						   const gchar *device)
{
    SRCPresentationItem *item;
    
    item = src_presentation_get_presentation_item_from_conditional_ (node, device);
    item->type  = SRC_PRESENTATION_INFO_TYPE_COND;
    item->info = src_presentation_get_item_type (xmlGetProp (node, "condition"));

    return item;
}


static SRCPresentationItem*
src_presentation_get_presentation_item_from_conditional1 (xmlNode *node,
						   const gchar *device)
{
    gchar *cond, *end, *condo;
    SRCPresentationItem *item;
    
    item = src_presentation_get_presentation_item_from_conditional_ (node, device);
    item->type  = SRC_PRESENTATION_INFO_TYPE_COND1;
    cond = xmlGetProp (node, "condition");
    end = strstr (cond, "=1");
    sru_assert (end);
    condo = g_strndup (cond, end - cond);
    item->info = src_presentation_get_item_type (condo);
    g_free (condo);
    
    return item;
}

static SRCPresentationItem*
src_presentation_get_presentation_item_from_attribute (xmlNode *node,
						   const gchar *device)
{
    SRCPresentationItem *item;

    sru_assert (xmlStrcmp (node->name, "attribute") == 0);
    
    item = src_presentation_item_new ();
    if (!item)
        return NULL;

    item->type  = SRC_PRESENTATION_INFO_TYPE_INFO;
    
    item->info = src_presentation_get_item_type (xmlNodeGetContent (node));
    item->voice = g_strdup (xmlGetProp (node, "voice"));

    return item;
}

static SRCPresentationItem*
src_presentation_get_presentation_item_from_paranthesis (xmlNode *node,
							 const gchar *device)
{
    gchar *info;
    SRCPresentationItem *item;

    sru_assert (xmlStrcmp (node->name, "attribute") == 0);
    
    item = src_presentation_item_new ();
    if (!item)
        return NULL;

    item->type  = SRC_PRESENTATION_INFO_TYPE_PARANTHESIS;
    
    info = g_strdup (xmlNodeGetContent (node));
    info[strlen (info) - 1] = '\0';
    item->info = src_presentation_get_item_type (info + 1);
    item->voice = g_strdup (xmlGetProp (node, "voice"));
    g_free (info);

    return item;

}


static gboolean 
src_presentation_get_presentation_items_from_node
	    (xmlNode *node, const gchar *device, SRCPresentationItem  ***presentation);


static SRCPresentationItem*
src_presentation_get_presentation_item_from_children (xmlNode *node,
						       const gchar *device)
{
    SRCPresentationItem *item;
    SRCPresentationItem **item1;

    sru_assert (xmlStrcmp (node->name, "children") == 0);

    item = src_presentation_item_new ();
    if (!item)
        return NULL;

    item->type  = SRC_PRESENTATION_INFO_TYPE_CHILDREN;

    src_presentation_get_presentation_items_from_node (node, device, &item1);
    
    item->data1 = item1;
    item->data2 = g_strdup (xmlGetProp (node, "sep"));

    return item;
}


static gboolean
src_presentation_get_presentation_items_from_node (xmlNode *node,
							      const gchar *device,
							      SRCPresentationItem  ***presentation)
{
    xmlNode *crt;
    GSList *list = NULL;

    sru_assert (presentation);

    *presentation = NULL;
    for (crt = node->xmlChildrenNode; crt; crt = crt->next)
    {
	SRCPresentationItem *item = NULL;
	xmlNode *ant;
	if (xmlStrcmp (crt->name, "literal") == 0)
	{
	    const xmlNode *literal = src_presentation_get_literal_node_lang (crt, (xmlChar*)srcp_get_lang ());
	    if (literal)
		item = src_presentation_get_presentation_item_from_literal ((xmlNode*)literal, device);
	    for (ant = crt ; ant && xmlStrcmp (ant->name, "literal") == 0; ant = ant->next)
		crt = ant;
	}
	else if (xmlStrcmp (crt->name, "brdots") == 0)
	    item = src_presentation_get_presentation_item_from_dots (crt, device);
	else if (xmlStrcmp (crt->name, "conditional") == 0)
	{
	    gchar *cond, *e1;
	    cond = xmlGetProp (crt, "condition");
	    e1 = strstr (cond, "=1");
	    if (e1)
		item = src_presentation_get_presentation_item_from_conditional1 (crt, device);
	    else
		item = src_presentation_get_presentation_item_from_conditional (crt, device);
	}
	else if (xmlStrcmp (crt->name, "attribute") == 0)
	{
	    gchar *attr;
	    attr = xmlNodeGetContent (crt);
	    if (attr[0] == '(')
		item = src_presentation_get_presentation_item_from_paranthesis (crt, device);
	    else
		item = src_presentation_get_presentation_item_from_attribute (crt, device);
	}
	else if (xmlStrcmp (crt->name, "children") == 0)
	    item = src_presentation_get_presentation_item_from_children (crt, device);
	else 
	    sru_warning ("unknown \"%s\" tag", crt->name);

	if (item)
	    list = g_slist_append (list, item);
    }

    if (list)
    {	
	GSList *crt_list;
	gint i;
	*presentation = g_new (SRCPresentationItem *, g_slist_length (list) + 1);
	if (!(*presentation))
	    return FALSE;

	for (crt_list = list, i = 0; crt_list; crt_list = crt_list->next, i++)
	{
	    (*presentation)[i] = (SRCPresentationItem *) crt_list->data;
	}
	(*presentation)[i] = NULL;
	g_slist_free (list);
    }

    return *presentation ? TRUE : FALSE;    
}


static GHashTable *src_presentation_chunks_braille = NULL;
static GHashTable *src_presentation_chunks_speech  = NULL;
static GHashTable *src_presentation_chunks_magnifier  = NULL;

static void*
src_presentation_find_in_hash_table (GHashTable *hash,
				     const gchar *role,
				     const gchar *event)
{
    gchar *key;
    void *rv;

    sru_assert (hash && role && event);
    key = g_strconcat (role, "_", event, NULL);
    rv = g_hash_table_lookup (hash, key);

    g_free (key);

    return rv;
}

static void*
src_presentation_find_chunk_in_hash_table (GHashTable *hash,
					    const gchar *role,
					    const gchar *event)
{
    void *rv;
    
    sru_assert (role && event && hash);

    rv = src_presentation_find_in_hash_table (hash, role, event);
    if (!rv)
	rv = src_presentation_find_in_hash_table (hash, role, "generic");
    if (!rv)
	rv = src_presentation_find_in_hash_table (hash, "generic", event);
    if (!rv)
	rv = src_presentation_find_in_hash_table (hash, "generic", "generic");
    return rv;
}


static gboolean
src_presentation_find_chunk_for_braille (const gchar *role,
					 const gchar *event,
					 SRCPresentationItem ***presentation)
{
    sru_assert (role && event && presentation);
    sru_assert (src_presentation_chunks_braille);


    *presentation = (SRCPresentationItem **)src_presentation_find_chunk_in_hash_table (
			    src_presentation_chunks_braille, role, event);

    return *presentation ? TRUE : FALSE;
}



static gboolean
src_presentation_find_chunk_for_speech (const gchar *role,
					const gchar *event,
					SRCPresentationItem ***presentation)
{
    sru_assert (role && event && presentation);
    sru_assert (src_presentation_chunks_speech);

    *presentation = (SRCPresentationItem **)src_presentation_find_chunk_in_hash_table (
			    src_presentation_chunks_speech, role, event);

    return *presentation ? TRUE : FALSE;
}

static gboolean
srcp_load_pres_for_presentation_for_mag (xmlNode *node,
					 gchar *device,
					 GHashTable *hash)
{
    xmlNode *crt, *crt2, *crt3;

    sru_assert (node && device && hash);
    srl_assert (xmlStrcmp (node->name, "presentation") == 0);

    for (crt = node->xmlChildrenNode; crt; crt = crt->next)
    {
	if (xmlStrcmp (crt->name, "role") == 0)
	{
	    gchar *role = xmlGetProp (crt, "name");
	    for (crt2 = crt->xmlChildrenNode; crt2; crt2 = crt2->next)
	    {
		if (xmlStrcmp (crt2->name, "event") == 0)
		{
		    gchar *event = xmlGetProp (crt2, "name");
		    for (crt3 = crt2->xmlChildrenNode; crt3; crt3 = crt3->next)
		    {
			if (xmlStrcmp (crt3->name, "bounds") == 0)
			{
			    gchar *bounds = xmlGetProp (crt3, "type");
			    gchar *key = g_strconcat (role, "_", event, NULL);
			    SRCPresentationBoundsType btype = SRC_PRESENTATION_BOUNDS_TYPE_OBJECT;
			    if (xmlStrcmp (bounds, "text:char") == 0)
				btype = SRC_PRESENTATION_BOUNDS_TYPE_TEXT_CHAR;
			    else if (xmlStrcmp (bounds, "object") == 0)
				btype = SRC_PRESENTATION_BOUNDS_TYPE_OBJECT;
			    else
				sru_warning ("unknown \"%s\" bouds type", bounds);
			    g_hash_table_insert (hash, key, GINT_TO_POINTER (btype));
			}
		    }
		}
	    }
	}
    }

    return TRUE;
}



static gboolean
srcp_load_pres_for_presentation (xmlNode *node,
				 gchar *device,
				 GHashTable *hash)
{
    xmlNode *crt;
    
    srl_assert (node && device && hash);
    srl_assert (xmlStrcmp (node->name, "presentation") == 0);

    for (crt = node->xmlChildrenNode; crt; crt = crt->next)
    {
	if (xmlStrcmp (crt->name, "role") == 0)
	{
	    gchar *role;
	    xmlNode *crt2;
	    srl_assert (xmlStrcmp (crt->name, "role") == 0);
	    role = xmlGetProp (crt, "name");
	    for (crt2 = crt->xmlChildrenNode; crt2; crt2 = crt2->next)
	    {
		gchar *event;
		gchar *key;
		SRCPresentationItem  **pres;

		srl_assert (xmlStrcmp (crt2->name, "event") == 0);
		event = xmlGetProp (crt2, "name");
		src_presentation_get_presentation_items_from_node (crt2,
		    		device, &pres); 
		/* sru_assert (pres); */
		key = g_strconcat (role, "_", event, NULL);
		g_hash_table_insert (hash, key, pres);
	    }
	}
    }

    return TRUE;
}

static gboolean
srcp_load_pres_from_root_node (xmlNode *rootnode)
{
    xmlNode *crt;

    g_assert (rootnode);
    g_assert (xmlStrcmp (rootnode->name, (const xmlChar *) "PresentationProfile") == 0);

    for (crt = rootnode->xmlChildrenNode; crt; crt = crt->next)
    {
	if (xmlStrcmp (crt->name, (const xmlChar *) "device") == 0)
	{
	    xmlChar *name = xmlGetProp (crt, (const xmlChar *) "name");
	    if (name)
	    {
		if (xmlStrcmp (name, "braille") == 0)
		    srcp_load_pres_for_presentation (crt->xmlChildrenNode, "braille", src_presentation_chunks_braille);
		else if (xmlStrcmp (name, "speech") == 0)
		    srcp_load_pres_for_presentation (crt->xmlChildrenNode, "speech", src_presentation_chunks_speech);
		else if (xmlStrcmp (name, "magnifier") == 0)
		    srcp_load_pres_for_presentation_for_mag (crt->xmlChildrenNode, "magnifier", src_presentation_chunks_magnifier);
		else
		    sru_warning ("Unknown \"%s\" device", name);
	    }
	}
    }
    return TRUE;
}


static gboolean
srcp_load_pres_from_file (gchar *fname)
{
    xmlDoc *doc;
    xmlNode *rootnode;
    gboolean rv = FALSE;

    g_assert (fname);

    xmlKeepBlanksDefault (0);
    if ((doc = xmlParseFile (fname)))
    {
	if ((rootnode = xmlDocGetRootElement (doc))) 
	{
	    if (xmlSearchNsByHref (doc, rootnode, (const xmlChar *) "http://www.gnome.org/gnopernicus"))
	    {
		if (xmlStrcmp (rootnode->name, (const xmlChar *) "PresentationProfile") == 0)
		{
		    rv = srcp_load_pres_from_root_node (rootnode);
		}
		else
		{
		    g_error ("File \"%s\" has not a \"PresentationProfile\" node as root node.", fname);
		}
	    }
	    else
	    {
		g_error ("File \"%s\" has not an incorrect namespace.\n", fname);
	    }
	}
	else 
	{
	    g_error ("File \"%s\" has not a valid root node.\n", fname);
	};
	xmlFreeDoc (doc);
    }
    else
    {
    	g_error ("File \"%s\" cannot be parsed.\n", fname);
    }

    return rv;
} 


gboolean
src_presentation_init (gchar *fname)
{
    src_presentation_chunks_speech = 
		g_hash_table_new_full (g_str_hash, g_str_equal,
				g_free, (GDestroyNotify)src_presentation_items_terminate);
    src_presentation_chunks_braille = 
		g_hash_table_new_full (g_str_hash, g_str_equal,
				g_free, (GDestroyNotify)src_presentation_items_terminate);
    src_presentation_chunks_magnifier = 
		g_hash_table_new_full (g_str_hash, g_str_equal,
				g_free, NULL);

    if (!src_presentation_chunks_speech || !src_presentation_chunks_braille || !src_presentation_chunks_magnifier)
	return FALSE;

    return srcp_load_pres_from_file (fname);
}


gboolean
src_presentation_terminate ()
{
    g_hash_table_destroy (src_presentation_chunks_braille);
    src_presentation_chunks_braille = NULL;
    g_hash_table_destroy (src_presentation_chunks_speech);
    src_presentation_chunks_speech = NULL;
    g_hash_table_destroy (src_presentation_chunks_magnifier);
    src_presentation_chunks_magnifier = NULL;

    return TRUE;
}


static struct
{
    gchar	*role;
    gchar 	*braille_short;
    gchar	*braille_long;
    gchar 	*speech_short;
}src_presentation_role_pres[] =
    {
	{"unknown",		N_("UNK"),	N_("UNKNOWN"),		N_("Unknown")			},
	{"invalid",		N_("???"),	N_("INVALID"),		N_("Invalid")			},
	{"alert",		N_("ALR"),	N_("ALERT"),		N_("Alert")			},
	{"canvas",		N_("CNV"),	N_("CANVAS"),		N_("Canvas")			},
	{"check-box",		N_("CHK"),	N_("CHECK_BOX"),	N_("Check box")			},
	{"check-menu-item",	N_("MIT"),	N_("MENU ITEM"),	N_("Check Menu item")		},
	{"color-chooser",	N_("CCH"),	N_("COLOR CHOOSER"),	N_("Color chooser")		},
	{"column-header",	N_("CHD"),	N_("COLUMN HEADER"),	N_("Column header")		},
	{"combo-box",		N_("CBO"),	N_("COMBO BOX"),	N_("Combo box")			},
	{"desktop-icon",	N_("DIC"),	N_("DESKTOP ICON"),	N_("Desktop icon")		},
	{"desktop-frame",	N_("DFR"),	N_("DESKTOP FRAME"),	N_("Desktop frame")		},
	{"dialog",		N_("DLG"),	N_("DIALOG"),		N_("Dialog")			},
	{"directory-pane",	N_("DIP"),	N_("DIRECTORY PANE"),	N_("Directory pane")		},
	{"file-chooser",	N_("FCH"),	N_("FILE CHOOSER"),	N_("File chooser")		},
	{"filler",		N_("FLR"),	N_("FILLER"),		N_("Filler")			},
	{"frame",		N_("FRM"),	N_("FRAME"),		N_("Frame")			},
	{"glass-pane",		N_("GPN"),	N_("GLASS PANE"),	N_("Glass pane")		},
	{"HTML-container",	N_("HTM"),	N_("HTML CONTAINER"),	N_("H T M L container")		},
	{"hyper link",		N_("LNK"),	N_("LINK"),		N_("link")			},
	{"icon",		N_("ICO"),	N_("ICON"),		N_("Icon")			},
	{"internal-frame",	N_("IFR"),	N_("INTERNAL FRAME"),	N_("Internal frame")		},
	{"label",		N_("LBL"),	N_("LABEL"),		N_("Label")			},
	{"layered-pane",	N_("LPN"),	N_("LAYERED PANE"),	N_("Layered pane")		},
	{"link",		N_("LNK"),	N_("LINK"),		N_("link")			},
	{"list",		N_("LST"),	N_("LIST"),		N_("List")			},
	{"list-item",		N_("LIT"),	N_("LIST ITEM"),	N_("List item")			},
	{"menu",		N_("MNU"),	N_("MENU"),		N_("Menu")			},
	{"menu-bar",		N_("MBR"),	N_("MENU BAR"),		N_("Menu bar")			},
	{"menu-item",		N_("MIT"),	N_("MENU ITEM"),	N_("Menu item")			},
	{"option-pane",		N_("OPN"),	N_("OPTION PANE"),	N_("Option pane")		},
	{"page-tab",		N_("PGT"),	N_("PAGE TAB"),		N_("Page tab")			},
	{"page-tab-list",	N_("PTL"),	N_("PAGE TAB LIST"),	N_("Page tab list")		},
	{"panel",		N_("PNL"),	N_("PANEL"),		N_("Panel")			},
	{"password-text",	N_("PWD"),	N_("PASSWORD TEXT"),	N_("Password text")		},
	{"popup-menu",		N_("PMN"),	N_("POPUP MENU"),	N_("Popup menu")		},
	{"progress-bar",	N_("PRG"),	N_("PROGRESS BAR"),	N_("Progress bar")		},
	{"push-button",		N_("PBT"),	N_("PUSH BUTTON"),	N_("Push button")		},
	{"radio-button",	N_("RAD"),	N_("RADIO BUTTON"),	N_("Radio button")		},
	{"radio-menu-item",	N_("MIT"),	N_("MENU ITEM"),	N_("Radio Menu item")		},
	{"root-pane",		N_("RPN"),	N_("ROOT PANE"),	N_("Root pane")			},
	{"row-header",		N_("RHD"),	N_("ROW HEADER"),	N_("Row header")		},
	{"scroll-bar",		N_("SCR"),	N_("SCROLL BAR"),	N_("Scroll bar")		},
	{"scroll-pane",		N_("SPN"),	N_("SCROLL PANE"),	N_("Scroll pane")		},
	{"separator",		N_("SEP"),	N_("SEPARATOR"),	N_("Separator")			},
	{"slider",		N_("SLD"),	N_("SLIDER"),		N_("Slider")			},
	{"split-pane",		N_("SPP"),	N_("SPLIT PANE"),	N_("Split pane")		},
	{"status-bar",		N_("STA"),	N_("STATUS BAR"),	N_("Status bar")		},
	{"table",		N_("TAB"),	N_("TABLE"),		N_("Table")			},
	{"table-cell",		N_("CEL"),	N_("TABLE CELL"),	N_("Table cell")		},
	{"table-column-header",	N_("TCH"),	N_("TABLE COLUMN HEADER"),N_("Table column header")	},
	{"table-row-header",	N_("TRH"),	N_("TABLE ROW HEADER"),	N_("Table row header")		},
	{"multi-line-text",	N_("TXT"),	N_("MULTI LINE TEXT"),	N_("Multi Line Text")		},
	{"single-line-text",	N_("TXT"),	N_("SINGLE LINE TEXT"),	N_("Single Line Text")		},
	{"toggle-button",	N_("TOG"),	N_("TOGGLE BUTTON"),	N_("Toggle button")		},
	{"tool-bar",		N_("TOL"),	N_("TOOL BAR"),		N_("Tool bar")			},
	{"tool-tip",		N_("TIP"),	N_("TOOL TIP"),		N_("Tool tip")			},
	{"tree",		N_("TRE"),	N_("TREE"),		N_("Tree")			},
	{"tree-item",		N_("TRI"),	N_("TREE ITEM"),	N_("Tree item")			},
	{"tree-table",		N_("TRT"),	N_("TREE TABLE"),	N_("Tree table")		},
	{"viewport",		N_("VWP"),	N_("VIEWPORT"),		N_("Viewport")			},
	{"window",		N_("WND"),	N_("WINDOW"),		N_("Window")			},
	{"accelerator-label",	N_("ACC"),	N_("ACCELERATOR LABEL"),N_("Accelerator label")		},
	{"animation",		N_("ANI"),	N_("ANIMATION"),	N_("Animation")			},
	{"arrow",		N_("ARR"),	N_("ARROW"),		N_("Arrow")			},
	{"calendar",		N_("CAL"),	N_("CALENDAR"),		N_("Calendar")			},
	{"date-editor",		N_("DAT"),	N_("DATE EDITOR"),	N_("Date editor")		},
	{"dial",		N_("DIL"),	N_("DIAL"),		N_("Dial")			},
	{"drawing-area",	N_("DRW"),	N_("DRAWING AREA"),	N_("Drawing area")		},
	{"font-chooser",	N_("FNT"),	N_("FONT CHOOSER"),	N_("Font chooser")		},
	{"image",		N_("IMG"),	N_("IMAGE"),		N_("Image")			},
	{"spin-button",		N_("SPN"),	N_("SPIN BUTTON"),	N_("Spin button")		},
	{"terminal",		N_("TRM"),	N_("TERMINAL"),		N_("Terminal")			},
	{"extended",		N_("EXT"),	N_("EXTENDED"),		N_("Extended")			},
	{"table-line",		N_("TLI"),	N_("TABLE LINE"),	N_("Table line")		},
	{"table-columns-header",N_("TCH"),	N_("TABLE COLUMNS HEADER"),N_("Table columns header")	},
	{"title-bar",		N_("TIT"),	N_("TITLE BAR"),	N_("Title bar")			},
	{"edit-bar",		N_("EDB"),	N_("EDIT BAR"),		N_("Edit bar")			}
    };

static gint
src_presentation_get_role_index (const gchar *role)
{
    gint i;

    sru_assert (role);

    for (i = 0; i < G_N_ELEMENTS (src_presentation_role_pres); i++)
	if (strcmp (src_presentation_role_pres[i].role, role) == 0)
	    return i;
    
    return 0;
}




static G_CONST_RETURN gchar* 
src_presentation_get_role_name_for_braille (const gchar *role)
{
    gint index;
    sru_assert (role);

    index = src_presentation_get_role_index (role);

    if (index)
	return _(src_presentation_role_pres[index].braille_short);
    else
	return role;
}

static G_CONST_RETURN gchar* 
src_presentation_get_role_name_for_speech (const gchar *role)
{
    gint index;

    sru_assert (role);

    index = src_presentation_get_role_index (role);

    if (index)
	return _(src_presentation_role_pres[index].speech_short);
    else
	return role;
}


G_CONST_RETURN gchar* 
src_presentation_get_role_name_for_device (const gchar *role,
					   const gchar *device)
{
    if (strcmp (device, "braille") == 0)
	return src_presentation_get_role_name_for_braille (role);
    else if (strcmp (device, "speech") == 0)
	return src_presentation_get_role_name_for_speech (role);
    else
	sru_assert_not_reached ();

    return NULL;
}



static gboolean
src_presentation_get_info_from_sro (SRObject *obj,
				    SRCPresentationItemType info_type,
				    const gchar *device,
				    gpointer info)
{
    gboolean rv = FALSE;

    sru_assert (obj && info && device);

    if (info)
	*((gchar**)info) = NULL;

    switch (info_type)
    {
	case SRC_PRESENTATION_LEVEL:
	    {
		SRLong tmp;
		rv = sro_tree_item_get_level (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{	
		    *((gchar **)info) = g_strdup_printf ("%ld", tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_NAME:
	    {
		gchar *tmp;
		rv = sro_get_name (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{	
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_DESCRIPTION:
	    {
		gchar *tmp;
		rv = sro_get_description (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{	
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_ROLE:
	    {
		gchar *tmp;
		sro_get_role_name (obj, &tmp, SR_INDEX_CONTAINER);
		
		if (tmp)
		{
		    G_CONST_RETURN gchar *tmp2;
		    
		    tmp2 = src_presentation_get_role_name_for_device (tmp, device);
		    if (tmp2)
		    {
			*((gchar **)info) = g_strdup (tmp2);
			rv = TRUE;
		    }
		    SR_freeString (tmp);
		}
	    }    
	    break;
	case SRC_PRESENTATION_ACCELERATOR:
	    {
		gchar *tmp;
		rv = sro_get_accelerator (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }    
	    break;
	case SRC_PRESENTATION_SHORTCUT:
	    {
		gchar *tmp;
		rv = sro_get_shortcut (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }    
	    break;
	case SRC_PRESENTATION_STATE_CHECKED:
	    {
		SRState state;
		if (sro_get_state (obj, &state, SR_INDEX_CONTAINER))
		    rv = state & SR_STATE_CHECKED ? TRUE : FALSE;
	    }
	    break;
	case SRC_PRESENTATION_STATE_UNCHECKED:
	    {
		SRState state;
		if (sro_get_state (obj, &state, SR_INDEX_CONTAINER))
		    rv = (state & SR_STATE_CHECKABLE) && !(state & SR_STATE_CHECKED) ? TRUE : FALSE;
	    }
	    break;
	case SRC_PRESENTATION_STATE_EXPANDED:
	    {
		SRState state;
		if (sro_get_state (obj, &state, SR_INDEX_CONTAINER))
		    rv = state & SR_STATE_EXPANDED ? TRUE : FALSE;
	    }
	    break;
	case SRC_PRESENTATION_STATE_COLLAPSED:
	    {
		SRState state;
		if (sro_get_state (obj, &state, SR_INDEX_CONTAINER))
		    rv = (state & SR_STATE_EXPANDABLE) && !(state & SR_STATE_EXPANDED) ? TRUE : FALSE;
	    }
	    break;
	case SRC_PRESENTATION_STATE_MINIMIZED:
	    {
		SRState state;
		if (sro_get_state (obj, &state, SR_INDEX_CONTAINER))
		    rv = (state & SR_STATE_MINIMIZED) ? TRUE : FALSE;
	    }
	    break;
	case SRC_PRESENTATION_STATE_ENABLED:
	    {
		SRState state;
		if (sro_get_state (obj, &state, SR_INDEX_CONTAINER))
		    rv = state & SR_STATE_ENABLED;
	    }
	    break;
	case SRC_PRESENTATION_CHILDREN_COUNT:
	    {
		guint32 cc;
		if (!sro_get_children_count (obj, &cc))
		    cc = 0;
		*((gchar **)info) = g_strdup_printf ("%d", cc);
		rv = TRUE;
	    }
	    break;
	case SRC_PRESENTATION_TEXT_LINE:
	    {
		gchar *tmp;
		rv = sro_text_get_text_from_caret (obj, SR_TEXT_BOUNDARY_LINE,
					    &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_TEXT_BEST:
	    {
		static SRLong last_offset = 0, last_offset2 = 0;
		SRLong crt_offset;
		SRTextBoundaryType type;
		gchar *tmp;
		sro_text_get_abs_offset (obj, &crt_offset, SR_INDEX_CONTAINER);
		
		type = SR_TEXT_BOUNDARY_LINE;
		
		if (last_offset == crt_offset)
		    last_offset = last_offset2;
		if (abs (crt_offset - last_offset) == 1 &&
		    sro_text_is_same_line (obj, last_offset, SR_INDEX_CONTAINER))
		    type = SR_TEXT_BOUNDARY_CHAR;
		else if (abs (crt_offset - last_offset) == 1 &&
		         !sro_text_is_same_line (obj, last_offset, SR_INDEX_CONTAINER))
		    type = SR_TEXT_BOUNDARY_LINE;    
		else if (sro_text_is_same_line (obj, last_offset, SR_INDEX_CONTAINER) ||
		         sro_is_word_navigation (obj, crt_offset, last_offset, SR_INDEX_CONTAINER))
		    type = SR_TEXT_BOUNDARY_WORD;
		else
		    type = SR_TEXT_BOUNDARY_LINE;
		if (sro_text_get_text_from_caret (obj, type,
				&tmp, SR_INDEX_CONTAINER))
		    {
			*((gchar **)info) = g_strdup (tmp);
			rv = TRUE;
			SR_freeString (tmp);
		    }
		last_offset2 = last_offset;
		last_offset = crt_offset;
	    }
	    break;
	case SRC_PRESENTATION_TEXT_CHAR:
	    {
		gchar *tmp;
		rv = sro_text_get_text_from_caret (obj, SR_TEXT_BOUNDARY_CHAR,
					    &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_TEXT_SELECTION:
	    {
		gchar **tmp;
		rv = sro_text_get_selections (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp[0]);
		    SR_strfreev (tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_TEXT_DIFFERENCE:
	    {
		gchar *tmp;
		rv = sro_text_get_difference (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_VALUE_CRT:
	    {
		gdouble val;
		rv = FALSE;
		if (sro_value_get_crt_val (obj, &val, SR_INDEX_CONTAINER))
		{
		    if (val == (int)val)
			*((gchar **)info) = g_strdup_printf ("%.0f", val);
		    else	
			*((gchar **)info) = g_strdup_printf ("%.2f", val);
		    rv = TRUE;
		}
	    }
	    break;
	case SRC_PRESENTATION_COLUMN_HEADER:
	    {
		gchar *tmp;
		rv = sro_get_column_header (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }
	    break;
	case SRC_PRESENTATION_ROW_HEADER:
	    {
		gchar *tmp;
		rv = sro_get_row_header (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }
	    break;    
	case SRC_PRESENTATION_CELL:
	    {
		gchar *tmp;
		rv = sro_get_cell (obj, &tmp, SR_INDEX_CONTAINER);
		if (rv)
		{
		    *((gchar **)info) = g_strdup (tmp);
		    SR_freeString (tmp);
		}
	    }    
	    break;
	default:
	    fprintf (stderr, "\n%d", info_type);/* removable */
	    sru_assert_not_reached ();	
	    break;
    }
    if (info && *(gchar**)info)
    {
	if (!g_utf8_validate (*((gchar**)info), -1, NULL))
	{
	    sru_warning ("Invalid info queried (%d) ", info_type);
	    g_free (*(gchar**)info);
	    *(gchar**)info = NULL;
	    rv = FALSE;
	}
    }
    return rv;
}


static gchar*
src_presentation_sro_get_pres_for_device_ (SRObject *obj,
					  const gchar *device,
					  SRCPresentationItem **chunk)
{
    gint i;
    gchar *rv;
    gboolean has_voice;
    sru_assert (obj && chunk && *chunk);

    has_voice = strcmp (device, "speech") == 0 ? TRUE : FALSE;
    i = 0;
    rv = g_strdup ("");
    while (chunk[i])
    {
	gchar *add = NULL;

	switch (chunk[i]->type)
	{
	    case SRC_PRESENTATION_INFO_TYPE_INFO:
		{
		    gchar *info;
		    if (src_presentation_get_info_from_sro (obj, chunk[i]->info, device, &info))
		    {
			if (info)
			{
			    add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, info);
			    if (chunk[i]->info != SRC_PRESENTATION_TEXT_LINE)
					src_caret_position += strlen (info);
				g_free (info);
			}
		    }
		}
		break;
	    case SRC_PRESENTATION_INFO_TYPE_CHILDREN:
		{
		    guint32 cnt, j;
		    
		    add = g_strdup ("");		    
		    sro_get_children_count (obj, &cnt);
		    if (!sro_manages_descendants (obj)) 
		    {
		        for (j = 0; j < cnt; j++)
			{
			    SRObject *child;
			    gchar *tmp, *tmp2, *tmp3;
			  
			    if (!sro_get_i_child (obj, j, &child))
			      continue;
			    
			    tmp = add;
			    tmp2 = src_presentation_sro_get_pres_for_device_ (child, device, (SRCPresentationItem **)chunk[i]->data1);
			    
			    tmp3 = NULL;
			    if (j != cnt - 1 && chunk[i]->data2)
			    {
			        gchar *tmp4;
				tmp4 = g_strdup_printf ("dot%s", (gchar *)chunk[i]->data2);
				if (tmp4)
				{
				    tmp3 = src_xml_make_part ("DOTS", NULL, tmp4);
				    g_free (tmp4);
				}
			    }
			    add = g_strconcat (add, tmp2, tmp3, NULL);
			    g_free (tmp);
			    g_free (tmp2);
			    g_free (tmp3);
			    sro_release_reference (child);
			}
		    }
		    else
		    {
		      /* FIXME: presentation of first N visible children?
			 Or just a notation of "Object contains %d children" ?
		      */
		    }
		}
		break;
	    case SRC_PRESENTATION_INFO_TYPE_COND:
		{
		    gchar *info;
		    if (src_presentation_get_info_from_sro (obj, chunk[i]->info, device, &info))
		    {
			add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, (gchar *)chunk[i]->data1);
			if (info)
			{
				src_caret_position += strlen (info);
			    g_free (info);
			}
		    }
		    else
		    {
			add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, (gchar *)chunk[i]->data2);
		    }
		}
		break;
	    case SRC_PRESENTATION_INFO_TYPE_PARANTHESIS:
		{
		    gchar *info, *info2;
		    info2 = NULL;
		    if (src_presentation_get_info_from_sro (obj, chunk[i]->info, device, &info))
		    {
			info2 = g_strconcat ("(", info, ")", NULL);
			g_free (info);
		    }
		    
		    if (info2)
		    {
			add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, info2);
			src_caret_position +=  strlen (info2);
			g_free (info2);
		    }
		}
		break;
	    case SRC_PRESENTATION_INFO_TYPE_COND1:
		{
		    gchar *info;
		    if (src_presentation_get_info_from_sro (obj, chunk[i]->info, device, &info))
		    {
			if (info && strcmp (info, "1") ==0)
			    add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, (gchar *)chunk[i]->data1);
			else
			    add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, (gchar *)chunk[i]->data2);
			if (info)
			{
			   src_caret_position += strlen (info);
			   g_free (info);
			}
		    }
		    else
		    {
			add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, (gchar *)chunk[i]->data2);
		    }
		}
		break;
	    case SRC_PRESENTATION_INFO_TYPE_TEXT:
		if (chunk[i]->data1)
		    add = src_xml_format ("TEXT", has_voice ? src_speech_get_voice ((gchar *)chunk[i]->voice) : NULL, (gchar *)chunk[i]->data1);
		break; 
	    case SRC_PRESENTATION_INFO_TYPE_SPECIAL:
		{
		    if (chunk[i]->data1)
		    {
			gchar *tmp;
			tmp = g_strdup_printf ("dot%s", (gchar *)chunk[i]->data1);
			if (tmp)
			{
			    add = src_xml_make_part ("DOTS", NULL, tmp);
				src_caret_position += strlen (tmp);
			    g_free (tmp);
			}
		    }
		}
		break;
	    default:
		sru_assert_not_reached ();
	}
	if (add)
	{
	    gchar *tmp = rv;
	    if (strcmp (device, "braille") == 0)
	    {
		src_caret_position ++;
		rv = g_strconcat (rv, rv[0] != '\0' ? "<TEXT> </TEXT>" : "", add, NULL); 
	    }
	    else
	    {
		rv = g_strconcat (rv, add, NULL);
	    }
	    g_free (tmp);
	}
	i++;
    }
    /*fprintf (stderr, "\n%s", rv);*/
    return rv;
}



gchar*
src_presentation_sro_get_pres_for_magnifier (SRObject *obj, 
					    const gchar *reason)
{
    gchar *role;
    SRCPresentationBoundsType btype;
    gchar *presentation_ = NULL;

    sru_assert (obj && reason);

    if (sro_get_role_name (obj, &role, SR_INDEX_CONTAINER))
    {
	SRRectangle location;
	gboolean get = TRUE;
	btype = (SRCPresentationBoundsType)(GPOINTER_TO_INT (
				src_presentation_find_chunk_in_hash_table (
					src_presentation_chunks_magnifier, role, reason)));
	switch (btype)
	{
	    case  SRC_PRESENTATION_BOUNDS_TYPE_OBJECT:
		    sro_get_location (obj, SR_COORD_TYPE_SCREEN, &location, SR_INDEX_CONTAINER);
		    break;
	    case SRC_PRESENTATION_BOUNDS_TYPE_TEXT_CHAR:
		    sro_text_get_caret_location (obj, SR_COORD_TYPE_SCREEN, 
				&location, SR_INDEX_CONTAINER);
		    break;
/*	    case SRC_PRESENTATION_BOUNDS_TYPE_TEXT_LINE:
		     sro_text_get_text_location_from_caret (obj, SR_TEXT_BOUNDARY_LINE,
	    	    	    SR_COORD_TYPE_SCREEN, &location, SR_INDEX_CONTAINER);
		    break;
*/
	    default:
		get = FALSE;
		sru_assert_not_reached ();
		break;	    
	}
	if (get)
	{
	    presentation_ = g_strdup_printf ("ROILeft =\"%d\" ROITop =\"%d\" ROIWidth =\"%d\" ROIHeight=\"%d\"",
					location.x,
					location.y,
					location.x + location.width,
					location.y + location.height);
	}
    	SR_freeString (role);    
    }
    return presentation_;
}


gchar*
src_presentation_sro_get_pres_for_speech (SRObject *obj, 
					 const gchar *reason)
{
    SRCPresentationItem **presentation;
    gchar *role;
    gchar *device = "speech";
    gchar *presentation_ = NULL;

    sru_assert (obj && reason);

    if (sro_get_role_name (obj, &role, SR_INDEX_CONTAINER))
    {
	if (strcmp (reason, "window:create") == 0 && strcmp (role, "alert") == 0)
	{
	    gchar *title, *text, *button, *add1, *add2, *add3;

	    title = text = button = NULL;
	    add1 = add2 = add3 = NULL;
	    sro_alert_get_info (obj, &title, &text, &button);
	    if (title)
	    {
		add1 = src_xml_format ("TEXT", src_speech_get_voice ("name"), title);
		SR_freeString (title);
	    }
	    if (text)
	    {
		add2 = src_xml_format ("TEXT", src_speech_get_voice ("name"), text);
		SR_freeString (text);
	    }
	    if (button)
	    {
		add3 = src_xml_format ("TEXT", src_speech_get_voice ("name"), button);
		SR_freeString (button);
	    }
	    if (add1 || add2 || add3)
	    {
		presentation_ = g_strconcat (add1 ? add1 : "", add2 ? add2 : "", add3 ? add3 : "", NULL);
	    }
	    g_free (add1);
	    g_free (add2);
	    g_free (add3);
	}
	else if (src_presentation_find_chunk_for_speech (role, reason, &presentation))
	{
	    presentation_ = src_presentation_sro_get_pres_for_device_ (obj, device, presentation);
	}
	SR_freeString (role);    
    }

    return presentation_;
}

gchar*
src_presentation_sro_get_pres_for_braille (SRObject *obj, 
					  const gchar *reason)
{
    SRCPresentationItem  **presentation;
    gchar *role;
    gchar *device = "braille";
    gchar *presentation_ = NULL;

    sru_assert (obj && reason);

    if (sro_get_role_name (obj, &role, SR_INDEX_CONTAINER))
    {
	if (src_presentation_find_chunk_for_braille (role, reason, &presentation))
	{
	    presentation_ = src_presentation_sro_get_pres_for_device_ (obj, device, presentation);
	}
	SR_freeString (role);    
    }

    return presentation_;
}


gchar *
src_presentation_sro_get_pres_for_device (SRObject *obj, 
					 const gchar *reason, 
					 const gchar *device)
{
    gchar *presentation_ = NULL;
    
    sru_assert (obj && device && reason);

    if (strcmp (device, "magnifier") == 0)
	presentation_ = src_presentation_sro_get_pres_for_magnifier (obj, reason);
    else  if (strcmp (device, "speech") == 0)
	presentation_ = src_presentation_sro_get_pres_for_speech (obj, reason);
    else if (strcmp (device, "braille") == 0)
	presentation_ = src_presentation_sro_get_pres_for_braille (obj, reason);
    else
	sru_assert_not_reached ();

    return presentation_;
}


G_CONST_RETURN gchar* 
src_pres_get_role_name_for_speech (const gchar *role)
{
    gint index;

    sru_assert (role);

    index = src_presentation_get_role_index (role);

    if (index)
	return _(src_presentation_role_pres[index].speech_short);
    else
	return role;
}
