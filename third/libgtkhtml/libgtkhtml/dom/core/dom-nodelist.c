#include "dom-nodelist.h"

static GObjectClass *parent_class = NULL;

/**
 * dom_NodeList__get_length:
 * @list: a DomNodeList
 * 
 * Returns the number of nodes in the list. The range of valid child nodes is 0 to length-1.
 * 
 * Return value: The number of nodes in the list.
 **/
gulong
dom_NodeList__get_length (DomNodeList *list)
{
	return list->length (list);
}

/**
 * dom_NodeList__get_item:
 * @list: a DomNodeList
 * @index: Index into the node list.
 * 
 * Returns the indexth item in the collection or NULL if the index value specified is invalid.
 * 
 * Return value: The node at the indexth position in the node list.
 **/
DomNode *
dom_NodeList__get_item (DomNodeList *list, gulong index)
{
	return list->item (list, index);
}

static void
dom_node_list_finalize (GObject *object)
{
	DomNodeList *list = DOM_NODE_LIST (object);

	if (list->node)
		g_object_unref (list->node);
	
	if (list->str)
		g_free (list->str);

	parent_class->finalize (object);
}

static void
dom_node_list_class_init (DomNodeListClass *klass)
{
	GObjectClass *object_class = (GObjectClass *)klass;

	object_class->finalize = dom_node_list_finalize;

	parent_class = g_type_class_peek_parent (klass);
}

static void
dom_node_list_init (DomNodeList *list)
{
	list->node = NULL;
	list->str = NULL;
}

GType
dom_node_list_get_type (void)
{
	static GType dom_node_list_type = 0;

	if (!dom_node_list_type) {
		static const GTypeInfo dom_node_list_info = {
			sizeof (DomNodeListClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dom_node_list_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (DomNodeList),
			16,   /* n_preallocs */
			(GInstanceInitFunc) dom_node_list_init,
		};

		dom_node_list_type = g_type_register_static (G_TYPE_OBJECT, "DomNodeList", &dom_node_list_info, 0);
	}

	return dom_node_list_type;
}
