#include <gtk/gtk.h>
#include <string.h>

#include "lighthouseblue_style.h"
#include "lighthouseblue_rc_style.h"

#include "util.c"

#define DETAIL(xx)   ((detail) && (!strcmp(xx, detail)))

static void lighthouseblue_style_init       (LighthouseBlueStyle      *style);
static void lighthouseblue_style_class_init (LighthouseBlueStyleClass *klass);

static GtkStyleClass *parent_class = NULL;


static void draw_hline (GtkStyle *style,
						GdkWindow *window,
						GtkStateType state_type,
						GdkRectangle *area, GtkWidget *widget,
						const gchar *detail, gint x1, gint x2, gint y)
{	
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	if (area)
    {
		gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
    }
  
	gdk_draw_line (window, style->dark_gc[state_type], x1, y, x2, y);
	
	if (DETAIL ("menuitem"))
	{
		gdk_draw_line (window, style->light_gc[state_type], x1, y + 1, x2, y + 1);	
	}
	  
	if (area)
    {
		gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
    }		
}


static void draw_vline (GtkStyle *style,
						GdkWindow *window,
						GtkStateType state_type,
						GdkRectangle *area,
						GtkWidget *widget,
						const gchar *detail, gint y1, gint y2, gint x)
{
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	if (area)
    {
		gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
    }
  
	gdk_draw_line (window, style->dark_gc[state_type], x, y1, x, y2);
  
	if (area)
    {
		gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
    }	
}

static void draw_shadow (GtkStyle *style,
						 GdkWindow *window,
						 GtkStateType state_type,
						 GtkShadowType shadow_type,
						 GdkRectangle *area,
						 GtkWidget *widget,
						 const gchar *detail,
						 gint x, gint y, gint width, gint height)
{
	GdkGC *gc1;
	GdkGC *gc2;
	GdkGC *outer_gc;
	GdkGC *bg_gc;
	
	gint xthickness = 1;
	gint ythickness = 1;
	
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	if (DETAIL ("buttondefault"))
	{
		return;			
	}
	
	sanitize_size (window, &width, &height);

	if (DETAIL ("button") || DETAIL ("optionmenu"))
	{
		outer_gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6];
	}
	else
	{
		outer_gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[5];
	}
	
	if (GTK_IS_WIDGET (widget) && gtk_widget_get_parent (widget) != NULL)
	{
		bg_gc = (gtk_widget_get_parent (widget))->style->bg_gc[GTK_STATE_NORMAL];
	}
	else
	{
		bg_gc = style->bg_gc[GTK_STATE_NORMAL];
	}
	
	switch (shadow_type)
	{
		case GTK_SHADOW_NONE:
			return;
		case GTK_SHADOW_IN:
		case GTK_SHADOW_ETCHED_IN:
			gc1 = style->light_gc[state_type]; /* white_gc ? */
			gc2 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2];
			break;
		case GTK_SHADOW_OUT:
		case GTK_SHADOW_ETCHED_OUT:
			gc1 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2];
			gc2 = style->light_gc[state_type]; /* white_gc ? */
			break;
	}

	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, area);
		gdk_gc_set_clip_rectangle (gc2, area);
		gdk_gc_set_clip_rectangle (bg_gc, area);
		
		if (shadow_type == GTK_SHADOW_IN || shadow_type == GTK_SHADOW_OUT)
		{
			gdk_gc_set_clip_rectangle (style->black_gc, area);
			gdk_gc_set_clip_rectangle (outer_gc, area);
		}
	}

	switch (shadow_type)
	{
		case GTK_SHADOW_NONE:
			return;
		
		case GTK_SHADOW_IN:
			theme_draw_rectangle (window, outer_gc, FALSE, x, y, width - 1, height - 1);
			gdk_draw_point (window, bg_gc, x, y);
			gdk_draw_point (window, bg_gc, x + width - 1, y);
			gdk_draw_point (window, bg_gc, x, y + height - 1);
			gdk_draw_point (window, bg_gc, x + width - 1, y + height - 1);
		
			/* Light around right and bottom edge */
			if (ythickness > 0)
			{
				gdk_draw_line (window, gc1, x + 1, y + height - 2, x + width - 2, y + height - 2);
			}
			
			if (xthickness > 0)
			{
				gdk_draw_line (window, gc1, x + width - 2, y + 1, x + width - 2, y + height - 2);
			}

			/* Dark around left and top */
			if (ythickness > 0)
			{
				gdk_draw_line (window, gc2, x + 1, y + 1, x + width - 3, y + 1);
			}
			
			if (xthickness > 0)
			{
				gdk_draw_line (window, gc2, x + 1, y + 1, x + 1, y + height - 3);
			}
			break;
			
		case GTK_SHADOW_OUT:
			theme_draw_rectangle (window, outer_gc, FALSE, x, y, width - 1, height - 1);
			gdk_draw_point (window, bg_gc, x, y);
			gdk_draw_point (window, bg_gc, x + width - 1, y);
			gdk_draw_point (window, bg_gc, x, y + height - 1);
			gdk_draw_point (window, bg_gc, x + width - 1, y + height - 1);

			/* Dark around right and bottom edge */
			if (ythickness > 0)
			{
				gdk_draw_line (window, gc1, x + 2, y + height - 2, x + width - 2, y + height - 2);
			}
			
			if (xthickness > 0)
			{
				gdk_draw_line (window, gc1, x + width - 2, y + 2, x + width - 2, y + height - 2);
			}
	  
			/* Light around top and left */
			if (ythickness > 0)
			{
				gdk_draw_line (window, gc2, x + 1, y + 1, x + width - 2, y + 1);
			}
			
			if (xthickness > 0)
			{
				gdk_draw_line (window, gc2, x + 1, y + 1, x + 1, y + height - 2);
			}
			break;
			
		case GTK_SHADOW_ETCHED_IN:
		case GTK_SHADOW_ETCHED_OUT:
			gdk_draw_line (window, gc1, x + 1, y + height - 1, x + width - 3, y + height - 1);
			gdk_draw_line (window, gc1, x + width - 1, y + 1, x + width - 1, y + height - 3);

			gdk_draw_line (window, gc2, x + 1, y, x + width - 3, y);
			gdk_draw_line (window, gc2, x, y + 1, x, y + height - 3);

			gdk_draw_line (window, gc1, x + 1, y + 1, x + width - 2, y + 1);
			gdk_draw_line (window, gc1, x + 1, y + 1, x + 1, y + height - 2);

			gdk_draw_line (window, gc2, x + 1, y + height - 2, x + width - 3, y + height - 2);
			gdk_draw_line (window, gc2, x + width - 2, y + 1, x + width - 2, y + height - 3);
			break;
	}
	
	if (DETAIL ("button") && GTK_WIDGET_HAS_DEFAULT (widget))/* && !GTK_WIDGET_HAS_FOCUS (widget))*/
	{
		GdkGC *gc = style->base_gc[GTK_STATE_SELECTED];/*(LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6];*/
		theme_draw_rectangle (window, gc, FALSE, x + 3, y + 3, width - 7, height - 7);
	}
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, NULL);
		gdk_gc_set_clip_rectangle (gc2, NULL);
		gdk_gc_set_clip_rectangle (bg_gc, NULL);
		
		if (shadow_type == GTK_SHADOW_IN || shadow_type == GTK_SHADOW_OUT)
		{
			gdk_gc_set_clip_rectangle (style->black_gc, NULL);
			gdk_gc_set_clip_rectangle (outer_gc, NULL);
		}
	}
}

