#ifndef __DOM_HTML_ANCHOR_ELEMENT_H__
#define __DOM_HTML_ANCHOR_ELEMENT_H__

#include <libgtkhtml/dom/dom-types.h>

#include <libgtkhtml/dom/html/dom-htmlelement.h>
#include <libgtkhtml/dom/html/dom-htmlcollection.h>

#define DOM_TYPE_HTML_ANCHOR_ELEMENT             (dom_html_anchor_element_get_type ())
#define DOM_HTML_ANCHOR_ELEMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_HTML_ANCHOR_ELEMENT, DomHTMLAnchorElement))
#define DOM_HTML_ANCHOR_ELEMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_HTML_ELEMENT, DomHTMLAnchorElementClass))
#define DOM_IS_HTML_ANCHOR_ELEMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_HTML_ANCHOR_ELEMENT))
#define DOM_IS_HTML_ANCHOR_ELEMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_HTML_ANCHOR_ELEMENT))
#define DOM_HTML_ANCHOR_ELEMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_HTML_ANCHOR_ELEMENT, DomHTMLAnchorElementClass))

struct _DomHTMLAnchorElement {
	DomHTMLElement parent;
};

struct _DomHTMLAnchorElementClass {
	DomHTMLElementClass parent_class;
};

GType dom_html_anchor_element_get_type (void);

#endif /* __DOM_HTML_FORM_ELEMENT_H__ */
