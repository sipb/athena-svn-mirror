#include <string.h>

#include "dom-nodefilter.h"

gshort
dom_NodeFilter_acceptNode (DomNodeFilter *filter, const DomNode *n)
{
	return DOM_NODE_FILTER_GET_IFACE (filter)->acceptNode (filter, n);
}

GType
dom_node_filter_get_type (void)
{
	static GType node_filter_type = 0;

	if (!node_filter_type) {
		static const GTypeInfo node_filter_info =
		{
			sizeof (DomNodeFilterIface), /* class_size */
			NULL, /* base_init */
			NULL,		/* base_finalize */
			NULL,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			0,
			0,              /* n_preallocs */
			NULL
		};
		
		node_filter_type = g_type_register_static (G_TYPE_INTERFACE, "DomNodeFilter", &node_filter_info, 0);
		g_type_interface_add_prerequisite (node_filter_type, G_TYPE_OBJECT);
	}
	
	return node_filter_type;
}
