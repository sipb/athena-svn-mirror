/********************************************************************/
/*                                                                  */
/*                      N A M E T A B L E                           */
/*                                                                  */
/*                Name and scope table management                   */
/*                                                                  */
/********************************************************************/

/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ========================================================================== 
 */



#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "idl_base.h"
#include "errors.h"
#include "utils.h"
#include "nametbl.h"
#include "sysdep.h"

/********************************************************************/
/*                                                                  */
/*              Data types private to this module.                  */
/*                                                                  */
/********************************************************************/

typedef struct bindingType {
        int     bindingLevel;
        char   *theBinding;
        struct bindingType *nextBindingThisLevel;
        struct bindingType *oldBinding;
        struct hash_slot   *boundBy;
}                           
bindingType;

typedef struct hash_slot {
        boolean available;              /* This slot is available               */
        char   *id;                         /* Actual identifier                    */
        char   *upcased_id;             /* For comparisons...                   */
        struct bindingType *bindings;
        /* So someone actually use this...      */
}                           
hash_slot;

typedef hash_slot * hashSlotPT;

/********************************************************************/
/*                                                                  */
/*              Private data declarations.                          */
/*                                                                  */
/********************************************************************/


#define MAX_LEVELS 20

static  bindingType * levelStack[MAX_LEVELS];
/* Each entry points to the list of     */
/* bindings made at the current level   */

static int  currentLevel=0;       /* Current scoping level                */

    /* The name table for the compiler      */
#ifndef     NAMETABLE_SIZE
#ifdef MSDOS
#define     NAMETABLE_SIZE     1583
#else
#define     NAMETABLE_SIZE     8013 
#endif
#endif

static  int free_slots = NAMETABLE_SIZE;

#ifndef     STRTAB_SIZE
#define     STRTAB_SIZE     10*1024
#endif 


#ifdef MSDOS
    /* Compiling with Microsoft C: use huge modifier to put
       name and string tables in separate segments */
    static hash_slot huge nameTableData[NAMETABLE_SIZE] = {0};
#   define nameTable(index,member) nameTableData[index].member
    static char huge STRTAB[STRTAB_SIZE];/* The string table */
#else
    /* Compiling for UNIX or AEGIS */
    static hash_slot nameTableData[NAMETABLE_SIZE];
#   define nameTable(index,member) nameTableData[index].member
    static char STRTAB[STRTAB_SIZE];/* The string table */
#endif

static int  STRTAB_index;       /* Current index...                      
                                */

/********************************************************************/
/*                                                                  */
/*              Private Functions.                                  */
/*                                                                  */
/********************************************************************/

/*--------------------------------------------------------------------*/

/*
 *  hash1 and hash2 are subsidiary functions used by 
 *  namtab_add_id. 
 *
 */


#ifndef __STDC__
long     hash1 (id)
     char   *id;
#else
long hash1 (char *id)
#endif    
{
  char   *p;
  unsigned long h = 0,
  g;
  
  for (p = id; *p; p++)
    {
      h = (h << 4) + (*p) + 1;
      
      if (g = h & 0xf0000000L)
        {
          h = h ^ (g >> 24);
          h = h ^ g;
        }
    }
  return (h);
}


#ifndef __STDC__
long     hash2 (id)
     char   *id;