static void draw_arrow (GtkStyle *style,
						GdkWindow *window,
						GtkStateType state_type,
						GtkShadowType shadow_type,
						GdkRectangle *area,
						GtkWidget *widget,
						const gchar *detail,
						GtkArrowType arrow_type,
						gboolean fill, gint x, gint y, gint width, gint height)
{
	GdkGC *fg_gc;
	gint orig_width;
	gint orig_height;
	
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);
	
	if (DETAIL ("menuitem") && state_type == GTK_STATE_INSENSITIVE)
	{
		return;
	}
	
	sanitize_size (window, &width, &height);
	
	//fg_gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[7];
	fg_gc = style->fg_gc[state_type];

	if (DETAIL ("menuitem"))
    {
		height += 2;
		y -= 1;
    }
	
	calculate_arrow_geometry (arrow_type, &x, &y, &width, &height);
	
	theme_draw_arrow (window, fg_gc, area, arrow_type, x, y, width, height);
}

static void draw_box (GtkStyle *style,
					  GdkWindow *window,
					  GtkStateType state_type,
					  GtkShadowType shadow_type,
					  GdkRectangle *area,
					  GtkWidget *widget,
					  const gchar *detail,
					  gint x, gint y, gint width, gint height)
{
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	sanitize_size (window, &width, &height);
	
	if (DETAIL ("menubar") || DETAIL ("toolbar") || DETAIL ("dockitem_bin"))
	{
		GdkGC *gc;
		
		gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2];
		
		if (area)
		{
			gdk_gc_set_clip_rectangle (gc, area);
		}

		gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
											state_type, area, x, y, width, height);
		
		if (!DETAIL ("menubar")) /* Only toolbars have the top line */
		{
			gdk_draw_line (window, (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[0], x, y, x + width, y);
		}		
		gdk_draw_line (window, gc, x, y + height - 1, x + width, y + height - 1);
		
		
		if (area)
		{
			gdk_gc_set_clip_rectangle (gc, NULL);
		}
	}
	else if (DETAIL ("menuitem"))
	{
		if (area)
		{
			gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
		}
      
		gdk_draw_rectangle (window, style->bg_gc[state_type], TRUE, x + 1, y + 1, width - 2, height - 2);
		theme_draw_rectangle (window, style->bg_gc[state_type], FALSE, x, y, width - 1, height - 1);

		if (area)
		{
			gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
		}
	}
	else if (DETAIL ("trough"))
	{
		GdkGC *gc1;
		GdkGC *gc2;
		GdkGC *gc3;
		
		gc1 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[4];
		gc2 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6];
		gc3 = (gtk_widget_get_style (gtk_widget_get_parent (widget)))->bg_gc[GTK_STATE_NORMAL];

		if (area)
		{
			gdk_gc_set_clip_rectangle (gc1, area);
			gdk_gc_set_clip_rectangle (gc2, area);
			gdk_gc_set_clip_rectangle (gc3, area);
		}

		if (GTK_IS_HSCALE (widget))
		{
			y += (height / 2) - 2;
			height = 5;
		}
		if (GTK_IS_VSCALE (widget))
		{
			x += (width / 2) - 2;
			width = 5;
		}
		
		gdk_draw_rectangle (window, gc1, TRUE, x, y, width, height);
        gdk_draw_rectangle (window, gc3, FALSE, x, y, width - 1, height - 1);
		theme_draw_rectangle (window, gc2, FALSE, x, y, width - 1, height - 1);
		
		if (area)
		{
			gdk_gc_set_clip_rectangle (gc1, NULL);
			gdk_gc_set_clip_rectangle (gc2, NULL);
			gdk_gc_set_clip_rectangle (gc3, NULL);
		}

	}
	else if (DETAIL ("vscrollbar") || DETAIL ("hscrollbar"))
	{
		GdkGC *gc1;
		GdkGC *gc2;
		GdkGC *gc3;
		GdkGC *gc4;
		
		switch (state_type)
		{
			case GTK_STATE_ACTIVE:
				gc2 = style->light_gc[state_type];
				gc1 = style->dark_gc[state_type];
				break;
			default:
				gc1 = style->light_gc[state_type];
				gc2 = style->dark_gc[state_type];
				break;
		}

		gc3 = style->bg_gc[state_type];
		gc4 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6];
		
		if (area)
		{
			gdk_gc_set_clip_rectangle (gc1, area);
			gdk_gc_set_clip_rectangle (gc2, area);
			gdk_gc_set_clip_rectangle (gc3, area);
			gdk_gc_set_clip_rectangle (gc4, area);
		}
		
		gdk_draw_rectangle (window, gc3, TRUE, x + 1, y + 1,  width - 3, height - 3);

		gdk_draw_line (window, gc2, x + 2, y + height - 2, x + width - 2, y + height - 2);
		gdk_draw_line (window, gc2, x + width - 2, y + 2, x + width - 2, y + height - 2);
		gdk_draw_line (window, gc1, x + 1, y + 1, x + width - 2, y + 1);
		gdk_draw_line (window, gc1, x + 1, y + 1, x + 1, y + height - 2);
	
		if (is_stepper_a (widget, x, y))
		{
			if (DETAIL ("hscrollbar"))
			{
				gdk_draw_line (window, gc4, x + width - 1, y, x + width - 1, y + height - 1);
			}
			else
			{
				gdk_draw_line (window, gc4, x, y + height - 1, x + width - 2, y + height - 1);
			}
		}

		if (is_stepper_d (widget, x, y))
		{
			if (DETAIL ("hscrollbar"))
			{
				gdk_draw_line (window, gc4, x, y, x, y + height - 1);
			}
			else
			{
				gdk_draw_line (window, gc4, x, y, x + width - 2, y);
			}
		}

		if (area)
		{
			gdk_gc_set_clip_rectangle (gc1, NULL);
			gdk_gc_set_clip_rectangle (gc2, NULL);
			gdk_gc_set_clip_rectangle (gc3, NULL);
			gdk_gc_set_clip_rectangle (gc4, NULL);
		}			
	}
	else if (DETAIL ("spinbutton_up") || DETAIL ("spinbutton_down"))
    {
		/* Make the top button overlap the first line of the bottom button */
		if (strcmp (detail, "spinbutton_up") == 0)
		{
			height += 1;
		}
      
		gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
											state_type, area, x, y, width, height);
		gtk_paint_shadow (style, window, state_type, shadow_type, area, widget, detail,x, y, width, height);
    }
	else if (widget && DETAIL ("bar") && GTK_IS_PROGRESS_BAR (widget))
    {
		GtkProgressBarOrientation orientation;
      
		if (area)
		{
			gdk_gc_set_clip_rectangle (style->base_gc[GTK_STATE_SELECTED], area);
		}
      
		orientation = gtk_progress_bar_get_orientation (GTK_PROGRESS_BAR (widget));

		if (orientation == GTK_PROGRESS_LEFT_TO_RIGHT || GTK_PROGRESS_RIGHT_TO_LEFT)
		{
			if (width > 2)
			{
				width -= 1;
			}
		}
		else
		{
			if (height > 2)
			{
				height -= 1;
			}
		}

		gdk_draw_rectangle (window, style->base_gc[GTK_STATE_SELECTED], TRUE,
							x, y, width - 1,  height);
      
		if (area)
		{
			gdk_gc_set_clip_rectangle (style->base_gc[GTK_STATE_SELECTED], NULL);
		}
    }
	else
	{
		parent_class->draw_box (style, window, state_type, shadow_type, area,
								widget, detail, x, y, width, height);		
	}
}

static gint theme_on_configure (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	theme_generate_pixmap (widget, event->width, event->height);
	return FALSE;
}

