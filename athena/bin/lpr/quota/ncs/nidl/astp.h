/********************************************************************/
/*                                                                  */
/*                        A S T P . H                               */
/*                                                                  */
/*              IDL compiler AST building routines                  */
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

#ifndef astp_incl
#define astp_incl

#ifdef DSEE
#include "$(base.idl).h"
#else
#include "base.h"
#endif

#include "ast.h"

#ifndef __PROTOTYPE
#ifdef __STDC__
#define __PROTOTYPE(x) x
#else
#define __PROTOTYPE(x) ()
#endif
#endif

/*
 *  A declarator node is used hold the information
 *  about a IDL/C declarator which will eventually
 *  be merged with the type node and turned into a
 *  binding.  A declarator node never actually appears
 *  in the AST which is why it is not declared in ast.h
 */

typedef struct dt
{
    NAMETABLE_id_t id;
    type_kind      declarator_kind;
    array_index_t* indices;
    type_t*        type_node_ptr;
    parameter_t*   routine_params;
    struct dt*     last_declarator;
    struct dt*     next_declarator;

} declarator_t;

typedef enum {c_style, pascal_style} style_t;
extern style_t language_style;

/*
 *  Port defintion
 */

typedef struct {
    NAMETABLE_id_t  family_name;
    int             port_number;
} port_t;


/*
 *  Type attribute node.
 *
 *  A Type attribute node is used to hold the
 *  the attributes for a type during the parse.
 */
                
typedef enum {xmit_name, last_is_name, max_is_name, is_handle} attr_t;

typedef struct type_attr_t {
    struct type_attr_t * next_attr;
    struct type_attr_t * last_attr;
    int             source_line;
    attr_t          which;
    union {
      NAMETABLE_id_t  xmit_type_name;
      NAMETABLE_id_t  last_is;
      NAMETABLE_id_t  max_is;
    } the_attr;
} type_attr_t;


/*
 *  Interface Attributes
 */
 
extern boolean         interface_auto_bound;
extern uuid_$t         interface_uuid;
extern int             interface_version;
extern NAMETABLE_id_t  interface_implicit_handle_var;
extern type_t          *interface_implicit_handle_type;
extern boolean         interface_is_local;

extern port_t          interface_ports[];
extern int             interface_port_count;

/*
 *  Routine attributes
 */

extern boolean  routine_secure;
extern boolean  routine_idempotent;
extern boolean  routine_maybe;
extern boolean  routine_broadcast;
 
/*
 *  Parameter Attributes
 */
 
extern boolean         comm_status_parameter;
extern boolean         in_parameter;
extern boolean         out_parameter;
extern boolean         ref_parameter;
extern boolean         val_parameter;
extern boolean         requires_pointer;
extern NAMETABLE_id_t  last_is_of_field;
extern NAMETABLE_id_t  max_is_of_field;

/*
 * state of C integer type modifiers
 */

typedef enum { small_int, short_int, long_int, hyper_int } c_int_size_t;

extern c_int_size_t int_size;
extern boolean int_signed;
 
/*
 *  Interface just parsed
 */
 
extern binding_t*      the_interface;

/*
 *  Builtin in constants
 */
 
extern constant_t*     zero_constant_p;
extern uuid_$t         null_uuid;

/*
 * Builtin names
 */

extern NAMETABLE_id_t   c_style_id;
extern NAMETABLE_id_t   pascal_style_id;

/*
 * error count
 */

extern int error_count;

type_attr_t*    AST_attribute_node __PROTOTYPE((attr_t kind));

binding_t*      AST_binding_node __PROTOTYPE((NAMETABLE_id_t id, structure_kind kind, structure_t *binding));
                    
constant_t *    AST_boolean_constant  __PROTOTYPE((boolean value));

component_t*    AST_component_node __PROTOTYPE((NAMETABLE_id_t label, tag_t *field_tags, field_t *fields));

tag_t*          AST_component_tag_node __PROTOTYPE((constant_t *constant_node_ptr));
                                                 
component_t*    AST_concat_components __PROTOTYPE((component_t *component_list, component_t *component));

declarator_t*   AST_concat_declarators __PROTOTYPE((declarator_t* declarator_list, declarator_t *declarator)); 

binding_t*      AST_concat_exports __PROTOTYPE((binding_t *export_list, binding_t *new_export));
           
field_t*        AST_concat_field_nodes __PROTOTYPE((field_t *field_node_list, field_t *field_node));

import_t*       AST_concat_imports __PROTOTYPE((import_t *import_list, import_t *new_import));

array_index_t*  AST_concat_indices __PROTOTYPE((array_index_t *index1, array_index_t *index2));

parameter_t*    AST_concat_parameters __PROTOTYPE((parameter_t *param_list, parameter_t *new_param));

