/********************************************************************/
/*                                                                  */
/*                      A S T P . C                                 */
/*                                                                  */     
/*        Abstract Syntax tree builder for IDL Compiler             */
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

#include <stdio.h>
#include "idl_base.h"
#include "utils.h"
#include "nametbl.h"
#include "errors.h"
#include "ast.h"
#include "astp.h"
#include "sysdep.h"

#ifdef DSEE
#include "$(base.idl).h"
#include "$(socket.idl).h"
#else
#include "base.h"
#include "socket.h"
#endif

extern int  yylineno;

/*
 *  Interface Attributes
 */

boolean interface_auto_bound = false;
uuid_$t interface_uuid ;

int     interface_version = 0;
NAMETABLE_id_t interface_implicit_handle_var = NAMETABLE_NIL_ID;
type_t  *interface_implicit_handle_type = NULL;
boolean interface_is_local = false;

/*
 *  Well known ports for interface
 */

port_t  interface_ports[socket_$num_families];
int     interface_port_count = 0 ;

/*
 *  Family names
 */

NAMETABLE_id_t unix_family_name_id;
NAMETABLE_id_t ip_family_name_id;
NAMETABLE_id_t implink_family_name_id;
NAMETABLE_id_t pup_family_name_id;
NAMETABLE_id_t chaos_family_name_id;
NAMETABLE_id_t ns_family_name_id;
NAMETABLE_id_t nbs_family_name_id;
NAMETABLE_id_t ecma_family_name_id;
NAMETABLE_id_t datakit_family_name_id;
NAMETABLE_id_t ccitt_family_name_id;
NAMETABLE_id_t sna_family_name_id;
NAMETABLE_id_t dds_family_name_id;


/*
 *  Routine attributes
 */

boolean routine_secure = false;
boolean routine_idempotent = false;
boolean routine_maybe = false;
boolean routine_broadcast = false ;

/*
 *  Parameter Attributes
 */

boolean comm_status_parameter       = false;
boolean in_parameter                = false;
boolean out_parameter               = false;
boolean ref_parameter               = false;
boolean val_parameter               = false;
boolean requires_pointer            = false;
NAMETABLE_id_t last_is_of_field = NAMETABLE_NIL_ID;
NAMETABLE_id_t max_is_of_field  = NAMETABLE_NIL_ID;

/*
 *
 */
style_t language_style = pascal_style;

/*
 *
 */

c_int_size_t int_size = long_int;
boolean int_signed    = true;

/*
 *  Interface just parsed
 */

binding_t * the_interface = 0;

/*
 *  Builtin in constants
 */

constant_t * zero_constant_p;
uuid_$t      null_uuid;
/*
 *  Builtin names
 */

NAMETABLE_id_t c_style_id;
NAMETABLE_id_t pascal_style_id;

/*
 *  Forward declarations
 */

void AST_dump_interface  __PROTOTYPE((interface_t *));
void AST_dump_type  __PROTOTYPE((type_t *, char *, int));
void AST_dump_constant  __PROTOTYPE((constant_t *));
void AST_dump_export_list  __PROTOTYPE((binding_t *));
void AST_dump_exported_item  __PROTOTYPE((binding_t *));
void AST_dump_routine __PROTOTYPE((routine_t *, int));
void AST_dump_fixed_array  __PROTOTYPE((fixed_array_t *, int));
void AST_dump_open_array  __PROTOTYPE((open_array_t *, int));
void AST_dump_record  __PROTOTYPE((field_t *, int));
void AST_dump_variant_part  __PROTOTYPE((variant_t *, int));
void AST_dump_subrange  __PROTOTYPE((subrange_t *, int));
void AST_dump_enumerators  __PROTOTYPE((enumeration_t *, int));
void AST_dump_parameter  __PROTOTYPE((parameter_t *, int));
void AST_dump_component  __PROTOTYPE((component_t *, int));
void AST_dump_indices __PROTOTYPE((array_index_t *, int));

extern void abort __PROTOTYPE((void));
#define LARGEST_SHORT_ENUM 65536
/*---------------------------------------------------------------------*/

/*
 *                                                                     
 *  A S T _ a t t r i b u t e _ n o d e
 *  ===================================
 */

type_attr_t * AST_attribute_node(kind)
attr_t kind ;
{
    type_attr_t * attr_node_p ;
    attr_node_p = (type_attr_t *) alloc(sizeof(type_attr_t));
    attr_node_p->which = kind ;
    attr_node_p->next_attr = NULL;
    attr_node_p->last_attr = NULL;
    attr_node_p->source_line = yylineno ;

    return attr_node_p ;
}

/*---------------------------------------------------------------------*/

/*                                     
 *  A S T _ c o n c a t _ a t t r s 
 *  ===============================
 */

type_attr_t * AST_concat_type_attrs (attr_list, attr)
type_attr_t * attr_list;
type_attr_t * attr;
{
    if (attr_list->last_attr == NULL) {
        attr_list->last_attr = (struct type_attr_t *) attr;
        attr_list->next_attr = (struct type_attr_t *) attr;
        return attr_list;
    }

    attr_list->last_attr->next_attr = (struct type_attr_t *) attr;
    attr_list->last_attr = (struct type_attr_t *) attr;

    return attr_list;

}

/*---------------------------------------------------------------------*/

/*
 *                                                                     
 *  A S T _ b i n d i n g _ n o d e                                    
 *  ===============================                                    
 */

binding_t * AST_binding_node (id, kind, binding)
NAMETABLE_id_t id;
structure_kind kind;
structure_t * binding;
{
    binding_t * binding_node_ptr;
    char   *identifier;

    binding_node_ptr = (binding_t *) alloc (sizeof (binding_t));

    binding_node_ptr->source_pos = yylineno;
    binding_node_ptr->kind = kind;
    binding_node_ptr->name = id;
    binding_node_ptr->binding = binding;
    binding_node_ptr->next = NULL;
    binding_node_ptr->last = NULL;

    if (!NAMETABLE_add_binding (id, binding_node_ptr)) {
        NAMETABLE_id_to_string (id, &identifier);
        log_error (yylineno, "Name already declared: %s\n", identifier);
    }

    return binding_node_ptr;

}

/*---------------------------------------------------------------------*/

/*
 *                                                                     
 *  A S T _ b o o l e a n _ c o n s t a n t
 *  =======================================
 */
                                           
constant_t * AST_boolean_constant(value)
boolean value ;
{
    constant_t  * constant_node_p ;

    constant_node_p = AST_constant_node(boolean_const_k) ;
    constant_node_p->value.boolean_value = value ;

    return constant_node_p;
}

/*---------------------------------------------------------------------*/
 
/*
 *                                                                     
 *  A S T _ c o m p o n e n t _ n o d e                                
 *  ===================================                                
 */                                                                     



component_t * AST_component_node (label, field_tags, fields)
NAMETABLE_id_t label;
tag_t * field_tags;
field_t * fields;
{
    component_t * component_node_ptr;

    component_node_ptr = (component_t *) alloc (sizeof (component_t));

    component_node_ptr->source_pos = yylineno;
    component_node_ptr->label =
        (label != NAMETABLE_NIL_ID)
        ? label
        : NAMETABLE_add_derived_name (
              constant_to_id (&(field_tags->tag_value)),
              "case_%s");
    component_node_ptr->tags = field_tags;
    component_node_ptr->fields = fields;
    component_node_ptr->next_component = NULL;
    component_node_ptr->last_component = NULL;

    return component_node_ptr;
}


NAMETABLE_id_t constant_to_id (cp)
constant_t *cp;
{
    char       *str;
    char       exp[max_string_len];

    switch (cp->kind) {
        case named_const_k:
            return cp->value.named_val.name;

        case integer_k:
            sprintf (exp, "%ld", cp->value.int_val);
            return NAMETABLE_add_id (exp, false);

        case real_k:
            sprintf (exp, "%f", cp->value.real_val);
            return NAMETABLE_add_id (exp, false);

        case string_k:
            STRTAB_str_to_string (cp->value.string_val, &str);
            sprintf (exp, "\"%s\"", str);
            return NAMETABLE_add_id (exp, false);

        case boolean_const_k:
            return NAMETABLE_add_id (
                cp->value.boolean_value ? "true" : "false",
                false);
        }
    /* XXX */
    fprintf(stderr, "unexpected tag in constant_to_id!!\n");
    abort();
    return 0;
}