static void draw_flat_box (GtkStyle *style,
						   GdkWindow *window,
						   GtkStateType state_type,
						   GtkShadowType shadow_type,
						   GdkRectangle *area,
						   GtkWidget *widget,
						   const gchar *detail,
						   gint x, gint y, gint width, gint height)
{
	GdkGC *gc1;
	GdkGC *freeme = NULL;

	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	sanitize_size (window, &width, &height);

	if (DETAIL ("base") && widget != NULL)
    {
		/*if (GTK_IS_MENU (gtk_bin_get_child (GTK_BIN (widget))))*/
		if ((GTK_WINDOW (widget)->type != GTK_WINDOW_TOPLEVEL)
			&& ( (GTK_IS_MENU (gtk_bin_get_child (GTK_BIN (widget)))) 
				|| (GTK_IS_LABEL (gtk_bin_get_child (GTK_BIN (widget)))) ))
		{
			g_signal_connect (GTK_OBJECT (widget), "configure_event", (GtkSignalFunc) theme_on_configure, NULL);
			parent_class->draw_flat_box (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);

			return;
		}
    }

  if (detail)
  {
	  if (state_type == GTK_STATE_SELECTED)
      {
		  if (DETAIL ("text"))
		  {
			  gc1 = style->bg_gc[GTK_STATE_SELECTED];
		  }
          else if (DETAIL ("cell_even") || DETAIL ("cell_odd"))
		  {
			  /* This has to be really broken; alex made me do it. -jrb */
			  if (GTK_WIDGET_HAS_FOCUS (widget))
				gc1 = style->base_gc[state_type];
			  else 
				gc1 = style->base_gc[GTK_STATE_ACTIVE];
          }
          else
          {
			  gc1 = style->bg_gc[state_type];
          }
      }
      else
      {
		  if (DETAIL ("viewportbin"))
	      {
			  gc1 = style->bg_gc[GTK_STATE_NORMAL];
		  }
          else if (DETAIL ("entry_bg"))
	      {
			  gc1 = style->base_gc[state_type];
	      }

          /* For trees: even rows are base color, odd rows are a shade of
           * the base color, the sort column is a shade of the original color
           * for that row.
           */

          /* FIXME when we have style properties, clean this up.
           */
          
          else if (!strcmp ("cell_even", detail) ||
                   !strcmp ("cell_odd", detail) ||
                   !strcmp ("cell_even_ruled", detail))
            {
	      gc1 = style->base_gc[state_type];
            }
          /*else if (!strcmp ("cell_even_sorted", detail) ||
                   !strcmp ("cell_odd_sorted", detail) ||
                   !strcmp ("cell_odd_ruled", detail) ||
                   !strcmp ("cell_even_ruled_sorted", detail))
            {
	      freeme = get_darkened_gc (window, &style->base[state_type], 1);
              gc1 = freeme;
            }
          else if (!strcmp ("cell_odd_ruled_sorted", detail))
            {
              freeme = get_darkened_gc (window, &style->base[state_type], 2);
              gc1 = freeme;
	      }*/
          else
	    {
	      gc1 = style->bg_gc[state_type];
	    }
      }
  }
  else
  {
      gc1 = style->bg_gc[state_type];
  }

  if (!style->bg_pixmap[state_type] || gc1 != style->bg_gc[state_type] ||
      GDK_IS_PIXMAP (window))
  {
    if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, area);
	}
    
	if (state_type == GTK_STATE_SELECTED)
	{
	  if (GTK_IS_TREE_VIEW (widget))
	  {
		  GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW (widget));
	      guint length = g_list_length (columns);
	      guint col_n = 1;

	      if (x > 0)
		  {
			  guint n;
			  gint cell_offset = 0;
			  for (n = 0; n < length - 1; n++)
		      {
				  GtkTreeViewColumn *column = g_list_nth_data (columns, n);
		          cell_offset += column->width;
		          if (x == cell_offset)
				  {
					  break;
				  }
		      }
		    col_n = n + 2;
		  }
	      g_list_free (columns);

	      if (length > 1)
		  {
		  if (col_n == 1)
		    {
		      gdk_draw_rectangle (window, gc1, TRUE, x + 3, y, width - 3, height);
		      theme_draw_rectangle (window, gc1, FALSE, x + 2, y, width - 3, height - 1);
		    }
		  else if (col_n == length)
		    {
		      gdk_draw_rectangle (window, gc1, TRUE, x, y, width - 3, height);
		      theme_draw_rectangle (window, gc1, FALSE, x, y, width - 3, height - 1);
		    }
		  else
		    {
		      gdk_draw_rectangle (window, gc1, TRUE, x, y, width, height);
		    }
		  }
	      else
		  {
		    gdk_draw_rectangle (window, gc1, TRUE, x + 3, y, width - 6, height - 1);
		    theme_draw_rectangle (window, gc1, FALSE, x + 2, y, width - 5, height - 1);
		  }
	  } /* IF GTK_IS_TREEVIEW */
	  else
	  {
	      gdk_draw_rectangle (window, gc1, TRUE, x + 3, y, width - 6, height - 1);
	      theme_draw_rectangle (window, gc1, FALSE, x + 2, y, width - 5, height - 1);
	  }
	}
    else /* NOT SELECTED */
	{
	  gdk_draw_rectangle (window, gc1, TRUE, x, y, width, height);
	}

    if (detail && !strcmp ("tooltip", detail))
	{
	  gdk_draw_rectangle (window, (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6], FALSE,
			              x, y, width - 1, height - 1);
	}

    if (area)
	{
	  gdk_gc_set_clip_rectangle (gc1, NULL);
	}
  }
  else
  {
      gtk_style_apply_default_background (style, window,
					  widget && !GTK_WIDGET_NO_WINDOW (widget),
					  state_type, area, x, y, width, height);
  }

  if (freeme)
    g_object_unref (G_OBJECT (freeme));
}
				 
static void draw_check (GtkStyle *style,
						GdkWindow *window,
						GtkStateType state_type,
						GtkShadowType shadow_type,
						GdkRectangle *area,
						GtkWidget *widget,
						const gchar *detail,
						gint x, gint y, gint width, gint height)
{
	GdkGC *gc1;
    GdkGC *gc2;
    GdkGC *gc3;
	GdkGC *gc4;
	
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);
	
	if (DETAIL ("check")) /* menuitem */
	{
		parent_class->draw_check (style, window, state_type, shadow_type, area, widget, detail,
								  x, y, width, height);
		return;
	}
	
	if (DETAIL ("cellcheck"))
	{
		theme_draw_rectangle (window, style->text_gc[state_type], FALSE, x, y, width, height);
		if (shadow_type == GTK_SHADOW_IN)
		{
			gdk_draw_rectangle (window, style->text_gc[state_type], TRUE, x + 3, y + 3, width - 5, height - 5);
		}

		return;
	}

	gc1 = style->light_gc[state_type];
	gc2 = style->dark_gc[state_type];
	gc3 = style->bg_gc[state_type];
	gc4 = style->fg_gc[GTK_STATE_SELECTED];
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, area);
		gdk_gc_set_clip_rectangle (gc2, area);
		gdk_gc_set_clip_rectangle (gc3, area);
		gdk_gc_set_clip_rectangle (gc4, area);
	}
	
	if (gtk_toggle_button_get_inconsistent (GTK_TOGGLE_BUTTON (widget)))
	{
		gdk_draw_line (window, gc1, x + 1, y + height, x + width - 2, y + height);
		gdk_draw_line (window, gc1, x + width, y + 1, x + width, y + height - 2);

		gdk_draw_line (window, gc2, x + 1, y, x + width - 2, y);
		gdk_draw_line (window, gc2, x, y + 1, x, y + height - 2);

		gdk_draw_line (window, gc1, x + 1, y + 1, x + width - 1, y + 1);
		gdk_draw_line (window, gc1, x + 1, y + 1, x + 1, y + height - 1);

		gdk_draw_line (window, gc2, x + 1, y + height - 1, x + width - 2, y + height - 1);
		gdk_draw_line (window, gc2, x + width - 1, y + 1, x + width - 1, y + height - 2);
	}
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{
		gdk_draw_rectangle (window, gc3, TRUE, x + 1, y + 1,  width - 1, height - 1);
		gdk_draw_line (window, gc2, x + 1, y, x + width - 1, y);
		gdk_draw_line (window, gc2, x, y + 1, x, y + height - 1);
		
		gdk_draw_line (window, gc1, x + 1, y + height, x + width - 1, y + height);
		gdk_draw_line (window, gc1, x + width, y + 1, x + width, y + height - 1);	
		gdk_draw_rectangle (window, gc4, TRUE, x + 3, y + 3,  width - 5, height - 5);
	}
	else		
	{
		gdk_draw_rectangle (window, gc3, TRUE, x + 1, y + 1,  width - 1, height - 1);
		gdk_draw_line (window, gc1, x + 1, y, x + width - 1, y);
		gdk_draw_line (window, gc1, x, y + 1, x, y + height - 1);
		
		gdk_draw_line (window, gc2, x + 1, y + height, x + width - 1, y + height);
		gdk_draw_line (window, gc2, x + width, y + 1, x + width, y + height - 1);		
	}
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, NULL);
		gdk_gc_set_clip_rectangle (gc2, NULL);
		gdk_gc_set_clip_rectangle (gc3, NULL);
		gdk_gc_set_clip_rectangle (gc4, NULL);
	}	
}

