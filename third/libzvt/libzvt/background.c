/*  zvtterm.c - Zed's Virtual Terminal
 *  Copyright (C) 1999  Michael Zucchi
 *
 *  GdkPixbuf image loaders and manupulations for zvt.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gdk/gdk.h>

#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_filterlevel.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_rgb_affine.h>
#include <libart_lgpl/art_rgb_rgba_affine.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>

#include <libzvt/libzvt.h>

static void zvt_background_set(ZvtTerm *term);
static gboolean zvt_configure_window(GtkWidget *widget, GdkEventConfigure *event, ZvtTerm *term);
static void zvt_background_set_translate(ZvtTerm *term);

#define d(x)

/**
 * Utility functions
 */

#if 0
static void
pixmap_free_atom(GdkPixmap *pp)
{
  gdk_xid_table_remove (GDK_WINDOW_XWINDOW(pp));
  g_dataset_destroy (pp);
  g_free (pp);
}
#endif

static GdkPixmap *
pixmap_from_atom(GdkWindow *win, GdkAtom pmap)
{
  unsigned char *data;
  GdkAtom type;
  GdkPixmap *pm = 0;

  d(printf("getting property off root window ...\n"));

  gdk_error_trap_push();
  if (gdk_property_get(win, pmap, 0, 0, 10, FALSE, &type, NULL, NULL, &data)) {
    if (type == GDK_TARGET_PIXMAP) {
      d(printf("converting to pixmap\n"));
      pm = gdk_pixmap_foreign_new(*((Pixmap *)data));
    }
    g_free(data);
  } else {
    g_warning("Cannot get window property %ld\n", gdk_x11_atom_to_xatom (pmap));
  }
  gdk_flush ();
  gdk_error_trap_pop();

  return pm;
}

static GdkPixbuf *
pixbuf_from_atom(GdkWindow *win, GdkAtom pmap)
{
  GdkPixbuf *pb;
  GdkPixmap *pp;
  int pwidth, pheight;

  pp = pixmap_from_atom(win, pmap);
  if (pp) {
    gdk_drawable_get_size(pp, &pwidth, &pheight);

    gdk_error_trap_push ();
    pb = gdk_pixbuf_get_from_drawable(NULL,
				      pp,
				      gdk_drawable_get_colormap(win),
				      0, 0, 0, 0,
				      pwidth, pheight);
    gdk_flush ();
    gdk_error_trap_pop ();
    
    /* Theoretically this should not destroy the pixmap on X, otherwise
     * we'll need another hack like pixmap_free_atom */
    g_object_unref(pp);
    return pb;
  }
  return NULL;
}

static void
pixbuf_shade(GdkPixbuf *pb, int r, int g, int b, int a)
{
  unsigned char *buf = gdk_pixbuf_get_pixels(pb);
  int rowstride = gdk_pixbuf_get_rowstride(pb);
  int pbwidth = gdk_pixbuf_get_width(pb);
  int pbheight = gdk_pixbuf_get_height(pb);
  unsigned char *p;
  int i,j;
  int offset;
  
  d(printf("applying shading\n"));

  if (gdk_pixbuf_get_has_alpha(pb))
    offset=4;
  else
    offset=3;

  d(printf("offset = %d\n", offset));
  d(printf("(r,g,b,a) = (%d, %d, %d, %d)\n", r, g, b, a));
  
  for (i=0;i<pbheight;i++) {
    p = buf;
    for (j=0;j<pbwidth;j++) {
      p[0] += ((r-p[0])*a)>>8;
      p[1] += ((g-p[1])*a)>>8;
      p[2] += ((b-p[2])*a)>>8;
      p+=offset;
    }
    buf+=rowstride;
  }
}


struct _watchwin {
  struct _watchwin *next;
  struct _watchwin *prev;
  GdkWindow *win;
  int propmask;
  struct vt_list watchlist;
};

struct _watchatom {
  struct _watchatom *next;
  struct _watchatom *prev;
  GdkAtom atom;
  void (*atom_changed)(GdkAtom atom, int state, void *data);
  void *data;
};

static struct vt_list watchlist = { (struct vt_listnode *)&watchlist.tail,
				    0,
				    (struct vt_listnode *)&watchlist.head };

