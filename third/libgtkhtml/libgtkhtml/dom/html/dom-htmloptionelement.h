#ifndef __DOM_HTML_OPTION_ELEMENT_H__
#define __DOM_HTML_OPTION_ELEMENT_H__

#include <libgtkhtml/dom/dom-types.h>

#include <libgtkhtml/dom/html/dom-htmlformelement.h>

#define DOM_TYPE_HTML_OPTION_ELEMENT             (dom_html_option_element_get_type ())
#define DOM_HTML_OPTION_ELEMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_HTML_OPTION_ELEMENT, DomHTMLOptionElement))
#define DOM_HTML_OPTION_ELEMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_HTML_ELEMENT, DomHTMLOptionElementClass))
#define DOM_IS_HTML_OPTION_ELEMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_HTML_OPTION_ELEMENT))
#define DOM_IS_HTML_OPTION_ELEMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_HTML_OPTION_ELEMENT))
#define DOM_HTML_OPTION_ELEMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_HTML_OPTION_ELEMENT, DomHTMLOptionElementClass))

struct _DomHTMLOptionElement {
	DomHTMLElement parent;
};

struct _DomHTMLOptionElementClass {
	DomHTMLElementClass parent_class;
};

GType dom_html_option_element_get_type (void);

DomHTMLFormElement *dom_HTMLOptionElement__get_form (DomHTMLOptionElement *element);

DomBoolean dom_HTMLOptionElement__get_defaultSelected  (DomHTMLOptionElement *element);
DomBoolean dom_HTMLOptionElement__get_disabled  (DomHTMLOptionElement *element);
DomBoolean dom_HTMLOptionElement__get_selected  (DomHTMLOptionElement *element);
DomString *dom_HTMLOptionElement__get_text  (DomHTMLOptionElement *element);
DomString *dom_HTMLOptionElement__get_value  (DomHTMLOptionElement *element);
DomString *dom_HTMLOptionElement__get_label  (DomHTMLOptionElement *element);
glong dom_HTMLOptionElement__get_index  (DomHTMLOptionElement *element);

void dom_HTMLOptionElement__set_defaultSelected  (DomHTMLOptionElement *element, DomBoolean defaultSelected);
void dom_HTMLOptionElement__set_disabled  (DomHTMLOptionElement *element, DomBoolean defaultSelected);
void dom_HTMLOptionElement__set_label      (DomHTMLOptionElement *element, const DomString *label);
void dom_HTMLOptionElement__set_value      (DomHTMLOptionElement *element, const DomString *value);
void dom_HTMLOptionElement__set_selected   (DomHTMLOptionElement *element, DomBoolean selected);

void dom_html_option_element_new_character_data (DomHTMLOptionElement *option);

#endif /* __DOM_HTML_OPTION_ELEMENT_H__ */
