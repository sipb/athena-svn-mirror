/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-win.c: The Bonobo Window implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include "config.h"
#include <gnome.h>
#include <bonobo.h>
#include <liboaf/liboaf.h>

#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-win.h>

#include <bonobo/bonobo-ui-toolbar.h>
#include <bonobo/bonobo-ui-toolbar-button-item.h>
#include <bonobo/bonobo-ui-toolbar-toggle-button-item.h>
#include <bonobo/bonobo-ui-toolbar-separator-item.h>
#include <bonobo/bonobo-ui-toolbar-popup-item.h>
#include <bonobo/bonobo-ui-toolbar-control-item.h>

#include <bonobo/bonobo-ui-node.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>

#undef STATE_SYNC_DEBUG
#undef WIDGET_SYNC_DEBUG
#undef XML_MERGE_DEBUG

#define	BINDING_MOD_MASK()				\
	(gtk_accelerator_get_default_mod_mask () | GDK_RELEASE_MASK)

#define BONOBO_WINDOW_PRIV_KEY "BonoboWindow::Priv"

GtkWindowClass *bonobo_window_parent_class = NULL;

typedef struct _StateUpdate StateUpdate;

struct _BonoboWindowPrivate {
	BonoboWindow  *win;

	BonoboObject  *container;

	int            frozen;

	GnomeDock     *dock;

	GnomeDockItem *menu_item;
	GtkMenuBar    *menu;

	BonoboUIXml   *tree;

	GtkAccelGroup *accel_group;

	char          *name;		/* Win name */
	char          *prefix;		/* Win prefix */

	GHashTable    *radio_groups;
	GHashTable    *keybindings;
	GSList        *components;
	GSList        *popups;

	GtkWidget     *main_vbox;

	GtkBox        *status;
	GtkStatusbar  *main_status;

	GtkWidget     *client_area;

	GSList        *state_updates;
};

typedef enum {
	ROOT_WIDGET   = 0x1,
	CUSTOM_WIDGET = 0x2
} NodeType;

#define NODE_IS_ROOT_WIDGET(n)   ((n->type & ROOT_WIDGET) != 0)
#define NODE_IS_CUSTOM_WIDGET(n) ((n->type & CUSTOM_WIDGET) != 0)

typedef struct {
	BonoboUIXmlData parent;

	int             type;
	GtkWidget      *widget;
	Bonobo_Unknown  object;
} NodeInfo;

static BonoboUIXmlData *
info_new_fn (void)
{
	NodeInfo *info = g_new0 (NodeInfo, 1);

	info->object = CORBA_OBJECT_NIL;

	return (BonoboUIXmlData *) info;
}

static void
info_free_fn (BonoboUIXmlData *data)
{
	NodeInfo *info = (NodeInfo *) data;

	if (info->object != CORBA_OBJECT_NIL) {
		bonobo_object_release_unref (info->object, NULL);
		info->object = CORBA_OBJECT_NIL;
	}
	info->widget = NULL;

	g_free (data);
}

#define WIDGET_NODE_KEY "BonoboWindow:NodeKey"

static BonoboUINode *
widget_get_node (GtkWidget *widget)
{
	g_return_val_if_fail (widget != NULL, NULL);
	
	return gtk_object_get_data (GTK_OBJECT (widget),
				    WIDGET_NODE_KEY);
}

static void
info_dump_fn (BonoboUIXml *tree, BonoboUINode *node)
{
	NodeInfo *info = bonobo_ui_xml_get_data (tree, node);

	if (info) {
		fprintf (stderr, " '%15s' object %8p type %d ",
			 (char *)info->parent.id, info->object, info->type);

		if (info->widget) {
			BonoboUINode *attached_node = widget_get_node (info->widget);

			fprintf (stderr, "widget '%8p' with node '%8p' attached ",
				 info->widget, attached_node);

			if (attached_node == NULL)
				fprintf (stderr, "is NULL\n");

			else if (attached_node != node)
				fprintf (stderr, "Serious mismatch attaches should be '%8p'\n",
					 node);
			else {
				if (info->widget->parent)
					fprintf (stderr, "and matching; parented\n");
				else
					fprintf (stderr, "and matching; BUT NO PARENT!\n");
			}
 		} else
			fprintf (stderr, " no associated widget\n");
	} else
		fprintf (stderr, " very weird no data on node '%p'\n", node);
}

/* We need to map the shell to the item */
static GtkWidget *
get_item_widget (GtkWidget *widget)
{
	GtkWidget *ret;

	if (!widget)
		return NULL;

	if (GTK_IS_MENU (widget)) {
		ret = gtk_menu_get_attach_widget (
			GTK_MENU (widget));
	} else
		ret = widget;

	return ret;
}

static void
widget_set_node (GtkWidget    *widget,
		 BonoboUINode *node)
{
	if (widget) {
		GtkWidget *aux_widget;

		gtk_object_set_data (GTK_OBJECT (widget),
				     WIDGET_NODE_KEY, node);

		if ((aux_widget = get_item_widget (widget)) != widget)

			gtk_object_set_data (GTK_OBJECT (aux_widget),
					     WIDGET_NODE_KEY, node);
	}
}

static char *
node_get_id (BonoboUINode *node)
{
	xmlChar *txt;
	char    *ret;

	g_return_val_if_fail (node != NULL, NULL);

	if (!(txt = bonobo_ui_node_get_attr (node, "id"))) {
		txt = bonobo_ui_node_get_attr (node, "verb");
		if (txt && txt [0] == '\0') {
			bonobo_ui_node_free_string (txt);
			txt = bonobo_ui_node_get_attr (node, "name");
		}
	}

	if (txt) {
		ret = g_strdup (txt);
		bonobo_ui_node_free_string (txt);
	} else
		ret = NULL;

	return ret;
}

static char *
node_get_id_or_path (BonoboUINode *node)
{
	char *txt;

	g_return_val_if_fail (node != NULL, NULL);

	if ((txt = node_get_id (node)))
		return txt;

	return bonobo_ui_xml_make_path (node);
}

/*
 *  This indirection is needed so we can serialize user settings
 * on a per component basis in future.
 */
typedef struct {
	char          *name;
	Bonobo_Unknown object;
} WinComponent;

typedef struct {
	GtkMenu          *menu;
	char             *path;
} WinPopup;

static WinComponent *
win_component_get (BonoboWindowPrivate *priv, const char *name)
{
	WinComponent *component;
	GSList       *l;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (priv != NULL, NULL);

	for (l = priv->components; l; l = l->next) {
		component = l->data;
		
		if (!strcmp (component->name, name))
			return component;
	}
	
	component = g_new (WinComponent, 1);
	component->name = g_strdup (name);
	component->object = CORBA_OBJECT_NIL;

	priv->components = g_slist_prepend (priv->components, component);

	return component;
}

static WinComponent *
win_component_get_by_ref (BonoboWindowPrivate *priv, CORBA_Object obj)
{
	GSList *l;
	WinComponent *component = NULL;
	CORBA_Environment ev;

	g_return_val_if_fail (priv != NULL, NULL);
	g_return_val_if_fail (obj != CORBA_OBJECT_NIL, NULL);

	CORBA_exception_init (&ev);

	for (l = priv->components; l; l = l->next) {
		gboolean equiv;
		component = l->data;

		equiv = CORBA_Object_is_equivalent (component->object, obj, &ev);

		if (BONOBO_EX (&ev)) { /* Something very badly wrong */
			component = NULL;
			break;
		} else if (equiv)
			break;
	}

	CORBA_exception_free (&ev);

	return component;
}

static Bonobo_Unknown
win_component_objref (BonoboWindowPrivate *priv, const char *name)
{
	WinComponent *component = win_component_get (priv, name);

	g_return_val_if_fail (component != NULL, CORBA_OBJECT_NIL);

	return component->object;
}

static void
win_components_dump (BonoboWindowPrivate *priv)
{
	GSList *l;

	g_return_if_fail (priv != NULL);

	fprintf (stderr, "Component mappings:\n");

	for (l = priv->components; l; l = l->next) {
		WinComponent *component = l->data;
		
		fprintf (stderr, "\t'%s' -> '%p'\n",
			 component->name, component->object);
	}
}

/*
 * Use the pointer identity instead of a costly compare
 */
static char *
win_component_cmp_name (BonoboWindowPrivate *priv, const char *name)
{
	WinComponent *component;

	/*
	 * NB. For overriding if we get a NULL we just update the
	 * node without altering the id.
	 */
	if (!name || name [0] == '\0') {
		g_warning ("This should never happen");
		return NULL;
	}

	component = win_component_get (priv, name);
	g_return_val_if_fail (component != NULL, NULL);

	return component->name;
}

static void
win_component_destroy (BonoboWindowPrivate *priv, WinComponent *component)
{
	priv->components = g_slist_remove (priv->components, component);

	if (component) {
		g_free (component->name);
		if (component->object != CORBA_OBJECT_NIL)
			bonobo_object_release_unref (component->object, NULL);
		g_free (component);
	}
}

void
bonobo_window_deregister_dead_components (BonoboWindow *win)
{
	WinComponent      *component;
	GSList            *l, *next;
	CORBA_Environment  ev;

	g_return_if_fail (BONOBO_IS_WINDOW (win));

	for (l = win->priv->components; l; l = next) {
		next = l->next;

		component = l->data;
		CORBA_exception_init (&ev);

		if (CORBA_Object_non_existent (component->object, &ev))
			bonobo_window_deregister_component (win, component->name);

		CORBA_exception_free (&ev);
	}
}

void
bonobo_window_register_component (BonoboWindow  *win,
				  const char    *name,
				  Bonobo_Unknown component)
{
	WinComponent *wincomp;

	g_return_if_fail (BONOBO_IS_WINDOW (win));

	if ((wincomp = win_component_get (win->priv, name))) {
		if (wincomp->object != CORBA_OBJECT_NIL)
			bonobo_object_release_unref (wincomp->object, NULL);
	}

	if (component != CORBA_OBJECT_NIL)
		wincomp->object = bonobo_object_dup_ref (component, NULL);
	else
		wincomp->object = CORBA_OBJECT_NIL;
}

void
bonobo_window_deregister_component (BonoboWindow *win,
				    const char   *name)
{
	WinComponent *component;

	g_return_if_fail (BONOBO_IS_WINDOW (win));

	if ((component = win_component_get (win->priv, name))) {
		bonobo_window_xml_rm (win, "/", component->name);
		win_component_destroy (win->priv, component);
	} else
		g_warning ("Attempting to deregister non-registered "
			   "component '%s'", name);
}

void
bonobo_window_deregister_component_by_ref (BonoboWindow  *win,
					   Bonobo_Unknown ref)
{
	WinComponent *component;

	g_return_if_fail (BONOBO_IS_WINDOW (win));

	if ((component = win_component_get_by_ref (win->priv, ref))) {
		bonobo_window_xml_rm (win, "/", component->name);
		win_component_destroy (win->priv, component);
	} else
		g_warning ("Attempting to deregister non-registered "
			   "component");
}

static gboolean
widget_is_special (GtkWidget *widget)
{
	g_return_val_if_fail (widget != NULL, TRUE);

	if (GTK_IS_TEAROFF_MENU_ITEM (widget))
		return TRUE;

	if (BONOBO_IS_UI_TOOLBAR_POPUP_ITEM (widget))
		return TRUE;

	return FALSE;
}

static gboolean
node_is_dirty (BonoboWindowPrivate *priv, BonoboUINode *node)
{
	BonoboUIXmlData *data = bonobo_ui_xml_get_data (priv->tree, node);

	if (!data)
		return TRUE;
	else
		return data->dirty;
}

#if 0
static gboolean
node_is_placeholder (BonoboUINode *node)
{
	g_return_val_if_fail (node != NULL, FALSE);
	return !strcmp (bonobo_ui_node_get_name (node), "placeholder");
}

/**
 * next_node:
 * @node: the node to start from
 * @first: Whether we are looking for the first valid node or
 *         the next node.
 * 
 * This iterator flattens a placeholder tree into a list of valid items.
 * 
 * Return value: the first or next item depending on 'first'
 **/
static BonoboUINode *
next_node (BonoboUINode *node, gboolean first)
{
	BonoboUINode *ret;

	if (node == NULL)
		return NULL;

	if (first && node_is_placeholder (node)) {
		if ((ret = next_node (bonobo_ui_node_children (node), TRUE))) {
			g_assert (ret == NULL || !node_is_placeholder (ret));
			return ret;
		} else /* it was an empty placeholder tree */
			first = FALSE;
	}
		
	/* not a placeholder */
	if (first) {
		g_assert (node == NULL || !node_is_placeholder (node));
		return node;
	}

	if (bonobo_ui_node_next (node))
		ret = next_node (bonobo_ui_node_next (node), TRUE);
	else { /* we hit the end of a list */
		BonoboUINode *parent;

		while ((parent = bonobo_ui_node_parent (node)) &&
		       node_is_placeholder (parent)) {
			node = parent;
			
			if (bonobo_ui_node_next (node))
				break;
		}

		ret = next_node (bonobo_ui_node_next (node), TRUE);
	}

	g_assert (ret == NULL || !node_is_placeholder (ret));

	return ret;
}
#endif

