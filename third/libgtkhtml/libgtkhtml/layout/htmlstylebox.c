#include <string.h>

#include "htmlstyle.h"

void
html_style_box_ref (HtmlStyleBox *box)
{
	box->refcount++;
}

void
html_style_box_unref (HtmlStyleBox *box)
{
	if (!box)
		return;
	
	box->refcount--;

	if (box->refcount <= 0) {
		g_free (box);
	}
}

void
html_style_set_style_box (HtmlStyle *style, HtmlStyleBox *box)
{
	if (style->box == box)
		return;

	if (style->box)
		html_style_box_unref (style->box);

	if (box) {
		style->box = box;
		html_style_box_ref (style->box);
	}
}

HtmlStyleBox *
html_style_box_new (void)
{
	HtmlStyleBox *result = g_new0 (HtmlStyleBox, 1);
#if 0
	result->refcount = 1;
#endif	
	return result;
}


HtmlStyleBox *
html_style_box_dup (HtmlStyleBox *box)
{
	HtmlStyleBox *result = html_style_box_new ();

	if (box)
		memcpy (result, box, sizeof (HtmlStyleBox));

	result->refcount = 0;

	return result;
}

void
html_style_set_width (HtmlStyle *style, const HtmlLength *width)
{
	if (!(style->box && html_length_equals (&style->box->width, width))) {
		if (style->box->refcount > 1)
			html_style_set_style_box (style, html_style_box_dup (style->box));
		html_length_set (&style->box->width, width);
	}
}

void
html_style_set_height (HtmlStyle *style, const HtmlLength *height)
{
	if (!(style->box && html_length_equals (&style->box->height, height))) {
		if (style->box->refcount > 1)
			html_style_set_style_box (style, html_style_box_dup (style->box));
		html_length_set (&style->box->height, height);
	}
}

void
html_style_set_min_width (HtmlStyle *style, const HtmlLength *min_width)
{
	if (!(style->box && html_length_equals (&style->box->min_width, min_width))) {
		if (style->box->refcount > 1)
			html_style_set_style_box (style, html_style_box_dup (style->box));
		html_length_set (&style->box->min_width, min_width);
	}
}

void
html_style_set_max_width (HtmlStyle *style, const HtmlLength *max_width)
{
	if (!(style->box && html_length_equals (&style->box->max_width, max_width))) {
		if (style->box->refcount > 1)
			html_style_set_style_box (style, html_style_box_dup (style->box));
		html_length_set (&style->box->max_width, max_width);
	}
}


void
html_style_set_min_height (HtmlStyle *style, const HtmlLength *min_height)
{
	if (!(style->box && html_length_equals (&style->box->min_height, min_height))) {
		if (style->box->refcount > 1)
			html_style_set_style_box (style, html_style_box_dup (style->box));
		html_length_set (&style->box->min_height, min_height);
	}
}


void
html_style_set_max_height (HtmlStyle *style, const HtmlLength *max_height)
{
	if (!(style->box && html_length_equals (&style->box->max_height, max_height))) {
		if (style->box->refcount > 1)
			html_style_set_style_box (style, html_style_box_dup (style->box));
		html_length_set (&style->box->max_height, max_height);
	}
}

