/* SRTest.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "SRTest.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "SREvent.h"


GtkWidget* create_dialog1 (void);

gboolean 
SRTest_init (int *argc, 
	     char ***argv) 
{
    GtkWidget *widget;
    gtk_init (argc, argv);
    widget = create_dialog1 ();
    gtk_widget_show (widget);
    return TRUE ;
}


gboolean 
SRTest_terminate ()
{
    return TRUE;
}



GtkEntry *name, *role, *description, *location, 
	 *index_in_p, *childno, *specialization, *text;


gboolean
SRTest_show_obj (SRObject *obj)
{
    {
	char *str_name;
	sro_get_name (obj, &str_name, SR_INDEX_CONTAINER);
	gtk_entry_set_text (name, str_name ? str_name : "(none)");
	SR_freeString (str_name);
    }
    
    {
	char *desc;
	sro_get_description (obj, &desc, SR_INDEX_CONTAINER);
	gtk_entry_set_text (description, desc ? desc : "(none)");
	SR_freeString (desc);
    }
    
    if (sro_is_component (obj, SR_INDEX_CONTAINER))    
    {
	SRRectangle rect;
	SRCoordinateType coord;
	gint32 x, y, w, h;
	char tmp[40] = "";	
	char tmp1[30];
	
	coord = SR_COORD_TYPE_SCREEN;
	sro_get_location (obj, coord, &rect, SR_INDEX_CONTAINER);
	if (coord == SR_COORD_TYPE_WINDOW)
	    strcat (tmp, "(window) : ");
	else if (coord == SR_COORD_TYPE_SCREEN)
	    strcat (tmp, "(screen) : ");
	
	sr_rectangle_get_x 	(&rect, &x);
	sr_rectangle_get_y 	(&rect, &y);
	sr_rectangle_get_width  (&rect, &w);
	sr_rectangle_get_height (&rect, &h);
	sprintf (tmp1, "(%d,%d)-(%d,%d)", x, y, x + w, y + h);
	strcat (tmp, tmp1);
	gtk_entry_set_text (location, tmp);
    }
    else
    {
	gtk_entry_set_text (location, "");
    }


    {
	SRObjectRoles sr_role;
	char *role_name;
	char tmp[50];
	sro_get_role (obj, &sr_role, SR_INDEX_CONTAINER);
	sro_get_role_name (obj, &role_name, SR_INDEX_CONTAINER);
	
	sprintf (tmp, "%s(%d)", role_name ? role_name : "(none)", sr_role);
	gtk_entry_set_text (role, tmp);	 
	SR_freeString (role_name);
    }
    
    {
	char tmp[50] , tmp1[10];
	tmp[0] = tmp1[0] = '\0';
	if (sro_is_action (obj, SR_INDEX_CONTAINER))		{	sprintf (tmp1, "ACT ");		strcat (tmp, tmp1); };
        if (sro_is_component (obj, SR_INDEX_CONTAINER))		{	sprintf (tmp1, "LOC ");		strcat (tmp, tmp1); };
        if (sro_is_editable_text (obj, SR_INDEX_CONTAINER))	{ 	sprintf (tmp1, "EDT ");		strcat (tmp, tmp1); };
        if (sro_is_hypertext (obj, SR_INDEX_CONTAINER))		{	sprintf (tmp1, "HTXT ");	strcat (tmp, tmp1); };
        if (sro_is_image (obj, SR_INDEX_CONTAINER))	 	{	sprintf (tmp1, "IMG ");		strcat (tmp, tmp1); };
        if (sro_is_selection (obj, SR_INDEX_CONTAINER))		{	sprintf (tmp1, "SEL ");		strcat (tmp, tmp1); };
        if (sro_is_table (obj, SR_INDEX_CONTAINER)) 		{	sprintf (tmp1, "TBL ");		strcat (tmp, tmp1); };
        if (sro_is_text (obj, SR_INDEX_CONTAINER))		{	sprintf (tmp1, "TXT ");		strcat (tmp, tmp1); };
        if (sro_is_value (obj, SR_INDEX_CONTAINER)) 		{	sprintf (tmp1, "VAL ");		strcat (tmp, tmp1); };
	gtk_entry_set_text (specialization, tmp);
    }
    
    {
	guint32 index;
	char tmp[20];
	sro_get_index_in_parent (obj, &index);
	sprintf (tmp, "%d", index);
	gtk_entry_set_text (index_in_p, tmp);
    }

    {
	guint32 cc;
	char tmp[20];
	sro_get_children_count (obj, &cc);
	sprintf (tmp, "%d", cc);
	gtk_entry_set_text (childno, tmp);
    }
    return TRUE ;
}







void
on_quit_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_main_quit ();
}


void
on_name_realize                        (GtkWidget       *widget,
                                        gpointer         user_data)
{
   	name = GTK_ENTRY (widget );
}


void
on_description_realize                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
	description = GTK_ENTRY (widget);
}


void
on_role_realize                        (GtkWidget       *widget,
                                        gpointer         user_data)
{
	role = GTK_ENTRY (widget);
}


void
on_child_no_realize                    (GtkWidget       *widget,
                                        gpointer         user_data)
{
	childno = GTK_ENTRY (widget);
}


void
on_index_in_parent_realize             (GtkWidget       *widget,
                                        gpointer         user_data)
{
	index_in_p = GTK_ENTRY (widget);
}


void
on_location_realize                    (GtkWidget       *widget,
                                        gpointer         user_data)
{
	location = GTK_ENTRY (widget);
}


void
on_specialization_realize              (GtkWidget       *widget,
                                        gpointer         user_data)
{
	specialization = GTK_ENTRY (widget);
}

void
on_text_realize              		(GtkWidget       *widget,
                                        gpointer         user_data)
{
	text = GTK_ENTRY (widget);
}

void
exit_test              			(GtkWidget       *widget,
                                        gpointer         user_data)
{
	sru_exit_loop ();
}


GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *table1;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *label7;
  GtkWidget *label8;
  GtkWidget *name;
  GtkWidget *description;
  GtkWidget *role;
  GtkWidget *specialization;
  GtkWidget *child_no;
  GtkWidget *index_in_parent;
  GtkWidget *location;
  GtkWidget *text;
  GtkWidget *dialog_action_area1;
  GtkWidget *quit;

  dialog1 = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog1), "dialog1", dialog1);
  gtk_window_set_title (GTK_WINDOW (dialog1), "SRLow Screen Reader tester");
  gtk_window_set_policy (GTK_WINDOW (dialog1), TRUE, TRUE, FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog1), "dialog_vbox1", dialog_vbox1);
  gtk_widget_show (dialog_vbox1);
  
  table1 = gtk_table_new (8, 2, FALSE);
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "table1", table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), table1, TRUE, TRUE, 0);

  label1 = gtk_label_new ("Name");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  label2 = gtk_label_new ("Description");
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  label3 = gtk_label_new ("Role");
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label4 = gtk_label_new ("Child no");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  label5 = gtk_label_new ("Index in Parent");
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  label6 = gtk_label_new ("Location");
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  label7 = gtk_label_new ("Text");
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  label8 = gtk_label_new ("Specialization");
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  name = gtk_entry_new ();
  gtk_widget_ref (name);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "name", name,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (name);
  gtk_table_attach (GTK_TABLE (table1), name, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  description = gtk_entry_new ();
  gtk_widget_ref (description);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "description", description,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (description);
  gtk_table_attach (GTK_TABLE (table1), description, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  role = gtk_entry_new ();
  gtk_widget_ref (role);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "role", role,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (role);
  gtk_table_attach (GTK_TABLE (table1), role, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  specialization = gtk_entry_new ();
  gtk_widget_ref (specialization);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "specialization", specialization,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show ( specialization);
  gtk_table_attach (GTK_TABLE (table1),  specialization, 1, 2,6 , 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  child_no = gtk_entry_new ();
  gtk_widget_ref (child_no);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "child_no", child_no,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (child_no);
  gtk_table_attach (GTK_TABLE (table1), child_no, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  index_in_parent = gtk_entry_new ();
  gtk_widget_ref (index_in_parent);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "index_in_parent", index_in_parent,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (index_in_parent);
  gtk_table_attach (GTK_TABLE (table1), index_in_parent, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  location = gtk_entry_new ();
  gtk_widget_ref (location);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "location", location,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (location);
  gtk_table_attach (GTK_TABLE (table1), location, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  text = gtk_entry_new ();
  gtk_widget_ref (text);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "text", text,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (text);
  gtk_table_attach (GTK_TABLE (table1), text, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog1), "dialog_action_area1", dialog_action_area1);
  gtk_widget_show (dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  quit = gtk_button_new_with_label ("QUIT");
  gtk_widget_ref (quit);
  gtk_object_set_data_full (GTK_OBJECT (dialog1), "quit", quit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (quit);
  gtk_box_pack_start (GTK_BOX (dialog_action_area1), quit, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (name), "realize",
                      GTK_SIGNAL_FUNC (on_name_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (description), "realize",
                      GTK_SIGNAL_FUNC (on_description_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (role), "realize",
                      GTK_SIGNAL_FUNC (on_role_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (child_no), "realize",
                      GTK_SIGNAL_FUNC (on_child_no_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (index_in_parent), "realize",
                      GTK_SIGNAL_FUNC (on_index_in_parent_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (location), "realize",
                      GTK_SIGNAL_FUNC (on_location_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (specialization), "realize",
                      GTK_SIGNAL_FUNC (on_specialization_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (text), "realize",
                      GTK_SIGNAL_FUNC (on_text_realize),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (quit), "clicked",
			GTK_SIGNAL_FUNC(exit_test),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (dialog1), "destroy",
			GTK_SIGNAL_FUNC(exit_test),
                      NULL);
		      

  return dialog1;
}

void SRTest_print_obj (SRObject *sr_obj, char *msg)
{

    fprintf (stderr, "\n\n\n OBJECT FROM %s : ", msg);
    
    {
	char *name;
	sro_get_name (sr_obj, &name, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\tName : %s", name ? name : "(none)");
	SR_freeString (name);
    }

    {
	char *description;
	sro_get_description (sr_obj, &description, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\tDescription : %s", description ? description : "(none)");
	SR_freeString (description);
    }

    {
	SRState state = 0;
	sro_get_state (sr_obj, &state, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\tState : ");
	if (state & SR_STATE_CHECKED)
	    fprintf (stderr, "CHECKED ");
	if (state & SR_STATE_CHECKABLE)
	    fprintf (stderr, "CHECKABLE ");
	if (state & SR_STATE_EXPANDABLE)
	    fprintf (stderr, "EXPANDABLE ");
	if (state & SR_STATE_FOCUSABLE)
	    fprintf (stderr, "FOCUSABLE ");
    }

    {
	char *shortcut;
	if (sro_is_action (sr_obj, SR_INDEX_CONTAINER))
	if (sro_get_shortcut (sr_obj, &shortcut, SR_INDEX_CONTAINER))
	{
	    fprintf (stderr, "\n\tShortcut : %s", shortcut ? shortcut : "(none)");
	    SR_freeString (shortcut);
	}
    }

    {
	char *accelerator;
	if (sro_is_action (sr_obj, SR_INDEX_CONTAINER))
	if (sro_get_accelerator (sr_obj, &accelerator, SR_INDEX_CONTAINER))
	{
	    fprintf (stderr, "\n\tAccelerator : %s", accelerator ? accelerator :"(none)");
	    SR_freeString (accelerator);
	}
    }
   
    {
	char *str_role;
	SRObjectRoles sr_role;
	sro_get_role_name (sr_obj, &str_role, SR_INDEX_CONTAINER);
	sro_get_role (sr_obj, &sr_role, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\tRole : %s(%d)", str_role ? str_role : "(none)", sr_role);
	SR_freeString (str_role);
    }

    {
	SRRelation relation;
	sro_get_relation (sr_obj, &relation, SR_INDEX_CONTAINER);
	if (relation & SR_RELATION_MEMBER_OF)
	{
	    SRLong index;
	    SRObject **objs;
	    sro_get_index_in_group (sr_obj, &index, 0);
	    fprintf (stderr, "\n\tIndex in group : %ld", index);
	    sro_get_objs_for_relation (sr_obj, SR_RELATION_MEMBER_OF, &objs, 0);
	    if (objs)
	    {
		int i;
		gchar *name;
		for (i = 0; objs[i]; i++)
		{
		    sro_get_name (objs[i], &name, 0);
		    fprintf (stderr, "\n\t\t%d member %s", i, name);
		    SR_freeString (name);
		    sro_release_reference (objs[i]);
		}
		g_free (objs);
	    }	    
	}	
    }

    {
	guint32 index;
	sro_get_index_in_parent (sr_obj, &index);	
	fprintf (stderr, "\n\tIndex in parent  : %d", index);
    }
    
    {
	guint32 child_no;
	sro_get_children_count (sr_obj, &child_no);	
	fprintf (stderr, "\n\tChildren count : %d", child_no);
    }

    if (sro_is_component (sr_obj, SR_INDEX_CONTAINER))    
    {
	SRRectangle rect;
	SRCoordinateType coord;
	gint32 x, y, w, h;
	
	coord = SR_COORD_TYPE_SCREEN;
	sro_get_location (sr_obj, coord, &rect, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\tLocation ");
	if (coord == SR_COORD_TYPE_WINDOW)
	    fprintf (stderr, "(window) : ");
	else if (coord == SR_COORD_TYPE_SCREEN)
	    fprintf (stderr, "(screen) : ");
	
	sr_rectangle_get_x 	(&rect, &x);
	sr_rectangle_get_y 	(&rect, &y);
	sr_rectangle_get_width  (&rect, &w);
	sr_rectangle_get_height (&rect, &h);
	fprintf (stderr, "(%d,%d)-(%d,%d)", x, y, x + w, y + h);
    }

    {						
	fprintf (stderr, "\n\tIs : ");
	if (sro_is_action (sr_obj, SR_INDEX_CONTAINER)) 	fprintf (stderr, "Action ");
	if (sro_is_component (sr_obj, SR_INDEX_CONTAINER)) 	fprintf (stderr, "Location ");
	if (sro_is_editable_text (sr_obj, SR_INDEX_CONTAINER)) 	fprintf (stderr, "EditText ");
	if (sro_is_hypertext (sr_obj, SR_INDEX_CONTAINER))	fprintf (stderr, "HyperText ");
	if (sro_is_image (sr_obj, SR_INDEX_CONTAINER))		fprintf (stderr, "Image ");
	if (sro_is_selection (sr_obj, SR_INDEX_CONTAINER))	fprintf (stderr, "Selection ");
	if (sro_is_table (sr_obj, SR_INDEX_CONTAINER))		fprintf (stderr, "Table ");
	if (sro_is_text (sr_obj, SR_INDEX_CONTAINER))		fprintf (stderr, "Text ");
	if (sro_is_value (sr_obj, SR_INDEX_CONTAINER))		fprintf (stderr, "Value ");
    }

    if (sro_is_text (sr_obj, SR_INDEX_CONTAINER))
    {
	SRLong line_offset;
	gchar *text;
	gchar **selections;
	gchar ch;
	SRRectangle rect;
	SRTextBoundaryType type_text;
	SRCoordinateType type_coord;
	gint32 x, y, w, h;
	SRTextAttribute *attr;
	
	fprintf (stderr, "\n\tTEXT :");
	sro_text_get_caret_offset (sr_obj, &line_offset, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\t\tCaret offset in current line : %ld", line_offset);
	sro_text_get_selections (sr_obj, &selections, SR_INDEX_CONTAINER);
	if (selections)
	{
	    int i;
	    for (i = 0; selections[i]; i++)
		fprintf (stderr, "\n\t\t%d selection : %s", i, selections[i]);
	    g_strfreev (selections);
	}
	if (line_offset != -1)
	{
    	    sro_text_get_attributes_at_index (sr_obj, line_offset, &attr, SR_INDEX_CONTAINER);
	    if (attr)
	    {
		fprintf (stderr, "\n\t\tAttributes for curent character : %s", attr[0] ? attr[0] : "(none)");
		sra_get_attribute_values_string (attr[0], NULL, &text);
		fprintf (stderr, "\n\t\tAttributes for curent character : %s", text ? text : "(none)");
		SR_freeString (text);
		sra_get_attribute_value (attr[0], "font-name", &text);
		fprintf (stderr, "\n\t\tFont at index %ld : %s", line_offset, text ? text : "(none)");
		SR_freeString (text);
	    }
	    if (sro_text_get_char_at_index (sr_obj, line_offset, &ch, 0))
		fprintf (stderr, "\n\t\tChar at index %ld : %c", line_offset, ch);
	    fprintf (stderr, "\n\t\tText from caret :");
	    type_text = SR_TEXT_BOUNDARY_CHAR;
	    type_coord = SR_COORD_TYPE_SCREEN;
	    sro_text_get_text_from_caret (sr_obj, type_text, &text, SR_INDEX_CONTAINER);
	    sro_text_get_text_location_from_caret (sr_obj, type_text, type_coord, &rect, SR_INDEX_CONTAINER); 
	    sr_rectangle_get_x 	(&rect, &x);
	    sr_rectangle_get_y 	(&rect, &y) ;
	    sr_rectangle_get_width  (&rect, &w);
	    sr_rectangle_get_height (&rect, &h);
	    fprintf (stderr, "\n\t\t\tChar : (%d,%d)-(%d,%d) : %s", x, y, x + w, y + h, 
					    text ? text : "(none)");
	    SR_freeString (text);	
	    type_text = SR_TEXT_BOUNDARY_WORD;
	    type_coord = SR_COORD_TYPE_SCREEN;
	    sro_text_get_text_from_caret (sr_obj, type_text, &text, SR_INDEX_CONTAINER);
	    sro_text_get_text_location_from_caret (sr_obj, type_text, type_coord, &rect, SR_INDEX_CONTAINER); 
	    sr_rectangle_get_x 	(&rect,&x);
	    sr_rectangle_get_y 	(&rect,&y);
	    sr_rectangle_get_width  (&rect,&w);
	    sr_rectangle_get_height (&rect,&h);
	    fprintf (stderr, "\n\t\t\tWord : (%d,%d)-(%d,%d) : %s", x, y, x + w, y + h, 
					    text ? text : "(none)");
	    SR_freeString (text);	
	
	    type_text = SR_TEXT_BOUNDARY_LINE;
	    type_coord = SR_COORD_TYPE_SCREEN;
	    sro_text_get_text_from_caret (sr_obj, type_text, &text, SR_INDEX_CONTAINER);
	    sro_text_get_text_location_from_caret (sr_obj, type_text, type_coord, &rect, SR_INDEX_CONTAINER); 
	    sr_rectangle_get_x 	(&rect,&x);
	    sr_rectangle_get_y 	(&rect,&y);
	    sr_rectangle_get_width  (&rect,&w);
	    sr_rectangle_get_height (&rect,&h);
	    fprintf (stderr, "\n\t\t\tLine : (%d,%d)-(%d,%d) : %s", x, y, x + w, y + h,
					     text ? text : "(none)");
	    SR_freeString (text);	
	
	    type_text = SR_TEXT_BOUNDARY_SENTENCE;
	    sro_text_get_text_from_caret (sr_obj, type_text, &text, SR_INDEX_CONTAINER);
	    fprintf (stderr, "\n\t\t\tSentence : %s", text ? text : "(none)");
	    SR_freeString (text);
	    {
		SRTextAttribute *attr;
		gchar *val;
		int i;
	
		type_text = SR_TEXT_BOUNDARY_SENTENCE;
		sro_text_get_text_attr_from_caret (sr_obj, type_text, &attr, SR_INDEX_CONTAINER);
		for (i = 0; attr && attr[i]; i++)
		{
		    fprintf (stderr, "\n\t\t\tSentence attr %d :", i);
		    fprintf (stderr, "\n\t\t\t  Sentence attr : %s", attr[i]);
		    sra_get_attribute_values_string (attr[i], NULL, &val);
		    fprintf (stderr, "\n\t\t\t  Sentence attr : %s", val);
		    SR_freeString (val);
		    sra_get_attribute_values_string (attr[i], "bold:size", &val);
		    fprintf (stderr, "\n\t\t\t  Sentence attr value for bold and size : %s", val ? val : "(none)");
		    SR_freeString (val);
		    sra_get_attribute_value (attr[i], "font-name", &val);
		    fprintf (stderr, "\n\t\t\t  Sentence attr value for font-name : %s", val ? val : "(none)");
		    SR_freeString (val);
		}
	    }
	}	
    }

    if (sro_is_action (sr_obj, SR_INDEX_CONTAINER))
    {
	SRLong count;
	int i;
	fprintf (stderr, "\n\tACTION : ");	
	sro_action_get_count (sr_obj, &count, SR_INDEX_CONTAINER);
	
	for (i = 0; i < count; i++)
	{
	    gchar *tmp;
	    fprintf (stderr, "\n\t\tAction no %d :", i);
	    sro_action_get_name (sr_obj, i, &tmp, SR_INDEX_CONTAINER);
	    fprintf (stderr, "\n\t\t\tAction name : %s", tmp ? tmp : "(none)");
	    SR_freeString (tmp);
	    sro_action_get_description (sr_obj, i, &tmp, SR_INDEX_CONTAINER);
	    fprintf (stderr, "\n\t\t\tAction description : %s", tmp ? tmp : "(none)");
	    SR_freeString (tmp);
	    sro_action_get_key (sr_obj, i, &tmp, SR_INDEX_CONTAINER);
	    fprintf (stderr, "\n\t\t\tAction key binding : %s", tmp ? tmp : "(none)");
	    SR_freeString (tmp);
	}
    };

    if (sro_is_value (sr_obj, SR_INDEX_CONTAINER))
    {
	gdouble val;
	
	fprintf (stderr, "\n\tVALUE : ");	
	sro_value_get_min_val (sr_obj, &val, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\t\tMinimum value : %f", val);
	sro_value_get_max_val (sr_obj, &val, SR_INDEX_CONTAINER);
        fprintf (stderr, "\n\t\tMaximum value : %f", val);
        sro_value_get_crt_val (sr_obj, &val, SR_INDEX_CONTAINER);
        fprintf (stderr, "\n\t\tCurrent value : %f", val);
	    
    };

    if (sro_is_image (sr_obj, SR_INDEX_CONTAINER))
    {
	char *desc;
	SRRectangle rect;
        SRCoordinateType coord;
        gint32 x, y, w, h;
	
        fprintf (stderr, "\n\tIMAGE : ");	
	sro_image_get_description (sr_obj, &desc, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n\t\tDescription : %s", desc ? desc : "(none)");
	SR_freeString (desc);
	coord = SR_COORD_TYPE_SCREEN;	
	sro_image_get_location (sr_obj, coord, &rect, SR_INDEX_CONTAINER);
	fprintf (stderr,"\n\t\tPosition ");
	if (coord == SR_COORD_TYPE_WINDOW)
	    fprintf (stderr,"(window) : ");
	else if (coord == SR_COORD_TYPE_SCREEN)
	    fprintf (stderr, "(screen) : ");
        sr_rectangle_get_x 	(&rect,&x);
        sr_rectangle_get_y 	(&rect,&y);
        sr_rectangle_get_width  (&rect,&w);
        sr_rectangle_get_height (&rect,&h);
        fprintf (stderr, "(%d,%d)-(%d,%d)", x, y, x + w, y + h);
    };
    fflush (stdout);
}
