/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter, Red Hat Software
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Dick Porter <dick@cymru.net>
 *          Elliot Lee <sopwith@redhat.com>
 *
 */

#ifndef _ORBIT_DYNANY_H_
#define _ORBIT_DYNANY_H_

#include "orbit_types.h"

typedef struct CORBA_DynAny_type *CORBA_DynAny;
typedef struct CORBA_DynEnum_type *CORBA_DynEnum;
typedef struct CORBA_DynStruct_type *CORBA_DynStruct;
typedef struct CORBA_DynUnion_type *CORBA_DynUnion;
typedef struct CORBA_DynSequence_type *CORBA_DynSequence;
typedef struct CORBA_DynArray_type *CORBA_DynArray;

typedef struct CORBA_DynAny_Invalid {
	int dummy;
} CORBA_DynAny_Invalid;

typedef struct CORBA_DynAny_InvalidValue {
	int dummy;
} CORBA_DynAny_InvalidValue;

typedef struct CORBA_DynAny_TypeMismatch {
	int dummy;
} CORBA_DynAny_TypeMismatch;

typedef struct CORBA_DynAny_InvalidSeq {
	int dummy;
} CORBA_DynAny_InvalidSeq;

typedef struct CORBA_DynFixed_Invalid {
	int dummy;
} CORBA_DynFixed_Invalid;

typedef struct CORBA_DynFixed_InvalidValue {
	int dummy;
} CORBA_DynFixed_InvalidValue;

typedef struct CORBA_DynFixed_TypeMismatch {
	int dummy;
} CORBA_DynFixed_TypeMismatch;

typedef struct CORBA_DynFixed_InvalidSeq {
	int dummy;
} CORBA_DynFixed_InvalidSeq;

typedef struct CORBA_DynEnum_Invalid {
	int dummy;
} CORBA_DynEnum_Invalid;

typedef struct CORBA_DynEnum_InvalidValue {
	int dummy;
} CORBA_DynEnum_InvalidValue;

typedef struct CORBA_DynEnum_TypeMismatch {
	int dummy;
} CORBA_DynEnum_TypeMismatch;

typedef struct CORBA_DynEnum_InvalidSeq {
	int dummy;
} CORBA_DynEnum_InvalidSeq;

typedef struct CORBA_DynStruct_Invalid {
	int dummy;
} CORBA_DynStruct_Invalid;

typedef struct CORBA_DynStruct_InvalidValue {
	int dummy;
} CORBA_DynStruct_InvalidValue;

typedef struct CORBA_DynStruct_TypeMismatch {
	int dummy;
} CORBA_DynStruct_TypeMismatch;

typedef struct CORBA_DynStruct_InvalidSeq {
	int dummy;
} CORBA_DynStruct_InvalidSeq;

typedef struct CORBA_DynUnion_Invalid {
	int dummy;
} CORBA_DynUnion_Invalid;

typedef struct CORBA_DynUnion_InvalidValue {
	int dummy;
} CORBA_DynUnion_InvalidValue;

typedef struct CORBA_DynUnion_TypeMismatch {
	int dummy;
} CORBA_DynUnion_TypeMismatch;

typedef struct CORBA_DynUnion_InvalidSeq {
	int dummy;
} CORBA_DynUnion_InvalidSeq;

typedef struct CORBA_DynSequence_Invalid {
	int dummy;
} CORBA_DynSequence_Invalid;

typedef struct CORBA_DynSequence_InvalidValue {
	int dummy;
} CORBA_DynSequence_InvalidValue;

typedef struct CORBA_DynSequence_TypeMismatch {
	int dummy;
} CORBA_DynSequence_TypeMismatch;

typedef struct CORBA_DynSequence_InvalidSeq {
	int dummy;
} CORBA_DynSequence_InvalidSeq;

typedef struct CORBA_DynArray_Invalid {
	int dummy;
} CORBA_DynArray_Invalid;

typedef struct CORBA_DynArray_InvalidValue {
	int dummy;
} CORBA_DynArray_InvalidValue;

typedef struct CORBA_DynArray_TypeMismatch {
	int dummy;
} CORBA_DynArray_TypeMismatch;

typedef struct CORBA_DynArray_InvalidSeq {
	int dummy;
} CORBA_DynArray_InvalidSeq;

struct CORBA_DynAny_type {
	struct ORBit_PseudoObject_struct parent;

	gpointer items;
};

struct CORBA_DynFixed_type {
	struct CORBA_DynAny_type parent;
	
};

struct CORBA_DynEnum_type {
	struct CORBA_DynAny_type parent;

};

struct CORBA_DynStruct_type {
	struct CORBA_DynAny_type parent;
};

struct CORBA_DynUnion_type {
	struct CORBA_DynAny_type parent;

};

struct CORBA_DynSequence_type {
	struct CORBA_DynAny_type parent;

};

struct CORBA_DynArray_type {
	struct CORBA_DynAny_type parent;
};

CORBA_DynAny CORBA_ORB_create_dyn_any(CORBA_ORB obj,
				      CORBA_any *value,
				      CORBA_Environment *ev);

CORBA_DynAny CORBA_ORB_create_basic_dyn_any(CORBA_ORB obj,
					    CORBA_TypeCode type,
					    CORBA_Environment *ev);

CORBA_DynStruct CORBA_ORB_create_dyn_struct(CORBA_ORB obj,
					    CORBA_TypeCode type,
					    CORBA_Environment *ev);

CORBA_DynSequence CORBA_ORB_create_dyn_sequence(CORBA_ORB obj,
						CORBA_TypeCode type,
						CORBA_Environment *ev);

CORBA_DynArray CORBA_ORB_create_dyn_array(CORBA_ORB obj,
					  CORBA_TypeCode type,
					  CORBA_Environment *ev);

CORBA_DynUnion CORBA_ORB_create_dyn_union(CORBA_ORB obj,
					  CORBA_TypeCode type,
					  CORBA_Environment *ev);

CORBA_DynEnum CORBA_ORB_create_dyn_enum(CORBA_ORB obj,
					CORBA_TypeCode type,
					CORBA_Environment *ev);