static GdkFilterReturn
zvt_filter_prop_change(GdkXEvent *_xevent, GdkEvent *event, void *data)
{
  struct _watchwin *w = data;
  struct _watchatom *a;
  XEvent *xevent = (XEvent *)_xevent;

  d(printf("got window event ... %d\n", xevent->type));
  if (xevent->type == PropertyNotify) {

    a = (struct _watchatom *)w->watchlist.head;
    while (a->next) {
      if (gdk_x11_atom_to_xatom (a->atom) == xevent->xproperty.atom) {
	d(printf("atom %ld has changed\n", gdk_x11_atom_to_xatom (a->atom)));
	a->atom_changed(a->atom, xevent->xproperty.state, a->data);
      }
      a = a->next;
    }

  }
  return GDK_FILTER_REMOVE;
}

static void
add_winwatch(GdkWindow *win, GdkAtom atom, void *callback, void *data)
{
  struct _watchwin *w;
  struct _watchatom *a;

  w = (struct _watchwin *)watchlist.head;
  while (w->next) {
    if (w->win == win) {
      goto got_win;
    }
    w = w->next;
  }
  w = g_malloc0(sizeof(*w));
  vt_list_new(&w->watchlist);
  g_object_ref(win);
  w->win = win;
  w->propmask = gdk_window_get_events(win);
  gdk_window_add_filter(win, zvt_filter_prop_change, w);
  gdk_window_set_events(win, w->propmask | GDK_PROPERTY_CHANGE_MASK);
  vt_list_addtail(&watchlist, (struct vt_listnode *)w);

 got_win:
  a = g_malloc0(sizeof(*a));
  a->atom = atom;
  a->data = data;
  a->atom_changed = callback;
  vt_list_addtail(&w->watchlist, (struct vt_listnode *)a);
}

static void
del_winwatch(GdkWindow *win, void *data)
{
  struct _watchwin *w, *wn;
  struct _watchatom *a, *an;

  w = (struct _watchwin *)watchlist.head;
  wn = w->next;
  while (wn) {
    if (w->win == win) {
      a = (struct _watchatom *)w->watchlist.head;
      an = a->next;
      while (an) {
	if (a->data == data) {
	  vt_list_remove((struct vt_listnode *)a);
	  g_free(a);
	}
	a = an;
	an = an->next;
      }
    }
    if (vt_list_empty(&w->watchlist)) {
      gdk_window_set_events(w->win, w->propmask);
      gdk_window_remove_filter(w->win, zvt_filter_prop_change, w);
      g_object_unref(w->win);
      vt_list_remove((struct vt_listnode *)w);
      g_free(w);
    }
    w = wn;
    wn = wn->next;
  }
}

void
zvt_term_update_toplevel_watch (struct _ZvtTerm *term,
                                gboolean         in_destroy)
{
  struct _zvtprivate *zp;
  GtkWidget *top;
  
  zp = _ZVT_PRIVATE (term);

  if (zp->background_watch_window)
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (zp->background_watch_window),
                                            G_CALLBACK (zvt_configure_window),
                                            term);
      g_object_unref (G_OBJECT (zp->background_watch_window));
      zp->background_watch_window = NULL;
    }

  if (!in_destroy)
    {
      top = gtk_widget_get_toplevel (GTK_WIDGET (term));
      
      if (top && GTK_WIDGET_TOPLEVEL (top))
        {      
          g_signal_connect (G_OBJECT (top),
                            "configure_event",
                            G_CALLBACK (zvt_configure_window),
                            term);
          zp->background_watch_window = top;
          g_object_ref (G_OBJECT (zp->background_watch_window));
        }
    }
}

/*
 * Main (xternal) functions
 */
/**
 * zvt_term_background_new:
 * @t: Terminal.
 * 
 * Create a new background data structure.  This structure can be
 * manipulated by various calls, and then loaded into the terminal
 * with zvt_term_background_load().  Background structures are
 * refcounted.
 * 
 * Return value: A new &zvt_background data structure.
 **/
struct zvt_background *
zvt_term_background_new(ZvtTerm *t)
{
  struct zvt_background *b = g_malloc0(sizeof(*b));
  b->refcount=1;
  return b;
}

/**
 * zvt_term_background_unref:
 * @b: A background.
 * 
 * Unreference this background @b.  If there are no more referees
 * then the structure is freed.
 **/
void
zvt_term_background_unref(struct zvt_background *b)
{
  if (b) {
    if (b->refcount==1) {
      zvt_term_background_set_pixmap(b, 0, 0);
      g_free(b);
    } else {
      b->refcount--;
    }
  }
}

/**
 * zvt_term_background_ref:
 * @b: A background.
 * 
 * Add a new reference to the background.
 **/
