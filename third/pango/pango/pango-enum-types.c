
/* Generated data (by glib-mkenums) */

#include <pango.h>

/* enumerations from "pango-attributes.h" */
GType
pango_attr_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_ATTR_INVALID, "PANGO_ATTR_INVALID", "invalid" },
      { PANGO_ATTR_LANGUAGE, "PANGO_ATTR_LANGUAGE", "language" },
      { PANGO_ATTR_FAMILY, "PANGO_ATTR_FAMILY", "family" },
      { PANGO_ATTR_STYLE, "PANGO_ATTR_STYLE", "style" },
      { PANGO_ATTR_WEIGHT, "PANGO_ATTR_WEIGHT", "weight" },
      { PANGO_ATTR_VARIANT, "PANGO_ATTR_VARIANT", "variant" },
      { PANGO_ATTR_STRETCH, "PANGO_ATTR_STRETCH", "stretch" },
      { PANGO_ATTR_SIZE, "PANGO_ATTR_SIZE", "size" },
      { PANGO_ATTR_FONT_DESC, "PANGO_ATTR_FONT_DESC", "font-desc" },
      { PANGO_ATTR_FOREGROUND, "PANGO_ATTR_FOREGROUND", "foreground" },
      { PANGO_ATTR_BACKGROUND, "PANGO_ATTR_BACKGROUND", "background" },
      { PANGO_ATTR_UNDERLINE, "PANGO_ATTR_UNDERLINE", "underline" },
      { PANGO_ATTR_STRIKETHROUGH, "PANGO_ATTR_STRIKETHROUGH", "strikethrough" },
      { PANGO_ATTR_RISE, "PANGO_ATTR_RISE", "rise" },
      { PANGO_ATTR_SHAPE, "PANGO_ATTR_SHAPE", "shape" },
      { PANGO_ATTR_SCALE, "PANGO_ATTR_SCALE", "scale" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoAttrType", values);
  }
  return etype;
}

GType
pango_underline_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_UNDERLINE_NONE, "PANGO_UNDERLINE_NONE", "none" },
      { PANGO_UNDERLINE_SINGLE, "PANGO_UNDERLINE_SINGLE", "single" },
      { PANGO_UNDERLINE_DOUBLE, "PANGO_UNDERLINE_DOUBLE", "double" },
      { PANGO_UNDERLINE_LOW, "PANGO_UNDERLINE_LOW", "low" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoUnderline", values);
  }
  return etype;
}


/* enumerations from "pango-coverage.h" */
GType
pango_coverage_level_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_COVERAGE_NONE, "PANGO_COVERAGE_NONE", "none" },
      { PANGO_COVERAGE_FALLBACK, "PANGO_COVERAGE_FALLBACK", "fallback" },
      { PANGO_COVERAGE_APPROXIMATE, "PANGO_COVERAGE_APPROXIMATE", "approximate" },
      { PANGO_COVERAGE_EXACT, "PANGO_COVERAGE_EXACT", "exact" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoCoverageLevel", values);
  }
  return etype;
}


/* enumerations from "pango-font.h" */
GType
pango_style_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_STYLE_NORMAL, "PANGO_STYLE_NORMAL", "normal" },
      { PANGO_STYLE_OBLIQUE, "PANGO_STYLE_OBLIQUE", "oblique" },
      { PANGO_STYLE_ITALIC, "PANGO_STYLE_ITALIC", "italic" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoStyle", values);
  }
  return etype;
}

GType
pango_variant_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_VARIANT_NORMAL, "PANGO_VARIANT_NORMAL", "normal" },
      { PANGO_VARIANT_SMALL_CAPS, "PANGO_VARIANT_SMALL_CAPS", "small-caps" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoVariant", values);
  }
  return etype;
}

