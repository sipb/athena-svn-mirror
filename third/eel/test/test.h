#ifndef TEST_H
#define TEST_H

#include <gtk/gtkwindow.h>

void       test_init                            (int                    *argc,
						 char                 ***argv);
int        test_quit                            (int                     exit_code);
void       test_delete_event                    (GtkWidget              *widget,
						 GdkEvent               *event,
						 gpointer                callback_data);
GtkWidget *test_window_new                      (const char             *title,
						 guint                   border_width);
void       test_gtk_widget_set_background_image (GtkWidget              *widget,
						 const char             *image_name);
void       test_gtk_widget_set_background_color (GtkWidget              *widget,
						 const char             *color_spec);
GdkPixbuf *test_pixbuf_new_named                (const char             *name,
						 float                   scale);
GtkWidget *test_image_new                       (const char             *pixbuf_name,
						 float                   scale,
						 gboolean                with_background);
GtkWidget *test_label_new                       (const char             *text,
						 gboolean                with_background,
						 int                     num_sizes_larger);
void       test_pixbuf_draw_rectangle_tiled     (GdkPixbuf              *pixbuf,
						 const char             *tile_name,
						 int                     x0,
						 int                     y0,
						 int                     x1,
						 int                     y1,
						 int                     opacity);
void       test_window_set_title_with_pid       (GtkWindow              *window,
						 const char             *title);
char *     eel_pixmap_file                      (const char             *partial_path);

// /* Preferences hacks */
// void test_text_caption_set_text_for_int_preferences            (EelTextCaption       *text_caption,
// 								const char                *name);
// void test_text_caption_set_text_for_string_preferences         (EelTextCaption       *text_caption,
// 								const char                *name);
// void test_text_caption_set_text_for_default_int_preferences    (EelTextCaption       *text_caption,
// 								const char                *name);
// void test_text_caption_set_text_for_default_string_preferences (EelTextCaption       *text_caption,
// 								const char                *name);

#include <eel/eel-debug.h>
#include <eel/eel.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs-init.h>

#endif /* TEST_H */
