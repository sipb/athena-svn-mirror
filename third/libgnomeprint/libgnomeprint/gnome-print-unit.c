/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-unit.c: Unit utility functions
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
 *
 *  Authors:
 *    Dirk Luetjens <dirk@luedi.oche.de>
 *    Yves Arrouye <Yves.Arrouye@marin.fdn.fr>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 1998 The Free Software Foundation and 2001-2202 Ximian, Inc.
 */

#include <config.h>

#define GNOME_PRINT_UNSTABLE_API

#include <libgnomeprint/gnome-print-unit.h>
#include <libgnomeprint/gnome-print-i18n.h>

static const GnomePrintUnit gp_units[] = {
	/* Do not insert any elements before/between first 4 */
	{0, GNOME_PRINT_UNIT_DIMENSIONLESS, 1.0, N_("Unit"), "", N_("Units"), ""},
	{0, GNOME_PRINT_UNIT_ABSOLUTE, 1.0, N_("Point"), N_("Pt"), N_("Points"), N_("Pts")},
	{0, GNOME_PRINT_UNIT_USERSPACE, 1.0, N_("Userspace unit"), N_("User"), N_("Userspace units"), N_("User")},
	{0, GNOME_PRINT_UNIT_DEVICE, 1.0, N_("Pixel"), N_("Px"), N_("Pixels"), N_("Px")},
	/* You can add new elements from this point forward */
	{0, GNOME_PRINT_UNIT_DIMENSIONLESS, 0.01, N_("Percent"), N_("%"), N_("Percents"), N_("%")},
	{0, GNOME_PRINT_UNIT_ABSOLUTE, (72.0 / 25.4), N_("Millimeter"), N_("mm"), N_("Millimeters"), N_("mm")},
	{0, GNOME_PRINT_UNIT_ABSOLUTE, (72.0 / 2.54), N_("Centimeter"), N_("cm"), N_("Centimeters"), N_("cm")},
	{0, GNOME_PRINT_UNIT_ABSOLUTE, (72.0), N_("Inch"), N_("in"), N_("Inches"), N_("in")}
};

#define gp_num_units (sizeof (gp_units) / sizeof (gp_units[0]))

static GnomePrintUnit *
gnome_print_unit_copy (GnomePrintUnit *unit)
{
	return unit;
}

static void
gnome_print_unit_free (GnomePrintUnit *unit)
{
     /* Empty */
}

GType
gnome_print_unit_get_type (void)
{
	static GType type = 0;
	
	if (type == 0) {
		type = g_boxed_type_register_static
			("GnomePrintUnit",
			 (GBoxedCopyFunc) gnome_print_unit_copy,
			 (GBoxedFreeFunc) gnome_print_unit_free);
	}

	return type;
}

/**
 * gnome_print_unit_get_identity:
 * @base: The base of the #GnomePrintUnit to retrieve
 * 
 * Retrieves the #GnomePrintUnit structure referenced by base @base. 
 * 
 * Returns: The #GnomePrintUnit structure representing @base. %NULL on error
 **/
const GnomePrintUnit *
gnome_print_unit_get_identity (guint base)
{
	switch (base) {
	case GNOME_PRINT_UNIT_DIMENSIONLESS:
		return &gp_units[0];
	case GNOME_PRINT_UNIT_ABSOLUTE:
		return &gp_units[1];
	case GNOME_PRINT_UNIT_DEVICE:
		return &gp_units[2];
	case GNOME_PRINT_UNIT_USERSPACE:
		return &gp_units[3];
	default:
		g_warning ("file %s: line %d: Illegal unit base %d", __FILE__, __LINE__, base);
		return NULL;
	}
}

/* FIXME: return a gettexted default unit so that translators can
 *        set the default unit on a per locale basis (Chema)
 */
/**
 * gnome_print_unit_get_default:
 * 
 * Used to get the default #GnomePrintUnit structure.
 *
 * Returns: A pointer to the default #GnomePrintUnit structure
 *
 **/
const GnomePrintUnit *
gnome_print_unit_get_default (void)
{
	return &gp_units[0];
}

/**
 * gnome_print_unit_get_by_name:
 * @name: Name of the unit, as a string pointer
 *
 * Get a unit based on its name, for example "Millimeter" or "Inches".
 *
 * Returns: A constant pointer to a #GnomePrintUnit, %NULL on error
 *
 **/
const GnomePrintUnit *
gnome_print_unit_get_by_name (const guchar *name)
{
	gint i;

	g_return_val_if_fail (name != NULL, NULL);

	for (i = 0; i < gp_num_units; i++) {
		if (!g_ascii_strcasecmp (name, gp_units[i].name))
			return &gp_units[i];
		if (!g_ascii_strcasecmp (name, gp_units[i].plural))
			return &gp_units[i];
	}

	return NULL;
}

/**
 * gnome_print_unit_get_by_abbreviation:
 * @abbreviation: Abbreviation of the unit, as a string pointer
 * 
 * Get a unit based on its abbreviation, for example "cm" "pts" or "in".
 * 
 * Returns: A constant pointer to a GnomePrintUnit, %NULL on error
 **/
const GnomePrintUnit *
gnome_print_unit_get_by_abbreviation (const guchar *abbreviation)
{
	gint i;

	g_return_val_if_fail (abbreviation != NULL, NULL);

	for (i = 0; i < gp_num_units; i++) {
		if (!g_ascii_strcasecmp (abbreviation, gp_units[i].abbr))
			return &gp_units[i];
		if (!g_ascii_strcasecmp (abbreviation, gp_units[i].abbr_plural))
			return &gp_units[i];
	}

	return NULL;
}