static void
placeholder_sync (BonoboWindowPrivate *priv,
		  BonoboUINode     *node,
		  GtkWidget        *widget,
		  GtkWidget        *parent)
{
	gboolean show = FALSE;
	char    *txt;

	if ((txt = bonobo_ui_node_get_attr (node, "delimit"))) {
		show = !strcmp (txt, "top");
		bonobo_ui_node_free_string (txt);
	}

	if (show)
		gtk_widget_show (widget);	
	else
		gtk_widget_hide (widget);
}

static void
hide_all_widgets (BonoboWindowPrivate *priv,
		  BonoboUINode *node)
{
	NodeInfo *info;
	BonoboUINode *child;
	
	info = bonobo_ui_xml_get_data (priv->tree, node);
	if (info->widget)
		gtk_widget_hide (info->widget);
	
	for (child = bonobo_ui_node_children (node);
	     child != NULL;
	     child = bonobo_ui_node_next (child))
		hide_all_widgets (priv, child);
}

static gboolean
contains_visible_widget (BonoboWindowPrivate *priv,
			 BonoboUINode *node)
{
	BonoboUINode *child;
	NodeInfo *info;
	
	for (child = bonobo_ui_node_children (node);
	     child != NULL;
	     child = bonobo_ui_node_next (child)) {
		info = bonobo_ui_xml_get_data (priv->tree, child);
		if (info->widget && GTK_WIDGET_VISIBLE (info->widget))
			return TRUE;
		if (contains_visible_widget (priv, child))
			return TRUE;
	}

	return FALSE;
}

static void
hide_placeholder_if_empty_or_hidden (BonoboWindowPrivate *priv,
				     BonoboUINode *node)
{
	NodeInfo *info;
	char *txt;
	gboolean hide_placeholder_and_contents;
	gboolean has_visible_separator;

	txt = bonobo_ui_node_get_attr (node, "hidden");
	hide_placeholder_and_contents = txt && atoi (txt);
	bonobo_ui_node_free_string (txt);

	info = bonobo_ui_xml_get_data (priv->tree, node);
	has_visible_separator = info && info->widget
		&& GTK_WIDGET_VISIBLE (info->widget);

	if (hide_placeholder_and_contents)
		hide_all_widgets (priv, node);
	else if (has_visible_separator
		 && !contains_visible_widget (priv, node))
		gtk_widget_hide (info->widget);
}

typedef void       (*SyncStateFn)   (BonoboWindowPrivate *priv,
				     BonoboUINode     *node,
				     GtkWidget        *widget,
				     GtkWidget        *parent);
typedef GtkWidget *(*BuildWidgetFn) (BonoboWindowPrivate *priv,
				     BonoboUINode     *node,
				     int              *pos,
				     NodeInfo         *info,
				     GtkWidget        *parent);

static void
sync_generic_widgets (BonoboWindowPrivate *priv,
		      BonoboUINode     *node,
		      GtkWidget        *parent,
		      GList           **widgets,
		      int              *pos,
		      SyncStateFn       sync_state,
		      BuildWidgetFn     build_widget,
		      BuildWidgetFn     make_placeholder)
{
	BonoboUINode *a;
	GList        *b, *nextb;

#ifdef WIDGET_SYNC_DEBUG
	printf ("In sync to pos %d with widgets:\n", *pos);
	for (b = *widgets; b; b = b->next) {
		BonoboUINode *node = widget_get_node (b->data);

		if (node)
			printf ("\t'%s'\n", bonobo_ui_xml_make_path (node));
		else
			printf ("\tno node ptr\n");
	}
#endif

	b = *widgets;
	for (a = node; a; b = nextb) {
		gboolean same;

		nextb = b ? b->next : NULL;

		if (b && widget_is_special (b->data)) {
			(*pos)++;
			continue;
		}

		same = (b != NULL) && (widget_get_node (b->data) == a);

#ifdef WIDGET_SYNC_DEBUG
		printf ("Node '%s' Dirty '%d' same %d on b %d widget %p\n",
			bonobo_ui_xml_make_path (a),
			node_is_dirty (priv, a), same, b != NULL, b?b->data:NULL);
#endif

		if (node_is_dirty (priv, a)) {
			SyncStateFn   ss;
			BuildWidgetFn bw;
			
			if (bonobo_ui_node_has_name (a, "placeholder")) {
				ss = placeholder_sync;
				bw = make_placeholder;
			} else {
				ss = sync_state;
				bw = build_widget;
			}

			if (same) {
#ifdef WIDGET_SYNC_DEBUG
				printf ("-- just syncing state --\n");
#endif
				ss (priv, a, b->data, parent);
				(*pos)++;
			} else {
				NodeInfo   *info;
				GtkWidget  *widget;

				info = bonobo_ui_xml_get_data (priv->tree, a);

#ifdef WIDGET_SYNC_DEBUG
				printf ("re-building widget\n");
#endif

				widget = bw (priv, a, pos, info, parent);

#ifdef WIDGET_SYNC_DEBUG
				printf ("Built item '%p' '%s' and inserted at '%d'\n",
					widget, bonobo_ui_node_get_name (a), *pos);
#endif

				info->widget = widget;
				if (widget) {
					widget_set_node (widget, a);
					ss (priv, a, widget, parent);
				}
#ifdef WIDGET_SYNC_DEBUG
				else
					printf ("Failed to build widget\n");
#endif

				nextb = b; /* NB. don't advance 'b' */
			}

		} else {
			if (!same) {
				BonoboUINode *bn = b ? widget_get_node (b->data) : NULL;
				NodeInfo   *info;

				info = bonobo_ui_xml_get_data (priv->tree, a);

				if (!info->widget) {
					/*
					 *  A control that hasn't been filled out yet
					 * and thus has no widget in 'b' list but has
					 * a node in 'a' list, thus we want to stick
					 * on this 'b' node until a more favorable 'a'
					 */
					nextb = b;
					(*pos)--;
					g_assert (info->type | CUSTOM_WIDGET);
#ifdef WIDGET_SYNC_DEBUG
					printf ("not dirty & not same, but has no widget\n");
#endif
				} else {
					g_warning ("non dirty node, but widget mismatch "
						   "a: '%s:%s', b: '%s:%s' '%p'",
						   bonobo_ui_node_get_name (a),
						   bonobo_ui_node_get_attr (a, "name"),
						   bn ? bonobo_ui_node_get_name (bn) : "NULL",
						   bn ? bonobo_ui_node_get_attr (bn, "name") : "NULL",
						   info->widget);
				}
			}
#ifdef WIDGET_SYNC_DEBUG
			else
				printf ("not dirty & same: no change\n");
#endif
			(*pos)++;
		}

		if (bonobo_ui_node_has_name (a, "placeholder")) {
			sync_generic_widgets (priv, bonobo_ui_node_children (a),
					      parent, &nextb, pos, sync_state,
					      build_widget, make_placeholder);
			hide_placeholder_if_empty_or_hidden (priv, a);
		}

		a = bonobo_ui_node_next (a);
	}

	while (b && widget_is_special (b->data))
		b = b->next;

	*widgets = b;
}

static void
check_excess_widgets (BonoboWindowPrivate *priv, GList *wptr)
{
	if (wptr) {
		GList *b;
		int warned = 0;

		for (b = wptr; b; b = b->next) {
			BonoboUINode *node;

			if (widget_is_special (b->data))
				continue;
			
			if (!warned++)
				g_warning ("Excess widgets at the "
					   "end of the container; weird");

			node = widget_get_node (b->data);
			g_message ("Widget type '%s' with node: '%s'",
				   gtk_type_name (GTK_OBJECT (b->data)->klass->type),
				   node ? bonobo_ui_xml_make_path (node) : "NULL");
		}
	}
}

/*
 * Returns: TRUE if hiddenness changed.
 */
static gboolean
do_show_hide (GtkWidget *widget, BonoboUINode *node)
{
	char    *txt;
	gboolean changed;

	widget = get_item_widget (widget);

	if (!widget)
		return FALSE;

	if ((txt = bonobo_ui_node_get_attr (node, "hidden"))) {
		if (atoi (txt)) {
			changed = GTK_WIDGET_VISIBLE (widget);
			gtk_widget_hide (widget);
		} else {
			changed = !GTK_WIDGET_VISIBLE (widget);
			gtk_widget_show (widget);
		}
		bonobo_ui_node_free_string (txt);
	} else {
		changed = !GTK_WIDGET_VISIBLE (widget);
		gtk_widget_show (widget);
	}

	return changed;
}

struct _StateUpdate {
	GtkWidget *widget;
	char      *state;
};

/* Update the state later, but other aspects of the widget right now.
 * It's dangerous to update the state now because we can reenter if we
 * do that.
 */
static StateUpdate *
state_update_new (GtkWidget *widget, BonoboUINode *node)
{
	char *hidden, *sensitive;
	StateUpdate *su;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	hidden = bonobo_ui_node_get_attr (node, "hidden");
	if (hidden && atoi (hidden))
		gtk_widget_hide (widget);
	else
		gtk_widget_show (widget);
	bonobo_ui_node_free_string (hidden);

	sensitive = bonobo_ui_node_get_attr (node, "sensitive");
	if (sensitive)
		gtk_widget_set_sensitive (widget, atoi (sensitive));
	bonobo_ui_node_free_string (sensitive);

	su = g_new0 (StateUpdate, 1);
	su->widget = widget;
	gtk_widget_ref (su->widget);
	su->state = bonobo_ui_node_get_attr (node, "state");

	return su;
}

static void
state_update_destroy (StateUpdate *su)
{
	if (su) {
		gtk_widget_unref (su->widget);
		bonobo_ui_node_free_string (su->state);

		g_free (su);
	}
}

static void
state_update_exec (StateUpdate *su)
{
	g_return_if_fail (su != NULL);
	g_return_if_fail (su->widget != NULL);

	if (su->state) {
		if (BONOBO_IS_UI_TOOLBAR_ITEM (su->widget))
			bonobo_ui_toolbar_item_set_state (
				BONOBO_UI_TOOLBAR_ITEM (su->widget), su->state);

		else if (GTK_IS_CHECK_MENU_ITEM (su->widget)) {
#ifdef STATE_SYNC_DEBUG
			g_warning ("Setting check menu item '%p' to '%s'",
				   su->widget, su->state);
#endif
			gtk_check_menu_item_set_active (
				GTK_CHECK_MENU_ITEM (su->widget), 
				atoi (su->state));
		} else
			g_warning ("TESTME: strange, setting "
				   "state '%s' on weird object '%s'",
				   su->state, gtk_type_name (GTK_OBJECT (
					   su->widget)->klass->type));
	}
}

static void
widget_set_state (GtkWidget *widget, BonoboUINode *node)
{
	StateUpdate *su;

	if (!widget)
		return;

	su = state_update_new (widget, node);
	
	state_update_exec (su);

	state_update_destroy (su);
}

static void
widget_queue_state (BonoboWindowPrivate *priv,
		    GtkWidget           *widget,
		    BonoboUINode        *node)
{
	StateUpdate *su = state_update_new (widget, node);
	
	priv->state_updates = g_slist_prepend (
		priv->state_updates, su);
}

static GSList *
find_widgets_for_command (BonoboWindowPrivate *priv,
			  GSList              *list,
			  BonoboUINode        *search,
			  BonoboUINode        *state,
			  const char          *search_id)
{
	BonoboUINode *l;
	char *id = node_get_id (search);

/*	printf ("Update cmd state if %s == %s on node '%s'\n", search_id, id,
	bonobo_ui_xml_make_path (search));*/

	if (id && !strcmp (search_id, id)) { /* Sync its state */
		NodeInfo *info = bonobo_ui_xml_get_data (priv->tree, search);

		if (info->widget)
			list = g_slist_prepend (
				list, state_update_new (info->widget, state));
	}

	g_free (id);

	for (l = bonobo_ui_node_children (search); l;
             l = bonobo_ui_node_next (l))
		list = find_widgets_for_command (priv, list, l, state, search_id);

	return list;
}

static void
update_cmd_state (BonoboWindowPrivate *priv, BonoboUINode *search,
		  BonoboUINode *state, const char *search_id)
{
	GSList *updates, *l;

	g_return_if_fail (search_id != NULL);

/*	printf ("Update cmd state on %s from node '%s'\n", search_id,
	bonobo_ui_xml_make_path (state));*/

	updates = find_widgets_for_command (
		priv, NULL, priv->tree->root, state, search_id);

	for (l = updates; l; l = l->next)
		state_update_exec (l->data);

	for (l = updates; l; l = l->next)
		state_update_destroy (l->data);

	g_slist_free (updates);
}

static void
update_commands_state (BonoboWindowPrivate *priv)
{
	BonoboUINode *cmds, *l;

	cmds = bonobo_ui_xml_get_path (priv->tree, "/commands");

/*	g_warning ("Update commands state!");
	bonobo_window_dump (priv->win, "before update");*/

	if (!cmds)
		return;

	for (l = bonobo_ui_node_children (cmds); l;
             l = bonobo_ui_node_next (l)) {
		BonoboUIXmlData *data = bonobo_ui_xml_get_data (priv->tree, l);
		char *cmd_name;

		cmd_name = bonobo_ui_node_get_attr (l, "name");
		if (!cmd_name)
			g_warning ("Internal error; cmd with no id");

		else if (data->dirty)
			update_cmd_state (priv, priv->tree->root, l, cmd_name);

		data->dirty = FALSE;
		bonobo_ui_node_free_string (cmd_name);
	}
}

