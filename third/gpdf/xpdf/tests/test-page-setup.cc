/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Experiments with affines (real men would use pencil and paper) 
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
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

#include <stdlib.h>
#include <string.h>
#include <libart_lgpl/libart.h>


#define TEST_INITIALIZATION /* empty */

#include <unit-test.h>

#define TEST_STR(expected, got)						\
	TESTING_MESSAGE ("string value of " #got);			\
	if (strcmp (expected, got) != 0) {				\
		g_print ("\n Expected \"%s\", got \"%s\"", expected, got); \
		FAIL_TEST ();						\
	}								\
	g_print ("Okay!\n");

static void
setup ()
{
}

static void
tear_down ()
{
}

static void
setup_page_transform (gdouble *transform, 
		      gdouble x1, gdouble y1, gdouble x2, gdouble y2,
		      gint angle)
{
	double width;
	double height;
	double rotate [6];
	double translate [6];

	width = x2 - x1;
	height = y2 - y1;

	art_affine_identity (transform);
	art_affine_flip (transform, transform, FALSE, TRUE);
	art_affine_rotate (rotate, angle);
	art_affine_multiply (transform, transform, rotate);

	if (angle == 90) {
		transform [4] = 0.0;
		transform [5] = 0.0;
	} else if (angle == 180) {
		transform [4] = width;
		transform [5] = 0.0;
	} else if (angle == 270) {
		transform [4] = height;
		transform [5] = width;
	} else /* if (angle == 0) */ {
		transform [4] = 0.0;
		transform [5] = height;
	}

	art_affine_translate (translate, -x1, -y1);
	art_affine_multiply (transform, translate, transform);
}	

double affine [6];
char ps [128];

#define TEST_AFFINE(expected_as_ps, affine)		\
art_affine_to_string (ps, affine);			\
TEST_STR (expected_as_ps, ps);

TEST_BEGIN (Affines, setup, tear_down)

TEST_NEW (straightforward)
{
	setup_page_transform (affine, 0, 0, 50, 70, 0);
	TEST_AFFINE ("[ 1 0 0 -1 0 70 ] concat", affine);
}

TEST_NEW (rotated_90)
{
	setup_page_transform (affine, 0, 0, 50, 70, 90);
	TEST_AFFINE ("[ 0 1 1 0 0 0 ] concat", affine);
}

TEST_NEW (rotated_180)
{
	setup_page_transform (affine, 0, 0, 50, 70, 180);
	TEST_AFFINE ("[ -1 0 0 1 50 0 ] concat", affine);
}

TEST_NEW (rotated_270)
{
	setup_page_transform (affine, 0, 0, 50, 70, 270);
	TEST_AFFINE ("[ 0 -1 -1 0 70 50 ] concat", affine);
}

TEST_NEW (offset)
{
	setup_page_transform (affine, 30, 20, 80, 90, 0);
	TEST_AFFINE ("[ 1 0 0 -1 -30 90 ] concat", affine);
}

TEST_NEW (offset_rotated_90)
{
	setup_page_transform (affine, 30, 20, 80, 90, 90);
	TEST_AFFINE ("[ 0 1 1 0 -20 -30 ] concat", affine);
}

TEST_NEW (offset_rotated_180)
{
	setup_page_transform (affine, 30, 20, 80, 90, 180);
	TEST_AFFINE ("[ -1 0 0 1 80 -20 ] concat", affine);
}

TEST_NEW (offset_rotated_270)
{
	setup_page_transform (affine, 30, 20, 80, 90, 270);
	TEST_AFFINE ("[ 0 -1 -1 0 90 80 ] concat", affine);
}

TEST_END ()
