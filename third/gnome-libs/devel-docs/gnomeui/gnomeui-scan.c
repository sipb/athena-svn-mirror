#include <string.h>
#include <stdio.h>

#include <libgnomeui/libgnomeui.h>

GtkType object_types[54];

GtkType *
get_object_types ()
{
    gint i = 0;
    object_types[i++] = gnome_about_get_type ();
    object_types[i++] = gnome_animator_get_type ();
    object_types[i++] = gnome_app_get_type ();
    object_types[i++] = gnome_appbar_get_type ();
    object_types[i++] = gnome_calculator_get_type ();
    object_types[i++] = gnome_canvas_ellipse_get_type ();
    object_types[i++] = gnome_canvas_get_type ();
    object_types[i++] = gnome_canvas_group_get_type ();
    object_types[i++] = gnome_canvas_image_get_type ();
    object_types[i++] = gnome_canvas_item_get_type ();
    object_types[i++] = gnome_canvas_line_get_type ();
    object_types[i++] = gnome_canvas_polygon_get_type ();
    object_types[i++] = gnome_canvas_re_get_type ();
    object_types[i++] = gnome_canvas_rect_get_type ();
    object_types[i++] = gnome_canvas_text_get_type ();
    object_types[i++] = gnome_canvas_widget_get_type ();
    object_types[i++] = gnome_client_get_type ();
    object_types[i++] = gnome_color_picker_get_type ();
    object_types[i++] = gnome_date_edit_get_type ();
    object_types[i++] = gnome_dentry_edit_get_type ();
    object_types[i++] = gnome_dialog_get_type ();
    object_types[i++] = gnome_dock_band_get_type ();
    object_types[i++] = gnome_dock_get_type ();
    object_types[i++] = gnome_dock_item_get_type ();
    object_types[i++] = gnome_dock_layout_get_type ();
    object_types[i++] = gnome_entry_get_type ();
    object_types[i++] = gnome_file_entry_get_type ();
    object_types[i++] = gnome_font_picker_get_type ();
    object_types[i++] = gnome_font_selector_get_type ();
    object_types[i++] = gnome_guru_get_type ();
    object_types[i++] = gnome_href_get_type ();
    object_types[i++] = gnome_icon_entry_get_type ();
    object_types[i++] = gnome_icon_list_get_type ();
    object_types[i++] = gnome_icon_selection_get_type ();
    object_types[i++] = gnome_icon_text_item_get_type ();
    object_types[i++] = gnome_less_get_type ();
    object_types[i++] = gnome_mdi_child_get_type ();
    object_types[i++] = gnome_mdi_generic_child_get_type ();
    object_types[i++] = gnome_mdi_get_type ();
    object_types[i++] = gnome_message_box_get_type ();
    object_types[i++] = gnome_number_entry_get_type ();
    object_types[i++] = gnome_paper_selector_get_type ();
    object_types[i++] = gnome_pixmap_entry_get_type ();
    object_types[i++] = gnome_pixmap_get_type ();
    object_types[i++] = gnome_proc_bar_get_type ();
    object_types[i++] = gnome_property_box_get_type ();
    object_types[i++] = gnome_scores_get_type ();
    object_types[i++] = gnome_spell_get_type ();
    object_types[i++] = gnome_stock_get_type ();
    object_types[i++] = gtk_clock_get_type ();
    object_types[i++] = gtk_dial_get_type ();
    object_types[i++] = gtk_pixmap_menu_item_get_type ();
    object_types[i++] = gtk_ted_get_type ();
    object_types[i] = 0;

    return object_types;
}

/*
 * This uses GTK type functions to output signal prototypes and the widget
 * hierarchy.
 */

/* The output files */
gchar *signals_filename = "gnomeui.signals";
gchar *hierarchy_filename = "gnomeui.hierarchy";


extern GtkType* get_object_types();

static void output_signals ();
static void output_widget_signals (FILE *fp,
				   GtkType object_type);
static void output_widget_signal (FILE *fp,
				  GtkType object_type,
				  gchar *object_class_name,
				  guint signal_id);
static gchar * get_type_name (GtkType type,
			      gboolean * is_pointer);
static gchar * get_gdk_event (const gchar * signal_name);
static gchar ** lookup_signal_arg_names (gchar * type,
					 const gchar * signal_name);

