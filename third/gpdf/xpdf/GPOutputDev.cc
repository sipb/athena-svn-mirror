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

#include <aconf.h>

#include <stdio.h>
#include <unistd.h>
#include "Gfx.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "GPOutputDev.h"
#include "FoFiTrueType.h"
#include "FoFiType1C.h"

#ifdef HAVE_FONT_EMBEDDING
#  include "gpdf-g-switch.h"
#    include "gpdf-font-face.h"
#  include "gpdf-g-switch.h"
#endif

#define noPDF_DEBUG
#ifdef PDF_DEBUG
#  define DBG(x) x
#else
#  define DBG(x)
#endif

#define HAS_NON_NULL_PRINT_CONTEXT \
(this->gpc && GNOME_IS_PRINT_CONTEXT(this->gpc))

const struct {
  const gchar *psName;
  const gchar *name;
} baseFontPSNames [] = {
  {"Times-Roman",           "Nimbus Roman No9 L Regular"},	      //"Times Roman"},
  {"Times-Italic",          "Nimbus Roman No9 L Regular Italic"},	      //"Times Italic"},
  {"Times-Bold",            "Nimbus Roman No9 L Medium"},	      //"Times Bold"},
  {"Times-BoldItalic",      "Nimbus Roman No9 L Medium Italic"},     //"Times Bold Italic"},
  {"Helvetica",             "Nimbus Sans L Regular"},	      //"Helvetica"},
  {"Helvetica-Oblique",     "Nimbus Sans L Regular Italic"},     //"Helvetica Oblique"},
  {"Helvetica-Bold",        "Nimbus Sans L Bold"},	      //"Helvetica Bold"},
  {"Helvetica-BoldOblique", "Nimbus Sans L Bold Italic"},//"Helvetica Bold Oblique"},
  {"Courier",               "Nimbus Mono L Regular"},		      //"Courier"},
  {"Courier-Oblique",       "Nimbus Mono L Regular Oblique"},	      //"Courier Oblique"},
  {"Courier-Bold",          "Nimbus Mono L Bold"},	      //"Courier Bold"},
  {"Courier-BoldOblique",   "Nimbus Mono L Bold Oblique"},  //"Courier Bold Oblique"},
  {"Symbol",                "Standard Symbols L Regular"},		      //"Symbol"}
  {"ZapfDingbats",          "Dingbats Regular"}      //"ITC Zapf Dingbats"}
};

static void fileWrite(void *stream, char *data, int len) {
  fwrite(data, 1, len, (FILE *)stream);
}

GPOFontMap::GPOFontMap() : embeddedFonts(NULL)
{
  initBase14Fonts();
}

GPOFontMap::~GPOFontMap()
{
  g_hash_table_destroy(base14Fonts);

  if (embeddedFonts != NULL) {
    g_hash_table_destroy(embeddedFonts);
    embeddedFonts = NULL;
  }
}

void GPOFontMap::startDoc(XRef *xrefA)
{
  this->xref = xrefA;
  
  if (embeddedFonts != NULL)
    g_hash_table_destroy(embeddedFonts);

  embeddedFonts = g_hash_table_new_full(g_str_hash, g_str_equal,
					g_free, g_object_unref);
}

void GPOFontMap::initBase14Fonts()
{
  base14Fonts = g_hash_table_new_full(g_str_hash, g_str_equal,
				      NULL, g_object_unref);

  for (guint i = 0; i < G_N_ELEMENTS(baseFontPSNames); ++i) {
    GnomeFontFace *gff = gnome_font_face_find(
			  (const guchar *)baseFontPSNames[i].name);
    if (gff != NULL)
      g_hash_table_insert(base14Fonts,
			  (gchar *)baseFontPSNames[i].psName, gff);
    else
      g_warning("Could not find base font %s", baseFontPSNames[i].name);
  }
}

GnomeFontFace *GPOFontMap::getFontFaceBase14(GfxFont *font)
{
  GString *gfont_name = font->getName();

  g_assert(gfont_name != NULL);

  GnomeFontFace *gff = GNOME_FONT_FACE (
    g_hash_table_lookup(base14Fonts, gfont_name->getCString()));
  g_object_ref(G_OBJECT (gff));

  return gff;
}

