#ifndef __HTMLBOXROOT_H__
#define __HTMLBOXROOT_H__

#include "htmlboxblock.h"

G_BEGIN_DECLS

#define HTML_TYPE_BOX_ROOT (html_box_root_get_type ())
#define HTML_BOX_ROOT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), HTML_TYPE_BOX_ROOT, HtmlBoxRoot))
#define HTML_BOX_ROOT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_BOX_ROOT, HtmlBoxRootClass))
#define HTML_IS_BOX_ROOT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), HTML_TYPE_BOX_ROOT))
#define HTML_IS_BOX_ROOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_BOX_ROOT))

typedef struct _HtmlBoxRoot HtmlBoxRoot;
typedef struct _HtmlBoxRootClass HtmlBoxRootClass;

struct _HtmlBoxRoot {
	HtmlBoxBlock parent_object;

	GSList *float_left_list;
	GSList *float_right_list;
	GSList *positioned_list;

	gint min_width, min_height;
};

struct _HtmlBoxRootClass {
	HtmlBoxBlockClass parent_class;
};

GType html_box_root_get_type (void);

HtmlBox *html_box_root_new (void);

GSList *html_box_root_get_float_left_list      (HtmlBoxRoot *root);
GSList *html_box_root_get_float_right_list     (HtmlBoxRoot *root);
GSList *html_box_root_get_positioned_list      (HtmlBoxRoot *root);
void    html_box_root_clear_float_left_list    (HtmlBoxRoot *root);
void    html_box_root_clear_float_right_list   (HtmlBoxRoot *root);
void    html_box_root_clear_positioned_list    (HtmlBoxRoot *root);
void    html_box_root_add_float                (HtmlBoxRoot *root, HtmlBox *box);
void    html_box_root_add_positioned           (HtmlBoxRoot *root, HtmlBox *box);
void    html_box_root_mark_floats_relayouted   (HtmlBoxRoot *root, HtmlBox *box);
void    html_box_root_mark_floats_unrelayouted (HtmlBoxRoot *root, HtmlBox *box);
void    html_box_root_paint_fixed_list         (HtmlPainter *painter, HtmlBox *root, gint tx, gint ty, GSList *list);

G_END_DECLS

#endif /* __HTMLBOXROOT_H__ */
