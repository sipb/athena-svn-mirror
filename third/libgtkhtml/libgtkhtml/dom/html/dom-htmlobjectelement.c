#include <string.h>
#include <stdlib.h>

#include "dom-htmlobjectelement.h"

DomHTMLFormElement *
dom_HTMLObjectElement__get_form (DomHTMLObjectElement *object)
{
	DomNode *form = dom_Node__get_parentNode (DOM_NODE (object));
	
	while (form && !DOM_IS_HTML_FORM_ELEMENT (form))
		form = dom_Node__get_parentNode (form);
	
	return (DomHTMLFormElement *)form;
}

DomString *
dom_HTMLObjectElement__get_name (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "name");
}

DomString *
dom_HTMLObjectElement__get_code (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "code");
}

DomString *
dom_HTMLObjectElement__get_archive (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "archive");
}

DomString *
dom_HTMLObjectElement__get_border (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "border");
}

DomString *
dom_HTMLObjectElement__get_codeBase (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "codebase");
}

DomString *
dom_HTMLObjectElement__get_codeType (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "codetype");
}

DomString *
dom_HTMLObjectElement__get_data (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "data");
}

DomString *
dom_HTMLObjectElement__get_width (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "width");
}

DomString *
dom_HTMLObjectElement__get_height (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "height");
}

DomString *
dom_HTMLObjectElement__get_type (DomHTMLObjectElement *object)
{
	return dom_Element_getAttribute (DOM_ELEMENT (object), "type");
}

void
dom_HTMLObjectElement__set_name (DomHTMLObjectElement *object, const DomString *name)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "name", name);
}

void
dom_HTMLObjectElement__set_code (DomHTMLObjectElement *object, const DomString *code)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "code", code);
}

void
dom_HTMLObjectElement__set_archive (DomHTMLObjectElement *object, const DomString *archive)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "archive", archive);
}

void
dom_HTMLObjectElement__set_border (DomHTMLObjectElement *object, const DomString *border)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "border", border);
}

void
dom_HTMLObjectElement__set_codeBase (DomHTMLObjectElement *object, const DomString *codeBase)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "codebase", codeBase);
}

void
dom_HTMLObjectElement__set_codeType (DomHTMLObjectElement *object, const DomString *codeType)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "codetype", codeType);
}

void
dom_HTMLObjectElement__set_data (DomHTMLObjectElement *object, const DomString *data)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "data", data);
}

void
dom_HTMLObjectElement__set_width (DomHTMLObjectElement *object, const DomString *width)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "width", width);
}

void
dom_HTMLObjectElement__set_height (DomHTMLObjectElement *object, const DomString *height)
{
	dom_Element_setAttribute (DOM_ELEMENT (object), "height", height);
}

static gboolean
is_focusable (DomElement *element)
{
	return TRUE;
}

static void
dom_html_object_element_class_init (GObjectClass *klass)
{
	DomElementClass *element_class = (DomElementClass *)klass;

	element_class->is_focusable = is_focusable;
}

static void
dom_html_object_element_init (DomHTMLObjectElement *object)
{
}

GType
dom_html_object_element_get_type (void)
{
	static GType dom_html_object_element_type = 0;

	if (!dom_html_object_element_type) {
		static const GTypeInfo dom_html_object_element_info = {
			sizeof (DomHTMLObjectElementClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dom_html_object_element_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (DomHTMLObjectElement),
			16,   /* n_preallocs */
			(GInstanceInitFunc) dom_html_object_element_init,
		};

		dom_html_object_element_type = g_type_register_static (DOM_TYPE_HTML_ELEMENT, 
								     "DomHTMLObjectElement", &dom_html_object_element_info, 0);
	}
	return dom_html_object_element_type;
}
