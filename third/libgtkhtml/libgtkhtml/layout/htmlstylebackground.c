#include <string.h>

#include "htmlstyle.h"

HtmlStyleBackground *
html_style_background_ref (HtmlStyleBackground *background)
{
	background->refcount++;
	return background;
}

void
html_style_background_unref (HtmlStyleBackground *background)
{
	if (!background)
		return;

	background->refcount--;

	if (background->refcount <= 0) {
		if (background->image)
			g_object_unref (G_OBJECT (background->image));
		g_free (background);
	}
}

void
html_style_set_style_background (HtmlStyle *style, HtmlStyleBackground *background)
{
	if (style->background == background)
		return;

	if (style->background)
		html_style_background_unref (style->background);

	if (background) {
		style->background = background;
		html_style_background_ref (style->background);
	}
}

HtmlStyleBackground *
html_style_background_new (void)
{
	HtmlStyleBackground *result = g_new0 (HtmlStyleBackground, 1);
	result->color.transparent = TRUE;
	return result;
}

HtmlStyleBackground *
html_style_background_dup (HtmlStyleBackground *background)
{
	HtmlStyleBackground *result = html_style_background_new ();

	if (background) {
 		memcpy (result, background, sizeof (HtmlStyleBackground));

		result->refcount = 0;

		if (background->image)
		    result->image = g_object_ref (background->image);
	}
	return result;
}

void
html_style_set_background_color (HtmlStyle *style, HtmlColor *color)
{
	if (!html_color_equal (&style->background->color, color)) {
		if (style->background->refcount > 1)
			html_style_set_style_background (style, html_style_background_dup (style->background));
		style->background->color.transparent = color->transparent;
		style->background->color.red = color->red;
		style->background->color.green = color->green;
		style->background->color.blue = color->blue;
	}
}

void
html_style_set_background_image (HtmlStyle *style, HtmlImage *image)
{
	if (style->background->image != image) {

		if (style->background->refcount > 1)
			html_style_set_style_background (style, html_style_background_dup (style->background));
		style->background->image = g_object_ref (G_OBJECT (image));
	}
}

void
html_style_set_background_repeat (HtmlStyle *style, HtmlBackgroundRepeatType repeat)
{
	if (style->background->repeat != repeat) {

		if (style->background->refcount > 1)
			html_style_set_style_background (style, html_style_background_dup (style->background));
		style->background->repeat = repeat;
	}
}
