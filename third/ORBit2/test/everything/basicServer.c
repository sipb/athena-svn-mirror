/*
 * CORBA C language mapping tests
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Phil Dawes <philipd@users.sourceforge.net>
 */

#include "everything.h"
#include "constants.h"
#include <stdio.h>

static CORBA_char *
BasicServer__get_foo (PortableServer_Servant servant,
		      CORBA_Environment     *ev)
{
	return CORBA_string_dup (constants_STRING_RETN);  
}

static void
BasicServer__set_foo (PortableServer_Servant servant,
		      const CORBA_char      *val,
		      CORBA_Environment     *ev)
{
	g_assert (!strcmp (val, constants_STRING_IN));
}

static CORBA_long
BasicServer__get_bah (PortableServer_Servant servant,
		      CORBA_Environment     *ev)
{
	return constants_LONG_RETN;
}

static CORBA_char *
BasicServer_opString (PortableServer_Servant   servant,
		      const CORBA_char        *inArg, 
		      CORBA_char             **inoutArg,
		      CORBA_char             **outArg,
		      CORBA_Environment       *ev)
{
	g_assert (!strcmp (inArg, constants_STRING_IN));
	g_assert (!strcmp (*inoutArg, constants_STRING_INOUT_IN));
  
	CORBA_free (*inoutArg);
	*inoutArg = CORBA_string_dup (constants_STRING_INOUT_OUT);
	*outArg = CORBA_string_dup (constants_STRING_OUT);

	return CORBA_string_dup (constants_STRING_RETN);
}

static CORBA_long
BasicServer_opLong (PortableServer_Servant  servant,
		    const CORBA_long        inArg, 
		    CORBA_long             *inoutArg,
		    CORBA_long             *outArg,
		    CORBA_Environment      *ev)
{
	g_assert (inArg == constants_LONG_IN);
	g_assert (*inoutArg == constants_LONG_INOUT_IN);
  
	*inoutArg = constants_LONG_INOUT_OUT;
	*outArg = constants_LONG_OUT;;

	return constants_LONG_RETN;
}

static CORBA_long_long
BasicServer_opLongLong (PortableServer_Servant  servant,
			const CORBA_long_long   inArg, 
			CORBA_long_long        *inoutArg,
			CORBA_long_long        *outArg,
			CORBA_Environment      *ev)
{
	g_assert (inArg == constants_LONG_LONG_IN);
	g_assert (*inoutArg == constants_LONG_LONG_INOUT_IN);
  
	*inoutArg = constants_LONG_LONG_INOUT_OUT;
	*outArg = constants_LONG_LONG_OUT;;

	return constants_LONG_LONG_RETN;
}

static CORBA_float
BasicServer_opFloat (PortableServer_Servant  servant,
		     const CORBA_float       inArg, 
		     CORBA_float            *inoutArg,
		     CORBA_float            *outArg,
		     CORBA_Environment      *ev)
{
	g_assert (inArg == constants_FLOAT_IN);
	g_assert (*inoutArg == constants_FLOAT_INOUT_IN);
  
	*inoutArg = constants_FLOAT_INOUT_OUT;
	*outArg = constants_FLOAT_OUT;;

	return constants_FLOAT_RETN;
}

static CORBA_double
BasicServer_opDouble (PortableServer_Servant  servant,
		      const CORBA_double      inArg, 
		      CORBA_double           *inoutArg,
		      CORBA_double           *outArg,
		      CORBA_Environment      *ev)
{
	g_assert (inArg == constants_DOUBLE_IN);
	g_assert (*inoutArg == constants_DOUBLE_INOUT_IN);
  
	*inoutArg = constants_DOUBLE_INOUT_OUT;
	*outArg = constants_DOUBLE_OUT;;

	return constants_DOUBLE_RETN;
}

static CORBA_long_double
BasicServer_opLongDouble (PortableServer_Servant   servant,
			  const CORBA_long_double  inArg, 
			  CORBA_long_double       *inoutArg,
			  CORBA_long_double       *outArg,
			  CORBA_Environment       *ev)
{
	g_assert (inArg == constants_LONG_DOUBLE_IN);
	g_assert (*inoutArg == constants_LONG_DOUBLE_INOUT_IN);
  
	*inoutArg = constants_LONG_DOUBLE_INOUT_OUT;
	*outArg = constants_LONG_DOUBLE_OUT;;

	return constants_LONG_DOUBLE_RETN;
}

static test_AnEnum
BasicServer_opEnum (PortableServer_Servant  servant,
		    const test_AnEnum       inArg, 
		    test_AnEnum            *inoutArg,
		    test_AnEnum            *outArg,
		    CORBA_Environment      *ev)
{
	g_assert (inArg == test_ENUM_IN);
	g_assert (*inoutArg == test_ENUM_INOUT_IN);
  
	*inoutArg = test_ENUM_INOUT_OUT;
	*outArg = test_ENUM_OUT;

	return test_ENUM_RETN;
}

static void
BasicServer_opException (PortableServer_Servant  servant,
			 CORBA_Environment      *ev)
{
	test_TestException *ex = test_TestException__alloc ();

	ex->reason           = CORBA_string_dup (constants_STRING_IN);
	ex->number           = constants_LONG_IN;
	ex->aseq._buffer     = CORBA_sequence_CORBA_long_allocbuf (1);
	ex->aseq._length     = 1;
	ex->aseq._buffer [0] = constants_LONG_IN;
	ex->factory          = getFactoryInstance(ev);

	CORBA_sequence_set_release (&ex->aseq, CORBA_TRUE);

	CORBA_exception_set (
		ev, CORBA_USER_EXCEPTION, ex_test_TestException,ex);
}

static void
BasicServer_opOneWay (PortableServer_Servant servant,
		      const CORBA_char      *str,
		      CORBA_Environment     *ev)
{
	g_assert (!strcmp (str, constants_STRING_IN));
}

static void
BasicServer_testLargeStringSeq (PortableServer_Servant servant,
				const test_StrSeq     *seq,
				CORBA_Environment     *ev)
{
}

PortableServer_ServantBase__epv BasicServer_base_epv = {NULL, NULL, NULL};

POA_test_BasicServer__epv BasicServer_epv = {
	NULL,
	BasicServer__get_foo,
	BasicServer__set_foo,
	BasicServer__get_bah,
	BasicServer_opString,
	BasicServer_opLong,
	BasicServer_opLongLong,
	BasicServer_opFloat,
	BasicServer_opDouble,
	BasicServer_opLongDouble,
	BasicServer_opEnum,
	BasicServer_opException,
	BasicServer_opOneWay,
	NULL, /* noImplement */
	BasicServer_testLargeStringSeq
};

POA_test_BasicServer__vepv BasicServer_vepv = {&BasicServer_base_epv, &BasicServer_epv};

POA_test_BasicServer BasicServer_servant = {NULL, &BasicServer_vepv};  /* Singleton */