/**
 * gnome_print_unit_get_list:
 * @bases: The bases to include in the list
 *
 * Gets a list of the units represented by the bases @bases.  To get
 * a list of all units then use #GNOME_PRINT_UNITS_ALL.  The list that
 * is returned should be freed using #gnome_print_unit_free_list.
 *
 * Returns: A pointer to a #GList, %NULL on error
 *
 **/
GList *
gnome_print_unit_get_list (guint bases)
{
	GList *units;
	gint i;

	g_return_val_if_fail ((bases & ~GNOME_PRINT_UNITS_ALL) == 0, NULL);

	units = NULL;

	for (i = 0; i < gp_num_units; i++) {
		if (bases & gp_units[i].base) {
			units = g_list_prepend (units, (gpointer) &gp_units[i]);
		}
	}

	units = g_list_reverse (units);

	return units;
}

/**
 * gnome_print_unit_free_list:
 * @units: A pointer to a GList to be freed
 *
 * Used to free the list of units created by #gnome_print_unit_get_list.
 *
 **/
void
gnome_print_unit_free_list (GList *units)
{
	g_list_free (units);
}

/**
 * gnome_print_convert_distance:
 * @distance: The distance to convert, and the converted value on success
 * @from: Units to convert from
 * @to: Units to convert to
 * 
 * Check whether a conversion between @from and @to can be made
 * 
 * Returns: %TRUE if the conversion is possible, %FALSE if
 *          it is not or on error
 **/
gboolean
gnome_print_convert_distance (gdouble *distance, const GnomePrintUnit *from, const GnomePrintUnit *to)
{
	g_return_val_if_fail (distance != NULL, FALSE);
	g_return_val_if_fail (from != NULL, FALSE);
	g_return_val_if_fail (to != NULL, FALSE);

	if ((from->base == GNOME_PRINT_UNIT_DIMENSIONLESS) || (to->base == GNOME_PRINT_UNIT_DIMENSIONLESS)) {
		*distance = *distance * from->unittobase / to->unittobase;
	}

	if (from->base != to->base) return FALSE;

	*distance = *distance * from->unittobase / to->unittobase;

	return TRUE;
}

/* ctm is for userspace, devicetransform is for device units */

/**
 * gnome_print_convert_distance_full:
 * @distance: The distance to convert, and the result on success
 * @from: Units to convert from
 * @to: Units to convert to
 * @ctmscale: The userspace scale to use
 * @devicescale: The device scale to use
 * 
 * Convert a distance from one unit to another.  You should supply a scale
 * as necessary.
 *
 * ctmscale is userspace->absolute, devicescale is device->absolute
 * 
 * Returns: %TRUE if the conversion is possible, %FALSE if
 *          it is not or on error
 **/
gboolean
gnome_print_convert_distance_full (gdouble *distance, const GnomePrintUnit *from, const GnomePrintUnit *to,
				   gdouble ctmscale, gdouble devicescale)
{
	gdouble absolute;

	g_return_val_if_fail (distance != NULL, FALSE);
	g_return_val_if_fail (from != NULL, FALSE);
	g_return_val_if_fail (to != NULL, FALSE);

	if (from->base == to->base)
		return gnome_print_convert_distance (distance, from, to);

	if ((from->base == GNOME_PRINT_UNIT_DIMENSIONLESS) || (to->base == GNOME_PRINT_UNIT_DIMENSIONLESS)) {
		*distance = *distance * from->unittobase / to->unittobase;
	}

	switch (from->base) {
	case GNOME_PRINT_UNIT_ABSOLUTE:
		absolute = *distance * from->unittobase;
		break;
	case GNOME_PRINT_UNIT_DEVICE:
		if (devicescale) {
			absolute = *distance * from->unittobase * devicescale;
		} else {
			return FALSE;
		}
		break;
	case GNOME_PRINT_UNIT_USERSPACE:
		if (ctmscale) {
			absolute = *distance * from->unittobase * ctmscale;
		} else {
			return FALSE;
		}
		break;
	default:
		g_warning ("file %s: line %d: Illegal unit (base %d)", __FILE__, __LINE__, from->base);
		return FALSE;
		break;
	}

	switch (to->base) {
	case GNOME_PRINT_UNIT_DIMENSIONLESS:
	case GNOME_PRINT_UNIT_ABSOLUTE:
		*distance = absolute / to->unittobase;
		break;
	case GNOME_PRINT_UNIT_DEVICE:
		if (devicescale) {
			*distance = absolute / (to->unittobase * devicescale);
		} else {
			return FALSE;
		}
		break;
	case GNOME_PRINT_UNIT_USERSPACE:
		if (ctmscale) {
			*distance = absolute / (to->unittobase * ctmscale);
		} else {
			return FALSE;
		}
		break;
	default:
		g_warning ("file %s: line %d: Illegal unit (base %d)", __FILE__, __LINE__, to->base);
		return FALSE;
		break;
	}

	return TRUE;
}

/**
 * gnome_print_unit_get_name:
 * @unit: The unit that we want to get the name from
 * @plural: flag to specify single or plural name [Inch v.s. Inches]
 * @abbreviation: flag to specify abbreviation or full name [Inch v.s. in]
 * @flags: 0 for now, used for future expansion
 *
 * Returns the translated user visible name for @unit
 *
 * Return Value: a translated malloced string on success, NULL otherwise
 **/
gchar *
gnome_print_unit_get_name (const GnomePrintUnit *unit, gboolean plural, gboolean abbreviation, gint flags)
{
	gchar *name;

	name = g_strdup (_((abbreviation) ? (plural) ? unit->abbr_plural : unit->abbr : (plural) ? unit->plural : unit->name));

	return name;
}
