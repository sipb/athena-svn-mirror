/*  zvtterm.h - Zed's Virtual Terminal
 *  Copyright (C) 1998  Michael Zucchi
 *
 *  Data structures for working with background images.
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
 
#ifndef __ZVT_BACKGROUND_H__
#define __ZVT_BACKGROUND_H__

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef enum {
  ZVT_BGTYPE_NONE,		/* no background */
  ZVT_BGTYPE_ATOM,		/* pixmap id contained in atom */
  ZVT_BGTYPE_PIXMAP,		/* normal pixmap */
  ZVT_BGTYPE_FILE,		/* file */
  ZVT_BGTYPE_PIXBUF		/* pixbuf */
} zvt_background_t;

typedef enum {
  ZVT_BGSCALE_NONE,		/* no scaling */
  ZVT_BGSCALE_WINDOW,		/* scale to window */
  ZVT_BGSCALE_FIXED,		/* scale fixed amount */
  ZVT_BGSCALE_ABSOLUTE		/* scale absolute coords */
} zvt_background_scale_t;

typedef enum {
  ZVT_BGTRANSLATE_NONE,		/* normal view, pixmap in window */
  ZVT_BGTRANSLATE_SCROLL,	/* background scrolls with scrolling */
  ZVT_BGTRANSLATE_ROOT		/* relative to root ('viewport') */
} zvt_background_translate_t;

struct zvt_background {
  zvt_background_t type;

  /* possible sources of base image */
  union {
    struct {
      GdkPixmap *pixmap;
      GdkColormap *cmap;
    } pixmap;
    GdkPixbuf *pixbuf;
    char *pixmap_file;
    struct {
      GdkAtom atom;			/* pixmap id is in atom */
      GdkWindow *window;	/*  on this window */
    } atom;
  } data;

  int refcount;

  struct {
    int r, g, b, a;		/* colour offset, 255=1 */
  } shade;

  struct {
    int x, y, w, h;		/* position of window for this background */
  } pos;

  struct {
    int x, y;			/* scale amount */
    zvt_background_scale_t type;
  } scale;

  struct {
    int x, y;			/* x/y offset */
    zvt_background_translate_t type;
  } offset;
};

/* forward reference the real thing */
struct _ZvtTerm;

struct zvt_background *zvt_term_background_new(struct _ZvtTerm *t);
void zvt_term_background_unref(struct zvt_background *b);
void zvt_term_background_ref(struct zvt_background *b);
void zvt_term_background_set_pixmap(struct zvt_background *b, GdkPixmap *p, GdkColormap *cmap);
void zvt_term_background_set_pixmap_atom(struct zvt_background *b, GdkWindow *win, GdkAtom atom);
void zvt_term_background_set_pixmap_file(struct zvt_background *b, char *filename);
void zvt_term_background_set_pixbuf(struct zvt_background *b, GdkPixbuf *pb);
void zvt_term_background_set_shade(struct zvt_background *bg, int r, int g, int b, int a);
void zvt_term_background_set_scale(struct zvt_background *b, zvt_background_scale_t type, int x, int y);
void zvt_term_background_set_translate(struct zvt_background *b, zvt_background_translate_t type, int x, int y);
int zvt_term_background_load(struct _ZvtTerm *term, struct zvt_background *b);
void zvt_term_background_unload(struct _ZvtTerm *term);

void zvt_term_update_toplevel_watch (struct _ZvtTerm *term,
                                     gboolean         in_destroy);

#endif /* ! __ZVT_BACKGROUND_H__ */