static BonoboUINode *
cmd_get_node (BonoboWindowPrivate *priv,
	      BonoboUINode     *from_node)
{
	char         *path;
	BonoboUINode *ret;
	char         *cmd_name;

	g_return_val_if_fail (priv != NULL, NULL);

	if (!from_node)
		return NULL;

	if (!(cmd_name = node_get_id (from_node)))
		return NULL;

	path = g_strconcat ("/commands/", cmd_name, NULL);
	ret  = bonobo_ui_xml_get_path (priv->tree, path);

	if (!ret) {
		BonoboUIXmlData *data_from;
		BonoboUINode *commands;
		BonoboUINode *node;

		commands = bonobo_ui_node_new ("commands");
		node     = bonobo_ui_node_new_child (commands, "cmd");

		bonobo_ui_node_set_attr (node, "name", cmd_name);

		data_from   = bonobo_ui_xml_get_data (priv->tree, from_node);

		bonobo_ui_xml_merge (
			priv->tree, "/", commands, data_from->id);
		
		ret = bonobo_ui_xml_get_path (priv->tree, path);
		g_assert (ret != NULL);
	}

	g_free (path);
	g_free (cmd_name);

	return ret;
}

static char *
cmd_get_attr (BonoboUINode     *node,
	      BonoboUINode     *cmd_node,
	      const char       *attr)
{
	char *txt;

	if ((txt = bonobo_ui_node_get_attr (node, attr)))
		return txt;

	if (cmd_node && (txt = bonobo_ui_node_get_attr (cmd_node, attr)))
		return txt;

	return NULL;
}

static GdkPixbuf *
cmd_get_toolbar_pixbuf (BonoboUINode     *node,
			BonoboUINode     *cmd_node)
{
	GdkPixbuf *icon_pixbuf;
	char      *type;

	if ((type = bonobo_ui_node_get_attr (node, "pixtype"))) {
		icon_pixbuf = bonobo_ui_util_xml_get_icon_pixbuf (node, FALSE);
		bonobo_ui_node_free_string (type);
		return icon_pixbuf;
	}

	if ((type = bonobo_ui_node_get_attr (cmd_node, "pixtype"))) {
		icon_pixbuf = bonobo_ui_util_xml_get_icon_pixbuf (cmd_node, FALSE);
		bonobo_ui_node_free_string (type);
		return icon_pixbuf;
	}

	return NULL;
}

static GtkWidget *
cmd_get_menu_pixmap (BonoboUINode     *node,
		     BonoboUINode     *cmd_node)
{
	GtkWidget *pixmap;
	char      *type;

	if ((type = bonobo_ui_node_get_attr (node, "pixtype"))) {
		pixmap = bonobo_ui_util_xml_get_icon_pixmap_widget (node, TRUE);
		bonobo_ui_node_free_string (type);
		return pixmap;
	}

	if ((type = bonobo_ui_node_get_attr (cmd_node, "pixtype"))) {
		pixmap = bonobo_ui_util_xml_get_icon_pixmap_widget (cmd_node, TRUE);
		bonobo_ui_node_free_string (type);
		return pixmap;
	}

	return NULL;
}

/*
 * set_cmd_attr:
 *   Syncs cmd / widgets on events [ event flag set ]
 *   or helps evil people who set state on menu /
 *      toolbar items instead of on the associated verb / id.
 **/
static void
set_cmd_attr (BonoboWindowPrivate *priv,
	      BonoboUINode     *node,
	      const char       *prop,
	      const char       *value,
	      gboolean          event)
{
	BonoboUINode *cmd_node;

	g_return_if_fail (priv != NULL);
	g_return_if_fail (prop != NULL);
	g_return_if_fail (node != NULL);
	g_return_if_fail (value != NULL);

	if (!(cmd_node = cmd_get_node (priv, node))) { /* A non cmd widget */
		NodeInfo *info = bonobo_ui_xml_get_data (priv->tree, node);

		bonobo_ui_node_set_attr (node, prop, value);
		widget_set_state (info->widget, node);
		return;
	}

#ifdef STATE_SYNC_DEBUG
	fprintf (stderr, "Set '%s' : '%s' to '%s' (%d)",
		 cmd_name, prop, value, immediate_update);
#endif

	bonobo_ui_node_set_attr (cmd_node, prop, value);

	if (event) {
		char *cmd_name = bonobo_ui_node_get_attr (cmd_node, "name");
		update_cmd_state (priv, priv->tree->root, cmd_node, cmd_name);
		bonobo_ui_node_free_string (cmd_name);
	} else {
		BonoboUIXmlData *data =
			bonobo_ui_xml_get_data (priv->tree, cmd_node);

		data->dirty = TRUE;
	}
}

static void
real_emit_ui_event (BonoboWindowPrivate *priv, const char *component_name,
		    const char *id, int type, const char *new_state)
{
	Bonobo_UIComponent component;

	g_return_if_fail (id != NULL);
	g_return_if_fail (new_state != NULL);

	if (!component_name) /* Auto-created entry, no-one can listen to it */
		return;

	gtk_object_ref (GTK_OBJECT (priv->win));

	component = win_component_objref (priv, component_name);

	if (component != CORBA_OBJECT_NIL) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);

		Bonobo_UIComponent_uiEvent (
			component, id, type, new_state, &ev);

		if (priv->container)
			bonobo_object_check_env (
				priv->container, component, &ev);

		if (BONOBO_EX (&ev))
			g_warning ("Exception emitting state change to %d '%s' '%s'"
				   "major %d, %s",
				   type, id, new_state, ev._major, ev._repo_id);
		
		CORBA_exception_free (&ev);
	} else
		g_warning ("NULL Corba handle of name '%s'", component_name);

	gtk_object_unref (GTK_OBJECT (priv->win));
}

static void
custom_widget_unparent (NodeInfo *info)
{
	GtkContainer *container;

	g_return_if_fail (info != NULL);

	if (!info->widget)
		return;

	g_return_if_fail (GTK_IS_WIDGET (info->widget));

	if (info->widget->parent) {
		container = GTK_CONTAINER (info->widget->parent);
		g_return_if_fail (container != NULL);

		gtk_widget_ref (info->widget);
		gtk_container_remove (container, info->widget);
	}
}

static GnomeDockItem *
get_dock_item (BonoboWindowPrivate *priv,
	       const char       *dockname)
{
	guint dummy;
	
	return gnome_dock_get_item_by_name (priv->dock,
					    dockname,
					    &dummy, &dummy,
					    &dummy, &dummy);
}

static void
replace_override_fn (GtkObject        *object,
		     BonoboUINode     *new,
		     BonoboUINode     *old,
		     BonoboWindowPrivate *priv)
{
	NodeInfo  *info = bonobo_ui_xml_get_data (priv->tree, new);
	NodeInfo  *old_info = bonobo_ui_xml_get_data (priv->tree, old);
	GtkWidget *old_widget;

	g_return_if_fail (info != NULL);
	g_return_if_fail (old_info != NULL);

/*	g_warning ("Replace override on '%s' '%s' widget '%p'",
		   old->name, bonobo_ui_node_get_attr (old, "name"), old_info->widget);
	info_dump_fn (old_info);
	info_dump_fn (info);*/

	/* Copy useful stuff across */
	old_widget = old_info->widget;
	old_info->widget = NULL;

	info->type = old_info->type;
	info->widget = old_widget;

	/* Re-stamp the widget */
	widget_set_node (info->widget, new);

	/* Steal object reference */
	info->object = old_info->object;
	old_info->object = CORBA_OBJECT_NIL;
}

static void
prune_widget_info (BonoboWindowPrivate *priv,
		   BonoboUINode     *node,
		   gboolean          save_custom)
{
	BonoboUINode *l;
	NodeInfo     *info;

	if (!node)
		return;

	for (l = bonobo_ui_node_children (node); l;
             l = bonobo_ui_node_next (l))
		prune_widget_info (priv, l, save_custom);

	info = bonobo_ui_xml_get_data (priv->tree, node);

	if (info->widget) {
		gboolean save;
		
		save = NODE_IS_CUSTOM_WIDGET (info) && save_custom;

		if (!NODE_IS_ROOT_WIDGET (info) && !save) {
			GtkWidget *item = get_item_widget (info->widget);

#ifdef XML_MERGE_DEBUG
			printf ("Destroy widget '%s' '%p'\n",
				bonobo_ui_xml_make_path (node), item);
#endif

			gtk_widget_destroy (item);
		} else {
			if (save)
				custom_widget_unparent (info);
/*			printf ("leave widget '%s'\n",
			bonobo_ui_xml_make_path (node));*/
		}

		if (!save)
			info->widget = NULL;
	}
}


static void
override_fn (GtkObject *object, BonoboUINode *node, BonoboWindowPrivate *priv)
{
	char     *id = node_get_id_or_path (node);

#ifdef XML_MERGE_DEBUG
	fprintf (stderr, "Override '%s'\n", 
		 bonobo_ui_xml_make_path (node));
#endif

	prune_widget_info (priv, node, TRUE);

	g_free (id);
}

static void
reinstate_fn (GtkObject *object, BonoboUINode *node, BonoboWindowPrivate *priv)
{
	char     *id = node_get_id_or_path (node);

#ifdef XML_MERGE_DEBUG
	fprintf (stderr, "Reinstate '%s'\n", 
		 bonobo_ui_xml_make_path (node));
#endif

	prune_widget_info (priv, node, TRUE);

	g_free (id);
}

static void
rename_fn (GtkObject *object, BonoboUINode *node, BonoboWindowPrivate *priv)
{
#ifdef XML_MERGE_DEBUG
	fprintf (stderr, "Rename '%s'\n", 
		 bonobo_ui_xml_make_path (node));
#endif
}

static void
remove_fn (GtkObject *object, BonoboUINode *node, BonoboWindowPrivate *priv)
{
	char     *id = node_get_id_or_path (node);

#ifdef XML_MERGE_DEBUG
	fprintf (stderr, "Remove on '%s'\n",
		 bonobo_ui_xml_make_path (node));
#endif

	prune_widget_info (priv, node, FALSE);

	if (bonobo_ui_node_has_name (node, "dockitem")) {
		char *name = bonobo_ui_node_get_attr (node, "name");

		if (name) {
			GnomeDockItem *item;

			item = get_dock_item (priv, name);
			if (item)
				gtk_widget_destroy (GTK_WIDGET (item));
		}

		bonobo_ui_node_free_string (name);
	} else if (bonobo_ui_node_has_name (node, "menu") &&
		   bonobo_ui_node_parent (node) == priv->tree->root) {
/* Makes evolution look like a dog 
   gtk_widget_hide (GTK_WIDGET (priv->menu_item)); */
	}

	g_free (id);
}

/*
 * Doesn't the GtkRadioMenuItem API suck badly !
 */
#define MAGIC_RADIO_GROUP_KEY "Bonobo::RadioGroupName"

static void
radio_group_remove (GtkRadioMenuItem *menuitem,
		    char             *group_name)
{
	GtkRadioMenuItem *master;
	char             *orig_key;
	GSList           *l;
	BonoboWindowPrivate *priv =
		gtk_object_get_data (GTK_OBJECT (menuitem),
				     MAGIC_RADIO_GROUP_KEY);

	if (!g_hash_table_lookup_extended
	    (priv->radio_groups, group_name, (gpointer *)&orig_key,
	     (gpointer *)&master)) {
		g_warning ("Radio group hash inconsistancy");
		return;
	}
	
	l = master->group;
	while (l && l->data == menuitem)
		l = l->next;
	
	g_hash_table_remove (priv->radio_groups, group_name);
	g_free (orig_key);

	if (l) { /* Entries left in group */
		g_hash_table_insert (priv->radio_groups,
				     group_name, l->data);
	} else /* alloced in signal_connect; grim hey */
		g_free (group_name);
}

static void
radio_group_add (BonoboWindowPrivate *priv,
		 GtkRadioMenuItem *menuitem,
		 const char       *group_name)
{
	GtkRadioMenuItem *master;

	g_return_if_fail (priv != NULL);
	g_return_if_fail (menuitem != NULL);
	g_return_if_fail (group_name != NULL);

	if (!(master = g_hash_table_lookup (priv->radio_groups, group_name))) {
		g_hash_table_insert (priv->radio_groups, g_strdup (group_name),
				     menuitem);
	} else {
		gtk_radio_menu_item_set_group (
			menuitem, gtk_radio_menu_item_group (master));
		
		/* Since we created this item without a group, it's
		 * active, but now we are adding it to a group so it
		 * should not be active.
		 */
		GTK_CHECK_MENU_ITEM (menuitem)->active = FALSE;
	}

	gtk_object_set_data (GTK_OBJECT (menuitem),
			     MAGIC_RADIO_GROUP_KEY, priv);

	gtk_signal_connect (GTK_OBJECT (menuitem), "destroy",
			    (GtkSignalFunc) radio_group_remove,
			    g_strdup (group_name));
}

