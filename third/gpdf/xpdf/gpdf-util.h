/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#ifndef GPDF_UTIL_H
#define GPDF_UTIL_H

/* This is for C++ _definitions_ to be exported with C linkage */
/* Hide braces and get back one level of indentation */
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }


#include <bonobo/bonobo-macros.h>

/* FIXME kill when GNOME's macros are fixed */
/*
 * Copied from gnome-macros by George Lebl, added casts for C++
 */

#define GPDF_CLASS_BOILERPLATE(type, type_as_function,			\
			       parent_type, parent_type_macro)		\
	BONOBO_BOILERPLATE(type, type_as_function, type,		\
			   parent_type, parent_type_macro,		\
			   GPDF_REGISTER_TYPE)

#define GPDF_REGISTER_TYPE(type, type_as_function, corba_type,		\
		    	   parent_type, parent_type_macro)		\
	g_type_register_static (parent_type_macro, #type, &object_info, \
				(GTypeFlags)0)

#endif /* GPDF_UTIL_H */
