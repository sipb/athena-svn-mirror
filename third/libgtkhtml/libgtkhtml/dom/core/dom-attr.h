#ifndef __DOM_ATTR_H__
#define __DOM_ATTR_H__

#include "dom-node.h"

#define DOM_TYPE_ATTR             (dom_attr_get_type ())
#define DOM_ATTR(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_ATTR, DomAttr))
#define DOM_ATTR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_ATTR, DomAttrClass))
#define DOM_IS_ATTR(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_ATTR))
#define DOM_IS_ATTR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_ATTR))
#define DOM_ATTR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_ATTR, DomAttrClass))

struct _DomAttr {
	DomNode parent;
};

struct _DomAttrClass {
	DomNodeClass parent_class;
};

GType dom_attr_get_type (void);

DomString *dom_Attr__get_name (DomAttr *attr);
DomString *dom_Attr__get_value (DomAttr *attr);
void dom_Attr__set_value (DomAttr *attr, const DomString *value, DomException *exc);
DomBoolean dom_Attr_get_specified (DomAttr *attr);
DomElement *dom_Attr__get_ownerElement (DomAttr *attr);

#endif /* __DOM_ATTR_H__ */
