#ifndef __DOM_HTML_OBJECT_ELEMENT_H__
#define __DOM_HTML_OBJECT_ELEMENT_H__

#include <libgtkhtml/dom/dom-types.h>

#include <libgtkhtml/dom/html/dom-htmlformelement.h>

#define DOM_TYPE_HTML_OBJECT_ELEMENT             (dom_html_object_element_get_type ())
#define DOM_HTML_OBJECT_ELEMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_HTML_OBJECT_ELEMENT, DomHTMLObjectElement))
#define DOM_HTML_OBJECT_ELEMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_HTML_ELEMENT, DomHTMLObjectElementClass))
#define DOM_IS_HTML_OBJECT_ELEMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_HTML_OBJECT_ELEMENT))
#define DOM_IS_HTML_OBJECT_ELEMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_HTML_OBJECT_ELEMENT))
#define DOM_HTML_OBJECT_ELEMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_HTML_OBJECT_ELEMENT, DomHTMLObjectElementClass))

struct _DomHTMLObjectElement {
	DomHTMLElement parent;
	/*
           attribute DOMString        align;
           attribute boolean          declare;
           attribute DOMString        hspace;
           attribute DOMString        standby;
           attribute long             tabIndex;
           attribute DOMString        type;
           attribute DOMString        useMap;
           attribute DOMString        vspace;
  // Introduced in DOM Level 2:
  readonly attribute Document         contentDocument;
*/
};

struct _DomHTMLObjectElementClass {
	DomHTMLElementClass parent_class;
};

GType dom_html_object_element_get_type (void);

DomString *dom_HTMLObjectElement__get_name     (DomHTMLObjectElement *element);
DomString *dom_HTMLObjectElement__get_code     (DomHTMLObjectElement *element);
DomString *dom_HTMLObjectElement__get_archive  (DomHTMLObjectElement *element);
DomString *dom_HTMLObjectElement__get_border   (DomHTMLObjectElement *border);
DomString *dom_HTMLObjectElement__get_codeBase (DomHTMLObjectElement *codeBase);
DomString *dom_HTMLObjectElement__get_codeType (DomHTMLObjectElement *codeType);
DomString *dom_HTMLObjectElement__get_data     (DomHTMLObjectElement *data);
DomString *dom_HTMLObjectElement__get_width    (DomHTMLObjectElement *data);
DomString *dom_HTMLObjectElement__get_height   (DomHTMLObjectElement *data);
DomString *dom_HTMLObjectElement__get_type     (DomHTMLObjectElement *data);

DomHTMLFormElement *dom_HTMLObjectElement__get_form (DomHTMLObjectElement *element);

void dom_HTMLObjectElement__set_name      (DomHTMLObjectElement *element, const DomString *name);
void dom_HTMLObjectElement__set_code      (DomHTMLObjectElement *element, const DomString *code);
void dom_HTMLObjectElement__set_archive   (DomHTMLObjectElement *element, const DomString *archive);
void dom_HTMLObjectElement__set_border    (DomHTMLObjectElement *element, const DomString *border);
void dom_HTMLObjectElement__set_codeBase  (DomHTMLObjectElement *element, const DomString *codeBase);
void dom_HTMLObjectElement__set_codeType  (DomHTMLObjectElement *element, const DomString *codeType);
void dom_HTMLObjectElement__set_data      (DomHTMLObjectElement *element, const DomString *data);
void dom_HTMLObjectElement__set_width     (DomHTMLObjectElement *element, const DomString *width);
void dom_HTMLObjectElement__set_height    (DomHTMLObjectElement *element, const DomString *height);
void dom_HTMLObjectElement__set_type      (DomHTMLObjectElement *element, const DomString *height);

#endif /* __DOM_HTML_OBJECT_ELEMENT_H__ */
