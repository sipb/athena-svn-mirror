#ifndef GTKHTML_IM_H_
#define GTKHTML_IM_H_

#include "gtkhtml-types.h"

void gtk_html_im_focus_in (GtkHTML *html);
void gtk_html_im_focus_out (GtkHTML *html);
void gtk_html_im_realize (GtkHTML *html);
void gtk_html_im_unrealize (GtkHTML *html); 
void gtk_html_im_size_allocate (GtkHTML *html); 
void gtk_html_im_style_set (GtkHTML *html);
void gtk_html_im_position_update (GtkHTML *html, int x, int y);

#endif
