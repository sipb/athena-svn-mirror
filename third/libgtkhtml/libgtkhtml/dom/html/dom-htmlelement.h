#ifndef __DOM_HTML_ELEMENT_H__
#define __DOM_HTML_ELEMENT_H__

#include <libgtkhtml/dom/dom-types.h>
#include <libgtkhtml/document/htmldocument.h>

#include <libgtkhtml/dom/core/dom-element.h>

#define DOM_TYPE_HTML_ELEMENT             (dom_html_element_get_type ())
#define DOM_HTML_ELEMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_HTML_ELEMENT, DomHTMLElement))
#define DOM_HTML_ELEMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_HTML_ELEMENT, DomHTMLElementClass))
#define DOM_IS_HTML_ELEMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_HTML_ELEMENT))
#define DOM_IS_HTML_ELEMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_HTML_ELEMENT))
#define DOM_HTML_ELEMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_HTML_ELEMENT, DomHTMLElementClass))

struct _DomHTMLElement {
	DomElement parent;
};

struct _DomHTMLElementClass {
	DomElementClass parent_class;
	void (*parse_html_properties) (DomHTMLElement *element, HtmlDocument *document);
};

GType dom_html_element_get_type (void);

void dom_html_element_parse_html_properties (DomHTMLElement *element, HtmlDocument *document);

DomString *dom_HTMLElement__get_id (DomHTMLElement *element);
DomString *dom_HTMLElement__get_dir (DomHTMLElement *element);
DomString *dom_HTMLElement__get_lang (DomHTMLElement *element);
DomString *dom_HTMLElement__get_title (DomHTMLElement *element);
DomString *dom_HTMLElement__get_className (DomHTMLElement *element);

void dom_HTMLElement__set_id (DomHTMLElement *element, const DomString *id);
void dom_HTMLElement__set_dir (DomHTMLElement *element, const DomString *dir);
void dom_HTMLElement__set_lang (DomHTMLElement *element, const DomString *lang);
void dom_HTMLElement__set_title (DomHTMLElement *element, const DomString *title);
void dom_HTMLElement__set_className (DomHTMLElement *element, const DomString *className);

#endif /* __DOM_HTML_ELEMENT_H__ */