tag_t*          AST_concat_tags __PROTOTYPE((tag_t *tag_list, tag_t *tag));

type_attr_t*    AST_concat_type_attrs __PROTOTYPE((type_attr_t* attr_list, type_attr_t* attr));

constant_t*     AST_constant_node __PROTOTYPE((constant_kind kind));

declarator_t*   AST_declarator_node __PROTOTYPE((NAMETABLE_id_t id, type_kind kind, array_index_t *indices)); 
                     
binding_t*      AST_declarators_to_bindings __PROTOTYPE((declarator_t *declarators_ptr));

field_t*        AST_declarators_to_fields __PROTOTYPE((declarator_t *declarators_ptr));

parameter_t*    AST_declarators_to_params __PROTOTYPE((declarator_t *declarators_ptr));

void            AST_depointerize_params __PROTOTYPE((parameter_t *param_list));
 
binding_t*      AST_enum_constant_binding __PROTOTYPE((NAMETABLE_id_t id));

structure_t*    AST_enumerator_node __PROTOTYPE((binding_t *enum_constants, type_kind enumeration_kind));
                                                              
field_t*        AST_field_node __PROTOTYPE((NAMETABLE_id_t field_name));

structure_t*    AST_fixed_array_node __PROTOTYPE((array_index_t *index, type_t *elements));

void            AST_fix_declarator_type __PROTOTYPE((declarator_t *declarator_ptr));

constant_t*     AST_floating_constant __PROTOTYPE((double value));

import_t*       AST_import_node __PROTOTYPE((STRTAB_str_t imported_file));

array_index_t*  AST_index_node __PROTOTYPE((type_t *type, boolean adjust));

array_index_t*  AST_index_node_c __PROTOTYPE((constant_t *size));

void            AST_init __PROTOTYPE((void));
    

constant_t*     AST_integer_constant __PROTOTYPE((long int value));

structure_t*    AST_integer_type_node __PROTOTYPE((void));

structure_t*    AST_interface_node __PROTOTYPE((import_t *import_list, binding_t *export_list));
           
constant_t*     AST_named_const __PROTOTYPE((NAMETABLE_id_t const_name));

structure_t*    AST_named_type __PROTOTYPE((NAMETABLE_id_t type_name));

parameter_t*    AST_parameter_node __PROTOTYPE((NAMETABLE_id_t identifier));

structure_t*    AST_pointer_node __PROTOTYPE((type_t *pointee));

field_t*        AST_propagate_field_type __PROTOTYPE((field_t *field_nodes, type_t *type));

parameter_t*    AST_propagate_param_attrs __PROTOTYPE((parameter_t *param_nodes, type_t *type_node_ptr));

parameter_t*    AST_propagate_param_type __PROTOTYPE((parameter_t *param_nodes, type_t *type_node_ptr));

declarator_t*   AST_prop_declarator_type __PROTOTYPE((type_t *type_node_ptr, declarator_t *declarators_ptr));

structure_t*    AST_record_node __PROTOTYPE((field_t *fields, variant_t *variant_ptr));

structure_t*    AST_routine_node __PROTOTYPE((parameter_t *parameters, NAMETABLE_id_t result_name, type_t *result_type));
                                     
structure_t*    AST_routine_ptr_node __PROTOTYPE((parameter_t *params, type_t *result));

routine_t * AST_set_routine_attrs __PROTOTYPE((routine_t *routine_node_ptr));
    
void            AST_set_interface_attributes __PROTOTYPE((interface_t *interface_node));
   
structure_t*    AST_bitset_node __PROTOTYPE((type_t *base_type, boolean widenable, boolean is_short));
   
structure_t*    AST_set_type_attrs __PROTOTYPE((structure_t *type_node_ptr, type_attr_t * attr_list_ptr));

void            AST_set_well_known_ports __PROTOTYPE((interface_t *interface_node_p));

constant_t*     AST_string_constant __PROTOTYPE((STRTAB_str_t value));

structure_t*    AST_fixed_string_zero_node __PROTOTYPE((constant_t *upper_bound));

structure_t*    AST_subrange_node __PROTOTYPE((constant_t *lower_bound, constant_t *upper_bound));
             
tag_t*          AST_tag_node __PROTOTYPE((constant_t *tag_value));
 
structure_t*    AST_type_node __PROTOTYPE((type_kind kind));

variant_t*      AST_variant_node __PROTOTYPE((NAMETABLE_id_t label, NAMETABLE_id_t tag_id, type_t *tag_type, component_t *components));

structure_t*    AST_open_array_node __PROTOTYPE((constant_t *lower_bound, array_index_t * indices, type_t *elements));

NAMETABLE_id_t constant_to_id  __PROTOTYPE((constant_t *cp));

#endif