void
zvt_term_background_ref(struct zvt_background *b)
{
  if (b)
    b->refcount++;
}

/**
 * zvt_term_background_set_pixmap:
 * @b: Background.
 * @p: A Pixmap.
 * @cmap: Colourmap for this pixmap.
 * 
 * Set a pixmap, or other drawable as the base image for the background.
 * If @p is just a pixmap, then @cmap should be the colourmap for that
 * pimxap, otherwise it may be %NULL if it is a window.
 **/
void
zvt_term_background_set_pixmap(struct zvt_background *b, GdkPixmap *p, GdkColormap *cmap)
{
  switch (b->type) {
  case ZVT_BGTYPE_NONE:		/* no background */
    break;
  case ZVT_BGTYPE_ATOM:		/* pixmap id contained in atom */
    g_object_unref(b->data.atom.window);
    break;
  case ZVT_BGTYPE_PIXMAP:	/* normal pixmap */
    if (b->data.pixmap.pixmap)
      g_object_unref(b->data.pixmap.pixmap);
    if (b->data.pixmap.cmap)
      g_object_unref(b->data.pixmap.cmap);
    break;
  case ZVT_BGTYPE_FILE:		/* file */
    g_free(b->data.pixmap_file);
    break;
  case ZVT_BGTYPE_PIXBUF:		/* pixbuf */
    g_object_unref(b->data.pixbuf);
    break;
  }
  b->data.pixmap.pixmap = p;
  if (p)
    g_object_ref(p);
  b->data.pixmap.cmap = cmap;
  if (cmap)
    g_object_ref(cmap);
  b->type = ZVT_BGTYPE_PIXMAP;
}

/**
 * zvt_term_background_set_pixmap_atom:
 * @b: Background.
 * @win: Window for which the pixmap property is available.
 * @atom: The property containing the pixmap id.
 * 
 * Set a pixmap, who's id is stored in a window property @atom
 * of the window @win.  This atom will be tracked, and changes
 * to it will be picked up as required.
 **/
void
zvt_term_background_set_pixmap_atom(struct zvt_background *b, GdkWindow *win, GdkAtom atom)
{
  zvt_term_background_set_pixmap(b, 0, 0);
  b->data.atom.atom = atom;
  g_object_ref(win);
  b->data.atom.window = win;
  b->type = ZVT_BGTYPE_ATOM;
}

/**
 * zvt_term_background_set_pixmap_file:
 * @b: Background.
 * @filename: Name of image file.
 * 
 * Set the base image to come from a file.  The file
 * must be loadable by gdk-pixbuf.
 **/
void
zvt_term_background_set_pixmap_file(struct zvt_background *b, char *filename)
{
  zvt_term_background_set_pixmap(b, 0, 0);
  b->data.pixmap_file = g_strdup(filename);
  b->type = ZVT_BGTYPE_FILE;
}

/**
 * zvt_term_background_set_pixbuf:
 * @b: Background.
 * @pb: An initialised GdkPixbuf.
 * 
 * Set the base image to be a pixbuf image @pb.
 **/
void
zvt_term_background_set_pixbuf(struct zvt_background *b, GdkPixbuf *pb)
{
  zvt_term_background_set_pixmap(b, 0, 0);
  g_object_ref(pb);
  b->data.pixbuf = pb;
  b->type = ZVT_BGTYPE_PIXBUF;
}

/**
 * zvt_term_background_set_shade:
 * @bg: Background.
 * @r: Red colour.
 * @g: Green colour.
 * @b: Blue colour.
 * @a: Colour intensity.
 * 
 * Set the shading value for this background.  This is a processing
 * option which applies the colour (@r, @g, @b) with opacity
 * @a to the image.  A 0 @a results in no change, a full @a results
 * in a solid colour.
 *
 * The range of each value is 0-65535.
 **/
void
zvt_term_background_set_shade(struct zvt_background *bg, int r, int g, int b, int a)
{
  bg->shade.r = r>>8;
  bg->shade.g = g>>8;
  bg->shade.b = b>>8;
  bg->shade.a = a>>8;
}

