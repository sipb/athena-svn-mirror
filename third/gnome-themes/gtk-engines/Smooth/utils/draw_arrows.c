#include "misc_functions.h"
#include "math.h"

/* This function is based on XFCE's & CleanIce draw arrow routines, both which  were based on ThinIce's */
static void
SmoothDrawCleanArrow(GdkWindow * window, 
		        GdkRectangle * area,
			GdkGC * gc,
			
			gint x,
			gint y,
			gint width,
			gint height,
			
			GtkArrowType arrow_type,
			gint arrow_tail,
			
			gboolean outside_edge)
{
	gint i;

	gint steps, extra;
	gint start, increment;
	gint aw, ah;

	aw = width;
	ah = height;
	
	if (area) gdk_gc_set_clip_rectangle (gc, area);

	if ((arrow_type == GTK_ARROW_UP) || (arrow_type == GTK_ARROW_DOWN))
	{
		gdouble tmp=((aw+1)/2) - ((height%2)?1:0);
      
		if (tmp > ah) 
		{
			aw = 2*ah - 1 - ((height%2)?1:0);
			ah = (aw+1)/2;
		} 
		else 
		{
			ah = (gint) tmp;
			aw = 2*ah - 1;
		}  

		if ((aw < 5) || (ah < 3)) 
		{
			aw = 5;
			ah = 3;
		}

		ah += arrow_tail;

		x += (width - aw) / 2 ;
		y += (height - ah) / 2;
		width = aw;
		height = ah;
		
		/* W/H Ratio Only Works If Forced Odd 
		 */
		width += width % 2 - 1;

		/* The Arrow W/H is always the same ratio, so any extra
		 * space is drawn as an extension to the base.
		 */
		steps = 1 + width / 2;
		extra = height - steps;

		/* Drawing is in the direction of the arrow, that is,
		 * Starts From the Base, and increments, or decrements, 
		 * to the Point.
		 */
		if (arrow_type == GTK_ARROW_DOWN)
		{
			start = y;
			increment = 1;
		}
		else
		{
			start = y + height - 1;
			increment = -1;
		}

		/* Draw Any Extra Space As an extension off the Base
		 * i.e, as a tail.
		 */
		for (i = 0; i < extra; i++)
		{
			gdk_draw_line(window, gc, x, start + i * increment, x + width - 1, start + i * increment);
		}

		/* Finally Draw Actual Arrow
		 */
		for (; i < height; i++)
		{
			gdk_draw_line(window, gc, x + (i - extra), start + i * increment, x + width - (i - extra) - 1, start + i * increment);
		}
	}
	else
	{
		gdouble tmp=((ah+1)/2) - ((width%2)?1:0);

		if (tmp > aw) 
		{
			ah = 2*aw - 1 - ((width%2)?1:0);
			aw = (ah+1)/2;
		} 
		else 
		{
			aw = (gint) tmp;
			ah = 2*aw - 1;
		}  

		if ((ah < 5) || (aw < 3)) 
		{
			ah = 5;
			aw = 3;
		}

		aw += arrow_tail;

		x += (width - aw) / 2 ;
		y += (height - ah) / 2;
		width = aw;
		height = ah;

		/* W/H Ratio Only Works If Forced Odd 
		 */
		height += height % 2 - 1;

		/* The Arrow W/H is always the same ratio, so any extra
		 * space is drawn as an extension to the base.
		 */
		steps = 1 + height / 2;
		extra = width - steps;

		/* Drawing is in the direction of the arrow, that is,
		 * Starts From the Base, and increments, or decrements, 
		 * to the Point.
		 */
		if (arrow_type == GTK_ARROW_RIGHT)
		{
			start = x;
			increment = 1;
		}
		else
		{
			start = x + width - 1;
			increment = -1;
		}

		/* Draw Any Extra Space As an extension off the Base
		 * i.e, as a tail.
		 */
		for (i = 0; i < extra; i++)
		{
			gdk_draw_line(window, gc, start + i * increment, y, start + i * increment, y + height - 1);
		}

		/* Finally Draw Actual Arrow
		 */
		for (; i < width; i++)
		{
			gdk_draw_line(window, gc, start + i * increment, y + (i - extra), start + i * increment, y + height - (i - extra) - 1);
		}
	}

	if (area) gdk_gc_set_clip_rectangle (gc, NULL);
}

