#include "cssvalue.h"

gchar *
css_value_to_string (CssValue *value)
{
	switch (value->value_type) {
	case CSS_IDENT:
		return g_strdup (html_atom_list_get_string (html_atom_list, value->v.atom));
		break;
	case CSS_STRING:
		return g_strdup (value->v.s);
		break;
	case CSS_NUMBER:
		return g_strdup_printf ("%f", value->v.d);
		break;
	default:
		return NULL;
	}
	return NULL;
}

CssValue *
css_value_dimension_new (gdouble d, CssValueType type)
{
	CssValue *result;

	result = g_new (CssValue, 1);
	result->ref_count = 1;
	result->value_type = type;
	result->v.d = d;

	return result;
}

CssValue *
css_value_function_new (HtmlAtom name, CssValue *args)
{
	CssValue *result;
	CssFunction *function;

	function = g_new (CssFunction, 1);
	function->name = name;
	function->args = args;
	
	result = g_new (CssValue, 1);
	result->ref_count = 1;
	result->value_type = CSS_FUNCTION;
	result->v.function = function;

	return result;
}

CssValue *
css_value_string_new (gchar *str)
{
	CssValue *result;

	result = g_new (CssValue, 1);
	result->ref_count = 1;
	result->value_type = CSS_STRING;
	result->v.s = g_strdup (str);

	return result;
}

CssValue *
css_value_ident_new (HtmlAtom atom)
{
	CssValue *result;

	result = g_new (CssValue, 1);
	result->ref_count = 1;
	result->value_type = CSS_IDENT;
	result->v.atom = atom;

	return result;
}

CssValue *
css_value_list_new (void)
{
	CssValue *result;

	result = g_new (CssValue, 1);
	result->ref_count = 1;
	result->value_type = CSS_VALUE_LIST;
	result->v.entry = NULL;

	return result;
}


gint
css_value_list_get_length (CssValue *list)
{
	CssValueEntry *entry;
	gint i = 0;
	
	if (list->value_type != CSS_VALUE_LIST)
		return -1;

	entry = list->v.entry;

	while (entry) {
		i++;
		entry = entry->next;
	}

	return i;
}

void
css_value_list_append (CssValue *list, CssValue *element, gchar list_sep)
{
	CssValueEntry *entry, *tmp_entry;
	
	if (list->value_type != CSS_VALUE_LIST)
		return;

	entry = g_new (CssValueEntry, 1);
	entry->value = element;
	entry->list_sep = list_sep;
	entry->next = NULL;
	
	if (list->v.entry == NULL)
		list->v.entry = entry;
	else {
		tmp_entry = list->v.entry;

		while (tmp_entry->next) {
			tmp_entry = tmp_entry->next;
		}

		tmp_entry->next = entry;
	}
}

void
css_value_unref (CssValue *val)
{
	CssValueEntry *entry;
	
	g_return_if_fail (val != NULL);

	val->ref_count--;
	
	if (val->ref_count == 0) {
		switch (val->value_type) {
		case CSS_STRING:
			g_free (val->v.s);
			break;
		case CSS_PX:
		case CSS_EMS:
		case CSS_IDENT:
		case CSS_NUMBER:
		case CSS_PERCENTAGE:
		case CSS_DEG:
		case CSS_IN:
		case CSS_PT:
		case CSS_EXS:
		case CSS_CM:
		case CSS_MM:
		case CSS_PC:
			break;
		case CSS_VALUE_LIST:
			entry = val->v.entry;

			while (entry) {
				CssValueEntry *tmp_entry = entry->next;

				css_value_unref (entry->value);
				g_free (entry);
				
				entry = tmp_entry;
			}
			break;
		case CSS_FUNCTION:
			css_value_unref (val->v.function->args);
			g_free (val->v.function);
			break;
		default:
			g_warning ("css_value_unref: Unhandled case: %d\n", val->value_type);
		}

		g_free (val);
	}
}