static void draw_option (GtkStyle *style,
						 GdkWindow *window,
						 GtkStateType state_type,
						 GtkShadowType shadow_type,
						 GdkRectangle *area,
						 GtkWidget *widget,
						 const gchar *detail,
						 gint x, gint y, gint width, gint height)
{
	GdkGC *gc1;
    GdkGC *gc2;
    GdkGC *gc3;
	GdkGC *gc4;
	
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	if (DETAIL ("option")) /* menuitem */
	{
		parent_class->draw_option (style, window, state_type, shadow_type, area, widget, detail,
								   x, y, width, height);
		return;
	}
	
	if (DETAIL ("cellradio"))
	{
		
		gdk_draw_arc(window, style->text_gc[state_type], FALSE, x, y, width, height, 0, 360 * 64);
		gdk_draw_arc(window, style->text_gc[state_type], TRUE, x + 2, y + 2, width - 4, height - 4, 0, 360 * 64);
		
		return;
	}
	
	gc1 = style->light_gc[state_type];
	gc2 = style->dark_gc[state_type];
	gc3 = style->bg_gc[state_type];
	gc4 = style->fg_gc[GTK_STATE_SELECTED];
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, area);
		gdk_gc_set_clip_rectangle (gc2, area);
		gdk_gc_set_clip_rectangle (gc3, area);
		gdk_gc_set_clip_rectangle (gc4, area);
	}
	
	if (gtk_toggle_button_get_inconsistent (GTK_TOGGLE_BUTTON (widget)))
	{
		gdk_draw_arc(window, gc1, FALSE, x+1, y+1, width, height, 0, 360 * 64);
		gdk_draw_arc(window, gc2, FALSE, x, y, width, height, 0, 360 * 64);
	}
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{
		gdk_draw_arc(window, gc3, TRUE, x, y, width, height, 0, 360 * 64);
		gdk_draw_arc(window, gc2, FALSE, x, y, width, height, 45 * 64, 225 * 64);
		gdk_draw_arc(window, gc1, FALSE, x, y, width, height, 225 * 64, 180 * 64);
		gdk_draw_arc(window, gc4, TRUE, x + 2, y + 2, width - 4, height - 4, 0, 360 * 64);
	}
	else
	{
		gdk_draw_arc(window, gc3, TRUE, x, y, width, height, 0, 360 * 64);
		gdk_draw_arc(window, gc1, FALSE, x, y, width, height, 45 * 64, 225 * 64);
		gdk_draw_arc(window, gc2, FALSE, x, y, width, height, 225 * 64, 180 * 64);
	}
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, NULL);
		gdk_gc_set_clip_rectangle (gc2, NULL);
		gdk_gc_set_clip_rectangle (gc3, NULL);
		gdk_gc_set_clip_rectangle (gc4, NULL);
	}
}

static void draw_tab (GtkStyle *style,
					  GdkWindow *window,
					  GtkStateType state_type,
					  GtkShadowType shadow_type,
					  GdkRectangle *area,
					  GtkWidget *widget,
					  const gchar *detail,
					  gint x, gint y, gint width, gint height)
{	
#define ARROW_SPACE 4
#define ARROW_LINE_HEIGHT 2
#define ARROW_LINE_WIDTH 5

	GtkRequisition indicator_size;
	GtkBorder indicator_spacing;
	gint arrow_height;
  
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	option_menu_get_props (widget, &indicator_size, &indicator_spacing);

	indicator_size.width += (indicator_size.width % 2) - 1;
	arrow_height = indicator_size.width / 2 + 1 - 3;

	x += (width - indicator_size.width) / 2;
	y += (height - (2 * arrow_height + ARROW_SPACE)) / 2;

	if (state_type == GTK_STATE_INSENSITIVE)
    {
		theme_draw_arrow (window, style->fg_gc[GTK_STATE_INSENSITIVE], area,
						  GTK_ARROW_RIGHT, x, y + 2,
						  indicator_size.width - 2, arrow_height);
    }
  
	theme_draw_arrow (window, (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[7], area,
					  GTK_ARROW_RIGHT, x, y + 2,
					  indicator_size.width - 2, arrow_height);	
}
				
static void draw_shadow_gap (GtkStyle *style,
							 GdkWindow *window,
							 GtkStateType state_type,
							 GtkShadowType shadow_type,
							 GdkRectangle *area,
							 GtkWidget *widget,
							 const gchar *detail,
							 gint x,
							 gint y,
							 gint width,
							 gint height,
							 GtkPositionType gap_side,
							 gint gap_x, gint gap_width)
{
	GdkGC *gc;
	
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);
	
	if (DETAIL ("frame"))
	{
		sanitize_size (window, &width, &height);
        gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[5];
		
		if (area)
		{
			gdk_gc_set_clip_rectangle (gc, area);
		}
		
		gdk_draw_line (window, gc, x, y + 1, x, y + height - 2);
		gdk_draw_line (window, gc, x + 1, y + height - 1, x + width - 2, y + height - 1);
		gdk_draw_line (window, gc, x + width - 1, y + 1, x + width - 1, y + height - 2);
		if (gap_x > 0)
		{
			gdk_draw_line (window, gc, x + 1, y, x + gap_x - 1, y);
		}
		if ((width - (gap_x + gap_width)) > 0)
		{
			gdk_draw_line (window, gc, x + gap_x + gap_width, y, x + width - 2, y);
		}
		
		if (area)
		{
			gdk_gc_set_clip_rectangle (gc, NULL);
		}

	}
	else
	{
		parent_class->draw_shadow_gap (style, window, state_type, shadow_type, area, widget, detail, 
									   x, y, width, height, gap_side, gap_x, gap_width);
		
	}
}

