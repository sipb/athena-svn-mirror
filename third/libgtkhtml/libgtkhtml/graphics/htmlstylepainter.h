#ifndef __HTMLSTYLEPAINTER_H__
#define __HTMLSTYLEPAINTER_H__

#include "layout/htmlbox.h"

void html_style_painter_draw_background_color (HtmlBox *self, HtmlPainter *painter, GdkRectangle *area, gint tx, gint ty);
void html_style_painter_draw_border (HtmlBox *self, HtmlPainter *painter, GdkRectangle *area, gint tx, gint ty);
void html_style_painter_draw_background_image (HtmlBox *self, HtmlPainter *painter, GdkRectangle *area, gint tx, gint ty);
void html_style_painter_draw_outline (HtmlBox *self, HtmlStyle *style, HtmlPainter *painter, GdkRectangle *area, gint tx, gint ty);

#endif /* __HTMLSTYLEPAINTER_H__ */