gboolean GPOFontMap::getStreamContents(Stream *str,
				       guchar **contents,
				       gsize *length)
{
#define STARTING_ALLOC 64

  size_t total_bytes = 0;
  size_t total_allocated = STARTING_ALLOC;
  guchar *dest = (guchar *)g_malloc(STARTING_ALLOC);

  gboolean eof = FALSE;

  while (!eof) {
    guchar buf[2048];
    guchar *p;
    size_t bytes;

    for (p = buf, bytes = 0; bytes < 2048; ++p, ++bytes) {
      int c = str->getChar();
      if (c == EOF) {
	eof = TRUE;
	break;
      }

      *p = (guchar) c;
    }

    while ((total_bytes + bytes + 1) > total_allocated) {
      total_allocated *= 2;
      dest = (guchar *)g_try_realloc(dest, total_allocated);

      if (dest == NULL) {
	g_warning ("Could not allocate enough bytes to read font");
	goto error;
      }
    }

    memcpy(dest + total_bytes, buf, bytes);
    total_bytes += bytes;
  }

  str->close();

  dest[total_bytes] = '\0';

  if (length)
    *length = total_bytes;

  *contents = dest;

  return TRUE;

 error:
  g_free(dest);
  str->close();

  return FALSE;
}

GnomeFontFace *GPOFontMap::getFontFaceEmbedded(GfxFont *font)
{
#ifdef HAVE_FONT_EMBEDDING

  g_return_val_if_fail(isEmbeddedFont(font), NULL);

  GString *gfont_name = font->getName();

  if (gfont_name == NULL)
    return getFontFaceFallback(font);

  gchar *font_name = gfont_name->getCString();

  GnomeFontFace *gff = (GnomeFontFace *)g_hash_table_lookup(embeddedFonts,
							    font_name);

  if (gff != NULL) {
    DBG (g_message("Reusing font face for %s", font_name));

    g_object_ref (G_OBJECT(gff));
    return gff;
  }

  // Extract the embedded font file
  Ref embRef;
  Object refObj, strObj;
  
  font->getEmbeddedFontID(&embRef);

  refObj.initRef(embRef.num, embRef.gen);
  refObj.fetch(xref, &strObj);
  refObj.free();

  guchar *contents;
  gsize length;

  strObj.streamReset();
  if (!getStreamContents(strObj.getStream(), &contents, &length))
    return getFontFaceFallback(font);

  strObj.free();

  switch (font->getType()) {
  case fontType1:
  case fontType1C: {
    gchar **encoding;
    gushort *code_to_gid;
    const char *name;

    gff = gpdf_font_face_download((const guchar *)font_name,
				  (const guchar *)"",
				  GNOME_FONT_REGULAR, FALSE,
				  contents, length);

    encoding = ((Gfx8BitFont *)font)->getEncoding();
    code_to_gid = g_new0(gushort, 256);

    GFF_LOADED(gff);

    for (int i = 0; i < 256; ++i)
      if ((name = encoding[i]) != NULL)
	code_to_gid[i] = (gushort)FT_Get_Name_Index(gff->ft_face,
						    (FT_String *)name);

    g_object_set_data((GObject *)gff, "code-to-gid-len",
		      GINT_TO_POINTER(256));
    g_object_set_data_full((GObject *)gff, "code-to-gid", code_to_gid,
			   (GDestroyNotify)g_free);
    break;
  }
  case fontTrueType: {
    FoFiTrueType *ff;
    gint fd;
    gchar *temp_name;
    FILE *f;
    gushort *code_to_gid;

    ff = FoFiTrueType::make((char *)contents, length);
    if (!ff)
      return getFontFaceFallback(font);

    code_to_gid = ((Gfx8BitFont *)font)->getCodeToGIDMap(ff); // this is g(oo)malloc'd

    fd = g_file_open_tmp("gpdf-ttf-XXXXXX", &temp_name, NULL);
    f = fdopen(fd, "wb");
    ff->writeTTF(&fileWrite, f);
    delete ff;
    g_free(contents);
    fclose(f);

    g_file_get_contents(temp_name, (gchar **)&contents, &length, NULL);
    unlink(temp_name);
    g_free(temp_name);

    gff = gpdf_font_face_download((const guchar *)font_name,
				  (const guchar *)"",
				  GNOME_FONT_REGULAR, FALSE,
				  contents, length);

    g_object_set_data((GObject *)gff, "code-to-gid-len",
		      GINT_TO_POINTER(256));
    g_object_set_data_full((GObject *)gff, "code-to-gid", code_to_gid,
			   (GDestroyNotify)gfree);
    break;
  }
  case fontCIDType0: {
    gff = gpdf_font_face_download((const guchar *)font_name,
				  (const guchar *)"",
				  GNOME_FONT_REGULAR, FALSE,
				  contents, length);
    g_object_set_data((GObject *)gff, "code-to-gid-len",
		      GINT_TO_POINTER(0));
    break;
  }
  case fontCIDType0C: {
    FoFiType1C *ff;
    gint n_cids;
#if HAVE_FREETYPE_217_OR_OLDER
    gushort *code_to_gid;
#endif
    
    ff = FoFiType1C::make((char *)contents, length);
    if (!ff)
      return getFontFaceFallback(font);

#if HAVE_FREETYPE_217_OR_OLDER
    code_to_gid = ff->getCIDToGIDMap(&n_cids); // this is g(oo)malloc'd
#endif
    delete ff;

    gff = gpdf_font_face_download((const guchar *)font_name,
				  (const guchar *)"",
				  GNOME_FONT_REGULAR, FALSE,
				  contents, length);

#if HAVE_FREETYPE_217_OR_OLDER
    /* freetype >= 2.1.8 can handle Type1C fonts fine without this */
    g_object_set_data((GObject *)gff, "code-to-gid-len",
		      GINT_TO_POINTER(n_cids));
    g_object_set_data_full((GObject *)gff, "code-to-gid", code_to_gid,
			   (GDestroyNotify)gfree);
#endif
    break;
  }
  case fontCIDType2: {
    FoFiTrueType *ff;
    gint fd;
    gchar *temp_name;
    FILE *f;
    gint n_cids;    
    gushort *code_to_gid;

    ff = FoFiTrueType::make((char *)contents, length);
    if (!ff)
      return getFontFaceFallback(font);

    fd = g_file_open_tmp("gpdf-ttf-XXXXXX", &temp_name, NULL);
    f = fdopen(fd, "wb");
    ff->writeTTF(&fileWrite, f);
    delete ff;
    g_free(contents);
    fclose(f);

    g_file_get_contents(temp_name, (gchar **)&contents, &length, NULL);
    unlink(temp_name);
    g_free(temp_name);

    gff = gpdf_font_face_download((const guchar *)font_name,
				  (const guchar *)"",
				  GNOME_FONT_REGULAR, FALSE,
				  contents, length);

    n_cids = ((GfxCIDFont *)font)->getCIDToGIDLen();
    code_to_gid = (gushort *)g_memdup(((GfxCIDFont *)font)->getCIDToGID(),
				      n_cids * sizeof(gushort));

    g_object_set_data((GObject *)gff, "code-to-gid-len",
		      GINT_TO_POINTER(n_cids));
    g_object_set_data_full((GObject *)gff, "code-to-gid", code_to_gid,
			   (GDestroyNotify)g_free);
    break;
  }
  default:
    return getFontFaceFallback(font);
  }

  DBG (g_message("Adding font face for %s", font_name));
  
  g_hash_table_insert(embeddedFonts, g_strdup(font_name), gff);
    
  g_object_ref (G_OBJECT(gff));
  return gff;
#else
  return NULL;
#endif /* HAVE_FONT_EMBEDDING */
}