static void draw_box_gap (GtkStyle *style,
						  GdkWindow * window,
						  GtkStateType state_type,
						  GtkShadowType shadow_type,
						  GdkRectangle * area,
						  GtkWidget * widget,
						  const gchar * detail,
						  gint x,
						  gint y,
						  gint width,
						  gint height,
						  GtkPositionType gap_side, gint gap_x, gint gap_width)
{
	GdkGC *gc1 = NULL;
    GdkGC *gc2 = NULL;
    GdkGC *outer_gc = NULL;
	gint offset = 0;

	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	sanitize_size (window, &width, &height);
	
    if ((LIGHTHOUSEBLUE_RC_STYLE (style->rc_style))->has_notebook_patch)
    {
		offset = 1;
    }

	switch (gap_side)
    {
		case GTK_POS_LEFT:
		case GTK_POS_RIGHT:
			gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
											    state_type, area, x, y + 1, width, height - 2);  
			break;
		
		case GTK_POS_TOP:
		case GTK_POS_BOTTOM:
			gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
												state_type, area, x + 1, y, width - 2, height);
			break;
    }  

	switch (shadow_type)
    {
		case GTK_SHADOW_NONE:
			return;
		
		case GTK_SHADOW_IN:
		case GTK_SHADOW_ETCHED_IN:
			gc1 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2];
			gc2 = style->light_gc[state_type];/*style->white_gc;*/
			break;

		case GTK_SHADOW_OUT:
		case GTK_SHADOW_ETCHED_OUT:
			gc1 = style->light_gc[state_type];/*style->white_gc;*/
			gc2 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2];
			break;
    }

	outer_gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6];
	
	if (area)
    {
		gdk_gc_set_clip_rectangle (outer_gc, area);
        gdk_gc_set_clip_rectangle (gc1, area);
        gdk_gc_set_clip_rectangle (gc2, area);
    }

	switch (gap_side)
    {
		case GTK_POS_TOP:
			gdk_draw_line (window, outer_gc, x, y + offset, x, y + height - 2);

			gdk_draw_line (window, gc1, x + 1, y, x + 1, y + height - 2);
          
            gdk_draw_line (window, gc2, x + 1, y + height - 2, x + width - 2, y + height - 2);
            gdk_draw_line (window, gc2, x + width - 2, y, x + width - 2, y + height - 2);

            gdk_draw_line (window, outer_gc, x + 1, y + height - 1, x + width - 2, y + height - 1);
            gdk_draw_line (window, outer_gc, x + width - 1, y + offset, x + width - 1, y + height - 2);
          
		    if (gap_x > 0)
            {
				gdk_draw_line (window, outer_gc, x + offset, y, x + gap_x - 1, y);
                gdk_draw_line (window, gc1, x + 1, y + 1, x + gap_x - 1, y + 1);
            }
            if ((width - (gap_x + gap_width)) > 0)
            {
                gdk_draw_line (window, outer_gc, x + gap_x + gap_width, y, x + width - 2, y);
                gdk_draw_line (window, gc1, x + gap_x + gap_width, y + 1, x + width - 2, y + 1);
            }
			break;
			
        case  GTK_POS_BOTTOM:
            gdk_draw_line (window, outer_gc, x + 1, y, x + width - 2, y);
            gdk_draw_line (window, outer_gc, x, y + 1, x, y + height - 1 - offset);
            gdk_draw_line (window, gc1, x + 1, y + 1, x + width - 2, y + 1);
            gdk_draw_line (window, gc1, x + 1, y + 1, x + 1, y + height - 1);
          
            gdk_draw_line (window, gc2, x + width - 2, y + 1, x + width - 2, y + height - 1);
            gdk_draw_line (window, outer_gc, x + width - 1, y + 1, x + width - 1, y + height - 1 - offset);
            
		    if (gap_x > 0)
            {
                gdk_draw_line (window, outer_gc, x + offset, y + height - 1, x + gap_x - 1, y + height - 1);
                gdk_draw_line (window, gc2, x + 1, y + height - 2, x + gap_x - 1, y + height - 2);
            }
            if ((width - (gap_x + gap_width)) > 0)
            {
                gdk_draw_line (window, outer_gc, x + gap_x + gap_width, y + height - 1, x + width - 2, y + height - 1);
                gdk_draw_line (window, gc2, x + gap_x + gap_width, y + height - 2, x + width - 2, y + height - 2);
    	    }
            break;
			
        case GTK_POS_LEFT:
            gdk_draw_line (window, outer_gc, x + offset, y, x + width - 2, y);
            gdk_draw_line (window, gc1, x, y + 1, x + width - 2, y + 1);
          
            gdk_draw_line (window, gc2, x, y + height - 2, x + width - 2, y + height - 2);
            gdk_draw_line (window, gc2, x + width - 2, y + 1, x + width - 2, y + height - 2);
            gdk_draw_line (window, outer_gc, x + offset, y + height - 1, x + width - 2, y + height - 1);
            gdk_draw_line (window, outer_gc, x + width - 1, y + 1, x + width - 1, y + height - 2);
		
            if (gap_x > 0)
            {
                gdk_draw_line (window, outer_gc, x, y + offset, x, y + gap_x - 1);
                gdk_draw_line (window, gc1, x + 1, y + 1, x + 1, y + gap_x - 1);
            }
            if ((width - (gap_x + gap_width)) > 0)
            {
                gdk_draw_line (window, outer_gc, x, y + gap_x + gap_width, x, y + height - 2);
                gdk_draw_line (window, gc1, x + 1, y + gap_x + gap_width, x + 1, y + height - 2);
            }
            break;
			
        case GTK_POS_RIGHT:
            gdk_draw_line (window, outer_gc, x + 1, y, x + width - 1 - offset, y);
            gdk_draw_line (window, outer_gc, x, y + 1, x, y + height - 2);
            gdk_draw_line (window, gc1, x + 1, y + 1, x + width - 1, y + 1);
            gdk_draw_line (window, gc1, x + 1, y + 1, x + 1, y + height - 2);
          
            gdk_draw_line (window, gc2, x + 1, y + height - 2, x + width - 1, y + height - 2);
            gdk_draw_line (window, outer_gc, x + 1, y + height - 1, x + width - 1 - offset, y + height - 1);
          
		    if (gap_x > 0)
            {
                gdk_draw_line (window, outer_gc, x + width - 1, y + offset, x + width - 1, y + gap_x - 1);
                gdk_draw_line (window, gc2, x + width - 2, y + 1, x + width - 2, y + gap_x - 1);
            }
            if ((width - (gap_x + gap_width)) > 0)
            {
                gdk_draw_line (window, outer_gc, x + width - 1, y + gap_x + gap_width, x + width - 1, y + height - 2);
                gdk_draw_line (window, gc2, x + width - 2, y + gap_x + gap_width, x + width - 2, y + height - 2);
            }
            break;
    }

    if (area)
    {
        gdk_gc_set_clip_rectangle (outer_gc, NULL);
        gdk_gc_set_clip_rectangle (gc1, NULL);
        gdk_gc_set_clip_rectangle (gc2, NULL);
    }	
}

