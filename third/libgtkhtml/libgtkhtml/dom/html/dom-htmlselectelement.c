#include <stdlib.h>
#include "dom-htmlselectelement.h"
#include "dom-htmloptionelement.h"

GObjectClass *parent_class = NULL;

DomHTMLFormElement *
dom_HTMLSelectElement__get_form (DomHTMLSelectElement *select)
{
	DomNode *form = dom_Node__get_parentNode (DOM_NODE (select));
	
	while (form && !DOM_IS_HTML_FORM_ELEMENT (form))
		form = dom_Node__get_parentNode (form);
	
	return (DomHTMLFormElement *)form;
}

DomString *
dom_HTMLSelectElement__get_type (DomHTMLSelectElement *select)
{
	if (dom_HTMLSelectElement__get_multiple (select))
		return g_strdup ("select-multiple");
	else
		return g_strdup ("select-one");
}

DomString *
dom_HTMLSelectElement__get_name (DomHTMLSelectElement *select)
{
	return dom_Element_getAttribute (DOM_ELEMENT (select), "name");
}

glong
dom_HTMLSelectElement__get_size (DomHTMLSelectElement *select)
{
	gchar *str;
	glong value = 1;

	if ((str = dom_Element_getAttribute (DOM_ELEMENT (select), "size"))) {
		g_strchug (str);
		value = atoi (str);
		xmlFree (str);
	}
	return value;
}

DomBoolean
dom_HTMLSelectElement__get_multiple (DomHTMLSelectElement *select)
{
	return dom_Element_hasAttribute (DOM_ELEMENT (select), "multiple");
}

void
dom_HTMLSelectElement__set_name (DomHTMLSelectElement *select, const DomString *name)
{
	dom_Element_setAttribute (DOM_ELEMENT (select), "name", name);
}

void
dom_HTMLSelectElement__set_size (DomHTMLSelectElement *select, glong size)
{
	gchar *str = g_strdup_printf ("%ld", size);
	dom_Element_setAttribute (DOM_ELEMENT (select), "size", str);
	g_free (str);
}

void
dom_HTMLSelectElement_add (DomHTMLSelectElement *select, DomHTMLElement *element, DomHTMLElement *before, DomException *exception)
{
	GtkTreeIter iter;
	gint position = -1;

	*exception = DOM_NO_EXCEPTION;

	if (before) {

		position = g_slist_index (select->options, before);

		if (position == -1) {

			*exception = DOM_NOT_FOUND_ERR;
			return;
		}
		g_slist_insert (select->options, element, position);
	}
	else {
		select->options = g_slist_append (select->options, element);
	}
	if (position != -1)
		gtk_list_store_insert (select->list_store, &iter, position);
	else
		gtk_list_store_append (select->list_store, &iter);

	gtk_list_store_set (select->list_store, &iter, 0, "", 1, "", 2, element, -1);
}

void
dom_HTMLSelectElement_remove (DomHTMLSelectElement *select, glong index)
{
	DomHTMLElement *element = (DomHTMLElement *)g_slist_nth (select->options, index);

	if (element)
		select->options = g_slist_remove (select->options, element);
}

GtkTreeModel *
dom_html_select_element_get_tree_model (DomHTMLSelectElement *select)
{
	return GTK_TREE_MODEL (select->list_store);
}

void
dom_html_select_element_update_option_data (DomHTMLSelectElement *select, DomHTMLOptionElement *element)
{
	gint position = g_slist_index (select->options, element);

	if (position >= 0) {
		gchar *label = NULL, *value = NULL;
		DomNode *labelnode;

		if ((labelnode = dom_Node__get_firstChild (DOM_NODE (element)))) {

			DomException exc;

			if ((label = dom_Node__get_nodeValue (DOM_NODE (labelnode), &exc))) {

				gboolean use_gfree = FALSE;
				GtkTreeIter  iter;

				value = dom_HTMLOptionElement__get_value (DOM_HTML_OPTION_ELEMENT (element));

				if (value == NULL) {
					use_gfree = TRUE;
					value = g_strdup (label);
				}
				 gtk_tree_model_get_iter_root (GTK_TREE_MODEL (select->list_store), &iter);

				while (position--) 
					gtk_tree_model_iter_next (GTK_TREE_MODEL (select->list_store), &iter);

				gtk_list_store_set (select->list_store, &iter, 0, label, 1, value, 2, element, -1);

				g_free (label);

				if (use_gfree)
					g_free (value);
				else
					xmlFree (value);
			}
		}
	}
}

static void
finalize (GObject *object)
{
	DomHTMLSelectElement *select = (DomHTMLSelectElement *)object;

	g_object_unref (G_OBJECT (select->list_store));

	parent_class->finalize (object);
}


static void
dom_html_select_element_class_init (GObjectClass *klass)
{
	klass->finalize = finalize;

	parent_class = g_type_class_peek_parent (klass);
}

static void
dom_html_select_element_init (DomHTMLSelectElement *select)
{
	select->list_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_OBJECT);
}

GType
dom_html_select_element_get_type (void)
{
	static GType dom_html_select_element_type = 0;

	if (!dom_html_select_element_type) {
		static const GTypeInfo dom_html_select_element_info = {
			sizeof (DomHTMLSelectElementClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dom_html_select_element_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (DomHTMLSelectElement),
			16,   /* n_preallocs */
			(GInstanceInitFunc) dom_html_select_element_init,
		};

		dom_html_select_element_type = g_type_register_static (DOM_TYPE_HTML_ELEMENT, 
								     "DomHTMLSelectElement", &dom_html_select_element_info, 0);
	}
	return dom_html_select_element_type;
}
