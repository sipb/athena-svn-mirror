/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-paper.c: GnomePrintPaper and GnomePrintUnit
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
 *  Copyright (C) 1998 the Free Software Foundation and 2001-2202 Ximian, Inc.
 */

#include <config.h>
#include <math.h>

#include <libgnomeprint/gpa/gpa-node.h>
#include <libgnomeprint/gpa/gpa-config.h>
#include <libgnomeprint/gnome-print-i18n.h>
#include <libgnomeprint/gnome-print-paper.h>

/* FIXME: fill in all papers and units. (Lauris) */
/* FIXME: load papers from file (Lauris) */
/* FIXME: use some fancy unit program (Lauris) */

/*
 * WARNING! Do not mess up with that - we use hardcoded numbers for base units!
 */

static const GnomePrintPaper gp_paper_default = {0, N_("A4"), 21.0 * 72.0 / 2.54, 29.7 * 72.0 / 2.54};

GList *gp_papers = NULL;

/* Load System paper config file */

static void
gnome_print_papers_load (void)
{
	GPANode *config;

	config = GPA_NODE (gpa_config_new ());

	if (config) {
		GPANode *papers;
		papers = gpa_node_get_child_from_path (config, "Globals.Media.PhysicalSizes");
		if (papers) {
			GPANode *paper;
			for (paper = gpa_node_get_child (papers, NULL); paper != NULL; paper = gpa_node_get_child (papers, paper)) {
				guchar *name;
				gdouble width, height;
				name = gpa_node_get_path_value (paper, "Name");
				gpa_node_get_length_path_value (paper, "Width", &width);
				gpa_node_get_length_path_value (paper, "Height", &height);
				if (name && (width >= 1.0) && (height >= 1.0)) {
					GnomePrintPaper *gpp;
					gpp = g_new (GnomePrintPaper, 1);
					gpp->version = 0;
					gpp->name = name;
					gpp->width = width;
					gpp->height = height;
					gp_papers = g_list_prepend (gp_papers, gpp);
				} else {
					if (name) g_free (name);
				}
				gpa_node_unref (paper);
			}
			gp_papers = g_list_reverse (gp_papers);
			gpa_node_unref (papers);
		}
		gpa_node_unref (config);
	}

	if (!gp_papers) {
		gp_papers = g_list_prepend (NULL, (gpointer) &gp_paper_default);
	}
}

/* Returned papers are const, but lists have to be freed */

/**
 * gnome_print_paper_get_default:
 *
 * Get a pointer to the default paper for the system.  The returned
 * pointer should not be freed.
 *
 * Returns: A pointer to the default #GnomePrintPaper.
 *
 **/
const GnomePrintPaper *
gnome_print_paper_get_default (void)
{
	if (!gp_papers) gnome_print_papers_load ();

	return gp_papers->data;
}

/**
 * gnome_print_paper_get_by_name
 * @name: The name of the paper to get
 *
 * Gets a pointer the paper represented by name @name, for example: "A4".
 * The returned pointer should not be freed.
 *
 * Returns: A pointer to the #GnomePrintPaper, %NULL if not found.
 *
 **/
const GnomePrintPaper *
gnome_print_paper_get_by_name (const guchar *name)
{
	GList *l;

	g_return_val_if_fail (name != NULL, NULL);

	if (!gp_papers) gnome_print_papers_load ();

	for (l = gp_papers; l != NULL; l = l->next) {
		if (!g_ascii_strcasecmp (name, ((GnomePrintPaper *) l->data)->name)) return l->data;
	}

	return NULL;
}

#define GP_CLOSE_ENOUGH(a,b) (fabs ((a) - (b)) < 0.1)
#define GP_LESS_THAN(a,b) ((a) - (b) < 0.01)

/**
 * gnome_print_paper_get_by_size
 * @width: The width of the paper
 * @height: The height of the paper
 *
 * Gets a pointer the paper with width @width and height @height.
 * The returned pointer should not be freed.
 *
 * Returns: A pointer to the #GnomePrintPaper, %NULL if not found.
 *
 **/
const GnomePrintPaper *
gnome_print_paper_get_by_size (gdouble width, gdouble height)
{
	GList *l;

	/* Should we allow papers <= 1/5184 sq inch? */
	g_return_val_if_fail (width > 1.0, NULL);
	g_return_val_if_fail (height > 1.0, NULL);

	if (!gp_papers) gnome_print_papers_load ();

	for (l = gp_papers; l != NULL; l = l->next) {
		if (GP_CLOSE_ENOUGH (((GnomePrintPaper *) l->data)->width, width) &&
		    GP_CLOSE_ENOUGH (((GnomePrintPaper *) l->data)->height, height)) {
			return l->data;
		}
	}

	return NULL;
}

/**
 * gnome_print_paper_get_closest_by_size
 * @width: The width of the paper
 * @height: The height of the paper
 * @mustfit: Should @width and @height fit within paper
 *
 * Gets a pointer the paper with dimensions closest to width @width and 
 * height @height.  If @mustfit is %TRUE then @width and @height must fit
 * within the dimensions of the returned paper.  The returned pointer 
 * should not be freed.
 *
 * Returns: A pointer to the #GnomePrintPaper, %NULL if not found.
 *
 **/
const GnomePrintPaper *
gnome_print_paper_get_closest_by_size (gdouble width, gdouble height, gboolean mustfit)
{
	const GnomePrintPaper *bestpaper;
	gdouble dist, best;
	GList *l;

	/* Should we allow papers <= 1/5184 sq inch? */
	g_return_val_if_fail (width > 1.0, NULL);
	g_return_val_if_fail (height > 1.0, NULL);

	if (!gp_papers) gnome_print_papers_load ();

	bestpaper = NULL;
	best = 1e18;

	for (l = gp_papers; l != NULL; l = l->next) {
		if (!mustfit ||
		    (GP_LESS_THAN (width, ((GnomePrintPaper *) l->data)->width) &&
		     GP_LESS_THAN (height, ((GnomePrintPaper *) l->data)->height))) {
			gdouble dx, dy;
			/* We fit, or it is not important */
			dx = width - ((GnomePrintPaper *) l->data)->width;
			dy = height - ((GnomePrintPaper *) l->data)->height;
			dist = sqrt (dx * dx + dy * dy);
			if (dist < best) {
				best = dist;
				bestpaper = l->data;
			}
		}
	}

	return bestpaper;
}

/**
 * gnome_print_paper_get_list:
 * 
 * Get a #GList of all the papers available on the system.  The list returned
 * must be freed with #gnome_print_paper_free_list when you are finished.
 *
 * Returns: A pointer to the list of papers.
 *
 **/
GList *
gnome_print_paper_get_list (void)
{
	GList *papers;

	if (!gp_papers) gnome_print_papers_load ();

	papers = g_list_copy (gp_papers);

	return papers;
}

/**
 * gnome_print_paper_free_list:
 * @papers: A pointer to a #GList of papers to free
 *
 * Used to free the list created using #gnome_print_paper_get_list.
 *
 **/
void
gnome_print_paper_free_list (GList *papers)
{
	g_list_free (papers);
}