/* This function is based on EnGradient/IceGradient's draw arrow routines */
static void
SmoothDrawDirtyArrow(GdkWindow * window, 
		        GdkRectangle * area,
			GdkGC * gc,
			
			gint x,
			gint y,
			gint width,
			gint height,
			
			GtkArrowType arrow_type,
			gint arrow_tail,

			gboolean outside_edge)
{
	gint size, half_size, tail=0;
	
	GdkPoint points[4];

	width += width % 2 - 1;
	height += height % 2 - 1;
  
	size = MIN(width, height);  
	half_size = size / 2;

	x += (width - size)/2;
	y += (height - size)/2;
	
	switch (arrow_type) {
		case GTK_ARROW_UP:
			if (size - height > 0)
			{
				tail = size - height;
				//y += (height - size)/2;
			}

			points[0].x = x + half_size; points[0].y = y;
			points[1].x = x;             points[1].y = y + size - 1;

			points[2].x = x + size - 1;  points[2].y = y + size - 1;
			points[3].x = x + half_size; points[3].y = y;
		break;

		case GTK_ARROW_DOWN:			
			if (size - height > 0)
			{
				tail = size - height;
				//y += (height - size)/2;
			}

			points[0].x = x + half_size; points[0].y = y + size - 1;
			points[1].x = x + size - 1;  points[1].y = y;

			points[2].x = x;             points[2].y = y;
			points[3].x = x + half_size; points[3].y = y + size - 1;
		break;

		case GTK_ARROW_LEFT:
			if (size - height > 0)
			{
				tail = size - width;
				//x += (width - size)/2;
			}

			points[0].x = x;            points[0].y = y + half_size;
			points[1].x = x + size - 1; points[1].y = y  + size - 1;

			points[2].x = x + size - 1; points[2].y = y;
			points[3].x = x;            points[3].y = y + half_size;
		break;

		case GTK_ARROW_RIGHT:
			if (size - height > 0)
			{
				tail = size - width;
				//x += (width - size)/2;
			}

			points[0].x = x + size - 1; points[0].y = y + half_size;
			points[1].x = x;            points[1].y = y;

			points[2].x = x;            points[2].y = y + size - 1;
			points[3].x = x + size - 1; points[3].y = y + half_size;
		break;
	}    

	if (!outside_edge)
	{
		gdk_draw_polygon(window, gc, TRUE, points, 4);
	}	

	gdk_draw_polygon(window, gc, FALSE, points, 4);
}

/* This function is based on the Wonderland theme engine, 
 * it is essentially calculate_arrow_geometry and draw_arrow,
 * concatted into one composite whole...
 */
