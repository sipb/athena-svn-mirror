/* -*- mode: C; c-file-style: "gnu" -*- */
/* pango
 * pango-attributes.c: Attributed text
 *
 * Copyright (C) 2000-2002 Red Hat Software
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include "pango-attributes.h"
#include "pango-utils.h"

struct _PangoAttrList
{
  guint ref_count;
  GSList *attributes;
  GSList *attributes_tail;
};

struct _PangoAttrIterator
{
  GSList *next_attribute;
  GList *attribute_stack;
  int start_index;
  int end_index;
};

static PangoAttribute *pango_attr_color_new  (const PangoAttrClass *klass,
                                              guint16               red,
                                              guint16               green,
                                              guint16               blue);
static PangoAttribute *pango_attr_string_new (const PangoAttrClass *klass,
                                              const char           *str);
static PangoAttribute *pango_attr_int_new    (const PangoAttrClass *klass,
                                              int                   value);
static PangoAttribute *pango_attr_float_new  (const PangoAttrClass *klass,
                                              double                value);


/**
 * pango_attr_type_register:
 * @name: an identifier for the type. (Currently unused.)
 * 
 * Allocate a new attribute type ID.
 * 
 * Return value: the new type ID.
 **/
PangoAttrType
pango_attr_type_register (const gchar *name)
{
  static guint current_type = 0x1000000;

  return current_type++;
}

/**
 * pango_attribute_copy:
 * @attr: a #PangoAttribute.
 * 
 * Make a copy of an attribute.
 * 
 * Return value: a newly allocated #PangoAttribute.
 **/
PangoAttribute *
pango_attribute_copy (const PangoAttribute *attr)
{
  PangoAttribute *result;

  g_return_val_if_fail (attr != NULL, NULL);

  result = attr->klass->copy (attr);
  result->start_index = attr->start_index;
  result->end_index = attr->end_index;

  return result;
}

/**
 * pango_attribute_destroy:
 * @attr: a #PangoAttribute.
 * 
 * Destroy a #PangoAttribute and free all associated memory.
 **/
void
pango_attribute_destroy (PangoAttribute *attr)
{
  g_return_if_fail (attr != NULL);

  attr->klass->destroy (attr);
}

/**
 * pango_attribute_equal:
 * @attr1: a #PangoAttribute
 * @attr2: another #PangoAttribute
 * 
 * Compare two attributes for equality. This compares only the
 * actual value of the two attributes and not the ranges that the
 * attributes apply to.
 * 
 * Return value: %TRUE if the two attributes have the same value.
 **/
gboolean
pango_attribute_equal (const PangoAttribute *attr1,
		       const PangoAttribute *attr2)
{
  g_return_val_if_fail (attr1 != NULL, FALSE);
  g_return_val_if_fail (attr2 != NULL, FALSE);

  if (attr1->klass->type != attr2->klass->type)
    return FALSE;

  return attr1->klass->equal (attr1, attr2);
}

static PangoAttribute *
pango_attr_string_copy (const PangoAttribute *attr)
{
  return pango_attr_string_new (attr->klass, ((PangoAttrString *)attr)->value);
}

static void
pango_attr_string_destroy (PangoAttribute *attr)
{
  g_free (((PangoAttrString *)attr)->value);
  g_free (attr);
}

static gboolean
pango_attr_string_equal (const PangoAttribute *attr1,
			 const PangoAttribute *attr2)
{
  return strcmp (((PangoAttrString *)attr1)->value, ((PangoAttrString *)attr2)->value) == 0;
}

static PangoAttribute *
pango_attr_string_new (const PangoAttrClass *klass,
		       const char           *str)
{
  PangoAttrString *result = g_new (PangoAttrString, 1);

  result->attr.klass = klass;
  result->value = g_strdup (str);

  return (PangoAttribute *)result;
}