// index: {fixed:8, serif:4, sans-serif:0} + bold*2 + italic
static const char *fontSubst[16] = {
  "Sans Regular",
  "Sans Italic",
  "Sans Bold",
  "Sans BoldOblique",
  "Serif Regular",
  "Serif Italic",
  "Serif Bold",
  "Serif Bold Italic",
  "Monospace Regular",
  "Monospace Italic",
  "Monospace Bold",
  "Monospace Bold Italic"
};

GnomeFontFace *GPOFontMap::getFontFaceFallback(GfxFont *font)
{
  int subst_index;

  if (font->isFixedWidth())
    subst_index = 8;
  else if (font->isSerif())
    subst_index = 4;
  else
    subst_index = 0;

  if (font->isBold())
    subst_index += 2;
  
  if (font->isItalic())
    subst_index += 1;

  return gnome_font_face_find((const guchar *)fontSubst[subst_index]);  
}

GnomeFontFace *GPOFontMap::getFontFace(GfxFont *font)
{
#ifdef HAVE_FONT_EMBEDDING
  if (isEmbeddedFont(font))
    return getFontFaceEmbedded(font);
  else
#endif
  if (isBase14Font(font))
    return getFontFaceBase14(font);
  else
    return getFontFaceFallback(font);
}

GPOutputDev::GPOutputDev() : gpc(NULL),
			     page_name(NULL),
			     stroke_color_set(FALSE),
			     fill_color_set(FALSE),
			     gnome_font(NULL),
			     text_pos_x(0.0), text_pos_y(0.0) {}

