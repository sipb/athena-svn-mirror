#include <string.h>
#include "libgtkhtml/dom/core/dom-element.h"
#include "dom-traversal-utils.h"

typedef struct _DomNodeFilterFocusClass DomNodeFilterFocusClass;

struct _DomNodeFilterFocusClass {
	GObjectClass parent_class;
};


static gshort
dom_node_filter_focus_acceptNode (DomNodeFilter *filter, const DomNode *node)
{
	if (strcasecmp (node->xmlnode->name, "a") != 0) {
		return DOM_NODE_FILTER_SKIP;
	}

	if (dom_Element_hasAttribute (DOM_ELEMENT (node), "href") != TRUE) {
		return DOM_NODE_FILTER_SKIP;
	}

	return DOM_NODE_FILTER_ACCEPT;
}

static void
dom_node_filter_focus_node_filter_init (DomNodeFilterIface *iface)
{
	iface->acceptNode = dom_node_filter_focus_acceptNode;
}


static GType
dom_node_filter_focus_get_type (void)
{
	static GType dom_type = 0;

	if (!dom_type) {
		static const GTypeInfo dom_info = {
			sizeof (GObjectClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			NULL, /* class_init */
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (GObject),
			16,   /* n_preallocs */
			NULL,
		};

		static const GInterfaceInfo dom_node_filter_info = {
			(GInterfaceInitFunc) dom_node_filter_focus_node_filter_init,
			NULL,
			NULL
		};

		dom_type = g_type_register_static (G_TYPE_OBJECT, "DomNodeFilterFocus", &dom_info, 0);
		g_type_add_interface_static (dom_type,
					     DOM_TYPE_NODE_FILTER,
					     &dom_node_filter_info);

	}

	
	return dom_type;

}

DomNodeFilter *
dom_node_filter_focus_new (void)
{
	return DOM_NODE_FILTER (g_object_new (dom_node_filter_focus_get_type (), NULL));
}