/**
 * pango_attr_family_new:
 * @family: the family or comma separated list of families
 * 
 * Create a new font family attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_family_new (const char *family)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_FAMILY,
    pango_attr_string_copy,
    pango_attr_string_destroy,
    pango_attr_string_equal
  };

  g_return_val_if_fail (family != NULL, NULL);
  
  return pango_attr_string_new (&klass, family);
}

static PangoAttribute *
pango_attr_language_copy (const PangoAttribute *attr)
{
  return g_memdup (attr, sizeof (PangoAttrLanguage));
}

static void
pango_attr_language_destroy (PangoAttribute *attr)
{
  g_free (attr);
}

static gboolean
pango_attr_language_equal (const PangoAttribute *attr1,
			   const PangoAttribute *attr2)
{
  return ((PangoAttrLanguage *)attr1)->value == ((PangoAttrLanguage *)attr2)->value;
}

/**
 * pango_attr_language_new:
 * @language: language tag
 * 
 * Create a new language tag attribute. 
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_language_new (PangoLanguage *language)
{
  PangoAttrLanguage *result;
  
  static const PangoAttrClass klass = {
    PANGO_ATTR_LANGUAGE,
    pango_attr_language_copy,
    pango_attr_language_destroy,
    pango_attr_language_equal
  };

  g_return_val_if_fail (language != NULL, NULL);
  
  result = g_new (PangoAttrLanguage, 1);

  result->attr.klass = &klass;
  result->value = language;

  return (PangoAttribute *)result;
}

static PangoAttribute *
pango_attr_color_copy (const PangoAttribute *attr)
{
  const PangoAttrColor *color_attr = (PangoAttrColor *)attr;
  
  return pango_attr_color_new (attr->klass,
			       color_attr->color.red,
                               color_attr->color.green,
                               color_attr->color.blue);
}

static void
pango_attr_color_destroy (PangoAttribute *attr)
{
  g_free (attr);
}

static gboolean
pango_attr_color_equal (const PangoAttribute *attr1,
			const PangoAttribute *attr2)
{
  const PangoAttrColor *color_attr1 = (const PangoAttrColor *)attr1;
  const PangoAttrColor *color_attr2 = (const PangoAttrColor *)attr2;

  return (color_attr1->color.red == color_attr2->color.red &&
	  color_attr1->color.blue == color_attr2->color.blue &&
	  color_attr1->color.green == color_attr2->color.green);
}

static PangoAttribute *
pango_attr_color_new (const PangoAttrClass *klass,
		      guint16               red,
		      guint16               green,
		      guint16               blue)
{
  PangoAttrColor *result = g_new (PangoAttrColor, 1);
  result->attr.klass = klass;
  result->color.red = red;
  result->color.green = green;
  result->color.blue = blue;

  return (PangoAttribute *)result;
}

/**
 * pango_attr_foreground_new:
 * @red: the red value (ranging from 0 to 65535)
 * @green: the green value
 * @blue: the blue value
 * 
 * Create a new foreground color attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_foreground_new (guint16 red,
			   guint16 green,
			   guint16 blue)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_FOREGROUND,
    pango_attr_color_copy,
    pango_attr_color_destroy,
    pango_attr_color_equal
  };

  return pango_attr_color_new (&klass, red, green, blue);
}

/**
 * pango_attr_background_new:
 * @red: the red value (ranging from 0 to 65535)
 * @green: the green value
 * @blue: the blue value
 * 
 * Create a new background color attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_background_new (guint16 red,
			   guint16 green,
			   guint16 blue)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_BACKGROUND,
    pango_attr_color_copy,
    pango_attr_color_destroy,
    pango_attr_color_equal
  };

  return pango_attr_color_new (&klass, red, green, blue);
}

static PangoAttribute *
pango_attr_int_copy (const PangoAttribute *attr)
{
  const PangoAttrInt *int_attr = (PangoAttrInt *)attr;
  
  return pango_attr_int_new (attr->klass, int_attr->value);
}

static void
pango_attr_int_destroy (PangoAttribute *attr)
{
  g_free (attr);
}

static gboolean
pango_attr_int_equal (const PangoAttribute *attr1,
		      const PangoAttribute *attr2)
{
  const PangoAttrInt *int_attr1 = (const PangoAttrInt *)attr1;
  const PangoAttrInt *int_attr2 = (const PangoAttrInt *)attr2;
  
  return (int_attr1->value == int_attr2->value);
}

static PangoAttribute *
pango_attr_int_new (const PangoAttrClass *klass,
		    int                   value)
{
  PangoAttrInt *result = g_new (PangoAttrInt, 1);
  result->attr.klass = klass;
  result->value = value;

  return (PangoAttribute *)result;
}

static PangoAttribute *
pango_attr_float_copy (const PangoAttribute *attr)
{
  const PangoAttrFloat *float_attr = (PangoAttrFloat *)attr;
  
  return pango_attr_float_new (attr->klass, float_attr->value);
}

static void
pango_attr_float_destroy (PangoAttribute *attr)
{
  g_free (attr);
}

static gboolean
pango_attr_float_equal (const PangoAttribute *attr1,
			const PangoAttribute *attr2)
{
  const PangoAttrFloat *float_attr1 = (const PangoAttrFloat *)attr1;
  const PangoAttrFloat *float_attr2 = (const PangoAttrFloat *)attr2;
  
  return (float_attr1->value == float_attr2->value);
}

static PangoAttribute*
pango_attr_float_new  (const PangoAttrClass *klass,
                       double                value)
{
  PangoAttrFloat *result = g_new (PangoAttrFloat, 1);
  result->attr.klass = klass;
  result->value = value;

  return (PangoAttribute *)result;
}

/**
 * pango_attr_size_new:
 * @size: the font size, in 1000ths of a point.
 * 
 * Create a new font-size attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_size_new (int size)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_SIZE,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, size);
}

/**
 * pango_attr_style_new:
 * @style: the slant style
 * 
 * Create a new font slant style attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_style_new (PangoStyle style)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_STYLE,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, (int)style);
}

/**
 * pango_attr_weight_new:
 * @weight: the weight
 * 
 * Create a new font weight attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_weight_new (PangoWeight weight)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_WEIGHT,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, (int)weight);
}

/**
 * pango_attr_variant_new:
 * @variant: the variant
 *
 * Create a new font variant attribute (normal or small caps)
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_variant_new (PangoVariant variant)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_VARIANT,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, (int)variant);
}

/**
 * pango_attr_stretch_new:
 * @stretch: the stretch
 * 
 * Create a new font stretch attribute
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_stretch_new (PangoStretch  stretch)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_STRETCH,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, (int)stretch);
}

static PangoAttribute *
pango_attr_font_desc_copy (const PangoAttribute *attr)
{
  const PangoAttrFontDesc *desc_attr = (const PangoAttrFontDesc *)attr;
  
  return pango_attr_font_desc_new (desc_attr->desc);
}

static void
pango_attr_font_desc_destroy (PangoAttribute *attr)
{
  PangoAttrFontDesc *desc_attr = (PangoAttrFontDesc *)attr;

  pango_font_description_free (desc_attr->desc);
  g_free (attr);
}

static gboolean
pango_attr_font_desc_equal (const PangoAttribute *attr1,
			    const PangoAttribute *attr2)
{
  const PangoAttrFontDesc *desc_attr1 = (const PangoAttrFontDesc *)attr1;
  const PangoAttrFontDesc *desc_attr2 = (const PangoAttrFontDesc *)attr2;
  
  return pango_font_description_equal (desc_attr1->desc, desc_attr2->desc);
}

/**
 * pango_attr_font_desc_new:
 * @desc: 
 * 
 * Create a new font description attribute. (This attribute
 * allows setting family, style, weight, variant, stretch,
 * and size simultaneously.)
 * 
 * Return value: 
 **/
