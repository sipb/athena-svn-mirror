#include "port.h"
#ifndef port_dictionary_TYPE
#define port_dictionary_TYPE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

typedef struct _port_dictionary_binding {
    struct _port_dictionary_binding *next;       /* PRIVATE */
    char *key;                                     /* READ-ONLY */
    port value;
} port_dictionary_binding;

typedef struct _port_dictionary {                /* PRIVATE */
    int size;
    port_dictionary_binding **slots;
} *port_dictionary;

/*
 *    port_dictionary port_dictionary_Create(int size):
 *        Requires: size > 0
 *        Effects: Returns a new empty dictionary containing no bindings.
 *                 The returned dictionary must be destroyed using
 *                 port_dictionary_Destroy.  Size is a time vs space
 *                 parameter.  For this implementation, space used is
 *                 proportional to size and time used is proportional
 *                 to number of bindings divided by size.  It is preferable
 *                 that size is a prime number.
 */

extern port_dictionary port_dictionary_Create(/* int size */);

/*
 *    void port_dictionary_Destroy(port_dictionary d):
 *        Requires: d is a non-destroyed port_dictionary
 *        Modifies: d
 *        Effects: Destroys dictionary d freeing up the space it consumes.
 *                 Dictionary d should never be referenced again.  Note that
 *                 free is NOT called on the values of the bindings.  If
 *                 this is needed, the client must do this first using
 *                 port_dictionary_Enumerate.
 */

extern void port_dictionary_Destroy(/* port_dictionary d */);

/*
 *    void port_dictionary_Enumerate(port_dictionary d; void (*proc)()):
 *        Requires: proc is a void procedure taking 1 argument, a
 *                  port_dictionary_binding pointer, which does not
 *                  make any calls using dictionary d.
 *        Effects: Calls proc once with each binding in dictionary d.
 *                 Order of bindings passed is undefined.  Note that
 *                 only the value field of the binding should be considered
 *                 writable by proc.
 */

extern void port_dictionary_Enumerate(/* port_dictionary d, 
					   void (*proc)() */);

/*
 *    port_dictionary_binding *port_dictionary_Lookup(port_dictionary d,
 *                                                        char *key):
 *        Effects: If key is not bound in d, returns 0.  Othersize,
 *                 returns a pointer to the binding that binds key.
 *                 Note the access restrictions on bindings...
 */

extern port_dictionary_binding *port_dictionary_Lookup(/* d, key */);

/*
 *    port_dictionary_binding *port_dictionary_Define(port_dictionary d,
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

extern port_dictionary_binding *port_dictionary_Define();

/*
 *    void port_dictionary_Delete(port_dictionary d,
 *                                  port_dictionary_binding *b):
 *        Requires: *b is a binding in d.
 *        Modifies: d
 *        Effects: Removes the binding *b from d.  Note that if 
 *                 b->value needs to be freed, it should be freed
 *                 before making this call.
 */

extern void port_dictionary_Delete();

#endif
