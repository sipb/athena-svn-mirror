#ifndef __DOM_NODEFILTER_H__
#define __DOM_NODEFILTER_H__

typedef struct _DomNodeFilter DomNodeFilter;
typedef struct _DomNodeFilterIface DomNodeFilterIface;

#include <glib-object.h>

#include <libgtkhtml/dom/dom-types.h>

#define DOM_TYPE_NODE_FILTER             (dom_node_filter_get_type ())
#define DOM_NODE_FILTER(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_NODE_FILTER, DomNodeFilter))
#define DOM_NODE_FILTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_NODE_FILTER, DomNodeFilterClass))
#define DOM_IS_NODE_FILTER(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_NODE_FILTER))
#define DOM_IS_NODE_FILTER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_NODE_FILTER))
#define DOM_NODE_FILTER_GET_IFACE(obj)  ((DomNodeFilterIface *)g_type_interface_peek (((GTypeInstance *)DOM_NODE_FILTER (obj))->g_class, DOM_TYPE_NODE_FILTER))

struct _DomNodeFilterIface {
	GTypeInterface g_iface;

	gshort (* acceptNode) (DomNodeFilter *filter, const DomNode *n);
};

#define DOM_NODE_FILTER_ACCEPT 1
#define DOM_NODE_FILTER_REJECT 2
#define DOM_NODE_FILTER_SKIP 3

GType dom_node_filter_get_type (void);

gshort dom_NodeFilter_acceptNode (DomNodeFilter *filter, const DomNode *n);

#endif /* __DOM_NODEFILTER_H__ */
