/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-node.c: Code to manipulate BonoboUINode objects
 *
 * Author:
 *	Havoc Pennington <hp@redhat.com>
 *
 * Copyright 2000 Red Hat, Inc.
 */

#include "config.h"
#include <bonobo/bonobo-ui-node.h>

#include <gnome-xml/parser.h>
#include <gnome-xml/parserInternals.h>
#include <gnome-xml/xmlmemory.h>

/* Having this struct here makes debugging nicer. */
struct _BonoboUINode {
	xmlNode real_node;
};

#define XML_NODE(x) (&(x)->real_node)
#define BNODE(x) ((BonoboUINode *)(x))

BonoboUINode*
bonobo_ui_node_new (const char   *name)
{
        return BNODE (xmlNewNode (NULL, name));
}

BonoboUINode*
bonobo_ui_node_new_child (BonoboUINode *parent,
                          const char   *name)
{
        return BNODE (xmlNewChild (XML_NODE (parent), NULL, name, NULL));
}

BonoboUINode*
bonobo_ui_node_copy (BonoboUINode *node,
                     gboolean recursive)
{
        return BNODE (xmlCopyNode (XML_NODE (node), recursive));
}

void
bonobo_ui_node_free (BonoboUINode *node)
{
        xmlFreeNode (XML_NODE (node));
}

void
bonobo_ui_node_set_data (BonoboUINode *node,
                         gpointer      data)
{
        XML_NODE (node)->_private = data;
}

gpointer
bonobo_ui_node_get_data (BonoboUINode *node)
{
        return XML_NODE (node)->_private;
}

static xmlAttrPtr
get_attr (xmlNode *node, const char *name)
{
        xmlAttrPtr prop;

        if ((node == NULL) || (name == NULL)) return(NULL);
        /*
         * Check on the properties attached to the node
         */
        prop = node->properties;
        while (prop != NULL) {
                if (!xmlStrcmp(prop->name, name))  {
                        return(prop);
                }
                prop = prop->next;
      }
        
      return(NULL);
}

void
bonobo_ui_node_set_attr (BonoboUINode *node,
                         const char   *name,
                         const char   *value)
{
        if (value == NULL) {
                xmlAttrPtr attr = get_attr (XML_NODE (node), name);
                if (attr)
                        xmlRemoveProp (attr);
        } else {
                xmlSetProp (XML_NODE (node), name, value);
        }
}

char*
bonobo_ui_node_get_attr (BonoboUINode *node,
                         const char   *name)
{
        return xmlGetProp (XML_NODE (node), name);
}

gboolean
bonobo_ui_node_has_attr (BonoboUINode *node,
                         const char   *name)
{
        return get_attr (XML_NODE (node), name) != NULL;
}

void
bonobo_ui_node_remove_attr (BonoboUINode *node,
                            const char   *name)
{
        xmlAttrPtr attr = get_attr (XML_NODE (node), name);
        if (attr)
                xmlRemoveProp (attr);
}

void
bonobo_ui_node_add_child   (BonoboUINode *parent,
                            BonoboUINode *child)
{
        xmlAddChild (XML_NODE (parent), XML_NODE (child));
}

void
bonobo_ui_node_insert_before (BonoboUINode *sibling,
                              BonoboUINode *prev_sibling)
{
        xmlAddPrevSibling (XML_NODE (sibling), XML_NODE (prev_sibling));
}

void
bonobo_ui_node_unlink (BonoboUINode *node)
{
	xmlUnlinkNode (XML_NODE (node));
}

void
bonobo_ui_node_replace     (BonoboUINode *old_node,
			    BonoboUINode *new_node)
{
	/* libxml has these args indisputably backward */
	xmlReplaceNode (XML_NODE (new_node),
			XML_NODE (old_node));
}

void
bonobo_ui_node_set_content (BonoboUINode *node,
                            const char   *content)
{
        xmlNodeSetContent (XML_NODE (node), content);
}

char *
bonobo_ui_node_get_content (BonoboUINode *node)
{
        return xmlNodeGetContent (XML_NODE (node));
}

BonoboUINode*
bonobo_ui_node_next (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->next);
}

BonoboUINode*
bonobo_ui_node_prev (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->prev);
}

BonoboUINode*
bonobo_ui_node_children (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->xmlChildrenNode);
}

BonoboUINode*
bonobo_ui_node_parent (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->parent);
}

const char*
bonobo_ui_node_get_name (BonoboUINode *node)
{
        return XML_NODE (node)->name;
}

gboolean
bonobo_ui_node_has_name (BonoboUINode *node,
			 const char   *name)
{
        return strcmp (XML_NODE (node)->name, name) == 0;
}

void
bonobo_ui_node_free_string (char *str)
{
        if (str)
                xmlFree (str);
}

char *
bonobo_ui_node_to_string (BonoboUINode *node,
			  gboolean      recurse)
{
	xmlDoc     *doc;
	xmlChar    *mem = NULL;
	int         size;

	doc = xmlNewDoc ("1.0");
	g_return_val_if_fail (doc != NULL, NULL);

	doc->xmlRootNode = XML_NODE(bonobo_ui_node_copy (node, TRUE));
	g_return_val_if_fail (doc->xmlRootNode != NULL, NULL);

	if (!recurse && bonobo_ui_node_children (BNODE (doc->xmlRootNode))) {
		BonoboUINode *tmp;
		while ((tmp = bonobo_ui_node_children (BNODE (doc->xmlRootNode)))) {
			xmlUnlinkNode (XML_NODE(tmp));
			bonobo_ui_node_free (tmp);
		}
	}

	xmlDocDumpMemory (doc, &mem, &size);

	g_return_val_if_fail (mem != NULL, NULL);

	xmlFreeDoc (doc);

	return mem;
}

BonoboUINode*
bonobo_ui_node_from_string (const char *xml)
{
	/* We have crap error reporting for this function */
	xmlDoc  *doc;
	BonoboUINode *node;
	
	doc = xmlParseDoc ((char *)xml);
	if (!doc)
		return NULL;
	
	node = BNODE (doc->xmlRootNode);

	doc->xmlRootNode = NULL;
	
	xmlFreeDoc (doc);

	return node;
}

BonoboUINode*
bonobo_ui_node_from_file (const char *fname)
{
	/* Error reporting blows here too (because it blows
	 * in libxml)
	 */
	xmlDoc  *doc;
	BonoboUINode *node;

	g_return_val_if_fail (fname != NULL, NULL);
	
	doc = xmlParseFile (fname);

	g_return_val_if_fail (doc != NULL, NULL);

	node = BNODE (doc->xmlRootNode);

	doc->xmlRootNode = NULL;
	xmlFreeDoc (doc);

	return node;
}

gboolean
bonobo_ui_node_transparent (BonoboUINode *node)
{
	xmlNode *n = XML_NODE (node);
	gboolean ret = FALSE;

	g_return_val_if_fail (n != NULL, TRUE);

	if (n->content) {
		ret = FALSE;

	} else if (!n->properties) {
		ret = TRUE;

	} else if (!n->properties->next) {
		if (!strcmp (n->properties->name, "name"))
			ret = TRUE;
	}

	return ret;
}

void
bonobo_ui_node_copy_attrs (BonoboUINode *src,
			   BonoboUINode *dest)
{
	xmlAttr *attr;
	
	for (attr = XML_NODE (src)->properties; attr; attr = attr->next) {
		char *txt = xmlGetProp (XML_NODE (src), attr->name);

		g_assert (txt != NULL);

		xmlSetProp (XML_NODE (dest), attr->name, txt);

		xmlFree (txt);
	}
}
