#ifndef __DOM_DOM_IMPLEMENTATION_H__
#define __DOM_DOM_IMPLEMENTATION_H__

#include <dom/core/dom-node.h>

#define DOM_TYPE_DOM_IMPLEMENTATION             (dom_dom_implementation_get_type ())
#define DOM_DOM_IMPLEMENTATION(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_DOM_IMPLEMENTATION, DomDOMImplementation))
#define DOM_DOM_IMPLEMENTATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_DOM_IMPLEMENTATION, DomDOMImplementationClass))
#define DOM_IS_DOM_IMPLEMENTATION(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_DOM_IMPLEMENTATION))
#define DOM_IS_DOM_IMPLEMENTATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_DOM_IMPLEMENTATION))
#define DOM_DOM_IMPLEMENTATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_DOM_IMPLEMENTATION, DomDOMImplementationClass))

typedef struct _DomDOMImplementation DomDOMImplementation;
typedef struct _DomDOMImplementationClass DomDOMImplementationClass;

struct _DomDOMImplementation {
	GObject parent;
};

struct _DomDOMImplementationClass {
	GObjectClass parent_class;
};

GType dom_dom_implementation_get_type (void);
DomBoolean dom_DOMImplementation_hasFeature (DomString *feature, DomString *version);
DomDOMImplementation *dom_DOMImplementation_new (void);

#endif /* __DOM_DOM_IMPLEMENTATION_H__ */