/**
 * zvt_term_background_set_scale:
 * @b: Background.
 * @type: zvt_background_scale_t of scaling operation to apply.
 * @x: When required, the x or width scale value.
 * @y: When required, the y or height scale value.
 * 
 * Set scaling options to be applied to the base image.  Scaling
 * options can be set for any image.
 *
 * The following scaling options are available:
 * ZVT_BGSCALE_NONE: No scaling is applied.
 * ZVT_BGSCALE_WINDOW: The image is always scaled to fit exactly
 * into the window.
 * ZVT_BGSCALE_FIXED: The image is always scaled to a fixed ratio.
 * In this case @x and @y are fixed-point values, where 16384 is 1.0.
 * ZVT_BGSCALE_ABSOLUTE: The image is scaled to an absolute size.
 * In this case @x and @y are the new image size.
 **/
void
zvt_term_background_set_scale(struct zvt_background *b, zvt_background_scale_t type, int x, int y)
{
  b->scale.x = x;
  b->scale.y = y;
  b->scale.type = type;
}

/**
 * zvt_term_background_set_translate:
 * @b: Background.
 * @type: A zvt_background_translate_t type, controling the background
 * translations applied.
 * @x: X offset.
 * @y: Y offset.
 * 
 * Set translation options on the image.  This controls how the image
 * sits inside the background window.
 *
 * The following options are available:
 *
 * ZVT_BGTRANSLATE_NONE: No special translation is applied.  @x and @y
 * are still used to offset the image.
 * ZVT_BGTRANSLATE_SCROLL: The image scrolls as text does.  @x and @y
 * provide a fixed offset as well.
 * ZVT_BGTRANSLATE_ROOT: The image stays in the same location in absolute
 * coordinates (i.e. the window becomes a viewport onto a larger image).
 * @x and @y provide additional fixed offsets.
 **/
void
zvt_term_background_set_translate(struct zvt_background *b, zvt_background_translate_t type, int x, int y)
{
  b->offset.x = x;
  b->offset.y = y;
  b->offset.type = type;
}

/* called when the root pixmap atom changes */
static void
zvt_root_atom_changed(GdkAtom atom, int state, ZvtTerm *term)
{
  if (state == GDK_PROPERTY_NEW_VALUE) {
    zvt_background_set(term);
    gtk_widget_queue_draw(GTK_WIDGET(term));
  }
  /* FIXME: If GDK_PROPERTY_DELETE, must remove root pixmap option
     from terminal */
}

/**
 * zvt_term_background_unload:
 * @term: Terminal for which a background may have been loaded.
 * 
 * Unload the current background settings from the @term.
 **/
void
zvt_term_background_unload(ZvtTerm *term)
{
  struct _zvtprivate *zp = _ZVT_PRIVATE(term);
  struct zvt_background *b = zp->background;

  if (b) {
    switch(b->type) {
    case ZVT_BGTYPE_ATOM:		/* pixmap id contained in atom */
      del_winwatch(b->data.atom.window, term);
      break;
    case ZVT_BGTYPE_NONE:
    case ZVT_BGTYPE_PIXMAP:
    case ZVT_BGTYPE_FILE:
    case ZVT_BGTYPE_PIXBUF:
      break;
    }

    zvt_term_background_unref(b);
    zp->background = 0;
  }

  /* free pixmap also ... must take into account root pixmaps (what?) */
  if (zp->background_pixmap) {
      if (term->back_gc)
	  gdk_gc_set_fill (term->back_gc, GDK_SOLID);
      g_object_unref (zp->background_pixmap);
  }
  
  zp->background_pixmap = 0;
  gtk_widget_queue_draw(GTK_WIDGET(term));
}

/**
 * zvt_term_background_load:
 * @term: A terminal.
 * @b: Background to set.  This will be referenced by
 * the terminal, and should be unref'd by the caller
 * when finished with.
 * 
 * Load the background settings into the terminal window.
 * 
 * Return value: Returns 0 on success, or non-zero for 
 * a loader failure.  The reason for the load failure
 * may depend on the image file not existing, or the image
 * pixmap not existing, etc.
 **/
