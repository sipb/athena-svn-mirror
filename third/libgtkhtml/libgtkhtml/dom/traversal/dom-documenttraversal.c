#include <string.h>

#include "dom-documenttraversal.h"

DomNodeIterator *
dom_DocumentTraversal_createNodeIterator (DomDocumentTraversal *traversal, DomNode *root, gulong whatToShow, DomNodeFilter *filter, gboolean entityReferenceExpansion, DomException *exc)
{
	return DOM_DOCUMENT_TRAVERSAL_GET_IFACE (traversal)->createNodeIterator (traversal, root, whatToShow, filter, entityReferenceExpansion, exc);
}


GType
dom_document_traversal_get_type (void)
{
	static GType document_traversal_type = 0;

	if (!document_traversal_type) {
		static const GTypeInfo document_traversal_info =
		{
			sizeof (DomDocumentTraversalIface), /* class_size */
			NULL, /* base_init */
			NULL,		/* base_finalize */
			NULL,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			0,
			0,              /* n_preallocs */
			NULL
		};
		
		document_traversal_type = g_type_register_static (G_TYPE_INTERFACE, "DomDocumentTraversal", &document_traversal_info, 0);
		g_type_interface_add_prerequisite (document_traversal_type, G_TYPE_OBJECT);
	}
	
	return document_traversal_type;
}
