#ifndef _BONOBO_UI_NODE_PRIVATE_H_
#define _BONOBO_UI_NODE_PRIVATE_H_

/* All this for xmlChar, xmlStrdup !? */
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <bonobo/bonobo-ui-node.h>

G_BEGIN_DECLS

struct _BonoboUINode {
	/* Tree management */
	BonoboUINode *parent;
	BonoboUINode *children;
	BonoboUINode *prev;
	BonoboUINode *next;

	/* The useful bits */
	GQuark        name_id;
	int           ref_count;
	xmlChar      *content;
	GArray       *attrs;
	gpointer      user_data;
};

typedef struct {
	GQuark        id;
	xmlChar      *value;
} BonoboUIAttr;

gboolean      bonobo_ui_node_try_set_attr   (BonoboUINode *node,
					     GQuark        prop,
					     const char   *value);
void          bonobo_ui_node_set_attr_by_id (BonoboUINode *node,
					     GQuark        id,
					     const char   *value);
const char   *bonobo_ui_node_get_attr_by_id (BonoboUINode *node,
					     GQuark        id);
const char   *bonobo_ui_node_peek_attr      (BonoboUINode *node,
					     const char   *name);
const char   *bonobo_ui_node_peek_content   (BonoboUINode *node);
gboolean      bonobo_ui_node_has_name_by_id (BonoboUINode *node,
					     GQuark        id);
void          bonobo_ui_node_add_after      (BonoboUINode *before,
					     BonoboUINode *new_after);
void          bonobo_ui_node_move_children  (BonoboUINode *from,
					     BonoboUINode *to);
#define       bonobo_ui_node_same_name(a,b) ((a)->name_id == (b)->name_id)
BonoboUINode *bonobo_ui_node_get_path_child (BonoboUINode *node,
					     const char   *name);
BonoboUINode *bonobo_ui_node_ref            (BonoboUINode *node);
void          bonobo_ui_node_unref          (BonoboUINode *node);


G_END_DECLS

#endif /* _BONOBO_UI_NODE_PRIVATE_H_ */