GType
pango_weight_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_WEIGHT_ULTRALIGHT, "PANGO_WEIGHT_ULTRALIGHT", "ultralight" },
      { PANGO_WEIGHT_LIGHT, "PANGO_WEIGHT_LIGHT", "light" },
      { PANGO_WEIGHT_NORMAL, "PANGO_WEIGHT_NORMAL", "normal" },
      { PANGO_WEIGHT_BOLD, "PANGO_WEIGHT_BOLD", "bold" },
      { PANGO_WEIGHT_ULTRABOLD, "PANGO_WEIGHT_ULTRABOLD", "ultrabold" },
      { PANGO_WEIGHT_HEAVY, "PANGO_WEIGHT_HEAVY", "heavy" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoWeight", values);
  }
  return etype;
}

GType
pango_stretch_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_STRETCH_ULTRA_CONDENSED, "PANGO_STRETCH_ULTRA_CONDENSED", "ultra-condensed" },
      { PANGO_STRETCH_EXTRA_CONDENSED, "PANGO_STRETCH_EXTRA_CONDENSED", "extra-condensed" },
      { PANGO_STRETCH_CONDENSED, "PANGO_STRETCH_CONDENSED", "condensed" },
      { PANGO_STRETCH_SEMI_CONDENSED, "PANGO_STRETCH_SEMI_CONDENSED", "semi-condensed" },
      { PANGO_STRETCH_NORMAL, "PANGO_STRETCH_NORMAL", "normal" },
      { PANGO_STRETCH_SEMI_EXPANDED, "PANGO_STRETCH_SEMI_EXPANDED", "semi-expanded" },
      { PANGO_STRETCH_EXPANDED, "PANGO_STRETCH_EXPANDED", "expanded" },
      { PANGO_STRETCH_EXTRA_EXPANDED, "PANGO_STRETCH_EXTRA_EXPANDED", "extra-expanded" },
      { PANGO_STRETCH_ULTRA_EXPANDED, "PANGO_STRETCH_ULTRA_EXPANDED", "ultra-expanded" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoStretch", values);
  }
  return etype;
}

GType
pango_font_mask_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { PANGO_FONT_MASK_FAMILY, "PANGO_FONT_MASK_FAMILY", "family" },
      { PANGO_FONT_MASK_STYLE, "PANGO_FONT_MASK_STYLE", "style" },
      { PANGO_FONT_MASK_VARIANT, "PANGO_FONT_MASK_VARIANT", "variant" },
      { PANGO_FONT_MASK_WEIGHT, "PANGO_FONT_MASK_WEIGHT", "weight" },
      { PANGO_FONT_MASK_STRETCH, "PANGO_FONT_MASK_STRETCH", "stretch" },
      { PANGO_FONT_MASK_SIZE, "PANGO_FONT_MASK_SIZE", "size" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("PangoFontMask", values);
  }
  return etype;
}


/* enumerations from "pango-layout.h" */
GType
pango_alignment_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_ALIGN_LEFT, "PANGO_ALIGN_LEFT", "left" },
      { PANGO_ALIGN_CENTER, "PANGO_ALIGN_CENTER", "center" },
      { PANGO_ALIGN_RIGHT, "PANGO_ALIGN_RIGHT", "right" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoAlignment", values);
  }
  return etype;
}

GType
pango_wrap_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_WRAP_WORD, "PANGO_WRAP_WORD", "word" },
      { PANGO_WRAP_CHAR, "PANGO_WRAP_CHAR", "char" },
      { PANGO_WRAP_WORD_CHAR, "PANGO_WRAP_WORD_CHAR", "word-char" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoWrapMode", values);
  }
  return etype;
}


/* enumerations from "pango-tabs.h" */
GType
pango_tab_align_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_TAB_LEFT, "PANGO_TAB_LEFT", "left" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoTabAlign", values);
  }
  return etype;
}


/* enumerations from "pango-types.h" */
GType
pango_direction_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PANGO_DIRECTION_LTR, "PANGO_DIRECTION_LTR", "ltr" },
      { PANGO_DIRECTION_RTL, "PANGO_DIRECTION_RTL", "rtl" },
      { PANGO_DIRECTION_TTB_LTR, "PANGO_DIRECTION_TTB_LTR", "ttb-ltr" },
      { PANGO_DIRECTION_TTB_RTL, "PANGO_DIRECTION_TTB_RTL", "ttb-rtl" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PangoDirection", values);
  }
  return etype;
}


/* Generated data ends here */