static void
real_exec_verb (BonoboWindowPrivate *priv,
		const char       *component_name,
		const char       *verb)
{
	Bonobo_UIComponent component;

	g_return_if_fail (priv != NULL);
	g_return_if_fail (verb != NULL);
	g_return_if_fail (component_name != NULL);

	gtk_object_ref (GTK_OBJECT (priv->win));

	component = win_component_objref (priv, component_name);

	if (component != CORBA_OBJECT_NIL) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);

		Bonobo_UIComponent_execVerb (
			component, verb, &ev);

		if (priv->container)
			bonobo_object_check_env (
				priv->container, component, &ev);

		if (BONOBO_EX (&ev))
			g_warning ("Exception executing verb '%s' '%s'"
				   "major %d, %s",
				   verb, component_name, ev._major, ev._repo_id);
		
		CORBA_exception_free (&ev);
	} else
		g_warning ("NULL Corba handle of name '%s'", component_name);

	gtk_object_unref (GTK_OBJECT (priv->win));
}

static gint
exec_verb_cb (GtkWidget *item, BonoboWindowPrivate  *priv)
{
	BonoboUINode      *node = widget_get_node (GTK_WIDGET (item));
	CORBA_char        *verb;
	BonoboUIXmlData   *data;
	
	g_return_val_if_fail (node != NULL, FALSE);

	data = bonobo_ui_xml_get_data (NULL, node);
	g_return_val_if_fail (data != NULL, FALSE);

	verb = node_get_id (node);
	if (!verb)
		return FALSE;

	if (!data->id) {
		g_warning ("Weird; no ID on verb '%s'", verb);
		bonobo_ui_node_free_string (verb);
		return FALSE;
	}

	real_exec_verb (priv, data->id, verb);

	g_free (verb);

	return FALSE;
}

static gint
menu_toggle_emit_ui_event (GtkCheckMenuItem *item, BonoboWindowPrivate *priv)
{
	BonoboUINode     *node = widget_get_node (GTK_WIDGET (item));
	char             *id, *state;
	BonoboUIXmlData  *data;
	char             *component_id;

	g_return_val_if_fail (node != NULL, FALSE);

	if (!(id = node_get_id (node)))
		return FALSE;

	data = bonobo_ui_xml_get_data (NULL, node);
	g_return_val_if_fail (data != NULL, FALSE);

	if (item->active)
		state = "1";
	else
		state = "0";

	component_id = g_strdup (data->id);

	/* This could invoke a CORBA method that might de-register the component */
	set_cmd_attr (priv, node, "state", state, TRUE);

	real_emit_ui_event (priv, component_id, id,
			    Bonobo_UIComponent_STATE_CHANGED,
			    state);

	g_free (component_id);
	g_free (id);

	return FALSE;
}

static gint
win_item_emit_ui_event (BonoboUIToolbarItem *item,
			const char          *state,
			BonoboWindowPrivate *priv)
{
	BonoboUINode     *node = widget_get_node (GTK_WIDGET (item));
	char             *id;
	BonoboUIXmlData  *data;
	char             *component_id;

	g_return_val_if_fail (node != NULL, FALSE);

	if (!(id = node_get_id (node)))
		return FALSE;

	data = bonobo_ui_xml_get_data (NULL, node);
	g_return_val_if_fail (data != NULL, FALSE);

	component_id = g_strdup (data->id);

	/* This could invoke a CORBA method that might de-register the component */
	set_cmd_attr (priv, node, "state", state, TRUE);

	real_emit_ui_event (priv, component_id, id,
			    Bonobo_UIComponent_STATE_CHANGED,
			    state);

	g_free (component_id);
	g_free (id);

	return FALSE;
}

static void
put_hint_in_statusbar (GtkWidget *menuitem, BonoboWindowPrivate *priv)
{
	BonoboUINode *node = widget_get_node (menuitem);
	BonoboUINode *cmd_node = cmd_get_node (priv, node);
	char *hint, *txt;
	gboolean err;

	g_return_if_fail (priv != NULL);
	g_return_if_fail (node != NULL);

	hint = cmd_get_attr (node, cmd_node, "tip");

/*	g_warning ("Getting tooltip on '%s', '%s' : '%s'",
		   bonobo_ui_xml_make_path (node),
		   cmd_node ? bonobo_ui_xml_make_path (cmd_node) : "no cmd",
		   hint);*/
	if (!hint)
		return;

	txt = bonobo_ui_util_decode_str (hint, &err);
	if (err) {
		g_warning ("Encoding error in tip on '%s', you probably forgot to "
			   "put an '_' before tip in your xml file",
			   bonobo_ui_xml_make_path (node));
	} else if (priv->main_status) {
		guint id;

		id = gtk_statusbar_get_context_id (priv->main_status,
						  "BonoboWindow:menu-hint");
		gtk_statusbar_push (priv->main_status, id, txt);
	}

	g_free (txt);
	bonobo_ui_node_free_string (hint);
}

static void
remove_hint_from_statusbar (GtkWidget *menuitem, BonoboWindowPrivate *priv)
{
	BonoboUINode *node = widget_get_node (menuitem);

	g_return_if_fail (priv != NULL);
	g_return_if_fail (node != NULL);

	if (priv->main_status) {
		guint id;

		id = gtk_statusbar_get_context_id (priv->main_status,
						  "BonoboWindow:menu-hint");
		gtk_statusbar_pop (priv->main_status, id);
	}
}

/*
 * Insert slightly cleverly.
 *  NB. it is no use inserting into the default placeholder here
 *  since this will screw up path addressing and subsequent merging.
 */
static void
add_node_fn (BonoboUINode *parent, BonoboUINode *child)
{
	BonoboUINode *insert = parent;
	char    *pos;
/*	BonoboUINode *l;
	if (!bonobo_ui_node_get_attr (child, "noplace"))
		for (l = bonobo_ui_node_children (parent); l; l = l->next) {
			if (!strcmp (l->name, "placeholder") &&
			    !bonobo_ui_node_get_attr (l, "name")) {
			    insert = l;
				g_warning ("Found default placeholder");
			}
		}*/

	if (bonobo_ui_node_children (insert) &&
	    (pos = bonobo_ui_node_get_attr (child, "pos"))) {
		if (!strcmp (pos, "top")) {
			g_warning ("TESTME: unused code branch");
                        bonobo_ui_node_insert_before (bonobo_ui_node_children (insert),
                                                      child);
		} else
			bonobo_ui_node_add_child (insert, child);
		bonobo_ui_node_free_string (pos);
	} else /* just add to bottom */
		bonobo_ui_node_add_child (insert, child);
}

static void
debug_reparents (BonoboControl *control, const char *str)
{
	g_warning ("Control '%s' unmapped\n", str);
}

static GtkWidget *
build_control (BonoboWindowPrivate *priv,
	       BonoboUINode     *node)
{
	GtkWidget *control = NULL;
	NodeInfo  *info = bonobo_ui_xml_get_data (priv->tree, node);

/*	fprintf (stderr, "Control '%p', type '%d' object '%p'\n",
	info->widget, info->type, info->object);*/

	if (info->widget) {
		control = info->widget;
		g_assert (info->widget->parent == NULL);
	} else if (info->object != CORBA_OBJECT_NIL) {

		control = bonobo_widget_new_control_from_objref
			(bonobo_object_dup_ref (info->object, NULL),
			 CORBA_OBJECT_NIL);
		g_return_val_if_fail (control != NULL, NULL);
		
		info->type |= CUSTOM_WIDGET;
	}

	if (info->widget)
		gtk_signal_connect (GTK_OBJECT (info->widget), "unmap",
				    (GtkSignalFunc) debug_reparents,
				    bonobo_ui_xml_make_path (node));

	do_show_hide (control, node);

/*	fprintf (stderr, "Type on '%s' '%s' is %d widget %p\n",
		 bonobo_ui_node_get_name (node),
		 bonobo_ui_node_get_attr (node, "name"),
		 info->type, info->widget);*/

	return control;
}

static GtkWidget *
menu_build_item (BonoboWindowPrivate *priv,
		 BonoboUINode     *node,
		 int              *pos,
		 NodeInfo         *info,
		 GtkWidget        *parent)
{
	GtkWidget    *menu_widget = NULL;
	GtkWidget    *ret_widget;
	BonoboUINode *cmd_node;
	char         *type;

	cmd_node = cmd_get_node (priv, node);

	if (bonobo_ui_node_has_name (node, "separator")) {
		menu_widget = gtk_menu_item_new ();
		gtk_widget_set_sensitive (menu_widget, FALSE);

	} else if (bonobo_ui_node_has_name (node, "control")) {
		GtkWidget *control = build_control (priv, node);
		if (!control)
			return NULL;
		else if (!GTK_IS_MENU_ITEM (control)) {
			menu_widget = gtk_menu_item_new ();
			gtk_container_add (GTK_CONTAINER (menu_widget), control);
		} else
			menu_widget = control;

	} else if (bonobo_ui_node_has_name (node, "menuitem") ||
		   bonobo_ui_node_has_name (node, "submenu")) {

		/* Create menu item */
		if ((type = cmd_get_attr (node, cmd_node, "type"))) {
			if (!strcmp (type, "radio")) {
				char *group = cmd_get_attr (node, cmd_node, "group");

				menu_widget = gtk_radio_menu_item_new (NULL);

				if (group)
					radio_group_add (
						priv,
						GTK_RADIO_MENU_ITEM (menu_widget),
						group);

				bonobo_ui_node_free_string (group);
			} else if (!strcmp (type, "toggle"))
				menu_widget = gtk_check_menu_item_new ();
			
			else
				menu_widget = NULL;
			
			gtk_check_menu_item_set_show_toggle (
				GTK_CHECK_MENU_ITEM (menu_widget), TRUE);

			gtk_signal_connect (GTK_OBJECT (menu_widget), "toggled",
					    (GtkSignalFunc) menu_toggle_emit_ui_event,
					    priv);

			bonobo_ui_node_free_string (type);
		} else {
			char *txt;
			
			/* FIXME: why not always create pixmap menu items ? */
			if ((txt = cmd_get_attr (node, cmd_node, "pixtype")))
				menu_widget = gtk_pixmap_menu_item_new ();
			else
				menu_widget = gtk_menu_item_new ();

			bonobo_ui_node_free_string (txt);
		}

		if (!menu_widget)
			return NULL;
			
		gtk_signal_connect (GTK_OBJECT (menu_widget),
				    "select",
				    GTK_SIGNAL_FUNC (put_hint_in_statusbar),
				    priv);
		
		gtk_signal_connect (GTK_OBJECT (menu_widget),
				    "deselect",
				    GTK_SIGNAL_FUNC (remove_hint_from_statusbar),
				    priv);
	}

	if (!menu_widget)
		return NULL;	
	   
	if (bonobo_ui_node_has_name (node, "submenu")) {
		GtkMenuShell *shell;
		GtkMenu      *menu;
		GtkWidget    *tearoff;
		
		shell = GTK_MENU_SHELL (parent);

		/* Create the menu shell. */
		menu = GTK_MENU (gtk_menu_new ());

		gtk_menu_set_accel_group (menu, priv->accel_group);

		/*
		 * Create the tearoff item at the beginning of the menu shell,
		 * if appropriate.
		 */
		if (gnome_preferences_get_menus_have_tearoff ()) {
			tearoff = gtk_tearoff_menu_item_new ();
			gtk_widget_show (tearoff);
			gtk_menu_prepend (GTK_MENU (menu), tearoff);
		}

		/*
		 * Associate this menu shell with the menu item for
		 * this submenu.
		 */
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_widget),
					   GTK_WIDGET (menu));

		/* We don't recurse here, it is done once in the state set */

		gtk_widget_show (GTK_WIDGET (menu));
		gtk_widget_show (GTK_WIDGET (shell));

		ret_widget = GTK_WIDGET (menu);
	} else
		ret_widget = menu_widget;

	if (!GTK_IS_CHECK_MENU_ITEM (menu_widget))
		gtk_signal_connect (GTK_OBJECT (menu_widget), "activate",
				    (GtkSignalFunc) exec_verb_cb, priv);

	gtk_widget_show (menu_widget);
	gtk_menu_shell_insert (GTK_MENU_SHELL (parent),
			       menu_widget, (*pos)++);

	return ret_widget;
}

static void update_menus (BonoboWindowPrivate *priv, BonoboUINode *node);

