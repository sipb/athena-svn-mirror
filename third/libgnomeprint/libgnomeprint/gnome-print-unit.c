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
	{0, GNOME_PRINT_UNIT_ABSOLUTE, (72.0), N_("Inch"), N_("in"), N_("Inches"), N_("in")},
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
 * @base: 
 * 
 *
 * 
 * Return Value: 
 **/
const GnomePrintUnit *
gnome_print_unit_get_identity (guint base)
{
	switch (base) {
	case GNOME_PRINT_UNIT_DIMENSIONLESS:
		return &gp_units[0];
		break;
	case GNOME_PRINT_UNIT_ABSOLUTE:
		return &gp_units[1];
		break;
	case GNOME_PRINT_UNIT_DEVICE:
		return &gp_units[2];
		break;
	case GNOME_PRINT_UNIT_USERSPACE:
		return &gp_units[3];
		break;
	default:
		g_warning ("file %s: line %d: Illegal unit base %d", __FILE__, __LINE__, base);
		return FALSE;
		break;
	}
}

/* FIXME: return a gettexted default unit so that translators can
 *        set the default unit on a per locale basis (Chema)
 */
const GnomePrintUnit *
gnome_print_unit_get_default (void)
{
	return &gp_units[0];
}

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
 * @abbreviation: an ascii string poiting to the abbreviation
 * 
 * get a unit based on its abbreviation like "cm" "pts" or "in".
 * 
 * Return Value: a constant pointer to a GnomePrintUnit, NULL on error
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

void
gnome_print_unit_free_list (GList *units)
{
	g_list_free (units);
}

/**
 * gnome_print_convert_distance:
 * @distance: 
 * @from: 
 * @to: 
 * 
 * Check wether a conversion between @from and @to can be made
 * 
 * Return Value: TRUE if the conversion is possible, FALSE if
 *               it is not or on error
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
 * @distance: 
 * @from: 
 * @to: 
 * @ctmscale: 
 * @devicescale: 
 * 
 * ctmscale is userspace->absolute, devicescale is device->absolute
 * 
 * Return Value: 
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

