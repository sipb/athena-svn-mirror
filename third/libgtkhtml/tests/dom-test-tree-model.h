#ifndef __DOM_TEST_TREE_MODEL_H__
#define __DOM_TEST_TREE_MODEL_H__

#include <glib-object.h>
#include <dom/dom.h>

typedef struct _DomTestTreeModel DomTestTreeModel;
typedef struct _DomTestTreeModelClass DomTestTreeModelClass;

#define DOM_TYPE_TEST_TREE_MODEL             (dom_test_tree_model_get_type ())
#define DOM_TEST_TREE_MODEL(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_TEST_TREE_MODEL, DomTestTreeModel))
#define DOM_TEST_TREE_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_TEST_TREE_MODEL, DomTestTreeModelClass))
#define DOM_IS_TEST_TREE_MODEL(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_TEST_TREE_MODEL))
#define DOM_IS_TEST_TREE_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_TEST_TREE_MODEL))
#define DOM_TEST_TREE_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_TEST_TREE_MODEL, DomTestTreeModelClass))

typedef enum _DomTestTreeModelType DomTestTreeModelType;

enum _DomTestTreeModelType {
	DOM_TEST_TREE_MODEL_TREE,
	DOM_TEST_TREE_MODEL_LIST,
};

struct _DomTestTreeModel {
	GObject parent;

	DomNode *root;

	DomTestTreeModelType type;
};

struct _DomTestTreeModelClass {
	GObjectClass parent_class;
};


GType dom_test_tree_model_get_type (void);

DomTestTreeModel *dom_test_tree_model_new (DomNode *root, DomTestTreeModelType type);

#endif /* __DOM_TEST_TREE_MODEL_H__ */

