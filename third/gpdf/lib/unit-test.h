/* Unit test macros
 * Copyright (c) 2001  Arjan Molenaar <arjan@xirion.nl> and Phlip
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/* unit-test.h
 * -----------
 * This is a very small testing framework from DiaCanvas2.
 * 
 * You should create a function that sets up the testing environment and
 * a function that does the cleanup, like this:
 * 	void do_setup (void)
 * 	void do_teardown (void)
 *
 * To start the testing part use the TEST_BEGIN() macro. It takes three
 * arguments (don't forget TEST_END() at the end):
 *	name: a string that represents the name of the test case.
 *	setup: the function used to set up the environment (e.g. do_setup)
 *	teardown: the functio used to do the cleanup (e.g. do_teardown)
 *
 * Now you can create a new test with the TEST_NEW() macro:
 * 	TEST_NEW(name_of_function_to_test)
 *
 * You can thread the TEST_NEW() macro as the beginning of a function. The test
 * is written just like a C function, enclosed in curly braces ('{' and '}').
 *
 * To test a value you can use the following macros:
 *	TEST(expr)
 *		Test 'expr'. 'expr' should be a boolean expression.
 *	TESTFL(v1, v2)
 *		Test if 'v1' is equal to 'v2', within some margin
 *		(TESTFL_EPSILON).
 *
 * Other useful macros:
 *	TEST_INITIALIZATION
 *		You can set this macro to a series of operations that has to
 *		be done before the tests are executed. If not set it defaults
 *		to 'g_type_init()'.
 *	TEST_WEAK_REF(obj, notifier)
 *		You should initialize 'notifier' to FALSE. If 'obj' is
 *		destroyed 'notifier' id set to TRUE.
 *	TEST_EMPTY_QUEUE()
 *		Process all pending events in the GLib main context.
 */
#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__

#include <math.h>
#include <glib.h>


#define TESTFL_EPSILON 10e-4

#define TESTING_MESSAGE(expr) g_print (" Testing '" expr "' on line %d... ", __LINE__);

#ifndef TEST_INITIALIZATION
# define TEST_INITIALIZATION g_type_init();
#endif

/* Outputting the failure in "FILE:LINE:MESSAGE" format lets my
 * interactive editor navigate to the offending line automatically, like
 * a gcc error message.  */
#define FAIL_TEST() \
	g_print ("\n%s:%i: Test Failed!\n", __FILE__, __LINE__); \
	exit (1);

#define TOLERATE(x,y) (fabs((x)-(y)) < TESTFL_EPSILON)

#define TEST(expr) \
	TESTING_MESSAGE(#expr); \
	if (!(expr)) { FAIL_TEST(); } \
	g_print ("Okay!\n");

#define TESTFL(expr1, expr2) \
	G_STMT_START { \
		double val1 = (expr1), val2 = (expr2); \
		TESTING_MESSAGE(#expr1 " == " #expr2); \
		if (!TOLERATE(val1, val2)) { \
			g_print ("%g != %g\n", val1, val2); \
			FAIL_TEST(); } \
		g_print ("Okay!\n"); \
	} G_STMT_END

#define TEST_BEGIN(name, setup, teardown) \
int main (int argc, char *argv[]) { \
	const char *_module_name = #name; \
	GVoidFunc _unit_test_setup = setup; \
	GVoidFunc _unit_test_teardown = teardown; \
	TEST_INITIALIZATION; \
	g_print ("======== Unit tests for GPdf " __DATE__ " ========\n");\
	_unit_test_setup (); \
	G_STMT_START { gboolean t = 0; test_weak_notify (&t); g_assert (t); \
		       t = 0; test_property_notify (NULL, NULL, &t); g_assert (t); }

#define TEST_END() \
	G_STMT_END; \
	_unit_test_teardown (); \
	return 0; \
}

#define TEST_NEW(name) \
	G_STMT_END; \
	_unit_test_teardown (); \
	g_print ("-------- Starting test: %s." #name " --------\n", \
		 _module_name ); \
	_unit_test_setup (); \
	G_STMT_START

#define TEST_PROPERTY_NOTIFY(obj, property, notifier) \
	g_signal_connect (G_OBJECT (obj), \
			  "notify::" property, \
			  (GCallback) test_property_notify, \
			  &(notifier))

#define TEST_WEAK_REF(obj, notifier) \
	g_object_weak_ref (G_OBJECT (obj), \
			   (GWeakNotify) test_weak_notify, \
			   &(notifier))

static void
test_weak_notify (gboolean *notify)
{
	*notify = TRUE;
}

static void
test_property_notify (void *object, void *pspec, gboolean *notify)
{
	*notify = TRUE;
}

/* Run events in the GTK+ event queue. */
#define TEST_EMPTY_QUEUE() \
	while (g_main_context_pending (NULL)) \
		g_main_context_iteration (NULL, FALSE);

#endif /* __UNIT_TEST_H__ */