static void
menu_sync_state (BonoboWindowPrivate *priv, BonoboUINode *node,
		 GtkWidget *widget, GtkWidget *parent)
{
	NodeInfo     *info;
	GtkWidget    *menu_widget;
	BonoboUINode *cmd_node;
	char         *sensitive = NULL, *hidden = NULL, *state = NULL;
	char         *type, *txt, *label_attr;
	static int    warned = 0;

	info = bonobo_ui_xml_get_data (priv->tree, node);
	g_return_if_fail (parent != NULL);

	cmd_node = cmd_get_node (priv, node);

	if ((hidden    = bonobo_ui_node_get_attr (node, "hidden")) ||
	    (sensitive = bonobo_ui_node_get_attr (node, "sensitive")) ||
	    (state     = bonobo_ui_node_get_attr (node, "state"))) {
		if (cmd_node) {
			if (!warned++) {
				char *txt;
				g_warning ("FIXME: We have an attribute '%s' at '%s' breaking "
					   "cmd/widget separation, please fix",
					   hidden?"hidden":((sensitive)?"sensitive":((state)?"state":"error")),
					   (txt = bonobo_ui_xml_make_path (node)));
				g_free (txt);
			}
			if (hidden)
				set_cmd_attr (priv, node, "hidden", hidden, FALSE);
			if (sensitive)
				set_cmd_attr (priv, node, "sensitive", sensitive, FALSE);
			if (state && (txt = bonobo_ui_node_get_attr (node, "type"))) {
				set_cmd_attr (priv, node, "state", state, FALSE);
				bonobo_ui_node_free_string (txt);
			}
		}
	}
	bonobo_ui_node_free_string (state);
	bonobo_ui_node_free_string (sensitive);
	bonobo_ui_node_free_string (hidden);

	if (bonobo_ui_node_has_name (node, "placeholder") ||
	    bonobo_ui_node_has_name (node, "separator")) {
		
		widget_set_state (widget, node);
		return;
	}

	if (bonobo_ui_node_has_name (node, "submenu")) {
		menu_widget = get_item_widget (widget);

		/* Recurse here just once, don't duplicate in the build. */
		update_menus (priv, node);

	} else if (bonobo_ui_node_has_name (node, "menuitem"))
		menu_widget = widget;
	else
		return;

	if ((type = cmd_get_attr (node, cmd_node, "type")))
		bonobo_ui_node_free_string (type);
	else {
		if (GTK_IS_PIXMAP_MENU_ITEM (menu_widget)) {
			GtkWidget *pixmap;
			GtkPixmapMenuItem *gack = GTK_PIXMAP_MENU_ITEM (menu_widget);

			pixmap = cmd_get_menu_pixmap (node, cmd_node);

			if (pixmap) {
				/* Since this widget sucks we must claw inside its guts */
				if (gack->pixmap) {
					gtk_widget_destroy (gack->pixmap);
					gack->pixmap = NULL;
				}
				gtk_widget_show (GTK_WIDGET (pixmap));
				gtk_pixmap_menu_item_set_pixmap (
					GTK_PIXMAP_MENU_ITEM (menu_widget),
					GTK_WIDGET (pixmap));
			}
		}
	}

	if ((label_attr = cmd_get_attr (node, cmd_node, "label"))) {
		GtkWidget *label;
		guint      keyval;
		gboolean   err;

		txt = bonobo_ui_util_decode_str (label_attr, &err);
		if (err) {
			g_warning ("Encoding error in label on '%s', you probably forgot to "
				   "put an '_' before label in your xml file",
				   bonobo_ui_xml_make_path (node));
			return;
		}

		label = gtk_accel_label_new (txt);

		/*
		 * Setup the widget.
		 */
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_widget_show (label);
		
		/*
		 * Insert it into the menu item widget and setup the
		 * accelerator. FIXME: rather inefficient.
		 */
		if (GTK_BIN (menu_widget)->child)
			gtk_widget_destroy (GTK_BIN (menu_widget)->child);

		gtk_container_add (GTK_CONTAINER (menu_widget), label);
		gtk_accel_label_set_accel_widget (
			GTK_ACCEL_LABEL (label), menu_widget);
	
		keyval = gtk_label_parse_uline (GTK_LABEL (label), txt);

		bonobo_ui_node_free_string (label_attr);
		g_free (txt);

		if (keyval != GDK_VoidSymbol) {
			if (GTK_IS_MENU (parent))
				gtk_widget_add_accelerator (
					menu_widget, "activate_item",
					gtk_menu_ensure_uline_accel_group (
						GTK_MENU (parent)),
					keyval, 0, 0);

			else if (GTK_IS_MENU_BAR (parent) &&
				 priv->accel_group != NULL)
				gtk_widget_add_accelerator (
					menu_widget, "activate_item",
					priv->accel_group,
					keyval, GDK_MOD1_MASK, 0);
			else
				g_warning ("Adding accelerator went bananas");
		}
	}
	
	if ((txt = cmd_get_attr (node, cmd_node, "accel"))) {
		guint           key;
		GdkModifierType mods;
		char           *signal;

/*		fprintf (stderr, "Accel name is afterwards '%s'\n", text); */
		bonobo_ui_util_accel_parse (txt, &key, &mods);
		bonobo_ui_node_free_string (txt);

		if (!key)
			return;

/*		if (GTK_IS_CHECK_MENU_ITEM (menu_widget))
			signal = "toggled";
			else*/
		signal = "activate";

		gtk_widget_add_accelerator (menu_widget,
					    signal,
					    priv->accel_group,
					    key, mods,
					    GTK_ACCEL_VISIBLE);
	}

	widget_queue_state (priv, menu_widget, cmd_node != NULL ? cmd_node : node);
}

static GtkWidget *
menu_build_placeholder (BonoboWindowPrivate *priv,
			BonoboUINode     *node,
			int              *pos,
			NodeInfo         *info,
			GtkWidget        *parent)
{
	GtkWidget *widget;

	widget = gtk_menu_item_new ();
	gtk_widget_set_sensitive (widget, FALSE);

	gtk_menu_shell_insert (GTK_MENU_SHELL (parent),
			       GTK_WIDGET (widget), (*pos)++);

	return widget;
}

static void
update_menus (BonoboWindowPrivate *priv, BonoboUINode *node)
{
	int       pos;
	GList    *widgets, *wptr;
	NodeInfo *info;

	info = bonobo_ui_xml_get_data (priv->tree, node);

	wptr = widgets = gtk_container_children (GTK_CONTAINER (info->widget));
	pos = 0;
	sync_generic_widgets (priv, bonobo_ui_node_children (node),
			      info->widget, &wptr, &pos,
			      menu_sync_state,
			      menu_build_item,
			      menu_build_placeholder);
	check_excess_widgets (priv, wptr);

	if (bonobo_ui_node_parent (node) == priv->tree->root)
		do_show_hide (info->widget, node);
	else
		do_show_hide (info->widget, node);

	g_list_free  (widgets);
}

static gboolean 
string_array_contains (char **str_array, const char *match)
{
	int i = 0;
	char *string;

	while ((string = str_array [i++]))
		if (strcmp (string, match) == 0)
			return TRUE;

	return FALSE;
}

static GnomeDockItem *
create_dockitem (BonoboWindowPrivate *priv,
		 BonoboUINode     *node,
		 const char       *dockname)
{
	GnomeDockItem *item;
	GnomeDockItemBehavior beh = 0;
	char *prop;
	char **behavior_array;
	gboolean force_detachable = FALSE;
	GnomeDockPlacement placement = GNOME_DOCK_TOP;
	gint band_num = 1;
	gint position = 0;
	guint offset = 0;
	gboolean in_new_band = TRUE;

	if ((prop = bonobo_ui_node_get_attr (node, "behavior"))) {
		if (!strcmp (prop, "detachable"))
			force_detachable = TRUE;
		bonobo_ui_node_free_string (prop);
	}

	if ((prop = bonobo_ui_node_get_attr (node, "behavior"))) {
		behavior_array = g_strsplit (prop, ",", -1);
		bonobo_ui_node_free_string (prop);
	
		if (string_array_contains (behavior_array, "detachable"))
			force_detachable = TRUE;

		if (string_array_contains (behavior_array, "exclusive"))
			beh |= GNOME_DOCK_ITEM_BEH_EXCLUSIVE;

		if (string_array_contains (behavior_array, "never vertical"))
			beh |= GNOME_DOCK_ITEM_BEH_NEVER_VERTICAL;

		if (string_array_contains (behavior_array, "never floating"))
			beh |= GNOME_DOCK_ITEM_BEH_NEVER_FLOATING;

		if (string_array_contains (behavior_array, "never horizontal"))
			beh |= GNOME_DOCK_ITEM_BEH_NEVER_HORIZONTAL;

		g_strfreev (behavior_array);
	}

	if (!force_detachable && !gnome_preferences_get_toolbar_detachable())
		beh |= GNOME_DOCK_ITEM_BEH_LOCKED;

	item = GNOME_DOCK_ITEM (gnome_dock_item_new (
		dockname, beh));

	gtk_container_set_border_width (GTK_CONTAINER (item), 2);

	if ((prop = bonobo_ui_node_get_attr (node, "placement"))) {
		if (!strcmp (prop, "top"))
			placement = GNOME_DOCK_TOP;
		else if (!strcmp (prop, "right"))
			placement = GNOME_DOCK_RIGHT;
		else if (!strcmp (prop, "bottom"))
			placement = GNOME_DOCK_BOTTOM;
		else if (!strcmp (prop, "left"))
			placement = GNOME_DOCK_LEFT;
		else if (!strcmp (prop, "floating"))
			placement = GNOME_DOCK_FLOATING;
		bonobo_ui_node_free_string (prop);
	}

	if ((prop = bonobo_ui_node_get_attr (node, "band_num"))) {
		band_num = atoi (prop);
		bonobo_ui_node_free_string (prop);
	}

	if ((prop = bonobo_ui_node_get_attr (node, "position"))) {
		position = atoi (prop);
		bonobo_ui_node_free_string (prop);
	}

	if ((prop = bonobo_ui_node_get_attr (node, "offset"))) {
		offset = atoi (prop);
		bonobo_ui_node_free_string (prop);
	}

	if ((prop = bonobo_ui_node_get_attr (node, "in_new_band"))) {
		in_new_band = atoi (prop);
		bonobo_ui_node_free_string (prop);
	}	

	gnome_dock_add_item (priv->dock, item,
			     placement, band_num,
			     position, offset, in_new_band);

	return item;
}

