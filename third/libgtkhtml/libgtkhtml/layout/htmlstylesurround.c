#include <string.h>

#include "htmlstyle.h"

void
html_style_surround_ref (HtmlStyleSurround *surround)
{
	surround->refcount++;
}

void
html_style_surround_unref (HtmlStyleSurround *surround)
{
	if (!surround)
		return;

	surround->refcount--;

	if (surround->refcount <= 0)
		g_free (surround);
}

void
html_style_set_style_surround (HtmlStyle *style, HtmlStyleSurround *surround)
{
	if (style->surround == surround)
		return;

	if (style->surround)
		html_style_surround_unref (style->surround);

	if (surround) {
		style->surround = surround;
		html_style_surround_ref (style->surround);
	}
}

HtmlStyleSurround *
html_style_surround_new (void)
{
	HtmlStyleSurround *result = g_new0 (HtmlStyleSurround, 1);
	return result;
}

HtmlStyleSurround *
html_style_surround_dup (HtmlStyleSurround *surround)
{
	HtmlStyleSurround *result = html_style_surround_new ();

	if (surround)
		memcpy (result, surround, sizeof (HtmlStyleSurround));

	result->refcount = 0;

	return result;
}

void
html_style_set_position_top (HtmlStyle *style, const HtmlLength *length)
{
	if (!(style->surround && html_length_equals (&style->surround->position.top, length))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->position.top, length);
	}
}

void
html_style_set_position_right (HtmlStyle *style, const HtmlLength *length)
{
	if (!(style->surround && html_length_equals (&style->surround->position.right, length))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->position.right, length);
	}
}

void
html_style_set_position_bottom (HtmlStyle *style, const HtmlLength *length)
{
	if (!(style->surround && html_length_equals (&style->surround->position.bottom, length))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->position.bottom, length);
	}
}

void
html_style_set_position_left (HtmlStyle *style, const HtmlLength *length)
{
	if (!(style->surround && html_length_equals (&style->surround->position.left, length))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->position.left, length);
	}
}

void
html_style_set_margin_top (HtmlStyle *style, const HtmlLength *margin)
{
	if (!(style->surround && html_length_equals (&style->surround->margin.top, margin))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->margin.top, margin);
	}
}

void
html_style_set_margin_bottom (HtmlStyle *style, const HtmlLength *margin)
{
	if (!(style->surround && html_length_equals (&style->surround->margin.bottom, margin))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->margin.bottom, margin);
	}
}

void
html_style_set_margin_left (HtmlStyle *style, const HtmlLength *margin)
{
	if (!(style->surround && html_length_equals (&style->surround->margin.left, margin))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->margin.left, margin);
	}
}

void
html_style_set_margin_right (HtmlStyle *style, const HtmlLength *margin)
{
	if (!(style->surround && html_length_equals (&style->surround->margin.right, margin))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->margin.right, margin);
	}
}

void
html_style_set_padding_left (HtmlStyle *style, const HtmlLength *padding)
{
	if (!(style->surround && html_length_equals (&style->surround->padding.left, padding))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->padding.left, padding);
	}
}

void
html_style_set_padding_right (HtmlStyle *style, const HtmlLength *padding)
{
	if (!(style->surround && html_length_equals (&style->surround->padding.right, padding))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->padding.right, padding);
	}
}

void
html_style_set_padding_top (HtmlStyle *style, const HtmlLength *padding)
{
	if (!(style->surround && html_length_equals (&style->surround->padding.top, padding))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->padding.top, padding);
	}
}

void
html_style_set_padding_bottom (HtmlStyle *style, const HtmlLength *padding)
{
	if (!(style->surround && html_length_equals (&style->surround->padding.bottom, padding))) {
		if (style->surround->refcount > 1)
			html_style_set_style_surround (style, html_style_surround_dup (style->surround));
		html_length_set (&style->surround->padding.bottom, padding);
	}
}
