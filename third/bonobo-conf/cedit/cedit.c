/*
 * cedit.c: bonobo configuration editor
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include "config.h"
#include <bonobo.h>

static void
create_subtree (char *path, GtkWidget *parent_item)
{
/*
	GtkWidget *tree, *subtree, *item;
	GSList *elist, *l;
	char *name;

	elist = l =  gconf_client_all_dirs (client, path, NULL);

	tree = gtk_tree_new ();

	while (l) {
		name = (char *)l->data;
		item = gtk_tree_item_new_with_label (&name[1]);

		gtk_widget_show (item);
		gtk_tree_append (GTK_TREE (tree), item);
		
		subtree = gtk_tree_new ();
		gtk_tree_item_set_subtree(GTK_TREE_ITEM (item), subtree);

		g_free (name);
		l = l->next;
	}

	g_slist_free (elist);

	gtk_tree_item_set_subtree(GTK_TREE_ITEM (parent_item), tree);
*/
}

static void
create_main_window ()
{
	BonoboUIContainer *ui_container;
	GtkWidget         *window;	
	GtkWidget         *hpan;
	GtkWidget         *tree, *root_item;

	window = bonobo_window_new ("cedit", "Bonobo Configuration Editor");
	ui_container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (ui_container, BONOBO_WINDOW (window));
	
	gtk_window_set_default_size (GTK_WINDOW (window), 640, 400);

	hpan = gtk_hpaned_new ();

	tree = gtk_tree_new ();
	gtk_paned_add1 (GTK_PANED (hpan), tree);

	{
		GtkWidget *e;
		
		e = gtk_entry_new ();
		gtk_paned_add2 (GTK_PANED (hpan), e);
	
	}

	root_item = gtk_tree_item_new_with_label("default database");
	gtk_tree_append(GTK_TREE (tree), root_item);
	create_subtree ("/", root_item);

	gtk_paned_set_position (GTK_PANED (hpan), 200);

	bonobo_window_set_contents (BONOBO_WINDOW (window), hpan);

	gtk_widget_show_all (window);
}

int
main (int argc, char **argv)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	gnome_init ("cedit", VERSION, argc, argv);

	if ((oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");

	if (bonobo_init (NULL, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");

	create_main_window ();

	bonobo_main ();

	return 0;
}