PangoAttribute *
pango_attr_font_desc_new (const PangoFontDescription *desc)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_FONT_DESC,
    pango_attr_font_desc_copy,
    pango_attr_font_desc_destroy,
    pango_attr_font_desc_equal
  };

  PangoAttrFontDesc *result = g_new (PangoAttrFontDesc, 1);
  result->attr.klass = &klass;
  result->desc = pango_font_description_copy (desc);

  return (PangoAttribute *)result;
}


/**
 * pango_attr_underline_new:
 * @underline: the underline style.
 * 
 * Create a new underline-style object.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_underline_new (PangoUnderline underline)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_UNDERLINE,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, (int)underline);
}

/**
 * pango_attr_strikethrough_new:
 * @strikethrough: %TRUE if the text should be struck-through.
 * 
 * Create a new font strike-through attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_strikethrough_new (gboolean strikethrough)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_STRIKETHROUGH,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, (int)strikethrough);
}

/**
 * pango_attr_rise_new:
 * @rise: the amount that the text should be displaced vertically,
 *        in 10'000ths of an em. Positive values displace the
 *        text upwards.
 * 
 * Create a new baseline displacement attribute.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute *
pango_attr_rise_new (int rise)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_RISE,
    pango_attr_int_copy,
    pango_attr_int_destroy,
    pango_attr_int_equal
  };

  return pango_attr_int_new (&klass, (int)rise);
}

/**
 * pango_attr_scale_new:
 * @scale_factor: factor to scale the font
 * 
 * Create a new font size scale attribute. The base font for the
 * affected text will have its size multiplied by @scale_factor.
 * 
 * Return value: the new #PangoAttribute.
 **/
PangoAttribute*
pango_attr_scale_new (double scale_factor)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_SCALE,
    pango_attr_float_copy,
    pango_attr_float_destroy,
    pango_attr_float_equal
  };

  return pango_attr_float_new (&klass, scale_factor);
}

static PangoAttribute *
pango_attr_shape_copy (const PangoAttribute *attr)
{
  const PangoAttrShape *shape_attr = (PangoAttrShape *)attr;
  
  return pango_attr_shape_new (&shape_attr->ink_rect, &shape_attr->logical_rect);
}

static void
pango_attr_shape_destroy (PangoAttribute *attr)
{
  g_free (attr);
}

static gboolean
pango_attr_shape_equal (const PangoAttribute *attr1,
			const PangoAttribute *attr2)
{
  const PangoAttrShape *shape_attr1 = (const PangoAttrShape *)attr1;
  const PangoAttrShape *shape_attr2 = (const PangoAttrShape *)attr2;
  
  return (shape_attr1->logical_rect.x == shape_attr2->logical_rect.x &&
	  shape_attr1->logical_rect.y == shape_attr2->logical_rect.y &&
	  shape_attr1->logical_rect.width == shape_attr2->logical_rect.width &&
	  shape_attr1->logical_rect.height == shape_attr2->logical_rect.height &&
	  shape_attr1->ink_rect.x == shape_attr2->ink_rect.x &&
	  shape_attr1->ink_rect.y == shape_attr2->ink_rect.y &&
	  shape_attr1->ink_rect.width == shape_attr2->ink_rect.width &&
	  shape_attr1->ink_rect.height == shape_attr2->ink_rect.height);
}

/**
 * pango_attr_shape_new:
 * @ink_rect:     ink rectangle to assign to each character
 * @logical_rect: logical rectangle assign to each character
 * 
 * Create a new shape attribute. A shape is used to impose a
 * particular ink and logical rect on the result of shaping a
 * particular glyph. This might be used, for instance, for
 * embedding a picture or a widget inside a PangoLayout.
 * 
 * Return value: the newly created attribute
 **/
