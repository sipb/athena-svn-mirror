/********************************************************************/
/*                                                                  */
/*                      C H E C K E R                               */
/*                                                                  */
/*             Semantic checker for IDL compilers                   */
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



#ifdef DSEE
#include "$(base.idl).h"
#else
#include "base.h"
#endif

#include <stdio.h>  
#include <string.h>

#ifdef vms
#  include <types.h>
#  include <stat.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include "idl_base.h"
#include "utils.h"
#include "ast.h"
#include "astp.h"
#include "errors.h"
#include "files.h"
#include "frontend.h"
#include "nametbl.h"
#include "sysdep.h"

#ifndef MAX_IMPORTS
#define MAX_IMPORTS 100             /* Maximum number of imported files */
#endif

#define MAX_SHORT 65535             /* Largest possible  short          */
#define MAX_SMALL 255               /* Largest possible 8 bit integer   */

#define ENUMERATORS_PER_SHORT 16

#define RPC_IMPORT_FN "rpc.idl"     /* File name for auto-imported rpc.idl  */

 /* 
  *  Static data declarations
  */

static int  source_pos;             /* Current source position      */
static int  import_count = 0;       /* Imported file count          */

static struct {                     /* Table of checked imports     */
    char           imported_fn[max_string_len];
    NAMETABLE_id_t interface_name;
    dev_t          st_dev;
    ino_t          st_ino;
}               
checked_interfaces[MAX_IMPORTS];

static STRTAB_str_t rpc_fn;        /* String table id of "rpc.idl" */

 /* 
  *  Type declarations.
  */

typedef enum {
    top_level, record, parameter_list
} 
scope_t;

typedef int interface_attributes;   /* Attributes for interface     */
#define INTERFACE_NULL_ATTRS            0x00000000
#define INTERFACE_IMPLICITLY_BOUND      0x00000001
#define INTERFACE_AUTOMATICALLY_BOUND   0x00000002
#define INTERFACE_LOCAL_ONLY            0x00000004

interface_attributes interface_attrs = 0;
char                 *interface_name = NULL;

extern int  yynerrs;                /* Errors dectected by parser   */
extern  boolean emit_env;           /* -env was specified           */

 /* 
  *  Forward declarations.
  */

binding_t *parse();
void CHECKER_check_interface __PROTOTYPE ((binding_t *binding_node_p, char **import_dirs, char **def_strings));

static  void check_type  __PROTOTYPE((type_t *type_p, scope_t scope, boolean user_marshalled));
static  void check_field __PROTOTYPE((field_t *field_p, boolean has_xmit_type));
static  void resolve_type __PROTOTYPE((type_t *type_p));
static  void resolve_parameter_types __PROTOTYPE((routine_t *routine_p));
static  boolean pointer_kind __PROTOTYPE((type_t *type_p));

extern  void file_not_found __PROTOTYPE((char *file_name));
extern void exit __PROTOTYPE((int));
extern void unimp_type_exit __PROTOTYPE((type_kind kind, char *label));
/*--------------------------------------------------------------------*/

#ifndef __STDC__
static int count_ops(export_list_p)
binding_t * export_list_p;
#else
static int count_ops(binding_t *export_list_p)
#endif

{
    binding_t * export_p;
    int op_count;

    op_count = 0; 

    for (export_p = export_list_p; export_p; export_p = export_p->next)
        if (export_p->kind == routine_k)
            op_count++;

    return op_count;
}


/*--------------------------------------------------------------------*/

