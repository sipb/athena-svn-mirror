/*
 *  This module contains the speller routines for C header files.
 */

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

#if defined(MSDOS) || defined(SYS5) || defined(vaxc)
#  include <string.h>
#else
#  include <strings.h>
#endif

#ifdef DSEE
#  include "$(rpc.idl).h"
#else
#  include "rpc.h"
#endif

#include "utils.h"
#include "nametbl.h"
#include "ast.h"
#include "astp.h"
#include "files.h"
#include "backend.h"
#include "sysdep.h"
#include "cspell.h"

void CSPELL_parameter_list __PROTOTYPE((FILE *fid, parameter_t *pp));
void CSPELL_func_decl __PROTOTYPE((FILE *fid, type_t *type, NAMETABLE_id_t name, parameter_t *paramlist));

void abort __PROTOTYPE((void));

long
cardinality (s)
subrange_t s;
{
    constant_t upper, lower;

    lower = *(s.lower_bound);
    upper = *(s.upper_bound);

    if (lower.kind == named_const_k)
        lower = *(lower.value.named_val.resolution);
    if (upper.kind == named_const_k)
        upper = *(upper.value.named_val.resolution);

    switch (lower.kind) {
        case integer_k:
            return (upper.value.int_val - lower.value.int_val + 1);
        case enum_k:
            return (upper.value.enum_val.ordinal_mapping
                    - lower.value.enum_val.ordinal_mapping + 1);
        default:
            fprintf(stderr, "Unknown kind in cardinality()!\n");
            abort();
        }
}

static
#ifndef __STDC__
void constant_val_to_string (cp, str)
constant_t *cp;
char       *str;
#else
void constant_val_to_string (constant_t *cp, char *str)
#endif
{
    char       *str2;

    switch (cp->kind) {
        case nil_k:
            sprintf (str, "NULL");
            break;
        case boolean_const_k:
            if (cp->value.boolean_value)
                sprintf (str, "ndr_$true");
            else
                sprintf (str, "ndr_$false");
            break;
        case integer_k:
            sprintf (str, "%ld", cp->value.int_val);
            break;
        case real_k:
            sprintf (str, "%f", cp->value.real_val);
            break;
        case string_k:
            STRTAB_str_to_string (cp->value.string_val, &str2);
            sprintf (str, "\"%s\"", str2);
            break;
        case named_const_k:
            NAMETABLE_id_to_string (cp->value.named_val.name, &str2);
            sprintf (str, "%s", str2);
            break;
        default:
            fprintf(stderr, "Unknown tag in constant_val_to_string\n");
            abort();
        }
}

type_t *gen_type_node (kind)
type_kind kind;
{
    structure_t *sp;
    sp = AST_type_node (kind);
    return &sp->type;
}

boolean record_type (tp)
type_t *tp;
{
    if (tp == NULL)
        return false;

    switch (tp->kind) {

        case drep_k:
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
        case hyper_integer_k:
        case hyper_unsigned_k:
        case long_real_k:
        case fixed_array_k:
        case open_array_k:
        case fixed_string_zero_k:
        case open_string_zero_k:
        case pointer_k:
        case routine_ptr_k:
            return false;

        case record_k:
            return true;

        case user_marshalled_k:
            return
                record_type (tp->type_structure.user_marshalled.user_type);

        case named_k:
            return 
                record_type(tp->type_structure.named.resolution);

        default:
            unimp_type_exit (tp->kind, "record_type");
            return 0;

        }
    /*lint -unreachable */
}


