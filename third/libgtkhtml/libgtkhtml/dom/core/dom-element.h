#ifndef __DOM_ELEMENT_H__
#define __DOM_ELEMENT_H__

#include <libgtkhtml/dom/dom-types.h>

#include <libgtkhtml/dom/core/dom-node.h>

#define DOM_TYPE_ELEMENT             (dom_element_get_type ())
#define DOM_ELEMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_ELEMENT, DomElement))
#define DOM_ELEMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_ELEMENT, DomElementClass))
#define DOM_IS_ELEMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_ELEMENT))
#define DOM_IS_ELEMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_ELEMENT))
#define DOM_ELEMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_ELEMENT, DomElementClass))

struct _DomElement {
	DomNode parent;

	gint tabindex;
};

struct _DomElementClass {
	DomNodeClass parent_class;

	gboolean (* is_focusable) (DomElement *element);
};

GType dom_element_get_type (void);

DomAttr *dom_Element_getAttributeNode (DomElement *element, const DomString *name);
DomString *dom_Element__get_tagName (DomElement *element);
DomString *dom_Element_getAttribute (DomElement *element, const DomString *name);
void dom_Element_setAttribute (DomElement *element, DomString *name, const DomString *value);
DomBoolean dom_Element_hasAttribute (DomElement *element, const DomString *name);
void dom_Element_removeAttribute (DomElement *element, const DomString *name);

gboolean dom_element_is_focusable (DomElement *element);

#endif /* __DOM_ELEMENT_H__ */
