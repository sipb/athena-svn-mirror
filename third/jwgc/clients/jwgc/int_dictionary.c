/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

/*
 * dictionary - a module implementing a generic dictionary.  That is,
 *              any type can be used for the values that keys are bound to.
 *              Keys are always strings.
 *
 * Overview:
 *
 *        A dictionary is a set of bindings which bind values of some
 *    type (this type is the generic parameter of the dictionary) to
 *    strings.  At most one value can be bound to any one string.
 *    The value that a string is bound to can be changed later.
 *    Bindings can also be deleted later.  It is also possible to
 *    enumerate all of the bindings in a dictionary.  Dictionarys
 *    are heap based and must be created & destroyed accordingly.
 *
 *    Note: This module assumes that malloc NEVER returns 0 for reasonable
 *          requests.  It is the users responsibility to either ensure that
 *          this happens or supply a version of malloc with error
 *          handling.
 *
 *    Dictionarys are mutable.
 *
 * Implementation:
 *
 *        A standard chaining hash table is used to implement dictionarys.
 *    Each dictionary has an associated size (# of slots), allowing
 *    different size dictionaries as needed.
 */

#include "int_dictionary.h"
#include "new_string.h"

#include "main.h"

#ifndef NULL
#define NULL 0
#endif

/*
 *    int_dictionary int_dictionary_Create(int size):
 *        Requires: size > 0
 *        Effects: Returns a new empty dictionary containing no bindings.
 *                 The returned dictionary must be destroyed using
 *                 int_dictionary_Destroy.  Size is a time vs space
 *                 parameter.  For this implementation, space used is
 *                 proportional to size and time used is proportional
 *                 to number of bindings divided by size.  It is preferable
 *                 that size is a prime number.
 */

int_dictionary 
int_dictionary_Create(size)
	int size;
{
	int i;
	int_dictionary result;

	result = (int_dictionary) malloc(sizeof(struct _int_dictionary));
	result->size = size;
	result->slots = (int_dictionary_binding **) malloc(
				size * sizeof(int_dictionary_binding *));

	for (i = 0; i < size; i++)
		result->slots[i] = NULL;

	return (result);
}

/*
 *    void int_dictionary_Destroy(int_dictionary d):
 *        Requires: d is a non-destroyed int_dictionary
 *        Modifies: d
 *        Effects: Destroys dictionary d freeing up the space it consumes.
 *                 Dictionary d should never be referenced again.  Note that
 *                 free is NOT called on the values of the bindings.  If
 *                 this is needed, the client must do this first using
 *                 int_dictionary_Enumerate.
 */

void 
int_dictionary_Destroy(d)
	int_dictionary d;
{
	int i;
	int_dictionary_binding *binding_ptr, *new_binding_ptr;

	for (i = 0; i < d->size; i++) {
		binding_ptr = d->slots[i];
		while (binding_ptr) {
			new_binding_ptr = binding_ptr->next;
			free(binding_ptr->key);
			free(binding_ptr);
			binding_ptr = new_binding_ptr;
		}
	}
	free(d->slots);
	free(d);
}

/*
 *    void int_dictionary_Enumerate(int_dictionary d; void (*proc)()):
 *        Requires: proc is a void procedure taking 1 argument, a
 *                  int_dictionary_binding pointer, which does not
 *                  make any calls using dictionary d.
 *        Effects: Calls proc once with each binding in dictionary d.
 *                 Order of bindings passed is undefined.  Note that
 *                 only the value field of the binding should be considered
 *                 writable by proc.
 */

void 
int_dictionary_Enumerate(d, proc)
	int_dictionary d;
	void (*proc) ( /* int_dictionary_binding *b */ );
{
	int i;
	int_dictionary_binding *binding_ptr;

	for (i = 0; i < d->size; i++) {
		binding_ptr = d->slots[i];
		while (binding_ptr) {
			proc(binding_ptr);
			binding_ptr = binding_ptr->next;
		}
	}
}

/*
 *  Private routine:
 *
 *    unsigned int dictionary__hash(char *s):
 *        Effects: Hashs s to an unsigned integer.  This number mod the
 *                 hash table size is supposed to roughly evenly distribute
 *                 keys over the table's slots.
 */

static unsigned int 
dictionary__hash(s)
	char *s;
{
	unsigned int result = 0;

	if (!s)
		return (result);

	while (s[0]) {
		result <<= 1;
		result += s[0];
		s++;
	}

	return (result);
}

/*
 *    int_dictionary_binding *int_dictionary_Lookup(int_dictionary d,
 *                                                        char *key):
 *        Effects: If key is not bound in d, returns 0.  Othersize,
 *                 returns a pointer to the binding that binds key.
 *                 Note the access restrictions on bindings...
 */

int_dictionary_binding *
int_dictionary_Lookup(d, key)
	int_dictionary d;
	char *key;
{
	int_dictionary_binding *binding_ptr;

	binding_ptr = d->slots[dictionary__hash(key) % (d->size)];
	while (binding_ptr) {
		if (string_Eq(key, binding_ptr->key))
			return (binding_ptr);
		binding_ptr = binding_ptr->next;
	}

	return (NULL);
}

/*
 *    int_dictionary_binding *int_dictionary_Define(int_dictionary d,
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

int_dictionary_binding *
int_dictionary_Define(d, key, already_existed)
	int_dictionary d;
	char *key;
	int *already_existed;
{
	int_dictionary_binding **ptr_to_the_slot, *binding_ptr;

	ptr_to_the_slot = &(d->slots[dictionary__hash(key) % (d->size)]);

	binding_ptr = *ptr_to_the_slot;
	while (binding_ptr) {
		if (string_Eq(binding_ptr->key, key)) {
			if (already_existed)
				*already_existed = 1;
			return (binding_ptr);
		}
		binding_ptr = binding_ptr->next;
	}

	if (already_existed)
		*already_existed = 0;
	binding_ptr = (int_dictionary_binding *) malloc(
					 sizeof(int_dictionary_binding));
	binding_ptr->next = *ptr_to_the_slot;
	binding_ptr->key = string_Copy(key);
	*ptr_to_the_slot = binding_ptr;
	return (binding_ptr);
}

/*
 *    void int_dictionary_Delete(int_dictionary d,
 *                                  int_dictionary_binding *b):
 *        Requires: *b is a binding in d.
 *        Modifies: d
 *        Effects: Removes the binding *b from d.  Note that if
 *                 b->value needs to be freed, it should be freed
 *                 before making this call.
 */

void 
int_dictionary_Delete(d, b)
	int_dictionary d;
	int_dictionary_binding *b;
{
	int_dictionary_binding **ptr_to_binding_ptr;

	ptr_to_binding_ptr = &(d->slots[dictionary__hash(b->key) % (d->size)]);

	while (*ptr_to_binding_ptr != b)
		ptr_to_binding_ptr = &((*ptr_to_binding_ptr)->next);

	*ptr_to_binding_ptr = b->next;
	free(b->key);
	free(b);
}