#ifndef __STDC__
void CSPELL_epv_type_and_vars (fid, if_name, bp, mmmv)
FILE *fid;
NAMETABLE_id_t if_name;
binding_t *bp;
boolean mmmv;
#else
void CSPELL_epv_type_and_vars (FILE *fid, NAMETABLE_id_t if_name, binding_t *bp, boolean mmmv)
#endif
{

    /* emits the declaration of the client stub's epv type and
         an extern declaration for a variable of this type */

    NAMETABLE_id_t     type_name, var_name;
    routine_t          *rp;

    fprintf (fid, "typedef struct ");
    type_name = NAMETABLE_add_derived_name (if_name, "%s$epv_t");
    spell_name (fid, type_name);
    fprintf (fid, " {\n");
    for (; bp; bp = bp->next)
        if (bp->kind == routine_k) {
            rp = &bp->binding->routine;
            CSPELL_func_decl (fid, rp->result->type, bp->name, rp->parameters);
        }
    fprintf (fid, "} ");
    spell_name (fid, type_name);
    fprintf (fid, ";\n");

    var_name = NAMETABLE_add_derived_name (if_name, "%s$client_epv");
    fprintf (fid, "globalref ");
    spell_name (fid, type_name);
    fprintf (fid, " ");
    spell_name (fid, var_name);
    fprintf (fid, ";\n");

    if (!mmmv) {
        var_name = NAMETABLE_add_derived_name (if_name, "%s$manager_epv");
        fprintf (fid, "globalref ");
        spell_name (fid, type_name);
        fprintf (fid, " ");
        spell_name (fid, var_name);
        fprintf (fid, ";\n");
        }
}


#ifndef __STDC__
static
void CSPELL_server_epv (fid, if_name, mmmv)
FILE *fid;
NAMETABLE_id_t if_name;
boolean mmmv;
#else
void CSPELL_server_epv (FILE *fid, NAMETABLE_id_t if_name, boolean mmmv)
#endif
{
    /* emits an extern declaration for server stub's epv */

    if (mmmv)
        fprintf (fid, "globalref rpc_$generic_epv_t ");
    else
        fprintf (fid, "globalref rpc_$epv_t ");
    spell_name (fid, NAMETABLE_add_derived_name(if_name, "%s$server_epv"));
    fprintf (fid, ";\n");
}

void CSPELL_name_declarator (fid, name, pointer, array, function, paramlist, pointer_elements)
FILE *fid;
NAMETABLE_id_t name;
boolean pointer;
array_dimension_t *array;
boolean function;
parameter_t *paramlist;
boolean pointer_elements;
{
    char *exp;
    long i;

    NAMETABLE_id_to_string (name, &exp);

    if (array ? pointer_elements : pointer)
        fprintf (fid, "*");

    if (array && pointer)
        fprintf (fid, "(*");

    if (function) {
        fprintf (fid, "(*%s)", exp);
        fprintf (fid, "\n#ifdef __STDC__\n");
            CSPELL_parameter_list (fid, paramlist);
        fprintf (fid, "\n#else\n");
            fprintf (fid, "()");
        fprintf (fid, "\n#endif\n");
        }
    else
        fprintf (fid, "%s", exp);

    if (array && pointer)
        fprintf (fid, ")");

    if (array)
        for (i = 0; i < array->num_dims; i ++)
            if (array->dims[i] == 0)
                fprintf (fid, "[1]");
            else
                fprintf (fid, "[%ld]", array->dims[i]);
}

void CSPELL_typed_name (fid, type, name)
FILE   *fid;
type_t *type;
NAMETABLE_id_t name;
{
    boolean            pointer;
    array_dimension_t  *array_dimensions;
    boolean            function;
    parameter_t        *paramlist;
    boolean            pointer_elements;

    CSPELL_type_exp (
        fid,
        type,
        NAMETABLE_NIL_ID,
        &pointer,
        &array_dimensions,
        &function,
        &paramlist,
        &pointer_elements);
    fprintf (fid, " ");
    CSPELL_name_declarator (
        fid,
        name,
        pointer,
        array_dimensions,
        function,
        paramlist,
        pointer_elements);
}

void CSPELL_func_decl (fid, type, name, paramlist)
FILE   *fid;
type_t *type;
NAMETABLE_id_t name;
parameter_t *paramlist;
{
    boolean            pointer;
    array_dimension_t  *array_dimensions;
    boolean            function;
    parameter_t        *pl;
    boolean            pointer_elements;

    CSPELL_type_exp (
        fid,
        type,
        NAMETABLE_NIL_ID,
        &pointer,
        &array_dimensions,
        &function,
        &pl,
        &pointer_elements);
    fprintf (fid, " ");
    function = true;
    pl = paramlist;
    CSPELL_name_declarator (
        fid,
        name,
        pointer,
        array_dimensions,
        function,
        pl,
        pointer_elements);
    fprintf (fid, ";\n");
}