static void draw_extension (GtkStyle *style,
							GdkWindow *window,
							GtkStateType state_type,
							GtkShadowType shadow_type,
							GdkRectangle *area,
							GtkWidget *widget,
							const gchar *detail,
							gint x,
							gint y,
							gint width, gint height, GtkPositionType gap_side)
{
	GdkGC *gc1;
	GdkGC *gc2;
	GdkGC *outer_gc;
	GdkGC *bg_gc;
	gint xthickness = 2;
	gint ythickness = 2;
	gint d = 0;

	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	sanitize_size (window, &width, &height);
  
	outer_gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6];
	bg_gc = (gtk_widget_get_style (gtk_widget_get_parent (widget)))->bg_gc[GTK_STATE_NORMAL];
	
	switch (shadow_type)
	{
		case GTK_SHADOW_NONE:
			return;
		
		case GTK_SHADOW_IN:			
		case GTK_SHADOW_ETCHED_IN:
			gc1 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2];
			gc2 = style->light_gc[state_type];/*style->white_gc;*/
			break;

		case GTK_SHADOW_OUT:
		case GTK_SHADOW_ETCHED_OUT:
			gc1 = style->light_gc[state_type];/*style->white_gc;*/
			gc2 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2];
			break;
	}

	if (area)
    {
		gdk_gc_set_clip_rectangle (gc1, area);
		gdk_gc_set_clip_rectangle (gc2, area);
		gdk_gc_set_clip_rectangle (outer_gc, area);
		gdk_gc_set_clip_rectangle (bg_gc, area);
	}

	//Unselected tab should be drawn 2 pixels lower than selected tab
	if (state_type == GTK_STATE_ACTIVE)
    {
		d = 2;
    }

    switch (gap_side)
	{
		case GTK_POS_TOP:
			gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
                                                state_type, area,
                                                x + xthickness, y, 
                                                width - (2 * xthickness), 
                                                height - (ythickness) - d);

			gdk_draw_line (window, outer_gc, x, y, x, y + height - 2 - d);

			gdk_draw_line (window, gc1, x + 1, y, x + 1, y + height - 2 - d);
          
			gdk_draw_line (window, gc2, x + 2, y + height - 2 - d, x + width - 2, y + height - 2 - d);
			gdk_draw_line (window, gc2, x + width - 2, y, x + width - 2, y + height - 2 - d);

			gdk_draw_line (window, outer_gc, x + 1, y + height - 1 - d, x + width - 2, y + height - 1 - d);
			gdk_draw_line (window, outer_gc, x + width - 1, y, x + width - 1, y + height - 2 - d);

	        gdk_draw_point (window, bg_gc, x, y + height - 1 - d);
	        gdk_draw_point (window, bg_gc, x + width - 1, y + height - 1 - d);
            break;
		
        case GTK_POS_BOTTOM:
            gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
                                                state_type, area,
                                                x + xthickness, 
                                                y + d + ythickness, 
                                                width - (2 * xthickness), 
                                                height - (ythickness) - d);

            gdk_draw_line (window, outer_gc, x + 1, y + d, x + width - 2, y + d);
            gdk_draw_line (window, outer_gc, x, y + d + 1, x, y + height - 1);
          
	        gdk_draw_line (window, gc1, x + 1, y + d + 1, x + width - 2, y + d + 1);
            gdk_draw_line (window, gc1, x + 1, y + d + 1, x + 1, y + height - 1);
          
            gdk_draw_line (window, gc2, x + width - 2, y + d + 1, x + width - 2, y + height - 1);
          
	        gdk_draw_line (window, outer_gc, x + width - 1, y + d + 1, x + width - 1, y + height - 1);

	        gdk_draw_point (window, bg_gc, x, y + d);
	        gdk_draw_point (window, bg_gc, x + width - 1, y + d);
            break;
			
        case GTK_POS_LEFT:
            gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
                                                state_type, area,
                                                x, y + ythickness, 
                                                width - (xthickness) - d, 
                                                height - (2 * ythickness));

            gdk_draw_line (window, outer_gc, x, y, x + width - 2 - d, y);
            gdk_draw_line (window, gc1, x, y + 1, x + width - 2 - d, y + 1);
          
            gdk_draw_line (window, gc2, x, y + height - 2, x + width - 2 - d, y + height - 2);
            gdk_draw_line (window, gc2, x + width - 2 - d, y + 2, x + width - 2 - d, y + height - 2);

            gdk_draw_line (window, outer_gc, x, y + height - 1, x + width - 2 - d, y + height - 1);
            gdk_draw_line (window, outer_gc, x + width - 1 - d, y + 1, x + width - 1 - d, y + height - 2);

	        gdk_draw_point (window, bg_gc, x + width - 1 - d, y);
	        gdk_draw_point (window, bg_gc, x + width - 1 - d, y + height - 1);
            break; 
		
        case GTK_POS_RIGHT:
            gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
                                                state_type, area,
                                                x + xthickness + d, 
                                                y + ythickness, 
                                                width - (xthickness) - d, 
                                                height - (2 * ythickness));

            gdk_draw_line (window, outer_gc, x + 1 + d, y, x + width - 1, y);
            gdk_draw_line (window, outer_gc, x + d, y + 1, x + d, y + height - 2);

            gdk_draw_line (window, gc1, x + 1 + d, y + 1, x + width - 1, y + 1);
            gdk_draw_line (window, gc1, x + 1 + d, y + 1, x + 1 + d, y + height - 1);
          
            gdk_draw_line (window, gc2, x + 1 + d, y + height - 2, x + width - 1, y + height - 2);

            gdk_draw_line (window, outer_gc, x + 1 + d, y + height - 1, x + width - 1, y + height - 1);

	        gdk_draw_point (window, bg_gc, x + d, y);
	        gdk_draw_point (window, bg_gc, x + d, y + height - 1);
            break;
    }

	if (area)
    {
	  gdk_gc_set_clip_rectangle (bg_gc, NULL);		
      gdk_gc_set_clip_rectangle (outer_gc, NULL);
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
    }	
}

static void draw_focus (GtkStyle *style,
						GdkWindow *window,
						GtkStateType state_type,
						GdkRectangle *area,
						GtkWidget *widget,
						const gchar *detail,
						gint x, gint y, gint width, gint height)
{
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	/*printf ("draw_focus: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);*/

	if (DETAIL ("treeview")) /* focus is represented by color only (see base color definitions) */
	{
		return;
	}
	
	if (DETAIL ("button") && GTK_WIDGET_HAS_DEFAULT (widget))
	{
		return; /* focus is represented by blue rectangle painted in draw_shadow */
	}
	
	parent_class->draw_focus (style, window, state_type, area, widget, detail, x, y, width, height);
}
				  
static void draw_slider (GtkStyle *style,
						 GdkWindow *window,
						 GtkStateType state_type,
						 GtkShadowType shadow_type,
						 GdkRectangle *area,
						 GtkWidget *widget,
						 const gchar *detail,
						 gint x,
						 gint y,
						 gint width, gint height, GtkOrientation orientation)
{
	GdkGC *gc1;
	GdkGC *gc2;
	GdkGC *gc3;
	GdkGC *gc4;

	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);

	sanitize_size (window, &width, &height);

	gc1 = style->light_gc[state_type];
	gc2 = style->dark_gc[state_type];

	gc3 = style->bg_gc[state_type];
	gc4 = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[6];
		
	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, area);
		gdk_gc_set_clip_rectangle (gc2, area);
		gdk_gc_set_clip_rectangle (gc3, area);
		gdk_gc_set_clip_rectangle (gc4, area);
	}

	if (GTK_IS_SCROLLBAR (widget) || GTK_IS_SCALE (widget))
	{
		gdk_draw_rectangle (window, gc3, TRUE, x + 1, y + 1,  width - 3, height - 3);

		gdk_draw_line (window, gc2, x + 2, y + height - 2, x + width - 2, y + height - 2);
		gdk_draw_line (window, gc2, x + width - 2, y + 2, x + width - 2, y + height - 2);
		gdk_draw_line (window, gc1, x + 1, y + 1, x + width - 2, y + 1);
		gdk_draw_line (window, gc1, x + 1, y + 1, x + 1, y + height - 2);
	
		if (GTK_IS_VSCROLLBAR (widget) || GTK_IS_VSCALE (widget))
		{
			if (GTK_IS_VSCROLLBAR (widget))
			{
				gdk_draw_line (window, gc4, x, y, x + width - 1, y);
				gdk_draw_line (window, gc4, x, y + height - 1, x + width - 1, y + height - 1);
			}
			else
			{
				theme_draw_rectangle (window, gc4, FALSE, x, y, width - 1, height - 1);
			}

			if (height >= 14 && width >= 12)
			{
				gint gy = y + height / 2 - 8;
				gint glength = width - 10;				
				
				gdk_draw_line (window, gc1, x + 5, gy + 5, x + 5 + glength, gy + 5);
				gdk_draw_line (window, gc2, x + 4, gy + 4, x + 4 + glength, gy + 4);
			
				gdk_draw_line (window, gc1, x + 5, gy + 8, x + 5 + glength, gy + 8);
				gdk_draw_line (window, gc2, x + 4, gy + 7, x + 4 + glength, gy + 7);

				gdk_draw_line (window, gc1, x + 5, gy + 11, x + 5 + glength, gy + 11);
				gdk_draw_line (window, gc2, x + 4, gy + 10, x + 4 + glength, gy + 10);
			}
		}
		else
		{
			if (GTK_IS_HSCROLLBAR (widget))
			{
				gdk_draw_line (window, gc4, x, y, x, y + height - 1);
				gdk_draw_line (window, gc4, x + width - 1, y, x + width - 1, y + height - 1);			
			}
			else
			{
				theme_draw_rectangle (window, gc4, FALSE, x, y, width - 1, height - 1);
			}

			if (width >= 14 && height >= 12)
			{
				gint gx = x + width / 2 - 8;
				gint glength = height - 10;
				
				gdk_draw_line (window, gc1, gx + 5, y + 5, gx + 5, y + 5 + glength);
				gdk_draw_line (window, gc2, gx + 4, y + 4, gx + 4, y + 4 + glength);
			
				gdk_draw_line (window, gc1, gx + 8, y + 5, gx + 8, y + 5 + glength);
				gdk_draw_line (window, gc2, gx + 7, y + 4, gx + 7, y + 4 + glength);

				gdk_draw_line (window, gc1, gx + 11, y + 5, gx + 11, y + 5  + glength);
				gdk_draw_line (window, gc2, gx + 10, y + 4, gx + 10, y + 4 + glength);
			}
		}
	}
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (gc1, area);
		gdk_gc_set_clip_rectangle (gc2, area);
		gdk_gc_set_clip_rectangle (gc3, area);
		gdk_gc_set_clip_rectangle (gc4, area);
	}
}
				   