#else
long hash2 (char *id)
#endif
{
  char   *p;
  unsigned long    h = 0,
  g;
  
  for (p = id; *p; p++) {
    h = (h << 4) ^ (*p) + 3;
    
    if (g = h & 0x0ff00fL) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return (h);
}


/************************************************************************************************/
/*                                                                                              */
/*                           P U B L I C   I N T E R F A C E S                                  */
/*                                                                                              */
/************************************************************************************************/

/*--------------------------------------------------------------------*/

/*
 * Function:  add an identifier to the id table.
 *
 * Inputs:    id     - The character string to be added.
 *
 * Outputs:
 *
 * Functional Value: a handle which can be used to refer to the
 *                   stored identifier.
 *
 * Notes:      This is based on algorithm D from Knuth Vol. 3
 *
 */

NAMETABLE_id_t NAMETABLE_add_id (id, upcase)
char   *id;
boolean upcase;
{
        long     hidx,
        hidx2;
        long     hval;
        int     id_len;
        int     i;
        char    upcased_id[max_string_len];
                           
    if (free_slots == 0)
        error("Symbol table overflow\n") ;

        strcpy (upcased_id, id);

        if (upcase)
                for (i = 0; id[i]; i++)
                        if (islower (upcased_id[i]))
                                upcased_id[i] = toupper (upcased_id[i]);



        hval = hash1 (upcased_id);
        hidx = hval % NAMETABLE_SIZE;

        /* 
          * Scan the table looking for either the identifier
          * or an empty slot into which the identifier will
          * be stuck.
          *
  */


        if (!nameTable(hidx,available) || hidx == NAMETABLE_NIL_ID) {

                hidx2 = (hval = hash2 (upcased_id)) % NAMETABLE_SIZE;

                while ((!nameTable(hidx,available)) || (hidx == NAMETABLE_NIL_ID)) {
                        /* Has the identifier already been put in ? */
                        if ((hidx != NAMETABLE_NIL_ID) && (strcmp (nameTable(hidx,upcased_id), upcased_id) == 0))
                                return (int)hidx;

                        /* Nope, rehash and try again */
                        if ((hidx -= hidx2) < 0)
                                hidx += NAMETABLE_SIZE;
                };
        };

        /* 
          * The identifier was not in the table, but we found an empty slot.
          * Fill in the slot and return.
          *
  */

        id_len = strlen (upcased_id) + 1;
        nameTable(hidx,id) = alloc (id_len);
        nameTable(hidx,upcased_id) = alloc (id_len);
        strcpy (nameTable(hidx,upcased_id), upcased_id);
        strcpy (nameTable(hidx,id), id);
        nameTable(hidx,bindings) = 0;
        nameTable(hidx,available) = false;
    --free_slots ;
/*printf("add_id: index %d upcase %s(%Fp) id %s(%Fp) free_slots %d\n",
        (int)hidx,
        nameTable(hidx,upcased_id), nameTable(hidx,upcased_id),
        nameTable(hidx,id), nameTable(hidx,id),
        free_slots);
*/      
        return (int)hidx;

}

/*--------------------------------------------------------------------*/

/*
 * Function:  Converts an namtab identifier back to the original string.
 *
 * Inputs:    id      - The handle returned by namtabadd_id
 *
 * Outputs:   str_ptr - The character string is returned through here.
 *
 * Functional Value: void
 *
 * Notes:   
 *
 */

void NAMETABLE_id_to_string (NAMETABLE_id, str_ptr)
NAMETABLE_id_t NAMETABLE_id;
char  **str_ptr;
{
    if (NAMETABLE_id == NAMETABLE_NIL_ID)
        *str_ptr = "";
    else
        *str_ptr = nameTable(NAMETABLE_id,id);
}

/*--------------------------------------------------------------------*/

/*
 * Function:  Binds a symbol to a value.
 *
 * Inputs:    id      - The handle returned by NAMETABLE_add_id
 *
 *            binding - a pointer representing the binding
 *
 * Outputs:
 *
 * Functional Value: void
 *
 * Notes:     For each symbol in the name table, there is a binding
 *            stack.  Conceptually the stack gets pushed every time
 *            a new scope is entered, and popped on scope exit.  The
 *            symbol table is "shallowly bound" rather than 
 *            "deeply bound' in the lisp since.  This makes lookup
 *            fast, but pushing and popping slow.
 *
 */

#ifndef __STDC__
boolean NAMETABLE_add_binding (id, binding)
NAMETABLE_id_t id;
char   *binding;
#else
boolean NAMETABLE_add_binding (NAMETABLE_id_t id, void *binding)
#endif
{
        hash_slot * idP;
        bindingType * bindingP;
        bindingType * newBindingP;

        idP = (hash_slot *) &(nameTable(id,available));

        if (!idP->bindings) {
                /* 
                     *  First binding for this identifier.
                     *
     */

                bindingP = idP->bindings = (bindingType *) alloc (sizeof (bindingType));
                bindingP->oldBinding = NULL;
        }
        else {
                bindingP = idP->bindings;
                if (bindingP->bindingLevel == currentLevel)
                        return false;
                newBindingP = (bindingType *) alloc (sizeof (bindingType));
                newBindingP->oldBinding = bindingP;
                bindingP = newBindingP;
        }

        bindingP->theBinding = binding;
        idP->bindings = bindingP;
        bindingP->bindingLevel = currentLevel;
        bindingP->nextBindingThisLevel = levelStack[currentLevel];
        bindingP->boundBy = (struct hash_slot *) idP;
        levelStack[currentLevel] = bindingP;
        return true;
}

/*--------------------------------------------------------------------*/

/*
 * Function:  Returns the binding associated with the id
 *
 * Inputs:    id      - The handle returned by NAMETABLE_add_id
 *
 *
 * Outputs:
 *
 * Functional Value:  - A pointer to the binding if there is one,
 *                      0 otherwise.
 *
 */

char   *NAMETABLE_lookup_binding (identifier)
NAMETABLE_id_t identifier;
{
        if (identifier == NAMETABLE_NIL_ID)
                return NULL;

        if (nameTable(identifier,bindings) == NULL)
                return NULL;

        return nameTable(identifier,bindings)->theBinding;
}

/*--------------------------------------------------------------------*/

/*
 * Function:  Returns the binding associated with the id.  The binding must
 *            be within the local scope.
 *
 * Inputs:    id      - The handle returned by NAMETABLE_add_id
 *
 *
 * Outputs:
 *
 * Functional Value:  - A pointer to the binding if there is one,
 *                      0 otherwise.
 *
 */

char   *NAMETABLE_lookup_local (identifier)
NAMETABLE_id_t identifier;
{
        if (identifier == NAMETABLE_NIL_ID)
                return NULL;

        if (nameTable(identifier,bindings) == NULL)
                return NULL;

        if (nameTable(identifier,bindings)->bindingLevel < currentLevel)
                return NULL;

        return nameTable(identifier,bindings)->theBinding;
}

/*--------------------------------------------------------------------*/

/*
 * Function:  Pushes a new scoping level.
 *
 * Inputs:    
 *
 * Outputs:   
 *
 * Functional Value: void
 *
 * Notes:    Aborts with an error if too many levels are pushed.
 *
 */

void NAMETABLE_push_level () {
        if (currentLevel < MAX_LEVELS)
                levelStack[++currentLevel] = NULL;
        else
                error ("too many scoping levels\n");
}

/*--------------------------------------------------------------------*/

/*
 * Function:  Pops a scoping level.
 *
 * Inputs:    
 *
 * Outputs:   
 *
 * Functional Value: void
 *
 * Notes:    Aborts with an error if there are no more levels.
 *
 */

void NAMETABLE_pop_level () {
        bindingType * bp;
        bindingType * obp;
        struct hash_slot   *hsp;

        if (currentLevel == 0)
                error ("Can't pop symbol table\n");

        bp = levelStack[currentLevel--];

        while (bp) {
                hsp = bp->boundBy;
                hsp->bindings = (struct bindingType  *) bp->oldBinding;
                obp = bp;
                bp = obp->nextBindingThisLevel;
                free ((char *)obp);
        }
}

/*--------------------------------------------------------------------*/

/*
 * Function:  Enters a string into the string table
 *
 * Inputs:    
 *
 * Outputs:   
 *
 * Functional Value: void
 *
 * Notes:   
 *
 */

STRTAB_str_t STRTAB_add_string (string)
char   *string;
{
        int     string_handle;

        strcpy (&STRTAB[STRTAB_index], string);
        string_handle = STRTAB_index;
        STRTAB_index += strlen (string) + 1;
        return string_handle;
}

/*--------------------------------------------------------------------*/

/*
 * Function:  Given a string ID, returns the corresponding string
 *
 * Inputs:    id - a previously obtained string id
 *
 * Outputs:   strp - a pointer to corresponding string is return here.
 *
 * Functional Value: void
 *
 * Notes:   
 *
 */

void STRTAB_str_to_string (str, strp)
STRTAB_str_t str;
char  **strp;
{
        *strp = &STRTAB[str];
}

/*--------------------------------------------------------------------*/

/*
 * Function: Initialize the name table.
 *
 * Inputs: 
 *         
 *
 * Outputs:   
 *
 * Functional Value: 
 *
 * Notes:   
 *
 */

#ifdef MSDOS
#ifdef TURBOC
static hash_slot huge *nameTablePtr(i)
int i;
{
        char huge *tmp;
        hash_slot huge *return_val;
        long offset;
        tmp = (char *) nameTableData;
        offset = (long) sizeof(hash_slot)  * (long)i;
        tmp += offset;
        return_val = (hash_slot *)tmp;
        return (return_val);
}
#endif
#endif

void NAMETABLE_init () {
        int     i;

#ifdef MSDOS
#ifdef TURBOC
        {
                void far *farcalloc();
                nameTableData = farcalloc((unsigned long)NAMETABLE_SIZE,
                                      (unsigned long) sizeof(hash_slot));
                if(!nameTableData)
                        error("Couldn't allocate memory for nameTable");
        }
#endif
#endif
        for (i = 0; i < NAMETABLE_SIZE; i++) {
                nameTable(i,available) = true;
                nameTable(i,bindings) = NULL;
        };
}

/*--------------------------------------------------------------------*/

/*
 * Function: Dump a name table in human-readable form.
 *
 * Inputs:   name_table - the table to be dumped.
 *
 * Outputs:   
 *
 * Functional Value: 
 *
 * Notes:   
 *
 */

void NAMETABLE_dump_tab () {
        int     i;
        bindingType * p;


        for (i = 0; i < NAMETABLE_SIZE; i++)
                if (!nameTable(i,available)) {
                        printf ("\n\n\nid = %s\n", nameTable(i,id));
                        printf ("Bindings for id @ %Fp\n", (hash_slot *) &nameTable(i,available));
                        printf ("------------------------\n\n");

                        p = nameTable(i,bindings);
                        while (p != NULL) {
                                printf ("\tbindingLevel = %d\n", p->bindingLevel);
                                printf ("\ttheBinding = %lx\n", p->theBinding);
                                printf ("\tnextBindingThisLevel = %lx\n", p->nextBindingThisLevel);
                                printf ("\toldBinding = %lx\n", p->oldBinding);
                                printf ("\tboundBy = %lx\n\n", p->boundBy);
                                p = p->oldBinding;
                        }
                }
}

/*--------------------------------------------------------------------*/

/*
 *
 * Function:    Initializes the string table.
 *
 * Inputs:      
 *
 * Outputs:
 *
 * Notes:       The string table is a fixed size allocation
 *
 */

void STRTAB_init () {
        STRTAB_index = 0;
}

/*--------------------------------------------------------------------*/

/*
 *
 * Function:    Given a nametable id and a string, creates a new
 *              nametable entry with the name dentoed by the id
 *              concatenated with the given string.
 *
 * Inputs:      id - a nametable id.
 *              suffix - a string to be added to the name.
 *
 * Outputs:
 *
 * Functional value: the new nametable id.
 *
 */

NAMETABLE_id_t NAMETABLE_add_derived_name (id, matrix)
NAMETABLE_id_t id;
char   *matrix;
{
        char    new_name[max_string_len];
        char   *old_name_p;

        NAMETABLE_id_to_string (id, &old_name_p);
        sprintf (new_name, matrix, old_name_p);

        return NAMETABLE_add_id (new_name, true);
}

NAMETABLE_id_t NAMETABLE_add_derived_name2 (id1, id2, matrix)
NAMETABLE_id_t id1, id2;
char   *matrix;
{
        char    new_name[max_string_len];
        char   *old_name1_p,
        *old_name2_p;

        NAMETABLE_id_to_string (id1, &old_name1_p);
        NAMETABLE_id_to_string (id2, &old_name2_p);
        sprintf (new_name, matrix, old_name1_p, old_name2_p);

        return NAMETABLE_add_id (new_name, true);
}