void CSPELL_var_decl (fid, type, name)
FILE   *fid;
type_t *type;
NAMETABLE_id_t name;
{
    CSPELL_typed_name (fid, type, name);
    fprintf (fid, ";\n");
}

void CSPELL_constant_val (fid, cp)
FILE       *fid;
constant_t *cp;
{
    char str[max_string_len];

    constant_val_to_string (cp, str);
    fprintf (fid, "%s", str);
}


void CSPELL_constant_def (fid, bp)
FILE      *fid;
binding_t *bp;
{
    fprintf (fid, "#define ");
    spell_name (fid, bp->name);
    fprintf (fid, " ");
    CSPELL_constant_val (fid, &bp->binding->constant);
    fprintf (fid, "\n");
}


static
#ifndef __STDC__
void CSPELL_field (fid, fp)
FILE    *fid;
field_t *fp;
#else
void CSPELL_field (FILE *fid, field_t *fp)
#endif
{
    CSPELL_var_decl (fid, fp->type, fp->name);
}


static
#ifndef __STDC__
void CSPELL_enumerators (fid, ep)
FILE      *fid;
binding_t *ep; /* list of enumeration name bindings */
#else
void CSPELL_enumerators (FILE *fid, binding_t *ep)
#endif
{
    boolean first = true;

    for (; ep; ep = ep->next) {
        if (first)
             first = false;
        else
             fprintf (fid, ",\n  ");
        spell_name (fid, ep->name);
        }
}



static
#ifndef __STDC__
void CSPELL_tags (fid, tgp)
FILE  *fid;
tag_t *tgp;
#else
void CSPELL_tags (FILE *fid, tag_t *tgp)
#endif
{
    boolean first = true;

    fprintf (fid, "/* case(s): ");
    for (; tgp; tgp = tgp->next_tag) {
        if (first)
            first = false;
        else
            fprintf (fid, ", ");
        CSPELL_constant_val (fid, &tgp->tag_value);
        };
    fprintf (fid, " */\n");
}


static
#ifndef __STDC__
void CSPELL_component_fields (fid, fp, name)
FILE *fid;
field_t *fp;
NAMETABLE_id_t name;
#else
void CSPELL_component_fields (FILE *fid, field_t *fp, NAMETABLE_id_t name)
#endif
{
    if (fp != NULL)
        if (fp->next_field != NULL) {
            /* multiple fields (from NIDL/Pascal): wrap in a struct */
            fprintf (fid, "struct {\n");
            for (; fp; fp = fp->next_field)
                CSPELL_field (fid, fp);
            fprintf (fid, "} ");
            spell_name (fid, name);
            fprintf (fid, ";\n");
            }
        else
            /* only one field: no need to wrap in a struct */
            CSPELL_field (fid, fp);
}

#ifndef __STDC__
void CSPELL_scalar_type_suffix (fid, tp)
FILE              *fid;
type_t            *tp;
#else
void CSPELL_scalar_type_suffix (FILE *fid, type_t *tp)
#endif
{

    /* prepend a 'u' for unsigned types */
    switch (tp->kind) {
        case small_bitset_k:
        case small_enumeration_k:
        case small_unsigned_k:
        case short_bitset_k:
        case short_enumeration_k:
        case short_unsigned_k:
        case long_bitset_k:
        case long_enumeration_k:
        case long_unsigned_k:
        case hyper_unsigned_k:
            fprintf (fid, "u");
            break;

        default:
            break;
        }

    switch (tp->kind) {
        case boolean_k:
            fprintf (fid, "boolean");
            break;

        case byte_k:
            fprintf (fid, "byte");
            break;

        case small_integer_k:
        case small_bitset_k:
        case small_enumeration_k:
        case small_unsigned_k:
            fprintf (fid, "small_int");
            break;

        case short_integer_k:
        case short_subrange_k:
        case short_bitset_k:
        case short_enumeration_k:
        case short_unsigned_k:
            fprintf (fid, "short_int");
            break;

        case long_integer_k:
        case long_subrange_k:
        case long_bitset_k:
        case long_enumeration_k:
        case long_unsigned_k:
            fprintf (fid, "long_int");
            break;

        case hyper_integer_k:
        case hyper_unsigned_k:
            fprintf (fid, "hyper_int");
            break;

        case character_k:
            fprintf (fid, "char");
            break;

        case short_real_k:
            fprintf (fid, "short_float");
            break;

        case long_real_k:
            fprintf (fid, "long_float");
            break;
        }
}