CORBA_DynFixed CORBA_ORB_create_dyn_fixed(CORBA_ORB obj,
					  CORBA_TypeCode type,
					  CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynAny_type(CORBA_DynAny obj,
				 CORBA_Environment *ev);

void CORBA_DynAny_assign(CORBA_DynAny obj,
			 CORBA_DynAny dyn_any,
			 CORBA_Environment *ev);

void CORBA_DynAny_from_any(CORBA_DynAny obj,
			   CORBA_any value,
			   CORBA_Environment *ev);

CORBA_any *CORBA_DynAny_to_any(CORBA_DynAny obj,
			       CORBA_Environment *ev);

void CORBA_DynAny_destroy(CORBA_DynAny obj,
			  CORBA_Environment *ev);

CORBA_DynAny CORBA_DynAny_copy(CORBA_DynAny obj,
			       CORBA_Environment *ev);

void CORBA_DynAny_insert_boolean(CORBA_DynAny obj,
				 CORBA_boolean value,
				 CORBA_Environment *ev);

void CORBA_DynAny_insert_octet(CORBA_DynAny obj,
			       CORBA_octet value,
			       CORBA_Environment *ev);

void CORBA_DynAny_insert_char(CORBA_DynAny obj,
			      CORBA_char value,
			      CORBA_Environment *ev);

void CORBA_DynAny_insert_short(CORBA_DynAny obj,
			       CORBA_short value,
			       CORBA_Environment *ev);

void CORBA_DynAny_insert_ushort(CORBA_DynAny obj,
				CORBA_unsigned_short value,
				CORBA_Environment *ev);

void CORBA_DynAny_insert_long(CORBA_DynAny obj,
			      CORBA_long value,
			      CORBA_Environment *ev);

void CORBA_DynAny_insert_ulong(CORBA_DynAny obj,
			       CORBA_unsigned_long value,
			       CORBA_Environment *ev);

void CORBA_DynAny_insert_float(CORBA_DynAny obj,
			       CORBA_float value,
			       CORBA_Environment *ev);

void CORBA_DynAny_insert_double(CORBA_DynAny obj,
				CORBA_double value,
				CORBA_Environment *ev);

void CORBA_DynAny_insert_string(CORBA_DynAny obj,
				CORBA_char *value,
				CORBA_Environment *ev);

void CORBA_DynAny_insert_reference(CORBA_DynAny obj,
				   CORBA_Object value,
				   CORBA_Environment *ev);

void CORBA_DynAny_insert_typecode(CORBA_DynAny obj,
				  CORBA_TypeCode value,
				  CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
void CORBA_DynAny_insert_longlong(CORBA_DynAny obj,
				  CORBA_long_long value,
				  CORBA_Environment *ev);

void CORBA_DynAny_insert_ulonglong(CORBA_DynAny obj,
				   CORBA_unsigned_long_long value,
				   CORBA_Environment *ev);
#endif

void CORBA_DynAny_insert_longdouble(CORBA_DynAny obj,
				    CORBA_long_double value,
				    CORBA_Environment *ev);

void CORBA_DynAny_insert_wchar(CORBA_DynAny obj,
			       CORBA_wchar value,
			       CORBA_Environment *ev);

void CORBA_DynAny_insert_wstring(CORBA_DynAny obj,
				 CORBA_wchar *value,
				 CORBA_Environment *ev);

void CORBA_DynAny_insert_any(CORBA_DynAny obj,
			     CORBA_any value,
			     CORBA_Environment *ev);

CORBA_boolean CORBA_DynAny_get_boolean(CORBA_DynAny obj,
				       CORBA_Environment *ev);

CORBA_octet CORBA_DynAny_get_octet(CORBA_DynAny obj,
				   CORBA_Environment *ev);

CORBA_char CORBA_DynAny_get_char(CORBA_DynAny obj,
				 CORBA_Environment *ev);

CORBA_short CORBA_DynAny_get_short(CORBA_DynAny obj,
				   CORBA_Environment *ev);

CORBA_unsigned_short CORBA_DynAny_get_ushort(CORBA_DynAny obj,
					     CORBA_Environment *ev);

CORBA_long CORBA_DynAny_get_long(CORBA_DynAny obj,
				 CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynAny_get_ulong(CORBA_DynAny obj,
					   CORBA_Environment *ev);

CORBA_float CORBA_DynAny_get_float(CORBA_DynAny obj,
				   CORBA_Environment *ev);

CORBA_double CORBA_DynAny_get_double(CORBA_DynAny obj,
				     CORBA_Environment *ev);

CORBA_char *CORBA_DynAny_get_string(CORBA_DynAny obj,
				    CORBA_Environment *ev);

CORBA_Object CORBA_DynAny_get_reference(CORBA_DynAny obj,
					CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynAny_get_typecode(CORBA_DynAny obj,
					 CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
CORBA_long_long CORBA_DynAny_get_longlong(CORBA_DynAny obj,
					  CORBA_Environment *ev);

CORBA_unsigned_long_long CORBA_DynAny_get_ulonglong(CORBA_DynAny obj,
						    CORBA_Environment *ev);
#endif

CORBA_long_double CORBA_DynAny_get_longdouble(CORBA_DynAny obj,
					      CORBA_Environment *ev);

CORBA_wchar CORBA_DynAny_get_wchar(CORBA_DynAny obj,
				   CORBA_Environment *ev);

CORBA_wchar *CORBA_DynAny_get_wstring(CORBA_DynAny obj,
				      CORBA_Environment *ev);

CORBA_any *CORBA_DynAny_get_any(CORBA_DynAny obj,
				CORBA_Environment *ev);

CORBA_DynAny CORBA_DynAny_current_component(CORBA_DynAny obj,
					    CORBA_Environment *ev);

CORBA_boolean CORBA_DynAny_next(CORBA_DynAny obj,
				CORBA_Environment *ev);

CORBA_boolean CORBA_DynAny_seek(CORBA_DynAny obj,
				CORBA_long index,
				CORBA_Environment *ev);

void CORBA_DynAny_rewind(CORBA_DynAny obj,
			 CORBA_Environment *ev);

CORBA_DynFixed_OctetSeq *CORBA_DynFixed_get_value(CORBA_DynFixed obj,
						  CORBA_Environment *ev);

void CORBA_DynFixed_set_value(CORBA_DynFixed obj,
			      CORBA_DynFixed_OctetSeq *val,
			      CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynFixed_type(CORBA_DynFixed obj,
				   CORBA_Environment *ev);

void CORBA_DynFixed_assign(CORBA_DynFixed obj,
			   CORBA_DynAny dyn_any,
			   CORBA_Environment *ev);

void CORBA_DynFixed_from_any(CORBA_DynFixed obj,
			     CORBA_any value,
			     CORBA_Environment *ev);

CORBA_any *CORBA_DynFixed_to_any(CORBA_DynFixed obj,
				 CORBA_Environment *ev);

void CORBA_DynFixed_destroy(CORBA_DynFixed obj,
			    CORBA_Environment *ev);

CORBA_DynAny CORBA_DynFixed_copy(CORBA_DynFixed obj,
				 CORBA_Environment *ev);

void CORBA_DynFixed_insert_boolean(CORBA_DynFixed obj,
				   CORBA_boolean value,
				   CORBA_Environment *ev);

void CORBA_DynFixed_insert_octet(CORBA_DynFixed obj,
				 CORBA_octet value,
				 CORBA_Environment *ev);

void CORBA_DynFixed_insert_char(CORBA_DynFixed obj,
				CORBA_char value,
				CORBA_Environment *ev);

void CORBA_DynFixed_insert_short(CORBA_DynFixed obj,
				 CORBA_short value,
				 CORBA_Environment *ev);

void CORBA_DynFixed_insert_ushort(CORBA_DynFixed obj,
				  CORBA_unsigned_short value,
				  CORBA_Environment *ev);

void CORBA_DynFixed_insert_long(CORBA_DynFixed obj,
				CORBA_long value,
				CORBA_Environment *ev);

void CORBA_DynFixed_insert_ulong(CORBA_DynFixed obj,
				 CORBA_unsigned_long value,
				 CORBA_Environment *ev);

void CORBA_DynFixed_insert_float(CORBA_DynFixed obj,
				 CORBA_float value,
				 CORBA_Environment *ev);

void CORBA_DynFixed_insert_double(CORBA_DynFixed obj,
				  CORBA_double value,
				  CORBA_Environment *ev);

void CORBA_DynFixed_insert_string(CORBA_DynFixed obj,
				  CORBA_char *value,
				  CORBA_Environment *ev);

void CORBA_DynFixed_insert_reference(CORBA_DynFixed obj,
				     CORBA_Object value,
				     CORBA_Environment *ev);

void CORBA_DynFixed_insert_typecode(CORBA_DynFixed obj,
				    CORBA_TypeCode value,
				    CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
void CORBA_DynFixed_insert_longlong(CORBA_DynFixed obj,
				    CORBA_long_long value,
				    CORBA_Environment *ev);

void CORBA_DynFixed_insert_ulonglong(CORBA_DynFixed obj,
				     CORBA_unsigned_long_long value,
				     CORBA_Environment *ev);
#endif

void CORBA_DynFixed_insert_longdouble(CORBA_DynFixed obj,
				      CORBA_long_double value,
				      CORBA_Environment *ev);

void CORBA_DynFixed_insert_wchar(CORBA_DynFixed obj,
				 CORBA_wchar value,
				 CORBA_Environment *ev);

void CORBA_DynFixed_insert_wstring(CORBA_DynFixed obj,
				   CORBA_wchar *value,
				   CORBA_Environment *ev);

void CORBA_DynFixed_insert_any(CORBA_DynFixed obj,
			       CORBA_any value,
			       CORBA_Environment *ev);

CORBA_boolean CORBA_DynFixed_get_boolean(CORBA_DynFixed obj,
					 CORBA_Environment *ev);

CORBA_octet CORBA_DynFixed_get_octet(CORBA_DynFixed obj,
				     CORBA_Environment *ev);

CORBA_char CORBA_DynFixed_get_char(CORBA_DynFixed obj,
				   CORBA_Environment *ev);

CORBA_short CORBA_DynFixed_get_short(CORBA_DynFixed obj,
				     CORBA_Environment *ev);

CORBA_unsigned_short CORBA_DynFixed_get_ushort(CORBA_DynFixed obj,
					       CORBA_Environment *ev);

CORBA_long CORBA_DynFixed_get_long(CORBA_DynFixed obj,
				   CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynFixed_get_ulong(CORBA_DynFixed obj,
					     CORBA_Environment *ev);

CORBA_float CORBA_DynFixed_get_float(CORBA_DynFixed obj,
				     CORBA_Environment *ev);

CORBA_double CORBA_DynFixed_get_double(CORBA_DynFixed obj,
				       CORBA_Environment *ev);

CORBA_char *CORBA_DynFixed_get_string(CORBA_DynFixed obj,
				      CORBA_Environment *ev);

CORBA_Object CORBA_DynFixed_get_reference(CORBA_DynFixed obj,
					  CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynFixed_get_typecode(CORBA_DynFixed obj,
					   CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
CORBA_long_long CORBA_DynFixed_get_longlong(CORBA_DynFixed obj,
					    CORBA_Environment *ev);

CORBA_unsigned_long_long CORBA_DynFixed_get_ulonglong(CORBA_DynFixed obj,
						      CORBA_Environment *ev);
#endif

CORBA_long_double CORBA_DynFixed_get_longdouble(CORBA_DynFixed obj,
						CORBA_Environment *ev);

CORBA_wchar CORBA_DynFixed_get_wchar(CORBA_DynFixed obj,
				     CORBA_Environment *ev);

CORBA_wchar *CORBA_DynFixed_get_wstring(CORBA_DynFixed obj,
					CORBA_Environment *ev);

CORBA_any *CORBA_DynFixed_get_any(CORBA_DynFixed obj,
				  CORBA_Environment *ev);

CORBA_DynAny CORBA_DynFixed_current_component(CORBA_DynFixed obj,
					      CORBA_Environment *ev);

CORBA_boolean CORBA_DynFixed_next(CORBA_DynFixed obj,
				  CORBA_Environment *ev);

CORBA_boolean CORBA_DynFixed_seek(CORBA_DynFixed obj,
				  CORBA_long index,
				  CORBA_Environment *ev);

void CORBA_DynFixed_rewind(CORBA_DynFixed obj,
			   CORBA_Environment *ev);

CORBA_char *CORBA_DynEnum__get_value_as_string(CORBA_DynEnum obj,
					       CORBA_Environment *ev);

void CORBA_DynEnum__set_value_as_string(CORBA_DynEnum obj,
					CORBA_char *value_as_string,
					CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynEnum__get_value_as_ulong(CORBA_DynEnum obj,
						      CORBA_Environment *ev);

void CORBA_DynEnum__set_value_as_ulong(CORBA_DynEnum obj,
				       CORBA_unsigned_long value_as_ulong,
				       CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynEnum_type(CORBA_DynEnum obj,
				  CORBA_Environment *ev);

void CORBA_DynEnum_assign(CORBA_DynEnum obj,
			  CORBA_DynAny dyn_any,
			  CORBA_Environment *ev);

void CORBA_DynEnum_from_any(CORBA_DynEnum obj,
			    CORBA_any value,
			    CORBA_Environment *ev);

CORBA_any *CORBA_DynEnum_to_any(CORBA_DynEnum obj,
				CORBA_Environment *ev);

void CORBA_DynEnum_destroy(CORBA_DynEnum obj,
			   CORBA_Environment *ev);

CORBA_DynAny CORBA_DynEnum_copy(CORBA_DynEnum obj,
				CORBA_Environment *ev);

void CORBA_DynEnum_insert_boolean(CORBA_DynEnum obj,
				  CORBA_boolean value,
				  CORBA_Environment *ev);

void CORBA_DynEnum_insert_octet(CORBA_DynEnum obj,
				CORBA_octet value,
				CORBA_Environment *ev);

void CORBA_DynEnum_insert_char(CORBA_DynEnum obj,
			       CORBA_char value,
			       CORBA_Environment *ev);

void CORBA_DynEnum_insert_short(CORBA_DynEnum obj,
				CORBA_short value,
				CORBA_Environment *ev);

void CORBA_DynEnum_insert_ushort(CORBA_DynEnum obj,
				 CORBA_unsigned_short value,
				 CORBA_Environment *ev);

void CORBA_DynEnum_insert_long(CORBA_DynEnum obj,
			       CORBA_long value,
			       CORBA_Environment *ev);

void CORBA_DynEnum_insert_ulong(CORBA_DynEnum obj,
				CORBA_unsigned_long value,
				CORBA_Environment *ev);

void CORBA_DynEnum_insert_float(CORBA_DynEnum obj,
				CORBA_float value,
				CORBA_Environment *ev);

void CORBA_DynEnum_insert_double(CORBA_DynEnum obj,
				 CORBA_double value,
				 CORBA_Environment *ev);

void CORBA_DynEnum_insert_string(CORBA_DynEnum obj,
				 CORBA_char *value,
				 CORBA_Environment *ev);

void CORBA_DynEnum_insert_reference(CORBA_DynEnum obj,
				    CORBA_Object value,
				    CORBA_Environment *ev);

void CORBA_DynEnum_insert_typecode(CORBA_DynEnum obj,
				   CORBA_TypeCode value,
				   CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
void CORBA_DynEnum_insert_longlong(CORBA_DynEnum obj,
				   CORBA_long_long value,
				   CORBA_Environment *ev);

void CORBA_DynEnum_insert_ulonglong(CORBA_DynEnum obj,
				    CORBA_unsigned_long_long value,
				    CORBA_Environment *ev);
#endif

void CORBA_DynEnum_insert_longdouble(CORBA_DynEnum obj,
				     CORBA_long_double value,
				     CORBA_Environment *ev);

void CORBA_DynEnum_insert_wchar(CORBA_DynEnum obj,
				CORBA_wchar value,
				CORBA_Environment *ev);

void CORBA_DynEnum_insert_wstring(CORBA_DynEnum obj,
				  CORBA_wchar *value,
				  CORBA_Environment *ev);

void CORBA_DynEnum_insert_any(CORBA_DynEnum obj,
			      CORBA_any value,
			      CORBA_Environment *ev);

CORBA_boolean CORBA_DynEnum_get_boolean(CORBA_DynEnum obj,
					CORBA_Environment *ev);

CORBA_octet CORBA_DynEnum_get_octet(CORBA_DynEnum obj,
				    CORBA_Environment *ev);

CORBA_char CORBA_DynEnum_get_char(CORBA_DynEnum obj,
				  CORBA_Environment *ev);

CORBA_short CORBA_DynEnum_get_short(CORBA_DynEnum obj,
				    CORBA_Environment *ev);

CORBA_unsigned_short CORBA_DynEnum_get_ushort(CORBA_DynEnum obj,
					      CORBA_Environment *ev);

CORBA_long CORBA_DynEnum_get_long(CORBA_DynEnum obj,
				  CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynEnum_get_ulong(CORBA_DynEnum obj,
					    CORBA_Environment *ev);

CORBA_float CORBA_DynEnum_get_float(CORBA_DynEnum obj,
				    CORBA_Environment *ev);

CORBA_double CORBA_DynEnum_get_double(CORBA_DynEnum obj,
				      CORBA_Environment *ev);

CORBA_char *CORBA_DynEnum_get_string(CORBA_DynEnum obj,
				     CORBA_Environment *ev);

CORBA_Object CORBA_DynEnum_get_reference(CORBA_DynEnum obj,
					 CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynEnum_get_typecode(CORBA_DynEnum obj,
					  CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
CORBA_long_long CORBA_DynEnum_get_longlong(CORBA_DynEnum obj,
					   CORBA_Environment *ev);

CORBA_unsigned_long_long CORBA_DynEnum_get_ulonglong(CORBA_DynEnum obj,
						     CORBA_Environment *ev);
#endif

CORBA_long_double CORBA_DynEnum_get_longdouble(CORBA_DynEnum obj,
					       CORBA_Environment *ev);

CORBA_wchar CORBA_DynEnum_get_wchar(CORBA_DynEnum obj,
				    CORBA_Environment *ev);

CORBA_wchar *CORBA_DynEnum_get_wstring(CORBA_DynEnum obj,
				       CORBA_Environment *ev);

CORBA_any *CORBA_DynEnum_get_any(CORBA_DynEnum obj,
				 CORBA_Environment *ev);

CORBA_DynAny CORBA_DynEnum_current_component(CORBA_DynEnum obj,
					     CORBA_Environment *ev);

CORBA_boolean CORBA_DynEnum_next(CORBA_DynEnum obj,
				 CORBA_Environment *ev);

CORBA_boolean CORBA_DynEnum_seek(CORBA_DynEnum obj,
				 CORBA_long index,
				 CORBA_Environment *ev);

void CORBA_DynEnum_rewind(CORBA_DynEnum obj,
			  CORBA_Environment *ev);

CORBA_FieldName CORBA_DynStruct_current_member_name(CORBA_DynStruct obj,
						    CORBA_Environment *ev);

CORBA_TCKind CORBA_DynStruct_current_member_kind(CORBA_DynStruct obj,
						 CORBA_Environment *ev);

CORBA_NameValuePairSeq *CORBA_DynStruct_get_members(CORBA_DynStruct obj,
						    CORBA_Environment *ev);

void CORBA_DynStruct_set_members(CORBA_DynStruct obj,
				 CORBA_NameValuePairSeq *value,
				 CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynStruct_type(CORBA_DynStruct obj,
				    CORBA_Environment *ev);

void CORBA_DynStruct_assign(CORBA_DynStruct obj,
			    CORBA_DynAny dyn_any,
			    CORBA_Environment *ev);

void CORBA_DynStruct_from_any(CORBA_DynStruct obj,
			      CORBA_any value,
			      CORBA_Environment *ev);

CORBA_any *CORBA_DynStruct_to_any(CORBA_DynStruct obj,
				  CORBA_Environment *ev);

void CORBA_DynStruct_destroy(CORBA_DynStruct obj,
			     CORBA_Environment *ev);

CORBA_DynAny CORBA_DynStruct_copy(CORBA_DynStruct obj,
				  CORBA_Environment *ev);

void CORBA_DynStruct_insert_boolean(CORBA_DynStruct obj,
				    CORBA_boolean value,
				    CORBA_Environment *ev);

void CORBA_DynStruct_insert_octet(CORBA_DynStruct obj,
				  CORBA_octet value,
				  CORBA_Environment *ev);

void CORBA_DynStruct_insert_char(CORBA_DynStruct obj,
				 CORBA_char value,
				 CORBA_Environment *ev);

void CORBA_DynStruct_insert_short(CORBA_DynStruct obj,
				  CORBA_short value,
				  CORBA_Environment *ev);

void CORBA_DynStruct_insert_ushort(CORBA_DynStruct obj,
				   CORBA_unsigned_short value,
				   CORBA_Environment *ev);

void CORBA_DynStruct_insert_long(CORBA_DynStruct obj,
				 CORBA_long value,
				 CORBA_Environment *ev);

void CORBA_DynStruct_insert_ulong(CORBA_DynStruct obj,
				  CORBA_unsigned_long value,
				  CORBA_Environment *ev);

void CORBA_DynStruct_insert_float(CORBA_DynStruct obj,
				  CORBA_float value,
				  CORBA_Environment *ev);

void CORBA_DynStruct_insert_double(CORBA_DynStruct obj,
				   CORBA_double value,
				   CORBA_Environment *ev);

void CORBA_DynStruct_insert_string(CORBA_DynStruct obj,
				   CORBA_char *value,
				   CORBA_Environment *ev);

void CORBA_DynStruct_insert_reference(CORBA_DynStruct obj,
				      CORBA_Object value,
				      CORBA_Environment *ev);

void CORBA_DynStruct_insert_typecode(CORBA_DynStruct obj,
				     CORBA_TypeCode value,
				     CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
void CORBA_DynStruct_insert_longlong(CORBA_DynStruct obj,
				     CORBA_long_long value,
				     CORBA_Environment *ev);

void CORBA_DynStruct_insert_ulonglong(CORBA_DynStruct obj,
				      CORBA_unsigned_long_long value,
				      CORBA_Environment *ev);
#endif

void CORBA_DynStruct_insert_longdouble(CORBA_DynStruct obj,
				       CORBA_long_double value,
				       CORBA_Environment *ev);

void CORBA_DynStruct_insert_wchar(CORBA_DynStruct obj,
				  CORBA_wchar value,
				  CORBA_Environment *ev);

void CORBA_DynStruct_insert_wstring(CORBA_DynStruct obj,
				    CORBA_wchar *value,
				    CORBA_Environment *ev);

void CORBA_DynStruct_insert_any(CORBA_DynStruct obj,
				CORBA_any value,
				CORBA_Environment *ev);

CORBA_boolean CORBA_DynStruct_get_boolean(CORBA_DynStruct obj,
					  CORBA_Environment *ev);

CORBA_octet CORBA_DynStruct_get_octet(CORBA_DynStruct obj,
				      CORBA_Environment *ev);

CORBA_char CORBA_DynStruct_get_char(CORBA_DynStruct obj,
				    CORBA_Environment *ev);

CORBA_short CORBA_DynStruct_get_short(CORBA_DynStruct obj,
				      CORBA_Environment *ev);

CORBA_unsigned_short CORBA_DynStruct_get_ushort(CORBA_DynStruct obj,
						CORBA_Environment *ev);

CORBA_long CORBA_DynStruct_get_long(CORBA_DynStruct obj,
				    CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynStruct_get_ulong(CORBA_DynStruct obj,
					      CORBA_Environment *ev);

CORBA_float CORBA_DynStruct_get_float(CORBA_DynStruct obj,
				      CORBA_Environment *ev);

CORBA_double CORBA_DynStruct_get_double(CORBA_DynStruct obj,
					CORBA_Environment *ev);

CORBA_char *CORBA_DynStruct_get_string(CORBA_DynStruct obj,
				       CORBA_Environment *ev);

CORBA_Object CORBA_DynStruct_get_reference(CORBA_DynStruct obj,
					   CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynStruct_get_typecode(CORBA_DynStruct obj,
					    CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
CORBA_long_long CORBA_DynStruct_get_longlong(CORBA_DynStruct obj,
					     CORBA_Environment *ev);

CORBA_unsigned_long_long CORBA_DynStruct_get_ulonglong(CORBA_DynStruct obj,
						       CORBA_Environment *ev);
#endif

CORBA_long_double CORBA_DynStruct_get_longdouble(CORBA_DynStruct obj,
						 CORBA_Environment *ev);

CORBA_wchar CORBA_DynStruct_get_wchar(CORBA_DynStruct obj,
				      CORBA_Environment *ev);

CORBA_wchar *CORBA_DynStruct_get_wstring(CORBA_DynStruct obj,
					 CORBA_Environment *ev);

CORBA_any *CORBA_DynStruct_get_any(CORBA_DynStruct obj,
				   CORBA_Environment *ev);

CORBA_DynAny CORBA_DynStruct_current_component(CORBA_DynStruct obj,
					       CORBA_Environment *ev);

CORBA_boolean CORBA_DynStruct_next(CORBA_DynStruct obj,
				   CORBA_Environment *ev);

CORBA_boolean CORBA_DynStruct_seek(CORBA_DynStruct obj,
				   CORBA_long index,
				   CORBA_Environment *ev);

void CORBA_DynStruct_rewind(CORBA_DynStruct obj,
			    CORBA_Environment *ev);

CORBA_DynAny CORBA_DynUnion_discriminator(CORBA_DynUnion obj,
					  CORBA_Environment *ev);

CORBA_TCKind CORBA_DynUnion_discriminator_kind(CORBA_DynUnion obj,
					       CORBA_Environment *ev);

CORBA_DynAny CORBA_DynUnion_member(CORBA_DynUnion obj,
				   CORBA_Environment *ev);

CORBA_TCKind CORBA_DynUnion_member_kind(CORBA_DynUnion obj,
					CORBA_Environment *ev);

CORBA_boolean CORBA_DynUnion__get_set_as_default(CORBA_DynUnion obj,
						 CORBA_Environment *ev);

void CORBA_DynUnion__set_set_as_default(CORBA_DynUnion obj,
					CORBA_boolean set_as_default,
					CORBA_Environment *ev);

CORBA_FieldName CORBA_DynUnion__get_member_name(CORBA_DynUnion obj,
						CORBA_Environment *ev);

void CORBA_DynUnion__set_member_name(CORBA_DynUnion obj,
				     CORBA_FieldName member_name,
				     CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynUnion_type(CORBA_DynUnion obj,
				   CORBA_Environment *ev);

void CORBA_DynUnion_assign(CORBA_DynUnion obj,
			   CORBA_DynAny dyn_any,
			   CORBA_Environment *ev);

void CORBA_DynUnion_from_any(CORBA_DynUnion obj,
			     CORBA_any value,
			     CORBA_Environment *ev);

CORBA_any *CORBA_DynUnion_to_any(CORBA_DynUnion obj,
				 CORBA_Environment *ev);

void CORBA_DynUnion_destroy(CORBA_DynUnion obj,
			    CORBA_Environment *ev);

CORBA_DynAny CORBA_DynUnion_copy(CORBA_DynUnion obj,
				 CORBA_Environment *ev);

void CORBA_DynUnion_insert_boolean(CORBA_DynUnion obj,
				   CORBA_boolean value,
				   CORBA_Environment *ev);

void CORBA_DynUnion_insert_octet(CORBA_DynUnion obj,
				 CORBA_octet value,
				 CORBA_Environment *ev);

void CORBA_DynUnion_insert_char(CORBA_DynUnion obj,
				CORBA_char value,
				CORBA_Environment *ev);

void CORBA_DynUnion_insert_short(CORBA_DynUnion obj,
				 CORBA_short value,
				 CORBA_Environment *ev);

void CORBA_DynUnion_insert_ushort(CORBA_DynUnion obj,
				  CORBA_unsigned_short value,
				  CORBA_Environment *ev);

void CORBA_DynUnion_insert_long(CORBA_DynUnion obj,
				CORBA_long value,
				CORBA_Environment *ev);

void CORBA_DynUnion_insert_ulong(CORBA_DynUnion obj,
				 CORBA_unsigned_long value,
				 CORBA_Environment *ev);

void CORBA_DynUnion_insert_float(CORBA_DynUnion obj,
				 CORBA_float value,
				 CORBA_Environment *ev);

void CORBA_DynUnion_insert_double(CORBA_DynUnion obj,
				  CORBA_double value,
				  CORBA_Environment *ev);

void CORBA_DynUnion_insert_string(CORBA_DynUnion obj,
				  CORBA_char *value,
				  CORBA_Environment *ev);

void CORBA_DynUnion_insert_reference(CORBA_DynUnion obj,
				     CORBA_Object value,
				     CORBA_Environment *ev);

void CORBA_DynUnion_insert_typecode(CORBA_DynUnion obj,
				    CORBA_TypeCode value,
				    CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
void CORBA_DynUnion_insert_longlong(CORBA_DynUnion obj,
				    CORBA_long_long value,
				    CORBA_Environment *ev);

void CORBA_DynUnion_insert_ulonglong(CORBA_DynUnion obj,
				     CORBA_unsigned_long_long value,
				     CORBA_Environment *ev);
#endif

void CORBA_DynUnion_insert_longdouble(CORBA_DynUnion obj,
				      CORBA_long_double value,
				      CORBA_Environment *ev);

void CORBA_DynUnion_insert_wchar(CORBA_DynUnion obj,
				 CORBA_wchar value,
				 CORBA_Environment *ev);

void CORBA_DynUnion_insert_wstring(CORBA_DynUnion obj,
				   CORBA_wchar *value,
				   CORBA_Environment *ev);

void CORBA_DynUnion_insert_any(CORBA_DynUnion obj,
			       CORBA_any value,
			       CORBA_Environment *ev);

CORBA_boolean CORBA_DynUnion_get_boolean(CORBA_DynUnion obj,
					 CORBA_Environment *ev);

CORBA_octet CORBA_DynUnion_get_octet(CORBA_DynUnion obj,
				     CORBA_Environment *ev);

CORBA_char CORBA_DynUnion_get_char(CORBA_DynUnion obj,
				   CORBA_Environment *ev);

CORBA_short CORBA_DynUnion_get_short(CORBA_DynUnion obj,
				     CORBA_Environment *ev);

CORBA_unsigned_short CORBA_DynUnion_get_ushort(CORBA_DynUnion obj,
					       CORBA_Environment *ev);

CORBA_long CORBA_DynUnion_get_long(CORBA_DynUnion obj,
				   CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynUnion_get_ulong(CORBA_DynUnion obj,
					     CORBA_Environment *ev);

CORBA_float CORBA_DynUnion_get_float(CORBA_DynUnion obj,
				     CORBA_Environment *ev);

CORBA_double CORBA_DynUnion_get_double(CORBA_DynUnion obj,
				       CORBA_Environment *ev);

CORBA_char *CORBA_DynUnion_get_string(CORBA_DynUnion obj,
				      CORBA_Environment *ev);

CORBA_Object CORBA_DynUnion_get_reference(CORBA_DynUnion obj,
					  CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynUnion_get_typecode(CORBA_DynUnion obj,
					   CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
CORBA_long_long CORBA_DynUnion_get_longlong(CORBA_DynUnion obj,
					    CORBA_Environment *ev);

CORBA_unsigned_long_long CORBA_DynUnion_get_ulonglong(CORBA_DynUnion obj,
						      CORBA_Environment *ev);
#endif

CORBA_long_double CORBA_DynUnion_get_longdouble(CORBA_DynUnion obj,
						CORBA_Environment *ev);

CORBA_wchar CORBA_DynUnion_get_wchar(CORBA_DynUnion obj,
				     CORBA_Environment *ev);

CORBA_wchar *CORBA_DynUnion_get_wstring(CORBA_DynUnion obj,
					CORBA_Environment *ev);

CORBA_any *CORBA_DynUnion_get_any(CORBA_DynUnion obj,
				  CORBA_Environment *ev);

CORBA_DynAny CORBA_DynUnion_current_component(CORBA_DynUnion obj,
					      CORBA_Environment *ev);

CORBA_boolean CORBA_DynUnion_next(CORBA_DynUnion obj,
				  CORBA_Environment *ev);

CORBA_boolean CORBA_DynUnion_seek(CORBA_DynUnion obj,
				  CORBA_long index,
				  CORBA_Environment *ev);

void CORBA_DynUnion_rewind(CORBA_DynUnion obj,
			   CORBA_Environment *ev);

CORBA_AnySeq *CORBA_DynSequence_get_elements(CORBA_DynSequence obj,
					     CORBA_Environment *ev);

void CORBA_DynSequence_set_elements(CORBA_DynSequence obj,
				    CORBA_AnySeq *value,
				    CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynSequence__get_length(CORBA_DynSequence obj,
						  CORBA_Environment *ev);

void CORBA_DynSequence__set_length(CORBA_DynSequence obj,
				   CORBA_unsigned_long length,
				   CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynSequence_type(CORBA_DynSequence obj,
				      CORBA_Environment *ev);

void CORBA_DynSequence_assign(CORBA_DynSequence obj,
			      CORBA_DynAny dyn_any,
			      CORBA_Environment *ev);

void CORBA_DynSequence_from_any(CORBA_DynSequence obj,
				CORBA_any value,
				CORBA_Environment *ev);

CORBA_any *CORBA_DynSequence_to_any(CORBA_DynSequence obj,
				    CORBA_Environment *ev);

void CORBA_DynSequence_destroy(CORBA_DynSequence obj,
			       CORBA_Environment *ev);

CORBA_DynAny CORBA_DynSequence_copy(CORBA_DynSequence obj,
				    CORBA_Environment *ev);

void CORBA_DynSequence_insert_boolean(CORBA_DynSequence obj,
				      CORBA_boolean value,
				      CORBA_Environment *ev);

void CORBA_DynSequence_insert_octet(CORBA_DynSequence obj,
				    CORBA_octet value,
				    CORBA_Environment *ev);

void CORBA_DynSequence_insert_char(CORBA_DynSequence obj,
				   CORBA_char value,
				   CORBA_Environment *ev);

void CORBA_DynSequence_insert_short(CORBA_DynSequence obj,
				    CORBA_short value,
				    CORBA_Environment *ev);

void CORBA_DynSequence_insert_ushort(CORBA_DynSequence obj,
				     CORBA_unsigned_short value,
				     CORBA_Environment *ev);

void CORBA_DynSequence_insert_long(CORBA_DynSequence obj,
				   CORBA_long value,
				   CORBA_Environment *ev);

void CORBA_DynSequence_insert_ulong(CORBA_DynSequence obj,
				    CORBA_unsigned_long value,
				    CORBA_Environment *ev);

void CORBA_DynSequence_insert_float(CORBA_DynSequence obj,
				    CORBA_float value,
				    CORBA_Environment *ev);

void CORBA_DynSequence_insert_double(CORBA_DynSequence obj,
				     CORBA_double value,
				     CORBA_Environment *ev);

void CORBA_DynSequence_insert_string(CORBA_DynSequence obj,
				     CORBA_char *value,
				     CORBA_Environment *ev);

void CORBA_DynSequence_insert_reference(CORBA_DynSequence obj,
					CORBA_Object value,
					CORBA_Environment *ev);

void CORBA_DynSequence_insert_typecode(CORBA_DynSequence obj,
				       CORBA_TypeCode value,
				       CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
void CORBA_DynSequence_insert_longlong(CORBA_DynSequence obj,
				       CORBA_long_long value,
				       CORBA_Environment *ev);

void CORBA_DynSequence_insert_ulonglong(CORBA_DynSequence obj,
					CORBA_unsigned_long_long value,
					CORBA_Environment *ev);
#endif

void CORBA_DynSequence_insert_longdouble(CORBA_DynSequence obj,
					 CORBA_long_double value,
					 CORBA_Environment *ev);

void CORBA_DynSequence_insert_wchar(CORBA_DynSequence obj,
				    CORBA_wchar value,
				    CORBA_Environment *ev);

void CORBA_DynSequence_insert_wstring(CORBA_DynSequence obj,
				      CORBA_wchar *value,
				      CORBA_Environment *ev);

void CORBA_DynSequence_insert_any(CORBA_DynSequence obj,
				  CORBA_any value,
				  CORBA_Environment *ev);

CORBA_boolean CORBA_DynSequence_get_boolean(CORBA_DynSequence obj,
					    CORBA_Environment *ev);

CORBA_octet CORBA_DynSequence_get_octet(CORBA_DynSequence obj,
					CORBA_Environment *ev);

CORBA_char CORBA_DynSequence_get_char(CORBA_DynSequence obj,
				      CORBA_Environment *ev);

CORBA_short CORBA_DynSequence_get_short(CORBA_DynSequence obj,
					CORBA_Environment *ev);

CORBA_unsigned_short CORBA_DynSequence_get_ushort(CORBA_DynSequence obj,
						  CORBA_Environment *ev);

CORBA_long CORBA_DynSequence_get_long(CORBA_DynSequence obj,
				      CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynSequence_get_ulong(CORBA_DynSequence obj,
						CORBA_Environment *ev);

CORBA_float CORBA_DynSequence_get_float(CORBA_DynSequence obj,
					CORBA_Environment *ev);

CORBA_double CORBA_DynSequence_get_double(CORBA_DynSequence obj,
					  CORBA_Environment *ev);

CORBA_char *CORBA_DynSequence_get_string(CORBA_DynSequence obj,
					 CORBA_Environment *ev);

CORBA_Object CORBA_DynSequence_get_reference(CORBA_DynSequence obj,
					     CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynSequence_get_typecode(CORBA_DynSequence obj,
					      CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
CORBA_long_long CORBA_DynSequence_get_longlong(CORBA_DynSequence obj,
					       CORBA_Environment *ev);

CORBA_unsigned_long_long CORBA_DynSequence_get_ulonglong(CORBA_DynSequence obj,
							 CORBA_Environment *ev);
#endif

CORBA_long_double CORBA_DynSequence_get_longdouble(CORBA_DynSequence obj,
						   CORBA_Environment *ev);

CORBA_wchar CORBA_DynSequence_get_wchar(CORBA_DynSequence obj,
					CORBA_Environment *ev);

CORBA_wchar *CORBA_DynSequence_get_wstring(CORBA_DynSequence obj,
					   CORBA_Environment *ev);

CORBA_any *CORBA_DynSequence_get_any(CORBA_DynSequence obj,
				     CORBA_Environment *ev);

CORBA_DynAny CORBA_DynSequence_current_component(CORBA_DynSequence obj,
						 CORBA_Environment *ev);

CORBA_boolean CORBA_DynSequence_next(CORBA_DynSequence obj,
				     CORBA_Environment *ev);

CORBA_boolean CORBA_DynSequence_seek(CORBA_DynSequence obj,
				     CORBA_long index,
				     CORBA_Environment *ev);

void CORBA_DynSequence_rewind(CORBA_DynSequence obj,
			      CORBA_Environment *ev);

CORBA_AnySeq *CORBA_DynArray_get_elements(CORBA_DynArray obj,
					  CORBA_Environment *ev);

void CORBA_DynArray_set_elements(CORBA_DynArray obj,
				 CORBA_AnySeq *value,
				 CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynArray_type(CORBA_DynArray obj,
				   CORBA_Environment *ev);

void CORBA_DynArray_assign(CORBA_DynArray obj,
			   CORBA_DynAny dyn_any,
			   CORBA_Environment *ev);

void CORBA_DynArray_from_any(CORBA_DynArray obj,
			     CORBA_any value,
			     CORBA_Environment *ev);

CORBA_any *CORBA_DynArray_to_any(CORBA_DynArray obj,
				 CORBA_Environment *ev);

void CORBA_DynArray_destroy(CORBA_DynArray obj,
			    CORBA_Environment *ev);

CORBA_DynAny CORBA_DynArray_copy(CORBA_DynArray obj,
				 CORBA_Environment *ev);

void CORBA_DynArray_insert_boolean(CORBA_DynArray obj,
				   CORBA_boolean value,
				   CORBA_Environment *ev);

void CORBA_DynArray_insert_octet(CORBA_DynArray obj,
				 CORBA_octet value,
				 CORBA_Environment *ev);

void CORBA_DynArray_insert_char(CORBA_DynArray obj,
				CORBA_char value,
				CORBA_Environment *ev);

void CORBA_DynArray_insert_short(CORBA_DynArray obj,
				 CORBA_short value,
				 CORBA_Environment *ev);

void CORBA_DynArray_insert_ushort(CORBA_DynArray obj,
				  CORBA_unsigned_short value,
				  CORBA_Environment *ev);

void CORBA_DynArray_insert_long(CORBA_DynArray obj,
				CORBA_long value,
				CORBA_Environment *ev);

void CORBA_DynArray_insert_ulong(CORBA_DynArray obj,
				 CORBA_unsigned_long value,
				 CORBA_Environment *ev);

void CORBA_DynArray_insert_float(CORBA_DynArray obj,
				 CORBA_float value,
				 CORBA_Environment *ev);

void CORBA_DynArray_insert_double(CORBA_DynArray obj,
				  CORBA_double value,
				  CORBA_Environment *ev);

void CORBA_DynArray_insert_string(CORBA_DynArray obj,
				  CORBA_char *value,
				  CORBA_Environment *ev);

void CORBA_DynArray_insert_reference(CORBA_DynArray obj,
				     CORBA_Object value,
				     CORBA_Environment *ev);

void CORBA_DynArray_insert_typecode(CORBA_DynArray obj,
				    CORBA_TypeCode value,
				    CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
void CORBA_DynArray_insert_longlong(CORBA_DynArray obj,
				    CORBA_long_long value,
				    CORBA_Environment *ev);

void CORBA_DynArray_insert_ulonglong(CORBA_DynArray obj,
				     CORBA_unsigned_long_long value,
				     CORBA_Environment *ev);
#endif

void CORBA_DynArray_insert_longdouble(CORBA_DynArray obj,
				      CORBA_long_double value,
				      CORBA_Environment *ev);

void CORBA_DynArray_insert_wchar(CORBA_DynArray obj,
				 CORBA_wchar value,
				 CORBA_Environment *ev);

void CORBA_DynArray_insert_wstring(CORBA_DynArray obj,
				   CORBA_wchar *value,
				   CORBA_Environment *ev);

void CORBA_DynArray_insert_any(CORBA_DynArray obj,
			       CORBA_any value,
			       CORBA_Environment *ev);

CORBA_boolean CORBA_DynArray_get_boolean(CORBA_DynArray obj,
					 CORBA_Environment *ev);

CORBA_octet CORBA_DynArray_get_octet(CORBA_DynArray obj,
				     CORBA_Environment *ev);

CORBA_char CORBA_DynArray_get_char(CORBA_DynArray obj,
				   CORBA_Environment *ev);

CORBA_short CORBA_DynArray_get_short(CORBA_DynArray obj,
				     CORBA_Environment *ev);

CORBA_unsigned_short CORBA_DynArray_get_ushort(CORBA_DynArray obj,
					       CORBA_Environment *ev);

CORBA_long CORBA_DynArray_get_long(CORBA_DynArray obj,
				   CORBA_Environment *ev);

CORBA_unsigned_long CORBA_DynArray_get_ulong(CORBA_DynArray obj,
					     CORBA_Environment *ev);

CORBA_float CORBA_DynArray_get_float(CORBA_DynArray obj,
				     CORBA_Environment *ev);

CORBA_double CORBA_DynArray_get_double(CORBA_DynArray obj,
				       CORBA_Environment *ev);

CORBA_char *CORBA_DynArray_get_string(CORBA_DynArray obj,
				      CORBA_Environment *ev);

CORBA_Object CORBA_DynArray_get_reference(CORBA_DynArray obj,
					  CORBA_Environment *ev);

CORBA_TypeCode CORBA_DynArray_get_typecode(CORBA_DynArray obj,
					   CORBA_Environment *ev);

#ifdef HAVE_CORBA_LONG_LONG
CORBA_long_long CORBA_DynArray_get_longlong(CORBA_DynArray obj,
					    CORBA_Environment *ev);

CORBA_unsigned_long_long CORBA_DynArray_get_ulonglong(CORBA_DynArray obj,
						      CORBA_Environment *ev);
#endif

CORBA_long_double CORBA_DynArray_get_longdouble(CORBA_DynArray obj,
						CORBA_Environment *ev);

CORBA_wchar CORBA_DynArray_get_wchar(CORBA_DynArray obj,
				     CORBA_Environment *ev);

CORBA_wchar *CORBA_DynArray_get_wstring(CORBA_DynArray obj,
					CORBA_Environment *ev);

CORBA_any *CORBA_DynArray_get_any(CORBA_DynArray obj,
				  CORBA_Environment *ev);

CORBA_DynAny CORBA_DynArray_current_component(CORBA_DynArray obj,
					      CORBA_Environment *ev);

CORBA_boolean CORBA_DynArray_next(CORBA_DynArray obj,
				  CORBA_Environment *ev);

CORBA_boolean CORBA_DynArray_seek(CORBA_DynArray obj,
				  CORBA_long index,
				  CORBA_Environment *ev);

void CORBA_DynArray_rewind(CORBA_DynArray obj,
			   CORBA_Environment *ev);

#endif /* !_ORBIT_DYNANY_H_ */