#ifndef __STDC__
boolean already_imported (imported_file_name, if_name, directory_list)
STRTAB_str_t imported_file_name;
NAMETABLE_id_t * if_name;
char ** directory_list;
#else
boolean already_imported (STRTAB_str_t imported_file_name, NAMETABLE_id_t *if_name, char **directory_list)
#endif
{
    int    i;
    char   imported_fn[max_string_len];
    struct stat stat_buf;

    if (! lookup(imported_file_name, directory_list, &stat_buf, imported_fn))
        return false;

    for (i = 0; i < import_count; i++)
#ifdef HASINODES
        if ((stat_buf.st_dev == checked_interfaces[i].st_dev) &&
            (stat_buf.st_ino == checked_interfaces[i].st_ino))
#else
            if (strcmp(imported_fn, checked_interfaces[i].imported_fn) == 0)
#endif
                {
                    *if_name = checked_interfaces[i].interface_name;
                    return true;
                }
    return false;
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void record_import (imported_file_name, if_name, directory_list)
STRTAB_str_t imported_file_name;
NAMETABLE_id_t if_name;
char ** directory_list;
#else 
static  void record_import (STRTAB_str_t imported_file_name, NAMETABLE_id_t if_name, char **directory_list)
#endif
{

    char   imported_fn[max_string_len];
    struct stat stat_buf;

    if (! lookup(imported_file_name, directory_list, &stat_buf, imported_fn)) 
        return;

    if (import_count < MAX_IMPORTS) {
        checked_interfaces[import_count].st_dev = stat_buf.st_dev;
        checked_interfaces[import_count].st_ino = stat_buf.st_ino;
        strcpy(checked_interfaces[import_count].imported_fn,imported_fn);
        checked_interfaces[import_count].interface_name = if_name;
        ++import_count;
    }
    else
        error ("Too many imported files");
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static boolean open_type (tp)
type_t *tp;
#else
static boolean open_type (type_t *tp)
#endif
{
    field_t *fp;

    if (tp == NULL)
        return false;

    switch (tp->kind) {

        case handle_k:
        case boolean_k:
        case byte_k:
        case character_k:
        case small_bitset_k:
        case small_enumeration_k:
        case small_integer_k:
        case small_unsigned_k:
        case small_subrange_k:
        case short_bitset_k:
        case short_integer_k:
        case short_unsigned_k:
        case short_subrange_k:
        case short_enumeration_k:
        case short_real_k:
        case long_enumeration_k:
        case long_bitset_k:
        case long_integer_k:
        case long_unsigned_k:
        case long_subrange_k:
        case long_real_k:
        case fixed_array_k:
        case fixed_string_zero_k:
        case routine_ptr_k:
        case void_k:
            return false;

        case named_k:
            return open_type ((type_t *) tp->type_structure.named.resolution);

        case open_array_k:
        case open_record_k:
        case open_string_zero_k:
            return true;

        case pointer_k:
            if (! tp->type_structure.pointer.visited) {
                tp->type_structure.pointer.visited = true;
                tp->type_structure.pointer.open    = open_type (tp->type_structure.pointer.pointee);
            }
            tp->type_structure.pointer.visited = false;
            return tp->type_structure.pointer.open;


        case record_k:
            /* only non-variant records can be open */
            if (tp->type_structure.record.variant != NULL)
                return false;
            /* only the last field can be open so only check it */
            for (fp = tp->type_structure.record.fields;
                 fp->next_field;
                 fp = fp->next_field)
                continue;
            return open_type(fp->type);

        case user_marshalled_k:
            return 
                open_type (tp->type_structure.user_marshalled.xmit_type);

        default:
            unimp_type_exit (tp->kind, "open_type");
            return 0;
        }
    /*lint -unreachable*/
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static boolean array_kind(type_p)
type_t * type_p;
#else
static boolean array_kind(type_t *type_p)
#endif
{                
    if (type_p->kind == named_k)
        type_p = type_p->type_structure.named.resolution;

    switch (type_p->kind) {
        case open_array_k:
        case fixed_array_k:
        case open_string_zero_k:
        case fixed_string_zero_k:
            return true;
        default:
            return false;
    }
    /*lint -unreachable*/
}

/*--------------------------------------------------------------------*/
              
#ifndef __STDC__
static boolean pointer_kind(type_p)
type_t * type_p;
#else 
static boolean pointer_kind(type_t *type_p)
#endif
{                
    if (type_p->kind == named_k)
        type_p = type_p->type_structure.named.resolution;

    return (type_p->kind == pointer_k);
}


/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_last_is (type_p)
type_t * type_p;
#else 
static  void check_last_is (type_t *type_p)
#endif
{
    binding_t * last_is_binding;
    type_kind last_is_kind, kind;


    if (type_p->last_is == NAMETABLE_NIL_ID)
        return;

    kind = type_p->kind;
    if (kind  == named_k)
        kind = type_p->type_structure.named.resolution->kind;
    switch (kind) {
        case open_array_k:
        case fixed_array_k:
            break;
        default:
            log_error (type_p->source_pos, "%s\n",
                       "[last_is()] attribute can only be applied to array types");
            return;
    }

    last_is_binding = (binding_t *) NAMETABLE_lookup_local (type_p->last_is);

     /* 
      *    Ensure that the variable really exists.
      */
    if (last_is_binding == NULL) {
        log_error (type_p->source_pos, "%s\n", "last_is variable does not exist");
        return;
    }

     /* 
      *    Make sure that it is bound to a type definer.
      */
    if (last_is_binding->kind != type_k) {
        log_error (type_p->source_pos, "%s\n", "last_is variable must be a scalar type");
        return;
    }

     /* 
      *    Make sure that it is a scalar type (excluding reals)
      */
    last_is_kind = last_is_binding->binding->type.kind;
    if (last_is_kind == named_k)
        last_is_kind = last_is_binding->binding->type.type_structure.named.resolution->kind;

    if (last_is_kind != small_integer_k &&
        last_is_kind != short_integer_k &&
        last_is_kind != long_integer_k &&
        last_is_kind != hyper_integer_k &&
        last_is_kind != small_unsigned_k &&
        last_is_kind != short_unsigned_k &&
        last_is_kind != long_unsigned_k &&
        last_is_kind != hyper_unsigned_k &&
        last_is_kind != short_enumeration_k &&
        last_is_kind != long_enumeration_k &&
        last_is_kind != small_subrange_k &&
        last_is_kind != short_subrange_k &&
        last_is_kind != long_subrange_k) {
        log_error(type_p->source_pos,
                  "%s\n",
                  "last_is must be a scalar\n");
    }

}

/*--------------------------------------------------------------------*/
                                    
#ifndef __STDC__
static  void check_max_is (type_p)
type_t * type_p;
#else
static  void check_max_is (type_t *type_p)
#endif
{
    binding_t * max_is_binding;
    type_kind max_is_kind, kind;

    if (type_p->max_is == NAMETABLE_NIL_ID)
        return;

    kind = type_p->kind;
    if (kind  == named_k)
        kind = type_p->type_structure.named.resolution->kind;
    switch (kind) {
        case open_array_k:
            break;
        default:
            log_error (type_p->source_pos, "%s\n",
                       "[max_is()] attribute can only be applied to open array types");
            return;
    }

    max_is_binding = (binding_t *) NAMETABLE_lookup_local (type_p->max_is);

     /* 
      *    Ensure that the variable really exists.
      */
    if (max_is_binding == NULL) {
        log_error (type_p->source_pos, "%s\n", "max_is variable does not exist");
        return;
    }

     /* 
      *    Make sure that it is bound to a type definer.
      */
    if (max_is_binding->kind != type_k) {
        log_error (type_p->source_pos, "%s\n", "max_is variable must be have scalar type");
        return;
    }

     /* 
      *    Make sure that it is a scalar type (excluding reals)
      */
    max_is_kind = max_is_binding->binding->type.kind;
    if (max_is_kind == named_k)
        max_is_kind = max_is_binding->binding->type.type_structure.named.resolution->kind;

    if (max_is_kind != small_integer_k &&
        max_is_kind != short_integer_k &&
        max_is_kind != long_integer_k &&
        max_is_kind != hyper_integer_k &&
        max_is_kind != small_unsigned_k &&
        max_is_kind != short_unsigned_k &&
        max_is_kind != long_unsigned_k &&
        max_is_kind != hyper_unsigned_k &&
        max_is_kind != short_enumeration_k &&
        max_is_kind != long_enumeration_k &&
        max_is_kind != small_subrange_k &&
        max_is_kind != short_subrange_k &&
        max_is_kind != long_subrange_k) {
        log_error(type_p->source_pos,
                  "%s\n",
                  "max_is must be a scalar\n");
        return;
    }

}

/*--------------------------------------------------------------------*/
#if 0
/* Also found in frontend.c */
#ifndef __STDC__
static  void file_not_found (file_name)
char   *file_name;
#else
static  void file_not_found (char *file_name)
#endif
{
    char    message[max_string_len];
    sprintf (message, "File: %s not found", file_name);
    error (message);
}
#endif

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  import_t * check_import (import_node_ptr, directory_list, def_strings)
import_t * import_node_ptr;
char ** directory_list;
char ** def_strings;
#else
static  import_t * check_import (import_t *import_node_ptr, char **directory_list, char **def_strings)
#endif
{
    binding_t * binding_node_p;
    extern  FILE * yyin;
    char    old_file[max_string_len];
    interface_attributes old_attrs;
  
    /*
     *  Parse the imported interface
     */

    yynerrs = 0;
    source_pos = import_node_ptr->source_pos;
    old_attrs = interface_attrs;
    inq_name_for_errors(old_file);
    language_style = pascal_style;
    binding_node_p = (binding_t *) parse (import_node_ptr->file_name, file_not_found, directory_list, def_strings);
    if (yynerrs > 0)
        exit(0);
    if (error_count > 0) {
        (void) print_errors(binding_node_p->binding->interface.source_file);
        exit(0);
    }
 
     /* 
      *  If the imported interface parsed w/o errors,
      *  run the semantic checks on it.
      */

    import_node_ptr->name_of_interface = binding_node_p->name;
    record_import (import_node_ptr->file_name, import_node_ptr->name_of_interface, directory_list);
    CHECKER_check_interface (binding_node_p, directory_list, def_strings);
    set_name_for_errors (old_file);
    interface_attrs = old_attrs;
    return binding_node_p->binding->interface.imports;
}
/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  import_t * glue_imports (import_list_a, import_list_b)
import_t * import_list_a,
*import_list_b;
#else
static  import_t * glue_imports (import_t *import_list_a, import_t *import_list_b)
#endif
{
    import_t * ip;

     /* 
      *  Walk down import_list_a to find the end. 
      */

    for (ip = import_list_a; ip->next_import; ip = ip->next_import)
        continue;
    ip->next_import = import_list_b;

    import_list_a->last_import = import_list_b->last_import;
    return import_list_a;
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_imported_interfaces (import_list, directory_list, def_strings)
import_t * import_list;
char ** directory_list;
char ** def_strings;
#else
static  void check_imported_interfaces (import_t *import_list, char **directory_list, char **def_strings)
#endif
{
    import_t * ip;
    import_t * nested_imports;

    NAMETABLE_id_t if_name;

     /* 
      *    Iterate through the import list, queuing nested imports
      *    on the front, so they won't be checked twice.
      */

    for (ip = import_list; ip; ip = ip->next_import) {
        if (already_imported (ip->file_name, &if_name, directory_list)) {
            ip->name_of_interface = if_name;
            continue;
        }
        if ((nested_imports = check_import (ip, directory_list, def_strings)) != NULL)
            import_list = glue_imports (nested_imports, import_list);
    }

}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static boolean is_enum_type(type_knd)
type_kind type_knd;
#else
static boolean is_enum_type(type_kind type_knd)
#endif
{
    switch (type_knd) {
        case small_enumeration_k:
        case short_enumeration_k:
        case long_enumeration_k: return true;

        default: return false;
    }
    /*lint -unreachable*/
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static boolean is_integer_type(type_knd)
type_kind type_knd;
#else
static boolean is_integer_type(type_kind type_knd)
#endif
{
    switch (type_knd) {
        case small_integer_k:
        case short_integer_k:
        case long_integer_k:
        case small_unsigned_k:
        case short_unsigned_k:
        case long_unsigned_k: return true;

        default: return false;
    }
    /*lint -unreachable*/
}
        
/*--------------------------------------------------------------------*/

#ifndef __STDC__
static void check_tag(disc_type_p, const_node_p)
type_t * disc_type_p;
constant_t * const_node_p;
#else
static void check_tag(type_t *disc_type_p, constant_t *const_node_p)
#endif

{                  
    type_kind discriminator_kind;

    if (disc_type_p-> kind == named_k)
        disc_type_p = disc_type_p->type_structure.named.resolution;

    discriminator_kind = disc_type_p->kind;

    switch (const_node_p->kind) {
        case boolean_const_k: 
             if (discriminator_kind != boolean_k)
                 log_error (const_node_p->source_pos,"%s\n", "tag type does not agree with discriminator type");
             break;
         case integer_k:
             if (! is_integer_type(discriminator_kind))
                 log_error (const_node_p->source_pos,"%s\n", "tag type does not agree with discriminator type");
             break;
         case enum_k:
             if (! is_enum_type(discriminator_kind))
                 log_error (const_node_p->source_pos,"%s\n", "tag type does not agree with discriminator type");
             break;
        case named_const_k:
            check_tag(disc_type_p, const_node_p->value.named_val.resolution);
            break;
        case real_k:
        case string_k:
            log_error(const_node_p->source_pos, "%s\n", "Invalid tag type");
            break;
    }
}
/*--------------------------------------------------------------------*/
      
#ifndef __STDC__
static boolean contains_open_array(type_p)
type_t * type_p;
#else
static boolean contains_open_array(type_t *type_p)
#endif
{            
    field_t * fp;
    component_t * cp;
    variant_t * vp;

    if (type_p->kind == named_k)
        type_p = type_p-> type_structure.named.resolution;
    if (type_p->kind == open_array_k)
        return true;
    if (type_p->kind == record_k) {
        for (fp = type_p->type_structure.record.fields; fp; fp = fp->next_field)
            if (contains_open_array(fp->type))
                return true;
        if ((vp = type_p->type_structure.record.variant) != NULL)
            for (cp = vp->components;cp; cp = cp->next_component)
                for (fp = cp->fields; fp; fp=fp->next_field) 
                    if (contains_open_array(fp->type))
                        return true;
    }

    return false;
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_variant_part (variant_p, user_marshalled) 
variant_t * variant_p;
boolean     user_marshalled;
#else
static  void check_variant_part (variant_t *variant_p, boolean user_marshalled) 
#endif
{
    component_t * cp;
    tag_t       * tp;
    field_t     * fp;

    for (cp = variant_p->components; cp; cp = cp->next_component) {
        for (tp = cp->tags; tp; tp = tp->next_tag) 
            check_tag(variant_p->tag_type, &tp->tag_value);
        for (fp = cp->fields; fp; fp = fp->next_field) {
            if (contains_open_array(fp->type)) {
                log_error(variant_p->source_pos,
                         "%s\n",
                         "open arrays illegal in unions or variant records");
                continue;
            }
            check_field(fp, user_marshalled);
        }
    }
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_pointer (type_p, user_marshalled)
type_t * type_p;
boolean user_marshalled;
#else
static  void check_pointer (type_t *type_p, boolean user_marshalled)
#endif
{
    switch (type_p->type_structure.pointer.pointee->kind) {
    case pointer_k: 
        if (! user_marshalled && !(interface_attrs & INTERFACE_LOCAL_ONLY))
           log_error (type_p->source_pos, "%s\n", "Only top level pointers are allowed in NIDL");
        break;

    case named_k: 
        if (type_p->type_structure.pointer.pointee->type_structure.named.resolution->kind == pointer_k) {
            if (! user_marshalled && !(interface_attrs & INTERFACE_LOCAL_ONLY))
                log_error (type_p->source_pos, "%s\n", "Only top level pointers are allowed in NIDL");
            return;
        };
        break;
    case small_enumeration_k: 
    case short_enumeration_k: 
    case long_enumeration_k: 
        if (! user_marshalled) {
            log_error (type_p->source_pos,
            "%s\n",
            "Enumeration is invalid in this context");
        }
        break;
    }
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
boolean check_index (index_node_p, scope, user_marshalled)
type_t * index_node_p;
scope_t scope;
boolean user_marshalled;
#else
boolean check_index (type_t *index_node_p, scope_t scope, boolean user_marshalled)
#endif
{
    type_t * ip;

    ip = index_node_p;
    if (ip->kind == named_k)
        ip = ip->type_structure.named.resolution;

    switch (ip->kind) {
    case small_subrange_k: 
    case short_subrange_k: 
    case long_subrange_k: 
        check_type (ip, scope, user_marshalled);
        return true;

    case small_enumeration_k: 
    case short_enumeration_k: 
    case long_enumeration_k: 
        check_type (ip, scope, user_marshalled);
        return true;

    default: 
        return false;

    }
    /*lint -unreachable*/
}
/*--------------------------------------------------------------------*/

#ifndef __STDC__
static void adjust_index(index_type_p)
type_t * index_type_p;
#else
static void adjust_index(type_t *index_type_p)
#endif
{

    constant_t  *cp;
    constant_t  *new_upper_bound_p;
    if (index_type_p->kind == named_k)
        index_type_p = index_type_p->type_structure.named.resolution;
    if (index_type_p->kind == small_subrange_k ||
        index_type_p->kind == short_subrange_k ||
        index_type_p->kind == long_subrange_k)
    {
        cp = index_type_p->type_structure.subrange.upper_bound;
        if (cp->kind == named_const_k)
            cp = cp->value.named_val.resolution;
        new_upper_bound_p  = AST_constant_node(integer_k);
        *new_upper_bound_p = *cp;
        -- new_upper_bound_p->value.int_val;
        index_type_p->type_structure.subrange.upper_bound = new_upper_bound_p;
    }

}
/*--------------------------------------------------------------------*/

#ifndef __STDC__
void check_indices (indexp, scope)
array_index_t *indexp;
scope_t scope;
#else
void check_indices (array_index_t *indexp, scope_t scope)
#endif
{
    for (; indexp; indexp = indexp->next) {
        if (!check_index (indexp->type, scope, indexp->adjust)) {
            log_error(indexp->type->source_pos,
                      "%s\n",
                      "Invalid array index type");
            continue;
        }
        if (indexp->adjust) {
            adjust_index(indexp->type);
            indexp->adjust = false;
        }
    }
}
/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_open_array (type_p, scope, user_marshalled)
type_t * type_p;
scope_t  scope;
boolean user_marshalled;
#else
static  void check_open_array (type_t *type_p, scope_t scope, boolean user_marshalled)
#endif

{
    open_array_t * open_array_p;

    open_array_p = &type_p->type_structure.open_array;
    if (open_array_p->elements->kind == small_enumeration_k ||
        open_array_p->elements->kind == short_enumeration_k ||
        open_array_p->elements->kind == long_enumeration_k) {
        log_error (type_p->source_pos,
        "%s\n",
        "Enumeration is invalid in this context");
        return;
    }
    if (contains_open_array(open_array_p->elements))
    {
        log_error (type_p->source_pos,
        "%s\n",
        "Array elements cannot be open arrays");
        return;
    }

    if (! user_marshalled && !(interface_attrs & INTERFACE_LOCAL_ONLY))

        if (pointer_kind(open_array_p->elements)) {
            log_error(type_p->source_pos,
                      "%s\n",
                      "Array of pointers requires a transmit_as");
            return;
        }

    check_indices (open_array_p->indices, scope);
    check_type (open_array_p->elements, scope, user_marshalled);
}

/*--------------------------------------------------------------------*/
    
#ifndef __STDC__
static  void check_fixed_array (type_p, scope, user_marshalled)
type_t * type_p;
scope_t scope;
boolean user_marshalled;
#else
static  void check_fixed_array (type_t *type_p, scope_t scope, boolean user_marshalled)
#endif
{
    fixed_array_t * fixed_array_p;

    fixed_array_p = &type_p->type_structure.fixed_array;
    if (fixed_array_p->elements->kind == small_enumeration_k ||
        fixed_array_p->elements->kind == short_enumeration_k ||
        fixed_array_p->elements->kind == long_enumeration_k) {
        log_error (type_p->source_pos,
        "%s\n",
        "Enumeration is invalid in this context");
        return;
    }
    if (contains_open_array(fixed_array_p->elements))
    {
        log_error (type_p->source_pos,
        "%s\n",
        "Array elements cannot be open arrays");
         return;
    }
    
    if (! user_marshalled && !(interface_attrs & INTERFACE_LOCAL_ONLY))
        if (pointer_kind(fixed_array_p->elements)) {
            log_error(type_p->source_pos,
                      "%s\n",
                      "Array of pointers requires a transmit_as");
            return;
        }
    check_indices (fixed_array_p->indices, scope);
    check_type (fixed_array_p->elements, scope, user_marshalled);
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void back_patch_field (field_ptr, record_ptr)
field_t * field_ptr;
record_t * record_ptr;
#else
static  void back_patch_field (field_t *field_ptr, record_t *record_ptr)
#endif

{
    field_t * fp;

    for (fp = record_ptr->fields; fp; fp = fp->next_field)
        if (field_ptr->type->last_is == fp->name) {
            field_ptr->last_is_ref = fp;
            break;         
        }

    for (fp = record_ptr->fields; fp; fp = fp->next_field)
        if (field_ptr->type->max_is == fp->name) {
            field_ptr->max_is_ref = fp;
            break;
        }
}
/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_field (field_p, has_xmit_type)
field_t * field_p;
boolean has_xmit_type;
#else
static  void check_field (field_t *field_p, boolean has_xmit_type)
#endif
{
    type_t * tp;

    tp = field_p->type;
    if (tp->kind == named_k)
        tp = tp->type_structure.named.resolution;
     
        /*
         * Check for field being a pointer
         */

    if (tp->kind == pointer_k && !(interface_attrs & INTERFACE_LOCAL_ONLY)) {
        if (has_xmit_type || tp->xmit_type != NULL)
            return;
        else
        {
            log_error (field_p->source_pos,
            "%s\n",
            "Records containing pointers require a transmit_as type");
            return;
        }
    }           
        /*
         *  Check for field being open array 
         */

    else if (open_type(tp)) {
        if (field_p->next_field != NULL) {
            log_error(field_p->source_pos,
                      "%s\n",
                      "Open field must be last in record");
            return;
        }
        else if (tp->kind != record_k) {
            if (!has_xmit_type && tp->last_is == NAMETABLE_NIL_ID) {
                log_error(field_p->source_pos,
                          "%s\n",
                          "last_is required");
                return;
            }
        }
    }
    check_type (tp, record, has_xmit_type);
}


/*--------------------------------------------------------------------*/
#if 0
/* Not currently referenced */
#ifndef __STDC__
static void check_field_attributes(fp)
field_t * fp;
#else
static void check_field_attributes(field_t *fp)
#endif

{
    if (fp->last_is_ref != NULL)
        if (! array_kind(fp->type)) 
            log_error(fp->source_pos,
            "%s",
            "last_is must be applied to an array\n");
    if (fp->max_is_ref != NULL)
        if (! array_kind(fp->type)) 
            log_error(fp->source_pos,
            "%s",
            "max_is must be applied to an array\n");
}
#endif

/*--------------------------------------------------------------------*/
      
#ifndef __STDC__
static  void check_record (type_p, user_marshalled)
type_t * type_p;
boolean user_marshalled;
#else
static  void check_record (type_t *type_p, boolean user_marshalled)
#endif
{
    record_t * record_p;
    field_t * fp;

    record_p = &type_p->type_structure.record;

    NAMETABLE_push_level ();
     /* 
      *  Make one pass over the record creating field name-->field type bindings.
      */
    for (fp = record_p->fields; fp; fp = fp->next_field)
        (void) AST_binding_node (fp->name, type_k, (structure_t *) fp->type);

     /* 
      *  Now type check each field.
      */
    for (fp = record_p->fields; fp; fp = fp->next_field) {
        check_field (fp, user_marshalled);
        if (fp->type->last_is != NAMETABLE_NIL_ID) 
            back_patch_field (fp, record_p);
        if (fp->type->max_is != NAMETABLE_NIL_ID) 
            back_patch_field (fp, record_p);
        /*
        if (fp->last_is_ref != NULL || fp->max_is_ref != NULL)
            check_field_attributes(fp);
        */

    }

    /*
     * Type check the variant if there is one.
     */

    if (record_p->variant != NULL)
        check_variant_part(record_p->variant, user_marshalled);

    NAMETABLE_pop_level ();
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_bitset (type_p)
type_t * type_p;
#else
static  void check_bitset (type_t *type_p)
#endif

{
    type_t * base_type;

    base_type = type_p->type_structure.bitset.base_type;
    if (base_type->kind == named_k) 
        base_type = base_type->type_structure.named.resolution;

    if (base_type->kind != short_enumeration_k &&
        base_type->kind != long_enumeration_k) {
        log_error (type_p->source_pos, "%s\n", "Sets can only be over enumerations");
        return;
    }

    if (type_p->kind == short_bitset_k)
        if (base_type->type_structure.enumeration.cardinality > ENUMERATORS_PER_SHORT)
            if (type_p->type_structure.bitset.widenable)
                type_p->kind = long_bitset_k;
            else
                log_error (type_p->source_pos, "%s\n", "Short bitsets can only have 16 elements");
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_subrange (type_node_ptr)
type_t * type_node_ptr;
#else
static  void check_subrange (type_t *type_node_ptr)
#endif
{
    constant_t * lower_const_p,
               * upper_const_p;

    long    lower_val,
            upper_val,
            size;

     /* 
      *  Make sure the names referred to in the subrange constants
      *  either enum constants or numeric constants.
      */

    lower_const_p = type_node_ptr->type_structure.subrange.lower_bound;
    upper_const_p = type_node_ptr->type_structure.subrange.upper_bound;

    if (lower_const_p->kind == named_const_k)
        lower_const_p = lower_const_p->value.named_val.resolution;
    if (upper_const_p->kind == named_const_k)
        upper_const_p = upper_const_p->value.named_val.resolution;


    if (lower_const_p->kind != integer_k && lower_const_p->kind != enum_k) {
        log_error (type_node_ptr->source_pos, "Subranges must be integer or enumeration\n", (char *) NULL);
        return;
    }

    if (upper_const_p->kind != integer_k && upper_const_p->kind != enum_k) {
        log_error (type_node_ptr->source_pos, "Subranges must be integer or enumeration\n", (char *) NULL);
        return;
    }



    if (lower_const_p->kind == integer_k)
        lower_val = lower_const_p->value.int_val;
    else
        lower_val = lower_const_p->value.enum_val.ordinal_mapping;

    if (upper_const_p->kind == integer_k)
        upper_val = upper_const_p->value.int_val;
    else
        upper_val = upper_const_p->value.enum_val.ordinal_mapping;

    if (lower_val > upper_val) {
        log_error (type_node_ptr->source_pos, "Lower bound must be less than upper bound\n", (char *) NULL);
        return;
    }

    size = upper_val - lower_val;
    if (size <= MAX_SHORT)
        type_node_ptr->kind = short_subrange_k;
    else
        type_node_ptr->kind = long_subrange_k;
}


/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_type (type_p, scope, user_marshalled)
type_t * type_p;
scope_t scope;
boolean user_marshalled;
#else
static  void check_type (type_t *type_p, scope_t scope, boolean user_marshalled)
#endif
{

    check_last_is (type_p);
    check_max_is (type_p);

    switch (type_p->kind) {

    case user_marshalled_k:
        check_type(type_p->type_structure.user_marshalled.xmit_type, scope, false);
        check_type(type_p->type_structure.user_marshalled.user_type, scope, true);
        break;

    case fixed_string_zero_k:
        check_indices (type_p->type_structure.fixed_string_zero.index, scope);
        break;

    case fixed_array_k: 
        check_fixed_array (type_p, scope, user_marshalled);
        break;

    case open_array_k: 
        check_open_array (type_p, scope, user_marshalled);
        break;

    case record_k: 
        check_record (type_p, user_marshalled);
        break;

    case pointer_k: 
        check_pointer (type_p, user_marshalled);
        break;

    case short_bitset_k: 
    case long_bitset_k: 
        check_bitset (type_p);
        break;

    case short_subrange_k: 
    case long_subrange_k: 
        check_subrange (type_p);
        break;

    case reference_k:
        if (scope == record || scope == top_level)
            log_error(type_p->source_pos, 
                      "%s\n",
                      "reference types can only be used in parameters");
        break;

    case drep_k:
        if (!(interface_attrs & INTERFACE_LOCAL_ONLY))
            log_error(type_p->source_pos, 
                      "%s\n",
                      "rpc_$drep_t type can only be used in local interfaces");
        break;
    }

}
/*--------------------------------------------------------------------*/
       
#ifndef __STDC__
static void make_into_reference (parameter_p)
parameter_t * parameter_p;
#else
static void make_into_reference (parameter_t *parameter_p)
#endif

{
    parameter_p->ref = true;
    parameter_p->type = parameter_p->type->type_structure.reference.pointee;
}

/*--------------------------------------------------------------------*/
      
#ifndef __STDC__
static  void back_patch_parameter (parameter_ptr, routine_node_ptr)
parameter_t * parameter_ptr;
routine_t * routine_node_ptr;
#else
static  void back_patch_parameter (parameter_t *parameter_ptr, routine_t *routine_node_ptr)
#endif
{
    parameter_t * pp;
    type_t      * tp;

    /* 
      * Scan the parameter list looking for a matching name
      */

    for (pp = routine_node_ptr->parameters; pp; pp = pp->next_param) {
        if (pp->name == parameter_ptr->type->last_is) {
            parameter_ptr->last_is_ref = pp;
        }
        if (pp->name == parameter_ptr->type->max_is) {
            parameter_ptr->max_is_ref = pp;
        }
    }

    for (pp = routine_node_ptr->parameters; pp; pp = pp->next_param) {
        if (pp->name == parameter_ptr->type->last_is) {
            tp = parameter_ptr->type;
            if (tp->kind == named_k)
                tp = tp->type_structure.named.resolution;
            if (!(interface_attrs & INTERFACE_LOCAL_ONLY) && (tp->kind == open_array_k) && (parameter_ptr->out) &&
                 (pp->out) && (! pp->in) && (parameter_ptr->max_is_ref == NULL))
            {
                log_error(pp->source_pos,
                          "%s\n",
                          "last_is must be input parameter");
                return;
            }
        }
        if (pp->name == parameter_ptr->type->max_is) {
            if (!(interface_attrs & INTERFACE_LOCAL_ONLY) && !(pp->in))
                log_error(pp->source_pos,
                          "%s\n",
                          "max_is must be an input parameter");
        }
    }

}
/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  boolean requires_last_is (param_p)
parameter_t * param_p;
#else
static  boolean requires_last_is (parameter_t *param_p)
#endif
{
    type_t * type_p;
    type_p = param_p->type;

    if (interface_attrs & INTERFACE_LOCAL_ONLY)
        return false;
    if (param_p->type->kind == user_marshalled_k)
        return false;
    if (type_p->kind == named_k)
        type_p = type_p->type_structure.named.resolution;
    if (type_p->kind == open_array_k)
        return true;
    return false;

}

/*--------------------------------------------------------------------*/
#ifndef __STDC__
static  boolean requires_max_is (param_p)
parameter_t * param_p;
#else
static  boolean requires_max_is (parameter_t *param_p)
#endif
{
    type_t * type_p;

    if (interface_attrs & INTERFACE_LOCAL_ONLY)
        return false;
    type_p = param_p->type;
    if (type_p->kind == user_marshalled_k)
        return false;
    if (type_p->kind == named_k)
        type_p = type_p->type_structure.named.resolution;
    if (! param_p->out)
        return false;
    if (type_p->kind == open_array_k)
        return true;
    return false;
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static void check_handle(type_p, src_pos) 
type_t * type_p;
int src_pos;
#else
static void check_handle(type_t *type_p, int src_pos) 
#endif
{
    if (type_p->kind == reference_k)
        type_p = type_p->type_structure.reference.pointee;
    if (type_p->kind == pointer_k)
        type_p = type_p->type_structure.pointer.pointee;

    if (!(type_p->is_handle) && (type_p->kind != handle_k))
            log_error (src_pos,"%s\n", "type must be a handle");
    
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static void check_param_type(param_node_p)
parameter_t *   param_node_p;
#else
static void check_param_type(parameter_t *param_node_p)
#endif

{
    type_t  * type_node_p;
    field_t * fp;
    component_t * cp;
    variant_t * vp;

    type_node_p = param_node_p->type;
    if (type_node_p->kind == named_k)
        type_node_p = type_node_p->type_structure.named.resolution;
    switch (type_node_p->kind) {
        case pointer_k:
            if ((param_node_p->out) &&
                (!param_node_p->in) &&
                !(interface_attrs & INTERFACE_LOCAL_ONLY)
                &&(param_node_p->was_pointer))
            {
                log_error(param_node_p->source_pos,
                          "%s\n",
                          "[out] pointers not allowed\n");
                return;
            }
            break;

        case univ_ptr_k:
            if (!(interface_attrs & INTERFACE_LOCAL_ONLY)) {
                log_error(param_node_p->source_pos,
                          "%s\n",
                          "This type requires the [transmit_as()] attribute\n");
                return;
            }

        case record_k:
            for (fp = type_node_p->type_structure.record.fields;fp; fp = fp->next_field) {
                if (open_type(fp->type)) {
                    if (param_node_p->out && (!param_node_p->in))
                        log_error(param_node_p->source_pos,
                                  "%s\n",
                                  "Records/structs containing open arrays must be [in] or [in, out]\n");
                    if (!(param_node_p->ref || param_node_p->was_pointer || param_node_p->out))
                        log_error(param_node_p->source_pos,
                                  "%s\n",
                                  "Records/structs containing open arrays must be passed by reference\n");
                }
            }
            if ((vp = type_node_p->type_structure.record.variant) != NULL)
                for (cp = vp->components; cp; cp = cp->next_component)
                    for (fp = cp->fields; fp; fp = fp->next_field)
                        if (open_type(fp->type))
                            log_error(param_node_p->source_pos,
                                      "%s\n",
                                      "variant records/structs containing open fields are not allowed\n");
            break;
    }
}


/*--------------------------------------------------------------------*/
#if 0
/* Not currently referenced */
#ifndef __STDC__
static void check_parameter_attributes(param_node_p) 
parameter_t * param_node_p;
#else
static void check_parameter_attributes(parameter_t *param_node_p) 
#endif
{
    if (param_node_p->last_is_ref != NULL) 
        if (! array_kind(param_node_p->type))
            log_error(param_node_p->source_pos,
                      "%s",
                      "last_is must be applied to an array\n");

    if (param_node_p->max_is_ref != NULL) 
        if (! array_kind(param_node_p->type))
            log_error(param_node_p->source_pos,
                      "%s",
                      "max_is must be applied to an array\n");

}
#endif

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static boolean contains_handle(type_p)
type_t * type_p;
#else
static boolean contains_handle(type_t *type_p)
#endif
{            
    field_t * fp;
    component_t * cp;
    variant_t * vp;
               
    if (type_p->kind == handle_k)
        return true;
    if (type_p->kind == named_k)
        return contains_handle(type_p = type_p->type_structure.named.resolution);
    if (type_p->kind == open_array_k)
        return (contains_handle(type_p->type_structure.open_array.elements));
    if (type_p->kind == fixed_array_k)
        return (contains_handle(type_p->type_structure.fixed_array.elements));
    if (type_p->kind == record_k) {
        for (fp = type_p->type_structure.record.fields;fp; fp = fp->next_field)
            if (contains_handle(fp->type))
                return true;
        if ((vp = type_p->type_structure.record.variant) != NULL)
            for (cp = vp->components;cp; cp = cp->next_component)
                for (fp = cp->fields;fp;fp=fp->next_field) 
                    if (contains_handle(fp->type))
                        return true;
    }
    return false;
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_routine (routine_binding_p)
binding_t * routine_binding_p;
#else
static  void check_routine (binding_t *routine_binding_p)
#endif
{
    routine_t * routine_p;
    parameter_t * pp;
    type_t * param_type_p;
    boolean first;
    int comm_status_count = 0;

    routine_p = &routine_binding_p->binding->routine;

     /* 
      *  Push a level and walk the parameter list,
      *  type checking each parameter in turn.
      */

    NAMETABLE_push_level ();

     /* 
      *  Make one pass binding parameter names --> parameter types.
      */
    for (pp = routine_p->parameters; pp; pp = pp->next_param) 
        (void) AST_binding_node (pp->name, type_k, (structure_t *) pp->type);
                                                    
    if ((!(interface_attrs & INTERFACE_IMPLICITLY_BOUND)) &&
        (!(interface_attrs & INTERFACE_LOCAL_ONLY))) {
        /*
         *  Ensure that there is at least one parameter.
         */
        if (routine_p->parameters == NULL) {
            log_error(routine_p->source_pos,
                      "%s\n",
                      "Handle parameter required");
            return;
        }
        /*
         *  Validate the handle
         */
        check_handle(routine_p->parameters->type, routine_p->parameters->source_pos);
        if (!routine_p->parameters->in) {
            log_error(routine_p->parameters->source_pos,
                      "%s\n",
                      "Handle parameter must be an IN parameter");
        }
    }

    /* 
     *  Make another  pass to check the parameter types.
     */
    for (pp = routine_p->parameters, first=true; pp; pp = pp->next_param, first=false) {
        if (pp->out && routine_p->maybe)
            log_error(pp->source_pos,
                      "%s\n",
                      "MAYBE routines cannot have output parameters");
        param_type_p = pp->type;
        check_type (param_type_p, parameter_list, param_type_p->xmit_type != NULL);
        if (param_type_p->last_is != NAMETABLE_NIL_ID)
            back_patch_parameter (pp, routine_p);
        if (param_type_p->max_is != NAMETABLE_NIL_ID)
            back_patch_parameter (pp, routine_p);

        if (param_type_p->kind == reference_k)
            make_into_reference(pp);

        check_param_type(pp);
        
        if (pp->requires_pointer)
            if (!(pointer_kind(pp->type) || pp->was_pointer || array_kind(pp->type)))
                log_error(pp->source_pos, "%s\n", "Out parameters must be passed by reference.");
            
        if (requires_last_is (pp)) 
            if (pp->type->last_is == NAMETABLE_NIL_ID)
                log_error (pp->source_pos, "%s\n", "This type requires a last_is attribute");
        if (requires_max_is (pp))
            if (pp->type->max_is == NAMETABLE_NIL_ID) 
                if (pp->type->last_is == NAMETABLE_NIL_ID)
                    log_error (pp->source_pos, "%s\n", "This type requires a max_is attribute");
        if ((! first ) && (pp->type->xmit_type == NULL))
            if (contains_handle(pp->type))
                log_error(pp->source_pos, "%s\n", "Handle_t legal only in first parameter position.");

        /* check [comm_status] parameter attribute correctness */
        if (pp->comm_status) {
            if (comm_status_count++) 
                log_error (pp->source_pos, "%s\n", "Too many [comm_status] parameters");
            if (!pp->out)
                log_error (pp->source_pos, "%s\n", "[comm_status] parameter must be an out");
            if (param_type_p->kind != named_k)
                log_error (pp->source_pos, "%s\n", "[comm_status] parameter must be of type status_$t");
            else {
                char *str;
                NAMETABLE_id_to_string (param_type_p->type_structure.named.name, &str);
                if (strcmp (str, "status_$t") != 0)
                    log_error (pp->source_pos, "%s\n", "[comm_status] parameter must be of type status_$t");
            }
        }
    }

    NAMETABLE_pop_level ();


     /* 
      *  Check the result type, if non-nil.
      */

    if (routine_p->result)
        check_type (routine_p->result->type, parameter_list,routine_p->result->type->xmit_type != NULL);


}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void check_export (binding_p)
binding_t * binding_p;
#else
static  void check_export (binding_t *binding_p)
#endif
{
     /* 
      *  Only routines and types need to be checked for
      *  well-formedness, as constants only need have
      *  names resolved which has already been done.
      */

    switch (binding_p->kind) {
    case routine_k: 
        check_routine (binding_p);
        break;

    case type_k: 
        check_type (&binding_p->binding->type, top_level, false);
        break;
    }
}

/*--------------------------------------------------------------------*/
                            
#ifndef __STDC__
static  void resolve_constant (const_p)
constant_t * const_p;
#else 
static  void resolve_constant (constant_t *const_p)
#endif
{
    binding_t * bp;
    char   *const_name;

     /* 
      * If this is not a named constant, then just return.
      */

    if (const_p->kind != named_const_k)
        return;

     /* 
      *  Lookup the name to see if it is 
      *  defined and bound to the correct
      *  type of node.
      */

    bp = (binding_t *) NAMETABLE_lookup_binding (const_p->value.named_val.name);

    if (bp == NULL) {
        NAMETABLE_id_to_string (const_p->value.named_val.name, &const_name);
        log_error (const_p->source_pos, "Constant name: %s not found\n", const_name);
        return;
    }

     /* 
      *  Name must be bound to a constant node.
      */
    if (bp->kind != constant_k) {
        NAMETABLE_id_to_string (const_p->value.named_val.name, &const_name);
        log_error (const_p->source_pos, "Not a constant: %s\n", const_name);
        return;
    }

     /* 
      *  If name is bound to a named constant,
      *  rescursively resolve the new name.
      */
    if (bp->binding->constant.kind == named_const_k) {
        resolve_constant (&bp->binding->constant);
        const_p->value.named_val.resolution = bp->binding->constant.value.named_val.resolution;
    }
    else
        const_p->value.named_val.resolution = &bp->binding->constant;
}

/*--------------------------------------------------------------------*/
      
#ifndef __STDC__
static type_t *resolve_type_name(type_id, src_pos, is_handle, xmit_as)
NAMETABLE_id_t  type_id;
int             src_pos;
boolean         *is_handle;
NAMETABLE_id_t  *xmit_as;
#else
static type_t *resolve_type_name(NAMETABLE_id_t type_id, int src_pos, boolean *is_handle, NAMETABLE_id_t *xmit_as)
#endif
{   
    binding_t       *referent_p;
    char            *type_name;

    referent_p = (binding_t *) NAMETABLE_lookup_binding (type_id);

     /* 
      *  Ensure that the name is really bound to something.
      */
    if (referent_p == NULL)
        return NULL;

     /* 
      *  The name must be bound to a type definer.
      */
    if (referent_p->kind != type_k) {
        NAMETABLE_id_to_string(type_id, &type_name);
        log_error(src_pos, "Name: %s is not a type.\n", type_name);
    }

     /* 
      *  If the name is bound to another named type,
      *  recursively resolve its type.
      */                                       
    if (*xmit_as == NAMETABLE_NIL_ID)
        *xmit_as = referent_p->binding->type.xmit_type_name;
    *is_handle  |= referent_p->binding->type.is_handle;

    if (referent_p->binding->type.kind == named_k) 
        return resolve_type_name (referent_p->binding->type.type_structure.named.name,
                                  source_pos,
                                  is_handle,
                                  xmit_as);
    else
        return &referent_p->binding->type;

}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void resolve_type_names (type_p)
type_t * type_p;
#else
static  void resolve_type_names (type_t *type_p)
#endif
{
    char   *type_name;
    structure_t * consed_type_node_p;
    structure_t * pointer_node_p;

    /*
     * If this type is a named type, then try to find its resolution.
     * resolution needs to be done.
     */
                            
    if (type_p->kind == named_k) {
        type_p->type_structure.named.resolution = 
                  resolve_type_name (type_p-> type_structure.named.name,
                                     type_p-> source_pos,
                                     &type_p->is_handle,
                                     &type_p->xmit_type_name);

        if (type_p->type_structure.named.resolution == NULL)
        {
            NAMETABLE_id_to_string (type_p->type_structure.named.name, &type_name);
            if ( ! (interface_attrs & INTERFACE_LOCAL_ONLY)) {
                log_error (type_p->source_pos, "Type name: %s not found.\n", type_name);
            } 
            else {
                consed_type_node_p = AST_type_node (void_k);
                pointer_node_p = AST_pointer_node (&consed_type_node_p->type);
                type_p->type_structure.named.resolution = &pointer_node_p->type;
                log_warning (type_p->source_pos,
                "(Warning) Type name: %s not_found.\nType void* assumed.\n", type_name);
            }
        }
    }
}
        
/*--------------------------------------------------------------------*/
                                         
#ifndef __STDC__
static void resolve_tags(tag_p)
tag_t * tag_p;
#else
static void resolve_tags(tag_t *tag_p)
#endif
{
    tag_t * tp;

    for (tp = tag_p; tp; tp = tp->next_tag)
        resolve_constant(&tp->tag_value);
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void resolve_fields (field_p)
field_t * field_p;
#else
static  void resolve_fields (field_t *field_p)
#endif
{
    field_t * fp;

    for (fp = field_p; fp; fp = fp->next_field)
        resolve_type (fp->type);
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void resolve_components (components_p)
component_t * components_p;
#else
static  void resolve_components (component_t *components_p)
#endif
{
    component_t * cp;
             
    for (cp = components_p; cp; cp = cp->next_component) {
        resolve_tags(cp->tags);
        resolve_fields (cp->fields);
    }
}

/*--------------------------------------------------------------------*/
                                              
#ifndef __STDC__
static  void resolve_indices (index_p)
array_index_t * index_p;
#else
static  void resolve_indices (array_index_t *index_p)
#endif
{
    for (; index_p; index_p = index_p->next )
        resolve_type (index_p->type);
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void resolve_type (type_p)
type_t * type_p;
#else
static  void resolve_type (type_t *type_p)
#endif

{
    variant_t *vp;

    resolve_type_names(type_p);

    switch (type_p->kind) {
 
    case user_marshalled_k:
        resolve_type ( type_p->type_structure.user_marshalled.xmit_type); 
        resolve_type ( type_p->type_structure.user_marshalled.user_type); 
        break;

    case record_k: 
        resolve_fields (type_p->type_structure.record.fields);
        if ((vp = type_p->type_structure.record.variant) != NULL) {
            resolve_type (vp->tag_type);
            resolve_components (vp->components);
            }
        break;

    case pointer_k: 
        resolve_type (type_p->type_structure.pointer.pointee);
        break;
  
    case reference_k: 
        resolve_type (type_p->type_structure.reference.pointee);
        break;

    case fixed_array_k: 
        resolve_indices (type_p->type_structure.fixed_array.indices);
        resolve_type (type_p->type_structure.fixed_array.elements);
        break;

    case open_array_k: 
        resolve_constant (type_p->type_structure.open_array.lower_bound);
        resolve_indices (type_p->type_structure.open_array.indices);
        resolve_type (type_p->type_structure.open_array.elements);
        break;
    case short_bitset_k: 
    case long_bitset_k: 
        resolve_type (type_p->type_structure.bitset.base_type);
        break;

    case short_subrange_k: 
    case long_subrange_k: 
        resolve_constant (type_p->type_structure.subrange.lower_bound);
        resolve_constant (type_p->type_structure.subrange.upper_bound);
        break;
    case routine_ptr_k: 
        resolve_parameter_types (&type_p->type_structure.routine_ptr);
        break;

    case fixed_string_zero_k:
        resolve_indices(type_p->type_structure.fixed_string_zero.index);
        break;

    }
}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void resolve_parameter_types (routine_p)
routine_t * routine_p;
#else
static  void resolve_parameter_types (routine_t *routine_p)
#endif
{
    parameter_t * pp;

    for (pp = routine_p->parameters; pp; pp = pp->next_param)
        resolve_type (pp->type);

    if (routine_p->result)
        resolve_type (routine_p->result->type);
}


/*--------------------------------------------------------------------*/

#ifndef __STDC__
static  void resolve_named_things (interface_node_p)
interface_t * interface_node_p;
#else
static  void resolve_named_things (interface_t *interface_node_p)
#endif
{
    binding_t * bp;
    constant_t * cp;
    type_t * tp;
    routine_t * rp;                  

    if (interface_node_p->implicit_handle_type != NULL)
        resolve_type(interface_node_p->implicit_handle_type);

    for (bp = interface_node_p->exports; bp; bp = bp->next) {
        if (bp->kind == constant_k) {
            cp = &bp->binding->constant;
            resolve_constant (cp);
        }

        if (bp->kind == type_k) {
            tp = &bp->binding->type;
            resolve_type (tp);
        }

        if (bp->kind == routine_k) {
            rp = &bp->binding->routine;
            resolve_parameter_types (rp);
        }

    }
}
/*--------------------------------------------------------------------*/
#ifndef __STDC__
static boolean compare_uuids(u1, u2)
uuid_$t u1,u2;
#else
static boolean compare_uuids(uuid_$t u1, uuid_$t u2)
#endif
{   
    int i;          

    if ( (u1.time_high !=  u2.time_high)  ||
        (u1.time_low   !=  u2.time_low)  ||
        (u1.family     !=  u2.family))
        return false;

    for (i = 0; i < 7; i++)
        if (u1.host[i] != u2.host[i])
            return false;
    return true;
}
/*--------------------------------------------------------------------*/

#ifndef __STDC__
void prepend_rpc_import(interface_node_p)
interface_t * interface_node_p;
#else
void prepend_rpc_import(interface_t *interface_node_p)
#endif
{
    import_t * import_node_p;

    import_node_p = AST_import_node(rpc_fn);

    if (interface_node_p->imports != NULL) {
        import_node_p->next_import = interface_node_p->imports;
        import_node_p->last_import = interface_node_p->imports->last_import;
    }
    interface_node_p->imports = import_node_p;
}


/*--------------------------------------------------------------------*/
                                
void CHECKER_check_interface (binding_node_p, import_dirs, def_strings)
binding_t * binding_node_p;
char ** import_dirs;
char ** def_strings;
{
    interface_t * interface_node_p;
    binding_t * bp;
    int     old_error_count;
    boolean has_ops;
    char * if_name;
             

    NAMETABLE_id_to_string (binding_node_p->name, &if_name);
    interface_node_p = &binding_node_p->binding->interface;

    if (error_count > 0) {
        (void) print_errors (interface_node_p->source_file);
        return;
    }

    interface_attrs = INTERFACE_NULL_ATTRS;
    if (interface_node_p->implicit_handle_var != NAMETABLE_NIL_ID)
        interface_attrs |= INTERFACE_IMPLICITLY_BOUND;
    if (interface_node_p->auto_binding)
        interface_attrs |= INTERFACE_AUTOMATICALLY_BOUND;
    if (interface_node_p->local_only)
        interface_attrs |= INTERFACE_LOCAL_ONLY;

     /* 
      *  Check each imported interface.
      */

    interface_node_p->op_count = count_ops(interface_node_p->exports);
    has_ops = interface_node_p->op_count > 0;
    if (! has_ops) {
        interface_node_p->local_only = true;
        interface_attrs |= INTERFACE_LOCAL_ONLY;
    }

    /*
     * If the interface has operations, then we prepend an
     * import for rpc.imp.idl, so they don't have to 
     * explicitly import it.  If it has no operations,
     * then we treat it as if it were completely local.
     */

    if (has_ops && !interface_node_p->local_only)
        prepend_rpc_import(interface_node_p);


    if (interface_node_p->imports)
        check_imported_interfaces (interface_node_p->imports, import_dirs, def_strings);


     /* 
      *  Resolve any named constants or types.  Then walk the
      *  AST validity checking each exported item.
      */

    old_error_count = error_count;
    resolve_named_things (interface_node_p);

     /* 
      *  If no errors (all names resolved), 
      *  then check all exports for well-formedness.
      */
    if (old_error_count == error_count) {

        if (interface_node_p->implicit_handle_type != NULL)
            check_handle(interface_node_p->implicit_handle_type, source_pos);

        for (bp = interface_node_p->exports; bp; bp = bp->next)
            check_export (bp);
             
            
        if (!interface_node_p->local_only)
            if (compare_uuids(interface_node_p->interface_uuid, null_uuid))
                log_error (interface_node_p->source_pos, "Interface uid must be specified\n", (char *)NULL);
    }

     /* 
      *  Print accumulated errors
      */

    (void) print_errors (interface_node_p->source_file);


}

/*--------------------------------------------------------------------*/
#ifndef __STDC__
void CHECKER_init()
#else
void CHECKER_init(void)
#endif
{
    rpc_fn = STRTAB_add_string(RPC_IMPORT_FN);
}