GPOutputDev::~GPOutputDev()
{
  if (gnome_font != NULL)
    g_object_unref(G_OBJECT(gnome_font));
  setPrintContext(NULL);
}

void GPOutputDev::setPrintContext(GnomePrintContext *pc)
{
  if (gpc)
    g_object_unref(gpc);

  gpc = pc;
  font_map.setPrintContext(gpc);

  if (pc)
    g_object_ref(pc);
}

void GPOutputDev::startDoc(XRef *xref)
{
  font_map.startDoc(xref);
}

void GPOutputDev::startPage(int pageNum, GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);
  g_return_if_fail(state != NULL);

  fill_color_set = stroke_color_set = FALSE;
  page_name = g_strdup_printf("%d", pageNum);

  gnome_print_beginpage(gpc, (const guchar *)page_name);
}

void GPOutputDev::endPage()
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  gnome_print_showpage(gpc);

  if (page_name) {
    g_free(page_name);
    page_name = NULL;
  }


  DBG (if (gnome_font)
       g_message("Font in endPage: %s", gnome_font_get_name(gnome_font)));
}


// save/restore graphics state

void GPOutputDev::saveState(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  gnome_print_gsave(gpc);
}

void GPOutputDev::restoreState(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  fill_color_set = stroke_color_set = FALSE;
  gnome_print_grestore(gpc);
}


// update graphics state

/*
 * This is a misnomer! The semantics are:
 *   - update CTM from state or
 *   - concat [mij] to CTM
 */
void GPOutputDev::updateCTM(GfxState *state, double m11, double m12,
			    double m21, double m22, double m31, double m32)
{
  const gdouble matrix[] = {m11, m12, m21, m22, m31, m32};

  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  /* We choose the second way */
  gnome_print_concat(gpc, matrix);
}

void GPOutputDev::updateLineDash(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  gint n_values;
  gdouble *values;
  gdouble offset;

  state->getLineDash(&values, &n_values, &offset);
  gnome_print_setdash(gpc, n_values, values, offset);
}

void GPOutputDev::updateLineJoin(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  gnome_print_setlinejoin(gpc, state->getLineJoin());
}

void GPOutputDev::updateLineCap(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

//   GdkCapStyle gdk_cap = GDK_CAP_BUTT;

  /* ??? FIXME fix gnome print preview which should do something like  */
//   switch (state->getLineCap()) {
//   case 0: gdk_cap = GDK_CAP_BUTT; break;
//   case 1: gdk_cap = GDK_CAP_ROUND; break;
//   case 2: gdk_cap = GDK_CAP_PROJECTING; break;
//   default: g_assert_not_reached();
//   }

  gnome_print_setlinecap(gpc, state->getLineCap());
}

void GPOutputDev::updateMiterLimit(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  gnome_print_setmiterlimit(gpc, state->getMiterLimit());
}

void GPOutputDev::updateLineWidth(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  gnome_print_setlinewidth(gpc, state->getLineWidth());
}

void GPOutputDev::updateFillColor(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  GfxRGB rgb;
  state->getFillRGB(&rgb);

  fill_color_set = TRUE;
  stroke_color_set = !fill_color_set;
  gnome_print_setrgbcolor(gpc, rgb.r, rgb.g, rgb.b);
  gnome_print_setopacity(gpc, state->getFillOpacity());
}

void GPOutputDev::updateFillColorIfNecessary(GfxState *state)
{
  if (!fill_color_set) updateFillColor(state);
}

void GPOutputDev::updateFillOpacity(GfxState *state)
{
  updateFillColor(state);
}

void GPOutputDev::updateStrokeColor(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  GfxRGB rgb;
  state->getStrokeRGB(&rgb);

  stroke_color_set = TRUE;
  fill_color_set = !stroke_color_set;
  gnome_print_setrgbcolor(gpc, rgb.r, rgb.g, rgb.b);
  gnome_print_setopacity(gpc, state->getStrokeOpacity());
}

