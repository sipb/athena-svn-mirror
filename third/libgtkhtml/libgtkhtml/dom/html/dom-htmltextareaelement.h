#ifndef __DOM_HTML_TEXT_AREA_ELEMENT_H__
#define __DOM_HTML_TEXT_AREA_ELEMENT_H__

#include <libgtkhtml/dom/dom-types.h>

#include <libgtkhtml/dom/html/dom-htmlformelement.h>

#define DOM_TYPE_HTML_TEXT_AREA_ELEMENT             (dom_html_text_area_element_get_type ())
#define DOM_HTML_TEXT_AREA_ELEMENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_HTML_TEXT_AREA_ELEMENT, DomHTMLTextAreaElement))
#define DOM_HTML_TEXT_AREA_ELEMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_HTML_ELEMENT, DomHTMLTextAreaElementClass))
#define DOM_IS_HTML_TEXT_AREA_ELEMENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_HTML_TEXT_AREA_ELEMENT))
#define DOM_IS_HTML_TEXT_AREA_ELEMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_HTML_TEXT_AREA_ELEMENT))
#define DOM_HTML_TEXT_AREA_ELEMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_HTML_TEXT_AREA_ELEMENT, DomHTMLTextAreaElementClass))

struct _DomHTMLTextAreaElement {
	DomHTMLElement parent;
	gchar *defaultValue;
	GtkTextBuffer *buffer;
	/*
           attribute DOMString        accessKey;
  void               blur();
  void               focus();
  void               select();


	 */
};

struct _DomHTMLTextAreaElementClass {
	DomHTMLElementClass parent_class;
};

GType dom_html_text_area_element_get_type (void);

DomString *dom_HTMLTextAreaElement__get_name  (DomHTMLTextAreaElement *element);
DomString *dom_HTMLTextAreaElement__get_type  (DomHTMLTextAreaElement *element);
DomString *dom_HTMLTextAreaElement__get_value (DomHTMLTextAreaElement *element);
DomString *dom_HTMLTextAreaElement__get_defaultValue (DomHTMLTextAreaElement *element);
glong      dom_HTMLTextAreaElement__get_rows  (DomHTMLTextAreaElement *element);
glong      dom_HTMLTextAreaElement__get_cols  (DomHTMLTextAreaElement *element);
DomBoolean dom_HTMLTextAreaElement__get_readOnly (DomHTMLTextAreaElement *element);
DomBoolean dom_HTMLTextAreaElement__get_disabled (DomHTMLTextAreaElement *element);
DomHTMLFormElement *dom_HTMLTextAreaElement__get_form (DomHTMLTextAreaElement *element);

void dom_HTMLTextAreaElement__set_name      (DomHTMLTextAreaElement *element, const DomString *name);
void dom_HTMLTextAreaElement__set_value     (DomHTMLTextAreaElement *element, const DomString *value);
void dom_HTMLTextAreaElement__set_defaultValue     (DomHTMLTextAreaElement *element, const DomString *value);
void dom_HTMLTextAreaElement__set_readOnly  (DomHTMLTextAreaElement *element, DomBoolean readonly);
void dom_HTMLTextAreaElement__set_disabled  (DomHTMLTextAreaElement *element, DomBoolean disabled);
void dom_HTMLTextAreaElement__set_rows      (DomHTMLTextAreaElement *element, glong rows);
void dom_HTMLTextAreaElement__set_cols      (DomHTMLTextAreaElement *element, glong cols);

GtkTextBuffer *dom_html_text_area_element_get_text_buffer (DomHTMLTextAreaElement *textarea);

DomString *dom_html_text_area_element_encode (DomHTMLTextAreaElement *textarea);
void       dom_html_text_area_element_reset  (DomHTMLTextAreaElement *textarea);

#endif /* __DOM_HTML_TEXT_AREA_ELEMENT_H__ */
