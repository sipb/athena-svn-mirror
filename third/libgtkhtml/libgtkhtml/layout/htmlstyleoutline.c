#include <string.h>

#include "graphics/htmlcolor.h"
#include "htmlstyle.h"

HtmlStyleOutline *
html_style_outline_ref (HtmlStyleOutline *outline)
{
	outline->refcount++;
	return outline;
}

void
html_style_outline_unref (HtmlStyleOutline *outline)
{
	if (!outline)
		return;

	outline->refcount--;

	if (outline->refcount <= 0) {
		if (outline->color)
			html_color_unref (outline->color);
		g_free (outline);
	}
}

void
html_style_set_style_outline (HtmlStyle *style, HtmlStyleOutline *outline)
{
	if (style->outline == outline)
		return;

	if (style->outline)
		html_style_outline_unref (style->outline);

	if (outline) {
		style->outline = html_style_outline_ref (outline);
	}
}

HtmlStyleOutline *
html_style_outline_new (void)
{
	return g_new0 (HtmlStyleOutline, 1);
}

HtmlStyleOutline *
html_style_outline_dup (HtmlStyleOutline *outline)
{
	HtmlStyleOutline *result = html_style_outline_new ();

	if (outline)
		memcpy (result, outline, sizeof (HtmlStyleOutline));

	result->refcount = 0;

	if (outline->color)
		result->color = html_color_ref (outline->color);

	return result;
}

void
html_style_set_outline_width (HtmlStyle *style, gint width)
{
	if (style->outline->width != width) {
		if (style->outline->refcount > 1)
			html_style_set_style_outline (style, html_style_outline_dup (style->outline));
		style->outline->width = width;
	}
}

void
html_style_set_outline_style (HtmlStyle *style, HtmlBorderStyleType outline_style)
{
	if (style->outline->style != outline_style) {
		if (style->outline->refcount > 1)
			html_style_set_style_outline (style, html_style_outline_dup (style->outline));
		style->outline->style = outline_style;
	}
}

void
html_style_set_outline_color (HtmlStyle *style, HtmlColor *color)
{
	if (!html_color_equal (style->outline->color, color)) {
		if (style->outline->refcount > 1)
			html_style_set_style_outline (style, html_style_outline_dup (style->outline));
		if (style->outline->color)
			html_color_unref (style->outline->color);
		if (color)
			style->outline->color = html_color_dup (color);
		else
			style->outline->color = NULL;
	}
}