void CSPELL_type_exp (fid, tp, tn, pointer, array_dimensions, function, paramlist, pointer_elements)
FILE              *fid;
type_t            *tp;
NAMETABLE_id_t    tn;         /* [in] tag name */
boolean           *pointer;
array_dimension_t **array_dimensions;
boolean           *function;
parameter_t       **paramlist;
boolean           *pointer_elements;
{
    field_t           *fp;
    component_t       *cp;
    long               num_dims;
    array_index_t     *indexp;
    variant_t         *vp;
    long               i, ele_num_dims;
    array_dimension_t *element_dimensions;
    boolean           tmp_bool;
    array_index_t     *indicesp;

    *pointer = false;
    *array_dimensions = NULL;
    *function = false;
    *paramlist = NULL;
    *pointer_elements = false;

    if (tp->type_name != NAMETABLE_NIL_ID) {
        spell_name (fid, tp->type_name);
        return;
        }

    switch (tp->kind) {

        case named_k:
            spell_name (fid, tp->type_structure.named.name);
            break;

        case boolean_k:
        case byte_k:
        case small_integer_k:
        case small_bitset_k:
        case small_enumeration_k:
        case small_unsigned_k:
        case short_integer_k:
        case short_subrange_k:
        case short_bitset_k:
        case short_enumeration_k:
        case short_unsigned_k:
        case long_bitset_k:
        case long_enumeration_k:
        case long_integer_k:
        case long_subrange_k:
        case long_unsigned_k:
        case hyper_integer_k:
        case hyper_unsigned_k:
        case character_k:
        case short_real_k:
        case long_real_k:
            fprintf (fid, "ndr_$");
            CSPELL_scalar_type_suffix (fid, tp);
            break;

        case open_string_zero_k:
            fprintf (fid, "ndr_$char");
            *pointer = true;
            break;

        case fixed_string_zero_k:
            fprintf (fid, "ndr_$char");

            /* allocate enough space for an array_dimension_t descriptor for a 1-d array */
            *array_dimensions = 
                (array_dimension_t *) alloc (sizeof (long) + sizeof(long));
            /* store dimension data */
            (*array_dimensions)->num_dims = 1;
            (*array_dimensions)->dims[0] =
                cardinality (
                    tp->type_structure.fixed_string_zero.index->
                        type->type_structure.subrange);
            break;

        case drep_k:
            fprintf (fid, "rpc_$drep_t");
            break;

        case handle_k:
            fprintf (fid, "handle_t");
            break;

        case fixed_array_k:
        case open_array_k:
            /* prepare to learn dimensions of the element type,
               i.e. if elements are stringzs */
            element_dimensions = NULL;
            CSPELL_type_exp (
                fid,
                (tp->kind == open_array_k)
                    ? tp->type_structure.open_array.elements
                    : tp->type_structure.fixed_array.elements,
                tn,
                pointer_elements,
                &element_dimensions,
                function,
                paramlist,
                &tmp_bool);
            ele_num_dims =
                (element_dimensions == NULL)
                    ? 0
                    : element_dimensions->num_dims;

            /* count the dimensions: one extra for open arrays
               plus element type's dimensions */
            num_dims = (tp->kind == open_array_k) ? 1 : 0;
            num_dims += ele_num_dims;
            indicesp = (tp->kind == open_array_k)
                       ? tp->type_structure.open_array.indices
                       : tp->type_structure.fixed_array.indices;
            for (indexp = indicesp; indexp; indexp = indexp->next)
                num_dims++;

            /* allocate enough space for an array_dimension_t descriptor */
            *array_dimensions =
                (array_dimension_t *) alloc ((int) ((num_dims+1)*sizeof (long)));
            (*array_dimensions)->num_dims = num_dims;

            /* fill dimension structure with dimension sizes */
            if (tp->kind == open_array_k) {
                (*array_dimensions)->dims[0] = 0;
                num_dims = 1;
                }
            else
                num_dims = 0;
            for (indexp = indicesp; indexp; indexp = indexp->next)
                (*array_dimensions)->dims[num_dims++]
                    = cardinality (indexp->type->type_structure.subrange);

            for (i = 0; i < ele_num_dims; )
                (*array_dimensions)->dims[num_dims++] = 
                    element_dimensions->dims[i++];

            if (element_dimensions != NULL)
                free ((char *)element_dimensions);
            break;

        case record_k:
            fprintf (fid, "struct ");
            spell_name (fid, tn);
            fprintf (fid, " {\n");
            for (fp = tp->type_structure.record.fields; fp; fp = fp->next_field)
                CSPELL_field (fid, fp);
            if ((vp = tp->type_structure.record.variant) != NULL) {
                CSPELL_type_exp (fid, vp->tag_type, tn, pointer, array_dimensions, function, paramlist, pointer_elements);
                fprintf (fid, " ");
                spell_name (fid, vp->tag_id);
                fprintf (fid, ";\nunion {\n");
                for (cp = vp->components; cp; cp = cp->next_component) {
                    CSPELL_tags (fid, cp->tags);
                    CSPELL_component_fields (fid, cp->fields, cp->label);
                    }
                fprintf (fid, "} ");
                spell_name (fid, vp->label);
                fprintf (fid, ";\n");
            }
            fprintf(fid, "}");
            break;

        case pointer_k:
            CSPELL_type_exp (fid, tp->type_structure.pointer.pointee, tn, pointer, array_dimensions, function, paramlist, pointer_elements);
            *pointer = true;
            break;

        case user_marshalled_k:
            CSPELL_type_exp (fid, tp->type_structure.user_marshalled.user_type, tn, pointer, array_dimensions, function, paramlist, pointer_elements);
            break;

        case void_k:
            fprintf (fid, "void");
            break;

        case routine_ptr_k:
            CSPELL_type_exp (fid, tp->type_structure.routine_ptr.result->type, tn, pointer, array_dimensions, function, paramlist, pointer_elements);
            pointerize_routine (&tp->type_structure.routine_ptr);
            *function = true;
            *paramlist = tp->type_structure.routine_ptr.parameters;
            break;

        case univ_ptr_k:
            fprintf(fid, "void *");
            break;

        default:
            unimp_type_exit (tp->kind, "CSPELL_type_exp");
            break;

        };
}