PangoAttribute *
pango_attr_shape_new (const PangoRectangle *ink_rect,
		      const PangoRectangle *logical_rect)
{
  static const PangoAttrClass klass = {
    PANGO_ATTR_SHAPE,
    pango_attr_shape_copy,
    pango_attr_shape_destroy,
    pango_attr_shape_equal
  };

  PangoAttrShape *result;

  g_return_val_if_fail (ink_rect != NULL, NULL);
  g_return_val_if_fail (logical_rect != NULL, NULL);
  
  result = g_new (PangoAttrShape, 1);
  result->attr.klass = &klass;
  result->ink_rect = *ink_rect;
  result->logical_rect = *logical_rect;

  return (PangoAttribute *)result;
}

GType
pango_attr_list_get_type (void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static ("PangoAttrList",
					     (GBoxedCopyFunc) pango_attr_list_copy,
					     (GBoxedFreeFunc) pango_attr_list_unref);

  return our_type;
}

/**
 * pango_attr_list_new:
 * 
 * Create a new empty attribute list with a reference count of 1.
 * 
 * Return value: the newly allocated #PangoAttrList.
 **/
PangoAttrList *
pango_attr_list_new (void)
{
  PangoAttrList *list = g_new (PangoAttrList, 1);

  list->ref_count = 1;
  list->attributes = NULL;
  list->attributes_tail = NULL;
  
  return list;
}

/**
 * pango_attr_list_ref:
 * @list: a #PangoAttrList
 * 
 * Increase the reference count of the given attribute list by one.
 **/
void
pango_attr_list_ref (PangoAttrList *list)
{
  g_return_if_fail (list != NULL);

  list->ref_count++;
}

/**
 * pango_attr_list_unref:
 * @list: a #PangoAttrList
 * 
 * Decrease the reference count of the given attribute list by one.
 * If the result is zero, free the attribute list and the attributes
 * it contains.
 **/
void
pango_attr_list_unref (PangoAttrList *list)
{
  GSList *tmp_list;
  
  g_return_if_fail (list != NULL);
  g_return_if_fail (list->ref_count > 0);

  list->ref_count--;
  if (list->ref_count == 0)
    {
      tmp_list = list->attributes;
      while (tmp_list)
	{
	  PangoAttribute *attr = tmp_list->data;
	  tmp_list = tmp_list->next;

	  attr->klass->destroy (attr);
	}

      g_slist_free (list->attributes);

      g_free (list);
    }
}

/**
 * pango_attr_list_copy:
 * @list: a #PangoAttrList
 * 
 * Copy @list and return an identical, new list.
 *
 * Return value: new attribute list
 **/
PangoAttrList *
pango_attr_list_copy (PangoAttrList *list)
{
  PangoAttrList *new;
  GSList *iter;
  GSList *new_attrs;
  
  g_return_val_if_fail (list != NULL, NULL);

  new = pango_attr_list_new ();

  iter = list->attributes;
  new_attrs = NULL;
  while (iter != NULL)
    {
      new_attrs = g_slist_prepend (new_attrs,
                                   pango_attribute_copy (iter->data));

      iter = g_slist_next (iter);
    }

  /* we're going to reverse the nodes, so head becomes tail */
  new->attributes_tail = new_attrs;
  new->attributes = g_slist_reverse (new_attrs);
  
  return new;
}

static void
pango_attr_list_insert_internal (PangoAttrList  *list,
				 PangoAttribute *attr,
				 gboolean        before)
{
  GSList *tmp_list, *prev, *link;
  gint start_index = attr->start_index;

  if (!list->attributes)
    {
      list->attributes = g_slist_prepend (NULL, attr);
      list->attributes_tail = list->attributes;
    }
  else if (((PangoAttribute *)list->attributes_tail->data)->start_index < start_index ||
	   (!before && ((PangoAttribute *)list->attributes_tail->data)->start_index == start_index))
    {
      g_slist_append (list->attributes_tail, attr);
      list->attributes_tail = list->attributes_tail->next;
      g_assert (list->attributes_tail);
    }
  else
    {
      prev = NULL;
      tmp_list = list->attributes;
      while (1)
	{
	  PangoAttribute *tmp_attr = tmp_list->data;
	  
	  if (tmp_attr->start_index > start_index ||
	      (before && tmp_attr->start_index == start_index))
	    {
	      link = g_slist_alloc ();
	      link->next = tmp_list;
	      link->data = attr;

	      if (prev)
		prev->next = link;
	      else
		list->attributes = link;

	      if (!tmp_list)
		list->attributes_tail = link;

	      break;
	    }
	  
	  prev = tmp_list;
	  tmp_list = tmp_list->next;
	}
    }
}

/**
 * pango_attr_list_insert:
 * @list: a #PangoAttrList
 * @attr: the attribute to insert. Ownership of this value is
 *        assumed by the list.
 * 
 * Insert the given attribute into the #PangoAttrList. It will
 * be inserted after all other attributes with a matching
 * @start_index.
 **/
void
pango_attr_list_insert (PangoAttrList  *list,
			PangoAttribute *attr)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (attr != NULL);

  pango_attr_list_insert_internal (list, attr, FALSE);
}

