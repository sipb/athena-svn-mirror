#ifndef __DOM_TEST_WINDOW_H__
#define __DOM_TEST_WINDOW_H__

#include <gtk/gtk.h>
#include "dom-test-tree-model.h"

#define DOM_TYPE_TEST_WINDOW             (dom_test_window_get_type ())
#define DOM_TEST_WINDOW(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_TEST_WINDOW, DomTestWindow))
#define DOM_TEST_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_TEST_WINDOW, DomTestWindowClass))
#define DOM_IS_TEST_WINDOW(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_TEST_WINDOW))
#define DOM_IS_TEST_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_TEST_WINDOW))
#define DOM_TEST_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_TEST_WINDOW, DomTestWindowClass))

typedef struct _DomTestWindow DomTestWindow;
typedef struct _DomTestWindowClass DomTestWindowClass;

struct _DomTestWindow {
	GtkWindow parent;

	DomNode *orphan_root_node;
	DomTestTreeModel *orphan_nodes;
	
	DomTestTreeModel *tree_model;

};

struct _DomTestWindowClass {
	GtkWindowClass parent_class;
};

GType dom_test_window_get_type (void);
GtkWidget *dom_test_window_new (void);
void dom_test_window_construct (DomTestWindow *window);

#endif /* __DOM_TEST_WINDOW_H__ */