void CSPELL_type_exp_simple (fid, tp)
FILE              *fid;
type_t            *tp;
{
    boolean            pointer;
    array_dimension_t *array_dimensions;
    boolean            function;
    parameter_t        *paramlist;
    boolean            pointer_elements;

    CSPELL_type_exp (fid, tp, NAMETABLE_NIL_ID, &pointer, &array_dimensions, &function, &paramlist, &pointer_elements);
}


void CSPELL_type_def (fid, bp)
FILE      *fid;
binding_t *bp;
{
    type_t             *tp;
    type_t             *spelled_tp;
    boolean            pointer;
    array_dimension_t *array_dimensions;
    boolean            function;
    parameter_t        *paramlist;
    boolean            pointer_elements;

    type_t             *pointee;
    type_t             *base_tp;
    binding_t          *ec;
    unsigned long      const_val;
    char               *cast;
    NAMETABLE_id_t     base_name;
    NAMETABLE_id_t     type_name;

    tp = &bp->binding->type;
    type_name = bp->name;

    spelled_tp = 
        (tp->kind == user_marshalled_k)
        ? tp->type_structure.user_marshalled.user_type
        : tp;

    switch (spelled_tp->kind) {

        case record_k:
            fprintf (fid, "typedef struct ");
            spell_name (fid, type_name);
            fprintf (fid, " ");
            spell_name (fid, type_name);
            fprintf (fid, ";\n");
            break;

        case pointer_k:
            /*
             if the type definition has the form  "xp = ^x" and x is the name
             of a record type then emit "typedef struct x *xp;" instead of
             "typedef x *xp"
            */
            fprintf (fid, "typedef ");
            pointee = spelled_tp->type_structure.pointer.pointee;
            if (  (pointee->type_name != NAMETABLE_NIL_ID)
               && record_type (pointee))
                fprintf (fid, "struct ");
            break;

        case short_bitset_k:
        case long_bitset_k:
            /*
                if the type definition is "set of <type>" where <type> is an enumerated type
                and 'mask' constants have not been generated for the enumeration constants
                then do so here
            */
            base_tp = tp->type_structure.bitset.base_type;
            if (base_tp->kind == named_k) {
                base_name = base_tp->type_structure.named.name;
                base_tp =   base_tp->type_structure.named.resolution;
                }
            else
                base_name = base_tp->type_name;

            if ((base_tp->kind == short_enumeration_k || base_tp->kind == long_enumeration_k)
            && !base_tp->type_structure.enumeration.masks_genned) {

                base_tp->type_structure.enumeration.masks_genned = true;
                const_val = 1;

                if (base_tp->kind == short_enumeration_k)
                    cast = "ndr_$ushort_int";
                else
                    cast = "ndr_$ulong_int";

                if (base_name != NAMETABLE_NIL_ID) {
                    fprintf (fid, "#define ");
                    spell_name (fid, base_name);
                    fprintf (fid, "_mask_none (%s) 0\n", cast);
                    }

                for (ec = base_tp->type_structure.enumeration.enum_constants; ec; ec = ec->next) {
                    fprintf (fid, "#define ");
                    spell_name (fid, ec->name);
                    fprintf (fid, "_mask (%s) 0x%lx\n", cast, const_val);
                    const_val *= 2;
                    }
                }
            fprintf (fid, "typedef ");
            break;

        case short_enumeration_k:
        case long_enumeration_k:
            /* convert short enum type defs like nidl/pascal's "foo = (a,b,c)" into
                     #define a ((ndr_$ushort_int) 0)
                     #define b ((ndr_$ushort_int) 1)
                     #define c ((ndr_$ushort_int) 2)
                     #typedef ndr_$ushort_int foo;    <== this is handled in CSPELL_type_exp
            and convert long enum type defs like nidl/C's "enum {a,b,c}" into
                     #define a ((ndr_$ulong_int) 0)
                     #define b ((ndr_$ulong_int) 1)
                     #define c ((ndr_$ulong_int) 2)
                     #typedef ndr_$ulong_int foo;    <== this is handled in CSPELL_type_exp
            */
            const_val = 0;
            for (ec = tp->type_structure.enumeration.enum_constants; ec; ec = ec->next) {
                fprintf (fid, "#define ");
                spell_name (fid, ec->name);
                fprintf (fid,
                    " ((ndr_$u%s_int) 0x%lx)\n",
                    (spelled_tp->kind == short_enumeration_k) ? "short" : "long",
                    const_val++);
                }
            fprintf (fid, "typedef ");
            break;

        default:
            fprintf (fid, "typedef ");
            break;

        }

    CSPELL_type_exp (fid, tp, type_name, &pointer, &array_dimensions, &function, &paramlist, &pointer_elements);
    if (spelled_tp->kind != record_k) {
        fprintf (fid, " ");
        CSPELL_name_declarator (fid, bp->name, pointer, array_dimensions, function, paramlist, pointer_elements);
        }
    fprintf (fid, ";\n");

    /* declare the "bind" and "unbind" routines as extern's for [handle] types */
    if ((tp->is_handle) && (tp->kind != handle_k)) {

        fprintf (fid, "#ifdef __STDC__\n");

            fprintf (fid, "handle_t ");
            spell_name (fid, bp->name);
            fprintf (fid, "_bind(");
            spell_name (fid, bp->name);
            fprintf (fid, " h);\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_unbind(");
            spell_name (fid, bp->name);
            fprintf (fid, " uh, handle_t h);\n");

        fprintf (fid, "#else\n");

            fprintf (fid, "handle_t ");
            spell_name (fid, bp->name);
            fprintf (fid, "_bind();\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_unbind();\n");

        fprintf (fid, "#endif\n");

        }

    /* declare the "from_xmit_rep", "to_xmit_rep", "free_xmit_rep", and "free"
         routines as extern's for types with the [transmit_as()] attribute */
    if (tp->kind == user_marshalled_k) {

        fprintf (fid, "#ifdef __STDC__\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_from_xmit_rep(");
            CSPELL_type_exp_simple (fid, tp->type_structure.user_marshalled.xmit_type);
            fprintf (fid, " *xmit_object, ");
            spell_name (fid, bp->name);
            fprintf (fid, " *object);\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_to_xmit_rep(");
            spell_name (fid, bp->name);
            fprintf (fid, " object, ");
            CSPELL_type_exp_simple (fid, tp->type_structure.user_marshalled.xmit_type);
            fprintf (fid, " **xmit_object);\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_free(");
            spell_name (fid, bp->name);
            fprintf (fid, " object);\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_free_xmit_rep(");
            CSPELL_type_exp_simple (fid, tp->type_structure.user_marshalled.xmit_type);
            fprintf (fid, " *xmit_object);\n");

        fprintf (fid, "#else\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_from_xmit_rep();\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_to_xmit_rep();\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_free();\n");

            fprintf (fid, "void ");
            spell_name (fid, bp->name);
            fprintf (fid, "_free_xmit_rep();\n");

        fprintf (fid, "#endif\n");
        }
}