/**
 * pango_attr_list_insert_before:
 * @list: a #PangoAttrList
 * @attr: the attribute to insert. Ownership of this value is
 *        assumed by the list.
 * 
 * Insert the given attribute into the #PangoAttrList. It will
 * be inserted before all other attributes with a matching
 * @start_index.
 **/
void
pango_attr_list_insert_before (PangoAttrList  *list,
			       PangoAttribute *attr)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (attr != NULL);

  pango_attr_list_insert_internal (list, attr, TRUE);
}

/**
 * pango_attr_list_change:
 * @list: a #PangoAttrList
 * @attr: the attribute to insert. Ownership of this value is
 *        assumed by the list.
 * 
 * Insert the given attribute into the #PangoAttrList. It will
 * replace any attributes of the same type on that segment
 * and be merged with any adjoining attributes that are identical.
 *
 * This function is slower than pango_attr_list_insert() for
 * creating a attribute list in order (potentially much slower
 * for large lists). However, pango_attr_list_insert() is not
 * suitable for continually changing a set of attributes 
 * since it never removes or combines existing attributes.
 **/
void
pango_attr_list_change (PangoAttrList  *list,
			PangoAttribute *attr)
{
  GSList *tmp_list, *prev, *link;
  gint start_index = attr->start_index;
  gint end_index = attr->end_index;
  
  g_return_if_fail (list != NULL);
  
  tmp_list = list->attributes;
  prev = NULL;
  while (1)
    {
      PangoAttribute *tmp_attr;

      if (!tmp_list ||
	  ((PangoAttribute *)tmp_list->data)->start_index > start_index)
	{
	  /* We need to insert a new attribute
	   */
	  link = g_slist_alloc ();
	  link->next = tmp_list;
	  link->data = attr;
	  
	  if (prev)
	    prev->next = link;
	  else
	    list->attributes = link;
	  
	  if (!tmp_list)
	    list->attributes_tail = link;
	  
	  prev = link;
	  tmp_list = prev->next;
	  break;
	}

      tmp_attr = tmp_list->data;

      if (tmp_attr->klass->type == attr->klass->type &&
	  tmp_attr->end_index >= start_index)
	{
	  /* We overlap with an existing attribute */
	  if (pango_attribute_equal (tmp_attr, attr))
	    {
	      /* We can merge the new attribute with this attribute
	       */
	      if (tmp_attr->end_index >= end_index)
		{
		  /* We are totally overlapping the previous attribute.
		   * No action is needed.
		   */
		  pango_attribute_destroy (attr);
		  return;
		}
	      tmp_attr->end_index = end_index;
	      pango_attribute_destroy (attr);
	      
	      attr = tmp_attr;
	      
	      prev = tmp_list;
	      tmp_list = tmp_list->next;
	      
	      break;
	    }
	  else
	    {
	      /* Split, truncate, or remove the old attribute
	       */
	      if (tmp_attr->end_index > attr->end_index)
		{
		  PangoAttribute *end_attr = pango_attribute_copy (tmp_attr);

		  end_attr->start_index = attr->end_index;
		  pango_attr_list_insert (list, end_attr);
		}

	      if (tmp_attr->start_index == attr->start_index)
		{
		  pango_attribute_destroy (tmp_attr);
		  tmp_list->data = attr;

		  prev = tmp_list;
		  tmp_list = tmp_list->next;
		  break;
		}
	      else
		{
		  tmp_attr->end_index = attr->start_index;
		}
	    }
	}
      prev = tmp_list;
      tmp_list = tmp_list->next;
    }
  /* At this point, prev points to the list node with attr in it,
   * tmp_list points to prev->next.
   */

  g_assert (prev->data == attr);
  g_assert (prev->next == tmp_list);
  
  /* We now have the range inserted into the list one way or the
   * other. Fix up the remainder
   */
  while (tmp_list)
    {
      PangoAttribute *tmp_attr = tmp_list->data;

      if (tmp_attr->start_index > end_index)
	break;
      else if (tmp_attr->klass->type == attr->klass->type)
	{
	  if (tmp_attr->end_index <= attr->end_index ||
	      pango_attribute_equal (tmp_attr, attr))
	    {
	      /* We can merge the new attribute with this attribute.
	       */
	      attr->end_index = MAX (end_index, tmp_attr->end_index);
	      
	      pango_attribute_destroy (tmp_attr);
	      prev->next = tmp_list->next;

	      if (!prev->next)
		list->attributes_tail = prev;
	      
	      g_slist_free_1 (tmp_list);
	      tmp_list = prev->next;

	      continue;
	    }
	  else
	    {
	      /* Trim the start of this attribute that it begins at the end
	       * of the new attribute. This may involve moving
	       * it in the list to maintain the required non-decreasing
	       * order of start indices
	       */
	      GSList *tmp_list2;
	      GSList *prev2;
	      
	      tmp_attr->start_index = attr->end_index;

	      tmp_list2 = tmp_list->next;
	      prev2 = tmp_list;

	      while (tmp_list2)
		{
		  PangoAttribute *tmp_attr2 = tmp_list2->data;

		  if (tmp_attr2->start_index >= tmp_attr->start_index)
		    break;

                  prev2 = tmp_list2;
		  tmp_list2 = tmp_list2->next;
		}

	      /* Now remove and insert before tmp_list2. We'll
	       * hit this attribute again later, but that's harmless.
	       */
	      if (prev2 != tmp_list)
		{
		  GSList *old_next = tmp_list->next;
		  
		  prev->next = old_next;
		  prev2->next = tmp_list;
		  tmp_list->next = tmp_list2;

		  if (!tmp_list->next)
		    list->attributes_tail = tmp_list;

		  tmp_list = old_next;

		  continue;
		}
	    }
	}
      
      prev = tmp_list;
      tmp_list = tmp_list->next;
    }
}