/*---------------------------------------------------------------------*/

/*
 *  A S T _ c o n s t a n t _ t a g _ n o d e
 *  =========================================
 */

tag_t * AST_component_tag_node (constant_node_ptr)
constant_t * constant_node_ptr;
{
    tag_t * tag_node_ptr;

    tag_node_ptr = (tag_t *) alloc (sizeof (tag_t));

    tag_node_ptr->source_pos = yylineno;
    tag_node_ptr->tag_value = *constant_node_ptr;
    tag_node_ptr->next_tag = NULL;
    tag_node_ptr->last_tag = NULL;

    return tag_node_ptr;
}

/*---------------------------------------------------------------------*/

/*                                     
 *  A S T _ c o n c a t _ c o m p o n e n t s
 *  =========================================
 */

component_t * AST_concat_components (component_list, component)
component_t * component_list;
component_t * component;
{
    if (component_list->last_component == NULL) {
        component_list->last_component = component;
        component_list->next_component = component;
        return component_list;
    }

    component_list->last_component->next_component = component;
    component_list->last_component = component;

    return component_list;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ c o n c a t _ d e c l a r a t o r s
 *  ============================================
 */

declarator_t * AST_concat_declarators (declarator_list, declarator)
declarator_t * declarator_list,
*declarator;
{
    if (declarator_list->last_declarator == NULL) {
        declarator_list->last_declarator = declarator;
        declarator_list->next_declarator = declarator;
        return declarator_list;
    }

    declarator_list->last_declarator->next_declarator = declarator;
    declarator_list->last_declarator = declarator;

    return declarator_list;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ c o n c a t _ e x p o r t s
 *  ==================================
 */

binding_t * AST_concat_exports (export_list, new_export)
binding_t * export_list,
*new_export;
{
    if (export_list == NULL) {
        export_list = new_export;
        return export_list;
    }

    if (export_list->last == NULL) {
        if (new_export->last == NULL) {
            export_list->last = new_export;
            export_list->next = new_export;
            return export_list;
        }

        export_list->last = new_export->last;
        export_list->next = new_export;
        return export_list;
    }

    if ((export_list->last != NULL) &&
        (new_export->last != NULL)) {
        export_list->last->next = new_export;
        export_list->last = new_export->last;
        return export_list;
    }

    export_list->last->next = new_export;
    export_list->last = new_export;

    return export_list;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ c o n c a t _ f i e l d _ n o d e s
 *  ===========================================
 */

field_t * AST_concat_field_nodes (field_node_list, field_node)
field_t * field_node_list,
*field_node;
{
    if (field_node_list == NULL) {
        field_node_list = field_node;
        return field_node_list;
    }

    if (field_node_list->last_field == NULL) {
        if (field_node->last_field == NULL) {
            field_node_list->last_field = field_node;
            field_node_list->next_field = field_node;
            return field_node_list;
        }

        field_node_list->last_field = field_node->last_field;
        field_node_list->next_field = field_node;
        return field_node_list;
    }

    if ((field_node_list->last_field != NULL) &&
        (field_node->last_field != NULL)) {
        field_node_list->last_field->next_field = field_node;
        field_node_list->last_field = field_node->last_field;
        return field_node_list;
    }

    field_node_list->last_field->next_field = field_node;
    field_node_list->last_field = field_node;

    return field_node_list;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ c o n c a t _ i m p o r t s
 *  ===================================
 */

import_t * AST_concat_imports (import_list, new_import)
import_t * import_list,
*new_import;
{
    struct import_t *ip;
    if (import_list->last_import == NULL) {
        import_list->last_import = (struct import_t  *) new_import;
        import_list->next_import = (struct import_t  *) new_import;
        return import_list;
    }

    ip = import_list->last_import;
    ip->next_import = (struct import_t   *) new_import;
    import_list->last_import = (struct import_t  *) new_import;

    return import_list;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ c o n c a t _ i n d i c e s
 *  ===================================
 */

array_index_t *AST_concat_indices (indices1, indices2)
array_index_t *indices1,
              *indices2;
{
  indices1->last->next = indices2;
  indices1->last         = indices2->last;

    return indices1;
}

/*---------------------------------------------------------------------*/

/*
 *   A S T _ c o n c a t _ p a r a m e t e r s
 *   =========================================
 */

parameter_t * AST_concat_parameters (param_list, new_param)
parameter_t * param_list,
*new_param;
{
    if (param_list->last_param == NULL) {
        if (new_param->last_param == NULL) {
            param_list->last_param = new_param;
            param_list->next_param = new_param;
            return param_list;
        }

        param_list->last_param = new_param->last_param;
        param_list->next_param = new_param;
        return param_list;
    }

    if ((param_list->last_param != NULL) &&
        (new_param->last_param != NULL)) {
        param_list->last_param->next_param = new_param;
        param_list->last_param = new_param->last_param;
        return param_list;
    }

    param_list->last_param->next_param = new_param;
    param_list->last_param = new_param;

    return param_list;

}

/*---------------------------------------------------------------------*/

/*
 *   A S T _ c o n c a t _ t a g s
 *   =============================
 */

tag_t * AST_concat_tags (tag_list, tag)
tag_t * tag_list,
*tag;
{
    if (tag_list->last_tag == NULL) {
        tag_list->last_tag = tag;
        tag_list->next_tag = tag;
        return tag_list;
    }

    tag_list->last_tag->next_tag = tag;
    tag_list->last_tag = tag;

    return tag_list;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ c o n s t a n t _ n o d e
 *  =================================
 */

constant_t * AST_constant_node (kind)
constant_kind kind;
{
    constant_t * constant_node_p;

    constant_node_p = (constant_t *) alloc (sizeof (constant_t));
    constant_node_p->source_pos = yylineno;
    constant_node_p->kind = kind;

    return constant_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ d e c l a r a t o r _ n o d e
 *  =====================================
 */

declarator_t * AST_declarator_node (id, kind, indices)
NAMETABLE_id_t id;
type_kind kind;
array_index_t *indices;
{
    declarator_t * declarator_node_ptr;

    declarator_node_ptr = (declarator_t *) alloc (sizeof (declarator_t));

    declarator_node_ptr->id = id;
    declarator_node_ptr->declarator_kind = kind;
    declarator_node_ptr->indices = indices;
    declarator_node_ptr->type_node_ptr = NULL;
    declarator_node_ptr->last_declarator = NULL;
    declarator_node_ptr->next_declarator = NULL;

    return declarator_node_ptr;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ d e c l a r a t o r s _ t o _ b i n d i n g s
 *  ====================================================
 */

binding_t * AST_declarators_to_bindings (declarators_ptr)
declarator_t * declarators_ptr;
{
    binding_t * binding_list;
    binding_t * new_binding;
    declarator_t * dp;

    binding_list = AST_binding_node (declarators_ptr->id,
                                     type_k,
                                     (structure_t *) declarators_ptr->type_node_ptr);

    for (dp = declarators_ptr->next_declarator; dp; dp = dp->next_declarator) {
        new_binding = AST_binding_node (dp->id,
                                        type_k,
                                        (structure_t *) dp->type_node_ptr);
        binding_list = AST_concat_exports (binding_list, new_binding);
    }

    return binding_list;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ d e c l a r a t o r s _ t o _ f i e l d s 
 *  ===============================================
 */

field_t * AST_declarators_to_fields (declarators_ptr)
declarator_t * declarators_ptr;
{
    field_t * field_list;
    field_t * new_field;
    declarator_t * dp;

    field_list = AST_field_node (declarators_ptr->id);
    field_list->type = (struct type_t *) declarators_ptr->type_node_ptr;

    for (dp = declarators_ptr->next_declarator; dp; dp = dp->next_declarator) {
        new_field = AST_field_node (dp->id);
        new_field->type = (struct type_t *) dp->type_node_ptr;
        field_list = AST_concat_field_nodes (field_list, new_field);
    }

    return field_list;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ d e c l a r a t o r s _ t o _ p a r a m s
 *  ===============================================
 */

parameter_t * AST_declarators_to_params (declarators_ptr)
declarator_t * declarators_ptr;
{
    parameter_t * parameter_list;
    parameter_t * new_parameter;
    declarator_t * dp;

    parameter_list = AST_parameter_node (declarators_ptr->id);
    parameter_list->type = (struct type_t *) declarators_ptr->type_node_ptr;

    for (dp = declarators_ptr->next_declarator; dp; dp = dp->next_declarator) {
        new_parameter = AST_parameter_node (dp->id);
        new_parameter->type = (struct type_t *) dp->type_node_ptr;
        parameter_list = AST_concat_parameters (parameter_list, new_parameter);
    }

    return parameter_list;
}

/*
 *  A S T _ d e p o i n t e r i z e _ p a r a m s 
 *  =============================================
 */

void AST_depointerize_params (param_list)
parameter_t * param_list;
{
    parameter_t * pp;

    for (pp = param_list; pp; pp = pp->next_param) {
        if (pp->out)
            pp->requires_pointer = true ;
        if (pp->out && pp->type->kind == pointer_k) {
            pp->type = pp->type->type_structure.pointer.pointee;
            pp->was_pointer = true ;
        }
    }
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ e n u m _ c o n s t a n t _ b i n d i n g 
 *  ================================================
 */

binding_t * AST_enum_constant_binding (id)
NAMETABLE_id_t id;
{
    constant_t * constant_node_p;
    binding_t * constant_binding_p;

    constant_node_p = AST_constant_node (enum_k);

    constant_node_p->value.enum_val.base_type = NULL;
    constant_node_p->value.enum_val.ordinal_mapping = 0;

    constant_binding_p = AST_binding_node (id,
                                           constant_k,
                                           (structure_t *)constant_node_p);

    return constant_binding_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ e n u m e r a t o r _ n o d e
 *  =====================================
 */

structure_t * AST_enumerator_node (enum_constants, enumeration_kind)
binding_t * enum_constants;
type_kind enumeration_kind;
{
    structure_t * enum_node_p;
    binding_t * ebp;
    long        ord_val;
    constant_t * cp;

    enum_node_p = AST_type_node (enumeration_kind);
    ord_val = 0;

    for (ebp = enum_constants; ebp; ebp = ebp->next) {
        cp = (constant_t *) & ebp->binding->constant;
        cp->value.enum_val.ordinal_mapping = ord_val;
        cp->value.enum_val.base_type = &enum_node_p->type;
        ++ord_val;
    }

    if (ord_val > LARGEST_SHORT_ENUM)
        enum_node_p->type.kind = long_enumeration_k;

    enum_node_p->type.type_structure.enumeration.masks_genned = false;
    enum_node_p->type.type_structure.enumeration.enum_constants = enum_constants;
    enum_node_p->type.type_structure.enumeration.cardinality = --ord_val;

    return enum_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ f i e l d _ n o d e 
 *  ===========================
 */

field_t * AST_field_node (field_name)
NAMETABLE_id_t field_name;
{
    field_t * field_node_ptr;

    field_node_ptr = (field_t *) alloc (sizeof (field_t));

    field_node_ptr->source_pos = yylineno;
    field_node_ptr->name = field_name;
    field_node_ptr->next_field = NULL;
    field_node_ptr->last_field = NULL;
    field_node_ptr->type = NULL;

    field_node_ptr->temp_helpers_p = NULL;

    return field_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ f i x e d _ a r r a y _ n o d e 
 *  ========================================
 */

structure_t * AST_fixed_array_node (indices, elements)
array_index_t * indices;
type_t * elements;
{
    structure_t * array_node_ptr;

    array_node_ptr = AST_type_node (fixed_array_k);

    array_node_ptr->type.type_structure.fixed_array.indices = (struct array_index_t *) indices;
    array_node_ptr->type.type_structure.fixed_array.elements = (struct type_t *) elements;
    return array_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ f i x _ d e c l a r a t o r _ t y p e
 *  =============================================
 */

void AST_fix_declarator_type (declarator_ptr)
declarator_t * declarator_ptr;
{
    type_kind declarator_kind;
    structure_t * type_node_ptr;
    structure_t * wtp ;
    NAMETABLE_id_t xmit_name;
    NAMETABLE_id_t last_is_name;
    NAMETABLE_id_t max_is_name;
    boolean is_handle ;


    declarator_kind = declarator_ptr->declarator_kind;

    /*
     *   Save away the type attributes.  They will be "hoisted up" to
     *   the real type node created by the procedure.
     */

    xmit_name    = declarator_ptr->type_node_ptr->xmit_type_name ;
    max_is_name  = declarator_ptr->type_node_ptr->max_is ;
    last_is_name = declarator_ptr->type_node_ptr->last_is ;
    is_handle    = declarator_ptr->type_node_ptr->is_handle ;
      
    declarator_ptr->type_node_ptr->xmit_type_name  = NAMETABLE_NIL_ID;
    declarator_ptr->type_node_ptr->max_is          = NAMETABLE_NIL_ID;
    declarator_ptr->type_node_ptr->last_is         = NAMETABLE_NIL_ID;
    declarator_ptr->type_node_ptr->is_handle       = NAMETABLE_NIL_ID;

    switch (declarator_kind) {
        case fixed_array_k:
            /* Create a new array node */
            if (declarator_ptr->type_node_ptr->kind != user_marshalled_k)
                type_node_ptr = AST_fixed_array_node (declarator_ptr->indices,
                                                      declarator_ptr->type_node_ptr);
            else {
                /* We need to construct a user marshalled array of type x */
                type_node_ptr = AST_type_node(user_marshalled_k) ;
                type_node_ptr->type.type_structure.user_marshalled.xmit_type = declarator_ptr->type_node_ptr ->
                        type_structure.user_marshalled.xmit_type ;
                type_node_ptr->type.type_structure.user_marshalled.user_type = (type_t *) 
                        AST_fixed_array_node(declarator_ptr->indices,
                                             declarator_ptr->type_node_ptr->type_structure.user_marshalled.user_type) ;
            }
            declarator_ptr->type_node_ptr = &type_node_ptr->type ;
            break ;

        case open_array_k:
            /* Cons up the array node */
    
            if (declarator_ptr->type_node_ptr->kind != user_marshalled_k)
                 type_node_ptr = AST_open_array_node (zero_constant_p,
                                                      declarator_ptr->indices,
                                                      declarator_ptr->type_node_ptr);
            else {
                /* We need to construct a user marshalled array of type x */
                type_node_ptr = AST_type_node(user_marshalled_k) ;
                type_node_ptr->type.type_structure.user_marshalled.xmit_type = declarator_ptr->type_node_ptr ->
                        type_structure.user_marshalled.xmit_type ;
                type_node_ptr->type.type_structure.user_marshalled.user_type = (type_t *)
                        AST_open_array_node(zero_constant_p,
                                            declarator_ptr->indices,
                                            declarator_ptr->type_node_ptr->type_structure.user_marshalled.user_type) ;
            }
            declarator_ptr->type_node_ptr = &type_node_ptr->type ;
            break ;

        case pointer_k:

         /* 
          *  Pointers need a new pointer node consed up to point
          *  to the referent.
          *  Just change the type node kind field to be pointer_k.
          *  If the pointee is a user_marshalled_k, then we don't want 
          *  to point to it, but rather to the user_type.  Later
          *  We'll fix it up so that we have a user marshalled pointer to 
          *  type.
          */
            if (declarator_ptr->type_node_ptr->kind != user_marshalled_k) {
                type_node_ptr = (structure_t *) alloc (sizeof (structure_t));
                type_node_ptr->type.kind = pointer_k ;
                type_node_ptr->type.type_structure.pointer.pointee =
                    (struct type_t *) declarator_ptr->type_node_ptr;
            }
            else {
                type_node_ptr = AST_type_node(user_marshalled_k) ;
                type_node_ptr->type.type_structure.user_marshalled.xmit_type = 
                    declarator_ptr->type_node_ptr->type_structure.user_marshalled.xmit_type ;
                wtp = AST_type_node(pointer_k) ;
                wtp->type.type_structure.pointer.pointee = 
                    declarator_ptr->type_node_ptr->type_structure.user_marshalled.user_type ;
                type_node_ptr->type.type_structure.user_marshalled.user_type = (type_t *) wtp ;
            }
            declarator_ptr->type_node_ptr = &type_node_ptr->type ;
               
            break ;

        case reference_k:
         /* 
          *  References need a new reference node consed up to point
          *  to the referent.
          *  Just change the type node kind field to be reference_k.
          */

            if (declarator_ptr->type_node_ptr->kind != user_marshalled_k) {
                type_node_ptr = (structure_t *) alloc (sizeof (structure_t));
                type_node_ptr->type.kind = reference_k ;
                type_node_ptr->type.type_structure.pointer.pointee =
                    (struct type_t *) declarator_ptr->type_node_ptr;
            }
            else {
                type_node_ptr = AST_type_node(user_marshalled_k) ;
                type_node_ptr->type.type_structure.user_marshalled.xmit_type = 
                    declarator_ptr->type_node_ptr->type_structure.user_marshalled.xmit_type ;
                wtp = AST_type_node(reference_k) ;
                wtp->type.type_structure.pointer.pointee = 
                    declarator_ptr->type_node_ptr->type_structure.user_marshalled.user_type ;
                type_node_ptr->type.type_structure.user_marshalled.user_type = (type_t *) wtp ;
            }
            declarator_ptr->type_node_ptr = &type_node_ptr->type ;
            break ;

        case routine_ptr_k:
         /* 
          *  Routine pointers need a type node consed up to contain
          *  the routine information.
          */

            type_node_ptr = AST_routine_ptr_node (declarator_ptr->routine_params,
            declarator_ptr->type_node_ptr);
            declarator_ptr->type_node_ptr = &type_node_ptr->type ;
            break ;
    }
        

    declarator_ptr->type_node_ptr->xmit_type_name = xmit_name ;
    declarator_ptr->type_node_ptr->max_is         = max_is_name ;
    declarator_ptr->type_node_ptr->last_is        = last_is_name ;
    declarator_ptr->type_node_ptr->is_handle      = is_handle ;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ f l o a t i n g _ c o n s t a n t
 *  =========================================
 */

constant_t * AST_floating_constant (value)
double   value;
{
    constant_t * constant_node_p;

    constant_node_p = AST_constant_node (real_k);

    constant_node_p->value.real_val = value;

    return constant_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ i m p o r t _ n o d e
 *  =============================
 */


import_t * AST_import_node (imported_file)
STRTAB_str_t imported_file;
{
    import_t * import_node_ptr;

    import_node_ptr = (import_t *) alloc (sizeof (import_t));
    import_node_ptr->source_pos = yylineno;
    import_node_ptr->next_import = NULL;
    import_node_ptr->last_import = NULL;
    import_node_ptr->file_name = imported_file;
    import_node_ptr->name_of_interface = 0;

    return import_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ i n d e x _ n o d e
 *  ===========================
 */


array_index_t * AST_index_node (type, adjust)
type_t *type;
boolean adjust ;
{
    array_index_t * index_node_ptr;

    index_node_ptr = (array_index_t *) alloc (sizeof (array_index_t));
    index_node_ptr->next   = NULL;
    index_node_ptr->last   = index_node_ptr;
    index_node_ptr->type   = type;
    index_node_ptr->adjust = adjust;

    return index_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ i n d e x _ n o d e _ c
 *  ===============================
 */


array_index_t * AST_index_node_c (size)
constant_t *size;
{
  array_index_t *index_node_ptr;
  type_t        *type_node_ptr;
  structure_t   *struct_node;  

  struct_node = AST_type_node(short_subrange_k);
  type_node_ptr = &(struct_node->type);
  type_node_ptr->type_structure.subrange.lower_bound = zero_constant_p;
  type_node_ptr->type_structure.subrange.upper_bound = size;


  index_node_ptr = AST_index_node(type_node_ptr, true) ;

  return index_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ i n i t 
 *  ===============
 */

void AST_init () 
{                            
                         
    int i ;

        /* Initialize some random constants */
    zero_constant_p = AST_integer_constant (0L);
    c_style_id = NAMETABLE_add_id ("c", false);
    pascal_style_id = NAMETABLE_add_id ("pascal", false);
    null_uuid.time_high = 0 ;
    null_uuid.time_low  = 0 ;
    null_uuid.family    = 0 ;
    for (i=0; i<6;i++)
        null_uuid.host[i] = 0;
    interface_uuid      = null_uuid ;                              

        /* Initialize family names  */
    unix_family_name_id     = NAMETABLE_add_id ("unix",true);
    ip_family_name_id       = NAMETABLE_add_id ("ip",true);
    implink_family_name_id  = NAMETABLE_add_id ("implink",true);
    pup_family_name_id      = NAMETABLE_add_id ("pup",true);
    chaos_family_name_id    = NAMETABLE_add_id ("chaos",true);
    ns_family_name_id       = NAMETABLE_add_id ("ns",true);
    nbs_family_name_id      = NAMETABLE_add_id ("nbs",true);
    ecma_family_name_id     = NAMETABLE_add_id ("ecma",true);
    datakit_family_name_id  = NAMETABLE_add_id ("datakit",true);
    ccitt_family_name_id    = NAMETABLE_add_id ("ccitt",true);
    sna_family_name_id      = NAMETABLE_add_id ("sna",true);
    dds_family_name_id      = NAMETABLE_add_id ("dds",true);

    return;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ i n t e g e r _ c o n s t a n t
 *  =====================================
 */

constant_t * AST_integer_constant (value)
long int    value;
{
    constant_t * constant_node_p;

    constant_node_p = AST_constant_node (integer_k);

    constant_node_p->value.int_val = value;

    return constant_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ i n t e g e r _ t y p e _ n o d e
 *  =====================================
 */

structure_t * AST_integer_type_node ()
{
    structure_t * type_node_p;

    /* allocate a type per state of C integer type modifiers */
    switch (int_size) {
        case small_int:
            if (int_signed)
                type_node_p = AST_type_node (small_integer_k);
            else
                type_node_p = AST_type_node (small_unsigned_k);
            break;
        case short_int:
            if (int_signed)
                type_node_p = AST_type_node (short_integer_k);
            else
                type_node_p = AST_type_node (short_unsigned_k);
            break;
        case long_int:
            if (int_signed)
                type_node_p = AST_type_node (long_integer_k);
            else
                type_node_p = AST_type_node (long_unsigned_k);
            break;
        case hyper_int:
            if (int_signed)
                type_node_p = AST_type_node (hyper_integer_k);
            else
                type_node_p = AST_type_node (hyper_unsigned_k);
            break;
        }

    /* reestablish defaults */
    int_size = long_int;
    int_signed = true;

    return type_node_p;
}

/*---------------------------------------------------------------------*/

/*                                         
 *  A S T _ i n t e r f a c e _ n o d e 
 *  ===================================
 */

structure_t * AST_interface_node (import_list, export_list)
import_t * import_list;
binding_t * export_list;
{
    structure_t * interface_struct_p;
    interface_t * interface_node_p;

    interface_struct_p = (structure_t *) alloc (sizeof (structure_t));
    interface_node_p = (interface_t *) & interface_struct_p->interface;
    interface_node_p->source_pos = yylineno;

    /* 
      *  Record imports and exports
      */

    interface_node_p->imports = (struct import_t *) import_list;
    interface_node_p->exports = (struct binding_t *) export_list;

    /* 
      *  Default interface attributes.
      *  These will be filled in later when we process the interface
      *  attributes.
      */
    interface_node_p->interface_uuid = null_uuid ;

    interface_node_p->interface_version = 0;
    interface_node_p->implicit_handle_var = NAMETABLE_NIL_ID;
    interface_node_p->auto_binding = false;

    return interface_struct_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ n a m e d _ c o n s t
 *  =============================================
 */

constant_t * AST_named_const (const_name)
NAMETABLE_id_t const_name;
{
    constant_t * new_const_p;

    new_const_p = AST_constant_node (named_const_k);
    new_const_p->value.named_val.name = const_name;
    new_const_p->value.named_val.resolution = NULL;

    return new_const_p;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ n a m e d _ t y p e
 *  ================================
 */



structure_t * AST_named_type (type_name)
NAMETABLE_id_t type_name;
{
    structure_t * named_type_node;


    named_type_node = AST_type_node (named_k);


    named_type_node->type.kind = named_k;
    named_type_node->type.type_structure.named.name = type_name;
    named_type_node->type.type_structure.named.resolution = NULL;

    return named_type_node;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ p a r a m e t e r _ n o d e
 *  ===================================
 */

parameter_t * AST_parameter_node (identifier)
NAMETABLE_id_t identifier;
{
    parameter_t * parameter_node_ptr;

    parameter_node_ptr = (parameter_t *) alloc (sizeof (parameter_t));

    parameter_node_ptr->source_pos = yylineno;
    parameter_node_ptr->next_param = NULL;
    parameter_node_ptr->last_param = NULL;
    parameter_node_ptr->type = NULL;

    parameter_node_ptr->in = false;
    parameter_node_ptr->out = false;
    parameter_node_ptr->requires_pointer = false ;

    parameter_node_ptr->patched = false;

    parameter_node_ptr->name = identifier;

    return parameter_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ p o i n t e r _ n o d e
 *  ===============================
 */

structure_t * AST_pointer_node (pointee)
type_t * pointee;
{
    structure_t * pointer_node_ptr;

    pointer_node_ptr = AST_type_node (pointer_k);
    pointer_node_ptr->type.type_structure.pointer.pointee = pointee;
    pointer_node_ptr->type.type_structure.pointer.visited = false;
    pointer_node_ptr->type.type_structure.pointer.open    = false;

    return pointer_node_ptr;
}

static
#ifndef __STDC__
void patch_field_attrs_to_type_node(type_node_p)
type_t * type_node_p;
#else
void patch_field_attrs_to_type_node(type_t *type_node_p)
#endif
{

    if (last_is_of_field != NAMETABLE_NIL_ID) {
        if (type_node_p->last_is != NAMETABLE_NIL_ID)
            log_error(type_node_p->source_pos,
                      "%s\n",
                      "[last_is()] attribute already specified") ;
        else
            type_node_p->last_is = last_is_of_field;
        last_is_of_field = NAMETABLE_NIL_ID;
    }

    if (max_is_of_field != NAMETABLE_NIL_ID) {
        if (type_node_p->max_is != NAMETABLE_NIL_ID)
            log_error(type_node_p->source_pos,
                      "%s\n",
                      "[max_is()] attribute already specified") ;
        else
            type_node_p->max_is = max_is_of_field;
        max_is_of_field = NAMETABLE_NIL_ID;
    }
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ p r o p a g a t e _ f i e l d _ t y p e
 *  ===============================================
 */

field_t * AST_propagate_field_type (field_nodes, type_node_p)
field_t * field_nodes;
type_t * type_node_p;
{

    field_t * fp;

    patch_field_attrs_to_type_node (type_node_p);

    for (fp = field_nodes; fp; fp = fp->next_field)
        fp->type = (struct type_t *) type_node_p;

    return field_nodes;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ p r o p a g a t e _ p a r a m _ a t t r s
 *  =================================================
 */

parameter_t * AST_propagate_param_attrs (param_nodes, type_node_p)
parameter_t * param_nodes;
type_t * type_node_p;
{
    parameter_t * param_node_ptr;

    for (param_node_ptr = param_nodes; param_node_ptr; param_node_ptr = param_node_ptr->next_param) {

        param_node_ptr->comm_status      = comm_status_parameter;
        param_node_ptr->in               = in_parameter;
        param_node_ptr->out              = out_parameter;
        param_node_ptr->ref              = ref_parameter;
        param_node_ptr->requires_pointer = requires_pointer;
    }

    comm_status_parameter = false;
    in_parameter          = false;
    out_parameter         = false;
    ref_parameter         = false;
    requires_pointer      = false;

    return param_nodes;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ p r o p a g a t e _ p a r a m _ t y p e
 *  ================================================
 */

parameter_t * AST_propagate_param_type (param_nodes, type_node_ptr)
parameter_t * param_nodes;
type_t * type_node_ptr;
{
    parameter_t * pp;


    patch_field_attrs_to_type_node (type_node_ptr);

    for (pp = param_nodes; pp; pp = pp->next_param)
        pp->type = (struct type_t *) type_node_ptr;

    return param_nodes;
}

/*---------------------------------------------------------------------*/

/*
 *
 *  A S T _ p r o p  _ d e c l a r a t o r _ t y p e
 *  =================================================
 */

declarator_t * AST_prop_declarator_type (type_node_ptr, declarators_ptr)
type_t * type_node_ptr;
declarator_t * declarators_ptr;
{
    declarator_t * dp;

    patch_field_attrs_to_type_node (type_node_ptr);

    for (dp = declarators_ptr; dp; dp = dp->next_declarator) {
        dp->type_node_ptr = type_node_ptr;
        AST_fix_declarator_type (dp);
    }

    return declarators_ptr;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ r e c o r d _ n o d e 
 *  ===========================
 */

structure_t * AST_record_node (fields, variant)
field_t * fields;
variant_t * variant;

{
    structure_t * record_node_ptr;

    record_node_ptr = AST_type_node (record_k);

    record_node_ptr->type.type_structure.record.fields = fields;
    record_node_ptr->type.type_structure.record.variant = variant;

    return record_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ r o u t i n e _ n o d e
 *  ===============================
 */

structure_t * AST_routine_node (parameters, result_name, result_type)
parameter_t * parameters;
NAMETABLE_id_t result_name;
type_t * result_type;

{
    structure_t * routine_node_p;
    parameter_t * result_node_p;

    routine_node_p = (structure_t *) alloc (sizeof (structure_t));

    routine_node_p->routine.source_pos = yylineno;

    routine_node_p->routine.idempotent = false;
    routine_node_p->routine.secure = false;
    routine_node_p->routine.parameters = parameters;
    routine_node_p->routine.result = NULL;

    result_node_p = AST_parameter_node (result_name);
    result_node_p->type = (struct type_t *) result_type;
    routine_node_p->routine.result = result_node_p;

    routine_node_p->routine.pointerized = false;
    routine_node_p->routine.backended = false;
    routine_node_p->routine.local_vars = NULL;

    return routine_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ r o u t i n e _ p t r _ n o d e 
 *  =======================================
 *
 */

structure_t * AST_routine_ptr_node (params, result)
parameter_t * params;
type_t * result;
{
    structure_t * routine_ptr_node_p;
    parameter_t * result_node_p;

    routine_ptr_node_p = AST_type_node (routine_ptr_k);

    routine_ptr_node_p->type.type_structure.routine_ptr.parameters = params;

    result_node_p = AST_parameter_node (NAMETABLE_NIL_ID);
    result_node_p->type = (struct type_t *) result;
    routine_ptr_node_p->type.type_structure.routine_ptr.result = result_node_p;

    routine_ptr_node_p->type.type_structure.routine_ptr.pointerized = false;


    return routine_ptr_node_p;
}


/*---------------------------------------------------------------------*/

/*
 *  A S T _ s e t _ i n t e r f a c e _ a t t r i b u t e s
 *  =======================================================
 */

void AST_set_interface_attributes (interface_node)
interface_t * interface_node;
{
    interface_node->auto_binding = interface_auto_bound;
    interface_node->interface_uuid = interface_uuid;
    interface_node->interface_version = interface_version;
    interface_node->implicit_handle_var = interface_implicit_handle_var;
    interface_node->implicit_handle_type = interface_implicit_handle_type ;
    interface_node->local_only = interface_is_local;

    AST_set_well_known_ports(interface_node) ;

    interface_auto_bound = false;
    interface_uuid = null_uuid ;
    interface_port_count = 0 ;
    interface_implicit_handle_var = NAMETABLE_NIL_ID;
    interface_implicit_handle_type = NULL ;
    interface_is_local = false;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ s e t _ n o d e
 *  =======================
 */

structure_t * AST_bitset_node (base_type, widenable, is_short)
type_t * base_type;
boolean widenable;
boolean is_short;
{
    structure_t * bitset_node_ptr;

    bitset_node_ptr = AST_type_node (is_short ? short_bitset_k : long_bitset_k);

    bitset_node_ptr->type.type_structure.bitset.base_type = (struct type_t *) base_type;
    bitset_node_ptr->type.type_structure.bitset.widenable  = widenable;

    return bitset_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ s e t _ r o u t i n e _ a t t r s  
 *  =========================================
 */

routine_t * AST_set_routine_attrs (routine_node_ptr)
routine_t * routine_node_ptr;
{

    routine_node_ptr->secure = routine_secure;
    routine_node_ptr->idempotent = routine_idempotent;
    routine_node_ptr->maybe = routine_maybe;
    routine_node_ptr->broadcast = routine_broadcast ;

    routine_secure = false;
    routine_idempotent = false;
    routine_maybe = false;
    routine_broadcast = false ;

    return routine_node_ptr;

}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ s e t _ t y p e _ a t t r s
 *  ===================================
 */

structure_t * AST_set_type_attrs (type_node_ptr, attr_list_ptr)
structure_t * type_node_ptr;
type_attr_t * attr_list_ptr;
{                        
    type_attr_t * attr_node_p ;
    boolean handle_seen, 
            last_is_seen,
            max_is_seen,
            xmit_seen,
            error ;
    structure_t *um_node_p ;
                 
    handle_seen = xmit_seen = false ;

    last_is_seen = (type_node_ptr->type.last_is != NAMETABLE_NIL_ID);
    max_is_seen  = (type_node_ptr->type.max_is  != NAMETABLE_NIL_ID);

    error = false ;              

    for (attr_node_p = attr_list_ptr; attr_node_p; attr_node_p = (type_attr_t *) attr_node_p->next_attr) {
        switch (attr_node_p->which) {
            case xmit_name: if (! xmit_seen) {
                                xmit_seen = true ;
                                type_node_ptr->type.xmit_type_name = attr_node_p->the_attr.xmit_type_name;
                                break ;
                            }
                            else {
                                error = true ;
                                break;
                            }
            case last_is_name: if (! last_is_seen) {
                                last_is_seen = true ;
                                type_node_ptr->type.last_is = attr_node_p->the_attr.last_is;
                                break ;
                            }
                            else {
                                error = true ;
                                break;
                            }
            case max_is_name: if (! max_is_seen) {
                                max_is_seen = true ;
                                type_node_ptr->type.max_is = attr_node_p->the_attr.max_is ;
                                break ;
                            }
                            else {
                                error = true ;
                                break;
                            }
             
            case is_handle: if (! handle_seen) {
                                handle_seen = true ;
                                type_node_ptr->type.is_handle = true ;
                                break ;
                            }
                            else {
                                error = true ;
                                break;
                            }
        }
             
        if (error) {
            log_error(attr_node_p->source_line,
                      "%s\n",
                      "Attribute already specified") ;
        }

        if (xmit_seen) {
            /* create a named type as the xmit_type */
            um_node_p = AST_type_node(user_marshalled_k) ;
            um_node_p->type.type_structure.user_marshalled.xmit_type =
              (type_t *)AST_named_type(type_node_ptr->type.xmit_type_name) ;

            um_node_p->type.type_structure.user_marshalled.user_type = &type_node_ptr->type ;
            um_node_p->type.is_handle = type_node_ptr->type.is_handle ;
            um_node_p->type.max_is = type_node_ptr->type.max_is ;
            um_node_p->type.last_is = type_node_ptr->type.last_is ;
            type_node_ptr = um_node_p ;
        }
    }

    return type_node_ptr;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ s e t _ w e l l _ k n o w n _ p o r t s
 *  ===============================================
 */

void AST_set_well_known_ports(interface_node_p)
interface_t     *interface_node_p ;
{
    NAMETABLE_id_t family_name;
    int         port_number;
    char        *family ;
    int         family_no ;
    int         i ;

    for (family_no = 0; family_no < socket_$num_families; family_no++)
       interface_node_p->well_known_ports[family_no] = socket_$unspec_port;

    for (i = 0; i < interface_port_count; i++) {
        family_name = interface_ports[i].family_name;
        port_number = interface_ports[i].port_number;
        if (family_name == unix_family_name_id)
          interface_node_p->well_known_ports[socket_$unix] = port_number;
        else if (family_name == ip_family_name_id)
          interface_node_p->well_known_ports[socket_$internet] = port_number;
        else if (family_name == implink_family_name_id)
          interface_node_p->well_known_ports[socket_$implink] = port_number;
        else if (family_name == pup_family_name_id)
          interface_node_p->well_known_ports[socket_$pup] = port_number;
        else if (family_name == chaos_family_name_id)
          interface_node_p->well_known_ports[socket_$chaos] = port_number;
        else if (family_name == ns_family_name_id)
          interface_node_p->well_known_ports[socket_$ns] = port_number;
        else if (family_name == nbs_family_name_id)
          interface_node_p->well_known_ports[socket_$nbs] = port_number;
        else if (family_name == ecma_family_name_id)
          interface_node_p->well_known_ports[socket_$ecma] = port_number;
        else if (family_name == datakit_family_name_id)
          interface_node_p->well_known_ports[socket_$datakit] = port_number;
        else if (family_name == ccitt_family_name_id)
          interface_node_p->well_known_ports[socket_$ccitt] = port_number;
        else if (family_name == sna_family_name_id)
          interface_node_p->well_known_ports[socket_$sna] = port_number;
        else if (family_name == dds_family_name_id)
          interface_node_p->well_known_ports[socket_$dds] = port_number;
        else {
          NAMETABLE_id_to_string(family_name, &family) ;
          log_error(yylineno, "Invalid family name: %s\n", family) ;
        }
    }
}


/*---------------------------------------------------------------------*/

/*
 *  A S T _ s t r i n g _ c o n s t a n t
 *  =========================================
 */

constant_t * AST_string_constant (value)
STRTAB_str_t value;
{
    constant_t * constant_node_p;

    constant_node_p = AST_constant_node (string_k);

    constant_node_p->value.string_val = value;

    return constant_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ f i x e d _ s t r i n g _z e r o _ n o d e 
 *  ===============================
 */

structure_t * AST_fixed_string_zero_node (upper_bound)
constant_t * upper_bound;
{
    structure_t * string_zero_node_p;

    string_zero_node_p = AST_type_node (fixed_string_zero_k);
    string_zero_node_p->type.type_structure.fixed_string_zero.index =
        AST_index_node_c (upper_bound);

    return string_zero_node_p;
}


/*---------------------------------------------------------------------*/

/*
 *  A S T _ s u b r a n g e _ n o d e
 *  =================================
 */

structure_t * AST_subrange_node (lower_bound, upper_bound)
constant_t * lower_bound,
*upper_bound;
{
    structure_t * type_node_p = AST_type_node (short_subrange_k);

    type_node_p->type.type_structure.subrange.lower_bound = lower_bound;
    type_node_p->type.type_structure.subrange.upper_bound = upper_bound;

    return type_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ t a g _ n o d e
 *  =======================
 */

tag_t * AST_tag_node (tag_value)
constant_t * tag_value;
{
    tag_t * tag_node_p;

    tag_node_p = (tag_t *) alloc (sizeof (tag_t));
    tag_node_p->source_pos = yylineno;
    tag_node_p->tag_value = *tag_value;
    tag_node_p->next_tag = NULL;
    tag_node_p->last_tag = NULL;

    return tag_node_p;
}

/*---------------------------------------------------------------------*/

/*
 *  A S T _ t y p e _ n o d e
 *  =========================
 */

structure_t * AST_type_node (kind)
type_kind kind;
{
    structure_t * type_node_ptr;

    type_node_ptr = (structure_t *) alloc (sizeof (structure_t));

    type_node_ptr->type.xmit_type_name = NAMETABLE_NIL_ID;
    type_node_ptr->type.last_is = NAMETABLE_NIL_ID;
    type_node_ptr->type.max_is = NAMETABLE_NIL_ID;
    type_node_ptr->type.type_name = NAMETABLE_NIL_ID;
    type_node_ptr->type.kind = kind;
    type_node_ptr->type.source_pos = yylineno;
    type_node_ptr->type.is_handle = false ;

    return type_node_ptr;
}


/*---------------------------------------------------------------------*/

/*
 *  A S T _  v a r i a n t _ n o d e
 *  ===========================
 */


variant_t * AST_variant_node (label, tag_id, tag_type, components)
NAMETABLE_id_t label;
NAMETABLE_id_t tag_id;
type_t * tag_type;
component_t * components;
{
    variant_t * variant_node_ptr;

    variant_node_ptr = (variant_t *) alloc (sizeof (variant_t));

    variant_node_ptr->label = 
        (label != NAMETABLE_NIL_ID)
        ? label
        : NAMETABLE_add_id ("tagged_union", false);
    variant_node_ptr->tag_id = tag_id;
    variant_node_ptr->tag_type = (struct type_t *) tag_type;
    variant_node_ptr->components = components;

    return variant_node_ptr;
}


/*---------------------------------------------------------------------*/

/*
 *  AST_open_array_node
 *  ======================
 */

structure_t * AST_open_array_node (lower_bound, indices, elements)
constant_t * lower_bound;
array_index_t * indices;
type_t * elements;
{
    structure_t * type_node_ptr;
    open_array_t * array_node_ptr;

    type_node_ptr = AST_type_node (open_array_k);

    array_node_ptr = &type_node_ptr->type.type_structure.open_array;
    array_node_ptr->elements = elements;
    array_node_ptr->lower_bound = lower_bound;
    array_node_ptr->indices = indices;

    return type_node_ptr;
}

/*---------------------------------------------------------------------*/

/********************************************************************/
/*                                                                  */
/*                    AST dumping routines                          */
/*                                                                  */
/********************************************************************/

#ifndef __STDC__
void indent (indentation)
int     indentation;
#else
void indent (int indentation)
#endif
{
    int     i;

    for (i = 0; i < indentation; i++)
        printf (".");
}

#ifndef __STDC__
void dump_nametable_id (format_string, id)
char   *format_string;
NAMETABLE_id_t id;
#else
void dump_nametable_id (char *format_string, NAMETABLE_id_t id)
#endif
{
    char   *name_ptr;

    if (id == NAMETABLE_NIL_ID)
        printf (format_string, "NAMETABLE_NIL_ID");
    else {
        NAMETABLE_id_to_string (id, &name_ptr);
        printf (format_string, name_ptr);
    }
}

#ifndef __STDC__
void print_boolean(format, value) 
    char * format;
    boolean value ;
#else
void print_boolean (char *format, boolean value)
#endif
{
    if (value)
        printf(format, "true");
    else
        printf(format, "false") ;
}

void AST_dump_interface (interface_node_ptr)
interface_t * interface_node_ptr;
{                                
    if (interface_node_ptr == NULL)
        return;

    printf ("Dumping interface\n\n");
    printf("UID\n");
    printf("\ttime_high = %08lx\n", interface_node_ptr->interface_uuid.time_high);
    printf("\ttime_low  = %04x\n", interface_node_ptr->interface_uuid.time_low);
    printf("\tfamily    = %02x\n", interface_node_ptr->interface_uuid.family);
    printf("\thost      = %02x.%02x.%02x.%02x.%02x.%02x.%02x\n",
            interface_node_ptr->interface_uuid.host[0],
            interface_node_ptr->interface_uuid.host[1],
            interface_node_ptr->interface_uuid.host[2],
            interface_node_ptr->interface_uuid.host[3],
            interface_node_ptr->interface_uuid.host[4],
            interface_node_ptr->interface_uuid.host[5],
            interface_node_ptr->interface_uuid.host[6]) ;  
    printf ("Version = %d\n", interface_node_ptr->interface_version);
    if (interface_node_ptr->implicit_handle_var != NAMETABLE_NIL_ID) {
        dump_nametable_id("Implicit handle var = %s\n", interface_node_ptr->implicit_handle_var);
        AST_dump_type(interface_node_ptr->implicit_handle_type,"Handle type = %s\n", 4);
    }
    AST_dump_export_list (interface_node_ptr->exports);
}


void AST_dump_export_list (export_list_ptr)
binding_t * export_list_ptr;
{
    binding_t * ep;

    for (ep = export_list_ptr; ep; ep = ep->next)
        AST_dump_exported_item (ep);
}


void AST_dump_exported_item (export_ptr)
binding_t * export_ptr;
{

    dump_nametable_id ("Exported Item: %s ", export_ptr->name);

    switch (export_ptr->kind) {
    case constant_k: 
        printf ("Type = constant_k\n");
        AST_dump_constant (&export_ptr->binding->constant);
        break;

    case type_k: 
        printf ("Type = type_k\n");
        AST_dump_type (&export_ptr->binding->type, "Type is %s\n", 4);
        break;

    case routine_k: 
        printf ("Type = routine_k\n");
        AST_dump_routine (&export_ptr->binding->routine, 4);
        break;
    }
}

void AST_dump_constant (constant_node_ptr)
constant_t * constant_node_ptr;
{
    char   *string_val_ptr;

    switch (constant_node_ptr->kind) {
    case integer_k: 
        printf ("integer constant: %ld\n", constant_node_ptr->value.int_val);
        break;

    case real_k: 
        /* XXX should be %f?? */
        printf ("real constant: %ld\n", constant_node_ptr->value.int_val);
        break;

    case string_k: 
        STRTAB_str_to_string (constant_node_ptr->value.string_val, &string_val_ptr);
        printf ("string constant: %s\n", string_val_ptr);
        break;
    case enum_k: 
        printf ("enum ordinal val = %ld\n", constant_node_ptr->value.enum_val.ordinal_mapping);
        break;
    case named_const_k: 
        dump_nametable_id ("named_constant: %s\n", constant_node_ptr->value.named_val.name);
        if (constant_node_ptr->value.named_val.resolution != NULL)
            AST_dump_constant (constant_node_ptr->value.named_val.resolution);
        break;
    }
}


#ifndef __STDC__
void print_type_name (format, type)
char   *format;
type_kind type;
#else
void print_type_name (char *format, type_kind type)
#endif
{
    switch (type) {
    case small_integer_k: 
        printf (format, "small_integer_k");
        break;
    case short_integer_k: 
        printf (format, "short integer_k");
        break;
    case long_integer_k: 
        printf (format, "long_integer_k");
        break;
    case hyper_integer_k: 
        printf (format, "hyper_integer_k");
        break;
    case small_unsigned_k: 
        printf (format, "small_unsigned_k");
        break;
    case short_unsigned_k: 
        printf (format, "short unsigned_k");
        break;
    case long_unsigned_k: 
        printf (format, "long_unsigned_k");
        break;
    case hyper_unsigned_k: 
        printf (format, "hyper_unsigned_k");
        break;
    case short_bitset_k: 
        printf (format, "short_bitset_k");
        break;
    case long_bitset_k: 
        printf (format, "long_bitset_k");
        break;
    case short_enumeration_k: 
        printf (format, "short_enumeration_k");
        break;
    case long_enumeration_k: 
        printf (format, "long_enumeration_k");
        break;
    case short_real_k: 
        printf (format, "short_real_k");
        break;
    case long_real_k: 
        printf (format, "long_real_k");
        break;
    case short_subrange_k: 
        printf (format, "short_subrange_k");
        break;
    case long_subrange_k: 
        printf (format, "long_subrange_k");
        break;
    case open_string_zero_k: 
        printf (format, "open_string_zero_k");
        break;
    case fixed_string_zero_k: 
        printf (format, "fixed_string_zero_k");
        break;
    case open_array_k: 
        printf (format, "open_array_k");
        break;
    case fixed_array_k: 
        printf (format, "fixed_array_k");
        break;
    case record_k: 
        printf (format, "record_k");
        break;
    case named_k: 
        printf (format, "named_k");
        break;
    case pointer_k: 
        printf (format, "pointer_k");
        break;
    case void_k: 
        printf (format, "void_k");
        break;
    case character_k: 
        printf (format, "character_k");
        break;
    case boolean_k: 
        printf (format, "boolean_k");
        break;
    case routine_ptr_k: 
        printf (format, "routine_ptr_k");
        break;
    case handle_k:
        printf (format, "handle_k") ;
    }
}

void AST_dump_open_array (array_node_ptr, indentation)
open_array_t * array_node_ptr;
int     indentation;
{
    indent(indentation+4);
    printf("lower bound = ");
    AST_dump_constant (array_node_ptr->lower_bound);
    printf("\n");
    AST_dump_type (array_node_ptr->elements, "Array elements = %s\n", indentation + 4);
}


void AST_dump_indices(index_node_ptr, indentation)
array_index_t * index_node_ptr ;
int indentation;
{
    for (; index_node_ptr; index_node_ptr = index_node_ptr->next)
        AST_dump_type(index_node_ptr->type, "Index_type = %s\n", indentation) ;
}


void AST_dump_type (type_node_ptr, format, indentation)
type_t * type_node_ptr;
char   * format;
int      indentation; 
{
    variant_t * vp;
    static int visit_count = 0;
#define MAX_DEPTH 10 

    if (visit_count > MAX_DEPTH) {
        printf("...\n") ;
        return ;
    } ;

    ++visit_count ;
    indent (indentation);
    print_type_name (format, type_node_ptr->kind);

    if (type_node_ptr->xmit_type_name != NAMETABLE_NIL_ID) {
        indent (indentation);
        dump_nametable_id ("\tTransmit type = %s\n", type_node_ptr->xmit_type_name);
    }

    if (type_node_ptr->last_is != NAMETABLE_NIL_ID) {
        indent (indentation);
        dump_nametable_id ("\tLast_is = %s\n", type_node_ptr->last_is);
    }

    if (type_node_ptr->max_is != NAMETABLE_NIL_ID) {
        indent (indentation);
        dump_nametable_id ("\tMax_is = %s\n", type_node_ptr->max_is);
    }

    if (type_node_ptr->is_handle) {
        indent (indentation);
        dump_nametable_id ("\t[handle]\n", NAMETABLE_NIL_ID);
    }

    switch (type_node_ptr->kind) {
    case short_enumeration_k: 
    case long_enumeration_k: 
        AST_dump_enumerators (&type_node_ptr->type_structure.enumeration, indentation);
        break;

    case short_subrange_k: 
    case long_subrange_k: 
        AST_dump_subrange (&type_node_ptr->type_structure.subrange, indentation);
        break;

    case fixed_string_zero_k: 
        indent (indentation + 4);
        printf ("upper bound = ");
        AST_dump_indices (type_node_ptr->type_structure.fixed_string_zero.index, indentation);
        break;

    case open_string_zero_k: 
        break;

    case record_k: 
        AST_dump_record (type_node_ptr->type_structure.record.fields, indentation);
        if ((vp = type_node_ptr->type_structure.record.variant) != NULL)
            AST_dump_variant_part (vp, indentation);
        break;

    case pointer_k: 
        indent (indentation);
        printf ("bound to type\n");
        AST_dump_type (type_node_ptr->type_structure.pointer.pointee, "Referent is %s\n", indentation + 4);
        break;

    case open_array_k: 
        AST_dump_open_array (&type_node_ptr->type_structure.open_array, indentation);
        break;
    case fixed_array_k: 
        AST_dump_fixed_array (&type_node_ptr->type_structure.fixed_array, indentation);
        break;

    case short_bitset_k: 
    case long_bitset_k: 
        AST_dump_type (type_node_ptr->type_structure.bitset.base_type, "Set base type = %s\n", indentation + 4);
        break;

    case named_k: 
        indent (indentation + 4);
        dump_nametable_id ("names %s\n", type_node_ptr->type_structure.named.name);
        if (type_node_ptr->type_structure.named.resolution != NULL)
            AST_dump_type (type_node_ptr->type_structure.named.resolution,
            "Bound to type: %s\n", indentation + 4);
        break;
    case routine_ptr_k: 
        indent (indentation + 4);
        AST_dump_routine (&type_node_ptr->type_structure.routine_ptr, indentation + 4);
        break;
     
    case user_marshalled_k:
        indent(indentation+4) ;
        printf("user_marshalled type\n");
        AST_dump_type(type_node_ptr->type_structure.user_marshalled.xmit_type,
          "Transmitted as: %s\n", indentation + 4);
        AST_dump_type(type_node_ptr->type_structure.user_marshalled.user_type,
          "User type: %s\n", indentation + 4);

    }
    --visit_count ;
}

void AST_dump_enumerators (enum_node_ptr, indentation)
enumeration_t * enum_node_ptr;
int indentation;
{
    binding_t * ebp;

    for (ebp = enum_node_ptr->enum_constants; ebp; ebp = ebp->next) {
        indent (indentation + 4);
        dump_nametable_id ("Enumerator name: %s. ", ebp->name);
        AST_dump_constant (&ebp->binding->constant);
    }

}

void AST_dump_subrange (subrange_node_ptr, indentation)
subrange_t * subrange_node_ptr;
int     indentation;
{
    indent (indentation);
    printf ("Lower bound = ");
    AST_dump_constant (subrange_node_ptr->lower_bound);
    indent (indentation);
    printf ("Upper bound = ");
    AST_dump_constant (subrange_node_ptr->upper_bound);
}


void AST_dump_variant_part (variant_ptr, indentation)
variant_t * variant_ptr;
int     indentation;
{
    component_t * cp;

    indent (indentation);
    dump_nametable_id ("\tVariant tag_id = %s\n", variant_ptr->tag_id);
    if (variant_ptr->tag_type != NULL)
        AST_dump_type (variant_ptr->tag_type, "Variant tag type = %s\n", indentation + 4);
    for (cp = variant_ptr->components; cp; cp = cp->next_component)
        AST_dump_component (cp, indentation + 4);
}

void AST_dump_component (component_node_ptr, indentation)
component_t * component_node_ptr;
int     indentation;
{
    tag_t * tp;
    field_t * fp;

    for (tp = component_node_ptr->tags; tp; tp = tp->next_tag) {
        indent (indentation);
        printf ("case tag value is ");
        AST_dump_constant (&tp->tag_value);
    }

    for (fp = component_node_ptr->fields; fp; fp = fp->next_field) {
        dump_nametable_id ("\tfield_name = %s\n", fp->name);
        AST_dump_type (fp->type, "Field type = %s\n", indentation + 4);
    }

}


void AST_dump_record (field_node_ptr, indentation)
field_t * field_node_ptr;
int     indentation;
{
    field_t * fp;

    for (fp = field_node_ptr; fp; fp = fp->next_field) {
        dump_nametable_id ("\tfield_name = %s\n", fp->name);
        AST_dump_type (fp->type, "Field type = %s\n", indentation + 4);
    }

}
                    

void AST_dump_fixed_array (array_node_ptr, indentation)
fixed_array_t * array_node_ptr;
int     indentation;
{

    AST_dump_indices (array_node_ptr->indices, indentation + 4);
    AST_dump_type (array_node_ptr->elements, "Array elements = %s\n", indentation + 4);
}

void AST_dump_routine (routine_node_ptr, indentation)
routine_t * routine_node_ptr;
int indentation;
{
    parameter_t * pp;
    
    printf("\troutine attributes\n") ;
    print_boolean("idempotent = %s\n", routine_node_ptr->idempotent) ;
    print_boolean("maybe = %s\n", routine_node_ptr->maybe) ;
    print_boolean("broadcast = %s\n", routine_node_ptr->broadcast) ;

    printf ("\troutine parameters\n");

    for (pp = routine_node_ptr->parameters; pp; pp = pp->next_param)
        AST_dump_parameter (pp, 4);

    if (routine_node_ptr->result != NULL) {
        indent (indentation);
        printf ("\tRoutine result\n");
        AST_dump_parameter (routine_node_ptr->result, 4);
    }
}

void AST_dump_parameter (param_node_ptr, indentation)
parameter_t * param_node_ptr;
int     indentation;
{

    indent (indentation);
    dump_nametable_id ("\tparameter name = %s\n", param_node_ptr->name);

    if (param_node_ptr->type->is_handle) {
        indent (4);
        printf ("Handle parameter\n");
    }

    if (param_node_ptr->in) {
        indent (4);
        printf ("input parameter\n");
    }

    if (param_node_ptr->out) {
        indent (4);
        printf ("output parameter\n");
    }

    if (param_node_ptr->ref) {
        indent (4);
        printf ("reference parameter\n");
    }

    AST_dump_type (param_node_ptr->type, "Parameter type = %s\n", 4);
}