type_t *pointerize_type (tp)
type_t      *tp;
{
    type_t    *new_tp;
    type_kind focus_kind;

    switch (tp->kind) {
        case named_k:
            focus_kind = tp->type_structure.named.resolution->kind;
            break;
        default:
            focus_kind = tp->kind;
            break;
        }

    switch (focus_kind) {

        case fixed_array_k:
        case open_array_k:
        case fixed_string_zero_k:
        case open_string_zero_k:
            /* these types are already pointers to C's way of thinking */
            return tp;

        default:
            /* create new pointer type node */
            new_tp = gen_type_node (pointer_k);
            new_tp->type_structure.pointer.pointee = tp;

            /* propagate pointee's attributes */
            new_tp->xmit_type   = tp->xmit_type;

            return new_tp;
        }
    /*lint -unreachable*/
}


void pointerize_routine (rp)
routine_t *rp;
{
    parameter_t *pp;

    if (!rp->pointerized) {
        rp->pointerized = true;
        for (pp = rp->parameters; pp; pp = pp->next_param)
            if (pp->out || pp->ref) {
                pp->type = pointerize_type (pp->type);
            }
        }
}


void CSPELL_parameter_list (fid, pp)
FILE        *fid;
parameter_t *pp;
{
    boolean            first = true;

    fprintf (fid, " (\n");
    if (pp)
        for (; pp; pp = pp->next_param) {
            if (first)
                first = false;
            else
                fprintf (fid, ",\n");
            fprintf (fid, "  /* [");
            if (pp->in)
                if (pp->out)
                    fprintf (fid, "in, out");
                else
                    fprintf (fid, "in");
            else
                fprintf  (fid, "out");
            fprintf (fid, "] */");
            CSPELL_typed_name (fid, pp->type, pp->name);
            }
    else
        fprintf (fid, "void");
    fprintf (fid, ")");
}