/**
 * pango_attr_list_splice:
 * @list: a #PangoAttrList
 * @other: another #PangoAttrList
 * @pos: the position in @list at which to insert @other
 * @len: the length of the spliced segment. (Note that this
 *       must be specified since the attributes in @other
 *       may only be present at some subsection of this range)
 * 
 * This function splices attribute list @other into @list.
 * This operation is equivalent to stretching every attribute
 * applies at position @pos in @list by an amount @len,
 * and then calling pango_attr_list_change() with a copy
 * of each attributes in @other in sequence (offset in position by @pos).
 *
 * This operation proves useful for, for instance, inserting
 * a preedit string in the middle of an edit buffer.
 **/
void
pango_attr_list_splice (PangoAttrList *list,
			PangoAttrList *other,
			gint           pos,
			gint           len)
{
  GSList *tmp_list;
  
  g_return_if_fail (list != NULL);
  g_return_if_fail (other != NULL); 
  g_return_if_fail (pos >= 0);
  g_return_if_fail (len >= 0);
 
  tmp_list = list->attributes;
  while (tmp_list)
    {
      PangoAttribute *attr = tmp_list->data;

      if (attr->start_index <= pos)
	{
	  if (attr->end_index > pos)
	    attr->end_index += len;
	}
      else
	{
	  attr->start_index += len;
	  attr->end_index += len;
	}

      tmp_list = tmp_list->next;
    }

  tmp_list = other->attributes;
  while (tmp_list)
    {
      PangoAttribute *attr = pango_attribute_copy (tmp_list->data);
      attr->start_index += pos;
      attr->end_index += pos;

      pango_attr_list_change (list, attr);
      
      tmp_list = tmp_list->next;
    }
}

/**
 * pango_attr_list_get_iterator:
 * @list: a #PangoAttrList
 * 
 * Create a iterator initialized to the beginning of the list.
 * 
 * Return value: a new #PangoIterator. @list must not be modified
 *               until this iterator is freed with pango_attr_iterator_destroy().
 **/
PangoAttrIterator *
pango_attr_list_get_iterator (PangoAttrList  *list)
{
  PangoAttrIterator *iterator;

  g_return_val_if_fail (list != NULL, NULL);

  iterator = g_new (PangoAttrIterator, 1);
  iterator->next_attribute = list->attributes;
  iterator->attribute_stack = NULL;

  iterator->start_index = 0;
  iterator->end_index = 0;

  if (!pango_attr_iterator_next (iterator))
    iterator->end_index = G_MAXINT;

  return iterator;
}

/**
 * pango_attr_iterator_range:
 * @iterator: a #PangoAttrIterator
 * @start: location to store the start of the range
 * @end: location to store the end of the range
 * 
 * Get the range of the current segment.
 **/
void
pango_attr_iterator_range (PangoAttrIterator *iterator,
			   gint              *start,
			   gint              *end)
{
  g_return_if_fail (iterator != NULL);

  if (start)
    *start = iterator->start_index;
  if (end)
    *end = iterator->end_index;
}

/**
 * pango_attr_iterator_next:
 * @iterator: a #PangoAttrIterator
 * 
 * Advance the iterator until the next change of style.
 * 
 * Return value: %FALSE if the iterator is at the end of the list, otherwise %TRUE
 **/
gboolean
pango_attr_iterator_next (PangoAttrIterator *iterator)
{
  GList *tmp_list;

  g_return_val_if_fail (iterator != NULL, -1);

  if (!iterator->next_attribute && !iterator->attribute_stack)
    return FALSE;

  iterator->start_index = iterator->end_index;
  iterator->end_index = G_MAXINT;
  
  tmp_list = iterator->attribute_stack;
  while (tmp_list)
    {
      GList *next = tmp_list->next;
      PangoAttribute *attr = tmp_list->data;

      if (attr->end_index == iterator->start_index)
	{
	  iterator->attribute_stack = g_list_remove_link (iterator->attribute_stack, tmp_list);
	  g_list_free_1 (tmp_list);
	}
      else
	{
	  iterator->end_index = MIN (iterator->end_index, attr->end_index);
	}
      
      tmp_list = next;
    }

  while (iterator->next_attribute &&
	 ((PangoAttribute *)iterator->next_attribute->data)->start_index == iterator->start_index)
    {
      iterator->attribute_stack = g_list_prepend (iterator->attribute_stack, iterator->next_attribute->data);
      iterator->end_index = MIN (iterator->end_index, ((PangoAttribute *)iterator->next_attribute->data)->end_index);
      iterator->next_attribute = iterator->next_attribute->next;
    }

  if (iterator->next_attribute)
    iterator->end_index = MIN (iterator->end_index, ((PangoAttribute *)iterator->next_attribute->data)->start_index);

  return TRUE;
}