static void output_widget_hierarchy ();
static void output_hierarchy (FILE *fp,
			      GtkType type,
			      gint level);


int
main (int argc, char *argv[])
{
  gtk_init (&argc, &argv);

  output_signals ();
  output_widget_hierarchy ();

  return 0;
}


static void
output_signals ()
{
  FILE *fp;
  GtkType *object_types;
  gint i;

  fp = fopen (signals_filename, "w");
  if (fp == NULL)
    {
      g_warning ("Couldn't open output file: %s", signals_filename);
      return;
    }

  object_types = get_object_types();

  for (i = 0; object_types[i]; i++)
    output_widget_signals (fp, object_types[i]);

  fclose (fp);
}


/* This outputs all the signals of one widget. */
static void
output_widget_signals (FILE *fp, GtkType object_type)
{
  GtkObjectClass *class;
  gchar *object_class_name;
  guint sig;

  class = gtk_type_class (object_type);
  if (!class || class->nsignals == 0)
    return;

  object_class_name = gtk_type_name (object_type);

  for (sig = 0; sig < class->nsignals; sig++)
    {
      if (!class->signals[sig])
        {
          /*g_print ("Signal slot [%u] is empty
", sig);*/
          continue;
        }

      output_widget_signal (fp, object_type, object_class_name,
			    class->signals[sig]);
    }
}


