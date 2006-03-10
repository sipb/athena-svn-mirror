#ifndef string_dictionary_aux_MODULE
#define string_dictionary_aux_MODULE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "string_dictionary.h"

/*
 *    void string_dictionary_Set(string_dictionary d, string key,string value):
 *        Modifies: d
 *        Effects: Binds key to value in d.  Automatically free's the
 *                 previous value of key, if any.  Value is copied on the
 *                 heap.
 */

extern void string__dictionary_Set();
#define string_dictionary_Set(a,b,c)         string__dictionary_Set(a,b,c)

/*
 *    char *string_dictionary_Fetch(string_dictionary d, string key)
 *        Effects: If key is not bound in d, returns 0.  Otherwise,
 *                 returns the value that key is bound to.  
 *                 Note that the returned string if any should not be
 *                 freed or modified in any way.  Note also that it may
 *                 disappear later if key is rebound.
 */

extern char *string_dictionary_Fetch();

/*
 *    void string_dictionary_SafeDestroy(string_dictionary d)
 *        Modifies: d
 *        Effects: Like string_dictionary_Destroy except first frees
 *                 all value's in the dictionary.
 */

extern void string_dictionary_SafeDestroy();

#endif