void GPOutputDev::updateStrokeColorIfNecessary(GfxState *state)
{
  if (!stroke_color_set) updateStrokeColor(state);
}

void GPOutputDev::updateStrokeOpacity(GfxState *state)
{
  updateStrokeColor(state);
}


// update text state
void GPOutputDev::updateFont(GfxState *state)
{
  if (state->getFont() == NULL)
    return;

  if (gnome_font)
    gnome_font_unref (gnome_font);

  GnomeFontFace *font_face = font_map.getFontFace(state->getFont());
  gnome_font = gnome_font_face_get_font_default(font_face,
						state->getFontSize());
  gnome_font_face_unref(font_face);
}

void GPOutputDev::updateTextPos(GfxState *state)
{
  text_pos_x = state->getLineX();
  text_pos_y = state->getLineY();
}

void GPOutputDev::updateTextShift(GfxState *state, double shift)
{
  gdouble text_shift = shift * state->getFontSize() * 0.001;

  if (state->getFont()->getWMode() == 0)
    text_pos_x -= text_shift;
  else
    text_pos_y -= text_shift;
}


// path painting
void GPOutputDev::doPath(GfxPath *path)
{
  gint n = path->getNumSubpaths();

  gnome_print_newpath(gpc);

  for (gint i = 0; i < n; ++i) {

    GfxSubpath *subpath = path->getSubpath(i);
    gint m = subpath->getNumPoints();
    gnome_print_moveto(gpc, subpath->getX(0), subpath->getY(0));

    for (gint j = 1; j < m; ) {
      if (subpath->getCurve(j)) {
	gnome_print_curveto(gpc, subpath->getX(j), subpath->getY(j),
			    subpath->getX(j+1), subpath->getY(j+1),
			    subpath->getX(j+2), subpath->getY(j+2));
	j += 3;
      } else {
	gnome_print_lineto(gpc, subpath->getX(j), subpath->getY(j));
	++j;
      }
    }

    if (subpath->isClosed())
      gnome_print_closepath(gpc);
  }
}

void GPOutputDev::stroke(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  doPath(state->getPath());
  updateStrokeColorIfNecessary(state);
  gnome_print_stroke(gpc);
}

void GPOutputDev::fill(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  doPath(state->getPath());
  updateFillColorIfNecessary(state);
  gnome_print_fill(gpc);
}

void GPOutputDev::eoFill(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  doPath(state->getPath());
  updateFillColorIfNecessary(state);
  gnome_print_eofill(gpc);
}

void GPOutputDev::clip(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  doPath(state->getPath());
  gnome_print_clip(gpc);
}

void GPOutputDev::eoClip(GfxState *state)
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  doPath(state->getPath());
  gnome_print_eoclip(gpc);
}

static gint getFillColorRGBA(GfxState *state)
{
  GfxRGB rgb;
  state->getFillRGB(&rgb);
  gdouble alpha = state->getFillOpacity();

  return (((gint)(CLAMP (rgb.r, 0.0, 1.0) * 255.999) << 24) |
	  ((gint)(CLAMP (rgb.g, 0.0, 1.0) * 255.999) << 16) |
	  ((gint)(CLAMP (rgb.b, 0.0, 1.0) * 255.999) <<  8) |
	  ((gint)(CLAMP (alpha, 0.0, 1.0) * 255.999)));
}

static gint lookupGlyph(GnomeFont *font, CharCode c)
{
  gint code_to_gid_len;
  const gushort *code_to_gid;

  code_to_gid = (const gushort *)g_object_get_data((GObject *)font->face,
						   "code-to-gid");
  code_to_gid_len = GPOINTER_TO_INT(g_object_get_data((GObject *)font->face,
						      "code-to-gid-len"));

  if (code_to_gid && (gint)c < code_to_gid_len)
    return code_to_gid[c];
  else
    return c;
}

