#include "sysdep.h"
#ifndef ulong_dictionary_TYPE
#define ulong_dictionary_TYPE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

typedef struct _ulong_dictionary_binding {
    struct _ulong_dictionary_binding *next;       /* PRIVATE */
    char *key;                                     /* READ-ONLY */
    ulong value;
} ulong_dictionary_binding;

typedef struct _ulong_dictionary {                /* PRIVATE */
    int size;
    ulong_dictionary_binding **slots;
} *ulong_dictionary;

/*
 *    ulong_dictionary ulong_dictionary_Create(int size):
 *        Requires: size > 0
 *        Effects: Returns a new empty dictionary containing no bindings.
 *                 The returned dictionary must be destroyed using
 *                 ulong_dictionary_Destroy.  Size is a time vs space
 *                 parameter.  For this implementation, space used is
 *                 proportional to size and time used is proportional
 *                 to number of bindings divided by size.  It is preferable
 *                 that size is a prime number.
 */

extern ulong_dictionary ulong_dictionary_Create(/* int size */);

/*
 *    void ulong_dictionary_Destroy(ulong_dictionary d):
 *        Requires: d is a non-destroyed ulong_dictionary
 *        Modifies: d
 *        Effects: Destroys dictionary d freeing up the space it consumes.
 *                 Dictionary d should never be referenced again.  Note that
 *                 free is NOT called on the values of the bindings.  If
 *                 this is needed, the client must do this first using
 *                 ulong_dictionary_Enumerate.
 */

extern void ulong_dictionary_Destroy(/* ulong_dictionary d */);

/*
 *    void ulong_dictionary_Enumerate(ulong_dictionary d; void (*proc)()):
 *        Requires: proc is a void procedure taking 1 argument, a
 *                  ulong_dictionary_binding pointer, which does not
 *                  make any calls using dictionary d.
 *        Effects: Calls proc once with each binding in dictionary d.
 *                 Order of bindings passed is undefined.  Note that
 *                 only the value field of the binding should be considered
 *                 writable by proc.
 */

extern void ulong_dictionary_Enumerate(/* ulong_dictionary d, 
					   void (*proc)() */);

/*
 *    ulong_dictionary_binding *ulong_dictionary_Lookup(ulong_dictionary d,
 *                                                        char *key):
 *        Effects: If key is not bound in d, returns 0.  Othersize,
 *                 returns a pointer to the binding that binds key.
 *                 Note the access restrictions on bindings...
 */

extern ulong_dictionary_binding *ulong_dictionary_Lookup(/* d, key */);

/*
 *    ulong_dictionary_binding *ulong_dictionary_Define(ulong_dictionary d,
 *                                            char *key,
 *                                            int *already_existed):
 *        Modifies: d
 *        Effects: If key is bound in d, returns a pointer to the binding
 *                 that binds key.  Otherwise, adds a binding of key to
 *                 d and returns its address.  If already_existed is non-zero
 *                 then *already_existed is set to 0 if key was not
 *                 previously bound in d and 1 otherwise.
 *                 Note the access restrictions on bindings...  Note also
 *                 that the value that key is bounded to if a binding is
 *                 created is undefined.  The caller should set the value
 *                 in this case.
 */

extern ulong_dictionary_binding *ulong_dictionary_Define();

/*
 *    void ulong_dictionary_Delete(ulong_dictionary d,
 *                                  ulong_dictionary_binding *b):
 *        Requires: *b is a binding in d.
 *        Modifies: d
 *        Effects: Removes the binding *b from d.  Note that if 
 *                 b->value needs to be freed, it should be freed
 *                 before making this call.
 */

extern void ulong_dictionary_Delete();

#endif
