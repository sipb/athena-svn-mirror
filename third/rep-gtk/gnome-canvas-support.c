/* gnome-canvas-support.c -- helper functions for GNOME Canvas binding
   $Id: gnome-canvas-support.c,v 1.1.1.2 2003-01-05 00:30:06 ghudson Exp $ */

#include <config.h>
#include <assert.h>
#include <gnome.h>
#include "rep-gtk.h"
#include "rep-gnome.h"
#include <string.h>

static int
list_length (repv list)
{
    repv len = Flength (list);
    return (len && rep_INTP (len)) ? rep_INT (len) : 0;
}

DEFUN ("gnome-canvas-item-new", Fgnome_canvas_item_new,
       Sgnome_canvas_item_new, (repv p_group, repv type_sym, repv scm_args),
       rep_Subr3)
{
#ifdef XXX_FINISHED_PORTING
    /* from guile-gnome/gnomeg.c */

  GnomeCanvasItem* cr_ret;
  GnomeCanvasGroup* c_group;

  int n_args;
  sgtk_object_info *info;
  GParameter *args;
  GObjectClass *objclass;

  rep_DECLARE (1, p_group, sgtk_is_a_gtkobj (gnome_canvas_group_get_type (), p_group));
  rep_DECLARE (2, type_sym, rep_SYMBOLP(type_sym));
  n_args = list_length (scm_args);
  rep_DECLARE (3, scm_args, n_args >= 0 && (n_args%2) == 0);
  n_args = n_args/2;

  info = sgtk_find_object_info (rep_STR(rep_SYM(type_sym)->name));
  rep_DECLARE (2, type_sym, info != NULL);

  c_group = (GnomeCanvasGroup*)sgtk_get_gtkobj (p_group);
  objclass = g_type_class_ref (info->header.type);
  args = sgtk_build_args (objclass, &n_args, scm_args,
			  (char *) &Sgnome_canvas_item_new);
  cr_ret = gnome_canvas_item_new (c_group, info->header.type, NULL);
  gnome_canvas_item_construct (c_group, info->header.type, 
n_args, args);
  sgtk_free_args (args, n_args);
  g_type_class_unref (objclass);

  return sgtk_wrap_gtkobj ((GtkObject*)cr_ret);
#else
  return Qnil;
#endif
}

DEFUN ("gnome-canvas-item-set", Fgnome_canvas_item_set,
       Sgnome_canvas_item_set, (repv p_item, repv scm_args), rep_Subr2)
{
#ifdef XXX_FINISHED_PORTING
    /* from guile-gnome/gnomeg.c */

  GnomeCanvasItem* c_item;
  int n_args;
  sgtk_object_info *info;
  GParameter *args;
  GObjectClass *objclass;

  rep_DECLARE (1, p_item, sgtk_is_a_gtkobj (gnome_canvas_item_get_type (), p_item));
  n_args = list_length (scm_args);
  rep_DECLARE (2, scm_args, n_args >= 0 && (n_args%2) == 0);
  n_args = n_args/2;

  c_item = (GnomeCanvasItem*)sgtk_get_gtkobj (p_item);
  info = sgtk_find_object_info_from_type (GTK_OBJECT_TYPE(c_item));
  rep_DECLARE (1, p_item, info != NULL);
  
  objclass = g_type_class_ref (info->header.type);
  args = sgtk_build_args (objclass, &n_args, scm_args, /* XXX p_item, */
			  (char *) &Sgnome_canvas_item_set);
  gnome_canvas_item_setv (c_item, n_args, args);
  sgtk_free_args (args, n_args);
  g_type_class_unref (objclass);
#endif
  return Qnil;
}

GnomeCanvasPoints *
sgtk_gnome_canvas_points_new (repv data)
{
    repv len = Flength (data);
    if (len && rep_INT (len) % 2 == 0)
    {
	int i, count = rep_INT (len);
	GnomeCanvasPoints *p = gnome_canvas_points_new (count / 2);
	if (rep_CONSP (data))
	{
	    for (i = 0; i < count; i++)
	    {
		p->coords[i] = sgtk_rep_to_double (rep_CAR (data));
		data = rep_CDR (data);
	    }
	}
	else if (rep_VECTORP (data))
	{
	    for (i = 0; i < count; i++)
		p->coords[i] = sgtk_rep_to_double (rep_VECTI (data, i));
	}
	return p;
    }
    else
	return 0;
}

repv
sgtk_gnome_canvas_points_conversion (repv arg)
{
    extern repv Fgnome_canvas_points_new (repv);

    if (rep_LISTP (arg) || rep_VECTORP (arg))
	return Fgnome_canvas_points_new (arg);
    else
	return arg;
}


/* dl hooks / init */

repv
rep_dl_init (void)
{
    repv s = rep_push_structure ("gui.gtk-2.gnome-canvas");
    sgtk_gnome_init_gnome_canvas_glue ();
    gdk_rgb_init ();
    rep_ADD_SUBR (Sgnome_canvas_item_new);
    rep_ADD_SUBR (Sgnome_canvas_item_set);
    return rep_pop_structure (s);
}