/**
 * pango_attr_iterator_copy:
 * @iterator: a #PangoAttrIterator.
 * 
 * Copy a #PangoAttrIterator
 *
 * Return value: Copy of @iterator
 **/
PangoAttrIterator *
pango_attr_iterator_copy (PangoAttrIterator *iterator)
{
  PangoAttrIterator *copy;

  g_return_val_if_fail (iterator != NULL, NULL);

  copy = g_new (PangoAttrIterator, 1);

  *copy = *iterator;

  copy->attribute_stack = g_list_copy (iterator->attribute_stack);

  return copy;
}

/**
 * pango_attr_iterator_destroy:
 * @iterator: a #PangoAttrIterator.
 * 
 * Destroy a #PangoAttrIterator and free all associated memory.
 **/
void
pango_attr_iterator_destroy (PangoAttrIterator *iterator)
{
  g_return_if_fail (iterator != NULL);
  
  g_list_free (iterator->attribute_stack);
  g_free (iterator);
}

/**
 * pango_attr_iterator_get:
 * @iterator: a #PangoAttrIterator
 * @type: the type of attribute to find.
 * 
 * Find the current attribute of a particular type at the iterator
 * location. When multiple attributes of the same type overlap,
 * the attribute whose range starts closest to the current location
 * is used.
 * 
 * Return value: the current attribute of the given type, or %NULL
 *               if no attribute of that type applies to the current
 *               location.
 **/
PangoAttribute *
pango_attr_iterator_get (PangoAttrIterator *iterator,
			 PangoAttrType      type)
{
  GList *tmp_list;

  g_return_val_if_fail (iterator != NULL, NULL);

  tmp_list = iterator->attribute_stack;
  while (tmp_list)
    {
      PangoAttribute *attr = tmp_list->data;

      if (attr->klass->type == type)
	return attr;

      tmp_list = tmp_list->next;
    }

  return NULL;
}

/**
 * pango_attr_iterator_get_font:
 * @iterator: a #PangoAttrIterator
 * @desc: a #PangoFontDescription to fill in with the current values.
 *        The family name in this structure will be set using
 *        pango_font_description_set_family_static using values from
 *        an attribute in the #PangoAttrList associated with the iterator,
 *        so if you plan to keep it around, you must call:
 *        pango_font_description_set_family (desc, pango_font_description_get_family (desc)).
 * @language: if non-%NULL, location to store language tag for item, or %NULL
 *            if non is found.
 * @extra_attrs: if non-%NULL, location in which to store a list of non-font
 *           attributes at the the current position; only the highest priority
 *           value of each attribute will be added to this list. In order
 *           to free this value, you must call pango_attribute_destroy() on
 *           each member.
 *
 * Get the font and other attributes at the current iterator position.
 **/
