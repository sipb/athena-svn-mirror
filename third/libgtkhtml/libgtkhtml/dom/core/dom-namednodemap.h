#ifndef __DOM_NAMEDNODEMAP_H__
#define __DOM_NAMEDNODEMAP_H__

#include <libxml/tree.h>
#include <libgtkhtml/dom/dom-types.h>

#define DOM_TYPE_NAMED_NODE_MAP             (dom_named_node_map_get_type ())
#define DOM_NAMED_NODE_MAP(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_NAMED_NODE_MAP, DomNamedNodeMap))
#define DOM_NAMED_NODE_MAP_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_NAMED_NODE_MAP, DomNamedNodeMapClass))
#define DOM_IS_NAMED_NODE_MAP(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_NAMED_NODE_MAP))
#define DOM_IS_NAMED_NODE_MAP_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_NAMED_NODE_MAP))
#define DOM_NAMED_NODE_MAP_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_NAMED_NODE_MAP, DomNamedNodeMapClass))

struct _DomNamedNodeMap {
	GObject parent;

	xmlNode *attr;

	xmlElementType type;
	
	gboolean readonly;
};

struct _DomNamedNodeMapClass {
	GObjectClass parent_class;
};

GType dom_named_node_map_get_type (void);

DomNode *dom_NamedNodeMap_getNamedItem (DomNamedNodeMap *map, const DomString *name);
DomNode *dom_NamedNodeMap_setNamedItem (DomNamedNodeMap *map, DomNode *arg, DomException *exc);
DomNode *dom_NamedNodeMap_removeNamedItem (DomNamedNodeMap *map, const DomString *name, DomException *exc);

DomNode *dom_NamedNodeMap__get_item (DomNamedNodeMap *map, gulong index);
gulong dom_NamedNodeMap__get_length (DomNamedNodeMap *map);

#endif /* __DOM_NAMEDNODEMAP_H__ */
