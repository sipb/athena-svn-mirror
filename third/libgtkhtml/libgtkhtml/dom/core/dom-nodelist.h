#ifndef __DOM_NODELIST_H__
#define __DOM_NODELIST_H__

typedef struct _DomNodeList DomNodeList;
typedef struct _DomNodeListClass DomNodeListClass;

#include "dom-node.h"

#define DOM_TYPE_NODE_LIST             (dom_node_list_get_type ())
#define DOM_NODE_LIST(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_NODE_LIST, DomNodeList))
#define DOM_NODE_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_NODE_LIST, DomNodeListClass))
#define DOM_IS_NODE_LIST(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_NODE_LIST))
#define DOM_IS_NODE_LIST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_NODE_LIST))
#define DOM_NODE_LIST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_NODE_LIST, DomNodeListClass))

struct _DomNodeList {
	GObject parent;

	gulong (* length) (DomNodeList *list);
	DomNode *(* item) (DomNodeList *list, gulong index);

	DomNode *node;
	gchar *str;
};

struct _DomNodeListClass {
	GObjectClass parent_class;
};

GType dom_node_list_get_type (void);

gulong dom_NodeList__get_length (DomNodeList *list);
DomNode *dom_NodeList__get_item (DomNodeList *list, gulong index);

#endif /* __DOM_NODELIST_H__ */