static GtkWidget *
toolbar_build_control (BonoboWindowPrivate *priv,
		       BonoboUINode     *node,
		       int              *pos,
		       NodeInfo         *info,
		       GtkWidget        *parent)
{
	GtkWidget  *item;
	
	g_return_val_if_fail (priv != NULL, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	if (info->widget) {
		item = info->widget;
		g_assert (info->widget->parent == NULL);

	} else if (info->object != CORBA_OBJECT_NIL) {
		item = bonobo_ui_toolbar_control_item_new (
			bonobo_object_dup_ref (info->object, NULL));
		if (!item) {
			return NULL;
		}

		info->type |= CUSTOM_WIDGET;
	} else {
		return NULL;
	}

	gtk_widget_show (item);

	bonobo_ui_toolbar_insert (BONOBO_UI_TOOLBAR (parent),
				  BONOBO_UI_TOOLBAR_ITEM (item),
				  (*pos)++);

	return item;
}

static GtkWidget *
toolbar_build_widget (BonoboWindowPrivate *priv,
		      BonoboUINode     *node,
		      int              *pos,
		      NodeInfo         *info,
		      GtkWidget        *parent)
{
	char         *type;
	GtkWidget    *item;
	BonoboUINode *cmd_node;

	g_return_val_if_fail (priv != NULL, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	cmd_node = cmd_get_node (priv, node);

	type = cmd_get_attr (node, cmd_node, "type");

	if (bonobo_ui_node_has_name (node, "separator")) {
		item = bonobo_ui_toolbar_separator_item_new ();
		gtk_widget_set_sensitive (item, FALSE);

	} else if (!type)
		item = bonobo_ui_toolbar_button_item_new (NULL, NULL);
	
	else if (!strcmp (type, "toggle"))
		item = bonobo_ui_toolbar_toggle_button_item_new (NULL, NULL);
	
	else {
		/* FIXME: Implement radio-toolbars */
		g_warning ("Invalid type '%s'", type);
		return NULL;
	}

	bonobo_ui_node_free_string (type);
	
	bonobo_ui_toolbar_insert (BONOBO_UI_TOOLBAR (parent),
				  BONOBO_UI_TOOLBAR_ITEM (item),
				  (*pos)++);
	gtk_widget_show (item);

	return item;
}

static GtkWidget *
toolbar_build_item (BonoboWindowPrivate *priv,
		    BonoboUINode     *node,
		    int              *pos,
		    NodeInfo         *info,
		    GtkWidget        *parent)
{
	GtkWidget *widget;
	char      *verb;
	
	if (bonobo_ui_node_has_name (node, "control"))
		widget = toolbar_build_control (priv, node, pos, info, parent);
	else
		widget = toolbar_build_widget (priv, node, pos, info, parent);

	if (widget) {
		/* FIXME: What about "id"s ! ? */
		if ((verb = bonobo_ui_node_get_attr (node, "verb"))) {
			gtk_signal_connect (GTK_OBJECT (widget), "activate",
					    (GtkSignalFunc) exec_verb_cb, priv);
			bonobo_ui_node_free_string (verb);
		}
		
		gtk_signal_connect (GTK_OBJECT (widget), "state_altered",
				    (GtkSignalFunc) win_item_emit_ui_event, priv);
	}

	return widget;
}

static GtkWidget *
toolbar_build_placeholder (BonoboWindowPrivate *priv,
			   BonoboUINode     *node,
			   int              *pos,
			   NodeInfo         *info,
			   GtkWidget        *parent)
{
	GtkWidget *widget;

	widget = bonobo_ui_toolbar_separator_item_new ();
	gtk_widget_set_sensitive (widget, FALSE);

	bonobo_ui_toolbar_insert (BONOBO_UI_TOOLBAR (parent),
				  BONOBO_UI_TOOLBAR_ITEM (widget),
				  (*pos)++);

	return widget;
}

static BonoboUIToolbarControlDisplay
decode_control_disp (const char *txt)
{
	if (!txt || !strcmp (txt, "control"))
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL;

	else if (!strcmp (txt, "button"))
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_BUTTON;

	else if (!strcmp (txt, "none"))
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE;

	else
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL;
}

static void
toolbar_sync_state (BonoboWindowPrivate *priv, BonoboUINode *node,
		    GtkWidget *widget, GtkWidget *parent)
{
	char *type, *sensitive = NULL, *state = NULL, *label, *txt, *hidden = NULL;
	char *min_width;
	char *behavior;
	char **behavior_array;
	GdkPixbuf    *icon_pixbuf;
	BonoboUINode *cmd_node;
	static int    warned = 0;

	cmd_node = cmd_get_node (priv, node);

	/*FIXME: to debug control problem */
	gtk_widget_show (widget);

	if ((hidden    = bonobo_ui_node_get_attr (node, "hidden")) ||
	    (sensitive = bonobo_ui_node_get_attr (node, "sensitive")) ||
	    (state     = bonobo_ui_node_get_attr (node, "state"))) {
		if (cmd_node) {
			if (!warned++) {
				g_warning ("FIXME: We have an attribute '%s' at '%s' breaking "
					   "cmd/widget separation, please fix",
					   hidden?"hidden":((sensitive)?"sensitive":((state)?"state":"error")),
					   bonobo_ui_xml_make_path (node));
			}
			if (hidden)
				set_cmd_attr (priv, node, "hidden", hidden, FALSE);
			if (sensitive)
				set_cmd_attr (priv, node, "sensitive", sensitive, FALSE);
			if (state)
				set_cmd_attr (priv, node, "state", state, FALSE);
		}
	}
	bonobo_ui_node_free_string (state);
	bonobo_ui_node_free_string (sensitive);
	bonobo_ui_node_free_string (hidden);

	if ((behavior = cmd_get_attr (node, cmd_node, "behavior"))) {
		
		behavior_array = g_strsplit (behavior, ",", -1);
		bonobo_ui_node_free_string (behavior);

		bonobo_ui_toolbar_item_set_expandable (
			BONOBO_UI_TOOLBAR_ITEM (widget),
			string_array_contains (behavior_array, "expandable"));

		bonobo_ui_toolbar_item_set_pack_end (
			BONOBO_UI_TOOLBAR_ITEM (widget),
			string_array_contains (behavior_array, "pack-end"));

		g_strfreev (behavior_array);
	}

	icon_pixbuf = cmd_get_toolbar_pixbuf (node, cmd_node);

	type = cmd_get_attr (node, cmd_node, "type");
	label = cmd_get_attr (node, cmd_node, "label");
	
	if (!type || !strcmp (type, "toggle")) {

		if (icon_pixbuf) {
			bonobo_ui_toolbar_button_item_set_icon (
				BONOBO_UI_TOOLBAR_BUTTON_ITEM (widget), icon_pixbuf);
			gdk_pixbuf_unref (icon_pixbuf);
		}

		if (label) {
			gboolean err;
			char *txt = bonobo_ui_util_decode_str (label, &err);
			if (err) {
				g_warning ("Encoding error in label on '%s', you probably forgot to "
					   "put an '_' before label in your xml file",
					   bonobo_ui_xml_make_path (node));
				return;
			}

			bonobo_ui_toolbar_button_item_set_label (
				BONOBO_UI_TOOLBAR_BUTTON_ITEM (widget), txt);

			g_free (txt);
		}
	}

	bonobo_ui_node_free_string (type);
	bonobo_ui_node_free_string (label);

	if (bonobo_ui_node_has_name (node, "control")) {
		char *txt;
		BonoboUIToolbarControlDisplay hdisp, vdisp;
		
		txt = bonobo_ui_node_get_attr (node, "hdisplay");
		hdisp = decode_control_disp (txt);
		bonobo_ui_node_free_string (txt);

		txt = bonobo_ui_node_get_attr (node, "vdisplay");
		vdisp = decode_control_disp (txt);
		bonobo_ui_node_free_string (txt);

		bonobo_ui_toolbar_control_item_set_display (
			BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget), hdisp, vdisp);
	}

	if ((min_width = cmd_get_attr (node, cmd_node, "min_width"))) {
		bonobo_ui_toolbar_item_set_minimum_width (BONOBO_UI_TOOLBAR_ITEM (widget),
							  atoi (min_width));
		bonobo_ui_node_free_string (min_width);
	}
	
	if ((txt = cmd_get_attr (node, cmd_node, "tip"))) {
		gboolean err;
		char *decoded_txt;

		decoded_txt = bonobo_ui_util_decode_str (txt, &err);
		if (err) {
			g_warning ("Encoding error in tip on '%s', you probably forgot to "
				   "put an '_' before tip in your xml file",
				   bonobo_ui_xml_make_path (node));
		} else {
			bonobo_ui_toolbar_item_set_tooltip (
				BONOBO_UI_TOOLBAR_ITEM (widget),
				bonobo_ui_toolbar_get_tooltips (
					BONOBO_UI_TOOLBAR (parent)), decoded_txt);
		}

		g_free (decoded_txt);
		bonobo_ui_node_free_string (txt);
	}

	widget_queue_state (priv, widget, cmd_node != NULL ? cmd_node : node);
}

static BonoboUIToolbarStyle
parse_look (const char *look)
{
	if (look) {
		if (!strcmp (look, "both"))
			return BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT;

		if (!strcmp (look, "icon"))
			return BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY;

		if (!strcmp (look, "text"))
			return BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT;	
	}

	return gnome_preferences_get_toolbar_labels ()
		? BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT
		: BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY;
}

static void
update_dockitem (BonoboWindowPrivate *priv, BonoboUINode *node)
{
	NodeInfo      *info = bonobo_ui_xml_get_data (priv->tree, node);
	char          *txt;
	char          *dockname = bonobo_ui_node_get_attr (node, "name");
	GnomeDockItem *item;
	BonoboUIToolbar *toolbar;
	GList *widgets, *wptr;
	BonoboUIToolbarStyle look;
	int    pos;

	item = get_dock_item (priv, dockname);
	
	if (!item) {
		item = create_dockitem (priv, node, dockname);
		
		toolbar = BONOBO_UI_TOOLBAR (bonobo_ui_toolbar_new ());

		gtk_container_add (GTK_CONTAINER (item),
				   GTK_WIDGET (toolbar));
		gtk_widget_show (GTK_WIDGET (toolbar));
	} else
		toolbar = BONOBO_UI_TOOLBAR (GTK_BIN (item)->child);

	info->type |= ROOT_WIDGET;
	info->widget = GTK_WIDGET (toolbar);

	/* Update the widgets */
/*	bonobo_window_dump (priv->win, "before build widgets");*/
	wptr = widgets = bonobo_ui_toolbar_get_children (toolbar);
	pos = 0;
	sync_generic_widgets (priv, bonobo_ui_node_children (node),
			      GTK_WIDGET (toolbar), &wptr, &pos,
			      toolbar_sync_state,
			      toolbar_build_item,
			      toolbar_build_placeholder);
	check_excess_widgets (priv, wptr);
	g_list_free  (widgets);
/*	bonobo_window_dump (priv->win, "after build widgets");*/

	/* Update the attributes */

	if ((txt = bonobo_ui_node_get_attr (node, "look"))) {
		look = parse_look (txt);
		bonobo_ui_toolbar_set_hv_styles (toolbar, look, look);
		bonobo_ui_node_free_string (txt);

	} else {
		BonoboUIToolbarStyle vlook, hlook;

		txt = bonobo_ui_node_get_attr (node, "hlook");
		hlook = parse_look (txt);
		bonobo_ui_node_free_string (txt);

		txt = bonobo_ui_node_get_attr (node, "vlook");
		vlook = parse_look (txt);
		bonobo_ui_node_free_string (txt);

		bonobo_ui_toolbar_set_hv_styles (toolbar, hlook, vlook);
	}		

#if 0
	if ((txt = bonobo_ui_node_get_attr (node, "relief"))) {

		if (!strcmp (txt, "normal"))
			bonobo_ui_toolbar_set_relief (
				toolbar, GTK_RELIEF_NORMAL);

		else if (!strcmp (txt, "half"))
			bonobo_ui_toolbar_set_relief (
				toolbar, GTK_RELIEF_HALF);
		else
			bonobo_ui_toolbar_set_relief (
				toolbar, GTK_RELIEF_NONE);
		bonobo_ui_node_free_string (txt);
	}
#endif

#if 0
	tooltips = TRUE;
	if ((txt = bonobo_ui_node_get_attr (node, "tips"))) {
		tooltips = atoi (txt);
		bonobo_ui_node_free_string (txt);
	}
	
	bonobo_ui_toolbar_set_tooltips (toolbar, tooltips);
#endif


       /*
	* FIXME: It shouldn't be necessary to explicitly resize the
	* dock, since resizing a widget is supposed to resize it's parent,
	* but the dock is not resized correctly on dockitem show / hides.
	*/
	if (do_show_hide (GTK_WIDGET (item), node))
		gtk_widget_queue_resize (GTK_WIDGET (priv->dock));

	bonobo_ui_node_free_string (dockname);
}

typedef struct {
	guint           key;
	GdkModifierType mods;
	BonoboUINode        *node;
} Binding;

static gboolean
keybindings_free (gpointer key,
		  gpointer value,
		  gpointer user_data)
{
	g_free (key);

	return TRUE;
}

/*
 * Shamelessly stolen from gtkbindings.c
 */
static guint
keybinding_hash_fn (gconstpointer  key)
{
  register const Binding *e = key;
  register guint h;

  h = e->key;
  h ^= e->mods;

  return h;
}

static gint
keybinding_compare_fn (gconstpointer a,
		       gconstpointer b)
{
	register const Binding *ba = a;
	register const Binding *bb = b;

	return (ba->key == bb->key && ba->mods == bb->mods);
}

static void
update_keybindings (BonoboWindowPrivate *priv, BonoboUINode *node)
{
	BonoboUINode    *l;
	BonoboUIXmlData *data;

	data = bonobo_ui_xml_get_data (priv->tree, node);
	if (!data->dirty)
		return;

	g_hash_table_foreach_remove (priv->keybindings, keybindings_free, NULL);

	for (l = bonobo_ui_node_children (node); l; l = bonobo_ui_node_next (l)) {
		guint           key;
		GdkModifierType mods;
		char           *name;
		Binding        *binding;
		
		name = bonobo_ui_node_get_attr (l, "name");
		if (!name)
			continue;
		
		bonobo_ui_util_accel_parse (name, &key, &mods);
		bonobo_ui_node_free_string (name);

		binding       = g_new0 (Binding, 1);
		binding->mods = mods & BINDING_MOD_MASK ();
		binding->key  = gdk_keyval_to_lower (key);
		binding->node = l;

		g_hash_table_insert (priv->keybindings, binding, binding);
	}

	bonobo_ui_xml_clean (priv->tree, node);
}

static void
status_sync_state (BonoboWindowPrivate *priv, BonoboUINode *node,
		   GtkWidget *widget, GtkWidget *parent)
{
	char *name;
		
	name = bonobo_ui_node_get_attr (node, "name");
	if (!name)
		return;

	if (!strcmp (name, "main")) {
		BonoboUIXmlData *data = bonobo_ui_xml_get_data (priv->tree, node);

		priv->main_status = GTK_STATUSBAR (widget);
			
		if (data->id) {
			guint id;
			char *txt;

			id = gtk_statusbar_get_context_id (
				priv->main_status, data->id);

			if ((txt = bonobo_ui_node_get_content (node))) {
				gboolean err;
				char    *status;

				status = bonobo_ui_util_decode_str (txt, &err);

				if (err)
					g_warning ("It looks like the status '%s' is not correctly "
						   "encoded, use bonobo_ui_component_set_status", txt);
				else
					gtk_statusbar_push (priv->main_status, id, status);

				g_free (status);
			} else
				gtk_statusbar_pop (priv->main_status, id);

			bonobo_ui_node_free_string (txt);
		}
	}

	bonobo_ui_node_free_string (name);
}

static void
main_status_null (GtkObject *dummy, BonoboWindowPrivate *priv)
{
	priv->main_status = NULL;
}

static GtkWidget *
status_build_item (BonoboWindowPrivate *priv,
		   BonoboUINode    *node,
		   int             *pos,
		   NodeInfo        *info,
		   GtkWidget       *parent)
{
	char *name;
	GtkWidget *widget = NULL;
		
	name = bonobo_ui_node_get_attr (node, "name");
	if (!name)
		return NULL;

	if (!strcmp (name, "main")) {
		widget = gtk_statusbar_new ();
		priv->main_status = GTK_STATUSBAR (widget);

		gtk_signal_connect (GTK_OBJECT (widget), "destroy",
				    (GtkSignalFunc) main_status_null, priv);

		/* insert a little padding so text isn't jammed against frame */
		gtk_misc_set_padding (
			GTK_MISC (GTK_STATUSBAR (widget)->label),
			GNOME_PAD, 0);
		gtk_widget_show (GTK_WIDGET (widget));

		gtk_box_pack_start (GTK_BOX (parent), widget, TRUE, TRUE, 0);
			
	} else if (bonobo_ui_node_has_name (node, "control")) {
		NodeInfo *info = bonobo_ui_xml_get_data (priv->tree, node);

		if (info->object == CORBA_OBJECT_NIL) {
			bonobo_ui_node_free_string (name);
			return NULL;
		}

		widget = build_control (priv, node);

		if (widget)
			gtk_box_pack_end (GTK_BOX (parent), widget,
					  FALSE, FALSE, 0);
	}
	bonobo_ui_node_free_string (name);

	if (widget)
		gtk_box_reorder_child (priv->status, widget, (*pos)++);

	return widget;
}

static GtkWidget *
status_build_placeholder (BonoboWindowPrivate *priv,
			  BonoboUINode    *node,
			  int             *pos,
			  NodeInfo        *info,
			  GtkWidget       *parent)
{
	GtkWidget *widget;

	g_warning ("TESTME: status bar placeholders");

	widget = bonobo_ui_toolbar_separator_item_new ();
	gtk_widget_set_sensitive (widget, FALSE);

	gtk_box_pack_end (GTK_BOX (parent), widget,
			  FALSE, FALSE, 0);

	if (widget)
		gtk_box_reorder_child (priv->status, widget, (*pos)++);

	return widget;
}

static GList *
box_get_children_in_order (GtkBox *box)
{
	GList       *ret = NULL;
	GList       *l;

	g_return_val_if_fail (GTK_IS_BOX (box), NULL);

	for (l = box->children; l; l = l->next) {
		GtkBoxChild *child = l->data;

		ret = g_list_prepend (ret, child->widget);
	}

	return g_list_reverse (ret);
}

static void
update_status (BonoboWindowPrivate *priv, BonoboUINode *node)
{
	GtkWidget       *item = GTK_WIDGET (priv->status);
	GList *widgets, *wptr;
	int pos;

	wptr = widgets = box_get_children_in_order (GTK_BOX (priv->status));
	pos = 0;
	sync_generic_widgets (priv, bonobo_ui_node_children (node),
			      GTK_WIDGET (priv->status), &wptr, &pos,
			      status_sync_state,
			      status_build_item,
			      status_build_placeholder);
	check_excess_widgets (priv, wptr);
	g_list_free  (widgets);

	do_show_hide (item, node);

	bonobo_ui_xml_clean (priv->tree, node);
}	

typedef enum {
	UI_UPDATE_MENU,
	UI_UPDATE_DOCKITEM
} UIUpdateType;

static void
seek_dirty (BonoboWindowPrivate *priv, BonoboUINode *node, UIUpdateType type)
{
	BonoboUIXmlData *info;

	if (!node)
		return;

	info = bonobo_ui_xml_get_data (priv->tree, node);
	if (info->dirty) { /* Rebuild tree from here down */
		
		switch (type) {
		case UI_UPDATE_MENU:
			update_menus (priv, node);
			break;
		case UI_UPDATE_DOCKITEM:
			update_dockitem (priv, node);
			break;
		default:
			g_warning ("Looking for unhandled super type");
			break;
		}

		bonobo_ui_xml_clean (priv->tree, node);

	} else {
		BonoboUINode *l;

		for (l = bonobo_ui_node_children (node); l;
		     l = bonobo_ui_node_next (l))
			seek_dirty (priv, l, type);
	}
}

static void
setup_root_widgets (BonoboWindowPrivate *priv)
{
	BonoboUINode  *node;
	NodeInfo *info;
	GSList   *l;

	if ((node = bonobo_ui_xml_get_path (
		priv->tree, "/menu"))) {
		info = bonobo_ui_xml_get_data (priv->tree, node);
		info->widget = GTK_WIDGET (priv->menu);
		info->type |= ROOT_WIDGET;
		do_show_hide (GTK_WIDGET (priv->menu_item), node);
	}

	for (l = priv->popups; l; l = l->next) {
		WinPopup *popup = l->data;

		if ((node = bonobo_ui_xml_get_path (priv->tree,
						    popup->path))) {
			info = bonobo_ui_xml_get_data (priv->tree, node);
			info->widget = GTK_WIDGET (popup->menu);
			info->type |= ROOT_WIDGET;
		} else
			g_warning ("Can't find path '%s' for popup widget",
				   popup->path);
	}
}

static void
dirty_by_cmd (BonoboWindowPrivate *priv,
	      BonoboUINode     *search,
	      const char       *search_id)
{
	BonoboUINode *l;
	char         *id = node_get_id (search);

	g_return_if_fail (search_id != NULL);

/*	printf ("Dirty node by cmd if %s == %s on node '%s'\n", search_id, id,
	bonobo_ui_xml_make_path (search));*/

	if (id && !strcmp (search_id, id)) /* Dirty it */
		bonobo_ui_xml_set_dirty (priv->tree, search);

	g_free (id);

	for (l = bonobo_ui_node_children (search); l;
             l = bonobo_ui_node_next (l))
		dirty_by_cmd (priv, l, search_id);
}

static void
move_dirt_cmd_to_widget (BonoboWindowPrivate *priv)
{
	BonoboUINode *cmds, *l;

	cmds = bonobo_ui_xml_get_path (priv->tree, "/commands");

	if (!cmds)
		return;

	for (l = bonobo_ui_node_children (cmds); l;
             l = bonobo_ui_node_next (l)) {
		BonoboUIXmlData *data = bonobo_ui_xml_get_data (priv->tree, l);

		if (data->dirty) {
			char *cmd_name;

			cmd_name = bonobo_ui_node_get_attr (l, "name");
			if (!cmd_name)
				g_warning ("Serious error, cmd without name");
			else
				dirty_by_cmd (priv, priv->tree->root, cmd_name);

			bonobo_ui_node_free_string (cmd_name);
		}
	}
}

static void
update_popups (BonoboWindowPrivate *priv, BonoboUINode *node)
{
	BonoboUINode *l;

	for (l = bonobo_ui_node_children (node); l; l = bonobo_ui_node_next (l)) {
		NodeInfo *info =
			bonobo_ui_xml_get_data (priv->tree, l);
		
		if (info->widget)
			seek_dirty (priv, l, UI_UPDATE_MENU);
		/* else we don't have a widget at the moment */
	}
}

static void
process_state_updates (BonoboWindowPrivate *priv)
{
	while (priv->state_updates) {
		StateUpdate *su = priv->state_updates->data;

		priv->state_updates = g_slist_remove (
			priv->state_updates, su);

		state_update_exec (su);
		state_update_destroy (su);
	}
}

static void
update_widgets (BonoboWindowPrivate *priv)
{
	BonoboUINode *node;

	if (priv->frozen)
		return;

	setup_root_widgets (priv);
	move_dirt_cmd_to_widget (priv);

/*	bonobo_window_dump (priv->win, "before update");*/

	for (node = bonobo_ui_node_children (priv->tree->root); node; node = bonobo_ui_node_next (node)) {
		if (!bonobo_ui_node_get_name (node))
			continue;

		if (bonobo_ui_node_has_name (node, "menu")) {
			seek_dirty (priv, node, UI_UPDATE_MENU);

		} else if (bonobo_ui_node_has_name (node, "popups")) {
			update_popups (priv, node);

		} else if (bonobo_ui_node_has_name (node, "dockitem")) {
			seek_dirty (priv, node, UI_UPDATE_DOCKITEM);

		} else if (bonobo_ui_node_has_name (node, "keybindings")) {
			update_keybindings (priv, node);

		} else if (bonobo_ui_node_has_name (node, "status")) {
			update_status (priv, node);

		} /* else unknown */
	}

	update_commands_state (priv);

	process_state_updates (priv);

/*	bonobo_window_dump (priv->win, "after update");*/
}

static void
popup_remove (BonoboWindowPrivate *priv,
	      WinPopup            *popup)
{
	BonoboUINode *node;

	g_return_if_fail (priv != NULL);
	g_return_if_fail (popup != NULL);

	gtk_signal_disconnect_by_data (GTK_OBJECT (popup->menu), popup);

	node = bonobo_ui_xml_get_path (priv->tree, popup->path);
	prune_widget_info (priv, node, TRUE);

	priv->popups = g_slist_remove (
		priv->popups, popup);
	
	g_free (popup->path);
	g_free (popup);
}

void
bonobo_window_remove_popup (BonoboWindow     *win,
			    const char    *path)
{
	GSList *l, *next;

	g_return_if_fail (path != NULL);
	g_return_if_fail (BONOBO_IS_WINDOW (win));
	g_return_if_fail (win->priv != NULL);

	for (l = win->priv->popups; l; l = next) {
		WinPopup *popup = l->data;

		next = l->next;
		if (!strcmp (popup->path, path))
			popup_remove (win->priv, popup);
	}
}

static void
popup_destroy (GtkObject *menu, WinPopup *popup)
{
	BonoboWindowPrivate *priv = gtk_object_get_data (
		GTK_OBJECT (menu), BONOBO_WINDOW_PRIV_KEY);

	g_return_if_fail (priv != NULL);
	popup_remove (priv, popup);
}

void
bonobo_window_add_popup (BonoboWindow *win,
			 GtkMenu      *menu,
			 const char   *path)
{
	WinPopup     *popup;
	BonoboUINode    *node;
	gboolean         wildcard;

	g_return_if_fail (path != NULL);
	g_return_if_fail (GTK_IS_MENU (menu));
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_window_remove_popup (win, path);

	popup       = g_new (WinPopup, 1);
	popup->menu = menu;
	popup->path = g_strdup (path);

	win->priv->popups = g_slist_prepend (win->priv->popups, popup);

	gtk_object_set_data (GTK_OBJECT (menu),
			     BONOBO_WINDOW_PRIV_KEY,
			     win->priv);

	gtk_signal_connect (GTK_OBJECT (menu), "destroy",
			    (GtkSignalFunc) popup_destroy, popup);

	node = bonobo_ui_xml_get_path_wildcard (
		win->priv->tree, path, &wildcard);

	if (node)
		bonobo_ui_xml_set_dirty (win->priv->tree, node);

	update_widgets (win->priv);
}

GList *
bonobo_window_deregister_get_component_names (BonoboWindow *win)
{
	WinComponent *component;
	GSList *l;
	GList *retval;

	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	retval = NULL;

	for (l = win->priv->components; l; l = l->next) {
		component = l->data;
	
		retval = g_list_prepend (retval, component->name);
	}

	return retval;
}


Bonobo_Unknown
bonobo_window_component_get (BonoboWindow *win,
			      const char  *name)
{
	WinComponent *component;
	GSList *l;

	g_return_val_if_fail (BONOBO_IS_WINDOW (win), CORBA_OBJECT_NIL);
	g_return_val_if_fail (name != NULL, CORBA_OBJECT_NIL);
		
	for (l = win->priv->components; l; l = l->next) {
		component = l->data;
		
		if (!strcmp (component->name, name))
			return component->object;
	}

	return CORBA_OBJECT_NIL;
}


void
bonobo_window_set_contents (BonoboWindow *win,
			    GtkWidget    *contents)
{
	g_return_if_fail (win != NULL);
	g_return_if_fail (win->priv != NULL);
	g_return_if_fail (win->priv->client_area != NULL);

	gtk_container_add (GTK_CONTAINER (win->priv->client_area), contents);
}

GtkWidget *
bonobo_window_get_contents (BonoboWindow *win)
{
	GList     *children;
	GtkWidget *widget;

	g_return_val_if_fail (win != NULL, NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);
	g_return_val_if_fail (win->priv->dock != NULL, NULL);

	children = gtk_container_children (
		GTK_CONTAINER (win->priv->client_area));

	widget = children ? children->data : NULL;

	g_list_free (children);

	return widget;
}

static gboolean
radio_group_destroy (gpointer	key,
		     gpointer	value,
		     gpointer	user_data)
{
	g_free (key);
	g_slist_free (value);

	return TRUE;
}

static void
destroy_priv (BonoboWindowPrivate *priv)
{
	if (priv->container)
		gtk_signal_disconnect_by_data (
			GTK_OBJECT (priv->container), priv->win);

	priv->win = NULL;

	while (priv->popups)
		popup_remove (priv, priv->popups->data);

	gtk_object_unref (GTK_OBJECT (priv->tree));
	priv->tree = NULL;

	g_free (priv->name);
	priv->name = NULL;

	g_free (priv->prefix);
	priv->prefix = NULL;

	g_hash_table_foreach_remove (priv->radio_groups,
				     radio_group_destroy, NULL);
	g_hash_table_destroy (priv->radio_groups);
	priv->radio_groups = NULL;

	g_hash_table_foreach_remove (priv->keybindings,
				     keybindings_free, NULL);
	g_hash_table_destroy (priv->keybindings);
	priv->keybindings = NULL;

	while (priv->components)
		win_component_destroy (priv, priv->components->data);
	
	g_free (priv);
}

static void
bonobo_window_finalize (GtkObject *object)
{
	BonoboWindow *win = (BonoboWindow *)object;
	
	if (win) {
		if (win->priv)
			destroy_priv (win->priv);
		win->priv = NULL;
	}
	GTK_OBJECT_CLASS (bonobo_window_parent_class)->finalize (object);
}

char *
bonobo_window_xml_get (BonoboWindow  *win,
		       const char *path,
		       gboolean    node_only)
{
 	char *str;
 	BonoboUINode    *node;
  	CORBA_char *ret;
  
  	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
  
  	node = bonobo_ui_xml_get_path (win->priv->tree, path);
  	if (!node)
  		return NULL;
 	else {		
 		str = bonobo_ui_node_to_string (node, !node_only);
 		ret = CORBA_string_dup (str);
 		bonobo_ui_node_free_string (str);
 		return ret;
  	}
}

gboolean
bonobo_window_xml_node_exists (BonoboWindow  *win,
			       const char *path)
{
	BonoboUINode *node;
	gboolean      wildcard;

	g_return_val_if_fail (BONOBO_IS_WINDOW (win), FALSE);

	node = bonobo_ui_xml_get_path_wildcard (
		win->priv->tree, path, &wildcard);

	if (!wildcard)
		return (node != NULL);
	else
		return (node != NULL &&
			bonobo_ui_node_children (node) != NULL);
}

BonoboUIXmlError
bonobo_window_object_set (BonoboWindow  *win,
			  const char    *path,
			  Bonobo_Unknown object,
			  CORBA_Environment *ev)
{
	BonoboUINode   *node;
	NodeInfo  *info;

	g_return_val_if_fail (BONOBO_IS_WINDOW (win),
			      BONOBO_UI_XML_BAD_PARAM);

	node = bonobo_ui_xml_get_path (win->priv->tree, path);
	if (!node)
		return BONOBO_UI_XML_INVALID_PATH;

	info = bonobo_ui_xml_get_data (win->priv->tree, node);

	if (info->object != CORBA_OBJECT_NIL) {
		bonobo_object_release_unref (info->object, ev);
		if (info->widget)
			gtk_widget_destroy (info->widget);
		info->widget = NULL;
	}

	if (object != CORBA_OBJECT_NIL)
		info->object = bonobo_object_dup_ref (object, ev);
	else
		info->object = CORBA_OBJECT_NIL;

	bonobo_ui_xml_set_dirty (win->priv->tree, node);

/*	fprintf (stderr, "Object set '%s'\n", path);
	bonobo_window_dump (win, "Before object set updatew");*/

	update_widgets (win->priv);

/*	bonobo_window_dump (win, "After object set updatew");*/

	return BONOBO_UI_XML_OK;
}

BonoboUIXmlError
bonobo_window_object_get (BonoboWindow   *win,
			  const char     *path,
			  Bonobo_Unknown *object,
			  CORBA_Environment *ev)
{
	BonoboUINode *node;
	NodeInfo *info;

	g_return_val_if_fail (object != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), BONOBO_UI_XML_BAD_PARAM);

	*object = CORBA_OBJECT_NIL;

	node = bonobo_ui_xml_get_path (win->priv->tree, path);
	if (!node)
		return BONOBO_UI_XML_INVALID_PATH;

	info = bonobo_ui_xml_get_data (win->priv->tree, node);

	if (info->object != CORBA_OBJECT_NIL)
		*object = bonobo_object_dup_ref (info->object, ev);

	return BONOBO_UI_XML_OK;
}