int
zvt_term_background_load(ZvtTerm *term, struct zvt_background *b)
{
  struct _zvtprivate *zp = _ZVT_PRIVATE(term);
  int watchatom=0;

  /* load it, if we can ... */
  if (!GTK_WIDGET_REALIZED (term)) {
    zvt_term_background_ref(b);
    zvt_term_background_unref(zp->background_queue);
    zp->background_queue = b;
    return 0;
  }

  zvt_term_background_unload(term);
#if 0
  if (zp->background) {
    if (b) {
      /* compare them to see if they're the same ... if so, dont change display */

      /* FIXME: this should compare the contents :) */
      if (b==zp->background) {
	/* but somehow we have to do this only if it was realised when called before? */
      }
    }
    zvt_term_background_unref(zp->background);
  }
#endif
  zp->background = b;

  /* assume no background is being set */
  term->vx->scroll_type=VT_SCROLL_ALWAYS;

  if (b) {
    zvt_term_background_ref(b);

    if (b->type == ZVT_BGTYPE_ATOM)
      watchatom = 1;
    if (watchatom) {
      add_winwatch(b->data.atom.window,
		   b->data.atom.atom,
		   zvt_root_atom_changed,
		   term);
    }

    switch (b->offset.type) {
    case ZVT_BGTRANSLATE_NONE:	/* normal view, pixmap in window */
    case ZVT_BGTRANSLATE_ROOT:	/* relative to root ('viewport') */
      term->vx->scroll_type=VT_SCROLL_NEVER;
      break;
    case ZVT_BGTRANSLATE_SCROLL: /* background scrolls with scrolling */
      term->vx->scroll_type=VT_SCROLL_SOMETIMES;
      break;
    }
  }
  zvt_background_set(term);
  if (b)
    gtk_widget_queue_draw(GTK_WIDGET(term));
  return 0;
}

static void
zvt_background_set(ZvtTerm *term)
{
  struct _zvtprivate *zp = _ZVT_PRIVATE(term);
  struct zvt_background *b = zp->background;
  GdkPixmap *pixmap = NULL;
  GdkPixbuf *pixbuf = NULL;
  int wwidth, wheight, wdepth;
  int process = 0;
  GdkColor pen;
  GdkColormap *cmap = 0;

  /* if we have no 'background image', use a solid colour */
  if (b == NULL
      || b->type == ZVT_BGTYPE_NONE) {
    if (term->back_gc) {
      gdk_gc_set_fill (term->back_gc, GDK_SOLID);
      pen.pixel = term->colors[17].pixel;
      gdk_gc_set_foreground (term->back_gc, &pen);
    }
    return;
  }

  process = (b->shade.a != 0
	     || b->scale.type != ZVT_BGSCALE_NONE);
      
  switch (b->type) {
  case ZVT_BGTYPE_NONE:		/* no background */
    break;
  case ZVT_BGTYPE_ATOM:		/* pixmap id contained in atom */
    if (process)
      pixbuf = pixbuf_from_atom(b->data.atom.window, b->data.atom.atom);
    else
      pixmap = pixmap_from_atom(b->data.atom.window, b->data.atom.atom);
    break;
  case ZVT_BGTYPE_PIXMAP:	/* normal pixmap/window */
    pixmap = b->data.pixmap.pixmap;
    cmap = b->data.pixmap.cmap;
    break;
  case ZVT_BGTYPE_FILE:		/* file */
    pixbuf = gdk_pixbuf_new_from_file(b->data.pixmap_file, NULL);
    break;
  case ZVT_BGTYPE_PIXBUF:		/* pixbuf */
    pixbuf = b->data.pixbuf;
    break;
  }

  gdk_window_get_geometry(GTK_WIDGET(term)->window,NULL,NULL,&wwidth,&wheight,&wdepth);

  if (process) {
    int width, height;
    if (pixbuf==NULL && pixmap!=NULL) {
      int pwidth, pheight;
      gdk_drawable_get_size(pixmap, &pwidth, &pheight);
      pixbuf = gdk_pixbuf_get_from_drawable(0, pixmap, cmap, 0, 0, 0, 0, pwidth, pheight);
      /* free the pixmap */
      g_object_unref (pixmap);
    }

    if (pixbuf != NULL) {
      width = gdk_pixbuf_get_width(pixbuf);
      height = gdk_pixbuf_get_height(pixbuf);
      
      if (b->shade.a != 0) {
	pixbuf_shade(pixbuf, b->shade.r, b->shade.g, b->shade.b, b->shade.a);
      }

      switch (b->scale.type) {
      case ZVT_BGSCALE_NONE:		/* no scaling */
	break;
      case ZVT_BGSCALE_WINDOW:		/* scale to window */
	width = wwidth;
	height = wheight;
	break;
      case ZVT_BGSCALE_FIXED:		/* scale fixed amount */
	width = (width * b->scale.x) >> 14;
	height = (height * b->scale.y) >> 14;
	break;
      case ZVT_BGSCALE_ABSOLUTE:		/* scale absolute coords */
	width = b->scale.x;
	height = b->scale.y;
	break;
      }
      if (b->scale.type != ZVT_BGSCALE_NONE) {
        GdkPixbuf *tmp = gdk_pixbuf_scale_simple(pixbuf, width, height,
						 GDK_INTERP_NEAREST);
	g_object_unref(pixbuf);
	pixbuf = tmp;
      }
    }
  }

  /* if we have a pixbuf, then we need to convert it to a pixmap to actually
     use it ... */
  if (pixbuf!=NULL) {

    pixmap = gdk_pixmap_new(GTK_WIDGET(term)->window,
			    gdk_pixbuf_get_width(pixbuf),
			    gdk_pixbuf_get_height(pixbuf), wdepth);
    
    /* render to pixmap */
    gdk_pixbuf_render_to_drawable(pixbuf, pixmap, GTK_WIDGET(term)->style->white_gc,
				  0, 0, 0, 0,
				  gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf),
				  GDK_RGB_DITHER_MAX,
				  0, 0);
    /* free working area */
    g_object_unref(pixbuf);
  }

  /* unref the previous pixmap */
  if (zp->background_pixmap)
      g_object_unref (zp->background_pixmap);
  
  zp->background_pixmap = pixmap;

  if (term->back_gc) {
    if (pixmap) {
      gdk_gc_set_tile (term->back_gc, pixmap);
      gdk_gc_set_fill (term->back_gc, GDK_TILED);
      zvt_background_set_translate(term);
    }
  }
}