static void
SmoothDrawSlickArrow(GdkWindow * window, 
		        GdkRectangle * area,
			GdkGC * gc,
			
			gint x,
			gint y,
			gint width,
			gint height,
			
			GtkArrowType arrow_type,
			gint arrow_tail,

			gboolean outside_edge)
{
	gint i;

	gint w, h, offset1, offset2;

	gboolean base_line;	
	gint base, increment;
	gint x1, x2, y1, y2;
			
	w = width;
	h = height;

	if ((arrow_type == GTK_ARROW_UP) || (arrow_type == GTK_ARROW_DOWN))
	{
		w += (w % 2) - 1;
		h = (w / 2 + 1) + 1;

		if (h > height)
		{
			h = height;
			w = 2 * (h - 1) - 1;
		}
      
		
		if ((w < 7) || (h < 5)) 
		{			
			SmoothDrawCleanArrow(window, area, gc, x, y, width, height, arrow_type, 0, FALSE);	
			return;		
		}

				/* Drawing is in the direction of the arrow, that is,
		 * Starts From the Base, and increments, or decrements, 
		 * to the Point.
		 */
		if (arrow_type == GTK_ARROW_DOWN)
		{
			if (height % 2 == 1 || h % 2 == 0)
			{
				height += 1;
			}
			
			base = 0;
			increment = 1;
		}
		else
		{
			if (height % 2 == 0 || h % 2 == 0)
			{
				height -= 1;
			}	

			base = h - 1;
			increment = -1;
		}

		x += (width - w) / 2;
		y += (height - h) / 2;

		/* Finally Draw Actual Arrow
		 */
		for (i = 0; i < h; i++)
		{
			x1 = x + i - 1;
			x2 = x + w - i;
			
			y1 = y2 = y + base + i * increment;
			base_line = (y1 - y) == base;
			
			if ((ABS(x2 - x1) < 7) && (!base_line))
			{
				gdk_draw_line(window, gc, x1, y1, x2, y2);
			}		
			else
			{
				offset1 = ((ABS(x2 - x1) > 7) || (!base_line))?2:1;
				offset2 = (base_line)?offset1:0;

				gdk_draw_line(window, gc, x1 + offset2, y1, x1 + offset1, y1);

				gdk_draw_line(window, gc, x2 - offset1, y2, x2 - offset2, y2);
			}
		}

	}
	else
	{
		h += (h % 2) - 1;
		w = (h / 2 + 1) + 1; 
      
		if (w > width)
		{
			w = width;
			h = 2 * (w - 1) - 1;
		}
      
		if ((h < 7) || (w < 5)) 
		{			
			SmoothDrawCleanArrow(window, area, gc, x, y, width, height, arrow_type, 0, FALSE);			
			return;		
		}

				/* Drawing is in the direction of the arrow, that is,
		 * Starts From the Base, and increments, or decrements, 
		 * to the Point.
		 */
		if (arrow_type == GTK_ARROW_RIGHT)
		{
			if ((width % 2 == 1) || (w % 2 == 0))
			{
				width += 1;
			}
			
			base = 0;
			increment = 1;
		}
		else
		{
			if ((width % 2 == 0) || (w % 2 == 0))
			{
				width -= 1;
			}	
			
			base = w;
			increment = -1;
			x -= 1;
		}

		x += (width - w) / 2;
		y += (height - h) / 2;

		/* Finally Draw Actual Arrow
		 */
		for (i = 0; i < w; i++)
		{
			y1 = y + i - 1;
			y2 = y + h - i;

			x1 = x2 = x + base + i * increment;
			base_line = (x1 - x) == base;

			if ((ABS(y2 - y1) < 7) && (!base_line))
			{
				gdk_draw_line(window, gc, x1, y1, x2, y2);
			}		
			else
			{
				offset1 = (base_line)?2:0;
	
				gdk_draw_line(window, gc, x1, y1 + offset1, x1, y1 + 2);

				gdk_draw_line(window, gc, x2, y2 - 2, x2, y2 - offset1);
			}	
		}
	}
}