BonoboUIXmlError
bonobo_window_xml_merge_tree (BonoboWindow *win,
			      const char   *path,
			      BonoboUINode *tree,
			      const char   *component)
{
	BonoboUIXmlError err;
	
	g_return_val_if_fail (win != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (win->priv != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (win->priv->tree != NULL, BONOBO_UI_XML_BAD_PARAM);

	if (!tree || !bonobo_ui_node_get_name(tree))
		return BONOBO_UI_XML_OK;

	bonobo_ui_xml_strip (&tree);

	if (!tree) {
		g_warning ("Stripped tree to nothing");
		return BONOBO_UI_XML_OK;
	}

	/*
	 *  Because peer to peer merging makes the code hard, and
	 * paths non-inituitive and since we want to merge root
	 * elements as peers to save lots of redundant CORBA calls
	 * we special case root.
	 */
	if (bonobo_ui_node_has_name (tree, "Root")) {
		err = bonobo_ui_xml_merge (
			win->priv->tree, path, bonobo_ui_node_children (tree),
			win_component_cmp_name (win->priv, component));
		bonobo_ui_node_free (tree);
	} else
		err = bonobo_ui_xml_merge (
			win->priv->tree, path, tree,
			win_component_cmp_name (win->priv, component));

/*	bonobo_window_dump (win, "after merge"); */

	update_widgets (win->priv);

	return err;
}

BonoboUIXmlError
bonobo_window_xml_merge (BonoboWindow *win,
			 const char   *path,
			 const char   *xml,
			 const char   *component)
{
	BonoboUIXmlError err;
	BonoboUINode *node;
	
	g_return_val_if_fail (win != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (xml != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (win->priv != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (win->priv->tree != NULL, BONOBO_UI_XML_BAD_PARAM);

/*	fprintf (stderr, "Merging :\n%s\n", xml);*/

	node = bonobo_ui_node_from_string (xml);
	
	if (!node)
		return BONOBO_UI_XML_INVALID_XML;

	err = bonobo_window_xml_merge_tree (win, path, node, component);

	return err;
}

BonoboUIXmlError
bonobo_window_xml_rm (BonoboWindow *win,
		      const char   *path,
		      const char   *by_component)
{
	BonoboUIXmlError err;

	g_return_val_if_fail (win != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (win->priv != NULL, BONOBO_UI_XML_BAD_PARAM);
	g_return_val_if_fail (win->priv->tree != NULL, BONOBO_UI_XML_BAD_PARAM);

	err = bonobo_ui_xml_rm (
		win->priv->tree, path,
		win_component_cmp_name (win->priv, by_component));

	update_widgets (win->priv);

	return err;
}

void
bonobo_window_freeze (BonoboWindow *win)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	win->priv->frozen++;
}

void
bonobo_window_thaw (BonoboWindow *win)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));
	
	if (--win->priv->frozen <= 0) {
		update_widgets (win->priv);
		win->priv->frozen = 0;
	}
}

void
bonobo_window_dump (BonoboWindow *win,
		    const char   *msg)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	fprintf (stderr, "Bonobo Win '%s': frozen '%d'\n",
		 win->priv->name, win->priv->frozen);

	win_components_dump (win->priv);

	bonobo_ui_xml_dump (win->priv->tree, win->priv->tree->root, msg);
}

GtkAccelGroup *
bonobo_window_get_accel_group (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	return win->priv->accel_group;
}


static gint
bonobo_window_binding_handle (GtkWidget           *widget,
			      GdkEventKey         *event,
			      BonoboWindowPrivate *priv)
{
	Binding  lookup, *binding;

	lookup.key  = gdk_keyval_to_lower (event->keyval);
	lookup.mods = event->state & BINDING_MOD_MASK ();

	if (!(binding = g_hash_table_lookup (priv->keybindings, &lookup)))
		return FALSE;
	else {
		BonoboUIXmlData *data;
		char *verb;
		
		data = bonobo_ui_xml_get_data (priv->tree, binding->node);
		g_return_val_if_fail (data != NULL, FALSE);
		
		real_exec_verb (priv, data->id,
				(verb = bonobo_ui_node_get_attr (binding->node, "verb")));
		bonobo_ui_node_free_string (verb);
		
		return TRUE;
	}
	
	return FALSE;
}

static BonoboWindowPrivate *
construct_priv (BonoboWindow *win)
{
	BonoboWindowPrivate *priv;
	GnomeDockItemBehavior behavior;

	priv = g_new0 (BonoboWindowPrivate, 1);

	priv->win = win;	

	/* Keybindings; the gtk_binding stuff is just too evil */
	gtk_signal_connect (GTK_OBJECT (win), "key_press_event",
			    (GtkSignalFunc) bonobo_window_binding_handle, priv);

	priv->dock = GNOME_DOCK (gnome_dock_new ());
	gtk_container_add (GTK_CONTAINER (win),
			   GTK_WIDGET    (priv->dock));

	behavior = (GNOME_DOCK_ITEM_BEH_EXCLUSIVE
		    | GNOME_DOCK_ITEM_BEH_NEVER_VERTICAL);
	if (!gnome_preferences_get_menubar_detachable ())
		behavior |= GNOME_DOCK_ITEM_BEH_LOCKED;

	priv->menu_item = GNOME_DOCK_ITEM (gnome_dock_item_new (
		"menu", behavior));
	priv->menu      = GTK_MENU_BAR (gtk_menu_bar_new ());
	gtk_container_add (GTK_CONTAINER (priv->menu_item),
			   GTK_WIDGET    (priv->menu));
	gnome_dock_add_item (priv->dock, priv->menu_item,
			     GNOME_DOCK_TOP, 0, 0, 0, TRUE);

	priv->main_vbox = gtk_vbox_new (FALSE, 0);
	gnome_dock_set_client_area (priv->dock, priv->main_vbox);

	priv->client_area = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->main_vbox), priv->client_area, TRUE, TRUE, 0);

	priv->status = GTK_BOX (gtk_hbox_new (FALSE, 0));
	gtk_box_pack_start (GTK_BOX (priv->main_vbox), GTK_WIDGET (priv->status), FALSE, FALSE, 0);

	priv->tree = bonobo_ui_xml_new (NULL,
					info_new_fn,
					info_free_fn,
					info_dump_fn,
					add_node_fn);

	bonobo_ui_util_build_skeleton (priv->tree);

	gtk_signal_connect (GTK_OBJECT (priv->tree), "override",
			    (GtkSignalFunc) override_fn, priv);

	gtk_signal_connect (GTK_OBJECT (priv->tree), "replace_override",
			    (GtkSignalFunc) replace_override_fn, priv);

	gtk_signal_connect (GTK_OBJECT (priv->tree), "reinstate",
			    (GtkSignalFunc) reinstate_fn, priv);

	gtk_signal_connect (GTK_OBJECT (priv->tree), "rename",
			    (GtkSignalFunc) rename_fn, priv);

	gtk_signal_connect (GTK_OBJECT (priv->tree), "remove",
			    (GtkSignalFunc) remove_fn, priv);

	priv->accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (win),
				    priv->accel_group);

	gtk_widget_show_all (GTK_WIDGET (priv->dock));
	gtk_widget_hide (GTK_WIDGET (priv->status));

	priv->radio_groups = g_hash_table_new (
		g_str_hash, g_str_equal);

	priv->keybindings = g_hash_table_new (keybinding_hash_fn, 
					      keybinding_compare_fn);	

	return priv;
}