// text drawing
void GPOutputDev::drawString(GfxState *state, GString *s) /* FIXME */
{
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  // check for invisible text -- this is used by Acrobat Capture
  if ((state->getRender() & 3) == 3)
    return;

  // ignore empty strings
  if (s->getLength() == 0)
    return;

  GfxFont *font = state->getFont();
  if (font == NULL || gnome_font == NULL)
    return;

  gnome_print_gsave(gpc);

  gdouble fontmat[6];
  memcpy(fontmat, state->getTextMat(), 6 * sizeof(gdouble));
  fontmat[0] *= state->getHorizScaling();
  fontmat[2] *= state->getHorizScaling();

  gnome_print_concat(gpc, fontmat);
  gnome_print_moveto(gpc, text_pos_x, text_pos_y);

  GnomeGlyphList *ggl = gnome_glyphlist_new();
  gnome_glyphlist_font(ggl, gnome_font);
  gnome_glyphlist_color(ggl, getFillColorRGBA(state));
  gnome_glyphlist_advance(ggl, FALSE);

  gnome_glyphlist_rmoveto(ggl, 0.0, state->getRise());

  gchar *p = s->getCString();
  gint len = s->getLength();
  while (len > 0) {
    CharCode code;
    Unicode u[8];
    gint uLen;
    gdouble dx = 0;
    gdouble dy = 0;
    gdouble originX, originY;

    gint n = font->getNextChar(p, len, &code,
			       u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
			       &dx, &dy, &originX, &originY);

    /* FIXME add vertical writing mode code */
    dx = dx * state->getFontSize() + state->getCharSpace();
    if (n == 1 && *p == ' ')
      dx += state->getWordSpace();
    dy *= state->getFontSize ();

    DBG (if (uLen > 1)
      g_warning("drawString: uLen > 1; p: %x", (gint)*p));

    Ref embRef;
    if (font->getEmbeddedFontID(&embRef)) // FIXME HACK
      gnome_glyphlist_glyph(ggl, lookupGlyph(gnome_font, code));
    else
      gnome_glyphlist_glyph(
	ggl, gnome_font_lookup_default(gnome_font, uLen > 0 ? u[0] : code));

    gnome_glyphlist_rmoveto(ggl, dx, dy);

    text_pos_x += dx;
    text_pos_y += dy;

    p += n;
    len -= n;
  }

  gnome_print_glyphlist(gpc, ggl);
  gnome_glyphlist_unref(ggl);

  gnome_print_grestore(gpc);
}

void GPOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				int width, int height, GBool invert,
				GBool inlineImg)
{
  /* FIXME: add image mask operation to gnome-print ? */
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  ImageStream *imgStr = new ImageStream(str, width, 1, 1);
  imgStr->reset();

  GfxRGB maskColor;
  state->getFillRGB(&maskColor);

  guchar red = (guchar)(maskColor.r * 255.999);
  guchar green = (guchar)(maskColor.g * 255.999);
  guchar blue = (guchar)(maskColor.b * 255.999);

  guchar *pixbuf = (guchar *)g_malloc(width * height * 4);
  g_return_if_fail(pixbuf != NULL);

  guchar *p = pixbuf;
  guchar alpha;
  for (gint i = 0; i < width * height; ++i) {
    imgStr->getPixel(&alpha);

    *p++ = red;
    *p++ = green;
    *p++ = blue;
    if (invert)
      *p++ = alpha ? 255 : 0;
    else
      *p++ = alpha ? 0 : 255;
  }

  gnome_print_rgbaimage(gpc, pixbuf, width, height, width * 4);

  g_free(pixbuf);
  delete imgStr;
}

void GPOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			    int width, int height, GfxImageColorMap *colorMap,
			    int *maskColors, GBool inlineImg)
{
  /* FIXME: transparency, special case grayscale images? */
  g_return_if_fail(HAS_NON_NULL_PRINT_CONTEXT);

  ImageStream *imgStr = new ImageStream(str, width,
					colorMap->getNumPixelComps(),
					colorMap->getBits());
  imgStr->reset();

  guchar *pixbuf = (guchar *)g_malloc(width * height * 3);
  g_return_if_fail(pixbuf != NULL);

  guchar *p = pixbuf;
  Guchar pix[gfxColorMaxComps];
  GfxRGB rgb;
  for (gint i = 0; i < width * height; ++i) {
    imgStr->getPixel(pix);
    colorMap->getRGB(pix, &rgb);

    *p++ = (guchar)(rgb.r * 255.999);
    *p++ = (guchar)(rgb.g * 255.999);
    *p++ = (guchar)(rgb.b * 255.999);
  }

  gnome_print_rgbimage(gpc, pixbuf, width, height, width * 3);

  g_free(pixbuf);
  delete imgStr;
}