static
#ifndef __STDC__
void CSPELL_routine_def (fid, bp)
FILE      *fid;
binding_t *bp;
#else
void CSPELL_routine_def (FILE *fid, binding_t *bp)
#endif
{
    routine_t          *rp;

    rp = &bp->binding->routine;

    pointerize_routine (rp);

    fprintf (fid, "extern  ");
    CSPELL_type_exp_simple (fid, rp->result->type);
    fprintf (fid, " ");
    spell_name (fid, bp->name);
    
    fprintf (fid, "\n#ifdef __STDC__\n");

        CSPELL_parameter_list (fid, rp->parameters);
        fprintf (fid, ";\n");

    fprintf (fid, "#else\n");

        fprintf (fid, " ( );\n");

    fprintf (fid, "#endif\n");
}


#ifndef __STDC__
void CSPELL_bindings (fid, bp)
FILE      *fid;
binding_t *bp;
#else
void CSPELL_bindings (FILE *fid, binding_t *bp)
#endif
{
    for (; bp; bp = bp->next) {
        switch (bp->kind) {
            case constant_k:
                CSPELL_constant_def (fid, bp);
                break;
            case interface_k:
                break;
            case routine_k:
                CSPELL_routine_def (fid, bp);
                break;
            case type_k:
                CSPELL_type_def (fid, bp);
                break;
            default:
               fprintf(fid, "Unknown binding type - %lx - in CSPELL_bindings.\n", (long) bp->kind);
               abort();
            }
        }
}