/*
 *   To kill bug reports of hiding not working
 * we want to stop show_all showing hidden menus etc.
 */
static void
bonobo_window_show_all (GtkWidget *widget)
{
	BonoboWindow *win = BONOBO_WINDOW (widget);

	if (win->priv->client_area)
		gtk_widget_show_all (win->priv->client_area);

	gtk_widget_show (widget);
}

static void
bonobo_window_class_init (BonoboWindowClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

	bonobo_window_parent_class =
		gtk_type_class (gtk_window_get_type ());

	object_class->finalize = bonobo_window_finalize;

	widget_class->show_all = bonobo_window_show_all;
}

static void
bonobo_window_init (BonoboWindow *win)
{
	win->priv = construct_priv (win);
}

static void
blank_container (BonoboUIContainer *container, BonoboWindow *win)
{
	if (win->priv)
		win->priv->container = NULL;
}

void
bonobo_window_set_ui_container (BonoboWindow *win,
				BonoboObject *container)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));
	g_return_if_fail (!container ||
			  BONOBO_IS_OBJECT (container));

	win->priv->container = container;
	if (container)
		gtk_signal_connect (GTK_OBJECT (container), "destroy",
				    (GtkSignalFunc) blank_container, win);
}

void
bonobo_window_set_name (BonoboWindow  *win,
			const char *win_name)
{
	BonoboWindowPrivate *priv;

	g_return_if_fail (BONOBO_IS_WINDOW (win));

	priv = win->priv;

	g_free (priv->name);
	g_free (priv->prefix);

	if (win_name) {
		priv->name = g_strdup (win_name);
		priv->prefix = g_strconcat ("/", win_name, "/", NULL);
	} else {
		priv->name = NULL;
		priv->prefix = g_strdup ("/");
	}
}

char *
bonobo_window_get_name (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);

	if (win->priv->name)
		return g_strdup (win->priv->name);
	else
		return NULL;
}

GtkWidget *
bonobo_window_construct (BonoboWindow *win,
			 const char   *win_name,
			 const char   *title)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	bonobo_window_set_name (win, win_name);

	if (title)
		gtk_window_set_title (GTK_WINDOW (win), title);

	return GTK_WIDGET (win);
}

GtkWidget *
bonobo_window_new (const char   *win_name,
		const char   *title)
{
	BonoboWindow *win;

	win = gtk_type_new (BONOBO_TYPE_WINDOW);

	return bonobo_window_construct (win, win_name, title);
}

/**
 * bonobo_window_get_type:
 *
 * Returns: The GtkType for the BonoboWindow class.
 */
GtkType
bonobo_window_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboWindow",
			sizeof (BonoboWindow),
			sizeof (BonoboWindowClass),
			(GtkClassInitFunc) bonobo_window_class_init,
			(GtkObjectInitFunc) bonobo_window_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_window_get_type (), &info);
	}

	return type;
}