static void draw_handle (GtkStyle *style,
						 GdkWindow *window,
						 GtkStateType state_type,
						 GtkShadowType shadow_type,
						 GdkRectangle *area,
						 GtkWidget *widget,
						 const gchar *detail,
						 gint x,
						 gint y,
						 gint width, gint height, GtkOrientation orientation)
{
	GdkGC *light_gc;
	GdkGC *dark_gc;
	
	g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
	g_return_if_fail (window != NULL);
  
	sanitize_size (window, &width, &height);
  
	light_gc = style->light_gc[state_type];
	dark_gc = (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[5];
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (light_gc, area);
		gdk_gc_set_clip_rectangle (dark_gc, area);
	}
	
	gtk_style_apply_default_background (style, window, widget && !GTK_WIDGET_NO_WINDOW (widget),
										state_type, area, x, y, width, height);

	if (DETAIL ("dockitem") ||
		(widget && strcmp (g_type_name (G_TYPE_FROM_INSTANCE (widget)), "PanelAppletFrame") == 0))
    {
		/* Fix orientation bug */
		if (orientation == GTK_ORIENTATION_VERTICAL)
		{
			orientation = GTK_ORIENTATION_HORIZONTAL;
		}
		else
		{
			orientation = GTK_ORIENTATION_VERTICAL;
		}
    }
			
	if ((DETAIL ("handlebox") && widget && GTK_IS_HANDLE_BOX (widget)) || DETAIL ("dockitem"))
	{
		/* toolbar stuff */
		gdk_draw_line (window, (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[0], x, y, x + width, y);
		gdk_draw_line (window, (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2], x, y + height - 1, x + width, y + height - 1);
		
		gdk_draw_line (window, (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[2], x + width - 1, y + 2, x + width - 1, y + height - 3);
		gdk_draw_line (window, (LIGHTHOUSEBLUE_STYLE (style))->shade_gc[0], x + width, y + 2, x + width, y + height - 3);
	}
	else if (DETAIL ("paned"))
	{
		gint px;
		gint py;
		
		if (orientation == GTK_ORIENTATION_HORIZONTAL)
		{
			py = y + height / 2 - 1;
			for (px = x + width / 2 - 15; px <= x + width / 2 + 15; px += 5)
			{					
				gdk_draw_point (window, light_gc, px, py);
				gdk_draw_point (window, light_gc, px + 1, py);
				gdk_draw_point (window, light_gc, px, py + 1);
				gdk_draw_point (window, dark_gc, px + 1, py + 2);
				gdk_draw_point (window, dark_gc, px + 2, py + 1);
				gdk_draw_point (window, dark_gc, px + 2, py + 2);
			}
		}
		else
		{
			px = x + width / 2 - 1;
			for (py = y + height / 2 - 15; py <= y + height / 2 + 15; py += 5)
			{
				gdk_draw_point (window, light_gc, px, py);
				gdk_draw_point (window, light_gc, px + 1, py);
				gdk_draw_point (window, light_gc, px, py + 1);
				gdk_draw_point (window, dark_gc, px + 1, py + 2);
				gdk_draw_point (window, dark_gc, px + 2, py + 1);
				gdk_draw_point (window, dark_gc, px + 2, py + 2);				
			}
		}
		
		return;
	}
	else
	{
		draw_box (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height); 
	}

	if (orientation == GTK_ORIENTATION_VERTICAL)
	{		
		if (height >= 14 && width >= 8)
		{			
			gint gy = y + height / 2 - 8;
			gint glength = width - 6;				
			
			gdk_draw_line (window, light_gc, x + 3, gy + 5, x + 3 + glength, gy + 5);
			gdk_draw_line (window, dark_gc, x + 2, gy + 4, x + 2 + glength, gy + 4);
			
			gdk_draw_line (window, light_gc, x + 3, gy + 8, x + 3 + glength, gy + 8);
			gdk_draw_line (window, dark_gc, x + 2, gy + 7, x + 2 + glength, gy + 7);

			gdk_draw_line (window, light_gc, x + 3, gy + 11, x + 3 + glength, gy + 11);
			gdk_draw_line (window, dark_gc, x + 2, gy + 10, x + 2 + glength, gy + 10);
		}
	}
	else
	{
		if (width > 14 && height >= 8)
		{
			gint gx = x + width / 2 - 8;
			gint glength = height - 6;
				
			gdk_draw_line (window, light_gc, gx + 5, y + 3, gx + 5, y + 3 + glength);
			gdk_draw_line (window, dark_gc, gx + 4, y + 2, gx + 4, y + 2 + glength);
			
			gdk_draw_line (window, light_gc, gx + 8, y + 3, gx + 8, y + 3 + glength);
			gdk_draw_line (window, dark_gc, gx + 7, y + 2, gx + 7, y + 2 + glength);

			gdk_draw_line (window, light_gc, gx + 11, y + 3, gx + 11, y + 3  + glength);
			gdk_draw_line (window, dark_gc, gx + 10, y + 2, gx + 10, y + 2 + glength);
		}
	}
	
	if (area)
	{
		gdk_gc_set_clip_rectangle (light_gc, NULL);
		gdk_gc_set_clip_rectangle (dark_gc, NULL);
	}
}
	
static void draw_expander (GtkStyle * style,
			 GdkWindow * window,
			 GtkStateType state_type,
			 GdkRectangle * area,
			 GtkWidget * widget,
			 const gchar * detail,
			 gint x, gint y, GtkExpanderStyle expander_style){}
				 
static void draw_layout (GtkStyle *style,
						 GdkWindow *window,
						 GtkStateType state_type,
						 gboolean use_text,
						 GdkRectangle *area,
						 GtkWidget *widget,
						 const gchar *detail,
						 gint x, gint y, PangoLayout * layout)
{
  g_return_if_fail (LIGHTHOUSEBLUE_IS_STYLE (style));
  g_return_if_fail (window != NULL);

  if (DETAIL ("label"))
  {
	  PangoContext *context;
	  PangoFontDescription *fontDescription;
	  GtkWidget *parent = NULL;
	  
	  context = pango_layout_get_context (layout);
      fontDescription = pango_context_get_font_description (context);
	  if (pango_font_description_get_weight (fontDescription) != PANGO_WEIGHT_BOLD)
	  {
		  parent = gtk_widget_get_parent (widget);

		  if (!GTK_IS_FRAME (parent) && !GTK_IS_NOTEBOOK (parent))
		  {
			  if (GTK_IS_ALIGNMENT (parent)
				  || GTK_IS_BOX (parent)
				  || GTK_IS_FIXED (parent)
				  || GTK_IS_PANED (parent)
				  || GTK_IS_LAYOUT (parent)
				  || GTK_IS_TABLE (parent))
			  {
				  if (GTK_IS_FRAME (gtk_widget_get_parent (parent)))
				  {
					  if (!(LIGHTHOUSEBLUE_RC_STYLE (style->rc_style))->make_frame_labels_bold)
					  {
						  parent = NULL;
					  }
					  else if (gtk_frame_get_label_widget (GTK_FRAME (gtk_widget_get_parent (parent))) != parent || pango_font_description_get_weight (fontDescription) == PANGO_WEIGHT_BOLD)
					  {
						  parent = NULL;
					  }
				  }
				  else if (GTK_IS_NOTEBOOK (gtk_widget_get_parent (parent)))
				  {
					  if ((LIGHTHOUSEBLUE_RC_STYLE (style->rc_style))->make_tab_labels_bold)
					  {
						  GtkWidget *tab_label;
						  gint cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (gtk_widget_get_parent (parent)));
						  if (cur_page > -1)
						  {
							  tab_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (gtk_widget_get_parent (parent)), 
																	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (gtk_widget_get_parent (parent)), cur_page));
							  if (tab_label != parent)
							  {
								  parent = NULL;
							  }
						  }
						  else
						  {
							  parent = NULL;
						  }
					  }
					  else
					  {
						  parent = NULL;
					  }
				  }
				  else
				  {
					  parent = NULL;
				  }
			  }
			  else
			  {
				  parent = NULL;
			  }
		  }
		  else if (GTK_IS_FRAME (parent))
		  {
			  if (!(LIGHTHOUSEBLUE_RC_STYLE (style->rc_style))->make_frame_labels_bold)
			  {
				  parent = NULL;
			  }			  
			  else if (gtk_frame_get_label_widget (GTK_FRAME (parent)) != widget || pango_font_description_get_weight (fontDescription) == PANGO_WEIGHT_BOLD)
			  {
				  parent = NULL;
			  }
		  }
		  else if (GTK_IS_NOTEBOOK (parent))
		  {
			  if ((LIGHTHOUSEBLUE_RC_STYLE (style->rc_style))->make_tab_labels_bold)
			  {
				  GtkWidget *tab_label;
				  gint cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (parent));
				  if (cur_page > -1)
				  {
					  tab_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (parent), 
															  gtk_notebook_get_nth_page (GTK_NOTEBOOK (parent), cur_page));
					  if (tab_label != widget)
					  {
						  parent = NULL;
					  }
				  }
				  else
				  {
					  parent = NULL;
				  }
			  }
			  else
			  {
				  parent = NULL;
			  }
		  }

		  if (parent != NULL)
		  {
			  pango_font_description_set_weight (fontDescription, PANGO_WEIGHT_BOLD);
			  pango_layout_context_changed (layout);
			  gtk_widget_queue_resize (widget);
			  //gtk_widget_queue_resize (parent);
			  return;
		  }
	  }
	  else if ((LIGHTHOUSEBLUE_RC_STYLE (style->rc_style))->make_tab_labels_bold)
	  {
		  parent = gtk_widget_get_parent (widget);

		  if (!GTK_IS_NOTEBOOK (parent))
		  {
			  if (GTK_IS_ALIGNMENT (parent)
				  || GTK_IS_BOX (parent)
				  || GTK_IS_FIXED (parent)
				  || GTK_IS_PANED (parent)
				  || GTK_IS_LAYOUT (parent)
				  || GTK_IS_TABLE (parent))
			  {
				  if (GTK_IS_NOTEBOOK (gtk_widget_get_parent (parent)))
				  {
					  GtkWidget *tab_label;
					  gint cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (gtk_widget_get_parent (parent)));
					  if (cur_page > -1)
					  {
						  tab_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (gtk_widget_get_parent (parent)), 
																  gtk_notebook_get_nth_page (GTK_NOTEBOOK (gtk_widget_get_parent (parent)), cur_page));
						  if (tab_label == parent)
						  {
							  parent = NULL;
						  }
					  }
					  else
					  {
						  parent = NULL;
					  }
				  }
				  else
				  {
					  parent = NULL;
				  }
			  }
			  else
			  {
				  parent = NULL;
			  }
		  }
		  else
		  {
			  GtkWidget *tab_label;
			  gint cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (parent));
			  if (cur_page > -1)
			  {
				  tab_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (parent), 
														  gtk_notebook_get_nth_page (GTK_NOTEBOOK (parent), cur_page));
				  if (tab_label == widget)
				  {
					  parent = NULL;
				  }
			  }
			  else
			  {
				  parent = NULL;
			  }
		  }

		  if (parent != NULL)
		  {
			  pango_font_description_set_weight (fontDescription, PANGO_WEIGHT_NORMAL);
			  pango_layout_context_changed (layout);
			  gtk_widget_queue_resize (widget);
			  //gtk_widget_queue_resize (parent);
			  return;
		  }		  
	  }
  }

  parent_class->draw_layout (style, window, state_type, use_text, area, widget, detail, x, y, layout);
	
}

