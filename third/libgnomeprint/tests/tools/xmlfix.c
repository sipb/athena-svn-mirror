/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* LGPL code, mostly stolen from libxml */

#include <libxml/tree.h>
#include <glib.h>
#include <popt.h>
#include <string.h>
#include <unistd.h>

static gint debug;

static struct poptOption options[] = {
	{ "debug",   '\0', POPT_ARG_INT, &debug, 0,
	  "debug level",          "0,1,2,3"},
	{ NULL }
};

static void
my_error (const gchar *format, ...)
{
	va_list args;
	gchar buffer [2048];
	
	va_start (args, format);
	g_print ("Fatal error:\n    ");
	vsnprintf (buffer, 2048, format, args);
	g_print (buffer);
	va_end (args);
	g_print ("\nAborting ...\n");
	exit (-1);
}

/* #undef xmlStringText */
const xmlChar xmlStringText[] = { 't', 'e', 'x', 't', 0 };
/* #undef xmlStringTextNoenc */
const xmlChar xmlStringTextNoenc[] =
              { 't', 'e', 'x', 't', 'n', 'o', 'e', 'n', 'c', 0 };
/* #undef xmlStringComment */
const xmlChar xmlStringComment[] = { 'c', 'o', 'm', 'm', 'e', 'n', 't', 0 };

static xmlNodePtr xmlStaticCopyNodeList (xmlNodePtr node, xmlDocPtr doc, xmlNodePtr parent);


#define UPDATE_LAST_CHILD_AND_PARENT(n) if ((n) != NULL) {		\
    xmlNodePtr ulccur = (n)->children;					\
    if (ulccur == NULL) {						\
        (n)->last = NULL;						\
    } else {								\
        while (ulccur->next != NULL) {					\
	       	ulccur->parent = (n);					\
		ulccur = ulccur->next;					\
	}								\
	ulccur->parent = (n);						\
	(n)->last = ulccur;						\
}}

static xmlNodePtr
xmlStaticCopyNode
(const xmlNodePtr node, xmlDocPtr doc, xmlNodePtr parent, int recursive)
{
	xmlNodePtr ret;
	gint i;
	gchar *s;
	gboolean ignore;

	if (node == NULL)
		return(NULL);
	switch (node->type) {
        case XML_TEXT_NODE:
		i = 0;
		s = node->content;
		ignore = FALSE;
		while (s[i] != 0) {
			if (s[i] != ' ' &&
			    s[i] != '\t' &&
			    s[i] != '\r' &&
			    s[i] != '\n')
				ignore = TRUE;
			i++;
		}
		if (!ignore) {
			/*	
			g_print ("remove ->%s<-\n", node->content);
			*/
			return NULL;
		}
        case XML_COMMENT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ELEMENT_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
		break;
        case XML_ATTRIBUTE_NODE:
		return((xmlNodePtr) xmlCopyProp(parent, (xmlAttrPtr) node));
        case XML_NAMESPACE_DECL:
		return((xmlNodePtr) xmlCopyNamespaceList((xmlNsPtr) node));
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
#ifdef LIBXML_DOCB_ENABLED
        case XML_DOCB_DOCUMENT_NODE:
#endif
		return((xmlNodePtr) xmlCopyDoc((xmlDocPtr) node, recursive));
        case XML_DOCUMENT_TYPE_NODE:
        case XML_DOCUMENT_FRAG_NODE:
        case XML_NOTATION_NODE:
        case XML_DTD_NODE:
        case XML_ELEMENT_DECL:
        case XML_ATTRIBUTE_DECL:
        case XML_ENTITY_DECL:
		return(NULL);
	}
	
	/*
	 * Allocate a new node and fill the fields.
	 */
	ret = (xmlNodePtr) xmlMalloc(sizeof(xmlNode));
	if (ret == NULL) {
		xmlGenericError(xmlGenericErrorContext,
				"xmlStaticCopyNode : malloc failed\n");
		return(NULL);
	}
	memset(ret, 0, sizeof(xmlNode));
	ret->type = node->type;
	
	ret->doc = doc;
	ret->parent = parent; 
	if (node->name == xmlStringText)
		ret->name = xmlStringText;
	else if (node->name == xmlStringTextNoenc)
		ret->name = xmlStringTextNoenc;
	else if (node->name == xmlStringComment)
		ret->name = xmlStringComment;
	else if (node->name != NULL)
		ret->name = xmlStrdup(node->name);
	if ((node->type != XML_ELEMENT_NODE) &&
	    (node->content != NULL) &&
	    (node->type != XML_ENTITY_REF_NODE) &&
	    (node->type != XML_XINCLUDE_END) &&
	    (node->type != XML_XINCLUDE_START)) {
		ret->content = xmlStrdup(node->content);
	}else{
		if (node->type == XML_ELEMENT_NODE)
			ret->content = (void*)(long) node->content;
	}
	if (parent != NULL) {
		xmlNodePtr tmp;
		
		tmp = xmlAddChild(parent, ret);
		/* node could have coalesced */
		if (tmp != ret)
			return(tmp);
	}
	
	if (!recursive) return(ret);
	if (node->nsDef != NULL)
		ret->nsDef = xmlCopyNamespaceList(node->nsDef);
	
	if (node->ns != NULL) {
		xmlNsPtr ns;
		
		ns = xmlSearchNs(doc, ret, node->ns->prefix);
		if (ns == NULL) {
			/*
			 * Humm, we are copying an element whose namespace is defined
			 * out of the new tree scope. Search it in the original tree
			 * and add it at the top of the new tree
			 */
			ns = xmlSearchNs(node->doc, node, node->ns->prefix);
			if (ns != NULL) {
				xmlNodePtr root = ret;
				
				while (root->parent != NULL) root = root->parent;
				ret->ns = xmlNewNs(root, ns->href, ns->prefix);
			}
		} else {
			/*
			 * reference the existing namespace definition in our own tree.
			 */
			ret->ns = ns;
		}
	}
	if (node->properties != NULL)
		ret->properties = xmlCopyPropList(ret, node->properties);
	if (node->type == XML_ENTITY_REF_NODE) {
		if ((doc == NULL) || (node->doc != doc)) {
			/*
			 * The copied node will go into a separate document, so
			 * to avoid dangling references to the ENTITY_DECL node
			 * we cannot keep the reference. Try to find it in the
			 * target document.
			 */
			ret->children = (xmlNodePtr) xmlGetDocEntity(doc, ret->name);
		} else {
			ret->children = node->children;
		}
		ret->last = ret->children;
	} else if (node->children != NULL) {
		ret->children = xmlStaticCopyNodeList(node->children, doc, ret);
		UPDATE_LAST_CHILD_AND_PARENT(ret)
			}
	return(ret);
}

