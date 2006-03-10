/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#ifndef X_DISPLAY_MISSING

#ifndef TRUEREVSTACK
#include "X_gram.h"

x_gram *bottom_gram = NULL;
x_gram *unlinked = NULL;
int reverse_stack = 0;

void 
add_to_bottom(gram)
	x_gram *gram;
{
	if (bottom_gram) {
		bottom_gram->below = gram;
		gram->below = NULL;
		gram->above = bottom_gram;
		bottom_gram = gram;
	}
	else {
		gram->above = NULL;
		gram->below = NULL;
		bottom_gram = gram;
	}
}

/* ARGSUSED */
void 
pull_to_top(gram)
	x_gram *gram;
{
}

/* ARGSUSED */
void 
push_to_bottom(gram)
	x_gram *gram;
{
}

void 
delete_gram(gram)
	x_gram *gram;
{
	if (gram == bottom_gram) {
		if (gram->above) {
			bottom_gram = gram->above;
			bottom_gram->below = NULL;
		}
		else {
			bottom_gram = NULL;
		}
	}
	else if (gram == unlinked) {
		if (gram->above) {
			unlinked = gram->above;
			unlinked->below = NULL;
		}
		else {
			unlinked = NULL;
		}
	}
	else {
		if (gram->above)
			gram->above->below = gram->below;
		gram->below->above = gram->above;
	}

	/*
	 * fix up above & below pointers so that calling delete_gram again is
	 * safe
	 */
	gram->below = gram;
	gram->above = gram;
}

void 
unlink_gram(gram)
	x_gram *gram;
{
	delete_gram(gram);

	if (unlinked) {
		unlinked->below = gram;
		gram->below = NULL;
		gram->above = unlinked;
		unlinked = gram;
	}
	else {
		gram->above = NULL;
		gram->below = NULL;
		unlinked = gram;
	}
}

#endif

#ifdef TRUEREVSTACK

#include "X_gram.h"
#include "main.h"
#include <stdio.h>

x_gram *bottom_gram = NULL;
static x_gram *top_gram = NULL;

void 
pull_to_top(gram)
	x_gram *gram;
{
	if (gram == top_gram) {
		/* already here */
		return;
	}
	else if (top_gram == NULL) {
		/* no grams at all.  Make gram both top and bottom */
		top_gram = gram;
		bottom_gram = gram;
	}
	else if (gram == bottom_gram) {
		/* bottom gram is special case */
		bottom_gram = bottom_gram->above;
		bottom_gram->below = NULL;
		top_gram->above = gram;
		gram->below = top_gram;
		top_gram = gram;
	}
	else {
		/* normal case of a gram in the middle */
		gram->above->below = gram->below;
		gram->below->above = gram->above;
		top_gram->above = gram;
		gram->below = top_gram;
		gram->above = NULL;
		top_gram = gram;
	}
}

void 
push_to_bottom(gram)
	x_gram *gram;
{
	if (gram == bottom_gram) {
		/* already here */
		return;
	}
	else if (bottom_gram == NULL) {
		/* no grams at all.  Make gram both top and bottom */
		gram->above = NULL;
		gram->below = NULL;
		top_gram = gram;
		bottom_gram = gram;
	}
	else if (gram == top_gram) {
		/* top gram is special case */
		top_gram = top_gram->below;
		top_gram->above = NULL;
		bottom_gram->below = gram;
		gram->above = bottom_gram;
		bottom_gram = gram;
	}
	else {
		/* normal case of a gram in the middle */
		gram->above->below = gram->below;
		gram->below->above = gram->above;
		bottom_gram->below = gram;
		gram->above = bottom_gram;
		gram->below = NULL;
		bottom_gram = gram;
	}
}

void 
unlink_gram(gram)
	x_gram *gram;
{
	if (top_gram == bottom_gram) {
		/* the only gram in the stack */
		top_gram = NULL;
		bottom_gram = NULL;
	}
	else if (gram == top_gram) {
		top_gram = gram->below;
		top_gram->above = NULL;
	}
	else if (gram == bottom_gram) {
		bottom_gram = gram->above;
		bottom_gram->below = NULL;
	}
	else {
		gram->above->below = gram->below;
		gram->below->above = gram->above;
	}
}

void 
add_to_bottom(gram)
	x_gram *gram;
{
	if (bottom_gram == NULL) {
		gram->above = NULL;
		gram->below = NULL;
		top_gram = gram;
		bottom_gram = gram;
	}
	else {
		bottom_gram->below = gram;
		gram->above = bottom_gram;
		gram->below = NULL;
		bottom_gram = gram;
	}
}

#endif				/* TRUEREVSTACK */

#endif				/* X_DISPLAY_MISSING */