/* This outputs one signal. */
static void
output_widget_signal (FILE *fp,
		      GtkType object_type,
		      gchar *object_name,
		      guint signal_id)
{
  GtkSignalQuery *query_info;
  gchar *ret_type, *pos, *type_name, *arg_name, *object_arg, *object_arg_start;
  gboolean is_pointer;
  gchar ret_type_buffer[1024], buffer[1024];
  gint i, param;
  gchar **arg_names;
  gint param_num, widget_num, event_num, callback_num;
  gint *arg_num;
  gchar signal_name[128];


  /*  g_print ("Object: %s Type: %i Signal: %u
", object_name, object_type,
      signal_id);*/

  param_num = 1;
  widget_num = event_num = callback_num = 0;

  query_info = gtk_signal_query (signal_id);
  if (query_info == NULL)
    {
      g_warning ("Couldn't query signal");
      return;
    }

  /* Output the return type and function name. */
  ret_type = get_type_name (query_info->return_val, &is_pointer);
  sprintf (ret_type_buffer, "%s%s", ret_type, is_pointer ? "*" : "");

  /* Output the signal object type and the argument name. We assume the
     type is a pointer - I think that is OK. We remove "Gtk" or "Gnome" and
     convert to lower case for the argument name. */
  pos = buffer;
  sprintf (pos, "%s ", object_name);
  pos += strlen (pos);

  if (!strncmp (object_name, "Gtk", 3))
      object_arg = object_name + 3;
  else if (!strncmp (object_name, "Gnome", 5))
      object_arg = object_name + 5;
  else
      object_arg = object_name;

  object_arg_start = pos;
  sprintf (pos, "*%s
", object_arg);
  pos += strlen (pos);
  g_strdown (object_arg_start);
  if (!strcmp (object_arg_start, "widget"))
    widget_num++;
  
  /* Convert signal name to use underscores rather than dashes '-'. */
  strcpy (signal_name, query_info->signal_name);
  for (i = 0; signal_name[i]; i++)
    {
      if (signal_name[i] == '-')
	signal_name[i] = '_';
    }

  /* Output the signal parameters. */
  arg_names = lookup_signal_arg_names (object_name, signal_name);

  for (param = 0; param < query_info->nparams; param++)
    {
      if (arg_names)
	{
	  sprintf (pos, "%s
", arg_names[param]);
	  pos += strlen (pos);
	}
      else
	{
	  type_name = get_type_name (query_info->params[param], &is_pointer);

	  /* Most arguments to the callback are called "arg1", "arg2", etc.
	     GdkWidgets are called "widget", "widget2", ...
	     GdkEvents are called "event", "event2", ...
	     GtkCallbacks are called "callback", "callback2", ... */
	  if (!strcmp (type_name, "GtkWidget"))
	    {
	      arg_name = "widget";
	      arg_num = &widget_num;
	    }
	  else if (!strcmp (type_name, "GdkEvent"))
	    {
	      type_name = get_gdk_event (signal_name);
	      arg_name = "event";
	      arg_num = &event_num;
	      is_pointer = TRUE;
	    }
	  else if (!strcmp (type_name, "GtkCallback")
		   || !strcmp (type_name, "GtkCCallback"))
	    {
	      arg_name = "callback";
	      arg_num = &callback_num;
	    }
	  else
	    {
	      arg_name = "arg";
	      arg_num = &param_num;
	    }
	  sprintf (pos, "%s ", type_name);
	  pos += strlen (pos);

	  if (!arg_num || *arg_num == 0)
	    sprintf (pos, "%s%s
", is_pointer ? "*" : " ", arg_name);
	  else
	    sprintf (pos, "%s%s%i
", is_pointer ? "*" : " ", arg_name,
		     *arg_num);
	      pos += strlen (pos);
	      
	      if (arg_num)
		*arg_num += 1;
	}
    }
  
  fprintf (fp,
	   "<SIGNAL>
<NAME>%s::%s</NAME>
<RETURNS>%s</RETURNS>
%s</SIGNAL>

",
	   object_name, query_info->signal_name, ret_type_buffer, buffer);
  g_free (query_info);
}


static gchar *
get_type_name (GtkType type, gboolean * is_pointer)
{
  static gchar *GbTypeNames[] =
  {
    "char", "gchar",
    "bool", "gboolean",
    "int", "gint",
    "uint", "guint",
    "long", "glong",
    "ulong", "gulong",
    "float", "gfloat",
    "double", "gdouble",
    "string", "gchar",
    "enum", "gint",
    "flags", "gint",
    "boxed", "gpointer",
    "foreign", "gpointer",
    "callback", "GtkCallback",	/* ?? */
    "args", "gpointer",

    "pointer", "gpointer",
    "signal", "gpointer",
    "c_callback", "GtkCallback",	/* ?? */

    NULL
  };

  GtkType parent_type;
  gchar *type_name, *parent_type_name;
  gint i;

  *is_pointer = FALSE;
  type_name = gtk_type_name (type);
  for (i = 0; GbTypeNames[i]; i += 2)
    {
      if (!strcmp (type_name, GbTypeNames[i]))
	{
	  if (!strcmp (type_name, "string"))
	    *is_pointer = TRUE;
	  return GbTypeNames[i + 1];
	}
    }

  for (;;)
    {
      parent_type = gtk_type_parent (type);
      if (parent_type == 0)
	break;
      type = parent_type;
    }
  parent_type_name = gtk_type_name (type);
  /*g_print ("Parent type name: %s
", parent_type_name);*/
  if (!strcmp (parent_type_name, "GtkObject")
      || !strcmp (parent_type_name, "boxed")
      || !strcmp (parent_type_name, "GtkBoxed"))
    *is_pointer = TRUE;

  return type_name;
}


static gchar *
get_gdk_event (const gchar * signal_name)
{
  static gchar *GbGDKEvents[] =
  {
    "button_press_event", "GdkEventButton",
    "button_release_event", "GdkEventButton",
    "motion_notify_event", "GdkEventMotion",
    "delete_event", "GdkEvent",
    "destroy_event", "GdkEvent",
    "expose_event", "GdkEventExpose",
    "key_press_event", "GdkEventKey",
    "key_release_event", "GdkEventKey",
    "enter_notify_event", "GdkEventCrossing",
    "leave_notify_event", "GdkEventCrossing",
    "configure_event", "GdkEventConfigure",
    "focus_in_event", "GdkEventFocus",
    "focus_out_event", "GdkEventFocus",
    "map_event", "GdkEvent",
    "unmap_event", "GdkEvent",
    "property_notify_event", "GdkEventProperty",
    "selection_clear_event", "GdkEventSelection",
    "selection_request_event", "GdkEventSelection",
    "selection_notify_event", "GdkEventSelection",
    "proximity_in_event", "GdkEventProximity",
    "proximity_out_event", "GdkEventProximity",
    "drag_begin_event", "GdkEventDragBegin",
    "drag_request_event", "GdkEventDragRequest",
    "drag_end_event", "GdkEventDragRequest",
    "drop_enter_event", "GdkEventDropEnter",
    "drop_leave_event", "GdkEventDropLeave",
    "drop_data_available_event", "GdkEventDropDataAvailable",
    "other_event", "GdkEventOther",
    "client_event", "GdkEventClient",
    "no_expose_event", "GdkEventNoExpose",
    NULL
  };

  gint i;

  for (i = 0; GbGDKEvents[i]; i += 2)
    {
      if (!strcmp (signal_name, GbGDKEvents[i]))
	return GbGDKEvents[i + 1];
    }
  return "GdkEvent";
}


/* This returns argument names to use for some known GTK signals.
    It is passed a widget name, e.g. 'GtkCList' and a signal name, e.g.
    'select_row' and it returns a pointer to an array of argument types and
    names. */
static gchar **
lookup_signal_arg_names (gchar * type, const gchar * signal_name)
{
  /* Each arg array starts with the object type name and the signal name,
     and then signal arguments follow. */
  static gchar *GbArgTable[][16] =
  {
    {"GtkCList", "select_row",
     "gint             row",
     "gint             column",
     "GdkEventButton  *event"},
    {"GtkCList", "unselect_row",
     "gint             row",
     "gint             column",
     "GdkEventButton  *event"},
    {"GtkCList", "click_column",
     "gint             column"},

    {"GtkCList", "resize_column",
     "gint             column",
     "gint             width"},

    {"GtkCList", "extend_selection",
     "GtkScrollType    scroll_type",
     "gfloat           position",
     "gboolean         auto_start_selection"},
    {"GtkCList", "scroll_vertical",
     "GtkScrollType    scroll_type",
     "gfloat           position"},
    {"GtkCList", "scroll_horizontal",
     "GtkScrollType    scroll_type",
     "gfloat           position"},
    {"GtkContainer", "focus",
     "GtkDirectionType direction"},
    {"GtkCTree", "tree_select_row",
     "GList           *node",
     "gint             column"},
    {"GtkCTree", "tree_unselect_row",
     "GList           *node",
     "gint             column"},

    {"GtkCTree", "tree_expand",
     "GList           *node"},
    {"GtkCTree", "tree_collapse",
     "GList           *node"},
    {"GtkCTree", "tree_move",
     "GList           *node",
     "GList           *new_parent",
     "GList           *new_sibling"},
    {"GtkCTree", "change_focus_row_expansion",
     "GtkCTreeExpansionType expansion"},

    {"GtkEditable", "insert_text",
     "gchar           *new_text",
     "gint             new_text_length",
     "gint            *position"},
    {"GtkEditable", "delete_text",
     "gint             start_pos",
     "gint             end_pos"},
    {"GtkEditable", "set_editable",
     "gboolean         is_editable"},
    {"GtkEditable", "move_cursor",
     "gint             x",
     "gint             y"},
    {"GtkEditable", "move_word",
     "gint             num_words"},
    {"GtkEditable", "move_page",
     "gint             x",
     "gint             y"},
    {"GtkEditable", "move_to_row",
     "gint             row"},
    {"GtkEditable", "move_to_column",
     "gint             column"},

    {"GtkEditable", "kill_char",
     "gint             direction"},
    {"GtkEditable", "kill_word",
     "gint             direction"},
    {"GtkEditable", "kill_line",
     "gint             direction"},


    {"GtkInputDialog", "enable_device",
     "gint             deviceid"},
    {"GtkInputDialog", "disable_device",
     "gint             deviceid"},

    {"GtkListItem", "extend_selection",
     "GtkScrollType    scroll_type",
     "gfloat           position",
     "gboolean         auto_start_selection"},
    {"GtkListItem", "scroll_vertical",
     "GtkScrollType    scroll_type",
     "gfloat           position"},
    {"GtkListItem", "scroll_horizontal",
     "GtkScrollType    scroll_type",
     "gfloat           position"},

    {"GtkMenuShell", "move_current",
     "GtkMenuDirectionType direction"},
    {"GtkMenuShell", "activate_current",
     "gboolean         force_hide"},


    {"GtkNotebook", "switch_page",
     "GtkNotebookPage *page",
     "gint             page_num"},
    {"GtkStatusbar", "text_pushed",
     "guint            context_id",
     "gchar           *text"},
    {"GtkStatusbar", "text_popped",
     "guint            context_id",
     "gchar           *text"},
    {"GtkTipsQuery", "widget_entered",
     "GtkWidget       *widget",
     "gchar           *tip_text",
     "gchar           *tip_private"},
    {"GtkTipsQuery", "widget_selected",
     "GtkWidget       *widget",
     "gchar           *tip_text",
     "gchar           *tip_private",
     "GdkEventButton  *event"},
    {"GtkToolbar", "orientation_changed",
     "GtkOrientation   orientation"},
    {"GtkToolbar", "style_changed",
     "GtkToolbarStyle  style"},
    {"GtkWidget", "draw",
     "GdkRectangle    *area"},
    {"GtkWidget", "size_request",
     "GtkRequisition  *requisition"},
    {"GtkWidget", "size_allocate",
     "GtkAllocation   *allocation"},
    {"GtkWidget", "state_changed",
     "GtkStateType     state"},
    {"GtkWidget", "style_set",
     "GtkStyle        *previous_style"},

    {"GtkWidget", "install_accelerator",
     "gchar           *signal_name",
     "gchar            key",
     "gint             modifiers"},

    {"GtkWidget", "add_accelerator",
     "guint            accel_signal_id",
     "GtkAccelGroup   *accel_group",
     "guint            accel_key",
     "GdkModifierType  accel_mods",
     "GtkAccelFlags    accel_flags"},

    {"GtkWidget", "parent_set",
     "GtkObject       *old_parent"},

    {"GtkWidget", "remove_accelerator",
     "GtkAccelGroup   *accel_group",
     "guint            accel_key",
     "GdkModifierType  accel_mods"},
    {"GtkWidget", "debug_msg",
     "gchar           *message"},
    {"GtkWindow", "move_resize",
     "gint            *x",
     "gint            *y",
     "gint             width",
     "gint             height"},
    {"GtkWindow", "set_focus",
     "GtkWidget       *widget"},

    {"GtkWidget", "selection_get",
     "GtkSelectionData *data",
     "guint            info",
     "guint            time"},
    {"GtkWidget", "selection_received",
     "GtkSelectionData *data",
     "guint            time"},

    {"GtkWidget", "drag_begin",
     "GdkDragContext  *drag_context"},
    {"GtkWidget", "drag_end",
     "GdkDragContext  *drag_context"},
    {"GtkWidget", "drag_data_delete",
     "GdkDragContext  *drag_context"},
    {"GtkWidget", "drag_leave",
     "GdkDragContext  *drag_context",
     "guint            time"},
    {"GtkWidget", "drag_motion",
     "GdkDragContext  *drag_context",
     "gint             x",
     "gint             y",
     "guint            time"},
    {"GtkWidget", "drag_drop",
     "GdkDragContext  *drag_context",
     "gint             x",
     "gint             y",
     "guint            time"},
    {"GtkWidget", "drag_data_get",
     "GdkDragContext  *drag_context",
     "GtkSelectionData *data",
     "guint            info",
     "guint            time"},
    {"GtkWidget", "drag_data_received",
     "GdkDragContext  *drag_context",
     "gint             x",
     "gint             y",
     "GtkSelectionData *data",
     "guint            info",
     "guint            time"},

    {NULL}
  };

  gint i;

  for (i = 0; GbArgTable[i][0]; i++)
    {
      if (!strcmp (type, GbArgTable[i][0])
	  && !strcmp (signal_name, GbArgTable[i][1]))
	return &GbArgTable[i][2];
    }
  return NULL;
}


/* This outputs the hierarchy of all widgets which have been initialized,
   i.e. by calling their XXX_get_type() initialization function. */
static void
output_widget_hierarchy ()
{
  FILE *fp;

  fp = fopen (hierarchy_filename, "w");
  if (fp == NULL)
    {
      g_warning ("Couldn't open output file: %s", hierarchy_filename);
      return;
    }
  output_hierarchy (fp, GTK_TYPE_OBJECT, 0);
  fclose (fp);
}


/* This is called recursively to output the hierarchy of a widget. */
static void
output_hierarchy (FILE *fp,
		  GtkType type,
		  gint level)
{
  GList *list;
  guint i;

  if (!type)
    return;

  for (i = 0; i < level; i++)
    fprintf (fp, "  ");
  fprintf (fp, gtk_type_name (type));
  fprintf (fp, "
");

  list = gtk_type_children_types (type);

  while (list)
    {
      GtkType child = (GtkType) list->data;
      output_hierarchy (fp, child, level + 1);
      list = list->next;
    }
}