static xmlNodePtr
xmlStaticCopyNodeList (xmlNodePtr node, xmlDocPtr doc, xmlNodePtr parent)
{
	xmlNodePtr ret = NULL;
	xmlNodePtr p = NULL,q;

	while (node != NULL) {
		if (node->type == XML_DTD_NODE ) {
			if (doc == NULL) {
				node = node->next;
				continue;
			}
			if (doc->intSubset == NULL) {
				q = (xmlNodePtr) xmlCopyDtd( (xmlDtdPtr) node );
				q->doc = doc;
				q->parent = parent;
				doc->intSubset = (xmlDtdPtr) q;
				xmlAddChild(parent, q);
			} else {
				q = (xmlNodePtr) doc->intSubset;
				xmlAddChild(parent, q);
			}
		} else
			q = xmlStaticCopyNode(node, doc, parent, 1);
		if (q == NULL) {
			node = node->next;
			continue;
		}
		if (ret == NULL) {
			q->prev = NULL;
			ret = p = q;
		} else if (p != q) {
			/* the test is required if xmlStaticCopyNode coalesced 2 text nodes */
			p->next = q;
			q->prev = p;
			p = q;
		}
		node = node->next;
	}
	return(ret);
}

static xmlDocPtr
my_xmldoc_copy (xmlDocPtr src)
{
	xmlDocPtr dest;
	gboolean recursive = TRUE;

	if (src == NULL)
		return NULL;

	dest = xmlNewDoc (src->version);
	if (dest == NULL)
		return NULL;
	
	if (src->name != NULL)
		dest->name = xmlMemStrdup (src->name);
	if (src->encoding != NULL)
		dest->encoding = xmlStrdup (src->encoding);

	dest->charset     = src->charset;
	dest->compression = src->compression;
	dest->standalone  = src->standalone;

	if (!recursive)
		return dest;

	dest->last = NULL;
	dest->children = NULL;
	if (src->intSubset != NULL) {
		dest->intSubset = xmlCopyDtd (src->intSubset);
		xmlSetTreeDoc ((xmlNodePtr)dest->intSubset, dest);
		dest->intSubset->parent = dest;
	}
	if (src->oldNs != NULL)
		dest->oldNs = xmlCopyNamespaceList(src->oldNs);
	if (src->children != NULL) {
		xmlNodePtr tmp;
		
		dest->children = xmlStaticCopyNodeList (src->children, dest,
							(xmlNodePtr)dest);
		dest->last = NULL;
		tmp = dest->children;
		while (tmp != NULL) {
			if (tmp->next == NULL)
				dest->last = tmp;
			tmp = tmp->next;
		}
	}
	
	return dest;
}

static void
my_fix_doc (const gchar *in)
{
	xmlDocPtr doc;
	xmlDocPtr new;
	
	doc = xmlParseFile (in);
	if (!doc)
		my_error ("Could not parse %s\n", in);

	new = my_xmldoc_copy (doc);
	unlink (in);
	xmlSaveFormatFile (in, new, 1);
}



int
main (int argc, const char *argv [])
{
	poptContext popt;
	const gchar **args;
	gchar *in_filename;

        /* Args */
	popt = poptGetContext ("checkxref", argc, argv, options, 0);
	poptGetNextOpt (popt);
	args = poptGetArgs (popt);
	if (!args || !args[0])
		my_error ("Input file not specified");
	if (debug)
		g_print ("Running xmlfix with debug %d\n", debug);
	in_filename = g_strdup (args [0]);

	my_fix_doc (in_filename);

	g_free (in_filename);
	
	return 0;
}
