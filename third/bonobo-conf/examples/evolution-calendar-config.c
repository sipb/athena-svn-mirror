/*
 * config-moniker-demo.c: a small demo for the configuration moniker
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <config.h>
#include <bonobo.h>
#include <bonobo-conf/bonobo-config-database.h>
#include <bonobo-conf/bonobo-property-editor.h>
#include <bonobo-conf/bonobo-property-frame.h>

static char *day_names[] = { N_("Monday"), N_("Tuesday"), N_("Wednesday"),
			     N_("Thursday"), N_("Friday"), N_("Saturday"),
			     N_("Sunday"), NULL };

static char *time_div_names[] = { N_("60 minutes"),  N_("30 minutes"),  
				  N_("15 minutes"),  N_("10 minutes"),  
				  N_("5 minutes") };  

static char *time_format_names[] = { N_("12 hour (am/pm)"), N_("24 hour") };

static void
create_calendar_page (GtkWidget *pf)
{
	Bonobo_PropertyBag bag;
	GtkWidget *v0, *f, *v, *h, *w;
	GtkObject *e;

	bag = BONOBO_OBJREF (BONOBO_PROPERTY_FRAME (pf)->proxy);

	v0 = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (pf), v0);

	f = gtk_frame_new (_("Work week"));
	gtk_box_pack_start (GTK_BOX (v0), f, 0, 0, 0);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);
	gtk_container_set_border_width (GTK_CONTAINER (v), 5);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v), h);

	e = bonobo_peditor_boolean_new (_("Mon"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "ww-mon", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start_defaults (GTK_BOX (h), w);

	e = bonobo_peditor_boolean_new (_("Tue"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "ww-tue", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start_defaults (GTK_BOX (h), w);

	e = bonobo_peditor_boolean_new (_("Wed"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "ww-wed", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start_defaults (GTK_BOX (h), w);

	e = bonobo_peditor_boolean_new (_("Thu"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "ww-thu", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start_defaults (GTK_BOX (h), w);

	e = bonobo_peditor_boolean_new (_("Fri"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "ww-fri", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start_defaults (GTK_BOX (h), w);

	e = bonobo_peditor_boolean_new (_("Sat"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "ww-sat", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start_defaults (GTK_BOX (h), w);

	e = bonobo_peditor_boolean_new (_("Sun"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "ww-sun", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start_defaults (GTK_BOX (h), w);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v), h);

	w = gtk_label_new (_("First day of week:"));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);
	
	e = bonobo_peditor_option_new (0, day_names);
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "first-day", 
				     TC_ulong, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);


	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v), h);

	w = gtk_label_new (_("Start of day:"));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	e = bonobo_peditor_new (bag, "start-of-day", TC_string, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	w = gtk_label_new (_("End of day:"));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	e = bonobo_peditor_new (bag, "end-of-day", TC_string, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	f = gtk_frame_new (_("Display options"));
	gtk_box_pack_start (GTK_BOX (v0), f, 0, 0, 0);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);
	gtk_container_set_border_width (GTK_CONTAINER (v), 5);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v), h);

	w = gtk_label_new (_("Time format:"));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);
	
	e = bonobo_peditor_option_new (1, time_format_names);
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "time-format", 
				     TC_ulong, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v), h);

	w = gtk_label_new (_("Time divisions:"));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);
	
	e = bonobo_peditor_option_new (0, time_div_names);
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "time-divisions",
				     TC_ulong, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Show appointment end times"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "show-end-times",
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Compress weekends"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e),  bag, 
				     "compress-weekends", TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);


	f = gtk_frame_new (_("Date navigator option"));
	gtk_box_pack_start (GTK_BOX (v0), f, 0, 0, 0);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);
	gtk_container_set_border_width (GTK_CONTAINER (v), 5);

	e = bonobo_peditor_boolean_new (_("Show week numbers"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, 
				     "show-week-numbers", TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);


	gtk_widget_show_all (v0);
}

static void
create_task_pad_page (GtkWidget *pf)
{
	Bonobo_PropertyBag bag;
	GtkWidget *v0, *h0, *v, *f, *t, *w, *l;
	GtkObject *e;

	bag = BONOBO_OBJREF (BONOBO_PROPERTY_FRAME (pf)->proxy);

	v0 = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (pf), v0);

	h0 = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v0), h0);

	f = gtk_frame_new (_("Show"));
	gtk_box_pack_start_defaults (GTK_BOX (h0), f);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);
	gtk_container_set_border_width (GTK_CONTAINER (v), 5);

	e = bonobo_peditor_boolean_new (_("Due Date"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "show-due-date", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Time Until Due"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, 
				     "show-time-until-due", TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Priority"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "show-priority", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	f = gtk_frame_new (_("Highlight"));
	gtk_box_pack_start_defaults (GTK_BOX (h0), f);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);
	gtk_container_set_border_width (GTK_CONTAINER (v), 5);

	e = bonobo_peditor_boolean_new (_("Overdue Items"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, 
				     "highlight-overdue", TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Items Due Today"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, 
				     "highlight-due-today", TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Items Not Yet Due"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, 
				     "highlight-not-yet-due", TC_boolean, 
				     NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	
	f = gtk_frame_new (_("Colors"));
	gtk_box_pack_start_defaults (GTK_BOX (v0), f);

	t = gtk_table_new (4, 3, FALSE);
	gtk_container_add (GTK_CONTAINER (f), t);
	gtk_container_set_border_width (GTK_CONTAINER (t), 5);

	l = gtk_label_new (_("Items Not Yet Due:"));
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	e = bonobo_peditor_new (bag, "color-not-due", TC_Bonobo_Config_Color, 
				NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_table_attach (GTK_TABLE (t), l, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (t), w, 1, 2, 0, 1, 0, 0, 5, 2);

	l = gtk_label_new (_("Items Due Today:"));
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	e = bonobo_peditor_new (bag, "color-due-totay", TC_Bonobo_Config_Color,
				NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_table_attach (GTK_TABLE (t), l, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (t), w, 1, 2, 1, 2, 0, 0, 5, 2);

	l = gtk_label_new (_("Overdue Items:"));
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	e = bonobo_peditor_new (bag, "color-overdue", TC_Bonobo_Config_Color,
				NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_table_attach (GTK_TABLE (t), l, 0, 1, 2, 3, GTK_FILL, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (t), w, 1, 2, 2, 3, 0, 0, 5, 2);

	gtk_widget_show_all (v0);
}

static void
create_reminders_page (GtkWidget *pf)
{
	Bonobo_PropertyBag bag;
	GtkWidget *v0, *f, *v, *h, *w;
	GtkObject *e;

	bag = BONOBO_OBJREF (BONOBO_PROPERTY_FRAME (pf)->proxy);

	v0 = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (pf), v0);

	f = gtk_frame_new (_("Defaults"));
	gtk_box_pack_start (GTK_BOX (v0), f, 0, 0, 0);

	h = gtk_hbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), h);
	gtk_container_set_border_width (GTK_CONTAINER (h), 5);

	e = bonobo_peditor_boolean_new (_("Remind me of all appointments"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "remind-me", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	e = bonobo_peditor_int_range_new (0, 100, 1);
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "remind-time", 
				     TC_long, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	bonobo_peditor_set_guard (w, bag, "remind-me");
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	w = gtk_label_new (_("minutes before they occur."));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	f = gtk_frame_new (_("Visula Alarms"));
	gtk_box_pack_start (GTK_BOX (v0), f, 0, 0, 0);
	
	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);
	gtk_container_set_border_width (GTK_CONTAINER (v), 5);

	e = bonobo_peditor_boolean_new (_("Beep when alarm window appear."));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "enable-beep", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (v), w, 0, 0, 0);

	f = gtk_frame_new (_("Audio Alarms"));
	gtk_box_pack_start (GTK_BOX (v0), f, 0, 0, 0);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);
	gtk_container_set_border_width (GTK_CONTAINER (v), 5);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (v), h, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Alarm timeout after"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "enable-alarm", 
				     TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	e = bonobo_peditor_int_range_new (0, 100, 1);
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "alarm-timeout",
				     TC_long, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	bonobo_peditor_set_guard (w, bag, "enable-alarm");
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	w = gtk_label_new (_("seconds"));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (v), h, 0, 0, 0);

	e = bonobo_peditor_boolean_new (_("Enable snoozing for"));
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, 
				     "enable-snoozing", TC_boolean, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	e = bonobo_peditor_int_range_new (0, 1000, 1);
	bonobo_peditor_set_property (BONOBO_PEDITOR (e), bag, "snooze-time", 
				     TC_long, NULL);
	w = bonobo_peditor_get_widget (BONOBO_PEDITOR (e)); 
	bonobo_peditor_set_guard (w, bag, "enable-snoozing");
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	w = gtk_label_new (_("seconds"));
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	gtk_widget_show_all (v0);
}

static gint
create_dialog ()
{
	GtkWidget *d, *l, *pf;
	char *mname;

	d = gnome_property_box_new ();

	gtk_signal_connect (GTK_OBJECT (d), "delete_event",
			    GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	gtk_window_set_title (GTK_WINDOW (d), _("Calendar Preferences"));
	
	gnome_dialog_button_connect (GNOME_DIALOG (d), 0, 
				     GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	gnome_dialog_button_connect (GNOME_DIALOG (d), 2, 
				     GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

 	mname = "xmldb:/tmp/calendar.xmldb#config:Calendar";
 	pf = bonobo_property_frame_new (NULL, mname);
	gtk_widget_show (pf);

	create_calendar_page (pf);

	l = gtk_label_new (_("Calendar"));
	gnome_property_box_append_page (GNOME_PROPERTY_BOX (d), pf, l);

 	mname = "xmldb:/tmp/calendar.xmldb#config:TaskPad";
 	pf = bonobo_property_frame_new (NULL, mname);
	gtk_widget_show (pf);

	create_task_pad_page (pf);

	l = gtk_label_new (_("TaskPad"));
	gnome_property_box_append_page (GNOME_PROPERTY_BOX (d), pf, l);

 	mname = "xmldb:/tmp/calendar.xmldb#config:Reminders";
 	pf = bonobo_property_frame_new (NULL, mname);
	gtk_widget_show (pf);

	create_reminders_page (pf);

	l = gtk_label_new (_("Reminders"));
	gnome_property_box_append_page (GNOME_PROPERTY_BOX (d), pf, l);

	gtk_widget_show (d);

	return 0;
}

int
main (int argc, char **argv)
{
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init ("moniker-test", "0.0", argc, argv);

	if ((oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");

	if (bonobo_init (NULL, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");

	gtk_idle_add ((GtkFunction) create_dialog, NULL);

	bonobo_main ();

	exit (0);
}