void
do_draw_arrow(GdkWindow * window,
              GdkRectangle * area,
	      GtkArrowType arrow_type,
	      
	      GdkGC * fill_gc,
  	      GdkGC * border_gc,
  	      GdkGC * midaa_gc,
  	      
	      gint x,
	      gint y,
	      gint width,
	      gint height,

	      gint arrow_style,
	      gboolean arrow_solid,
	      gboolean arrow_etched)
{
	gint aw=width, ah=height, arrow_tail=0;

	switch (arrow_style) {
		case ARROW_STYLE_DEFAULT : 
		case ARROW_STYLE_THINICE : 
		case ARROW_STYLE_XFCE :        
			if (arrow_style == ARROW_STYLE_THINICE) 
				arrow_tail=3;
			else if (arrow_style == ARROW_STYLE_XFCE) 
				arrow_tail=1;

			if (arrow_solid || arrow_etched)
			{
				if (arrow_etched)
				{
					SmoothDrawCleanArrow(window, area, border_gc, x + 1, y + 1, width, height, arrow_type, arrow_tail, FALSE);
				}

				SmoothDrawCleanArrow(window, area, fill_gc, x, y, width, height, arrow_type, arrow_tail, FALSE);
			}	
			else
			{
				SmoothDrawCleanArrow(window, area, border_gc, x, y, width, height, arrow_type, arrow_tail, TRUE);

				SmoothDrawCleanArrow(window, area, fill_gc, x + 1, y + 1, width - 2, height - 2, arrow_type, arrow_tail, FALSE);
			}	

		break;

		case ARROW_STYLE_ICEGRADIENT : 
			{
				x += 1;
				y += 1;
				width -= 2;
				height -= 2;

				if (arrow_etched)
				{					
					SmoothDrawDirtyArrow(window, area, border_gc, x + 1, y + 1, width, height, arrow_type, arrow_tail, FALSE);

					SmoothDrawDirtyArrow(window, area, midaa_gc, x - 1, y - 1, width + 1, height + 2, arrow_type, arrow_tail, FALSE);
				}
				else if (arrow_solid)
				{
					x += 1;
					y += 1;
					width -= 2;
					height -= 2;
					
					switch (arrow_type) {
						case GTK_ARROW_UP:
							SmoothDrawDirtyArrow(window, area, fill_gc, x-1, y-1, width+2, height+1, arrow_type, arrow_tail, FALSE);
							SmoothDrawDirtyArrow(window, area, midaa_gc, x-1, y-1, width+2, height+1, arrow_type, arrow_tail, TRUE);
						break;

						case GTK_ARROW_LEFT:
							SmoothDrawDirtyArrow(window, area, fill_gc, x-1, y-1, width+1, height+2, arrow_type, arrow_tail, FALSE);
							SmoothDrawDirtyArrow(window, area, midaa_gc, x-1, y-1, width+1, height+2, arrow_type, arrow_tail, TRUE);
						break;

						case GTK_ARROW_DOWN:			
							SmoothDrawDirtyArrow(window, area, fill_gc, x, y, width+1, height+2, arrow_type, arrow_tail, FALSE);
							SmoothDrawDirtyArrow(window, area, midaa_gc, x, y, width+1, height+2, arrow_type, arrow_tail, TRUE);
						break;
						
						case GTK_ARROW_RIGHT:
							SmoothDrawDirtyArrow(window, area, fill_gc,  x, y, width+2, height+1, arrow_type, arrow_tail, FALSE);
							SmoothDrawDirtyArrow(window, area, midaa_gc, x, y, width+2, height+1, arrow_type, arrow_tail, TRUE);
						break;
					}    
				}				
	
				SmoothDrawDirtyArrow(window, area, fill_gc, x, y, width, height, arrow_type, arrow_tail, FALSE);
			
				if (!arrow_etched && !arrow_solid)
				{
					SmoothDrawDirtyArrow(window, area, border_gc, x, y, width, height, arrow_type, arrow_tail, TRUE);
				}
			}	
		break;

		case ARROW_STYLE_WONDERLAND : 
			SmoothDrawSlickArrow(window, area, border_gc, x, y, width, height, arrow_type, arrow_tail, FALSE);
		break;

		case ARROW_STYLE_XPM : 
		default :
			do_draw_arrow(window, area, ARROW_STYLE_DEFAULT, fill_gc, border_gc, midaa_gc, x, y, width, height, arrow_style, arrow_solid, arrow_etched);
		break;
	}
}

gboolean 
TranslateArrowStyleName (gchar * str, gint *retval)
{
#define is_enum(XX)  (g_ascii_strncasecmp(str, XX, strlen(XX))==0)
  if (is_enum("icegradient"))
    *retval = ARROW_STYLE_ICEGRADIENT;
  else if (is_enum("thinice"))
    *retval = ARROW_STYLE_THINICE;
  else if (is_enum("wonderland"))
    *retval = ARROW_STYLE_WONDERLAND;
  else if (is_enum("default") || is_enum("cleanice"))
    *retval = ARROW_STYLE_DEFAULT;
  else if (is_enum("xfce"))
    *retval = ARROW_STYLE_XFCE;
  else if (is_enum("xpm"))
    *retval = ARROW_STYLE_XPM;
  else
    return FALSE; 

  return TRUE;
}