static void
zvt_background_set_translate(ZvtTerm *term)
{
  int offx, offy, x, y;
  Window childret;
  struct _zvtprivate *zp = _ZVT_PRIVATE(term);
  struct zvt_background *b = zp->background;

  offx = b->offset.x;
  offy = b->offset.y;

  switch(b->offset.type) {
  case ZVT_BGTRANSLATE_NONE:
  case ZVT_BGTRANSLATE_SCROLL:
    break;
  case ZVT_BGTRANSLATE_ROOT:
    XTranslateCoordinates (GDK_WINDOW_XDISPLAY (GTK_WIDGET(term)->window),
			   GDK_WINDOW_XWINDOW (GTK_WIDGET(term)->window),
			   GDK_ROOT_WINDOW (),
			   0, 0,
			   &x, &y,
			   &childret);
    offx -= x;
    offy -= y;
    break;
  }
  if (term->back_gc)
    gdk_gc_set_ts_origin(term->back_gc, offx, offy);
}

/*
 * If we configure window, work out if we have to reload the background
 * image, etc.
 */
static gboolean
zvt_configure_window(GtkWidget *widget, GdkEventConfigure *event, ZvtTerm *term)
{
  struct _zvtprivate *zp = _ZVT_PRIVATE(term);
  Window childret;
  int width, height, x, y;
  struct zvt_background *b = zp->background;
  int forcedraw = 0;

  if (b == NULL ||
      !(b->scale.type == ZVT_BGSCALE_WINDOW
        || b->offset.type == ZVT_BGTRANSLATE_ROOT))
    return FALSE;
  
  XTranslateCoordinates (GDK_WINDOW_XDISPLAY (GTK_WIDGET(term)->window),
			 GDK_WINDOW_XWINDOW (GTK_WIDGET(term)->window),
			 GDK_ROOT_WINDOW (),
			 0, 0,
			 &x, &y,
			 &childret);
  gdk_drawable_get_size(GTK_WIDGET(term)->window, &width, &height);

  d(printf("configure event, pos %d,%d size %d-%d\n", x, y, width, height));
  d(printf("last was at %d,%d %d-%d\n", b->pos.x, b->pos.y, b->pos.w, b->pos.h));

  /* see if we need to reload (scale) the image */
  if (b->scale.type == ZVT_BGSCALE_WINDOW
      && (b->pos.h != height || b->pos.w != width)) {
    zvt_background_set(term);
    forcedraw = 1;
  }

  /* if we are relative absolute coords, and we have moved, we must catch up */
  if (b->offset.type == ZVT_BGTRANSLATE_ROOT
      && (b->pos.x != x || b->pos.y != y)) {
    zvt_background_set_translate(term);
    forcedraw = 1;
  }

  /* update last rendered position */
  b->pos.x = x;
  b->pos.y = y;
  b->pos.w = width;
  b->pos.h = height;

  if (forcedraw) {
    gtk_widget_queue_draw(GTK_WIDGET(term));
  }

  d(printf("we moved/sized\n"));

  return FALSE;
}

