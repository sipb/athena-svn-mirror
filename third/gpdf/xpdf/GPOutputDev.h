/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*  Gnome Print PDF Output Device
 *
 *  Copyright (C) 2002 Martin Kretzschmar
 *
 *  Author:
 *    Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GP_OUTPUT_DEV_H
#define GP_OUTPUT_DEV_H

#include "OutputDev.h"
#include "gpdf-g-switch.h"
#  include <libgnomeprint/gnome-print.h>
#include "gpdf-g-switch.h"
#include "GfxState.h"
#include "GfxFont.h"

class GPOFontMap {
public:
  GPOFontMap();
  ~GPOFontMap();
  void startDoc(XRef *xref);
  void setPrintContext(GnomePrintContext *pc) { 
    gpc = pc;
    /* FIXME destroy embedded fonts hash? */
  }
  GnomeFontFace *getFontFace(GfxFont *font);
private:
  static gboolean isEmbeddedFont(GfxFont *font) {
    Ref embRef;
    return font->getEmbeddedFontID(&embRef);
  }

  gboolean isBase14Font(GfxFont *font) {
    GString *gfont_name = font->getName();

    return (gfont_name != NULL)
      ? (g_hash_table_lookup(base14Fonts, gfont_name->getCString()) != NULL)
      : FALSE;
  }

  static gboolean getStreamContents(Stream *str, guchar **contents,
				    gsize *length);

  GnomeFontFace *getFontFaceEmbedded(GfxFont *font);
  GnomeFontFace *getFontFaceBase14(GfxFont *font);
  GnomeFontFace *getFontFaceFallback (GfxFont *font);

  void initBase14Fonts();

  XRef *xref;
  GnomePrintContext *gpc;
  GHashTable *base14Fonts;
  GHashTable *embeddedFonts;
};

class GPOutputDev: public OutputDev {
public:

  GPOutputDev();

  virtual ~GPOutputDev();

  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gFalse; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gFalse; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }

  //----- initialization and control

  void setPrintContext(GnomePrintContext *pc);

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  //----- save/restore graphics state
  virtual void saveState(GfxState *state);
  virtual void restoreState(GfxState *state);

  //----- update graphics state
  virtual void updateCTM(GfxState *state, double m11, double m12,
			 double m21, double m22, double m31, double m32);
  virtual void updateLineDash(GfxState *state);
  virtual void updateLineJoin(GfxState *state);
  virtual void updateLineCap(GfxState *state);
  virtual void updateMiterLimit(GfxState *state);
  virtual void updateLineWidth(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);
  virtual void updateFillOpacity(GfxState *state);
  virtual void updateStrokeOpacity(GfxState *state);

  //----- update text state
  virtual void updateFont(GfxState *state);
  virtual void updateTextPos(GfxState *state);
  virtual void updateTextShift(GfxState *state, double shift);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  virtual void drawString(GfxState *state, GString *s);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 int *maskColors, GBool inlineImg);

  // Called to indicate that a new PDF document has been loaded.
  void startDoc(XRef *xref);

private:

  void doPath(GfxPath *path);
  void updateFillColorIfNecessary(GfxState *state);
  void updateStrokeColorIfNecessary(GfxState *state);

  GnomePrintContext *gpc;
  gchar *page_name;

  guint stroke_color_set : 1;
  guint fill_color_set : 1;

  GPOFontMap font_map;
  GnomeFont *gnome_font;
  gdouble text_pos_x, text_pos_y;
};

#endif /* GP_OUTPUT_DEV_H */
