#include "dom-documenttype.h"

/**
 * dom_DocumentType__get_name:
 * @dtd: a DomDocumentType
 * 
 * The name of the DTD, the name immediately following the DOCTYPE keyword.
 * 
 * Return value: The name of the DTD. The value must be freed.
 **/
DomString *
dom_DocumentType__get_name (DomDocumentType *dtd)
{
	return g_strdup (DOM_NODE (dtd)->xmlnode->name);
}

/**
 * dom_DocumentType__get_publicId:
 * @dtd: a DomDocumentType
 * 
 * Returns the public identifier of the external subset.
 * 
 * Return value: The public identifier of the external subset. This value must be freed.
 **/
DomString *
dom_DocumentType__get_publicId (DomDocumentType *dtd)
{
	return g_strdup (((xmlDtd *)DOM_NODE (dtd)->xmlnode)->ExternalID);
}

/**
 * dom_DocumentType__get_systemId:
 * @dtd: 
 * 
 * Returns the system identifier of the external subset.
 * 
 * Return value: The system identifier of the external subset. This value must be freed.
 **/
DomString *
dom_DocumentType__get_systemId (DomDocumentType *dtd)
{
	return g_strdup (((xmlDtd *)DOM_NODE (dtd)->xmlnode)->SystemID);
}

DomNamedNodeMap *
dom_DocumentType__get_entities (DomDocumentType *dtd)
{
	DomNamedNodeMap *map;

	map = g_object_new (DOM_TYPE_NAMED_NODE_MAP, NULL);
	
	map->attr = DOM_NODE (dtd)->xmlnode->children;
	map->readonly = TRUE;
	map->type = XML_ENTITY_DECL;
	
	return map;
}

static void
dom_document_type_class_init (DomDocumentTypeClass *klass)
{
}

static void
dom_document_type_init (DomDocumentType *doc)
{
}

GType
dom_document_type_get_type (void)
{
	static GType dom_document_type_type = 0;

	if (!dom_document_type_type) {
		static const GTypeInfo dom_document_type_info = {
			sizeof (DomDocumentTypeClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dom_document_type_class_init,
			NULL, /* class_finalize */
			NULL, /* class_type */
			sizeof (DomDocumentType),
			16,   /* n_preallocs */
			(GInstanceInitFunc) dom_document_type_init,
		};

		dom_document_type_type = g_type_register_static (DOM_TYPE_NODE, "DomDocumentType", &dom_document_type_info, 0);
	}

	return dom_document_type_type;
}
