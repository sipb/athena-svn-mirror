/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __CSSVALUE_H__
#define __CSSVALUE_H__

#include <glib.h>

#include <libgtkhtml/util/htmlatomlist.h>

G_BEGIN_DECLS

typedef enum {
	CSS_UNKNOWN = 0,
	CSS_NUMBER = 1,
	CSS_PERCENTAGE = 2,
	CSS_EMS = 3,
	CSS_EXS = 4,
	CSS_PX = 5,
	CSS_CM = 6,
	CSS_MM = 7,
	CSS_IN = 8,
	CSS_PT = 9,
	CSS_PC = 10,
	CSS_DEG = 11,
	CSS_RAD = 12,
	CSS_GRAD = 13,
	CSS_MS = 14,
	CSS_S = 15,
	CSS_HZ = 16,
	CSS_KHZ = 17,
	CSS_DIMENSION = 18,
	CSS_STRING = 19,
	CSS_URI = 20,
	CSS_IDENT = 21,
	CSS_ATTR = 22,
	CSS_COUNTER = 23,
	CSS_RECT = 24,
	CSS_RGBCOLOR = 25,
	CSS_VALUE_LIST = 26,
	CSS_FUNCTION = 27
} CssValueType;


typedef struct _CssValue CssValue;
typedef struct _CssValueEntry CssValueEntry;
typedef struct _CssFunction CssFunction;

struct _CssValue {
	CssValueType value_type;
	gint ref_count;
	
	union {
		gdouble d;
		HtmlAtom atom;
		CssValueEntry *entry;
		gchar *s;
		CssFunction *function;
	} v;
};

struct _CssValueEntry {
	CssValue *value;
	CssValueEntry *next;
	gchar list_sep;
};

struct _CssFunction {
	HtmlAtom name;
	CssValue *args;
};

void css_value_list_append (CssValue *list, CssValue *element, gchar list_sep);
CssValue *css_value_list_new (void);
CssValue *css_value_dimension_new (gdouble d, CssValueType type);
CssValue *css_value_function_new (HtmlAtom name, CssValue *args);
CssValue *css_value_ident_new (HtmlAtom atom);
CssValue *css_value_string_new (gchar *str);
gint css_value_list_get_length (CssValue *list);
void css_value_unref (CssValue *val);
gchar *css_value_to_string (CssValue *value);

G_END_DECLS

#endif /* __CSSVALUE_H__ */