static
#ifndef __STDC__
void CSPELL_interface_def (fid, bp)
FILE      *fid;
binding_t *bp;
#else
void CSPELL_interface_def (FILE *fid, binding_t *bp)
#endif
{
    interface_t *ifp;
    long        family;
    boolean     first;
    long        i;

    ifp = &bp->binding->interface;

    /* emit dcl of the binding info var for this interface
       if it has any routines */
    if (!ifp->local_only) {

        ifp->if_spec_name =
            NAMETABLE_add_derived_name (bp->name, "%s$if_spec");

        fprintf (fid, "static rpc_$if_spec_t ");
        spell_name (fid, ifp->if_spec_name);
        fprintf (fid, " = {\n");

        fprintf (fid, "  %d,\n", ifp->interface_version);

        fprintf (fid, "  {");
        first = true;
        for (family = 0; family < socket_$num_families; family++) {
            if (first)
                first = false;
            else
                fprintf (fid, ", ");
            fprintf (fid, "%d",ifp->well_known_ports[family]);
            }
        fprintf (fid, "},\n");

        fprintf (fid, "  %d,\n", ifp->op_count);

        fprintf (fid, "  {\n");
        fprintf (fid, "  0x%08.8lx,\n", ifp->interface_uuid.time_high);
        fprintf (fid, "  0x%04.4x,\n", ifp->interface_uuid.time_low);
        fprintf (fid, "  0,\n");
        fprintf (fid, "  0x%x,\n", ifp->interface_uuid.family);

        fprintf (fid, "  {");
        first = true;
        for (i = 0; i < 7; i++) {
            if (first)
                first = false;
            else
                fprintf (fid, ", ");
            fprintf (fid, "0x%x", ifp->interface_uuid.host[i]);
            }
        fprintf (fid, "}\n  }\n};\n");
        }
}

void CSPELL_gen_c_ins_file ( astp, fid, emit_env, mmmv)
binding_t *astp;
FILE      *fid;
boolean   emit_env;
boolean   mmmv;
{
    interface_t *ifp;
    import_t    *ip;
    NAMETABLE_id_t include_var_name;
    char        leaf_name [max_string_len];
    char        leaf_ext  [max_string_len];
    char        dir_name  [max_string_len];
            
    include_var_name = NAMETABLE_add_derived_name (astp->name, "%s_included");
    fprintf (fid, "#ifndef ");
    spell_name (fid, include_var_name);
    fprintf (fid, "\n#define ");
    spell_name (fid, include_var_name);
    fprintf (fid, "\n");

    ifp = &astp->binding->interface;

    fprintf (fid, "#include \"idl_base.h\"\n");

    for (ip = ifp->imports; ip; ip=ip->next_import) {
        parse_path_name (ip->file_name, dir_name, leaf_name, leaf_ext);
        if (emit_env)
            if (! contains_ev_ref(ip->file_name))
                 fprintf (fid, "#include \"$(%s.%s).h", leaf_name, leaf_ext);
            else
                if (strlen(dir_name) > 0)
                    fprintf(fid, "#include \"%s/%s.h", dir_name, leaf_name);
                else
                    fprintf(fid, "#include \"%s.h\"", leaf_name);
        else
            fprintf (fid, "#include \"%s.h", leaf_name);
        fprintf (fid, "\"\n");
        }

    CSPELL_interface_def (fid, astp);
    CSPELL_bindings (fid, ifp->exports);

    /* emit declarations of implicit binding variable and epv's */
    if (ifp->implicit_handle_var != NAMETABLE_NIL_ID) {
        fprintf (fid, "globalref ");
        CSPELL_type_exp_simple(fid, ifp->implicit_handle_type);
        fprintf(fid, " ");
        spell_name (fid, ifp->implicit_handle_var);
        fprintf (fid, ";\n");
        }

    if (!ifp->local_only) {
        CSPELL_epv_type_and_vars (fid, astp->name, ifp->exports, mmmv);
        CSPELL_server_epv (fid, astp->name, mmmv);
        }

    fprintf (fid, "#endif\n");
}