void
pango_attr_iterator_get_font (PangoAttrIterator     *iterator,
			      PangoFontDescription  *desc,
			      PangoLanguage        **language,
			      GSList               **extra_attrs)
{
  GList *tmp_list1;
  GSList *tmp_list2;

  PangoFontMask mask = 0;
  gboolean have_language = FALSE;

  g_return_if_fail (iterator != NULL);
  g_return_if_fail (desc != NULL);
  
  if (language)
    *language = NULL;
  
  if (extra_attrs)
    *extra_attrs = NULL;
  
  tmp_list1 = iterator->attribute_stack;
  while (tmp_list1)
    {
      PangoAttribute *attr = tmp_list1->data;
      tmp_list1 = tmp_list1->next;

      switch (attr->klass->type)
	{
	case PANGO_ATTR_FONT_DESC:
	  {
	    PangoFontMask new_mask = pango_font_description_get_set_fields (((PangoAttrFontDesc *)attr)->desc) & ~mask;
	    mask |= new_mask;
	    pango_font_description_unset_fields (desc, new_mask);
	    pango_font_description_merge_static (desc, ((PangoAttrFontDesc *)attr)->desc, FALSE);
	    
	    break;
          }
	case PANGO_ATTR_FAMILY:
	  if (!(mask & PANGO_FONT_MASK_FAMILY))
	    {
	      mask |= PANGO_FONT_MASK_FAMILY;
	      pango_font_description_set_family (desc, ((PangoAttrString *)attr)->value);
	    }
	  break;
	case PANGO_ATTR_STYLE:
	  if (!(mask & PANGO_FONT_MASK_STYLE))
	    {
	      mask |= PANGO_FONT_MASK_STYLE;
	      pango_font_description_set_style (desc, ((PangoAttrInt *)attr)->value);
	    }
	  break;
	case PANGO_ATTR_VARIANT:
	  if (!(mask & PANGO_FONT_MASK_VARIANT))
	    {
	      mask |= PANGO_FONT_MASK_VARIANT;
	      pango_font_description_set_variant (desc, ((PangoAttrInt *)attr)->value);
	    }
	  break;
	case PANGO_ATTR_WEIGHT:
	  if (!(mask & PANGO_FONT_MASK_WEIGHT))
	    {
	      mask |= PANGO_FONT_MASK_WEIGHT;
	      pango_font_description_set_weight (desc, ((PangoAttrInt *)attr)->value);
	    }
	  break;
	case PANGO_ATTR_STRETCH:
	  if (!(mask & PANGO_FONT_MASK_STRETCH))
	    {
	      mask |= PANGO_FONT_MASK_STRETCH;
	      pango_font_description_set_stretch (desc, ((PangoAttrInt *)attr)->value);
	    }
	  break;
	case PANGO_ATTR_SIZE:
	  if (!(mask & PANGO_FONT_MASK_SIZE))
	    {
	      mask |= PANGO_FONT_MASK_SIZE;
	      pango_font_description_set_size (desc, ((PangoAttrInt *)attr)->value);
	    }
	  break;
        case PANGO_ATTR_SCALE:
	  if (!(mask & PANGO_FONT_MASK_SIZE))
	    {
	      mask |= PANGO_FONT_MASK_SIZE;
	      pango_font_description_set_size (desc,
					       ((PangoAttrFloat *)attr)->value * pango_font_description_get_size (desc));
	    }
	  break;
	case PANGO_ATTR_LANGUAGE:
	  if (language)
	    {
	      if (!have_language)
		{
		  have_language = TRUE;
		  *language = ((PangoAttrLanguage *)attr)->value;
		}
	    }
	  break;
	default:
	  if (extra_attrs)
	    {
	      gboolean found = FALSE;
	  
	      tmp_list2 = *extra_attrs;
	      while (tmp_list2)
		{
		  PangoAttribute *old_attr = tmp_list2->data;
		  if (attr->klass->type == old_attr->klass->type)
		    {
		      found = TRUE;
		      break;
		    }

		  tmp_list2 = tmp_list2->next;
		}

	      if (!found)
		*extra_attrs = g_slist_prepend (*extra_attrs, pango_attribute_copy (attr));
	    }
	}
    }
}

/**
 * pango_attr_list_filter:
 * @list: a #PangoAttrList
 * @func: callback function; returns %TRUE if an atttribute
 *        should be filtered out.
 * @data: Data to be passed to @func
 * 
 * Given a PangoAttrList and callback function, removes any elements
 * of @list for which @func returns %TRUE and inserts them into
 * a new list.
 * 
 * Return value: a newly allocated %PangoAttrList or %NULL if
 *  no attributes of the given types were found.
 **/
PangoAttrList *
pango_attr_list_filter (PangoAttrList       *list,
			PangoAttrFilterFunc  func,
			gpointer             data)

{
  PangoAttrList *new = NULL;
  GSList *tmp_list;
  GSList *prev;
  GSList *new_attrs;
  
  g_return_val_if_fail (list != NULL, NULL);

  tmp_list = list->attributes;
  prev = NULL;
  new_attrs = NULL;
  while (tmp_list)
    {
      GSList *next = tmp_list->next;
      PangoAttribute *tmp_attr = tmp_list->data;

      if ((*func) (tmp_attr, data))
	{
	  if (!tmp_list->next)
	    list->attributes_tail = prev;
	  
	  if (prev)
	    prev->next = tmp_list->next;
	  else
	    list->attributes = tmp_list->next;
	  
	  tmp_list->next = NULL;
	  
	  if (!new)
	    {
	      new = pango_attr_list_new ();
	      new->attributes = new->attributes_tail = tmp_list;
	    }
	  else
	    {
	      new->attributes_tail->next = tmp_list;
	      new->attributes_tail = tmp_list;
	    }

	  goto next_attr;
	}

      prev = tmp_list;

    next_attr:
      tmp_list = next;
    }

  return new;
}

/**
 * pango_attr_iterator_get_attrs:
 * @iterator: a #PangAttrIterator
 * 
 * Gets a list all attributes a the current position of the
 * iterator.
 * 
 * Return value: a list of all attributes for the current range.
 *   To free this value, call pango_attributes_destroy() on
 *   each value and g_slist_free() on the list.
 **/
GSList *
pango_attr_iterator_get_attrs (PangoAttrIterator *iterator)
{
  GSList *attrs = NULL;
  GList *tmp_list;

  for (tmp_list = iterator->attribute_stack; tmp_list; tmp_list = tmp_list->next)
    {
      PangoAttribute *attr = tmp_list->data;
      GSList *tmp_list2;
      gboolean found = FALSE;

      for (tmp_list2 = attrs; tmp_list2; tmp_list2 = tmp_list2->next)
	{
	  PangoAttribute *old_attr = tmp_list2->data;
	  if (attr->klass->type == old_attr->klass->type)
	    {
	      found = TRUE;
	      break;
	    }
	}

      if (!found)
	attrs = g_slist_prepend (attrs, pango_attribute_copy (attr));
    }

  return attrs;
}