static void draw_resize_grip (GtkStyle *style,
							  GdkWindow * indow,
							  GtkStateType state_type,
							  GdkRectangle *area,
							  GtkWidget *widget,
							  const gchar *detail,
							  GdkWindowEdge edge,
							  gint x, gint y, gint width, gint height)
{
	return; /* no resize grip in this theme */
}
					

static void lighthouseblue_style_init_from_rc (GtkStyle *style, GtkRcStyle *rc_style)
{
	LighthouseBlueStyle *lighthouseblue_style = LIGHTHOUSEBLUE_STYLE (style);
	double shades[] = {1.065, 0.963, 0.896, 0.90, 0.768, 0.665, 0.4, 0.205};
	gint i;
		
	parent_class->init_from_rc (style, rc_style);
		
	for (i = 0; i < 8; i++)
	{
		shade (&style->bg[GTK_STATE_NORMAL], &lighthouseblue_style->shade[i], (shades[i] - 0.7) * 1.0 + 0.7);
	}
}

static void lighthouseblue_style_realize (GtkStyle *style)
{
	LighthouseBlueStyle *lighthouseblue_style = LIGHTHOUSEBLUE_STYLE (style);
	gint i;
	
	parent_class->realize (style);
	
	for (i = 0; i < 8; i++)
	{
		lighthouseblue_style->shade_gc[i] = realize_color (style, &lighthouseblue_style->shade[i]);
	}
}

static void lighthouseblue_style_unrealize (GtkStyle *style)
{
	LighthouseBlueStyle *lighthouseblue_style = LIGHTHOUSEBLUE_STYLE (style);
	gint i;
	
	for (i = 0; i < 8; i++)
	{
		gtk_gc_release (lighthouseblue_style->shade_gc[i]);
	}
	
	parent_class->unrealize (style);
}

static void lighthouseblue_style_init (LighthouseBlueStyle *style)
{}

static void lighthouseblue_style_class_init (LighthouseBlueStyleClass *klass)
{
  GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  style_class->realize = lighthouseblue_style_realize;
  style_class->unrealize = lighthouseblue_style_unrealize;
  style_class->init_from_rc = lighthouseblue_style_init_from_rc;
 
  style_class->draw_hline = draw_hline;
  style_class->draw_vline = draw_vline;
  style_class->draw_shadow = draw_shadow;

  style_class->draw_box = draw_box;
  style_class->draw_flat_box = draw_flat_box;
  style_class->draw_check = draw_check;
  style_class->draw_option = draw_option;
  style_class->draw_tab = draw_tab;
  style_class->draw_shadow_gap = draw_shadow_gap;
  style_class->draw_box_gap = draw_box_gap;
  style_class->draw_extension = draw_extension;
  style_class->draw_slider = draw_slider;
  style_class->draw_handle = draw_handle;

  style_class->draw_arrow = draw_arrow;

  style_class->draw_focus = draw_focus;

  /*style_class->draw_expander = draw_expander;*/
  style_class->draw_layout = draw_layout;

  style_class->draw_resize_grip = draw_resize_grip;
}

GType lighthouseblue_type_style = 0;

void lighthouseblue_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (LighthouseBlueStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) lighthouseblue_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (LighthouseBlueStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) lighthouseblue_style_init,
  };
  
  lighthouseblue_type_style = g_type_module_register_type (module, GTK_TYPE_STYLE,
														   "LighthouseBlueStyle", &object_info, 0);
}					
